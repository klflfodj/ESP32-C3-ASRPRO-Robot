#include <stdint.h>
#include "esp32-hal-gpio.h"
#include "OLED_Ruan.h"
#include <Wire.h>
#include <Arduino.h>

#define OLED_W_SCL(x)     digitalWrite(OLED_SCL_PIN, x)
#define OLED_W_SDA(x)     digitalWrite(OLED_SDA_PIN, x)

void OLED_I2C_Init(void);
void OLED_IIC_Start(void);//IIC的开启信号
void OLED_IIC_Stop(void);//IIC的停止信号
void OLED_IIC_SendByte(uint8_t Byte);//IIC发送一个字节
void OLED_WriteCommand(uint8_t Command);//IIC写命令
void OLED_WriteData(uint8_t Data);//IIC写数据
void OLED_SetCursor(uint8_t Y, uint8_t X);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

/*
	显示字符串
		使用示例：
		OLED_Print(1,1,"GOOD");
*/

void OLED_Print(uint8_t Line, uint8_t Column, const char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/*
	显示数字(可显示小数和符号)
	Line: 起始行位置，范围：1~4
    Column: 起始列位置，范围：1~128/字符宽度 (取决于您的OLED列数，通常1对应8像素列)
	Num: 要显示的数字，支持整数和浮点数
    DecimalPlaces: 小数点后保留的位数，0表示显示为整数
    ShowSign: 是否强制显示正负号
    0: 不显示符号（正数不显示'+'，负数显示'-'）
    1: 显示符号（正数显示'+'，负数显示'-'）

	使用示例：
	float num1 = 1.25;
    float num2 = -3.7;
    float num3 = 0.0;
    float num4 = 123.456;
	// 示例1：显示1.25，保留2位小数，不显示正号
    OLED_Print_Num(1, 1, num1, 2, 0);  // 显示: 1.25
    
    // 示例2：显示-3.7，保留1位小数，不显示正号
    OLED_Print_Num(2, 1, num2, 1, 0);  // 显示: -3.7
    
    // 示例3：显示0.0，保留1位小数，强制显示符号
    OLED_Print_Num(3, 1, num3, 1, 1);  // 显示: +0.0
    
    // 示例4：显示123.456，保留3位小数，不显示正号
    OLED_Print_Num(4, 1, num4, 3, 0);  // 显示: 123.456
    
    // 示例5：显示123.456，不显示小数部分
    OLED_Print_Num(1, 10, num4, 0, 0); // 显示: 123
*/

void OLED_Print_Num(uint8_t Line, uint8_t Column, float Num, uint8_t DecimalPlaces, uint8_t ShowSign)
{
    char str[20]; // 足够大的缓冲区存放转换后的字符串
    uint8_t i = 0;
    
    // 1. 处理符号
    if (Num < 0) {
        str[i++] = '-';
        Num = -Num; // 转换为正数以便处理
    } else if (ShowSign) {
        str[i++] = '+';
    }
    
    // 2. 提取整数部分
    int integerPart = (int)Num;
    
    // 3. 将整数部分转换为字符串（倒序存放在临时缓冲区）
    char temp[20];
    uint8_t j = 0;
    
    if (integerPart == 0) {
        temp[j++] = '0';
    } else {
        while (integerPart > 0) {
            temp[j++] = (integerPart % 10) + '0';
            integerPart /= 10;
        }
    }
    
    // 4. 将整数部分正序放入主字符串
    while (j > 0) {
        str[i++] = temp[--j];
    }
    
    // 5. 处理小数部分
    if (DecimalPlaces > 0) {
        str[i++] = '.'; // 添加小数点
        
        // 提取小数部分
        float decimalPart = Num - (int)Num;
        
        // 将小数部分转换为指定位数的整数
        for (uint8_t k = 0; k < DecimalPlaces; k++) {
            decimalPart *= 10;
        }
        int decimalInt = (int)(decimalPart + 0.5); // 四舍五入
        
        // 处理小数部分的每一位
        for (uint8_t k = 0; k < DecimalPlaces; k++) {
            temp[k] = (decimalInt % 10) + '0';
            decimalInt /= 10;
        }
        
        // 将小数部分正序放入主字符串
        for (int k = DecimalPlaces - 1; k >= 0; k--) {
            str[i++] = temp[k];
        }
    }
    
    str[i] = '\0'; // 字符串结束符
    
    // 6. 调用现有的显示函数
    OLED_Print(Line, Column, str);
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
  uint8_t i;
  OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

void OLED_I2C_Init(void)
{
  pinMode(OLED_SCL_PIN, OUTPUT);
  pinMode(OLED_SDA_PIN, OUTPUT);
  OLED_W_SCL(HIGH);
  OLED_W_SDA(HIGH);
}

void OLED_IIC_Start(void)
{
  OLED_W_SDA(HIGH);
  OLED_W_SCL(HIGH);
  OLED_W_SDA(LOW);
  OLED_W_SCL(LOW);
}

void OLED_IIC_Stop(void)
{
  OLED_W_SDA(LOW);
  OLED_W_SCL(HIGH);
  OLED_W_SDA(HIGH);
}

void OLED_IIC_SendByte(uint8_t Byte)
{
  uint8_t i;
  for(i = 0; i < 8; i++)
  {
    OLED_W_SDA(!!(Byte & (0x80 >> i)));
    OLED_W_SCL(HIGH);
    OLED_W_SCL(LOW);
  }
  OLED_W_SCL(HIGH);
  OLED_W_SCL(LOW);  
}

void OLED_WriteCommand(uint8_t Command)
{
  OLED_IIC_Start();
	OLED_IIC_SendByte(OLED_I2C_ADDRESS);		//从机地址
	OLED_IIC_SendByte(0x00);		//写命令
	OLED_IIC_SendByte(Command); 
	OLED_IIC_Stop();
}

void OLED_WriteData(uint8_t Data)//IIC写数据
{
	OLED_IIC_Start();
	OLED_IIC_SendByte(0x78);		//从机地址
	OLED_IIC_SendByte(0x40);		//写数据
	OLED_IIC_SendByte(Data);
	OLED_IIC_Stop();
}

/*
  OLED设置光标位置
  如果你想在屏幕第 3 行（Y=2）、第 50 列（X=49）开始显示内容：
  OLED_SetCursor(2, 49);
  接下来调用OLED的显示函数，就会从第 2 行第 49 列开始写
*/
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/*
  OLED清屏
*/
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < OLED_PAGE_NUM; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < OLED_WIDTH; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

void OLED_Init(void)
{
  uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}

// 通用显示函数
void OLED_ShowExpression(const uint8_t* data, uint8_t width, uint8_t height, uint8_t x, uint8_t y) {
    uint8_t page, col;
    uint16_t dataIndex = 0;
    uint8_t pageCount = (height + 7) / 8;  // 计算所需页数
    
    for (page = 0; page < pageCount; page++) {
        OLED_SetCursor(y/8 + page, x);
        for (col = 0; col < width; col++) {
            if (dataIndex < (width * pageCount)) {
                OLED_WriteData(data[dataIndex++]);
            }
        }
    }
}
// 使用示例
//OLED_ShowExpression(Expression_Smile_24x24, 64, 64, 0, 0);  // 显示64x64
// OLED_ShowExpression(other_expression, 32, 32, 0, 0);     // 显示32x32
