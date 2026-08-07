#include "OLED.h"
#include "RTOS.h"
#include "WiFie.h"

struct tm timeinfo;
HardwareSerial ASRSerial(1);
SemaphoreHandle_t TimeMutex;

void setup() 
{
  /* 外设功能初始化 */
  Serial.begin(115200);
  ASRSerial.begin(115200,SERIAL_8N1,5,4);
  delay(1000);
  Serial.println("ESP32");
  WiFie_Init();
  configTime(8 * 3600,0,"ntp.aliyun.com");
  
  /* 初始化 OLED */
  OLED_Init();

  /* 初始化 RTOS */
  delay(100);
  RTOS_Init();
  
}

void loop() 
{

}
