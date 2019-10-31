/*
*********************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.c
*	版    本 : V1.0
*	说    明 : 驱动LED指示灯
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-09-05 armfly  正式发布
*
*	Copyright (C), 2015-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

#define ALL_LED_GPIO_CLK_ENABLE() __HAL_RCC_GPIOI_CLK_ENABLE()

#define GPIO_LED1 GPIOI
#define PIN_LED1 GPIO_PIN_8

#define LED1_ON() GPIO_LED1->BSRRH = PIN_LED1  /* LED1 = 0 */
#define LED1_OFF() GPIO_LED1->BSRRL = PIN_LED1 /* LED1 = 1 */

#define LED1_IS_ON() ((GPIO_LED1->IDR & PIN_LED1) == 0) /* 如果已点亮，返回1 */

/*
*********************************************************************************************************
*	函 数 名: bsp_InitLed
*	功能说明: 配置LED指示灯相关的GPIO,  该函数被 bsp_Init() 调用。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
  GPIO_InitTypeDef gpio_init;

  /* 第1步：打开GPIO时钟 */
  ALL_LED_GPIO_CLK_ENABLE();

  bsp_LedOff(1);

  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   /* 设置开漏输出 */
  gpio_init.Pull = GPIO_NOPULL;           /* 上下拉电阻不使能 */
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; /* GPIO速度等级 */
  gpio_init.Pin = PIN_LED1;
  HAL_GPIO_Init(GPIO_LED1, &gpio_init);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 点亮指定的LED指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_LedOn(uint8_t _no)
{
  if (_no == 1)
  {
    LED1_ON();
  }
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOff
*	功能说明: 熄灭指定的LED指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_LedOff(uint8_t _no)
{
  if (_no == 1)
  {
    LED1_OFF();
  }
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedToggle
*	功能说明: 翻转指定的LED指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 按键代码
*********************************************************************************************************
*/
void bsp_LedToggle(uint8_t _no)
{
  if (bsp_IsLedOn(_no))
  {
    bsp_LedOff(_no);
  }
  else
  {
    bsp_LedOn(_no);
  }
}

/*
*********************************************************************************************************
*	函 数 名: bsp_IsLedOn
*	功能说明: 判断LED指示灯是否已经点亮。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 1表示已经点亮，0表示未点亮
*********************************************************************************************************
*/
uint8_t bsp_IsLedOn(uint8_t _no)
{
  if (_no == 1)
  {
    return LED1_IS_ON();
  }

  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_TestSDIO
*	功能说明: 翻转SDIO的GPIO. debug 用
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_TestSDIO(void)
{
  GPIO_InitTypeDef gpio_init;

  __HAL_RCC_GPIOA_CLK_ENABLE();

  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   /* 设置开漏输出 */
  gpio_init.Pull = GPIO_NOPULL;           /* 上下拉电阻不使能 */
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; /* GPIO速度等级 */
  gpio_init.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  ;
  HAL_GPIO_Init(GPIOC, &gpio_init);

  gpio_init.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &gpio_init);

  while (1)
  {
    GPIOC->BSRRH = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    GPIOD->BSRRH = GPIO_PIN_2;
    bsp_DelayMS(100);
    GPIOC->BSRRL = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    GPIOD->BSRRL = GPIO_PIN_2;
    bsp_DelayMS(100);
  }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
