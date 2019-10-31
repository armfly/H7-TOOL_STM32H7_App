/*
*********************************************************************************************************
*
*	模块名称 : 程序功能选择
*	文件名称 : status_MenuMain.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-12-06 armfly  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "param.h"
#include "main.h"
#include "images.h" /* 图标文件 */
#include "status_menu.h"
#include "modbus_reg_addr.h"

#define FORM_BACK_COLOR CL_BLUE

#define ICON_HEIGHT 48 /* 矩形图标高度 */
#define ICON_WIDTH 48  /* 矩形图标宽度 */

#define ICON_STEP_X 80 /* 图标之间的间隔 */
#define ICON_STEP_Y 70 /* 图标之间的间隔 */

#define MENU_COUNT 5
static const ICON_T s_tMainIcons[MENU_COUNT] =
    {
        {ID_ICON, 40, 20, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acchujiao, "1.联机模式"},
        {ID_ICON, 40, 20, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acLCD, "2.电压表"},
        {ID_ICON, 40, 20, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acRadio, "3.NTC测温"},
        {ID_ICON, 40, 20, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acRecorder, "4.电阻表"},
        {ID_ICON, 40, 20, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acchujiao, "5.负载电流"},
};

/*
*********************************************************************************************************
*	函 数 名: status_MenuMain
*	功能说明: 功能菜单
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_MenuMain(void)
{
  uint8_t ucKeyCode; /* 按键代码 */
  uint8_t fRefresh;
  FONT_T tFont;     /* 定义字体结构体变量 */
  FONT_T tIconFont; /* 定义一个字体结构体变量，用于图标文本 */
  static uint8_t s_menu_idx = 0;
  uint8_t ucIgnoreKey = 0;

  /* 设置字体参数 */
  {
    uint16_t x;
    uint16_t y;
    char buf[64];

    tFont.FontCode = FC_ST_16;         /* 字体代码 16点阵 */
    tFont.FrontColor = CL_YELLOW;      /* 字体颜色 */
    tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
    tFont.Space = 0;                   /* 文字间距，单位 = 像素 */

    LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

    x = 5;

    y = 3;
    LCD_DispStr(x, y, "选择功能", &tFont);

    y = y + 20;
    sprintf(buf, "MAC:%02X-%02X-%02X-%02X-%02X-%02X",
            g_tVar.MACaddr[0], g_tVar.MACaddr[1], g_tVar.MACaddr[2],
            g_tVar.MACaddr[3], g_tVar.MACaddr[4], g_tVar.MACaddr[5]);
    LCD_DispStr(x, y, buf, &tFont);

    y = y + 20;
    sprintf(buf, "IP:%d.%d.%d.%d",
            g_tVar.MACaddr[0], g_tVar.MACaddr[1], g_tVar.MACaddr[2],
            g_tVar.MACaddr[3], g_tVar.MACaddr[4], g_tVar.MACaddr[5]);
    LCD_DispStr(x, y, buf, &tFont);

    LCD_SetBackLight(BRIGHT_DEFAULT); /* 打开背光，设置为缺省亮度 */

    /* 设置字体参数 */
    {
      tIconFont.FontCode = FC_ST_16;   /* 字体代码 16点阵 */
      tIconFont.FrontColor = CL_WHITE; /* 字体颜色 */
      tIconFont.BackColor = CL_BLUE;   /* 文字背景颜色 */
      tIconFont.Space = 1;             /* 文字间距，单位 = 像素 */
    }
  }

  bsp_ClearKey(); /* 清缓存的按键 */

  /* 从其他界面返回后需要忽略第1个C键弹起事件 */
  if (bsp_GetKeyState(KID_S) || bsp_GetKeyState(KID_C))
  {
    ucIgnoreKey = 1;
  }

  fRefresh = 1;
  while (g_MainStatus == MS_MAIN_MENU)
  {
    bsp_Idle();

    if (fRefresh) /* 刷新整个界面 */
    {
      fRefresh = 0;
      LCD_DrawIcon32(&s_tMainIcons[s_menu_idx], &tIconFont, 0);
    }

    ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
    if (ucKeyCode != KEY_NONE)
    {
      /* 有键按下 */
      switch (ucKeyCode)
      {
      case KEY_UP_S: /* S键 */
        BEEP_KeyTone();
        LCD_ClrScr(CL_BLUE);
        if (++s_menu_idx >= MENU_COUNT)
        {
          s_menu_idx = 0;
        }
        fRefresh = 1;
        break;

      case KEY_UP_C: /* C键 */
        if (ucIgnoreKey == 1)
        {
          ucIgnoreKey = 0; /* 丢弃第1个按键弹起事件 */
          break;
        }
        BEEP_KeyTone();
        LCD_ClrScr(CL_BLUE);
        if (s_menu_idx > 0)
        {
          s_menu_idx--;
        }
        else
        {
          s_menu_idx = MENU_COUNT - 1;
        }
        fRefresh = 1;
        break;

      case KEY_LONG_S: /* S键 */
        BEEP_KeyTone();
        g_MainStatus = MS_SYSTEM_SET;
        break;

      case KEY_LONG_C: /* C键 */
        BEEP_KeyTone();
        if (++g_tParam.DispDir > 3)
        {
          g_tParam.DispDir = 0;
        }
        LCD_SetDirection(g_tParam.DispDir);
        SaveParam();
        LCD_ClrScr(CL_BLUE);
        fRefresh = 1;
        ucIgnoreKey = 1;
        break;

      default:
        break;
      }
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: status_LinkMode
*	功能说明: 联机模式（功能由上位机控制）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_LinkMode(void)
{
  uint8_t ucKeyCode; /* 按键代码 */
  uint8_t fRefresh;
  FONT_T tFont;     /* 定义字体结构体变量 */
  FONT_T tIconFont; /* 定义一个字体结构体变量，用于图标文本 */
  static uint8_t s_menu_idx = 0;
  uint8_t ucIgnoreKey = 0;

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_24;         /* 字体代码 16点阵 */
    tFont.FrontColor = CL_WHITE;       /* 字体颜色 */
    tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
    tFont.Space = 0;                   /* 文字间距，单位 = 像素 */

    LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

    LCD_DispStr(5, 3, "直流电压表", &tFont);
  }

  bsp_ClearKey(); /* 清缓存的按键 */

  /* 从其他界面返回后需要忽略第1个C键弹起事件 */
  if (bsp_GetKeyState(KID_S) || bsp_GetKeyState(KID_C))
  {
    ucIgnoreKey = 1;
  }

  /* 初始化DSO硬件电路 */
  {
    DSO_InitHard();
    DSO_SetDC(1, 1);   /* CH1 选DC耦合 */
    DSO_SetDC(2, 1);   /* CH2 选DC耦合 */
    DSO_SetGain(1, 0); /* CH1 增益选择最小（量程最大） */
    DSO_SetGain(2, 0); /* CH2 增益选择最小（量程最大） */
  }

  fRefresh = 1;
  bsp_StartAutoTimer(0, 100); /* 100ms 刷新 */
  while (g_MainStatus == MS_VOLT_METER)
  {
    bsp_Idle();

    if (fRefresh) /* 刷新整个界面 */
    {
      fRefresh = 0;
      ;
    }

    ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
    if (ucKeyCode != KEY_NONE)
    {
      /* 有键按下 */
      switch (ucKeyCode)
      {
      case KEY_UP_S: /* S键 */
        BEEP_KeyTone();
        LCD_ClrScr(CL_BLUE);
        fRefresh = 1;
        break;

      case KEY_UP_C: /* C键 */
        if (ucIgnoreKey == 1)
        {
          ucIgnoreKey = 0; /* 丢弃第1个按键弹起事件 */
          break;
        }
        BEEP_KeyTone();
        LCD_ClrScr(CL_BLUE);
        fRefresh = 1;
        break;

      case KEY_LONG_S: /* S键 */
        BEEP_KeyTone();
        g_MainStatus = MS_SYSTEM_SET;
        break;

      case KEY_LONG_C: /* C键 */
        BEEP_KeyTone();
        if (++g_tParam.DispDir > 3)
        {
          g_tParam.DispDir = 0;
        }
        LCD_SetDirection(g_tParam.DispDir);
        SaveParam();
        LCD_ClrScr(CL_BLUE);
        fRefresh = 1;
        ucIgnoreKey = 1;
        break;

      default:
        break;
      }
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: status_VoltMeter
*	功能说明: 电压表
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_VoltMeter(void)
{
  uint8_t ucKeyCode; /* 按键代码 */
  uint8_t fRefresh;
  FONT_T tFont;     /* 定义字体结构体变量 */
  FONT_T tIconFont; /* 定义一个字体结构体变量，用于图标文本 */
  static uint8_t s_menu_idx = 0;
  uint8_t ucIgnoreKey = 0;

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_24;         /* 字体代码 16点阵 */
    tFont.FrontColor = CL_WHITE;       /* 字体颜色 */
    tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
    tFont.Space = 0;                   /* 文字间距，单位 = 像素 */

    LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

    LCD_DispStr(5, 3, "直流电压表", &tFont);
  }

  bsp_ClearKey(); /* 清缓存的按键 */

  /* 从其他界面返回后需要忽略第1个C键弹起事件 */
  if (bsp_GetKeyState(KID_S) || bsp_GetKeyState(KID_C))
  {
    ucIgnoreKey = 1;
  }

  /* 初始化DSO硬件电路 */
  {
    DSO_InitHard();
    DSO_SetDC(1, 1);   /* CH1 选DC耦合 */
    DSO_SetDC(2, 1);   /* CH2 选DC耦合 */
    DSO_SetGain(1, 0); /* CH1 增益选择最小（量程最大） */
    DSO_SetGain(2, 0); /* CH2 增益选择最小（量程最大） */
  }

  fRefresh = 1;
  bsp_StartAutoTimer(0, 100); /* 100ms 刷新 */
  while (g_MainStatus == MS_VOLT_METER)
  {
    bsp_Idle();

    if (fRefresh) /* 刷新整个界面 */
    {
      fRefresh = 0;
      ;
    }

    ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
    if (ucKeyCode != KEY_NONE)
    {
      /* 有键按下 */
      switch (ucKeyCode)
      {
      case KEY_UP_S: /* S键 */
        BEEP_KeyTone();
        LCD_ClrScr(CL_BLUE);
        fRefresh = 1;
        break;

      case KEY_UP_C: /* C键 */
        if (ucIgnoreKey == 1)
        {
          ucIgnoreKey = 0; /* 丢弃第1个按键弹起事件 */
          break;
        }
        BEEP_KeyTone();
        LCD_ClrScr(CL_BLUE);
        fRefresh = 1;
        break;

      case KEY_LONG_S: /* S键 */
        BEEP_KeyTone();
        g_MainStatus = MS_SYSTEM_SET;
        break;

      case KEY_LONG_C: /* C键 */
        BEEP_KeyTone();
        if (++g_tParam.DispDir > 3)
        {
          g_tParam.DispDir = 0;
        }
        LCD_SetDirection(g_tParam.DispDir);
        SaveParam();
        LCD_ClrScr(CL_BLUE);
        fRefresh = 1;
        ucIgnoreKey = 1;
        break;

      default:
        break;
      }
    }
  }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
