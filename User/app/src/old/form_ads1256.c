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
}FormADS1256_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"ADS1256数据采集模块测试程序"

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
#define LabelS1_TEXT	"摇杆上下键选择增益, 左右键选择滤波参数"

static void InitFormADS1256(void);
static void DispADInitFace(void);
static void DispADStatus(void);

FormADS1256_T *FormADS1256;

/*
*********************************************************************************************************
*	函 数 名: TestADS1256
*	功能说明: 测试ADS1256模块。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestADS1256(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	int16_t tpX, tpY;
	FormADS1256_T form;
	uint8_t fRefresh;

	FormADS1256 = &form;

	InitFormADS1256();

	DispADInitFace();

	bsp_InitADS1256();	
	
	ADS1256_CfgADC(ADS1256_GAIN_1, ADS1256_30SPS);	/* 配置ADC参数： 增益1:1, 数据输出速率 30Hz */	
	
	/* 打印芯片ID (通过读ID可以判断硬件接口是否正常) , 正常时状态寄存器的高4bit = 3 */
	#if 0
	{
		uint8_t id;
			
		id = ADS1256_ReadChipID();
		
		if (id != 3)
		{
			printf("Error, ASD1256 Chip ID = 0x%X\r\n", id);
		}
		else
		{
			printf("Ok, ASD1256 Chip ID = 0x%X\r\n", id);
		}
	}
	#endif
	
	/* 进入主程序循环体 */
	bsp_StartAutoTimer(0, 500);

	ADS1256_StartScan();	/* 启动8通道ADC扫描 */
	fRefresh = 1;
	while (g_MainStatus == MS_ADS1256)
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
						FormADS1256->BtnRet.Focus = 1;
						LCD_DrawButton(&FormADS1256->BtnRet);
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormADS1256->BtnRet.Focus = 0;
						LCD_DrawButton(&FormADS1256->BtnRet);
						g_MainStatus = MS_MAIN_MENU;	/* 返回 */
					}
					else	/* 按钮失去焦点 */
					{
						FormADS1256->BtnRet.Focus = 0;
						LCD_DrawButton(&FormADS1256->BtnRet);
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
				case KEY_DOWN_K2:			/* K2键按下 */
				case KEY_DOWN_K3:			/* K3键按下 */
					break;

				case JOY_DOWN_U:			/* 摇杆UP键按下 */
					if (g_tADS1256.Gain < 6)
					{
						g_tADS1256.Gain++;
					}
					ADS1256_CfgADC(g_tADS1256.Gain, g_tADS1256.DataRate);	/* 配置ADC参数： 增益和数据输出速率 */
					ADS1256_StartScan();
					fRefresh = 1;
					break;

				case JOY_DOWN_D:			/* 摇杆DOWN键按下 */
					if (g_tADS1256.Gain > 0)
					{
						g_tADS1256.Gain--;
					}
					ADS1256_CfgADC(g_tADS1256.Gain, g_tADS1256.DataRate);	/* 配置ADC参数： 增益和数据输出速率 */
					ADS1256_StartScan();
					fRefresh = 1;
					break;

				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					if (g_tADS1256.DataRate < ADS1256_2d5SPS)
					{
						g_tADS1256.DataRate++;
					}
					ADS1256_CfgADC(g_tADS1256.Gain, g_tADS1256.DataRate);	/* 配置ADC参数： 增益和数据输出速率 */
					ADS1256_StartScan();
					fRefresh = 1;					
					break;

				case JOY_DOWN_R:		/* 摇杆RIGHT键按下 */
					if (g_tADS1256.DataRate > ADS1256_1000SPS)
					{
						g_tADS1256.DataRate--;
					}
					ADS1256_CfgADC(g_tADS1256.Gain, g_tADS1256.DataRate);	/* 配置ADC参数： 增益和数据输出速率 */
					ADS1256_StartScan();					
					break;

				case JOY_DOWN_OK:		/* 摇杆OK键按下 */
					break;

				default:
					break;
			}
		}
	}

	ADS1256_StopScan();		/* 停止ADS1256的中断响应 */
}

/*
*********************************************************************************************************
*	函 数 名: DispADStatus
*	功能说明: 显示ADC状态
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispADStatus(void)
{
	char buf[128];
	uint8_t i;
	const char *strGain[7] = { "X1 (±5V)", "X2 (±2.5V)", "X4 (±1.25V)", "X8 (±0.625V)", "X16 (±312.5mV)", 
								"X32 (±156.25mV)", "X64 (±78.125mV)"};				

	const char *strRate[ADS1256_DRATE_MAX] = 
		{ 
			"1, 30000SPS",
			"2, 15000SPS",
			"4, 7500SPS",
			"8, 3750SPS",
			"15, 2000SPS",
			"30, 1000SPS",
			"60, 500SPS",
			"300, 100SPS",
			"500, 60SPS",
			"600, 50SPS",
			"1000, 30SPS",
			"1200, 25SPS",
			"2000, 15SPS",
			"3000, 10SPS",
			"6000, 5SPS",
			"12000, 2d5SPS"
		};
								
	/* 显示 ADC结果 */
	for (i = 0; i < 8; i++)
	{
		int32_t iTemp;
		
		/* 计算实际电压值（近似估算的），如需准确，请进行校准 */		
		iTemp = ((int64_t)g_tADS1256.AdcNow[i] * 2500000) / 4194303;
		
		if (iTemp < 0)
		{
			iTemp = -iTemp;
			sprintf(buf, "%6d,(-%d.%03d %03d V) ", g_tADS1256.AdcNow[i] , iTemp /1000000, (iTemp%1000000)/1000, iTemp%1000);
		}
		else
		{
			sprintf(buf, "%6d,( %d.%03d %03d V) ", g_tADS1256.AdcNow[i] , iTemp/1000000, (iTemp%1000000)/1000, iTemp%1000);
		}		
		
		FormADS1256->LabelV[i].pCaption = buf;
		LCD_DrawLabel(&FormADS1256->LabelV[i]);
	}

	/* 显示 增益 */
	FormADS1256->LabelV[8].pCaption = (char *)strGain[g_tADS1256.Gain];
	LCD_DrawLabel(&FormADS1256->LabelV[8]);
	
	/* 显示 数据速率 */
	FormADS1256->LabelV[9].pCaption = (char *)strRate[g_tADS1256.DataRate];
	LCD_DrawLabel(&FormADS1256->LabelV[9]);
}

/*
*********************************************************************************************************
*	函 数 名: InitFormADS1256
*	功能说明: 初始化GPS初始界面控件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormADS1256(void)
{
	/* 分组框标题字体 */
	FormADS1256->FontBox.FontCode = FC_ST_16;
	FormADS1256->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormADS1256->FontBox.FrontColor = CL_BLACK;
	FormADS1256->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormADS1256->FontBlack.FontCode = FC_ST_16;
	FormADS1256->FontBlack.BackColor = CL_MASK;		/* 透明色 */
	FormADS1256->FontBlack.FrontColor = CL_BLACK;
	FormADS1256->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormADS1256->FontBlue.FontCode = FC_ST_16;
	FormADS1256->FontBlue.BackColor = CL_BTN_FACE;
	FormADS1256->FontBlue.FrontColor = CL_BLUE;
	FormADS1256->FontBlue.Space = 0;

	/* 按钮字体 */
	FormADS1256->FontBtn.FontCode = FC_ST_16;
	FormADS1256->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormADS1256->FontBtn.FrontColor = CL_BLACK;
	FormADS1256->FontBtn.Space = 0;

	/* 分组框 */
	FormADS1256->Box1.Left = BOX1_X;
	FormADS1256->Box1.Top = BOX1_Y;
	FormADS1256->Box1.Height = BOX1_H;
	FormADS1256->Box1.Width = BOX1_W;
	FormADS1256->Box1.pCaption = BOX1_TEXT;
	FormADS1256->Box1.Font = &FormADS1256->FontBox;

	/* 标签 */
	{
		uint8_t i;
		const char *strName[10] = {"通道1","通道2","通道3","通道4","通道5","通道6","通道7","通道8", 
				"增益(量程)", "滤波(速率)"};

		for (i = 0; i < 10; i++)
		{
			FormADS1256->LabelN[i].Left = LabelN1_X;
			FormADS1256->LabelN[i].Top = LabelN1_Y + i * 20;
			FormADS1256->LabelN[i].MaxLen = 0;
			FormADS1256->LabelN[i].pCaption = (char *)strName[i];
			FormADS1256->LabelN[i].Font = &FormADS1256->FontBlack;		/* 黑色 */

			FormADS1256->LabelV[i].Left = LabelN1_X + 60;
			FormADS1256->LabelV[i].Top = LabelN1_Y + i * 20;		/* 蓝色 */
			FormADS1256->LabelV[i].MaxLen = 0;
			FormADS1256->LabelV[i].pCaption = "-";
			FormADS1256->LabelV[i].Font = &FormADS1256->FontBlue;
		}
		
		FormADS1256->LabelV[8].Left = LabelN1_X + 120;
		FormADS1256->LabelV[9].Left = LabelN1_X + 120;		

		FormADS1256->LabelS1.Left = LabelS1_X;
		FormADS1256->LabelS1.Top = LabelS1_Y;
		FormADS1256->LabelS1.MaxLen = 0;
		FormADS1256->LabelS1.pCaption = LabelS1_TEXT;
		FormADS1256->LabelS1.Font = &FormADS1256->FontBlack;		/* 黑色 */
	}

	/* 按钮 */
	FormADS1256->BtnRet.Left = BTN_RET_X;
	FormADS1256->BtnRet.Top = BTN_RET_Y;
	FormADS1256->BtnRet.Height = BTN_RET_H;
	FormADS1256->BtnRet.Width = BTN_RET_W;
	FormADS1256->BtnRet.pCaption = BTN_RET_TEXT;
	FormADS1256->BtnRet.Font = &FormADS1256->FontBtn;
	FormADS1256->BtnRet.Focus = 0;
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
	LCD_DrawGroupBox(&FormADS1256->Box1);

	/* 标签 */
	{
		uint8_t i;

		for (i = 0; i < 10; i++)
		{
			LCD_DrawLabel(&FormADS1256->LabelN[i]);
		}

		LCD_DrawLabel(&FormADS1256->LabelS1);
	}

	/* 按钮 */
	LCD_DrawButton(&FormADS1256->BtnRet);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
