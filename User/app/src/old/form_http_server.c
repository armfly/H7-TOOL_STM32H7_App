/*
*********************************************************************************************************
*
*    模块名称 : web server 演示
*    文件名称 : http_server.c
*    版    本 : V1.0
*    说    明 : 提供WEB服务器功能。主要目的是测试DM9000AEP网卡电路和DM9161 PHY电路。
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2013-01-01 armfly  正式发布
*
*    Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stdio.h"
#include "bsp.h"
#include "num_pad.h"
#include "param.h"

/* for LwIP */
//#include "netconf.h"
//#include "main_lwip.h"
//#include "httpd_w.h"
#include "lwip/opt.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"

#include "ethernetif.h"
#include "app_ethernet.h"
#include "http_cgi_ssi.h"

void lwip_start(void);
void lwip_pro(void);

/*
*********************************************************************************************************
*    函 数 名: WebServer
*    功能说明: web服务器程序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void WebServer(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t ucTouch;     /* 触摸事件 */
    int16_t tpX, tpY;
    uint16_t x, y;
    uint16_t usLineCap = 18;
    char buf[128];
    uint8_t fRefresh;
    FONT_T tFont; /* 定义字体结构体变量 */

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;                 /* 字体代码 16点阵 */
        tFont.FrontColor = CL_BLACK;             /* 字体颜色 */
        tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
        tFont.Space = 0;                                     /* 文字间距，单位 = 像素 */

        LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

        LCD_DispStr(5, 3, "逻辑分析仪示波器", &tFont);
    }

    /* LwIP 初始化 */
    {
        /* 如果不插网线，此函数执行时间过长 */
        /* 网络参数存在在全局变量 g_tParam.lwip_ip, g_tParam.lwip_net_mask, g_tParam.lwip_gateway */
        lwip_start();
    }

    LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

    fRefresh = 1;
    while (1)
    {
        bsp_Idle();

        uip_pro();
        lwip_pro();

        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;
        }

        if (PHYLinkChanged == 1) /* DM9162联网状态变化 */
        {
            PHYLinkChanged = 0;

            fRefresh = 1;
        }

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_DOWN_S: /* S键 */
                break;

            case KEY_DOWN_C: /* C键 */
                break;

            default:
                break;
            }
        }
    }
}

/*
*********************************************************************************************************
*    下面是 LwIP 部分的代码
*********************************************************************************************************
*/

struct netif gnetif;

/*
*********************************************************************************************************
*    函 数 名: uip_ChangeNetParam
*    功能说明: 重新设置网络参数， 网络参数存在在全局变量 g_tParam.uip_ip, g_tParam.uip_net_mask, 
*              g_tParam.uip_gateway 
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void lwip_ChangeNetParam(void)
{
    //    //uip_ipaddr(ipaddr, 192,168,1,10);    /* 缺省IP */
    //    uip_sethostaddr((uint16_t *)g_tParam.uip_ip);

    //    //uip_ipaddr(ipaddr, 255,255,255,0);    /* 地址掩码 */
    //    uip_setnetmask((uint16_t *)g_tParam.uip_net_mask);

    //    //uip_ipaddr(ipaddr, 192,168,1,1);    /* 默认网关 */
    //    uip_setdraddr((uint16_t *)g_tParam.uip_gateway);
}

/**
  * @brief  Setup the network interface
  * @param  None
  * @retval None
  */
/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0 ((uint8_t)192U)
#define IP_ADDR1 ((uint8_t)168U)
#define IP_ADDR2 ((uint8_t)1U)
#define IP_ADDR3 ((uint8_t)86U)

/*NETMASK*/
#define NETMASK_ADDR0 ((uint8_t)255U)
#define NETMASK_ADDR1 ((uint8_t)255U)
#define NETMASK_ADDR2 ((uint8_t)255U)
#define NETMASK_ADDR3 ((uint8_t)0U)

/*Gateway Address*/
#define GW_ADDR0 ((uint8_t)192U)
#define GW_ADDR1 ((uint8_t)168U)
#define GW_ADDR2 ((uint8_t)1U)
#define GW_ADDR3 ((uint8_t)1U)

static void Netif_Config(void)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

#if LWIP_DHCP
    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
#else

    /* IP address default setting */
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

#endif

    /* add the network interface */
    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

    /*  Registers the default network interface */
    netif_set_default(&gnetif);

    ethernet_link_status_updated(&gnetif);

#if LWIP_NETIF_LINK_CALLBACK
    netif_set_link_callback(&gnetif, ethernet_link_status_updated);
#endif
}

/*
*********************************************************************************************************
*    函 数 名: lwip_start
*    功能说明: 启动lwip_start,  网络参数存在在全局变量 g_tParam.lwip_ip, g_tParam.lwip_net_mask, 
*              g_tParam.lwip_gateway 
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void lwip_start(void)
{
    /* Initialize the LwIP stack */
    lwip_init();

    /* Configure the Network interface */
    Netif_Config();

    /* Http webserver Init */
    http_server_init();
}

/*
*********************************************************************************************************
*    函 数 名: lwip_pro
*    功能说明: lwip 轮询，插入到主循环中
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void lwip_pro(void)
{
    /* Read a received packet from the Ethernet buffers and send it 
       to the lwIP for handling */
    ethernetif_input(&gnetif);

    /* Handle timeouts */
    sys_check_timeouts();

#if LWIP_NETIF_LINK_CALLBACK
    Ethernet_Link_Periodic_Handle(&gnetif);
#endif

#if LWIP_DHCP
    DHCP_Periodic_Handle(&gnetif);
#endif
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
