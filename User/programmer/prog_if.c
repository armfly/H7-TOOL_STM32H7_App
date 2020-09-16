/*
*********************************************************************************************************
*
*    模块名称 : 编程器接口文件
*    文件名称 : prog_if.c
*    版    本 : V1.0
*    说    明 : 
*
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2019-03-19  armfly  正式发布
*
*    Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "param.h"
#include "file_lib.h"
#include "lua_if.h"
#include "prog_if.h"
#include "swd_host.h"
#include "swd_flash.h"
#include "elf_file.h"
#include "main.h"
#include "stm8_flash.h"
#include "SW_DP_Multi.h"
#include "swd_host_multi.h"

extern const program_target_t flash_algo;

OFFLINE_PROG_T g_tProg = {0};

PROG_INI_T g_tProgIni = {0};

FIX_DATA_T g_tFixData = {0};

uint8_t flash_buff[sizeof(FsReadBuf)];

uint8_t PG_CheckFlashFix(uint32_t _FlashAddr, uint32_t _BuffSize, uint32_t _FileIndex);
uint8_t PG_CheckFixSplit(uint32_t _FlashAddr, uint32_t _BuffSize, uint32_t _FileIndex);
uint8_t PG_FixFlashMem(uint32_t _FlashAddr, char *_Buff, uint32_t _BuffSize, uint32_t _FileIndex, uint8_t _chan);

/*
*********************************************************************************************************
*    函 数 名: CancelKey
*    功能说明: 终止键按下
*    形    参: 无
*    返 回 值: 1表示需要立即终止
*********************************************************************************************************
*/
uint8_t ProgCancelKey(void)
{    
    if (bsp_GetKey() == KEY_LONG_DOWN_C)
    {
        return 1;
    }
    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: GetChipTypeFromLua
*    功能说明: 从lua中解析芯片类型. 存放在全局变量 g_tProg.ChipType
*    形    参: 无
*    返 回 值: 1表示需要立即终止
*********************************************************************************************************
*/
uint32_t GetChipTypeFromLua(lua_State *L)
{
    g_tProg.ChipType = CHIP_SWD_ARM;
    
    if (L > 0)
    {
        /* 器件接口类型: "SWD", "SWIM", "SPI", "I2C" */
        if (lua_getglobal(L, "CHIP_TYPE") != 0)
        {
            const char *name;
             
            if (lua_isstring(g_Lua, -1)) 
            {
                name = lua_tostring(g_Lua, -1);
                if (strcmp(name, "SWD") == 0)
                {
                    g_tProg.ChipType = CHIP_SWD_ARM;
                }
                else if (strcmp(name, "SWIM") == 0)
                {
                    g_tProg.ChipType = CHIP_SWIM_STM8;
                }            
                else if (strcmp(name, "SPI") == 0)
                {
                    g_tProg.ChipType = CHIP_SPI_FLASH;
                } 
                else if (strcmp(name, "I2C") == 0)
                {
                    g_tProg.ChipType = CHIP_I2C_EEPROM;
                }                
            }           
        }
        lua_pop(L, 1);
    }
    return g_tProg.ChipType;
}        

/*
*********************************************************************************************************
*    函 数 名: WaitChipRemove
*    功能说明: 检测芯片移除状态
*    形    参: 无
*    返 回 值: 1表示已经移除，0表示芯片还在位
*********************************************************************************************************
*/
extern void sysTickInit(void);
uint8_t WaitChipRemove(void)
{    
    /* 由LUA程序提供函数 */ 
    const char *ret_str;

    lua_do("ret_str = CheckChipRemove()"); 
    lua_getglobal(g_Lua, "ret_str"); 
    if (lua_isstring(g_Lua, -1))
    {
        ret_str = lua_tostring(g_Lua, -1); 
    }
    else
    {
        ret_str = "";
    }
    lua_pop(g_Lua, 1);
        
    if (strcmp(ret_str, "removed") == 0)
    {
        return 1;
    }

    return 0; 
}

/*
*********************************************************************************************************
*    函 数 名: WaitChipInsert
*    功能说明: 检测芯片插入状态
*    形    参: 无
*    返 回 值: 1表示已经插入，0表示未检测到
*********************************************************************************************************
*/
uint8_t WaitChipInsert(void)
{
    const char *ret_str;

    lua_do("ret_str = CheckChipInsert()"); 
    lua_getglobal(g_Lua, "ret_str"); 
    if (lua_isstring(g_Lua, -1))
    {
        ret_str = lua_tostring(g_Lua, -1); 
    }
    else
    {
        ret_str = "";
    }
    lua_pop(g_Lua, 1);
        
    if (strcmp(ret_str, "inserted") == 0)
    {
        return 1;
    }

    return 0;   
}

/*
*********************************************************************************************************
*    函 数 名: PG_ReloadLuaVar
*    功能说明: 读取lua文件全局变量. 
*    形    参: _str : 字符串
*    返 回 值: 无
*********************************************************************************************************
*/
extern void h7swim_ReadLuaVar(void);
void PG_ReloadLuaVar(void)
{
    GetChipTypeFromLua(g_Lua);

    if (g_tProg.ChipType == CHIP_SWD_ARM)
    {    
        /* VERIFY_MODE;  校验模式 */
        {
            /* 读取复位类型： 软件还是硬件复位 */
            lua_getglobal(g_Lua, "VERIFY_MODE");  
            if (lua_isinteger(g_Lua, -1)) 
            {    
                g_tProg.VerifyMode = lua_tointeger(g_Lua, -1);
            }
            else
            {
                /* 没有定义则用读回模式校验 */
                g_tProg.VerifyMode = VERIFY_READ_BACK;
            }
            lua_pop(g_Lua, 1);            
        }

        /* ERASE_CHIP_TIME;  全片擦除时间ms */
        {
            lua_getglobal(g_Lua, "ERASE_CHIP_TIME");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.EraseChipTime = lua_tointeger(g_Lua, -1);
                if (g_tProg.EraseChipTime == 0)
                {
                    g_tProg.EraseChipTime = 60 * 1000;
                }
            }
            else
            {
                g_tProg.EraseChipTime = 60 * 1000;
            }
            lua_pop(g_Lua, 1);            
        }       
        
        /* 0B VERIFY;  option byte 验证取消 */
        {
            lua_getglobal(g_Lua, "OB_VERIFY_DISABLE");  
            if (lua_isinteger(g_Lua, -1)) 
            {    
                g_tProg.VerifyOptionByteDisalbe = lua_tointeger(g_Lua, -1);
            }
            else
            {
                g_tProg.VerifyOptionByteDisalbe = 0;
            }
            lua_pop(g_Lua, 1);            
        }
        
        /*  SwdClockDelay;  SWD接口时序延迟 */
        {
            lua_getglobal(g_Lua, "SWD_CLOCK_DELAY");  
            if (lua_isinteger(g_Lua, -1)) 
            {    
                g_tProg.SwdClockDelay = lua_tointeger(g_Lua, -1);
            }
            else
            {
                g_tProg.SwdClockDelay = 0;
            }
            lua_pop(g_Lua, 1);            
        }
        
        /*  复位延迟 */
        {
            lua_getglobal(g_Lua, "RESET_DELAY");  
            if (lua_isinteger(g_Lua, -1)) 
            {    
                g_tProg.SwdResetDelay = lua_tointeger(g_Lua, -1);
            }
            else
            {
                g_tProg.SwdResetDelay = 20;
            }
            lua_pop(g_Lua, 1);            
        }

        /* 读取复位类型： 软件还是硬件复位 */
        {
           
            lua_getglobal(g_Lua, "RESET_MODE");  
            if (lua_isinteger(g_Lua, -1)) 
            {    
                g_tProg.ResetMode = lua_tointeger(g_Lua, -1);
            }
            else
            {
                g_tProg.ResetMode = 0;
            }
            lua_pop(g_Lua, 1);            
        }         
        
    }
    else if (g_tProg.ChipType == CHIP_SWIM_STM8)  
    {
        h7swim_ReadLuaVar();        /* 读取LUA中的全局变量 */
    }
}

/*
*********************************************************************************************************
*    函 数 名: PG_LuaFixData
*    功能说明: 在编程函数调用。用于动态生成UID,SN,USR数据
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint8_t PG_LuaFixData(void)
{   
    size_t len;
    int i;
    
    //lua FIX_TABLE结构
    //1, 0, 0x08000000, 05 00 00 00 
    //1, 1, 0x08000000, 02 
    //1, 2, 0x08000000, FF 
    //1, 3, 0x08000000, 01 
    //1, 1, 0x08000000, FB 32 C9 38 BE C8 06 F1 
    //1, 1, 0x08000000, 92 06 02 0E 90 7C C4 02 90 00 4A 0E 96 58 C4 02 
    
    //lua->stack，得到全局表，位置-1
    lua_getglobal(g_Lua, "FIX_TABLE");
    g_tFixData.Count = 0;
    for (i = 0; i < MAX_FIX_LINE; i++)
    {
        lua_pushinteger(g_Lua, i * 4 + 1);  //c->statck，设置key值，位置-1(上面的-1变为-2)
        lua_gettable(g_Lua, -2);//返回值为值的类型        
        if (lua_isinteger(g_Lua, -1))
        {
            g_tFixData.Lines[i].Enable = lua_tointeger(g_Lua, -1);
            lua_pop(g_Lua, 1);            
        }
        else
        {
            lua_pop(g_Lua, 1);
            break;
        }
        
        lua_pushinteger(g_Lua, i * 4 + 2);  //c->statck，设置key值，位置-1(上面的-1变为-2)
        lua_gettable(g_Lua, -2);//返回值为值的类型        
        if (lua_isinteger(g_Lua, -1))
        {
            g_tFixData.Lines[i].Chan = lua_tointeger(g_Lua, -1);
            lua_pop(g_Lua, 1);            
        }
        else
        {
            lua_pop(g_Lua, 1);
            break;
        }
        
        lua_pushinteger(g_Lua, i * 4 + 3);  //c->statck，设置key值，位置-1(上面的-1变为-2)
        lua_gettable(g_Lua, -2);//返回值为值的类型        
        if (lua_isinteger(g_Lua, -1))
        {
            g_tFixData.Lines[i].Addr = lua_tointeger(g_Lua, -1);
        }
        lua_pop(g_Lua, 1);

        lua_pushinteger(g_Lua, i * 4 + 4);  //c->statck，设置key值，位置-1(上面的-1变为-2)
        lua_gettable(g_Lua, -2);//返回值为值的类型        
        if (lua_isstring(g_Lua, -1))
        {
            g_tFixData.Lines[i].pData = luaL_checklstring(g_Lua, -1, &len); /* 1是参数的位置， len是string的长度 */  
            g_tFixData.Lines[i].Len = len;
        }
        lua_pop(g_Lua, 1);  

        g_tFixData.Count++;
    }
    
    lua_pop(g_Lua, 1);
    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: PG_PrintText
*    功能说明: 烧录过程输出消息
*    形    参: _str : 字符串
*    返 回 值: 无
*********************************************************************************************************
*/
void PG_PrintText(char *_str)
{
    char str[128];
    
    /* 输出文本 */
    StrUTF8ToGBK(_str, str, sizeof(str));

    if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
    {
        if (g_gMulSwd.Error[0] == 1)
        {
            strcat(str, " #1");
        }
        if (g_gMulSwd.Error[1] == 1)
        {
            strcat(str, " #2");
        }
        if (g_gMulSwd.Error[2] == 1)
        {
            strcat(str, " #3");
        }
        if (g_gMulSwd.Error[3] == 1)
        {
            strcat(str, " #4");
        }          
    }           
    
    if (g_MainStatus == MS_PROG_WORK)
    {
        DispProgProgress(str, -1, 0xFFFFFFFF);      /* -1表示不刷新进度 */
    }
    
    strcat(str, "\r\n"); 
    printf(str);    
    
    bsp_Idle();
}

/*
*********************************************************************************************************
*    函 数 名: PG_PrintPercent
*    功能说明: 烧录过程输出进度百分比
*    形    参: 百分比，浮点数
*    返 回 值: 无
*********************************************************************************************************
*/
extern void DispProgVoltCurrent(void);
void PG_PrintPercent(float _Percent, uint32_t _Addr)
{
    #if 1
        if (_Percent == 0)
        {
            printf("  %dms, %0.2f%%\r\n  ", bsp_CheckRunTime(g_tProg.Time), _Percent);
        }
        else if (_Percent == 100)
        {
            //printf("\r\n  %dms, %0.2f%%\r\n", bsp_CheckRunTime(g_tProg.Time), _Percent);
            printf("\r\n");
            printf("  %dms, %0.2f%%\r\n  ", bsp_CheckRunTime(g_tProg.Time), _Percent);
        }
        else
        {
            printf(".");
        }
    #else
        printf("  %dms, %0.2f%%\r\n", bsp_CheckRunTime(g_tProg.Time), _Percent);
    #endif
    if (g_MainStatus == MS_PROG_WORK)
    {       
        g_tProg.Percent = _Percent;
        
        DispProgProgress(0, _Percent, _Addr);   /* 0表示不刷新文本 */
        
        DispProgVoltCurrent();                  /* 刷新TVCC电压和电流 */
    }
    
    bsp_Idle();
}

/*
*********************************************************************************************************
*    函 数 名: PG_FixFlashMem
*    功能说明: 编程过程中修改数据，实现滚码、加密等功能。暂时只支持单通道。
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*               _BuffSize : 数据大小
*               _chan : 通道号0，1-4
*    返 回 值: 0表示地址不在范围内。 1表示在范围内，并已修改好内存
*********************************************************************************************************
*/
uint8_t PG_FixFlashMem(uint32_t _FlashAddr, char *_Buff, uint32_t _BuffSize, uint32_t _FileIndex, uint8_t _chan)
{
    uint16_t m;
    uint16_t i;
    uint8_t change = 0;
    
    for (m = 0; m < g_tFixData.Count; m++)
    {
        if (g_tFixData.Lines[m].Enable == _FileIndex)
        {
            if (g_tFixData.Lines[m].Chan == 0 || _chan == 0 || g_tFixData.Lines[m].Chan == _chan) 
            {
                for (i = 0; i < g_tFixData.Lines[m].Len; i++)
                {
                    if (g_tFixData.Lines[m].Addr + i >= _FlashAddr && g_tFixData.Lines[m].Addr + i < _FlashAddr + _BuffSize)
                    {
                        _Buff[g_tFixData.Lines[m].Addr - _FlashAddr + i] = g_tFixData.Lines[m].pData[i];
                        change = 1;
                    }
                }
            }
        }
    }
    
    return change;
}

/*
*********************************************************************************************************
*    函 数 名: PG_CheckFlashFix
*    功能说明: 判断flash区域是否存在fix区数据
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*               _FileIndex : 文件编号（1-10)
*    返 回 值: 0表示地址不在范围内。 1表示在范围内
*********************************************************************************************************
*/
uint8_t PG_CheckFlashFix(uint32_t _FlashAddr, uint32_t _BuffSize, uint32_t _FileIndex)
{
    uint16_t m;
    uint8_t change = 0;
    
    /*
        add    0000000000     0000000000    0000000000      0000000000
        fix  11111             11111               11111     111111111111
    */
    if (_BuffSize == 0)
    {
        return 0;
    }
    
    for (m = 0; m < g_tFixData.Count; m++)
    {
        if (g_tFixData.Lines[m].Enable == _FileIndex
            && g_tFixData.Lines[m].Addr + g_tFixData.Lines[m].Len > _FlashAddr 
            && g_tFixData.Lines[m].Addr < _FlashAddr + _BuffSize)
        {
            change = 1;
            break;
        }
    }
    return change;
}

/*
*********************************************************************************************************
*    函 数 名: PG_CheckFixSplit
*    功能说明: 判断fix区数据是否需要顺序写入
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*               _FileIndex : 文件编号（1-10)
*    返 回 值: 0表示地址不在范围内。 1表示在范围内,每个通道数据不一致，需要顺序写入
*********************************************************************************************************
*/
uint8_t PG_CheckFixSplit(uint32_t _FlashAddr, uint32_t _BuffSize, uint32_t _FileIndex)
{
    uint16_t m;
    uint8_t change = 0;
    
    /*
        add    0000000000     0000000000    0000000000      0000000000
        fix  11111             11111               11111     111111111111
    */
    
    for (m = 0; m < g_tFixData.Count; m++)
    {
        if (g_tFixData.Lines[m].Enable == _FileIndex
            && g_tFixData.Lines[m].Addr + g_tFixData.Lines[m].Len > _FlashAddr 
            && g_tFixData.Lines[m].Addr < _FlashAddr + _BuffSize)
        {
            if (g_tFixData.Lines[m].Chan >= 2)
            {
                change = 1;
                break;
            }
        }
    }
    return change;
}

/*
*********************************************************************************************************
*    函 数 名: PG_FixDataIsDiff
*    功能说明: 判断FIX数据是否每个通道都一样
*    形    参: 无
*    返 回 值: 0表示相同，1表示不同（需要顺序编程）
*********************************************************************************************************
*/
uint8_t PG_FixDataIsDiff(void)
{
    uint16_t m;
    
    //    --返回C程序用的table，结构为 {
    //    --  0, 0x08000000, "1234567",   0表示全部通道数据一样， 1表示第1通，4表示4通道
    //    --  1, 0x08000200, "AAA",
    //    --  2, 0x08000300, "BBBB",
    //    -- }

    for (m = 0; m < g_tFixData.Count; m++)
    {        
        if (g_tFixData.Lines[m].Chan >= 2)
        {
            return 1;       /* 至少2个通道存在不同的数据 */
        }        
    }
    
    return 0;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
