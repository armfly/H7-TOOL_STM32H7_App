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
#ifndef __USH_USR_H__
#define __USH_USR_H__

#include "ff.h"
#include "usbh_core.h"
#include "usbh_conf.h"
#include <stdio.h>

void usbh_OpenMassStorage(void);
void usbh_CloseMassStorage(void);
void usbh_Poll(void);

extern USBH_HandleTypeDef hUSBHost;
extern FATFS USBH_fatfs;
 
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
