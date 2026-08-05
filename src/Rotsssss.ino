// -----------------------
// 包含头文件
// -----------------------  
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "OLED_Ruan.h"
#include "EyeExpression.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// -----------------------
// 定义常量
// -----------------------
const char* ssid = "your_wifi_name";
const char* password = "your_wifi_password";

// -----------------------
// 定义全局变量
// -----------------------
struct tm timeinfo;
char* Week[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
String WeatherType = "Loading";       // 新增：天气类型英文
String Temperature = "--";

// -----------------------
// 定义硬件串口
// -----------------------
HardwareSerial ASRSerial(1);

// -----------------------
// 定义互斥锁
// -----------------------  
SemaphoreHandle_t TimeMutex;

// -----------------------
// 机器人状态
// -----------------------
enum RobotState
{
    STATE_IDLE,
    STATE_WAKEUP,
    STATE_INFO
};
RobotState CurrentState = STATE_IDLE;

// -----------------------
// WiFi 连接
// -----------------------
void WiFi_Connect()
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

// -----------------------
// 天气翻译
// -----------------------
String WeatherTranslate(String cn)
{
    if(cn=="晴") return "Sunny";
    if(cn=="多云") return "Cloudy";
    if(cn=="阴") return "Overcast";
    if(cn=="小雨") return "Rain";

    return "Unknown";
}

// -----------------------
// OLED 任务
// -----------------------
void TaskOLED(void *pvParameters)
{
    while(1)
    {
        switch(CurrentState)
        {
            // -----------------------
            // 空闲状态：显示随机表情
            // -----------------------
            case STATE_IDLE:
            EyeExpression_Update();

            vTaskDelay(pdMS_TO_TICKS(40));
            break;

            case STATE_WAKEUP:
            EyeExpression_Update();

            vTaskDelay(pdMS_TO_TICKS(40));
            break;

            case STATE_INFO:
            
            // -----------------------
            // 显示时间和天气信息
            // -----------------------
            xSemaphoreTake(TimeMutex,portMAX_DELAY);

            OLED_Print_Num(1,5, timeinfo.tm_hour,0,0);
            OLED_Print(1,7,":");
            OLED_Print_Num(1,8,timeinfo.tm_min,0,0);

            OLED_Print_Num(2,4,timeinfo.tm_year + 1900,0,0);
            OLED_Print(2,8,"-");
            OLED_Print_Num(2,9,timeinfo.tm_mon + 1,0,0);
            OLED_Print(2,10,"-");
            OLED_Print_Num(2,11,timeinfo.tm_mday,0,0);
            OLED_Print(2,14,Week[timeinfo.tm_wday]);

            xSemaphoreGive(TimeMutex);

            // -----------------------
            // 显示天气类型和温度
            // -----------------------  
            OLED_Print(3,1,(char*)WeatherType.c_str());
            OLED_Print(4,1,(char*)Temperature.c_str());
        
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        }
    }
}

// -----------------------
// 时钟任务
// -----------------------
void TaskColock(void *pvParameters)
{ 
    while(1)
    {
        xSemaphoreTake(TimeMutex,portMAX_DELAY);
        getLocalTime(&timeinfo);
        xSemaphoreGive(TimeMutex);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// -----------------------
// ASR 任务
// -----------------------
void TaskASR(void *pvParameters)
{
    while(1)
    {
        if(ASRSerial.available())
        {
            
            String cmd = ASRSerial.readStringUntil('\n');

            cmd.trim();

            Serial.print("Recv:");
            Serial.println(cmd);

            // -----------------------
            // 唤醒词：显示大眼睛
            // -----------------------
            if(cmd == "WAKEUP")
            {
                CurrentState = STATE_WAKEUP;
                if(WiFi.status() == WL_CONNECTED)
                {
                    // -----------------------
                    // 获取天气信息
                    // -----------------------
                    xSemaphoreTake(TimeMutex,portMAX_DELAY);
                    getLocalTime(&timeinfo);

                    HTTPClient http;
                    http.begin("http://t.weather.itboy.net/api/weather/city/101010100");
                    int httpCode = http.GET();

                    // -----------------------
                    // 解析天气信息
                    // -----------------------
                    if(httpCode == 200)
                    {
                        String payload = http.getString();

                        StaticJsonDocument<4096> doc;
                        DeserializationError error = deserializeJson(doc, payload);
                        if(!error)
                        {
                            String TypeCN =doc["data"]["forecast"][0]["type"];
                            String TypeEN =WeatherTranslate(TypeCN);
                            String Temp =doc["data"]["wendu"];

                            WeatherType = TypeEN;
                            Temperature = Temp;

                        }
                    }
                    http.end();
                }
            }

            // -----------------------
            // 查询数据指令：显示数据
            // -----------------------
            if(cmd == "TIME" || cmd == "WEATHER" || cmd == "DATE" || cmd == "WEEK")
            {
                CurrentState = STATE_INFO;
                OLED_Clear();
                if(cmd == "TIME")
                {
                    xSemaphoreTake(TimeMutex,portMAX_DELAY);
                    getLocalTime(&timeinfo);
                    ASRSerial.printf("%02d,%02d,\n", timeinfo.tm_hour, timeinfo.tm_min);
                    xSemaphoreGive(TimeMutex);
                }
                if(cmd == "WEATHER")
                {
                    int wCode = 0;
                    if(WeatherType.indexOf("晴") >= 0) wCode = 0;
                    else if(WeatherType.indexOf("多云") >= 0) wCode = 1;
                    else if(WeatherType.indexOf("雨") >= 0) wCode = 2;
                    else if(WeatherType.indexOf("阴") >= 0) wCode = 3;
                    else if(WeatherType.indexOf("雪") >= 0) wCode = 4;
                    ASRSerial.printf("%d,%s,\n", wCode, Temperature.c_str());
                }
                if(cmd == "DATE")
                {
                    xSemaphoreTake(TimeMutex,portMAX_DELAY);
                    getLocalTime(&timeinfo);
                    ASRSerial.printf("%02d,%02d,%02d,\n",
                        timeinfo.tm_year % 100,
                        timeinfo.tm_mon + 1,
                        timeinfo.tm_mday);
                    xSemaphoreGive(TimeMutex);
                }
                if(cmd == "WEEK")
                {
                    xSemaphoreTake(TimeMutex,portMAX_DELAY);
                    getLocalTime(&timeinfo);
                    ASRSerial.printf("%d,\n", timeinfo.tm_wday);
                    xSemaphoreGive(TimeMutex);
                }
            }
            
            // -----------------------
            // 休眠前：切换回随机表情
            // -----------------------
            if(cmd == "SLEEP")
            {
                CurrentState = STATE_IDLE;
                OLED_Clear();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// -----------------------
// 初始化任务
// -----------------------
void setup() 
{   
    Serial.begin(115200);
    ASRSerial.begin(115200,SERIAL_8N1,5,4);
    delay(1000);
    Serial.println("ESP32 + FreeRTOS + OLED Test");
    WiFi_Connect();
    configTime(8 * 3600,0,"ntp.aliyun.com");
    OLED_Init(); 
    EyeExpression_Init();
    EyeExpression_SetEmotion(EyeEmotion_Normal);
    EyeExpression_SetRandomBlink(true);
    EyeExpression_SetRandomLook(true);
    EyeExpression_SetRandomBehavior(true);   
    delay(100);
    TimeMutex = xSemaphoreCreateMutex();
    xTaskCreate(TaskColock,"Clock",4096,NULL,3,NULL);
    xTaskCreate(TaskOLED,"OLED",4096,NULL,2,NULL);
    xTaskCreate(TaskASR,"ASR",4096,NULL,1,NULL);
}
void loop() 
{

}

