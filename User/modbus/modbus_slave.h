/*
*********************************************************************************************************
*
*	模块名称 : MODBUS从站通信模块
*	文件名称 : modbus_slave.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __MODBUS_SLAVE_H
#define __MODBUS_SLAVE_H

/* RTU 应答代码 */
#define RSP_OK				0		/* 成功 */
#define RSP_ERR_CMD			0x01	/* 不支持的功能码 */
#define RSP_ERR_REG_ADDR	0x02	/* 寄存器地址错误 */
#define RSP_ERR_VALUE		0x03	/* 数据值域错误 */
#define RSP_ERR_WRITE		0x04	/* 写入失败 */

#define ERR_PACKAGE			0x05	/* 自己定义错误包应答 */

#define RX_BUF_SIZE	     (2 * 1024)
#define TX_BUF_SIZE      (2 * 1024)

typedef struct
{
	uint8_t *RxBuf;
	uint16_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[TX_BUF_SIZE];
	uint16_t TxCount;
	
	/*MODBUS TCP头部*/
	uint8_t TCP_Head[6];
	uint8_t TCP_Flag;
}MODS_T;

/* 传递波形数据的通信结构 60H功能码专用 */
typedef struct
{
	/* 保存PC机的指令参数 */
	uint32_t ChEn;	/* bit0 表示CH1， bit1表示CH2 */
	uint32_t SampleSize;	/* 每个通道样本个数 */
	uint16_t PackageSize;	/* 每通信包样本长度. 单位为1个样本 */
	uint32_t SampleOffset;
	
	/* 通信过程中，控制进度 */
	uint32_t TransPos;		/* 样本缓冲区的当前位置 */
	uint8_t StartTrans;		/* 开始传输波形的标志 */
}MOD_WAVE_T;

void uart_rx_isr(void);		/* 在 stm8s_it.c 中调用 */
uint8_t AnalyzeCmd(uint8_t *_DispBuf);
uint8_t MODS_Poll(uint8_t *_buf, uint16_t _len);
void MODS_SendAckErr(uint8_t _ucErrCode);
void MODS_SendWithCRC(void);
void MODS_SendAckOk(void);

extern MODS_T g_tModS;
extern MOD_WAVE_T g_tModWave;

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
