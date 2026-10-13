#include <Arduino.h>

#include "rtos_layer/src/rtos_layer.h"
#include "app/src/ui.h"

// -----------------------
// ASR voice module on UART1 - pin mapping (ESP32 side)
// -----------------------
// ASR TX  -> ESP32 RX = GPIO16
// ASR RX  -> ESP32 TX = GPIO17
// If your two wires are the other way round, just swap these two numbers.
#define ASR_RX_PIN 16
#define ASR_TX_PIN 17

struct tm timeinfo;
HardwareSerial ASRSerial(1);
SemaphoreHandle_t TimeMutex;
SemaphoreHandle_t WeatherMutex;

void setup() 
{
  /* 外设功能初始化 */
  Serial.begin(115200);
  ASRSerial.begin(115200,SERIAL_8N1,ASR_RX_PIN,ASR_TX_PIN);
  Serial.printf("ASR UART1: RX=GPIO%d TX=GPIO%d @115200 8N1\n", ASR_RX_PIN, ASR_TX_PIN);

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
