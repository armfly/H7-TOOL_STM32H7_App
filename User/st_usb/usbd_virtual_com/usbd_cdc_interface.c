/*
*********************************************************************************************************
*
*	模块名称 : CDC虚拟串口硬件配置
*	文件名称 : usbd_cdc_interface.c
*	版    本 : V1.0
*	说    明 : 
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2018-12-11  armfly  正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
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
#define APP_RX_DATA_SIZE  2048		// 好像大于512字节没啥作用 */
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

static void UARTRxCpltCallback(uint8_t _byte);

static COM_PORT_E s_NowCom = COM_USB1;

/*
*********************************************************************************************************
*	函 数 名: SelectCDCUart
*	功能说明: 选择CDC串口
*	形    参: _com : 1, 4, 8
*	返 回 值: 无
*********************************************************************************************************
*/
void SelectCDCUart(uint8_t _com)
{
    comSetCallbackReciveNew(COM1, 0);
    comSetCallbackReciveNew(COM4, 0);
    
	if (_com == 1)
	{
		s_NowCom = COM1;
        comSetCallbackReciveNew(COM1, UARTRxCpltCallback);        
	}
	else if (_com == 4)
	{
		s_NowCom = COM4;       
        comSetCallbackReciveNew(COM4, UARTRxCpltCallback);
	}	
	else if (_com == COM_USB1)
	{
		s_NowCom = COM_USB1;
	}	
	else if (_com == COM_USB2)
	{
		s_NowCom = COM_USB2;
	}	
    CDC_Itf_Init();    
}

/* Private functions --------------------------------------------------------- */

/**
  * @brief  Initializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Init(void)
{
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
*	函 数 名: HAL_TIM_PeriodElapsedCallback
*	功能说明: 定时轮询.  5ms ，最快可以调到1ms
*	形    参: _com : 1, 4
*	返 回 值: 无
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
				UserTxBufPtrOut = 0;	// test
			}
		}
	}
}

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
static void UARTRxCpltCallback(uint8_t _byte)
{
    UserTxBuffer[UserTxBufPtrIn] = _byte;
    
    /* Increment Index for buffer writing */
    UserTxBufPtrIn++;

    /* To avoid buffer overflow */
    if (UserTxBufPtrIn == APP_RX_DATA_SIZE)
    {
        UserTxBufPtrIn = 0;
    }
}

/*
*********************************************************************************************************
*	函 数 名: Uart1TxCpltCallback
*	功能说明: STM32串口发出的数据全部发送完毕时执行本函数、
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern void RS485_SendOver(void);
void Uart1TxCpltCallback(void)
{
	RS485_SendOver();
    
    /* Initiate next USB packet transfer once UART completes transfer
	* (transmitting data over Tx line) */
	USBD_CDC_ReceivePacket(&USBD_Device);
    
    comSetCallbackSendOver(COM1, 0);
}

/*
*********************************************************************************************************
*	函 数 名: Uart4TxCpltCallback
*	功能说明: STM32串口发出的数据全部发送完毕时执行本函数、
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Uart4TxCpltCallback(void)
{
	/* Initiate next USB packet transfer once UART completes transfer
	* (transmitting data over Tx line) */
	USBD_CDC_ReceivePacket(&USBD_Device);
    
    comSetCallbackSendOver(COM4, 0);
}

/*
*********************************************************************************************************
*	函 数 名: CDC_Itf_Receive
*	功能说明: STM32收到USB数据后执行本函数。 数据长度最大512字节。  CDC_DATA_HS_MAX_PACKET_SIZE = 512
*		  		如果上位机有大于512字节的数据，则会分包传输。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendBefor(void);
static int8_t CDC_Itf_Receive(uint8_t * Buf, uint32_t *Len)
{
	SCB_CleanDCache_by_Addr((uint32_t *)Buf, *Len);
        
	if (s_NowCom == COM_USB1)		/*  */
	{		
		uint32_t len;
		
		len = *Len;
		
		if (g_ModbusRxLen + len <= RX_BUF_SIZE)
		{
			memcpy(&g_ModbusRxBuf[g_ModbusRxLen], Buf, len);
			g_ModbusRxLen += len;
		}
		
		/* 判断长度不是太好的方案. 如果大于512字节 */
		if (len != 512)
		{		
			MODS_Poll(g_ModbusRxBuf, g_ModbusRxLen);
			g_ModbusRxLen = 0;
			
			if (g_tModS.TxCount > 0)
			{				
				USBCom_SendBufNow(0, g_tModS.TxBuf, g_tModS.TxCount);
			}
		}
		
		/* Initiate next USB packet transfer once UART completes transfer
		* (transmitting data over Tx line) */
		USBD_CDC_ReceivePacket(&USBD_Device);		
	}
	else if (s_NowCom == COM1)
	{
        RS485_SendBefor();
        comSetCallbackSendOver(COM1, Uart1TxCpltCallback);
        comSendBuf(COM1, Buf, *Len);
    }
	else if (s_NowCom == COM4)
	{
        comSetCallbackSendOver(COM4, Uart4TxCpltCallback);
        comSendBuf(COM4, Buf, *Len);
	}
	return (USBD_OK);
}

/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */


/**
  * @brief  ComPort_Config
  *         Configure the COM Port with the parameters received from host.
  * @param  None.
  * @retval None
  * @note   When a configuration is not supported, a default value is used.
  */
static void ComPort_Config(void)
{		
    uint32_t StopBits;
    uint32_t Parity;
    uint32_t WordLength;
    uint32_t BaudRate;
    
    
    if (s_NowCom >= COM_USB1)
    {
        return;
    }
        
	/* set the Stop bit */
	switch (LineCoding.format)
	{
		case 0:
			StopBits = UART_STOPBITS_1;
			break;
		case 2:
			StopBits = UART_STOPBITS_2;
			break;
		default:
			StopBits = UART_STOPBITS_1;
			break;
	}

	/* set the parity bit */
	switch (LineCoding.paritytype)
	{
        case 0:
            Parity = UART_PARITY_NONE;
            break;
        case 1:
            Parity = UART_PARITY_ODD;
            break;
        case 2:
            Parity = UART_PARITY_EVEN;
            break;
        default:
            Parity = UART_PARITY_NONE;
            break;
	}

	/* set the data type : only 8bits and 9bits is supported */
	switch (LineCoding.datatype)
	{
		case 0x07:
			/* With this configuration a parity (Even or Odd) must be set */
			WordLength = UART_WORDLENGTH_8B;
			break;
        
		case 0x08:
			if (Parity == UART_PARITY_NONE)
			{
				WordLength = UART_WORDLENGTH_8B;
			}
			else
			{
				WordLength = UART_WORDLENGTH_9B;
			}
			break;
            
		default:
			WordLength = UART_WORDLENGTH_8B;
			break;
	}

	BaudRate = LineCoding.bitrate;

    bsp_SetUartParam(s_NowCom, BaudRate, Parity, WordLength, StopBits);
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

    /* 2020-11-27 V1.36 移动到后面，在前面从DAP跳转时会死机 */
	/* ##  Configure the NVIC for TIMx ######################################## */
	/* Set Interrupt Group Priority */
	HAL_NVIC_SetPriority(TIMx_IRQn, 6, 0);

	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ(TIMx_IRQn);		    
}

/*
*********************************************************************************************************
*	函 数 名: USBCom_SendBufNow
*	功能说明: STM32发送一串数据到PC机USB，立即发送
*	形    参: 无
*	返 回 值: 无
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
*	函 数 名: USBCom_SendBuf
*	功能说明: 发送一串数据到USB串口, 仅仅缓存，不发送
*	形    参: 无
*	返 回 值: 无
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
