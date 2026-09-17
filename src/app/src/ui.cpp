#include "ui.h"

#include "rtos_layer/src/rtos_layer.h"

#include <Arduino.h>
#include <stdbool.h>

#include "oled_ssd1306.h"
#include "eye_expression_port.h"

// -----------------------
// UI 初始化
// -----------------------
// 说明：初始化 UI，包括 OLED 显示和动态表情
// -----------------------
bool ui_init()
{
    oled_ssd1306_init();

    EyeExpression_Init();
    EyeExpression_SetEmotion(EyeEmotion_Normal);
    EyeExpression_SetRandomBlink(true);
    EyeExpression_SetRandomLook(true);
    EyeExpression_SetRandomBehavior(true);
    
    return true;
}

// -----------------------
// OLED 显示信息页面
// -----------------------
// 说明：在 OLED 显示信息页面，包括日期、星期、时间、天气和温度
// -----------------------
void oled_show_info_page()
{
    struct tm ti;
    xSemaphoreTake(TimeMutex, portMAX_DELAY);
    ti = timeinfo;
    xSemaphoreGive(TimeMutex);

    String weatherType;
    String temperature;
    xSemaphoreTake(WeatherMutex, portMAX_DELAY);
    weatherType = WeatherType;
    temperature = Temperature;
    xSemaphoreGive(WeatherMutex);

    char dateBuf[16];
    char timeBuf[8];
    char weekBuf[8];

    snprintf(dateBuf, sizeof(dateBuf), "%04d.%02d.%02d",ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday);

    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d",ti.tm_hour, ti.tm_min);

    const char *cnweek[] = {"日", "一", "二", "三", "四", "五", "六"};
    snprintf(weekBuf, sizeof(weekBuf), "%s", cnweek[ti.tm_wday]);

    String bottomLine = weatherType + " " + temperature + "C";

    oled_clear();

    oled_set_font(u8g2_font_8x13_tr);
    oled.drawUTF8(2, 10, dateBuf);

    oled_set_font(u8g2_font_wqy12_t_gb2312);
    oled.drawUTF8(128 - oled.getUTF8Width(weekBuf) - 2, 10, weekBuf);

    oled_set_font(u8g2_font_logisoso28_tf);
    oled.drawUTF8((128 - oled.getUTF8Width(timeBuf)) / 2, 47, timeBuf);

    oled_set_font(u8g2_font_wqy12_t_gb2312);
    oled.drawUTF8(2, 62, bottomLine.c_str());

    oled_draw_weather_icon(104, 44, weatherType);
    oled_update();
}

// -----------------------
// OLED 显示天气图标
// -----------------------
// 说明：在指定位置显示天气图标
// -----------------------
void oled_draw_weather_icon(uint8_t x, uint8_t y, const String &weather)
{
    if (x >= 128 || y >= 64)
    {
        return;
    }

    bool sunny = weather.indexOf("晴") >= 0;
    bool rain  = weather.indexOf("雨") >= 0;
    bool snow  = weather.indexOf("雪") >= 0;

    if (sunny)
    {
        oled_draw_disc(x + 8, y + 8, 4);
        oled_draw_line(x + 8, y + 2, x + 8, y + 5);
        oled_draw_line(x + 8, y + 11, x + 8, y + 14);
        oled_draw_line(x + 2, y + 8, x + 5, y + 8);
        oled_draw_line(x + 11, y + 8, x + 14, y + 8);
        return;
    }

    oled_draw_disc(x + 4, y + 8, 4);
    oled_draw_disc(x + 8, y + 6, 5);
    oled_draw_disc(x + 12, y + 8, 4);
    oled_draw_box(x + 3, y + 8, 12, 4);

    if (rain)
    {
        oled_draw_line(x + 4, y + 14, x + 2, y + 19);
        oled_draw_line(x + 9, y + 14, x + 7, y + 19);
        oled_draw_line(x + 13, y + 14, x + 11, y + 19);
        return;
    }

    if (snow)
    {
        oled_draw_disc(x + 4, y + 16, 1);
        oled_draw_disc(x + 8, y + 15, 1);
        oled_draw_disc(x + 12, y + 16, 1);
    }
}












