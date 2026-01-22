#include <freertos/FreeRTOS.h>
#include <freertos/list.h>

struct PipMutexHolder {
  SemaphoreHandle_t semaphore;
  TaskHandle_t currentSemaphoreHolder;
  BaseType_t currentSemaphoreHolderBasePriority;
  List_t waitingQueue;
};

typedef struct PipMutexHolder *PipMutexHolderHandle_t;

List_t pipMutexes;

PipMutexHolderHandle_t xCreateNewPipMutex() {
  if (!listLIST_IS_INITIALISED(pipMutexes)) {
    vListInitialise(pipMutexes);
  }

  PipMutexHolderHandle_t newMutex =
      (PipMutexHolderHandle_t)malloc(sizeof(PipMutexHolder));

  newMutex->semaphore = xSemaphoreCreateBinary();
  if (newMutex->semaphore == nullptr) {
    return nullptr;
  }
  newMutex->currentSemaphoreHolder = nullptr;

  xSemaphoreGive(newMutex->semaphore);

  vListInitialise(newMutex->waitingQueue);

  return newMutex;
}

void pip_take_semaphore(PipMutexHolderHandle_t semaphore,
                        TickType_t xTicksToWait) {
  UBaseType_t currentTaskPriority = uxTaskPriorityGet(nullptr);
  xSemaphoreTake(semaphore, xTicksToWait);
  if (xTicksToWait != portMAX_DELAY &&
      xSemaphoreGetMutexHolder(semaphore) != xTaskGetCurrentTaskHandle()) {
  }
}

void pip_give_semaphore(PipMutexHolderHandle_t semaphore) {}
