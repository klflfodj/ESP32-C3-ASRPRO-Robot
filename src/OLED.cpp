#include "OLED.h"

#include <stdarg.h>
#include <stdio.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0,U8X8_PIN_NONE);
/*
* ESP32C3 Dev Module:
* SDA → GPIO 8
* SCL → GPIO 9
*/

// -----------------------
// OLED 显示初始化
// -----------------------
// 说明：初始化 OLED 显示，设置字体为 8x13，清屏显示
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
// OLED 显示清屏
// -----------------------
// 说明：清空 OLED 显示缓冲区，刷新显示
// -----------------------
void OLED_Clear(void)
{
  oled.clearBuffer();
}

// -----------------------
// OLED 显示刷新
// -----------------------
// 说明：刷新 OLED 显示缓冲区，更新显示
// -----------------------
void OLED_Update(void)
{
  oled.sendBuffer();
}

// -----------------------
// OLED 设置字体
// -----------------------
// 说明：设置 OLED 显示字体
// -----------------------
void OLED_SetFont(const uint8_t *font)
{
  oled.setFont(font);
}

// -----------------------
// OLED 显示文本
// -----------------------
// 说明：在指定行和列显示文本
// -----------------------
void OLED_Print(uint8_t Line, uint8_t Column, const char *string)
{
  uint8_t x = (Column-1)*8;
  uint8_t y = Line*16-2;
  oled.drawUTF8(x,y,string);
}

// -----------------------
// OLED 显示格式化文本
// -----------------------
// 说明：在指定行和列显示格式化文本,用法和printf类似
// -----------------------
// 参数：Line - 行号，Column - 列号，fmt - 格式化字符串
// -----------------------
// 注意：使用可变参数时，需要包含 <stdarg.h> 头文件
// -----------------------
void OLED_Printf(uint8_t Line, uint8_t Column, const char *fmt, ...)
{
  char str[64];
  va_list args;
  va_start(args,fmt);
  vsnprintf(str,sizeof(str),fmt,args);
  va_end(args);
  OLED_Print(Line, Column, str);
}

// -----------------------
// OLED 显示数字
// -----------------------
// 说明：在指定行和列显示数字
// -----------------------
// 参数：Line - 行号，Column - 列号，Num - 数字，DecimalPlaces - 小数位数，ShowSign - 是否显示符号
// -----------------------
void OLED_Print_Num(uint8_t Line, uint8_t Column, float Num, uint8_t DecimalPlaces, uint8_t ShowSign)
{
  char str[20];
  if (ShowSign)
  {
    sprintf(str, "%+.*f", DecimalPlaces, Num);
  }
  else
  {
    sprintf(str, "%.*f", DecimalPlaces, Num);
  }
  OLED_Print(Line, Column, str);
}

// -----------------------
// OLED 绘制像素点
// -----------------------
// 说明：在指定坐标绘制像素点
// -----------------------
// 参数：x - 横坐标，y - 纵坐标
// -----------------------
void OLED_Drawpixel(uint8_t x, uint8_t y)
{
  oled.drawPixel(x, y);
}

// -----------------------
// OLED 绘制线
// -----------------------
// 说明：在指定坐标绘制线
// -----------------------
// 参数：x1 - 起始横坐标，y1 - 起始纵坐标，x2 - 终止横坐标，y2 - 终止纵坐标
// -----------------------
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
  oled.drawLine(x1, y1, x2, y2);
}

// -----------------------
// OLED 绘制框
// -----------------------
// 说明：在指定坐标绘制框
// -----------------------
// 参数：x - 桪坐标，y - 框纵坐标，w - 框宽度，h - 框高度
// -----------------------
void OLED_DrawFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  oled.drawFrame(x, y, w, h);
}

// -----------------------
// OLED 绘制矩形
// -----------------------
// 说明：在指定坐标绘制矩形
// -----------------------
// 参数：x - 桪坐标，y - 框纵坐标，w - 框宽度，h - 框高度
// -----------------------
void OLED_DrawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  oled.drawBox(x, y, w, h);
}

// -----------------------
// OLED 绘制圆
// -----------------------
// 说明：在指定坐标绘制圆
// -----------------------
// 参数：x - 圆心横坐标，y - 圆心纵坐标，r - 圆半径
// -----------------------
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r)
{
  oled.drawCircle(x, y, r);
}

// -----------------------
// OLED 绘制实心圆
// -----------------------
// 说明：在指定坐标绘制实心圆
// -----------------------
// 参数：x - 圆心横坐标，y - 圆心纵坐标，r - 圆半径
// -----------------------
void OLED_DrawDisc(uint8_t x, uint8_t y, uint8_t r)
{
  oled.drawDisc(x, y, r);
}

// -----------------------
// OLED 绘制位图
// -----------------------
// 说明：在指定坐标绘制位图
// -----------------------
// 参数：x - 位图横坐标，y - 位图纵坐标，w - 位图宽度，h - 位图高度，bitmap - 位图数据指针
// -----------------------
void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap)
{
  oled.drawBitmap(x, y, w, h, bitmap);
}

