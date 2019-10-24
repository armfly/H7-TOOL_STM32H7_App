/*
*********************************************************************************************************
*
*	模块名称 : GT811电容触摸芯片驱动程序
*	文件名称 : bsp_ct811.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_GT811_H
#define _BSP_GT811_H

/*
	GT811 的从设备地址有三组可选，以方便主控调配。
	三组地址分别为：0xBA/0xBB、0x6E/0x6F和0x28/0x29
	
	由Sensor ID 引脚的状态决定.
	
	Sensor ID = NC   时，模组1   安富莱用于 800 * 480 
	Sensor ID = VDD  时，模组2   
	Sensor ID = GND  时，模组3   安富莱用于 1024 * 600 
*/
#define GT811_I2C_ADDR1	0xBA
#define GT811_I2C_ADDR2	0x6E
#define GT811_I2C_ADDR3	0x28

typedef struct
{
	uint8_t Enable;
	uint8_t TimerCount;
	uint8_t i2c_addr;
	
	uint8_t TouchpointFlag;
	uint8_t Touchkeystate;

	uint16_t X0;
	uint16_t Y0;
	uint8_t P0;

	uint16_t X1;
	uint16_t Y1;
	uint8_t P1;

	uint16_t X2;
	uint16_t Y2;
	uint8_t P2;

	uint16_t X3;
	uint16_t Y3;
	uint8_t P3;

	uint16_t X4;
	uint16_t Y4;
	uint8_t P4;
}GT811_T;

void GT811_InitHard(void);
uint16_t GT811_ReadVersion(void);
void GT811_Scan(void);
void GT811_Timer1ms(void);
uint8_t GT811_ReadSensorID(void);

extern GT811_T g_GT811;

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
