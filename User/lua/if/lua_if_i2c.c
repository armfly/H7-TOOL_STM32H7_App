#include "lauxlib.h"
#include "lualib.h"
#include "time.h"
#include "lua_if.h"
#include "bsp.h"

static int lua_I2C_Start(lua_State* L);
static int lua_I2C_Stop(lua_State* L);
static int lua_I2C_SendBytes(lua_State* L);
static int lua_I2C_ReciveBytes(lua_State* L);

void lua_i2c_RegisterFun(void)
{
    //将指定的函数注册为Lua的全局函数变量，其中第一个字符串参数为Lua代码
    //在调用C函数时使用的全局函数名，第二个参数为实际C函数的指针。
    lua_register(g_Lua, "i2c_start", lua_I2C_Start);
    lua_register(g_Lua, "i2c_stop", lua_I2C_Stop);
    lua_register(g_Lua, "i2c_send", lua_I2C_SendBytes);
    lua_register(g_Lua, "i2c_recive", lua_I2C_ReciveBytes);
}

static int lua_I2C_Start(lua_State* L)
{
    i2c_Start();
    return 1;
}

static int lua_I2C_Stop(lua_State* L)
{
    i2c_Stop();
    return 1;
}

/*
    形式1: i2c_send(buf);
    形式2: i2c_send(3    );    
*/
static int lua_I2C_SendBytes(lua_State* L)
{
    size_t i;
    size_t len;
    const char *data;
    uint8_t buf[1];
    int re;

    if (lua_type(L, 1) == LUA_TSTRING)     /* 判断第1个参数 */
    {        
        data = luaL_checklstring(L, 1, &len); /* 1是参数的位置， len是string 的长度 */        
    }
    
    if (lua_type(L, 1) == LUA_TNUMBER) /* 判断第1个参数 */
    {
        len = 1;
        buf[0] = luaL_checknumber(L, 1);
        
        data = (const char *)buf;
    }
    
    re = 1;
    for (i = 0; i < len; i++)
    {
        i2c_SendByte(data[i]);    
        if (i2c_WaitAck() != 0)
        {
            i2c_Stop();
            re = 0;
            break;
        }        
    }
    lua_pushnumber(L, re);    /* 返回值 */
    return 1;
}

/*
Lua给C++传buffer时，使用string就行，再C++的，tolua++中使用下面代码读取buffer
        size_t ld;
        const char *data = luaL_checklstring(tolua_S, 2, &ld); // 2是参数的位置， ld是buffer的长度
C++给Lua传buffer时，在C++代码中使用下面代码传入buffer
            LuaStack *stack = LuaEngine::getInstance()->getLuaStack();
            stack->

*/
// lua_I2C_ReciveBytes(2);
static int lua_I2C_ReciveBytes(lua_State* L)
{
    size_t len;
    size_t i;
    
    len = luaL_checknumber(L, 1);
    if (len == 0 || len > LUA_READ_LEN_MAX)
    {
        lua_pushnumber(L, 0);    /* 返回值 */
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        s_lua_read_buf[i] = i2c_ReadByte();    /* 读1个字节 */

        /* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
        if (i != len - 1)
        {
            i2c_Ack();    /* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
        }
        else
        {
            i2c_NAck();    /* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
        }
    }
    
    //lua_pushnumber(L, 1);    /* 返回值 */
//    strcpy(s_lua_read_buf, "s_lua_read_buf");
    lua_pushlstring(L, (char *)s_lua_read_buf, len); 
    return 1;
}







