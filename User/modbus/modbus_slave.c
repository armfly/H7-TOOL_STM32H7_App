/*
*********************************************************************************************************
*
*	模块名称 : MODBUS从机模块
*	文件名称 : modbus_slave.c
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "main.h"
#include "param.h"
#include "modbus_slave.h"
#include "modbus_reg_addr.h"
#include "modbus_register.h"
#include "modbus_iap.h"
#include "lua_if.h"

static void MODS_AnalyzeApp(void);

//static void MODS_RxTimeOut(void);

static void MODS_01H(void);
static void MODS_02H(void);
static void MODS_03H(void);
static void MODS_04H(void);
static void MODS_05H(void);
static void MODS_06H(void);
static void MODS_10H(void);
static void MODS_0FH(void);
static void MODS_64H(void);
static void MODS_65H(void);
static void MODS_66H(void);
static void MODS_67H(void);
static void MODS_68H(void);

static void MODS_60H(void); /* PC发这个指令读取波形数据 */

void MODS_ReciveNew(uint8_t _byte);

MODS_T g_tModS;
MOD_WAVE_T g_tModWave;

/*
*********************************************************************************************************
*	函 数 名: MODS_Poll
*	功能说明: 解析数据包. 在主程序中轮流调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t MODS_Poll(uint8_t *_buf, uint16_t _len)
{
  uint16_t addr;
  uint16_t crc1;

  g_tModS.RxBuf = _buf;
  g_tModS.RxCount = _len;

  g_tModS.TxCount = 0;
  //*_AckBuf = g_tModS.TxBuf;

  if (g_tModS.RxCount < 4)
  {
    goto err_ret;
  }

  if (g_tModS.TCP_Flag == 0)
  {
    /* 计算CRC校验和 */
    crc1 = CRC16_Modbus(g_tModS.RxBuf, g_tModS.RxCount);
    if (crc1 != 0)
    {
      MODS_SendAckErr(ERR_PACKAGE); /* 发送连包应答 */
      goto err_ret;
    }
  }
  else
  {
    g_tModS.RxCount += 2;
  }

  /* 站地址 (1字节） */
  addr = g_tModS.RxBuf[0]; /* 第1字节 站号 */
  if (addr != g_tParam.Addr485 && addr != 0xF4)
  {
    goto err_ret;
  }

  /* 分析应用层协议 */
  MODS_AnalyzeApp();
  g_tModS.RxCount = 0; /* 必须清零计数器，方便下次帧同步 */
  return 1;

err_ret:
  g_tModS.RxCount = 0; /* 必须清零计数器，方便下次帧同步 */
  return 0;
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: MODS_ReciveNew
*	功能说明: 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
//void MODS_ReciveNew(uint8_t _byte)
//{
//	/*
//		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
//		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
//		所以就是3.5个字符的时间，波特率高，这个时间间隔就小，波特率低，这个时间间隔相应就大

//		4800  = 7.297ms
//		9600  = 3.646ms
//		19200  = 1.771ms
//		38400  = 0.885ms
//	*/
//	uint32_t timeout;
//	
//	g_rtu_timeout = 0;
//	
//	timeout = 35000000 / g_tParam.Baud;		/* 计算超时时间，单位us */
//	
//	/* 硬件定时中断，定时精度us 硬件定时器1用于ADC, 定时器2用于Modbus */
//	bsp_StartHardTimer(2, timeout, (void *)MODS_RxTimeOut);

//	if (g_tModS.RxCount < RX_BUF_SIZE)
//	{
//		g_tModS.RxBuf[g_tModS.RxCount++] = _byte;
//	}
//}

///*
//*********************************************************************************************************
//*	函 数 名: MODS_RxTimeOut
//*	功能说明: 超过3.5个字符时间后执行本函数。 设置全局变量 g_rtu_timeout = 1; 通知主程序开始解码。
//*	形    参: 无
//*	返 回 值: 无
//*********************************************************************************************************
//*/
//static void MODS_RxTimeOut(void)
//{
//	g_rtu_timeout = 1;
//}
#endif

/*
*********************************************************************************************************
*	函 数 名: MODS_SendWithCRC
*	功能说明: 发送一串数据, 自动追加2字节CRC。数据在全局变量: g_tModS.TxBuf, g_tModS.TxCount
*	形    参: 无
*			  _ucLen 数据长度（不带CRC）
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_SendWithCRC(void)
{
  uint16_t crc;

  crc = CRC16_Modbus(g_tModS.TxBuf, g_tModS.TxCount);
  g_tModS.TxBuf[g_tModS.TxCount++] = crc >> 8;
  g_tModS.TxBuf[g_tModS.TxCount++] = crc;
}

/*
*********************************************************************************************************
*	函 数 名: MODS_SendAckErr
*	功能说明: 发送错误应答
*	形    参: _ucErrCode : 错误代码
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_SendAckErr(uint8_t _ucErrCode)
{
  g_tModS.TxCount = 0;
  g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];        /* 485地址 */
  g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1] | 0x80; /* 异常的功能码 */
  g_tModS.TxBuf[g_tModS.TxCount++] = _ucErrCode;              /* 错误代码(01,02,03,04) */

  MODS_SendWithCRC();
}

/*
*********************************************************************************************************
*	函 数 名: MODS_SendAckOk
*	功能说明: 发送正确的应答.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_SendAckOk(void)
{
  uint8_t i;

  g_tModS.TxCount = 0;
  for (i = 0; i < 6; i++)
  {
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[i];
  }
  MODS_SendWithCRC();
}

/*
*********************************************************************************************************
*	函 数 名: MODS_AnalyzeApp
*	功能说明: 分析应用层协议
*	形    参:
*		     _DispBuf  存储解析到的显示数据ASCII字符串，0结束
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_AnalyzeApp(void)
{
  switch (g_tModS.RxBuf[1]) /* 第2个字节 功能码 */
  {
  case 0x01: /* 读取线圈状态*/
    MODS_01H();
    break;

  case 0x02: /* 读取输入状态 */
    MODS_02H();
    break;

  case 0x03: /* 读取1个或多个参数保持寄存器 在一个或多个保持寄存器中取得当前的二进制值*/
    MODS_03H();
    break;

  case 0x04: /* 读取1个或多个模拟量输入寄存器 */
    MODS_04H();
    break;

  case 0x05: /* 强制单线圈（） */
    MODS_05H();
    break;

  case 0x06: /* 写单个参数保持寄存器 (存储在CPU的FLASH中，或EEPROM中的参数)*/
    MODS_06H();
    break;

  case 0x10: /* 写多个参数保持寄存器 (存储在CPU的FLASH中，或EEPROM中的参数)*/
    MODS_10H();
    break;

  case 0x0F:
    MODS_0FH(); /* 强制多个线圈（对应D01/D02/D03） */
    break;

    //		case 0x15:	/* 写文件寄存器 */
    //			MODS_15H();
    //			break;

  case 0x60: /* 读取波形数据专用功能码 */
    MODS_60H();
    break;

  case 0x64: /* 小程序下载接口 */
    MODS_64H();
    break;

  case 0x65: /* 临时执行小程序 */
    MODS_65H();
    break;

  default:
    g_tModS.RspCode = RSP_ERR_CMD;
    MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    break;
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_03H
*	功能说明: 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_03H(void)
{
  uint16_t regaddr;
  uint16_t num;
  uint16_t value;
  uint16_t i;

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount != 8)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  regaddr = BEBufToUint16(&g_tModS.RxBuf[2]);
  num = BEBufToUint16(&g_tModS.RxBuf[4]);
  if (num > (TX_BUF_SIZE - 5) / 2)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
      g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;

      /* 特殊处理，波形读取 ADC_BUFFER_SIZE */
      if (regaddr >= REG03_CH1_WAVE_0 && regaddr <= REG03_CH1_WAVE_END)
      {
        uint16_t m;
        uint32_t idx;
        uint8_t *p;

        m = regaddr - REG03_CH1_WAVE_0;
        idx = m * DSO_PACKAGE_SIZE / 4;
        for (i = 0; i < num / 2; i++)
        {

          p = (uint8_t *)&g_Ch1WaveBuf[idx++];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[3];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[2];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[1];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[0];
        }
      }
      else if (regaddr >= REG03_CH2_WAVE_0 && regaddr <= REG03_CH2_WAVE_END)
      {
        uint16_t m;
        uint32_t idx;
        uint8_t *p;

        m = regaddr - REG03_CH2_WAVE_0;
        idx = m * DSO_PACKAGE_SIZE / 4;
        ;
        for (i = 0; i < num / 2; i++)
        {
          p = (uint8_t *)&g_Ch2WaveBuf[idx++];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[3];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[2];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[1];
          g_tModS.TxBuf[g_tModS.TxCount++] = p[0];
        }
      }
      else
      {
        for (i = 0; i < num; i++)
        {
          if (ReadRegValue_03H(regaddr++, &value) == 0)
          {
            g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
            goto err_ret;
          }
          g_tModS.TxBuf[g_tModS.TxCount++] = value >> 8;
          g_tModS.TxBuf[g_tModS.TxCount++] = value;
        }
      }

      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_04H
*	功能说明: 读一个或多个模拟量寄存器。 结构和03H指令相同。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_04H(void)
{
  uint16_t regaddr;
  uint16_t num;
  uint16_t value;
  uint8_t i;

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount != 8)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  regaddr = BEBufToUint16(&g_tModS.RxBuf[2]);
  num = BEBufToUint16(&g_tModS.RxBuf[4]);
  if (num > (TX_BUF_SIZE - 5) / 2)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
      g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;

      for (i = 0; i < num; i++)
      {
        if (ReadRegValue_04H(regaddr++, &value) == 0)
        {
          g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
          goto err_ret;
        }
        g_tModS.TxBuf[g_tModS.TxCount++] = value >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = value;
      }

      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_06H
*	功能说明: 写单个寄存器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_06H(void)
{

  /*
    写保持寄存器。注意06指令只能操作单个保持寄存器，16指令可以设置单个或多个保持寄存器

    主机发送:
      11 从机地址
      06 功能码
      00 寄存器地址高字节
      01 寄存器地址低字节
      00 数据1高字节
      01 数据1低字节
      9A CRC校验高字节
      9B CRC校验低字节

    从机响应:
      11 从机地址
      06 功能码
      00 寄存器地址高字节
      01 寄存器地址低字节
      00 数据1高字节
      01 数据1低字节
      1B CRC校验高字节
      5A	CRC校验低字节

    例子:
      01 06 30 06 00 25  A710    ---- 触发电流设置为 2.5
      01 06 30 06 00 10  6707    ---- 触发电流设置为 1.0


      01 06 30 1B 00 00  F6CD    ---- SMA 滤波系数 = 0 关闭滤波
      01 06 30 1B 00 01  370D    ---- SMA 滤波系数 = 1
      01 06 30 1B 00 02  770C    ---- SMA 滤波系数 = 2
      01 06 30 1B 00 05  36CE    ---- SMA 滤波系数 = 5

      01 06 30 07 00 01  F6CB    ---- 测试模式修改为 T1
      01 06 30 07 00 02  B6CA    ---- 测试模式修改为 T2

      01 06 31 00 00 00  8736    ---- 擦除浪涌记录区
      01 06 31 01 00 00  D6F6    ---- 擦除告警记录区

*/

  uint16_t reg;
  uint16_t value;
  //	uint8_t i;

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount != 8)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  reg = BEBufToUint16(&g_tModS.RxBuf[2]);   /* 寄存器号 */
  value = BEBufToUint16(&g_tModS.RxBuf[4]); /* 寄存器值 */

  fResetReq_06H = 0;
  fSaveReq_06H = 0; /* 需要保存参数 */
  fSaveCalibParam = 0;
  if (WriteRegValue_06H(reg, value) == 0)
  {
    g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
  }
  else
  {
    if (fSaveReq_06H == 1) /* 需要先发 */
    {
      fSaveReq_06H = 1;
      SaveParam();
    }

    if (fSaveCalibParam == 1)
    {
      fSaveCalibParam = 0;
      SaveCalibParam(); /* 保存校准参数 */
    }

    if (fResetReq_06H == 1)
    {
      fResetReq_06H = 0;
      {
        /* 复位进入APP */
        *(uint32_t *)0x20000000 = 0;
        NVIC_SystemReset(); /* 复位CPU */
      }
    }
  }

err_ret:
  if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
  {
    MODS_SendAckOk();
  }
  else
  {
    MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_10H
*	功能说明: 连续写多个寄存器.  进用于改写时钟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_10H(void)
{
  /*
    从机地址为11H。保持寄存器的其实地址为0001H，寄存器的结束地址为0002H。总共访问2个寄存器。
    保持寄存器0001H的内容为000AH，保持寄存器0002H的内容为0102H。

    主机发送:
      11 从机地址
      10 功能码
      00 寄存器起始地址高字节
      01 寄存器起始地址低字节
      00 寄存器数量高字节
      02 寄存器数量低字节
      04 字节数
      00 数据1高字节
      0A 数据1低字节
      01 数据2高字节
      02 数据2低字节
      C6 CRC校验高字节
      F0 CRC校验低字节

    从机响应:
      11 从机地址
      06 功能码
      00 寄存器地址高字节
      01 寄存器地址低字节
      00 数据1高字节
      01 数据1低字节
      1B CRC校验高字节
      5A	CRC校验低字节

    例子:
      01 10 30 00 00 06 0C  07 DE  00 0A  00 01  00 08  00 0C  00 00     389A    ---- 写时钟 2014-10-01 08:12:00
      01 10 30 00 00 06 0C  07 DF  00 01  00 1F  00 17  00 3B  00 39     5549    ---- 写时钟 2015-01-31 23:59:57

  */
  uint16_t reg_addr;
  uint16_t reg_num;
  //	uint8_t byte_num;
  uint16_t value;
  uint8_t i;
  uint8_t *_pBuf;

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount < 11)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  fSaveReq_06H = 0;
  fSaveCalibParam = 0;
  fResetReq_06H = 0;

  reg_addr = BEBufToUint16(&g_tModS.RxBuf[2]); /* 寄存器号 */
  reg_num = BEBufToUint16(&g_tModS.RxBuf[4]);  /* 寄存器个数 */
                                               //	byte_num = g_tModS.RxBuf[6];	/* 后面的数据体字节数 */
  _pBuf = &g_tModS.RxBuf[7];

  for (i = 0; i < reg_num; i++)
  {
    value = BEBufToUint16(_pBuf);

    if (WriteRegValue_06H(reg_addr + i, value) == 0)
    {
      g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
      break;
    }

    _pBuf += 2;
  }

  if (fSaveReq_06H == 1) /* 需要先发 */
  {
    fSaveReq_06H = 1;
    SaveParam();
  }

  if (fSaveCalibParam == 1)
  {
    fSaveCalibParam = 0;
    SaveCalibParam(); /* 保存校准参数 */
  }

  if (fResetReq_06H == 1)
  {
    fResetReq_06H = 0;
    {
      /* 复位进入APP */
      *(uint32_t *)0x20000000 = 0;
      NVIC_SystemReset(); /* 复位CPU */
    }
  }

err_ret:
  if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
  {
    MODS_SendAckOk();
  }
  else
  {
    MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_01H
*	功能说明: 读取线圈状态（对应远程开关D01/D02/D03）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_01H(void)
{
  /*
    主机发送:
      11 从机地址
      01 功能码
      00 寄存器起始地址高字节
      13 寄存器起始地址低字节
      00 寄存器数量高字节
      25 寄存器数量低字节
      0E CRC校验高字节
      84 CRC校验低字节

    从机应答: 	1代表ON，0代表OFF。若返回的线圈数不为8的倍数，则在最后数据字节未尾使用0代替. BIT0对应第1个
      11 从机地址
      01 功能码
      05 返回字节数
      CD 数据1(线圈0013H-线圈001AH)
      6B 数据2(线圈001BH-线圈0022H)
      B2 数据3(线圈0023H-线圈002AH)
      0E 数据4(线圈0032H-线圈002BH)
      1B 数据5(线圈0037H-线圈0033H)
      45 CRC校验高字节
      E6 CRC校验低字节

    例子:
      01 01 10 01 00 03   29 0B	--- 查询D01开始的3个继电器状态
      01 01 10 03 00 01   09 0A   --- 查询D03继电器的状态
  */
  uint16_t reg;
  uint16_t num;
  uint16_t i;
  uint16_t m;
  uint8_t status[MODS_DO_NUM / 8 + 1];

  g_tModS.RspCode = RSP_OK;

  /* 没有外部继电器，直接应答错误 */
  if (g_tModS.RxCount != 8)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  reg = BEBufToUint16(&g_tModS.RxBuf[2]); /* 寄存器号 */
  num = BEBufToUint16(&g_tModS.RxBuf[4]); /* 寄存器个数 */

  if (num > MODS_DO_NUM)
  {
    g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
    goto err_ret;
  }

  m = (num + 7) / 8;
  if (m < sizeof(status))
  {
    for (i = 0; i < m; i++)
    {
      status[i] = 0;
    }
    for (i = 0; i < num; i++)
    {
      uint8_t value;

      if (MODS_GetDIState(i + reg, &value))
      {
        if (value == 1)
        {
          status[i / 8] |= (1 << (i % 8));
        }
      }
      else
      {
        g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
      }
    }
  }
  else
  {
    g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
  }

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
      g_tModS.TxBuf[g_tModS.TxCount++] = m; /* 返回字节数 */

      for (i = 0; i < m; i++)
      {
        g_tModS.TxBuf[g_tModS.TxCount++] = status[i]; /* 继电器状态 */
      }
      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

static void MODS_02H(void)
{
  /*
    主机发送:
      11 从机地址
      02 功能码
      00 寄存器地址高字节
      C4 寄存器地址低字节
      00 寄存器数量高字节
      16 寄存器数量低字节
      BA CRC校验高字节
      A9 CRC校验低字节

    从机应答:  响应各离散输入寄存器状态，分别对应数据区中的每位值，1 代表ON；0 代表OFF。
               第一个数据字节的LSB(最低字节)为查询的寻址地址，其他输入口按顺序在该字节中由低字节
               向高字节排列，直到填充满8位。下一个字节中的8个输入位也是从低字节到高字节排列。
               若返回的输入位数不是8的倍数，则在最后的数据字节中的剩余位至该字节的最高位使用0填充。
      11 从机地址
      02 功能码
      03 返回字节数
      AC 数据1(00C4H-00CBH)
      DB 数据2(00CCH-00D3H)
      35 数据3(00D4H-00D9H)
      20 CRC校验高字节
      18 CRC校验低字节

    例子:
    01 02 20 01 00 08  23CC  ---- 读取T01-08的状态
    01 02 20 04 00 02  B3CA  ---- 读取T04-05的状态
    01 02 20 01 00 12  A207   ---- 读 T01-18
  */

  uint16_t reg;
  uint16_t num;
  uint16_t i;
  uint16_t m;
  uint8_t status[MODS_DI_NUM / 8 + 1];

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount != 8)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    return;
  }

  reg = BEBufToUint16(&g_tModS.RxBuf[2]); /* 寄存器号 */
  num = BEBufToUint16(&g_tModS.RxBuf[4]); /* 寄存器个数 */

  m = (num + 7) / 8;
  if (m > 0 && m < sizeof(status))
  {
    for (i = 0; i < m; i++)
    {
      status[i] = 0;
    }
    for (i = 0; i < num; i++)
    {
      uint8_t state;

      if (MODS_GetDIState(reg + i, &state))
      {
        if (state == 1)
        {
          status[i / 8] |= (1 << (i % 8));
        }
      }
      else
      {
        g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
        break;
      }
    }
  }
  else
  {
    g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
  }

  if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
  {
    g_tModS.TxCount = 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0];
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
    g_tModS.TxBuf[g_tModS.TxCount++] = m; /* 返回字节数 */

    for (i = 0; i < m; i++)
    {
      g_tModS.TxBuf[g_tModS.TxCount++] = status[i]; /* T01-02状态 */
    }
    MODS_SendWithCRC();
  }
  else
  {
    MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_05H
*	功能说明: 
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_05H(void)
{
  /*
    主机发送: 写单个线圈寄存器。FF00H值请求线圈处于ON状态，0000H值请求线圈处于OFF状态
    。05H指令设置单个线圈的状态，15H指令可以设置多个线圈的状态。
      11 从机地址
      05 功能码
      00 寄存器地址高字节
      AC 寄存器地址低字节
      FF 数据1高字节
      00 数据2低字节
      4E CRC校验高字节
      8B CRC校验低字节

    从机应答:
      11 从机地址
      05 功能码
      00 寄存器地址高字节
      AC 寄存器地址低字节
      FF 寄存器1高字节
      00 寄存器1低字节
      4E CRC校验高字节
      8B CRC校验低字节

    例子:
    01 05 10 01 FF 00   D93A   -- D01打开
    01 05 10 01 00 00   98CA   -- D01关闭

    01 05 10 02 FF 00   293A   -- D02打开
    01 05 10 02 00 00   68CA   -- D02关闭

    01 05 10 03 FF 00   78FA   -- D03打开
    01 05 10 03 00 00   390A   -- D03关闭
  */
  uint16_t reg;
  uint16_t value;

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount != 8)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  reg = BEBufToUint16(&g_tModS.RxBuf[2]);   /* 寄存器号 */
  value = BEBufToUint16(&g_tModS.RxBuf[4]); /* 数据 */

  if (value == 0xff00)
  {
    if (MODS_WriteRelay(reg, 1) == 0)
    {
      g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
    }
  }
  else if (value == 0x0000)
  {
    if (MODS_WriteRelay(reg, 0) == 0)
    {
      g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
    }
  }
  else
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 寄存器值域错误 */
  }

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      MODS_SendAckOk();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_0FH
*	功能说明: 写一个或多个输出继电器，改变输出继电器状态。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_0FH(void)
{
  uint16_t regaddr;
  uint16_t num;
  uint8_t m;
  uint8_t i;
  uint8_t *_pBuf;
  uint8_t status[MODS_DO_NUM / 8 + 1];

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount < 10)
  {
    goto fail;
  }

  regaddr = BEBufToUint16(&g_tModS.RxBuf[2]);
  num = BEBufToUint16(&g_tModS.RxBuf[4]);
  m = g_tModS.RxBuf[6];
  if ((m + 9) != g_tModS.RxCount)
  {
    g_tModS.RspCode = RSP_ERR_VALUE;
    goto fail;
  }
  if ((num + 7) / 8 != m)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 寄存器地址错误 */
    goto fail;
  }
  _pBuf = &g_tModS.RxBuf[7];

  if (m > 0 && m < sizeof(status))
  {
    for (i = 0; i < m; i++)
    {
      status[i] = *(_pBuf++);
    }
    for (i = 0; i < num; i++)
    {
      if (status[i / 8] & (1 << (i % 8)))
      {
        if (MODS_WriteRelay(regaddr, 1) == 0)
        {
          g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
          break;
        }
      }
      else
      {
        if (MODS_WriteRelay(regaddr, 0) == 0)
        {
          g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
          break;
        }
      }
      regaddr++;
    }
  }
  else
  {
    g_tModS.RspCode = RSP_ERR_REG_ADDR; /* 寄存器地址错误 */
  }

fail:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;

      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;

      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[2];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[3];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[4];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[5];

      MODS_SendWithCRC();
      return;
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_64H
*	功能说明: 小程序下载接口
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_64H(void)
{
  /*
    主机发送: 小程序数据
      01  ; 站号
      64  ; 功能码
      0100 0000 ; 总长度 4字节
      0000 0000 : 偏移地址 4字节
      0020 0000 : 本包数据长度 4字节
      xx ... xx : 程序数据，n个
      CCCC      : CRC16
  
    从机应答:
      01  ; 从机地址
      64  ; 功能码			
      00  ; 执行结果，0表示OK  1表示错误
      CCCC : CRC16
  */
  uint32_t total_len; /* 程序长度 */
  uint32_t offset_addr;
  uint32_t package_len; /* 本包数据长度 */

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount < 11)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }
  total_len = BEBufToUint32(&g_tModS.RxBuf[2]);
  offset_addr = BEBufToUint32(&g_tModS.RxBuf[6]);
  package_len = BEBufToUint32(&g_tModS.RxBuf[10]);

  lua_DownLoad(offset_addr, &g_tModS.RxBuf[14], package_len, total_len); /* 将lua程序保存到内存 */

  if (offset_addr + package_len >= total_len)
  {
    luaL_dostring(g_Lua, s_lua_prog_buf);
  }

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;             /* 功能码 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x00;             /* 执行结果 00 */
      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_65H
*	功能说明: 执行临时脚本
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_65H(void)
{
  /*
    65H - 执行临时的LUA程序，命令帧带程序
      01  ; 从机地址
      65  ; 功能码
      0100 : 本包数据长度 2字节
      xxxx : 脚本数据，0结束
      CCCC : CRC16
  
    从机应答:
      01  ; 从机地址
      65  ; 功能码
      00  ; 执行结果，0表示OK  1表示错误
      CCCC : CRC16	
      
  */
  //	uint16_t lual_len;		/* 程序长度 */

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount < 11)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }
  //	lual_len = BEBufToUint16(&g_tModS.RxBuf[2]);

  if (g_Lua > 0)
  {
    luaL_dostring(g_Lua, (char *)&g_tModS.RxBuf[4]);
  }

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x65;             /* 功能码 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x00;             /* 执行结果 00 */

      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_66H
*	功能说明: wirte文件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_66H(void)
{
  /*
    66H - write file
    主机发送:
      01  ; 站号
      66  ; 功能码
      0100 0000 ; 总长度 4字节
      0000 0000 : 偏移地址 4字节
      0020 0000 : 本包数据长度 4字节
      xx ... xx : 数据，n个
      CCCC      : CRC16	
      
    从机应答:
      01  ; 从机地址
      66  ; 功能码
      00  ; 执行结果，0表示OK  1表示错误
      CCCC : CRC16	
  */
  uint32_t total_len; /* 程序长度 */
  uint32_t offset_addr;
  uint32_t package_len; /* 本包数据长度 */

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount < 11)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }
  total_len = BEBufToUint32(&g_tModS.RxBuf[2]);
  offset_addr = BEBufToUint32(&g_tModS.RxBuf[6]);
  package_len = BEBufToUint32(&g_tModS.RxBuf[10]);

  lua_DownLoad(offset_addr, &g_tModS.RxBuf[14], package_len, total_len); /* 将数据保存到内存 */

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;             /* 功能码 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x00;             /* 执行结果 00 */
      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_67H
*	功能说明: read file
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_67H(void)
{
  /*
    67H -  read file
    主机发送:
      01  ; 站号
      67  ; 功能码
      0100 0000 ; 总长度 4字节 -- 先保留，好像用不到
      0000 0000 : 偏移地址 4字节
      0020 0000 : 数据长度 4字节
      CCCC      : CRC16	
      
    从机应答:
      01  ; 从机地址
      67  ; 功能码
      00  ; 执行结果，0表示OK  1表示错误
      0000 0000 : 偏移地址 4字节
      0000 0000 : 后续数据长度
      xxx  : 数据体
      CCCC : CRC16	

  */
  //	uint32_t total_len;		/* 程序长度 */
  uint32_t offset_addr;
  uint32_t package_len; /* 本包数据长度 */

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount < 11)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }
  //	total_len = BEBufToUint32(&g_tModS.RxBuf[2]);
  offset_addr = BEBufToUint32(&g_tModS.RxBuf[6]);
  package_len = BEBufToUint32(&g_tModS.RxBuf[10]);

  lua_67H_Read(offset_addr, &g_tModS.TxBuf[11], package_len);

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x67;             /* 功能码 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x00;             /* 执行结果 00 */

      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[6];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[7];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[8];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[9];

      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[10];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[11];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[12];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[13];

      g_tModS.TxCount += package_len;

      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: MODS_60H
*	功能说明: PC机读取波形数据（浮点格式）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_60H(void)
{
  /*
    PC发送 60H 
      01  ; 从机地址
      60  ; 功能码
      00  : 00表示PC下发，01表示设备应答 （仅仅用于人工分析）
      01 00 00 00  : 通道号使能标志 32bit，bit0表示CH1，bit1表示CH2
      00 00 04 00: 每个通道样本个数
      01 00 : 每通信包样本长度. 单位为1个样本。
      00 00 00 00 : 通道数据偏移 （样本单位，用于重发）
      CC CC : CRC16
  
    从机首先应答: 60H -  
      
      01  ; 从机地址
      60  ; 功能码
      01  : 00表示PC下发，01表示设备应答 （仅仅用于人工分析）
      01 00 00 00  : 通道号使能标志 32bit，bit0表示CH1，bit1表示CH2
      00 00 04 00 : 每个通道样本个数
      01 00 : 每通信包样本长度. 单位为1个样本。
      00 00 00 00 : 通道数据偏移 （样本单位，用于重发）
      CCCC : CRC16
  
    从机应答: （然后开始多包连续应答)
      01  ; 从机地址
      61  ; 功能码
      00  ; 通道号，00表示通道1,01表示通道2,
      00 00 00 00 : 偏移地址（样本单位）
      01 00 : 本包数据长度。样本单位。每个样本4字节。0x100表示1024字节。
      ..... : 数据体
      CCCC : CRC16
  */
  //	uint16_t lual_len;		/* 程序长度 */

  g_tModS.RspCode = RSP_OK;

  if (g_tModS.RxCount != 19)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  if (g_tModS.RxBuf[2] != 00)
  {
    g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    goto err_ret;
  }

  g_tModWave.ChEn = BEBufToUint32(&g_tModS.RxBuf[3]);
  g_tModWave.SampleSize = BEBufToUint32(&g_tModS.RxBuf[7]);
  g_tModWave.PackageSize = BEBufToUint16(&g_tModS.RxBuf[11]);
  g_tModWave.SampleOffset = BEBufToUint32(&g_tModS.RxBuf[13]);

  g_tModWave.TransPos = 0;   /* 传输的位置计数 */
  g_tModWave.StartTrans = 1; /* 开始传输的标志 */

err_ret:
  if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
  {
    if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
    {
      g_tModS.TxCount = 0;
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
      g_tModS.TxBuf[g_tModS.TxCount++] = 0x60;             /* 功能码 */
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[2];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[3];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[4];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[5];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[6];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[7];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[8];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[9];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[10];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[11];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[12];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[13];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[14];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[15];
      g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[16];

      MODS_SendWithCRC();
    }
    else
    {
      MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: Send_61H
*	功能说明: 传输波形。 自动连续多包传输。废弃，不稳定。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void Send_61H(uint8_t _Ch, uint32_t _Offset, uint16_t _PackageLen)
{
  /*
    从机应答: （然后开始多包连续应答)
      01  ; 从机地址
      61  ; 功能码
      00  ; 通道号，00表示通道1,01表示通道2,
      00 00 00 00 : 偏移地址（样本单位）
      01 00 : 本包数据长度。样本单位。每个样本4字节。0x100表示1024字节。
      ..... : 数据体
      CRC16
  */
  uint16_t i;
  uint8_t *p;

  g_tModS.TxCount = 0;
  g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
  g_tModS.TxBuf[g_tModS.TxCount++] = 0x61;             /* 功能码 */
  g_tModS.TxBuf[g_tModS.TxCount++] = _Ch;
  g_tModS.TxBuf[g_tModS.TxCount++] = _Offset >> 24;
  g_tModS.TxBuf[g_tModS.TxCount++] = _Offset >> 16;
  g_tModS.TxBuf[g_tModS.TxCount++] = _Offset >> 8;
  g_tModS.TxBuf[g_tModS.TxCount++] = _Offset;
  g_tModS.TxBuf[g_tModS.TxCount++] = _PackageLen >> 8;
  g_tModS.TxBuf[g_tModS.TxCount++] = _PackageLen;

  if (_Ch == 0)
  {
    for (i = 0; i < _PackageLen; i++)
    {
      p = (uint8_t *)&g_Ch1WaveBuf[_Offset + i];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[3];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[2];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[1];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[0];
    }
  }
  else if (_Ch == 1)
  {
    for (i = 0; i < _PackageLen; i++)
    {
      p = (uint8_t *)&g_Ch2WaveBuf[_Offset + i];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[3];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[2];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[1];
      g_tModS.TxBuf[g_tModS.TxCount++] = p[0];
    }
  }

  MODS_SendWithCRC();

  USBCom_SendBufNow(0, g_tModS.TxBuf, g_tModS.TxCount);
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: TransWaveTask
*	功能说明: 传输波形任务。 插入bsp_Idle运行. 废弃
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TransWaveTask(void)
{
  static uint32_t s_TransPos = 0;
  static uint8_t s_ChPos = 0;
    
  if (g_tModWave.StartTrans == 0)
  {
    return;
  }
  
  while (g_tModWave.StartTrans != 0)
  {
    lwip_pro();		/* 以太网协议栈轮询 */	
  
    wifi_task();
    
    switch (g_tModWave.StartTrans)
    {
      case 1:
        s_ChPos = 0;
        s_TransPos = g_tModWave.SampleOffset;
        g_tModWave.StartTrans++;
        break;
      
      case 2:
        if (g_tModWave.ChEn & (1 << s_ChPos))
        {
          g_tModWave.StartTrans++;
        }
        else
        {
          s_ChPos++;
          
          if (s_ChPos >= 2)
          {
            g_tModWave.StartTrans = 100;	/* 传输完毕 */
          }
          else
          {
            g_tModWave.StartTrans++;
          }
        }
        break;
      
      case 3:
        Send_61H(s_ChPos, s_TransPos, g_tModWave.PackageSize);
        g_tModWave.StartTrans++;
        break;
      
      case 4:	/* 等待发送完毕 - 暂时未做 */
        g_tModWave.StartTrans++;
        break;
      
      case 5: 
        s_TransPos += g_tModWave.PackageSize;
        if (s_TransPos >= g_tModWave.SampleSize)
        {
          s_ChPos++;
          s_TransPos = g_tModWave.SampleOffset;
          g_tModWave.StartTrans = 2;
        }
        else
        {
          g_tModWave.StartTrans = 3;
        }
        break;
      
      case 100:
        g_tModWave.StartTrans = 0;	/* 传输结束 */
        break;
    }
  }
}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
