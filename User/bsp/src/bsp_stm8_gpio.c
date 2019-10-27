/*
*********************************************************************************************************
*
*	模块名称 : I2C GPIO模块驱动模块
*	文件名称 : bsp_stm8_gpio.c
*	版    本 : V1.0
*	说    明 : STM8S i2c GPIO模块
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2019-04-27 armfly  
*
*	Copyright (C), 2016-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	移植事项
	1. 控制PWM背光的函数需要更改。  LCD_SetPwmBackLight
	2. FSMC配置的片选需要更改	LCD_FSMCConfig
*/

#include "bsp.h"

static uint8_t I2C_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);
static uint8_t I2C_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

static uint8_t s_stms_out[2];
static uint16_t s_stms_pwm[2];

/*
*********************************************************************************************************
*	函 数 名: STM8_InitHard
*	功能说明: 初始化硬件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t STM8_InitHard(void)
{
	uint8_t buf[8];
	uint8_t i;

	for (i = 0; i < 10; i++)
	{
		memset(buf, 0x55, 8);
		I2C_ReadBytes(buf, REG_ID, 8);

		if (buf[0] == 0x75)
		{
			break;
		}
		bsp_DelayUS(10 * 1000);
	}

	//	/* 配置GPIO为输出口 */
	//	buf[0] = 0xFF;
	//	buf[1] = 0;
	//	I2C_WriteBytes(buf, REG_CFG1, 2);

	//	s_stms_out[0] = 0x12;
	//	s_stms_out[1] = 0x34;
	//	I2C_WriteBytes(s_stms_out, REG_OUT1, 2);
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: STM8_WriteGPIO
*	功能说明: 设置GPIO输出状态
*	形    参: _pin : 0-7, 8-13
*			  _value : 0,1
*	返 回 值: 无
*********************************************************************************************************
*/
void STM8_WriteGPIO(uint8_t _pin, uint8_t _value)
{
	if (_pin < 8)
	{
		if (_value == 1)
		{
			s_stms_out[0] |= (1 << _pin);
		}
		else
		{
			s_stms_out[0] &= (~(1 << _pin));
		}
	}
	else
	{
		if (_value == 1)
		{
			s_stms_out[1] |= (1 << (_pin - 8));
		}
		else
		{
			s_stms_out[1] &= (~(1 << (_pin - 8)));
		}
	}
	I2C_WriteBytes(s_stms_out, REG_OUT1, 2);
}

/*
*********************************************************************************************************
*	函 数 名: STM8_WriteGPIO
*	功能说明: 设置GPIO输出状态
*	形    参: _ch : 通道1,2
*			  _value : PWM值 (0-4095)
*	返 回 值: 无
*********************************************************************************************************
*/
void STM8_WritePWM(uint8_t _ch, uint16_t _value)
{
	uint8_t buf[2];

	if (_ch == 1)
	{
		s_stms_pwm[0] = _value;
		buf[0] = _value >> 8;
		buf[1] = _value;
		I2C_WriteBytes(buf, REG_PWM1_H, 2);
	}
	else
	{
		s_stms_pwm[1] = _value;
		buf[0] = _value >> 8;
		buf[1] = _value;
		I2C_WriteBytes(buf, REG_PWM2_H, 2);
	}
}

/*
*********************************************************************************************************
*	函 数 名: I2C_ReadBytes
*	功能说明: 从从机指定地址处开始读取若干数据
*	形    参:  _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
static uint8_t I2C_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i;

	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */

	/* 第1步：发起I2C总线启动信号 */
	i2c_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_SendByte(I2C_DEV_ADDR | I2C_WR); /* 此处是写指令 */

	/* 第3步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	if (I2C_ADDR_BYTES == 1)
	{
		i2c_SendByte((uint8_t)_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail; /* EEPROM器件无应答 */
		}
	}
	else
	{
		i2c_SendByte(_usAddress >> 8);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail; /* EEPROM器件无应答 */
		}

		i2c_SendByte(_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail; /* EEPROM器件无应答 */
		}
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	i2c_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_SendByte(I2C_DEV_ADDR | I2C_RD); /* 此处是读指令 */

	/* 第8步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail; /* EEPROM器件无应答 */
	}

	/* 第9步：循环读取数据 */
	for (i = 0; i < _usSize; i++)
	{
		_pReadBuf[i] = i2c_ReadByte(); /* 读1个字节 */

		/* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
		if (i != _usSize - 1)
		{
			i2c_Ack(); /* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
		}
		else
		{
			i2c_NAck(); /* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		}
	}
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 1; /* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: I2C_WriteBytes
*	功能说明: 向从机指定地址写入若干数据，采用页写操作提高写入效率
*	形    参:  _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pWriteBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t I2C_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i, m;
	uint16_t usAddr;

	/*
		写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
		对于24xx02，page size = 8
		简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
		为了提高连续写的效率: 本函数采用page wirte操作。
	*/

	usAddr = _usAddress;
	for (i = 0; i < _usSize; i++)
	{
		/* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
		if ((i == 0))
		{
			/*　第０步：发停止信号，启动内部写操作　*/
			i2c_Stop();

			/* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
				CLK频率为200KHz时，查询次数为30次左右
			*/
			for (m = 0; m < 1000; m++)
			{
				/* 第1步：发起I2C总线启动信号 */
				i2c_Start();

				/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
				i2c_SendByte(I2C_DEV_ADDR | I2C_WR); /* 此处是写指令 */

				/* 第3步：发送一个时钟，判断器件是否正确应答 */
				if (i2c_WaitAck() == 0)
				{
					break;
				}
			}
			if (m == 1000)
			{
				goto cmd_fail; /* EEPROM器件写超时 */
			}

			/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
			if (I2C_ADDR_BYTES == 1)
			{
				i2c_SendByte((uint8_t)usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail; /* EEPROM器件无应答 */
				}
			}
			else
			{
				i2c_SendByte(usAddr >> 8);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail; /* EEPROM器件无应答 */
				}

				i2c_SendByte(usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail; /* EEPROM器件无应答 */
				}
			}
		}

		/* 第6步：开始写入数据 */
		i2c_SendByte(_pWriteBuf[i]);

		/* 第7步：发送ACK */
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail; /* EEPROM器件无应答 */
		}

		usAddr++; /* 地址增1 */
	}

	/* 命令执行成功，发送I2C总线停止信号 */
	i2c_Stop();
	return 1;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 0;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
