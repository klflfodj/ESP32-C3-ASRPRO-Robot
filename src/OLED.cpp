#include "OLED.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0,U8X8_PIN_NONE);
/*
* ESP32C3 Dev Module:
* SDA → GPIO 8
* SCL → GPIO 9
*/

// -----------------------
// OLED 显示初始化
// -----------------------
void OLED_Init(void)
{
  oled.begin();
  oled.enableUTF8Print();
  oled.setFont(u8g2_font_8x13_tr);
  oled.clearBuffer();
  oled.sendBuffer();
}

// -----------------------
// OLED 显示字符串
// -----------------------
void OLED_Print(uint8_t Line,uint8_t Column,const char *String)
{
  uint8_t x = (Column - 1) * 8;
  uint8_t y = Line * 16 -2;
  oled.drawStr(x,y,String);
  oled.sendBuffer();
}

// -----------------------
// OLED 显示数字
// -----------------------
// Line: 行号，范围 1-4
// Column: 列号，范围 1-16
// Num: 数字
// DecimalPlaces: 小数位数，范围 0-6
// ShowSign: 是否显示符号，0 不显示，1 显示
// -----------------------
void OLED_Print_Num(uint8_t Line,uint8_t Column,float Num,uint8_t DecimalPlaces,uint8_t ShowSign)
{
  char str[20];

  if (ShowSign)
  {
    sprintf(str,"%+.*f",DecimalPlaces,Num);
  }
  else
  {
    sprintf(str,"%.*f",DecimalPlaces,Num);
  }
  OLED_Print(Line, Column, str);
}

// -----------------------
// OLED 显示表情
// -----------------------
// data: 表情数据
// width: 表情宽度
// height: 表情高度
// x: 表情显示位置 X 坐标
// y: 表情显示位置 Y 坐标
// -----------------------
void OLED_ShowExpression(const uint8_t* data,uint8_t width,uint8_t height,uint8_t x,uint8_t y)
{
  oled.drawXBMP(x,y,width,height,data);
  oled.sendBuffer();
}

// -----------------------
// OLED 清屏
// -----------------------
void OLED_Clear(void)
{
  oled.clearBuffer();
  oled.sendBuffer();
}
