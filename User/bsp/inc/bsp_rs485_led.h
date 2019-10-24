/*
*********************************************************************************************************
*
*	模块名称 : 安富莱LED-485-XXX系列数码管的驱动程序
*	文件名称 : bsp_rs485_led.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_RS485_LED_H
#define __BSP_RS485_LED_H

#define MODH_RX_SIZE		128
#define MODH_TX_SIZE      128

typedef struct
{
	uint8_t RxBuf[MODH_RX_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t AppRxBuf[MODH_RX_SIZE];
	uint8_t AppRxCount;

	uint32_t Baud;

	uint8_t RspCode;

	uint8_t TxBuf[MODH_TX_SIZE];
	uint8_t TxLen;
	uint8_t fAckOK;		/* 应答OK标志 */
}MODH_T;

extern MODH_T g_tModH;

/* 控制LED-485-xxx 数码管显示模块的API函数 */

void LED485_SetProtRTU(uint8_t _addr);
void LED485_SetProtAscii(uint8_t _addr);
void LED485_DispNumber(uint8_t _addr, int16_t _iNumber);
void LED485_SetDispDot(uint8_t _addr, uint8_t _dot);
void LED485_SetBright(uint8_t _addr, uint8_t _bright);
void LED485_ModifyAddr(uint8_t _addr, uint8_t _NewAddr);
void LED485_DispNumberWithDot(uint8_t _addr, int16_t _iNumber, uint8_t _dot);
void LED485_DispStr(uint8_t _addr, char *_str);

void LED485_TestOk(uint8_t _addr);
void LED485_ReadModel(uint8_t _addr);
void LED485_ReadVersion(uint8_t _addr);
void LED485_ReadBright(uint8_t _addr);
void LED485_SetBrightA(uint8_t _addr, uint8_t _bright);
void LED485_ModifyAddrA(uint8_t _addr, uint8_t _NewAddr);

void LED485_DispNumberA(uint8_t _addr, int16_t _iNumber);
void LED485_DispStrA(uint8_t _addr, char *_str);

void MODH_Poll(void);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
