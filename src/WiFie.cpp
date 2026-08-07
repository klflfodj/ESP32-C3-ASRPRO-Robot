#include "WiFie.h"

const char* ssid = "your_wifi_name";   //wifi名称
const char* password = "your_wifi_password";   //wifi密码

// -----------------------
// WiFie 初始化函数
// -----------------------
// 任务功能：初始化 WiFi 连接，等待连接成功或超时
// -----------------------
// 连接时间超过 10 秒后，打印 WiFi 连接失败信息
// 连接成功后，打印 WiFi 连接成功信息
// -----------------------
void WiFie_Init(void)
{
  WiFi.begin(ssid,password);

  int timeout = 0;

  while(WiFi.status()!=WL_CONNECTED && timeout<20)
  {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  if(WiFi.status()==WL_CONNECTED)
  {
    Serial.println();
    Serial.println("WiFi Connected");
  }
  else
  {
    Serial.println();
    Serial.println("WiFi Connect Failed");
    WiFi.disconnect();
  }
}

