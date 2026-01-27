#include <freertos/FreeRTOS.h>
#include <vector>

struct PipMutexHolder {
  SemaphoreHandle_t semaphore;
  TaskHandle_t currentSemaphoreHolder;
  BaseType_t currentSemaphoreHolderBasePriority;
};

typedef struct PipMutexHolder *PipMutexHolderHandle_t;

PipMutexHolderHandle_t xCreateNewPipMutex() {
  PipMutexHolderHandle_t newMutex =
      (PipMutexHolderHandle_t)malloc(sizeof(PipMutexHolder));

  newMutex->semaphore = xSemaphoreCreateBinary();
  if (newMutex->semaphore == nullptr) {
    return nullptr;
  }
  newMutex->currentSemaphoreHolder = nullptr;

  xSemaphoreGive(newMutex->semaphore);

  return newMutex;
}

void pip_take_semaphore(PipMutexHolderHandle_t mutex, TickType_t xTicksToWait) {
  UBaseType_t currentTaskPriority = uxTaskPriorityGet(nullptr);
  xSemaphoreTake(mutex->semaphore, xTicksToWait);
  if (xTicksToWait != portMAX_DELAY) {
  }
}

void pip_give_semaphore(PipMutexHolderHandle_t mutex) {
  TaskHandle_t task = mutex->currentSemaphoreHolder;
  BaseType_t basePrio = mutex->currentSemaphoreHolderBasePriority;
  xSemaphoreGive(mutex->semaphore);
  vTaskPrioritySet(task, basePrio);
}
