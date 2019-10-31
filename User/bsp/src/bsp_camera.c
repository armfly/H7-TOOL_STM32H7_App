/*
*********************************************************************************************************
*
*	模块名称 : 摄像头驱动BSP模块(For OV7670)
*	文件名称 : bsp_camera.c
*	版    本 : V1.0
*	说    明 : OV7670驱动程序。本程序适用于 guanfu_wang  的OV7670摄像头（不带FIFO,不带LDO，不带24M晶振)
*			  安富莱STM32-V5开发板集成了3.0V LDO给OV7670供电，主板集成了24M有源晶振提供给摄像头。
*
*			  本代码参考了 guanfu_wang 提供的例子。http://mcudiy.taobao.com/
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-03-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
  安富莱STM32-V7开发板摄像头接口GPIO定义： 【DCIM设备： 摄像头  和串口6, AD7606 模块不能同时使用】
  PA6/DCMI_PIXCLK
  PA4/DCMI_HSYNC/DAC_OUT1
  PC6/DCMI_D0/AD7606_CONVST
  PC7/DCMI_D1/USART6_RX
  PG10/DCMI_D2/NRF24L01_CSN
  PG11/DCMI_D3/ETH_RMII_TX_EN
  PE4/DCMI_D4/NRF24L01_IRQ
  PD3/DCMI_D5
  PE5/DCMI_D6/AD7606_BUSY
  PE6/DCMI_D7/NRF905_CD
  PB7/DCMI_VSYNC
  
  --- I2C总线控制摄像头
  PB6/I2C2_SCL
  PB9/I2C2_SDA
*/

#define DCMI_DR_ADDRESS 0x50050028
#define OV_REG_NUM 116 //OV7670

/*　DMA通道定义,可选的有 DMA2_Stream1 +  DMA_Channel_1、 DMA2_Stream7 +  DMA_Channel_1  */

#define DMA_CLOCK RCC_AHB1Periph_DMA2
#define DMA_STREAM DMA2_Stream7
#define DMA_CHANNEL DMA_Channel_1
#define DMA_IRQ DMA2_Stream7_IRQn
#define DMA_IT_TCIF DMA_IT_TCIF7
#define DMA_IRQHandler DMA2_Stream7_IRQHandler

/* 调分辨率

12 17 18 19 1A 03。。。

*/

/*
  以下为OV7670 QVGA RGB565参数  (by guanfu_wang)  http://mcudiy.taobao.com

  由于RA8875图形模式下，扫描方向为从左到右，从上到下。
  和wang_guanfu提供的缺省值不同。因此做了适当的调整。
*/
static const unsigned char OV_reg[OV_REG_NUM][2] =
    {
        {0x3a, 0x0c},
        {0x67, 0xc0},
        {0x68, 0x80},

        {0x40, 0xd0}, //RGB565
        //{0x40, 0x10}, //RGB565
        {0x12, 0x14}, //Output format, QVGA,RGB

        {0x32, 0x80},
        {0x17, 0x16},
        {0x18, 0x04},
        {0x19, 0x02},
        {0x1a, 0x7a}, //0x7a,

        {0x03, 0x05}, //0x0a,
        {0x0c, 0x00},
        {0x3e, 0x00}, //

        {0x70, 0x00},
        {0x71, 0x01},
        {0x72, 0x11},
        {0x73, 0x00}, //
        {0xa2, 0x02},
        {0x11, 0x01},

        {0x7a, 0x2C},
        {0x7b, 0x11},
        {0x7c, 0x1a},
        {0x7d, 0x2a},
        {0x7e, 0x42},
        {0x7f, 0x4c},
        {0x80, 0x56},
        {0x81, 0x5f},
        {0x82, 0x67},
        {0x83, 0x70},
        {0x84, 0x78},
        {0x85, 0x87},
        {0x86, 0x95},
        {0x87, 0xaf},
        {0x88, 0xc8},
        {0x89, 0xdf},

        ////////////////
        {0x13, 0xe0},
        {0x00, 0x00}, //AGC

        {0x10, 0x00},
        {0x0d, 0x00},
        {0x14, 0x10}, //0x38, limit the max gain
        {0xa5, 0x05},
        {0xab, 0x07},

        {0x24, 0x75}, //40
        {0x25, 0x63},
        {0x26, 0xA5},
        {0x9f, 0x78},
        {0xa0, 0x68},

        {0xa1, 0x03}, //0x0b,
        {0xa6, 0xdf}, //0xd8,
        {0xa7, 0xdf}, //0xd8,
        {0xa8, 0xf0},
        {0xa9, 0x90},

        {0xaa, 0x94}, //50
        {0x13, 0xe5},
        {0x0e, 0x61},
        {0x0f, 0x4b},
        {0x16, 0x02},

#if 1
        {0x1e, 0x37}, //0x07, 0x17, 0x27, 0x37 选择1个，决定扫描方向. 需要和LCD的扫描方向匹配。
#else
        {0x1e, 0x27}, //0x07,
#endif

        {0x21, 0x02},
        {0x22, 0x91},
        {0x29, 0x07},
        {0x33, 0x0b},

        {0x35, 0x0b}, //60
        {0x37, 0x1d},
        {0x38, 0x71},
        {0x39, 0x2a},
        {0x3c, 0x78},

        {0x4d, 0x40},
        {0x4e, 0x20},
        {0x69, 0x5d},
        {0x6b, 0x0a}, //PLL
        {0x74, 0x19},
        {0x8d, 0x4f},

        {0x8e, 0x00}, //70
        {0x8f, 0x00},
        {0x90, 0x00},
        {0x91, 0x00},
        {0x92, 0x00}, //0x19,//0x66

        {0x96, 0x00},
        {0x9a, 0x80},
        {0xb0, 0x84},
        {0xb1, 0x0c},
        {0xb2, 0x0e},

        {0xb3, 0x82}, //80
        {0xb8, 0x0a},
        {0x43, 0x14},
        {0x44, 0xf0},
        {0x45, 0x34},

        {0x46, 0x58},
        {0x47, 0x28},
        {0x48, 0x3a},
        {0x59, 0x88},
        {0x5a, 0x88},

        {0x5b, 0x44}, //90
        {0x5c, 0x67},
        {0x5d, 0x49},
        {0x5e, 0x0e},
        {0x64, 0x04},
        {0x65, 0x20},

        {0x66, 0x05},
        {0x94, 0x04},
        {0x95, 0x08},
        {0x6c, 0x0a},
        {0x6d, 0x55},

        {0x4f, 0x80},
        {0x50, 0x80},
        {0x51, 0x00},
        {0x52, 0x22},
        {0x53, 0x5e},
        {0x54, 0x80},

        {0x76, 0xe1},

        {0x6e, 0x11}, //100
        {0x6f, 0x9f}, //0x9e for advance AWB
        {0x55, 0x00}, //亮度
        {0x56, 0x40}, //对比度
        {0x57, 0x80}, //0x40,
};

static void CAM_ConfigCPU(void);
static uint8_t OV_InitReg(void);
static void OV_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue);
static uint8_t OV_ReadReg(uint8_t _ucRegAddr);

CAM_T g_tCam;

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

/*
*********************************************************************************************************
*	函 数 名: bsp_InitCamera
*	功能说明: 配置摄像头GPIO和CAMERA设备.
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitCamera(void)
{
  CAM_ConfigCPU();

#if 1 /* 下面的代码，验证读写寄存器是否正确 */
  {
    uint8_t read;

    read = OV_ReadReg(0x3A);

    OV_WriteReg(0x3A, read + 1);

    read = OV_ReadReg(0x3A);

    OV_WriteReg(0x3A, read + 1);

    read = OV_ReadReg(0x3A);
  }
#endif

  OV_InitReg();
}

/*
*********************************************************************************************************
*	函 数 名: CAM_ConfigCPU
*	功能说明: 配置摄像头GPIO和CAMERA设备。0V7670的I2C接口配置在 bsp_gpio_i2c.c 文件实现。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CAM_ConfigCPU(void)
{
  /* 安富莱STM32-V7开发板采用有源晶振提供24M时钟，因此不用PA8产生时钟 */

  /*
    将DCMI相关的GPIO设置为复用模式  - STM32-V7
      PA4/DCMI_HSYNC/DAC_OUT1		
      PA6/DCMI_PIXCLK
      PB7/DCMI_VSYNC			
      PC6/DCMI_D0/AD7606_CONVST
      PC7/DCMI_D1/USART6_RX
      PD3/DCMI_D5
      PE4/DCMI_D4/NRF24L01_IRQ
      PE5/DCMI_D6/AD7606_BUSY
      PE6/DCMI_D7/NRF905_CD			
      PG10/DCMI_D2/NRF24L01_CSN
      PG11/DCMI_D3/ETH_RMII_TX_EN
  */
  {
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Peripheral clock enable */
    __HAL_RCC_DCMI_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;

    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  }

  /* 配置 DCMIC 参数 */
  {
    /* DCMI INIT */
    hdcmi.Instance = DCMI;
    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.CaptureRate = DCMI_CR_ALTERNATE_2_FRAME;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
    hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
    hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
    hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
    hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
    if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    /* DCMI interrupt Init */
    HAL_NVIC_SetPriority(DCMI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
  }
}

/*
*********************************************************************************************************
*	函 数 名: OV_InitReg
*	功能说明: 复位OV7670, 配置OV7670的寄存器，QVGA
*	形    参: 无
*	返 回 值: 0 表示正确，1表示失败
*********************************************************************************************************
*/
static uint8_t OV_InitReg(void)
{
  uint8_t i;

  CAM_ConfigCPU();
  //bsp_InitI2C();		/* 配置I2C总线, 在 bsp.c 文件执行了 */

  OV_WriteReg(0x12, 0x80); /* Reset SCCB */

  bsp_DelayMS(5);

  /* 设置 OV7670寄存器 */
  for (i = 0; i < OV_REG_NUM; i++)
  {
    OV_WriteReg(OV_reg[i][0], OV_reg[i][1]);
  }
  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: CAM_Start
*	功能说明: 启动DMA和DCMI，开始传送图像数据到LCD显存
*	形    参: _uiDispMemAddr 显存地址
*	返 回 值: 无
*********************************************************************************************************
*/
void CAM_Start(uint32_t _uiDispMemAddr)
{
  memset((char *)_uiDispMemAddr, 0, 2000);
  /* DCMI DMA Init */
  {
    hdma_dcmi.Instance = DMA1_Stream7;
    hdma_dcmi.Init.Request = DMA_REQUEST_DCMI;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dcmi.Init.Mode = DMA_CIRCULAR;
    hdma_dcmi.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma_dcmi);

    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Stream7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
  }

  //HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)buff, 128);
  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)_uiDispMemAddr, 265 * 320 * 2);

  g_tCam.CaptureOk = 0; /* 全局标志 */
}

/*
*********************************************************************************************************
*	函 数 名: CAM_Stop
*	功能说明: 停止DMA和DCMI
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void CAM_Stop(void)
{
  HAL_DCMI_Stop(&hdcmi);
}

void DMA1_Stream7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream7_IRQn 0 */

  /* USER CODE END DMA1_Stream7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_dcmi);
  /* USER CODE BEGIN DMA1_Stream7_IRQn 1 */

  /* USER CODE END DMA1_Stream7_IRQn 1 */
}

/*
*********************************************************************************************************
*	函 数 名: DMA2_Stream1_IRQHandler
*	功能说明: DMA传输完成中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&hdcmi);

  /* 关闭摄像 */
  CAM_Stop();
  g_tCam.CaptureOk = 1; /* 表示DMA传输结束 */
}

/*
*********************************************************************************************************
*	函 数 名: OV_WriteReg
*	功能说明: 写0V7670寄存器
*	形    参: _ucRegAddr  : 寄存器地址
*			  _ucRegValue : 寄存器值
*	返 回 值: 无
*********************************************************************************************************
*/
static void OV_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue)
{
  i2c_Start(); /* 总线开始信号 */

  i2c_SendByte(OV7670_SLAVE_ADDRESS); /* 发送设备地址+写信号 */
  i2c_WaitAck();

  i2c_SendByte(_ucRegAddr); /* 发送寄存器地址 */
  i2c_WaitAck();

  i2c_SendByte(_ucRegValue); /* 发送寄存器数值 */
  i2c_WaitAck();

  i2c_Stop(); /* 总线停止信号 */
}

/*
*********************************************************************************************************
*	函 数 名: OV_ReadReg
*	功能说明: 读0V7670寄存器
*	形    参: _ucRegAddr  : 寄存器地址
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
static uint8_t OV_ReadReg(uint8_t _ucRegAddr)
{
  uint16_t usRegValue;

  i2c_Start();                        /* 总线开始信号 */
  i2c_SendByte(OV7670_SLAVE_ADDRESS); /* 发送设备地址+写信号 */
  i2c_WaitAck();
  i2c_SendByte(_ucRegAddr); /* 发送地址 */
  i2c_WaitAck();

  i2c_Stop(); /* 0V7670 需要插入 stop, 否则读取寄存器失败 */

  i2c_Start();                            /* 总线开始信号 */
  i2c_SendByte(OV7670_SLAVE_ADDRESS + 1); /* 发送设备地址+读信号 */
  i2c_WaitAck();

  usRegValue = i2c_ReadByte(); /* 读出高字节数据 */
  i2c_NAck();
  i2c_Stop(); /* 总线停止信号 */

  return usRegValue;
}

/*
*********************************************************************************************************
*	函 数 名: OV_ReadID
*	功能说明: 读0V7670的芯片ID
*	形    参: 无
*	返 回 值: 芯片ID. 正常应该返回 0x7673
*********************************************************************************************************
*/
uint16_t OV_ReadID(void)
{
  uint8_t idh, idl;

  idh = OV_ReadReg(0x0A);
  idl = OV_ReadReg(0x0B);
  return (idh << 8) + idl;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
