/*
*********************************************************************************************************
*
*    模块名称 : 编程器接口
*    文件名称 : prog_if.h
*    版    本 : V1.0
*    说    明 : 头文件
*
*    Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __PROG_IF_H_
#define __PROG_IF_H_

#define DEV_SYS 1
#define DEV_GPIO 2
#define DEV_TIM 3
#define DEV_DAC 4
#define DEV_ADC 5
#define DEV_I2C 6
#define DEV_SPI 7
#define DEV_UART 8
#define DEV_485 9
#define DEV_CAN 10
#define DEV_SWD 11

#define I2C_START (DEV_I2C + 00)
#define I2C_STOP (DEV_I2C + 01)
#define I2C_SEND_BYTE (DEV_I2C + 02)
#define I2C_SEND_BYTES (DEV_I2C + 03)
#define I2C_READ_BYTES (DEV_I2C + 04)

void PG_Poll(void);

void PG_Install(uint16_t _addr, uint8_t *_buf, uint16_t _len, uint16_t _total_len);
uint8_t PG_WaitRunCompleted(uint16_t _usTimeout);

extern uint8_t s_prog_ack_buf[2 * 1024];
extern uint16_t s_prog_ack_len;

#endif
