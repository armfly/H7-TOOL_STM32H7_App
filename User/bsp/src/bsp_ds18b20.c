/*
*********************************************************************************************************
*
*	模块名称 : DS18B20 驱动模块(1-wire 数字温度传感器）
*	文件名称 : bsp_ds18b20.c
*	版    本 : V1.0
*	说    明 : DS18B20和CPU之间采用1个GPIO接口。
*
*	修改记录 :
*		版本号  日期         作者     说明
*		V1.0    2014-01-24  armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
  DS18B20 可以直接查到STM32-V5开发板的U16 (3P) 插座.

    DS18B20     STM32F407开发板
    VCC   ------  3.3V
    DQ    ------  PB1   (开发板上有 4.7K 上拉电阻)
    GND   ------  GND
*/

/* 定义GPIO端口 */
#define DQ_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define DQ_GPIO GPIOB
#define DQ_PIN GPIO_PIN_1

#define DQ_0() DQ_GPIO->BSRRH = DQ_PIN
#define DQ_1() DQ_GPIO->BSRRL = DQ_PIN

/* 判断DQ输入是否为低 */
#define DQ_IS_LOW() ((DQ_GPIO->IDR & DQ_PIN) == 0)

/*
*********************************************************************************************************
*	函 数 名: bsp_InitDS18B20
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 DS18B20
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDS18B20(void)
{
  GPIO_InitTypeDef gpio_init;

  /* 打开GPIO时钟 */
  DQ_CLK_ENABLE();

  DQ_1();

  /* 配置DQ为开漏输出 */
  gpio_init.Mode = GPIO_MODE_OUTPUT_OD;  /* 设置开漏输出 */
  gpio_init.Pull = GPIO_NOPULL;          /* 上下拉电阻不使能 */
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW; /* GPIO速度等级 */
  gpio_init.Pin = DQ_PIN;
  HAL_GPIO_Init(DQ_GPIO, &gpio_init);
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_Reset
*	功能说明: 复位DS18B20。 拉低DQ为低，持续最少480us，然后等待
*	形    参: 无
*	返 回 值: 0 失败； 1 表示成功
*********************************************************************************************************
*/
uint8_t DS18B20_Reset(void)
{
  /*
    复位时序, 见DS18B20 page 15

    首先主机拉低DQ，持续最少 480us
    然后释放DQ，等待DQ被上拉电阻拉高，约 15-60us
    DS18B20 将驱动DQ为低 60-240us， 这个信号叫 presence pulse  (在位脉冲,表示DS18B20准备就绪 可以接受命令)
    如果主机检测到这个低应答信号，表示DS18B20复位成功
  */

  uint8_t i;
  uint16_t k;

  DISABLE_INT(); /* 禁止全局中断 */

  /* 复位，如果失败则返回0 */
  for (i = 0; i < 1; i++)
  {
    DQ_0();           /* 拉低DQ */
    bsp_DelayUS(520); /* 延迟 520uS， 要求这个延迟大于 480us */
    DQ_1();           /* 释放DQ */

    bsp_DelayUS(15); /* 等待15us */

    /* 检测DQ电平是否为低 */
    for (k = 0; k < 10; k++)
    {
      if (DQ_IS_LOW())
      {
        break;
      }
      bsp_DelayUS(10); /* 等待65us */
    }
    if (k >= 10)
    {
      continue; /* 失败 */
    }

    /* 等待DS18B20释放DQ */
    for (k = 0; k < 30; k++)
    {
      if (!DQ_IS_LOW())
      {
        break;
      }
      bsp_DelayUS(10); /* 等待65us */
    }
    if (k >= 30)
    {
      continue; /* 失败 */
    }

    break;
  }

  ENABLE_INT(); /* 使能全局中断 */

  bsp_DelayUS(5);

  if (i >= 1)
  {
    return 0;
  }

  return 1;
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_WriteByte
*	功能说明: 向DS18B20写入1字节数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DS18B20_WriteByte(uint8_t _val)
{
  /*
    写数据时序, 见DS18B20 page 16
  */
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    DQ_0();
    bsp_DelayUS(2);

    if (_val & 0x01)
    {
      DQ_1();
    }
    else
    {
      DQ_0();
    }
    bsp_DelayUS(60);
    DQ_1();
    bsp_DelayUS(2);
    _val >>= 1;
  }
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadByte
*	功能说明: 向DS18B20读取1字节数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t DS18B20_ReadByte(void)
{
  /*
    写数据时序, 见DS18B20 page 16
  */
  uint8_t i;
  uint8_t read = 0;

  for (i = 0; i < 8; i++)
  {
    read >>= 1;

    DQ_0();
    bsp_DelayUS(3);
    DQ_1();
    bsp_DelayUS(3);

    if (DQ_IS_LOW())
    {
      ;
    }
    else
    {
      read |= 0x80;
    }
    bsp_DelayUS(60);
  }

  return read;
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadTempReg
*	功能说明: 读温度寄存器的值（原始数据）
*	形    参: 无
*	返 回 值: 温度寄存器数据 （除以16得到 1摄氏度单位, 也就是小数点前面的数字)
*********************************************************************************************************
*/
int16_t DS18B20_ReadTempReg(void)
{
  uint8_t temp1, temp2;

  /* 总线复位 */
  if (DS18B20_Reset() == 0)
  {
    return 0;
  }

  DS18B20_WriteByte(0xcc); /* 发命令 */
  DS18B20_WriteByte(0x44); /* 发转换命令 */

  DS18B20_Reset(); /* 总线复位 */

  DS18B20_WriteByte(0xcc); /* 发命令 */
  DS18B20_WriteByte(0xbe);

  temp1 = DS18B20_ReadByte(); /* 读温度值低字节 */
  temp2 = DS18B20_ReadByte(); /* 读温度值高字节 */

  return ((temp2 << 8) | temp1); /* 返回16位寄存器值 */
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadID
*	功能说明: 读DS18B20的ROM ID， 总线上必须只有1个芯片
*	形    参: _id 存储ID
*	返 回 值: 0 表示失败， 1表示检测到正确ID
*********************************************************************************************************
*/
uint8_t DS18B20_ReadID(uint8_t *_id)
{
  uint8_t i;

  /* 总线复位 */
  if (DS18B20_Reset() == 0)
  {
    return 0;
  }

  DS18B20_WriteByte(0x33); /* 发命令 */
  for (i = 0; i < 8; i++)
  {
    _id[i] = DS18B20_ReadByte();
  }

  DS18B20_Reset(); /* 总线复位 */

  return 1;
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadTempByID
*	功能说明: 读指定ID的温度寄存器的值（原始数据）
*	形    参: 无
*	返 回 值: 温度寄存器数据 （除以16得到 1摄氏度单位, 也就是小数点前面的数字)
*********************************************************************************************************
*/
int16_t DS18B20_ReadTempByID(uint8_t *_id)
{
  uint8_t temp1, temp2;
  uint8_t i;

  DS18B20_Reset(); /* 总线复位 */

  DS18B20_WriteByte(0x55); /* 发命令 */
  for (i = 0; i < 8; i++)
  {
    DS18B20_WriteByte(_id[i]);
  }
  DS18B20_WriteByte(0x44); /* 发转换命令 */

  DS18B20_Reset(); /* 总线复位 */

  DS18B20_WriteByte(0x55); /* 发命令 */
  for (i = 0; i < 8; i++)
  {
    DS18B20_WriteByte(_id[i]);
  }
  DS18B20_WriteByte(0xbe);

  temp1 = DS18B20_ReadByte(); /* 读温度值低字节 */
  temp2 = DS18B20_ReadByte(); /* 读温度值高字节 */

  return ((temp2 << 8) | temp1); /* 返回16位寄存器值 */
}

/*
*********************************************************************************************************
*	函 数 名: DS18B20_ReadTempRegCRC
*	功能说明: 读温度寄存器的值（原始数据），带CRC校验。
*	形    参: 无
*	返 回 值: 温度寄存器数据 （除以16得到 1摄氏度单位, 也就是小数点前面的数字)
*********************************************************************************************************
*/
uint8_t dallas_crc8(uint8_t *data, uint8_t size)
{
  unsigned char crc = 0, inbyte, j, mix;
  unsigned int i;

  for (i = 0; i < size; ++i)
  {
    inbyte = data[i];
    for (j = 0; j < 8; ++j)
    {
      mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      inbyte >>= 1;
    }
  }

  return crc;
}

uint8_t DS18B20_ReadTempRegCRC(int16_t *read_temp)
{
  uint8_t i;
  uint8_t temp[10], crc;

  for (i = 0; i < 3; i++)
  {
    /* 总线复位 */
    if (DS18B20_Reset() == 0)
    {
      continue;
    }

    DS18B20_WriteByte(0xcc); /* 发命令 */
    DS18B20_WriteByte(0x44); /* 发转换命令 */

    DS18B20_Reset(); /* 总线复位 */

    DS18B20_WriteByte(0xcc); /* 发命令 */
    DS18B20_WriteByte(0xbe);

    temp[0] = DS18B20_ReadByte(); /* 读温度值低字节 */
    temp[1] = DS18B20_ReadByte(); /* 读温度值高字节 */

    temp[2] = DS18B20_ReadByte(); /* Alarm High Byte    */
    temp[3] = DS18B20_ReadByte(); /* Alarm Low Byte     */
    temp[4] = DS18B20_ReadByte(); /* Reserved Byte 1    */
    temp[5] = DS18B20_ReadByte(); /* Reserved Byte 2    */
    temp[6] = DS18B20_ReadByte(); /* Count Remain Byte  */
    temp[7] = DS18B20_ReadByte(); /* Count Per degree C */
    crc = DS18B20_ReadByte();

    /* 最高的125°对应0x07D0，这里返回0x0800表示错误*/
    if (crc != dallas_crc8(temp, 8))
    {
      continue;
    }

    *read_temp = (temp[1] << 8) | temp[0]; /* 返回16位寄存器值 */

    return 1; /* 正确 */
  }

  return 0; /* 出错 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
