/*
*********************************************************************************************************
*
*    妯″潡鍚岖О : USB devie 铏氭嫙涓插彛椹卞姩
*    鏂囦欢鍚岖О : usbd_user.c
*    鐗?   链?: V1.0
*    璇?   鏄?: 灏佽铏氭嫙涓插彛鎿崭綔鍑芥暟锛屾彁渚涚粰APP浣跨敤.
*
*    淇敼璁板綍 :
*        鐗堟湰鍙? 镞ユ湡        浣滆€?    璇存槑
*        V1.0    2018-12-11  armfly  姝ｅ纺鍙戝竷
*
*    Copyright (C), 2018-2030, 瀹夊瘜銮辩数瀛?www.armfly.com
*
*********************************************************************************************************
*/

#include "usbd_user.h"

USBD_HandleTypeDef USBD_Device = {0};
extern PCD_HandleTypeDef hpcd;

extern void SelectCDCUart(uint8_t _com);
/*
*********************************************************************************************************
*    鍑?鏁?鍚? usbd_OpenCDC
*    锷熻兘璇存槑: 镓揿紑USB
*    褰?   鍙? _com : 1, 4
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
void usbd_OpenCDC(uint8_t _com)
{
    SelectCDCUart(_com);        /* 阃夋嫨uart1鎴杣art4浣滀负铏氭嫙涓插彛 */
    
    /* Init Device Library */
    USBD_Init(&USBD_Device, &VCP_Desc, 0);

    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

    /* Add CDC Interface Class */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

    /* Start Device Process */
    USBD_Start(&USBD_Device);
}

/*
*********************************************************************************************************
*    鍑?鏁?鍚? usbd_CloseCDC
*    锷熻兘璇存槑: 鍏抽棴USB
*    褰?   鍙? 镞?
*    杩?锲?链? 镞?
*********************************************************************************************************
*/
void usbd_CloseCDC(void)
{
    if (USBD_Device.dev_config == 0)
    {
        return;        
    }
    
    USBD_Stop(&USBD_Device);
    
    USBD_DeInit(&USBD_Device);
}

/***************************** 瀹夊瘜銮辩数瀛?www.armfly.com (END OF FILE) *********************************/
