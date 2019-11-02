/*
*********************************************************************************************************
*
*   模块名称 : 高侧电压电流表
*   文件名称 : status_current_meter.c
*   版    本 : V1.1
*   说    明 : 
*   修改记录 :
*       版本号  日期        作者     说明
*       V1.1    2019-10-19  armfly  正式发布
*       V1.2    2019-11-03  armfly  完善功能。增加电池容量计算。
*
*   Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

static void DispCurrentVolt(void);
static void AutoCurrentRange(void);
static void DispHelpCurrentMeter(void);

/*
*********************************************************************************************************
*   函 数 名: status_CurrentMeter
*   功能说明: 高侧电压电流表态.
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
void status_CurrentMeter(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    uint8_t ucIgnoreKey = 0;

    DispHeader("高侧电流表");
    DispHelpCurrentMeter();
    
    fRefresh = 1;

    bsp_StartAutoTimer(0, 300);     
    bsp_StartAutoTimer(1, 1000);
    while (g_MainStatus == MS_CURRENT_METER)
    {
        bsp_Idle();

        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

            DispCurrentVolt();
            AutoCurrentRange();
        }

        if (bsp_CheckTimer(0))
        {
            fRefresh = 1;
        }
        
        /* 每秒统计一次 */
        if (bsp_CheckTimer(1))
        {
            if (g_tVar.StartBatCap == 1)
            {
                /* 1mAh = 0.001安培*3600秒 = 3.6安培秒 = 3.6库仑 */                
                g_tVar.BatteryCapacity += g_tVar.HighSideCurr / 3600;
            }
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
                if (g_tVar.StartBatCap == 0)
                {
                    g_MainStatus = NextStatus(MS_CURRENT_METER);
                }
                
                break;

            case KEY_LONG_S:    /* S键长按 */
                g_tVar.StartBatCap = 1;
                g_tVar.BatteryCapacity = 0;
                BEEP_KeyTone();
                break;

            case KEY_DOWN_C:    /* C键按下 */
                break;

            case KEY_UP_C:      /* C键释放 */
                if (g_tVar.StartBatCap == 0 && ucIgnoreKey == 0)
                {                
                    g_MainStatus = LastStatus(MS_CURRENT_METER);
                }
                ucIgnoreKey = 0;
                break;

            case KEY_LONG_C:    /* C键长按 */
                g_tVar.StartBatCap = 0;
                ucIgnoreKey = 1;    /* 需要丢弃即将到来的C键弹起事件 */
                BEEP_KeyTone();
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
*   函 数 名: AutoCurrentRange
*   功能说明: 自动调节量程
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void AutoCurrentRange(void)
{
    if (g_tVar.HighSideCurr < 120)
    {
        WriteRegValue_06H(0x0211, 0);   /* 0表示120mA, 1表示1.2A量程 */
    }
    else
    {
        WriteRegValue_06H(0x0211, 1);   /* 0表示120mA, 1表示1.2A量程 */
    }
}

/*
*********************************************************************************************************
*   函 数 名: DispHelpCurrentMeter
*   功能说明: 显示操作提示
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void DispHelpCurrentMeter(void)
{
    FONT_T tFont; /* 定义字体结构体变量 */

    tFont.FontCode = FC_ST_16;          /* 字体代码 16点阵 */
    tFont.FrontColor = CL_YELLOW;       /* 字体颜色 */
    tFont.BackColor = FORM_BACK_COLOR;  /* 文字背景颜色 */
    tFont.Space = 0;                    /* 文字间距，单位 = 像素 */
    tFont.FrontColor = CL_BLACK;        /* 黑字 */

    LCD_DispStr(5, 240 - 40, "长按S开始测量电池放电容量", &tFont);
    LCD_DispStr(5, 240 - 20, "长按C停止测量", &tFont);
}
        
/*
*********************************************************************************************************
*   函 数 名: DispCurrentVolt
*   功能说明: 显示电压电流功率
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void DispCurrentVolt(void)
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

    sprintf(buf, "电压: %8.3fV", g_tVar.HighSideVolt);
    LCD_DispStrEx(10, 50 + 32 * 0, buf, &tFont, 220, ALIGN_CENTER);

    sprintf(buf, "电流: %8.1fmA", g_tVar.HighSideCurr);
    LCD_DispStrEx(10, 50 + 32 * 1, buf, &tFont, 220, ALIGN_CENTER);

    sprintf(buf, "功率: %8.3fW", g_tVar.HighSideVolt * g_tVar.HighSideCurr / 1000);
    LCD_DispStrEx(10, 50 + 32 * 2, buf, &tFont, 220, ALIGN_CENTER); 

    sprintf(buf, "容量: %8.3fmAh", g_tVar.BatteryCapacity);
    LCD_DispStrEx(10, 50 + 32 * 3, buf, &tFont, 220, ALIGN_CENTER);     
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
