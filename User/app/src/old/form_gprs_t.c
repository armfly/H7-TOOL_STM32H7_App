/*
*********************************************************************************************************
*
*	模块名称 : 测试SIM800 GPRS模块
*	文件名称 : gprs_test.c
*	版    本 : V1.0
*	说    明 : 测试GPRS模块 SIM800
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2015-08-01 armfly  首发
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_gprs.h"
#include "num_pad.h"

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack; /* 静态的文字 */
	FONT_T FontBlue;	/* 变化的文字字体 */
	FONT_T FontRed;
	FONT_T FontBtn; /* 按钮的字体 */
	FONT_T FontBox; /* 分组框标题字体 */

	GROUP_T Box1;

	LABEL_T Label1;
	LABEL_T Label2;
	LABEL_T Label3;
	LABEL_T Label4;
	LABEL_T Label5;
	LABEL_T Label6;
	LABEL_T Label7;
	LABEL_T Label8;

	LABEL_T Label9;

	EDIT_T Edit1; /* 电话号码 */

	BUTTON_T Btn1; /* 拨号 */
	BUTTON_T Btn2; /* 挂机 */
	BUTTON_T Btn3; /* 接听来话 */
	BUTTON_T Btn4; /* 拨打10086 */
	BUTTON_T Btn5; /* 拨打10010 */

	BUTTON_T BtnRet; /* 返回 */

	char strHardInfo[32]; /* 模块硬件信息 */
	uint8_t ucNetStatus;	/* 网络状态 */
	uint8_t ucAudioCh;		/* 当前音频通道 0， 1 */
	uint8_t ucEarVolume;	/* 耳机音量 0 - 5 */
	int16_t ucMicGain;		/* MIC音量  -12：最小增益  12：最大增益  13：静音*/

} FormGPRS_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H (g_LcdHeight - BOX1_Y - 10)
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_TEXT "GPRS模块测试程序."

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 60
#define BTN_RET_X ((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define BTN_RET_Y ((BOX1_Y + BOX1_H) - BTN_RET_H - 4)
#define BTN_RET_TEXT "返回"

#define LABEL1_X (BOX1_X + 6)
#define LABEL1_Y (BOX1_Y + 20)
#define LABEL1_TEXT "模块版本 : "

#define LABEL2_X (LABEL1_X + 100)
#define LABEL2_Y LABEL1_Y
#define LABEL2_TEXT "---"

#define LABEL3_X (LABEL1_X)
#define LABEL3_Y (LABEL1_Y + 20)
#define LABEL3_TEXT "网络状态 : "

#define LABEL4_X (LABEL3_X + 100)
#define LABEL4_Y (LABEL3_Y)
#define LABEL4_TEXT "---"

#define LABEL5_X (LABEL1_X)
#define LABEL5_Y (LABEL1_Y + 20 * 2)
#define LABEL5_TEXT "音频通道 : "

#define LABEL6_X (LABEL5_X + 100)
#define LABEL6_Y LABEL5_Y
#define LABEL6_TEXT "1"

#define LABEL7_X (LABEL1_X)
#define LABEL7_Y (LABEL1_Y + 20 * 3)
#define LABEL7_TEXT "耳机音量和MIC增益 : "

#define LABEL8_X (LABEL7_X + 160)
#define LABEL8_Y LABEL7_Y
#define LABEL8_TEXT "---"

#define LABEL9_X (LABEL1_X)
#define LABEL9_Y (LABEL1_Y + 20 * 5)
#define LABEL9_TEXT "电话号码 : "

/* Edit */
#define EDIT1_X (LABEL9_X + 90)
#define EDIT1_Y (LABEL9_Y - 4)
#define EDIT1_H 26
#define EDIT1_W 132

/* 按钮 */
#define BTN1_H 32
#define BTN1_W 60
#define BTN1_X LABEL1_X
#define BTN1_Y (EDIT1_Y + EDIT1_H + 10)
#define BTN1_TEXT "拨号"

#define BTN2_H 32
#define BTN2_W 60
#define BTN2_X (BTN1_X + BTN1_W + 10)
#define BTN2_Y BTN1_Y
#define BTN2_TEXT "挂机"

#define BTN3_H 32
#define BTN3_W 90
#define BTN3_X (BTN1_X + 2 * (BTN1_W + 10))
#define BTN3_Y BTN1_Y
#define BTN3_TEXT "接听来话"

#define BTN4_H 32
#define BTN4_W 124
#define BTN4_X BTN1_X
#define BTN4_Y (BTN1_Y + BTN1_H + 10)
#define BTN4_TEXT "拨打10086"

#define BTN5_H 32
#define BTN5_W 124
#define BTN5_X BTN4_X + BTN4_W + 10
#define BTN5_Y (BTN1_Y + BTN1_H + 10)
#define BTN5_TEXT "拨打10010"

static void InitFormGPRS(void);
static void DispFormGPRS(void);

FormGPRS_T *FormGPRS;

void TestGPRS_SIM800(void);
void TestGPRS_MG323(void);

/*
*********************************************************************************************************
*	函 数 名: TestGPRS
*	功能说明: 测试华为GPRS模块
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestGPRS(void)
{
	TestGPRS_SIM800();
}

/*
*********************************************************************************************************
*	函 数 名: TestGPRS_SIM800
*	功能说明: 测试SIM800 GPRS模块
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestGPRS_SIM800(void)
{
	uint8_t ucKeyCode; /* 按键代码 */
	uint8_t ucTouch;	 /* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	uint8_t ucValue;
	uint8_t fRefresh = 1;
	FormGPRS_T form;

	FormGPRS = &form;

	LCD_ClrScr(CL_BTN_FACE);

	InitFormGPRS();

	DispFormGPRS();

	bsp_InitSIM800();

	printf("正在给GPRS模块上电...\r\n");

	/* 给GSM模块上电 */
	if (SIM800_PowerOn())
	{
		printf("\r\n上电完成\r\n");
	}
	else
	{
		printf("\r\n模块无应答\r\n");
	}

	{
		SIM800_INFO_T tInfo;

		if (SIM800_GetHardInfo(&tInfo))
		{
			FormGPRS->Label2.Font = &FormGPRS->FontBlue;
			sprintf(FormGPRS->strHardInfo, "%s %s %s", tInfo.Manfacture, tInfo.Model, tInfo.Revision);
		}
		else
		{
			FormGPRS->Label2.Font = &FormGPRS->FontRed;
			sprintf(FormGPRS->strHardInfo, "未检测到模块");
		}
		FormGPRS->Label2.pCaption = FormGPRS->strHardInfo;
		LCD_DrawLabel(&FormGPRS->Label2);
	}

	//SIM800_SetAutoReport();		/* 设置事件自动上报 */

	SIM800_SetMicGain(0, FormGPRS->ucMicGain);	/* 设置MIC增益 */
	SIM800_SetEarVolume(FormGPRS->ucEarVolume); /* 设置耳机音量 */

	bsp_DelayMS(100);
	SIM800_SendAT("AT+CMICBIAS=0"); /* 打开MIC偏置电路 */

	bsp_StartAutoTimer(0, 500);
	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		/* 从MG323收到的数据发送到串口1 */
		if (comGetChar(COM_SIM800, &ucValue))
		{
			comSendChar(COM1, ucValue);
			continue;
		}
		/* 将串口1的数据发送到MG323模块 */
		if (comGetChar(COM1, &ucValue))
		{
			comSendChar(COM_SIM800, ucValue);
			continue;
		}

		/* 每隔0.5秒查询一下网络注册状态 */
		if (bsp_CheckTimer(0))
		{
			FormGPRS->ucNetStatus = SIM800_GetNetStatus();

			if ((FormGPRS->ucNetStatus == CREG_LOCAL_OK) || (FormGPRS->ucNetStatus == CREG_REMOTE_OK))
			{
				bsp_StopTimer(0); /* 停止自动刷新 */

				/* 配置WM8978芯片，输入为LINE接口，输出为耳机 和 扬声器 */
				wm8978_CfgAudioPath(LINE_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON);
			}
			else if (FormGPRS->ucNetStatus == CREG_NO_REG)
			{
				bsp_StopTimer(0); /* 停止自动刷新 */
			}
			fRefresh = 1;
		}

		if (fRefresh)
		{
			fRefresh = 0;

			DispFormGPRS(); /* 刷新所有控件 */
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
			case TOUCH_DOWN: /* 触笔按下事件 */
				LCD_ButtonTouchDown(&FormGPRS->BtnRet, tpX, tpY);
				LCD_ButtonTouchDown(&FormGPRS->Btn1, tpX, tpY);
				LCD_ButtonTouchDown(&FormGPRS->Btn2, tpX, tpY);
				LCD_ButtonTouchDown(&FormGPRS->Btn3, tpX, tpY);
				LCD_ButtonTouchDown(&FormGPRS->Btn4, tpX, tpY);
				LCD_ButtonTouchDown(&FormGPRS->Btn5, tpX, tpY);

				/* 编辑框 */
				if (TOUCH_InRect(tpX, tpY, EDIT1_X, EDIT1_Y, EDIT1_H, EDIT1_W))
				{
					{
						if (InputNumber(NUMPAD_TEL, "设置电话号码", 0, (void *)FormGPRS->Edit1.Text))
						{
							;
						}

						ClearWinNumPad(FORM_BACK_COLOR); /* 清除数字键盘窗口 */
						fRefresh = 1;
					}
				}
				break;

			case TOUCH_RELEASE: /* 触笔释放事件 */
				if (LCD_ButtonTouchRelease(&FormGPRS->BtnRet, tpX, tpY))
				{
					fQuit = 1; /* 返回 */
				}
				else if (LCD_ButtonTouchRelease(&FormGPRS->Btn1, tpX, tpY)) /* 拨号 */
				{
					/* 拨打电话 */
					SIM800_DialTel(FormGPRS->Edit1.Text);
				}
				else if (LCD_ButtonTouchRelease(&FormGPRS->Btn2, tpX, tpY))
				{
					/* 挂机 */
					SIM800_Hangup();
				}
				else if (LCD_ButtonTouchRelease(&FormGPRS->Btn3, tpX, tpY)) /* 接听来话 */
				{
					SIM800_SendAT("ATA");
				}
				else if (LCD_ButtonTouchRelease(&FormGPRS->Btn4, tpX, tpY)) /* 拨打10086 */
				{
					SIM800_DialTel("10086");
				}
				else if (LCD_ButtonTouchRelease(&FormGPRS->Btn5, tpX, tpY)) /* 拨打10010 */
				{
					SIM800_DialTel("10010");
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
			case KEY_DOWN_K1:							/* K1键 */
				SIM800_SendAT("ATD10086;"); /* 拨打10086 */
				break;

			case KEY_DOWN_K2:				/* K2键按下 */
				SIM800_SendAT("ATH"); /* 挂断电话 */
				break;

			case KEY_DOWN_K3: /* K3键按下 */
				fRefresh = 1;
				break;

			case JOY_DOWN_U: /* 摇杆UP键按下  调节耳机音量 */
				if (FormGPRS->ucEarVolume < EAR_VOL_MAX)
				{
					FormGPRS->ucEarVolume++;
					SIM800_SetEarVolume(FormGPRS->ucEarVolume);
				}
				fRefresh = 1;
				break;

			case JOY_DOWN_D: /* 摇杆DOWN键按下  调节耳机音量 */
				if (FormGPRS->ucEarVolume > EAR_VOL_MIN)
				{
					FormGPRS->ucEarVolume--;
					SIM800_SetEarVolume(FormGPRS->ucEarVolume);
				}
				fRefresh = 1;
				break;

			case JOY_DOWN_L: /* 摇杆LEFT键按下  调节MIC增益 */
				if (FormGPRS->ucMicGain > MIC_GAIN_MIN)
				{
					FormGPRS->ucMicGain--;
					SIM800_SetMicGain(0, FormGPRS->ucMicGain);
				}
				fRefresh = 1;
				break;

			case JOY_DOWN_R: /* 摇杆RIGHT键按下  调节MIC增益 */
				if (FormGPRS->ucMicGain < MIC_GAIN_MAX)
				{
					FormGPRS->ucMicGain++;
					SIM800_SetMicGain(0, FormGPRS->ucMicGain);
				}
				fRefresh = 1;
				break;

			case JOY_DOWN_OK: /* 摇杆OK键按下 */
				break;

			default:
				break;
			}
		}
	}

	/* 关闭WM8978芯片 */
	wm8978_PowerDown();

	SIM800_PowerOff(); /* 下电 */
	printf("GPRS模块已下电.\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: InitFormGPRS
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormGPRS(void)
{
	/* 分组框标题字体 */
	FormGPRS->FontBox.FontCode = FC_ST_16;
	FormGPRS->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
	FormGPRS->FontBox.FrontColor = CL_BLACK;
	FormGPRS->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormGPRS->FontBlack.FontCode = FC_ST_16;
	FormGPRS->FontBlack.BackColor = CL_MASK; /* 透明色 */
	FormGPRS->FontBlack.FrontColor = CL_BLACK;
	FormGPRS->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormGPRS->FontBlue.FontCode = FC_ST_16;
	FormGPRS->FontBlue.BackColor = CL_BTN_FACE;
	FormGPRS->FontBlue.FrontColor = CL_BLUE;
	FormGPRS->FontBlue.Space = 0;

	FormGPRS->FontRed.FontCode = FC_ST_16;
	FormGPRS->FontRed.BackColor = CL_BTN_FACE;
	FormGPRS->FontRed.FrontColor = CL_RED;
	FormGPRS->FontRed.Space = 0;

	/* 按钮字体 */
	FormGPRS->FontBtn.FontCode = FC_ST_16;
	FormGPRS->FontBtn.BackColor = CL_MASK; /* 透明背景 */
	FormGPRS->FontBtn.FrontColor = CL_BLACK;
	FormGPRS->FontBtn.Space = 0;

	/* 分组框 */
	FormGPRS->Box1.Left = BOX1_X;
	FormGPRS->Box1.Top = BOX1_Y;
	FormGPRS->Box1.Height = BOX1_H;
	FormGPRS->Box1.Width = BOX1_W;
	FormGPRS->Box1.pCaption = BOX1_TEXT;
	FormGPRS->Box1.Font = &FormGPRS->FontBox;

	/* 静态标签 */
	FormGPRS->Label1.Left = LABEL1_X;
	FormGPRS->Label1.Top = LABEL1_Y;
	FormGPRS->Label1.MaxLen = 0;
	FormGPRS->Label1.pCaption = LABEL1_TEXT;
	FormGPRS->Label1.Font = &FormGPRS->FontBlack;

	FormGPRS->Label3.Left = LABEL3_X;
	FormGPRS->Label3.Top = LABEL3_Y;
	FormGPRS->Label3.MaxLen = 0;
	FormGPRS->Label3.pCaption = LABEL3_TEXT;
	FormGPRS->Label3.Font = &FormGPRS->FontBlack;

	FormGPRS->Label5.Left = LABEL5_X;
	FormGPRS->Label5.Top = LABEL5_Y;
	FormGPRS->Label5.MaxLen = 0;
	FormGPRS->Label5.pCaption = LABEL5_TEXT;
	FormGPRS->Label5.Font = &FormGPRS->FontBlack;

	FormGPRS->Label7.Left = LABEL7_X;
	FormGPRS->Label7.Top = LABEL7_Y;
	FormGPRS->Label7.MaxLen = 0;
	FormGPRS->Label7.pCaption = LABEL7_TEXT;
	FormGPRS->Label7.Font = &FormGPRS->FontBlack;

	FormGPRS->Label9.Left = LABEL9_X;
	FormGPRS->Label9.Top = LABEL9_Y;
	FormGPRS->Label9.MaxLen = 0;
	FormGPRS->Label9.pCaption = LABEL9_TEXT;
	FormGPRS->Label9.Font = &FormGPRS->FontBlack;

	/* 动态标签 */
	FormGPRS->Label2.Left = LABEL2_X;
	FormGPRS->Label2.Top = LABEL2_Y;
	FormGPRS->Label2.MaxLen = 0;
	FormGPRS->Label2.pCaption = LABEL2_TEXT;
	FormGPRS->Label2.Font = &FormGPRS->FontBlue;

	FormGPRS->Label4.Left = LABEL4_X;
	FormGPRS->Label4.Top = LABEL4_Y;
	FormGPRS->Label4.MaxLen = 0;
	FormGPRS->Label4.pCaption = LABEL4_TEXT;
	FormGPRS->Label4.Font = &FormGPRS->FontBlue;

	FormGPRS->Label6.Left = LABEL6_X;
	FormGPRS->Label6.Top = LABEL6_Y;
	FormGPRS->Label6.MaxLen = 0;
	FormGPRS->Label6.pCaption = LABEL6_TEXT;
	FormGPRS->Label6.Font = &FormGPRS->FontBlue;

	FormGPRS->Label8.Left = LABEL8_X;
	FormGPRS->Label8.Top = LABEL8_Y;
	FormGPRS->Label8.MaxLen = 0;
	FormGPRS->Label8.pCaption = LABEL8_TEXT;
	FormGPRS->Label8.Font = &FormGPRS->FontBlue;

	/* 按钮 */
	FormGPRS->BtnRet.Left = BTN_RET_X;
	FormGPRS->BtnRet.Top = BTN_RET_Y;
	FormGPRS->BtnRet.Height = BTN_RET_H;
	FormGPRS->BtnRet.Width = BTN_RET_W;
	FormGPRS->BtnRet.pCaption = BTN_RET_TEXT;
	FormGPRS->BtnRet.Font = &FormGPRS->FontBtn;
	FormGPRS->BtnRet.Focus = 0;

	FormGPRS->Btn1.Left = BTN1_X;
	FormGPRS->Btn1.Top = BTN1_Y;
	FormGPRS->Btn1.Height = BTN1_H;
	FormGPRS->Btn1.Width = BTN1_W;
	FormGPRS->Btn1.pCaption = BTN1_TEXT;
	FormGPRS->Btn1.Font = &FormGPRS->FontBtn;
	FormGPRS->Btn1.Focus = 0;

	FormGPRS->Btn2.Left = BTN2_X;
	FormGPRS->Btn2.Top = BTN2_Y;
	FormGPRS->Btn2.Height = BTN2_H;
	FormGPRS->Btn2.Width = BTN2_W;
	FormGPRS->Btn2.pCaption = BTN2_TEXT;
	FormGPRS->Btn2.Font = &FormGPRS->FontBtn;
	FormGPRS->Btn2.Focus = 0;

	FormGPRS->Btn3.Left = BTN3_X;
	FormGPRS->Btn3.Top = BTN3_Y;
	FormGPRS->Btn3.Height = BTN3_H;
	FormGPRS->Btn3.Width = BTN3_W;
	FormGPRS->Btn3.pCaption = BTN3_TEXT;
	FormGPRS->Btn3.Font = &FormGPRS->FontBtn;
	FormGPRS->Btn3.Focus = 0;

	FormGPRS->Btn4.Left = BTN4_X;
	FormGPRS->Btn4.Top = BTN4_Y;
	FormGPRS->Btn4.Height = BTN4_H;
	FormGPRS->Btn4.Width = BTN4_W;
	FormGPRS->Btn4.pCaption = BTN4_TEXT;
	FormGPRS->Btn4.Font = &FormGPRS->FontBtn;
	FormGPRS->Btn4.Focus = 0;

	FormGPRS->Btn5.Left = BTN5_X;
	FormGPRS->Btn5.Top = BTN5_Y;
	FormGPRS->Btn5.Height = BTN5_H;
	FormGPRS->Btn5.Width = BTN5_W;
	FormGPRS->Btn5.pCaption = BTN5_TEXT;
	FormGPRS->Btn5.Font = &FormGPRS->FontBtn;
	FormGPRS->Btn5.Focus = 0;

	/* 编辑框 */
	FormGPRS->Edit1.Left = EDIT1_X;
	FormGPRS->Edit1.Top = EDIT1_Y;
	FormGPRS->Edit1.Height = EDIT1_H;
	FormGPRS->Edit1.Width = EDIT1_W;
	sprintf(FormGPRS->Edit1.Text, "10086");
	FormGPRS->Edit1.pCaption = FormGPRS->Edit1.Text;
	FormGPRS->Edit1.Font = &FormGPRS->FontBtn;

	FormGPRS->ucMicGain = MIC_GAIN_DEFAULT;	/* 缺省MIC增益 */
	FormGPRS->ucEarVolume = EAR_VOL_DEFAULT; /* 缺省耳机音量 */
}

/*
*********************************************************************************************************
*	函 数 名: DispFormGPRS
*	功能说明: 显示所有的控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormGPRS(void)
{
	//LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormGPRS->Box1);

	/* 静态标签 */
	LCD_DrawLabel(&FormGPRS->Label1);
	LCD_DrawLabel(&FormGPRS->Label3);
	LCD_DrawLabel(&FormGPRS->Label5);
	LCD_DrawLabel(&FormGPRS->Label7);
	LCD_DrawLabel(&FormGPRS->Label9);

	/* 按钮 */
	LCD_DrawButton(&FormGPRS->BtnRet);
	LCD_DrawButton(&FormGPRS->Btn1);
	LCD_DrawButton(&FormGPRS->Btn2);
	LCD_DrawButton(&FormGPRS->Btn3);
	LCD_DrawButton(&FormGPRS->Btn4);
	LCD_DrawButton(&FormGPRS->Btn5);

	/* 编辑框 */
	LCD_DrawEdit(&FormGPRS->Edit1);

	/* 动态标签 */
	LCD_DrawLabel(&FormGPRS->Label2);

	/* 网络注册状态 */
	if (FormGPRS->ucNetStatus == CREG_NO_REG)
	{
		FormGPRS->Label4.pCaption = "没有注册, 已停止搜寻";
	}
	else if (FormGPRS->ucNetStatus == CREG_LOCAL_OK)
	{
		FormGPRS->Label4.pCaption = "已注册到本地网络      ";
	}
	else if (FormGPRS->ucNetStatus == CREG_SEARCH)
	{
		FormGPRS->Label4.pCaption = "正在搜寻网络运营商  ";
	}
	else if (FormGPRS->ucNetStatus == CREG_REJECT)
	{
		FormGPRS->Label4.pCaption = "注册被拒绝          ";
	}
	else if (FormGPRS->ucNetStatus == CREG_REMOTE_OK)
	{
		FormGPRS->Label4.pCaption = "已注册到漫游网络     ";
	}
	else
	{
		FormGPRS->Label4.pCaption = "xxx                 ";
	}
	LCD_DrawLabel(&FormGPRS->Label4);

	if (FormGPRS->ucAudioCh == 0)
	{
		FormGPRS->Label6.pCaption = "第1通道 INTMIC, INTEAR";
	}
	else
	{
		FormGPRS->Label6.pCaption = "第2通道 EXTMIC, EXTEAR";
	}
	LCD_DrawLabel(&FormGPRS->Label6);

	/* 耳机音量 */
	{
		char buf[64];

		sprintf(buf, "Ear = %d, Mic = %d", FormGPRS->ucEarVolume, FormGPRS->ucMicGain);
		FormGPRS->Label8.pCaption = buf;
		LCD_DrawLabel(&FormGPRS->Label8);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
