/*
*********************************************************************************************************
*
*	模块名称 : GPS定位模块驱动程序
*	文件名称 : bsp_uart_gps.c
*	版    本 : V1.1
*	说    明 : 解码GPS协议 NMEA-0183协议, 安富莱原创
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*		V1.1    2014-02-04 armfly  增加全局标志表示 GPS模块串口正常
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* 定义下面这个宏，表示将GPS模块的数据转发到COM1口，便于通过电脑观察原始数据 */
//#define DEBUG_GPS_TO_COM1

void Analyze0183(uint8_t *_ucaBuf, uint16_t _usLen);
int32_t StrToInt(char *_pStr);
int32_t StrToIntFix(char *_pStr, uint8_t _ucLen);
void HexToAscii(uint8_t *_ucpHex, uint8_t *_ucpAscII, uint8_t _ucLenasc);

GPS_T g_tGPS;

/*
*********************************************************************************************************
*	函 数 名: bsp_InitGPS
*	功能说明: 配置GPS串口
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitGPS(void)
{
	/*
		GPS 模块使用 UART 串口发送定位数据至 STM32, 每秒钟发送一组数据

		因此，只需要配置串口即可。 bsp_uart_fifo.c 中已经配置好串口，此处不必再配置
	*/

	g_tGPS.PositionOk = 0;  /* 数据设置为无效 */
	g_tGPS.TimeOk = 0;  /* 数据设置为无效 */
	
	g_tGPS.UartOk = 0;	/* 串口通信正常的标志, 如果以后收到了校验合格的命令串则设置为1 */
}

/*
*********************************************************************************************************
*	函 数 名: gps_pro
*	功能说明: 轮询GPS数据包。插入到主程序中执行即可。分析结果存放在全局变量 g_tGPS
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void gps_pro(void)
{
	uint8_t ucData;
	static uint8_t ucGpsHead = 0;
	static uint8_t ucaGpsBuf[512];
	static uint16_t usGpsPos = 0;

	/* 从 GPS模块串口读取1个字节 comGetChar() 函数由 bsp_uart_fifo.c 实现 */
	while (1)
	{
		if (comGetChar(COM2, &ucData))
		{
			#ifdef DEBUG_GPS_TO_COM1
				/* 将收到的GPS模块数据按原样 打印到COM1 口，便于跟踪调试 */
				comSendChar(COM1, ucData);
			#endif

			if (ucGpsHead == 0)
			{
				if (ucData == '$')
				{
					ucGpsHead = 1;
					usGpsPos = 0;
				}
			}
			else
			{
				if (usGpsPos < sizeof(ucaGpsBuf))
				{
					ucaGpsBuf[usGpsPos++] = ucData;

					if ((ucData == '\r') || (ucData == '\n'))
					{
						Analyze0183(ucaGpsBuf, usGpsPos-1);
						ucGpsHead = 0;
						
						g_tGPS.UartOk = 1;	/* 接收到正确的命令 */
					}
				}
				else
				{
					ucGpsHead = 0;
				}

			}

			continue;	/* 可能还有数据，继续分析 */
		}

		break;	/* 分析完毕，退出函数 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: CheckXor
*	功能说明: 检查0183数据包的校验和是否正确
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: TRUE 或 FALSE.
*********************************************************************************************************
*/
uint8_t CheckXor(uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint8_t ucXorSum;
	uint8_t ucaBuf[2];
	uint16_t i;

	if (_usLen < 3)
	{
		return FALSE;
	}

	/* 如果没有校验字节，也认为出错 */
	if (_ucaBuf[_usLen - 3] != '*')
	{
		return FALSE;
	}


	/* 不允许出现非ASCII字符 */
	for (i = 0; i < _usLen - 3; i++)
	{
		if ((_ucaBuf[i] & 0x80) || (_ucaBuf[i] == 0))
		{
			return FALSE;
		}
	}

	ucXorSum = _ucaBuf[0];
	for (i = 1; i < _usLen - 3; i++)
	{
		ucXorSum = ucXorSum ^ _ucaBuf[i];
	}

	HexToAscii(&ucXorSum, ucaBuf, 2);

	if (memcmp(&_ucaBuf[_usLen - 2], ucaBuf, 2) == 0)
	{
		return TRUE;
	}

	return FALSE;
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPGGA
*	功能说明: 分析0183数据包中的 GPGGA 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F
字段0：$GPGGA，语句ID，表明该语句为Global Positioning System Fix Data（GGA）GPS定位信息
字段1：UTC 时间，hhmmss.sss，时分秒格式
字段2：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段3：纬度N（北纬）或S（南纬）
字段4：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段5：经度E（东经）或W（西经）
字段6：GPS状态，0=未定位，1=非差分定位，2=差分定位，3=无效PPS，6=正在估算
字段7：正在使用的卫星数量（00 - 12）（前导位数不足则补0）
字段8：HDOP水平精度因子（0.5 - 99.9）
字段9：海拔高度（-9999.9 - 99999.9）
字段10：地球椭球面相对大地水准面的高度
字段11：差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
字段12：差分站ID号0000 - 1023（前导位数不足则补0，如果不是差分定位将为空）
字段13：校验值
*/
void gpsGPGGA(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 UTC 时间，hhmmss.sss，时分秒格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Hour = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Min = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Sec = StrToIntFix(p, 2);
	p += 2;

	/* 字段2 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段3 纬度N（北纬）或S（南纬） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'S')
	{
		g_tGPS.NS = 'S';
	}
	else if (*p == 'N')
	{
		g_tGPS.NS = 'N';
	}
	else
	{
		return;
	}

	/* 字段4  经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.JingDu_Du = StrToIntFix(p, 3);
	p += 3;
	g_tGPS.JingDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段5 经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'E')
	{
		g_tGPS.EW = 'E';
	}
	else if (*p == 'W')
	{
		g_tGPS.EW = 'W';
	}

	/* 字段6 GPS状态，0=未定位，1=非差分定位，2=差分定位，3=无效PPS，6=正在估算 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if ((*p == '1') || (*p == '2'))
	{
		g_tGPS.PositionOk = 1;
	}
	else
	{
		g_tGPS.PositionOk = 0;
	}

	/* 字段7：正在使用的卫星数量（00 - 12）（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.ViewNumber = StrToInt(p);
	p += 2;

	/* 字段8：HDOP水平精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.HDOP = StrToInt(p);

	/* 字段9：海拔高度（-9999.9 - 99999.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Altitude = StrToInt(p);

	/* 后面的字段信息丢弃 */
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPGSA
*	功能说明: 分析0183数据包中的 GPGSA 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A
字段0：$GPGSA，语句ID，表明该语句为GPS DOP and Active Satellites（GSA）当前卫星信息
字段1：定位模式，A=自动手动2D/3D，M=手动2D/3D
字段2：定位类型，1=未定位，2=2D定位，3=3D定位
字段3：PRN码（伪随机噪声码），第1信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段4：PRN码（伪随机噪声码），第2信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段5：PRN码（伪随机噪声码），第3信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段6：PRN码（伪随机噪声码），第4信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段7：PRN码（伪随机噪声码），第5信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段8：PRN码（伪随机噪声码），第6信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段9：PRN码（伪随机噪声码），第7信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段10：PRN码（伪随机噪声码），第8信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段11：PRN码（伪随机噪声码），第9信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段12：PRN码（伪随机噪声码），第10信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段13：PRN码（伪随机噪声码），第11信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段14：PRN码（伪随机噪声码），第12信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段15：PDOP综合位置精度因子（0.5 - 99.9）
字段16：HDOP水平精度因子（0.5 - 99.9）
字段17：VDOP垂直精度因子（0.5 - 99.9）
字段18：校验值
*/
void gpsGPGSA(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;
	uint8_t i;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 定位模式，A=自动手动2D/3D，M=手动2D/3D */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.ModeAM = *p;

	/* 字段2 定位类型，1=未定位，2=2D定位，3=3D定位 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Mode2D3D = *p;

	/* 字段3 - 字段14 第1-12信道正在使用的卫星PRN码编号 */
	for (i = 0; i < 12; i++)
	{
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.SateID[i] = StrToInt(p);
	}

	/* 字段15：PDOP综合位置精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.PDOP = StrToInt(p);

	/* 字段16：HDOP水平精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.HDOP = StrToInt(p);

	/* 字段17：VDOP垂直精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.VDOP = StrToInt(p);
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPGSV
*	功能说明: 分析0183数据包中的 GPGSV 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70

$GPGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
$GPGSV,2,2,07,09,23,313,42,04,19,159,41,15,12,041,42*41

字段0：$GPGSV，语句ID，表明该语句为GPS Satellites in View（GSV）可见卫星信息
字段1：本次GSV语句的总数目（1 - 3）
字段2：本条GSV语句是本次GSV语句的第几条（1 - 3）
字段3：当前可见卫星总数（00 - 12）（前导位数不足则补0）

字段4：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段5：卫星仰角（00 - 90）度（前导位数不足则补0）
字段6：卫星方位角（00 - 359）度（前导位数不足则补0）
字段7：信噪比（00－99）dbHz

字段8：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段9：卫星仰角（00 - 90）度（前导位数不足则补0）
字段10：卫星方位角（00 - 359）度（前导位数不足则补0）
字段11：信噪比（00－99）dbHz

字段12：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段13：卫星仰角（00 - 90）度（前导位数不足则补0）
字段14：卫星方位角（00 - 359）度（前导位数不足则补0）
字段15：信噪比（00－99）dbHz
字段16：校验值
*/
void gpsGPGSV(uint8_t *_ucaBuf, uint16_t _usLen)
{
//	uint8_t s_total = 0;	/* 语句总数目 */
	uint8_t s_no = 0;		/* 语句序号 */
	uint8_t i;
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1：本次GSV语句的总数目（1 - 3） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
//	s_total = StrToInt(p);

	/* 字段2：本条GSV语句是本次GSV语句的第几条（1 - 3） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	s_no = StrToInt(p);

	/* 字段3：当前可见卫星总数（00 - 12）（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.ViewNumber = StrToInt(p);

	for (i = 0; i < 4; i++)
	{
		/* 字段4：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0） */
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.SateID[(s_no - 1) * 4 + i] = StrToInt(p);

		/* 字段5：卫星仰角（00 - 90）度（前导位数不足则补0）*/
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.Elevation[(s_no - 1) * 4 + i] = StrToInt(p);

		/* 字段6：卫星方位角（00 - 359）度（前导位数不足则补0） */
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.Azimuth[(s_no - 1) * 4 + i] = StrToInt(p);

		/* 字段7：信噪比（00－99）dbHz */
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.SNR[(s_no - 1) * 4 + i] = StrToInt(p);
	}
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPRMC
*	功能说明: 分析0183数据包中的 GPGSV 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPRMC,024813.640,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*50
字段0：$GPRMC，语句ID，表明该语句为Recommended Minimum Specific GPS/TRANSIT Data（RMC）推荐最小定位信息
字段1：UTC时间，hhmmss.sss格式
字段2：状态，A=定位，V=未定位
字段3：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段4：纬度N（北纬）或S（南纬）
字段5：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段6：经度E（东经）或W（西经）
字段7：速度，节，Knots
字段8：方位角，度
字段9：UTC日期，DDMMYY格式
字段10：磁偏角，（000 - 180）度（前导位数不足则补0）
字段11：磁偏角方向，E=东W=西
字段16：校验值
*/
void gpsGPRMC(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 UTC时间，hhmmss.sss格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Hour = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Min = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Sec = StrToIntFix(p, 2);
	p += 3;
	g_tGPS.mSec = StrToIntFix(p, 3);

	/* 字段2 状态，A=定位，V=未定位 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p != 'A')
	{
		/* 未定位则直接返回 */
		g_tGPS.PositionOk = 0;
		return;
	}
	g_tGPS.PositionOk = 1;
	p += 1;

	/* 字段3 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段4 纬度N（北纬）或S（南纬）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'S')
	{
		g_tGPS.NS = 'S';
	}
	else if (*p == 'N')
	{
		g_tGPS.NS = 'N';
	}
	else
	{
		return;
	}

	/* 字段5 经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.JingDu_Du = StrToIntFix(p, 3);
	p += 3;
	g_tGPS.JingDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段6：经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'E')
	{
		g_tGPS.EW = 'E';
	}
	else if (*p == 'W')
	{
		g_tGPS.EW = 'W';
	}

	/* 字段7：速度，节，Knots  10.05,*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.SpeedKnots = StrToInt(p);

	/* 字段8：方位角，度 ,324.27 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.TrackDegTrue = StrToInt(p);

	/* 字段9：UTC日期，DDMMYY格式  150706 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Day = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Month = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Year = StrToIntFix(p, 2);
	p += 2;
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPVTG
*	功能说明: 分析0183数据包中的 GPVTG 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPVTG,89.68,T,,M,0.00,N,0.0,K*5F
字段0：$GPVTG，语句ID，表明该语句为Track Made Good and Ground Speed（VTG）地面速度信息
字段1：运动角度，000 - 359，（前导位数不足则补0）
字段2：T=真北参照系
字段3：运动角度，000 - 359，（前导位数不足则补0）
字段4：M=磁北参照系
字段5：水平运动速度（0.00）（前导位数不足则补0）
字段6：N=节，Knots
字段7：水平运动速度（0.00）（前导位数不足则补0）
字段8：K=公里/时，km/h
字段9：校验值
*/
void gpsGPVTG(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1：运动角度，000 - 359，（前导位数不足则补0）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.TrackDegTrue = StrToInt(p);

	/* 字段2：T=真北参照系 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;

	/* 字段3：运动角度，000 - 359，（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.TrackDegMag = StrToInt(p);

	/* 字段4：M=磁北参照系 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;

	/* 字段5：地面速率（000.0~999.9节，前面的0也将被传输） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.SpeedKnots = StrToInt(p);

	/* 字段6：N=节，Knots */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;

	/* 字段7：地面速率（0000.0~1851.8公里/小时，前面的0也将被传输） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.SpeedKM = StrToInt(p);

	/* 字段8：K=公里/时，km/h	 */
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPGLL
*	功能说明: 分析0183数据包中的 GPGLL 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGLL,4250.5589,S,14718.5084,E,092204.999,A*2D
字段0：$GPGLL，语句ID，表明该语句为Geographic Position（GLL）地理定位信息
字段1：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段2：纬度N（北纬）或S（南纬）
字段3：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段4：经度E（东经）或W（西经）
字段5：UTC时间，hhmmss.sss格式
字段6：状态，A=定位，V=未定位
字段7：校验值
*/
void gpsGPGLL(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段2 纬度N（北纬）或S（南纬）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'S')
	{
		g_tGPS.NS = 'S';
	}
	else if (*p == 'N')
	{
		g_tGPS.NS = 'N';
	}
	else
	{
		return;
	}

	/* 字段3 经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.JingDu_Du = StrToIntFix(p, 3);
	p += 3;
	g_tGPS.JingDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段4：经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'E')
	{
		g_tGPS.EW = 'E';
	}
	else if (*p == 'W')
	{
		g_tGPS.EW = 'W';
	}

	/* 字段5 UTC时间，hhmmss.sss格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Hour = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Min = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Sec = StrToIntFix(p, 2);
	p += 2;

	/* 字段6 状态，A=定位，V=未定位 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p != 'A')
	{
		/* 未定位则直接返回 */
		return;
	}
}

/*
*********************************************************************************************************
*	函 数 名: Analyze0183
*	功能说明: 分析0183数据包
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void Analyze0183(uint8_t *_ucaBuf, uint16_t _usLen)
{

	if (CheckXor(_ucaBuf, _usLen) != TRUE)
	{
		return;
	}

	if (memcmp(_ucaBuf, "GPGGA,", 6) == 0)
	{
		gpsGPGGA(_ucaBuf, _usLen);
	}
	else if (memcmp(_ucaBuf, "GPGSA,", 6) == 0)
	{
		gpsGPGSA(_ucaBuf, _usLen);
	}
	else if (memcmp(_ucaBuf, "GPGSV,", 6) == 0)
	{
		gpsGPGSV(_ucaBuf, _usLen);
	}
	else if (memcmp(_ucaBuf, "GPRMC,", 6) == 0)
	{
		gpsGPRMC(_ucaBuf, _usLen);
	}
	else if (memcmp(_ucaBuf, "GPVTG,", 6) == 0)
	{
		gpsGPVTG(_ucaBuf, _usLen);
	}
	else if (memcmp(_ucaBuf, "GPGLL,", 6) == 0)
	{
		gpsGPGLL(_ucaBuf, _usLen);
	}
}

/*
*********************************************************************************************************
*	函 数 名: StrToInt
*	功能说明: 将ASCII码字符串转换成十进制
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
int32_t StrToInt(char *_pStr)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* 负数 */
		p++;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < 15; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p++;
			ucTemp = *p;
		}
		if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = ulInt * 10 + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}

/*
*********************************************************************************************************
*	函 数 名: StrToIntFix
*	功能说明: 将ASCII码字符串转换成十进制, 给定长度
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*			 _ucLen : 固定长度
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
int32_t StrToIntFix(char *_pStr, uint8_t _ucLen)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* 负数 */
		p++;
		_ucLen--;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < _ucLen; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p++;
			ucTemp = *p;
		}
		if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = ulInt * 10 + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}

/*
*********************************************************************************************************
*	函 数 名: HexToAscii
*	功能说明: 将hex码0x1f转换成'1'和'f'. 结尾填0.
*	形    参: ucpHex 输入缓冲区指针
*		     _ucpAscII 输出缓冲区指针
*		    _ucLenasc ASCII的字符长度.
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
void HexToAscii(uint8_t *_ucpHex, uint8_t *_ucpAscII, uint8_t _ucLenasc)
{
	uint8_t i;
	uint8_t ucTemp;

	for (i = 0; i < _ucLenasc; i++)
	{
		ucTemp = *_ucpHex;
		if ((i&0x01) == 0x00)
			ucTemp = ucTemp >> 4;
		else
		{
			ucTemp = ucTemp & 0x0f;
			_ucpHex++;
		}
		if (ucTemp < 0x0a)
			ucTemp += 0x30;
		else
			ucTemp += 0x37;
		_ucpAscII[i] = ucTemp;
	}
	//--------debug--------//
	_ucpAscII[i] = '\0';
	//--------end----------//
}

/*
*********************************************************************************************************
*	函 数 名: gps_FenToDu
*	功能说明: 将分转换为度的小数部分，保留6位小数。 将分换算为度。
*	形    参: 无
*	返 回 值: 返回度的小数部分（十进制）
*********************************************************************************************************
*/
uint32_t gps_FenToDu(uint32_t _fen)
{
	uint32_t du;
	
	/* g_tGPS.WeiDu_Fen;	纬度，分. 232475；  小数点后4位  表示 23.2475分 */
	
	du = (_fen * 100) / 60;
	
	return du;
}

/*
*********************************************************************************************************
*	函 数 名: gps_FenToMiao
*	功能说明: 将分的小数部分转化秒
*	形    参: 无
*	返 回 值: 秒 整数部分
*********************************************************************************************************
*/
uint16_t gps_FenToMiao(uint32_t _fen)
{
	uint32_t miao;
	
	/* g_tGPS.WeiDu_Fen;	纬度，分. 232475；  小数点后4位  表示 23.2475分 
		其中小数部分 0.2475 * 60 = 14.85 四舍五入为 15秒	
		
		
		2475 * 60 = 148500
		148500 / 10000 = 14;
		
		if ((148500 % 10000) >= 5000)
		{
			miao = 14 + 1
		}
	*/
	
	miao = ((_fen % 10000) * 60);
	
	if ((miao % 10000) >= 5000)
	{
		miao = miao / 10000 + 1;	/* 5入 */
	}
	else
	{
		miao = miao / 10000;		/* 4舍 */
	}
		
	return miao;
}

/*
*********************************************************************************************************
*	函 数 名: StrToIntFix
*	功能说明: 将ASCII码字符串转换成十进制, 给定长度
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*			 _ucLen : 固定长度
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
void UTCDate(void)
{
	#if 0
	/* 处理UTC时差 */
	{
		uint8_t ucaDays[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

		if (g_tParam.iUTCtime > 0)
		{
			g_tGPS.Hour += g_tParam.iUTCtime;
			if (g_tGPS.Hour > 23)
			{
				g_tGPS.Hour = g_tGPS.Hour - 24;
				g_tGPS.ucDay++;

				/* 闰年2月份为29天 */
				if (IsLeapYear(g_tGPS.usYear))
				{
					ucaDays[1] = 29;
				}
				else
				{
					ucaDays[1] = 28;
				}

				if (g_tGPS.ucDay > ucaDays[g_tGPS.ucMonth - 1])
				{
					g_tGPS.ucDay = 1;

					g_tGPS.ucMonth++;

					if (g_tGPS.ucMonth > 12)
					{
						g_tGPS.usYear++;
					}
				}
			}

		}
		else if (g_tParam.iUTCtime < 0)
		{
			int iHour;

			iHour = g_tGPS.Hour;
			iHour += g_tParam.iUTCtime;

			if (iHour < 0)
			{
				g_tGPS.Hour = 24 + iHour;

				if (g_tGPS.ucDay == 1)
				{
					if (g_tGPS.ucMonth == 1)
					{
						g_tGPS.usYear--;
						g_tGPS.ucMonth = 12;
						g_tGPS.ucDay = 31;
					}
					else
					{
						if (g_tGPS.ucMonth == 3)
						{
							g_tGPS.ucMonth = 2;

							/* 闰年2月份为29天 */
							if (IsLeapYear(g_tGPS.usYear))
							{
								g_tGPS.ucDay = 29;
							}
							else
							{
								g_tGPS.ucDay = 28;
							}
						}
						else
						{
							g_tGPS.ucMonth--;

							g_tGPS.ucDay = ucaDays[g_tGPS.ucMonth];
						}
					}
				}
				else
				{
					g_tGPS.ucDay--;
				}
			}
			else
			{
				g_tGPS.Hour = iHour;
			}
		}
	}
	#endif
}


/*
GPS上电后，每隔一定的时间就会返回一定格式的数据，数据格式为：

$信息类型，x，x，x，x，x，x，x，x，x，x，x，x，x

每行开头的字符都是‘$’，接着是信息类型，后面是数据，以逗号分隔开。一行完整的数据如下：

    $GPRMC,080655.00,A,4546.40891,N,12639.65641,E,1.045,328.42,170809,,,A*60



信息类型为：

GPGSV：可见卫星信息

GPGLL：地理定位信息

GPRMC：推荐最小定位信息

GPVTG：地面速度信息

GPGGA：GPS定位信息

GPGSA：当前卫星信息



1、 GPS DOP and Active Satellites（GSA）当前卫星信息

$GPGSA,<1>,<2>,<3>,<3>,,,,,<3>,<3>,<3>,<4>,<5>,<6>,<7>

<1>模式 ：M = 手动， A = 自动。
<2>定位型式 1 = 未定位， 2 = 二维定位， 3 = 三维定位。
<3>PRN 数字：01 至 32 表天空使用中的卫星编号，最多可接收12颗卫星信息。
<4> PDOP位置精度因子（0.5~99.9）
<5> HDOP水平精度因子（0.5~99.9）
<6> VDOP垂直精度因子（0.5~99.9）
<7> Checksum.(检查位).

2、 GPS Satellites in View（GSV）可见卫星信息

$GPGSV, <1>,<2>,<3>,<4>,<5>,<6>,<7>,?<4>,<5>,<6>,<7>,<8>

<1> GSV语句的总数
<2> 本句GSV的编号
<3> 可见卫星的总数，00 至 12。
<4> 卫星编号， 01 至 32。
<5>卫星仰角， 00 至 90 度。
<6>卫星方位角， 000 至 359 度。实际值。
<7>讯号噪声比（C/No）， 00 至 99 dB；无表未接收到讯号。
<8>Checksum.(检查位).
第<4>,<5>,<6>,<7>项个别卫星会重复出现，每行最多有四颗卫星。其余卫星信息会于次一行出现，若未使用，这些字段会空白。


3、Global Positioning System Fix Data（GGA）GPS定位信息
$GPGGA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,M,<10>,M,<11>,<12>*hh

<1> UTC时间，hhmmss（时分秒）格式
<2> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
<3> 纬度半球N（北半球）或S（南半球）
<4> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
<5> 经度半球E（东经）或W（西经）
<6> GPS状态：0=未定位，1=非差分定位，2=差分定位，6=正在估算
<7> 正在使用解算位置的卫星数量（00~12）（前面的0也将被传输）
<8> HDOP水平精度因子（0.5~99.9）
<9> 海拔高度（-9999.9~99999.9）
<10> 地球椭球面相对大地水准面的高度
<11> 差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
<12> 差分站ID号0000~1023（前面的0也将被传输，如果不是差分定位将为空）


4、Recommended Minimum Specific GPS/TRANSIT Data（RMC）推荐定位信息
$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh

<1> UTC时间，hhmmss（时分秒）格式
<2> 定位状态，A=有效定位，V=无效定位
<3> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
<4> 纬度半球N（北半球）或S（南半球）
<5> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
<6> 经度半球E（东经）或W（西经）
<7> 地面速率（000.0~999.9节，前面的0也将被传输）
<8> 地面航向（000.0~359.9度，以真北为参考基准，前面的0也将被传输）
<9> UTC日期，ddmmyy（日月年）格式
<10> 磁偏角（000.0~180.0度，前面的0也将被传输）
<11> 磁偏角方向，E（东）或W（西）
<12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）


5、 Track Made Good and Ground Speed（VTG）地面速度信息
$GPVTG,<1>,T,<2>,M,<3>,N,<4>,K,<5>*hh

<1> 以真北为参考基准的地面航向（000~359度，前面的0也将被传输）
<2> 以磁北为参考基准的地面航向（000~359度，前面的0也将被传输）
<3> 地面速率（000.0~999.9节，前面的0也将被传输）
<4> 地面速率（0000.0~1851.8公里/小时，前面的0也将被传输）
<5> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）

*/


/* 实测武汉地区 GPS数据
$GPGGA,064518.046,,,,,0,00,,,M,0.0,M,,0000*5A
$GPGLL,,,,,064518.046,V,N*76
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPGSV,3,1,12,18,56,351,,22,51,026,,14,51,206,21,19,48,285,*78
$GPGSV,3,2,12,26,38,041,,24,37,323,,03,37,281,,09,31,097,*78
$GPGSV,3,3,12,21,17,122,,25,13,176,,31,13,054,,20,00,266,*7A
$GPRMC,064518.046,V,,,,,,,250213,,,N*46
$GPVTG,,T,,M,,N,,K,N*2C

//蔡甸区
$GPGGA,161037.000,3030.6548,N,11402.4568,E,1,04,5.2,51.1,M,-15.5,M,,0000*42
$GPGSA,A,3,05,12,02,25,,,,,,,,,6.0,5.2,2.9*3B
$GPGSV,3,1,10,02,49,314,31,05,37,225,41,12,33,291,32,25,09,318,33*7C
$GPGSV,3,2,10,10,85,027,18,04,57,019,18,17,45,123,20,13,26,075,*7F
$GPGSV,3,3,10,23,14,050,23,40,18,253,33*71
$GPRMC,161037.000,A,3030.6548,N,11402.4568,E,0.00,,010613,,,A*71
$GPVTG,,T,,M,0.00,N,0.0,K,A*13

//第2次
$GPGGA,165538.000,3030.6519,N,11402.4480,E,2,05,1.9,39.5,M,-15.5,M,6.8,0000*68
$GPGSA,A,3,26,05,25,12,02,,,,,,,,2.7,1.9,2.0*3A
$GPGSV,3,1,11,10,63,029,18,02,58,344,23,05,55,247,46,04,50,053,26*75
$GPGSV,3,2,11,12,31,265,39,17,27,139,22,13,22,053,23,25,17,301,37*78
$GPGSV,3,3,11,26,11,180,43,23,04,036,,40,18,253,33*4A
$GPRMC,165538.000,A,3030.6519,N,11402.4480,E,0.00,71.87,010613,,,D*5E
$GPVTG,71.87,T,,M,0.00,N,0.0,K,D*31
$GPGGA,165539.000,3030.6519,N,11402.4480,E,2,05,1.9,39.5,M,-15.5,M,7.8,0000*68
$GPRMC,

度分秒 换算: 30度 30分 65

3030.6519 = 30度 + 30.6519分， 60进制， 
30.6519 分  --> 30.6519 / 60 = 0.510865度。  30.510865度

*/

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
