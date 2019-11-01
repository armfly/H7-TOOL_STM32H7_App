/*
*********************************************************************************************************
*
*    模块名称 : 电压表
*    文件名称 : status_voltage_meter.c
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

static void DispCH1CH2(void);
static void AutoVoltRange(void);

/*
*********************************************************************************************************
*    函 数 名: status_VoltageMeter
*    功能说明: 电压表状态.
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_VoltageMeter(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;

    DispHeader("电压表");

    DSO_StartMode2(); /* 示波器启动模式2-低速多通道扫描 */

    fRefresh = 1;

    bsp_StartAutoTimer(0, 300);
    while (g_MainStatus == MS_VOLTAGE_METER)
    {
        bsp_Idle();

        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

            DispCH1CH2();
            AutoVoltRange();
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
            case KEY_DOWN_S: /* S键按下 */
                break;

            case KEY_UP_S: /* S键释放 */
                g_MainStatus = NextStatus(MS_VOLTAGE_METER);
                break;

            case KEY_LONG_S: /* S键长按 */
                break;

            case KEY_DOWN_C: /* C键按下 */
                break;

            case KEY_UP_C: /* C键释放 */
                g_MainStatus = LastStatus(MS_VOLTAGE_METER);
                break;

            case KEY_LONG_C: /* C键长按 */
                break;

            default:
                break;
            }
        }
    }
    bsp_StopTimer(0);
}

/*
*********************************************************************************************************
*    函 数 名: AutoVoltRange
*    功能说明: 自动调节量程
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void AutoVoltRange(void)
{
    float range[] = {13.8, 6.4, 3.2, 1.6, 0.8, 0.4, 0.2, 0.1};
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if (g_tVar.CH1Volt >= -range[7 - i] && g_tVar.CH1Volt <= range[7 - i])
        {
            WriteRegValue_06H(0x0202, 7 - i); /* CH1通道量程0-7 */
            break;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if (g_tVar.CH2Volt >= -range[7 - i] && g_tVar.CH2Volt <= range[7 - i])
        {
            WriteRegValue_06H(0x0203, 7 - i); /* CH2通道量程0-7 */
            break;
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: DispCH1CH2
*    功能说明: 显示CH1 CH2电压
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispCH1CH2(void)
{
    FONT_T tFont;
    char buf[64];

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_24;                /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE;            /* 字体颜色 */
        tFont.BackColor = HEAD_BAR_COLOR; /* 文字背景颜色 */
        tFont.Space = 0;                                    /* 文字间距，单位 = 像素 */
    }

    sprintf(buf, "CH1: %8.3fV", g_tVar.CH1Volt);
    LCD_DispStrEx(10, 50, buf, &tFont, 220, ALIGN_CENTER);

    sprintf(buf, "CH2: %8.3fV", g_tVar.CH2Volt);
    LCD_DispStrEx(10, 100, buf, &tFont, 220, ALIGN_CENTER);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
