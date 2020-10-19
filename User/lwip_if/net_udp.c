/*
*********************************************************************************************************
*
*    模块名称 : UDP搜索模块
*    文件名称 : net_udp.c
*    版    本 : V1.0
*    说    明 : 使用UDP广播，搜索局域网内的设备，并配置IP等参数
*
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2016-11-22  armfly  正式发布
*
*    Copyright (C), 2016-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "lwip/udp.h"
#include "net_udp.h"
#include "param.h"
#include "modbus_slave.h"

#define MODS_ADDR 0x01 /* MODBSU 从站地址 */

#define UDP_TX_SIZE (1500 + 8)

struct udp_pcb *g_udp_pcb;
struct pbuf *p_udp_tx;

static uint8_t udp_tx_buf[UDP_TX_SIZE];
//static uint16_t udp_tx_len;

//typedef void (*udp_recv_fn)(void *arg, struct udp_pcb *pcb, struct pbuf *p,
//    const ip_addr_t *addr, u16_t port);

static void udp_server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p_rx, const ip_addr_t *addr, u16_t port);

ip_addr_t destAddr;

#define PRINT_FIFO_SIZE 1024
static uint8_t udp_print_buf[PRINT_FIFO_SIZE];
static uint16_t udp_print_putp = 0;
static uint16_t udp_print_getp = 0;
static uint16_t udp_print_count = 0;

void udp_print_put(uint8_t ch)
{ 
//    while (udp_print_count == PRINT_FIFO_SIZE - 1)
//    {
//        bsp_Idle();
//    }
    
    udp_print_buf[udp_print_putp] = ch;
    if (++udp_print_putp >= PRINT_FIFO_SIZE)
    {
        udp_print_putp = 0;
    }
    udp_print_count++;
}

uint8_t udp_print_get(uint8_t *ch)
{
    uint8_t data;

    if (udp_print_putp != udp_print_getp)
    {
        data = udp_print_buf[udp_print_getp];
        if (++udp_print_getp >= PRINT_FIFO_SIZE)
        {
            udp_print_getp = 0;
        }
        *ch = data;
        
        if (udp_print_count > 0)
        {
            udp_print_count--;
        }
        return 1;
    }
    return 0;
}

void udp_print_send(void)
{
    uint16_t len;
    uint8_t data;

    len = 0;
    while (1)
    {
        if (udp_print_get(&data))
        {
            udp_tx_buf[len] = data;

            if (++len >= UDP_TX_SIZE)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    /* 准备应答数据 */
    p_udp_tx->payload = udp_tx_buf;
    p_udp_tx->len = len;
    p_udp_tx->tot_len = len;

    udp_sendto(g_udp_pcb, p_udp_tx, &destAddr, LUA_UDP_PORT); /* 数据发送出去 */
    
    //ERR_OK
}

/*
*********************************************************************************************************
*    函 数 名: udp_SendBuf
*    功能说明: 发送UDP数据包，用于lua. 先缓存再发送.超时汇总后集中发送
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void lua_udp_SendBuf(uint8_t *_buf, uint16_t _len, uint16_t _port)
{
    uint16_t i;
    uint8_t nowsend = 0;

    for (i = 0; i < _len; i++)
    {
        udp_print_put(_buf[i]);
        if (_buf[i] == 0x0A)
        {
            nowsend = 1;
        }
    }
    
    if (nowsend == 1)
    {
        udp_print_send();
    }
    else
    {
        bsp_StartHardTimer(3, 5000, udp_print_send);
    }
}

/*
*********************************************************************************************************
*    函 数 名: UDP_server_init
*    功能说明: 创建UDP服务器，端口号固定 30010
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void udp_server_init(void)
{
    g_udp_pcb = udp_new(); //申请udp控制块
    //udp_bind(pcb,IP_ADDR_ANY,UDP_LOCAL_PORT);

    p_udp_tx = pbuf_alloc(PBUF_RAW, sizeof(udp_tx_buf), PBUF_RAM); // 按照指定类型分配一个pbuf结构体  // struct pbuf *p_tx;
    p_udp_tx->payload = (void *)udp_tx_buf;

    //g_udp_pcb->so_options |= SOF_BROADCAST;
    udp_bind(g_udp_pcb, IP_ADDR_ANY, 30010);        /* 绑定本地IP地址和端口号（作为udp服务器） */
    udp_recv(g_udp_pcb, udp_server_recv, NULL); /* 设置UDP段到时的回调函数 */
}

/*
*********************************************************************************************************
*    函 数 名: udp_server_recv
*    功能说明: 接收到UDP数据包的回调函数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
//static void udp_server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p_rx, struct ip_addr *addr, u16_t port)
static void udp_server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p_rx, const ip_addr_t *addr, u16_t port)
{
    // 2019-07-03 destAddr改为全局变量
    //ip_addr_t destAddr = *addr;     /* 获取远程主机 IP地址 */
    destAddr = *addr;

    if (p_rx != NULL)
    {
        // EIO_SetOutLevel(EIO_D0, 1);   测试高脉冲时间 40us

        /* 分析UDP数据包 */
        {
            const uint8_t mac_ff[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            uint16_t i;

            g_tModS.TCP_Flag = 0;
            if (p_rx->len >= 8)
            {
                /* 6字节MAC地址， 0xFFFFFF 是广播MAC. */
                if (memcmp(p_rx->payload, mac_ff, 6) == 0 || memcmp(p_rx->payload, &g_tVar.MACaddr, 6) == 0)
                {
                    /* 带MAC前缀的udp数据包 */
                    MODS_Poll((uint8_t *)p_rx->payload + 6, p_rx->len - 6);

                    for (i = 0; i < g_tModS.TxCount; i++)
                    {
                        g_tModS.TxBuf[g_tModS.TxCount - i - 1 + 6] = g_tModS.TxBuf[g_tModS.TxCount - i - 1];
                    }

                    //MODS_Analyze((uint8_t *)p_rx->payload + 6, p_rx->len - 6, &udp_tx_buf[6], &udp_tx_len);    /* 分析MODBUS数据帧 */
                    if (g_tModS.TxCount > 0)
                    {
                        memcpy(g_tModS.TxBuf, &g_tVar.MACaddr, 6); /* 本机MAC放到应答数据包前缀 */
                        g_tModS.TxCount += 6;
                    }

                    IP4_ADDR(&destAddr, 255, 255, 255, 255); //设置网络接口的ip地址
                }
                else /* 不带MAC前缀 */
                {
                    MODS_Poll(p_rx->payload, p_rx->len); /* 分析MODBUS数据帧 */
                }
            }
        }

        if (g_tModS.TxCount > 0)
        {
            /* 准备应答数据 */
            p_udp_tx->payload = (void *)g_tModS.TxBuf;
            p_udp_tx->len = g_tModS.TxCount;
            p_udp_tx->tot_len = g_tModS.TxCount;

            udp_sendto(pcb, p_udp_tx, &destAddr, port); /* 发送数据 */

            // EIO_SetOutLevel(EIO_D0, 0);  时间测试
        }

        pbuf_free(p_rx); /* 释放该UDP段 */
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
