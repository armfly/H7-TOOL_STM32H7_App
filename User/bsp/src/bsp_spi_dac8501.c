/*
*********************************************************************************************************
*
*	模块名称 : DAC8501 驱动模块(单通道带16位DAC)
*	文件名称 : bsp_dac8501.c
*	版    本 : V1.0
*	说    明 : DAC8501模块和CPU之间采用SPI接口。本驱动程序支持硬件SPI接口。
*			  通过宏切换。
*
*	修改记录 :
*		版本号  日期         作者     说明
*		V1.0    2015-10-11  armfly  正式发布
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	DAC8501基本特性:
	1、供电2.7 - 5V;  【本例使用3.3V】
	4、参考电压2.5V (推荐缺省的，外置的）

	对SPI的时钟速度要求: 高达30MHz， 速度很快.
	SCLK下降沿读取数据, 每次传送24bit数据， 高位先传
*/

#define CS1_CLK_ENABLE() 	__HAL_RCC_GPIOG_CLK_ENABLE()
#define CS1_GPIO			GPIOG
#define CS1_PIN				GPIO_PIN_10

#define CS1_1()				CS1_GPIO->BSRRL = CS1_PIN
#define CS1_0()				CS1_GPIO->BSRRH = CS1_PIN

#define CS2_1()				HC574_SetPin(NRF24L01_CE, 1);
#define CS2_0()				HC574_SetPin(NRF24L01_CE, 0);

/* 定义电压和DAC值间的关系。 两点校准 x是dac y 是电压 0.1mV */
#define X1	100
#define Y1  50

#define X2	65000
#define Y2  49400

/*
*********************************************************************************************************
*	函 数 名: bsp_InitDAC8501
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 ADS1256
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDAC8501(void)
{
	/* 配置CS GPIO */
	{
		GPIO_InitTypeDef gpio_init;

		/* 打开GPIO时钟 */
		CS1_CLK_ENABLE();
		
		gpio_init.Mode = GPIO_MODE_OUTPUT_PP;		/* 设置推挽输出 */
		gpio_init.Pull = GPIO_NOPULL;				/* 上下拉电阻不使能 */
		gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  	/* GPIO速度等级 */	
		gpio_init.Pin = CS1_PIN;	
		HAL_GPIO_Init(CS1_GPIO, &gpio_init);	

		/* CS2 使用扩展IO. 在 bsp_fmc_io.c 已配置 */
	}

	DAC8501_SetDacData(0, 0);	/* CH1输出0 */
	DAC8501_SetDacData(1, 0);	/* CH2输出0 */
}

/*
*********************************************************************************************************
*	函 数 名: DAC8501_SetCS1
*	功能说明: DAC8501 片选控制函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8501_SetCS1(uint8_t _Level)
{
	if (_Level == 0)
	{
		bsp_SpiBusEnter();	/* 占用SPI总线  */	
		bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_8, SPI_PHASE_1EDGE, SPI_POLARITY_LOW);		
		CS1_0();
	}
	else
	{		
		CS1_1();	
		bsp_SpiBusExit();	/* 释放SPI总线 */
	}	
}

/*
*********************************************************************************************************
*	函 数 名: DAC8501_SetCS2(0)
*	功能说明: 设置CS2。 用于运行中SPI共享。
*	形    参: 无
	返 回 值: 无
*********************************************************************************************************
*/
void DAC8501_SetCS2(uint8_t _level)
{
	if (_level == 0)
	{
		bsp_SpiBusEnter();	/* 占用SPI总线  */
		bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_8, SPI_PHASE_1EDGE, SPI_POLARITY_LOW);		
		CS2_1();
	}
	else
	{
		CS2_1();
		bsp_SpiBusExit();	/* 释放SPI总线 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: DAC8501_SetDacData
*	功能说明: 设置DAC数据
*	形    参: _ch, 通道,
*		     _data : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8501_SetDacData(uint8_t _ch, uint16_t _dac)
{
	uint32_t data;

	/*
		DAC8501.pdf page 12 有24bit定义

		DB24:18 = xxxxx 保留
		DB17： PD1
		DB16： PD0

		DB15：0  16位数据

		其中 PD1 PD0 决定4种工作模式
		      0   0  ---> 正常工作模式
		      0   1  ---> 输出接1K欧到GND
		      1   0  ---> 输出100K欧到GND
		      1   1  ---> 输出高阻
	*/

	data = _dac; /* PD1 PD0 = 00 正常模式 */

	if (_ch == 0)
	{
		DAC8501_SetCS1(0);
	}
	else
	{
		DAC8501_SetCS2(0);
	}

	/*　DAC8501 SCLK时钟高达30M，因此可以不延迟 */
	g_spiLen = 0;
	g_spiTxBuf[g_spiLen++] = (data >> 16);
	g_spiTxBuf[g_spiLen++] = (data >> 8);
	g_spiTxBuf[g_spiLen++] = (data);
	bsp_spiTransfer();	

	if (_ch == 0)
	{
		DAC8501_SetCS1(1);
	}
	else
	{
		DAC8501_SetCS2(1);
	}
}

/*
*********************************************************************************************************
*	函 数 名: DAC8501_DacToVoltage
*	功能说明: 将DAC值换算为电压值，单位0.1mV
*	形    参: _dac  16位DAC字
*	返 回 值: 电压。单位0.1mV
*********************************************************************************************************
*/
int32_t DAC8501_DacToVoltage(uint16_t _dac)
{
	int32_t y;

	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	y =  CaculTwoPoint(X1, Y1, X2, Y2, _dac);
	if (y < 0)
	{
		y = 0;
	}
	return y;
}

/*
*********************************************************************************************************
*	函 数 名: DAC8501_DacToVoltage
*	功能说明: 将DAC值换算为电压值，单位 0.1mV
*	形    参: _volt 电压。单位0.1mV
*	返 回 值: 16位DAC字
*********************************************************************************************************
*/
uint32_t DAC8501_VoltageToDac(int32_t _volt)
{
	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	return CaculTwoPoint(Y1, X1, Y2, X2, _volt);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
