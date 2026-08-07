#include "RTOS.h"
#include "OLED_DrawWeatherIcon.h"
#include "EyeExpression.h"
#include "WiFie.h"

String WeatherType = "Loading";       // 新增：天气类型英文
String Temperature = "--";

// -----------------------
// 机器人状态枚举
// -----------------------
// STATE_IDLE: 空闲状态，显示随机表情
// STATE_WAKEUP: 唤醒状态，显示大眼睛
// STATE_INFO: 信息状态，显示时间、天气、日期或星期
// -----------------------
enum RobotState
{
  STATE_IDLE,
  STATE_WAKEUP,
  STATE_INFO
};
RobotState CurrentState = STATE_IDLE;

// -----------------------
// RTOS 初始化函数
// -----------------------
void RTOS_Init(void)
{
  TimeMutex = xSemaphoreCreateMutex();

  xTaskCreate(TaskColock,"Clock",4096,NULL,3,NULL);
  xTaskCreate(TaskOLED,"OLED",4096,NULL,1,NULL);
  xTaskCreate(TaskASR,"ASR",4096,NULL,2,NULL);
  xTaskCreate(TaskWiFie,"WiFie",2048,NULL,4,NULL);
}

// -----------------------
// OLED 任务函数
// -----------------------
// 任务功能：更新 OLED 显示，根据当前状态显示不同的信息
// -----------------------
void TaskOLED(void *pvParameters)
{
    while (1)
    {
        switch (CurrentState)
        {
            case STATE_IDLE:
            case STATE_WAKEUP:
                EyeExpression_Update();
                vTaskDelay(pdMS_TO_TICKS(40));
                break;

            case STATE_INFO:
                OLED_ShowInfoPage();
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;
        }
    }
}


// -----------------------
// 时钟任务函数
// -----------------------
void TaskColock(void *pvParameters)
{ 
  while(1)
  {
    xSemaphoreTake(TimeMutex,portMAX_DELAY);
    getLocalTime(&timeinfo);
    xSemaphoreGive(TimeMutex);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// -----------------------
// ASR 任务函数
// -----------------------
// 任务功能：接收 ASR 模块的指令，并根据指令执行相应的操作
// -----------------------
void TaskASR(void *pvParameters)
{
    while(1)
    {
        // -----------------------
        // 接收 ASR 模块的指令
        // -----------------------
        // 读取 ASR 模块的串口数据，直到遇到换行符为止
        // 如果接收到的指令是 WAKEUP，则切换到唤醒状态，显示大眼睛
        // 如果接收到的指令是 TIME，则切换到信息状态，显示时间
        // 如果接收到的指令是 WEATHER，则切换到信息状态，显示天气
        // 如果接收到的指令是 DATE，则切换到信息状态，显示日期
        // 如果接收到的指令是 WEEK，则切换到信息状态，显示星期
        // 如果接收到的指令是 SLEEP，则切换回空闲状态，显示随机表情
        // -----------------------
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
                    getLocalTime(&timeinfo);

                    HTTPClient http;
                    http.begin("http://t.weather.itboy.net/api/weather/city/101010100");
                    int httpCode = http.GET();

                    if(httpCode == 200)
                    {
                        String payload = http.getString();

                        JsonDocument doc;
                        DeserializationError error = deserializeJson(doc, payload);
                        if(!error)
                        {
                            String TypeCN =doc["data"]["forecast"][0]["type"];
                            String Temp =doc["data"]["wendu"];

                            WeatherType = TypeCN;
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
                    // 天气类型：0-晴，1-多云，2-雨，3-阴，4-雪
                    // 温度：单位摄氏度
                    // 发送格式：天气类型,温度,
                    // 例如：0,25,
                    int wCode = 0;
                    if(WeatherType.indexOf("晴") >= 0) wCode = 0;
                    else if(WeatherType.indexOf("云") >= 0) wCode = 1;
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
                //OLED_Clear();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// -----------------------
// WiFie 任务函数
// -----------------------
// 任务功能：检查 WiFi 连接状态，如果断开则重新连接
// -----------------------
// 每 1 分钟检查一次 WiFi 连接状态
// -----------------------
void TaskWiFie(void *pvParameters)
{
  while(1)
  {
    if(WiFi.status() !=WL_CONNECTED)
    {
        Serial.println("WiFi Disconnected, Reconnecting...");
        WiFie_Init();
    }
    vTaskDelay(pdMS_TO_TICKS(60000)); // 每 1 分钟检查一次 WiFi 连接状态
  }
}
