/*
*********************************************************************************************************
*
*	模块名称 : main模块
*	文件名称 : main.h
*
*********************************************************************************************************
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "ui_def.h"
#include "param.h"
#include "modbus_register.h"

#define SWITCH_BEEP_ENABLE 1 /* 1表示切换主状态时，蜂鸣器鸣叫 */

/* 主程序状态字定义, MS = Main Status */
enum
{
  MS_LINK_MODE = 0, /* 联机状态 */

  MS_SYSTEM_SET, /* 系统设置 */
  MS_HARD_INFO,  /* 关于-硬件信息 */
  MS_ESP32_TEST, /* ESP32模块升级状态 */
  MS_USB_UART1,  /* 虚拟串口状态。RS232 RS485 TTL-UART */

  MS_PROGRAMMER,     /* 脱机下载器 */
  MS_VOLTAGE_METER,  /* 电压表 */
  MS_CURRENT_METER,  /* 高侧电流表 */
  MS_TEMP_METER,     /* 温度表 */
  MS_RESISTOR_METER, /* 电阻表 */
};

void DispHeader(char *_str);
uint16_t NextStatus(uint16_t _NowStatus);
uint16_t LastStatus(uint16_t _NowStatus);
void DSO_StartMode2(void);

extern uint16_t g_MainStatus;

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
