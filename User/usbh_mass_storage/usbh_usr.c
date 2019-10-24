/*
*********************************************************************************************************
*
*	模块名称 : U盘驱动用户接口
*	文件名称 : usbh_usr.c
*	版    本 : V1.0
*	说    明 : 封装U盘操作函数，提供给APP使用.
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
#include "usbh_usr.h"
#include "usbh_msc.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
 
#define usbh_printf	printf
 
USBH_HandleTypeDef hUSBHost;
FATFS USBH_fatfs;
char USBDISKPath[4];            /* USB Host logical drive path */
  
static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id);

/*
*********************************************************************************************************
*	函 数 名: USBH_UserProcess
*	功能说明: USB host 回调函数. 可以在此给app发送消息
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
	switch (id)
	{
		case HOST_USER_SELECT_CONFIGURATION:
			break;

		case HOST_USER_DISCONNECTION:		/* U盘断开 */
			if (f_mount(NULL, "", 0) != FR_OK)
			{
				usbh_printf("ERROR : Cannot DeInitialize FatFs! \n");
			}
			if (FATFS_UnLinkDriver(USBDISKPath) != 0)
			{
				usbh_printf("ERROR : Cannot UnLink FatFS Driver! \n");
			}
			break;

		case HOST_USER_CLASS_ACTIVE:
			break;

		case HOST_USER_CONNECTION:		/* U盘插入 */
			if (FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0)
			{
				if (f_mount(&USBH_fatfs, "", 0) != FR_OK)
				{
					usbh_printf("ERROR : Cannot Initialize FatFs! \n");
				}
			}
			break;

		default:
		break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: usbh_OpenMassStorage
*	功能说明: 打开U盘设备
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void usbh_OpenMassStorage(void)
{
	/* Init Host Library */
	USBH_Init(&hUSBHost, USBH_UserProcess, 0);

	/* Add Supported Class */
	USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

	/* Start Host Process */
	USBH_Start(&hUSBHost);
	
	HAL_PWREx_EnableUSBVoltageDetector();
}

/*
*********************************************************************************************************
*	函 数 名: usbh_CloseMassStorage
*	功能说明: 关闭U盘设备
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void usbh_CloseMassStorage(void)
{
	USBH_Stop(&hUSBHost);
	
	USBH_DeInit(&hUSBHost);
}

/*
*********************************************************************************************************
*	函 数 名: usbh_Poll
*	功能说明: 主程序需要轮询执行本函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void usbh_Poll(void)
{
	/* USB Host Background task , 在 usbh_core.c 中调用 */
    USBH_Process(&hUSBHost);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
