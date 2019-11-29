/*
*********************************************************************************************************
*
*    模块名称 : 脱机编程器
*    文件名称 : status_programmer.c
*    版    本 : V1.0
*    说    明 : 脱机编程器
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

/*
*********************************************************************************************************
*    函 数 名: status_Programmmer
*    功能说明: 脱机编程器
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_Programmer(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;

    DispHeader("烧录器");

    fRefresh = 1;
    while (g_MainStatus == MS_PROGRAMMER)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;
        }

        bsp_Idle();
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_DOWN_S: /* S键按下 */
                break;

            case KEY_UP_S: /* S键释放 */
                g_MainStatus = NextStatus(g_MainStatus);
                break;

            case KEY_LONG_S: /* S键长按 */
                break;

            case KEY_DOWN_C: /* C键按下 */
                break;

            case KEY_UP_C: /* C键释放 */
                g_MainStatus = LastStatus(g_MainStatus);
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
