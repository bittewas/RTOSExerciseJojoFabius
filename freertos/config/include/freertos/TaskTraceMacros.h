#ifndef __ASSEMBLER__

#include "stdint.h"
uint32_t getCurrentSystemTimeFromWatchy();

#define traceTASK_CREATE(pxNewTCB)                                             \
  {                                                                            \
    typedef struct __attribute__((__packed__)) TaskTraceData {                 \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      TaskHandle_t taskIdentifier;                                             \
      TaskHandle_t affectedTask;                                               \
      TickType_t delay;                                                        \
    } TaskTraceData_Fix;                                                       \
                                                                               \
    extern volatile unsigned int GLOBAL_TASK_MESSAGE_INDEX;                    \
    extern volatile unsigned int GLOBAL_TASK_MESSAGE_ELEMENT_SIZE;             \
    extern volatile char *GLOBAL_TASK_MESSAGE_BUFFER;                          \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (GLOBAL_TASK_MESSAGE_BUFFER != 0 && GLOBAL_TASK_MESSAGE_INDEX < 1000) { \
                                                                               \
      TaskTraceData_Fix *currentMessage =                                      \
          (TaskTraceData_Fix *)(GLOBAL_TASK_MESSAGE_BUFFER +                   \
                                GLOBAL_TASK_MESSAGE_INDEX *                    \
                                    GLOBAL_TASK_MESSAGE_ELEMENT_SIZE);         \
                                                                               \
      currentMessage->messageType = 0;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->affectedTask = (TaskHandle_t)pxNewTCB;                   \
      currentMessage->delay = 0;                                               \
                                                                               \
      GLOBAL_TASK_MESSAGE_INDEX++;                                             \
    }                                                                          \
  }

#define traceTASK_CREATE_FAILED(pxNewTCB)                                      \
  {                                                                            \
    typedef struct __attribute__((__packed__)) TaskTraceData {                 \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      TaskHandle_t taskIdentifier;                                             \
      TaskHandle_t affectedTask;                                               \
      TickType_t delay;                                                        \
    } TaskTraceData_Fix;                                                       \
                                                                               \
    extern volatile unsigned int GLOBAL_TASK_MESSAGE_INDEX;                    \
    extern volatile unsigned int GLOBAL_TASK_MESSAGE_ELEMENT_SIZE;             \
    extern volatile char *GLOBAL_TASK_MESSAGE_BUFFER;                          \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (GLOBAL_TASK_MESSAGE_BUFFER != 0 && GLOBAL_TASK_MESSAGE_INDEX < 1000) { \
                                                                               \
      TaskTraceData_Fix *currentMessage =                                      \
          (TaskTraceData_Fix *)(GLOBAL_TASK_MESSAGE_BUFFER +                   \
                                GLOBAL_TASK_MESSAGE_INDEX *                    \
                                    GLOBAL_TASK_MESSAGE_ELEMENT_SIZE);         \
                                                                               \
      currentMessage->messageType = 0;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->affectedTask = (TaskHandle_t)pxNewTCB;                   \
      currentMessage->delay = 0;                                               \
                                                                               \
      GLOBAL_TASK_MESSAGE_INDEX++;                                             \
    }                                                                          \
  }

#define traceTASK_DELETE(pxTCB)                                                \
  {                                                                            \
    typedef struct __attribute__((__packed__)) TaskTraceData {                 \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      TaskHandle_t taskIdentifier;                                             \
      TaskHandle_t affectedTask;                                               \
      TickType_t delay;                                                        \
    } TaskTraceData_Fix;                                                       \
                                                                               \
    extern volatile unsigned int GLOBAL_TASK_MESSAGE_INDEX;                    \
    extern volatile unsigned int GLOBAL_TASK_MESSAGE_ELEMENT_SIZE;             \
    extern volatile char *GLOBAL_TASK_MESSAGE_BUFFER;                          \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (GLOBAL_TASK_MESSAGE_BUFFER != 0 && GLOBAL_TASK_MESSAGE_INDEX < 1000) { \
                                                                               \
      TaskTraceData_Fix *currentMessage =                                      \
          (TaskTraceData_Fix *)(GLOBAL_TASK_MESSAGE_BUFFER +                   \
                                GLOBAL_TASK_MESSAGE_INDEX *                    \
                                    GLOBAL_TASK_MESSAGE_ELEMENT_SIZE);         \
                                                                               \
      currentMessage->messageType = 0;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->affectedTask = (TaskHandle_t)pxTCB;                      \
      currentMessage->delay = 0;                                               \
                                                                               \
      GLOBAL_TASK_MESSAGE_INDEX++;                                             \
    }                                                                          \
  }

#endif
