/*
*********************************************************************************************************
*
*    妯″潡鍚岖О : CDC铏氭嫙涓插彛纭欢閰岖疆
*    鏂囦欢鍚岖О : usbd_cdc_interface.c
*    鐗?   链?: V1.0
*    璇?   鏄?: 
*
*    淇敼璁板綍 :
*        鐗堟湰鍙? 镞ユ湡        浣滆€?    璇存槑
*        V1.0    2018-12-11  armfly  姝ｅ纺鍙戝竷
*
*    Copyright (C), 2018-2030, 瀹夊瘜銮辩数瀛?www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "usbd_user.h"
#include "modbus_slave.h"

uint8_t g_ModbusRxBuf[RX_BUF_SIZE];    
uint16_t g_ModbusRxLen = 0;


/* Private typedef ----------------------------------------------------------- */
/* Private define ------------------------------------------------------------ */
#define APP_RX_DATA_SIZE  2048        // 濂藉儚澶т簬512瀛楄妭娌″暐浣灭敤 */
#define APP_TX_DATA_SIZE  2048

/* Private macro ------------------------------------------------------------- */
/* Private variables --------------------------------------------------------- */
USBD_CDC_LineCodingTypeDef LineCoding = {
  115200,                       /* baud rate */
  0x00,                         /* stop bits-1 */
  0x00,                         /* parity - none */
  0x08                          /* nb. of bits 8 */
};

/* Prescaler declaration */
uint32_t uwPrescalerValue = 0;

uint8_t UserRxBuffer[APP_RX_DATA_SIZE]; /* Received Data over USB are stored in
                                         * this buffer */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE]; /* Received Data over UART (CDC
                                         * interface) are stored in this buffer
                                         */

uint32_t UserTxBufPtrIn = 0;    /* Increment this pointer or roll it back to
                                 * start address when data are received over
                                 * USART */
uint32_t UserTxBufPtrOut = 0;   /* Increment this pointer or roll it back to
                                 * start address when data are sent over USB */

/* UART handler declaration */
UART_HandleTypeDef A_UartHandle;
UART_HandleTypeDef B_UartHandle;

UART_HandleTypeDef    *NowUartHandle = &A_UartHandle;    /* 褰揿墠阃夋嫨镄刄art */

/* TIM handler declaration */
TIM_HandleTypeDef TimHandle;
/* USB handler declaration */
extern USBD_HandleTypeDef USBD_Device;

/* Private function prototypes ----------------------------------------------- */
static int8_t CDC_Itf_Init(void);
static int8_t CDC_Itf_DeInit(void);
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t * pbuf, uint16_t length);
static int8_t CDC_Itf_Receive(uint8_t * pbuf, uint32_t * Len);

static void ComPort_Config(void);
static void TIM_Config(void);

USBD_CDC_ItfTypeDef USBD_CDC_fops = {
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive
};

/*
*********************************************************************************************************
*    鍑?鏁?鍚? SelectCDCUart
*    锷熻兘璇存槑: 阃夋嫨CDC涓插彛
*    褰?   鍙? _com : 1, 4
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
void SelectCDCUart(uint8_t _com)
{
    if (_com == 1)
    {
        NowUartHandle = &A_UartHandle;
    }
    else if (_com == 4)
    {
        NowUartHandle = &B_UartHandle;
    }    
    else
    {
        NowUartHandle = 0;
    }
}
    
/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this application: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef * huart)
{
    static DMA_HandleTypeDef hdma_tx;
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

    if (huart == &A_UartHandle)
    {
        /* ##-1- Enable peripherals and GPIO Clocks ################################# 
        */
        /* Enable GPIO clock */
        A_USARTx_TX_GPIO_CLK_ENABLE();
        A_USARTx_RX_GPIO_CLK_ENABLE();

        /* Select SysClk as source of USART1 clocks */
        RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART16;
        RCC_PeriphClkInit.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

        /* Enable USARTx clock */
        A_USARTx_CLK_ENABLE();

        /* Enable DMAx clock */
        A_DMAx_CLK_ENABLE();

        /* ##-2- Configure peripheral GPIO ########################################## 
        */
        /* UART TX GPIO pin configuration */
        GPIO_InitStruct.Pin = A_USARTx_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = A_USARTx_TX_AF;

        HAL_GPIO_Init(A_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration */
        GPIO_InitStruct.Pin = A_USARTx_RX_PIN;
        GPIO_InitStruct.Alternate = A_USARTx_RX_AF;

        HAL_GPIO_Init(A_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

        /* ##-4- Configure the DMA streams ########################################## 
        */
        /* Configure the DMA handler for Transmission process */
        hdma_tx.Instance = A_USARTx_TX_DMA_STREAM;
        hdma_tx.Init.Request = A_USARTx_TX_DMA_CHANNEL;
        hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_tx.Init.Mode = DMA_NORMAL;
        hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        hdma_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hdma_tx.Init.MemBurst = DMA_MBURST_INC4;
        hdma_tx.Init.PeriphBurst = DMA_PBURST_INC4;


        HAL_DMA_Init(&hdma_tx);

        /* Associate the initialized DMA handle to the UART handle */
        __HAL_LINKDMA(huart, hdmatx, hdma_tx);

        /* ##-5- Configure the NVIC for DMA ######################################### 
        */
        /* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
        HAL_NVIC_SetPriority(A_USARTx_DMA_TX_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(A_USARTx_DMA_TX_IRQn);

        /* ##-6- Configure the NVIC for UART ######################################## 
        */
        HAL_NVIC_SetPriority(A_USARTx_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(A_USARTx_IRQn);
    }
    else if (huart == &B_UartHandle)
    {
        /* ##-1- Enable peripherals and GPIO Clocks ################################# 
        */
        /* Enable GPIO clock */
        B_USARTx_TX_GPIO_CLK_ENABLE();
        B_USARTx_RX_GPIO_CLK_ENABLE();

        /* Select SysClk as source of USART1 clocks */
        RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART16;
        RCC_PeriphClkInit.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

        /* Enable USARTx clock */
        B_USARTx_CLK_ENABLE();

        /* Enable DMAx clock */
        B_DMAx_CLK_ENABLE();

        /* ##-2- Configure peripheral GPIO ########################################## 
        */
        /* UART TX GPIO pin configuration */
        GPIO_InitStruct.Pin = B_USARTx_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = B_USARTx_TX_AF;

        HAL_GPIO_Init(B_USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        /* UART RX GPIO pin configuration */
        GPIO_InitStruct.Pin = B_USARTx_RX_PIN;
        GPIO_InitStruct.Alternate = B_USARTx_RX_AF;

        HAL_GPIO_Init(B_USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

        /* ##-4- Configure the DMA streams ########################################## 
        */
        /* Configure the DMA handler for Transmission process */
        hdma_tx.Instance = B_USARTx_TX_DMA_STREAM;
        hdma_tx.Init.Request = B_USARTx_TX_DMA_CHANNEL;
        hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_tx.Init.Mode = DMA_NORMAL;
        hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        hdma_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hdma_tx.Init.MemBurst = DMA_MBURST_INC4;
        hdma_tx.Init.PeriphBurst = DMA_PBURST_INC4;


        HAL_DMA_Init(&hdma_tx);

        /* Associate the initialized DMA handle to the UART handle */
        __HAL_LINKDMA(huart, hdmatx, hdma_tx);

        /* ##-5- Configure the NVIC for DMA ######################################### 
        */
        /* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
        HAL_NVIC_SetPriority(B_USARTx_DMA_TX_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(B_USARTx_DMA_TX_IRQn);

        /* ##-6- Configure the NVIC for UART ######################################## 
        */
        HAL_NVIC_SetPriority(B_USARTx_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(B_USARTx_IRQn);
    }

//    宸插埌TIM閰岖疆鍑芥暟锛屽拰UART镞犲叧
//    /* ##-7- Enable TIM peripherals Clock ####################################### 
//    */
//    TIMx_CLK_ENABLE();

//    /* ##-8- Configure the NVIC for TIMx ######################################## 
//    */
//    /* Set Interrupt Group Priority */
//    HAL_NVIC_SetPriority(TIMx_IRQn, 6, 0);

//    /* Enable the TIMx global Interrupt */
//    HAL_NVIC_EnableIRQ(TIMx_IRQn);    
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this application:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef * huart)
{
    if (huart == &A_UartHandle)
    {
        /* ##-1- Reset peripherals ################################################## 
        */
        A_USARTx_FORCE_RESET();
        A_USARTx_RELEASE_RESET();

        /* ##-2- Disable peripherals and GPIO Clocks ################################ 
        */
        /* Configure UART Tx as alternate function */
        HAL_GPIO_DeInit(A_USARTx_TX_GPIO_PORT, A_USARTx_TX_PIN);
        /* Configure UART Rx as alternate function */
        HAL_GPIO_DeInit(A_USARTx_RX_GPIO_PORT, A_USARTx_RX_PIN);

        /* ##-3- Disable the NVIC for UART ########################################## 
        */
        HAL_NVIC_DisableIRQ(A_USARTx_IRQn);

        /* ##-4- Reset TIM peripheral ############################################### 
        */
        //    TIMx_FORCE_RESET();
        //    TIMx_RELEASE_RESET();
    }
    else if (huart == &B_UartHandle)
    {
        /* ##-1- Reset peripherals ################################################## 
        */
        B_USARTx_FORCE_RESET();
        B_USARTx_RELEASE_RESET();

        /* ##-2- Disable peripherals and GPIO Clocks ################################ 
        */
        /* Configure UART Tx as alternate function */
        HAL_GPIO_DeInit(B_USARTx_TX_GPIO_PORT, B_USARTx_TX_PIN);
        /* Configure UART Rx as alternate function */
        HAL_GPIO_DeInit(B_USARTx_RX_GPIO_PORT, B_USARTx_RX_PIN);

        /* ##-3- Disable the NVIC for UART ########################################## 
        */
        HAL_NVIC_DisableIRQ(B_USARTx_IRQn);

        /* ##-4- Reset TIM peripheral ############################################### 
        */
        //    TIMx_FORCE_RESET();
        //    TIMx_RELEASE_RESET();
    }    
}

/* Private functions --------------------------------------------------------- */

/**
  * @brief  Initializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Init(void)
{
    /* ##-1- Configure the UART peripheral ###################################### */
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* USART configured as follow: - Word Length = 8 Bits - Stop Bit = One Stop
    * bit - Parity = No parity - BaudRate = 115200 baud - Hardware flow control
    * disabled (RTS and CTS signals) */
    if (NowUartHandle == &A_UartHandle)
    {
        A_UartHandle.Instance = A_USARTx;
        A_UartHandle.Init.BaudRate = 115200;
        A_UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
        A_UartHandle.Init.StopBits = UART_STOPBITS_1;
        A_UartHandle.Init.Parity = UART_PARITY_NONE;
        A_UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        A_UartHandle.Init.Mode = UART_MODE_TX_RX;
        A_UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
        if (HAL_UART_Init(&A_UartHandle) != HAL_OK)
        {
            /* Initialization Error */
            ERROR_HANDLER();
        }

        /* ##-2- Put UART peripheral in IT reception process ######################## */
        /* Any data received will be stored in "UserTxBuffer" buffer */
        if (HAL_UART_Receive_IT(&A_UartHandle, (uint8_t *) UserTxBuffer, 1) != HAL_OK)
        {
            /* Transfer error in reception process */
            ERROR_HANDLER();
        }        
    }
    else if (NowUartHandle == &B_UartHandle)
    {
        B_UartHandle.Instance = B_USARTx;
        B_UartHandle.Init.BaudRate = 115200;
        B_UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
        B_UartHandle.Init.StopBits = UART_STOPBITS_1;
        B_UartHandle.Init.Parity = UART_PARITY_NONE;
        B_UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        B_UartHandle.Init.Mode = UART_MODE_TX_RX;
        B_UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
        if (HAL_UART_Init(&B_UartHandle) != HAL_OK)
        {
            /* Initialization Error */
            ERROR_HANDLER();
        }
        
        /* ##-2- Put UART peripheral in IT reception process ######################## */
        /* Any data received will be stored in "UserTxBuffer" buffer */
        if (HAL_UART_Receive_IT(&B_UartHandle, (uint8_t *) UserTxBuffer, 1) != HAL_OK)
        {
            /* Transfer error in reception process */
            ERROR_HANDLER();
        }        
    }

    /* ##-3- Configure the TIM Base generation ################################# */
    TIM_Config();

    /* ##-4- Start the TIM Base generation in interrupt mode #################### */
    /* Start Channel1 */
    if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
    {
        /* Starting Error */
        ERROR_HANDLER();
    }

    /* ##-5- Set Application Buffers ############################################ */
    USBD_CDC_SetTxBuffer(&USBD_Device, UserTxBuffer, 0);
    USBD_CDC_SetRxBuffer(&USBD_Device, UserRxBuffer);

    return (USBD_OK);
}

/**
  * @brief  CDC_Itf_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_DeInit(void)
{
    if (NowUartHandle == 0)
    {
        return (USBD_OK);
    }
    
    /* DeInitialize the UART peripheral */
    if (HAL_UART_DeInit(NowUartHandle) != HAL_OK)
    {
        /* Initialization Error */
        ERROR_HANDLER();
    }

    return (USBD_OK);
}

/**
  * @brief  CDC_Itf_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t * pbuf, uint16_t length)
{
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    /* Add your code here */
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    /* Add your code here */
    break;

  case CDC_SET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_GET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_CLEAR_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_SET_LINE_CODING:
    LineCoding.bitrate = (uint32_t) (pbuf[0] | (pbuf[1] << 8) |
                                     (pbuf[2] << 16) | (pbuf[3] << 24));
    LineCoding.format = pbuf[4];
    LineCoding.paritytype = pbuf[5];
    LineCoding.datatype = pbuf[6];

    /* Set the new configuration */
    ComPort_Config();
    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t) (LineCoding.bitrate);
    pbuf[1] = (uint8_t) (LineCoding.bitrate >> 8);
    pbuf[2] = (uint8_t) (LineCoding.bitrate >> 16);
    pbuf[3] = (uint8_t) (LineCoding.bitrate >> 24);
    pbuf[4] = LineCoding.format;
    pbuf[5] = LineCoding.paritytype;
    pbuf[6] = LineCoding.datatype;
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    /* Add your code here */
    break;

  case CDC_SEND_BREAK:
    /* Add your code here */
    break;

  default:
    break;
  }

  return (USBD_OK);
}

/**
  * @brief  TIM period elapsed callback
  * @param  htim: TIM handle
  * @retval None
  */
/*
*********************************************************************************************************
*    鍑?鏁?鍚? HAL_TIM_PeriodElapsedCallback
*    锷熻兘璇存槑: 瀹氭椂杞.  5ms 锛屾渶蹇彲浠ヨ皟鍒?ms
*    褰?   鍙? _com : 1, 4
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim)
{
    uint32_t buffptr;
    uint32_t buffsize;

    if (UserTxBufPtrOut != UserTxBufPtrIn)
    {
        if (UserTxBufPtrOut > UserTxBufPtrIn) /* Rollback */
        {
            buffsize = APP_TX_DATA_SIZE - UserTxBufPtrOut;
        }
        else
        {
            buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
        }

        buffptr = UserTxBufPtrOut;

        USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t *) & UserTxBuffer[buffptr],
            buffsize);

        if (USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK)
        {
            UserTxBufPtrOut += buffsize;
            if (UserTxBufPtrOut == APP_RX_DATA_SIZE)
            {
                UserTxBufPtrOut = 0;    // test
            }
        }
    }
}

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
  /* Increment Index for buffer writing */
  UserTxBufPtrIn++;

  /* To avoid buffer overflow */
  if (UserTxBufPtrIn == APP_RX_DATA_SIZE)
  {
    UserTxBufPtrIn = 0;
  }

  /* Start another reception: provide the buffer pointer with offset and the
   * buffer size */
  HAL_UART_Receive_IT(huart, (uint8_t *) (UserTxBuffer + UserTxBufPtrIn), 1);
}


/**
  * @brief  CDC_Itf_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
/*
*********************************************************************************************************
*    鍑?鏁?鍚? CDC_Itf_Receive
*    锷熻兘璇存槑: STM32鏀跺埌USB鏁版嵁鍚庢墽琛屾湰鍑芥暟銆?鏁版嵁闀垮害链€澶?12瀛楄妭銆? CDC_DATA_HS_MAX_PACKET_SIZE = 512
*                  濡傛灉涓娄綅链烘湁澶т簬512瀛楄妭镄勬暟鎹紝鍒欎细鍒嗗寘浼犺緭銆?
*    褰?   鍙? 镞?
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
static int8_t CDC_Itf_Receive(uint8_t * Buf, uint32_t *Len)
{
    SCB_CleanDCache_by_Addr((uint32_t *)Buf, *Len);

    if (NowUartHandle == 0)        /*  */
    {        
        uint32_t len;
        
        len = *Len;
        
        if (g_ModbusRxLen + len <= RX_BUF_SIZE)
        {
            memcpy(&g_ModbusRxBuf[g_ModbusRxLen], Buf, len);
            g_ModbusRxLen += len;
        }
        
        /* 鍒ゆ柇闀垮害涓嶆槸澶ソ镄勬柟妗? 濡傛灉澶т簬512瀛楄妭 */
        if (len != 512)
        {        
            MODS_Poll(g_ModbusRxBuf, g_ModbusRxLen);
            g_ModbusRxLen = 0;
            
            if (g_tModS.TxCount > 0)
            {
                
                USBCom_SendBufNow(0, g_tModS.TxBuf, g_tModS.TxCount);
                
//                for (i = 0; i < g_tModS.TxCount / 512; i++)
//                {
//                    USBCom_SendBuf(0, &g_tModS.TxBuf[pos], 512);
//                    pos += 512;                    
//                }
//                
//                if (g_tModS.TxCount % 512)
//                {
//                    USBCom_SendBuf(0, &g_tModS.TxBuf[pos], g_tModS.TxCount % 512);
//                }
            }
        }
        
        /* Initiate next USB packet transfer once UART completes transfer
        * (transmitting data over Tx line) */
        USBD_CDC_ReceivePacket(&USBD_Device);        
    }
    else
    {
        if (NowUartHandle == &A_UartHandle)
        {
            RS485_TX_EN();    /* 485鍙戦€佷娇鑳?*/

            HAL_UART_Transmit_DMA(&A_UartHandle, Buf, *Len);

        }
        else if (NowUartHandle == &B_UartHandle)
        {
            HAL_UART_Transmit_DMA(&B_UartHandle, Buf, *Len);
        }
    }
    return (USBD_OK);
}

/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
/*
*********************************************************************************************************
*    鍑?鏁?鍚? HAL_UART_TxCpltCallback
*    锷熻兘璇存槑: STM32涓插彛鍙戝嚭镄勬暟鎹叏閮ㄥ彂阃佸畲姣曟椂镓ц链嚱鏁般€?
*    褰?   鍙? 镞?
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart)
{
    if (NowUartHandle == &A_UartHandle)
    {
        RS485_RX_EN();    /* 涓插彛鍙戦€佸畲姣曘€傜姝?85鍙戦€侊紝璁剧疆涓烘帴鏀舵ā寮?*/
    }
    else if (NowUartHandle == &B_UartHandle)
    {
        ;
    }
    /* Initiate next USB packet transfer once UART completes transfer
    * (transmitting data over Tx line) */
    USBD_CDC_ReceivePacket(&USBD_Device);
}

/**
  * @brief  ComPort_Config
  *         Configure the COM Port with the parameters received from host.
  * @param  None.
  * @retval None
  * @note   When a configuration is not supported, a default value is used.
  */
static void ComPort_Config(void)
{            
    if (NowUartHandle == 0)
    {
        return;
    }
    
    if (HAL_UART_DeInit(NowUartHandle) != HAL_OK)
    {
        /* Initialization Error */
        ERROR_HANDLER();
    }

    /* set the Stop bit */
    switch (LineCoding.format)
    {
        case 0:
            NowUartHandle->Init.StopBits = UART_STOPBITS_1;
            break;
        case 2:
            NowUartHandle->Init.StopBits = UART_STOPBITS_2;
            break;
        default:
            NowUartHandle->Init.StopBits = UART_STOPBITS_1;
            break;
    }

    /* set the parity bit */
    switch (LineCoding.paritytype)
    {
        case 0:
            NowUartHandle->Init.Parity = UART_PARITY_NONE;
            break;
        case 1:
            NowUartHandle->Init.Parity = UART_PARITY_ODD;
            break;
        case 2:
            NowUartHandle->Init.Parity = UART_PARITY_EVEN;
            break;
        default:
            NowUartHandle->Init.Parity = UART_PARITY_NONE;
            break;
    }

    /* set the data type : only 8bits and 9bits is supported */
    switch (LineCoding.datatype)
    {
        case 0x07:
            /* With this configuration a parity (Even or Odd) must be set */
            NowUartHandle->Init.WordLength = UART_WORDLENGTH_8B;
            break;
        case 0x08:
            if (NowUartHandle->Init.Parity == UART_PARITY_NONE)
            {
                NowUartHandle->Init.WordLength = UART_WORDLENGTH_8B;
            }
            else
            {
                NowUartHandle->Init.WordLength = UART_WORDLENGTH_9B;
            }

            break;
        default:
            NowUartHandle->Init.WordLength = UART_WORDLENGTH_8B;
            break;
    }

    NowUartHandle->Init.BaudRate = LineCoding.bitrate;
    NowUartHandle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    NowUartHandle->Init.Mode = UART_MODE_TX_RX;
    NowUartHandle->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(NowUartHandle) != HAL_OK)
    {
        /* Initialization Error */
        ERROR_HANDLER();
    }

    /* Start reception: provide the buffer pointer with offset and the buffer
    * size */
    HAL_UART_Receive_IT(NowUartHandle, (uint8_t *) (UserTxBuffer + UserTxBufPtrIn),
                      1);
}

/**
  * @brief  TIM_Config: Configure TIMx timer
  * @param  None.
  * @retval None
  */
static void TIM_Config(void)
{
    /* ##  Enable TIM peripherals Clock ####################################### */
    TIMx_CLK_ENABLE();

    /* ##  Configure the NVIC for TIMx ######################################## */
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriority(TIMx_IRQn, 6, 0);

    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIMx_IRQn);        

    /* Set TIMx instance */
    TimHandle.Instance = TIMx;

    /* Compute the prescaler value to have TIMx counter clock equal to 10000 Hz */
    uwPrescalerValue = (uint32_t) (SystemCoreClock / (2 * 10000)) - 1;

    /* Initialize TIM3 peripheral as follows: + Period = (CDC_POLLING_INTERVAL *
    * 10000) - 1 + Prescaler = ((APB1 frequency / 1000000) - 1) + ClockDivision
    * = 0 + Counter direction = Up */
    TimHandle.Init.Period = (CDC_POLLING_INTERVAL * 10) - 1;
    TimHandle.Init.Prescaler = uwPrescalerValue;
    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
        /* Initialization Error */
        ERROR_HANDLER();
    }
    /* ##-2- Start the TIM Base generation in interrupt mode #################### */
    /* Start Channel1 */
    if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
    {
        /* Starting Error */
        ERROR_HANDLER();
    }
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef * UartHandle)
{
  /* Transfer error occured in reception and/or transmission process */
  //ERROR_HANDLER();
}

/*
*********************************************************************************************************
*    鍑?鏁?鍚? USBCom_SendBufNow
*    锷熻兘璇存槑: STM32鍙戦€佷竴涓叉暟鎹埌PC链篣SB锛岀珛鍗冲彂阃?
*    褰?   鍙? 镞?
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
uint8_t USBCom_SendBufNow(int _Port, uint8_t *_Buf, uint16_t _Len)
{
    memcpy(UserTxBuffer, _Buf, _Len);
    USBD_CDC_SetTxBuffer(&USBD_Device, UserTxBuffer, _Len);
    if (USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK)
    {
        return 0;
    }
    return 1;
}
    
/*
*********************************************************************************************************
*    鍑?鏁?鍚? USBCom_SendBuf
*    锷熻兘璇存槑: 鍙戦€佷竴涓叉暟鎹埌USB涓插彛, 浠呬粎缂揿瓨锛屼笉鍙戦€?
*    褰?   鍙? 镞?
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
uint8_t USBCom_SendBuf(int _Port, uint8_t *_Buf, uint16_t _Len)
{
    uint16_t i;

    for (i = 0; i < _Len; i++)
    {
        UserTxBuffer[UserTxBufPtrIn] = _Buf[i];
        
        if (++UserTxBufPtrIn == APP_RX_DATA_SIZE)
        {
            UserTxBufPtrIn = 0;
        }
    }
    return 1;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
