/**
 * @file    SW_DP.c
 * @brief   SWD driver
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        20. May 2015
 * $Revision:    V1.10
 *
 * Project:      CMSIS-DAP Source
 * Title:        SW_DP.c CMSIS-DAP SW DP I/O
 *
 *---------------------------------------------------------------------------*/

#include "DAP_config.h"
#include "DAP.h"

#define SPI_MODE_ENABLE        1                            /* 1 表示SPI模式， 0表示GPIO模式 */

/* 
    SPI_BAUDRATEPRESCALER_4 = 50MHz SCK时钟，不能工作。
    SPI_BAUDRATEPRESCALER_8 = 25MHz SCK时钟，稳定读写。 写并读2048字节，耗时5ms，等效传输速度 800KB/s
    
    SPI_BAUDRATEPRESCALER_16 = 12.5MHz
    SPI_BAUDRATEPRESCALER_32 = 6.25MHz
    SPI_BAUDRATEPRESCALER_64 = 3.125MHz
    SPI_BAUDRATEPRESCALER_128 = 1.5625MHz
    
    实测ST-LINK下载640K程序，擦除需要9秒（这是CPU的FLASH擦除时间较慢导致），编程和校验需要13秒。
    按H7-TOOL的800KB/S的速度，SWD时序传输可以在1秒内完成，剩下的时间主要靠USB通信和CPU运算来提高效率。
    因此在SWD底层时序上优化已经没有多大的必要了，也许可以提高到1MB/S。但是对整体效率没有多大改善。
*/
#define SPI_MODE_BAUD        SPI_BAUDRATEPRESCALER_8        /* SPI_BAUDRATEPRESCALER_4 不稳定 */


#define PIN_SWCLK_SET PIN_SWCLK_TCK_SET
#define PIN_SWCLK_CLR PIN_SWCLK_TCK_CLR

#define SW_CLOCK_CYCLE()                \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define SW_WRITE_BIT(bit)               \
  PIN_SWDIO_OUT(bit);                   \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define SW_READ_BIT(bit)                \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  bit = PIN_SWDIO_IN();                 \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define PIN_DELAY() PIN_DELAY_SLOW(DAP_Data.clock_delay)




#if (DAP_SWD != 0)


// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
#define SWD_TransferFunction(speed)     /**/                                    \
uint8_t SWD_Transfer##speed (uint32_t request, uint32_t *data) {                \
  uint32_t ack;                                                                 \
  uint32_t bit;                                                                 \
  uint32_t val;                                                                 \
  uint32_t parity;                                                              \
                                                                                \
  uint32_t n;                                                                   \
                                                                                \
  /* Packet Request */                                                          \
  parity = 0U;                                                                  \
  SW_WRITE_BIT(1U);                     /* Start Bit */                         \
  bit = request >> 0;                                                           \
  SW_WRITE_BIT(bit);                    /* APnDP Bit */                         \
  parity += bit;                                                                \
  bit = request >> 1;                                                           \
  SW_WRITE_BIT(bit);                    /* RnW Bit */                           \
  parity += bit;                                                                \
  bit = request >> 2;                                                           \
  SW_WRITE_BIT(bit);                    /* A2 Bit */                            \
  parity += bit;                                                                \
  bit = request >> 3;                                                           \
  SW_WRITE_BIT(bit);                    /* A3 Bit */                            \
  parity += bit;                                                                \
  SW_WRITE_BIT(parity);                 /* Parity Bit */                        \
  SW_WRITE_BIT(0U);                     /* Stop Bit */                          \
  SW_WRITE_BIT(1U);                     /* Park Bit */                          \
                                                                                \
  /* Turnaround */                                                              \
  PIN_SWDIO_OUT_DISABLE();                                                      \
  for (n = DAP_Data.swd_conf.turnaround; n; n--) {                              \
    SW_CLOCK_CYCLE();                                                           \
  }                                                                             \
                                                                                \
  /* Acknowledge response */                                                    \
  SW_READ_BIT(bit);                                                             \
  ack  = bit << 0;                                                              \
  SW_READ_BIT(bit);                                                             \
  ack |= bit << 1;                                                              \
  SW_READ_BIT(bit);                                                             \
  ack |= bit << 2;                                                              \
                                                                                \
  if (ack == DAP_TRANSFER_OK) {         /* OK response */                       \
    /* Data transfer */                                                         \
    if (request & DAP_TRANSFER_RnW) {                                           \
      /* Read data */                                                           \
      val = 0U;                                                                 \
      parity = 0U;                                                              \
      for (n = 32U; n; n--) {                                                   \
        SW_READ_BIT(bit);               /* Read RDATA[0:31] */                  \
        parity += bit;                                                          \
        val >>= 1;                                                              \
        val  |= bit << 31;                                                      \
      }                                                                         \
      SW_READ_BIT(bit);                 /* Read Parity */                       \
      if ((parity ^ bit) & 1U) {                                                \
        ack = DAP_TRANSFER_ERROR;                                               \
      }                                                                         \
      if (data) { *data = val; }                                                \
      /* Turnaround */                                                          \
      for (n = DAP_Data.swd_conf.turnaround; n; n--) {                          \
        SW_CLOCK_CYCLE();                                                       \
      }                                                                         \
      PIN_SWDIO_OUT_ENABLE();                                                   \
    } else {                                                                    \
      /* Turnaround */                                                          \
      for (n = DAP_Data.swd_conf.turnaround; n; n--) {                          \
        SW_CLOCK_CYCLE();                                                       \
      }                                                                         \
      PIN_SWDIO_OUT_ENABLE();                                                   \
      /* Write data */                                                          \
      val = *data;                                                              \
      parity = 0U;                                                              \
      for (n = 32U; n; n--) {                                                   \
        SW_WRITE_BIT(val);              /* Write WDATA[0:31] */                 \
        parity += val;                                                          \
        val >>= 1;                                                              \
      }                                                                         \
      SW_WRITE_BIT(parity);             /* Write Parity Bit */                  \
    }                                                                           \
    /* Idle cycles */                                                           \
    n = DAP_Data.transfer.idle_cycles;                                          \
    if (n) {                                                                    \
      PIN_SWDIO_OUT(0U);                                                        \
      for (; n; n--) {                                                          \
        SW_CLOCK_CYCLE();                                                       \
      }                                                                         \
    }                                                                           \
    PIN_SWDIO_OUT(1U);                                                          \
    return ((uint8_t)ack);                                                      \
  }                                                                             \
                                                                                \
  if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) {              \
    /* WAIT or FAULT response */                                                \
    if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0U)) { \
      for (n = 32U+1U; n; n--) {                                                \
        SW_CLOCK_CYCLE();               /* Dummy Read RDATA[0:31] + Parity */   \
      }                                                                         \
    }                                                                           \
    /* Turnaround */                                                            \
    for (n = DAP_Data.swd_conf.turnaround; n; n--) {                            \
      SW_CLOCK_CYCLE();                                                         \
    }                                                                           \
    PIN_SWDIO_OUT_ENABLE();                                                     \
    if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U)) { \
      PIN_SWDIO_OUT(0U);                                                        \
      for (n = 32U+1U; n; n--) {                                                \
        SW_CLOCK_CYCLE();               /* Dummy Write WDATA[0:31] + Parity */  \
      }                                                                         \
    }                                                                           \
    PIN_SWDIO_OUT(1U);                                                          \
    return ((uint8_t)ack);                                                      \
  }                                                                             \
                                                                                \
  /* Protocol error */                                                          \
  for (n = DAP_Data.swd_conf.turnaround + 32U + 1U; n; n--) {                   \
    SW_CLOCK_CYCLE();                   /* Back off data phase */               \
  }                                                                             \
  PIN_SWDIO_OUT_ENABLE();                                                       \
  PIN_SWDIO_OUT(1U);                                                            \
  return ((uint8_t)ack);                                                        \
}


#undef  PIN_DELAY
#define PIN_DELAY() PIN_DELAY_FAST()
SWD_TransferFunction(Fast);

#undef  PIN_DELAY
#define PIN_DELAY() PIN_DELAY_SLOW(DAP_Data.clock_delay)
SWD_TransferFunction(Slow);


uint8_t SWD_TransferFastH7(uint32_t request, uint32_t *data);
uint8_t SWD_TransferFastH7_ok(uint32_t request, uint32_t *data);
// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
uint8_t  SWD_Transfer(uint32_t request, uint32_t *data) 
{
    if (DAP_Data.fast_clock) 
    {
        //return SWD_TransferFast(request, data);
        return SWD_TransferFastH7(request, data);
    } 
    else 
    {
        return SWD_TransferSlow(request, data);
    }
}

/********************************* armfly 优化时序速度 ***************************/
// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]                              
#undef  PIN_DELAY
#define PIN_DELAY() PIN_DELAY_FAST()

uint8_t GetParity(uint32_t data)
{
    uint8_t parity = 0;
        
    parity =  (data >> 31) + (data >> 30) + (data >> 29) + (data >> 28)
            + (data >> 27) + (data >> 26) + (data >> 25) + (data >> 24)
            + (data >> 23) + (data >> 22) + (data >> 21) + (data >> 20)
            + (data >> 19) + (data >> 18) + (data >> 17) + (data >> 16)
            + (data >> 15) + (data >> 14) + (data >> 13) + (data >> 12)
            + (data >> 11) + (data >> 10) + (data >> 9) + (data >> 8)
            + (data >> 7) + (data >> 6) + (data >> 5) + (data >> 4)    
            + (data >> 3) + (data >> 2) + (data >> 1) + (data >> 0);
        
    if (parity & 1)
    {
        parity = 1;
    }
    else
    {
        parity = 0;
    }
    
    return parity;
}

void SWD_SendBits(uint8_t _bits, uint32_t _data)
{
#if SPI_MODE_ENABLE == 1
    
//    EIO_SetOutLevel(2, 1);
    
//    SPI2->CFG1 = SPI_MODE_BAUD | _bits;
//    SPI2->CR1 = SPI_CR1_SSI | SPI_CR1_HDDIR;
//    SPI2->CR1 = SPI_CR1_SPE | SPI_CR1_HDDIR | SPI_CR1_SSI;    
//    SPI2->CR1 = SPI_CR1_SPE | SPI_CR1_HDDIR | SPI_CR1_SSI | SPI_CR1_CSTART;        
    
    SPI2->CFG1 = SPI_MODE_BAUD | (_bits - 1);
    SPI2->CR1 = SPI_CR1_HDDIR;
    SPI2->CR1 = SPI_CR1_SPE | SPI_CR1_HDDIR;    
    SPI2->CR1 = SPI_CR1_SPE | SPI_CR1_HDDIR | SPI_CR1_CSTART;    
    
    //while ((SPI2->SR & SPI_FLAG_TXE) == 0);
//    EIO_SetOutLevel(2, 0);
    
    if (_bits > 16)
    {
        *((__IO uint32_t *)&SPI2->TXDR) = _data;
    }
    else if (_bits > 8)
    {
        *((__IO uint16_t *)&SPI2->TXDR) = _data;
    }
    else
    {
        *((__IO uint8_t *)&SPI2->TXDR) = _data;
    }
    
    
//    EIO_SetOutLevel(2, 1);

    while ((SPI2->SR & SPI_SR_TXC) == 0);    

    SPI2->IFCR = SPI_IFCR_EOTC | SPI_IFCR_TXTFC;
    
    SPI2->CR1 &= ~(SPI_CR1_SPE);
    
//    EIO_SetOutLevel(2, 0);
#else    
    uint8_t i;
    
    for (i = 0; i < _bits; i++)
    {
        SW_WRITE_BIT(_data >> i);
    }
#endif
}

uint32_t SWD_ReadBits(uint8_t _bits)
{
#if SPI_MODE_ENABLE == 1
//if (hspi->Instance->SR & (SPI_FLAG_RXWNE|SPI_FLAG_EOT))

//    {
//      /* Check the RXWNE/FRLVL flag */
//      if (hspi->Instance->SR & (SPI_FLAG_RXWNE|SPI_FLAG_FRLVL))
//      {
//        if (hspi->Instance->SR & SPI_FLAG_RXWNE)
//        {
//          *((uint32_t *)hspi->pRxBuffPtr) = *((__IO uint32_t *)&hspi->Instance->RXDR);
//          hspi->pRxBuffPtr += sizeof(uint32_t);
//          hspi->RxXferCount-=2;
//        }
//        else
//        {
//          *((uint16_t *)hspi->pRxBuffPtr) = *((__IO uint16_t *)&hspi->Instance->RXDR);
//          hspi->pRxBuffPtr += sizeof(uint16_t);
//          hspi->RxXferCount--;
//        }
//      }
//  }

//    {
//      /* Check the RXWNE/FRLVL flag */
//      if (hspi->Instance->SR & (SPI_FLAG_RXWNE|SPI_FLAG_FRLVL))
//      {
//        if (hspi->Instance->SR & SPI_FLAG_RXWNE)
//        {
//          *((uint32_t *)hspi->pRxBuffPtr) = *((__IO uint32_t *)&hspi->Instance->RXDR);
//          hspi->pRxBuffPtr += sizeof(uint32_t);
//          hspi->RxXferCount-=4;
//        }
//        else if ((hspi->Instance->SR & SPI_FLAG_FRLVL) > SPI_FRLVL_QUARTER_FULL)
//        {
//          *((uint16_t *)hspi->pRxBuffPtr) = *((__IO uint16_t *)&hspi->Instance->RXDR);
//          hspi->pRxBuffPtr += sizeof(uint16_t);
//          hspi->RxXferCount-=2;
//        }
//        else
//        {
//          *((uint8_t *)hspi->pRxBuffPtr) = *((__IO uint8_t *)&hspi->Instance->RXDR);
//          hspi->pRxBuffPtr += sizeof(uint8_t);
//          hspi->RxXferCount--;
//        }
//      }
//  }
    uint32_t ret;
    
    _bits--;
    
    EIO_SetOutLevel(2, 1);
    
    SPI2->CFG1 = SPI_MODE_BAUD | _bits;
//    SPI2->CR1 = SPI_CR1_SSI ;
//    SPI2->CR2 = 1;    
    SPI2->CR1 = SPI_CR1_SPE;// | SPI_CR1_SSI;        

    SPI2->CR1 = SPI_CR1_SPE | SPI_CR1_CSTART ; //| SPI_CR1_SSI ;
    
//    while ((SPI2->SR & SPI_FLAG_TXE) == 0);
    
    if (_bits > 15)
    {
        *((__IO uint32_t *)&SPI2->TXDR) = 0;
    }
    else if (_bits > 7)
    {
        *((__IO uint16_t *)&SPI2->TXDR) = 0;
    }
    else
    {
        *((__IO uint8_t *)&SPI2->TXDR) = 0;
    }
    
    while ((SPI2->SR & SPI_SR_TXC) == 0);    
    SPI2->IFCR = SPI_IFCR_EOTC | SPI_IFCR_TXTFC;
    
    ret = SPI2->RXDR;    
    SPI2->CR1 &= ~(SPI_CR1_SPE);    
    
    EIO_SetOutLevel(2, 0);
    return ret;

#else    
    uint8_t bit;
    uint8_t i;
    uint32_t val = 0;
    
    for (i = 0; i < _bits; i++)
    {
        SW_READ_BIT(bit);               /* Read RDATA[0:31] */       
        val >>= 1;                                                             
        val  |= bit << (_bits - 1);  
    }
    return val;
#endif
}

void SWD_DIO_OutDisable(void)
{
#if SPI_MODE_ENABLE == 1    
    SPI2->CR1 = SPI_CR1_SSI;    
    BSP_SET_GPIO_0(GPIOG, GPIO_PIN_9);    /* PG9 = 0 是输入        */        
#else
    PIN_SWDIO_OUT_DISABLE(); 
#endif
}

void SWD_DIO_OutEnable(void)
{
#if SPI_MODE_ENABLE == 1
    BSP_SET_GPIO_1(GPIOG, GPIO_PIN_9);    /* PG9 = 1 是输出       */    
    SPI2->CR1 = SPI_CR1_SSI | SPI_CR1_HDDIR;
#else
    PIN_SWDIO_OUT_ENABLE(); 
#endif
}


#undef  PIN_DELAY
#define PIN_DELAY() PIN_DELAY_SLOW(DAP_Data.clock_delay)
// Generate SWJ Sequence
//   count:  sequence bit count
//   data:   pointer to sequence bit data
//   return: none
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
void SWJ_Sequence (uint32_t count, const uint8_t *data) {
#if SPI_MODE_ENABLE == 1
    uint8_t i;
    uint32_t val;
    uint8_t rem;
    
    rem = count % 32;
    
    if (rem >= 1 && rem < 4)    /* 1,2,3 */
    {
        ;
    }
    else
    {    
        for (i = 0; i < count / 32; i++)
        {
            val = *(uint32_t *)data;
            data += 4;
            SWD_SendBits(32, val);
        }
        
        if (rem)
        {
            val = *(uint32_t *)data;        /* 可能多访问内存 */
            SWD_SendBits(rem, val);
        }
    }
#else  
    
  uint32_t val;
  uint32_t n;

  val = 0U;
  n = 0U;
  while (count--) {
    if (n == 0U) {
      val = *data++;
      n = 8U;
    }
    if (val & 1U) {
      PIN_SWDIO_TMS_SET();
    } else {
      PIN_SWDIO_TMS_CLR();
    }
    SW_CLOCK_CYCLE();
    val >>= 1;
    n--;
  }
#endif
}
#endif
    
uint8_t SWD_TransferFastH7(uint32_t request, uint32_t *data) 
{  
    uint32_t ack;                                                                
//    uint32_t bit;                                                                
    uint32_t val;         
    uint32_t temp1,temp2; 
    uint32_t parity;                                                             

    uint32_t n;                                                                  

    /* Packet Request */                                                         
//    parity = 0U;                                                                 
//    SW_WRITE_BIT(1U);                     /* Start Bit */                        
//    bit = request >> 0;                                                          
//    SW_WRITE_BIT(bit);                    /* APnDP Bit */                        
//    parity += bit;                                                               
//    bit = request >> 1;                                                          
//    SW_WRITE_BIT(bit);                    /* RnW Bit */                          
//    parity += bit;                                                               
//    bit = request >> 2;                                                          
//    SW_WRITE_BIT(bit);                    /* A2 Bit */                           
//    parity += bit;                                                               
//    bit = request >> 3;                                                          
//    SW_WRITE_BIT(bit);                    /* A3 Bit */                           
//    parity += bit;                                                               
//    SW_WRITE_BIT(parity);                 /* Parity Bit */                       
//    SW_WRITE_BIT(0U);                     /* Stop Bit */                         
//    SW_WRITE_BIT(1U);                     /* Park Bit */       

    parity = GetParity(request);
    val = (1u << 7) | (0 << 6) | (parity << 5) | (request << 1) | (1 << 0);
    SWD_SendBits(8, val);

    
    /* Turnaround */                                                             
    SWD_DIO_OutDisable();   
    
//    for (n = DAP_Data.swd_conf.turnaround; n; n--) 
//    {                             
//        SW_CLOCK_CYCLE();                                                          
//    }                                                                            

//    /* Acknowledge response */                                                   
//    SW_READ_BIT(bit);                                                            
//    ack  = bit << 0;                                                             
//    SW_READ_BIT(bit);                                                            
//    ack |= bit << 1;                                                             
//    SW_READ_BIT(bit);                                                            
//    ack |= bit << 2;         

//    ack = SWD_ReadBits(3 + DAP_Data.swd_conf.turnaround);
//    ack >>= DAP_Data.swd_conf.turnaround;
    

//    if (ack == DAP_TRANSFER_OK) 
    {         /* OK response */                      
        /* Data transfer */                                                        
        if (request & DAP_TRANSFER_RnW)     /* 读指令 - 32 + 1 bit */
        {                       
            ack = SWD_ReadBits(3 + DAP_Data.swd_conf.turnaround);
            ack >>= DAP_Data.swd_conf.turnaround;            
            ack &= 0x07;
            
            /* Read data */                                                          
//            val = 0U;                                                                
//            parity = 0U;                                                             
//            for (n = 32U; n; n--) 
//            {                                                  
//                SW_READ_BIT(bit);               /* Read RDATA[0:31] */                 
//                parity += bit;                                                         
//                val >>= 1;                                                             
//                val  |= bit << 31;                                                     
//            }                                                                        
//            SW_READ_BIT(bit);                 /* Read Parity */                      
//            if ((parity ^ bit) & 1U) 
//            {                                               
//                ack = DAP_TRANSFER_ERROR;                                              
//            }                                                                        
//            if (data) { *data = val; }  
//            
//            /* Turnaround */                                                         
//            for (n = DAP_Data.swd_conf.turnaround; n; n--) 
//            {                         
//                SW_CLOCK_CYCLE();                                                      
//            }                 

            if (ack == DAP_TRANSFER_OK)
            {
                temp1 = SWD_ReadBits(16);
                temp2 = SWD_ReadBits(16 + 1 + DAP_Data.swd_conf.turnaround);
                val = temp1 + ((temp2 & 0xFFFF) << 16);
                if (temp2 & (1 << 16))
                {
                    parity = 1;
                }
                else
                {
                    parity = 0;
                }
                if (parity != GetParity(val))
                {
                    ack = DAP_TRANSFER_ERROR;
                }
                else
                {
                    *data = val;
                }

                SWD_DIO_OutEnable();   

                /* Idle cycles */                                                          
                n = DAP_Data.transfer.idle_cycles;                                         
                if (n) 
                {                                                                   
                    PIN_SWDIO_OUT(0U);                                                       
                    for (; n; n--) 
                    {                                                         
                        SW_CLOCK_CYCLE();                                                      
                    }                                                                        
                }                                                                          
                PIN_SWDIO_OUT(1U);                                                         
                return ((uint8_t)ack);                              
            }
        } 
        else  // 写指令
        {                 
            ack = SWD_ReadBits(3 + DAP_Data.swd_conf.turnaround + 1);
            ack >>= DAP_Data.swd_conf.turnaround;    
            ack &= 0x07;
            
            /* Turnaround */                                                         
//            for (n = DAP_Data.swd_conf.turnaround; n; n--) 
//            {                         
//                SW_CLOCK_CYCLE();                                                      
//            }                                                                        
              
            if (ack == DAP_TRANSFER_OK)
            {
                SWD_DIO_OutEnable(); 
                
                /* Write data */                                                         
//                val = *data;                                                             
//                parity = 0U;                                                             
//                for (n = 32U; n; n--) 
//                {                                                  
//                    SW_WRITE_BIT(val);              /* Write WDATA[0:31] */                
//                    parity += val;                                                         
//                    val >>= 1;                                                             
//                }                                                                        
//                SW_WRITE_BIT(parity);             /* Write Parity Bit */    

//                /* Idle cycles */                                                          
//                n = DAP_Data.transfer.idle_cycles;                                         
//                if (n) 
//                {                                                                   
//                    PIN_SWDIO_OUT(0U);                                                       
//                    for (; n; n--) 
//                    {                                                         
//                        SW_CLOCK_CYCLE();                                                      
//                    }                                                                        
//                }                                                                          
//                PIN_SWDIO_OUT(1U);           
                
                val = *data; 
                parity = GetParity(val);
                SWD_SendBits(16, val);
                SWD_SendBits(16 + 1 + DAP_Data.transfer.idle_cycles, (val >> 16) + (parity << 16));
                
                //PIN_SWDIO_OUT(1U);     好像不是必须的。

                return ((uint8_t)ack);                  
            }            
        }                                                                          
                                                   
    }                                                                            

    if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) 
    {             
        if (request & DAP_TRANSFER_RnW)     /* 读异常 */
        {
            /* WAIT or FAULT response */                                               
//            if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0U)) 
//            {
//                for (n = 32U+1U; n; n--) 
//                {                                               
//                    SW_CLOCK_CYCLE();               /* Dummy Read RDATA[0:31] + Parity */  
//                }                                                                        
//            }                                                                          
//            /* Turnaround */                                                           
//            for (n = DAP_Data.swd_conf.turnaround; n; n--) 
//            {                           
//                SW_CLOCK_CYCLE();                                                        
//            }                   

            SWD_ReadBits(16);
            SWD_ReadBits(16 + 1 + DAP_Data.swd_conf.turnaround);
            SWD_DIO_OutEnable();         
            
//            if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U)) 
//            {
//                PIN_SWDIO_OUT(0U);                                                       
//                for (n = 32U+1U; n; n--) 
//                {                                               
//                    SW_CLOCK_CYCLE();               /* Dummy Write WDATA[0:31] + Parity */ 
//                }                                                                        
//            }                                                                          
            PIN_SWDIO_OUT(1U);                                                         
            return ((uint8_t)ack);   
        }            
        else    /* 写异常 - 前面多发了1个clk */
        {
            /* WAIT or FAULT response */                                               
//            if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0U)) 
//            {
//                for (n = 32U+1U; n; n--) 
//                {                                               
//                    SW_CLOCK_CYCLE();               /* Dummy Read RDATA[0:31] + Parity */  
//                }                                                                        
//            }                                                                          
            /* Turnaround */                                                           
//            for (n = DAP_Data.swd_conf.turnaround; n; n--) 
//            {                           
//                SW_CLOCK_CYCLE();                                                        
//            }                                                                          
            SWD_DIO_OutEnable();   
            
//            if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U)) 
//            {
//                PIN_SWDIO_OUT(0U);                                                       
//                for (n = 32U+1U; n; n--) 
//                {                                               
//                    SW_CLOCK_CYCLE();               /* Dummy Write WDATA[0:31] + Parity */ 
//                }                                                                        
//            }      
            if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U))
            {
                SWD_SendBits(16, 0);
                SWD_SendBits(16 + 1, 0);
            }
            PIN_SWDIO_OUT(1U);                    
            return ((uint8_t)ack);  
        }
    }                                                                            

    /* Protocol error */                                                         
//    for (n = DAP_Data.swd_conf.turnaround + 32U + 1U; n; n--) 
//    {                  
//        SW_CLOCK_CYCLE();                   /* Back off data phase */              
//    }             
    SWD_ReadBits(16);
    SWD_ReadBits(16 + 1 + DAP_Data.swd_conf.turnaround);
    SWD_DIO_OutEnable();    
    
    PIN_SWDIO_OUT(1U);                                                           
    return ((uint8_t)ack);                                                         
}

uint8_t SWD_TransferFastH7_ok(uint32_t request, uint32_t *data) 
{  
    uint32_t ack;                                                                
    uint32_t bit;                                                                
    uint32_t val;                                                                
    uint32_t parity;                                                             

    uint32_t n;                                                                  

    /* Packet Request */                                                         
    parity = 0U;                                                                 
    SW_WRITE_BIT(1U);                     /* Start Bit */                        
    bit = request >> 0;                                                          
    SW_WRITE_BIT(bit);                    /* APnDP Bit */                        
    parity += bit;                                                               
    bit = request >> 1;                                                          
    SW_WRITE_BIT(bit);                    /* RnW Bit */                          
    parity += bit;                                                               
    bit = request >> 2;                                                          
    SW_WRITE_BIT(bit);                    /* A2 Bit */                           
    parity += bit;                                                               
    bit = request >> 3;                                                          
    SW_WRITE_BIT(bit);                    /* A3 Bit */                           
    parity += bit;                                                               
    SW_WRITE_BIT(parity);                 /* Parity Bit */                       
    SW_WRITE_BIT(0U);                     /* Stop Bit */                         
    SW_WRITE_BIT(1U);                     /* Park Bit */                         

    /* Turnaround */                                                             
    PIN_SWDIO_OUT_DISABLE();                                                     
    for (n = DAP_Data.swd_conf.turnaround; n; n--) 
    {                             
        SW_CLOCK_CYCLE();                                                          
    }                                                                            

    /* Acknowledge response */                                                   
    SW_READ_BIT(bit);                                                            
    ack  = bit << 0;                                                             
    SW_READ_BIT(bit);                                                            
    ack |= bit << 1;                                                             
    SW_READ_BIT(bit);                                                            
    ack |= bit << 2;                                                             

    if (ack == DAP_TRANSFER_OK) 
    {         /* OK response */                      
        /* Data transfer */                                                        
        if (request & DAP_TRANSFER_RnW)     /* 读指令 - 32 + 1 bit */
        {                                          
            /* Read data */                                                          
            val = 0U;                                                                
            parity = 0U;                                                             
            for (n = 32U; n; n--) 
            {                                                  
                SW_READ_BIT(bit);               /* Read RDATA[0:31] */                 
                parity += bit;                                                         
                val >>= 1;                                                             
                val  |= bit << 31;                                                     
            }                                                                        
            SW_READ_BIT(bit);                 /* Read Parity */                      
            if ((parity ^ bit) & 1U) 
            {                                               
                ack = DAP_TRANSFER_ERROR;                                              
            }                                                                        
            if (data) { *data = val; }  
            
            /* Turnaround */                                                         
            for (n = DAP_Data.swd_conf.turnaround; n; n--) 
            {                         
                SW_CLOCK_CYCLE();                                                      
            }                                                                        
            PIN_SWDIO_OUT_ENABLE();                                                  
        } 
        else 
        {                                                                   
            /* Turnaround */                                                         
            for (n = DAP_Data.swd_conf.turnaround; n; n--) 
            {                         
                SW_CLOCK_CYCLE();                                                      
            }                                                                        
            PIN_SWDIO_OUT_ENABLE();                                                  
            /* Write data */                                                         
            val = *data;                                                             
            parity = 0U;                                                             
            for (n = 32U; n; n--) 
            {                                                  
                SW_WRITE_BIT(val);              /* Write WDATA[0:31] */                
                parity += val;                                                         
                val >>= 1;                                                             
            }                                                                        
            SW_WRITE_BIT(parity);             /* Write Parity Bit */                 
        }                                                                          
        /* Idle cycles */                                                          
        n = DAP_Data.transfer.idle_cycles;                                         
        if (n) 
        {                                                                   
            PIN_SWDIO_OUT(0U);                                                       
            for (; n; n--) 
            {                                                         
                SW_CLOCK_CYCLE();                                                      
            }                                                                        
        }                                                                          
        PIN_SWDIO_OUT(1U);                                                         
        return ((uint8_t)ack);                                                     
    }                                                                            

    if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) 
    {             
        /* WAIT or FAULT response */                                               
        if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0U)) 
        {
            for (n = 32U+1U; n; n--) 
            {                                               
                SW_CLOCK_CYCLE();               /* Dummy Read RDATA[0:31] + Parity */  
            }                                                                        
        }                                                                          
        /* Turnaround */                                                           
        for (n = DAP_Data.swd_conf.turnaround; n; n--) 
        {                           
            SW_CLOCK_CYCLE();                                                        
        }                                                                          
        PIN_SWDIO_OUT_ENABLE();                                                    
        if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U)) 
        {
            PIN_SWDIO_OUT(0U);                                                       
            for (n = 32U+1U; n; n--) 
            {                                               
                SW_CLOCK_CYCLE();               /* Dummy Write WDATA[0:31] + Parity */ 
            }                                                                        
        }                                                                          
        PIN_SWDIO_OUT(1U);                                                         
        return ((uint8_t)ack);                                                     
    }                                                                            

    /* Protocol error */                                                         
    for (n = DAP_Data.swd_conf.turnaround + 32U + 1U; n; n--) 
    {                  
        SW_CLOCK_CYCLE();                   /* Back off data phase */              
    }                                                                            
    PIN_SWDIO_OUT_ENABLE();                                                      
    PIN_SWDIO_OUT(1U);                                                           
    return ((uint8_t)ack);                                                       
}

#endif  /* (DAP_SWD != 0) */
