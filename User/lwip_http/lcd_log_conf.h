/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Raw/Inc/lcd_log_conf.h
  * @author  MCD Application Team
  * @brief   lcd_log configuration file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __LCD_LOG_CONF_H__
#define __LCD_LOG_CONF_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32h743i_eval_lcd.h"
#include <stdio.h>

/** @addtogroup LCD_LOG
  * @{
  */

/** @defgroup LCD_LOG
  * @brief This file is the 
  * @{
  */

/** @defgroup LCD_LOG_CONF_Exported_Defines
  * @{
  */

/* Comment the line below to disable the scroll back and forward features */
#define LCD_SCROLL_ENABLED 1

/* Define the Fonts  */
#define LCD_LOG_HEADER_FONT Font16
#define LCD_LOG_FOOTER_FONT Font12
#define LCD_LOG_TEXT_FONT Font12

/* Define the LCD LOG Color  */
#define LCD_LOG_BACKGROUND_COLOR LCD_COLOR_WHITE
#define LCD_LOG_TEXT_COLOR LCD_COLOR_DARKBLUE

#define LCD_LOG_SOLID_BACKGROUND_COLOR LCD_COLOR_BLUE
#define LCD_LOG_SOLID_TEXT_COLOR LCD_COLOR_WHITE

/* Define the cache depth */
#define CACHE_SIZE 100
#define YWINDOW_SIZE 14

#if (YWINDOW_SIZE > 14)
#error "Wrong YWINDOW SIZE"
#endif

/* Redirect the printf to the LCD */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define LCD_LOG_PUTCHAR int __io_putchar(int ch)
#else
#define LCD_LOG_PUTCHAR int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/** @defgroup LCD_LOG_CONF_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @defgroup LCD_LOG_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup LCD_LOG_CONF_Exported_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup LCD_LOG_CONF_Exported_FunctionsPrototype
  * @{
  */

/**
  * @}
  */

#endif //__LCD_LOG_CONF_H__

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
