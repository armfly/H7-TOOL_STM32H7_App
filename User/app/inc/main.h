/*
*********************************************************************************************************
*
*    模块名称 : main模块
*    文件名称 : main.h
*
*********************************************************************************************************
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "ui_def.h"
#include "param.h"
#include "modbus_register.h"
    
/* 主程序状态字定义, MS = Main Status */
enum
{
    MS_LINK_MODE = 0,       /* 联机状态 */

    MS_SYSTEM_SET,          /* 系统设置 */
    MS_HARD_INFO,           /* 关于-硬件信息 */
    MS_ESP32_TEST,          /* ESP32模块升级状态 */
    MS_USB_EMMC,            /* 虚拟串口状态。RS232 RS485 TTL-UART */
    MS_MODIFY_PARAM,        /* 参数设置 */
    MS_FILE_MANAGE,         /* 文件管理 */
    
    MS_PROG_SELECT_FILE,    /* 脱机下载器浏览文件 */
    MS_PROG_WORK,           /* 脱机下载器工作界面 */
    MS_PROG_SETTING,        /* 脱机下载器参数设置 */
    MS_PROG_MODIFY_PARAM,   /* 修改参数比如复位类型 */
    
    MS_VOLTAGE_METER,       /* 电压表 */
    MS_CURRENT_METER,       /* 高侧电流表 */
    MS_TEMP_METER,          /* 温度表 */
    MS_RESISTOR_METER,      /* 电阻表 */
    MS_TVCC_POWER,          /* 微型数控电源 */    
    MS_PULSE_METER,         /* 脉冲计 */
    
    MS_MINI_DSO,            /* 迷你示波器 */
    
    MS_EXTEND_MENU1,        /* 第1级菜单  */
    MS_EXTEND_MENU_LUA,     /* 第2级菜单-LUA程序 */      
    MS_EXTEND_MENU_REC,     /* 第2级菜单-数据记录仪 */    
    
    MS_LUA_SELECT_FILE,     /* 浏览lua文件 */
    MS_LUA_EXEC_FILE,       /* 执行lua文件 */
    
    MS_MONITOR_UART,        /* 串口监视器 */
    MS_MONITOR_CAN,         /* CAN监视器 */
    MS_MONITOR_GPIO,        /* IO监视器 */
    MS_MONITOR_ANALOG,      /* 模拟量监视器 */    
};

void DispHeader(char *_str);
void DispHeaderSn(uint8_t _idx);
void DispMeasBar(uint8_t _ucLine, char *_pName, char *_pValue, char *_pUnit);
void DispMeasBarEx(uint8_t _ucLine, char *_pName, char *_pValue, char *_pUnit, uint16_t _usFillColor);
void DispParamBar(uint8_t _ucLine, char *_pName, char *_pValue, uint8_t _ucActive);
void DispHelpBar(char *_str1, char *_str2);
void DispHeader2(uint8_t _idx, char *_str);
void DispHeaderStr(char *_str);
void DispInfoBar16(uint8_t _ucLine, char *_pName, char *_pValue);
void DispInfoBar16Ex(uint8_t _ucLine, char *_pName, char *_pValue, uint16_t _ucColor);
void DispBox(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void DispLabel(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
    uint16_t _usColor, char *_pStr, FONT_T *_tFont);
void DispProgressBar(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, 
    char *_str1, float _Percent, char *_str2, FONT_T *_tFont);
void ProgressBarSetColor(uint16_t _Color);
uint16_t NextStatus(uint16_t _NowStatus);
uint16_t LastStatus(uint16_t _NowStatus);
void DSO_StartMode2(void);
void PlayKeyTone(void);
void DispMsgBox(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, char *_str);

extern uint16_t g_MainStatus;

extern uint8_t qspi_file_buf[1024];
extern uint8_t qspi_read_buf[1024];


#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
