#include "rtos_layer.h"

#include "app/src/ui.h"
#include "wifi_hal.h"
#include "wifi_data.h"
#include "eye_expression_port.h"
#include "oled_ssd1306.h"

RobotState CurrentState = STATE_IDLE;

static wifi_hal_t wifi_hal;
static wifi_hal_config_t wifi_cfg;
static const uint32_t WIFI_HAL_CONNECT_TIMEOUT_MS = 5000;

String WeatherType = "Loading";  
String Temperature = "--";

bool rtos_layer_init()
{
    TimeMutex    = xSemaphoreCreateMutex();
    WeatherMutex = xSemaphoreCreateMutex();

    esp_task_wdt_init(6, true); // 6秒超时，超时触发 panic 重启
    Serial.println("Task WDT Started"); // 打印看门狗初始化成功信息

    xTaskCreate(TaskClock,"Clock",2048,NULL,3,NULL);
    xTaskCreate(TaskOLED,"OLED",2048,NULL,1,NULL);
    xTaskCreate(TaskASR,"ASR",4096,NULL,2,NULL);
    xTaskCreate(TaskWiFi,"WiFi",4096,NULL,4,NULL);

    return true;
}

// -----------------------
// OLED 任务函数
// -----------------------
// 任务功能：更新 OLED 显示，根据当前状态显示不同的信息
// -----------------------
void TaskOLED(void *pvParameters)
{
    esp_task_wdt_add(NULL);
    while (1)
    {
        switch (CurrentState)
        {
            case STATE_IDLE:
                EyeExpression_Update();
                esp_task_wdt_reset();
                vTaskDelay(pdMS_TO_TICKS(40));

                break;
            case STATE_WAKEUP:    
            case STATE_INFO:
                oled_show_info_page();
                esp_task_wdt_reset();
                vTaskDelay(pdMS_TO_TICKS(1000));

                break;
        }
    }
}

void TaskClock(void *pvParameters)
{
    esp_task_wdt_add(NULL);
    while (1)
    {
        xSemaphoreTake(TimeMutex,portMAX_DELAY);
        getLocalTime(&timeinfo, 100);
        xSemaphoreGive(TimeMutex);
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskWiFi(void *pvParameters)
{
    wifi_cfg.timeout_ms = WIFI_HAL_CONNECT_TIMEOUT_MS;

    wifi_hal_init(&wifi_hal, &wifi_cfg);

    const uint32_t backoff_table[] = 
    {
        1000,     // 第1次失败后 1秒
        10000,    // 第2次失败后 10秒
        30000,    // 第3次失败后 30秒
        60000,    // 第4次失败后 1分钟
        600000,   // 第5次失败后 10分钟
        1800000,  // 第6次失败后 30分钟
        3600000   // 第7次失败后 1小时
    };

    const uint8_t backoff_table_size = sizeof(backoff_table) / sizeof(backoff_table[0]);// 重试次数表大小

    uint8_t  netIndex  = 0;          // 轮内游标：当前试到码表第几个
    uint32_t cycle     = 0;          // 轮间档位：整圈失败次数，连上即清零
    bool     time_synced = false;     // 每次"新连上"只补发一次 NTP 校时
    bool     was_linked  = false;     // 上一轮循环链路是否处于已连接

    while (1)
    {
        wifi_hal_update(&wifi_hal);

        switch (wifi_hal_get_state(&wifi_hal))
        {
            case WIFI_HAL_CONNECTED:
            {
                was_linked = true;
                cycle      = 0;

                if (!time_synced)
                {
                    configTime(8 * 3600, 0, "ntp.aliyun.com");
                    time_synced = true;
                }
                
                if (WiFi.status() != WL_CONNECTED)
                {
                    Serial.printf("WiFi lost: %s, retry same AP\n",
                                  wifi_table[netIndex].ssid);
                    wifi_hal_reset(&wifi_hal);   // -> IDLE，下轮重连
                    time_synced = false;
                    was_linked  = false;              
                }
                break;
            }
            case WIFI_HAL_FAILED:
            {
                 Serial.printf("WiFi fail (%u/%u): %s\n",
                              (unsigned)(netIndex + 1),
                              (unsigned)wifi_table_size,
                              wifi_table[netIndex].ssid);
                wifi_hal_reset(&wifi_hal);       // -> IDLE，下轮由 IDLE 分支发起下一个
                netIndex++;
                if (netIndex >= wifi_table_size)
                {
                    // 一整圈都不行 -> 退避
                    netIndex = 0;

                    uint32_t delay_ms = (cycle < backoff_table_size)
                                        ? backoff_table[cycle]
                                        : backoff_table[backoff_table_size - 1];
                    if (cycle < backoff_table_size) cycle++;

                    Serial.printf("WiFi all %u APs failed, backoff %lums (cycle %lu)\n",
                                  (unsigned)wifi_table_size,
                                  (unsigned long)delay_ms,
                                  (unsigned long)cycle);

                    // 分片延时，顺手喂狗、顺便响应掉线
                    for (uint32_t t = 0; t < delay_ms; t += 500)
                    {
                        uint32_t chunk = (delay_ms - t > 500) ? 500 : (delay_ms - t);
                        vTaskDelay(pdMS_TO_TICKS(chunk));
                    }
                }
                break;
            }
            case WIFI_HAL_IDLE:
            case WIFI_HAL_CONNECTING:
            default:
                if (wifi_hal_get_state(&wifi_hal) == WIFI_HAL_IDLE)
                {
                    Serial.printf("WiFi connecting (%u/%u): %s\n",
                                  (unsigned)(netIndex + 1),
                                  (unsigned)wifi_table_size,
                                  wifi_table[netIndex].ssid);
                    wifi_hal_connect(&wifi_hal,
                                     wifi_table[netIndex].ssid,
                                     wifi_table[netIndex].password);
                }
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskASR(void *pvParameters)
{
    esp_task_wdt_add(NULL);
    uint32_t NoDataLoops = 0;      // 连续未收到数据的循环次数
    bool RxHintPrinted = false;    // 引脚提示只打印一次
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
            NoDataLoops = 0;

            cmd.trim();

            Serial.print("Recv:");
            Serial.println(cmd);

            // -----------------------
            // 唤醒词：获取天气、时间、日期、星期
            // -----------------------
            if(cmd == "WAKEUP")
            {
                CurrentState = STATE_WAKEUP;
                if(WiFi.status() == WL_CONNECTED)
                {
                    xSemaphoreTake(TimeMutex, portMAX_DELAY);
                    getLocalTime(&timeinfo, 100);
                    xSemaphoreGive(TimeMutex);

                    HTTPClient http;
                    http.setTimeout(3000);
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

                            // 天气数据：本任务写、TaskOLED 读，用 WeatherMutex 保护
                            // 注意：HTTP 请求放在锁外面，否则会阻塞 OLED 最长 3 秒
                            xSemaphoreTake(WeatherMutex, portMAX_DELAY);
                            WeatherType = TypeCN;
                            Temperature = Temp;
                            xSemaphoreGive(WeatherMutex);

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
                oled_clear();
                if(cmd == "TIME")
                {
                    xSemaphoreTake(TimeMutex,portMAX_DELAY);
                    getLocalTime(&timeinfo, 100);
                    ASRSerial.printf("%02d,%02d,\n", timeinfo.tm_hour, timeinfo.tm_min);
                    xSemaphoreGive(TimeMutex);
                }
                if(cmd == "WEATHER")
                {
                    // 天气类型：0-晴，1-多云，2-雨，3-阴，4-雪
                    // 温度：单位摄氏度
                    // 发送格式：天气类型,温度,
                    // 例如：0,25,
                    // 先加锁读到局部副本，避免发送途中被本任务的 WAKEUP 分支改写
                    xSemaphoreTake(WeatherMutex, portMAX_DELAY);
                    String weatherType = WeatherType;
                    String temperature = Temperature;
                    xSemaphoreGive(WeatherMutex);

                    int wCode = 0;
                    if(weatherType.indexOf("晴") >= 0) wCode = 0;
                    else if(weatherType.indexOf("云") >= 0) wCode = 1;
                    else if(weatherType.indexOf("雨") >= 0) wCode = 2;
                    else if(weatherType.indexOf("阴") >= 0) wCode = 3;
                    else if(weatherType.indexOf("雪") >= 0) wCode = 4;
                    ASRSerial.printf("%d,%s,\n", wCode, temperature.c_str());
                }
                if(cmd == "DATE")
                {
                    xSemaphoreTake(TimeMutex,portMAX_DELAY);
                    getLocalTime(&timeinfo, 100);
                    ASRSerial.printf("%02d,%02d,%02d,\n",
                        timeinfo.tm_year % 100,
                        timeinfo.tm_mon + 1,
                        timeinfo.tm_mday);
                    xSemaphoreGive(TimeMutex);
                }
                if(cmd == "WEEK")
                {
                    xSemaphoreTake(TimeMutex,portMAX_DELAY);
                    getLocalTime(&timeinfo, 100);
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
                oled_clear();
            }
        }
        else
        {
            NoDataLoops++;
            // 15 秒（1500 × 10ms）没收到任何字节：多半是 ASR 的 TX/RX 接反了
            if(!RxHintPrinted && NoDataLoops > 1500)
            {
                RxHintPrinted = true;
                Serial.println("ASR: no data for 15s - check ASR TX -> ESP32 RX pin (or swap RX/TX)");
            }
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}












