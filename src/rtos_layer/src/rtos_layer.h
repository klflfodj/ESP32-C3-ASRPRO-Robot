#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_task_wdt.h"

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <stdbool.h>

extern struct tm timeinfo;
extern HardwareSerial ASRSerial;
extern SemaphoreHandle_t TimeMutex;
extern SemaphoreHandle_t WeatherMutex;

// 天气数据：TaskASR 写、TaskOLED 读，访问时用 WeatherMutex 保护
extern String WeatherType;
extern String Temperature;


enum RobotState
{
    STATE_IDLE = 0,     // 空闲状态
    STATE_WAKEUP,       // 唤醒状态
    STATE_INFO,         // 信息状态
};
extern RobotState CurrentState;

bool rtos_layer_init();

void TaskColock(void *pvParameters);
void TaskOLED(void *pvParameters);
void TaskASR(void *pvParameters);
void TaskWiFi(void *pvParameters);


