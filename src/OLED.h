#ifndef __OLED_H
#define __OLED_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <stdarg.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;

/************ 初始化 ************/
void OLED_Init(void);

/************ 刷新 ************/
void OLED_Update(void);

void OLED_Clear(void);

/************ 字体 ************/
void OLED_SetFont(const uint8_t *font);

/************ 文本 ************/
void OLED_Print(uint8_t Line,uint8_t Column,const char *String);

void OLED_Printf(uint8_t Line, uint8_t Column, const char *fmt, ...);

void OLED_PrintNum(uint8_t Line,uint8_t Column,float Num,uint8_t DecimalPlaces,uint8_t ShowSign);

/************ 图形 ************/
void OLED_DrawPixel(uint8_t x,uint8_t y);

void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);

void OLED_DrawFrame(uint8_t x,uint8_t y,uint8_t w,uint8_t h);

void OLED_DrawBox(uint8_t x,uint8_t y,uint8_t w,uint8_t h);

void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r);

void OLED_DrawDisc(uint8_t x,uint8_t y,uint8_t r);

void OLED_DrawBitmap(uint8_t x,uint8_t y,uint8_t w,uint8_t h,const uint8_t *bitmap);

#endif
