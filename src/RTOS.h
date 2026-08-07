#ifndef __RTOS_H
#define __RTOS_H

#include "OLED.h"

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern struct tm timeinfo;
extern HardwareSerial ASRSerial;
extern SemaphoreHandle_t TimeMutex;

void RTOS_Init(void);
void TaskColock(void *pvParameters);
void TaskASR(void *pvParameters);
void TaskOLED(void *pvParameters);
void TaskWiFie(void *pvParameters);

#endif
