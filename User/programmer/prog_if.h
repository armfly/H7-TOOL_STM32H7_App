/*
*********************************************************************************************************
*
*    模块名称 : 编程器接口
*    文件名称 : prog_if.h
*    版    本 : V1.0
*    说    明 : 头文件
*
*    Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __PROG_IF_H_
#define __PROG_IF_H_

#include "lua_if.h"

/* */
typedef enum
{
    CHIP_SWD_ARM     = 0,
    CHIP_SWIM_STM8   = 1,
    CHIP_SPI_FLASH   = 2,
    CHIP_I2C_EEPROM  = 3,
}CHIP_TYPE_E;

typedef enum
{
    SOFT_RESET  = 0,    
    HARD_RESET  = 1
}REST_TYPE_E;

/* 校验模式 */
typedef enum
{
    VERIFY_AUTO = 0,            /* 自动选择. FLM中有CRC就执行, 没有就采用ReadBack  */
    VERIFY_READ_BACK  = 1,      /* 读回校验 */
    VERIFY_SOFT_CRC   = 2,      /* 软件CRC */
    VERIFY_STM32_CRC  = 3,      /* STM32 CRC硬件算法 */
}VERIFY_MODE_E;

typedef struct 
{
    char FilePath[128];         /* lua文件路径 */    
    
    CHIP_TYPE_E ChipType;       /* 芯片类型 */
    
    REST_TYPE_E ResetType;      /* 复位类型 */
    
    uint32_t Time;
    
    float Percent;              /* 烧录进度 */
    
    uint8_t Err;
    
    uint32_t FLMFuncTimeout;    /* 执行算法函数超时时间。擦除整片时可能耗时20秒 */
    uint8_t FLMFuncDispProgress;/* 临时处理，正在擦除全片， 需要显示进度 */
    uint32_t FLMFuncDispAddr;   /* 进度条中的地址栏 */
    
    uint8_t AutoStart;      /* 检测到芯片后自动开始编程 */
    
    int32_t NowProgCount;   /* 编程次数。掉电清零 */
    
    uint32_t VerifyMode;    /* 校验模式 */
    uint32_t EraseChipTime; /* 全片擦除时间 */
    
    uint8_t VerifyOptionByteDisalbe;   /* 1 表示芯片设置读保护时,不校验 (STM32F207VC设置读保护后会即可生效导致无法校验) */
    
    uint32_t SwdClockDelay;     /* SWD时钟延迟，0表示最快，值越大速度越慢 */
    
    uint8_t UidEnable;      /* 编程时是否填充UID */
    uint32_t UidAddr;       /* 加密后的UID存储地址 */
    const char *UidData;    /* UID数据缓冲区指针，由Lua生成 */
    uint16_t UidLen;        /* UID数据缓冲区长度 */
    uint8_t UidBlank;       /* 空标志 */
    
    uint8_t UsrEnable;      /* 编程时是否填充用户指定数据 */    
    uint32_t UsrAddr;       /* 用户数据存储地址 */
    const char *UsrData;    /* 用户数据缓冲区指针，由Lua生成 */
    uint16_t UsrLen;        /* 用户数据缓冲区长度 */
    uint8_t UsrBlank;       /* 空标志 */
    
    uint8_t SnEnable;       /* 编程时是否填充SN */
    uint32_t SnAddr;        /* SN数据存储地址 */
    const char *SnData;     /* SN数据缓冲区指针，由Lua生成 */
    uint16_t SnLen;         /* SN数据缓冲区长度 */ 
    uint8_t SndBlank;       /* 空标志 */    
    
}OFFLINE_PROG_T;

extern OFFLINE_PROG_T g_tProg;

void PG_ReloadLuaVar(void);

uint16_t PG_SWD_ProgFile(char *_Path, uint32_t _FlashAddr);

uint16_t PG_SWD_ProgBuf(uint32_t _FlashAddr, uint8_t *_DataBuf, uint32_t _BufLen, uint8_t _Mode);

uint16_t PG_SWD_ProgBuf_OB(uint32_t _FlashAddr, uint8_t *_DataBuf, uint32_t _BufLen);

uint16_t PG_SWD_EraseChip(uint32_t _FlashAddr);
uint16_t PG_SWD_EraseSector(uint32_t _FlashAddr);

void DispProgProgress(char *_str, float _progress, uint32_t _addr);

uint32_t GetChipTypeFromLua(lua_State *L);

uint8_t WaitChipInsert(void);
uint8_t WaitChipRemove(void);

#endif
