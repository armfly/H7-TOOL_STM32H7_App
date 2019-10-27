/*
*********************************************************************************************************
*
*	模块名称 : MODSBUS通信程序 （主机）
*	文件名称 : modbus_host.c
*	版    本 : V1.0
*	说    明 : 无线通信程序。通信协议基于MODBUS
*	修改记录 :
*		版本号  日期        作者    说明
*       V1.0   2015-04-18 修改协议
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "param.h"
#include "main.h"
#include "main_run.h"
#include "tcp_modbus_host.h"
#include "tcp_echoclient.h"

/* 保存每个从机的计数器值 */

MODH_T g_tModH;
uint8_t g_modh_timeout = 0;

//static void MODH_RxTimeOut(void);
static void MODH_AnalyzeApp(void);

static void MODH_Read_01H(void);
static void MODH_Read_02H(void);
static void MODH_Read_03H(void);
static void MODH_Read_04H(void);
static void MODH_Read_05H(void);
static void MODH_Read_06H(void);
static void MODH_Read_10H(void);
//static void MODH_Read_30H(void);

#if 0
/*
*********************************************************************************************************
*	函 数 名: MODH_SendPacket
*	功能说明: 发送数据包 COM1口
*	形    参: _buf : 数据缓冲区
*			  _len : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_SendPacket(uint8_t *_buf, uint16_t _len)
{
	RS485_SendBuf1(_buf, _len);
}
#endif

/*
*********************************************************************************************************
*	函 数 名: MODH_SendAckWithCRC
*	功能说明: 发送应答,自动加CRC.  
*	形    参: 无。发送数据在 g_tModH.TxBuf[], [g_tModH.TxCount
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_SendAckWithCRC(void)
{
	uint16_t crc;

	crc = CRC16_Modbus(g_tModH.TxBuf, g_tModH.TxCount);
	g_tModH.TxBuf[g_tModH.TxCount++] = crc >> 8;
	g_tModH.TxBuf[g_tModH.TxCount++] = crc;

	memcpy(g_tClient.TxData, g_tModH.TxBuf, g_tModH.TxCount); /* 要发送的数据放入TCP客户端发送缓冲区 */
	g_tClient.TxCount = g_tModH.TxCount;											/* TCP将要发送的数据长度 */

	//MODH_SendPacket(g_tModH.TxBuf, g_tModH.TxCount);
	tcp_client_usersent(echoclient_pcb); /* 用户发送g_tClient.TxData中的数据 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_AnalyzeApp
*	功能说明: 分析应用层协议。处理应答。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_AnalyzeApp(void)
{
	switch (g_tModH.RxBuf[1]) /* 第2个字节 功能码 */
	{
	case 0x01: /* 读取线圈状态（对应远程开关D01/D02/D03） */
		MODH_Read_01H();
		break;

	case 0x02: /* 读取输入状态（对应X0～X7） */
		MODH_Read_02H();
		break;

	case 0x03: /* 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值 */
		MODH_Read_03H();
		break;

	case 0x04: /* 读取输入寄存器（对应A01/A02） ） */
		MODH_Read_04H();
		break;

	case 0x05: /* 强制单线圈（对应Y1/Y2） */
		MODH_Read_05H();
		break;

	case 0x06: /* 写单个寄存器  */
		MODH_Read_06H();
		break;

	case 0x10: /* 写多个寄存器  */
		MODH_Read_10H();
		break;

	default:
		break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send10H
*	功能说明: 发送10H指令，连续写多个保持寄存器. 最多一次支持23个寄存器。
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数n (每个寄存器2个字节) 值域
*			  _buf : n个寄存器的数据。长度 = 2 * n
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send10H(uint8_t _addr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
	uint16_t i;

	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;			/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x10;			/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;			/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;			/* 寄存器个数 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 2 * _num;	/* 数据字节数 */

	for (i = 0; i < 2 * _num; i++)
	{
		if (g_tModH.TxCount > H_RX_BUF_SIZE - 3)
		{
			return; /* 数据超过缓冲区超度，直接丢弃不发送 */
		}
		g_tModH.TxBuf[g_tModH.TxCount++] = _buf[i]; /* 后面的数据长度 */
	}

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send05H
*	功能说明: 发送05H指令，写强置单线圈
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _value : 寄存器值,2字节
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send05H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;				/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x05;				/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;		/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;				/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8; /* 寄存器值 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;			/* 寄存器值 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */

	g_tModH.fAck05H = 0; /* 如果收到从机的应答，则这个标志会设为1 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send06H
*	功能说明: 发送06H指令，写1个保持寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _value : 寄存器值,2字节
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send06H(uint8_t _addr, uint16_t _reg, uint16_t _value)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;				/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x06;				/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8;		/* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;				/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value >> 8; /* 寄存器值 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _value;			/* 寄存器值 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */

	g_tModH.fAck06H = 0; /* 如果收到从机的应答，则这个标志会设为1 */
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send03H
*	功能说明: 发送03H指令，查询1个或多个保持寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send03H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	g_tModH.TxCount = 0;
	g_tModH.TxBuf[g_tModH.TxCount++] = _addr;			/* 从站地址 */
	g_tModH.TxBuf[g_tModH.TxCount++] = 0x03;			/* 功能码 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _reg;			/* 寄存器编号 低字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
	g_tModH.TxBuf[g_tModH.TxCount++] = _num;			/* 寄存器个数 低字节 */

	MODH_SendAckWithCRC(); /* 发送数据，自动加CRC */
	g_tModH.fAck03H = 0;	 /* 清接收标志 */
	g_tModH.RegNum = _num; /* 寄存器个数 */
	g_tModH.Reg03H = _reg; /* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: MODH_ReciveNew
*	功能说明: 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
*	形    参: 
*	返 回 值: 1 表示有数据
*********************************************************************************************************
*/
void MODH_ReciveNew(uint8_t _data)
{
	/*
		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
		所以就是3.5个字符的时间，波特率高，这个时间间隔就小，波特率低，这个时间间隔相应就大

		4800  = 7.297ms
		9600  = 3.646ms
		19200  = 1.771ms
		38400  = 0.885ms
	*/
	uint32_t timeout;

	g_modh_timeout = 0;
	
	
	timeout = 48000000 / g_tParam.HBaud485;		/* 计算超时时间，单位us 35000000*/
	
	//timeout = 35000000 / g_tParam.Baud485;		/* 计算超时时间，单位us 35000000*/
	
	/* 硬件定时中断，定时精度us 硬件定时器2用于MODBUS从机, 定时器3用于MODBUS从机主机*/
	bsp_StartHardTimer(3, timeout, (void *)MODH_RxTimeOut);

	if (g_tModH.RxCount < H_RX_BUF_SIZE)
	{
		g_tModH.RxBuf[g_tModH.RxCount++] = _data;
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_RxTimeOut
*	功能说明: 超过3.5个字符时间后执行本函数。 设置全局变量 g_rtu_timeout = 1; 通知主程序开始解码。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_RxTimeOut(void)
{
	g_modh_timeout = 1;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: MODH_Poll
*	功能说明: 接收控制器指令. 1ms 响应时间。
*	形    参: 无
*	返 回 值: 0 表示无数据 1表示收到正确命令
*********************************************************************************************************
*/
void MODH_Poll(void)
{
	uint16_t crc1;

	if (g_modh_timeout == 0)
	{
		/* 没有超时，继续接收。不要清零 g_tModH.RxCount */
		return;
	}

	/* 收到命令
		05 06 00 88 04 57 3B70 (8 字节)
			05    :  数码管屏的号站，
			06    :  指令
			00 88 :  数码管屏的显示寄存器
			04 57 :  数据,,,转换成 10 进制是 1111.高位在前,
			3B70  :  二个字节 CRC 码	从05到 57的校验
	*/
	g_modh_timeout = 0;

	if (g_tModH.RxCount < 4)
	{
		goto err_ret;
	}

	/* 计算CRC校验和 */
	crc1 = CRC16_Modbus(g_tModH.RxBuf, g_tModH.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* 分析应用层协议 */
	MODH_AnalyzeApp();

err_ret:
	g_tModH.RxCount = 0; /* 必须清零计数器，方便下次帧同步 */
}

static void MODH_Read_01H(void)
{
	;
}

static void MODH_Read_02H(void)
{
	;
}

static void MODH_Read_04H(void)
{
	;
}

static void MODH_Read_05H(void)
{
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr) //g_tParam.HAddr485
		{
			g_tModH.fAck05H = 1; /* 接收到应答 */
		}
	};
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_06H
*	功能说明: 分析06H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_06H(void)
{
	if (g_tModH.RxCount > 0)
	{
		if (g_tModH.RxBuf[0] == SlaveAddr) //g_tParam.HAddr485
		{
			g_tModH.fAck06H = 1; /* 接收到应答 */
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_03H
*	功能说明: 分析03H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Read_03H(void)
{
#if 0
	uint8_t bytes;
	uint8_t *p;
	
	if (g_tModH.RxCount > 0)
	{
		//BEEP_KeyTone();

		bytes = g_tModH.RxBuf[2];	/* 数据长度 字节数 */				
		switch (g_tModH.Reg03H)
		{
			case REG_P01:
				if (bytes == 32)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.P01 = BEBufToUint16(p); p += 2;	/* 寄存器 */	
					g_tVar.P02 = BEBufToUint16(p); p += 2;	/* 寄存器 */	
					g_tVar.P03 = BEBufToUint16(p); p += 2;
					g_tVar.P04 = BEBufToUint16(p); p += 2;
					g_tVar.P05 = BEBufToUint16(p); p += 2;
					g_tVar.P06 = BEBufToUint16(p); p += 2;
					g_tVar.P07 = BEBufToUint16(p); p += 2;
					g_tVar.P08 = BEBufToUint16(p); p += 2;
					g_tVar.P09 = BEBufToUint16(p); p += 2;
					
					g_tVar.P10 = BEBufToUint16(p); p += 2;	
					g_tVar.P11 = BEBufToUint16(p); p += 2;
					g_tVar.P12 = BEBufToUint16(p); p += 2;
					g_tVar.P13 = BEBufToUint16(p); p += 2;
					g_tVar.P14 = BEBufToUint16(p); p += 2;
					g_tVar.P15 = BEBufToUint16(p); p += 2;
					g_tVar.P16 = BEBufToUint16(p); p += 2;
				//	bsp_PutMsg(MSG_READ_FINISH, 0);			/* 开机时读取一次数据，发送消息表示读取完毕 */
					g_tModH.fAck03H = 1;
				}
				break;
			
			case REG_P01 + 16:
				if (bytes == 32)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.P17 = BEBufToUint16(p); p += 2;
					g_tVar.P18 = BEBufToUint16(p); p += 2;
					g_tVar.P19 = BEBufToUint16(p); p += 2;
						
					g_tVar.P20 = BEBufToUint16(p); p += 2;
					g_tVar.P21 = BEBufToUint16(p); p += 2;
					g_tVar.P22 = BEBufToUint16(p); p += 2;
					g_tVar.P23 = BEBufToUint16(p); p += 2;
					g_tVar.P24 = BEBufToUint16(p); p += 2;
					g_tVar.P25 = BEBufToUint16(p); p += 2;
					g_tVar.P26 = BEBufToUint16(p); p += 2;
					g_tVar.P27 = BEBufToUint16(p); p += 2;
					g_tVar.P28 = BEBufToUint16(p); p += 2;
					g_tVar.P29 = BEBufToUint16(p); p += 2;

					g_tVar.P30 = BEBufToUint16(p); p += 2;
					g_tVar.P31 = BEBufToUint16(p); p += 2;
					g_tVar.P32 = BEBufToUint16(p); p += 2;

					g_tModH.fAck03H = 2;
				}
				break;
				
			case REG_P01 + 32:
				if (bytes == 32)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.P33 = BEBufToUint16(p); p += 2;
					g_tVar.P34 = BEBufToUint16(p); p += 2;
					g_tVar.P35 = BEBufToUint16(p); p += 2;
					g_tVar.P36 = BEBufToUint16(p); p += 2;
					g_tVar.P37 = BEBufToUint16(p); p += 2;
					g_tVar.P38 = BEBufToUint16(p); p += 2;
					g_tVar.P39 = BEBufToUint16(p); p += 2;

					g_tVar.P40 = BEBufToUint16(p); p += 2;
					g_tVar.P41 = BEBufToUint16(p); p += 2;
					g_tVar.P42 = BEBufToUint16(p); p += 2;
					g_tVar.P43 = BEBufToUint16(p); p += 2;
					g_tVar.P44 = BEBufToUint16(p); p += 2;
					g_tVar.P45 = BEBufToUint16(p); p += 2;
					g_tVar.P46 = BEBufToUint16(p); p += 2;
					g_tVar.P47 = BEBufToUint16(p); p += 2;
					g_tVar.P48 = BEBufToUint16(p); p += 2;

					g_tModH.fAck03H = 3;
				}
				break;
		
			case REG_P01 + 48:
				if (bytes == 32)
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.P49 = BEBufToUint16(p); p += 2;	

					g_tVar.P50 = BEBufToUint16(p); p += 2;
					g_tVar.P51 = BEBufToUint16(p); p += 2;
					g_tVar.P52 = BEBufToUint16(p); p += 2;
					g_tVar.P53 = BEBufToUint16(p); p += 2;
					g_tVar.P54 = BEBufToUint16(p); p += 2;
					g_tVar.P55 = BEBufToUint16(p); p += 2;
					g_tVar.P56 = BEBufToUint16(p); p += 2;
					g_tVar.P57 = BEBufToUint16(p); p += 2;
					g_tVar.P58 = BEBufToUint16(p); p += 2;
					g_tVar.P59 = BEBufToUint16(p); p += 2;
						
					g_tVar.P60 = BEBufToUint16(p); p += 2;
					g_tVar.P61 = BEBufToUint16(p); p += 2;
					g_tVar.P62 = BEBufToUint16(p); p += 2;
					g_tVar.P63 = BEBufToUint16(p); p += 2;
					g_tVar.P64 = BEBufToUint16(p); p += 2;

					g_tModH.fAck03H = 4;
				}
				break;	
				
			case REG_H40:				/* 总运行次数 */
				if (bytes == 28)		/* 扫描读 */
				{
					p = &g_tModH.RxBuf[3];	
					
					g_tVar.H40 = BEBufToUint32(p); p += 4;	
					g_tVar.H42 = BEBufToUint32(p); p += 4;
					g_tVar.H44 = BEBufToUint16(p); p += 2;
					g_tVar.H45 = BEBufToUint16(p); p += 2;
					g_tVar.H46 = BEBufToUint16(p); p += 2;
					g_tVar.H47 = BEBufToUint16(p); p += 2;
					g_tVar.H48 = BEBufToUint16(p); p += 2;
					g_tVar.H49 = BEBufToUint16(p); p += 2;
					g_tVar.H4A = BEBufToUint16(p); p += 2;
					g_tVar.H4B = BEBufToUint16(p); p += 2;
					
					g_tVar.H4C = BEBufToUint16(p); p += 2;	
					g_tVar.H4D = BEBufToUint16(p); p += 2;

					g_tModH.fAck03H = 5;
				}

#if 0
				if (bytes == 4)
				{
					p = &g_tModH.RxBuf[3];	
					g_tVar.H40 = BEBufToUint32(p);
					g_tModH.fAck03H = 1;
				}
#endif
				break;
				
			case REG_H44:				/* 故障内容,查询到主板上报的故障时，自动测试界面返回主菜单 */
				if (bytes == 2)
				{
					p = &g_tModH.RxBuf[3];	
					g_tVar.H44 = BEBufToUint16(p);
					
					g_tModH.fAck03H = 1;
				}
				break;

#if 0			
			case REG_H4A:				/* 输入状态查询 */
				if (bytes == 2)
				{
					p = &g_tModH.RxBuf[3];	
					g_tVar.H4A = BEBufToUint16(p);
					g_tModH.fAck03H = 1;
				}
				break;

			case REG_H42:			 	/* 维护后运行的次数 */
				if (bytes == 4)
				{
					p = &g_tModH.RxBuf[3];	
					g_tVar.H42 = BEBufToUint32(p);
				}
				break;
			
			case REG_H47:				/* 系统查询1 */
				break;
				
		    case REG_H49:				/* 系统查询2 */
				break;
			
			case REG_H48:				/* 系统查询3 */
				break;
			
			case REG_H4D:				/* 系统查询4 */
				break;
			
			case REG_H4C:				/* 系统查询5 */
				break;
			
			case REG_P16:				/* 系统查询6 */
				break;
			
			case REG_P18:				/* 系统查询7 */
				break;
			
			case REG_H4B:				/* 系统查询8 */
				break;
			
			case REG_P63: 				/* 系统版本 */
				if (bytes == 4)
				{
					p = &g_tModH.RxBuf[3];	
					g_tVar.P63 = BEBufToUint16(p); p += 2;
					g_tVar.P64 = BEBufToUint16(p);
				}
#endif
		}
	}
#endif
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_10H
*	功能说明: 分析10H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Read_10H(void)
{
	/*
		10H指令的应答:
			从机地址                11
			功能码                  10
			寄存器起始地址高字节	00
			寄存器起始地址低字节    01
			寄存器数量高字节        00
			寄存器数量低字节        02
			CRC校验高字节           12
			CRC校验低字节           98
	*/
	//uint8_t addr;
	//uint8_t bytes;
	//uint8_t reg;

	if (g_tModH.RxCount > 0)
	{
		//		BEEP_KeyTone();

/* 发送消息. 消息体是寄存器地址和数量 */
#if 0
		{
			uint8_t buf[4];
			
			buf[0] = g_tModH.RxBuf[0];	/* 从机地址 */
			buf[1] = g_tModH.RxBuf[5];	/* 寄存器长度 */
			buf[2] = g_tModH.RxBuf[2];	/* 寄存器地址高16位 */
			buf[3] = g_tModH.RxBuf[3];	/* 寄存器地址低16位 */
			bsp_PutMsg(MSG_RX_10H, (uint32_t)buf);
		}
#endif
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_WriteParam
*	功能说明: 单个参数. 通过发送06H指令实现，发送之后，等待从机应答。循环10次写命令
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
extern uint8_t g_fTcpState;
uint8_t MODH_WriteParam(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < 3; i++)
	{
		MODH_Send06H(SLAVE_ADDR, _reg, _value);
		/* 将TCP状态标志设置为非接收到数据状态 */
		g_tClient.TcpState = g_fTcpState; /* 2016-10-17 add by xd */
		time1 = bsp_GetRunTime();					/* 记录命令发送的时刻 */

		while (1)
		{
			bsp_Idle();

			/* 超时大于 150ms ，则认为异常,  100ms有时会超时，数据没有接收到，PC软件应答时间间隔几十到一百多浮动 */
			if (bsp_CheckRunTime(time1) > 200) //100
			{
				break;
			}

			if (g_tClient.TcpState == 2)
			{
				break;
			}
		}

		if (g_tClient.TcpState == 2)
		{
			break;
		}
	}

	if (g_tClient.TcpState == 2) /* 接收到数据 */
	{
		return 1;
	}
	else
	{
		return 0; /* 通信超时了 */
	}
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: MODH_Read_30H
*	功能说明: 分析30H指令的应答数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Read_30H(void)
{
	uint8_t addr;
	uint8_t bytes;
	
	if (g_tModH.RxCount > 0)
	{
		//BEEP_KeyTone();
		
		addr = g_tModH.RxBuf[0];
		bytes = g_tModH.RxBuf[2];
#if 0		
		if (g_tModH.Reg30H == SREG_STATUS)	/* 主机查询状态字 - 实时产量 - 当班员工工号 */
		{		
			if ((addr < DEV_NUM_MAX) && (addr > 0) && (addr != ADDR_FF) && (bytes >= 10))
			{
				addr--;

				g_tVar.Dev[addr].CommErr = 0;	/* 通信错误清零 */
				
				g_tVar.Dev[addr].Status = BEBufToUint16(&g_tModH.RxBuf[3]);

				if (bytes >= 4)
				{
					g_tVar.Dev[addr].CountNow = BEBufToUint16(&g_tModH.RxBuf[5]);
				}
				if (bytes >= 6)
				{			
					g_tVar.Dev[addr].GH[0] = BEBufToUint16(&g_tModH.RxBuf[7]);
				}
				if (bytes >= 8)
				{				
					g_tVar.Dev[addr].GH[1] = BEBufToUint16(&g_tModH.RxBuf[9]);
				}
				if (bytes >= 10)		
				{
					g_tVar.Dev[addr].GH[2] = BEBufToUint16(&g_tModH.RxBuf[11]);
				}
				
				bsp_PutMsg(MSG_433M_OK, addr + 1);	/* 433M 通信OK, 通知监控界面刷新图标 */		
			}
		}
#endif
	}
}

/*
*********************************************************************************************************
*	函 数 名: MODH_WriteParam_05H
*	功能说明: 单个参数. 通过发送05H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_WriteParam_05H(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < 10; i++)
	{
		MODH_Send05H (SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */
		
		g_LastTime = bsp_GetRunTime();			/* 300ms */
		while (1)
		{
			bsp_Idle();
			
			/* 超时大于 150ms ，则认为异常 */
			if (bsp_CheckRunTime(time1) > 150)		//100
			{
				g_LastTime = bsp_GetRunTime();
				break;	/* 通信超时了 */
			}
			
			if (g_tModH.fAck05H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck05H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck05H == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/* 读一个寄存器 */
uint8_t MODH_ReadParam_03H(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;
	
	for (i = 0; i < 10; i++)
	{
		MODH_Send03H (SlaveAddr, _reg, _num);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */
		
		g_LastTime = bsp_GetRunTime();			/* 300ms */
		while (1)
		{
			bsp_Idle();
			
			/* 超时大于 150ms ，则认为异常 */
			if (bsp_CheckRunTime(time1) > 150)		//100
			{
				g_LastTime = bsp_GetRunTime();
				break;	/* 通信超时了 */
			}
			
			if (g_tModH.fAck03H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck03H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck03H == 0)
	{
		return 0;
	}
	else 
	{
		return 1;	/* 写入03H参数成功 */
	}
}

/* 读64个寄存器的参数,读10次 */
uint8_t MODH_ReadParam_03H_16(uint16_t _reg, uint16_t _num)
{
	int32_t time1;
	uint8_t i;
	uint8_t State = 0;
	
	while (1)
	{	
		for (i = 0; i < 10; i++)
		{
			/* 主板的地址就定0x55,最多连续读16个参数（32字节）*/
			MODH_Send03H (SlaveAddr, _reg + State * _num, _num);		/* 发送查询命令 */
			
			time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */
			
			g_LastTime = bsp_GetRunTime();			/* 300ms */
			while (1)
			{
				bsp_Idle();
				
				/* 超时大于 200ms ，则认为异常 */
				if (bsp_CheckRunTime(time1) > 200)		//200
				{
					g_LastTime = bsp_GetRunTime();
					break;	/* 通信超时了 */
				}
				
				if (g_tModH.fAck03H > 0)
				{
					State++;
					break;
				}
			}
			
			if (g_tModH.fAck03H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck03H == 0)
		{
			return 0;
		}
		
		if (State == 4)
		{
			return 1;
		}
	}
}

/* 轮询读14个参数 */
uint8_t MODH_CycleRead_03H(uint16_t _reg)
{
	int32_t time1;
	
	/* 主板的地址就定0x55,最多连续读16个参数（32字节）*/
	MODH_Send03H (SlaveAddr, _reg, 14);		/* 发送查询命令 */
	
	time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */
	
	g_LastTime = bsp_GetRunTime();		/* 300ms */
	while (1)
	{
		bsp_Idle();
		
		/* 超时大于 200ms ，则认为异常 */
		if (bsp_CheckRunTime(time1) > 200)	
		{
			g_LastTime = bsp_GetRunTime();
			return 0;	/* 通信超时了 */
		}
		
		if (g_tModH.fAck03H > 0)
		{
			break;
		}
	}
	
	return 1;
}


/*
*********************************************************************************************************
*	函 数 名: MODH_AllowRun_05H
*	功能说明: 允许禁止运行专用05指令，不调用bsp_Idle(); 调用解析命令函数
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t MODH_AllowRun_05H(uint16_t _reg, uint16_t _value)
{
	int32_t time1;
	uint8_t i;

	for (i = 0; i < 10; i++)
	{
		MODH_Send05H (SlaveAddr, _reg, _value);
		time1 = bsp_GetRunTime();	/* 记录命令发送的时刻 */
	
		g_LastTime = bsp_GetRunTime();
		while (1)
		{
			//bsp_Idle();
			MODH_Poll();
			MODS_Poll();
			
			/* 超时大于 150ms ，则认为异常 */
			if (bsp_CheckRunTime(time1) > 150)		//100
			{
				g_LastTime = bsp_GetRunTime();
				break;	/* 通信超时了 */
			}
			
			if (g_tModH.fAck05H > 0)
			{
				break;
			}
		}
		
		if (g_tModH.fAck05H > 0)
		{
			break;
		}
	}
	
	if (g_tModH.fAck05H == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
