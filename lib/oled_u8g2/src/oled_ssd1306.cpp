#include "oled_ssd1306.h"

#include <Arduino.h>

static uint8_t FontWidth;
static uint8_t FontHeight;
static uint8_t FontAscent;

static const uint8_t height = 64;
static const uint8_t width = 128;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
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
bool oled_ssd1306_init()
{
    oled.begin();
    oled.enableUTF8Print();
    oled_set_font(u8g2_font_8x13_tr);  //u8g2_font_wqy13_t_gb2312中文用这个
    oled.clearBuffer();
    oled.sendBuffer();

    return true;
}

// -----------------------
// OLED 显示清屏
// -----------------------
// 说明：清空 OLED 显示缓冲区，刷新显示
// -----------------------
void oled_clear()
{
    oled.clearBuffer();
}

// -----------------------
// OLED 显示刷新
// -----------------------
// 说明：刷新 OLED 显示缓冲区，更新显示
// -----------------------
void oled_update()
{
    oled.sendBuffer();
}

// -----------------------
// OLED 设置字体
// -----------------------
// 说明：设置 OLED 显示字体
// -----------------------
void oled_set_font(const uint8_t *font)
{
    if (font == NULL)
    {
        return;
    }

    oled.setFont(font);

    FontWidth  = oled.getMaxCharWidth();
    FontHeight = oled.getMaxCharHeight();
    FontAscent = oled.getAscent();
}

// -----------------------
// OLED 显示文本
// -----------------------
// 说明：在指定行和列显示文本
// -----------------------
void oled_print(uint8_t Line, uint8_t Column, const char *String)
{
    if (Line == 0 || Line > height || Column == 0 || Column > width || String == NULL)
    {
        return;
    }

    uint8_t x = (Column - 1) * FontWidth;
    uint8_t y = (Line - 1) * FontHeight + FontAscent;

    oled.drawUTF8(x, y, String);
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
void oled_printf(uint8_t Line, uint8_t Column, const char *fmt, ...)
{
    if (Line == 0 || Line > height || Column == 0 || Column > width || fmt == NULL)
    {
        return;
    }

    char str[64];
    va_list args;
    va_start(args,fmt);
    vsnprintf(str,sizeof(str),fmt,args);
    va_end(args);

    oled_print(Line, Column, str);
}

// -----------------------
// OLED 显示数字
// -----------------------
// 说明：在指定行和列显示数字
// -----------------------
// 参数：Line - 行号，Column - 列号，Num - 数字，DecimalPlaces - 小数位数，ShowSign - 是否显示符号
// -----------------------
void oled_print_num(uint8_t Line, uint8_t Column, float Num, uint8_t DecimalPlaces, uint8_t ShowSign)
{
    if (Line == 0 || Line > height || Column == 0 || Column > width)
    {
        return;
    }

    char str[20];
    if (ShowSign)
    {
        sprintf(str, "%+.*f", DecimalPlaces, Num);
    }
    else
    {
        sprintf(str, "%.*f", DecimalPlaces, Num);
    }
    oled_print(Line, Column, str);
}

// -----------------------
// OLED 绘制像素点
// -----------------------
// 说明：在指定坐标绘制像素点
// -----------------------
// 参数：x - 横坐标，y - 纵坐标
// -----------------------
void oled_draw_pixel(uint8_t x, uint8_t y)
{
    if (x >= width || y >= height)
    {
        return;
    }

    oled.drawPixel(x, y);
}

// -----------------------
// OLED 绘制线
// -----------------------
// 说明：在指定坐标绘制线
// -----------------------
// 参数：x1 - 起始横坐标，y1 - 起始纵坐标，x2 - 终止横坐标，y2 - 终止纵坐标
// -----------------------
void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    if (x1 >= width || y1 >= height || x2 >= width || y2 >= height)
    {
        return;
    }
    

    oled.drawLine(x1, y1, x2, y2);
}

// -----------------------
// OLED 绘制框
// -----------------------
// 说明：在指定坐标绘制框
// -----------------------
// 参数：x - 框横坐标，y - 框纵坐标，w - 框宽度，h - 框高度
// -----------------------
void oled_draw_frame(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    if (x >= width || y >= height || w <= 0 || h <= 0 || x + w > width || y + h > height)
    {
        return;
    }

    oled.drawFrame(x, y, w, h);
}

// -----------------------
// OLED 绘制矩形
// -----------------------
// 说明：在指定坐标绘制矩形
// -----------------------
// 参数：x - 框横坐标，y - 框纵坐标，w - 框宽度，h - 框高度
// -----------------------
void oled_draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    if (x >= width || y >= height || w <= 0 || h <= 0 || x + w > width || y + h > height)
    {
        return;
    }

    oled.drawBox(x, y, w, h);
}

// -----------------------
// OLED 绘制圆
// -----------------------
// 说明：在指定坐标绘制圆
// -----------------------
// 参数：x - 圆心横坐标，y - 圆心纵坐标，r - 圆半径
// -----------------------
void oled_draw_circle(uint8_t x, uint8_t y, uint8_t r)
{
    if (x >= width || y >= height || r <= 0 || x + r > width || y + r > height)
    {
        return;
    }

    oled.drawCircle(x, y, r);
}

// -----------------------
// OLED 绘制实心圆
// -----------------------
// 说明：在指定坐标绘制实心圆
// -----------------------
// 参数：x - 圆心横坐标，y - 圆心纵坐标，r - 圆半径
// -----------------------
void oled_draw_disc(uint8_t x, uint8_t y, uint8_t r)
{
    if (x >= width || y >= height || r <= 0 || x + r > width || y + r > height)
    {
        return;
    }

    oled.drawDisc(x, y, r);
}

// -----------------------
// OLED 绘制位图
// -----------------------
// 说明：在指定坐标绘制位图
// -----------------------
// 参数：x - 位图横坐标，y - 位图纵坐标，w - 位图宽度，h - 位图高度，bitmap - 位图数据指针
// -----------------------
void oled_draw_bitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *bitmap)
{
    if (x >= width || y >= height || x + w > width || y + h > height || bitmap == NULL)
    {
        return;
    }

    oled.drawBitmap(x, y, w, h, bitmap);
}





