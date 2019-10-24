/*
*********************************************************************************************************
*
*	模块名称 : STM8 GPIO芯片驱动
*	文件名称 : bsp_stm8_gpio.h
*	版    本 : V2.0
*	说    明 : 头文件
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _BSP_I2C_LED8_H
#define _BSP_I2C_LED8_H

#define I2C_DEV_ADDR			0x94		/* 设备地址 */
#define I2C_ADDR_BYTES			1			/* 地址字节个数 */

/* 写单个 */
#define REG_ID					 0x00        /* 芯片ID, 固定值 0x75 */
#define REG_SOFTVER              0x01        /* 软件版本 */
#define REG_CFG1				 0x02		 /* GPIO 方向配置寄存器, 7：0  */
#define REG_CFG2				 0x03		 /* GPIO 方向配置寄存器, 10：8  */
#define REG_IN1					 0x04		 /* GPIO 输入状态寄存器, 7：0  */
#define REG_IN2					 0x05		 /* GPIO 输入状态寄存器, 10：8  */
#define REG_OUT1				 0x06		 /* GPIO 输出状态寄存器, 7：0  */
#define REG_OUT2				 0x07		 /* GPIO 输出状态寄存器, 10：8  */
#define REG_PWM1_H				 0x08		 /* PWM1占空比高字节 */
#define REG_PWM1_L				 0x09		 /* PWM1占空比低字节 */
#define REG_PWM2_H				 0x0A		 /* PWM2占空比高字节 */
#define REG_PWM2_L				 0x0B		 /* PWM2占空比低字节 */

uint8_t STM8_InitHard(void);
void STM8_WriteGPIO(uint8_t _pin, uint8_t _value);

#endif


