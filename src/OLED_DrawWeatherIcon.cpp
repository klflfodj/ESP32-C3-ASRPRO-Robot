#include "OLED_DrawWeatherIcon.h"
#include "OLED.h"
#include "RTOS.h"

extern String WeatherType;
extern String Temperature;

//-----------------------
// OLED 绘制天气图标
//-----------------------
// 说明：在指定坐标绘制天气图标
// 参数：x - 横坐标，y - 纵坐标，weather - 天气类型
//-----------------------
void OLED_DrawWeatherIcon(uint8_t x, uint8_t y, const String &weather)
{
    bool sunny = weather.indexOf("晴") >= 0;
    bool rain  = weather.indexOf("雨") >= 0;
    bool snow  = weather.indexOf("雪") >= 0;

    if (sunny)
    {
        OLED_DrawDisc(x + 8, y + 8, 4);
        OLED_DrawLine(x + 8, y + 2, x + 8, y + 5);
        OLED_DrawLine(x + 8, y + 11, x + 8, y + 14);
        OLED_DrawLine(x + 2, y + 8, x + 5, y + 8);
        OLED_DrawLine(x + 11, y + 8, x + 14, y + 8);
        return;
    }

    OLED_DrawDisc(x + 4, y + 8, 4);
    OLED_DrawDisc(x + 8, y + 6, 5);
    OLED_DrawDisc(x + 12, y + 8, 4);
    OLED_DrawBox(x + 3, y + 8, 12, 4);

    if (rain)
    {
        OLED_DrawLine(x + 4, y + 14, x + 2, y + 19);
        OLED_DrawLine(x + 9, y + 14, x + 7, y + 19);
        OLED_DrawLine(x + 13, y + 14, x + 11, y + 19);
        return;
    }

    if (snow)
    {
        OLED_DrawDisc(x + 4, y + 16, 1);
        OLED_DrawDisc(x + 8, y + 15, 1);
        OLED_DrawDisc(x + 12, y + 16, 1);
    }
}

//-----------------------
// OLED 显示信息页面
//-----------------------
// 说明：在OLED上显示当前时间、日期、星期、天气类型和温度
//-----------------------
void OLED_ShowInfoPage()
{
    struct tm ti;
    xSemaphoreTake(TimeMutex, portMAX_DELAY);
    ti = timeinfo;
    xSemaphoreGive(TimeMutex);

    char dateBuf[16];
    char timeBuf[8];
    char weekBuf[8];

    snprintf(dateBuf, sizeof(dateBuf), "%04d.%02d.%02d",ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday);

    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d",ti.tm_hour, ti.tm_min);

    const char *cnWeek[] = {"日", "一", "二", "三", "四", "五", "六"};
    snprintf(weekBuf, sizeof(weekBuf), "周%s", cnWeek[ti.tm_wday]);

    String bottomLine = WeatherType + " " + Temperature + "C";

    OLED_Clear();

    OLED_SetFont(u8g2_font_8x13_tr);
    oled.drawUTF8(2, 10, dateBuf);

    OLED_SetFont(u8g2_font_wqy12_t_gb2312);
    oled.drawUTF8(128 - oled.getUTF8Width(weekBuf) - 2, 10, weekBuf);

    OLED_SetFont(u8g2_font_logisoso28_tf);
    oled.drawUTF8((128 - oled.getUTF8Width(timeBuf)) / 2, 47, timeBuf);

    OLED_SetFont(u8g2_font_wqy12_t_gb2312);
    oled.drawUTF8(2, 62, bottomLine.c_str());

    OLED_DrawWeatherIcon(104, 44, WeatherType);
    OLED_Update();
}
