#include <Arduino.h>
#include <Display.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxEPD2_BW.h>
#include <Watchy.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <esp_log.h>

#include "DS3232RTC.h"
#include "TimeLib.h"
#include "Wire.h"
#include "esp_cpu.h"
#include "esp_private/systimer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>

#define TICKS_PER_MS 1000

#define BOTTOM_LEFT 26
#define TOP_LEFT 25
#define BOTTOM_RIGHT 4
#define TOP_RIGHT 35
#define DISPLAY_CS 5
#define DISPLAY_RES 9
#define DISPLAY_DC 10
#define DISPLAY_BUSY 19

typedef volatile struct __attribute__((__packed__)) QueueTraceData {
  UBaseType_t messageType;
  TickType_t c_time;
  uint32_t timeStamp;
  QueueHandle_t xQueue;
  TickType_t xTicksToWait;
  TaskHandle_t taskIdentifier;
} QueueTraceData_Fix;

volatile unsigned int GLOBAL_QUEUE_MESSAGE_INDEX;
volatile unsigned int GLOBAL_QUEUE_MESSAGE_ELEMENT_SIZE =
    sizeof(QueueTraceData_Fix);
volatile char *GLOBAL_QUEUE_MESSAGE_BUFFER =
    (char *)malloc(500 * sizeof(QueueTraceData_Fix));

typedef struct __attribute__((__packed__)) TickTraceData {
  TickType_t c_time;
  uint32_t timeStamp;
  TickType_t newTickTime;
  TaskHandle_t taskIdentifier;
} TickTraceData_Fix;

volatile unsigned int GLOBAL_TICK_MESSAGE_INDEX;
volatile unsigned int GLOBAL_TICK_MESSAGE_ELEMENT_SIZE =
    sizeof(TickTraceData_Fix);
volatile char *GLOBAL_TICK_MESSAGE_BUFFER =
    (char *)malloc(1000 * sizeof(TickTraceData_Fix));

typedef struct __attribute__((__packed__)) TaskTraceData {
  UBaseType_t messageType;
  TickType_t c_time;
  uint32_t timeStamp;
  TaskHandle_t taskIdentifier;
  TaskHandle_t affectedTask;
  TickType_t delay;
} TaskTraceData_Fix;

unsigned int GLOBAL_TASK_MESSAGE_INDEX = 0;
unsigned int GLOBAL_TASK_MESSAGE_ELEMENT_SIZE = sizeof(TaskTraceData_Fix);
char *GLOBAL_TASK_MESSAGE_BUFFER =
    (char *)malloc(200 * sizeof(TaskTraceData_Fix));

unsigned char ERROR_FLAG = 0;

TaskHandle_t MONITOR_TASK = 0;

GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display(WatchyDisplay{});
QueueHandle_t xQueueHandle;
DS3232RTC realTimeClock(Wire);

uint32_t getCurrentSystemTimeFromWatchy() {
  return (uint32_t)esp_cpu_get_cycle_count();
}

char *getAndIncrementCurrentTaskMessageBuffer() {
  if (GLOBAL_TASK_MESSAGE_BUFFER == 0) {
    GLOBAL_TASK_MESSAGE_ELEMENT_SIZE = sizeof(TaskTraceData_Fix);
    GLOBAL_TASK_MESSAGE_BUFFER =
        (char *)malloc(200 * sizeof(TaskTraceData_Fix));
  }
  char *position = GLOBAL_TASK_MESSAGE_BUFFER +
                   GLOBAL_TASK_MESSAGE_INDEX * GLOBAL_TASK_MESSAGE_ELEMENT_SIZE;
  GLOBAL_TASK_MESSAGE_INDEX += 1;
  if (GLOBAL_TASK_MESSAGE_INDEX >= 200) {
    GLOBAL_TASK_MESSAGE_INDEX = 199;
    ERROR_FLAG |= 0x04;
  }
  return position;
}

void initDisplay(void *pvParameters) {
  ESP_LOGI("initDisplay", "initializing display");

  /* Setting gpio pin types, always necessary at the start. */
  pinMode(DISPLAY_CS, OUTPUT);
  pinMode(DISPLAY_RES, OUTPUT);
  pinMode(DISPLAY_DC, OUTPUT);
  pinMode(DISPLAY_BUSY, OUTPUT);
  pinMode(BOTTOM_LEFT, INPUT);
  pinMode(BOTTOM_RIGHT, INPUT);
  pinMode(TOP_LEFT, INPUT);
  pinMode(TOP_RIGHT, INPUT);

  /* Init the display. */
  display.epd2.initWatchy();
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(0, 90);
  display.print("Johannes Bingel!\nFabius Meeettner!");
  display.display(false);

  /* Delete the display initialization task. */
  ESP_LOGI("initDisplay", "finished display initialization");
  vTaskDelete(NULL);
}

void buttonWatch(void *pvParameters) {
  unsigned int refresh = 0;

  while (true) {
    if (digitalRead(BOTTOM_LEFT) == HIGH) {
      ESP_LOGI("buttonWatch", "Bottom Left pressed!");
      display.fillRoundRect(0, 150, 50, 50, 20, GxEPD_WHITE);
      display.display(true);
      vTaskDelay(500);
      display.fillRoundRect(0, 150, 50, 50, 20, GxEPD_BLACK);
      display.display(true);
      refresh++;
    } else if (digitalRead(BOTTOM_RIGHT) == HIGH) {
      ESP_LOGI("buttonWatch", "Bottom Right pressed!");
      display.fillRoundRect(150, 150, 50, 50, 20, GxEPD_WHITE);
      display.display(true);
      vTaskDelay(500);
      display.fillRoundRect(150, 150, 50, 50, 20, GxEPD_BLACK);
      display.display(true);
      refresh++;
    } else if (digitalRead(TOP_LEFT) == HIGH) {
      ESP_LOGI("buttonWatch", "Top Left pressed!");
      display.fillRoundRect(0, 0, 50, 50, 20, GxEPD_WHITE);
      display.display(true);
      vTaskDelay(500);
      display.fillRoundRect(0, 0, 50, 50, 20, GxEPD_BLACK);
      display.display(true);
      refresh++;
    } else if (digitalRead(TOP_RIGHT) == HIGH) {
      ESP_LOGI("buttonWatch", "Top Right pressed!");
      display.fillRoundRect(150, 0, 50, 50, 20, GxEPD_WHITE);
      display.display(true);
      vTaskDelay(500);
      display.fillRoundRect(150, 0, 50, 50, 20, GxEPD_BLACK);
      display.display(true);
      refresh++;
    } else if (refresh >= 10) {
      ESP_LOGI("buttonWatch", "Performing full refresh of display");
      display.display(false);
      refresh = 0;
    }
  }
}
void clockCounter(void *pvParameters) {
  const TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  BaseType_t xCounterJojo = 0;
  BaseType_t xCounterFabi = 0;

  while (true) {
    display.setTextColor(GxEPD_WHITE);
    display.fillRoundRect(0, 0, 50, 50, 0, GxEPD_BLACK);
    display.setCursor(0, 50);
    display.print(xCounterJojo);
    xCounterJojo++;

    ESP_LOGI("counter fabius", "Counter increased!");
    display.setTextColor(GxEPD_WHITE);
    display.fillRoundRect(100, 140, 100, 10, 0, GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(100, 150);
    display.printf("%2u", xCounterFabi);

    xCounterFabi += 1;

    display.display(true);

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

struct ProducerParameters {
  TickType_t xFrequency;
  QueueHandle_t xQueueHandle;
  char name;
};

void producerTasks(void *pvParameters) {
  const TickType_t xFrequency =
      ((ProducerParameters *)pvParameters)->xFrequency;
  const QueueHandle_t xQueueHandle =
      ((ProducerParameters *)pvParameters)->xQueueHandle;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  const char *xFunGeneratedString = "Hallo dies ist eine Nachricht von ";
  while (true) {

    char *messageToSend = (char *)malloc(
        strlen(xFunGeneratedString) * sizeof(char) + sizeof(char));
    memcpy(messageToSend, xFunGeneratedString, strlen(xFunGeneratedString));
    messageToSend[strlen(xFunGeneratedString)] =
        ((ProducerParameters *)pvParameters)->name;
    messageToSend[strlen(xFunGeneratedString) + 1] = '\0';

    if (xQueueSendToBack(xQueueHandle, &messageToSend, (TickType_t)0) !=
        pdPASS) {
      // TODO: Failed to post message ;(
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void printingTask(void *pvParameters) {
  const QueueHandle_t xQueueHandle = *(QueueHandle_t *)pvParameters;

  char *ppxNextStringToPrint;
  const char *PRINTER_TAG = "PRINTER AHHHHHHHHH";
  while (true) {
    if (xQueueReceive(xQueueHandle, &ppxNextStringToPrint, 10000) == pdPASS) {
      ESP_LOGI(PRINTER_TAG, "%s", ppxNextStringToPrint);
      free(ppxNextStringToPrint);
    }
  }
}

const BaseType_t TASK_COUNT = 4;
TaskHandle_t *taskList = new TaskHandle_t[TASK_COUNT];

void debugPrintTask(void *pvParameters) {
  vTaskDelay(1000);

  // Kill all created tasks
  for (BaseType_t i = 0; i < TASK_COUNT; i++) {
    ESP_LOGI("TASK_NAME", "%d;%s", taskList[i], pcTaskGetName(taskList[i]));
    if (taskList[i] != xTaskGetCurrentTaskHandle() && taskList[i] != NULL)
      vTaskDelete(taskList[i]);
  }

  ESP_LOGI("QUEUE_DEBUG",
           "Message Type;Queue;C Time;Timestamp;Task ID;Ticks to wait");
  unsigned int uiMessageIndex = 0;
  while (uiMessageIndex < GLOBAL_QUEUE_MESSAGE_INDEX) {
    QueueTraceData_Fix *currentMessage =
        (QueueTraceData_Fix *)(GLOBAL_QUEUE_MESSAGE_BUFFER +
                               uiMessageIndex *
                                   GLOBAL_QUEUE_MESSAGE_ELEMENT_SIZE);
    ESP_LOGI("QUEUE_DEBUG", "%d;%d;%d;%d;%d;%d", currentMessage->messageType,
             currentMessage->xQueue, currentMessage->c_time,
             currentMessage->timeStamp, currentMessage->taskIdentifier,
             currentMessage->xTicksToWait);
    uiMessageIndex++;
  }

  ESP_LOGI("TICK_DEBUG", "C Time;Timestamp;New Tick Time;Task ID");
  uiMessageIndex = 0;
  while (uiMessageIndex < GLOBAL_TICK_MESSAGE_INDEX) {
    TickTraceData_Fix *currentMessage =
        (TickTraceData_Fix *)(GLOBAL_TICK_MESSAGE_BUFFER +
                              uiMessageIndex *
                                  GLOBAL_TICK_MESSAGE_ELEMENT_SIZE);
    ESP_LOGI("TICK_DEBUG", "%d;%d;%d;%d", currentMessage->c_time,
             currentMessage->timeStamp, currentMessage->newTickTime,
             currentMessage->taskIdentifier);
    uiMessageIndex++;
  }

  ESP_LOGI("TASK_DEBUG",
           "Message Type;C Time;Timestamp;Task ID;Affected Task ID;Delay",
           GLOBAL_TASK_MESSAGE_INDEX);
  uiMessageIndex = 0;
  while (uiMessageIndex < GLOBAL_TASK_MESSAGE_INDEX) {
    TaskTraceData_Fix *currentMessage =
        (TaskTraceData_Fix *)(GLOBAL_TASK_MESSAGE_BUFFER +
                              uiMessageIndex *
                                  GLOBAL_TASK_MESSAGE_ELEMENT_SIZE);
    ESP_LOGI("TASK_DEBUG", "%d;%d;%d;%d;%d;%d", currentMessage->messageType,
             currentMessage->c_time, currentMessage->timeStamp,
             currentMessage->taskIdentifier, currentMessage->affectedTask,
             currentMessage->delay);
    uiMessageIndex++;
  }

  ESP_LOGI("FINISH_FLAG", "%x", ERROR_FLAG);
  vTaskDelete(NULL);
  while (true) {
  }
}

extern "C" void app_main() {
  while (GLOBAL_TASK_MESSAGE_BUFFER == 0) {
    GLOBAL_TASK_MESSAGE_ELEMENT_SIZE = sizeof(TaskTraceData_Fix);
    GLOBAL_TASK_MESSAGE_BUFFER =
        (char *)malloc(200 * sizeof(TaskTraceData_Fix));
    ESP_LOGI("MAIN", "Buffer created %d", GLOBAL_TASK_MESSAGE_BUFFER);
  }

  xQueueHandle = xQueueCreate(10, sizeof(void *));
  if (xQueueHandle == nullptr) {
    // TODO: Queue was not created!
  }

  // realTimeClock.begin();

  // ESP_LOGI("app_main", "%s", realTimeClock.get());

  xTaskCreate(debugPrintTask, "debugTask", 4096, NULL, configMAX_PRIORITIES - 1,
              &MONITOR_TASK);
  /* Only priorities from 1-25 (configMAX_PRIORITIES) possible. */
  /* Initialize the display first. */
  xTaskCreate(initDisplay, "initDisplay", 4096, NULL, configMAX_PRIORITIES - 1,
              NULL);

  ProducerParameters *producer1Params =
      (ProducerParameters *)malloc(sizeof(ProducerParameters));
  producer1Params->xFrequency = pdMS_TO_TICKS(100);
  producer1Params->xQueueHandle = xQueueHandle;
  producer1Params->name = 'a';
  ProducerParameters *producer2Params =
      (ProducerParameters *)malloc(sizeof(ProducerParameters));
  producer2Params->xFrequency = pdMS_TO_TICKS(200);
  producer2Params->xQueueHandle = xQueueHandle;
  producer2Params->name = 'b';
  ProducerParameters *producer3Params =
      (ProducerParameters *)malloc(sizeof(ProducerParameters));
  producer3Params->xFrequency = pdMS_TO_TICKS(300);
  producer3Params->xQueueHandle = xQueueHandle;
  producer3Params->name = 'c';

  xTaskCreate(producerTasks, "producerTask1", 4096, (void *)producer1Params, 1,
              (&taskList[0]));
  xTaskCreate(producerTasks, "producerTask2", 4096, (void *)producer2Params, 1,
              (&taskList[1]));
  xTaskCreate(producerTasks, "producerTask3", 4096, (void *)producer3Params, 1,
              (&taskList[2]));
  xTaskCreate(printingTask, "printer", 4096, &xQueueHandle, 1, (&taskList[3]));

  // xTaskCreate(buttonWatch, "watch", 8192, NULL, 1, NULL);
  // xTaskCreate(clockCounter, "clock", 16384, NULL, 1, NULL);

  ESP_LOGI("app_main", "Starting scheduler from app_main()");
  vTaskStartScheduler();
  /* vTaskStartScheduler is blocking - this should never be reached */
  ESP_LOGE("app_main", "insufficient RAM! aborting");
  abort();
}
