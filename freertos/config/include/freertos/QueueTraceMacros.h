#ifndef __ASSEMBLER__

#include "stdint.h"
uint32_t getCurrentSystemTimeFromWatchy();

char *getAndIncrementCurrentQueueMessageBuffer();

#define traceQUEUE_RECEIVE(xQueue)                                             \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 0;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)xTicksToWait;                 \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_RECEIVE_FAILED(xQueue)                                      \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 &&  MONITOR_TASK != currentTaskHandle) {             \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 1;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)xTicksToWait;                 \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_RECEIVE_FROM_ISR(xQueue)                                    \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 2;                                         \
      currentMessage->c_time = xTaskGetTickCountFromISR();                     \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)0;                            \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_RECEIVE_FROM_ISR_FAILED(xQueue)                             \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 3;                                         \
      currentMessage->c_time = xTaskGetTickCountFromISR();                     \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)0;                            \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_SEND(xQueue)                                                \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 4;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)xTicksToWait;                 \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_SET_SEND(xQueue)                                            \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 8;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)0;                            \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_SEND_FAILED(xQueue)                                         \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 5;                                         \
      currentMessage->c_time = xTaskGetTickCount();                            \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)xTicksToWait;                 \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_SEND_FROM_ISR(xQueue)                                       \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 6;                                         \
      currentMessage->c_time = xTaskGetTickCountFromISR();                     \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)0;                            \
                                                                               \
    }                                                                          \
  }

#define traceQUEUE_SEND_FROM_ISR_FAILED(xQueue)                                \
  {                                                                            \
    typedef struct QueueTraceData {                                            \
      UBaseType_t messageType;                                                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      QueueHandle_t xQueue;                                                    \
      TickType_t xTicksToWait;                                                 \
      TaskHandle_t taskIdentifier;                                             \
    } QueueTraceData_Fix;                                                      \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      QueueTraceData_Fix *currentMessage =                                     \
          (QueueTraceData_Fix *) getAndIncrementCurrentQueueMessageBuffer();   \
                                                                               \
      currentMessage->messageType = 7;                                         \
      currentMessage->c_time = xTaskGetTickCountFromISR();                     \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->xQueue = xQueue;                                         \
      currentMessage->xTicksToWait = (TickType_t)0;                            \
                                                                               \
    }                                                                          \
  }

#endif
