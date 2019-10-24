/*
*********************************************************************************************************
*
	模块名称 : 电阻式触摸芯片STMPE811驱动模块
	文件名称 : TOUCH_STMPE811.c
*	版    本 : V1.1
*	说    明 : STMPE811 驱动程序。I2C接口. H7-TOOL 用该芯片做普通输出IO使用。
*	修改记录 :
*		版本号  日期        作者    说明
*		V1.0    2014-06-08  armfly  首发。安富莱电子原创。
*		V1.1	2019-04-26	armfly	增加GPIO输出控制函数
*
*	Copyright (C), 2014-2015, 武汉安富莱电子有限公司 www.armfly.com
*
*********************************************************************************************************
*/

/*
	应用说明：访问STMPE811前，请先调用一次 bsp_InitI2C()函数配置好I2C相关的GPIO.
*/

#include "bsp.h"

#define touch_printf(...)
//#define touch_printf  printf

static void STMPE811_Reset(void);
static uint16_t s_AdcX, s_AdcY;

/*
*********************************************************************************************************
*	函 数 名: STMPE811_InitHardForGPIO
*	功能说明: 配置STMPE811寄存器, 8个IO全部作为输出口
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void STMPE811_InitHardForGPIO(void)
{	
	STMPE811_Reset();
	
	
	#if 1
	{
		static uint16_t id;
		
		id = STMPE811_ReadID();
		
		//printf("id = \r\n", id);
	}
	#endif
	
	/* STMPE811.pdf 第 45页, 推荐的编程步骤: 
	The following are the steps to configure the touch screen controller (TSC):
	a)  Disable the clock gating for the touch screen controller and ADC in the SYS_CFG2 register.		
	【SYS_CFG2】
		[7:4] RESERVED
		[3] TS_OFF: Switch off the clock supply to the temperature sensor
			1: Switches off the clock supply to the temperature sensor
		[2] GPIO_OFF: Switch off the clock supply to the GPIO
			1: Switches off the clock supply to the GPIO
		[1] TSC_OFF: Switch off the clock supplyto the touch screen controller
			1: Switches off the clock supply to the touch screen controller
		[0] ADC_OFF: Switch off the clock supply to the ADC
			1: Switches off the clock supply to the ADC	
	*/
	STMPE811_WriteReg1(REG811_SYS_CTRL2, 0x00);		/* 注意1是表示关闭时钟， 此处是打开所有的时钟 */
	

	/* Select Sample Time, bit number and ADC Reference */
	
	/* 选择采样时间，ADC分辨率，ADC参考电源
	【ADC_CTRL1】
		[7] RESERVED
		[6:4] SAMPLE_TIMEn: ADC conversion time in number of clock   --- ADC转换时间（多少个时钟）
			000: 36
			001: 44
			010: 56
			011: 64
			100: 80
			101: 96
			110: 124
			111: Not valid
		[3] MOD_12B: Selects 10 or 12-bit ADC operation
			1: 12 bit ADC              【选择12位】
			0: 10 bit ADC
		[2] RESERVED
		[1] REF_SEL: Selects between internal or external reference for the ADC
			1: External reference
			0: Internal reference      【选择内部参考】
		[0] RESERVED
	*/
	STMPE811_WriteReg1(REG811_ADC_CTRL1, (4 << 4) | (1 << 3) | (0 << 1));
	
	/* 等待20ms */
	bsp_DelayMS(20);  
	
	/* 选择ADC时钟速度 : 3.25 MHz 
	【ADC_CTRL2】
		[7] RESERVED
		[6] RESERVED
		[5] RESERVED
		[4] RESERVED
		[3] RESERVED
		[2] RESERVED
		[1:0] ADC_FREQ: Selects the clock speed of ADC
			00: 1.625 MHz typ.
			01: 3.25 MHz typ.
			10: 6.5 MHz typ.
			11: 6.5 MHz typ.	
	*/
	STMPE811_WriteReg1(REG811_ADC_CTRL2, 0x01);
		
	/* 配置8个管脚的工作模式。 1表示GPIO模式，0表示为触摸屏控制器/ADC			
	【GPIO_ALT_FUNCT】
		Reset: 0x0F
		Description: Alternate function register. "‘0’ sets the corresponding pin to function as touch 
			screen/ADC, and ‘1’ sets it into GPIO mode.	
	*/  
	STMPE811_WriteReg1(REG811_GPIO_AF, 0xFF);	/* 全部用于GPIO */
	
	/* Select 2 nF filter capacitor 
	【TSC_CFG】
		Description: Touch screen controller configuration register.
		[7:6] [AVE_CTRL_1/0: Average control   ---- 样本平均的个数， 选择4个样本平均
			00=1 sample
			01=2 samples
			10=4 samples 
			11=8 samples
		[5:3] TOUCH_DET_DELAY_2/1/0: Touch detect delay  ---- 触摸检测延迟， 选择 500us
			000 - 10 μs
			001 - 50 μs
			010 = 100μs
			011 = 500μs
			100 = 1ms
			101 = 5ms
			110 = 10 ms
			111 = 50 ms
		[2:0] SETTLING: Panel driver settling time     ----- 触摸板驱动建立时间，  选择 5ms
			000 = 10μs
			001 = 100μs
			010 = 500μS
			011 = 1ms
			100 = 5ms
			101 = 10 ms
			110 = 50 ms
			111=100ms
			
		1. For large panels (> 6”), a capacitor of 10 nF is recommended at the touch screen terminals for noise filtering. 
		In this case, settling time of 1 ms or more is recommended.	
	*/
	STMPE811_WriteReg1(REG811_TSC_CFG, (2 << 6) | (3 << 3) | (4 << 0));   
	
	/* 设置触发中断的样本个数。 此处设置为1，即只要有触摸就触发中断
	【FIFO_TH】
	Description: Triggers an interrupt upon reaching or exceeding the threshold value. This field must not be set as zero.
		[7:0] FIFO_TH: touch screen controller FIFO threshold	
	*/
	STMPE811_WriteReg1(REG811_FIFO_TH, 0x00);		/* 不要中断 */
	
	/* Write 0x01 to clear the FIFO memory content. 
	【FIFO_STA】
		Description: Current status of FIFO..
		[7] FIFO_OFLOW: 
			Reads 1 if FIFO is overflow
		[6] FIFO_FULL: 
			Reads 1 if FIFO is full
		[5] FIFO_EMPTY:
			Reads 1 if FIFO is empty
		[4] FIFO_TH_TRIG:
			0 = Current FIFO size is still below the threshold value
			1 = Current FIFO size is at or beyond the threshold value
		[3:1] RESERVED
		[0] FIFO_RESET:
			Write '0' : FIFO put out of reset mode
			Write '1' : Resets FIFO. All data in FIFO will be cleared.
			When TSC is enabled, FIFOresets automatically.	---- 当 TSC使能时，FIFO是自动清空的	
	*/
	STMPE811_WriteReg1(REG811_FIFO_STA, 0x01);	/* 复位FIFO */
	STMPE811_WriteReg1(REG811_FIFO_STA, 0x00);	/* 退出FIFO复位状态 */
	
	/* set the data format for Z value: 7 fractional part and 1 whole part 
	【TSC_FRACTION_Z】
		Reset: 0x00
		Description: This register allows to select the range and accuracy of the pressure measurement
		[7:3] RESERVED
		[2:0] FRACTION_Z: 
			000: Fractional part is 0, whole part is 8
			001: Fractional part is 1, whole part is 7
			010: Fractional part is 2, whole part is 6
			011: Fractional part is 3, whole part is 5
			100: Fractional part is 4, whole part is 4
			101: Fractional part is 5, whole part is 3
			110: Fractional part is 6, whole part is 2
			111: Fractional part is 7, whole part is 1	
	*/
	STMPE811_WriteReg1(REG811_TSC_FRACT_XYZ, 0x01);
	
	/* set the driving capability of the device for TSC pins: 50mA 
	【TSC_I_DRIVE】
		Description: This register sets the current limit value of the touch screen drivers
		[7:1] RESERVED
		[0] DRIVE: maximum current on the touch screen controller (TSC) driving channel
			0: 20 mA typical, 35 mA max
			1: 50 mA typical, 80 mA max	
	*/
	STMPE811_WriteReg1(REG811_TSC_I_DRIVE, 0x01);		/* 选择50ms驱动电流 */
	
	/* Use no tracking index, touch-panel controller operation mode (XYZ) and 
	 enable the TSC
	【TSC_CTRL】
		Description: 4-wire touch screen controller (TSC) setup.
		[7] TSC_STA: TSC status
			Reads '1' when touch is detected
			Reads '0' when touch is not detected
			Writing to this register has no effect
		[6:4] TRACK: Tracking index  
		  --- 运动追踪。如果当前坐标点和上个坐标点的位移超过设定值才会上报
		  公式: (Current X - Previously Reported X) + (Current Y - Previously Reported Y) > Tracking Index
			000: No window tracking
			001: 4
			010: 8
			011: 16
			100: 32
			101: 64
			110: 92
			111: 127
		[3:1] OP_MOD: TSC operating mode
			000: X, Y, Z acquisition
			001: X, Y only			----- 选择 X Y 数据模式，不需要Z轴压力
			010: X only
			011: Y only
			100: Z only
			This field cannot be written on, when EN = 1
		[0] EN: Enable TSC			 
	*/
	STMPE811_WriteReg1(REG811_TSC_CTRL, (0 << 4) | (0 << 1) | (0 << 0));		/* 禁止TSC */
	
	/*  Clear all the status pending bits 
	【INT_STA】
		Type: R
		Reset: 0x10
		Description: The interrupt status register monitors the status of the interruption from a particular 
		interrupt source to the host. Regardless of whether the INT_EN bits are enabled, the 
		INT_STA bits are still updated. Writing '1' to this register clears the corresponding 
		bits. Writing '0' has no effect.
		
		[7] GPIO: Any enabled GPIO interrupts
		[6] ADC: Any enabled ADC interrupts
		[5] TEMP_SENS: Temperature threshold triggering
		[4] FIFO_EMPTY: FIFO is empty
		[3] FIFO_FULL: FIFO is full
		[2] FIFO_OFLOW: FIFO is overflowed
		[1] FIFO_TH: FIFO is equal or above threshold value.
			This bit is set when FIFO level equals to threshold value. It will only be asserted again if FIFO 
			level drops to < threshold value, and increased back to threshold value.
		[0] TOUCH_DET: Touch is detected	
	*/
	STMPE811_WriteReg1(REG811_INT_STA, 0xFF); 
	
	STMPE811_WriteReg1(REG811_INT_EN, 0x01); 
	
	STMPE811_WriteReg1(REG811_INT_CTRL, 0x01); 
	
	
	/*  设置GPIO方向
	【GPIO_DIR】
	Description: GPIO set pin direction register. 
		Writing ‘0’ sets the corresponding GPIO to input state, and ‘1’ sets it to output state. 
		All bits are ‘0’ on reset.
	*/
	STMPE811_WriteReg1(REG811_GPIO_DIR, 0xFC);		/* bit7:2是输出， bit1:0是输入 */
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_InitHard
*	功能说明: 配置STMPE811寄存器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void STMPE811_InitHard(void)
{	
	STMPE811_Reset();
	
	/* STMPE811.pdf 第 45页, 推荐的编程步骤: 
	The following are the steps to configure the touch screen controller (TSC):
	a)  Disable the clock gating for the touch screen controller and ADC in the SYS_CFG2 register.		
	【SYS_CFG2】
		[7:4] RESERVED
		[3] TS_OFF: Switch off the clock supply to the temperature sensor
			1: Switches off the clock supply to the temperature sensor
		[2] GPIO_OFF: Switch off the clock supply to the GPIO
			1: Switches off the clock supply to the GPIO
		[1] TSC_OFF: Switch off the clock supplyto the touch screen controller
			1: Switches off the clock supply to the touch screen controller
		[0] ADC_OFF: Switch off the clock supply to the ADC
			1: Switches off the clock supply to the ADC	
	*/
	STMPE811_WriteReg1(REG811_SYS_CTRL2, 0x00);		/* 注意1是表示关闭时钟， 此处是打开所有的时钟 */
	

	/* Select Sample Time, bit number and ADC Reference */
	
	/* 选择采样时间，ADC分辨率，ADC参考电源
	【ADC_CTRL1】
		[7] RESERVED
		[6:4] SAMPLE_TIMEn: ADC conversion time in number of clock   --- ADC转换时间（多少个时钟）
			000: 36
			001: 44
			010: 56
			011: 64
			100: 80
			101: 96
			110: 124
			111: Not valid
		[3] MOD_12B: Selects 10 or 12-bit ADC operation
			1: 12 bit ADC              【选择12位】
			0: 10 bit ADC
		[2] RESERVED
		[1] REF_SEL: Selects between internal or external reference for the ADC
			1: External reference
			0: Internal reference      【选择内部参考】
		[0] RESERVED
	*/
	STMPE811_WriteReg1(REG811_ADC_CTRL1, (4 << 4) | (1 << 3) | (0 << 1));
	
	/* 等待20ms */
	bsp_DelayMS(20);  
	
	/* 选择ADC时钟速度 : 3.25 MHz 
	【ADC_CTRL2】
		[7] RESERVED
		[6] RESERVED
		[5] RESERVED
		[4] RESERVED
		[3] RESERVED
		[2] RESERVED
		[1:0] ADC_FREQ: Selects the clock speed of ADC
			00: 1.625 MHz typ.
			01: 3.25 MHz typ.
			10: 6.5 MHz typ.
			11: 6.5 MHz typ.	
	*/
	STMPE811_WriteReg1(REG811_ADC_CTRL2, 0x01);
		
	/* 配置8个管脚的工作模式。 1表示GPIO模式，0表示为触摸屏控制器/ADC			
	【GPIO_ALT_FUNCT】
		Reset: 0x0F
		Description: Alternate function register. "‘0’ sets the corresponding pin to function as touch 
			screen/ADC, and ‘1’ sets it into GPIO mode.	
	*/  
	STMPE811_WriteReg1(REG811_GPIO_AF, 0x0F);	/* 高4位用于TSC, 低4位用于GPIO */
	
	/* Select 2 nF filter capacitor 
	【TSC_CFG】
		Description: Touch screen controller configuration register.
		[7:6] [AVE_CTRL_1/0: Average control   ---- 样本平均的个数， 选择4个样本平均
			00=1 sample
			01=2 samples
			10=4 samples 
			11=8 samples
		[5:3] TOUCH_DET_DELAY_2/1/0: Touch detect delay  ---- 触摸检测延迟， 选择 500us
			000 - 10 μs
			001 - 50 μs
			010 = 100μs
			011 = 500μs
			100 = 1ms
			101 = 5ms
			110 = 10 ms
			111 = 50 ms
		[2:0] SETTLING: Panel driver settling time     ----- 触摸板驱动建立时间，  选择 5ms
			000 = 10μs
			001 = 100μs
			010 = 500μS
			011 = 1ms
			100 = 5ms
			101 = 10 ms
			110 = 50 ms
			111=100ms
			
		1. For large panels (> 6”), a capacitor of 10 nF is recommended at the touch screen terminals for noise filtering. 
		In this case, settling time of 1 ms or more is recommended.	
	*/
	STMPE811_WriteReg1(REG811_TSC_CFG, (2 << 6) | (3 << 3) | (4 << 0));   
	
	/* 设置触发中断的样本个数。 此处设置为1，即只要有触摸就触发中断
	【FIFO_TH】
	Description: Triggers an interrupt upon reaching or exceeding the threshold value. This field must not be set as zero.
		[7:0] FIFO_TH: touch screen controller FIFO threshold	
	*/
	STMPE811_WriteReg1(REG811_FIFO_TH, 0x01);
	
	/* Write 0x01 to clear the FIFO memory content. 
	【FIFO_STA】
		Description: Current status of FIFO..
		[7] FIFO_OFLOW: 
			Reads 1 if FIFO is overflow
		[6] FIFO_FULL: 
			Reads 1 if FIFO is full
		[5] FIFO_EMPTY:
			Reads 1 if FIFO is empty
		[4] FIFO_TH_TRIG:
			0 = Current FIFO size is still below the threshold value
			1 = Current FIFO size is at or beyond the threshold value
		[3:1] RESERVED
		[0] FIFO_RESET:
			Write '0' : FIFO put out of reset mode
			Write '1' : Resets FIFO. All data in FIFO will be cleared.
			When TSC is enabled, FIFOresets automatically.	---- 当 TSC使能时，FIFO是自动清空的	
	*/
	STMPE811_WriteReg1(REG811_FIFO_STA, 0x01);	/* 复位FIFO */
	STMPE811_WriteReg1(REG811_FIFO_STA, 0x00);	/* 退出FIFO复位状态 */
	
	/* set the data format for Z value: 7 fractional part and 1 whole part 
	【TSC_FRACTION_Z】
		Reset: 0x00
		Description: This register allows to select the range and accuracy of the pressure measurement
		[7:3] RESERVED
		[2:0] FRACTION_Z: 
			000: Fractional part is 0, whole part is 8
			001: Fractional part is 1, whole part is 7
			010: Fractional part is 2, whole part is 6
			011: Fractional part is 3, whole part is 5
			100: Fractional part is 4, whole part is 4
			101: Fractional part is 5, whole part is 3
			110: Fractional part is 6, whole part is 2
			111: Fractional part is 7, whole part is 1	
	*/
	STMPE811_WriteReg1(REG811_TSC_FRACT_XYZ, 0x01);
	
	/* set the driving capability of the device for TSC pins: 50mA 
	【TSC_I_DRIVE】
		Description: This register sets the current limit value of the touch screen drivers
		[7:1] RESERVED
		[0] DRIVE: maximum current on the touch screen controller (TSC) driving channel
			0: 20 mA typical, 35 mA max
			1: 50 mA typical, 80 mA max	
	*/
	STMPE811_WriteReg1(REG811_TSC_I_DRIVE, 0x01);		/* 选择50ms驱动电流 */
	
	/* Use no tracking index, touch-panel controller operation mode (XYZ) and 
	 enable the TSC
	【TSC_CTRL】
		Description: 4-wire touch screen controller (TSC) setup.
		[7] TSC_STA: TSC status
			Reads '1' when touch is detected
			Reads '0' when touch is not detected
			Writing to this register has no effect
		[6:4] TRACK: Tracking index  
		  --- 运动追踪。如果当前坐标点和上个坐标点的位移超过设定值才会上报
		  公式: (Current X - Previously Reported X) + (Current Y - Previously Reported Y) > Tracking Index
			000: No window tracking
			001: 4
			010: 8
			011: 16
			100: 32
			101: 64
			110: 92
			111: 127
		[3:1] OP_MOD: TSC operating mode
			000: X, Y, Z acquisition
			001: X, Y only			----- 选择 X Y 数据模式，不需要Z轴压力
			010: X only
			011: Y only
			100: Z only
			This field cannot be written on, when EN = 1
		[0] EN: Enable TSC			 
	*/
	STMPE811_WriteReg1(REG811_TSC_CTRL, (0 << 4) | (1 << 1) | (1 << 0));
	
	/*  Clear all the status pending bits 
	【INT_STA】
		Type: R
		Reset: 0x10
		Description: The interrupt status register monitors the status of the interruption from a particular 
		interrupt source to the host. Regardless of whether the INT_EN bits are enabled, the 
		INT_STA bits are still updated. Writing '1' to this register clears the corresponding 
		bits. Writing '0' has no effect.
		
		[7] GPIO: Any enabled GPIO interrupts
		[6] ADC: Any enabled ADC interrupts
		[5] TEMP_SENS: Temperature threshold triggering
		[4] FIFO_EMPTY: FIFO is empty
		[3] FIFO_FULL: FIFO is full
		[2] FIFO_OFLOW: FIFO is overflowed
		[1] FIFO_TH: FIFO is equal or above threshold value.
			This bit is set when FIFO level equals to threshold value. It will only be asserted again if FIFO 
			level drops to < threshold value, and increased back to threshold value.
		[0] TOUCH_DET: Touch is detected	
	*/
	STMPE811_WriteReg1(REG811_INT_STA, 0xFF); 
	
	STMPE811_WriteReg1(REG811_INT_EN, 0x01); 
	
	STMPE811_WriteReg1(REG811_INT_CTRL, 0x01); 
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ClearInt
*	功能说明: 清楚触笔中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void STMPE811_ClearInt(void)
{
	STMPE811_WriteReg1(REG811_INT_STA, 0xFF); 
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_Reset
*	功能说明: 软件复位STMPE811芯片
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void STMPE811_Reset(void)
{
	/* 
	【SYS_CTRL1】
		[7:2] RESERVED
		[1] SOFT_RESET: Reset the STMPE811 using the serial communication interface
		[0] HIBERNATE: Force the device into hibernation mode.
			Forcing the device into hibernation mode by writing ‘1’ to this bit would disable the h
				feature. If the hot-key feature is required, use the default auto-hibernation mode.
	*/
	STMPE811_WriteReg1(REG811_SYS_CTRL1, 0x02);
	
	/* 等待芯片复位 */
	bsp_DelayMS(20); 
	
	/* 退出复位状态，所有的寄存器恢复缺省值 */
	STMPE811_WriteReg1(REG811_SYS_CTRL1, 0x00);
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadID
*	功能说明: 读芯片ID. 应该返回
*	形    参: 无
*	返 回 值: 返回ID. 0x0811
*********************************************************************************************************
*/
uint16_t STMPE811_ReadID(void)
{
	return STMPE811_ReadReg2(REG811_CHIP_ID);
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadBytes
*	功能说明: 连续读取若干数据
*	形    参: 
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*			 _ucAddress : 起始地址
*			 _ucSize : 数据长度，单位为字节
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t STMPE811_ReadBytes(uint8_t *_pReadBuf, uint8_t _ucAddress, uint8_t _ucSize)
{
	uint8_t i;

	/* 第1步：发起I2C总线启动信号 */
	i2c_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_SendByte(STMPE811_I2C_ADDRESS | I2C_WR);	/* 此处是写指令 */

	/* 第3步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送寄存器地址 */
	i2c_SendByte((uint8_t)_ucAddress);
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	i2c_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_SendByte(STMPE811_I2C_ADDRESS | I2C_RD);	/* 此处是读指令 */

	/* 第8步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第9步：循环读取数据 */
	for (i = 0; i < _ucSize; i++)
	{
		_pReadBuf[i] = i2c_ReadByte();	/* 读1个字节 */

		/* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
		if (i != _ucSize - 1)
		{
			i2c_Ack();	/* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
		}
		else
		{
			i2c_NAck();	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		}
	}
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 1;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_WriteBytes
*	功能说明: 连续写入若干数据
*	形    参: 
*			 _pWriteBuf : 将要写入到芯片的数据的缓冲区指针
*			 _ucAddress : 起始地址
*			 _ucSize : 数据长度，单位为字节
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t STMPE811_WriteBytes(uint8_t *_pWriteBuf, uint8_t _ucAddress, uint8_t _ucSize)
{
	uint8_t i;

	/* 第1步：发起I2C总线启动信号 */
	i2c_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_SendByte(STMPE811_I2C_ADDRESS | I2C_WR);	/* 此处是写指令 */

	/* 第3步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送寄存器地址 */
	i2c_SendByte((uint8_t)_ucAddress);
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第5步：循环写数据 */
	for (i = 0; i < _ucSize; i++)
	{
		i2c_SendByte(_pWriteBuf[i]);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
	}
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 1;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_WriteReg1
*	功能说明: 写1个字节到寄存器
*	形    参: _ucRegAddr : 寄存器地址
*			 _ucValue    : 寄存器值
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
void STMPE811_WriteReg1(uint8_t _ucRegAddr, uint8_t _ucValue)
{
	STMPE811_WriteBytes(&_ucValue, _ucRegAddr, 1);
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_WriteReg1
*	功能说明: 写2个字节到寄存器
*	形    参: _ucRegAddr : 寄存器地址
*			 _ucValue    : 寄存器值
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
void STMPE811_WriteReg2(uint8_t _ucRegAddr, uint16_t _usValue)
{
	uint8_t buf[2];
	
	buf[0] = _usValue >> 8;
	buf[1] = _usValue;
	STMPE811_WriteBytes(buf, _ucRegAddr, 2);
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadReg1
*	功能说明: 读寄存器1字节
*	形    参: _ucRegAddr : 寄存器地址
*			 _ucValue    : 寄存器值
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
uint8_t STMPE811_ReadReg1(uint8_t _ucRegAddr)
{
	uint8_t read;
	
	STMPE811_ReadBytes(&read, _ucRegAddr, 1);
	
	return read;
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadReg2
*	功能说明: 读2个字节
*	形    参: _ucRegAddr : 寄存器地址
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
uint16_t STMPE811_ReadReg2(uint8_t _ucRegAddr)
{
	uint8_t buf[2];
	
	STMPE811_ReadBytes(buf, _ucRegAddr, 2);
	
	return (((uint16_t)buf[0] << 8) | buf[1]);
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadX
*	功能说明: 读取X坐标adc
*	形    参: 无
*	返 回 值: X坐标值adc
*********************************************************************************************************
*/
uint16_t STMPE811_ReadX(void)
{
	/* 按照 XY 读取模式，连续读取3字节数据，然后分解出X,Y 	
	 |  byte0   |     byte1      |   byte2  |  
	 | X[11:4], | X[3:0],Y[11:8] | Y[7:0]   |
	*/
	uint8_t buf[3];
	
#if 0
	STMPE811_ReadBytes(buf, REG811_TSC_DATA1, 3);
	
	s_AdcX = ((uint16_t)buf[0] << 4) | (buf[1] >> 4);
	s_AdcY = ((uint16_t)(buf[1] & 0xF) << 8) | buf[2];	
#else
	if (STMPE811_ReadReg1(REG811_TSC_CTRL) & 0x80)
	{	
		STMPE811_ReadBytes(buf, REG811_TSC_DATA1, 3);
		
		s_AdcX = ((uint16_t)buf[0] << 4) | (buf[1] >> 4);
		s_AdcY = ((uint16_t)(buf[1] & 0xF) << 8) | buf[2];
		
		#if 0
		/* for debug */
		{
			static int32_t s_t1 = 0;
			int32_t tt;
						
			tt = bsp_GetRunTime();
			if (tt - s_t1 > 1000)
			{
				printf("\r\n");
				s_t1 = tt;
			}
			printf("(%7d) %5d %5d\r\n", tt, s_AdcX, s_AdcY);
		}
		#endif
	}
	else
	{
		s_AdcX = 0;
		s_AdcY = 0;
	}
#endif
	
	return s_AdcX;
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadX
*	功能说明: 读取Y坐标adc
*	形    参: 无
*	返 回 值: Y坐标值adc
*********************************************************************************************************
*/
uint16_t STMPE811_ReadY(void)
{
	return  s_AdcY;
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadGPIO
*	功能说明: 读取GPIO状态
*	形    参: 无
*	返 回 值: GPIO状态字
*********************************************************************************************************
*/
uint8_t STMPE811_ReadGPIO(void)
{
	/*  设置GPIO方向
	【GPIO_DIR】
	Description: GPIO set pin direction register. 
		Writing ‘0’ sets the corresponding GPIO to input state, and ‘1’ sets it to output state. 
		All bits are ‘0’ on reset.
	*/
	STMPE811_WriteReg1(REG811_GPIO_DIR, 0);		/* 全部是输入 */
	
	return STMPE811_ReadReg1(REG811_GPIO_MP_STA);		/* 读GPIO状态 */	
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_WriteGPIO
*	功能说明: 设置GPIO输出值
*	形    参: _pin : 0-7
*			  _vlaue : 0或1
*	返 回 值: 无
*********************************************************************************************************
*/
void STMPE811_WriteGPIO(uint8_t _pin, uint8_t _vlaue)
{
	if (_vlaue == 0)
	{
		STMPE811_WriteReg1(REG811_GPIO_CLR_PIN, 1 << _pin);
	}
	else
	{
		STMPE811_WriteReg1(REG811_GPIO_SET_PIN, 1 << _pin);
	}
}

/*
*********************************************************************************************************
*	函 数 名: STMPE811_ReadIO
*	功能说明: 根据GPIO的 BIT3，BIT2，BIT0 三根IO的电平状态识别显示器类型。 仅用于安富莱电子生产的显示模块。
*	形    参: 无
*	返 回 值: IO状态
*********************************************************************************************************
*/
uint8_t STMPE811_ReadIO(void)
{
	uint8_t gpio;
	uint8_t type;
	
	gpio = STMPE811_ReadGPIO();
	
	/* BIT3  BIT2 BIT0 */
	type = ((gpio >> 1) & 0x6) | (gpio & 0x01);
	
	return type;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
