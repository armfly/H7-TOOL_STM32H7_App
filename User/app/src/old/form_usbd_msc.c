/*
*********************************************************************************************************
*
*	模块名称 : SD卡和NAND Flash模拟U盘程序。
*	文件名称 : usbd_msc_test.c
*	版    本 : V1.1
*	说    明 : 使用USB Device接口，在PC上虚拟出2个U盘设备:SD卡 和 NAND Flash
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2013-02-01 armfly  首发
*		V1.1	2015-10-13 armfly  K3键启动NAND坏块重新标记, 之前先关闭USB，避免PC和STM32同时访问NAND。
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "usbd_usr.h"	/* usb底层驱动 */

#include "form_usbd_msc.h"

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack;	/* 静态的文字 */
	FONT_T FontBlue;	/* 变化的文字字体 蓝色 */
	FONT_T FontRed;	/* 变化的文字字体 红色 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;

	LABEL_T Label1;	LABEL_T Label2;	/* SD卡状态 */
	LABEL_T Label3; LABEL_T Label4;	/* NAND状态 */
	LABEL_T Label5; LABEL_T Label6;	/* USBD状态 */

	BUTTON_T Btn1;
	BUTTON_T Btn2;
	BUTTON_T Btn3;
	BUTTON_T Btn4;

	BUTTON_T BtnRet;

}FormUSB_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 4个框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"SD卡和NAND Flash模拟U盘"

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	60
#define	BTN_RET_X	((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define	BTN_RET_Y	((BOX1_Y  + BOX1_H) - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"返回"

#define BTN1_H	32
#define BTN1_W	100
#define	BTN1_X	(BOX1_X + 5)
#define	BTN1_Y	(BOX1_Y + 100)
#define	BTN1_TEXT	"打开模拟U盘"

#define BTN2_H	32
#define BTN2_W	100
#define	BTN2_X	(BTN1_X + BTN1_W + 10)
#define	BTN2_Y	BTN1_Y
#define	BTN2_TEXT	"关闭模拟U盘"

#define BTN3_H	32
#define BTN3_W	100
#define	BTN3_X	BTN1_X
#define	BTN3_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN3_TEXT	"低格NAND"

#define BTN4_H	32
#define BTN4_W	100
#define	BTN4_X	(BTN1_X + BTN1_W + 10)
#define	BTN4_Y	(BTN1_Y + BTN1_H + 10)
#define	BTN4_TEXT	"扫描坏块"

/* 标签 */
#define LABEL1_X  	(BOX1_X + 6)
#define LABEL1_Y	(BOX1_Y + 20)
#define LABEL1_TEXT	"SD卡状态 : "

	#define LABEL2_X  	(LABEL1_X + 100)
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"--"

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"NAND状态 : "

	#define LABEL4_X  	(LABEL3_X + 100)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	"--"

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 2)
#define LABEL5_TEXT	"USBD状态 : "

	#define LABEL6_X  	(LABEL5_X + 100)
	#define LABEL6_Y	(LABEL5_Y)
	#define LABEL6_TEXT	"--"

static void InitFormUSB(void);
static void DispUSBInitFace(void);

FormUSB_T *FormUSB;

/*
*********************************************************************************************************
*	函 数 名: TestUsbdMsc
*	功能说明: 虚拟U盘程序入口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestUsbdMsc(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	uint8_t ucNandOk;
	uint8_t ucCardOk;
	uint8_t ucUsbOk;
	FormUSB_T form;
	uint8_t fRefresh;

	char buf[64];

	FormUSB = &form;

	InitFormUSB();
	DispUSBInitFace();


	{	
	#if 1		
		NAND_DispBadBlockInfo();	/* 向串口1打印NAND Flash坏块信息 (此函数开头初始FSMC) */

		/* 配置FSMC用于NAND Flash， 复位NAND Flash，重建LUT表 */
		if (NAND_Init() == NAND_OK)
		{
			printf("NAND_Init() Ok\r\n");
			ucNandOk = 1;
		}
		else
		{
			/* 建议在正式的产品中采用人为干预的方式启动低级格式化 */
			/* 自动检测nand flash是否进行了低级格式化，如果没有则执行格式化（2秒） */
			printf("NAND_Init() Error! \r\n");
			printf("Start Format(Low Level) NAND Flash......\r\n");
			if (NAND_Format() == NAND_OK)
			{
				printf("NAND Flash Format Ok\r\n");
			}
			else
			{
				printf("NAND Flash Format Error\r\n");
			}

			ucNandOk = 1;
		}

		if (NAND_GetBlockInfo(&nand) == 1)
		{
			ucNandOk = 1;
			
			#if 0
			NAND_DispParamPage();	/* 显示NAND 芯片版本信息 */
			#endif
		}
		else
		{
			ucNandOk = 0;
		}	
	#endif
		if (BSP_SD_Init() == MSD_OK)
		{
			ucCardOk = 1;
		}
		else
		{
			ucCardOk = 0;
		}
	}


	ucUsbOk = 1;
	usbd_OpenMassStorage();		/* 初始化USB Device，配置为Mass Storage */

	fRefresh = 1;

	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		if (fRefresh)
		{
			fRefresh = 0;

			if (ucCardOk)
			{
				FormUSB->Label2.Font = &FormUSB->FontBlue;
				FormUSB->Label2.pCaption = "Ok ";
			}
			else
			{
				FormUSB->Label2.Font = &FormUSB->FontRed;
				FormUSB->Label2.pCaption = "Err";
			}
			LCD_DrawLabel(&FormUSB->Label2);

			if (ucNandOk)
			{
				FormUSB->Label4.pCaption = buf;
				sprintf(buf, "OK, %s, Bad=%d, Used=%d, Free=%d", nand.ChipName, nand.Bad, nand.Used, nand.Free);
				
				if (nand.Bad > 30)
				{
					/* 如果坏块个数大于30个，则显示红色 */
					FormUSB->Label4.Font = &FormUSB->FontRed;
				}
				else
				{
					/* 坏块数量在正常范围内，显示蓝色 */
					FormUSB->Label4.Font = &FormUSB->FontBlue;
				}
			}
			else
			{
				FormUSB->Label4.Font = &FormUSB->FontRed;
				FormUSB->Label4.pCaption = "Err";
			}
			LCD_DrawLabel(&FormUSB->Label4);

			if (ucUsbOk)
			{
				FormUSB->Label6.Font = &FormUSB->FontBlue;
				FormUSB->Label6.pCaption = "已打开 ";
			}
			else
			{
				FormUSB->Label6.Font = &FormUSB->FontRed;
				FormUSB->Label6.pCaption = "已关闭";
			}
			LCD_DrawLabel(&FormUSB->Label6);
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */					
					LCD_ButtonTouchDown(&FormUSB->BtnRet, tpX, tpY);
					LCD_ButtonTouchDown(&FormUSB->Btn1, tpX, tpY);
					LCD_ButtonTouchDown(&FormUSB->Btn2, tpX, tpY);
					LCD_ButtonTouchDown(&FormUSB->Btn3, tpX, tpY);
					LCD_ButtonTouchDown(&FormUSB->Btn4, tpX, tpY);					
					break;

				case TOUCH_MOVE:		/* 触笔移动事件 */
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (LCD_ButtonTouchRelease(&FormUSB->BtnRet, tpX, tpY))
					{
						FormUSB->BtnRet.Focus = 0;
						LCD_DrawButton(&FormUSB->BtnRet);
						fQuit = 1;	/* 返回 */
					}
					else if (LCD_ButtonTouchRelease(&FormUSB->Btn1, tpX, tpY))
					{
						FormUSB->Btn1.Focus = 0;
						LCD_DrawButton(&FormUSB->Btn1);

						if (BSP_SD_Init() == MSD_OK)
						{
							ucCardOk = 1;
						}
						else
						{
							ucCardOk = 0;
						}
						usbd_OpenMassStorage();	/* 打开U盘 */
						ucUsbOk = 1;
						fRefresh = 1;
					}
					else if (LCD_ButtonTouchRelease(&FormUSB->Btn2, tpX, tpY))
					{
						usbd_CloseMassStorage();	/* 关闭U盘 */
						ucUsbOk = 0;
						fRefresh = 1;
					}
					else if (LCD_ButtonTouchRelease(&FormUSB->Btn3, tpX, tpY))
					{						
						if (NAND_Format() == NAND_OK)
						{
							FormUSB->Label4.pCaption = "低级格式化成功";
						}
						else
						{
							FormUSB->Label4.pCaption = "低级格式化失败";
						}
						LCD_DrawLabel(&FormUSB->Label4);
						
					}			
					else if (LCD_ButtonTouchRelease(&FormUSB->Btn4, tpX, tpY))
					{
						if (ucUsbOk == 0)	/* 关闭USB连接时才能访问NAND，否则USB中断程序会干扰扫描过程 */
						{
							/* 重新扫描测试坏块，执行时间很长。 用于NAND 磁盘修复 */						
							uint32_t i;
							
							FormUSB->Label4.pCaption = buf;
							FormUSB->Label4.Font = &FormUSB->FontRed;
							for (i = 0; i < NAND_BLOCK_COUNT; i++)
							{
								sprintf(buf, "扫描坏块... %d (%d%%) --- K1键终止", i, (i + 1) * 100 / NAND_BLOCK_COUNT);
								LCD_DrawLabel(&FormUSB->Label4);
								
								if (NAND_ScanBlock(i) == NAND_OK)
								{
									;
								}
								else
								{
									NAND_MarkBadBlock(i);	/* 标记坏块 */
								}
								
								/* 如果有任意键按下，则退出 */
								if (bsp_GetKey() == KEY_DOWN_K1)
								{
									sprintf(buf, "扫描终止");
									LCD_DrawLabel(&FormUSB->Label4);
									break;
								}
							}		
						}
		
						else
						{
							FormUSB->Label4.Font = &FormUSB->FontRed;
							FormUSB->Label4.pCaption = buf;
							sprintf(buf, "请先关闭模拟U盘功能");
							LCD_DrawLabel(&FormUSB->Label4);
						}
					}				
					else	/* 按钮失去焦点 */
					{
						LCD_ButtonTouchRelease(&FormUSB->BtnRet, tpX, tpY);
						LCD_ButtonTouchRelease(&FormUSB->Btn1, tpX, tpY);
						LCD_ButtonTouchRelease(&FormUSB->Btn2, tpX, tpY);
						LCD_ButtonTouchRelease(&FormUSB->Btn3, tpX, tpY);
						LCD_ButtonTouchRelease(&FormUSB->Btn4, tpX, tpY);
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
					//printf("【1 - 移除U盘】\r\n");
					//usbd_CloseMassStorage();
					break;

				case KEY_DOWN_K2:		/* K2键按下 */
					/* 使能U盘，软件模拟U盘插入 */
					//printf("【2 - 使能U盘】\r\n");
					//usbd_OpenMassStorage();
					break;

				case KEY_DOWN_K3:		/* K3键按下 */
					;
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

	usbd_CloseMassStorage();	/* 移除U盘 */
}

/*
*********************************************************************************************************
*	函 数 名: InitFormUSB
*	功能说明: 初始化GPS初始界面控件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormUSB(void)
{
	/* 分组框标题字体 */
	FormUSB->FontBox.FontCode = FC_ST_16;
	FormUSB->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormUSB->FontBox.FrontColor = CL_BLACK;
	FormUSB->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormUSB->FontBlack.FontCode = FC_ST_16;
	FormUSB->FontBlack.BackColor = CL_MASK;		/* 透明色 */
	FormUSB->FontBlack.FrontColor = CL_BLACK;
	FormUSB->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormUSB->FontBlue.FontCode = FC_ST_16;
	FormUSB->FontBlue.BackColor = CL_BTN_FACE;
	FormUSB->FontBlue.FrontColor = CL_BLUE;
	FormUSB->FontBlue.Space = 0;

	/* 字体3 用于变化的文字 */
	FormUSB->FontRed.FontCode = FC_ST_16;
	FormUSB->FontRed.BackColor = CL_BTN_FACE;
	FormUSB->FontRed.FrontColor = CL_RED;
	FormUSB->FontRed.Space = 0;

	/* 按钮字体 */
	FormUSB->FontBtn.FontCode = FC_ST_16;
	FormUSB->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormUSB->FontBtn.FrontColor = CL_BLACK;
	FormUSB->FontBtn.Space = 0;

	/* 分组框 */
	FormUSB->Box1.Left = BOX1_X;
	FormUSB->Box1.Top = BOX1_Y;
	FormUSB->Box1.Height = BOX1_H;
	FormUSB->Box1.Width = BOX1_W;
	FormUSB->Box1.pCaption = BOX1_TEXT;
	FormUSB->Box1.Font = &FormUSB->FontBox;

	/* 静态标签 */
	FormUSB->Label1.Left = LABEL1_X;
	FormUSB->Label1.Top = LABEL1_Y;
	FormUSB->Label1.MaxLen = 0;
	FormUSB->Label1.pCaption = LABEL1_TEXT;
	FormUSB->Label1.Font = &FormUSB->FontBlack;

	FormUSB->Label3.Left = LABEL3_X;
	FormUSB->Label3.Top = LABEL3_Y;
	FormUSB->Label3.MaxLen = 0;
	FormUSB->Label3.pCaption = LABEL3_TEXT;
	FormUSB->Label3.Font = &FormUSB->FontBlack;

	FormUSB->Label5.Left = LABEL5_X;
	FormUSB->Label5.Top = LABEL5_Y;
	FormUSB->Label5.MaxLen = 0;
	FormUSB->Label5.pCaption = LABEL5_TEXT;
	FormUSB->Label5.Font = &FormUSB->FontBlack;

	/* 动态标签 */
	FormUSB->Label2.Left = LABEL2_X;
	FormUSB->Label2.Top = LABEL2_Y;
	FormUSB->Label2.MaxLen = 0;
	FormUSB->Label2.pCaption = LABEL2_TEXT;
	FormUSB->Label2.Font = &FormUSB->FontBlue;

	FormUSB->Label4.Left = LABEL4_X;
	FormUSB->Label4.Top = LABEL4_Y;
	FormUSB->Label4.MaxLen = 0;
	FormUSB->Label4.pCaption = LABEL4_TEXT;
	FormUSB->Label4.Font = &FormUSB->FontBlue;

	FormUSB->Label6.Left = LABEL6_X;
	FormUSB->Label6.Top = LABEL6_Y;
	FormUSB->Label6.MaxLen = 0;
	FormUSB->Label6.pCaption = LABEL6_TEXT;
	FormUSB->Label6.Font = &FormUSB->FontBlue;

	/* 按钮 */
	FormUSB->BtnRet.Left = BTN_RET_X;
	FormUSB->BtnRet.Top = BTN_RET_Y;
	FormUSB->BtnRet.Height = BTN_RET_H;
	FormUSB->BtnRet.Width = BTN_RET_W;
	FormUSB->BtnRet.pCaption = BTN_RET_TEXT;
	FormUSB->BtnRet.Font = &FormUSB->FontBtn;
	FormUSB->BtnRet.Focus = 0;

	FormUSB->Btn1.Left = BTN1_X;
	FormUSB->Btn1.Top = BTN1_Y;
	FormUSB->Btn1.Height = BTN1_H;
	FormUSB->Btn1.Width = BTN1_W;
	FormUSB->Btn1.pCaption = BTN1_TEXT;
	FormUSB->Btn1.Font = &FormUSB->FontBtn;
	FormUSB->Btn1.Focus = 0;

	FormUSB->Btn2.Left = BTN2_X;
	FormUSB->Btn2.Top = BTN2_Y;
	FormUSB->Btn2.Height = BTN2_H;
	FormUSB->Btn2.Width = BTN2_W;
	FormUSB->Btn2.pCaption = BTN2_TEXT;
	FormUSB->Btn2.Font = &FormUSB->FontBtn;
	FormUSB->Btn2.Focus = 0;

	FormUSB->Btn3.Left = BTN3_X;
	FormUSB->Btn3.Top = BTN3_Y;
	FormUSB->Btn3.Height = BTN3_H;
	FormUSB->Btn3.Width = BTN3_W;
	FormUSB->Btn3.pCaption = BTN3_TEXT;
	FormUSB->Btn3.Font = &FormUSB->FontBtn;
	FormUSB->Btn3.Focus = 0;


	FormUSB->Btn4.Left = BTN4_X;
	FormUSB->Btn4.Top = BTN4_Y;
	FormUSB->Btn4.Height = BTN4_H;
	FormUSB->Btn4.Width = BTN4_W;
	FormUSB->Btn4.pCaption = BTN4_TEXT;
	FormUSB->Btn4.Font = &FormUSB->FontBtn;
	FormUSB->Btn4.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispUSBInitFace
*	功能说明: 显示所有的控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispUSBInitFace(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormUSB->Box1);

	/* 标签 */
	LCD_DrawLabel(&FormUSB->Label1);
	LCD_DrawLabel(&FormUSB->Label2);
	LCD_DrawLabel(&FormUSB->Label3);
	LCD_DrawLabel(&FormUSB->Label4);
	LCD_DrawLabel(&FormUSB->Label5);
	LCD_DrawLabel(&FormUSB->Label6);

	/* 按钮 */
	LCD_DrawButton(&FormUSB->BtnRet);
	LCD_DrawButton(&FormUSB->Btn1);
	LCD_DrawButton(&FormUSB->Btn2);
	LCD_DrawButton(&FormUSB->Btn3);
	LCD_DrawButton(&FormUSB->Btn4);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
