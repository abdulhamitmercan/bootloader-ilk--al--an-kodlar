#ifndef TASK_API_H
#define TASK_API_H

#include "Arduino.h"

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
TaskHandle_t ModemTaskptr;

void setupTasks(TaskFunction_t task1code, TaskFunction_t task2code, TaskFunction_t task3code){
   xTaskCreatePinnedToCore(
    task1code,   /* Task function. */
    "Task1",     /* name of task. */
    5000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 1 */

  xTaskCreatePinnedToCore(
    task2code,   /* Task function. */
    "Task2",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task2,      /* Task handle to keep track of created task */
    0);          /* pin task to core 1 */

  xTaskCreatePinnedToCore(
    task3code,   /* Task function. */
    "Task3",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task3,      /* Task handle to keep track of created task */
    0);          /* pin task to core 1 */
}


#endif
