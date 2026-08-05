#include "WiFie.h"

const char* ssid = "your_wifi_name";   //wifi名称
const char* password = "your_wifi_password";   //wifi密码

void WiFie_Init(void)
{
  WiFi.begin(ssid,password);

  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected");
}

