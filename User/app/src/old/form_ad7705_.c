/*
*********************************************************************************************************
*
*	模块名称 : 等待开发的程序界面
*	文件名称 : reserve.c
*	版    本 : V1.0
*	说    明 : 测试MPU-6050, HCM5833L, BMP085, BH1750
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2013-02-01 armfly  首发
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_ad7705.h"

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack;	/* 静态的文字 */
	FONT_T FontBlue;	/* 变化的文字字体 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;

	LABEL_T Label1;	LABEL_T Label2;
	LABEL_T Label3; LABEL_T Label4;
	LABEL_T Label5; 
	
	BUTTON_T BtnRet;
}FormAD7705_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"AD7705模块测试程序..."

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	60
#define	BTN_RET_X	((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define	BTN_RET_Y	((BOX1_Y  + BOX1_H) - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"返回"

#define LABEL1_X  	(BOX1_X + 6)
#define LABEL1_Y	(BOX1_Y + 20)
#define LABEL1_TEXT	"通道1 : "

	#define LABEL2_X  	(LABEL1_X + 64)
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"0000.0000"

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"通道2 : "

	#define LABEL4_X  	(LABEL3_X + 64)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	"00000.0000"

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 3)
#define LABEL5_TEXT	"----"


static void InitFormAD7705(void);
static void DispFormAD7705(void);

FormAD7705_T *FormAD7705;

/*
*********************************************************************************************************
*	函 数 名: TestAD7705
*	功能说明: 测试AD7705模块。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestAD7705(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormAD7705_T form;
	uint16_t adc1, adc2;

	FormAD7705 = &form;
	InitFormAD7705();
	DispFormAD7705();

	bsp_InitTM7705();			/* 初始化配置TM7705 */

	if (g_TM7705_OK == 0)	
	{
		FormAD7705->Label5.pCaption = "未检测到 AD7705";
		LCD_DrawLabel(&FormAD7705->Label5);
	}
	else	
	{		
		FormAD7705->Label5.pCaption = "已检测到 AD7705";
		LCD_DrawLabel(&FormAD7705->Label5);
		
		TM7705_CalibSelf(1);		/* 自校准。执行时间较长，约180ms */
		adc1 = TM7705_ReadAdc(1);

		TM7705_CalibSelf(2);		/* 自校准。执行时间较长，约180ms */
		adc2 = TM7705_ReadAdc(2);
	}

	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		//TM7705_Scan1();		/* 扫描ADC通道1 */
		TM7705_Scan2();		/* 扫描两个个ADC通道, 无等待的 */

		{
			/* 读取扫描结果 (结果定时读取即可) */
			adc1 = TM7705_GetAdc1();
			adc2 = TM7705_GetAdc2();
			
			/* 打印采集数据 */
			{
				int volt1, volt2;
				char buf[64];

				/* 计算实际电压值（近似估算的），如需准确，请进行校准 */
				volt1 = (adc1 * 5000) / 65535;
				volt2 = (adc2 * 5000) / 65535;

				/* 显示ADC采样结果 */
				sprintf(buf, "%5d (%5dmV)", adc1, volt1);
				FormAD7705->Label2.pCaption = buf;
				LCD_DrawLabel(&FormAD7705->Label2);

				sprintf(buf, "%5d (%5dmV)", adc2, volt2);
				FormAD7705->Label4.pCaption = buf;
				LCD_DrawLabel(&FormAD7705->Label4);
			}
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormAD7705->BtnRet.Focus = 1;
						LCD_DrawButton(&FormAD7705->BtnRet);
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormAD7705->BtnRet.Focus = 0;
						LCD_DrawButton(&FormAD7705->BtnRet);
						fQuit = 1;	/* 返回 */
					}
					else	/* 按钮失去焦点 */
					{
						FormAD7705->BtnRet.Focus = 0;
						LCD_DrawButton(&FormAD7705->BtnRet);
					}
					break;
			}
		}

		/* 处理按键事件 */
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:		/* K1键 */
					break;

				case KEY_DOWN_K2:		/* K2键按下 */
					break;

				case KEY_DOWN_K3:		/* K3键按下 */
					break;

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					break;

				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					break;

				case JOY_DOWN_R:		/* 摇杆RIGHT键按下 */
					break;

				case JOY_DOWN_OK:		/* 摇杆OK键按下 */
					break;

				default:
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: InitFormAD7705
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormAD7705(void)
{
	/* 分组框标题字体 */
	FormAD7705->FontBox.FontCode = FC_ST_16;
	FormAD7705->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormAD7705->FontBox.FrontColor = CL_BLACK;
	FormAD7705->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormAD7705->FontBlack.FontCode = FC_ST_16;
	FormAD7705->FontBlack.BackColor = CL_BTN_FACE;		/* 透明色 */
	FormAD7705->FontBlack.FrontColor = CL_BLACK;
	FormAD7705->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormAD7705->FontBlue.FontCode = FC_ST_16;
	FormAD7705->FontBlue.BackColor = CL_BTN_FACE;
	FormAD7705->FontBlue.FrontColor = CL_BLUE;
	FormAD7705->FontBlue.Space = 0;

	/* 按钮字体 */
	FormAD7705->FontBtn.FontCode = FC_ST_16;
	FormAD7705->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormAD7705->FontBtn.FrontColor = CL_BLACK;
	FormAD7705->FontBtn.Space = 0;

	/* 分组框 */
	FormAD7705->Box1.Left = BOX1_X;
	FormAD7705->Box1.Top = BOX1_Y;
	FormAD7705->Box1.Height = BOX1_H;
	FormAD7705->Box1.Width = BOX1_W;
	FormAD7705->Box1.pCaption = BOX1_TEXT;
	FormAD7705->Box1.Font = &FormAD7705->FontBox;

	/* 静态标签 */
	FormAD7705->Label1.Left = LABEL1_X;
	FormAD7705->Label1.Top = LABEL1_Y;
	FormAD7705->Label1.MaxLen = 0;
	FormAD7705->Label1.pCaption = LABEL1_TEXT;
	FormAD7705->Label1.Font = &FormAD7705->FontBlack;

	FormAD7705->Label3.Left = LABEL3_X;
	FormAD7705->Label3.Top = LABEL3_Y;
	FormAD7705->Label3.MaxLen = 0;
	FormAD7705->Label3.pCaption = LABEL3_TEXT;
	FormAD7705->Label3.Font = &FormAD7705->FontBlack;

	FormAD7705->Label5.Left = LABEL5_X;
	FormAD7705->Label5.Top = LABEL5_Y;
	FormAD7705->Label5.MaxLen = 0;
	FormAD7705->Label5.pCaption = LABEL5_TEXT;
	FormAD7705->Label5.Font = &FormAD7705->FontBlack;


	/* 动态标签 */
	FormAD7705->Label2.Left = LABEL2_X;
	FormAD7705->Label2.Top = LABEL2_Y;
	FormAD7705->Label2.MaxLen = 0;
	FormAD7705->Label2.pCaption = LABEL2_TEXT;
	FormAD7705->Label2.Font = &FormAD7705->FontBlue;

	FormAD7705->Label4.Left = LABEL4_X;
	FormAD7705->Label4.Top = LABEL4_Y;
	FormAD7705->Label4.MaxLen = 0;
	FormAD7705->Label4.pCaption = LABEL4_TEXT;
	FormAD7705->Label4.Font = &FormAD7705->FontBlue;

	/* 按钮 */
	FormAD7705->BtnRet.Left = BTN_RET_X;
	FormAD7705->BtnRet.Top = BTN_RET_Y;
	FormAD7705->BtnRet.Height = BTN_RET_H;
	FormAD7705->BtnRet.Width = BTN_RET_W;
	FormAD7705->BtnRet.pCaption = BTN_RET_TEXT;
	FormAD7705->BtnRet.Font = &FormAD7705->FontBtn;
	FormAD7705->BtnRet.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispFormAD7705
*	功能说明: 显示所有的控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormAD7705(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormAD7705->Box1);

	/* 静态标签 */
	LCD_DrawLabel(&FormAD7705->Label1);
	LCD_DrawLabel(&FormAD7705->Label3);
	LCD_DrawLabel(&FormAD7705->Label5);

	/* 动态标签 */
	LCD_DrawLabel(&FormAD7705->Label2);
	LCD_DrawLabel(&FormAD7705->Label4);

	/* 按钮 */
	LCD_DrawButton(&FormAD7705->BtnRet);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
