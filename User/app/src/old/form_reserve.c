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
#include "form_reserve.h"

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
	LABEL_T Label5; LABEL_T Label6;
	LABEL_T Label7; LABEL_T Label8;

	BUTTON_T BtnRet;
}FormRSV_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"程序开发中..."

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	60
#define	BTN_RET_X	((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define	BTN_RET_Y	((BOX1_Y  + BOX1_H) - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"返回"

#define LABEL1_X  	(BOX1_X + 6)
#define LABEL1_Y	(BOX1_Y + 20)
#define LABEL1_TEXT	"xxxx : "

	#define LABEL2_X  	(LABEL1_X + 64)
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"0000.0000"

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"xxxx : "

	#define LABEL4_X  	(LABEL3_X + 64)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	"00000.0000"

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 2)
#define LABEL5_TEXT	"xxxx : "

	#define LABEL6_X  	(LABEL5_X + 64)
	#define LABEL6_Y	LABEL5_Y
	#define LABEL6_TEXT	"0.0"

#define LABEL7_X  	(LABEL1_X)
#define LABEL7_Y	(LABEL1_Y + 20 * 3)
#define LABEL7_TEXT	"xxxx : "

	#define LABEL8_X  	(LABEL7_X + 64)
	#define LABEL8_Y	LABEL7_Y
	#define LABEL8_TEXT	"0.0"

static void InitFormRSV(void);
static void DispFormRSV(void);

FormRSV_T *FormRSV;

/*
*********************************************************************************************************
*	函 数 名: ReserveFunc
*	功能说明: 保留功能。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void ReserveFunc(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormRSV_T form;

	FormRSV= &form;

	InitFormRSV();

	DispFormRSV();

	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormRSV->BtnRet.Focus = 1;
						LCD_DrawButton(&FormRSV->BtnRet);
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormRSV->BtnRet.Focus = 0;
						LCD_DrawButton(&FormRSV->BtnRet);
						fQuit = 1;	/* 返回 */
					}
					else	/* 按钮失去焦点 */
					{
						FormRSV->BtnRet.Focus = 0;
						LCD_DrawButton(&FormRSV->BtnRet);
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
*	函 数 名: InitFormRSV
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormRSV(void)
{
	/* 分组框标题字体 */
	FormRSV->FontBox.FontCode = FC_ST_16;
	FormRSV->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormRSV->FontBox.FrontColor = CL_BLACK;
	FormRSV->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormRSV->FontBlack.FontCode = FC_ST_16;
	FormRSV->FontBlack.BackColor = CL_MASK;		/* 透明色 */
	FormRSV->FontBlack.FrontColor = CL_BLACK;
	FormRSV->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormRSV->FontBlue.FontCode = FC_ST_16;
	FormRSV->FontBlue.BackColor = CL_BTN_FACE;
	FormRSV->FontBlue.FrontColor = CL_BLUE;
	FormRSV->FontBlue.Space = 0;

	/* 按钮字体 */
	FormRSV->FontBtn.FontCode = FC_ST_16;
	FormRSV->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormRSV->FontBtn.FrontColor = CL_BLACK;
	FormRSV->FontBtn.Space = 0;

	/* 分组框 */
	FormRSV->Box1.Left = BOX1_X;
	FormRSV->Box1.Top = BOX1_Y;
	FormRSV->Box1.Height = BOX1_H;
	FormRSV->Box1.Width = BOX1_W;
	FormRSV->Box1.pCaption = BOX1_TEXT;
	FormRSV->Box1.Font = &FormRSV->FontBox;

	/* 静态标签 */
	FormRSV->Label1.Left = LABEL1_X;
	FormRSV->Label1.Top = LABEL1_Y;
	FormRSV->Label1.MaxLen = 0;
	FormRSV->Label1.pCaption = LABEL1_TEXT;
	FormRSV->Label1.Font = &FormRSV->FontBlack;

	FormRSV->Label3.Left = LABEL3_X;
	FormRSV->Label3.Top = LABEL3_Y;
	FormRSV->Label3.MaxLen = 0;
	FormRSV->Label3.pCaption = LABEL3_TEXT;
	FormRSV->Label3.Font = &FormRSV->FontBlack;

	FormRSV->Label5.Left = LABEL5_X;
	FormRSV->Label5.Top = LABEL5_Y;
	FormRSV->Label5.MaxLen = 0;
	FormRSV->Label5.pCaption = LABEL5_TEXT;
	FormRSV->Label5.Font = &FormRSV->FontBlack;

	FormRSV->Label7.Left = LABEL7_X;
	FormRSV->Label7.Top = LABEL7_Y;
	FormRSV->Label7.MaxLen = 0;
	FormRSV->Label7.pCaption = LABEL7_TEXT;
	FormRSV->Label7.Font = &FormRSV->FontBlack;

	/* 动态标签 */
	FormRSV->Label2.Left = LABEL2_X;
	FormRSV->Label2.Top = LABEL2_Y;
	FormRSV->Label2.MaxLen = 0;
	FormRSV->Label2.pCaption = LABEL2_TEXT;
	FormRSV->Label2.Font = &FormRSV->FontBlue;

	FormRSV->Label4.Left = LABEL4_X;
	FormRSV->Label4.Top = LABEL4_Y;
	FormRSV->Label4.MaxLen = 0;
	FormRSV->Label4.pCaption = LABEL4_TEXT;
	FormRSV->Label4.Font = &FormRSV->FontBlue;

	FormRSV->Label6.Left = LABEL6_X;
	FormRSV->Label6.Top = LABEL6_Y;
	FormRSV->Label6.MaxLen = 0;
	FormRSV->Label6.pCaption = LABEL6_TEXT;
	FormRSV->Label6.Font = &FormRSV->FontBlue;

	FormRSV->Label8.Left = LABEL8_X;
	FormRSV->Label8.Top = LABEL8_Y;
	FormRSV->Label8.MaxLen = 0;
	FormRSV->Label8.pCaption = LABEL8_TEXT;
	FormRSV->Label8.Font = &FormRSV->FontBlue;

	/* 按钮 */
	FormRSV->BtnRet.Left = BTN_RET_X;
	FormRSV->BtnRet.Top = BTN_RET_Y;
	FormRSV->BtnRet.Height = BTN_RET_H;
	FormRSV->BtnRet.Width = BTN_RET_W;
	FormRSV->BtnRet.pCaption = BTN_RET_TEXT;
	FormRSV->BtnRet.Font = &FormRSV->FontBtn;
	FormRSV->BtnRet.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispFormRSV
*	功能说明: 显示所有的控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormRSV(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormRSV->Box1);

	/* 静态标签 */
	LCD_DrawLabel(&FormRSV->Label1);
	LCD_DrawLabel(&FormRSV->Label3);
	LCD_DrawLabel(&FormRSV->Label5);
	LCD_DrawLabel(&FormRSV->Label7);

	/* 动态标签 */
	LCD_DrawLabel(&FormRSV->Label2);
	LCD_DrawLabel(&FormRSV->Label4);
	LCD_DrawLabel(&FormRSV->Label6);
	LCD_DrawLabel(&FormRSV->Label8);

	/* 按钮 */
	LCD_DrawButton(&FormRSV->BtnRet);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
