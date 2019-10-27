/*
*********************************************************************************************************
*
*	模块名称 : DAC8501 DAC8562模块测试界面
*	文件名称 : form_dac8501.c
*	版    本 : V1.0
*	说    明 : 驱动安富莱DAC8501模块和DAC8562模块。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2014-10-15 armfly  首发
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_dac8501.h"
#include "math.h"

#define TIMx TIM5
#define TIMx_IRQHandler TIM5_IRQHandler

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack; /* 黑色 */
	FONT_T FontBlue;	/* 蓝色 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;

	BUTTON_T BtnRet;

	BUTTON_T Btn1;
	BUTTON_T Btn2;
	BUTTON_T Btn3;
	BUTTON_T Btn4;
	BUTTON_T Btn5;
	BUTTON_T Btn6;
	BUTTON_T Btn7;
	BUTTON_T Btn8;
	BUTTON_T Btn9;
	BUTTON_T Btn10;
	BUTTON_T Btn11;
	BUTTON_T Btn12;
	BUTTON_T Btn13;
	BUTTON_T Btn14;
	BUTTON_T Btn15;

	LABEL_T Label1;
	LABEL_T Label2;
	LABEL_T Label3;
	LABEL_T Label4;
	LABEL_T Label5;
	LABEL_T Label6;

	uint16_t DacValue; /* DAC数据 */
	int32_t Voltage;	 /* 输出电压 */
} Form8501_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H (g_LcdHeight - 53)
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_T "DAC8501 双路DAC模块(16bit, 0 -> 5V)"

#define BTN1_H 32
#define BTN1_W 105

/* 第1行按钮坐标 */
#define BTN1_X (BOX1_X + 10)
#define BTN1_Y (BOX1_Y + 20)
#define BTN1_T "0V"

#define BTN2_H BTN1_H
#define BTN2_W BTN1_W
#define BTN2_X (BTN1_X + BTN1_W + 10)
#define BTN2_Y BTN1_Y
#define BTN2_T "2.5V"

#define BTN3_H BTN1_H
#define BTN3_W BTN1_W
#define BTN3_X (BTN1_X + 2 * (BTN1_W + 10))
#define BTN3_Y BTN1_Y
#define BTN3_T "5.0V"

/* 第2行按钮坐标 */
#define BTN4_H BTN1_H
#define BTN4_W BTN1_W
#define BTN4_X BTN1_X
#define BTN4_Y (BTN1_Y + BTN1_H + 10)
#define BTN4_T "DAC值+1"

#define BTN5_H BTN1_H
#define BTN5_W BTN1_W
#define BTN5_X (BTN1_X + 1 * (BTN1_W + 10))
#define BTN5_Y BTN4_Y
#define BTN5_T "DAC值-1"

#define BTN6_H BTN1_H
#define BTN6_W BTN1_W
#define BTN6_X (BTN1_X + 2 * (BTN1_W + 10))
#define BTN6_Y BTN4_Y
#define BTN6_T "DAC值+100"

#define BTN7_H BTN1_H
#define BTN7_W BTN1_W
#define BTN7_X (BTN1_X + 3 * (BTN1_W + 10))
#define BTN7_Y BTN4_Y
#define BTN7_T "DAC值-100"

/* 第3行按钮坐标 */
#define BTN8_H BTN1_H
#define BTN8_W BTN1_W
#define BTN8_X BTN1_X
#define BTN8_Y (BTN1_Y + 2 * (BTN1_H + 10))
#define BTN8_T "电压+1mV"

#define BTN9_H BTN1_H
#define BTN9_W BTN1_W
#define BTN9_X (BTN1_X + 1 * (BTN1_W + 10))
#define BTN9_Y BTN8_Y
#define BTN9_T "电压-1mV"

#define BTN10_H BTN1_H
#define BTN10_W BTN1_W
#define BTN10_X (BTN1_X + 2 * (BTN1_W + 10))
#define BTN10_Y BTN8_Y
#define BTN10_T "电压+100mV"

#define BTN11_H BTN1_H
#define BTN11_W BTN1_W
#define BTN11_X (BTN1_X + 3 * (BTN1_W + 10))
#define BTN11_Y BTN8_Y
#define BTN11_T "电压-100mV"

/* 第4行按钮坐标 */
#define BTN12_H BTN1_H
#define BTN12_W BTN1_W
#define BTN12_X BTN1_X
#define BTN12_Y (BTN1_Y + 3 * (BTN1_H + 10))
#define BTN12_T "同步正弦波"

#define BTN13_H BTN1_H
#define BTN13_W BTN1_W
#define BTN13_X (BTN1_X + 1 * (BTN1_W + 10))
#define BTN13_Y BTN12_Y
#define BTN13_T "相差90度正弦"

#define BTN14_H BTN1_H
#define BTN14_W BTN1_W
#define BTN14_X (BTN1_X + 2 * (BTN1_W + 10))
#define BTN14_Y BTN12_Y
#define BTN14_T "相差180度正弦"

#define BTN15_H BTN1_H
#define BTN15_W BTN1_W
#define BTN15_X (BTN1_X + 3 * (BTN1_W + 10))
#define BTN15_Y BTN12_Y
#define BTN15_T "停止正弦输出"

#define LBL1_X BOX1_X + 5
#define LBL1_Y 190
#define LBL1_T "当前DAC值:"

#define LBL2_X LBL1_X + 85
#define LBL2_Y LBL1_Y
#define LBL2_T ""

#define LBL3_X LBL1_X
#define LBL3_Y LBL1_Y + 20
#define LBL3_T " 输出电压:"

#define LBL4_X LBL3_X + 85
#define LBL4_Y LBL3_Y
#define LBL4_T " "

/* 按钮 */
/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 80
#define BTN_RET_X (g_LcdWidth - BTN_RET_W - 8)
#define BTN_RET_Y (g_LcdHeight - BTN_RET_H - 4)
#define BTN_RET_T "返回"

static void InitForm8501(void);
static void DispForm8501(void);

static void DispDacValue(void);
static void DispDacVoltage(void);
static void MakeSinTable(uint16_t *_pBuf, uint16_t _usSamples, uint16_t _usBottom, uint16_t _usTop);

Form8501_T *Form8501;

#define DAC_OUT_FREQ 10000 /* DAC 输出样本频率 10KHz */
#define WAVE_SAMPLES 200	 /* 每周期样本数， 越大波形幅度越细腻，但是输出最大频率会降低 */

static uint16_t s_WaveBuf[WAVE_SAMPLES];
static uint16_t s_WavePos1, s_WavePos2;
/*
*********************************************************************************************************
*	函 数 名: InitForm8501
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitForm8501(void)
{
	/* 分组框标题字体 */
	Form8501->FontBox.FontCode = FC_ST_16;
	Form8501->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
	Form8501->FontBox.FrontColor = CL_BLACK;
	Form8501->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	Form8501->FontBlack.FontCode = FC_ST_16;
	Form8501->FontBlack.BackColor = CL_MASK; /* 透明色 */
	Form8501->FontBlack.FrontColor = CL_BLACK;
	Form8501->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	Form8501->FontBlue.FontCode = FC_ST_16;
	Form8501->FontBlue.BackColor = CL_BTN_FACE;
	Form8501->FontBlue.FrontColor = CL_BLUE;
	Form8501->FontBlue.Space = 0;

	/* 按钮字体 */
	Form8501->FontBtn.FontCode = FC_ST_16;
	Form8501->FontBtn.BackColor = CL_MASK; /* 透明背景 */
	Form8501->FontBtn.FrontColor = CL_BLACK;
	Form8501->FontBtn.Space = 0;

	/* 分组框 */
	Form8501->Box1.Left = BOX1_X;
	Form8501->Box1.Top = BOX1_Y;
	Form8501->Box1.Height = BOX1_H;
	Form8501->Box1.Width = BOX1_W;
	Form8501->Box1.pCaption = BOX1_T;
	Form8501->Box1.Font = &Form8501->FontBox;

	/* 标签 */
	Form8501->Label1.Left = LBL1_X;
	Form8501->Label1.Top = LBL1_Y;
	Form8501->Label1.MaxLen = 0;
	Form8501->Label1.pCaption = LBL1_T;
	Form8501->Label1.Font = &Form8501->FontBlack;

	Form8501->Label2.Left = LBL2_X;
	Form8501->Label2.Top = LBL2_Y;
	Form8501->Label2.MaxLen = 0;
	Form8501->Label2.pCaption = LBL2_T;
	Form8501->Label2.Font = &Form8501->FontBlue;

	Form8501->Label3.Left = LBL3_X;
	Form8501->Label3.Top = LBL3_Y;
	Form8501->Label3.MaxLen = 0;
	Form8501->Label3.pCaption = LBL3_T;
	Form8501->Label3.Font = &Form8501->FontBlack;

	Form8501->Label4.Left = LBL4_X;
	Form8501->Label4.Top = LBL4_Y;
	Form8501->Label4.MaxLen = 0;
	Form8501->Label4.pCaption = LBL4_T;
	Form8501->Label4.Font = &Form8501->FontBlue;

	/* 按钮 */
	Form8501->BtnRet.Left = BTN_RET_X;
	Form8501->BtnRet.Top = BTN_RET_Y;
	Form8501->BtnRet.Height = BTN_RET_H;
	Form8501->BtnRet.Width = BTN_RET_W;
	Form8501->BtnRet.pCaption = BTN_RET_T;
	Form8501->BtnRet.Font = &Form8501->FontBtn;
	Form8501->BtnRet.Focus = 0;

	Form8501->Btn1.Left = BTN1_X;
	Form8501->Btn1.Top = BTN1_Y;
	Form8501->Btn1.Height = BTN1_H;
	Form8501->Btn1.Width = BTN1_W;
	Form8501->Btn1.pCaption = BTN1_T;
	Form8501->Btn1.Font = &Form8501->FontBtn;
	Form8501->Btn1.Focus = 0;

	Form8501->Btn2.Left = BTN2_X;
	Form8501->Btn2.Top = BTN2_Y;
	Form8501->Btn2.Height = BTN2_H;
	Form8501->Btn2.Width = BTN2_W;
	Form8501->Btn2.pCaption = BTN2_T;
	Form8501->Btn2.Font = &Form8501->FontBtn;
	Form8501->Btn2.Focus = 0;

	Form8501->Btn3.Left = BTN3_X;
	Form8501->Btn3.Top = BTN3_Y;
	Form8501->Btn3.Height = BTN3_H;
	Form8501->Btn3.Width = BTN3_W;
	Form8501->Btn3.pCaption = BTN3_T;
	Form8501->Btn3.Font = &Form8501->FontBtn;
	Form8501->Btn3.Focus = 0;

	Form8501->Btn4.Left = BTN4_X;
	Form8501->Btn4.Top = BTN4_Y;
	Form8501->Btn4.Height = BTN4_H;
	Form8501->Btn4.Width = BTN4_W;
	Form8501->Btn4.pCaption = BTN4_T;
	Form8501->Btn4.Font = &Form8501->FontBtn;
	Form8501->Btn4.Focus = 0;

	Form8501->Btn5.Left = BTN5_X;
	Form8501->Btn5.Top = BTN5_Y;
	Form8501->Btn5.Height = BTN5_H;
	Form8501->Btn5.Width = BTN5_W;
	Form8501->Btn5.pCaption = BTN5_T;
	Form8501->Btn5.Font = &Form8501->FontBtn;
	Form8501->Btn5.Focus = 0;

	Form8501->Btn6.Left = BTN6_X;
	Form8501->Btn6.Top = BTN6_Y;
	Form8501->Btn6.Height = BTN6_H;
	Form8501->Btn6.Width = BTN6_W;
	Form8501->Btn6.pCaption = BTN6_T;
	Form8501->Btn6.Font = &Form8501->FontBtn;
	Form8501->Btn6.Focus = 0;

	Form8501->Btn7.Left = BTN7_X;
	Form8501->Btn7.Top = BTN7_Y;
	Form8501->Btn7.Height = BTN7_H;
	Form8501->Btn7.Width = BTN7_W;
	Form8501->Btn7.pCaption = BTN7_T;
	Form8501->Btn7.Font = &Form8501->FontBtn;
	Form8501->Btn7.Focus = 0;

	Form8501->Btn8.Left = BTN8_X;
	Form8501->Btn8.Top = BTN8_Y;
	Form8501->Btn8.Height = BTN8_H;
	Form8501->Btn8.Width = BTN8_W;
	Form8501->Btn8.pCaption = BTN8_T;
	Form8501->Btn8.Font = &Form8501->FontBtn;
	Form8501->Btn8.Focus = 0;

	Form8501->Btn9.Left = BTN9_X;
	Form8501->Btn9.Top = BTN9_Y;
	Form8501->Btn9.Height = BTN9_H;
	Form8501->Btn9.Width = BTN9_W;
	Form8501->Btn9.pCaption = BTN9_T;
	Form8501->Btn9.Font = &Form8501->FontBtn;
	Form8501->Btn9.Focus = 0;

	Form8501->Btn10.Left = BTN10_X;
	Form8501->Btn10.Top = BTN10_Y;
	Form8501->Btn10.Height = BTN10_H;
	Form8501->Btn10.Width = BTN10_W;
	Form8501->Btn10.pCaption = BTN10_T;
	Form8501->Btn10.Font = &Form8501->FontBtn;
	Form8501->Btn10.Focus = 0;

	Form8501->Btn11.Left = BTN11_X;
	Form8501->Btn11.Top = BTN11_Y;
	Form8501->Btn11.Height = BTN11_H;
	Form8501->Btn11.Width = BTN11_W;
	Form8501->Btn11.pCaption = BTN11_T;
	Form8501->Btn11.Font = &Form8501->FontBtn;
	Form8501->Btn11.Focus = 0;

	Form8501->Btn12.Left = BTN12_X;
	Form8501->Btn12.Top = BTN12_Y;
	Form8501->Btn12.Height = BTN12_H;
	Form8501->Btn12.Width = BTN12_W;
	Form8501->Btn12.pCaption = BTN12_T;
	Form8501->Btn12.Font = &Form8501->FontBtn;
	Form8501->Btn12.Focus = 0;

	Form8501->Btn13.Left = BTN13_X;
	Form8501->Btn13.Top = BTN13_Y;
	Form8501->Btn13.Height = BTN13_H;
	Form8501->Btn13.Width = BTN13_W;
	Form8501->Btn13.pCaption = BTN13_T;
	Form8501->Btn13.Font = &Form8501->FontBtn;
	Form8501->Btn13.Focus = 0;

	Form8501->Btn14.Left = BTN14_X;
	Form8501->Btn14.Top = BTN14_Y;
	Form8501->Btn14.Height = BTN14_H;
	Form8501->Btn14.Width = BTN14_W;
	Form8501->Btn14.pCaption = BTN14_T;
	Form8501->Btn14.Font = &Form8501->FontBtn;
	Form8501->Btn14.Focus = 0;

	Form8501->Btn15.Left = BTN15_X;
	Form8501->Btn15.Top = BTN15_Y;
	Form8501->Btn15.Height = BTN15_H;
	Form8501->Btn15.Width = BTN15_W;
	Form8501->Btn15.pCaption = BTN15_T;
	Form8501->Btn15.Font = &Form8501->FontBtn;
	Form8501->Btn15.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: FormMainDAC8501
*	功能说明: DAC8501 DAC8562测试主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void FormMainDAC8501(void)
{
	uint8_t ucKeyCode; /* 按键代码 */
	uint8_t ucTouch;	 /* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	Form8501_T form;
	uint8_t fDispVolt = 1;

	Form8501 = &form;

	InitForm8501();
	DispForm8501();

	bsp_InitDAC8501();

	MakeSinTable(s_WaveBuf, WAVE_SAMPLES, 0, 65535);

	Form8501->DacValue = 0;

	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		if (fDispVolt)
		{
			fDispVolt = 0;

			DAC8501_SetDacData(0, Form8501->DacValue);
			DAC8501_SetDacData(1, Form8501->DacValue);

			DispDacValue();
			DispDacVoltage();
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
			case TOUCH_DOWN: /* 触笔按下事件 */
				if (LCD_ButtonTouchDown(&Form8501->BtnRet, tpX, tpY))
				{
					//fQuit = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn1, tpX, tpY))
				{
					Form8501->DacValue = 0;
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn2, tpX, tpY))
				{
					Form8501->DacValue = 32767;
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn3, tpX, tpY))
				{
					Form8501->DacValue = 65535;
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn4, tpX, tpY))
				{
					Form8501->DacValue++;
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn5, tpX, tpY))
				{
					Form8501->DacValue--;
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn6, tpX, tpY))
				{
					Form8501->DacValue += 100;
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn7, tpX, tpY))
				{
					Form8501->DacValue -= 100;
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn8, tpX, tpY))
				{
					/* +1mV */
					Form8501->Voltage = DAC8501_DacToVoltage(Form8501->DacValue);
					Form8501->Voltage += 10;
					Form8501->DacValue = DAC8501_VoltageToDac(Form8501->Voltage);
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn9, tpX, tpY))
				{
					/* -1mV */
					Form8501->Voltage = DAC8501_DacToVoltage(Form8501->DacValue);
					Form8501->Voltage -= 10;
					Form8501->DacValue = DAC8501_VoltageToDac(Form8501->Voltage);
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn10, tpX, tpY))
				{
					/* +100mV */
					Form8501->Voltage = DAC8501_DacToVoltage(Form8501->DacValue);
					Form8501->Voltage += 1000;
					Form8501->DacValue = DAC8501_VoltageToDac(Form8501->Voltage);
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn11, tpX, tpY))
				{
					/* -100mV */
					Form8501->Voltage = DAC8501_DacToVoltage(Form8501->DacValue);
					Form8501->Voltage -= 1000;
					Form8501->DacValue = DAC8501_VoltageToDac(Form8501->Voltage);
					fDispVolt = 1;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn12, tpX, tpY))
				{
					bsp_SetTIMforInt(TIMx, DAC_OUT_FREQ, 0, 0);
					s_WavePos1 = 0; /* 波形1超前 0度 */
					s_WavePos2 = 0;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn13, tpX, tpY))
				{
					bsp_SetTIMforInt(TIM4, DAC_OUT_FREQ, 0, 0);
					s_WavePos1 = WAVE_SAMPLES / 4; /* 波形1超前 90度 */
					s_WavePos2 = 0;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn14, tpX, tpY))
				{
					bsp_SetTIMforInt(TIM4, DAC_OUT_FREQ, 0, 0);
					s_WavePos1 = WAVE_SAMPLES / 2; /* 波形1超前 180度 */
					s_WavePos2 = 0;
				}
				else if (LCD_ButtonTouchDown(&Form8501->Btn15, tpX, tpY))
				{
					bsp_SetTIMforInt(TIM4, 0, 0, 0); /* 关闭用于波形发生的定时器 */
					fDispVolt = 1;
				}
				break;

			case TOUCH_RELEASE: /* 触笔释放事件 */
				if (LCD_ButtonTouchRelease(&Form8501->BtnRet, tpX, tpY))
				{
					fQuit = 1; /* 返回 */
				}
				else
				{
					LCD_ButtonTouchRelease(&Form8501->BtnRet, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn1, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn2, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn3, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn4, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn5, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn6, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn7, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn8, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn9, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn10, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn11, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn12, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn13, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn14, tpX, tpY);
					LCD_ButtonTouchRelease(&Form8501->Btn15, tpX, tpY);
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
			case KEY_DOWN_K1: /* K1键 + 1*/
				break;

			case KEY_DOWN_K2: /* K2键 - 1 */
				break;

			case KEY_DOWN_K3: /* K3键 - 清0 */
				break;

			case JOY_DOWN_U: /* 摇杆UP键按下 */
				break;

			case JOY_DOWN_D: /* 摇杆DOWN键按下 */
				break;

			case JOY_DOWN_L: /* 摇杆LEFT键按下 */
				break;

			case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
				break;

			case JOY_DOWN_OK: /* 摇杆OK键按下 */
				break;

			default:
				break;
			}
		}
	}

	bsp_SetTIMforInt(TIM4, 0, 0, 0); /* 关闭用于波形发生的定时器 */
}

/*
*********************************************************************************************************
*	函 数 名: DispForm8501
*	功能说明: 显示所有的静态控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispForm8501(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&Form8501->Box1);

	LCD_DrawLabel(&Form8501->Label1);
	LCD_DrawLabel(&Form8501->Label2);
	LCD_DrawLabel(&Form8501->Label3);
	LCD_DrawLabel(&Form8501->Label4);

	/* 按钮 */
	LCD_DrawButton(&Form8501->Btn1);
	LCD_DrawButton(&Form8501->Btn2);
	LCD_DrawButton(&Form8501->Btn3);
	LCD_DrawButton(&Form8501->Btn4);
	LCD_DrawButton(&Form8501->Btn5);
	LCD_DrawButton(&Form8501->Btn6);
	LCD_DrawButton(&Form8501->Btn5);
	LCD_DrawButton(&Form8501->Btn6);
	LCD_DrawButton(&Form8501->Btn7);
	LCD_DrawButton(&Form8501->Btn8);
	LCD_DrawButton(&Form8501->Btn9);
	LCD_DrawButton(&Form8501->Btn10);
	LCD_DrawButton(&Form8501->Btn11);
	LCD_DrawButton(&Form8501->Btn12);
	LCD_DrawButton(&Form8501->Btn13);
	LCD_DrawButton(&Form8501->Btn14);
	LCD_DrawButton(&Form8501->Btn15);
	LCD_DrawButton(&Form8501->BtnRet);
}

/*
*********************************************************************************************************
*	函 数 名: DispDacValue
*	功能说明: 显示当前DAC数值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispDacValue(void)
{
	char buf[10];

	sprintf(buf, "%d", Form8501->DacValue);

	Form8501->Label2.pCaption = buf;
	LCD_DrawLabel(&Form8501->Label2);
}

/*
*********************************************************************************************************
*	函 数 名: DispDacVoltage
*	功能说明: 显示电压
*	形    参: 无
*			  _
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispDacVoltage(void)
{
	char buf[10];

	Form8501->Voltage = DAC8501_DacToVoltage(Form8501->DacValue);

	sprintf(buf, "%d.%04dV", Form8501->Voltage / 10000, Form8501->Voltage % 10000);

	Form8501->Label4.pCaption = buf;
	LCD_DrawLabel(&Form8501->Label4);
}

/*
*********************************************************************************************************
*	函 数 名: MakeSinTable
*	功能说明: 计算产生正弦波数组
*	形    参: _pBuf : 目标缓冲区
*			  _usSamples : 每个周期的样本数 （建议大于32，并且是偶数）
*			 _usBottom : 波谷值
*			 _usTop : 波峰值
*	返 回 值: 无
*********************************************************************************************************
*/
static void MakeSinTable(uint16_t *_pBuf, uint16_t _usSamples, uint16_t _usBottom, uint16_t _usTop)
{
	uint16_t i;
	uint16_t mid; /* 中值 */
	uint16_t att; /* 幅度 */

	mid = (_usBottom + _usTop) / 2; /* 0位的值 */
	att = (_usTop - _usBottom) / 2; /* 正弦波幅度，峰峰值除以2 */

	for (i = 0; i < _usSamples; i++)
	{
		_pBuf[i] = mid + (int32_t)(att * sin((i * 2 * 3.14159) / _usSamples));
	}
}

/*
*********************************************************************************************************
*	函 数 名: TIM4_IRQHandler
*	功能说明: TIM4 中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/

void TIMx_IRQHandler(void)
{
	uint16_t dac;

	if (READ_BIT(TIMx->SR, TIM_IT_UPDATE) != 0)
	{
		CLEAR_BIT(TIMx->SR, TIM_IT_UPDATE);

		/* 3.5寸屏，如果触摸程序正在访问SPI触摸芯片TSC2046，则丢弃本次 */
		if (bsp_SpiBusBusy())
		{
			return;
		}

		/* 波形1 */
		dac = s_WaveBuf[s_WavePos1++];
		if (s_WavePos1 >= WAVE_SAMPLES)
		{
			s_WavePos1 = 0;
		}
		DAC8501_SetDacData(0, dac); /* 改变第1通道 DAC输出电压 */

		/* 波形1 */
		dac = s_WaveBuf[s_WavePos2++];
		if (s_WavePos2 >= WAVE_SAMPLES)
		{
			s_WavePos2 = 0;
		}
		DAC8501_SetDacData(1, dac); /* 改变第2通道 DAC输出电压 */
	}
}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
