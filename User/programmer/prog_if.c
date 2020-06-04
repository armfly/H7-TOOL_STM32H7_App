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
        /* g_tParam.ResetType;   0表示由lua脚本决定  1表示强制硬件复位 2表示强制软件复位 */
        if (g_tParam.ResetType == 0)    /* 缺省 */
        {
            /* 读取复位类型： 软件还是硬件复位 */
            lua_getglobal(g_Lua, "RESET_TYPE");  
            if (lua_isinteger(g_Lua, -1)) 
            {
                g_tProg.ResetType = (REST_TYPE_E)lua_tointeger(g_Lua, -1);
            }
            else
            {
                g_tProg.ResetType = SOFT_RESET;
            }
            lua_pop(g_Lua, 1);            
        }
        else if (g_tParam.ResetType == 1)
        {
            g_tProg.ResetType = HARD_RESET;     /* 强制硬件复位 */
        }
        else
        {
            g_tProg.ResetType = SOFT_RESET;     /* 强制软件复位 */
        }      

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
            /* 读取复位类型： 软件还是硬件复位 */
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
            /* 读取复位类型： 软件还是硬件复位 */
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
            printf("\r\n  %dms, %0.2f%%\r\n", bsp_CheckRunTime(g_tProg.Time), _Percent);
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
    if (g_tProg.SnEnable > 0)
    {
        for (i = 0; i < g_tProg.SnLen; i++)
        {
            if (g_tProg.SnAddr + i >= _FlashAddr && g_tProg.SnAddr + i < _FlashAddr + _BuffSize)
            {
                _Buff[g_tProg.SnAddr - _FlashAddr + i] = g_tProg.SnData[i];
                g_tProg.SnEnable = 2;
                change = 1;
            }
        }
    }  
    
    /* UID加密存储 */
    if (g_tProg.UidEnable > 0)
    {
        for (i = 0; i < g_tProg.UidLen; i++)
        {
            if (g_tProg.UidAddr + i >= _FlashAddr && g_tProg.UidAddr + i < _FlashAddr + _BuffSize)
            {
                _Buff[g_tProg.UidAddr - _FlashAddr + i] = g_tProg.UidData[i];
                g_tProg.UidEnable = 2;
                change = 1;           
            }
        }
    }
   
    /* USR 用户数据存储 */
    if (g_tProg.UsrEnable > 0)
    {
        for (i = 0; i < g_tProg.UsrLen; i++)
        {
            if (g_tProg.UsrAddr + i >= _FlashAddr && g_tProg.UsrAddr + i < _FlashAddr + _BuffSize)
            {
                _Buff[g_tProg.UsrAddr - _FlashAddr + i] = g_tProg.UsrData[i];
                g_tProg.UsrEnable = 2;
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
    if (g_tProg.SnEnable > 0)
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
    if (g_tProg.UidEnable > 0)
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
    if (g_tProg.UsrEnable > 0)
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
    
    /* 第0步 ******************** 检查文件 **********************/
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
    if (_FlashAddr < g_tFLM.Device.DevAdr || _FlashAddr + FileLen > g_tFLM.Device.DevAdr + g_tFLM.Device.szDev)
    {
        PG_PrintText("目标地址超过芯片范围");    
        err = 1;
        goto quit; 
    }

    /* 第1步 ******************** 查空 **********************/
    
    /* 加载MCU的编程算法到目标机内存 */
    {
        /* SWD进入debug状态 */
        err_t = target_flash_enter_debug_program();
        if (err_t != ERROR_SUCCESS)
        {            
            err = 1;
            goto quit;  
        }
        /* 装载芯片厂家的FLM算法代码到目标机内存 */
        LoadAlgoToTarget();
        
        /* 装载算法并执行init函数 */
        err_t = target_flash_init(_FlashAddr);
        if (err_t != ERROR_SUCCESS)
        {
            PG_PrintText("error: target_flash_init()");
            err = 1;
            goto quit;
        }
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
            uint32_t bytes;
            uint32_t i, j;
            uint8_t en;
               
            bytes = FileLen;            
            if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
            {
                if (bytes > sizeof(flash_buff) / 4)
                {
                    bytes = sizeof(flash_buff) / 4;
                }
            }
            else
            {
                if (bytes > sizeof(flash_buff))
                {
                    bytes = sizeof(flash_buff);
                }
            }
                
            fBlankChip = 1;
            if (flash_algo.check_blank > 0)     /* 如果FLM有查空函数，则调用该函数，提高效率 */
            {                
                if (target_flash_check_blank(_FlashAddr,  FileLen) == 0)
                {
                    fBlankChip = 0;     /* 0表示不空 */
                }
                PG_PrintPercent(100, _FlashAddr); 
            }
            else    /* FLM没有查空函数*/
            {
            #if 1
                /* 加载check blank的算法到目标机内存 */
                {
                    /* 装载算法代码到目标机内存 */
                    LoadCheckBlankAlgoToTarget();
                    
                    if (target_flash_check_blank_ex(_FlashAddr,  FileLen) == 0)
                    {
                        fBlankChip = 0;     /* 0表示不空 */
                    }
                    PG_PrintPercent(100, _FlashAddr); 
                
                    /* 恢复芯片厂家的FLM算法代码到目标机内存 */
                    LoadAlgoToTarget();
                }
            #else
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
                    {
                        if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
                        {
                            uint8_t m;
                            
                            if (MUL_swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                            {
                                err = 1;
                            }

                            if (err == 1)
                            {
                                char buf[128];
                                
                                sprintf(buf, "swd_read_memory error, addr = %X, len = %X", g_tFLM.Device.DevAdr + addr, bytes);
                                PG_PrintText(buf); 
                                err = 1;
                                goto quit;                 
                            }
                            
                            for (m = 0; m < 4; m++)
                            {
                                if (g_gMulSwd.Active[m] == 1)
                                {
                                    for (i = 0; i < bytes; i++)
                                    {
                                        if (flash_buff[bytes * m + i] != g_tFLM.Device.valEmpty)
                                        {
                                            fBlankChip = 0;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                            {
                                err = 1;
                            } 
                            if (err == 1)
                            {
                                char buf[128];
                                
                                sprintf(buf, "swd_read_memory error, addr = %X, len = %X", g_tFLM.Device.DevAdr + addr, bytes);
                                PG_PrintText(buf);   
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
                        }
                    }
                    
                    if (fBlankChip == 0)
                    {
                        break;
                    }
                    
                    /* MM32L373  (128KB FLASH) 需要加这个延迟 */
                    bsp_DelayUS(1000);
                    
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
            #endif
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
                    if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
                    {
                        uint8_t m;
                        
                        if (MUL_swd_read_memory(addr, flash_buff, bytes) == 0)
                        {
                            char buf[128];
                            
                            sprintf(buf, "swd_read_memory error, addr = %X, len = %X", addr, bytes);
                            PG_PrintText(buf);                             
                            err = 1;
                            goto quit;                 
                        }
                        
                        for (m = 0; m < 4; m++)
                        {
                            if (g_gMulSwd.AckOk[m] == 1)
                            {
                                for (i = 0; i < bytes; i++)
                                {
                                    if (flash_buff[bytes * m + i] != g_tFLM.Device.valEmpty)
                                    {
                                        fBlankChip = 0;
                                        break;
                                    }
                                }
                            }
                            
                            if (fBlankChip == 0)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (swd_read_memory(addr, flash_buff, bytes) == 0)
                        {
                            char buf[128];
                            sprintf(buf, "swd_read_memory error, addr = %X, len = %X", addr, bytes);
                            PG_PrintText(buf);                               
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
                    }
                    
                    if (fBlankChip == 0)
                    {
                        break;
                    } 
                }
            }
        }
    }
    
    /* 第2步 ******************** 擦除 **********************/
    if (EraseChipEnable == 0)   /* 按扇区擦除 */
    {
        if (fBlankChip == 0)    /* 不是空片才进行擦除 */
        {
            uint32_t j;
            uint32_t addr;
            uint32_t FinishedSize = 0;
            uint8_t fEraseReq = 0;
            uint8_t waittime = 0;
            
            /* 根据算法名称判断芯片 */
            if (strstr(flash_algo.algo_file_name, "/MindMotion/"))
            {
                waittime = 5;
                
                bsp_DelayMS(20);
            }
            
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
                    
                    /* MM32F031x6 扇区擦除后需要加一点延迟才能继续，原因未知 */
                    bsp_DelayMS(waittime);
                    
                    if (target_flash_erase_sector(addr) != 0)
                    {
                        //PG_PrintText("扇区擦除失败");
                        {
                            char buf[128];
                            
                            sprintf(buf, "扇区擦除失败, 0x%08X", addr);
                            PG_PrintText(buf); 
                        }                        
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
            PG_PrintText("正在擦除整片...");    
            PG_PrintPercent(0, 0xFFFFFFFF);   
            bsp_Idle();            
        
            /* 根据算法名称判断芯片 */
            if (strstr(flash_algo.algo_file_name, "/MindMotion/"))
            {
                if (PG_SWD_EraseChip(g_tFLM.Device.DevAdr) == 1)
                {
                    PG_PrintText("整片擦除失败");        
                    err = 1;
                    goto quit;                    
                }
            }
            else
            {
                /*　开始擦除 */
                if (target_flash_erase_chip() != 0)
                {
                    PG_PrintText("整片擦除失败");        
                    err = 1;
                    goto quit;
                }                    

                PG_PrintPercent(100, 0xFFFFFFFF);                       
            }
        }
    }

    /* 第2步 ******************** 编程 **********************/
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
        uint32_t AddrEnd = 0;
   
		PageSize = g_tFLM.Device.szPage;
		if (PageSize > sizeof(FsReadBuf))
		{
			PageSize = sizeof(FsReadBuf);
		}
		
        /* 整定文件缓冲区大小为PageSize的整数倍, 芯片的pagesize一般为 128 256 512 1024 2048 4096 */
        FileBuffSize = sizeof(FsReadBuf);   
        FileBuffSize = (FileBuffSize / PageSize) * PageSize;
        
        addr = _FlashAddr;   /* 目标地址 */
        for (j = 0; j < (FileLen + FileBuffSize - 1) / FileBuffSize; j++)
        {
            if (ProgCancelKey())
            {
                PG_PrintText("用户终止运行");    
                err = 1;
                goto quit;
            }

            /* 读文件, 按最大缓冲区读取到内存 */
            bytes = ReadFileToMem(path, FileOffset, FsReadBuf, FileBuffSize);               
            if (bytes == 0)
            {
                PG_PrintText("读取数据文件失败");        
                err = 1;
                goto quit;
            }
            else if (bytes != FileBuffSize) /* 文件末尾，最后一包不足FileBuffSize */
            {
                /* V1.10 修正bug : 烧写非整数倍PageSize的文件失败 */
                if (bytes % PageSize)
                {
                    memset(&FsReadBuf[bytes], g_tFLM.Device.valEmpty, PageSize - (bytes % PageSize));      /* 填充空值 00 */
                    
                    bytes = ((bytes + PageSize - 1) / PageSize) * PageSize;
                }                    
            }
            else    /* bytes == FileBuffSize */
            {
                ;
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
        AddrEnd = addr; /* 程序以外的第1个page地址 */

        /* 如果UID加密数据或产品序号在文件以外的空间 */
        {
            /* 因为扫描是文件以外的空间，因此需要避免分段烧写时，重复填入UID */
            addr = g_tFLM.Device.DevAdr;   /* 芯片起始地址 */        
            for (i = 0; i < g_tFLM.Device.DevAdr / PageSize; i++)
            {
                if (addr < _FlashAddr || addr >= AddrEnd)
                {
                    if (PG_CheckFlashMem(addr, FsReadBuf, PageSize))
                    {
                        memset(FsReadBuf, g_tFLM.Device.valEmpty, PageSize);
                        
                        /* 修改缓冲区，填充UID加密数据或产品序号 */
                        if (PG_FixFlashMem(addr, FsReadBuf, PageSize) == 1)
                        {                        
                            if (target_flash_program_page(addr, (uint8_t *)FsReadBuf, PageSize) != 0)
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
                    }
                }
                addr += PageSize;
            }
            
            /* 如果本次任务已处理uid烧录，后面的任务无需再重复执行 */
            if (g_tProg.UsrEnable == 2)
            {
                g_tProg.UsrEnable = 0;
            }            
            if (g_tProg.UidEnable == 2)
            {
                g_tProg.UidEnable = 0;
            }
            if (g_tProg.SnEnable == 2)
            {
                g_tProg.SnEnable = 0;
            }    
        }
    }
    
    /* 第3步 ******************** 校验 **********************/ 
    /* 
        2020-05-22 记录: STM32F207RCT6，256K Flash
            FLM_CRC32    150ms
            READ_BACK    461ms
            STM32_CRC32  170ms    
            SOFT_CRC32   275ms
    
        2020-05-23   STM32G474CET6, 512K Flash
            READ_BACK    931ms
            SOFT_CRC32   558ms  
            STM32_CRC32  316ms    
    */
    if (flash_algo.cacul_crc32 > 0 && g_tProg.VerifyMode == VERIFY_AUTO)
    {
        PG_PrintText("正在校验...(FLM_CRC32)"); 
    }
    else
    {        
        if (g_tProg.VerifyMode == VERIFY_SOFT_CRC)
        {
            PG_PrintText("正在校验...(SOFT_CRC32)");  
        }
        else if (g_tProg.VerifyMode == VERIFY_STM32_CRC)
        {
            PG_PrintText("正在校验...(STM32_CRC32)"); 
        }    
        else    /* VERIFY_READ_BACK */
        {
            if (flash_algo.verify > 0)                
            {    
                PG_PrintText("正在校验...(FLM_Verify)");  
            }
            else
            {
                PG_PrintText("正在校验...(Readback)");  
            }
        }
        
        /* 加载MCU的编程算法到目标机内存 */
        if (g_tProg.VerifyMode == VERIFY_SOFT_CRC || g_tProg.VerifyMode == VERIFY_STM32_CRC)
        {
            /* 装载算法代码到目标机内存 */
            LoadCheckCRCAlgoToTarget();
        }
    }
    
    PG_PrintPercent(0, 0xFFFFFFFF);    
    {
        uint32_t addr;
		uint32_t FileOffset = 0;
		uint16_t PageSize;
		uint32_t bytes;
   
        if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
        {
            if (flash_algo.cacul_crc32 > 0)
            {
                PageSize = sizeof(flash_buff);  
            }
            else
            {
                PageSize = sizeof(flash_buff) / 4;
            }
		}
        else
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
                
                memset(&FsReadBuf[bytes], g_tFLM.Device.valEmpty ,PageSize - bytes);
			}		
			
            /* 修改缓冲区，填充UID加密数据或产品序号 */
            PG_FixFlashMem(g_tFLM.Device.DevAdr + addr, FsReadBuf, PageSize);
           
            
            if ( (flash_algo.cacul_crc32 > 0 && g_tProg.VerifyMode == VERIFY_AUTO)
                || g_tProg.VerifyMode == VERIFY_SOFT_CRC 
                || g_tProg.VerifyMode == VERIFY_STM32_CRC)  /* 由目标机执行CRC校验 */
            {
                uint32_t crc1, crc2;

                /* 文件长度不是4字节整数倍，则补齐后再进行硬件CRC32 */
                {
                    uint8_t rem;
                    uint8_t k;
                    
                    rem = bytes % 4;
                    if (rem > 0)
                    {
                        rem = 4 - rem;
                        for (k = 0; k < rem; k++)
                        {
                            FsReadBuf[bytes + k] = g_tFLM.Device.valEmpty;
                        }
                        bytes += rem;
                    }
                }
                
                if (flash_algo.cacul_crc32 > 0 && g_tProg.VerifyMode == VERIFY_AUTO)     /* 执行FLM中的crc算法 */
                {
                    crc1 = target_flash_cacul_crc32(g_tFLM.Device.DevAdr + addr, bytes, 0xFFFFFFFF);
                    crc2 = STM32_CRC32_Word((uint32_t *)FsReadBuf, bytes);      /* 目前仅支持STM32算法 */                     
                }
                else    /* 临时加载到目标机的通用算法 */
                {
                    crc1 = target_flash_cacul_crc32_ex(g_tFLM.Device.DevAdr + addr, bytes, 0xFFFFFFFF);
                    if (g_tProg.VerifyMode == VERIFY_STM32_CRC)
                    {
                        crc2 = STM32_CRC32_Word((uint32_t *)FsReadBuf, bytes);      /* 目前仅支持STM32算法 */     
                    }
                    else    /* (g_tProg.VerifyMode == VERIFY_SOFT_CRC) */
                    {
                        crc2 = soft_crc32((uint8_t *)FsReadBuf, bytes);   
                    }
                }
                    
                if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
                {                    
                    uint8_t i;
                    uint8_t err0 = 0;
                    char errstr[32];
      
                    strcpy(errstr, "CRC校验失败");
                    
                    for (i = 0; i < 4; i++)
                    {
                        if (g_gMulSwd.Active[i] == 1)
                        {
                            if (crc2 != ((uint32_t *)crc1)[i])
                            {                     
                                err0 = 1;
                                
                                sprintf(&errstr[strlen(errstr)], " #%d", i + 1);                                
                            }
                        }
                    }

                    if (err0 == 1)
                    {
                        PG_PrintText(errstr);  
                        err = 1;
                        goto quit;	                        
                    }
                }
                else
                {
                    if (crc1 != crc2)
                    {              
                        {
                            char buf[128];
                            
                            sprintf(buf, "校验失败, 0x%08X", g_tFLM.Device.DevAdr + addr);
                            PG_PrintText(buf); 
                            
                            printf("crc_read = %08X  crc_ok = %08X\r\n", crc1, crc2);
                        } 
                        err = 1;
                        goto quit;	                    
                    }                    
                }                
                addr += PageSize;
                FileOffset += PageSize;                  
            }
            else if (flash_algo.verify > 0)     /* FLM有verify校验函数 */
            {
                if (target_flash_verify_page(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) != 0)
                {
                    {
                        char buf[128];
                        
                        sprintf(buf, "校验失败, 0x%08X", g_tFLM.Device.DevAdr + addr);
                        PG_PrintText(buf); 
                    }   
                    err = 1;
                    goto quit;                    
                }
            }
            else    /* readback 校验 */
            {
                
                if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
                {
                    uint8_t i;
                    
                    /* 读回进行校验 */                    
                    if (MUL_swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                    {
                        char buf[128];
                        
                        sprintf(buf, "swd_read_memory error, addr = %X, len = %X", g_tFLM.Device.DevAdr + addr, bytes);
                        PG_PrintText(buf);    
                        err = 1;
                        goto quit;  
                    }

                    for (i = 0; i < 4; i++)
                    {
                        if (g_gMulSwd.Active[i] == 1)
                        {                    
                            if (memcmp(&FsReadBuf[PageSize * i], &flash_buff[bytes * i], bytes) != 0)
                            {
                                {
                                    char buf[128];
                                    
                                    sprintf(buf, "校验失败, 0x%08X", g_tFLM.Device.DevAdr + addr);
                                    PG_PrintText(buf); 
                                }                     
                                err = 1;
                                goto quit;				
                            } 
                        }
                    }
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
                    
                    /* V1.11 修正bug : 烧写非整数倍PageSize的文件失败 */
                    if (bytes % PageSize)
                    {
                        memset(&FsReadBuf[bytes], 0x00 ,PageSize - (bytes % PageSize));      /* 填充空值 00 */
                        
                        bytes = ((bytes + PageSize - 1) / PageSize) * PageSize;
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

        /* 如果本次任务已处理uid烧录，后面的任务无需再重复执行 */
        if (g_tProg.UsrEnable == 2)
        {
            g_tProg.UsrEnable = 0;
        }            
        if (g_tProg.UidEnable == 2)
        {
            g_tProg.UidEnable = 0;
        }
        if (g_tProg.SnEnable == 2)
        {
            g_tProg.SnEnable = 0;
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
    
    /* SWD进入debug状态 */
    err_t = target_flash_enter_debug_program();
    if (err_t != ERROR_SUCCESS)
    {
        err = 1;
        goto quit;  
    }
    /* 装载算法代码到目标机内存 */
    LoadAlgoToTarget();
    
    /* 装载算法并执行init函数 */
	err_t = target_flash_init(_FlashAddr);
    if (err_t != ERROR_SUCCESS)
    {
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
            
            if (_Mode == 2) /* 擦除所在扇区后，写入，其他数据会清空. 用于Options bytes编程 */
            {
                //PG_PrintText("正在擦除...");   
                                
                /*　开始擦除 - STM32F429 包含解除读保护 */ 
                printf("\r\nOption Bytes: erase_chip()\r\n");
                if (target_flash_erase_chip() != 0)
                {
                    PG_PrintText("整片擦除失败");        
                    err = 1;
                    goto quit;
                }      

                /* STM32F103 option bytes 编程时需要执行 erase_sector */
                g_tProg.FLMFuncDispProgress = 1;
                g_tProg.FLMFuncDispAddr = g_tFLM.Device.DevAdr + PageStartAddr;
                printf("Option Bytes: erase_sector()\r\n");
                if (target_flash_erase_sector(g_tFLM.Device.DevAdr + PageStartAddr) != 0)
                {
                    PG_PrintText("扇区擦除失败");        
                    err = 1;
                    goto quit;
                } 
                
                /* 未判断返回值。写STM32F103 OPTION BYTES会返回错误, 但是写入已经成功 */
                //PG_PrintText("正在编程...");
                g_tProg.FLMFuncDispProgress = 1;
                g_tProg.FLMFuncDispAddr = g_tFLM.Device.DevAdr + PageStartAddr;
                printf("Option Bytes: program_page()\r\n");
                if (target_flash_program_page(g_tFLM.Device.DevAdr + PageStartAddr, _DataBuf, PageSize) != 0)
                {
                    PG_PrintText("program_page failed");        
                    err = 1;
                    goto quit;                    
                }
                
                /*  */
                //PG_PrintText("正在校验..."); 
                
                if (g_tProg.VerifyOptionByteDisalbe == 0)
                {
                    printf("\r\nOption Bytes: verify\r\n");
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
                        if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
                        {
                            uint8_t i;
                            
                            /* 读回进行校验 */
                            if (MUL_swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                            {
                                PG_PrintText("swd_read_memory error");        
                                err = 1;
                                goto quit;				
                            }                     
                            for (i = 0; i < 4; i++)
                            {
                                if (g_gMulSwd.Active[i] == 1)
                                {                    
                                    if (memcmp(&_DataBuf[FileOffset], &flash_buff[bytes * i], bytes) != 0)
                                    {
                                        PG_PrintText("校验数据失败");                            
                                        err = 1;
                                        goto quit;				
                                    } 
                                }
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
                }
                else
                {
                    printf("Verify Option bytes cancelled\r\n");
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

                if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
                {
                    MUL_swd_read_memory(PageStartAddr + g_tFLM.Device.DevAdr, flash_buff, PageSize);
                }
                else
                {
                    swd_read_memory(PageStartAddr + g_tFLM.Device.DevAdr, flash_buff, PageSize);
                }
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
                if (g_gMulSwd.MultiMode > 0)   /* 多路模式 */
                {
                    uint8_t i;
                    
                    if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
                    {
                        PG_PrintText("swd_read_memory error");        
                        err = 1;
                        goto quit;				
                    }                
                    for (i = 0; i < 4; i++)
                    {
                        if (g_gMulSwd.Active[i] == 1)
                        {                    
                            if (memcmp(&_DataBuf[FileOffset], &flash_buff[bytes * i], bytes) != 0)
                            {
                                PG_PrintText("校验数据失败");                            
                                err = 1;
                                goto quit;				
                            } 
                        }
                    }                     
                }
                else
                {
                    if (MUL_swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
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
            else if (_Mode == 0)    /* 读 - 修改 - 写，同page内其他数据保持不变 */
            {
// 这个分支暂时未用到，屏蔽                
//                swd_read_memory(PageStartAddr + g_tFLM.Device.DevAdr, flash_buff, PageSize);        /* 读取一个page  */
//                
//                /* 如果内容相同则不处理 */
//                if (memcmp(&flash_buff[addr % PageSize], (uint8_t *)&_DataBuf[FileOffset], bytes) != 0)
//                {
//                    /* 如果不是空值就擦除 */
//                    //if (CheckBlankBuf((char *)&flash_buff[addr % PageSize], bytes, g_tFLM.Device.valEmpty) == 0)  
//                    {
//                        /*　开始擦除 */
//                        if (target_flash_erase_sector(g_tFLM.Device.DevAdr + PageStartAddr) != 0)
//                        {
//                            PG_PrintText("扇区擦除失败");        
//                            err = 1;
//                            goto quit;
//                        }
//                        
//                        //swd_read_memory(PageStartAddr + g_tFLM.Device.DevAdr, flash_buff, PageSize);
//                    }                                
//                    
//                    memcpy(&flash_buff[addr % PageSize], (uint8_t *)&_DataBuf[FileOffset], bytes);                   
//                    
//                    /* 整页编程 */
//                    if (target_flash_program_page(g_tFLM.Device.DevAdr + PageStartAddr, flash_buff, PageSize) != 0)
//                    {
//                        PG_PrintText("编程失败");        
//                        err = 1;
//                        goto quit;
//                    }
//                    
//                    memset(flash_buff, 0xFF, PageSize);
//                    
//                    /* 读回进行校验 */
//                    if (swd_read_memory(g_tFLM.Device.DevAdr + addr, flash_buff, bytes) == 0)
//                    {
//                        PG_PrintText("swd_read_memory error");        
//                        err = 1;
//                        goto quit;				
//                    }   
//                  
//                    if (memcmp((uint8_t *)&_DataBuf[FileOffset], flash_buff, bytes) != 0)
//                    {
//                        PG_PrintText("校验数据失败");        
//                        err = 1;
//                        goto quit;				
//                    }            
//                }
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
    
    err = PG_SWD_ProgBuf(_FlashAddr, _DataBuf, _BufLen, 2);
    
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
    
    /* SWD进入debug状态 */
    err_t = target_flash_enter_debug_program();
    if (err_t != ERROR_SUCCESS)
    {
        err = 1;
        goto quit;  
    }
    /* 装载算法代码到目标机内存 */
    LoadAlgoToTarget();
    
    /* 装载算法并执行init函数 */
	err_t = target_flash_init(_FlashAddr);
    if (err_t != ERROR_SUCCESS)
    {
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
    
    /* SWD进入debug状态 */
    err_t = target_flash_enter_debug_program();
    if (err_t != ERROR_SUCCESS)
    {
        err = 1;
        goto quit;  
    }    

    /* 装载算法代码到目标机内存 */
    LoadAlgoToTarget();
    
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
