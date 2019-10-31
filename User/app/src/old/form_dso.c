/*
*********************************************************************************************************
*
*	模块名称 : 双通道示波器程序
*	文件名称 : form_dso.c
*	版    本 : V1.0
*	说    明 : 使用STM32内部ADC测量波形。CH1 = PC0， CH2 = PC1
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2015-06-23  armfly  正式发布
*		V1.1    2015-08-07  armfly  使用堆栈保存大尺寸的数据缓冲区，解决全局变量。
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_dso.h"

#define DSO_VER "V0.6" /* 当前版本 */

/* 主程序状态字 */
enum
{
  S_HELP = 0, /* 帮助页面 */
  S_RUN = 1,  /* 采集状态 */

  S_EXIT = 3
};

/* 时间分度表， g_DSO->TimeBaseId作为数组索引  */
const uint32_t g_TimeTable[] =
    {
        10,
        20, 50, 100,
        200, 500, 1000,
        2000, 5000, 10000,
        20000, 50000, 100000,
        200000, 500000, 1000000};

/* 衰减倍数表  


计算公式：
  iTemp = g_DSO->Ch1VOffset + (int16_t)((2024 - g_DSO->Ch1Buf[i + 1]) * 10) / g_DSO->Ch1Attenuation;

  g_DSO->Ch1Attenuation 是ADC值和像素之间的倍率的10倍。

  1V 档位时:  ADC = 4096 / 5.0 = 819； 像素 = 25
  g_DSO->Ch1Attenuation = 819 / 25 = 32.76

*/
#define ATT_COUNT 6

#ifdef D112_2
const uint32_t g_AttTable[ATT_COUNT][2] =
    {
        /* {除数*0.1, 每大格电压}  1:1 */
        {327, 5000}, /* GAIN = 3, 放大1倍 */
        {260, 2000}, /* GAIN = 2  放大 2倍 */

        {155 * 2, 1000}, /* GAIN = 1 放大 5 倍 */
        {155, 500},      /* GAIN = 1 放大 5倍  */
        {112, 200},      /* Gain = 0 放大 10倍 */
        {112 / 2, 100},  /* Gain = 0 放大 10倍 */
};
#else

#define Y_RATE 327

const uint32_t g_AttTable[ATT_COUNT][2] =
    {
        /* {除数*0.1, 每大格电压}  1:1 */
        //{Y_RATE * 5,	5000},
        //{Y_RATE * 2,	2000},
        {Y_RATE * 5 / 5, 5000}, /* GAIN = 3 */
        {Y_RATE * 2 / 5, 2000},

        {Y_RATE, 1000}, /*　GAIN = 1　*/
        {Y_RATE / 2, 500},
        {Y_RATE / 5, 200},  /*  Gain = 0 */
        {Y_RATE / 10, 100}, /* Gain = 0, 放大 */
};
#endif

static void DsoHelp(uint8_t *pMainStatus);
static void DsoRun(uint8_t *pMainStatus);

/* 按钮 */
/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 80
#define BTN_RET_X (g_LcdWidth - BTN_RET_W - 8)
#define BTN_RET_Y (g_LcdHeight - BTN_RET_H - 4)
#define BTN_RET_T "返回"

DSO_T *g_DSO; /* 全局变量，是一个结构体 */

/* 定义界面结构 */
typedef struct
{
  FONT_T FontBtn; /* 按钮的字体 */

  BUTTON_T BtnRet;

  BUTTON_T Btn1;
  BUTTON_T Btn2;
  BUTTON_T Btn3;
  BUTTON_T Btn4;
  BUTTON_T Btn5;
  BUTTON_T Btn6;
  BUTTON_T Btn7;
  BUTTON_T Btn8;
} FormDSO_T;

FormDSO_T *FormDSO;

static void InitFormDSO(void);

/*
*********************************************************************************************************
*	函 数 名: InitFormDSO
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormDSO(void)
{
  /* 按钮字体 */
  FormDSO->FontBtn.FontCode = FC_ST_16;
  FormDSO->FontBtn.BackColor = CL_MASK; /* 透明背景 */
  FormDSO->FontBtn.FrontColor = CL_BLACK;
  FormDSO->FontBtn.Space = 0;

  /* 按钮 */
  FormDSO->BtnRet.Left = BTN_RET_X;
  FormDSO->BtnRet.Top = BTN_RET_Y;
  FormDSO->BtnRet.Height = BTN_RET_H;
  FormDSO->BtnRet.Width = BTN_RET_W;
  FormDSO->BtnRet.pCaption = BTN_RET_T;
  FormDSO->BtnRet.Font = &FormDSO->FontBtn;
  FormDSO->BtnRet.Focus = 0;

  /*  AC/DC 的Y坐标 = 224 */
  /* void LCD_InitButton(BUTTON_T *_btn, uint16_t _x, uint16_t _y, uint16_t _h, uint16_t _w, 
    char *_pCaption, FONT_T *_pFont); */

  LCD_InitButton(&FormDSO->Btn1, 10, 244, 24, 30, "AC", &FormDSO->FontBtn);       /* 通道1 AC-DC切换 */
  LCD_InitButton(&FormDSO->Btn2, 10 + 35, 244, 24, 30, "+", &FormDSO->FontBtn);   /* 通道1 幅度+ */
  LCD_InitButton(&FormDSO->Btn3, 10 + 65, 244, 24, 30, "-", &FormDSO->FontBtn);   /* 通道1 幅度- */
  LCD_InitButton(&FormDSO->Btn4, 10 + 110, 244, 24, 30, "AC", &FormDSO->FontBtn); /* 通道2 AC-DC切换 */
  LCD_InitButton(&FormDSO->Btn5, 10 + 145, 244, 24, 30, "+", &FormDSO->FontBtn);  /* 通道2 幅度+ */
  LCD_InitButton(&FormDSO->Btn6, 10 + 175, 244, 24, 30, "-", &FormDSO->FontBtn);  /* 通道2 幅度- */

  LCD_InitButton(&FormDSO->Btn7, 10 + 225, 244, 24, 30, "+", &FormDSO->FontBtn); /* 时基+ */
  LCD_InitButton(&FormDSO->Btn8, 10 + 265, 244, 24, 30, "-", &FormDSO->FontBtn); /* 时基- */

  /* 绘制按钮 */
  LCD_DrawButton(&FormDSO->BtnRet);
  LCD_DrawButton(&FormDSO->Btn1);
  LCD_DrawButton(&FormDSO->Btn2);
  LCD_DrawButton(&FormDSO->Btn3);
  LCD_DrawButton(&FormDSO->Btn4);
  LCD_DrawButton(&FormDSO->Btn5);
  LCD_DrawButton(&FormDSO->Btn6);
  LCD_DrawButton(&FormDSO->Btn7);
  LCD_DrawButton(&FormDSO->Btn8);
}

/*
*********************************************************************************************************
*	函 数 名: DsoMain
*	功能说明: 示波器程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DsoMain(void)
{
  uint8_t MainStatus = S_RUN; /* 程序执行状态 */
  DSO_T tDSO;                 /* 很大的一个变量，存放在堆栈。节约全局变量空间 */

  memset(&tDSO, 0, sizeof(tDSO));
  g_DSO = &tDSO;

  /* DAC1输出10KHz，峰峰值2V的正弦波 */
  dac1_SetSinWave(1638, 10000); /* 使用STM32内部DAC1输出正弦波,  第1个参数是幅度(0-2048) 第2个是频率 */

  /* PE6/TIM15_CH2 输出10KHz PWM，占空比5000 = 50% */
  bsp_SetTIMOutPWM(GPIOE, GPIO_PIN_6, TIM15, 2, 10000, 5000);

  {
    DSO_ConfigCtrlGPIO(); /* 配置示波器模块的控制GPIO: 耦合通道和增益 */

    g_DSO->CH1_DC = 0;   /* CH1选择AC耦合 */
    g_DSO->CH2_DC = 0;   /* CH1选择AC耦合 */
    g_DSO->CH1_Gain = 0; /* CH1选择小增益 衰减1/5, 第2个参数1表示不衰减1;1 */
    g_DSO->CH2_Gain = 0; /* CH2选择小增益 衰减1/5, 第2个参数1表示不衰减1;1 */

    DSO_SetDC(1, g_DSO->CH1_DC);
    DSO_SetDC(2, g_DSO->CH2_DC);
    DSO_SetGain(1, g_DSO->CH1_Gain);
    DSO_SetGain(2, g_DSO->CH2_Gain);
  }

  /* 因为蜂鸣器用了TIM1_CH1,  和示波器的ADC采集冲突，因此临时屏蔽按钮提示音 */
  BEEP_Pause();

  /* 进入主程序循环体 */
  while (1)
  {
    switch (MainStatus)
    {
    case S_HELP:
      DsoHelp(&MainStatus); /* 显示帮助 */
      break;

    case S_RUN:
      DsoRun(&MainStatus); /* 全速采集，实时显示 */
      break;

    case S_EXIT:
      dac1_StopWave(); /* 关闭DAC1输出 */
                       //				dac2_StopWave();	/* 关闭DAC2输出 */
                       //				bsp_SetTIMOutPWM(GPIOF, GPIO_PIN_9, TIM14, 1, 0, 5000);

      BEEP_Resume(); /* 恢复蜂鸣器功能 */
      return;

    default:
      break;
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: DispHelp1
*	功能说明: 显示操作提示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispHelp1(void)
{
  uint16_t y;
  uint16_t LineCap;
  FONT_T font;

  /* 分组框标题字体 */
  font.FontCode = FC_ST_16;
  font.BackColor = CL_BLUE;   /* 和背景色相同 */
  font.FrontColor = CL_WHITE; /* 白色文字 */
  font.Space = 0;

  LCD_ClrScr(CL_BLUE); /* 清屏，背景蓝色 */

  y = 0;
  LineCap = 18; /* 行间距 */
  LCD_DispStr(20, y, "安富莱STM32-V5开发板  www.armfly.com", &font);

  font.FrontColor = CL_YELLOW; /* 黄色文字 */

  y += LineCap;
  LCD_DispStr(30, y, "QQ:1295744630     Email:armfly@qq.com", &font);
  y += LineCap;

  y += LineCap;

  LCD_DispStr(30, y, "操作提示:", &font);
  y += LineCap;
  LCD_DispStr(50, y, "K1键     = 切换通道焦点。CH1或CH2", &font);
  y += LineCap;
  LCD_DispStr(50, y, "K2键     = 显示帮助或退出帮助", &font);
  y += LineCap;
  LCD_DispStr(50, y, "K3键     = 暂停或实时运行", &font);
  y += LineCap;
  LCD_DispStr(50, y, "摇杆上键 = 放大波形垂直幅度或向上移动", &font);
  y += LineCap;
  LCD_DispStr(50, y, "摇杆下键 = 缩小波形垂直幅度或向下移动", &font);
  y += LineCap;
  LCD_DispStr(50, y, "摇杆左键 = 水平展宽波形", &font);
  y += LineCap;
  LCD_DispStr(50, y, "摇杆右键 = 水平缩小波形", &font);
  y += LineCap;
  LCD_DispStr(50, y, "摇杆OK键 = 切换摇杆调节模式。幅度或位置", &font);
}

/*
*********************************************************************************************************
*	函 数 名: DsoHelp
*	功能说明: 显示操作提示的状态机
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DsoHelp(uint8_t *pMainStatus)
{
  uint8_t KeyCode;

  uint8_t fRefresh = 1; /* LCD刷新标志 */
  uint8_t SubStatus = 0;

  while (*pMainStatus == S_HELP)
  {
    bsp_Idle();

    if (fRefresh)
    {
      fRefresh = 0;

      if (SubStatus == 0)
      {
        DispHelp1();
      }
    }

    /* 读取按键，大于0表示有键按下 */
    KeyCode = bsp_GetKey();
    if (KeyCode > 0)
    {
      /* 有键按下 */
      switch (KeyCode)
      {
      case KEY_DOWN_K2:
        /* 退出,进入全速运行状态 */
        *pMainStatus = S_RUN;
        break;

      case JOY_DOWN_L: /* 摇杆LEFT键按下 */
      case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
      case KEY_DOWN_K3:
      case JOY_DOWN_OK: /* 摇杆OK键 */
        /* 退出,进入全速运行状态 */
        *pMainStatus = S_EXIT;
        break;

      case JOY_DOWN_U: /* 摇杆UP键按下 */
        break;

      case JOY_DOWN_D: /* 摇杆DOWN键按下 */
        break;

      default:
        break;
      }
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: DispFrame
*	功能说明: 能：显示波形窗口的边框和刻度线
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFrame(void)
{
  uint16_t x, y;

  /* 绘制一个实线矩形框 x, y, h, w */
  LCD_DrawRect(9, 19, 202, 302, CL_WHITE);

  /* 绘制垂直刻度点 */
  for (x = 0; x < 13; x++)
  {
    for (y = 0; y < 41; y++)
    {
      LCD_PutPixel(10 + (x * 25), 20 + (y * 5), CL_WHITE);
    }
  }

  /* 绘制水平刻度点 */
  for (y = 0; y < 9; y++)
  {
    for (x = 0; x < 61; x++)
    {
      LCD_PutPixel(10 + (x * 5), 20 + (y * 25), CL_WHITE);
    }
  }

  /* 绘制垂直中心刻度点 */
  for (y = 0; y < 41; y++)
  {
    LCD_PutPixel(9 + (6 * 25), 20 + (y * 5), CL_WHITE);
    LCD_PutPixel(11 + (6 * 25), 20 + (y * 5), CL_WHITE);
  }

  /* 绘制水平中心刻度点 */
  for (x = 0; x < 61; x++)
  {
    LCD_PutPixel(10 + (x * 5), 19 + (4 * 25), CL_WHITE);
    LCD_PutPixel(10 + (x * 5), 21 + (4 * 25), CL_WHITE);
  }
}

/*
*********************************************************************************************************
*	函 数 名: DispButton
*	功能说明: 显示波形窗口右边的功能按钮（待扩展）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispButton(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: DispCh1Wave
*	功能说明: 显示通道1波形
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispCh1Wave(void)
{
  int16_t i; /* 有符号数 */
  //uint16_t pos;
  uint16_t *px;
  uint16_t *py;
  int16_t iTemp;

  if (g_DSO->Ch1Buf == 0)
  {
    return; /* 还未采集数据直接返回 */
  }

  /* 显示通道1电平标记 */
  {
    static uint16_t y = 70;

    LCD_DrawLine(1, y, 7, y, CL_BLUE); /* 选择蓝色 */

    y = g_DSO->Ch1VOffset;

    if (y < 20)
    {
      y = 20;
    }
    else if (y > 220)
    {
      y = 220;
    }
    LCD_DrawLine(1, y, 5, y, CL_YELLOW);
  }

  //	if (s_DispFirst == 0)
  //	{
  //		s_DispFirst = 1;
  //		LCD_ClrScr(CL_BLUE);  			/* 清屏，背景蓝色 */
  //	}

  if (g_DSO->BufUsed == 0)
  {
    g_DSO->BufUsed = 1;
  }
  else
  {
    g_DSO->BufUsed = 0;
  }

  if (g_DSO->BufUsed == 0)
  {
    px = g_DSO->xCh1Buf1;
    py = g_DSO->yCh1Buf1;
  }
  else
  {
    px = g_DSO->xCh1Buf2;
    py = g_DSO->yCh1Buf2;
  }

  /* 计算当前最新的数据位置，向前递减400个样本 */
  //pos = SAMPLE_COUNT - DMA_GetCurrDataCounter(DMA1_Channel1);
  //pos = 0;

  for (i = 0; i < 300; i++)
  {
    px[i] = 10 + i;
    /* ADC = 2048 是BNC悬空输入时的ADC数值，统计多块板子获得的  */
    iTemp = g_DSO->Ch1VOffset + (int16_t)((2048 - g_DSO->Ch1Buf[i + 1]) * 10) / g_DSO->Ch1Attenuation;

    if (iTemp > 220)
    {
      iTemp = 220;
    }
    else if (iTemp < 20)
    {
      iTemp = 20;
    }
    py[i] = iTemp;
  }

  /* 清除上帧波形 */
  if (g_DSO->BufUsed == 0)
  {
    LCD_DrawPoints(g_DSO->xCh1Buf2, g_DSO->yCh1Buf2, 300, CL_BLUE);
  }
  else
  {
    LCD_DrawPoints(g_DSO->xCh1Buf1, g_DSO->yCh1Buf1, 300, CL_BLUE);
  }

  /* 显示更新的波形 */
  LCD_DrawPoints((uint16_t *)px, (uint16_t *)py, 300, CL_YELLOW);
}

/*
*********************************************************************************************************
*	函 数 名: DispCh2Wave
*	功能说明: 显示通道2波形
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispCh2Wave(void)
{
  int16_t i; /* 有符号数 */
  //uint16_t pos;
  uint16_t *px;
  uint16_t *py;
  int16_t iTemp;

  if (g_DSO->Ch2Buf == 0)
  {
    return; /* 还未采集数据直接返回 */
  }

  /* 显示通道2电平标记 */
  {
    static uint16_t y = 170;

    LCD_DrawLine(1, y, 5, y, CL_BLUE);

    y = g_DSO->Ch2VOffset;

    if (y < 20)
    {
      y = 20;
    }
    else if (y > 220)
    {
      y = 220;
    }
    LCD_DrawLine(1, y, 5, y, CL_GREEN);
  }

  if (g_DSO->BufUsed == 0)
  {
    px = g_DSO->xCh2Buf1;
    py = g_DSO->yCh2Buf1;
  }
  else
  {
    px = g_DSO->xCh2Buf2;
    py = g_DSO->yCh2Buf2;
  }

  /* 计算当前最新的数据位置，向前递减400个样本 */
  //pos = SAMPLE_COUNT - DMA_GetCurrDataCounter(DMA2_Channel5);
  //pos = 0;

  for (i = 0; i < 300; i++)
  {
    px[i] = 10 + i;

    /* ADC = 2048 是BNC悬空输入时的ADC数值，统计多块板子获得的  */
    iTemp = g_DSO->Ch2VOffset + (int16_t)((2048 - g_DSO->Ch2Buf[i + 1]) * 10) / g_DSO->Ch2Attenuation;

    if (iTemp > 220)
    {
      iTemp = 220;
    }
    else if (iTemp < 20)
    {
      iTemp = 20;
    }
    py[i] = iTemp;
  }

  /* 清除上帧波形 */
  if (g_DSO->BufUsed == 0)
  {
    LCD_DrawPoints(g_DSO->xCh2Buf2, g_DSO->yCh2Buf2, 300, CL_BLUE);
  }
  else
  {
    LCD_DrawPoints(g_DSO->xCh2Buf1, g_DSO->yCh2Buf1, 300, CL_BLUE);
  }
  /* 显示更新的波形 */
  LCD_DrawPoints((uint16_t *)px, (uint16_t *)py, 300, CL_GREEN);
}

/*
*********************************************************************************************************
*	函 数 名: DispChInfo
*	功能说明: 显示通道信息
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispChInfo(void)
{
  char buf[32]; /* 字符显示缓冲区 */
  FONT_T font;

  /* 分组框标题字体 */
  font.FontCode = FC_ST_16;
  font.BackColor = CL_BLUE;   /* 和背景色相同 */
  font.FrontColor = CL_WHITE; /* 白色文字 */
  font.Space = 0;

  /* 显示示波器程序版本 */
  LCD_DispStr(10, 2, DSO_VER, &font);

  /* 显示通道1信息 */
  if (g_DSO->CH1_DC == 1)
  {
    strcpy(buf, "CH1 DC ");
  }
  else
  {
    strcpy(buf, "CH1 AC ");
  }

  if (g_DSO->Ch1VScale >= 1000)
  {
    sprintf(&buf[7], "%d.00V", g_DSO->Ch1VScale / 1000);
  }
  else
  {
    sprintf(&buf[7], "%dmV", g_DSO->Ch1VScale);
  }

  if (g_DSO->ActiveCH == 1)
  {
    font.BackColor = CL_YELLOW;   /* 黄色 */
    font.FrontColor = CL_MAGENTA; /* 紫色 */
  }
  else
  {
    font.BackColor = CL_YELLOW; /* 黄色 */
    font.FrontColor = CL_BLUE;  /* 蓝色 */
  }

  LCD_DispStr(10, 224, buf, &font);

  /* 显示通道2信息 */
  font.FrontColor = CL_RED; /* CH2 红色 */
  if (g_DSO->CH2_DC == 1)
  {
    strcpy(buf, "CH2 DC ");
  }
  else
  {
    strcpy(buf, "CH2 AC ");
  }

  if (g_DSO->Ch2VScale >= 1000)
  {
    sprintf(&buf[7], "%d.00V", g_DSO->Ch2VScale / 1000);
  }
  else
  {
    sprintf(&buf[7], "%dmV", g_DSO->Ch2VScale);
  }
  if (g_DSO->ActiveCH == 2)
  {
    font.BackColor = CL_GREEN;    /* 绿色 */
    font.FrontColor = CL_MAGENTA; /* 紫色 */
  }
  else
  {
    font.BackColor = CL_GREEN; /* 绿色 */
    font.FrontColor = CL_BLUE; /* 紫色 */
  }
  LCD_DispStr(120, 224, buf, &font);

  /* 显示时基 */
  font.FrontColor = CL_WHITE; /* 白色 */
  font.BackColor = CL_BLUE;   /* 蓝色 */

  if (g_DSO->TimeBase < 1000)
  {
    sprintf(buf, "Time %3dus", g_DSO->TimeBase);
  }
  else if (g_DSO->TimeBase < 1000000)
  {
    sprintf(buf, "Time %3dms", g_DSO->TimeBase / 1000);
  }
  else
  {
    sprintf(buf, "Time %3ds ", g_DSO->TimeBase / 1000000);
  }
  LCD_DispStr(230, 224, buf, &font);

  /* 显示调节模式 */
  font.FrontColor = CL_WHITE; /* 白字 */
  font.BackColor = CL_BLUE;   /* 蓝底 */

  if (g_DSO->AdjustMode == 1)
  {
    LCD_DispStr(245, 2, "调节位置", &font);
  }
  else
  {
    LCD_DispStr(245, 2, "调节幅度", &font);
  }

  sprintf(buf, "采样频率:%7dHz", g_DSO->SampleFreq);
  LCD_DispStr(75, 2, buf, &font);
}

/*
*********************************************************************************************************
*	函 数 名: DispDSO
*	功能说明: DispDSO
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispDSO(void)
{
  DispButton();

  DispFrame();  /* 绘制刻度框 */
  DispChInfo(); /* 显示通道信息(幅度，时间档位) */

  DispCh1Wave(); /* 显示波形1 */
  DispCh2Wave(); /* 显示波形2 */
}

/*
*********************************************************************************************************
*	函 数 名: InitDsoParam
*	功能说明: 初始化全局参数变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitDsoParam(void)
{
  g_DSO->Ch1Attenuation = 23; /* 设置缺省衰减系数 */
  g_DSO->Ch2Attenuation = 23; /* 设置缺省衰减系数 */
  g_DSO->Ch1VScale = 1000;    /* 缺省是1V */
  g_DSO->Ch2VScale = 1000;    /* 缺省是1V */

  g_DSO->Ch1VOffset = 70;  /* 通道1 GND线位置 */
  g_DSO->Ch2VOffset = 170; /* 通道2 GND线位置 */

  g_DSO->ActiveCH = 1;   /* 缺省是CH1 */
  g_DSO->AdjustMode = 1; /* 缺省是调节垂直偏移， 可以切换到2调节幅度 */

  g_DSO->HoldEn = 0;

  g_DSO->TimeBaseId = 2;
  g_DSO->TimeBase = g_TimeTable[g_DSO->TimeBaseId];
  g_DSO->SampleFreq = 25000000 / g_DSO->TimeBase;

  g_DSO->Ch1AttId = 2;
  g_DSO->Ch1Attenuation = g_AttTable[g_DSO->Ch1AttId][0];
  g_DSO->Ch1VScale = g_AttTable[g_DSO->Ch1AttId][1];

  g_DSO->Ch2AttId = 2;
  g_DSO->Ch2Attenuation = g_AttTable[g_DSO->Ch2AttId][0];
  g_DSO->Ch2VScale = g_AttTable[g_DSO->Ch2AttId][1];
}

/*
*********************************************************************************************************
*	函 数 名: IncSampleFreq
*	功能说明: 增加采样频率，按 1-2-5 
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
  时间轴分度（每1个大格的时长)
    10us 	      2500000	
    20us 	      1250000
    50us 		   500000
    100us		   250000
    200us		   125000
    500us		    50000
    1ms				 2500
    2ms				 1250
    5ms				  500 
    10ms			  250
    20ms			  125
    50ms			   50
    100ms			   25

//		200ms			   12.5
//		500ms			    5

  g_DSO->TimeScale = 25000000 / g_DSO->SampleRate;
*/
static void IncSampleFreq(void)
{
  if (g_DSO->TimeBaseId < (sizeof(g_TimeTable) / 4) - 1)
  {
    g_DSO->TimeBaseId++;
  }

  g_DSO->TimeBase = g_TimeTable[g_DSO->TimeBaseId];
  g_DSO->SampleFreq = 25000000 / g_DSO->TimeBase;

  DSO_SetSampRate(g_DSO->SampleFreq); /* 改变采样频率 */
}

/*
*********************************************************************************************************
*	函 数 名: DecSampleFreq
*	功能说明: 降低采样频率，按 1-2-5 
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DecSampleFreq(void)
{
  if (g_DSO->TimeBaseId > 0)
  {
    g_DSO->TimeBaseId--;
  }

  g_DSO->TimeBase = g_TimeTable[g_DSO->TimeBaseId];
  g_DSO->SampleFreq = 25000000 / g_DSO->TimeBase;

  DSO_SetSampRate(g_DSO->SampleFreq); /* 改变采样频率 */
}

/*
*********************************************************************************************************
*	函 数 名: AdjustAtt
*	功能说明: 调节电压衰减档位，按 1-2-5 
*	形    参: ch   : 通道号，1或2
*			  mode : 0 降低， 1增加
*	返 回 值: 无
*********************************************************************************************************
*/
static void AdjustAtt(uint8_t ch, uint8_t mode)
{

  if (ch == 1)
  {
    if (mode == 0) /* 降低 */
    {
      if (g_DSO->Ch1AttId > 0)
      {
        g_DSO->Ch1AttId--;
      }
    }
    else /* 增加 */
    {
      if (g_DSO->Ch1AttId < ATT_COUNT - 1)
      {
        g_DSO->Ch1AttId++;
      }
    }

    g_DSO->Ch1Attenuation = g_AttTable[g_DSO->Ch1AttId][0];
    g_DSO->Ch1VScale = g_AttTable[g_DSO->Ch1AttId][1];
  }
  else if (ch == 2)
  {
    if (mode == 0) /* 降低 */
    {
      if (g_DSO->Ch2AttId > 0)
      {
        g_DSO->Ch2AttId--;
      }
    }
    else /* 增加 */
    {
      if (g_DSO->Ch2AttId < ATT_COUNT - 1)
      {
        g_DSO->Ch2AttId++;
      }
    }
    g_DSO->Ch2Attenuation = g_AttTable[g_DSO->Ch2AttId][0];
    g_DSO->Ch2VScale = g_AttTable[g_DSO->Ch2AttId][1];
  }
}

/*
*********************************************************************************************************
*	函 数 名: DsoRun
*	功能说明: DSO运行状态
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DsoRun(uint8_t *pMainStatus)
{
  uint8_t KeyCode;
  uint8_t fRefresh = 1; /* LCD刷新标志 */
  FormDSO_T form;
  uint8_t ucTouch;
  int16_t tpX, tpY;

  InitDsoParam(); /* 初始化示波器参数 */

  LCD_ClrScr(CL_BLUE); /* 清屏，背景蓝色 */

  FormDSO = &form;

  InitFormDSO(); /* 绘制按钮 */

  bsp_StartTimer(1, 150); /* 启动定时器1，100ms刷新1次 */
  while (*pMainStatus == S_RUN)
  {
    bsp_Idle();

    if (fRefresh)
    {
      fRefresh = 0;

      DSO_SetDC(1, g_DSO->CH1_DC);
      DSO_SetDC(2, g_DSO->CH2_DC);

      /* 自动切换硬件量程 */
      switch (g_DSO->Ch1AttId)
      {
      case 0: /* 5V */
        DSO_SetGain(1, 3);
        break;

      case 1: /* 2V */
        DSO_SetGain(1, 2);
        break;

      case 2: /* 1V */
      case 3: /* 500mV */
        DSO_SetGain(1, 1);
        break;

      case 4: /* 200mV */
      case 5: /* 100mV */
        DSO_SetGain(1, 0);
        break;
      }

      switch (g_DSO->Ch2AttId)
      {
      case 0: /* 5V */
        DSO_SetGain(2, 3);
        break;

      case 1: /* 2V */
        DSO_SetGain(2, 2);
        break;

      case 2: /* 1V */
      case 3: /* 500mV */
        DSO_SetGain(2, 1);
        break;

      case 4: /* 200mV */
      case 5: /* 100mV */
        DSO_SetGain(2, 0);
        break;
      }

      /* 根据增益设置，改变硬件衰减 */

      if (g_DSO->HoldEn == 1)
      {
        DispDSO();
      }
    }

    if (bsp_CheckTimer(1))
    {
      bsp_StartTimer(1, 200); /* 启动定时器1，200ms刷新1次 */

      /* 运行状态。每隔100ms刷新1次波形 */
      if (g_DSO->HoldEn == 0)
      {
        DSO_PauseADC(); /* 暂停采样 */

        DispDSO(); /* 显示波形 */

        /* 开始采样 */
        DSO_StartADC(&g_DSO->Ch1Buf, &g_DSO->Ch2Buf, g_DSO->SampleFreq);
      }
    }

    ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
    if (ucTouch != TOUCH_NONE)
    {
      switch (ucTouch)
      {
      case TOUCH_DOWN: /* 触笔按下事件 */
        if (LCD_ButtonTouchDown(&FormDSO->BtnRet, tpX, tpY))
        {
          // *pMainStatus = S_EXIT;  <--- 在松开时退出界面
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn1, tpX, tpY))
        {
          /* 通道1 AC-DC耦合切换 */
          if (g_DSO->CH1_DC == 0)
          {
            g_DSO->CH1_DC = 1;
          }
          else
          {
            g_DSO->CH1_DC = 0;
          }
          fRefresh = 1;
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn2, tpX, tpY))
        {
          AdjustAtt(1, 1); /* 通道1 幅度调节+ */
          fRefresh = 1;
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn3, tpX, tpY))
        {
          AdjustAtt(1, 0); /* 通道1 幅度调节- */
          fRefresh = 1;
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn4, tpX, tpY))
        {
          /* 通道2 AC-DC耦合切换 */
          if (g_DSO->CH2_DC == 0)
          {
            g_DSO->CH2_DC = 1;
          }
          else
          {
            g_DSO->CH2_DC = 0;
          }
          fRefresh = 1;
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn5, tpX, tpY))
        {
          AdjustAtt(2, 1); /* 通道2 幅度调节+ */
          fRefresh = 1;
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn6, tpX, tpY))
        {
          AdjustAtt(2, 0); /* 通道2 幅度调节- */
          fRefresh = 1;
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn7, tpX, tpY))
        {
          DecSampleFreq(); /* 递减采样频率 */
          fRefresh = 1;    /* 请求刷新LCD */
        }
        else if (LCD_ButtonTouchDown(&FormDSO->Btn8, tpX, tpY))
        {
          IncSampleFreq(); /* 递增采样频率 */
          fRefresh = 1;    /* 请求刷新LCD */
        }
        break;

      case TOUCH_RELEASE: /* 触笔释放事件 */
        if (LCD_ButtonTouchRelease(&FormDSO->BtnRet, tpX, tpY))
        {
          *pMainStatus = S_EXIT; /*　返回键退出　*/
        }
        else
        {
          LCD_ButtonTouchRelease(&FormDSO->Btn1, tpX, tpY);
          LCD_ButtonTouchRelease(&FormDSO->Btn2, tpX, tpY);
          LCD_ButtonTouchRelease(&FormDSO->Btn3, tpX, tpY);
          LCD_ButtonTouchRelease(&FormDSO->Btn4, tpX, tpY);
          LCD_ButtonTouchRelease(&FormDSO->Btn5, tpX, tpY);
          LCD_ButtonTouchRelease(&FormDSO->Btn6, tpX, tpY);
          LCD_ButtonTouchRelease(&FormDSO->Btn7, tpX, tpY);
          LCD_ButtonTouchRelease(&FormDSO->Btn8, tpX, tpY);
        }
        break;
      }
    }

    /* 读取按键，大于0表示有键按下 */
    KeyCode = bsp_GetKey();
    if (KeyCode > 0)
    {
      /* 有键按下 */
      switch (KeyCode)
      {
      case KEY_DOWN_K1: /* TAMPER 键，通道选择(CH1或CH2) */
        if (g_DSO->ActiveCH == 1)
        {
          g_DSO->ActiveCH = 2;
        }
        else
        {
          g_DSO->ActiveCH = 1;
        }
        fRefresh = 1; /* 请求刷新LCD */
        break;

      case KEY_DOWN_K2: /* WAKEUP 键, 调节模式选择(幅度或者垂直偏移) */
        /* 退出,进入全速运行状态 */
        *pMainStatus = S_HELP;
        break;

      case KEY_DOWN_K3: /* USER 键 */
        if (g_DSO->HoldEn == 0)
        {
          g_DSO->HoldEn = 1;

          /* 保存暂停时的时基,为了水平扩展用 */
          g_DSO->TimeBaseIdHold = g_DSO->TimeBaseId;

          DSO_StopADC();
        }
        else
        {
          g_DSO->HoldEn = 0;
        }
        fRefresh = 1; /* 请求刷新LCD */
        break;

      case JOY_DOWN_L: /* 摇杆LEFT键按下 */
        if (g_DSO->HoldEn == 0)
        {
          DecSampleFreq(); /* 递减采样频率 */
          fRefresh = 1;    /* 请求刷新LCD */
        }
        else
        {
          ; /* 波形水平移动，待完善 */
        }
        break;

      case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
        if (g_DSO->HoldEn == 0)
        {
          IncSampleFreq(); /* 递增采样频率 */
          fRefresh = 1;    /* 请求刷新LCD */
        }
        else
        {
          ; /* 波形水平移动，待完善 */
        }
        break;

      case JOY_DOWN_OK: /* 摇杆OK键 */
        if (g_DSO->AdjustMode == 0)
        {
          g_DSO->AdjustMode = 1;
        }
        else
        {
          g_DSO->AdjustMode = 0;
        }
        fRefresh = 1; /* 请求刷新LCD */
        break;

      case JOY_DOWN_U:            /* 摇杆UP键按下 */
        if (g_DSO->ActiveCH == 1) /* 当前激活的是CH1 */
        {
          if (g_DSO->AdjustMode == 0) /* 调节幅度放大倍数 */
          {
            AdjustAtt(1, 1);
          }
          else /* 调节上下偏移 */
          {
            g_DSO->Ch1VOffset -= 5;
          }
        }
        else /* 当前激活的是CH2 */
        {
          if (g_DSO->AdjustMode == 0) /* 调节幅度放大倍数 */
          {
            AdjustAtt(2, 1);
          }
          else /* 调节上下偏移 */
          {
            g_DSO->Ch2VOffset -= 5;
          }
        }
        fRefresh = 1; /* 请求刷新LCD */
        break;

      case JOY_DOWN_D:            /* 摇杆DOWN键按下 */
        if (g_DSO->ActiveCH == 1) /* 当前激活的是CH1 */
        {
          if (g_DSO->AdjustMode == 0) /* 调节幅度放大倍数 */
          {
            AdjustAtt(1, 0);
          }
          else /* 调节上下偏移 */
          {
            g_DSO->Ch1VOffset += 5;
          }
        }
        else /* 当前激活的是CH2 */
        {
          if (g_DSO->AdjustMode == 0) /* 调节幅度放大倍数 */
          {
            AdjustAtt(2, 0);
          }
          else /* 调节上下偏移 */
          {
            g_DSO->Ch2VOffset += 5;
          }
        }
        fRefresh = 1; /* 请求刷新LCD */
        break;

      default:
        break;
      }
    }
  }

  DSO_StopADC(); /* 关闭采样 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
