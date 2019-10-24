/*
*********************************************************************************************************
*
*	模块名称 : 电阻表
*	文件名称 : status_resitor_meter.c
*	版    本 : V1.0
*	说    明 : 电阻表
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-12-06 armfly  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

static void DispResistor(void);

/*
*********************************************************************************************************
*	函 数 名: status_ResistorMeter
*	功能说明: 电阻表状态
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_ResistorMeter(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t fRefresh;

	DispHeader("电阻测量");
	
	fRefresh = 1;
	bsp_StartAutoTimer(0, 300);
	while (g_MainStatus == MS_RESISTOR_METER)
	{
		bsp_Idle();

		if (fRefresh)	/* 刷新整个界面 */
		{
			fRefresh = 0;					
			DispResistor();
		}

		/* 短路蜂鸣 20欧 */
		if (g_tVar.NTCRes < (float)0.02)
		{
			BEEP_Start(10,10,0);
		}
		else
		{
			BEEP_Stop();
		}
		
		if (bsp_CheckTimer(0))
		{
			fRefresh = 1;
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
					g_MainStatus = NextStatus(MS_RESISTOR_METER);
					break;

				case  KEY_LONG_S:		/* S键长按 */
					break;				

				case  KEY_DOWN_C:		/* C键按下 */
					break;

				case  KEY_UP_C:			/* C键释放 */
					g_MainStatus = LastStatus(MS_RESISTOR_METER);
					break;

				case  KEY_LONG_C:		/* C键长按 */
					break;	
				
				default:
					break;
			}
		}
	}
	bsp_StopTimer(0);
	BEEP_Stop();
}

/*
*********************************************************************************************************
*	函 数 名: DispResistor
*	功能说明: 显示电阻值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispResistor(void)
{
	FONT_T tFont;
	char buf[64];
	
	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_24;			/* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;		/* 字体颜色 */
		tFont.BackColor = HEAD_BAR_COLOR;	/* 文字背景颜色 */
		tFont.Space = 0;					/* 文字间距，单位 = 像素 */
	}
	
	if (g_tVar.NTCRes < 1.0)
	{
		sprintf(buf, "电阻: %0.1fΩ", g_tVar.NTCRes * 1000);
	}
	else if (g_tVar.NTCRes < 1000)
	{
		sprintf(buf, "电阻: %0.3fKΩ", g_tVar.NTCRes);
	}
	else
	{
		sprintf(buf, "电阻: > 1MΩ" );
	}

	LCD_DispStrEx(10, 50, buf, &tFont, 220, ALIGN_CENTER);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
