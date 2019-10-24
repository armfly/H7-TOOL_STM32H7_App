/*
*********************************************************************************************************
*
*	模块名称 : AD7606数据采集模块
*	文件名称 : bsp_ad7606.c
*	版    本 : V1.0
*	说    明 : AD7606挂在STM32的FMC总线上。
*
*			本例子使用了 TIM3 作为硬件定时器，定时启动ADC转换
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2015-10-11 armfly  正式发布
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	STM32-V7开发板 + AD7606模块， 控制采集的IO:
	
	PC6/TIM3_CH1/TIM8_CH1     ----> AD7606_CONVST  (和摄像头复用),  输出PWM方波，作为ADC启动信号
	PE5/DCMI_D6/AD7606_BUSY   <---- AD7606_BUSY    , CPU在BUSY中断服务程序中读取采集结果
*/

#include "bsp.h"

/* CONVST 启动ADC转换的GPIO = PC6 */
#define CONVST_RCC_GPIO_CLK_ENABLE	__HAL_RCC_GPIOC_CLK_ENABLE
#define CONVST_GPIO		GPIOC
#define CONVST_PIN		GPIO_PIN_6
#define CONVST_TIMX		TIM8
#define CONVST_TIMCH	1

/* BUSY 转换完毕信号 = PE5 */
#define BUSY_RCC_GPIO_CLK_ENABLE	__HAL_RCC_GPIOE_CLK_ENABLE
#define BUSY_GPIO		GPIOE
#define BUSY_PIN		GPIO_PIN_5
#define BUSY_IRQn		EXTI9_5_IRQn
#define BUSY_IRQHandler	EXTI9_5_IRQHandler

/* 设置过采样的IO, 在扩展的74HC574上 */
#define OS0_1()		HC574_SetPin(AD7606_OS0, 1)
#define OS0_0()		HC574_SetPin(AD7606_OS0, 0)
#define OS1_1()		HC574_SetPin(AD7606_OS1, 1)
#define OS1_0()		HC574_SetPin(AD7606_OS1, 0)
#define OS2_1()		HC574_SetPin(AD7606_OS2, 1)
#define OS2_0()		HC574_SetPin(AD7606_OS2, 0)

/* 启动AD转换的GPIO : PC6 */
#define CONVST_1()	CONVST_GPIO->BSRRL = CONVST_PIN
#define CONVST_0()	CONVST_GPIO->BSRRH = CONVST_PIN

/* 设置输入量程的GPIO, 在扩展的74HC574上 */
#define RANGE_1()	HC574_SetPin(AD7606_RANGE, 1)
#define RANGE_0()	HC574_SetPin(AD7606_RANGE, 0)

/* AD7606复位口线, 在扩展的74HC574上 */
#define RESET_1()	HC574_SetPin(AD7606_RESET, 1)
#define RESET_0()	HC574_SetPin(AD7606_RESET, 0)

/* AD7606 FSMC总线地址，只能读，无需写 */
#define AD7606_RESULT()	*(__IO uint16_t *)0x60003000

AD7606_VAR_T g_tAD7606;		/* 定义1个全局变量，保存一些参数 */
AD7606_FIFO_T g_tAdcFifo;	/* 定义FIFO结构体变量 */

static void AD7606_CtrlLinesConfig(void);
static void AD7606_FSMCConfig(void);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitAD7606
*	功能说明: 配置连接外部SRAM的GPIO和FSMC
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitAD7606(void)
{
	AD7606_CtrlLinesConfig();
	AD7606_FSMCConfig();

	AD7606_SetOS(AD_OS_NO);		/* 无过采样 */
	AD7606_SetInputRange(0);	/* 0表示输入量程为正负5V, 1表示正负10V */

	AD7606_Reset();

	CONVST_1();					/* 启动转换的GPIO平时设置为高 */
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_CtrlLinesConfig
*	功能说明: 配置GPIO口线，FMC管脚设置为复用功能
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	安富莱STM32-V7开发板接线方法：
	PD0/FMC_D2
	PD1/FMC_D3
	PD4/FMC_NOE		---- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FMC_NWE		-XX- 写控制信号，AD7606 只有读，无写信号
	PD8/FMC_D13
	PD9/FMC_D14
	PD10/FMC_D15
	PD14/FMC_D0
	PD15/FMC_D1

	PE7/FMC_D4
	PE8/FMC_D5
	PE9/FMC_D6
	PE10/FMC_D7
	PE11/FMC_D8
	PE12/FMC_D9
	PE13/FMC_D10
	PE14/FMC_D11
	PE15/FMC_D12
	
	PG0/FMC_A10		--- 和主片选FMC_NE2一起译码
	PG1/FMC_A11		--- 和主片选FMC_NE2一起译码
	PG9/FMC_NE2		--- 主片选（TFT, OLED 和 AD7606）	
*/

/* 
	控制AD7606参数的其他IO分配在扩展的74HC574上
	X13 - AD7606_OS0
	X14 - AD7606_OS1
	X15 - AD7606_OS2
	X24 - AD7606_RESET
	X25 - AD7606_RAGE	
	
	PE5 - AD7606_BUSY
*/
static void AD7606_CtrlLinesConfig(void)
{
	/* bsp_fm_io 已配置fmc。 此处不必重复配置 
		bsp_InitExtIO();
	*/
	
	/*

	PD0/FMC_D2
	PD1/FMC_D3
	PD4/FMC_NOE		---- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FMC_NWE		-XX- 写控制信号，AD7606 只有读，无写信号
	PD8/FMC_D13
	PD9/FMC_D14
	PD10/FMC_D15
	PD14/FMC_D0
	PD15/FMC_D1

	PE7/FMC_D4
	PE8/FMC_D5
	PE9/FMC_D6
	PE10/FMC_D7
	PE11/FMC_D8
	PE12/FMC_D9
	PE13/FMC_D10
	PE14/FMC_D11
	PE15/FMC_D12
	
	PG0/FMC_A10		--- 和主片选FMC_NE2一起译码
	PG1/FMC_A11		--- 和主片选FMC_NE2一起译码
	PG9/FMC_NE2		--- 主片选（OLED, 74HC574, DM9000, AD7606）	
	
//	PF0/FMC_A0      ---- RS
	*/	

	GPIO_InitTypeDef gpio_init_structure;

	/* 使能 GPIO时钟 */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();

	/* 使能FMC时钟 */
	__HAL_RCC_FMC_CLK_ENABLE();

	/* 设置 GPIOD 相关的IO为复用推挽输出 */
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FMC;
	
	/* 配置GPIOD */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
	                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	/* 配置GPIOE */
	gpio_init_structure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
	                            GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	/* 配置GPIOG */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_9;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
	
	/* 配置GPIOH */
	gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
						| GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOH, &gpio_init_structure);

	/* 配置GPIOI */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6
						| GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
	HAL_GPIO_Init(GPIOI, &gpio_init_structure);
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AD7606_FSMCConfig(void)
{
	/* bsp_fm_io 已配置fmc。 此处不必重复配置 
		bsp_InitExtIO();
	*/
	/*
		AD7606规格书要求(3.3V时)：RD读信号低电平脉冲宽度最短21ns，高电平脉冲最短宽度15ns。

		按照如下配置 读数均正常。为了和同BANK的LCD配置相同，选择3-0-6-1-0-0
		3-0-5-1-0-0  : RD高持续75ns， 低电平持续50ns.  1us以内可读取8路样本数据到内存。
		1-0-1-1-0-0  : RD高75ns，低电平执行12ns左右，下降沿差不多也12ns.  数据读取正确。
	*/
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_Timing = {0};
		
	/*
		AD7606规格书要求(3.3V时)：RD读信号低电平脉冲宽度最短21ns，高电平脉冲最短宽度15ns。

		按照如下配置 读数均正常。为了和同BANK的LCD配置相同，选择3-0-6-1-0-0
		3-0-5-1-0-0  : RD高持续75ns， 低电平持续50ns.  1us以内可读取8路样本数据到内存。
		1-0-1-1-0-0  : RD高75ns，低电平执行12ns左右，下降沿差不多也12ns.  数据读取正确。
	*/
	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;

	/* SRAM 总线时序配置 */  
	SRAM_Timing.AddressSetupTime       = 4;
	SRAM_Timing.AddressHoldTime        = 1;
	SRAM_Timing.DataSetupTime          = 5;
	SRAM_Timing.BusTurnAroundDuration  = 3;
	SRAM_Timing.CLKDivision            = 2;
	SRAM_Timing.DataLatency            = 2;
	SRAM_Timing.AccessMode             = FMC_ACCESS_MODE_A;

	hsram.Init.NSBank             = FMC_NORSRAM_BANK1;
	hsram.Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;
	hsram.Init.MemoryType         = FMC_MEMORY_TYPE_SRAM;
	hsram.Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_32;	/* 32位总线宽度 */
	hsram.Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;
	hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram.Init.WaitSignalActive   = FMC_WAIT_TIMING_BEFORE_WS;
	hsram.Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;
	hsram.Init.WaitSignal         = FMC_WAIT_SIGNAL_DISABLE;
	hsram.Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;
	hsram.Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram.Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;
	hsram.Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;

	/* 初始化SRAM控制器 */
	if (HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
	{
		/* 初始化错误 */
		Error_Handler(__FILE__, __LINE__);
	}	
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_SetOS
*	功能说明: 配置AD7606数字滤波器，也就设置过采样倍率。
*			 通过设置 AD7606_OS0、OS1、OS2口线的电平组合状态决定过采样倍率。
*			 启动AD转换之后，AD7606内部自动实现剩余样本的采集，然后求平均值输出。
*
*			 过采样倍率越高，转换时间越长。
*			 无过采样时，AD转换时间 4us;
*				2倍过采样时 = 8.7us;
*				4倍过采样时 = 16us
*			 	64倍过采样时 = 286us
*
*	形    参: _ucOS : 过采样倍率, 0 - 6
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetOS(uint8_t _ucOS)
{
	g_tAD7606.ucOS = _ucOS;
	switch (_ucOS)
	{
		case AD_OS_X2:
			OS2_0();
			OS1_0();
			OS0_1();
			break;

		case AD_OS_X4:
			OS2_0();
			OS1_1();
			OS0_0();
			break;

		case AD_OS_X8:
			OS2_0();
			OS1_1();
			OS0_1();
			break;

		case AD_OS_X16:
			OS2_1();
			OS1_0();
			OS0_0();
			break;

		case AD_OS_X32:
			OS2_1();
			OS1_0();
			OS0_1();
			break;

		case AD_OS_X64:
			OS2_1();
			OS1_1();
			OS0_0();
			break;

		case AD_OS_NO:
		default:
			g_tAD7606.ucOS = AD_OS_NO;
			OS2_0();
			OS1_0();
			OS0_0();
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_SetInputRange
*	功能说明: 配置AD7606模拟信号输入量程。
*	形    参: _ucRange : 0 表示正负5V   1表示正负10V
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetInputRange(uint8_t _ucRange)
{
	if (_ucRange == 0)
	{
		g_tAD7606.ucRange = 0;
		RANGE_0();	/* 设置为正负5V */
	}
	else
	{
		g_tAD7606.ucRange = 1;
		RANGE_1();	/* 设置为正负10V */
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_Reset
*	功能说明: 硬件复位AD7606。复位之后恢复到正常工作状态。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_Reset(void)
{
	RESET_0();	/* 退出复位状态 */

	RESET_1();	/* 进入复位状态 */
	RESET_1();	/* 仅用于延迟。 RESET复位高电平脉冲宽度最小50ns。 */
	RESET_1();
	RESET_1();

	RESET_0();	/* 退出复位状态 */
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StartConvst
*	功能说明: 启动1次ADC转换
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StartConvst(void)
{
	/* page 7：  CONVST 高电平脉冲宽度和低电平脉冲宽度最短 25ns */
	/* CONVST平时为高 */
	CONVST_0();
	CONVST_0();
	CONVST_0();

	CONVST_1();
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_ReadNowAdc
*	功能说明: 读取8路采样结果。结果存储在全局变量 g_tAD7606
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_ReadNowAdc(void)
{
	g_tAD7606.sNowAdc[0] = AD7606_RESULT();	/* 读第1路样本 */
	g_tAD7606.sNowAdc[1] = AD7606_RESULT();	/* 读第2路样本 */
	g_tAD7606.sNowAdc[2] = AD7606_RESULT();	/* 读第3路样本 */
	g_tAD7606.sNowAdc[3] = AD7606_RESULT();	/* 读第4路样本 */
	g_tAD7606.sNowAdc[4] = AD7606_RESULT();	/* 读第5路样本 */
	g_tAD7606.sNowAdc[5] = AD7606_RESULT();	/* 读第6路样本 */
	g_tAD7606.sNowAdc[6] = AD7606_RESULT();	/* 读第7路样本 */
	g_tAD7606.sNowAdc[7] = AD7606_RESULT();	/* 读第8路样本 */
}

/*
*********************************************************************************************************
*		下面的函数用于定时采集模式。 TIM5硬件定时中断中读取ADC结果，存在全局FIFO
*
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	函 数 名: AD7606_HasNewData
*	功能说明: 判断FIFO中是否有新数据
*	形    参:  _usReadAdc : 存放ADC结果的变量指针
*	返 回 值: 1 表示有，0表示暂无数据
*********************************************************************************************************
*/
uint8_t AD7606_HasNewData(void)
{
	if (g_tAdcFifo.usCount > 0)
	{
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_FifoFull
*	功能说明: 判断FIFO是否满
*	形    参:  _usReadAdc : 存放ADC结果的变量指针
*	返 回 值: 1 表示满，0表示未满
*********************************************************************************************************
*/
uint8_t AD7606_FifoFull(void)
{
	return g_tAdcFifo.ucFull;
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_ReadFifo
*	功能说明: 从FIFO中读取一个ADC值
*	形    参:  _usReadAdc : 存放ADC结果的变量指针
*	返 回 值: 1 表示OK，0表示暂无数据
*********************************************************************************************************
*/
uint8_t AD7606_ReadFifo(uint16_t *_usReadAdc)
{
	if (AD7606_HasNewData())
	{
		*_usReadAdc = g_tAdcFifo.sBuf[g_tAdcFifo.usRead];
		if (++g_tAdcFifo.usRead >= ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usRead = 0;
		}

		DISABLE_INT();
		if (g_tAdcFifo.usCount > 0)
		{
			g_tAdcFifo.usCount--;
		}
		ENABLE_INT();
		return 1;
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StartRecord
*	功能说明: 开始采集
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StartRecord(uint32_t _ulFreq)
{
	AD7606_StopRecord();

	AD7606_Reset();					/* 复位硬件 */
	AD7606_StartConvst();			/* 启动采样，避免第1组数据全0的问题 */

	g_tAdcFifo.usRead = 0;			/* 必须在开启TIM2之前清0 */
	g_tAdcFifo.usWrite = 0;
	g_tAdcFifo.usCount = 0;
	g_tAdcFifo.ucFull = 0;

	AD7606_EnterAutoMode(_ulFreq);
}


/*
*********************************************************************************************************
*	函 数 名: AD7606_EnterAutoMode
*	功能说明: 配置硬件工作在自动采集模式，结果存储在FIFO缓冲区。
*	形    参:  _ulFreq : 采样频率，单位Hz，	1k，2k，5k，10k，20K，50k，100k，200k
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_EnterAutoMode(uint32_t _ulFreq)
{
	/* 配置PC6为TIM1_CH1功能，输出占空比50%的方波 */
	bsp_SetTIMOutPWM(CONVST_GPIO, CONVST_PIN, CONVST_TIMX,  CONVST_TIMCH, _ulFreq, 5000);
	
	/* 配置PE5, BUSY 作为中断输入口，下降沿触发 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		
		CONVST_RCC_GPIO_CLK_ENABLE();	/* 打开GPIO时钟 */
		__HAL_RCC_SYSCFG_CLK_ENABLE();

		GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;	/* 中断下降沿触发 */
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		
		GPIO_InitStructure.Pin = BUSY_PIN;
		HAL_GPIO_Init(BUSY_GPIO, &GPIO_InitStructure);	

		HAL_NVIC_SetPriority(BUSY_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(BUSY_IRQn);		
	}		
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_StopRecord
*	功能说明: 停止采集定时器
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_StopRecord(void)
{
	/* 配置PC6 输出低电平，关闭TIM */
	bsp_SetTIMOutPWM(CONVST_GPIO, CONVST_PIN, CONVST_TIMX,  CONVST_TIMCH, 1000, 10000);
	CONVST_1();					/* 启动转换的GPIO平时设置为高 */	

	/* 配置 BUSY 作为中断输入口，下降沿触发 */
	{
		GPIO_InitTypeDef   GPIO_InitStructure;
		
		CONVST_RCC_GPIO_CLK_ENABLE();		/* 打开GPIO时钟 */

		/* Configure PC.13 pin as input floating */
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Pin = CONVST_PIN;
		HAL_GPIO_Init(CONVST_GPIO, &GPIO_InitStructure);	

		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);		
	}
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_ISR
*	功能说明: 定时采集中断服务程序
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_ISR(void)
{
	uint8_t i;

	AD7606_ReadNowAdc();

	for (i = 0; i < 8; i++)
	{
		g_tAdcFifo.sBuf[g_tAdcFifo.usWrite] = g_tAD7606.sNowAdc[i];
		if (++g_tAdcFifo.usWrite >= ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usWrite = 0;
		}
		if (g_tAdcFifo.usCount < ADC_FIFO_SIZE)
		{
			g_tAdcFifo.usCount++;
		}
		else
		{
			g_tAdcFifo.ucFull = 1;		/* FIFO 满，主程序来不及处理数据 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: EXTI9_5_IRQHandler
*	功能说明: 外部中断服务程序入口。PE5 / AD7606_BUSY 下降沿中断触发
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#ifndef EXTI9_5_ISR_MOVE_OUT		/* bsp.h 中定义此行，表示本函数移到 stam32h7xx_it.c。 避免重复定义 */
void BUSY_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(BUSY_PIN);
}

/*
*********************************************************************************************************
*	函 数 名: EXTI9_5_IRQHandler
*	功能说明: 外部中断服务程序入口。PI6 / AD7606_BUSY 下降沿中断触发
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BUSY_PIN)
	{
		AD7606_ISR();
	}
}

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
