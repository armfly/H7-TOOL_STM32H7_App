/*
*********************************************************************************************************
*
*	模块名称 : RS485 MODEBUS 通信模块
*	文件名称 : modbus_rs485.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __MODBUS_IAP_H_
#define __MODBUS_IAP_H_


/* 06H boot波特率寄存器 和 程序升级寄存器 */
#define SYS_RESET			0x9100
#define BOOT_BAUD			0x9101
#define BOOT_UPGRADE_FLAG	0x9102

void MODS_15H(void);
uint8_t IAP_Write06H(uint16_t reg_addr, uint16_t reg_value);

#endif


