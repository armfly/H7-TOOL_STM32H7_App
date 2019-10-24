/*
*********************************************************************************************************
*
*	模块名称 : 在线升级固件。
*	文件名称 : modbus_iap.c
*	版    本 : V1.0
*	说    明 : 
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"
#include "modbus_iap.h"
#include "param.h"
#include "modbus_slave.h"

uint32_t g_FlashAddr = 0;			/* CPU当前写入的地址 */
static uint8_t s_Databuf[1024];	/* 升级程序buf。接收到1024个字节一次性写入cpu flash */
uint32_t g_DataLen = 0;			/* 最后一包包长 */

uint8_t g_Upgrade = 0;
uint8_t g_Erase = 0;			/* APP应用区代码擦除标志 */

/*
*********************************************************************************************************
*	函 数 名: WriteRegValue
*	功能说明: 读取保持寄存器的值
*	形    参: reg_addr 寄存器地址
*			  reg_value 寄存器值
*	返 回 值: 1表示OK 0表示错误
*********************************************************************************************************
*/
extern uint8_t g_fReset;
extern uint8_t g_fBaud;
uint8_t IAP_Write06H(uint16_t reg_addr, uint16_t reg_value)
{
	switch (reg_addr)
	{
		case BOOT_UPGRADE_FLAG:			/* 程序升级标志寄存器 */
			g_Upgrade = reg_value;
			if (reg_value == 0)			/* 升级结束 */
			{
				if (bsp_WriteCpuFlash(g_FlashAddr, s_Databuf, g_DataLen) == 0)		/* 写最后一包数据(先读取，组成新的1K数据，再写入) */
				{
					//DispMessage("程序升级成功！");
					g_tParam.UpgradeFlag = 0x55AAA55A;
					return 1;
				}
				else
				{
					return 0;
				}
			}
			break;
		
		case BOOT_BAUD:					/* 波特率寄存器 */	
			break;
			
		case SYS_RESET:					/* 系统复位 */
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_3, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_4, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_5, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_6, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_7, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_8, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_9, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_10, VoltageRange_3);
//			FLASH_EraseSector(ADDR_FLASH_SECTOR_11, VoltageRange_3);
			break;
		
		default:
			return 0;		/* 参数异常，返回 0 */
	}

	return 1;		/* 成功 */
}

/*
*********************************************************************************************************
*	函 数 名: MODS_15H
*	功能说明: 写文件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern uint8_t g_Erase;
void MODS_15H(void)
{
	/*
		主机发送:
			11 从机地址
			15 功能码
			00 请求数据长度
			01 子请求x，参考类型
			00 子请求x，文件号
			01 子请求x，记录号
			9A 子请求x，记录长度
			9B 子请求x，记录数据
			18 校验高字节
			FC 校验低字节
	
		从机响应:
			11 从机地址
			15 功能码
			00 请求数据长度
			01 子请求x，参考类型
			00 子请求x，文件号
			01 子请求x，记录号
			9A 子请求x，记录长度
			9B 子请求x，记录数据
			18 校验高字节
			FC 校验低字节
*/
	uint8_t i;
//	uint8_t DateLen;		/* 请求数据长度 */
//	uint8_t Type;			/* 参考类型 */
//	uint16_t FileID;		/* 文件号 */
	uint16_t RecordID;		/* 记录号 */
	uint16_t RecordLen;		/* 记录长度 */
	uint32_t Packet;		/* 第几包数据 */
	static uint16_t s_LenBak;	/* 记录之前的数据长度，如果长度与之前的不同，则认为是最后一包数据,需要写入 */
	uint32_t Cpu_Offset;	/* CPU地址偏移 */
//	char buf[50];
	
	g_tModS.RspCode = RSP_OK;	
	
//	DateLen = g_tModS.RxBuf[2];
//	Type = g_tModS.RxBuf[3];
//	FileID = BEBufToUint16(&g_tModS.RxBuf[4]); 		/* 子请求x，文件号 */
	RecordID = BEBufToUint16(&g_tModS.RxBuf[6]); 		/* 子请求x，记录号 */
	RecordLen = BEBufToUint16(&g_tModS.RxBuf[8]); 	/* 子请求x，记录长度 */
	
	if (RecordID == 0)				/* 第一包数据，就把flash写入地址设为基地址,同时擦除应用区代码 */
	{
		//g_FlashAddr = APPLICATION_ADDRESS;
		g_Erase = 1;
		s_LenBak = RecordLen;		/* 第1包的数据长度，认为是收到每包的长度 */
	}
	
	Packet = RecordID + 1;
	Cpu_Offset = RecordID * s_LenBak / 1024 * 1024;			/* CPU写入的偏移地址 */
	g_FlashAddr = APP_BUF_ADDR + Cpu_Offset;			/* 下一包数据写入的位置，1024的整数倍 */
	
	memcpy(&s_Databuf[(RecordID * s_LenBak) % 1024], &g_tModS.RxBuf[10], RecordLen);		/* 组成1K数据再写入CPU flash */
		
	if ((Packet * s_LenBak) % 1024 != 0)					/* 判断当前数据包是否满足1K的整数倍 */
	{	
		g_DataLen = ((RecordID * s_LenBak) % 1024) + RecordLen;		/* 记录当前需要写入的包长 */
	}
	else													/* 满足1K的整数倍，此时才开始将1K数据写入CPU flash */
	{	
		if (bsp_WriteCpuFlash(g_FlashAddr, s_Databuf, 1024) == 0)		/* 每次写入1024个字节 */
		{
//			sprintf(buf, "进度：%d%%", RecordID * s_LenBak * 100 % 1024);
//			DispMessage(buf);
			//DispMessage("程序升级中...");
			g_tModS.RspCode = RSP_OK;
		}
		else
		{
			g_tModS.RspCode = RSP_ERR_WRITE;				/* 写入失败 */
			goto err_ret;
		}
	}

	
err_ret:
	if (g_tModS.RspCode == RSP_OK)			/* 正确应答 */
	{
		g_tModS.TxCount = 0;
		for (i = 0; i < 10; i++)
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[i];	/* 应答数据包 */
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);	/* 告诉主机命令错误 */
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/


