/*
*********************************************************************************************************
*
*	模块名称 : AD7606数据采集模块测试程序
*	文件名称 : ad7606_test.c
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
#include "main.h"

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack;	/* 静态的文字 */
	FONT_T FontBlue;	/* 变化的文字字体 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;

	LABEL_T LabelN[8 + 2];
	LABEL_T LabelV[8 + 2];

	LABEL_T LabelS1;

	BUTTON_T BtnRet;
}FormAD_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"AD7606数据采集模块测试程序"

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	60
#define	BTN_RET_X	((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define	BTN_RET_Y	((BOX1_Y  + BOX1_H) - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"返回"

#define LabelN1_X  		(BOX1_X + 6)
#define LabelN1_Y		(BOX1_Y + 20)
#define LabelN1_TEXT	"通道1 : "

	#define LabelV1_X  		(LabelN1_X + 64)
	#define LabelV1_Y		LabelN1_Y
	#define LabelV1_TEXT	"-"

#define LabelS1_X  		(BOX1_X + 6)
#define LabelS1_Y		(BOX1_Y + BOX1_H - 20)
#define LabelS1_TEXT	"K1键切换量程, 摇杆上下键选择过采样倍率"

static void InitFormAD(void);
static void DispADInitFace(void);
static void DispADStatus(void);

FormAD_T *FormAD;

/*
*********************************************************************************************************
*	函 数 名: TestAD7606
*	功能说明: 测试AD7606模块。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestAD7606(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	int16_t tpX, tpY;
	FormAD_T form;
	uint8_t fRefresh;

	FormAD = &form;

	InitFormAD();

	DispADInitFace();

	/* 进入主程序循环体 */
	bsp_StartAutoTimer(0, 500);		/* 每0.5秒定时显示状态 */

	bsp_InitAD7606();				/* 配置AD7606相关GPIO */
	AD7606_StartRecord(1000);		/* 进入自动采集模式，采样频率1KHz，数据存放在全局FIFO */

	fRefresh = 1;
	while (g_MainStatus == MS_AD7606)
	{
		bsp_Idle();

		if (bsp_CheckTimer(0) || (fRefresh == 1))
		{
			fRefresh = 0;
			DispADStatus();
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormAD->BtnRet.Focus = 1;
						LCD_DrawButton(&FormAD->BtnRet);
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormAD->BtnRet.Focus = 0;
						LCD_DrawButton(&FormAD->BtnRet);
						g_MainStatus = MS_MAIN_MENU;
					}
					else	/* 按钮失去焦点 */
					{
						FormAD->BtnRet.Focus = 0;
						LCD_DrawButton(&FormAD->BtnRet);
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
				case KEY_DOWN_K1:			/* K1键按下 切换量程 */
					if (g_tAD7606.ucRange == 0)
					{
						AD7606_SetInputRange(1);
					}
					else
					{
						AD7606_SetInputRange(0);
					}
					fRefresh = 1;
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					break;

				case KEY_DOWN_K3:			/* K3键按下 */
					break;

				case JOY_DOWN_U:			/* 摇杆UP键按下 */
					if (g_tAD7606.ucOS < 6)
					{
						g_tAD7606.ucOS++;
					}
					AD7606_SetOS(g_tAD7606.ucOS);
					fRefresh = 1;
					break;

				case JOY_DOWN_D:			/* 摇杆DOWN键按下 */
					if (g_tAD7606.ucOS > 0)
					{
						g_tAD7606.ucOS--;
					}
					AD7606_SetOS(g_tAD7606.ucOS);
					fRefresh = 1;
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

	bsp_StopTimer(0);		/* 停止软件定时器 */
	AD7606_StopRecord();	/* 停止自动采集 */
}

/*
*********************************************************************************************************
*	函 数 名: DispADStatus
*	功能说明: 显示AD7606状态
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispADStatus(void)
{
	char buf[128];
	uint8_t i;
	int32_t volt;
	uint8_t m;

	if (g_tAD7606.ucRange == 1)
	{
		FormAD->LabelV[8].pCaption = "±10V";
		m = 2;
	}
	else
	{
		FormAD->LabelV[8].pCaption = "±5V ";
		m = 1;
	}
	LCD_DrawLabel(&FormAD->LabelV[8]);
	
	for (i = 0; i < 8; i++)
	{
		/*  实测 21508 = 3.300V; 个体有差异 */
		volt = m * g_tAD7606.sNowAdc[i] * 3300 / 21508;
		sprintf(buf, "%6d  %6dmV  ", g_tAD7606.sNowAdc[i], volt);
		FormAD->LabelV[i].pCaption = buf;
		LCD_DrawLabel(&FormAD->LabelV[i]);
	}



	sprintf(buf,  "%d", g_tAD7606.ucOS);
	FormAD->LabelV[9].pCaption = buf;
	LCD_DrawLabel(&FormAD->LabelV[9]);
}

/*
*********************************************************************************************************
*	函 数 名: InitFormAD
*	功能说明: 初始化GPS初始界面控件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormAD(void)
{
	/* 分组框标题字体 */
	FormAD->FontBox.FontCode = FC_ST_16;
	FormAD->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormAD->FontBox.FrontColor = CL_BLACK;
	FormAD->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormAD->FontBlack.FontCode = FC_ST_16;
	FormAD->FontBlack.BackColor = CL_MASK;		/* 透明色 */
	FormAD->FontBlack.FrontColor = CL_BLACK;
	FormAD->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormAD->FontBlue.FontCode = FC_ST_16;
	FormAD->FontBlue.BackColor = CL_BTN_FACE;
	FormAD->FontBlue.FrontColor = CL_BLUE;
	FormAD->FontBlue.Space = 0;

	/* 按钮字体 */
	FormAD->FontBtn.FontCode = FC_ST_16;
	FormAD->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormAD->FontBtn.FrontColor = CL_BLACK;
	FormAD->FontBtn.Space = 0;

	/* 分组框 */
	FormAD->Box1.Left = BOX1_X;
	FormAD->Box1.Top = BOX1_Y;
	FormAD->Box1.Height = BOX1_H;
	FormAD->Box1.Width = BOX1_W;
	FormAD->Box1.pCaption = BOX1_TEXT;
	FormAD->Box1.Font = &FormAD->FontBox;

	/* 标签 */
	{
		uint8_t i;
		const char *strName[10] = {"通道1","通道2","通道3","通道4","通道5","通道6","通道7","通道8", "量程", "过采样"};

		for (i = 0; i < 10; i++)
		{
			FormAD->LabelN[i].Left = LabelN1_X;
			FormAD->LabelN[i].Top = LabelN1_Y + i * 20;
			FormAD->LabelN[i].MaxLen = 0;
			FormAD->LabelN[i].pCaption = (char *)strName[i];
			FormAD->LabelN[i].Font = &FormAD->FontBlack;		/* 黑色 */

			FormAD->LabelV[i].Left = LabelN1_X + 60;
			FormAD->LabelV[i].Top = LabelN1_Y + i * 20;		/* 蓝色 */
			FormAD->LabelV[i].MaxLen = 0;
			FormAD->LabelV[i].pCaption = "-";
			FormAD->LabelV[i].Font = &FormAD->FontBlue;
		}

		FormAD->LabelS1.Left = LabelS1_X;
		FormAD->LabelS1.Top = LabelS1_Y;
		FormAD->LabelS1.MaxLen = 0;
		FormAD->LabelS1.pCaption = LabelS1_TEXT;
		FormAD->LabelS1.Font = &FormAD->FontBlack;		/* 黑色 */
	}

	/* 按钮 */
	FormAD->BtnRet.Left = BTN_RET_X;
	FormAD->BtnRet.Top = BTN_RET_Y;
	FormAD->BtnRet.Height = BTN_RET_H;
	FormAD->BtnRet.Width = BTN_RET_W;
	FormAD->BtnRet.pCaption = BTN_RET_TEXT;
	FormAD->BtnRet.Font = &FormAD->FontBtn;
	FormAD->BtnRet.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispADInitFace
*	功能说明: 显示初始界面
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispADInitFace(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormAD->Box1);

	/* 标签 */
	{
		uint8_t i;

		for (i = 0; i < 10; i++)
		{
			LCD_DrawLabel(&FormAD->LabelN[i]);
		}

		LCD_DrawLabel(&FormAD->LabelS1);
	}

	/* 按钮 */
	LCD_DrawButton(&FormAD->BtnRet);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
