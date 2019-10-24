/*
*********************************************************************************************************
*
*	模块名称 : 系统设置主程序
*	文件名称 : status_system_set.c
*	版    本 : V1.0
*	说    明 : 提供一个菜单选择子功能.
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-12-06  armfly  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

#include "status_system_set.h"
#include "lcd_menu.h"
#include "wifi_if.h"
#include "usbd_user.h"

const uint8_t *g_Menu1_Text[] =
		{
				" 1 硬件信息",
				" 2 USB转串口模式",
				" 3 ESP32固件升级",

				/* 结束符号, 用于菜单函数自动识别菜单项个数 */
				"&"};

MENU_T g_tMenu1;

/*
*********************************************************************************************************
*	函 数 名: status_SystemSetMain
*	功能说明: 系统设置状态. 菜单选择
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_SystemSetMain(void)
{
	uint8_t ucKeyCode; /* 按键代码 */
	uint8_t fRefresh;
	FONT_T tFont; /* 定义字体结构体变量 */
	uint8_t ucFirstKey = 1;
	static uint8_t s_enter_sub_menu = 0;

	DispHeader("系统设置");

	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;				 /* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;			 /* 字体颜色 */
		tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
		tFont.Space = 0;									 /* 文字间距，单位 = 像素 */
	}

	if (s_enter_sub_menu == 0)
	{
		g_tMenu1.Left = MENU_LEFT;
		g_tMenu1.Top = 30;
		g_tMenu1.Height = MENU_HEIGHT;
		g_tMenu1.Width = MENU_WIDTH;
		g_tMenu1.LineCap = 2;
		g_tMenu1.ViewLine = 8;
		g_tMenu1.Font.FontCode = FC_ST_24;							/* 字体代码 16点阵 */
																										//		g_tMenu1.Font.FrontColor = CL_BLACK;		/* 字体颜色 */
																										//		g_tMenu1.Font.BackColor = FORM_BACK_COLOR;	/* 文字背景颜色 */
		g_tMenu1.Font.Space = 0;												/* 文字间距，单位 = 像素 */
		LCD_InitMenu(&g_tMenu1, (char **)g_Menu1_Text); /* 初始化菜单结构 */
	}
	LCD_DispMenu(&g_tMenu1);

	fRefresh = 1;
	while (g_MainStatus == MS_SYSTEM_SET)
	{
		bsp_Idle();

		if (fRefresh) /* 刷新整个界面 */
		{
			fRefresh = 0;

			if (g_tMenu1.Cursor == 0)
			{
				;
			}
		}

		ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
			case KEY_UP_S: /* S键 上 */
				if (ucFirstKey == 1)
				{
					ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
					break;
				}
				LCD_MoveUpMenu(&g_tMenu1);
				break;

			case KEY_LONG_S: /* S键 上 */
				BEEP_KeyTone();
				s_enter_sub_menu = 1;

				if (g_tMenu1.Cursor == 0)
				{
					g_MainStatus = MS_HARD_INFO;
				}
				else if (g_tMenu1.Cursor == 1)
				{
					g_MainStatus = MS_USB_UART1;
				}
				else if (g_tMenu1.Cursor == 2)
				{
					g_MainStatus = MS_ESP32_TEST;
				}
				break;

			case KEY_UP_C: /* C键 下 */
				if (ucFirstKey == 1)
				{
					ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
					break;
				}
				LCD_MoveDownMenu(&g_tMenu1);
				break;

			case KEY_LONG_C: /* C键长按 */
				BEEP_KeyTone();
				s_enter_sub_menu = 0;
				g_MainStatus = MS_LINK_MODE;
				break;

			default:
				break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: status_HardInfo
*	功能说明: 硬件信息测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
void status_HardInfo(void)
{
	uint8_t ucKeyCode; /* 按键代码 */
	FONT_T tFont;			 /* 定义字体结构体变量 */
	uint16_t x, y;
	uint16_t usLineCap = 18;
	char buf[128];

	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;				 /* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;			 /* 字体颜色 */
		tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
		tFont.Space = 0;									 /* 文字间距，单位 = 像素 */
	}

	LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

	x = 5;
	y = 3;
	LCD_DispStr(x, y, "H7-TOOL硬件信息", &tFont); /* 在(8,3)坐标处显示一串汉字 */
	y += usLineCap;

	/* 检测CPU ID */
	{
		uint32_t id[3];

		bsp_GetCpuID(id);

		sprintf(buf, "CPU : STM32H750IBK6  %dMHz", SystemCoreClock / 1000000);
		LCD_DispStr(x, y, buf, &tFont);
		y += usLineCap;

		sprintf(buf, "  %08X %08X %08X", id[0], id[1], id[2]);
		LCD_DispStr(x, y, buf, &tFont);
		y += usLineCap;
	}

	/* 显示TFT控制器型号和屏幕分辨率 */
	{
		LCD_DispStr(x, y, "LCD : ST7789 / 240x240", &tFont);
		y += usLineCap;
	}

	/* 测试eMMC */
	{
		BSP_MMC_CardInfo CardInfo;

		BSP_MMC_Init();
		BSP_MMC_GetCardInfo(&CardInfo);

		/* CardInfo.LogBlockSize = 512 */
		if (CardInfo.LogBlockSize == 512)
		{
			uint32_t kb;

			kb = CardInfo.LogBlockNbr / 2;
			sprintf(buf, "eMMC : %d.%02dGB Ok", kb / (1024 * 1024), (kb % (1024 * 1024)) / 10000);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "eMMC : Error, BlockSize %d", CardInfo.LogBlockSize);
			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	}

	/* 测试I2C设备 */
	{
		if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
		{
			sprintf(buf, "EEPROM : 24C16 Ok (0x%02X)", EE_DEV_ADDR);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "EEPROM : 24C16 Err (0x%02X)", EE_DEV_ADDR);
			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;

		if (i2c_CheckDevice(MCP4018_SLAVE_ADDRESS) == 0)
		{
			sprintf(buf, "POT : MCP4018 Ok (0x%02X)", MCP4018_SLAVE_ADDRESS);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "POT : MCP4018 Err (0x%02X)", MCP4018_SLAVE_ADDRESS);
			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	}

	/* QSPI检测 */
	{
		uint32_t id;
		char name[32];

		bsp_InitQSPI_W25Q256();

		id = QSPI_ReadID();
		if (id == W25Q64_ID)
		{
			strcpy(name, "W25Q64");
		}
		else if (id == W25Q128_ID)
		{
			strcpy(name, "W25Q128");
		}
		else if (id == W25Q256_ID)
		{
			strcpy(name, "W25Q256");
		}
		else
		{
			strcpy(name, "UNKNOW");
		}

		/* 检测串行Flash OK */
		if (id == W25Q256_ID)
		{
			sprintf(buf, "QSPI : W25Q256 Ok, %08X", id);
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "QSPI : W25Q256 Err, %08X", id);
			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	}

	/* 以太网MAC */
	{
		uint32_t PhyID[2];

		/*
			PhyID[0].15:0  = OUI:3:18
			PhyID[1].15:10 = OUI.24:19
		
			PhyID[1].9:4 = Model Number
			PhyID[1].3:0 = Revision Number
		*/
		ETH_PHY_IO_ReadReg(0, 2, &PhyID[0]);
		ETH_PHY_IO_ReadReg(0, 3, &PhyID[1]);

		if (((PhyID[1] >> 4) & 0x2F) == 0x0F)
		{
			sprintf(buf, "Eth Phy : LAN8720 Ok");
			LCD_DispStr(x, y, buf, &tFont);
		}
		else
		{
			sprintf(buf, "Eth Phy : LAN8720 Error");
			tFont.FrontColor = CL_RED;
			LCD_DispStr(x, y, buf, &tFont);
			tFont.FrontColor = CL_WHITE;
		}
		y += usLineCap;
	}

	/* WiFi MAC */
	{
		//uint16_t mac[6];

		//ESP32_GetMac(&mac);
	}

	bsp_StartAutoTimer(0, 1000);
	while (g_MainStatus == MS_HARD_INFO)
	{
		bsp_Idle();

		/* 显示时钟 */
		if (bsp_CheckTimer(0))
		{
			uint16_t x, y;

			tFont.FontCode = FC_ST_16;	 /* 字体代码 16点阵 */
			tFont.FrontColor = CL_WHITE; /* 字体颜色 */
			tFont.BackColor = CL_BLUE;	 /* 文字背景颜色 */
			tFont.Space = 0;						 /* 文字间距，单位 = 像素 */

			RTC_ReadClock(); /* 读时钟，结果在 g_tRTC */

			x = 5;
			y = LCD_GetHeight() - 20;

			sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d",
							g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
			LCD_DispStr(x, y, buf, &tFont);
		}

		ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
			case KEY_UP_C: /* C键 下 */
				break;

			case KEY_LONG_C: /* C键长按 */
				BEEP_KeyTone();
				g_MainStatus = MS_SYSTEM_SET;
				break;

			default:
				break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: status_DacTest
*	功能说明: 测试DAC输出波形. 废弃。参数过多，需要联机使用DAC
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if 0
void status_DacTest(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t fRefresh;
	FONT_T tFont;		/* 定义字体结构体变量 */
	uint8_t ucFirstKey = 1;
	uint8_t ucWaveIdx = 0;
	int16_t volt_min = -10000;
	int16_t volt_max = 10000;
	uint32_t freq = 1000;
	uint16_t duty = 50;

	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		tFont.FrontColor = CL_BLACK;	/* 字体颜色 */
		tFont.BackColor = FORM_BACK_COLOR;	/* 文字背景颜色 */
		tFont.Space = 0;				/* 文字间距，单位 = 像素 */

		LCD_ClrScr(FORM_BACK_COLOR);  	/* 清屏，背景蓝色 */

		LCD_DispStr(5, 3, "DAC输出正弦波", &tFont);
	}
	
	fRefresh = 1;
	while (g_MainStatus == MS_DAC_TEST)
	{
		bsp_Idle();

		if (fRefresh == 1)	/* 刷新整个界面 */
		{
			fRefresh = 0;
			
			g_tDacWave.VoltRange = 1;
			g_tDacWave.CycleSetting = 0;			
			g_tDacWave.VoltMin = volt_min;
			g_tDacWave.VoltMax = volt_max;
			g_tDacWave.Freq = freq;
			g_tDacWave.Duty = duty;
			
			if (ucWaveIdx == 0)
			{
				LCD_DispStr(5, 3, "DAC输出正弦波", &tFont);
				
				g_tDacWave.Type = DAC_WAVE_SIN;					
				dac1_StartDacWave();
			}
			else if (ucWaveIdx == 1)
			{
				LCD_DispStr(5, 3, "DAC输出方波", &tFont);
				g_tDacWave.Type = DAC_WAVE_SQUARE;			
				dac1_StartDacWave();				
			}
			else if (ucWaveIdx == 2)
			{
				LCD_DispStr(5, 3, "DAC输出三角波", &tFont);
				g_tDacWave.Type = DAC_WAVE_TRI;
				dac1_StartDacWave();				
			}
		}

		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{	
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case  KEY_UP_S:		/* S键 上 */
					if (ucFirstKey == 1)
					{
						ucFirstKey = 0;	/* 丢弃第1个按键弹起事件 */
						break;
					}					
					BEEP_KeyTone();
					if (++ucWaveIdx > 2)
					{
						ucWaveIdx = 0;
					}
					fRefresh = 1;					
					break;

				case  KEY_LONG_S:		/* S键 上 */
					break;				

				case  KEY_UP_C:			/* C键 下 */
					if (ucFirstKey == 1)
					{
						ucFirstKey = 0;	/* 丢弃第1个按键弹起事件 */
						break;
					}
					break;

				case  KEY_LONG_C:		/* C键长按 */
					BEEP_KeyTone();	
					g_MainStatus = MS_SYSTEM_SET;
					break;					
				
				default:
					break;
			}
		}
	}
	
	/* 停止DAC波形 */
	dac1_StopWave();
}
#endif

/*
*********************************************************************************************************
*	函 数 名: status_UsbUart1
*	功能说明: USB虚拟串口，映射到硬件串口
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_UsbUart1(void)
{
	uint8_t ucKeyCode; /* 按键代码 */
	FONT_T tFont;			 /* 定义字体结构体变量 */

	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;				 /* 字体代码 16点阵 */
		tFont.FrontColor = CL_BLACK;			 /* 字体颜色 */
		tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
		tFont.Space = 0;									 /* 文字间距，单位 = 像素 */

		LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

		LCD_DispStr(5, 3, "USB虚拟串口", &tFont);
	}

	usbd_CloseCDC();
	usbd_OpenCDC(1); /* 映射到串口1 */

	while (g_MainStatus == MS_USB_UART1)
	{
		bsp_Idle();

		ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
			case KEY_UP_S: /* S键 弹起 */
				break;

			case KEY_UP_C: /* C键 下 */
				break;

			case KEY_LONG_S: /* S键 上 */
				break;

			case KEY_LONG_C: /* C键长按 */
				BEEP_KeyTone();
				g_MainStatus = MS_SYSTEM_SET;
				break;

			default:
				break;
			}
		}
	}

	usbd_CloseCDC();
	usbd_OpenCDC(8); /* 映射到串口8. 和PC软件联机 */
}

/*
*********************************************************************************************************
*	函 数 名: status_ESP32Test
*	功能说明: esp32测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void status_ESP32Test(void)
{
	uint8_t ucKeyCode; /* 按键代码 */
	uint8_t fRefresh;
	FONT_T tFont; /* 定义字体结构体变量 */
	uint8_t ucFirstKey = 1;
	uint8_t isp_flag = 0;

	/* 设置字体参数 */
	{
		tFont.FontCode = FC_ST_16;				 /* 字体代码 16点阵 */
		tFont.FrontColor = CL_WHITE;			 /* 字体颜色 */
		tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
		tFont.Space = 0;									 /* 文字间距，单位 = 像素 */

		LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

		LCD_DispStr(5, 3, "ESP32模块测试", &tFont);
	}

	usbd_CloseCDC();
	usbd_OpenCDC(4);

	//bsp_InitESP32();

	ESP32_EnterISP();
	isp_flag = 1;

	wifi_state = WIFI_STOP;

	fRefresh = 1;
	while (g_MainStatus == MS_ESP32_TEST)
	{
		bsp_Idle();

		if (fRefresh == 1) /* 刷新整个界面 */
		{
			fRefresh = 0;

			if (isp_flag == 0)
			{
				LCD_DispStr(5, 60, "当前模式: AT", &tFont);
			}
			else
			{
				LCD_DispStr(5, 60, "当前模式: ISP", &tFont);
			}
		}

		ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
			case KEY_UP_S: /* S键 弹起 */
				if (ucFirstKey == 1)
				{
					ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
					break;
				}
				BEEP_KeyTone();

				if (isp_flag == 0)
				{
					isp_flag = 1;

					ESP32_EnterISP();
				}
				else
				{
					isp_flag = 0;

					ESP32_EnterAT();
				}
				fRefresh = 1;
				break;

			case KEY_UP_C: /* C键 下 */
				if (ucFirstKey == 1)
				{
					ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
					break;
				}

				fRefresh = 1;
				break;

			case KEY_LONG_S: /* S键 上 */
				break;

			case KEY_LONG_C: /* C键长按 */
				BEEP_KeyTone();
				g_MainStatus = MS_SYSTEM_SET;
				break;

			default:
				break;
			}
		}
	}
	usbd_CloseCDC();
	usbd_OpenCDC(8); /* 启用USB虚拟串口 */
}

#if 0	
		bsp_InitExtIO();		/* 输出端口初始化 */

		EIO_ConfigPort(EIO_D0, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D1, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D2, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D3, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D4, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D5, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D6, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D7, ES_FMC_OUT);
		EIO_ConfigPort(EIO_D8, ES_FMC_NOE);
		EIO_ConfigPort(EIO_D9, ES_FMC_NWE);

		
		
		//bsp_InitTimDMA1();
		
		{
			__IO uint16_t *p = (uint16_t *)0x60000000;
			while (1)
			{
				*p = 0xFF;
				*p = 0x00;
			}
		}

		DISABLE_INT();
		while (1)
		{
			__IO uint16_t *pPort = (uint16_t *)0x60000000;
			uint16_t *pMem = (uint16_t *)0x20000000;	// 0x38000000;0x30040000
			uint32_t i;
			
			for (i = 0; i < 16 * 1024 / 2; i++)	 //	18.18M
			{
				*pMem++ = *pPort;
			}		
		
			// 18.18M, 一样的没变化
//			do
//			{
//				*pMem++ = *pPort;
//			}while((uint32_t)pMem < 0x20000000 + 128 * 1024);
				
		}
//#else
//		EIO_ConfigPort(EIO_D0, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D1, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D2, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D3, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D4, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D5, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D6, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D7, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D8, ES_GPIO_OUT);
//		EIO_ConfigPort(EIO_D9, ES_GPIO_OUT);
//#endif

#if 0
	/* LwIP 初始化 */
	{
		/* 如果不插网线，此函数执行时间过长 */
		/* 网络参数存在在全局变量 g_tParam.lwip_ip, g_tParam.lwip_net_mask, g_tParam.lwip_gateway */
		lwip_start();
		
		while (1)
		{
			lwip_pro();
		}
	}
#endif		
//		usbd_OpenCDC();		/* 启用USB虚拟串口 */

//		bsp_DelayMS(1000);

//		/* 测试ESP32 */
//		bsp_InitESP32();
//		//comClearRxFifo(COM_ESP32);	/* 等待发送缓冲区为空，应答结束*/	
//		ESP32_PowerOn();
		
		bsp_SetDAC1(0);
		bsp_SetTVCC(47);
		bsp_StartAutoTimer(0, 200);
		
		DSO_InitHard();
		DSO_SetDC(1,1);
		DSO_SetDC(2,1);
		DSO_SetGain(1, 3);
		DSO_SetGain(2, 3);	
			
		lwip_start();
		
		while(1)
		{
			bsp_Idle();
			
			if (bsp_CheckTimer(0))
			{
				bsp_LedToggle(1);
				
				DSO_StartADC(200000);
				
				if (level == 0)
				{
					level = 1;
					//EIO_SetOutLevel(EIO_D0, 1);
					EIO_SetOutLevel(EIO_D1, 1);
					EIO_SetOutLevel(EIO_D2, 1);
					EIO_SetOutLevel(EIO_D3, 1);
					EIO_SetOutLevel(EIO_D4, 1);
					EIO_SetOutLevel(EIO_D5, 1);
					EIO_SetOutLevel(EIO_D6, 1);
					EIO_SetOutLevel(EIO_D7, 1);
					EIO_SetOutLevel(EIO_D8, 1);
					EIO_SetOutLevel(EIO_D9, 1);
				}
				else
				{
					level = 0;
					//EIO_SetOutLevel(EIO_D0, 0);
					EIO_SetOutLevel(EIO_D1, 0);
					EIO_SetOutLevel(EIO_D2, 0);
					EIO_SetOutLevel(EIO_D3, 0);
					EIO_SetOutLevel(EIO_D4, 0);
					EIO_SetOutLevel(EIO_D5, 0);
					EIO_SetOutLevel(EIO_D6, 0);
					EIO_SetOutLevel(EIO_D7, 0);
					EIO_SetOutLevel(EIO_D8, 0);
					EIO_SetOutLevel(EIO_D9, 0);
				}
			}
			
			ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
			if (ucKeyCode != KEY_NONE)
			{
				/* 有键按下 */
				switch (ucKeyCode)
				{
					case KEY_DOWN_S:		/* S键 */
						BEEP_KeyTone();
					
//						ESP32_EnterISP();
					
						LCD_ClrScr(CL_BLUE);
						if (++idx > 4)
							idx = 0;
					     
						LCD_DrawIcon32(&s_tMainIcons[idx], &tIconFont, 0);

						if (++tvcc > 127)
						{
							tvcc = 0;
						}		
						bsp_SetTVCC(tvcc);
						
						{
							char buf[32];
							
							sprintf(buf, "%3d", tvcc);
							LCD_DispStr(0, 110, buf, &tIconFont);
						}
						
						if (++gain > 3)
						{
							gain = 0;
						}
						DSO_SetGain(0, gain);
						DSO_SetGain(1, gain);						
						break;

					case KEY_DOWN_C:		/* C键 */			
						BEEP_KeyTone();
//					ESP32_ExitISP();
						if (bsp_GetVoltOutRange() == 0)
						{
							bsp_SetVoltOutRange(1);
						}
						else
						{
							bsp_SetVoltOutRange(0);
						}
						if (tvcc > 0)
						{
							tvcc--;
						}
						bsp_SetTVCC(tvcc);
						{
							char buf[32];
							
							sprintf(buf, "%3d", tvcc);
							LCD_DispStr(0, 110, buf, &tIconFont);
						}						
						break;
					
					default:
						break;
				}
			}
		}
	}

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
