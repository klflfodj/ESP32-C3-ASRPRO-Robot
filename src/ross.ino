#include <Arduino.h>

#include "rtos_layer/src/rtos_layer.h"
#include "app/src/ui.h"

struct tm timeinfo;
HardwareSerial ASRSerial(1);
SemaphoreHandle_t TimeMutex;
SemaphoreHandle_t WeatherMutex;

void setup() 
{
  /* 外设功能初始化 */
  Serial.begin(115200);
  ASRSerial.begin(115200,SERIAL_8N1,5,4);

  delay(500);

  Serial.println("ESP32");

  if (!ui_init())
  {
    Serial.println("ui init failed");
    return;
  }

  delay(100);

  if (!rtos_layer_init())
  {
    Serial.println("rtos layer init failed");
    return;
  }

  delay(100);

  configTime(8 * 3600,0,"ntp.aliyun.com");

  delay(100);
}

void loop() 
{

}
