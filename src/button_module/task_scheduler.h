#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <Arduino.h>

// adjust to your available RAM and needs
#define MAX_TASKS 100

struct Task {
  uint32_t invokeTime;
  void (*callback)();
};

class TaskScheduler {
public:
  void scheduleTask(void (*callback)(), uint32_t delay);
  void checkTasks();
  void begin();

private:
  int timer;
  Task tasks[MAX_TASKS];
  uint8_t taskCount = 0;
};

#endif