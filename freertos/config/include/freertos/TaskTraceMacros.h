#ifndef __ASSEMBLER__

#include "stdint.h"
#pragma once
uint32_t getCurrentSystemTimeFromWatchy();
char *getAndIncrementCurrentTaskMessageBuffer();

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
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    TaskTraceData_Fix *currentMessage =                                        \
        (TaskTraceData_Fix *)getAndIncrementCurrentTaskMessageBuffer();        \
                                                                               \
    currentMessage->messageType = 0;                                           \
    currentMessage->c_time = xTaskGetTickCount();                              \
    currentMessage->taskIdentifier = currentTaskHandle;                        \
    currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();              \
    currentMessage->affectedTask = (TaskHandle_t)pxNewTCB;                     \
    currentMessage->delay = 0;                                                 \
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
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    TaskTraceData_Fix *currentMessage =                                        \
        (TaskTraceData_Fix *)getAndIncrementCurrentTaskMessageBuffer();        \
                                                                               \
    currentMessage->messageType = 1;                                           \
    currentMessage->c_time = xTaskGetTickCount();                              \
    currentMessage->taskIdentifier = currentTaskHandle;                        \
    currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();              \
    currentMessage->affectedTask = (TaskHandle_t)pxNewTCB;                     \
    currentMessage->delay = 0;                                                 \
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
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    TaskTraceData_Fix *currentMessage =                                        \
        (TaskTraceData_Fix *)getAndIncrementCurrentTaskMessageBuffer();        \
    currentMessage->messageType = 2;                                           \
    currentMessage->c_time = xTaskGetTickCount();                              \
    currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();              \
    currentMessage->taskIdentifier = currentTaskHandle;                        \
    currentMessage->affectedTask = (TaskHandle_t)pxTCB;                        \
    currentMessage->delay = 0;                                                 \
  }

#define traceTASK_DELAY()                                                      \
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
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    TaskTraceData_Fix *currentMessage =                                        \
        (TaskTraceData_Fix *)getAndIncrementCurrentTaskMessageBuffer();        \
    currentMessage->messageType = 3;                                           \
    currentMessage->c_time = xTaskGetTickCount();                              \
    currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();              \
    currentMessage->taskIdentifier = currentTaskHandle;                        \
    currentMessage->affectedTask = (TaskHandle_t)0;                            \
    currentMessage->delay = xTicksToDelay;                                     \
  }

#define traceTASK_DELAY_UNTIL(x)                                               \
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
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    TaskTraceData_Fix *currentMessage =                                        \
        (TaskTraceData_Fix *)getAndIncrementCurrentTaskMessageBuffer();        \
    currentMessage->messageType = 4;                                           \
    currentMessage->c_time = xTaskGetTickCount();                              \
    currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();              \
    currentMessage->taskIdentifier = currentTaskHandle;                        \
    currentMessage->affectedTask = (TaskHandle_t)0;                            \
    currentMessage->delay = (TickType_t)x;                                     \
  }

#define traceTASK_SWITCHED_IN()                                                \
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
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    TaskTraceData_Fix *currentMessage =                                        \
        (TaskTraceData_Fix *)getAndIncrementCurrentTaskMessageBuffer();        \
    currentMessage->messageType = 5;                                           \
    currentMessage->c_time = xTaskGetTickCount();                              \
    currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();              \
    currentMessage->taskIdentifier = currentTaskHandle;                        \
    currentMessage->affectedTask = currentTaskHandle;                          \
    currentMessage->delay = (TickType_t)0;                                     \
  }

#define traceTASK_SWITCHED_OUT()                                               \
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
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    TaskTraceData_Fix *currentMessage =                                        \
        (TaskTraceData_Fix *)getAndIncrementCurrentTaskMessageBuffer();        \
    currentMessage->messageType = 6;                                           \
    currentMessage->c_time = xTaskGetTickCount();                              \
    currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();              \
    currentMessage->taskIdentifier = currentTaskHandle;                        \
    currentMessage->affectedTask = currentTaskHandle;                          \
    currentMessage->delay = (TickType_t)0;                                     \
  }

#endif
