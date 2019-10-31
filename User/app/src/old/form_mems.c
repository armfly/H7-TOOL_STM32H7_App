/*
*********************************************************************************************************
*
*	模块名称 : MEMS 传感器测试（I2C） 三轴陀螺仪，磁力计，气压计, 光照度传感器
*	文件名称 : mems_test.c
*	版    本 : V1.0
*	说    明 : 测试MPU-6050, HCM5833L, BMP085, BH1750
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2013-02-01 armfly  首发
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_mems.h"

/* 4个框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H 120
#define BOX1_W 190
#define BOX1_TEXT "MPU6050 陀螺仪"

#define BOX2_X (BOX1_X + BOX1_W + 5)
#define BOX2_Y BOX1_Y
#define BOX2_H BOX1_H
#define BOX2_W 250
#define BOX2_TEXT "磁力计"

#define BOX3_X BOX1_X
#define BOX3_Y (BOX1_Y + BOX1_H + 5)
#define BOX3_H 52
#define BOX3_W 220
#define BOX3_TEXT "BMP085 气压计和温度"

#define BOX4_X (BOX1_X + BOX3_W + 5)
#define BOX4_Y (BOX1_Y + BOX1_H + 5)
#define BOX4_H 52
#define BOX4_W 220
#define BOX4_TEXT "BH1750 光照度"

#define BOX5_X BOX1_X
#define BOX5_Y (BOX3_Y + BOX3_H + 5)
#define BOX5_H 52
#define BOX5_W 220
#define BOX5_TEXT "DS18B20 温度传感器"

#define BOX6_X (BOX1_X + BOX5_W + 5)
#define BOX6_Y (BOX3_Y + BOX3_H + 5)
#define BOX6_H 52
#define BOX6_W 220
#define BOX6_TEXT "DHT11 温湿度传感器"

/* 返回按钮的坐标(屏幕右下角) */
#define BUTTON_RET_ID 0
#define BUTTON_RET_H 32
#define BUTTON_RET_W 60
#define BUTTON_RET_X (g_LcdWidth - BUTTON_RET_W - 4)
#define BUTTON_RET_Y (g_LcdHeight - BUTTON_RET_H - 4)
#define BUTTON_RET_TEXT "返回"

static void DispInitFace(void);
static void DispMPU6050(void);
static void DispHMC5833L(void);
static void DispBMP085(void);
static void DispBH1750(float _value);
static void DispDS18B20(void);
static void DispDHT11(void);

static void DispButton(uint8_t _id, uint8_t _focus);

static uint8_t s_use_dht11 = 0; /* 用来自动选择 DS18B20和 DHT11 */

/*
*********************************************************************************************************
*	函 数 名: TestMems
*	功能说明: 测试陀螺仪，加速度计，磁力计，气压计，光照度
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestMems(void)
{
  uint8_t ucKeyCode; /* 按键代码 */
  uint8_t ucTouch;   /* 触摸事件 */
                     //	uint8_t fRefresh;		/* 刷屏请求标志,1表示需要刷新 */
  uint8_t fQuit = 0;

  int16_t tpX, tpY;

  DispInitFace();

  //	fRefresh = 1;

  /* 重新配置1次BMP085 */
  bsp_InitBMP085();
  BMP085_ReadTempPress(); /* 读取结果存放在全局变量 */

  if (i2c_CheckDevice(HMC5883L_SLAVE_ADDRESS) == 0)
  {
    bsp_InitHMC5883L();
  }

  bsp_InitMPU6050();

  bsp_InitDS18B20();

  /* 进入主程序循环体 */
  bsp_StartAutoTimer(0, 250); /* 陀螺仪 加速度 磁力计 定时采样周期 */
  bsp_DelayMS(125);
  bsp_StartAutoTimer(1, 1000); /* 气压计和光照度定时采样周期 */
  while (fQuit == 0)
  {
    bsp_Idle();

    if (bsp_CheckTimer(0))
    {
      MPU6050_ReadData();

      DispMPU6050();

      HMC5883L_ReadData();
      DispHMC5833L();

      DispDS18B20(); /* 读取并显示DS18B20的数据 */
    }

    if (bsp_CheckTimer(1))
    {
      BMP085_ReadTempPress();
      DispBMP085();

      DispBH1750(BH1750_GetLux());

      DispDHT11();
    }

    ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
    if (ucTouch != TOUCH_NONE)
    {
      switch (ucTouch)
      {
      case TOUCH_DOWN: /* 触笔按下事件 */
        if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
        {
          DispButton(BUTTON_RET_ID, 1);
        }
        break;

      case TOUCH_RELEASE: /* 触笔释放事件 */
        if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
        {
          DispButton(BUTTON_RET_ID, 0);
          fQuit = 1; /* 返回 */
        }
        else /* 按钮失去焦点 */
        {
          DispButton(BUTTON_RET_ID, 0);
        }
        break;
      }
    }

    /* 处理按键事件 */
    ucKeyCode = bsp_GetKey();
    if (ucKeyCode > 0)
    {
      /* 有键按下 */
      switch (ucKeyCode)
      {
      case KEY_DOWN_K1: /* K1键切换音频格式，在下次开始录音和放音时有效 */
        break;

      case KEY_DOWN_K2: /* K2键按下，录音 */
        break;

      case KEY_DOWN_K3: /* K3键按下，放音 */
        break;

      case JOY_DOWN_U: /* 摇杆UP键按下 */
        break;

      case JOY_DOWN_D: /* 摇杆DOWN键按下 */
        break;

      case JOY_DOWN_L: /* 摇杆LEFT键按下 */
        break;

      case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
        break;

      case JOY_DOWN_OK: /* 摇杆OK键按下 */
        break;

      default:
        break;
      }
    }
  }

  bsp_StopTimer(0);
  bsp_StopTimer(1);
}

/*
*********************************************************************************************************
*	函 数 名: DispMPU6050
*	功能说明: 显示MPU6050输出数据
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMPU6050(void)
{
  FONT_T tFont;
  char buf[128];
  uint16_t x, y;
  uint8_t ucLineCap = 17;

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
    tFont.FrontColor = CL_BLUE;    /* 字体颜色 */
    tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
    tFont.Space = 0;               /* 文字间距，单位 = 像素 */
  }

  x = BOX1_X + 5;
  y = BOX1_Y + 18;

  sprintf(buf, "GYRO_X  = %6d", g_tMPU6050.GYRO_X);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "GYRO_Y  = %6d", g_tMPU6050.GYRO_Y);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "GYRO_Z  = %6d", g_tMPU6050.GYRO_Z);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "Accel_X = %6d", g_tMPU6050.Accel_X);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "Accel_Y = %6d", g_tMPU6050.Accel_Y);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "Accel_Z = %6d", g_tMPU6050.Accel_Z);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;
}

/*
*********************************************************************************************************
*	函 数 名: DispHMC5833L
*	功能说明: 显示HMC5833L输出的数据
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispHMC5833L(void)
{
  FONT_T tFont;
  char buf[128];
  uint16_t x, y;
  uint8_t ucLineCap = 17;

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
    tFont.FrontColor = CL_BLUE;    /* 字体颜色 */
    tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
    tFont.Space = 0;               /* 文字间距，单位 = 像素 */
  }

  x = BOX2_X + 5;
  y = BOX2_Y + 18;

  sprintf(buf, "   当前 | 最小, 最大,范围");
  tFont.FrontColor = CL_GREY; /* 红色 异常*/
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "X =%5d|%5d,%5d,%5d", g_tMag.X, g_tMag.X_Min, g_tMag.X_Max,
          g_tMag.X_Max - g_tMag.X_Min);
  if (g_tMag.X_Max - g_tMag.X_Min < 500)
  {
    tFont.FrontColor = CL_RED; /* 红色 异常*/
  }
  else
  {
    tFont.FrontColor = CL_BLACK; /* 白色  正常 */
  }
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "Y =%5d|%5d,%5d,%5d", g_tMag.Y, g_tMag.Y_Min, g_tMag.Y_Max,
          g_tMag.Y_Max - g_tMag.Y_Min);
  if (g_tMag.Y_Max - g_tMag.Y_Min < 500)
  {
    tFont.FrontColor = CL_RED; /* 红色 异常*/
  }
  else
  {
    tFont.FrontColor = CL_BLACK; /* 白色  正常 */
  }
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "Z =%5d|%5d,%5d,%5d", g_tMag.Z, g_tMag.Z_Min, g_tMag.Z_Max,
          g_tMag.Z_Max - g_tMag.Z_Min);
  if (g_tMag.Z_Max - g_tMag.Z_Min < 500)
  {
    tFont.FrontColor = CL_RED; /* 红色 异常*/
  }
  else
  {
    tFont.FrontColor = CL_BLACK; /* 白色  正常 */
  }
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  /* 显示配置寄存器和ID寄存器 */
  y += 3;
  tFont.FrontColor = CL_GREY; /* 字体颜色 */

  sprintf(buf, "CFG_A = 0x%02X,CFG_B = 0x%02X", g_tMag.CfgRegA, g_tMag.CfgRegB);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "Mode  = 0x%02d,ID = %s", g_tMag.ModeReg, (char *)g_tMag.IDReg);
  LCD_DispStr(x, y, buf, &tFont);
}

/*
*********************************************************************************************************
*	函 数 名: DispBMP085
*	功能说明: 显示BMP085输出的数据
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispBMP085(void)
{
  FONT_T tFont;
  char buf[128];
  uint16_t x, y;
  uint8_t ucLineCap = 17;

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
    tFont.FrontColor = CL_BLUE;    /* 字体颜色 */
    tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
    tFont.Space = 0;               /* 文字间距，单位 = 像素 */
  }

  x = BOX3_X + 5;
  y = BOX3_Y + 18;

  /* 温度值， 单位 0.1摄氏度 */
  /* 压力值， 单位 Pa */
  sprintf(buf, "环境温度  = %3d.%d℃", g_tBMP085.Temp / 10, g_tBMP085.Temp % 10);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;

  sprintf(buf, "大气压力  = %3d.%03dKPa", g_tBMP085.Press / 1000, g_tBMP085.Press % 1000);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;
}

/*
*********************************************************************************************************
*	函 数 名: DispBH1750
*	功能说明: 显示BH1750输出的数据
*	形    参: _value : 浮点格式的光照度值
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispBH1750(float _value)
{
  FONT_T tFont;
  char buf[128];
  uint16_t x, y;
  uint8_t ucLineCap = 17;

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
    tFont.FrontColor = CL_BLUE;    /* 字体颜色 */
    tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
    tFont.Space = 0;               /* 文字间距，单位 = 像素 */
  }

  x = BOX4_X + 5;
  y = BOX4_Y + 18;

  sprintf(buf, "光照度  = %6.2f lux", _value);
  LCD_DispStr(x, y, buf, &tFont);
  y += ucLineCap;
}

/*
*********************************************************************************************************
*	函 数 名: DispDS18B20
*	功能说明: 显示DS18B20输出的数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispDS18B20(void)
{
  FONT_T tFont;
  char buf[128];
  uint16_t x, y;

  if (s_use_dht11 == 1)
  {
    /* 在DS18B20程序中 暂时屏蔽, 避免印象 DHT11 */
    return;
  }

  /* 读 ROM ID */
  {
    uint8_t id[8];
    uint8_t ret;
    uint8_t i;

    x = BOX5_X + 5;
    y = BOX5_Y + 18;

    ret = DS18B20_ReadID(id);

    if (ret == 0)
    {
      /* 设置字体参数 红色 */
      {
        tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
        tFont.FrontColor = CL_RED;     /* 字体颜色 */
        tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
        tFont.Space = 0;               /* 文字间距，单位 = 像素 */
      }
    }
    else
    {
      /* 设置字体参数 蓝色 */
      {
        tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
        tFont.FrontColor = CL_BLUE;    /* 字体颜色 */
        tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
        tFont.Space = 0;               /* 文字间距，单位 = 像素 */
      }
    }

    sprintf(buf, "id = ");
    for (i = 0; i < 8; i++)
    {
      sprintf(&buf[strlen(buf)], "%02X", id[i]);
      if (i == 3)
      {
        sprintf(&buf[strlen(buf)], " ");
      }
    }
    LCD_DispStr(x, y, buf, &tFont);
  }

  {
    int16_t reg;

    reg = DS18B20_ReadTempReg();
    sprintf(buf, "reg = 0x%04X -> %-4.04f℃ ", (uint16_t)reg, (float)reg / 16);

    x = BOX5_X + 5;
    y += 17;

    /* 设置字体参数 蓝色 */
    {
      tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
      tFont.FrontColor = CL_BLUE;    /* 字体颜色 */
      tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
      tFont.Space = 0;               /* 文字间距，单位 = 像素 */
    }
    LCD_DispStr(x, y, buf, &tFont);

    LCD_DispStr(x, y + 26, "DS18B20和DHT11不能同时接入开发板", &tFont);
  }
}

/*
*********************************************************************************************************
*	函 数 名: DispDHT11
*	功能说明: 显示DHT11输出的数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispDHT11(void)
{
  FONT_T tFont;
  char buf[128];
  uint16_t x, y;

  DHT11_T tDHT;
  uint8_t ret;

  x = BOX6_X + 5;
  y = BOX6_Y + 18;

  ret = DHT11_ReadData(&tDHT);
  if (ret == 1)
  {
    /* 设置字体参数 蓝色 */
    {
      tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
      tFont.FrontColor = CL_BLUE;    /* 字体颜色 */
      tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
      tFont.Space = 0;               /* 文字间距，单位 = 像素 */
    }
    sprintf(buf, "湿度 %d%%,温度 %d℃       ", tDHT.Hum, tDHT.Temp);
    LCD_DispStr(x, y, buf, &tFont);

    s_use_dht11 = 1; /* 在DS18B20程序中 暂时屏蔽 */
  }
  else
  {
    /* 设置字体参数 红色 */
    {
      tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
      tFont.FrontColor = CL_RED;     /* 字体颜色 */
      tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
      tFont.Space = 0;               /* 文字间距，单位 = 像素 */
    }
    sprintf(buf, "未发现DHT11       ");
    LCD_DispStr(x, y, buf, &tFont);

    s_use_dht11 = 0; /* 在DS18B20程序中 取消屏蔽 */
  }
}

/*
*********************************************************************************************************
*	函 数 名: DispInitFace
*	功能说明: 显示初始界面
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispInitFace(void)
{
  FONT_T tFontBox; /* 定义一个字体结构体变量，用于设置字体参数 */
  GROUP_T tBox;

  LCD_ClrScr(CL_BTN_FACE); /* 清屏，背景蓝色 */

  /* 设置字体参数 */
  {
    /* 分组框字体 */
    tFontBox.FontCode = FC_ST_16;
    tFontBox.BackColor = CL_BTN_FACE;
    tFontBox.FrontColor = CL_BLACK;
    tFontBox.Space = 0;
  }

  /* 显示分组框 */
  {
    tBox.Font = &tFontBox;

    tBox.Left = BOX1_X;
    tBox.Top = BOX1_Y;
    tBox.Height = BOX1_H;
    tBox.Width = BOX1_W;
    tBox.pCaption = BOX1_TEXT;
    LCD_DrawGroupBox(&tBox);

    tBox.Left = BOX2_X;
    tBox.Top = BOX2_Y;
    tBox.Height = BOX2_H;
    tBox.Width = BOX2_W;
    tBox.pCaption = BOX2_TEXT;
    LCD_DrawGroupBox(&tBox);

    tBox.Left = BOX3_X;
    tBox.Top = BOX3_Y;
    tBox.Height = BOX3_H;
    tBox.Width = BOX3_W;
    tBox.pCaption = BOX3_TEXT;
    LCD_DrawGroupBox(&tBox);

    tBox.Left = BOX4_X;
    tBox.Top = BOX4_Y;
    tBox.Height = BOX4_H;
    tBox.Width = BOX4_W;
    tBox.pCaption = BOX4_TEXT;
    LCD_DrawGroupBox(&tBox);

    tBox.Left = BOX5_X;
    tBox.Top = BOX5_Y;
    tBox.Height = BOX5_H;
    tBox.Width = BOX5_W;
    tBox.pCaption = BOX5_TEXT;
    LCD_DrawGroupBox(&tBox);

    tBox.Left = BOX6_X;
    tBox.Top = BOX6_Y;
    tBox.Height = BOX6_H;
    tBox.Width = BOX6_W;
    tBox.pCaption = BOX6_TEXT;
    LCD_DrawGroupBox(&tBox);
  }

  DispButton(BUTTON_RET_ID, 0);
}

/*
*********************************************************************************************************
*	函 数 名: DispButton
*	功能说明: 显示指定位置的按钮
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispButton(uint8_t _id, uint8_t _focus)
{
  FONT_T tFontBtn; /* 定义一个字体结构体变量，用于设置字体参数 */
  BUTTON_T tBtn;

  /* 设置字体参数 */
  {
    /* 按钮字体 */
    tFontBtn.FontCode = FC_ST_16;
    tFontBtn.BackColor = CL_MASK; /* 透明色 */
    tFontBtn.FrontColor = CL_BLACK;
    tFontBtn.Space = 0;
  }

  tBtn.Font = &tFontBtn;

  switch (_id)
  {
  case BUTTON_RET_ID:
    tBtn.Left = BUTTON_RET_X;
    tBtn.Top = BUTTON_RET_Y;
    tBtn.Height = BUTTON_RET_H;
    tBtn.Width = BUTTON_RET_W;
    tBtn.Focus = _focus;
    tBtn.pCaption = BUTTON_RET_TEXT;
    break;

  default:
    return;
  }

  LCD_DrawButton(&tBtn);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
