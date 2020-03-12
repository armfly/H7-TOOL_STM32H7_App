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

typedef struct 
{
    char FilePath[128];         /* lua文件路径 */    
    
    CHIP_TYPE_E ChipType;       /* 芯片类型 */
    
    uint32_t Time;
    
    uint32_t EraseChipTime1;
    uint32_t EraseChipTime2;
    
    float Percent;              /* 烧录进度 */
    
    uint8_t Err;
    
    uint32_t FLMFuncTimeout;    /* 执行算法函数超时时间。擦除整片时可能耗时20秒 */
    uint8_t FLMEraseChipFlag;   /* 临时处理，正在擦除全片*/

    uint8_t AutoStart;      /* 检测到芯片后自动开始编程 */
    
    int32_t NowProgCount;   /* 编程次数。掉电清零 */
    
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
