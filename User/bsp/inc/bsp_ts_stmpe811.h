/*
*********************************************************************************************************
*
  模块名称 : 电阻式触摸芯片STMPE811驱动模块
  文件名称 : TOUCH_STMPE811,h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _TOUCH_STMPE811_H
#define _TOUCH_STMPE811_H

/* 定义显示器硬件识别码 

  0 = 4.3寸 (480*272)
  1 = 5.0寸 (480*272)
  2 = 5.0寸 (800*480)
  3 = 7.0寸 (800*480)
  4 = 7.0寸 (1024*600)
  5 = 3.5寸 (480*272)

*/
typedef enum
{
  LCD429_43_480X272 = 0,
  LCD429_50_480X272 = 1,
  LCD429_50_800X480 = 2,
  LCD429_70_800X480 = 3,
  LCD429_70_1024X600 = 4,
  LCD429_35_480X272 = 5,
} LCD_TYPE_E;

/* I2C总线地址  0x82 或者 0x88 */
#define STMPE811_I2C_ADDRESS 0x88

/* 寄存器地址定义 */
enum
{
  /*                         地址      bit 读写  复位值    功能说明  */
  REG811_CHIP_ID = 0x00,       /* 16 R   0x0811 Device identification    */
  REG811_ID_VER = 0x02,        /*  8 R   0x03 Revision number,
              0x01 for engineering sample, 0x03 for final silicon */
  REG811_SYS_CTRL1 = 0x03,     /*  8 R/W 0x00 Reset control    */
  REG811_SYS_CTRL2 = 0x04,     /*  8 R/W 0x0F Clock control    */
  REG811_SPI_CFG = 0x08,       /*  8 R/W 0x01 SPI in terface configuration   */
  REG811_INT_CTRL = 0x09,      /*  8 R/W 0x00 Interrupt control register   */
  REG811_INT_EN = 0x0A,        /*  8 R/W 0x00 Interrupt enable register    */
  REG811_INT_STA = 0x0B,       /*  8 R   0x10 interrupt status register    */
  REG811_GPIO_EN = 0x0C,       /*  8 R/W 0x00 GPIO interrupt enable register      */
  REG811_GPIO_INT_STA = 0x0D,  /*  8 R   0x00 GPIO interrupt status register      */
  REG811_ADC_INT_EN = 0x0E,    /*  8 R/W 0x00 ADC interrupt enable register       */
  REG811_ADC_INT_STA = 0x0F,   /*  8 R   0x00 ADC interrupt status register       */
  REG811_GPIO_SET_PIN = 0x10,  /*  8 R/W 0x00 GPIO set pin register               */
  REG811_GPIO_CLR_PIN = 0x11,  /*  8 R/W 0x00 GPIO clear pin register             */
  REG811_GPIO_MP_STA = 0x12,   /*  8 R/W 0x00 GPIO monitor pin state register     */
  REG811_GPIO_DIR = 0x13,      /*  8 R/W 0x00 GPIO direction register             */
  REG811_GPIO_ED = 0x14,       /*  8 R/W 0x00 GPIO edge detect register           */
  REG811_GPIO_RE = 0x15,       /*  8 R/W 0x00 GPIO rising edge register           */
  REG811_GPIO_FE = 0x16,       /*  8 R/W 0x00 GPIO falling edge register          */
  REG811_GPIO_AF = 0x17,       /*  8 R/W 0x00 Alternate function register         */
  REG811_ADC_CTRL1 = 0x20,     /*  8 R/W 0x9C ADC control                         */
  REG811_ADC_CTRL2 = 0x21,     /*  8 R/W 0x01 ADC control                         */
  REG811_ADC_CAPT = 0x22,      /*  8 R/W 0xFF To initiate ADC data acquisition    */
  REG811_ADC_DATA_CH0 = 0x30,  /* 16 R   0x0000 ADC channel 0                     */
  REG811_ADC_DATA_CH1 = 0x32,  /* 16 R   0x0000 ADC channel 1                     */
  REG811_ADC_DATA_CH2 = 0x34,  /* 16 R   0x0000 ADC channel 2                     */
  REG811_ADC_DATA_CH3 = 0x36,  /* 16 R   0x0000 ADC channel 3                     */
  REG811_ADC_DATA_CH4 = 0x38,  /* 16 R   0x0000 ADC channel 4                     */
  REG811_ADC_DATA_CH5 = 0x3A,  /* 16 R   0x0000 ADC channel 5                     */
  REG811_ADC_DATA_CH6 = 0x3C,  /* 16 R   0x0000 ADC channel 6                     */
  REG811_ADC_DATA_CH7 = 0x3E,  /* 16 R   0x0000 ADC channel 7                     */
  REG811_TSC_CTRL = 0x40,      /*  8 R/W 0x90 4-wire touch screen controller setup     */
  REG811_TSC_CFG = 0x41,       /*  8 R/W 0x00 Touch screen controller configuration    */
  REG811_WDW_TR_X = 0x42,      /* 16 R/W 0x0FFF Window setup for top right X           */
  REG811_WDW_TR_Y = 0x44,      /* 16 R/W 0x0FFF Window setup for top right Y           */
  REG811_WDW_BL_X = 0x46,      /* 16 R/W 0x0000 Window setup for bottom left X         */
  REG811_WDW_BL_Y = 0x48,      /* 16 R/W 0x0000 Window setup for bottom left Y         */
  REG811_FIFO_TH = 0x4A,       /*  8 R/W 0x00 FIFO level to generate interrupt         */
  REG811_FIFO_STA = 0x4B,      /*  8 R/W 0x20 Current status of FIFO                   */
  REG811_FIFO_SIZE = 0x4C,     /*  8 R   0x00 Current filled level of FIFO             */
  REG811_TSC_DATA_X = 0x4D,    /* 16 R   0x0000 Data port for touch screen controller data access   */
  REG811_TSC_DATA_Y = 0x4F,    /* 16 R   0x0000 Data port for touch screen controller data access   */
  REG811_TSC_DATA_Z = 0x51,    /*  8 R   0x0000 Data port for touch screen controller data access   */
  REG811_TSC_DATA_XYZ = 0x52,  /* 32 R   0x00000000 Data port for touch screen controller data access  */
  REG811_TSC_FRACT_XYZ = 0x56, /*  8     0x00 Touch screen controller FRACTION_XYZ   */

  /* 触摸数据访问端口 (地址自动递增) */
  REG811_TSC_DATA = 0x57, /*  8 R   0x00 Data port for touch screen controller data access   */

  /* 触摸数据访问端口 (地址不递增) */
  REG811_TSC_DATA1 = 0xD7, /*  8 R   0x00 Data port for touch screen controller data access   */

  REG811_TSC_I_DRIVE = 0x58, /*  8 R/W 0x00 Touch screen controller drive I   */
  REG811_TSC_SHIELD = 0x59,  /*  8 R/W 0x00 Touch screen controller shield    */
  REG811_TEMP_CTRL = 0x60,   /*  8 R/W 0x00 Temperature sensor setup           */
  REG811_TEMP_DATA = 0x61,   /*  8 R   0x00 Temperature data access port      */
  REG811_TEMP_TH = 0x62,     /*  8 R/W 0x00 Threshold for temperature controlled interrupt   */
};

/* 可供外部模块调用的函数 */

void STMPE811_InitHard(void);
void STMPE811_InitHardForGPIO(void);
uint16_t STMPE811_ReadID(void);

uint8_t STMPE811_ReadBytes(uint8_t *_pReadBuf, uint8_t _ucAddress, uint8_t _ucSize);
uint8_t STMPE811_WriteBytes(uint8_t *_pWriteBuf, uint8_t _ucAddress, uint8_t _ucSize);

void STMPE811_WriteReg1(uint8_t _ucRegAddr, uint8_t _ucValue);
void STMPE811_WriteReg2(uint8_t _ucRegAddr, uint16_t _usValue);

uint8_t STMPE811_ReadReg1(uint8_t _ucRegAddr);
uint16_t STMPE811_ReadReg2(uint8_t _ucRegAddr);

uint16_t STMPE811_ReadX(void);
uint16_t STMPE811_ReadY(void);

uint8_t STMPE811_ReadIO(void);

void STMPE811_ClearInt(void);
void STMPE811_WriteGPIO(uint8_t _pin, uint8_t _vlaue);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
