/*
*********************************************************************************************************
*
*	模块名称 : AD9833 驱动模块(单通道带16位ADC)
*	文件名称 : bsp_AD9833.c
*	版    本 : V1.0
*	说    明 : AD9833模块和CPU之间采用SPI接口。本驱动程序支持硬件SPI接口和软件SPI接口。SPI总线的 
*			   SCK 、 MOSI 在bsp_spi_bus.c中配置, 通过宏切换硬件SPI还是软件SPI
*
*	修改记录 :
*		版本号  日期         作者     说明
*		V1.0   2015-08-09    armfly   首版
*
*	Copyright (C), 2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* SPI总线的 SCK、 MOSI 在bsp_spi_bus.c中配置， 支持硬件SPI和软件SPI */

/* AD9833晶振频率 = 25MHz */
#define AD9833_SYSTEM_CLOCK 25000000UL 

/*
	AD9833模块可以直接插到STM32-V6开发板CN19排母(2*4P 2.54mm)接口上 （SPI）

	AD9833 模块    STM32-V5开发板
	  GND   ------  GND
	  VCC   ------  3.3V
      
	  FSYNC ------  PG10/NRF24L01_CSN
	  SCLK  ------  PB3/SPI3_SCK
	  SDATA ------  PB5/SPI3_MOSI 
	  
			------  PB4/SPI3_MISO         --- DAC无读出需求

	  SPI的SCK时钟速度最快可以到40MHz
*/

/* SPI片选 CS,  芯片手册叫 FSYNC 帧同步，都是一个意思。  */
#define CS_CLK_ENABLE() 	__HAL_RCC_GPIOG_CLK_ENABLE()
#define CS_GPIO				GPIOG
#define CS_PIN				GPIO_PIN_10

#define CS_1()				CS_GPIO->BSRRL = CS_PIN
#define CS_0()				CS_GPIO->BSRRH = CS_PIN


/*定义波形的寄存器值*/
#define Triangle_Wave  	0x2002
#define Sine_Wave 		0x2028
#define square wave 	0x2000

static void AD9833_ConfigGPIO(void);
static void AD9833_Write16Bits(uint16_t _cmd);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitAD9833
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 AD9833
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitAD9833(void)
{
	AD9833_ConfigGPIO();	/* 配置片选。SPI总线在 bsp_spi_bus.c 中进行配置 */
	
	/* 初始化流程 */
	{
		/* 重启不会重新设置相位、频率和控制寄存器。 */
		AD9833_Write16Bits(0x0100);	/* 写控制寄存器，复位寄存器 RESET = 0 */
		
		AD9833_WriteFreqReg(0, 0);	/* 清零频率寄存器 0 */
		AD9833_WriteFreqReg(1, 0);	/* 清零频率寄存器 1 */
		
		AD9833_WritePhaseReg(0, 0);	/* 清零相位寄存器 0 */
		AD9833_WritePhaseReg(1, 0);	/* 清零相位寄存器 1 */		
		
		AD9833_Write16Bits(0x0000);	/* 写控制寄存器，复位寄存器 RESET = 1 */
	}
	
	AD9833_SelectWave(NONE_WAVE);	/* 停止输出 */
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_ConfigGPIO
*	功能说明: 配置GPIO。 不包括 SCK  MOSI  MISO 共享的SPI总线。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AD9833_ConfigGPIO(void)
{
	/* 配置CS GPIO */
	{
		GPIO_InitTypeDef gpio_init;

		/* 打开GPIO时钟 */
		CS_CLK_ENABLE();
		
		gpio_init.Mode = GPIO_MODE_OUTPUT_PP;		/* 设置推挽输出 */
		gpio_init.Pull = GPIO_NOPULL;				/* 上下拉电阻不使能 */
		gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;  	/* GPIO速度等级 */	
		gpio_init.Pin = CS_PIN;	
		HAL_GPIO_Init(CS_GPIO, &gpio_init);	

		CS_1();
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_SetCS
*	功能说明: 设置CS1。 用于运行中SPI共享。
*	形    参: 无
	返 回 值: 无
*********************************************************************************************************
*/
void AD9833_SetCS(uint8_t _Level)
{
	if (_Level == 0)
	{
		bsp_SpiBusEnter();	/* 占用SPI总线  */	
		bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_8, SPI_PHASE_1EDGE, SPI_POLARITY_HIGH);		
		CS_0();
	}
	else
	{		
		CS_1();	
		bsp_SpiBusExit();	/* 释放SPI总线 */
	}	
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_Write16Bits      
*	功能说明: 向SPI总线发送16个bit数据   发送控制字
*	形    参: _cmd : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
static void AD9833_Write16Bits(uint16_t _cmd)
{	
	AD9833_SetCS(0);
	
	g_spiLen = 0;
	g_spiTxBuf[g_spiLen++] = (_cmd >> 8);
	g_spiTxBuf[g_spiLen++] = (_cmd);
	bsp_spiTransfer();	
	
	AD9833_SetCS(1);
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_Write16Bits      
*	功能说明: 选择波形输出类型。NONE_WAVE表示停止输出。
*	形    参: _type : 波形类型
*					NONE_WAVE  表示停止输出
*					TRI_WAVE   表示输出三角波
*					SINE_WAVE  表示输出正弦波
*					SQU_WAVE   表示输出方波
*	返 回 值: 无
*********************************************************************************************************
*/
void AD9833_SelectWave(AD9833_WAVE_E _type) 
{	
	/* D5  D1 
		0   0  -- 正弦
	    0   1  -- 三角
	    1   0  -- 方波
	*/	
	if (_type == NONE_WAVE)
	{
		AD9833_Write16Bits(0x20C0); /* 无输出 */
	}
	else if (_type == TRI_WAVE)
	{
		AD9833_Write16Bits(0x2002); /* 频率寄存器输出三角波 */				
	}
	else if (_type == SINE_WAVE)
	{
		AD9833_Write16Bits(0x2000); /* 频率寄存器输出正弦波 */					
	}
	else if (_type == SQU_WAVE)
	{
		AD9833_Write16Bits(0x2028); /* 频率寄存器输出方波 */	
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_WriteFreqReg      
*	功能说明: 写频率寄存器。  需要确保执行过一次 AD9833_Write16Bits(0x2000); B28=1 表示写频率分2次写入
*	形    参: _mode : 0表示频率寄存器0， 1表示频率寄存器1
*			  _freq_reg : 频率寄存器的值； 28bit;
*	返 回 值: 无
*********************************************************************************************************
*/
void AD9833_WriteFreqReg(uint8_t _mode, uint32_t _freq_reg)
{
	uint16_t lsb_14bit;
	uint16_t msb_14bit;
	
	lsb_14bit = _freq_reg & 0x3FFF;
	msb_14bit = (_freq_reg >> 14) & 0x3FFF;
	if (_mode == 0)	/* 写频率寄存器0 */
	{
		AD9833_Write16Bits(0x4000 + lsb_14bit);	/* 写2次，第1次是低14位 */		
		AD9833_Write16Bits(0x4000 + msb_14bit);	/* 写2次，第2次是高14位 */	
	}
	else	/* 写频率寄存器1 */
	{	
		AD9833_Write16Bits(0x8000 + lsb_14bit);	/* 写2次，第1次是低14位 */		
		AD9833_Write16Bits(0x8000 + msb_14bit);	/* 写2次，第2次是高14位 */	
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_WritePhaseReg      
*	功能说明: 写相位寄存器。  
*	形    参: _mode : 0表示相位寄存器0， 1表示相位寄存器1
*			  _phase_reg : 相位寄存器的值, 12bit
*	返 回 值: 无
*********************************************************************************************************
*/
void AD9833_WritePhaseReg(uint8_t _mode, uint32_t _phase_reg)
{
	_phase_reg &= 0xFFF;
	if (_mode == 0)	/* 写相位寄存器0 */
	{
		AD9833_Write16Bits(0xC000 + _phase_reg);
	}
	else	/* 写相位寄存器1 */
	{	
		AD9833_Write16Bits(0xE000 + _phase_reg);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD9833_SetWaveFreq      
*	功能说明: 设置波形频率。单位0.1Hz
*	形    参: _WaveFreq 频率单位。0.1Hz，  100表示10.0Hz
*	返 回 值: 无
*********************************************************************************************************
*/
void AD9833_SetWaveFreq(uint32_t _WaveFreq)
{
	uint32_t freq_reg;

	/*
		输出频率 _WaveFreq = (25000000 / 2^28) * freq_reg
		寄存器值 freq_reg =  268435456 / 25000000	* _WaveFreq
	
		268435456 / 25000000 = 10.73741824
	*/	
	//freq = 268435456.0 / AD9833_SYSTEM_CLOCK * _freq;
	freq_reg = ((int64_t)_WaveFreq * 268435456) / (10 * AD9833_SYSTEM_CLOCK);	
	
	AD9833_WriteFreqReg(0, freq_reg);	/* 写频率寄存器 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
