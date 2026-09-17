#pragma once

#include <stdbool.h>
#include <stdint.h>

// wifi_hal 状态枚举类型
typedef enum
{
    WIFI_HAL_IDLE = 0,          // 空闲状态
    WIFI_HAL_CONNECTING,        // 连接中状态
    WIFI_HAL_CONNECTED,         // 已连接状态
    WIFI_HAL_FAILED,            // 连接失败状态
} wifi_hal_status_t;

// wifi_hal 配置结构体
typedef struct
{
    uint32_t timeout_ms;        // 连接超时时间
} wifi_hal_config_t;

// wifi_hal 实例结构体
typedef struct
{
    wifi_hal_config_t cfg;      // 配置参数
    wifi_hal_status_t state;      // 当前状态
    uint32_t started_ms;        // 连接开始时间
} wifi_hal_t;

bool wifi_hal_init(wifi_hal_t* hal, const wifi_hal_config_t* cfg);
bool wifi_hal_connect(wifi_hal_t* hal, const char* ssid, const char* password);
wifi_hal_status_t wifi_hal_get_state(wifi_hal_t* hal);
void wifi_hal_update(wifi_hal_t* hal);
void wifi_hal_reset(wifi_hal_t* hal);



