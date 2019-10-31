/*
*********************************************************************************************************
*
*	模块名称 : VS1053B mp3解码器模块
*	文件名称 : bsp_vs1053b.c
*	版    本 : V1.0
*	说    明 : VS1053B芯片底层驱动。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-07-12 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
  安富莱STM32-V7开发板和VS1053B的口线连接：
    PB3/SPI3_SCK
    PB4/SPI3_MISO
    PB5/SPI3_MOSI

    PB8/NRF905_DR/VS1053_DREQ
    PC2/NRF905_CSN/VS1053_XCS
    
    扩展IO /NRF905_TRX_CE/VS1053_XDCS  
    
    扩展IO /VS1053_XREST
  
  晶振 = 12.288MHz
  内部时钟CLKI = 36.864MHz
  SPI读取时钟最大： CLKI / 7 = 5.266MHz
  写入时钟最大： CLKI / 4 = 9.216M
  
  实测 SPI_BAUDRATEPRESCALER_1_5625M 正常，
  SPI_BAUDRATEPRESCALER_3_125M 不正常
*/

/* VS1053插座的片选GPIO端口 XCS  */
#define XCS_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define XCS_GPIO GPIOC
#define XCS_PIN GPIO_PIN_2
#define XCS_0() XCS_GPIO->BSRRH = XCS_PIN
#define XCS_1() XCS_GPIO->BSRRL = XCS_PIN

/* XDCS 数据访问的片选, 用扩展IO实现 */
#define XDCS_0() HC574_SetPin(VS1053_XDCS, 0);
#define XDCS_1() HC574_SetPin(VS1053_XDCS, 1);

/* RST 低电平复位, 用扩展IO实现 */
#define XRESET_0() HC574_SetPin(VS1053_XRESET, 0);
#define XRESET_1() HC574_SetPin(VS1053_XRESET, 1);

/* DREQ输入脚 */
#define DREQ_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define DREQ_GPIO GPIOB
#define DREQ_PIN GPIO_PIN_8
#define DREQ_IS_HIGH() ((DREQ_GPIO->IDR & DREQ_PIN) != 0)

#define DUMMY_BYTE 0xFF /* 可定义任意值 */

uint8_t vs1053ram[5] = {0, 0, 0, 0, 250};

const uint16_t plugin[605] = {
    /* Compressed plugin */
    0x0007,
    0x0001,
    0x8300,
    0x0006,
    0x01f2,
    0xb080,
    0x0024,
    0x0007, /*    0 */
    0x9257,
    0x3f00,
    0x0024,
    0x0030,
    0x0297,
    0x3f00,
    0x0024,
    0x0006, /*    8 */
    0x0017,
    0x3f10,
    0x0024,
    0x3f00,
    0x0024,
    0x0000,
    0xf6d7,
    0xf400, /*   10 */
    0x55c0,
    0x0000,
    0x0817,
    0xf400,
    0x57c0,
    0x0000,
    0x004d,
    0x000a, /*   18 */
    0x708f,
    0x0000,
    0xc44e,
    0x280f,
    0xe100,
    0x0006,
    0x2016,
    0x0000, /*   20 */
    0x028d,
    0x0014,
    0x1b01,
    0x2800,
    0xc795,
    0x0015,
    0x59c0,
    0x0000, /*   28 */
    0xfa0d,
    0x0039,
    0x324f,
    0x0000,
    0xd20e,
    0x2920,
    0x41c0,
    0x0000, /*   30 */
    0x0024,
    0x000a,
    0x708f,
    0x0000,
    0xc44e,
    0x280a,
    0xcac0,
    0x0000, /*   38 */
    0x028d,
    0x6fc2,
    0x0024,
    0x000c,
    0x0981,
    0x2800,
    0xcad5,
    0x0000, /*   40 */
    0x18c2,
    0x290c,
    0x4840,
    0x3613,
    0x0024,
    0x290c,
    0x4840,
    0x4086, /*   48 */
    0x184c,
    0x6234,
    0x0024,
    0x0000,
    0x0024,
    0x2800,
    0xcad5,
    0x0030, /*   50 */
    0x0317,
    0x3f00,
    0x0024,
    0x280a,
    0x71c0,
    0x002c,
    0x9d40,
    0x3613, /*   58 */
    0x0024,
    0x3e10,
    0xb803,
    0x3e14,
    0x3811,
    0x3e11,
    0x3805,
    0x3e00, /*   60 */
    0x3801,
    0x0007,
    0xc390,
    0x0006,
    0xa011,
    0x3010,
    0x0444,
    0x3050, /*   68 */
    0x4405,
    0x6458,
    0x0302,
    0xff94,
    0x4081,
    0x0003,
    0xffc5,
    0x48b6, /*   70 */
    0x0024,
    0xff82,
    0x0024,
    0x42b2,
    0x0042,
    0xb458,
    0x0003,
    0x4cd6, /*   78 */
    0x9801,
    0xf248,
    0x1bc0,
    0xb58a,
    0x0024,
    0x6de6,
    0x1804,
    0x0006, /*   80 */
    0x0010,
    0x3810,
    0x9bc5,
    0x3800,
    0xc024,
    0x36f4,
    0x1811,
    0x36f0, /*   88 */
    0x9803,
    0x283e,
    0x2d80,
    0x0fff,
    0xffc3,
    0x003e,
    0x2d4f,
    0x2800, /*   90 */
    0xe380,
    0x0000,
    0xcb4e,
    0x3413,
    0x0024,
    0x2800,
    0xd405,
    0xf400, /*   98 */
    0x4510,
    0x2800,
    0xd7c0,
    0x6894,
    0x13cc,
    0x3000,
    0x184c,
    0x6090, /*   a0 */
    0x93cc,
    0x38b0,
    0x3812,
    0x3004,
    0x4024,
    0x0000,
    0x0910,
    0x3183, /*   a8 */
    0x0024,
    0x3100,
    0x4024,
    0x6016,
    0x0024,
    0x000c,
    0x8012,
    0x2800, /*   b0 */
    0xd711,
    0xb884,
    0x104c,
    0x6894,
    0x3002,
    0x2939,
    0xb0c0,
    0x3e10, /*   b8 */
    0x93cc,
    0x4084,
    0x9bd2,
    0x4282,
    0x0024,
    0x0000,
    0x0041,
    0x2800, /*   c0 */
    0xd9c5,
    0x6212,
    0x0024,
    0x0000,
    0x0040,
    0x2800,
    0xdec5,
    0x000c, /*   c8 */
    0x8390,
    0x2a00,
    0xe240,
    0x34c3,
    0x0024,
    0x3444,
    0x0024,
    0x3073, /*   d0 */
    0x0024,
    0x3053,
    0x0024,
    0x3000,
    0x0024,
    0x6092,
    0x098c,
    0x0000, /*   d8 */
    0x0241,
    0x2800,
    0xe245,
    0x32a0,
    0x0024,
    0x6012,
    0x0024,
    0x0000, /*   e0 */
    0x0024,
    0x2800,
    0xe255,
    0x0000,
    0x0024,
    0x3613,
    0x0024,
    0x3001, /*   e8 */
    0x3844,
    0x2920,
    0x0580,
    0x3009,
    0x3852,
    0xc090,
    0x9bd2,
    0x2800, /*   f0 */
    0xe240,
    0x3800,
    0x1bc4,
    0x000c,
    0x4113,
    0xb880,
    0x2380,
    0x3304, /*   f8 */
    0x4024,
    0x3800,
    0x05cc,
    0xcc92,
    0x05cc,
    0x3910,
    0x0024,
    0x3910, /*  100 */
    0x4024,
    0x000c,
    0x8110,
    0x3910,
    0x0024,
    0x39f0,
    0x4024,
    0x3810, /*  108 */
    0x0024,
    0x38d0,
    0x4024,
    0x3810,
    0x0024,
    0x38f0,
    0x4024,
    0x34c3, /*  110 */
    0x0024,
    0x3444,
    0x0024,
    0x3073,
    0x0024,
    0x3063,
    0x0024,
    0x3000, /*  118 */
    0x0024,
    0x4080,
    0x0024,
    0x0000,
    0x0024,
    0x2839,
    0x53d5,
    0x4284, /*  120 */
    0x0024,
    0x3613,
    0x0024,
    0x2800,
    0xe585,
    0x6898,
    0xb804,
    0x0000, /*  128 */
    0x0084,
    0x293b,
    0x1cc0,
    0x3613,
    0x0024,
    0x000c,
    0x8117,
    0x3711, /*  130 */
    0x0024,
    0x37d1,
    0x4024,
    0x4e8a,
    0x0024,
    0x0000,
    0x0015,
    0x2800, /*  138 */
    0xe845,
    0xce9a,
    0x0024,
    0x3f11,
    0x0024,
    0x3f01,
    0x4024,
    0x000c, /*  140 */
    0x8197,
    0x408a,
    0x9bc4,
    0x3f15,
    0x4024,
    0x2800,
    0xea85,
    0x4284, /*  148 */
    0x3c15,
    0x6590,
    0x0024,
    0x0000,
    0x0024,
    0x2839,
    0x53d5,
    0x4284, /*  150 */
    0x0024,
    0x0000,
    0x0024,
    0x2800,
    0xd2d8,
    0x458a,
    0x0024,
    0x2a39, /*  158 */
    0x53c0,
    0x3009,
    0x3851,
    0x3e14,
    0xf812,
    0x3e12,
    0xb817,
    0x0006, /*  160 */
    0xa057,
    0x3e11,
    0x9fd3,
    0x0023,
    0xffd2,
    0x3e01,
    0x0024,
    0x0006, /*  168 */
    0x0011,
    0x3111,
    0x0024,
    0x6498,
    0x07c6,
    0x868c,
    0x2444,
    0x3901, /*  170 */
    0x8e06,
    0x0030,
    0x0551,
    0x3911,
    0x8e06,
    0x3961,
    0x9c44,
    0xf400, /*  178 */
    0x44c6,
    0xd46c,
    0x1bc4,
    0x36f1,
    0xbc13,
    0x2800,
    0xf615,
    0x36f2, /*  180 */
    0x9817,
    0x002b,
    0xffd2,
    0x3383,
    0x188c,
    0x3e01,
    0x8c06,
    0x0006, /*  188 */
    0xa097,
    0x468c,
    0xbc17,
    0xf400,
    0x4197,
    0x2800,
    0xf304,
    0x3713, /*  190 */
    0x0024,
    0x2800,
    0xf345,
    0x37e3,
    0x0024,
    0x3009,
    0x2c17,
    0x3383, /*  198 */
    0x0024,
    0x3009,
    0x0c06,
    0x468c,
    0x4197,
    0x0006,
    0xa052,
    0x2800, /*  1a0 */
    0xf544,
    0x3713,
    0x2813,
    0x2800,
    0xf585,
    0x37e3,
    0x0024,
    0x3009, /*  1a8 */
    0x2c17,
    0x36f1,
    0x8024,
    0x36f2,
    0x9817,
    0x36f4,
    0xd812,
    0x2100, /*  1b0 */
    0x0000,
    0x3904,
    0x5bd1,
    0x2a00,
    0xeb8e,
    0x3e11,
    0x7804,
    0x0030, /*  1b8 */
    0x0257,
    0x3701,
    0x0024,
    0x0013,
    0x4d05,
    0xd45b,
    0xe0e1,
    0x0007, /*  1c0 */
    0xc795,
    0x2800,
    0xfd95,
    0x0fff,
    0xff45,
    0x3511,
    0x184c,
    0x4488, /*  1c8 */
    0xb808,
    0x0006,
    0x8a97,
    0x2800,
    0xfd45,
    0x3009,
    0x1c40,
    0x3511, /*  1d0 */
    0x1fc1,
    0x0000,
    0x0020,
    0xac52,
    0x1405,
    0x6ce2,
    0x0024,
    0x0000, /*  1d8 */
    0x0024,
    0x2800,
    0xfd41,
    0x68c2,
    0x0024,
    0x291a,
    0x8a40,
    0x3e10, /*  1e0 */
    0x0024,
    0x2921,
    0xca80,
    0x3e00,
    0x4024,
    0x36f3,
    0x0024,
    0x3009, /*  1e8 */
    0x1bc8,
    0x36f0,
    0x1801,
    0x2808,
    0x9300,
    0x3601,
    0x5804,
    0x0007, /*  1f0 */
    0x0001,
    0x802e,
    0x0006,
    0x0002,
    0x2800,
    0xf700,
    0x0007,
    0x0001, /*  1f8 */
    0x8050,
    0x0006,
    0x0028,
    0x3e12,
    0x3800,
    0x2911,
    0xf140,
    0x3e10, /*  200 */
    0x8024,
    0xf400,
    0x4595,
    0x3593,
    0x0024,
    0x35f3,
    0x0024,
    0x3500, /*  208 */
    0x0024,
    0x0021,
    0x6d82,
    0xd024,
    0x44c0,
    0x0006,
    0xa402,
    0x2800, /*  210 */
    0x1815,
    0xd024,
    0x0024,
    0x0000,
    0x0000,
    0x2800,
    0x1815,
    0x000b, /*  218 */
    0x6d57,
    0x3009,
    0x3c00,
    0x36f0,
    0x8024,
    0x36f2,
    0x1800,
    0x2000, /*  220 */
    0x0000,
    0x0000,
    0x0024,
    0x0007,
    0x0001,
    0x8030,
    0x0006,
    0x0002, /*  228 */
    0x2800,
    0x1400,
    0x0007,
    0x0001,
    0x8064,
    0x0006,
    0x001c,
    0x3e12, /*  230 */
    0xb817,
    0x3e14,
    0xf812,
    0x3e01,
    0xb811,
    0x0007,
    0x9717,
    0x0020, /*  238 */
    0xffd2,
    0x0030,
    0x11d1,
    0x3111,
    0x8024,
    0x3704,
    0xc024,
    0x3b81, /*  240 */
    0x8024,
    0x3101,
    0x8024,
    0x3b81,
    0x8024,
    0x3f04,
    0xc024,
    0x2808, /*  248 */
    0x4800,
    0x36f1,
    0x9811,
    0x0007,
    0x0001,
    0x8028,
    0x0006,
    0x0002, /*  250 */
    0x2a00,
    0x190e,
    0x000a,
    0x0001,
    0x0300,
};

/*
*********************************************************************************************************
*	函 数 名: VS1053_Init
*	功能说明: 初始化vs1053B硬件设备
*	形    参: 无
*	返 回 值: 1 表示初始化正常，0表示初始化不正常
*********************************************************************************************************
*/
void VS1053_Init(void)
{
  XCS_1();
  XDCS_1();

  /* 配置GPIO */
  {
    GPIO_InitTypeDef gpio_init;

    /* 打开GPIO时钟 */
    XCS_CLK_ENABLE();
    DREQ_CLK_ENABLE();

    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   /* 设置推挽输出 */
    gpio_init.Pull = GPIO_NOPULL;           /* 上下拉电阻不使能 */
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; /* GPIO速度等级 */

    gpio_init.Pin = XCS_PIN;
    HAL_GPIO_Init(XCS_GPIO, &gpio_init);

    /* DREQ 设置为输入 */
    gpio_init.Mode = GPIO_MODE_INPUT;       /* 设置输入 */
    gpio_init.Pull = GPIO_NOPULL;           /* 上下拉电阻不使能 */
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; /* GPIO速度等级 */
    gpio_init.Pin = DREQ_PIN;
    HAL_GPIO_Init(DREQ_GPIO, &gpio_init);
  }

  XRESET_0();
  bsp_DelayUS(2000);
  XRESET_1(); /* 退出复位状态 */
  bsp_DelayUS(2000);
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_SetXCS
*	功能说明: VS1053控制接口片选CSN控制函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_SetXCS(uint8_t _Level)
{
  if (_Level == 0)
  {
    bsp_SpiBusEnter(); /* 占用SPI总线  */
    bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_1_5625M, SPI_PHASE_1EDGE, SPI_POLARITY_LOW);
    XCS_0();
  }
  else
  {
    XCS_1();
    bsp_SpiBusExit(); /* 释放SPI总线 */
  }
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_SetXDCS
*	功能说明: VS1053数据接口片选XDCS控制函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_SetXDCS(uint8_t _Level)
{
  if (_Level == 0)
  {
    bsp_SpiBusEnter(); /* 占用SPI总线  */
    bsp_InitSPIParam(SPI_BAUDRATEPRESCALER_1_5625M, SPI_PHASE_1EDGE, SPI_POLARITY_LOW);
    XDCS_0();
  }
  else
  {
    XDCS_1();
    bsp_SpiBusExit(); /* 释放SPI总线 */
  }
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_WriteCmd
*	功能说明: 向vs1053写命令
*	形    参: _ucAddr ： 地址； 		_usData ：数据
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_WriteCmd(uint8_t _ucAddr, uint16_t _usData)
{
  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return;
  }

  VS1053_SetXCS(0);

  g_spiLen = 0;
  g_spiTxBuf[g_spiLen++] = (VS_WRITE_COMMAND); /* 发送vs1053的写命令 */
  g_spiTxBuf[g_spiLen++] = (_ucAddr);          /* 寄存器地址 */
  g_spiTxBuf[g_spiLen++] = (_usData >> 8);     /* 发送高8位 */
  g_spiTxBuf[g_spiLen++] = (_usData);          /* 发送低8位 */
  bsp_spiTransfer();

  VS1053_SetXCS(1);
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_ReqNewData
*	功能说明: 判断vs1053是否请求新数据。 vs1053内部有0.5k缓冲区。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t VS1053_ReqNewData(void)
{
  if (DREQ_IS_HIGH())
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_PreWriteData
*	功能说明: 准备向vs1053写数据，调用1次即可
*	形    参: _无
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_PreWriteData(void)
{
  XCS_1();
  XDCS_0();
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_WriteData
*	功能说明: 向vs1053写数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_WriteData(uint8_t _ucData)
{
  VS1053_SetXDCS(0);

  g_spiLen = 0;
  g_spiTxBuf[g_spiLen++] = (_ucData);
  bsp_spiTransfer();

  VS1053_SetXDCS(1);
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_ReadReg
*	功能说明: 读vs1053的寄存器
*	形    参: _ucAddr:寄存器地址
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
uint16_t VS1053_ReadReg(uint8_t _ucAddr)
{
  uint16_t usTemp;

  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return 0;
  }

  VS1053_SetXCS(0);

  g_spiTxBuf[0] = (VS_READ_COMMAND); /* 发送vs1053读命令 */
  g_spiTxBuf[1] = (_ucAddr);         /* 发送地址 */
  g_spiLen = 4;
  bsp_spiTransfer();

  usTemp = (g_spiRxBuf[2] << 8) + g_spiRxBuf[3];
  VS1053_SetXCS(1);
  return usTemp;
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_ReadChipID
*	功能说明: 读vs1053芯片的版本即ID， 用于识别是VS1003还是 VS1053
*	形    参: 无
*	返 回 值: 4bit的芯片ID。Version
*********************************************************************************************************
*/
uint8_t VS1053_ReadChipID(void)
{
  uint16_t usStatus;
  /* pdf page 40
    SCI STATUS 状态寄存器的 Bit7:4 表示芯片的版本
    0 for VS1001
    1 for VS1011
    2 for VS1002
    3 for VS1003
    4 for VS1053,
    5 for VS1033,
    7 for VS1103.
  */
  usStatus = VS1053_ReadReg(SCI_STATUS);

  return ((usStatus >> 4) & 0x000F);
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_WaitBusy
*	功能说明: 等待芯片内部结束操作。根据DREQ口线的状态识别芯片是否忙。该函数用于指令操作间延迟。
*	形    参: 无
*	返 回 值: 0 表示超时， 1表示
*********************************************************************************************************
*/
uint8_t VS1053_WaitTimeOut(void)
{
  uint32_t i;

  for (i = 0; i < 4000000; i++)
  {
    if (DREQ_IS_HIGH())
    {
      break;
    }
  }

  if (i >= 4000000)
  {
    return 1; /* 超时无应答，硬件异常 */
  }

  return 0; /* 正常返回 */
}

void LoadUserPatch(void)
{
  int i = 0;

  while (i < sizeof(plugin) / sizeof(plugin[0]))
  {
    unsigned short addr, n, val;

    addr = plugin[i++];
    n = plugin[i++];
    if (n & 0x8000U)
    {
      /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = plugin[i++];
      while (n--)
      {
        VS1053_WriteCmd(addr, val);
      }
    }
    else
    {
      /* Copy run, copy n samples */
      while (n--)
      {
        val = plugin[i++];
        VS1053_WriteCmd(addr, val);
      }
    }
  }
  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return;
  }
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_TestRam
*	功能说明: 测试vs1053B的内部RAM
*	形    参: 无
*	返 回 值: 1表示OK, 0表示错误.
*********************************************************************************************************
*/
uint8_t VS1053_TestRam(void)
{
  uint16_t usRegValue;

  VS1053_WriteCmd(SCI_MODE, 0x0820); /* 进入vs1053的测试模式 */

  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return 0;
  }

  VS1053_SetXDCS(0);

  g_spiLen = 0;
  g_spiTxBuf[g_spiLen++] = (0x4d);
  g_spiTxBuf[g_spiLen++] = (0xea);
  g_spiTxBuf[g_spiLen++] = (0x6d);
  g_spiTxBuf[g_spiLen++] = (0x54);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  bsp_spiTransfer();

  VS1053_SetXDCS(1);

  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return 0;
  }

  usRegValue = VS1053_ReadReg(SCI_HDAT0); /* 如果得到的值为0x807F，则表明OK */

  if (usRegValue == 0x807F)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_TestSine
*	功能说明: 正弦测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_TestSine(void)
{
  /*
    正弦测试通过有序的8字节初始化，0x53 0xEF 0x6E n 0 0 0 0
    想要退出正弦测试模式的话，发送如下序列 0x45 0x78 0x69 0x74 0 0 0 0 .

    这里的n被定义为正弦测试使用，定义
    如下：
    n bits
    名称位 描述
    FsIdx 7：5 采样率索引
    S 4：0 正弦跳过速度
    正弦输出频率可通过这个公式计算：F=Fs×(S/128).
    例如：正弦测试值为126 时被激活，二进制为
    0b01111110。则FsIdx=0b011=3,所以Fs=22050Hz。
    S=0b11110=30, 所以最终的正弦输出频率为
    F=22050Hz×30/128=5168Hz。


    正弦输出频率可通过这个公式计算：F = Fs×(S/128).
  */

  VS1053_WriteCmd(0x0b, 0x2020);     /* 设置音量	*/
  VS1053_WriteCmd(SCI_MODE, 0x0820); /* 进入vs1053的测试模式	*/

  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return;
  }

  /*
     进入正弦测试状态
     命令序列：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
     其中n = 0x24, 设定vs1053所产生的正弦波的频率值
   */
  VS1053_SetXDCS(0);

  g_spiLen = 0;
  g_spiTxBuf[g_spiLen++] = (0x53);
  g_spiTxBuf[g_spiLen++] = (0xef);
  g_spiTxBuf[g_spiLen++] = (0x6e);
  g_spiTxBuf[g_spiLen++] = (0x24); /* 0x24 or 0x44 */
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  bsp_spiTransfer();

  VS1053_SetXDCS(1);

  /* 退出正弦测试 */
  VS1053_SetXDCS(0);
  g_spiLen = 0;
  g_spiTxBuf[g_spiLen++] = (0x45);
  g_spiTxBuf[g_spiLen++] = (0x78);
  g_spiTxBuf[g_spiLen++] = (0x69);
  g_spiTxBuf[g_spiLen++] = (0x74);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  g_spiTxBuf[g_spiLen++] = (0x00);
  bsp_spiTransfer();
  VS1053_SetXDCS(1);
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_SoftReset
*	功能说明: 软复位vs1053。 在歌曲之间需要执行本函数。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_SoftReset(void)
{
  uint8_t retry;

  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return;
  }

  //VS1053_WriteByte(0X00);//启动传输
  retry = 0;
  while (VS1053_ReadReg(SCI_MODE) != 0x0804) // 软件复位,新模式
  {
    /* 等待至少1.35ms  */
    VS1053_WriteCmd(SCI_MODE, 0x0804); // 软件复位,新模式

    /* 等待芯片内部操作完成 */
    if (VS1053_WaitTimeOut())
    {
      return;
    }

    if (retry++ > 5)
    {
      break;
    }
  }

#if 0
  VS1053_WriteCmd(SCI_CLOCKF,0x9800);
  VS1053_WriteCmd(SCI_AUDATA,0xBB81); /* 采样率48k，立体声 */

  VS1053_WriteCmd(SCI_BASS, 0x0000);	/* */
    VS1053_WriteCmd(SCI_VOL, 0x2020); 	/* 设置为最大音量,0是最大  */

  ResetDecodeTime();	/* 复位解码时间	*/

    /* 向vs1053发送4个字节无效数据，用以启动SPI发送 */
    VS1053_DS_0();//选中数据传输
  VS1053_WriteByte(0xFF);
  VS1053_WriteByte(0xFF);
  VS1053_WriteByte(0xFF);
  VS1053_WriteByte(0xFF);
  VS1053_DS_1();//取消数据传输
#else
  /* Set clock register, doubler etc. */
  VS1053_WriteCmd(SCI_CLOCKF, 0xA000);

  //VS1053_WriteCmd(SCI_BASS, 0x0000);	/* 低音高音增强控制， 0表示不启用 */
  //VS1053_WriteCmd(SCI_VOL, 0x2020); 	/* 设置为最大音量,0是最大  */

  /* 等待芯片内部操作完成 */
  if (VS1053_WaitTimeOut())
  {
    return;
  };
  LoadUserPatch();
#endif
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_SetVolume
*	功能说明: 设置vs1053音量。0 是静音， 254最大
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_SetVolume(uint8_t _ucVol)
{

  /* 对于 VS1053， 0表示最大音量，254表示静音 */
  if (_ucVol == 0)
  {
    _ucVol = 254;
  }
  else if (_ucVol == 255)
  {
    _ucVol = 0;
  }
  else
  {
    _ucVol = 254 - _ucVol;
  }

  VS1053_WriteCmd(SCI_VOL, (_ucVol << 8) | _ucVol);
}

/*
*********************************************************************************************************
*	函 数 名: VS1053_SetBASS
*	功能说明: 设置高音增强和低音增强
*	形    参: _cHighAmp     : 高音增强幅度 【-8, 7】  (0是关闭)
*			 _usHighFreqCut : 高音增强截止频率 【1000, 15000】 Hz
*			 _ucLowAmp      : 低音增强幅度 【0, 15】  (0是关闭)
*			 _usLowFreqCut : 低音增强截止频率 【20, 150】 Hz
*	返 回 值: 无
*********************************************************************************************************
*/
void VS1053_SetBASS(int8_t _cHighAmp, uint16_t _usHighFreqCut, uint8_t _ucLowAmp, uint16_t _usLowFreqCut)
{
  uint16_t usValue;

  /*
    SCI_BASS 寄存器定义:

    Bit15:12  高音控制 -8 ... 7  (0是关闭)
    Bit11:8   下限频率,单位1KHz,  1...15

    Bit7:4    低音控制 0...15 (0是关闭)
    Bit3:0    上限频率,单位10Hz, 2...15
  */

  /* 高音增强幅度 */
  if (_cHighAmp < -8)
  {
    _cHighAmp = -8;
  }
  else if (_cHighAmp > 7)
  {
    _cHighAmp = 7;
  }
  usValue = _cHighAmp << 12;

  /* 高音增强截止频率 */
  if (_usHighFreqCut < 1000)
  {
    _usHighFreqCut = 1000;
  }
  else if (_usHighFreqCut > 15000)
  {
    _usHighFreqCut = 15000;
  }
  usValue += ((_usHighFreqCut / 1000) << 8);

  /* 低音增强幅度 */
  if (_ucLowAmp > 15)
  {
    _ucLowAmp = 15;
  }
  usValue += (_ucLowAmp << 4);

  /* 低音增强截止频率 */
  if (_usLowFreqCut < 20)
  {
    _usLowFreqCut = 20;
  }
  else if (_usLowFreqCut > 150)
  {
    _usLowFreqCut = 150;
  }
  usValue += (_usLowFreqCut / 10);

  VS1053_WriteCmd(SCI_BASS, usValue);
}

/*
*********************************************************************************************************
*	函 数 名: ResetDecodeTime
*	功能说明: 重设解码时间
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void ResetDecodeTime(void)
{
  VS1053_WriteCmd(SCI_DECODE_TIME, 0x0000);
}

/*
*********************************************************************************************************
*	下面的代码还未调试
*********************************************************************************************************
*/

#if 0

//ram 测试
void VsRamTest(void)
{
  uint16_t u16 regvalue ;

  Mp3Reset();
   VS1053_CMD_Write(SPI_MODE,0x0820);// 进入vs1053的测试模式
  while ((GPIOC->IDR&MP3_DREQ)==0); // 等待DREQ为高
   MP3_DCS_SET(0);	       			  // xDCS = 1，选择vs1053的数据接口
  SPI1_ReadWriteByte(0x4d);
  SPI1_ReadWriteByte(0xea);
  SPI1_ReadWriteByte(0x6d);
  SPI1_ReadWriteByte(0x54);
  SPI1_ReadWriteByte(0x00);
  SPI1_ReadWriteByte(0x00);
  SPI1_ReadWriteByte(0x00);
  SPI1_ReadWriteByte(0x00);
  delay_ms(50);
  MP3_DCS_SET(1);
  regvalue=VS1053_REG_Read(SPI_HDAT0); // 如果得到的值为0x807F，则表明完好。
  printf("regvalueH:%x\n",regvalue>>8);//输出结果
  printf("regvalueL:%x\n",regvalue&0xff);//输出结果
}

//FOR WAV HEAD0 :0X7761 HEAD1:0X7665
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//比特率预定值
const uint16_t bitrate[2][16]=
{
  {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
  {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//返回Kbps的大小
//得到mp3&wma的波特率
uint16_t GetHeadInfo(void)
{
  unsigned int HEAD0;
  unsigned int HEAD1;

    HEAD0=VS1053_REG_Read(SPI_HDAT0);
    HEAD1=VS1053_REG_Read(SPI_HDAT1);
    switch(HEAD1)
    {
        case 0x7665:return 0;//WAV格式
        case 0X4D54:return 1;//MIDI格式
        case 0X574D://WMA格式
        {
            HEAD1=HEAD0*2/25;
            if((HEAD1%10)>5)return HEAD1/10+1;
            else return HEAD1/10;
        }
        default://MP3格式
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03;
            if(HEAD1==3)HEAD1=1;
            else HEAD1=0;
            return bitrate[HEAD1][HEAD0>>12];
        }
    }
}

//得到mp3的播放时间n sec
uint16_t GetDecodeTime(void)
{
    return VS1053_REG_Read(SPI_DECODE_TIME);
}
//加载频谱分析的代码到vs1053
void LoadPatch(void)
{
  uint16_t i;

  for (i=0;i<943;i++)VS1053_CMD_Write(atab[i],dtab[i]);
  delay_ms(10);
}
//得到频谱数据
void GetSpec(u8 *p)
{
  u8 byteIndex=0;
  u8 temp;
  VS1053_CMD_Write(SPI_WRAMADDR,0x1804);
  for (byteIndex=0;byteIndex<14;byteIndex++)
  {
    temp=VS1053_REG_Read(SPI_WRAM)&0x63;//取小于100的数
    *p++=temp;
  }
}

//设定vs1053播放的音量和高低音
void set1003(void)
{
    uint8 t;
    uint16_t bass=0; //暂存音调寄存器值
    uint16_t volt=0; //暂存音量值
    uint8_t vset=0;  //暂存音量值

    vset=255-vs1053ram[4];//取反一下,得到最大值,表示最大的表示
    volt=vset;
    volt<<=8;
    volt+=vset;//得到音量设置后大小
     //0,henh.1,hfreq.2,lenh.3,lfreq
    for(t=0;t<4;t++)
    {
        bass<<=4;
        bass+=vs1053ram[t];
    }
  VS1053_CMD_Write(SPI_BASS, 0x0000);//BASS
    VS1053_CMD_Write(SPI_VOL, 0x0000); //设音量
}

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
