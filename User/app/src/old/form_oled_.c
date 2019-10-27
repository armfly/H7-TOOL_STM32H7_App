/*
*********************************************************************************************************
*
*	模块名称 : 测试OLED显示器
*	文件名称 : oled_test.c
*	版    本 : V1.1
*	说    明 : 测试OLED显示器模块
*	修改记录 :
*		版本号  日期       作者     说明
*		v1.0    2013-02-01 armfly  首发
*		v1.1    2015-10-14 armfly  增加24点阵和32点阵汉字和ASCII显示功能
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_oled.h"

#define DEMO_PAGE_COUNT 7 /* OLED演示页面的个数 */

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack; /* 静态的文字 */
	FONT_T FontBlue;	/* 变化的文字字体 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;

	LABEL_T Label1;
	LABEL_T Label2;
	LABEL_T Label3;
	LABEL_T Label4;
	LABEL_T Label5;
	LABEL_T Label6;
	LABEL_T Label7;
	LABEL_T Label8;

	BUTTON_T BtnRet;
} FormOLED_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H (g_LcdHeight - BOX1_Y - 10)
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_TEXT "OLED显示模块测试程序"

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 60
#define BTN_RET_X ((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define BTN_RET_Y ((BOX1_Y + BOX1_H) - BTN_RET_H - 4)
#define BTN_RET_TEXT "返回"

#define LABEL1_X (BOX1_X + 6)
#define LABEL1_Y (BOX1_Y + 20)
#define LABEL1_TEXT "摇杆左、右键: "

#define LABEL2_X (LABEL1_X + 135)
#define LABEL2_Y LABEL1_Y
#define LABEL2_TEXT "切换OLED显示界面"

#define LABEL3_X (LABEL1_X)
#define LABEL3_Y (LABEL1_Y + 20)
#define LABEL3_TEXT "摇杆上、下键: "

#define LABEL4_X (LABEL3_X + 135)
#define LABEL4_Y (LABEL3_Y)
#define LABEL4_TEXT "调节OLED对比度"

#define LABEL5_X (LABEL1_X)
#define LABEL5_Y (LABEL1_Y + 20 * 2)
#define LABEL5_TEXT "摇杆OK键    : "

#define LABEL6_X (LABEL5_X + 135)
#define LABEL6_Y LABEL5_Y
#define LABEL6_TEXT "切换显示方向"

#define LABEL7_X (LABEL1_X)
#define LABEL7_Y (LABEL1_Y + 20 * 3)
#define LABEL7_TEXT "当前状态    :"

#define LABEL8_X (LABEL7_X + 135)
#define LABEL8_Y LABEL7_Y
#define LABEL8_TEXT "80"

static void InitFormOLED(void);
static void DispFormOLED(void);

FormOLED_T *FormOLED;

/*
*********************************************************************************************************
*	函 数 名: TestOLED
*	功能说明: 测试OLED显示模块
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestOLED(void)
{
	uint8_t ucKeyCode; /* 按键代码 */
	uint8_t ucTouch;	 /* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormOLED_T form;
	uint8_t fRefreshTFT;
	FONT_T tFont12, tFont16, tFont24, tFont32;
	uint8_t fRefreshOled;
	uint8_t ucItem;
	uint8_t ucContrast = 0x80; /* 对比度 */
	uint8_t ucDir = 0;				 /* 显示方向, 0 表示正常方向，1表示倒180度 */

	FormOLED = &form;
	InitFormOLED();
	DispFormOLED();

	OLED_InitHard();	 /* 初始化OLED硬件 */
	OLED_ClrScr(0x00); /* 清屏，0x00表示黑底； 0xFF 表示白底 */

	/* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16; /* 字体代码 16点阵 */
		tFont16.FrontColor = 1;			 /* 字体颜色 0 或 1 */
		tFont16.BackColor = 0;			 /* 文字背景颜色 0 或 1 */
		tFont16.Space = 0;					 /* 文字间距，单位 = 像素 */

		tFont12.FontCode = FC_ST_12; /* 字体代码 12点阵 */
		tFont12.FrontColor = 1;			 /* 字体颜色 0 或 1 */
		tFont12.BackColor = 0;			 /* 文字背景颜色 0 或 1 */
		tFont12.Space = 1;					 /* 文字间距，单位 = 像素 */

		tFont24.FontCode = FC_ST_24; /* 字体代码 24点阵 */
		tFont24.FrontColor = 1;			 /* 字体颜色 0 或 1 */
		tFont24.BackColor = 0;			 /* 文字背景颜色 0 或 1 */
		tFont24.Space = 1;					 /* 文字间距，单位 = 像素 */

		tFont32.FontCode = FC_ST_32; /* 字体代码 32点阵 */
		tFont32.FrontColor = 1;			 /* 字体颜色 0 或 1 */
		tFont32.BackColor = 0;			 /* 文字背景颜色 0 或 1 */
		tFont32.Space = 1;					 /* 文字间距，单位 = 像素 */
	}
	ucItem = 0;
	fRefreshOled = 1;
	fRefreshTFT = 1;
	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		if (fRefreshTFT)
		{
			char buf[32];

			fRefreshTFT = 0;

			sprintf(buf, "第%d页  对比度 = %3d", ucItem + 1, ucContrast);

			FormOLED->Label8.pCaption = buf;
			LCD_DrawLabel(&FormOLED->Label8);
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
			case TOUCH_DOWN: /* 触笔按下事件 */
				if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
				{
					FormOLED->BtnRet.Focus = 1;
					LCD_DrawButton(&FormOLED->BtnRet);
				}
				break;

			case TOUCH_RELEASE: /* 触笔释放事件 */
				if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
				{
					FormOLED->BtnRet.Focus = 0;
					LCD_DrawButton(&FormOLED->BtnRet);
					fQuit = 1; /* 返回 */
				}
				else /* 按钮失去焦点 */
				{
					FormOLED->BtnRet.Focus = 0;
					LCD_DrawButton(&FormOLED->BtnRet);
				}
				break;
			}
		}

		if (fRefreshOled == 1)
		{
			fRefreshOled = 0;

			switch (ucItem)
			{
			case 0:
				OLED_ClrScr(0); /* 清屏，黑底 */
				OLED_DrawRect(0, 0, 64, 128, 1);
				OLED_DispStr(8, 3, "安富莱OLED例程", &tFont16); /* 在(8,3)坐标处显示一串汉字 */
				OLED_DispStr(10, 22, "请操作摇杆!", &tFont16);
				OLED_DispStr(5, 22 + 20, "www.ARMfly.com", &tFont16);
				break;

			case 1:
				//OLED_StartDraw();	  调用改函数，只刷新缓冲区，不送显示
				OLED_ClrScr(0);
				OLED_DispStr(0, 0, "故人西辞黄鹤楼，", &tFont16);
				OLED_DispStr(0, 16, "烟花三月下扬州。", &tFont16);
				OLED_DispStr(0, 32, "孤帆远影碧空尽，", &tFont16);
				OLED_DispStr(0, 48, "唯见长江天际流。", &tFont16);
				//OLED_EndDraw();	  调用改函数，将缓冲区中数据送显示
				break;

			case 2:
				OLED_ClrScr(0);
				OLED_DispStr(5, 0, "《送孟浩然之广陵》", &tFont12);
				OLED_DispStr(0, 13, "故人西辞黄鹤楼，", &tFont12);
				OLED_DispStr(0, 26, "烟花三月下扬州。", &tFont12);
				OLED_DispStr(0, 39, "孤帆远影碧空尽，", &tFont12);
				OLED_DispStr(0, 52, "唯见长江天际流。", &tFont12);

				OLED_DispStr(110, 14, "安", &tFont16);
				OLED_DispStr(110, 30, "富", &tFont16);
				OLED_DispStr(110, 46, "莱", &tFont16);
				OLED_DrawRect(109, 13, 50, 17, 1);
				break;

			case 3:
				OLED_ClrScr(0);
				OLED_DispStr(5, 0, "安富莱123", &tFont24);
				OLED_DispStr(0, 26, "开发板8", &tFont32);
				break;

			case 4:
				OLED_ClrScr(0);
				OLED_DrawRect(0, 0, 10, 10, 1);		/* 在(0,0)坐标处绘制一个高10宽10的矩形 */
				OLED_DrawRect(10, 10, 20, 30, 1); /* 在(10,10)坐标处绘制一个高20宽30的矩形 */
				OLED_DrawCircle(64, 32, 30, 1);		/* 在(64,32)绘制半径30的圆 */
				OLED_DrawLine(127, 0, 0, 63, 1);	/* 在(127,0) 和 (0,63) 之间绘制一条直线 */
				break;

			case 5:
				OLED_ClrScr(0x00); /* 清屏，黑底 */
				break;

			case 6:
				OLED_ClrScr(0xFF); /* 清屏，白底 */
				{
					//char buf[32];

					//sprintf(buf, "%d", ucContrast);
					//OLED_DispStr(10,10,buf,&tFont16);
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
			case KEY_DOWN_K1: /* K1键 */
				break;

			case KEY_DOWN_K2: /* K2键按下 */
				break;

			case KEY_DOWN_K3: /* K3键按下 */
				break;

			case JOY_DOWN_U: /* 摇杆上键按下 */
				if (ucContrast < 255)
				{
					ucContrast++;
				}
				OLED_SetContrast(ucContrast);
				fRefreshOled = 1;
				fRefreshTFT = 1;
				break;

			case JOY_DOWN_D: /* 摇杆下键按下 */
				if (ucContrast > 0)
				{
					ucContrast--;
				}
				OLED_SetContrast(ucContrast);
				fRefreshOled = 1;
				fRefreshTFT = 1;
				break;

			case JOY_DOWN_L: /* 摇杆LEFT键按下 */
				if (ucItem > 0)
				{
					ucItem--;
				}
				else
				{
					ucItem = DEMO_PAGE_COUNT - 1;
				}
				fRefreshOled = 1;
				fRefreshTFT = 1;
				break;

			case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
				if (ucItem < DEMO_PAGE_COUNT - 1)
				{
					ucItem++;
				}
				else
				{
					ucItem = 0;
				}
				fRefreshOled = 1;
				fRefreshTFT = 1;
				break;

			case JOY_DOWN_OK: /* 摇杆OK键 */
				if (ucDir == 0)
				{
					ucDir = 1;
					OLED_SetDir(1); /* 设置显示方向 */
				}
				else
				{
					ucDir = 0;
					OLED_SetDir(0); /* 设置显示方向 */
				}
				fRefreshOled = 1;
				break;

			default:
				break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: InitFormOLED
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormOLED(void)
{
	/* 分组框标题字体 */
	FormOLED->FontBox.FontCode = FC_ST_16;
	FormOLED->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
	FormOLED->FontBox.FrontColor = CL_BLACK;
	FormOLED->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormOLED->FontBlack.FontCode = FC_ST_16;
	FormOLED->FontBlack.BackColor = CL_MASK; /* 透明色 */
	FormOLED->FontBlack.FrontColor = CL_BLACK;
	FormOLED->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormOLED->FontBlue.FontCode = FC_ST_16;
	FormOLED->FontBlue.BackColor = CL_BTN_FACE;
	FormOLED->FontBlue.FrontColor = CL_BLUE;
	FormOLED->FontBlue.Space = 0;

	/* 按钮字体 */
	FormOLED->FontBtn.FontCode = FC_ST_16;
	FormOLED->FontBtn.BackColor = CL_MASK; /* 透明背景 */
	FormOLED->FontBtn.FrontColor = CL_BLACK;
	FormOLED->FontBtn.Space = 0;

	/* 分组框 */
	FormOLED->Box1.Left = BOX1_X;
	FormOLED->Box1.Top = BOX1_Y;
	FormOLED->Box1.Height = BOX1_H;
	FormOLED->Box1.Width = BOX1_W;
	FormOLED->Box1.pCaption = BOX1_TEXT;
	FormOLED->Box1.Font = &FormOLED->FontBox;

	/* 静态标签 */
	FormOLED->Label1.Left = LABEL1_X;
	FormOLED->Label1.Top = LABEL1_Y;
	FormOLED->Label1.MaxLen = 0;
	FormOLED->Label1.pCaption = LABEL1_TEXT;
	FormOLED->Label1.Font = &FormOLED->FontBlack;

	FormOLED->Label3.Left = LABEL3_X;
	FormOLED->Label3.Top = LABEL3_Y;
	FormOLED->Label3.MaxLen = 0;
	FormOLED->Label3.pCaption = LABEL3_TEXT;
	FormOLED->Label3.Font = &FormOLED->FontBlack;

	FormOLED->Label5.Left = LABEL5_X;
	FormOLED->Label5.Top = LABEL5_Y;
	FormOLED->Label5.MaxLen = 0;
	FormOLED->Label5.pCaption = LABEL5_TEXT;
	FormOLED->Label5.Font = &FormOLED->FontBlack;

	FormOLED->Label7.Left = LABEL7_X;
	FormOLED->Label7.Top = LABEL7_Y;
	FormOLED->Label7.MaxLen = 0;
	FormOLED->Label7.pCaption = LABEL7_TEXT;
	FormOLED->Label7.Font = &FormOLED->FontBlack;

	/* 动态标签 */
	FormOLED->Label2.Left = LABEL2_X;
	FormOLED->Label2.Top = LABEL2_Y;
	FormOLED->Label2.MaxLen = 0;
	FormOLED->Label2.pCaption = LABEL2_TEXT;
	FormOLED->Label2.Font = &FormOLED->FontBlue;

	FormOLED->Label4.Left = LABEL4_X;
	FormOLED->Label4.Top = LABEL4_Y;
	FormOLED->Label4.MaxLen = 0;
	FormOLED->Label4.pCaption = LABEL4_TEXT;
	FormOLED->Label4.Font = &FormOLED->FontBlue;

	FormOLED->Label6.Left = LABEL6_X;
	FormOLED->Label6.Top = LABEL6_Y;
	FormOLED->Label6.MaxLen = 0;
	FormOLED->Label6.pCaption = LABEL6_TEXT;
	FormOLED->Label6.Font = &FormOLED->FontBlue;

	FormOLED->Label8.Left = LABEL8_X;
	FormOLED->Label8.Top = LABEL8_Y;
	FormOLED->Label8.MaxLen = 0;
	FormOLED->Label8.pCaption = LABEL8_TEXT;
	FormOLED->Label8.Font = &FormOLED->FontBlue;

	/* 按钮 */
	FormOLED->BtnRet.Left = BTN_RET_X;
	FormOLED->BtnRet.Top = BTN_RET_Y;
	FormOLED->BtnRet.Height = BTN_RET_H;
	FormOLED->BtnRet.Width = BTN_RET_W;
	FormOLED->BtnRet.pCaption = BTN_RET_TEXT;
	FormOLED->BtnRet.Font = &FormOLED->FontBtn;
	FormOLED->BtnRet.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispRSVInitFace
*	功能说明: 显示所有的静态控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormOLED(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormOLED->Box1);

	/* 静态标签 */
	LCD_DrawLabel(&FormOLED->Label1);
	LCD_DrawLabel(&FormOLED->Label3);
	LCD_DrawLabel(&FormOLED->Label5);
	LCD_DrawLabel(&FormOLED->Label7);

	/* 动态标签 */
	LCD_DrawLabel(&FormOLED->Label2);
	LCD_DrawLabel(&FormOLED->Label4);
	LCD_DrawLabel(&FormOLED->Label6);
	LCD_DrawLabel(&FormOLED->Label8);

	/* 按钮 */
	LCD_DrawButton(&FormOLED->BtnRet);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
