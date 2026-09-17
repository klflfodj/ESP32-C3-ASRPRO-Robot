#pragma once

#include <Arduino.h>

bool ui_init();
void oled_draw_weather_icon(uint8_t x, uint8_t y, const String &weather);
void oled_show_info_page();




