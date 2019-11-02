#include "lauxlib.h"
#include "lualib.h"
#include "time.h"
#include "lua_if.h"
#include "bsp.h"

#include "target_reset.h"
#include "target_config.h"
#include "swd_host.h"
#include "Systick_Handler.h"

/* 为了避免和DAP驱动中的函数混淆，本模块函数名前缀用 h7swd */

static int h7swd_Init(lua_State* L);
static int h7swd_ReadID(lua_State* L);
static int h7swd_WriteMemory(lua_State* L);
static int h7swd_ReadMemory(lua_State* L);

/*
*********************************************************************************************************
*    函 数 名: lua_swd_RegisterFun
*    功能说明: 注册lua C语言接口函数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void lua_swd_RegisterFun(void)
{
    //将指定的函数注册为Lua的全局函数变量，其中第一个字符串参数为Lua代码
    //在调用C函数时使用的全局函数名，第二个参数为实际C函数的指针。
    lua_register(g_Lua, "swd_init", h7swd_Init);    
    lua_register(g_Lua, "swd_getid", h7swd_ReadID);
    lua_register(g_Lua, "swd_write", h7swd_WriteMemory);    
    lua_register(g_Lua, "swd_read", h7swd_ReadMemory);
}
            
/*
*********************************************************************************************************
*    函 数 名: h7swd_Init
*    功能说明: 读芯片ID
*    形    参: vcc : CPU供电电压
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
    bsp_DelayUS(100 * 100);        /* 延迟100ms */
        
    swd_init_debug();            /* 进入swd debug状态 */
        
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
        data = luaL_checklstring(L, 2, &len); /* 1是参数的位置， len是stri        的长度 */        
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
        addr = luaL_checknumber(L, 1); /* 1是参数的位置， len是stri              的长度 */        
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
    return 1;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
