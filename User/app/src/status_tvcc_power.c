/*
*********************************************************************************************************
*
*   模块名称 : 微型数控电源
*   文件名称 : status_TVccPower.c
*   版    本 : V1.0
*   说    明 : 通过TVCC输出可调电压。电压范围1.2-5.0V，电流限制400mA
*   修改记录 :
*       版本号  日期        作者     说明
*       V1.0    2019-11-04  armfly  正式发布1
*
*   Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

static void DispTVccVoltCurr(void);
static void DispHelpTVCCPower(void);
static void DispTVccSetting(uint16_t _volt);

/*
*********************************************************************************************************
*   函 数 名: status_TVCCPower
*   功能说明: 微型电源状态。TVCC引脚输出电压可调，电流可以监视。400mA限流。
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
void status_TVCCPower(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    uint8_t ucIgnoreKey = 0;
    uint8_t ucAdjustMode = 0;
    uint16_t NowVolt;       /* 当前设置电压 mV */

    DispHeader("微型数控电源");
    DispHelpTVCCPower();
    
    fRefresh = 1;
    
    NowVolt = 3300;
    DispTVccSetting(NowVolt);
    bsp_StartAutoTimer(0, 300);     
    while (g_MainStatus == MS_TVCC_POWER)
    {
        bsp_Idle();

        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

            bsp_SetTVCC(NowVolt);
            DispTVccVoltCurr();
        }

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
            case KEY_DOWN_S:    /* S键按下 */
                break;

            case KEY_UP_S:      /* S键释放 */
                if (ucAdjustMode == 0)
                {
                    g_MainStatus = NextStatus(MS_TVCC_POWER);
                }
                else
                {
                    if (NowVolt < 5000)
                    {
                        NowVolt += 100;
                        DispTVccSetting(NowVolt);
                    }
                    else
                    {
                        BEEP_Start(5, 5, 3);    /* 叫50ms，停50ms，循环3次 */
                    }
                }
                break;

            case KEY_LONG_S:    /* S键长按 */
                ucAdjustMode = 1;
                BEEP_KeyTone();
                break;

            case KEY_DOWN_C:    /* C键按下 */
                break;

            case KEY_UP_C:      /* C键释放 */
                if (ucAdjustMode == 0 && ucIgnoreKey == 0)
                {                
                    g_MainStatus = LastStatus(MS_TVCC_POWER);
                }
                else
                {
                    if (NowVolt > 1200)
                    {
                        NowVolt -= 100;
                        DispTVccSetting(NowVolt);
                    }
                    else
                    {
                        BEEP_Start(5,5,3);      /* 叫50ms，停50ms，循环3次 */
                    }                    
                }                
                ucIgnoreKey = 0;
                break;

            case KEY_LONG_C:    /* C键长按 */
                ucAdjustMode = 0;
                ucIgnoreKey = 1;    /* 需要丢弃即将到来的C键弹起事件 */
                BEEP_KeyTone();
                break;

            default:
                break;
            }
        }
    }
    bsp_StopTimer(0);    
    
    bsp_SetTVCC(3300);      /* 退出时还原为3.3V */
}

/*
*********************************************************************************************************
*   函 数 名: DispHelpTVCCPower
*   功能说明: 显示操作提示
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void DispHelpTVCCPower(void)
{
    FONT_T tFont; /* 定义字体结构体变量 */

    tFont.FontCode = FC_ST_16;          /* 字体代码 16点阵 */
    tFont.FrontColor = CL_YELLOW;       /* 字体颜色 */
    tFont.BackColor = FORM_BACK_COLOR;  /* 文字背景颜色 */
    tFont.Space = 0;                    /* 文字间距，单位 = 像素 */
    tFont.FrontColor = CL_BLACK;        /* 黑字 */

    LCD_DispStr(5, 240 - 40, "TVCC输出1.2-5.0V, 限流400mA", &tFont);
    LCD_DispStr(5, 240 - 20, "长按S进入调节状态", &tFont);
}
        
/*
*********************************************************************************************************
*   函 数 名: DispTVccVoltCurr
*   功能说明: 显示电压电流
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void DispTVccVoltCurr(void)
{
    FONT_T tFont;
    char buf[64];

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_24;          /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE;        /* 字体颜色 */
        tFont.BackColor = HEAD_BAR_COLOR;   /* 文字背景颜色 */
        tFont.Space = 0;                    /* 文字间距，单位 = 像素 */
    }

    sprintf(buf, "电压: %8.3fV", g_tVar.TVCCVolt );
    LCD_DispStrEx(10, 50 + 32 * 0, buf, &tFont, 220, ALIGN_CENTER);

    sprintf(buf, "电流: %8.2fmA", g_tVar.TVCCCurr);
    LCD_DispStrEx(10, 50 + 32 * 1, buf, &tFont, 220, ALIGN_CENTER);

    sprintf(buf, "功率: %8.3fW", g_tVar.TVCCVolt * g_tVar.TVCCCurr / 1000);
    LCD_DispStrEx(10, 50 + 32 * 2, buf, &tFont, 220, ALIGN_CENTER);   
}

/*
*********************************************************************************************************
*   函 数 名: DispTVccSetting
*   功能说明: 显示设置电压
*   形    参: _volt : mV (1200 - 5000)
*   返 回 值: 无
*********************************************************************************************************
*/
static void DispTVccSetting(uint16_t _volt)
{
    FONT_T tFont;
    char buf[64];

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_24;          /* 字体代码 16点阵 */
        tFont.FrontColor = CL_YELLOW;       /* 字体颜色 */
        tFont.BackColor = HEAD_BAR_COLOR;   /* 文字背景颜色 */
        tFont.Space = 0;                    /* 文字间距，单位 = 像素 */
    }

    sprintf(buf, "设置电压: %d.%dV", _volt / 1000, (_volt % 1000) / 100);
    LCD_DispStrEx(10, 70 + 32 * 3, buf, &tFont, 220, ALIGN_CENTER);      
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
