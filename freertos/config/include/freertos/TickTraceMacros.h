#ifndef __ASSEMBLER__

#include "stdint.h"
uint32_t getCurrentSystemTimeFromWatchy();

char *getAndIncrementCurrentTickMessageBuffer();

#define traceTASK_INCREMENT_TICK(xTickCount)                                   \
  {                                                                            \
    typedef struct __attribute__((__packed__)) TickTraceData {                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      TickType_t newTickTime;                                                  \
      TaskHandle_t taskIdentifier;                                             \
    } TickTraceData_Fix;                                                       \
                                                                               \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      TickTraceData_Fix *currentMessage =                                      \
          (TickTraceData_Fix *) getAndIncrementCurrentTickMessageBuffer();     \
                                                                               \
      currentMessage->c_time = xTickCount;                                     \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->newTickTime = xTickCount + 1;                            \
    }                                                                          \
  }

#endif
