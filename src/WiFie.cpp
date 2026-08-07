#include "WiFie.h"

const char* ssid = "your_wifi_name";   //wifi名称
const char* password = "your_wifi_password";   //wifi密码

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
  }
}

