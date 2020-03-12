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

extern const program_target_t flash_algo;

OFFLINE_PROG_T g_tProg = {0};

PROG_INI_T g_tProgIni = {0};

uint8_t flash_buff[sizeof(FsReadBuf)];

uint8_t PG_LuaUidSnUsr(void);

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
#if 1   /* 由LUA程序提供函数 */ 
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
#else
    uint32_t id;
            
    if (g_tProg.ChipType == CHIP_SWD_ARM)
    {
        sysTickInit();          /* 这是DAP驱动中的初始化函数,全局变量初始化 */
        swd_init_debug();       /* 进入swd debug状态 */
    
        if (swd_read_idcode(&id) == 0)  /* 未检测到 */
        {
            return 1;
        }
        return 0;     
    }
    else if (g_tProg.ChipType == CHIP_SWIM_STM8)
    {
        if (SWIM_DetectIC(&id) == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    } 
    else
    {
        return 1;
    }
#endif    
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
#if 1   /* 由LUA程序提供函数 */ 
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
#else 
    uint32_t id;
            
    //g_tProg.ChipType == CHIP_SWD_ARM)
    {
        sysTickInit();          /* 这是DAP驱动中的初始化函数,全局变量初始化 */
        swd_init_debug();       /* 进入swd debug状态 */
    
        if (swd_read_idcode(&id) > 0)  /* 0 = 出错 */
        {
            return 1;
        }   
    }

    //g_tProg.ChipType == CHIP_SWIM_STM8)
    {
        if (SWIM_DetectIC(&id) > 0)
        {
            return 1;
        }
    } 
    
    return 0;
#endif    
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
    
    PG_LuaUidSnUsr();

    if (g_tProg.ChipType == CHIP_SWD_ARM)
    {
        ;
    }
    else if (g_tProg.ChipType == CHIP_SWIM_STM8)  
    {
        h7swim_ReadLuaVar();        /* 读取LUA中的全局变量 */
    }
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
    
    printf(str); 
    printf("\r\n");
            
    if (g_MainStatus == MS_PROG_WORK)
    {
        DispProgProgress(str, -1, 0xFFFFFFFF);      /* -1表示不刷新进度 */
    }
    
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
    printf("  %dms, %0.2f%%\r\n", bsp_CheckRunTime(g_tProg.Time), _Percent); 
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
*    功能说明: 编程过程中修改数据，填充加密的UID或产品序号。根据lua文件配置。
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*    返 回 值: 0表示地址不在范围内。 1表示在范围内，并已修改好内存
*********************************************************************************************************
*/
uint8_t PG_FixFlashMem(uint32_t _FlashAddr, char *_Buff, uint32_t _BuffSize)
{
    uint16_t i;
    uint8_t change = 0;
    
    /* 产品序号滚码 固定4字节整数 */
    if (g_tProg.SnEnable == 1)
    {
        for (i = 0; i < g_tProg.SnLen; i++)
        {
            if (g_tProg.SnAddr + i >= _FlashAddr && g_tProg.SnAddr + i < _FlashAddr + _BuffSize)
            {
                _Buff[g_tProg.SnAddr - _FlashAddr + i] = g_tProg.SnData[i];
                
                change = 1;
            }
        }
    }  
    
    /* UID加密存储 */
    if (g_tProg.UidEnable == 1)
    {
        for (i = 0; i < g_tProg.UidLen; i++)
        {
            if (g_tProg.UidAddr + i >= _FlashAddr && g_tProg.UidAddr + i < _FlashAddr + _BuffSize)
            {
                _Buff[g_tProg.UidAddr - _FlashAddr + i] = g_tProg.UidData[i];
                change = 1;
            }
        }
    }
   
    /* USR 用户数据存储 */
    if (g_tProg.UsrEnable == 1)
    {
        for (i = 0; i < g_tProg.UsrLen; i++)
        {
            if (g_tProg.UsrAddr + i >= _FlashAddr && g_tProg.UsrAddr + i < _FlashAddr + _BuffSize)
            {
                _Buff[g_tProg.UsrAddr - _FlashAddr + i] = g_tProg.UsrData[i];
                change = 1;
            }
        }
    }  
    
    return change;
}

/*
*********************************************************************************************************
*    函 数 名: PG_CheckFlashMem
*    功能说明: 判断编程过程中是否需要修改数据，填充加密的UID或产品序号。根据lua文件配置。
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*    返 回 值: 0表示地址不在范围内。 1表示在范围内，并已修改好内存
*********************************************************************************************************
*/
uint8_t PG_CheckFlashMem(uint32_t _FlashAddr, char *_Buff, uint32_t _BuffSize)
{
    uint16_t i;
    uint8_t change = 0;
    
    /* 产品序号滚码 固定4字节整数 */
    if (g_tProg.SnEnable == 1)
    {
        for (i = 0; i < g_tProg.UidLen; i++)
        {
            if (g_tProg.UidAddr + i >= _FlashAddr && g_tProg.UidAddr + i < _FlashAddr + _BuffSize)
            {        
                change = 1;
            }
        }
    }  
    
    /* UID加密存储 */
    if (g_tProg.UidEnable == 1)
    {
        for (i = 0; i < g_tProg.UidLen; i++)
        {
            if (g_tProg.UidAddr + i >= _FlashAddr && g_tProg.UidAddr + i < _FlashAddr + _BuffSize)
            {
                change = 1;
            }
        }
    }
   
    /* USR 用户数据存储 */
    if (g_tProg.UsrEnable == 1)
    {
        for (i = 0; i < g_tProg.UsrLen; i++)
        {
            if (g_tProg.UsrAddr + i >= _FlashAddr && g_tProg.UsrAddr + i < _FlashAddr + _BuffSize)
            {
                change = 1;
            }
        }
    }  
    
    return change;
}


/*
*********************************************************************************************************
*    函 数 名: PG_LuaUidSnUsr
*    功能说明: 在编程函数调用。用于动态生成UID,SN,USR数据
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint8_t PG_LuaUidSnUsr(void)
{   
    uint8_t err = 0;
    
    /* 从lua文件中获得变量：是否启用填充uid加密 */
    lua_getglobal(g_Lua, "UID_ENABLE");  
    if (lua_isinteger(g_Lua, -1))
    {
        g_tProg.UidEnable = lua_tointeger(g_Lua, -1);                
        if (g_tProg.UidEnable == 1)
        {
            lua_getglobal(g_Lua, "UID_SAVE_ADDR");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.UidAddr = lua_tointeger(g_Lua, -1);
            }
            else
            {
                err = 1;
            }
            lua_pop(g_Lua, 1);
            if (err == 1)
            {
                PG_PrintText("脚本错误 UID_SAVE_ADDR");                 
                goto err_quit;
            }
            
            lua_getglobal(g_Lua, "UID_DATA");  
            if (lua_isstring(g_Lua, -1)) 
            {
                g_tProg.UidData = lua_tostring(g_Lua, -1);
            }
            else
            {
                err = 1;
            }
            lua_pop(g_Lua, 1);
            if (err == 1)
            {
                PG_PrintText("脚本错误 UID_DATA");                 
                goto err_quit;
            }            
            
            lua_getglobal(g_Lua, "UID_LEN");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.UidLen = lua_tointeger(g_Lua, -1); 
            }
            else
            {
                err = 1;
            }
            lua_pop(g_Lua, 1);
            if (err == 1)
            {
                PG_PrintText("脚本错误 UID_LEN");                
                goto err_quit;
            }             
        }
    }
    else
    {
        g_tProg.UidEnable = 0;
    }
    lua_pop(g_Lua, 1);
    
    /* 从lua文件中获得变量：是否启用填充产品序号 */
    lua_getglobal(g_Lua, "SN_ENABLE");  
    if (lua_isinteger(g_Lua, -1))
    {
        g_tProg.SnEnable = lua_tointeger(g_Lua, -1);        
        if (g_tProg.SnEnable == 1)
        {
            lua_getglobal(g_Lua, "SN_SAVE_ADDR");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.SnAddr = lua_tointeger(g_Lua, -1);
            }
            else
            {
                err = 1;
            }
            lua_pop(g_Lua, 1);
            if (err == 1)
            {
                PG_PrintText("脚本错误 SN_SAVE_ADDR");              
                goto err_quit;
            }
            
            lua_getglobal(g_Lua, "SN_DATA");  
            if (lua_isstring(g_Lua, -1)) 
            {
                g_tProg.SnData = lua_tostring(g_Lua, -1);
            } 
            else
            {
                err = 1;
            } 
            lua_pop(g_Lua, 1);            
            if (err == 1)
            {
                PG_PrintText("脚本错误 SN_DATA");      
                goto err_quit;
            }            

            lua_getglobal(g_Lua, "SN_LEN");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.SnLen = lua_tointeger(g_Lua, -1); 
            }
            else
            {
                err = 1;
            } 
            lua_pop(g_Lua, 1);            
            if (err == 1)
            {
                PG_PrintText("脚本错误 SN_LEN");    
                goto err_quit;
            }            
        }
    }
    else
    {
        g_tProg.SnEnable = 0;
    }  
    lua_pop(g_Lua, 1);    
    
    /* 从lua文件中获得变量：是否启用填充用户定制数据 */
    lua_getglobal(g_Lua, "USR_ENABLE");  
    if (lua_isinteger(g_Lua, -1))
    {
        g_tProg.UsrEnable = lua_tointeger(g_Lua, -1);        
        if (g_tProg.UsrEnable == 1)
        {
            lua_getglobal(g_Lua, "USR_SAVE_ADDR");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.UsrAddr = lua_tointeger(g_Lua, -1);
            } 
            else
            {
                err = 1;
            } 
            lua_pop(g_Lua, 1);            
            if (err == 1)
            {
                PG_PrintText("脚本错误 USR_SAVE_ADDR");  
                goto err_quit;
            }            

            lua_getglobal(g_Lua, "USR_DATA");  
            if (lua_isstring(g_Lua, -1)) 
            {
                g_tProg.UsrData = lua_tostring(g_Lua, -1);
            } 
            else
            {
                err = 1;
            } 
            lua_pop(g_Lua, 1);            
            if (err == 1)
            {
                PG_PrintText("脚本错误 USR_DATA");
                goto err_quit;
            }             

            lua_getglobal(g_Lua, "USR_LEN");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.UsrLen = lua_tointeger(g_Lua, -1); 
            }
            else
            {
                err = 1;
            } 
            lua_pop(g_Lua, 1);            
            if (err == 1)
            {
                PG_PrintText("脚本错误 USR_LEN");
                goto err_quit;
            }             
        }
    }
    else
    {
        g_tProg.UsrEnable = 0;
    }
    lua_pop(g_Lua, 1);
    
    return 0;
    
err_quit:
    return 1;
}
           
/*
*********************************************************************************************************
*    函 数 名: PG_SWD_ProgFile
*    功能说明: SWD接口（STM32) 开始编程flash。 由lua程序调用。阻塞运行，只到编程结束。
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint16_t PG_SWD_ProgFile(char *_Path, uint32_t _FlashAddr)
{
    char path[256];
    uint16_t name_len;
    uint8_t err = 0;
    char ext_name[5];
    error_t err_t;
    uint32_t FileLen;
    uint8_t EraseChipEnable = 0;
    uint8_t fBlankChip = 0;
    
    /* 传入的文件名是相对路径 */
    if (strlen(_Path) + strlen(PROG_USER_DIR) > sizeof(path))
    {
        PG_PrintText("文件路径太长"); 
        err = 1;
        goto quit;        
    }
    
    if (_Path[0] == '0' && _Path[1] == ':')         /* 是绝对路径 */
    {
        strncpy(path, _Path, sizeof(path));
    }
    else    /* 是相对路路径 */ 
    {
        GetDirOfFileName(g_tProg.FilePath, path);   /* 从lua文件名、中获取目录 */
        strcat(path, "/");
        strcat(path, _Path);

        FixFileName(path);  /* 去掉路径中的..上级目录 */
    }
    
    /* 解析文件名 */
    name_len = strlen(_Path);
    if (name_len < 5)
    {      
        PG_PrintText("数据文件名过短 "); 
        err = 1;
        goto quit;
    }  
    
    /* 文件仅支持 bin */
    memcpy(ext_name, &_Path[name_len - 4], 5);
    strlwr(ext_name);   /* 转换为小写 */
    if (strcmp(ext_name, ".bin") == 0)
    {
        ;
    }
    else
    {
        PG_PrintText("数据文件格式错误");
        err = 1;
        goto quit;
    }
    
    /* 从lua文件中获得变量：是否整片擦除 */
    lua_getglobal(g_Lua, "ERASE_CHIP_ENABLE");  
    if (lua_isinteger(g_Lua, -1))
    {
        EraseChipEnable = lua_tointeger(g_Lua, -1);
        
        if (flash_algo.erase_chip == 0)     /* 算法中没有erase_chip函数 */
        {
            EraseChipEnable = 0;    /* 只能按扇区擦除 */
        }
    }
    else
    {
        EraseChipEnable = 0;
    }
    lua_pop(g_Lua, 1);           
        
	if (swd_init_debug() == 0)
    {
        PG_PrintText("SWD初始化失败！");        
        err = 1;
        goto quit;        
    }
    
	err_t = target_flash_init(_FlashAddr);
    if (err_t == ERROR_RESET)
    {
        PG_PrintText("复位目标MCU失败");
        err = 1;
        goto quit;        

    }
    else if (err_t == ERROR_ALGO_DL)
    {
        PG_PrintText("下载算法失败");        
        err = 1;
        goto quit;        
    }
    else if (err_t == ERROR_INIT)
    {
        PG_PrintText("执行init算法失败");        
        err = 1;
        goto quit;        
    }
        
    FileLen = GetFileSize(path);
    if (FileLen == 0)
    {
        PG_PrintText("读取数据文件失败");        
        err = 1;
        goto quit;         
    }  
    if (FileLen > g_tFLM.Device.szDev)
    {
        PG_PrintText("数据文件过大");    
        err = 1;
        goto quit; 
    }
	
	if (swd_read_memory(_FlashAddr, flash_buff, 4) == 0)
	{
        PG_PrintText("读flash数据失败");        
        err = 1;
        goto quit;  		
	}
    
    /* 动态生成加密UID, 产品序号，USR用户数据 */
    if (PG_LuaUidSnUsr() != 0)
    {
        err = 1;
        goto quit;         
    }

    if (EraseChipEnable == 2)
    {
        fBlankChip = 0; /* 不检查空片，直接整片擦除 */
    }
    else
    {
        /* 空片检查 */
        PG_PrintText("正在检查空片 ");  
        PG_PrintPercent(0, _FlashAddr);    
        {
            uint32_t addr;
            uint32_t FileOffset = 0;
            uint32_t bytes;
            uint32_t i, j;
            uint8_t en;
               
            bytes = FileLen;
            if (bytes > sizeof(flash_buff))
            {
                bytes = sizeof(flash_buff);
            }
                
            fBlankChip = 1;
            if (flash_algo.check_blank > 0)
            {
                if (target_flash_check_blank(_FlashAddr,  FileLen) == 0)
                {
                    fBlankChip = 0;     /* 0表示不空 */
                }
                PG_PrintPercent(100, _FlashAddr); 
            }
            else
            {
                addr = _FlashAddr - g_tFLM.Device.DevAdr;   /* 求相对地址, 方便后面计算 */
                for (; FileOffset < FileLen; )
                {
                    if (ProgCancelKey())
                    {
                        PG_PrintText("用户终止运行");    
                        err = 1;
                        goto quit;                
                    }	                   
                    
                    err = 0;
                    if (g_tFLM.Device.DevType ==  EXTSPI)
                    {
                        if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                        {
                            err = 1;
                        }
                    }
                    else
                    {
                        if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                        {
                            err = 1;
                        }
                    }
                    
                    if (err == 1)
                    {
                        PG_PrintText("swd_read_memory error");    
                        err = 1;
                        goto quit;                 
                    }
                    
                    for (i = 0; i < bytes; i++)
                    {
                        if (flash_buff[i] != g_tFLM.Device.valEmpty)
                        {
                            fBlankChip = 0;
                            break;
                        }
                    }
                    if (fBlankChip == 0)
                    {
                        break;
                    }
                           
                    addr += bytes;
                    FileOffset += bytes;
                    
                    /* 进度指示 */
                    {
                        float percent = -1;
                        
                        percent = ((float)addr / FileLen) * 100;    
                        if (percent > 100)
                        {
                            percent = 100;
                        }
                        PG_PrintPercent(percent, g_tFLM.Device.DevAdr + addr);           
                    }
                }
            }
            
            /* 检查 UID, SN, USR flash地址是否空*/        
            for (j = 0; j < 3; j++)
            {
                if (j == 0)
                {
                    addr = g_tProg.SnAddr;
                    bytes = g_tProg.SnLen;
                    en = g_tProg.SnEnable;
                }
                else if (j == 1)
                {
                    addr = g_tProg.UidAddr;
                    bytes = g_tProg.UidLen;
                    en = g_tProg.UidEnable;
                }           
                else if (j == 2)
                {
                    addr = g_tProg.UsrAddr;
                    bytes = g_tProg.UsrLen;
                    en = g_tProg.UsrEnable;
                }           
                
                if (en == 1)
                {
                    if (swd_read_memory(addr, flash_buff, bytes) == 0)
                    {
                        PG_PrintText("swd_read_memory error");    
                        err = 1;
                        goto quit;                 
                    }
                    for (i = 0; i < bytes; i++)
                    {
                        if (flash_buff[i] != g_tFLM.Device.valEmpty)
                        {
                            fBlankChip = 0;
                            break;
                        }
                    }
                    if (fBlankChip == 0)
                    {
                        break;
                    } 
                }
            }
        }
    }
    
    /* 擦除 */
    if (EraseChipEnable == 0)   /* 按扇区擦除 */
    {
        if (fBlankChip == 0)    /* 不是空片才进行擦除 */
        {
            uint32_t j;
            uint32_t addr;
            uint32_t FinishedSize = 0;
            uint8_t fEraseReq = 0;
            
            PG_PrintText("正在擦除扇区...");    
            PG_PrintPercent(0, 0xFFFFFFFF);  
            bsp_Idle();
            
            /* 遍历整个flash空间，决定哪个扇区需要擦除 */
            addr = g_tFLM.Device.DevAdr; 
            j = 0;            
            while (addr < g_tFLM.Device.DevAdr + g_tFLM.Device.szDev)
            {
                if (ProgCancelKey())
                {
                    PG_PrintText("用户终止运行");    
                    err = 1;
                    goto quit;                
                }
                
                fEraseReq = 0;
                
                /* 判断数据文件的目标扇区是否需要擦除 */
                if (_FlashAddr + FileLen >= addr && _FlashAddr < addr + g_tFLM.Device.sectors[j].szSector)
                {
                    fEraseReq = 1;   /* 需要擦除 */
                    
                    FinishedSize += g_tFLM.Device.sectors[j].szSector;  /* 已擦除的字节数 */
                }
                
                /* 判断SN的目标扇区是否需要擦除 */
                if (g_tProg.SnEnable == 1)
                {
                    if (g_tProg.SnAddr + g_tProg.SnLen >= addr && g_tProg.SnAddr < addr + g_tFLM.Device.sectors[j].szSector)
                    {
                        fEraseReq = 1;   /* 需要擦除 */
                    }
                }

                /* 判断UID的目标扇区是否需要擦除 */
                if (g_tProg.UidEnable == 1)
                {
                    if (g_tProg.UidAddr + g_tProg.UidLen >= addr && g_tProg.UidAddr < addr + g_tFLM.Device.sectors[j].szSector)
                    {
                        fEraseReq = 1;   /* 需要擦除 */
                    }
                }

                /* 判断USR的目标扇区是否需要擦除 */
                if (g_tProg.UsrEnable == 1)
                {
                    if (g_tProg.UsrAddr + g_tProg.UsrLen >= addr && g_tProg.UsrAddr < addr + g_tFLM.Device.sectors[j].szSector)
                    {
                        fEraseReq = 1;   /* 需要擦除 */
                    }
                }                
                    
                if (fEraseReq == 1)
                {              
                    if (target_flash_erase_sector(addr) != 0)
                    {
                        PG_PrintText("扇区擦除失败");
                        err = 1;
                        goto quit;
                    }

                    /* 擦除成功，进度指示。 STML051，扇区只有128字节，4ms一个扇区。显示进度太占用时间。 */
                    {
                        float percent;
                        static int32_t s_time1 = 0;
                                                
                        percent = ((float)FinishedSize / FileLen) * 100; 
                        if (percent >= 100)
                        {
                            percent = 100;
                            s_time1 = 0;    /* 强制显示1次 */
                        }
                        /* 控制一下显示间隔，最快100ms */
                        if (bsp_CheckRunTime(s_time1) > 100)
                        {
                            PG_PrintPercent(percent, addr);
                            s_time1 = bsp_GetRunTime();
                        }
                    }                     
                }
                
                /* 下一个扇区 */
                addr += g_tFLM.Device.sectors[j].szSector;
                if (g_tFLM.Device.sectors[j + 1].AddrSector == 0xFFFFFFFF)
                {
                    ;
                }
                else if (addr >= g_tFLM.Device.sectors[j + 1].AddrSector + g_tFLM.Device.DevAdr)
                {
                    j++;
                }               
            }
        }
    }
    else    /* 整片擦除 */
    {
        if (fBlankChip == 0)     /* 不是空片才进行擦除 */
        {            
            /* armfly : 根据擦除扇区的超时估算整片擦除时间 */
            g_tProg.FLMEraseChipFlag = 1;                          

            PG_PrintText("正在擦除整片...");    
            PG_PrintPercent(0, 0xFFFFFFFF);   
            bsp_Idle();
            
            {   
                if (g_tProg.EraseChipTime1 == 0)    /* 记录第一次全片擦除开始时刻 */
                {
                    g_tProg.EraseChipTime1 = bsp_GetRunTime();            
                }
                if (target_flash_erase_chip() != 0)
                {
                    PG_PrintText("整片擦除失败");        
                    err = 1;
                    g_tProg.FLMEraseChipFlag = 0; 
                    goto quit;
                }
                if (g_tProg.EraseChipTime2 == 0)    /* 记录第一次全片擦除完成时刻 */
                {
                    g_tProg.EraseChipTime2 = bsp_GetRunTime();
                    
                    /* 准备保存到ini文件 */
                    g_tProgIni.LastEraseChipTime = bsp_CheckRunTime(g_tProg.EraseChipTime1);
                }
            } 
            
            g_tProg.FLMEraseChipFlag = 0;            
        }
    }

    /* 编程page */
    PG_PrintText("正在编程...");    
    PG_PrintPercent(0, 0xFFFFFFFF);    
    {
        uint32_t addr;
		uint32_t FileOffset = 0;
		uint16_t PageSize;
		uint32_t bytes;
        uint32_t i;
        uint32_t j;
        uint32_t FileBuffSize; 
        float percent = 0;          
   
		PageSize = g_tFLM.Device.szPage;
		if (PageSize > sizeof(FsReadBuf))
		{
			PageSize = sizeof(FsReadBuf);
		}
		
        /* 整定文件缓冲区大小为PageSize的整数倍 */
        FileBuffSize = sizeof(FsReadBuf);   
        FileBuffSize = (FileBuffSize / PageSize) * PageSize;
        
        addr = g_tFLM.Device.DevAdr;   /* 求相对地址, 方便后面计算 */
        for (j = 0; j < (g_tFLM.Device.szDev + sizeof(FsReadBuf) - 1) / sizeof(FsReadBuf); j++)
        {
            if (ProgCancelKey())
            {
                PG_PrintText("用户终止运行");    
                err = 1;
                goto quit;                
            }
            
            /* 读文件, 按最大缓冲区读取到内存 */
            if (_FlashAddr + FileLen > addr && _FlashAddr < addr + sizeof(FsReadBuf))
            {
                bytes = ReadFileToMem(path, FileOffset, FsReadBuf, sizeof(FsReadBuf));   
                
                if (bytes != sizeof(FsReadBuf))
                {
                    if (FileOffset + sizeof(FsReadBuf) < FileLen)
                    {
                        PG_PrintText("读取数据文件失败");        
                        err = 1;
                        goto quit;
                    }
                    
                    if (bytes < PageSize)
                    {
                        memset(&FsReadBuf[bytes], g_tFLM.Device.valEmpty ,PageSize - bytes);
                        
                        bytes = PageSize;
                    }
                }
                
                /* 修改缓冲区，填充UID加密数据或产品序号 */
                PG_FixFlashMem(addr, FsReadBuf, bytes); 
                
                for (i = 0; i < bytes / PageSize; i++)
                {                                
                    if (target_flash_program_page(addr, (uint8_t *)&FsReadBuf[i * PageSize], PageSize) != 0)
                    {
                        {
                            char buf[128];
                            
                            sprintf(buf, "编程失败, 0x%08X", addr);
                            PG_PrintText(buf); 
                        }                              
                        err = 1;
                        goto quit;
                    }
                       
                    addr += PageSize;
                    FileOffset += PageSize;
                }  
                /* 进度指示 */
                {
                    if (percent < 100)
                    {        
                        percent = ((float)FileOffset / FileLen) * 100;
                        if (percent > 100)
                        {
                            percent = 100;
                        }                        
                        PG_PrintPercent(percent, addr);                   
                    }
                }                
            }
            else /* 文件以外的page */
            {
                bytes = sizeof(FsReadBuf);
                if (PG_CheckFlashMem(addr, FsReadBuf, bytes))
                {
                    memset(FsReadBuf, g_tFLM.Device.valEmpty, sizeof(FsReadBuf));
                    
                    for (i = 0; i < bytes / PageSize; i++)
                    {
                        /* 修改缓冲区，填充UID加密数据或产品序号 */
                        if (PG_FixFlashMem(addr, &FsReadBuf[i * PageSize], PageSize) == 1)
                        {                        
                            if (target_flash_program_page(addr, (uint8_t *)&FsReadBuf[i * PageSize], PageSize) != 0)
                            {
                                {
                                    char buf[128];
                                    
                                    sprintf(buf, "编程失败, 0x%08X", addr);
                                    PG_PrintText(buf); 
                                }        
                                err = 1;
                                goto quit;
                            }
                        }
                        addr += PageSize;
                    }                     
                }
                else
                {
                    addr += bytes;
                }
            }            
        }
    }
    
    /* 校验 */
    PG_PrintText("正在校验...");    
    PG_PrintPercent(0, 0xFFFFFFFF);    
    {
        uint32_t addr;
		uint32_t FileOffset = 0;
		uint16_t PageSize;
		uint32_t bytes;
   
        PageSize = sizeof(flash_buff);
		if (PageSize > sizeof(flash_buff))
		{
			PageSize = sizeof(flash_buff);
		}
        
        if (PageSize >= sizeof(FsReadBuf))
        {
            PageSize = sizeof(FsReadBuf);
        }
		
        addr = _FlashAddr - g_tFLM.Device.DevAdr;   /* 求相对地址, 方便后面计算 */
        for (; FileOffset < FileLen; )
        {
            if (ProgCancelKey())
            {
                PG_PrintText("用户终止运行");    
                err = 1;
                goto quit;                
            }
            
			bytes = ReadFileToMem(path, FileOffset, FsReadBuf, PageSize); 
			if (bytes != PageSize)
			{
                if (FileOffset + PageSize < FileLen)
                {
                    PG_PrintText("读取数据文件失败");        
                    err = 1;
                    goto quit;
                }
			}		
			
            /* 修改缓冲区，填充UID加密数据或产品序号 */
            PG_FixFlashMem(g_tFLM.Device.DevAdr + addr, FsReadBuf, PageSize);


            if (flash_algo.cacul_crc32 > 0)
            {
                uint32_t crc1, crc2;
                
                crc1 = target_flash_cacul_crc32(g_tFLM.Device.DevAdr + addr, bytes, 0xFFFFFFFF);
                crc2 = STM32_CRC32((uint32_t *)FsReadBuf, bytes);
                
                if (crc1 != crc2)
                {              
                    PG_PrintText("CRC校验失败");        
                    err = 1;
                    goto quit;	                    
                }
                addr += PageSize;
                FileOffset += PageSize;                  
            }
            else
            {
                /* 读回进行校验 */                    
                if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                {
                    char buf[128];
                    
                    sprintf(buf, "swd_read_memory error, addr = %X, len = %X", g_tFLM.Device.DevAdr + addr, bytes);
                    PG_PrintText(buf);    
                    err = 1;
                    goto quit;  
                }
                    
                if (memcmp(FsReadBuf, flash_buff, bytes) != 0)
                {
                    {
                        char buf[128];
                        
                        sprintf(buf, "校验失败, 0x%08X", g_tFLM.Device.DevAdr + addr);
                        PG_PrintText(buf); 
                    }                     
                    err = 1;
                    goto quit;				
                }  
                addr += PageSize;
                FileOffset += PageSize;                
            }            

            /* 进度指示 */
            {
                float percent = -1;
                
                percent = ((float)FileOffset / FileLen) * 100;
                if (percent > 100)
                {
                    percent = 100;
                }                
                PG_PrintPercent(percent, g_tFLM.Device.DevAdr + addr);                
            }
        }
    } 
quit:
    return err;
}

/*
*********************************************************************************************************
*    函 数 名: PG_SWIM_ProgFile
*    功能说明: SWIM接口（STM8) 开始编程flash。 由lua程序调用。阻塞运行，只到编程结束。
*    形    参:  _Path : 文件名
*               _FlashAddr : flash起始地址
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint16_t PG_SWIM_ProgFile(char *_Path, uint32_t _FlashAddr)
{
    char path[256];
    uint16_t name_len;
    uint8_t err = 0;
    char ext_name[5];
    uint32_t FileLen;
//    uint8_t EraseChipEnable = 0;
//    uint8_t fBlankChip = 0;
//    uint32_t BlockSize;
    uint8_t FlashProgFlag = 0;
        
    if (_FlashAddr >= 0x008000)
    {
        FlashProgFlag = 1;      /* Flash地址 */
    }
    else
    {
        FlashProgFlag = 0;      /* EEPROM地址 */
    }
    
    /* 传入的文件名是相对路径 */
    if (strlen(_Path) + strlen(PROG_USER_DIR) > sizeof(path))
    {
        PG_PrintText("文件路径太长"); 
        err = 1;
        goto quit;        
    }
    
    if (_Path[0] == '0' && _Path[1] == ':')     /* 是绝对路径 */
    {
        strncpy(path, _Path, sizeof(path));
    }
    else    /* 是相对路路径 */ 
    {    
        GetDirOfFileName(g_tProg.FilePath, path);   /* 从lua文件名、中获取目录 */
        strcat(path, "/");
        strcat(path, _Path);

        FixFileName(path);  /* 去掉路径中的..上级目录 */
    }
    
    /* 解析文件名 */
    name_len = strlen(_Path);
    if (name_len < 5)
    {      
        PG_PrintText("数据文件名过短 "); 
        err = 1;
        goto quit;
    }  
    
    /* 文件支持 1.hex,  2.bin */
    memcpy(ext_name, &_Path[name_len - 4], 5);
    strlwr(ext_name);   /* 转换为小写 */
    if (strcmp(ext_name, ".hex") == 0)
    {
        ;
    }
    else if (strcmp(ext_name, ".bin") == 0)
    {
        ;
    }
    else
    {
        PG_PrintText("数据文件扩展名不正确");
        err = 1;
        goto quit;
    }
    
//    /* 从lua文件中获得变量：是否整片擦除 */
//    lua_getglobal(g_Lua, "ERASE_CHIP_ENABLE");  
//    if (lua_isinteger(g_Lua, -1))
//    {
//        EraseChipEnable = lua_tointeger(g_Lua, -1);
//    }
//    else
//    {
//        EraseChipEnable = 0;
//    }
//    lua_pop(g_Lua, 1);
        
    FileLen = GetFileSize(path);
    if (FileLen == 0)
    {
        PG_PrintText("读取数据文件失败");        
        err = 1;
        goto quit;         
    }  
    
    {
        uint32_t DevSize;
        
        if (_FlashAddr >= 0x08000)
        {
            DevSize = s_STM8_FlashSize;
        }
        else
        {
            DevSize = s_STM8_EEPromSize;
        }
    
        if (FileLen > DevSize)
        {
            PG_PrintText("数据文件过大");    
            err = 1;
            goto quit; 
        }
    }
    
    /* 动态生成加密UID, 产品序号，USR用户数据 */
    if (PG_LuaUidSnUsr() != 0)
    {
        err = 1;
        goto quit;         
    }

    /* 编程page */
    PG_PrintText("正在编程...");    
    PG_PrintPercent(0, 0xFFFFFFFF);    
    {
        uint32_t addr;
		uint32_t FileOffset = 0;
		uint16_t PageSize;
		uint32_t bytes;
        uint32_t i;
        uint32_t j;
        uint32_t FileBuffSize;  
        uint32_t DeviceSize;
   
		PageSize = s_STM8_BlockSize;
		if (PageSize > sizeof(FsReadBuf))
		{
			PageSize = sizeof(FsReadBuf);
		}
		
        /* 整定文件缓冲区大小为PageSize的整数倍 */
        FileBuffSize = sizeof(FsReadBuf);   
        FileBuffSize = (FileBuffSize / PageSize) * PageSize;
        
        if (FlashProgFlag == 1)
        {
            addr = 0x008000;    /* Flash地址 */
            DeviceSize = s_STM8_FlashSize;
        }
        else
        {
            if (s_STM8_SerialType == STM8L)
            {
                addr = 0x001000;    /* EEPROM地址 */
            }
            else
            {
                addr = 0x004000;    /* EEPROM地址 */
            }
            DeviceSize = s_STM8_EEPromSize;
        }
        for (j = 0; j < (DeviceSize + sizeof(FsReadBuf) - 1) / sizeof(FsReadBuf); j++)
        {
            if (ProgCancelKey())
            {
                PG_PrintText("用户终止运行");    
                err = 1;
                goto quit;                
            }
            
            /* 读文件, 按最大缓冲区读取到内存 */
            if (_FlashAddr + FileLen > addr && _FlashAddr < addr + sizeof(FsReadBuf))
            {
                bytes = ReadFileToMem(path, FileOffset, FsReadBuf, sizeof(FsReadBuf));   
                
                if (bytes != sizeof(FsReadBuf))
                {
                    if (FileOffset + sizeof(FsReadBuf) < FileLen)
                    {
                        PG_PrintText("读取数据文件失败");        
                        err = 1;
                        goto quit;
                    }
                    
                    if (bytes < PageSize)
                    {
                        memset(&FsReadBuf[bytes], 0x00 ,PageSize - bytes);      /* 填充空值 00 */
                        
                        bytes = PageSize;
                    }
                }
                
                /* 修改缓冲区，填充UID加密数据或产品序号 */
                PG_FixFlashMem(addr, FsReadBuf, bytes); 
                
                for (i = 0; i < bytes / PageSize; i++)
                {
                    STM8_FLASH_Unlock();
                    
                    if (STM8_FLASH_ProgramBuf(addr, (uint8_t *)&FsReadBuf[i * PageSize], PageSize) == 0)
                    {
                        PG_PrintText("编程失败");        
                        err = 1;
                        goto quit;
                    }
                       
                    addr += PageSize;
                    FileOffset += PageSize;
                    
                            
                    /* 进度指示 - 每2KB刷新一次 */
                    if ((FileOffset % 2048) == 0)
                    {
                        float percent;
                        
                        percent = ((float)FileOffset / FileLen) * 100;
                        if (percent > 100)
                        {
                            percent = 100;
                        }                        
                        PG_PrintPercent(percent, addr);           
                    }                    
                }                  
            }
            else /* 文件以外的page */
            {
                bytes = sizeof(FsReadBuf);
                if (PG_CheckFlashMem(addr, FsReadBuf, bytes))
                {
                    memset(FsReadBuf, 0x00, sizeof(FsReadBuf));       /* 填充空值 00 */
                    
                    for (i = 0; i < bytes / PageSize; i++)
                    {
                        /* 修改缓冲区，填充UID加密数据或产品序号 */
                        if (PG_FixFlashMem(addr, &FsReadBuf[i * PageSize], PageSize) == 1)
                        {                        
                            if (STM8_FLASH_ProgramBuf(addr, (uint8_t *)&FsReadBuf[i * PageSize], PageSize) == 0)
                            {
                                PG_PrintText("编程失败");        
                                err = 1;
                                goto quit;
                            }
                        }
                        addr += PageSize;
                    }                     
                }
                else
                {
                    addr += bytes;
                }
            }
        }
    }
    
    /* 校验 */
    PG_PrintText("正在校验...");    
    PG_PrintPercent(0, 0xFFFFFFFF);    
    {
        uint32_t addr;
		uint32_t FileOffset = 0;
		uint16_t PageSize;
		uint32_t bytes;
        uint32_t DeviceSize;
   
        if (FlashProgFlag == 1)
        {
            addr = _FlashAddr;    /* Flash地址 */
            DeviceSize = s_STM8_FlashSize;
        }
        else
        {
            addr = _FlashAddr;    /* EEPROM地址 */
            DeviceSize = s_STM8_EEPromSize;
        }

        PageSize = sizeof(flash_buff);
		if (PageSize > sizeof(flash_buff))
		{
			PageSize = sizeof(flash_buff);
		}
        
        if (PageSize >= sizeof(FsReadBuf))
        {
            PageSize = sizeof(FsReadBuf);
        }
		
        if (PageSize >= DeviceSize)
        {
            PageSize = DeviceSize;
        }
            
        for (; FileOffset < FileLen; )
        {
            if (ProgCancelKey())
            {
                PG_PrintText("用户终止运行");    
                err = 1;
                goto quit;                
            }
            
			bytes = ReadFileToMem(path, FileOffset, FsReadBuf, PageSize); 
			if (bytes != PageSize)
			{
                if (FileOffset + PageSize < FileLen)
                {
                    PG_PrintText("读取数据文件失败");        
                    err = 1;
                    goto quit;
                }
			}		
			
            /* 修改缓冲区，填充UID加密数据或产品序号 */
            PG_FixFlashMem(addr, FsReadBuf, PageSize);

            {
                /* 读回进行校验 */
                STM8_FLASH_ReadBuf(addr, flash_buff, bytes);			
                if (memcmp(FsReadBuf, flash_buff, bytes) != 0)
                {
                    PG_PrintText("校验数据失败");        
                    err = 1;
                    goto quit;				
                }  
                addr += PageSize;
                FileOffset += PageSize;                
            }            

            /* 进度指示 */
            {
                float percent = -1;
                
                percent = ((float)FileOffset / FileLen) * 100;
                if (percent > 100)
                {
                    percent = 100;
                }                
                PG_PrintPercent(percent, addr);           
            }
        }
    } 
quit:
    return err;
}

/*
*********************************************************************************************************
*    函 数 名: PG_SWD_ProgBuf
*    功能说明: 开始编程flash。 读修改写，限制在一个page内
*    形    参:  _FlashAddr
*				_DataBuf : 数据buf
*               _BufLen : 数据长度
*               _Mode : 编程模式 0表示读回修改再写  1表示擦除所在扇区再写入
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint16_t PG_SWD_ProgBuf(uint32_t _FlashAddr, uint8_t *_DataBuf, uint32_t _BufLen, uint8_t _Mode)
{
    uint8_t err = 0;
    error_t err_t;
    uint32_t FileLen;     
    uint32_t bytes;    
    
    FileLen = _BufLen;
    
	if (swd_init_debug() == 0)
    {
        PG_PrintText("SWD初始化失败！");        
        err = 1;
        goto quit;        
    }
	
	err_t = target_flash_init(_FlashAddr);
    if (err_t == ERROR_RESET)
    {
        PG_PrintText("复位目标MCU失败");
        err = 1;
        goto quit;        

    }
    else if (err_t == ERROR_ALGO_DL)
    {
        PG_PrintText("下载算法失败");        
        err = 1;
        goto quit;        
    }
    else if (err_t == ERROR_INIT)
    {
        PG_PrintText("执行算法失败");        
        err = 1;
        goto quit;        
    } 
    
    /* 判断参数是否合法 */
    if (_FlashAddr >= g_tFLM.Device.DevAdr && _FlashAddr + _BufLen <= g_tFLM.Device.DevAdr + g_tFLM.Device.szDev)
    {
        ;
    }
    else
    {
        PG_PrintText("数据文件长度超过芯片容量");        
        err = 1;
        goto quit;             
    }    

    /* 循环执行：读回比对、查空、擦除、编程page、比对 */   
    PG_PrintPercent(0, 0xFFFFFFFF);    
    {
        uint32_t addr;
		uint32_t FileOffset = 0;
		uint16_t PageSize;
        uint32_t PageStartAddr = 0;
   
		PageSize = g_tFLM.Device.szPage;
		if (PageSize > sizeof(flash_buff))
		{
			PageSize = sizeof(flash_buff);
		}
		
        addr = _FlashAddr - g_tFLM.Device.DevAdr;   /* 求相对地址, 方便后面计算 */
        for (; FileOffset < FileLen; )
        {        
            bytes = PageSize;
            if (FileLen < bytes)
            {
                bytes = FileLen;
            }
            
            /* page起始地址 */
            PageSize = g_tFLM.Device.szPage;
            PageStartAddr = (addr / PageSize) * PageSize;
            if (PageSize > sizeof(flash_buff))
            {
                PG_PrintText("page size 过大");        
                err = 1;
                goto quit;
            }
            
            if (_Mode == 2) /* 擦除所在扇区后，写入，其他数据会清空 */
            {
                //PG_PrintText("正在擦除...");   
                
                /*　开始擦除 - STM32F429 包含解除读保护 */
                if (target_flash_erase_chip() != 0)
                {
                    PG_PrintText("整片擦除失败");        
                    err = 1;
                    goto quit;
                }      

                /* STM32F103 option bytes 编程时需要执行 erase_sector */
                if (target_flash_erase_sector(g_tFLM.Device.DevAdr + PageStartAddr) != 0)
                {
                    PG_PrintText("扇区擦除失败");        
                    err = 1;
                    goto quit;
                } 
                
                /* 未判断返回值。写STM32F103 OPTION BYTES会返回错误, 但是写入已经成功 */
                //PG_PrintText("正在编程..."); 
                if (target_flash_program_page(g_tFLM.Device.DevAdr + PageStartAddr, _DataBuf, PageSize) != 0)
                {
                    PG_PrintText("program_page failed");        
                    err = 1;
                    goto quit;                    
                }
                
                /*  */
                //PG_PrintText("正在校验..."); 
                if (flash_algo.verify > 0)
                {
                    if (target_flash_verify_page(g_tFLM.Device.DevAdr + PageStartAddr, _DataBuf, PageSize) != 0)
                    {
                        PG_PrintText("校验数据失败");        
                        err = 1;
                        goto quit;                    
                    }
                }
                else
                {	
                    /* 读回进行校验 */
                    if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                    {
                        PG_PrintText("swd_read_memory error");        
                        err = 1;
                        goto quit;				
                    }                     
                    if (memcmp((uint8_t *)&_DataBuf[FileOffset], flash_buff, bytes) != 0)
                    {
                        PG_PrintText("校验数据失败");        
                        err = 1;
                        goto quit;				
                    }  
                }
            }
            else if (_Mode == 1) /* 擦除所在扇区后，写入，其他数据会清空 */
            {
//                PG_PrintText("正在擦除扇区..."); 
                
                /*　开始擦除扇区 */
                if (target_flash_erase_sector(g_tFLM.Device.DevAdr + PageStartAddr) != 0)
                {
                    PG_PrintText("扇区擦除失败");        
                    err = 1;
                    goto quit;
                }         

                swd_read_memory(PageStartAddr + g_tFLM.Device.DevAdr, flash_buff, PageSize);
                
                memcpy(&flash_buff[addr % PageSize], (uint8_t *)&_DataBuf[FileOffset], bytes);               
                
//                PG_PrintText("正在编程..."); 
                /* 整页编程 */
                if (target_flash_program_page(g_tFLM.Device.DevAdr + PageStartAddr, flash_buff, PageSize) != 0)
                {
                    PG_PrintText("编程失败");        
                    err = 1;
                    goto quit;
                }

                memset(flash_buff, 0xFF, PageSize);
                
                /* 读回进行校验 */
                if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                {
                    PG_PrintText("swd_read_memory error");        
                    err = 1;
                    goto quit;				
                }                
                if (memcmp((uint8_t *)&_DataBuf[FileOffset], flash_buff, bytes) != 0)
                {
                    PG_PrintText("校验数据失败");        
                    err = 1;
                    goto quit;				
                }                                 
            }
            else if (_Mode == 0)    /* 读 - 修改 - 写，同page内其他数据保持不变 */
            {
                swd_read_memory(PageStartAddr + g_tFLM.Device.DevAdr, flash_buff, PageSize);        /* 读取一个page  */
                
                /* 如果内容相同则不处理 */
                if (memcmp(&flash_buff[addr % PageSize], (uint8_t *)&_DataBuf[FileOffset], bytes) != 0)
                {
                    /* 如果不是空值就擦除 */
                    //if (CheckBlankBuf((char *)&flash_buff[addr % PageSize], bytes, g_tFLM.Device.valEmpty) == 0)  
                    {
                        /*　开始擦除 */
                        if (target_flash_erase_sector(g_tFLM.Device.DevAdr + PageStartAddr) != 0)
                        {
                            PG_PrintText("扇区擦除失败");        
                            err = 1;
                            goto quit;
                        }
                        
                        //swd_read_memory(PageStartAddr + g_tFLM.Device.DevAdr, flash_buff, PageSize);
                    }                                
                    
                    memcpy(&flash_buff[addr % PageSize], (uint8_t *)&_DataBuf[FileOffset], bytes);                   
                    
                    /* 整页编程 */
                    if (target_flash_program_page(g_tFLM.Device.DevAdr + PageStartAddr, flash_buff, PageSize) != 0)
                    {
                        PG_PrintText("编程失败");        
                        err = 1;
                        goto quit;
                    }
                    
                    memset(flash_buff, 0xFF, PageSize);
                    
                    /* 读回进行校验 */
                    if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                    {
                        PG_PrintText("swd_read_memory error");        
                        err = 1;
                        goto quit;				
                    }   
                  
                    if (memcmp((uint8_t *)&_DataBuf[FileOffset], flash_buff, bytes) != 0)
                    {
                        PG_PrintText("校验数据失败");        
                        err = 1;
                        goto quit;				
                    }            
                }
            }
            
            addr += PageSize;
			FileOffset += PageSize;
            
            /* 进度指示 */
            {
                float percent;
                
                percent = ((float)addr / FileLen) * 100;
                if (percent > 100)
                {
                    percent = 100;
                }                
                PG_PrintPercent(percent, g_tFLM.Device.DevAdr + addr);           
            }
        }
    }
	
//	swd_set_target_state_hw(RUN);
quit:
    return err;
}

/*
*********************************************************************************************************
*    函 数 名: PG_SWD_ProgBuf_OB
*    功能说明: 开始编程option 。解锁读保护时，擦除或编程时间可能很长（比如20秒）。
*               本函数会处理执行期间的进度
*               显示
*    形    参:  _FlashAddr
*				_DataBuf : 数据buf
*               _BufLen : 数据长度
*               _Mode : 
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint16_t PG_SWD_ProgBuf_OB(uint32_t _FlashAddr, uint8_t *_DataBuf, uint32_t _BufLen)
{
    uint8_t err = 0;
    
    /* armfly : 根据擦除扇区的超时估算整片擦除时间 */
    g_tProg.FLMEraseChipFlag = 1;                
    
    err = PG_SWD_ProgBuf(_FlashAddr, _DataBuf, _BufLen, 2);
    
    g_tProg.FLMEraseChipFlag = 0;
    
    return err;
}

/*
*********************************************************************************************************
*    函 数 名: PG_SWD_EraseChip
*    功能说明: 开始擦除全片. 对于STM32 OPTION BYTES 会进行解除读保护操作
*    形    参: _FlashAddr 起始地址
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint16_t PG_SWD_EraseChip(uint32_t _FlashAddr)
{
    uint8_t err = 0;
    error_t err_t;
    
	if (swd_init_debug() == 0)
    {
        PG_PrintText("SWD初始化失败！");        
        err = 1;
        goto quit;        
    }
	
	err_t = target_flash_init(_FlashAddr);
    if (err_t == ERROR_RESET)
    {
        PG_PrintText("复位目标MCU失败");
        err = 1;
        goto quit;        

    }
    else if (err_t == ERROR_ALGO_DL)
    {
        PG_PrintText("下载算法失败");        
        err = 1;
        goto quit;        
    }
    else if (err_t == ERROR_INIT)
    {
        PG_PrintText("执行算法失败");        
        err = 1;
        goto quit;        
    } 
    
  
    /*　开始擦除 */
    if (target_flash_erase_chip() != 0)
    {
        PG_PrintText("整片擦除失败");        
        err = 1;
        goto quit;
    }                    

    PG_PrintPercent(100, 0xFFFFFFFF);           
	
quit:
    return err;
}

/*
*********************************************************************************************************
*    函 数 名: PG_SWD_EraseSector
*    功能说明: 开始擦除扇区
*    形    参: _FlashAddr 起始地址
*    返 回 值: 0 = ok, 其他表示错误
*********************************************************************************************************
*/
uint16_t PG_SWD_EraseSector(uint32_t _FlashAddr)
{
    uint8_t err = 0;
    error_t err_t;
    
	if (swd_init_debug() == 0)
    {
        PG_PrintText("SWD初始化失败！");        
        err = 1;
        goto quit;        
    }
	
	err_t = target_flash_init(_FlashAddr);
    if (err_t == ERROR_RESET)
    {
        PG_PrintText("复位目标MCU失败");
        err = 1;
        goto quit;        

    }
    else if (err_t == ERROR_ALGO_DL)
    {
        PG_PrintText("下载算法失败");        
        err = 1;
        goto quit;        
    }
    else if (err_t == ERROR_INIT)
    {
        PG_PrintText("执行算法失败");        
        err = 1;
        goto quit;        
    } 
    
    /*　开始擦除 */
    if (target_flash_erase_sector(_FlashAddr) != 0)
    {
        PG_PrintText("擦除扇区失败");        
        err = 1;
        goto quit;
    }                          
	
quit:
    return err;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
