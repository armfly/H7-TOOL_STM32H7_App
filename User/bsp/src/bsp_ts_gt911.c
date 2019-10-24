/*
*********************************************************************************************************
*
*	模块名称 : 电容触摸芯片GT911驱动程序
*	文件名称 : bsp_gt911.c
*	版    本 : V1.0
*	说    明 : GT911触摸芯片驱动程序。
*	修改记录 :
*		版本号   日期        作者     说明
*		V1.0    2017-12-02  armfly   正式发布
*
*	Copyright (C), 2017-2025, 安富莱电子 www.armfly.com
*********************************************************************************************************
*/

#include "bsp.h"

#define GT911_READ_XY_REG    0x814E /* 坐标寄存器 */ 

#define GT911_CLEARBUF_REG   0x814E /* 清除坐标寄存器 */ 

#define GT911_CONFIG_REG     0x8047 /* 配置参数寄存器 */ 

#define GT911_COMMAND_REG    0x8040 /* 实时命令 */ 

#define GT911_PRODUCT_ID_REG 0x8140 /* 芯片ID */ 

#define GT911_VENDOR_ID_REG  0x814A /* 当前模组选项信息 */ 

#define GT911_CONFIG_VERSION_REG   0x8047 /* 配置文件版本号 */ 

#define GT911_CONFIG_CHECKSUM_REG  0x80FF /* 配置文件校验码 */ 

#define GT911_FIRMWARE_VERSION_REG 0x8144 /* 固件版本号 */ 

/* GT911单个触点配置参数，一次性写入 */ 
const uint8_t s_GT911_CfgParams[]= 
{ 

#if 1	/* 1024 * 600 */
	0x00,0x00,0x04,0x58,0x02,0x0A,0x0D,0x00,
	0x01,0x08,0x28,0x05,0x50,0x32,0x03,0x05,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x8C,0x2E,0x0E,0x27,0x24,
	0xD0,0x07,0x00,0x00,0x01,0x99,0x04,0x1D,
	0x00,0x00,0x00,0x00,0x00,0x03,0x64,0x32,
	0x00,0x00,0x00,0x0F,0x23,0x94,0xC5,0x02,
	0x07,0x00,0x00,0x04,0xA2,0x10,0x00,0x8C,
	0x13,0x00,0x7C,0x16,0x00,0x68,0x1B,0x00,
	0x5C,0x20,0x00,0x5C,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,0x25,0x14,0x04,0x14,
	0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,
	0x12,0x14,0x16,0x18,0x1A,0x1C,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
	0x04,0x06,0x08,0x0A,0x0C,0x0F,0x10,0x12,
	0x13,0x14,0x16,0x18,0x1C,0x1D,0x1E,0x1F,
	0x20,0x21,0x22,0x24,0x26,0x28,0x29,0x2A,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x43,0x01	
#else

	0x00, //0x8047 版本号 

	0xE0,0x01, //0x8048/8049 X坐标输出最大值480 

	0x56,0x03, //0x804a/804b Y坐标输出最大值854 

	0x01, //0x804c 输出触点个数上限 

	0x35, //0x804d 软件降噪，下降沿触发 

	0x00, //0x804e reserved 

	0x02, //0x804f 手指按下去抖动次数 

	0x08, //0x8050 原始坐标窗口滤波值 

	0x28, //0x8051 大面积触点个数 

	0x0A, //0x8052 噪声消除值 

	0x5A, //0x8053 屏上触摸点从无到有的阈值 

	0x46, //0x8054 屏上触摸点从有到无的阈值 

	0x03, //0x8055 进低功耗时间 s 

	0x05, //0x8056 坐标上报率 

	0x00, //0x8057 X坐标输出门上限 

	0x00, //0x8058 Y坐标输出门上限 

	0x00,0X00, //0x8059-0x805a reserved 

	0x00, //0x805b reserved 

	0x00, //0x805c reserved 

	0x00, //0x805d 划线过程中小filter设置 

	0x18, //0x805e 拉伸区间 1 系数 

	0x1A, //0x805f 拉伸区间 2 系数 

	0x1E, //0x8060 拉伸区间 3 系数 

	0x14, //0x8061 各拉伸区间基数 

	0x8C, //0x8062 、、 

	0x28, //0x8063 、、 

	0x0C, //0x8064 、、 

	0x71, //0x8065 驱动组A的驱动频率倍频系数 

	0x73, //0x8066 驱动组B的驱动频率倍频系数 

	0xB2, //0x8067 驱动组A、B的基频 

	0x04, //0x8068 

	0x00, //0x8069 相邻两次驱动信号输出时间间隔 

	0x00, //0x806a 

	0x00, //0x806b 、、 

	0x02, //0x806c 、、 

	0x03, //0x806d 原始值放大系数 

	0x1D, //0x806e 、、 

	0x00, //0x806f reserved 

	0x01, //0x8070 、、 

	0x00,0x00, //reserved 

	0x00, //0x8073 、、 

	0x00,0x00,0x00,0x00,0x00,0x00, //0x8071 - 0x8079 reserved 

	0x50, //0x807a 跳频范围的起点频率 

	0xA0, //0x807b 跳频范围的终点频率 

	0x94, //0x807c 多次噪声检测后确定噪声量，1-63有效 

	0xD5, //0x807d 噪声检测超时时间 

	0x02, //0x807e 、、 

	0x07, //0x807f 判别有干扰的门限 

	0x00,0x00, //0x8081 reserved 

	0x04, //0x8082 跳频检测区间频段1中心点基频（适用于驱动A、B） 

	0xA4, //0x8083 

	0x55, //0x8084 跳频检测区间频段1中心点倍频系数 

	0x00, //0x8085 跳频检测区间频段2中心点基频(驱动A、B在此基础上换算) 

	0x91, //0x8086 

	0x62, //0x8087 跳频检测区间频段2中心点倍频系数 

	0x00, //0x8088 跳频检测区间频段3中心点基频（适用于驱动A、B） 

	0x80, //0x8089 

	0x71, //0x808a 跳频检测区间频段3中心点倍频系数 

	0x00, //0x808b 跳频检测区间频段4中心点基频（适用于驱动A、B） 

	0x71, //0x808c 

	0x82, //0x808d 跳频检测区间频段4中心点倍频系数 

	0x00, //0x808e 跳频检测区间频段5中心点基频（适用于驱动A、B） 

	0x65, //0x808f 

	0x95, //0x8090 跳频检测区间频段5中心点倍频系数 

	0x00, 0x65, //reserved 

	0x00, //0x8093 key1位置 0：无按键 

	0x00, //0x8094 key2位置 0：无按键 

	0x00, //0x8095 key3位置 0：无按键 

	0x00, //0x8096 key4位置 0：无按键 

	0x00, //0x8097 reserved 

	0x00, //0x8098 reserved 

	0x00, //0x8099 reserved 

	0x00, //0x809a reserved 

	0x00, //0x809b reserved 

	0x00, //0x809c reserved 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x809d-0x80b2 reserved 

	0x00, //0x80b3 合框距离 

	0x00, //0x80b4 

	0x00,0x00, //0x80b6 reserved 

	0x06, //0x80b7 

	0x08, //0x80b8 

	0x0A, //0x80b9 

	0x0C, //0x80ba 

	0x0E, //0x80bb 

	0x10, //0x80bc 

	0x12, //0x80bd 

	0x14, //0x80be 

	0x16, //0x80bf 
+	
	0x18, //0x80c0 

	0x1A, //0x80c1 

	0x1C, //0x80c2 

	0xFF, //0x80c3 

	0xFF, //0x80c4 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 

	0x00,0x00,0x00 



	,0x00, //0x80c5-0x80d4 reserved 

	0x00, //0x80d5 

	0x02, //0x80d6 

	0x04, //0x80d7 

	0x06, //0x80d8 

	0x08, //0x80d9 

	0x0A, //0x80da 

	0x0C, //0x80db 

	0x0F, //0x80dc 

	0x10, //0x80dd 

	0x12, //0x80de 

	0x13, //0x80df 

	0x14, //0x80e0 

	0x16, //0x80e1 

	0x18, //0x80e2 

	0x1C, //0x80e3 

	0x1D, //0x80e4 

	0x1E, //0x80e5 

	0x1F, //0x80e6 

	0x20, //0x80e7 

	0x21, //0x80e8 

	0xFF, //0x80e9 

	0xFF, //0x80ea 

	0xFF, //0x80eb 

	0xFF, //0x80ec 

	0xFF, //0x80ed 

	0xFF, //0x80ee 

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 

	0x00,0x00,0x00,0x00, //0x80ef-0x80fe reserved 

	0x0B, //0x80ff 配置信息校验 

	0x01 //0x8100 配置以更新标记 
#endif
}; 

static void GT911_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
static void GT911_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);

GT911_T g_GT911;

/*
*********************************************************************************************************
*	函 数 名: GT911_InitHard
*	功能说明: 配置触摸芯片.  在调用该函数前，请先执行 bsp_touch.c 中的函数 bsp_DetectLcdType() 识别id
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void GT911_InitHard(void)
{		
	/* 如果INT脚接了上拉电阻，需要发送INT唤醒脉冲。 */
	#if 0
		/* 定义触笔中断INT的GPIO端口 */
		#define RCC_TP_INT	RCC_AHB1Periph_GPIOH
		#define PORT_TP_INT	GPIOH
		#define PIN_TP_INT	GPIO_PIN_7	
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* 第1步：打开GPIO时钟 */
		RCC_AHB1PeriphClockCmd(RCC_TP_INT, ENABLE);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
		GPIO_InitStructure.GPIO_Pin = PIN_TP_INT;
		GPIO_Init(PORT_TP_INT, &GPIO_InitStructure);		
		
		PORT_TP_INT->BSRRL = PIN_TP_INT;
		bsp_DelayUS(2000);					/* 产生2ms脉冲唤醒 */
		PORT_TP_INT->BSRRH = PIN_TP_INT;	/* INT脚拉低。使GT911能正常工作 */		
		bsp_DelayUS(200);		
		
		/* 第2步：配置所有的按键GPIO为浮动输入模式。*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式（输入模式无意义） */
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		/* 配置内部上拉 */	
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */
		GPIO_InitStructure.GPIO_Pin = PIN_TP_INT;
		GPIO_Init(PORT_TP_INT, &GPIO_InitStructure);		
	}	
	#endif

	g_GT911.TimerCount = 0;
	
#if 0	/* 调试代码 */
	{
		uint16_t ver;
		uint32_t id;
	
		id = GT911_ReadID();	
		ver = GT911_ReadVersion();
		
		printf("GT911 ID :%08X, Version : %04X\r\n", id, ver);
	}
#endif
	
	bsp_DelayMS(50);
	
	/* I2C总线初始化在 bsp.c 中执行 */
	
	/*
		无需传送配置参数
		GT911_WriteReg(GT911_CONFIG_REG, (uint8_t *)s_GT911_CfgParams, sizeof(s_GT911_CfgParams));
	*/
	
	g_GT911.TimerCount = 0;
	g_GT911.Enable = 1;
}



/*
*********************************************************************************************************
*	函 数 名: GT911_ReadID
*	功能说明: 获得GT911的芯片ID
*	形    参: 无
*	返 回 值: 16位版本
*********************************************************************************************************
*/
uint32_t GT911_ReadID(void)
{
	uint8_t buf[4]; 

	GT911_ReadReg(GT911_PRODUCT_ID_REG, buf, 4); 

	return ((uint32_t)buf[3] << 24) + ((uint32_t)buf[2] << 16) + ((uint32_t)buf[1] <<8) + buf[0]; 
}

/*
*********************************************************************************************************
*	函 数 名: GT911_ReadVersion
*	功能说明: 获得GT911的芯片版本
*	形    参: 无
*	返 回 值: 16位版本
*********************************************************************************************************
*/
uint16_t GT911_ReadVersion(void)
{
	uint8_t buf[2]; 

	GT911_ReadReg(GT911_FIRMWARE_VERSION_REG, buf, 2); 

	return ((uint16_t)buf[1] << 8) + buf[0]; 
}


/*
*********************************************************************************************************
*	函 数 名: GT911_WriteReg
*	功能说明: 写1个或连续的多个寄存器
*	形    参: _usRegAddr : 寄存器地址
*			  _pRegBuf : 寄存器数据缓冲区
*			 _ucLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
static void GT911_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

    i2c_Start();					/* 总线开始信号 */

    i2c_SendByte(g_GT911.i2c_addr);	/* 发送设备地址+写信号 */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr >> 8);	/* 地址高8位 */
	i2c_WaitAck();

    i2c_SendByte(_usRegAddr);		/* 地址低8位 */
	i2c_WaitAck();

	for (i = 0; i < _ucLen; i++)
	{
	    i2c_SendByte(_pRegBuf[i]);		/* 寄存器数据 */
		i2c_WaitAck();
	}

    i2c_Stop();                   			/* 总线停止信号 */
}

/*
*********************************************************************************************************
*	函 数 名: GT911_ReadReg
*	功能说明: 读1个或连续的多个寄存器
*	形    参: _usRegAddr : 寄存器地址
*			  _pRegBuf : 寄存器数据缓冲区
*			 _ucLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
static void GT911_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;
	
	{
		i2c_Start();					/* 总线开始信号 */

		i2c_SendByte(g_GT911.i2c_addr);	/* 发送设备地址+写信号 */
		i2c_WaitAck();

		i2c_SendByte(_usRegAddr >> 8);	/* 地址高8位 */
		i2c_WaitAck();

		i2c_SendByte(_usRegAddr);		/* 地址低8位 */
		i2c_WaitAck();

		i2c_Start();
		i2c_SendByte(g_GT911.i2c_addr + 0x01);	/* 发送设备地址+读信号 */

		i2c_WaitAck();
	}
	
	for (i = 0; i < 30; i++);

	for (i = 0; i < _ucLen - 1; i++)
	{
	    _pRegBuf[i] = i2c_ReadByte();	/* 读寄存器数据 */
		i2c_Ack();
	}

	/* 最后一个数据 */
	 _pRegBuf[i] = i2c_ReadByte();		/* 读寄存器数据 */
	i2c_NAck();

    i2c_Stop();							/* 总线停止信号 */
}

/*
*********************************************************************************************************
*	函 数 名: GT911_Timer1ms
*	功能说明: 每隔1ms调用1次
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void GT911_Timer1ms(void)
{
	g_GT911.TimerCount++;
}

/*
*********************************************************************************************************
*	函 数 名: GT911_Scan
*	功能说明: 读取GT911触摸数据。读取全部的数据，需要 720us左右。放在 bsp_Idle()中执行
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void GT911_Scan(void)
{
	uint8_t buf[48];
	static uint8_t s_tp_down = 0;
	uint16_t x, y;
	static uint16_t x_save, y_save;
	uint8_t clear_flag = 0;
	static uint8_t s_count = 0;	

	if (g_GT911.Enable == 0)
	{
		return;
	}
	
	/* 20ms 执行一次 */
	if (g_GT911.TimerCount < 10)
	{
		return;
	}

	g_GT911.TimerCount = 0;

#if 1	/* 方案1: 检测INT引脚电平. */
	if (TOUCH_PenInt() == 0)
	{		
#else	/* 方案2：不用INT引脚，读状态寄存器 */
	GT911_ReadReg(GT911_READ_XY_REG, buf, 1);
	if (buf[0] == 0)
	{
#endif	
		if (s_tp_down == 1)
		{
			if (++s_count > 1)
			{
				s_count = 0;
				s_tp_down = 0;
				TOUCH_PutKey(TOUCH_RELEASE, x_save, y_save);
			}
		}
		return;
	}	
	s_count = 0;	

#if 1		/* 一般应用只读1点 */
	GT911_ReadReg(GT911_READ_XY_REG, buf, 8);
#else		/* 读5个触摸点 */
	GT911_ReadReg(GT911_READ_XY_REG, buf, 40);
#endif
	
	GT911_WriteReg(GT911_READ_XY_REG, &clear_flag,	1);		/* 读完坐标后必须写0清除 */
	
	/*
		0x814E R/W Bufferstatus Large_Detect number of touch points 
			Bit7: Buffer status，1表示坐标（或按键）已经准备好，主控可以读取；0表示未就绪，数据无效。当主控读取完坐标后，必须通过I2C将此标志（或整个字节）写为0。
			Bit4: HaveKey, 1表示有按键，0表示无按键（已经松键）。
			Bit3~0: Number of touch points, 屏上的坐标点个数
	
		0x814F R Point1 track id 
		0x8150 R Point1Xl 触摸点 1，X 坐标低 8 位 
		0x8151 R Point1Xh 触摸点 1，X 坐标高 8 位 
		0x8152 R Point1Yl 触摸点 1，Y 坐标低 8 位 
		0x8153 R Point1Yh 触摸点 1，Y 坐标高 8 位 
		0x8154 R Point1 触摸点 1，触摸面积低 8 位 
		0x8155 R Point1 触摸点 1，触摸面积高 8 位 
		0x8156 ----
	
		0x8157 R Point2 track id 
		0x8158 R Point2Xl 触摸点 2，X 坐标低 8 位 
		0x8159 R Point2Xh 触摸点 2，X 坐标高 8 位 
		0x815A R Point2Yl 触摸点 2，Y 坐标低 8 位 
		0x815B R Point2Yh 触摸点 2，Y 坐标高 8 位 
		0x815C R Point2 触摸点 2，触摸面积低 8 位 
		0x815D R Point2 触摸点 2，触摸面积高 8 位 
		0x815E ----

		0x815F R Point3 track id 
		0x8160 R Point3Xl 触摸点 3，X 坐标低 8 位 
		0x8161 R Point3Xh 触摸点 3，X 坐标高 8 位 
		0x8162 R Point3Yl 触摸点 3，Y 坐标低 8 位 
		0x8163 R Point3Yh 触摸点 3，Y 坐标高 8 位 
		0x8164 R Point3 触摸点 3，触摸面积低 8 位 
		0x8165 R Point3 触摸点 3，触摸面积高 8 位 
		0x8166 ----

		0x8167 R Point4 track id 
		0x8168 R Point4Xl 触摸点 4，X 坐标低 8 位 
		0x8169 R Point4Xh 触摸点 4，X 坐标高 8 位 
		0x816A R Point4Yl 触摸点 4，Y 坐标低 8 位 
		0x816B R Point4Yh 触摸点 4，Y 坐标高 8 位 
		0x816C R Point4 触摸点 4，触摸面积低 8 位 
		0x816D R Point4 触摸点 4，触摸面积高 8 位 
		0x816E ----

		0x816F R Point5 track id 
		0x8170 R Point5Xl 触摸点 5，X 坐标低 8 位 
		0x8171 R Point5Xh 触摸点 5，X 坐标高 8 位 
		0x8172 R Point5Yl 触摸点 5，Y 坐标低 8 位 
		0x8173 R Point5Yh 触摸点 5，Y 坐标高 8 位 
		0x8174 R Point5 触摸点 5，触摸面积低 8 位 
		0x8175 R Point5 触摸点 5，触摸面积高 8 位 
		0x8176 --
		
	*/
	g_GT911.TouchpointFlag = buf[0];
	g_GT911.Touchkeystate = buf[1];

	g_GT911.X0 = ((uint16_t)buf[3] << 8) + buf[2];
	g_GT911.Y0 = ((uint16_t)buf[5] << 8) + buf[4];
	g_GT911.P0 = ((uint16_t)buf[7] << 8) + buf[6];

	#if 0	/* 其余4点一般不用 */
		g_GT911.X1 = ((uint16_t)buf[9] << 8) + buf[10];
		g_GT911.Y1 = ((uint16_t)buf[11] << 8) + buf[12];
		g_GT911.P1 = ((uint16_t)buf[13] << 8) + buf[14];

		g_GT911.X2 = ((uint16_t)buf[17] << 8) + buf[16];
		g_GT911.Y2 = ((uint16_t)buf[19] << 8) + buf[18];
		g_GT911.P2 = ((uint16_t)buf[21] << 8) + buf[20];

		g_GT911.X3 = ((uint16_t)buf[24] << 8) + buf[23];
		g_GT911.Y3 = ((uint16_t)buf[26] << 8) + buf[25];
		g_GT911.P3 = ((uint16_t)buf[28] << 8) + buf[27];

		g_GT911.X4 = ((uint16_t)buf[31] << 8) + buf[30];
		g_GT911.Y4 = ((uint16_t)buf[33] << 8) + buf[32];
		g_GT911.P4 = ((uint16_t)buf[35] << 8) + buf[34];
	#endif

	/* 检测按下 */
	{
		/* 坐标转换 :
			电容触摸板左下角是 (0，0);  右上角是 (479，799)
			需要转到LCD的像素坐标 (左上角是 (0，0), 右下角是 (799，479)
		*/
		{
			x = g_GT911.X0;
			y = g_GT911.Y0;
			
			if (x > 799)
			{
				x = 799;
			}
			
			if (y > 479)
			{
				y = 479;
			}
		}
	}
	
	if (s_tp_down == 0)
	{
		s_tp_down = 1;
		
		TOUCH_PutKey(TOUCH_DOWN, x, y);
	}
	else
	{
		TOUCH_PutKey(TOUCH_MOVE, x, y);
	}
	x_save = x;	/* 保存坐标，用于释放事件 */
	y_save = y;

#if 0
	{
		uint8_t i;
		
		for (i = 0; i < 34; i++)
		{
			printf("%02X ", buf[i]);
		}
		printf("\r\n");

		printf("(%5d,%5d,%3d) ",  g_GT911.X0, g_GT911.Y0, g_GT911.P0);
		printf("(%5d,%5d,%3d) ",  g_GT911.X1, g_GT911.Y1, g_GT911.P1);
		printf("(%5d,%5d,%3d) ",  g_GT911.X2, g_GT911.Y2, g_GT911.P2);
		printf("(%5d,%5d,%3d) ",  g_GT911.X3, g_GT911.Y3, g_GT911.P3);
		printf("(%5d,%5d,%3d) ",  x, y, g_GT911.P4);
		printf("\r\n");
	}
#endif	
}

/*
*********************************************************************************************************
*	函 数 名: GT911_ReadSensorID
*	功能说明: 识别显示模块类别。读取GT911 SensorID引脚状态，有3个状态，悬空，接电源，接地。
*	形    参: 无
*	返 回 值: 显示模块类别, 0, 1, 2
*********************************************************************************************************
*/
uint8_t GT911_ReadSensorID(void)
{
	uint8_t value;
	
	/* 	0x721  R  TouchpointFlag      Sensor_ID  key  tp4  tp3  tp2  tp1  tp0 */
	GT911_ReadReg(0x721, &value, 1);
	
	return (value >> 6);
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
