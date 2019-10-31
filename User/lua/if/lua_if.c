#include "lua_if.h"
#include "bsp.h"
#include "param.h"

/* 
  lua 增加调试代码的方法:

lobject.c文件:
  const char *luaO_pushfstring (lua_State *L, const char *fmt, ...) {
  #if 1		
    printf("%s", msg);
  #endif

ldebug.c 文件 luaG_runerror 函数，增加printf

lauxlib.c 文件 luaL_error函数，增加printf

ldo.c 文件 luaD_throw 函数	printf("\r\nthrow errcode=%d\r\n", errcode);

lua.h 定义错误代码
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRGCMM	5
#define LUA_ERRERR	6

*/

/*
  luaconf.h 文件对浮点和整数的处理。 缺省64位整数，双精度浮点
    default configuration for 64-bit Lua ('long long' and 'double')
*/

lua_State *g_Lua = 0;

char s_lua_prog_buf[LUA_PROG_LEN_MAX];
uint32_t s_lua_prog_len;
uint32_t s_lua_func_init_idx;
uint32_t s_lua_func_write_idx;
uint32_t s_lua_func_read_idx;

uint8_t s_lua_read_buf[LUA_READ_LEN_MAX];
uint8_t s_lua_read_len;

static int get_runtime(lua_State *L);
static int check_runtime(lua_State *L);

static void lua_RegisterFunc(void);

void exit(int status)
{
  ;
}

int system(const char *cmd)
{
  return 0;
}

/* time_t : date/time in unix secs past 1-Jan-70 */
time_t time(time_t *_t)
{
/* 以下代码来自于： https://blog.csdn.net/qq_29350001/article/details/87637350 */
#define xMINUTE (60)         /* 1分的秒数 */
#define xHOUR (60 * xMINUTE) /* 1小时的秒数 */
#define xDAY (24 * xHOUR)    /* 1天的秒数 */
#define xYEAR (365 * xDAY)   /* 1年的秒数 */

  /* 将localtime（UTC+8北京时间）转为UNIX TIME，以1970年1月1日为起点 */
  static unsigned int month[12] =
      {
          /*01月*/ xDAY * (0),
          /*02月*/ xDAY * (31),
          /*03月*/ xDAY * (31 + 28),
          /*04月*/ xDAY * (31 + 28 + 31),
          /*05月*/ xDAY * (31 + 28 + 31 + 30),
          /*06月*/ xDAY * (31 + 28 + 31 + 30 + 31),
          /*07月*/ xDAY * (31 + 28 + 31 + 30 + 31 + 30),
          /*08月*/ xDAY * (31 + 28 + 31 + 30 + 31 + 30 + 31),
          /*09月*/ xDAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
          /*10月*/ xDAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
          /*11月*/ xDAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
          /*12月*/ xDAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)};
  unsigned int seconds = 0;
  unsigned int year = 0;

  RTC_ReadClock();
  year = g_tRTC.Year - 1970;                        /* 不考虑2100年千年虫问题 */
  seconds = xYEAR * year + xDAY * ((year + 1) / 4); /* 前几年过去的秒数 */
  seconds += month[g_tRTC.Mon - 1];                 /* 加上今年本月过去的秒数 */
  if ((g_tRTC.Mon > 2) && (((year + 2) % 4) == 0))  /* 2008年为闰年 */
    seconds += xDAY;                                /* 闰年加1天秒数 */
  seconds += xDAY * (g_tRTC.Day - 1);               /* 加上本天过去的秒数 */
  seconds += xHOUR * g_tRTC.Hour;                   /* 加上本小时过去的秒数 */
  seconds += xMINUTE * g_tRTC.Min;                  /* 加上本分钟过去的秒数 */
  seconds += g_tRTC.Sec;                            /* 加上当前秒数<br>　seconds -= 8 * xHOUR; */
  *_t = seconds;
  return *_t;
}

/*
print(\"Hello,I am lua!\\n--this is newline printf\")
function foo()
  local i = 0
  local sum = 1
    while i <= 10 do
         sum = sum * 2
         i = i + 1
    end
return sum
 end
print(\"sum =\", foo())
print(\"and sum = 2^11 =\", 2 ^ 11)
print(\"exp(200) =\", math.exp(200))
*/
const char lua_test[] = {
    "print(\"Hello,I am lua!\\n--this is newline printf\")\n"
    "function foo()\n"
    "  local i = 0\n"
    "  local sum = 1\n"
    "  while i <= 10 do\n"
    "    sum = sum * 2\n"
    "    i = i + 1\n"
    "  end\n"
    "return sum\n"
    "end\n"
    "print(\"sum =\", foo())\n"
    "print(\"and sum = 2^11 =\", 2 ^ 11)\n"
    "print(\"exp(200) =\", math.exp(200))\n"};

void lua_Test(void)
{
  luaL_dostring(g_Lua, lua_test); /* 运行Lua脚本 */

  luaL_dostring(g_Lua, "print(add_f(1.0, 9.09))\n print(sub_f(20.1,19.01))");
}

/*
*********************************************************************************************************
*	函 数 名: lua_Init
*	功能说明: 初始化lua虚拟机
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void lua_Init(void)
{
  g_Lua = luaL_newstate(); /* 建立Lua运行环境 */
  luaL_openlibs(g_Lua);
  luaopen_base(g_Lua);

  lua_RegisterFunc(); /* 注册c函数，供lua调用 */
}

/* 关闭释放Lua */
void lua_DeInit(void)
{
  lua_close(g_Lua); /* 释放内存 */
  g_Lua = 0;
}

void lua_DownLoad(uint32_t _addr, uint8_t *_buf, uint32_t _len, uint32_t _total_len)
{
  uint32_t i;

  for (i = 0; i < _len; i++)
  {
    if (_addr < LUA_PROG_LEN_MAX)
    {
      s_lua_prog_buf[_addr + i] = _buf[i];
    }
  }

  s_lua_prog_len = _total_len;
  s_lua_prog_buf[s_lua_prog_len] = 0;

  if (g_Lua > 0)
  {
    lua_DeInit();
  }
  lua_Init();

  //luaL_dostring(g_Lua, s_lua_prog_buf);
}

void lua_Poll(void)
{
  if (g_tVar.LuaRunOnce == 1)
  {
    g_tVar.LuaRunOnce = 0;
    luaL_dostring(g_Lua, s_lua_prog_buf);
  }
}

void lua_DoInit(void)
{
  luaL_dostring(g_Lua, "init()");
}

// 通信写文件
uint8_t lua_66H_Write(uint32_t _addr, uint8_t *_buf, uint32_t _len)
{
  uint8_t re;

  lua_getglobal(g_Lua, "write"); // 函数入栈 获取lua函数write
  lua_pushinteger(g_Lua, _addr);
  lua_pushlstring(g_Lua, (char *)_buf, _len);
  lua_pushinteger(g_Lua, _len);

  lua_pcall(g_Lua, 3, 1, 0);
  /*
    lua_pcall(lua_State *L,int nargs,int nresults,int errfunc)
    1
    nargs 参数个数
    nresults 返回值个数
    errFunc 错误处理函数，0表示无，表示错误处理函数在栈中的索引
  */
  re = lua_tonumber(g_Lua, -1);
  lua_pop(g_Lua, 1);
  return re;
}

// 通信读文件
uint8_t lua_67H_Read(uint32_t _addr, uint8_t *_buf, uint32_t _len)
{
  uint8_t re = 0;
  uint32_t i;

  lua_getglobal(g_Lua, "read"); // 函数入栈 获取lua函数write
  lua_pushinteger(g_Lua, _addr);
  lua_pushinteger(g_Lua, _len);

  lua_pcall(g_Lua, 2, 1, 0);
  /*
    lua_pcall(lua_State *L,int nargs,int nresults,int errfunc)
    1
    nargs 参数个数
    nresults 返回值个数
    errFunc 错误处理函数，0表示无，表示错误处理函数在栈中的索引
  */

  for (i = 0; i < _len; i++)
  {
    _buf[i] = s_lua_read_buf[i];
  }

  return re;
}

/*
*********************************************************************************************************
*	函 数 名: beep
*	功能说明: 蜂鸣函数 lua调用
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int beep(lua_State *L)
{
  //检查栈中的参数是否合法，1表示Lua调用时的第一个参数(从左到右)，依此类推。
  //如果Lua代码在调用时传递的参数不为number，该函数将报错并终止程序的执行。
  //    double op1 = luaL_checknumber(L, 1);
  //    double op2 = luaL_checknumber(L, 2);

  BEEP_KeyTone();

  //将函数的结果压入栈中。如果有多个返回值，可以在这里多次压入栈中。
  //lua_pushnumber(L, op1 + op2);

  //返回值用于提示该C函数的返回值数量，即压入栈中的返回值数量。
  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: delayus
*	功能说明: 微秒延迟 lua函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int delayus(lua_State *L)
{
  uint32_t n = luaL_checknumber(L, 1);

  bsp_DelayUS(n);
  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: delayms
*	功能说明: 毫秒延迟 lua函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int delayms(lua_State *L)
{
  uint32_t n = luaL_checknumber(L, 1);

  bsp_DelayMS(n);
  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: printhex
*	功能说明: 打印hex格式.  printhex(100, 2);	printhex("123");	
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
extern uint8_t USBCom_SendBuf(int _Port, uint8_t *_Buf, uint16_t _Len);
extern void lua_udp_SendBuf(uint8_t *_buf, uint16_t _len, uint16_t _port);
static int printhex(lua_State *L)
{
  if (lua_type(L, 1) == LUA_TSTRING) /* 判断第1个参数 */
  {
    const char *data;
    size_t len;

    data = luaL_checklstring(L, 1, &len); /* 1是参数的位置， len是string的长度 */
#if PRINT_TO_UDP == 1
    lua_udp_SendBuf((uint8_t *)data, len, LUA_UDP_PORT);
#else
    USBCom_SendBuf(1, (uint8_t *)data, len);
#endif
  }

  if (lua_type(L, 1) == LUA_TNUMBER) /* 判断第1个参数 */
  {
    char buf[32];
    uint32_t num;
    uint32_t bytes;

    num = luaL_checknumber(L, 1);
    if (lua_type(L, 2) == LUA_TNUMBER) /* 判断第2个参数 */
    {
      bytes = luaL_checknumber(L, 2);
      if (bytes == 1)
      {
        sprintf(buf, "0x%02X\r\n", num);
      }
      else if (bytes == 2)
      {
        sprintf(buf, "0x%04X\r\n", num);
      }
      else if (bytes == 3)
      {
        sprintf(buf, "0x%06X\r\n", num);
      }
      else if (bytes == 4)
      {
        sprintf(buf, "0x%08X\r\n", num);
      }
      else
      {
        sprintf(buf, "0x%X\r\n", num);
      }
    }
    else
    {
      sprintf(buf, "%X\r\n", num);
    }

#if PRINT_TO_UDP == 1
    lua_udp_SendBuf((uint8_t *)buf, strlen(buf), LUA_UDP_PORT);
#else
    USBCom_SendBuf(1, (uint8_t *)buf, strlen(buf));
#endif
  }
  return 1;
}

/*
*********************************************************************************************************
*	函 数 名: write_clock
*	功能说明: 写时钟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int write_clock(lua_State *L)
{
  uint16_t year;
  uint8_t mon, day, hour, min, sec;

  if (lua_type(L, 1) == LUA_TNUMBER) /* 判断第1个参数 */
  {
    year = luaL_checknumber(L, 1);
  }
  if (lua_type(L, 2) == LUA_TNUMBER)
  {
    mon = luaL_checknumber(L, 2);
  }
  if (lua_type(L, 3) == LUA_TNUMBER)
  {
    day = luaL_checknumber(L, 3);
  }
  if (lua_type(L, 4) == LUA_TNUMBER)
  {
    hour = luaL_checknumber(L, 4);
  }
  if (lua_type(L, 5) == LUA_TNUMBER)
  {
    min = luaL_checknumber(L, 5);
  }
  if (lua_type(L, 6) == LUA_TNUMBER)
  {
    sec = luaL_checknumber(L, 6);
  }

  RTC_WriteClock(year, mon, day, hour, min, sec);
  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: read_clock
*	功能说明: 写时钟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int read_clock(lua_State *L)
{
  static uint8_t rtc_buf[8];

  RTC_ReadClock();

  rtc_buf[0] = g_tRTC.Year >> 8;
  rtc_buf[1] = g_tRTC.Year;
  rtc_buf[2] = g_tRTC.Mon;
  rtc_buf[3] = g_tRTC.Day;
  rtc_buf[4] = g_tRTC.Hour;
  rtc_buf[5] = g_tRTC.Min;
  rtc_buf[6] = g_tRTC.Sec;
  rtc_buf[7] = g_tRTC.Week;

  lua_pushlstring(L, (const char *)rtc_buf, 8);
  return 1;
}

/*
*********************************************************************************************************
*	函 数 名: get_runtime
*	功能说明: 获得CPU运行时间。ms单位
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int get_runtime(lua_State *L)
{
  int32_t runtime;

  runtime = bsp_GetRunTime();
  lua_pushnumber(L, runtime);
  return 1;
}

/*
*********************************************************************************************************
*	函 数 名: check_runtime
*	功能说明: 判断时间长度
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int check_runtime(lua_State *L)
{
  int32_t lasttime;
  uint8_t re;

  if (lua_type(L, 1) == LUA_TNUMBER) /* 判断第1个参数 */
  {
    lasttime = luaL_checknumber(L, 1);
  }

  re = bsp_CheckRunTime(lasttime);

  lua_pushnumber(L, re);
  return 1;
}

/*
*********************************************************************************************************
*	函 数 名: lua_RegisterFunc
*	功能说明: 注册lua可调用的c函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void lua_RegisterFunc(void)
{
  //将指定的函数注册为Lua的全局函数变量，其中第一个字符串参数为Lua代码
  //在调用C函数时使用的全局函数名，第二个参数为实际C函数的指针。
  lua_register(g_Lua, "beep", beep);
  lua_register(g_Lua, "delayus", delayus);
  lua_register(g_Lua, "delayms", delayms);
  lua_register(g_Lua, "printhex", printhex);
  lua_register(g_Lua, "write_clock", write_clock);
  lua_register(g_Lua, "read_clock", read_clock);
  lua_register(g_Lua, "get_runtime", get_runtime);
  lua_register(g_Lua, "check_runtime", check_runtime);

  /* 注册接口函数 */
  lua_gpio_RegisterFun();
  lua_i2c_RegisterFun();
  lua_spi_RegisterFun();
  lua_tim_RegisterFun();
  lua_tcp_RegisterFun();
  lua_qspi_RegisterFun();
  lua_fatfs_RegisterFun();
  lua_swd_RegisterFun();

  lua_adc_RegisterFun();
  lua_dac_RegisterFun();
  lua_reg_RegisterFun();
  lua_extio_RegisterFun();
}
