#include "task_scheduler.h"

void TaskScheduler::scheduleTask(void (*callback)(), uint32_t delay) {
  if (taskCount < MAX_TASKS) {
    tasks[taskCount].callback = callback;
    tasks[taskCount].invokeTime = millis() + delay;
    taskCount++;
  }
}

void TaskScheduler::checkTasks() {
  uint32_t currentTime = millis();

  for (uint8_t i = 0; i < taskCount; i++) {
    if (tasks[i].invokeTime <= currentTime) {
      tasks[i].callback();

      for (uint8_t j = i; j < taskCount - 1; j++) {
        tasks[j] = tasks[j + 1];
      }

      taskCount--;
      i--;
    }
  }
}

void TaskScheduler::begin() {
  timer = millis();
  taskCount = 0;
}
