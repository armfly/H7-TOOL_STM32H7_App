/*
*********************************************************************************************************
*
*    妯″潡鍚岖О : USB device 涓柇链嶅姟绋嫔簭
*    鏂囦欢鍚岖О : usb_it.c
*    鐗?   链?: V1.0
*    璇?   鏄?: 链枃浠跺瓨鏀绾SB device涓柇链嶅姟绋嫔簭銆傚彧闇€灏呜鏂囦欢锷犲叆宸ョ▼鍗冲彲锛屾棤闇€鍐嶅埌 stm32f4xx_it.c 涓坊锷犺繖浜汭SR绋嫔簭
*
*    淇敼璁板綍 :
*        鐗堟湰鍙? 镞ユ湡        浣滆€?    璇存槑
*        V1.0    2018-09-08 armfly  姝ｅ纺鍙戝竷
*
*    Copyright (C), 2018-2030, 瀹夊瘜銮辩数瀛?www.armfly.com
*
*********************************************************************************************************
*/
/* Includes ------------------------------------------------------------------ */
#include "usbd_user.h"

/** @addtogroup STM32H7xx_HAL_Applications
  * @{
  */

/* Private typedef ----------------------------------------------------------- */
/* Private define ------------------------------------------------------------ */
/* Private macro ------------------------------------------------------------- */
/* Private variables --------------------------------------------------------- */
extern PCD_HandleTypeDef hpcd;

/* UART handler declared in "usbd_cdc_interface.c" file */
extern UART_HandleTypeDef *NowUartHandle;

/* TIM handler declared in "usbd_cdc_interface.c" file */
extern TIM_HandleTypeDef TimHandle;
/* Private function prototypes ----------------------------------------------- */
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void);
#else
void OTG_HS_IRQHandler(void);
#endif
void USARTx_DMA_TX_IRQHandler(void);
void USARTx_IRQHandler(void);
void TIMx_IRQHandler(void);

/**
  * @brief  This function handles USB-On-The-Go FS/HS global interrupt request.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  HAL_PCD_IRQHandler(&hpcd);
}

/******************************************************************************/
/* STM32H7xx Peripherals Interrupt Handlers */
/* Add here the Interrupt Handler for the used peripheral(s) (PPP), for the */
/* available peripheral interrupt handler's name please refer to the startup */
/* file (startup_stm32h7xx.s).  */
/******************************************************************************/
/**
  * @brief  This function handles DMA interrupt request.
  * @param  None
  * @retval None
  */
void A_USARTx_DMA_TX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(NowUartHandle->hdmatx);
}

//void B_USARTx_DMA_TX_IRQHandler(void)
//{
//  HAL_DMA_IRQHandler(NowUartHandle->hdmatx);
//}


/**
  * @brief  This function handles UART interrupt request.  
  * @param  None
  * @retval None
  */
void A_USARTx_IRQHandler(void)
{
  HAL_UART_IRQHandler(NowUartHandle);
}

void B_USARTx_IRQHandler(void)
{
  HAL_UART_IRQHandler(NowUartHandle);
}

/**
  * @brief  This function handles TIM interrupt request.
  * @param  None
  * @retval None
  */
void TIMx_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle);
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
