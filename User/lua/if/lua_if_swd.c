#include "lauxlib.h"
#include "lualib.h"
#include "time.h"
#include "lua_if.h"
#include "bsp.h"
#include "elf_file.h"
#include "file_lib.h"
#include "prog_if.h"
#include "target_reset.h"
#include "target_config.h"
#include "swd_host.h"
#include "Systick_Handler.h"
#include "main.h"
#include "target_family.h"

/* 为了避免和DAP驱动中的函数混淆，本模块函数名前缀用 h7swd */

static int h7swd_Init(lua_State* L);
static int h7swd_ReadID(lua_State* L);
static int h7swd_WriteMemory(lua_State* L);
static int h7swd_ReadMemory(lua_State* L);

static int h7_LoadAlgoFile(lua_State* L);
static int h7_ProgFile(lua_State* L);
static int h7_ProgBuf(lua_State* L);
static int h7_ProgBuf_OB(lua_State* L);
static int h7_reset(lua_State* L);
static int h7_DetectIC(lua_State* L);
static int h7_PrintText(lua_State* L);
static int h7_EraseChip(lua_State* L);

static int h7_Read_ProductSN(lua_State* L);
static int h7_Write_ProductSN(lua_State* L);

/*
*********************************************************************************************************
*    函 数 名: lua_swd_RegisterFun
*    功能说明: 注册lua C语言接口函数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
extern void PG_PrintText(char *_str);
void lua_swd_RegisterFun(void)
{
    //将指定的函数注册为Lua的全局函数变量，其中第一个字符串参数为Lua代码
    //在调用C函数时使用的全局函数名，第二个参数为实际C函数的指针。
    lua_register(g_Lua, "swd_init", h7swd_Init);    
    lua_register(g_Lua, "swd_getid", h7swd_ReadID);
    lua_register(g_Lua, "swd_write", h7swd_WriteMemory);    
    lua_register(g_Lua, "swd_read", h7swd_ReadMemory);
        
    lua_register(g_Lua, "pg_load_algo_file", h7_LoadAlgoFile);
    lua_register(g_Lua, "pg_prog_file", h7_ProgFile);
    lua_register(g_Lua, "pg_prog_buf", h7_ProgBuf);
    lua_register(g_Lua, "pg_prog_buf_ob", h7_ProgBuf_OB);
    lua_register(g_Lua, "pg_reset", h7_reset);
    lua_register(g_Lua, "pg_detect_ic", h7_DetectIC);
    lua_register(g_Lua, "pg_erase_chip", h7_EraseChip);    
    
    lua_register(g_Lua, "pg_print_text", h7_PrintText);
    lua_register(g_Lua, "pg_read_sn", h7_Read_ProductSN);
    lua_register(g_Lua, "pg_write_sn", h7_Write_ProductSN);
}
            
/*
*********************************************************************************************************
*    函 数 名: h7swd_Init
*    功能说明: 读芯片ID
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static int h7swd_Init(lua_State* L)
{
    float vcc;
        
    sysTickInit();    /* 这是DAP驱动中的初始化函数,全局变量初始化 */

    if (lua_type(L, 1) == LUA_TNUMBER) /* 判断第1个参数 */
    {
        vcc = luaL_checknumber(L, 1);    /* VCC电压，浮点，单位V */
    }
    
    bsp_SetTVCC(vcc * 1000);    /* 设置接口电平3.3V */
    bsp_DelayUS(100 * 100);     /* 延迟100ms */
        
    swd_init_debug();           /* 进入swd debug状态 */
        
    return 1;
}

/*
*********************************************************************************************************
*    函 数 名: h7swd_ReadID
*    功能说明: 读芯片ID
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static int h7swd_ReadID(lua_State* L)
{
    uint32_t id;

    if (swd_read_idcode(&id) == 0)
    {
        lua_pushnumber(L, 0);    /* 出错 */
    }
    else
    {
        lua_pushnumber(L, id);    /* 成功,返回ID */
    }    
    return 1;
}

/*
*********************************************************************************************************
*    函 数 名: h7swd_WriteMemory
*    功能说明: 写CPU内存（或寄存器）
*    形    参: addr : 目标地址
*                data : 数据缓冲区，含长度
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7swd_WriteMemory(lua_State* L)
{
    uint32_t addr = 0;
    const char *data;
    size_t len = 0;
    
    sysTickInit();    /* 这是DAP驱动中的初始化函数,全局变量初始化 */

    if (lua_type(L, 1) == LUA_TNUMBER) /* 判断第1个参数 */
    {
        addr = luaL_checknumber(L, 1);    /* 目标内存地址 */
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;        
    }

    if (lua_type(L, 2) == LUA_TSTRING)     /* 判断第2个参数 */
    {        
        data = luaL_checklstring(L, 2, &len); /* 1是参数的位置， len是string的长度 */        
    }
    
    if (len > 128 * 1024)
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    if (swd_write_memory(addr, (uint8_t *)data, len) == 0)
    {
        lua_pushnumber(L, 0);    /* 出错 */
    }
    else
    {
        lua_pushnumber(L, 1);    /* 成功 */
    }
        
    return 1;
}

/*
*********************************************************************************************************
*    函 数 名: h7swd_ReadMemory
*    功能说明: 读CPU内存（或寄存器）
*    形    参: addr : 目标地址
*                data : 数据缓冲区，含长度
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7swd_ReadMemory(lua_State* L)
{
    uint32_t addr;
    uint32_t num;

    if (lua_type(L, 1) == LUA_TNUMBER)     /* 判断第1个参数 */
    {        
        addr = luaL_checknumber(L, 1); /* 1是参数的位置， len是string的长度 */        
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    if (lua_type(L, 2) == LUA_TNUMBER) /* 判断第2个参数 */
    {
        num = luaL_checknumber(L, 2);
        
        memset(s_lua_read_buf, 0, num);        
    }
    
    if (num > LUA_READ_LEN_MAX)
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }

    if (swd_read_memory(addr, s_lua_read_buf, num) == 0)
    {
        lua_pushnumber(L, 0);    /* 出错 */
    }
    else
    {
        lua_pushnumber(L, 1);    /* 成功 */
    }

    lua_pushlstring(L, (char *)s_lua_read_buf, num); 
    return 2;
}

/*
*********************************************************************************************************
*    函 数 名: h7_LoadAlgoFile
*    功能说明: 装载算法文件
*    形    参: file_path 算法文件路径，是相对路径
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7_LoadAlgoFile(lua_State* L)
{
    // pg_load_algo_file(AlgoFile, AlgoRamAddr, AlgoRamSize)
    const char *data;
    size_t len = 0;

    g_AlgoRam.Valid = 0;
    
    if (lua_type(L, 1) == LUA_TSTRING)     /* 判断第1个参数 */
    {        
        data = luaL_checklstring(L, 1, &len); /* 1是参数的位置， len是string的长度 */   
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    if (lua_type(L, 2) == LUA_TNUMBER) /* 判断第2个参数 */
    {
        g_AlgoRam.Addr = luaL_checknumber(L, 2);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }    
    
    if (lua_type(L, 3) == LUA_TNUMBER) /* 判断第3个参数 */
    {
        g_AlgoRam.Size = luaL_checknumber(L, 3);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }    
    
    g_AlgoRam.Valid = 1;
    
    /* lua程序提供的路径是相对路径 */
    {
        char FullPath[128];

        if (strlen(PROG_FLM_DIR) + strlen(data) >= sizeof(FullPath))
        {
            lua_pushnumber(L, 0);    /* 出错 文件名过长 */
            return 1;
        }
        
        sprintf(FullPath, "%s/%s", PROG_FLM_DIR, data);
        if (ELF_ParseFile((char *)FullPath) != 0)
        {
            lua_pushnumber(L, 0);    /* 出错 */
            return 1;
        }
    }
    
    lua_pushnumber(L, 1);    /*OK */
    return 1;    
}

/*
*********************************************************************************************************
*    函 数 名: h7_ProgFile
*    功能说明: 开始编程，从文件读取数据
*    形    参: file_path 数据文件名称, 目标flash地址
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7_ProgFile(lua_State* L)
{
    const char *file_name;
    size_t len = 0;
    uint32_t FlashAddr;
    
    if (lua_type(L, 1) == LUA_TSTRING)     /* 判断第1个参数 */
    {        
        file_name = luaL_checklstring(L, 1, &len); /* 1是参数的位置， len是string的长度 */   
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    if (lua_type(L, 2) == LUA_TNUMBER) /* 判断第2个参数 */
    {
        FlashAddr = luaL_checknumber(L, 2);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    if (PG_ProgFile((char *)file_name, FlashAddr) == 0)
    {
        lua_pushnumber(L, 1);    /*OK */
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
    }
    
    return 1;    
}

/*
*********************************************************************************************************
*    函 数 名: h7_reset
*    功能说明: 执行硬件复位
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static int h7_reset(lua_State* L)
{
    /* 软件复位目标板 M3 - M7 好像都一样的操作 (该功能需要测试） */
    {
        uint32_t reg_addr;
        uint32_t reg_value;

        reg_addr = (uint32_t )&SCB->AIRCR;
        reg_value = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      | 
                     (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | 
                     SCB_AIRCR_SYSRESETREQ_Msk);                           /* Keep priority group unchanged */         
        swd_write_memory(reg_addr, (uint8_t *)&reg_value, 4);   
    }
    
    /* 硬件复位 */
    {
        swd_set_target_reset(1);
        osDelay(20);
        swd_set_target_reset(0);
        osDelay(20); 
    }
    
    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: h7_DetectIC
*    功能说明: 检测CPU ID
*    形    参: 无
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7_DetectIC(lua_State* L)
{
    uint8_t i;
    uint32_t id;
    
    for (i = 0; i < 10; i++)
    {
        sysTickInit();    /* 这是DAP驱动中的初始化函数,全局变量初始化 */
    
        bsp_DelayUS(100 * 100);     /* 延迟100ms */
        
        swd_init_debug();           /* 进入swd debug状态 */
        
        if (swd_read_idcode(&id) == 0)
        {
            id = 0;     /* 出错继续检测 */
        }
        else
        {
            break;            
        }   
    }
    
    lua_pushnumber(L, id);    /* 成功,返回ID */
    
    if (id == 0)
    {
        PG_PrintText("未检测到芯片");
    }
    return 1;
}

/*
*********************************************************************************************************
*    函 数 名: h7_PrintText
*    功能说明: 在LCD显示一串文本
*    形    参: 无
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7_PrintText(lua_State* L)
{
    const char *str;
     size_t len = 0;
    
    if (lua_type(L, 1) == LUA_TSTRING)     /* 判断第1个参数 */
    {        
        str = luaL_checklstring(L, 1, &len); /* 1是参数的位置， len是string的长度 */   
        
        if (g_MainStatus == MS_PROG_WORK)
        {
            printf(str);
                    
            if (g_MainStatus == MS_PROG_WORK)
            {
                DispProgProgress((char *)str, -1);      /* -1表示不刷新进度 */
            }
        }
    }

    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: h7_ProgBuf
*    功能说明: 开始编程，从新参读入数据。
*    形    参: Flash地址
*			   数据
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7_ProgBuf(lua_State* L)
{
    const char *str;
    uint8_t buf[2048];
    size_t len = 0;
    uint32_t FlashAddr;   
    uint8_t mode;    
    
    if (lua_type(L, 1) == LUA_TSTRING)     /* 判断第1个参数 */
    {        
        str = luaL_checklstring(L, 1, &len); /* 1是参数的位置， len是string的长度 */   
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    if (lua_type(L, 2) == LUA_TNUMBER)      /* 判断第2个参数 */
    {
        FlashAddr = luaL_checknumber(L, 2);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }

    if (lua_type(L, 3) == LUA_TNUMBER)      /* 判断第3个参数 */
    {
        mode = luaL_checknumber(L, 3);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }   
    
    /* 将字符换转换为2进制数组 
        "FF 00 12 34"   -> 0xFF,0x00,0x12,0x34
    */
    len = AsciiToHex((char *)str, buf, sizeof(buf));
        
    if (PG_ProgBuf(FlashAddr, buf, len, mode) == 0)
    {    
        lua_pushnumber(L, 1);    /*OK */
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
    }
    return 1;    
}

/*
*********************************************************************************************************
*    函 数 名: h7_ProgBuf_OB
*    功能说明: 开始编程，从新参读入数据。 用于option bytes
*    形    参: Flash地址
*			   数据
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7_ProgBuf_OB(lua_State* L)
{
    const char *str;
    uint8_t buf[2048];
    size_t len = 0;
    uint32_t FlashAddr;   
    
    if (lua_type(L, 1) == LUA_TSTRING)     /* 判断第1个参数 */
    {        
        str = luaL_checklstring(L, 1, &len); /* 1是参数的位置， len是string的长度 */   
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    if (lua_type(L, 2) == LUA_TNUMBER)      /* 判断第2个参数 */
    {
        FlashAddr = luaL_checknumber(L, 2);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    } 
    
    /* 将字符换转换为2进制数组 
        "FF 00 12 34"   -> 0xFF,0x00,0x12,0x34
    */
    len = AsciiToHex((char *)str, buf, sizeof(buf));
        
    if (PG_ProgBuf_OB(FlashAddr, buf, len) == 0)
    {    
        lua_pushnumber(L, 1);    /*OK */
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
    }
    return 1;    
}

/*
*********************************************************************************************************
*    函 数 名: h7_EraseChip
*    功能说明: 擦除全片，用于OPTION BYTES编程，去除读保护。
*    形    参: Flash地址
*			   数据
*    返 回 值: 0 失败   1 成功
*********************************************************************************************************
*/
static int h7_EraseChip(lua_State* L)
{
    uint32_t FlashAddr;     

    if (lua_type(L, 1) == LUA_TNUMBER)      /* 判断第1个参数 */
    {
        FlashAddr = luaL_checknumber(L, 1);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
        
    if (PG_EraseChip(FlashAddr) == 0)
    {    
        lua_pushnumber(L, 1);    /* OK */
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
    }
    return 1;    
}

/*
*********************************************************************************************************
*    函 数 名: h7_Read_ProductSN
*    功能说明: 读产品序号
*    形    参: 无
*    返 回 值: 序号（整数）
*********************************************************************************************************
*/
static int h7_Read_ProductSN(lua_State* L)
{
    lua_pushnumber(L, g_tProgIni.ProductSN);
    return 1;    
}

/*
*********************************************************************************************************
*    函 数 名: h7_Write_ProductSN
*    功能说明: 写产品序号
*    形    参: 无
*    返 回 值: 序号（整数）
*********************************************************************************************************
*/
static int h7_Write_ProductSN(lua_State* L)
{
    uint32_t sn;
    
    if (lua_type(L, 1) == LUA_TNUMBER)      /* 判断第1个参数 */
    {
        sn = luaL_checknumber(L, 1);       
    }
    else
    {
        lua_pushnumber(L, 0);    /* 出错 */
        return 1;
    }
    
    g_tProgIni.ProductSN = sn;    
    lua_pushnumber(L, 1);
    return 1;    
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
