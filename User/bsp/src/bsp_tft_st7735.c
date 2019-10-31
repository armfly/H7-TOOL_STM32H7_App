/*
*********************************************************************************************************
*
*	模块名称 : ST7735 TFT SPI接口驱动程序
*	文件名称 : bsp_tft_st7735.c
*	版    本 : V1.0
*	说    明 : SPI接口，显示驱动IC为ST7735，分辨率为128*128
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0	2018-12-06 armfly 
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "fonts.h"

/*
  H7-TOOL LCD口线分配
  
  PF3  --->  LCD_RS
  PG2	 --->  LCD_CS
  PC12 --->  LCD_SDA		SPI3_MOSI
  PC10 --->  LCD_SCK		SPI3_SCK
  
  PH9	 --->  BACK_LIGHT	TIM12_CH2
  
  第2版增加reset
  
  PB6  --->  LCD_RESET
*/

#define ALL_LCD_GPIO_CLK_ENABLE() \
  __HAL_RCC_GPIOC_CLK_ENABLE();   \
  __HAL_RCC_GPIOF_CLK_ENABLE();   \
  __HAL_RCC_GPIOG_CLK_ENABLE();   \
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* LCD_RS 寄存器选择 */
#define LCD_RS_GPIO GPIOF
#define LCD_RS_PIN GPIO_PIN_3
#define LCD_RS_0() LCD_RS_GPIO->BSRRH = LCD_RS_PIN
#define LCD_RS_1() LCD_RS_GPIO->BSRRL = LCD_RS_PIN

/* LCD_CS SPI片选*/
#define LCD_CS_GPIO GPIOG
#define LCD_CS_PIN GPIO_PIN_2
#define LCD_CS_0() LCD_CS_GPIO->BSRRH = LCD_CS_PIN
#define LCD_CS_1() LCD_CS_GPIO->BSRRL = LCD_CS_PIN

/* SPI 接口 PC12/SPI3_MOSI，  PC10/SPI3_SCK */
#define LCD_SCK_GPIO GPIOC
#define LCD_SCK_PIN GPIO_PIN_10
#define LCD_SCK_0() LCD_SCK_GPIO->BSRRH = LCD_SCK_PIN
#define LCD_SCK_1() LCD_SCK_GPIO->BSRRL = LCD_SCK_PIN

#define LCD_SDA_GPIO GPIOC
#define LCD_SDA_PIN GPIO_PIN_12
#define LCD_SDA_0() LCD_SDA_GPIO->BSRRH = LCD_SDA_PIN
#define LCD_SDA_1() LCD_SDA_GPIO->BSRRL = LCD_SDA_PIN

/* LCD_RESET 复位 */
#define LCD_RESET_GPIO GPIOB
#define LCD_RESET_PIN GPIO_PIN_6
#define LCD_RESET_0() LCD_RESET_GPIO->BSRRH = LCD_RESET_PIN
#define LCD_RESET_1() LCD_RESET_GPIO->BSRRL = LCD_RESET_PIN

static void ST7735_ConfigGPIO(void);
static void ST7735_initial(void);

/*
*********************************************************************************************************
*	函 数 名: ST7735_InitHard
*	功能说明: 初始化LCD
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_InitHard(void)
{
  ST7735_ConfigGPIO(); /* 配置429 CPU内部LTDC */

  ST7735_initial();

  g_LcdHeight = 128; /* 显示屏分辨率-高度 */
  g_LcdWidth = 128;  /* 显示屏分辨率-宽度 */
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_ConfigLTDC
*	功能说明: 配置LTDC
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ST7735_ConfigGPIO(void)
{
  /* 配置GPIO */
  {
    GPIO_InitTypeDef gpio_init;

    /* 打开GPIO时钟 */
    ALL_LCD_GPIO_CLK_ENABLE();

    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   /* 设置推挽输出 */
    gpio_init.Pull = GPIO_NOPULL;           /* 上下拉电阻不使能 */
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; /* GPIO速度等级 */

    gpio_init.Pin = LCD_RS_PIN;
    HAL_GPIO_Init(LCD_RS_GPIO, &gpio_init);

    gpio_init.Pin = LCD_CS_PIN;
    HAL_GPIO_Init(LCD_CS_GPIO, &gpio_init);

    gpio_init.Pin = LCD_SCK_PIN;
    HAL_GPIO_Init(LCD_SCK_GPIO, &gpio_init);

    gpio_init.Pin = LCD_SDA_PIN;
    HAL_GPIO_Init(LCD_SDA_GPIO, &gpio_init);

    gpio_init.Pin = LCD_RESET_PIN;
    HAL_GPIO_Init(LCD_RESET_GPIO, &gpio_init);
  }
}

/*写指令到 LCD 模块*/
void transfer_command(int data1)
{
  char i;

  LCD_CS_0();
  LCD_RS_0();
  for (i = 0; i < 8; i++)
  {
    LCD_SCK_0();
    if (data1 & 0x80)
      LCD_SDA_1();
    else
      LCD_SDA_0();
    LCD_SCK_1();
    data1 = data1 <<= 1;
  }
}

/*写数据到 LCD 模块*/
void transfer_data(int data1)
{
  char i;

  LCD_CS_0();
  LCD_RS_1();
  for (i = 0; i < 8; i++)
  {
    LCD_SCK_0();
    if (data1 & 0x80)
      LCD_SDA_1();
    else
      LCD_SDA_0();
    LCD_SCK_1();
    data1 = data1 <<= 1;
  }
}

// 连写2个字节（即 16 位）数据到LCD模块
void transfer_data_16(uint16_t data2)
{
  transfer_data(data2 >> 8);
  transfer_data(data2);
}

//LCD 初始化
static void ST7735_initial(void)
{
  bsp_DelayUS(50 * 1000);
  LCD_RESET_0(); /* 低电平：复位 */
  bsp_DelayUS(1000);
  LCD_RESET_1(); /* 高电平：复位结束 */
  bsp_DelayUS(10 * 1000);

  //开始初始化：
  transfer_command(0x11);
  transfer_command(0xb1);
  transfer_data(0x01);
  transfer_data(0x2c);
  transfer_data(0x2d);
  transfer_command(0xb2);
  transfer_data(0x01);
  transfer_data(0x2c);
  transfer_data(0x2d);
  transfer_command(0xb3);
  transfer_data(0x01);
  transfer_data(0x2c);
  transfer_data(0x2d);

  transfer_data(0x01);
  transfer_data(0x2d);
  transfer_data(0x2d);
  transfer_command(0xb4);
  transfer_data(0x02);
  transfer_command(0xb6);
  transfer_data(0xb4);
  transfer_data(0xf0);
  transfer_command(0xc0);
  transfer_data(0xa2);
  transfer_data(0x02);
  transfer_data(0x84);
  transfer_command(0xc1);
  transfer_data(0xc5);
  transfer_command(0xc2);
  transfer_data(0x0a);
  transfer_data(0x00);
  transfer_command(0xc3);
  transfer_data(0x8a);
  transfer_data(0x2a);
  transfer_command(0xc4);
  transfer_data(0x8a);
  transfer_data(0xee);
  transfer_command(0xc5);
  transfer_data(0x0e);
  transfer_command(0x36); //行扫描顺序，列扫描顺序，横放/竖放
  transfer_data(0x08);
  //MX = 1（行地址顺序：从左到右），MY = 1（列地址顺序：从上到下），MV = 0（竖放），ML = 0(纵向刷新：
  //从上到下），RGB = 1（依次为 RGB），MH = 0（横向刷新顺序：从左到右）
  //定义："normal"就是“0xc8”---正常竖放;
  //定义："CW180"就是“0x08"---在正常竖放基础上转 180 度竖放;
  //定义："CCW90”就是“0xa8"---在竖放基础上逆时针转 90 度横放;
  //定义：“CW90”就是“0x68"---在竖放基础上顺转 90 度横放;
  transfer_command(0xff);
  transfer_data(0x40);
  transfer_data(0x03);
  transfer_data(0x1a);

  transfer_command(0xfc);
  transfer_data(0x11);
  transfer_data(0x17);
  transfer_command(0xf0);
  transfer_data(0x01);
  transfer_command(0x3a);
  transfer_data(0x05);
  transfer_command(0xf6);
  transfer_data(0x00);
  transfer_command(0xe0);
  transfer_data(0x02);
  transfer_data(0x1c);
  transfer_data(0x07);
  transfer_data(0x12);
  transfer_data(0x37);
  transfer_data(0x32);
  transfer_data(0x29);
  transfer_data(0x2d);
  transfer_data(0x29);
  transfer_data(0x25);
  transfer_data(0x2b);
  transfer_data(0x39);
  transfer_data(0x00);
  transfer_data(0x01);
  transfer_data(0x03);
  transfer_data(0x10);
  transfer_command(0xe1);
  transfer_data(0x0b);
  transfer_data(0x14);
  transfer_data(0x09);
  transfer_data(0x26);
  transfer_data(0x27);
  transfer_data(0x22);
  transfer_data(0x1c);
  transfer_data(0x20);
  transfer_data(0x1d);
  transfer_data(0x1a);
  transfer_data(0x25);
  transfer_data(0x2d);
  transfer_data(0x06);
  transfer_data(0x06);

  transfer_data(0x02);
  transfer_data(0x0f);
  transfer_command(0x2a); //定义 X 地址的开始及结束位置
  transfer_data(0x00);
  transfer_data(0x00);
  transfer_data(0x00);
  transfer_data(0x7F);
  transfer_command(0x2b); //定义 Y 地址的开始及结束位置
  transfer_data(0x00);
  transfer_data(0x00);
  transfer_data(0x00);
  transfer_data(0x7F);
  transfer_command(0x29); //开显示
}

//定义窗口坐标：开始坐标（XS,YS)以及窗口大小（x_total,y_total)
//void lcd_address(int XS,int YS, int x_total, int y_total)
//{
//	int XE,YE;
//
//	XS = XS+2;
//	YS = YS+1;
//	XE = XS + x_total - 1;
//	YE = YS + y_total - 1;
//	transfer_command(0x2a); // 设置 X 开始及结束的地址
//	transfer_data_16(XS); // X 开始地址(16 位）
//	transfer_data_16(XE); // X 结束地址(16 位）
//	transfer_command(0x2b); // 设置 Y 开始及结束的地址
//	transfer_data_16(YS); // Y 开始地址(16 位）
//	transfer_data_16(YE); // Y 结束地址(16 位
//	transfer_command(0x2c); // 写数据开始
//}

////将单色的 8 位的数据（代表 8 个像素点）转换成彩色的数据传输给液晶屏
//void mono_transfer_data(int mono_data,int font_color,int back_color)
//{
//	int i;
//
//	for(i = 0; i < 8; i++)
//	{
//		if(mono_data&0x80)
//		{
//			transfer_data_16(font_color); //当数据是 1 时，显示字体颜色
//		}
//		else
//		{
//			transfer_data_16(back_color); //当数据是 0 时，显示底色
//		}
//		mono_data <<= 1;
//	}
//}

////显示单一色彩
//void display_color(int XS,int YS,int x_total,int y_total,int color)
//{
//	int i,j;
//
//	lcd_address(XS,YS,x_total,y_total);
//	for (i = 0; i < 128; i++)
//	{
//		for(j = 0;j<128;j++)
//		{
//			transfer_data_16(color);
//		}
//	}
//}

////显示一幅全屏彩图
//void display_image(uint8_t *dp)
//{
//	uint8_t i,j;
//
//	lcd_address(0,0,128,128);
//	for (i = 0; i < 128; i++)
//	{
//		for(j = 0;j<128;j++)
//		{
//			transfer_data(*dp); //传一个像素的图片数据的高位
//			dp++;
//			transfer_data(*dp); //传一个像素的图片数据的低位
//			dp++;
//		}
//	}
//}

// 连写2个字节（即 16 位）数据到LCD模块
void ST7735_WriteData16(uint16_t data2)
{
  transfer_data(data2 >> 8);
  transfer_data(data2);
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_GetChipDescribe
*	功能说明: 读取LCD驱动芯片的描述符号，用于显示
*	形    参: char *_str : 描述符字符串填入此缓冲区
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_GetChipDescribe(char *_str)
{
  strcpy(_str, "ST7735");
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_SetDispWin
*	功能说明: 设置显示窗口，进入窗口显示模式。
*	形    参:  
*		_usX : 水平坐标
*		_usY : 垂直坐标
*		_usHeight: 窗口高度
*		_usWidth : 窗口宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{
  /* 设置 X 开始及结束的地址 */
  transfer_command(0x2a);
  transfer_data_16(_usX + 2);
  transfer_data_16(_usX + 2 + _usWidth - 1);

  /* 设置 Y开始及结束的地址 */
  transfer_command(0x2b);
  transfer_data_16(_usY + 1);
  transfer_data_16(_usY + 1 + _usHeight - 1);

  transfer_command(0x2c); /* 写数据开始 */
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_QuitWinMode
*	功能说明: 退出窗口显示模式，变为全屏显示模式
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_QuitWinMode(void)
{
  ST7735_SetDispWin(0, 0, g_LcdHeight, g_LcdWidth);
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DispOn
*	功能说明: 打开显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DispOn(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DispOff
*	功能说明: 关闭显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DispOff(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_ClrScr
*	功能说明: 根据输入的颜色值清屏
*	形    参: _usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_ClrScr(uint16_t _usColor)
{
  ST7735_FillRect(0, 0, g_LcdHeight, g_LcdWidth, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色 ( RGB = 565 格式)
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
  ST7735_SetDispWin(_usX, _usY, 1, 1);
  transfer_data_16(_usColor);
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t ST7735_GetPixel(uint16_t _usX, uint16_t _usY)
{
  return CL_BLUE;
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usX2, _usY2 : 终止点Y坐标
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DrawLine(uint16_t _usX1, uint16_t _usY1, uint16_t _usX2, uint16_t _usY2, uint16_t _usColor)
{
  int32_t dx, dy;
  int32_t tx, ty;
  int32_t inc1, inc2;
  int32_t d, iTag;
  int32_t x, y;

  /* 采用 Bresenham 算法，在2点间画一条直线 */

  ST7735_PutPixel(_usX1, _usY1, _usColor);

  /* 如果两点重合，结束后面的动作。*/
  if (_usX1 == _usX2 && _usY1 == _usY2)
  {
    return;
  }

  iTag = 0;
  /* dx = abs ( _usX2 - _usX1 ); */
  if (_usX2 >= _usX1)
  {
    dx = _usX2 - _usX1;
  }
  else
  {
    dx = _usX1 - _usX2;
  }

  /* dy = abs ( _usY2 - _usY1 ); */
  if (_usY2 >= _usY1)
  {
    dy = _usY2 - _usY1;
  }
  else
  {
    dy = _usY1 - _usY2;
  }

  if (dx < dy) /*如果dy为计长方向，则交换纵横坐标。*/
  {
    uint16_t temp;

    iTag = 1;
    temp = _usX1;
    _usX1 = _usY1;
    _usY1 = temp;
    temp = _usX2;
    _usX2 = _usY2;
    _usY2 = temp;
    temp = dx;
    dx = dy;
    dy = temp;
  }
  tx = _usX2 > _usX1 ? 1 : -1; /* 确定是增1还是减1 */
  ty = _usY2 > _usY1 ? 1 : -1;
  x = _usX1;
  y = _usY1;
  inc1 = 2 * dy;
  inc2 = 2 * (dy - dx);
  d = inc1 - dx;
  while (x != _usX2) /* 循环画点 */
  {
    if (d < 0)
    {
      d += inc1;
    }
    else
    {
      y += ty;
      d += inc2;
    }
    if (iTag)
    {
      ST7735_PutPixel(y, x, _usColor);
    }
    else
    {
      ST7735_PutPixel(x, y, _usColor);
    }
    x += tx;
  }
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DrawHLine
*	功能说明: 绘制一条水平线. 使用STM32F429 DMA2D硬件绘制.
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usLen       : 线的长度
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DrawHLine(uint16_t _usX, uint16_t _usY, uint16_t _usLen, uint16_t _usColor)
{
#if 0
  ST7735_FillRect(_usX, _usY, 1, _usLen, _usColor);
#else
  uint16_t i;

  for (i = 0; i < _usLen; i++)
  {
    ST7735_PutPixel(_usX + i, _usY, _usColor);
  }
#endif
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DrawVLine
*	功能说明: 绘制一条垂直线。 使用STM32F429 DMA2D硬件绘制.
*	形    参:
*			_usX, _usY : 起始点坐标
*			_usLen       : 线的长度
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DrawVLine(uint16_t _usX, uint16_t _usY, uint16_t _usLen, uint16_t _usColor)
{
#if 0
  ST7735_FillRect(_usX, _usY, _usLen, 1, _usColor);
#else
  uint16_t i;

  for (i = 0; i < _usLen; i++)
  {
    ST7735_PutPixel(_usX, _usY + i, _usColor);
  }
#endif
}
/*
*********************************************************************************************************
*	函 数 名: ST7735_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     : 坐标数组
*			_usColor : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor)
{
  uint16_t i;

  for (i = 0; i < _usSize - 1; i++)
  {
    ST7735_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _usColor);
  }
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DrawRect
*	功能说明: 绘制水平放置的矩形。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
  /*
   ---------------->---
  |(_usX，_usY)        |
  V                    V  _usHeight
  |                    |
   ---------------->---
      _usWidth
  */
  ST7735_DrawHLine(_usX, _usY, _usWidth, _usColor);
  ST7735_DrawVLine(_usX + _usWidth - 1, _usY, _usHeight, _usColor);
  ST7735_DrawHLine(_usX, _usY + _usHeight - 1, _usWidth, _usColor);
  ST7735_DrawVLine(_usX, _usY, _usHeight, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_FillRect
*	功能说明: 用一个颜色值填充一个矩形。使用STM32F429内部DMA2D硬件绘制。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*			_usColor  : 颜色代码
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
  uint32_t i;

  ST7735_SetDispWin(_usX, _usY, _usHeight, _usWidth);

  for (i = 0; i < _usHeight * _usWidth; i++)
  {
    transfer_data_16(_usColor);
  }
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
  int32_t D;     /* Decision Variable */
  uint32_t CurX; /* 当前 X 值 */
  uint32_t CurY; /* 当前 Y 值 */

  D = 3 - (_usRadius << 1);
  CurX = 0;
  CurY = _usRadius;

  while (CurX <= CurY)
  {
    ST7735_PutPixel(_usX + CurX, _usY + CurY, _usColor);
    ST7735_PutPixel(_usX + CurX, _usY - CurY, _usColor);
    ST7735_PutPixel(_usX - CurX, _usY + CurY, _usColor);
    ST7735_PutPixel(_usX - CurX, _usY - CurY, _usColor);
    ST7735_PutPixel(_usX + CurY, _usY + CurX, _usColor);
    ST7735_PutPixel(_usX + CurY, _usY - CurX, _usColor);
    ST7735_PutPixel(_usX - CurY, _usY + CurX, _usColor);
    ST7735_PutPixel(_usX - CurY, _usY - CurX, _usColor);

    if (D < 0)
    {
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
*	形    参:  
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
  uint16_t i, k, y;
  const uint16_t *p;

  p = _ptr;
  y = _usY;
  for (i = 0; i < _usHeight; i++)
  {
    for (k = 0; k < _usWidth; k++)
    {
      ST7735_PutPixel(_usX + k, y, *p++);
    }

    y++;
  }
}

/*
*********************************************************************************************************
*	函 数 名: ST7735_SetDirection
*	功能说明: 设置显示屏显示方向（横屏 竖屏）
*	形    参: 显示方向代码 0 横屏正常, 1 = 横屏180度翻转, 2 = 竖屏, 3 = 竖屏180度翻转
*	返 回 值: 无
*********************************************************************************************************
*/
void ST7735_SetDirection(uint8_t _dir)
{
  uint16_t temp;

  if (_dir == 0 || _dir == 1) /* 横屏， 横屏180度 */
  {
    if (g_LcdWidth < g_LcdHeight)
    {
      temp = g_LcdWidth;
      g_LcdWidth = g_LcdHeight;
      g_LcdHeight = temp;
    }
  }
  else if (_dir == 2 || _dir == 3) /* 竖屏, 竖屏180°*/
  {
    if (g_LcdWidth > g_LcdHeight)
    {
      temp = g_LcdWidth;
      g_LcdWidth = g_LcdHeight;
      g_LcdHeight = temp;
    }
  }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
