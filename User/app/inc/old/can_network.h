/*
*********************************************************************************************************
*	                                  
*	模块名称 : CAN网络演示程序。
*	文件名称 : can_network.h
*	版    本 : V1.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-09-01 armfly  ST固件库V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _CAN_NETWORK_H
#define _CAN_NETWORK_H

void can_Init(void);
void can_DeInit(void);

/* 应用层协议 */
void can_LedOn(uint8_t _addr, uint8_t _led_no);
void can_LedOff(uint8_t _addr, uint8_t _led_no);
void can_BeepCtrl(uint8_t _addr, uint8_t _cmd);

void can1_Analyze(void);
void can2_Analyze(void);

#endif


