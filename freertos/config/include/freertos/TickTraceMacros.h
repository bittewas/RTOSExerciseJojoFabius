#ifndef __ASSEMBLER__

#include "stdint.h"
uint32_t getCurrentSystemTimeFromWatchy();

#define traceTASK_INCREMENT_TICK(xTickCount)                                   \
  {                                                                            \
    typedef struct __attribute__((__packed__)) TickTraceData {                 \
      TickType_t c_time;                                                       \
      uint32_t timeStamp;                                                      \
      TickType_t newTickTime;                                                  \
      TaskHandle_t taskIdentifier;                                             \
    } TickTraceData_Fix;                                                       \
                                                                               \
    extern volatile unsigned int GLOBAL_TICK_MESSAGE_INDEX;                    \
    extern volatile unsigned int GLOBAL_TICK_MESSAGE_ELEMENT_SIZE;             \
    extern volatile char *GLOBAL_TICK_MESSAGE_BUFFER;                          \
    extern TaskHandle_t MONITOR_TASK;                                          \
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();              \
                                                                               \
    if (GLOBAL_TICK_MESSAGE_BUFFER != 0 && GLOBAL_TICK_MESSAGE_INDEX < 1000 && \
        MONITOR_TASK != 0 && MONITOR_TASK != currentTaskHandle) {              \
                                                                               \
      TickTraceData_Fix *currentMessage =                                      \
          (TickTraceData_Fix *)(GLOBAL_TICK_MESSAGE_BUFFER +                   \
                                GLOBAL_TICK_MESSAGE_INDEX *                    \
                                    GLOBAL_TICK_MESSAGE_ELEMENT_SIZE);         \
                                                                               \
      currentMessage->c_time = xTickCount;                                     \
      currentMessage->taskIdentifier = currentTaskHandle;                      \
      currentMessage->timeStamp = getCurrentSystemTimeFromWatchy();            \
      currentMessage->newTickTime = xTickCount + 1;                            \
                                                                               \
      GLOBAL_TICK_MESSAGE_INDEX++;                                             \
    }                                                                          \
  }

#endif
