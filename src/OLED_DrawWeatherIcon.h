#ifndef OLED_DRAW_WEATHER_ICON_H
#define OLED_DRAW_WEATHER_ICON_H

#include <Arduino.h>

void OLED_DrawWeatherIcon(uint8_t x, uint8_t y, const String &weather);
void OLED_ShowInfoPage();

#endif
