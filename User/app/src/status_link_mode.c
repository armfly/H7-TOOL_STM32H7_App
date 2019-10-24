/*
*********************************************************************************************************
*
*	模块名称 : 联机界面
*	文件名称 : status_link_mode.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2019-10-06 armfly  正式发布
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

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
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t fRefresh;
	uint8_t fIgnoreKey = 0;

	DispHeader("联机模式");
	
	usbd_CloseCDC();
	usbd_OpenCDC(8);		/* 启用USB虚拟串口8， 用于和PC软件USB通信 */
	
	fRefresh = 1;
	while (g_MainStatus == MS_LINK_MODE)
	{
		bsp_Idle();

		if (fRefresh)	/* 刷新整个界面 */
		{
			fRefresh = 0;

			{
				FONT_T tFont;		/* 定义字体结构体变量 */
				uint16_t x;
				uint16_t y;
				char buf[64];
				uint8_t line_cap = 20;
				
				tFont.FontCode = FC_ST_16;		/* 字体代码 16点阵 */
				tFont.FrontColor = CL_YELLOW;	/* 字体颜色 */
				tFont.BackColor = FORM_BACK_COLOR;	/* 文字背景颜色 */
				tFont.Space = 0;				/* 文字间距，单位 = 像素 */

				x = 5;
				y = 2 * line_cap;
				sprintf(buf, "以太网MAC:%02X-%02X-%02X-%02X-%02X-%02X",
					g_tVar.MACaddr[0], g_tVar.MACaddr[1], g_tVar.MACaddr[2],
					g_tVar.MACaddr[3], g_tVar.MACaddr[4], g_tVar.MACaddr[5]);
				LCD_DispStr(x, y, buf, &tFont);

				y = y + line_cap;
				sprintf(buf, "IP:%d.%d.%d.%d", g_tParam.LocalIPAddr[0], g_tParam.LocalIPAddr[1],
					g_tParam.LocalIPAddr[2], g_tParam.LocalIPAddr[3]);					
				LCD_DispStr(x, y, buf, &tFont);	
				
				y = y + line_cap;
				sprintf(buf, "TCP端口号: %d", g_tParam.LocalTCPPort);					
				LCD_DispStr(x, y, buf, &tFont);	
				
				y = y + line_cap;
				sprintf(buf, "UDP端口号: %d", g_tParam.LocalTCPPort);					
				LCD_DispStr(x, y, buf, &tFont);						
					
				tFont.FrontColor = CL_BLACK;	/* 黑字 */
				y = 10 * line_cap;
				sprintf(buf, "长按S进入系统设置", &tFont);
				
				LCD_DispStr(x, y, buf, &tFont);	
				y = 11 * line_cap;
				sprintf(buf, "长按C切换方向", &tFont);
				LCD_DispStr(x, y, buf, &tFont);					
			}
		}

		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case  KEY_DOWN_S:		/* S键按下 */
					break;

				case  KEY_UP_S:			/* S键释放 */
					g_MainStatus = NextStatus(MS_LINK_MODE);
					break;

				case  KEY_LONG_S:		/* S键长按 */
					BEEP_KeyTone();	
					g_MainStatus = MS_SYSTEM_SET;
					break;								

				case  KEY_DOWN_C:		/* C键按下 */
					break;

				case  KEY_UP_C:			/* C键释放 */
					if (fIgnoreKey == 1)
					{
						fIgnoreKey = 0;
						break;
					}
					g_MainStatus = LastStatus(MS_LINK_MODE);
					break;

				case  KEY_LONG_C:		/* C键 */
					BEEP_KeyTone();	
					if (++g_tParam.DispDir > 3)
					{
						g_tParam.DispDir = 0;
					}
					LCD_SetDirection(g_tParam.DispDir);
					SaveParam();
					DispHeader("联机模式");
					fRefresh = 1;
					fIgnoreKey = 1;	/* 需要忽略J即将到来的弹起按键 */
					break;	
					
				default:
					break;
			}
		}
	}

	usbd_CloseCDC();	
	usbd_OpenCDC(1);		/* 启用USB虚拟串口1， 用于虚拟串口，RS232 RS485 TTL-UART */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
