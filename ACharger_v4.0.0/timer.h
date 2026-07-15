#ifndef TIMER_H
#define TIMER_H
#include "uiScreen.h"
#include <Arduino.h>
#include "esp_system.h"
#include "esp_timer.h"

//  task  bozuldu yerine timer esp timer freertos değil taskta resetleme problemi aldık freertos task kullandığımızda taskı başlatmasak bile bozuldu taskın çalışmadığını da kanıtladık 
static esp_timer_handle_t lvglPeriodTimer = NULL;
static volatile bool lvglInProgress = false;
static portMUX_TYPE lvglGuardMux = portMUX_INITIALIZER_UNLOCKED;
static SemaphoreHandle_t xLvglMutex = NULL;




// 800ms periyodik callback
static void lvglPeriodCb(void* arg) {
  //  ikinci kez girmeyo
  bool skip = false;
  portENTER_CRITICAL(&lvglGuardMux);
  if (lvglInProgress) skip = true;
  else lvglInProgress = true;
  portEXIT_CRITICAL(&lvglGuardMux);
  if (skip) return;

  
  if (xLvglMutex && xSemaphoreTakeRecursive(xLvglMutex, 0) == pdTRUE) {
    lv_timer_handler();
    xSemaphoreGiveRecursive(xLvglMutex);
  }

  portENTER_CRITICAL(&lvglGuardMux);
  lvglInProgress = false;
  portEXIT_CRITICAL(&lvglGuardMux);
}


void startLvglTask() {
  if (!xLvglMutex) xLvglMutex = xSemaphoreCreateRecursiveMutex();

  if (!lvglPeriodTimer) {
    const esp_timer_create_args_t args = {
      .callback = &lvglPeriodCb,
      .arg = NULL,
      .dispatch_method = ESP_TIMER_TASK,   // Timer Service Task
      .name = "lvglPeriod800ms"
    };
    if (esp_timer_create(&args, &lvglPeriodTimer) == ESP_OK) {
      esp_timer_start_periodic(lvglPeriodTimer, 800000); // 800 ms = 800,000 us
    }
  } else {
    
    esp_timer_start_periodic(lvglPeriodTimer, 800000);
  }
}


void stopLvglTask() {
  if (lvglPeriodTimer) esp_timer_stop(lvglPeriodTimer);
}

void deinitLvglTask() {
  if (lvglPeriodTimer) {
    esp_timer_stop(lvglPeriodTimer);
    esp_timer_delete(lvglPeriodTimer);
    lvglPeriodTimer = NULL;
  }
  if (xLvglMutex) {
    vSemaphoreDelete(xLvglMutex);
    xLvglMutex = NULL;
  }
  portENTER_CRITICAL(&lvglGuardMux);
  lvglInProgress = false;
  portEXIT_CRITICAL(&lvglGuardMux);
}










#endif