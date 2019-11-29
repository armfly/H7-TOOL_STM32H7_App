/*
*********************************************************************************************************
*
*    模块名称 : 频率计，脉冲计数
*    文件名称 : status_pulse_meter.c
*    版    本 : V1.0
*    说    明 : 电压表
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2019-10-06 armfly  正式发布
*
*    Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

static void DispPulse(void);
static void DispHelpPulseMeter(void);

/*
*********************************************************************************************************
*    函 数 名: status_VoltageMeter
*    功能说明: 电压表状态.
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_PulseMeter(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    uint8_t ucIgnoreKey = 0;

    DispHeader("脉冲测量");
    DispHelpPulseMeter();
    
    fRefresh = 1;
    
    bsp_StartAutoTimer(0, 1000);
    while (g_MainStatus == MS_PULSE_METER)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

            bsp_GetPulseParam();
            DispPulse();
        }

        bsp_Idle();
        
        if (bsp_CheckTimer(0))
        {
            fRefresh = 1;
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
                if (ucIgnoreKey == 0)
                {
                    g_MainStatus = NextStatus(g_MainStatus);
                }        
                ucIgnoreKey = 0;
                break;

            case KEY_LONG_S: /* S键长按 */
                bsp_StartDetectPulse(); 
                PlayKeyTone();
                ucIgnoreKey = 1;
                break;

            case KEY_DOWN_C: /* C键按下 */
                break;

            case KEY_UP_C: /* C键释放 */
                if (ucIgnoreKey == 0)
                {
                    g_MainStatus = LastStatus(g_MainStatus);
                }            
                ucIgnoreKey = 0;
                break;

            case KEY_LONG_C: /* C键长按 */
                bsp_StopDetectPulse(); 
                PlayKeyTone();                
                ucIgnoreKey = 1;
                break;

            default:
                break;
            }
        }
    }
    bsp_StopTimer(0);
    bsp_StopDetectPulse();     
}

/*
*********************************************************************************************************
*   函 数 名: DispHelpPulseMeter
*   功能说明: 显示操作提示
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void DispHelpPulseMeter(void)
{
    FONT_T tFont;   /* 定义字体结构体变量 */

    tFont.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
    tFont.FrontColor = HELP_TEXT_COLOR;     /* 字体颜色 */
    tFont.BackColor = HELP_BACK_COLOR;      /* 文字背景颜色 */
    tFont.Space = 0;                        /* 文字间距，单位 = 像素 */

    LCD_DispStr(5, 240 - 40, "D1输入信号 (0.1Hz-500KHz)", &tFont);
    LCD_DispStr(5, 240 - 20, "长按S开始测量", &tFont);
}

/*
*********************************************************************************************************
*    函 数 名: DispPulse
*    功能说明: 显示脉冲参数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispPulse(void)
{
    FONT_T tFont;
    char buf[64];

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_24;              /* 字体代码 */
        tFont.FrontColor = VALUE_TEXT_COLOR;    /* 字体颜色 */
        tFont.BackColor = VALUE_BACK_COLOR;     /* 文字背景颜色 */
        tFont.Space = 0;                        /* 文字间距，单位 = 像素 */
    }

    sprintf(buf, " 频  率: %7.3fHz", g_tPulse.Freq);
    LCD_DispStrEx(10, 50, buf, &tFont, 220, ALIGN_LEFT);

    sprintf(buf, " 占空比: %7.3f%%", g_tPulse.Duty);
    LCD_DispStrEx(10, 100, buf, &tFont, 220, ALIGN_LEFT);

    sprintf(buf, " 个  数: %7d", (int32_t)g_tPulse.Count);
    LCD_DispStrEx(10, 150, buf, &tFont, 220, ALIGN_LEFT);    
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
