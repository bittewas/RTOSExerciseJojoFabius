#include <Arduino.h>
#include <Display.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxEPD2_BW.h>
#include <Watchy.h>
#include <cstddef>
#include <cstring>
#include <esp_log.h>

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

struct QueueTraceData {
  TickType_t c_time;
  TickType_t timeStamp;
  QueueHandle_t xQueue;
  TickType_t xTicksToWait;
  TaskHandle_t taskIdentifier;
};

volatile unsigned int GLOBAL_QUEUE_MESSAGE_INDEX;
volatile unsigned int GLOBAL_QUEUE_MESSAGE_ELEMENT_SIZE =
    sizeof(QueueTraceData);
volatile char *GLOBAL_QUEUE_MESSAGE_BUFFER =
    (char *)malloc(1000 * sizeof(QueueTraceData));

GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display(WatchyDisplay{});
QueueHandle_t xQueueHandle;

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
  display.print("Johannes Bingel!\nFabius Mettner!");
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

void debugPrintTask(void *pvParameters) {
  const char *PRINTER_TAG = "QUEUE_DEBUG";
  unsigned int uiMessageIndex = 0;
  vTaskDelay(1000);
  while (uiMessageIndex < GLOBAL_QUEUE_MESSAGE_INDEX) {
    QueueTraceData *currentMessage =
        (QueueTraceData *)(GLOBAL_QUEUE_MESSAGE_BUFFER +
                           uiMessageIndex * GLOBAL_QUEUE_MESSAGE_ELEMENT_SIZE);
    ESP_LOGI(PRINTER_TAG, "%d;%d;%d;%d;%d", currentMessage->xQueue,
             currentMessage->c_time, currentMessage->timeStamp,
             currentMessage->taskIdentifier, currentMessage->xTicksToWait);
    uiMessageIndex++;
  }
  while (true) {
  }
}

extern "C" void app_main() {
  xQueueHandle = xQueueCreate(10, sizeof(void *));
  if (xQueueHandle == nullptr) {
    // TODO: Queue was not created!
  }

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

  xTaskCreate(producerTasks, "producerTask", 4096, (void *)producer1Params, 1,
              NULL);
  xTaskCreate(producerTasks, "producerTask", 4096, (void *)producer2Params, 1,
              NULL);
  xTaskCreate(producerTasks, "producerTask", 4096, (void *)producer3Params, 1,
              NULL);
  xTaskCreate(printingTask, "printer", 4096, &xQueueHandle, 1, NULL);
  xTaskCreate(debugPrintTask, "debugTask", 4096, NULL, configMAX_PRIORITIES - 1,
              NULL);
  // xTaskCreate(buttonWatch, "watch", 8192, NULL, 1, NULL);
  // xTaskCreate(clockCounter, "clock", 16384, NULL, 1, NULL);

  ESP_LOGI("app_main", "Starting scheduler from app_main()");
  vTaskStartScheduler();
  /* vTaskStartScheduler is blocking - this should never be reached */
  ESP_LOGE("app_main", "insufficient RAM! aborting");
  abort();
}
