#ifndef __OLED_RUAN_H
#define __OLED_RUAN_H

#include <stdint.h>
#include "OLED_Font.h"

#define OLED_I2C_ADDRESS    0x78
#define OLED_SCL_PIN        7
#define OLED_SDA_PIN        8
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGE_NUM       8       // 128x64 OLED 有 8 页（每页8行）

void OLED_Init(void);
void OLED_Print(uint8_t Line, uint8_t Column, const char *String);
void OLED_Print_Num(uint8_t Line, uint8_t Column, float Num, uint8_t DecimalPlaces, uint8_t ShowSign);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t Y, uint8_t X);

// 唯一的表情显示函数
void OLED_ShowExpression(const uint8_t* data, uint8_t width, uint8_t height, uint8_t x, uint8_t y);

#endif
