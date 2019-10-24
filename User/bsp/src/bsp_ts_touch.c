/*
*********************************************************************************************************
*
*	模块名称 : 电阻式触摸板驱动模块
*	文件名称 : bsp_touch.c
*	版    本 : V1.8
*	说    明 : 驱动i2c接口的触摸芯片。 
*	修改记录 :
*		版本号  日期        作者    说明
*       v1.0    2012-07-06 armfly  ST固件库V3.5.0版本。
*		v1.1    2012-10-22 armfly  增加4点校准
*		v1.2    2012-11-07 armfly  解决4点校准的XY交换分支的bug
*		v1.3    2012-12-17 armfly  触摸校准函数增加入口参数:等待时间
*		V1.4    2013-07-26 armfly  更改 TOUCH_DataFilter() 滤波算法
*		V1.5    2013-07-32 armfly  修改TOUCH_WaitRelease(),计数器需要清零
*		V1.6    2014-10-20 armfly
*					(1) 修改 TOUCH_PutKey() 函数，实现触摸屏的横屏和竖屏动态切换.
*					(2) param 结构增加校准时当前的屏幕方向变量 TouchDirection
*					(3) 调试3.5寸的触摸芯片。修改SPI相关配置函数。
*					(4) 由于触摸芯片TSC2046和串行FLASH,NRF24L01,MP3等模块共享SPI总线。因此需要
*						在触摸中断函数中判断总线冲突. 增加函数 bsp_SpiBusBusy() 判忙.
*					(5) TSC2046增加软件模拟SPI (软件模拟方式方便SPI设备共享)
*		V1.7    2015-01-02 armfly  计划将触摸扫描由1ms一次修改为10ms一次。未定。
*				2015-04-21 armfly 修改 TOUCH_InitHard() 函数。GT811_InitHard() 执行后直接return
*		V1.8	2015-10-30 armfly 增加 4.3寸电容触摸 FT5x06
*					(1) 添加 void TOUCH_CapScan(void) 函数
*		V2.0	2018-01-6 TOUCH_PutKey() 增加GT911，GT811，FT5X06支持
*		V2.1	2018-09-08 armfly 移植到STM32H7平台。
*					- 电阻触摸参数单独存放到eeprom固定地址， 16K字节最后256字节空间. 和APP的param.c独立
*					- 同时支持2点校准和4点校准，不采用条件编译了。
*					- void TOUCH_Calibration(uint8_t _PointCount); 增加形参
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* 调试打印语句 */
//#define touch_printf       printf
#define touch_printf(...) 

/* 定义触笔中断INT的GPIO端口 */
#define TP_INT_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOH_CLK_ENABLE()
#define TP_INT_GPIO_PORT              GPIOH
#define TP_INT_PIN                    GPIO_PIN_7

/* 每1ms扫描一次坐标 */
#define DOWN_VALID		30	/* 按下30ms 后, 开始统计ADC */
#define SAMPLE_COUNT	20	/* 按下后 20ms处理一次坐标 */

/*
	触摸屏校准点相对屏幕像素四角的偏移像素
	第1个点 ： x1 = CALIB_OFFSET, y1 = CALIB_OFFSET
	第2个点 ： x2 = LCD_GetWidth() - CALIB_OFFSET, y2 = LCD_GetHeight() - CALIB_OFFSET
*/
#define CALIB_OFFSET	20
#define TP_X1	CALIB_OFFSET
#define TP_Y1	CALIB_OFFSET

#define TP_X2	(LCD_GetWidth() - CALIB_OFFSET)
#define TP_Y2	(LCD_GetHeight() - CALIB_OFFSET)

#define TP_X3	CALIB_OFFSET
#define TP_Y3	(LCD_GetHeight() - CALIB_OFFSET)

#define TP_X4	(LCD_GetWidth() - CALIB_OFFSET)
#define TP_Y4	CALIB_OFFSET

/* 有效ADC值的判断门限. 太接近ADC临界值的坐标认为无效 */
#define ADC_VALID_OFFSET	2

/* 校准参数存放在EEPROM(AT24C128容量16KB) 最后64字节区域 */
#define TP_PARAM_EE_ADDR		(16*1024 - 64)

//#define TP_PARAM_FLASH_ADDR		ADDR_FLASH_SECTOR_3		/* 0x0800C000 中间的16KB扇区用来存放参数 */
//#define TP_PARAM_FLASH_ADDR		ADDR_FLASH_SECTOR_11	/* 0x080E0000 Flash最后128K扇区用来存放参数 */

/* 触屏模块用到的全局变量 */
TOUCH_T g_tTP;

uint8_t g_TouchType;
uint8_t g_LcdType;

TP_CALIB_PARAM_T g_tTPParam;

static uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY);
static uint16_t TOUCH_DataFilter(uint16_t *_pBuf, uint8_t _ucCount);
static void TOUCH_LoadParam(void);
static void TOUCH_SaveParam(void);
static int32_t CalTwoPoint(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x);
static int16_t TOUCH_TransX(uint16_t _usAdcX, uint16_t _usAdcY);
static int16_t TOUCH_TransY(uint16_t _usAdcX, uint16_t _usAdcY);
int32_t TOUCH_Abs(int32_t x);

/*
*********************************************************************************************************
*	函 数 名: bsp_DetectLcdType
*	功能说明: 通过I2C触摸芯片，识别LCD模组类型。结果存放在全局变量 g_LcdType 和 g_TouchType
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DetectLcdType(void)
{
	uint8_t i;
	
	g_TouchType = 0xFF;
	g_LcdType = 0xFF;
	
	bsp_DelayUS(5000);
	
	touch_printf("正在识别触摸屏型号\r\n");
	
	/* 50ms，等待GT811复位就绪，才能探测GT811芯片 ID */
	for (i = 0; i < 5; i++)
	{		
		/*
			GT811电容触摸板和GT911的I2C地址相同
			一般就 0x28 、 0xBA 两种。
			通过读取触摸IC的芯片ID来识别。
		*/
		if (i2c_CheckDevice(0x28) == 0)
		{
			uint32_t id;
	
			bsp_DelayUS(500);
			g_GT911.i2c_addr = 0x28;
			id = GT911_ReadID();			
			if (id == 0x00313139)
			{			
				g_GT911.i2c_addr = 0x28;
				g_TouchType = CT_GT911;
				g_LcdType = LCD_70_800X480;
				touch_printf("检测到7.0寸电容触摸屏GT911(0x28) 800x480\r\n");
			}
			else 	/* GT811 */
			{
				g_GT811.i2c_addr = 0x28;
				g_TouchType = CT_GT811;
				g_LcdType = LCD_70_800X480;		
				touch_printf("检测到7.0寸电容触摸屏GT811(0x28) 800x480\r\n");
			}
			break;
		}

		if (i2c_CheckDevice(0xBA) == 0)
		{
			uint32_t id;
	
			bsp_DelayUS(500);
			g_GT911.i2c_addr = 0xBA;
			id = GT911_ReadID();			
			if (id == 0x00313139)
			{			
				g_GT911.i2c_addr = 0xBA;
				g_TouchType = CT_GT911;
				g_LcdType = LCD_70_800X480;
				touch_printf("检测到7.0寸电容触摸屏GT911(0xBA) 800x480\r\n");
			}
			else 	/* GT811 */
			{
				g_GT811.i2c_addr = 0xBA;
				g_TouchType = CT_GT811;
				g_LcdType = LCD_70_800X480;		
				touch_printf("检测到7.0寸电容触摸屏GT811(0xBA) 800x480\r\n");
			}
			break;
		}		
		
		/* FT系列电容触摸触摸 : 4.3寸id = 0x55    5.0寸id = 0x0A  7.0寸id = 0x06 */
		if (i2c_CheckDevice(FT5X06_I2C_ADDR) == 0)
		{
			uint8_t id;
			
			bsp_DelayUS(50000);	/* 延迟50ms */
			id = FT5X06_ReadID();			
			if (id == 0x55)
			{
				g_TouchType = CT_FT5X06;
				g_LcdType = LCD_43_480X272;		
				touch_printf("检测到4.3寸电容触摸屏\r\n");
			}
			else if (id == 0x0A)
			{
				g_TouchType = CT_FT5X06;
				g_LcdType = LCD_50_800X480;		
				touch_printf("检测到5.0寸电容触摸屏\r\n");				
			}
			else	/* id == 0x06 表示7寸电容屏（FT芯片） */
			{
				g_TouchType = CT_FT5X06;
				g_LcdType = LCD_70_800X480;		
				touch_printf("检测到7.0寸电容触摸屏FT\r\n");					
			}
			break;
		}

		/* 电阻触摸板 */		
		if (i2c_CheckDevice(STMPE811_I2C_ADDRESS) == 0)
		{
			/*			
				0  = 4.3寸屏（480X272）
				1  = 5.0寸屏（480X272）
				2  = 5.0寸屏（800X480）
				3  = 7.0寸屏（800X480）
				4  = 7.0寸屏（1024X600）
				5  = 3.5寸屏（480X320）			
			*/					
			uint8_t id;			
			
			g_TouchType = CT_STMPE811;	/* 触摸类型 */
			
			STMPE811_InitHard();	/* 必须先配置才能读取ID */
			
			id = STMPE811_ReadIO();	/* 识别LCD硬件类型 */

			touch_printf("检测到电阻触摸屏, id = %d\r\n", id);
			switch (id)
			{
				case 0:
					g_LcdType = LCD_43_480X272;
					break;

				case 1:
					g_LcdType = LCD_50_480X272;
					break;

				case 2:
					g_LcdType = LCD_50_800X480;
					break;

				case 3:
					g_LcdType = LCD_70_800X480;
					break;

				case 4:
					g_LcdType = LCD_70_1024X600;
					break;

				case 5:
					g_LcdType = LCD_35_480X320;
					break;			
				
				default:
					g_LcdType = LCD_35_480X320;
					break;
			}			
			break;			
		}		
		
		bsp_DelayMS(10);
	}
	
	if (i == 5)
	{
		touch_printf("未识别出显示模块\r\n");
	}
}
	
/*
*********************************************************************************************************
*	函 数 名: TOUCH_InitHard
*	功能说明: 初始化触摸芯片。 再之前，必须先执行 bsp_DetectLcdType() 识别触摸出触摸芯片型号.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_InitHard(void)
{	
	/* 配置TP_INT触摸中断引脚 */
	{
		GPIO_InitTypeDef gpio_init;

		/* 第1步：打开GPIO时钟 */
		TP_INT_GPIO_CLK_ENABLE();
		
		/* 第2步：配置所有的按键GPIO为浮动输入模式 */
		gpio_init.Mode = GPIO_MODE_INPUT;   		/* 设置输入 */
		gpio_init.Pull = GPIO_NOPULL;                 /* 上下拉电阻不使能 */
		gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  /* GPIO速度等级 */
		gpio_init.Pin = TP_INT_PIN;
		HAL_GPIO_Init(TP_INT_GPIO_PORT, &gpio_init);	
	}

	bsp_DetectLcdType();	/* 自动识别触摸芯片型号 */
	
    g_tTP.Enable = 0;
	
	switch (g_TouchType)
	{
		case CT_GT811:			/* 电容触摸 7寸 */
			GT811_InitHard();
			break;

		case CT_GT911:			/* 电容触摸 7寸 */
			GT911_InitHard();
			break;		
		
		case CT_FT5X06:			/* 电容触摸 4.3寸 */
			FT5X06_InitHard();
			break;
		
		case CT_STMPE811:		/* 电阻的 */
			//STMPE811_InitHard();   < bsp_DetectLcdType() 内部已经执行初始化 
			g_tTP.usMaxAdc = 4095;	/* 12位ADC */	
		
			TOUCH_LoadParam();	/* 读取校准参数 */
			g_tTP.Write = g_tTP.Read = 0;
			g_tTP.Enable = 1;
			break;
		
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_PenInt
*	功能说明: 判断触摸按下
*	形    参: 无
*	返 回 值: 0表示无触笔按下，1表示有触笔按下
*********************************************************************************************************
*/
uint8_t TOUCH_PenInt(void)
{
	if ((TP_INT_GPIO_PORT->IDR & TP_INT_PIN) == 0)
	{
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_ReadAdcX
*	功能说明: 获得触摸板X方向ADC采样值， 已进行滤波处理
*	形    参:  无
*	返 回 值: X 方向ADC值
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcX(void)
{
	uint16_t usAdc;

	__set_PRIMASK(1);  		/* 关中断 */
	usAdc = g_tTP.usAdcNowX;
	__set_PRIMASK(0);  		/* 开中断 */

	return usAdc;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_ReadAdcY
*	功能说明: 获得触摸板Y方向ADC采样值， 已进行滤波处理
*	形    参:  无
*	返 回 值: Y 坐标值，允许负值
*********************************************************************************************************
*/
uint16_t TOUCH_ReadAdcY(void)
{
	uint16_t usAdc;

	__set_PRIMASK(1);  		/* 关中断 */
	usAdc = g_tTP.usAdcNowY;
	__set_PRIMASK(0);  		/* 开中断 */

	return usAdc;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_PutKey
*	功能说明: 将1个触摸点坐标值压入触摸FIFO缓冲区。电阻触摸屏形参是ADC值，电容触摸屏形参是坐标值
*	形    参: _usX, _usY 坐标值
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_PutKey(uint8_t _ucEvent, uint16_t _usX, uint16_t _usY)
{
	uint16_t xx, yy;
	uint16_t x = 0, y = 0;

	g_tTP.Event[g_tTP.Write] = _ucEvent;

	if (g_tTP.Enable == 1)	/* 电阻屏。 形参是ADC值 */
	{
		xx = TOUCH_TransX(_usX, _usY);
		yy = TOUCH_TransY(_usX, _usY);
	}
	else	/* GT811，FTX06，GT911 电容触摸走此分之 */
	{
		/* 无需转换， 直接是坐标值 */
		xx = _usX;
		yy = _usY;		
	}
	
	/* 横屏和竖屏方向识别 */
	switch (g_tTPParam.TouchDirection)
	{
		case 0:	/* 校准触摸时，屏幕方向为0 */
			if (g_LcdDirection == 0)		/* 横屏 */
			{
				x = xx;
				y = yy;
			}
			else if (g_LcdDirection == 1)	/* 横屏180°*/
			{
				x = g_LcdWidth - xx - 1;
				y = g_LcdHeight - yy - 1;
			}
			else if (g_LcdDirection == 2)	/* 竖屏 */
			{
				y = xx;
				x = g_LcdWidth - yy - 1;
			}
			else if (g_LcdDirection == 3)	/* 竖屏180° */
			{
				y = g_LcdHeight - xx - 1;
				x = yy;
			}
			break;

		case 1:	/* 校准触摸时，屏幕方向为1 */
			if (g_LcdDirection == 0)		/* 横屏 */
			{
				x = g_LcdWidth - xx - 1;
				y = g_LcdHeight - yy - 1;
			}
			else if (g_LcdDirection == 1)	/* 横屏180°*/
			{
				x = xx;
				y = yy;
			}
			else if (g_LcdDirection == 2)	/* 竖屏 */
			{
				y = g_LcdHeight - xx - 1;
				x = yy;
			}
			else if (g_LcdDirection == 3)	/* 竖屏180° */
			{
				y = xx;
				x = g_LcdWidth - yy - 1;
			}
			break;

		case 2:	/* 校准触摸时，屏幕方向为2 */
			if (g_LcdDirection == 0)		/* 横屏 */
			{
				y = xx;
				x = g_LcdWidth - yy - 1;
			}
			else if (g_LcdDirection == 1)	/* 横屏180°*/
			{
				y = g_LcdHeight - xx - 1;
				x = yy;
			}
			else if (g_LcdDirection == 2)	/* 竖屏 */
			{
				x = xx;
				y = yy;
			}
			else if (g_LcdDirection == 3)	/* 竖屏180° */
			{
				x = g_LcdWidth - xx - 1;
				y = g_LcdHeight - yy - 1;
			}
			break;

		case 3:	/* 校准触摸时，屏幕方向为3 */
			if (g_LcdDirection == 0)		/* 横屏 */
			{
				y = xx;
				x = g_LcdWidth - yy - 1;
			}
			else if (g_LcdDirection == 1)	/* 横屏180°*/
			{
				y = g_LcdHeight - xx - 1;
				x = yy;
			}
			else if (g_LcdDirection == 2)	/* 竖屏 */
			{
				x = g_LcdWidth - xx - 1;
				y = g_LcdHeight - yy - 1;
			}
			else if (g_LcdDirection == 3)	/* 竖屏180° */
			{
				x = xx;
				y = yy;
			}
			break;

		default:
			g_tTPParam.TouchDirection = 0;	/* 方向参数无效时，纠正为缺省的横屏 */
			break;
	}

	g_tTP.XBuf[g_tTP.Write] = x;
	g_tTP.YBuf[g_tTP.Write] = y;

	if (++g_tTP.Write  >= TOUCH_FIFO_SIZE)
	{
		g_tTP.Write = 0;
	}
	
	/* 调试语句，打印adc和坐标 */
	touch_printf("%d - (%d, %d) adcX=%d,adcY=%d\r\n", _ucEvent, x, y, g_tTP.usAdcNowX, g_tTP.usAdcNowY);
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_GetKey
*	功能说明: 从触摸FIFO缓冲区读取一个坐标值。
*	形    参: 无
*	返 回 值:
*			TOUCH_NONE      表示无事件
*			TOUCH_DOWN      按下
*			TOUCH_MOVE      移动
*			TOUCH_RELEASE	释放
*********************************************************************************************************
*/
uint8_t TOUCH_GetKey(int16_t *_pX, int16_t *_pY)
{
	uint8_t ret;

	if (g_tTP.Read == g_tTP.Write)
	{
		return TOUCH_NONE;
	}
	else
	{
		ret = g_tTP.Event[g_tTP.Read];
		*_pX = g_tTP.XBuf[g_tTP.Read];
		*_pY = g_tTP.YBuf[g_tTP.Read];

		if (++g_tTP.Read >= TOUCH_FIFO_SIZE)
		{
			g_tTP.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_CelarFIFO
*	功能说明: 清除触摸FIFO缓冲区
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_CelarFIFO(void)
{
	__set_PRIMASK(1);  		/* 关中断 */
	g_tTP.Write = g_tTP.Read;
	__set_PRIMASK(0);  		/* 开中断 */
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_InRect
*	功能说明: 判断当前坐标是否位于矩形框内
*	形    参:  _usX, _usY: 输入坐标
*			_usRectX,_usRectY: 矩形起点
*			_usRectH、_usRectW : 矩形高度和宽度
*	返 回 值: 1 表示在范围内
*********************************************************************************************************
*/
uint8_t TOUCH_InRect(uint16_t _usX, uint16_t _usY,
	uint16_t _usRectX, uint16_t _usRectY, uint16_t _usRectH, uint16_t _usRectW)
{
	if ((_usX > _usRectX) && (_usX < _usRectX + _usRectW)
		&& (_usY > _usRectY) && (_usY < _usRectY + _usRectH))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_MoveValid
*	功能说明: 判断当前坐标和上次坐标是否偏差太大
*	形    参:  _usX1, _usY1: 坐标1
*			  _usX2, _usY2: 坐标2
*	返 回 值: 1 表示有效点， 0 表示飞点
*********************************************************************************************************
*/
uint8_t TOUCH_MoveValid(uint16_t _usX1, uint16_t _usY1, uint16_t _usX2, uint16_t _usY2)
{
	int16_t iX, iY;
	static uint8_t s_invalid_count = 0;

	iX = TOUCH_Abs(_usX1 - _usX2);
	iY = TOUCH_Abs(_usY1 - _usY2);

	if ((iX < 25) && (iY < 25))
	{
		s_invalid_count = 0;
		return 1;
	}
	else
	{
		if (++s_invalid_count >= 3)
		{
			s_invalid_count = 0;
			return 1;
		}
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_CapScan
*	功能说明: I2C接口电容触摸板扫描函数，放在 bsp_Idle()执行！
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_CapScan(void)
{
	if (g_GT811.Enable == 1)
	{
		GT811_Scan();
		return;
	}
	
	if (g_tFT5X06.Enable == 1)
	{
		FT5X06_Scan();
		return;
	}
	
	if (g_GT911.Enable == 1)
	{
		GT911_Scan();
		return;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_Scan
*	功能说明: 触摸板事件检测程序。该函数被周期性调用，每ms调用1次. 见 bsp_Timer.c
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_Scan(void)
{
	uint16_t usAdcX;
	uint16_t usAdcY;
	static uint16_t s_usXBuf[SAMPLE_COUNT];
	static uint16_t s_usYBuf[SAMPLE_COUNT];
	static uint8_t s_ucPos = 0;
	static uint8_t s_count = 0;
	static uint8_t s_down = 0;
	static uint16_t s_usSaveAdcX, s_usSaveAdcY; /* 用于触笔抬起事件，保存按下和移动的最后采样值 */
	static uint8_t s_ms = 0;

	if (g_GT811.Enable == 1)
	{
		GT811_Timer1ms();	/* 电容触摸屏程序计数器 */
		return;
	}

	if (g_GT911.Enable == 1)
	{
		GT911_Timer1ms();	/* 电容触摸屏程序计数器 */
		return;
	}	
	
	if (g_tFT5X06.Enable == 1)
	{
		FT5X06_Timer1ms();	/* 电容触摸屏程序计数器 */
		return;
	}
	
	/* 下面用于电阻触摸 */
	
	if (g_tTP.Enable == 0)	
	{
		return;
	}
	
	if (++s_ms >= 2)
	{
		return;
	}
	
	/* 2ms进入一次 */
	s_ms = 0;
	
	/* 触笔中断发生 */
	if (TOUCH_PenInt())
	{
		/* 获得原始的ADC值，未滤波 */
		usAdcX = STMPE811_ReadX();
		usAdcY = STMPE811_ReadY();

		if (TOUCH_PressValid(usAdcX, usAdcY))
		{
			/* 按压30ms之后才开始采集数据 */
			if (s_count >= DOWN_VALID / 2)
			{
				s_usXBuf[s_ucPos] = usAdcX;
				s_usYBuf[s_ucPos] = usAdcY;

				/* 采集20ms数据进行滤波 */
				if (++s_ucPos >= SAMPLE_COUNT / 2)
				{
					s_ucPos = 0;

					/* 对ADC采样值进行软件滤波 */
					g_tTP.usAdcNowX = TOUCH_DataFilter(s_usXBuf, SAMPLE_COUNT / 2);
					g_tTP.usAdcNowY = TOUCH_DataFilter(s_usYBuf, SAMPLE_COUNT / 2);

					if (s_down == 0)
					{
						s_down = 1;
						/* 触摸按下事件 */
						TOUCH_PutKey(TOUCH_DOWN, g_tTP.usAdcNowX, g_tTP.usAdcNowY);
						
						s_usSaveAdcX = g_tTP.usAdcNowX;
						s_usSaveAdcY = g_tTP.usAdcNowY;
					}
					else
					{
						if (TOUCH_MoveValid(s_usSaveAdcX, s_usSaveAdcY, g_tTP.usAdcNowX, g_tTP.usAdcNowY))
						{
							/* 触摸移动事件 */
							TOUCH_PutKey(TOUCH_MOVE, g_tTP.usAdcNowX, g_tTP.usAdcNowY);
							
							s_usSaveAdcX = g_tTP.usAdcNowX;
							s_usSaveAdcY = g_tTP.usAdcNowY;
						}
						else
						{
							g_tTP.usAdcNowX = 0; /* for debug stop */
						}
					}
				}
			}
			else
			{
				s_count++;
			}
		}
		else
		{
			if (s_count > 0)
			{
				if (--s_count == 0)
				{
					/* 触摸释放事件 */
					//TOUCH_PutKey(TOUCH_RELEASE, g_tTP.usAdcNowX, g_tTP.usAdcNowY);
					TOUCH_PutKey(TOUCH_RELEASE, s_usSaveAdcX, s_usSaveAdcY);

					g_tTP.usAdcNowX = 0;
					g_tTP.usAdcNowY = 0;

					s_count = 0;
					s_down = 0;
					
					STMPE811_ClearInt();		/* 清触笔中断标志 */
				}
			}
			s_ucPos = 0;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: CalTwoPoint
*	功能说明: 根据2点直线方程，计算Y值
*	形    参:  2个点的坐标和x输入量
*	返 回 值: x对应的y值
*********************************************************************************************************
*/
static int32_t CalTwoPoint(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x)
{
	return y1 + ((int32_t)(y2 - y1) * (x - x1)) / (x2 - x1);
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_TransX
*	功能说明: 将触摸ADC值转换为像素坐标
*	形    参:  无
*	返 回 值: X 坐标值，允许负值
*********************************************************************************************************
*/
static int16_t TOUCH_TransX(uint16_t _usAdcX, uint16_t _usAdcY)
{
	if (g_tTPParam.CalibPointCount == 2)
	{
		uint16_t x;
		int32_t y;

		if (g_tTPParam.XYChange == 0)
		{
			x = _usAdcX;
			if (x == 0)
			{
				y = 0;
			}
			else
			{
				//y = CalTwoPoint(g_tTPParam.usAdcX1, TP_X1, g_tTPParam.usAdcX2, TP_X2, x);
				y = CalTwoPoint(g_tTPParam.usAdcX1, g_tTPParam.usLcdX1, g_tTPParam.usAdcX2, g_tTPParam.usLcdX2, x);
			}
		}
		else
		{
			x = _usAdcY;
			if (x == 0)
			{
				y = 0;
			}
			else
			{
				//y = CalTwoPoint(g_tTPParam.usAdcY1, TP_X1, g_tTPParam.usAdcY2, TP_X2, x);
				y = CalTwoPoint(g_tTPParam.usAdcY1, g_tTPParam.usLcdX1, g_tTPParam.usAdcY2, g_tTPParam.usLcdX2, x);
			}
		}
		return y;
	}
	else	/* 4点校准 */
	{
		uint16_t x, x1, x2;
		int32_t y;

		if (g_tTPParam.XYChange == 0)	/* X Y 坐标不交换 */
		{
			x = _usAdcX;

			/* 根据 Y ADC 实时计算直线方程的参考点x1, x2
				if  _usAdcY = usAdcY1 then  取点 = (AdcX1, TP_X1, AdcX4, TP_X4, _usAdcY)
				if  _usAdcY = usAdcY2 then  取点 = (AdcX3, TP_X3, AdcX2, TP_X2, _usAdcY)

				其中 TP_X1 = TP_X3;  TP_X4 = TP_X1 , 这是程序设定的校准位置的像素坐标, 是固定的。
				我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
			*/
			x1 = CalTwoPoint(g_tTPParam.usAdcY1, g_tTPParam.usAdcX1, g_tTPParam.usAdcY2,  g_tTPParam.usAdcX3, _usAdcY);
			x2 = CalTwoPoint(g_tTPParam.usAdcY1, g_tTPParam.usAdcX4, g_tTPParam.usAdcY2,  g_tTPParam.usAdcX2, _usAdcY);
		}
		else						/* X Y 坐标交换 */
		{
			x = _usAdcY;

			/* 根据 X ADC 实时计算直线方程的参考点x1, x2
				if  _usAdcX = usAdcX1 then  取点 = (AdcY1, TP_X1, AdcY4, TP_X4, _usAdcX)
				if  _usAdcX = usAdcX2 then  取点 = (AdcY3, TP_X3, AdcY2, TP_X2, _usAdcX)

				其中 TP_X1 = TP_X3;  TP_X4 = TP_X1 , 这是程序设定的校准位置的像素坐标, 是固定的。
				我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
			*/
			x1 = CalTwoPoint(g_tTPParam.usAdcX1, g_tTPParam.usAdcY1, g_tTPParam.usAdcX2,  g_tTPParam.usAdcY3, _usAdcX);
			x2 = CalTwoPoint(g_tTPParam.usAdcX1, g_tTPParam.usAdcY4, g_tTPParam.usAdcX2,  g_tTPParam.usAdcY2, _usAdcX);
		}

		if (x == 0)
		{
			y = 0;
		}
		else
		{
			/* 根据2点直线方程，计算坐标 */
			//y = CalTwoPoint(x1, TP_X1, x2, TP_X2, x);
			CalTwoPoint(x1, g_tTPParam.usLcdX1, x2, g_tTPParam.usLcdX2, x);
		}
		return y;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_TransY
*	功能说明: 将触摸ADC值转换为像素坐标
*	形    参:  无
*	返 回 值: Y 坐标值，允许负值
*********************************************************************************************************
*/
static int16_t TOUCH_TransY(uint16_t _usAdcX, uint16_t _usAdcY)
{
	if (g_tTPParam.CalibPointCount == 2)	/* 2点校准 */
	{
		int32_t x;
		int32_t y;

		if (g_tTPParam.XYChange == 0)
		{
			x = _usAdcY;
			if (x == 0)
			{
				y = 0;
			}
			else
			{
				//y = CalTwoPoint(g_tTPParam.usAdcY1, TP_Y1, g_tTPParam.usAdcY2, TP_Y2, x);
				y = CalTwoPoint(g_tTPParam.usAdcY1, g_tTPParam.usLcdY1, g_tTPParam.usAdcY2, g_tTPParam.usLcdY2, x);
			}
		}
		else
		{
			x = _usAdcX;
			if (x == 0)
			{
				y = 0;
			}
			else
			{
				//y = CalTwoPoint(g_tTPParam.usAdcX1, TP_Y1, g_tTPParam.usAdcX2, TP_Y2, x);
				y = CalTwoPoint(g_tTPParam.usAdcX1, g_tTPParam.usAdcY1, g_tTPParam.usAdcX2, g_tTPParam.usLcdY2, x);
			}
		}
		return y;
	}
	else /* 4点校准 */
	{
		int32_t x, x1, x2;
		int32_t y;

		if (g_tTPParam.XYChange == 0)	/* X Y 坐标不交换 */
		{
			x = _usAdcY;

			/* 根据 X ADC 实时计算直线方程的参考点x1, x2
				if  _usAdcX = usAdcX1 then  取点 = (AdcY1, TP_Y1, AdcY3, TP_Y3, _usAdcX)
				if  _usAdcX = usAdcX2 then  取点 = (AdcY4, TP_Y4, AdcY2, TP_Y2, _usAdcX)

				其中 TP_Y1 = TP_Y4;  TP_Y3 = TP_Y2 , 这是程序设定的校准位置的像素坐标, 是固定的。
				我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
			*/
			x1 = CalTwoPoint(g_tTPParam.usAdcX1, g_tTPParam.usAdcY1, g_tTPParam.usAdcX2,  g_tTPParam.usAdcY4, _usAdcX);
			x2 = CalTwoPoint(g_tTPParam.usAdcX1, g_tTPParam.usAdcY3, g_tTPParam.usAdcX2,  g_tTPParam.usAdcY2, _usAdcX);
		}
		else						/* X Y 坐标交换 */
		{
			x = _usAdcX;

			/* 根据 X ADC 实时计算直线方程的参考点x1, x2
				if  _usAdcY = usAdcY1 then  取点 = (AdcX1, TP_Y1, AdcX3, TP_Y3, _usAdcY)
				if  _usAdcY = usAdcY2 then  取点 = (AdcX4, TP_Y4, AdcX2, TP_Y2, _usAdcY)

				其中 TP_Y1 = TP_Y3;  TP_Y4 = TP_Y2 , 这是程序设定的校准位置的像素坐标, 是固定的。
				我们仅需要动态计算对第1个和第3个参数。同样采用2点直线方程计算。
			*/
			x1 = CalTwoPoint(g_tTPParam.usAdcY1, g_tTPParam.usAdcX1, g_tTPParam.usAdcY2,  g_tTPParam.usAdcX4, _usAdcY);
			x2 = CalTwoPoint(g_tTPParam.usAdcY1, g_tTPParam.usAdcX3, g_tTPParam.usAdcY2,  g_tTPParam.usAdcX2, _usAdcY);
		}

		if (x == 0)
		{
			y = 0;
		}
		else
		{
			/* 根据2点直线方程，计算坐标 */
			//y = CalTwoPoint(x1, TP_Y1, x2, TP_Y2, x);
			y = CalTwoPoint(x1, g_tTPParam.usAdcY1, x2, g_tTPParam.usLcdY2, x);
		}
		return y;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_GetX
*	功能说明: 获得当前的的触摸坐标X
*	形    参:  无
*	返 回 值: X 坐标值，允许负值
*********************************************************************************************************
*/
int16_t TOUCH_GetX(void)
{
	return TOUCH_TransX(TOUCH_ReadAdcX(), TOUCH_ReadAdcY());
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_GetY
*	功能说明: 获得当前的的触摸坐标Y
*	形    参:  无
*	返 回 值: Y 坐标值，允许负值
*********************************************************************************************************
*/
int16_t TOUCH_GetY(void)
{
	return TOUCH_TransY(TOUCH_ReadAdcX(), TOUCH_ReadAdcY());
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_DataFilter
*	功能说明: 对采样数据进行滤波
*	形    参:  无
*	返 回 值: X 坐标值，允许负值
*********************************************************************************************************
*/
static uint16_t TOUCH_DataFilter(uint16_t *_pBuf, uint8_t _ucCount)
{
#if 0
	uint8_t i;
	uint32_t uiSum;

	uiSum = 0;
	for (i = 0; i < _ucCount; i++)
	{
		uiSum += _pBuf[i];
	}
	return uiSum / _ucCount;
#else
	uint8_t flag;
	uint8_t i;
	uint16_t usTemp;
	uint32_t uiSum;

	/* 升序排列 */
    do
	{
		flag = 0;
		for (i = 0; i < _ucCount - 1; i++)
		{
			if (_pBuf[i] > _pBuf[i+1])
			{
				usTemp = _pBuf[i + 1];
				_pBuf[i+1] = _pBuf[i];
				_pBuf[i] = usTemp;
				flag = 1;
			}
		}
	}while(flag);

	uiSum = 0;
	for (i = 0; i < _ucCount / 3; i++)
	{
		uiSum += _pBuf[_ucCount / 3 + i];
	}
	usTemp = uiSum / (_ucCount / 3);
	return usTemp;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_DispPoint1
*	功能说明: 显示第1个校准点
*	形    参:  _ucIndex = 0 : 表示第1个点； _ucIndex = 1 表示第2个点;
*	返 回 值: 无
*********************************************************************************************************
*/
static void TOUCH_DispPoint(uint8_t _ucIndex)
{
	FONT_T tFont16;			/* 定义一个字体结构体变量，用于设置字体参数 */

	/* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		tFont16.FrontColor = CL_WHITE;		/* 字体颜色 0 或 1 */
		tFont16.BackColor = CL_BLUE;	/* 文字背景颜色 */
		tFont16.Space = 0;			/* 文字间距，单位 = 像素 */
	}

/*
	第1个点 ： x1 = CALIB_OFFSET, y1 = CALIB_OFFSET
	第2个点 ： x2 = LCD_GetHeight() - CALIB_OFFSET, y2 = LCD_GetWidth - CALIB_OFFSET
*/
	if (_ucIndex == 0)
	{
		LCD_ClrScr(CL_BLUE);  		/* 清屏，背景蓝色 */

		/* 在屏幕边沿绘制2个矩形框(用于检测面板边缘像素是否正常) */
		LCD_DrawRect(0, 0, LCD_GetHeight(), LCD_GetWidth(), CL_WHITE);
		LCD_DrawRect(2, 2, LCD_GetHeight() - 4, LCD_GetWidth() - 4, CL_YELLOW);

		LCD_DispStr(50, 10, "校准触摸屏", &tFont16);		/* 在(8,3)坐标处显示一串汉字 */

		LCD_DrawCircle(TP_X1, TP_Y1, 6, CL_WHITE);
	}
	else if (_ucIndex == 1)
	{
		LCD_DrawCircle(TP_X1, TP_Y1, 6, CL_BLUE);			/* 擦除第1个点 */

		LCD_DrawCircle(TP_X2, TP_Y2, 6, CL_WHITE);
	}
	else if (_ucIndex == 2)
	{
		LCD_DrawCircle(TP_X2, TP_Y2, 6, CL_BLUE);			/* 擦除第2个点 */

		LCD_DrawCircle(TP_X3, TP_Y3, 6, CL_WHITE);
	}
	else
	{
		LCD_DrawCircle(TP_X3, TP_Y3, 6, CL_BLUE);			/* 擦除第3个点 */

		LCD_DrawCircle(TP_X4, TP_Y4, 6, CL_WHITE);
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_PressValid
*	功能说明: 判断按压是否有效，根据X, Y的ADC值进行大致判断
*	形    参:  无
*	返 回 值: 1 表示有效； 0 表示无效
*********************************************************************************************************
*/
static uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY)
{
	if ((_usX <= ADC_VALID_OFFSET) || (_usY <= ADC_VALID_OFFSET)
		|| (_usX >= g_tTP.usMaxAdc - ADC_VALID_OFFSET)
		|| (_usY >= g_tTP.usMaxAdc - ADC_VALID_OFFSET))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_WaitRelease
*	功能说明: 等待触笔释放
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TOUCH_WaitRelease(void)
{
	uint8_t usCount = 0;

	for (;;)
	{
		if (TOUCH_PressValid(TOUCH_ReadAdcX(), TOUCH_ReadAdcY()) == 0)
		{
			if (++usCount > 5)
			{
				break;
			}
		}
		else
		{
			usCount = 0;
		}
		bsp_DelayMS(10);
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_Abs
*	功能说明: 计算绝对值
*	形    参: x : 有符合整数
*	返 回 值: 正整数
*********************************************************************************************************
*/
int32_t TOUCH_Abs(int32_t x)
{
	if (x >= 0)
	{
		return x;
	}
	else
	{
		return -x;
	}
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_Calibration
*	功能说明: 触摸屏校准
*	形    参: _PointCount : 校准点数，2 或 4.
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_Calibration(uint8_t _PointCount)
{
	uint16_t usAdcX;
	uint16_t usAdcY;
	uint8_t usCount;
	uint8_t i;
	uint32_t n;

	/* 校准点数，2点或4点 */
	if (_PointCount == 4)
	{
		g_tTPParam.CalibPointCount = 4;
	}
	else
	{
		g_tTPParam.CalibPointCount = 2;
	}	
	
	TOUCH_CelarFIFO();		/* 清除无效的触摸事件 */

	for (i = 0; i < g_tTPParam.CalibPointCount; i++)
	{
		TOUCH_DispPoint(i);		/* 显示校准点 */

		TOUCH_WaitRelease(); 	/* 等待触笔释放 */

		usCount = 0;
		for (n = 0; n < 500; n++)
		{
			usAdcX = TOUCH_ReadAdcX();
			usAdcY = TOUCH_ReadAdcY();

			if (TOUCH_PressValid(usAdcX, usAdcY))
			{
				if (++usCount > 5)
				{
					/* 按压有效, 保存校准点ADC采样值 */
					if (i == 0)
					{
						g_tTPParam.usAdcX1 = usAdcX;
						g_tTPParam.usAdcY1 = usAdcY;
					}
					else if (i == 1)
					{
						g_tTPParam.usAdcX2 = usAdcX;
						g_tTPParam.usAdcY2 = usAdcY;
					}
					else if (i == 2)
					{
						g_tTPParam.usAdcX3 = usAdcX;
						g_tTPParam.usAdcY3 = usAdcY;
					}
					else
					{
						g_tTPParam.usAdcX4 = usAdcX;
						g_tTPParam.usAdcY4 = usAdcY;
					}
					break;
				}
			}
			else
			{
				usCount = 0;
			}
			bsp_DelayMS(10);
		}
		if (n == 500)
		{
			return;
		}
	}

	TOUCH_WaitRelease(); 	/* 等待触笔释放 */

	/* 识别触摸的 X, Y 和 显示面板的 X，Y 是否需要交换 */
	g_tTPParam.XYChange = 0;		/* 1表示X Y需要交换 */
	if (LCD_GetHeight() < LCD_GetWidth())
	{
		if (TOUCH_Abs(g_tTPParam.usAdcX1 - g_tTPParam.usAdcX2) < TOUCH_Abs(g_tTPParam.usAdcY1 - g_tTPParam.usAdcY2))
		{
			g_tTPParam.XYChange = 1;
		}
	}
	else
	{
		if (TOUCH_Abs(g_tTPParam.usAdcX1 - g_tTPParam.usAdcX2) > TOUCH_Abs(g_tTPParam.usAdcY1 - g_tTPParam.usAdcY2))
		{
			g_tTPParam.XYChange = 1;
		}
	}

	g_tTPParam.usLcdX1 = TP_X1;
	g_tTPParam.usAdcY1 = TP_Y1;
	g_tTPParam.usLcdX2 = TP_X2;
	g_tTPParam.usLcdY2 = TP_Y2;
	g_tTPParam.usLcdX3 = TP_X3;
	g_tTPParam.usLcdY3 = TP_Y3;
	g_tTPParam.usLcdX4 = TP_X3;
	g_tTPParam.usLcdY4 = TP_Y3;

	/* 在最后一步，将校准参数保存入Flash 或者EEPROM */
	TOUCH_SaveParam();
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_SaveParam
*	功能说明: 保存校准参数	s_usAdcX1 s_usAdcX2 s_usAdcY1 s_usAdcX2
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TOUCH_SaveParam(void)
{
	
	g_tTPParam.TouchDirection = g_LcdDirection;	/* 2014-09-11 添加屏幕方向, 用于屏幕旋转时无需再次校准 */

	#if 1
		/* 写入EEPROM */
		ee_WriteBytes((uint8_t *)&g_tTPParam, TP_PARAM_EE_ADDR, sizeof(g_tTPParam));
	#else
		/* 写入CPU Flash */
		bsp_WriteCpuFlash(TP_PARAM_FLASH_ADDR, (uint8_t *)&g_tTPParam, sizeof(g_tTPParam));
	#endif	
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_LoadParam
*	功能说明: 读取校准参数
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TOUCH_LoadParam(void)
{
	#if 1
		/* 读取EEPROM中的参数 */
		ee_ReadBytes((uint8_t *)&g_tTPParam, TP_PARAM_EE_ADDR, sizeof(g_tTPParam));
	#else
		/* 读取CPU Flash中的参数 */
		bsp_ReadCpuFlash(TP_PARAM_FLASH_ADDR, (uint8_t *)&g_tTPParam, sizeof(g_tTPParam
	#endif	
	
	if (g_tTPParam.TouchDirection > 4)
	{
		g_tTPParam.TouchDirection = 0;
		TOUCH_SaveParam();
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
