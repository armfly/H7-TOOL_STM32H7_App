/*
*********************************************************************************************************
*
*	模块名称 : 高侧电压电流表
*	文件名称 : status_current_meter.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2019-10-19 armfly  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

/*
*********************************************************************************************************
*	函 数 名: status_CurrentMeter
*	功能说明: 高侧电压电流表态.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_CurrentMeter(void)
{
  uint8_t ucKeyCode; /* 按键代码 */
  uint8_t fRefresh;
  FONT_T tFont; /* 定义字体结构体变量 */

  DispHeader("高侧电流表");

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;         /* 字体代码 16点阵 */
    tFont.FrontColor = CL_BLACK;       /* 字体颜色 */
    tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
    tFont.Space = 0;                   /* 文字间距，单位 = 像素 */
  }

  fRefresh = 1;
  while (g_MainStatus == MS_CURRENT_METER)
  {
    bsp_Idle();

    if (fRefresh) /* 刷新整个界面 */
    {
      fRefresh = 0;
    }

    ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
    if (ucKeyCode != KEY_NONE)
    {
      /* 有键按下 */
      switch (ucKeyCode)
      {
      case KEY_DOWN_S: /* S键按下 */
        break;

      case KEY_UP_S: /* S键释放 */
        g_MainStatus = NextStatus(MS_CURRENT_METER);
        break;

      case KEY_LONG_S: /* S键长按 */
        break;

      case KEY_DOWN_C: /* C键按下 */
        break;

      case KEY_UP_C: /* C键释放 */
        g_MainStatus = LastStatus(MS_CURRENT_METER);
        break;

      case KEY_LONG_C: /* C键长按 */
        break;

      default:
        break;
      }
    }
  }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
