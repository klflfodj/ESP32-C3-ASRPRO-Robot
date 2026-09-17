#include "wifi_hal.h"

#include <Arduino.h>
#include <WiFi.h>

#define WIFI_HAL_DEFAULT_TIMEOUT_MS 10000

// -----------------------
// wifi_hal 初始化
// -----------------------
// 说明：初始化 wifi_hal 实例，设置默认超时时间 
// -----------------------
// 参数：hal - wifi_hal 实例指针，cfg - 配置指针
// -----------------------
bool wifi_hal_init(wifi_hal_t* hal, const wifi_hal_config_t* cfg)
{
    if (hal == nullptr)
    {
        return false;
    }

    hal->cfg.timeout_ms = (cfg != NULL && cfg->timeout_ms > 0) ? cfg->timeout_ms : WIFI_HAL_DEFAULT_TIMEOUT_MS;
    hal->state = WIFI_HAL_IDLE;
    hal->started_ms = 0;

    return true;
}

// -----------------------
// wifi_hal 连接
// -----------------------
// 说明：连接 wifi 网络，设置 ssid 和 password
// -----------------------
// 参数：hal - wifi_hal 实例指针，ssid - wifi 网络 ssid，password - wifi 网络密码
// -----------------------
bool wifi_hal_connect(wifi_hal_t* hal, const char* ssid, const char* password)
{
    if (hal == NULL || ssid == NULL)
    {
        return false;
    }

    WiFi.disconnect();  
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    hal->state = WIFI_HAL_CONNECTING;  // 设置状态为连接中状态
    hal->started_ms = millis();         // 记录连接开始时间    

    return true;
}

// -----------------------
// wifi_hal 获取状态
// -----------------------
// 说明：获取当前 wifi_hal 实例的状态
// -----------------------
wifi_hal_status_t wifi_hal_get_state(wifi_hal_t* hal)
{
    return (hal != NULL) ? hal->state : WIFI_HAL_IDLE;
}

// -----------------------
// wifi_hal 更新状态
// -----------------------
// 说明：根据当前状态和连接结果更新 wifi_hal 状态机
// -----------------------
// 参数：hal - wifi_hal 实例指针
// -----------------------
void wifi_hal_update(wifi_hal_t* hal)
{
    if (hal == NULL || hal->state != WIFI_HAL_CONNECTING)
    {
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        hal->state = WIFI_HAL_CONNECTED;
        return;
    }

    if (millis() - hal->started_ms >= hal->cfg.timeout_ms)
    {
        WiFi.disconnect();
        hal->state = WIFI_HAL_FAILED;
    }
}

// -----------------------
// wifi_hal 重置状态
// -----------------------
// 说明：重置 wifi_hal 实例的状态为空闲状态
// -----------------------
// 参数：hal - wifi_hal 实例指针
// -----------------------
void wifi_hal_reset(wifi_hal_t* hal)
{
    if (hal == NULL)
    {
        return;
    }

    WiFi.disconnect();
    hal->state = WIFI_HAL_IDLE;
    hal->started_ms = 0;
}









