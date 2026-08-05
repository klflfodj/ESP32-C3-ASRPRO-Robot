#ifndef __OLED_H
#define __OLED_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line,uint8_t Column,char Char);
void OLED_Print(uint8_t Line,uint8_t Column,const char *String);
void OLED_Print_Num(uint8_t Line,uint8_t Column,float Num,uint8_t DecimalPlaces,uint8_t ShowSign);
void OLED_ShowExpression(const uint8_t* data,uint8_t width,uint8_t height,uint8_t x,uint8_t y);

#endif
