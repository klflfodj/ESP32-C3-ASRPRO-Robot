#pragma once

#include <u8g2lib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;

/************ 初始化 ************/
bool oled_ssd1306_init();

/************ 刷新 ************/
void oled_update();

void oled_clear();

/************ 字体 ************/
void oled_set_font(const uint8_t *font);

/************ 文本 ************/
void oled_print(uint8_t Line,uint8_t Column,const char *String);

void oled_printf(uint8_t Line, uint8_t Column, const char *fmt, ...);

void oled_print_num(uint8_t Line,uint8_t Column,float Num,uint8_t DecimalPlaces,uint8_t ShowSign);

/************ 图形 ************/
void oled_draw_pixel(uint8_t x,uint8_t y);

void oled_draw_line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);    

void oled_draw_frame(uint8_t x,uint8_t y,uint8_t w,uint8_t h);

void oled_draw_box(uint8_t x,uint8_t y,uint8_t w,uint8_t h);

void oled_draw_circle(uint8_t x,uint8_t y,uint8_t r);

void oled_draw_disc(uint8_t x,uint8_t y,uint8_t r);

void oled_draw_bitmap(uint8_t x,uint8_t y,uint8_t w,uint8_t h,const uint8_t *bitmap);











