/*
*********************************************************************************************************
*
*	模块名称 : GT911电容触摸芯片驱动程序
*	文件名称 : bsp_GT911.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2017-2025, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_GT911_H
#define _BSP_GT911_H

#define GT911_I2C_ADDR1	0xBA

typedef struct
{
	uint8_t Enable;
	uint8_t TimerCount;
	uint8_t i2c_addr;
	
	uint8_t TouchpointFlag;
	uint8_t Touchkeystate;

	uint16_t X0;
	uint16_t Y0;
	uint16_t P0;

	uint16_t X1;
	uint16_t Y1;
	uint16_t P1;

	uint16_t X2;
	uint16_t Y2;
	uint16_t P2;

	uint16_t X3;
	uint16_t Y3;
	uint16_t P3;

	uint16_t X4;
	uint16_t Y4;
	uint16_t P4;
}GT911_T;

void GT911_InitHard(void);
uint32_t GT911_ReadID(void);
uint16_t GT911_ReadVersion(void);
void GT911_Scan(void);
void GT911_Timer1ms(void);


extern GT911_T g_GT911;

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
