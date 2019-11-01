
/****************************************************************************
* Copyright (C), 2009-2010, www.armfly.com  安富莱电子
*
* 【本驱动在安富莱STM32F103ZE-EK开发板上调试通过             】
* 【QQ: 1295744630, 旺旺：armfly, Email: armfly@qq.com       】
*
* 文件名: dm9k_uip.c
* 内容简述: Davicom DM9000A uP NIC fast Ethernet driver for uIP.
*
* 文件历史:
* 版本号  日期       作者    说明
* v0.1    2010-01-18 armfly  创建该文件
*
*/

//#define DM9000A_FLOW_CONTROL
//#define DM9000A_UPTO_100M
//#define Fifo_Point_Check
//#define Point_Error_Reset
#define Fix_Note_Address

#include "bsp.h"
#include "uip.h"
#include "dm9k_uip.h"

/* DM9000A 接收函数设置宏 */
//#define Rx_Int_enable
#define Max_Int_Count            1
#define Max_Ethernet_Lenth        1536
#define Broadcast_Jump
#define Max_Broadcast_Lenth        500

/* DM9000A 传送函数设置宏 */
#define Max_Send_Pack            2

/* 系统全域的设置宏
#undef    uint8_t
#define   uint8_t                unsigned char
#undef    uint16_t
#define   uint16_t                unsigned short
#undef    UINT32
#define   UINT32                unsigned long
*/
#define   NET_BASE_ADDR            0x60002000
//#define   NET_BASE_ADDR            0x64004000
#define   NET_REG_ADDR            (*((volatile uint16_t *) NET_BASE_ADDR))
#define   NET_REG_DATA            (*((volatile uint16_t *) (NET_BASE_ADDR + 0x00000004)))

//#define FifoPointCheck

/*=============================================================================
  系统全域的变量
  =============================================================================*/
#define ETH_ADDR_LEN            6

static unsigned char DEF_MAC_ADDR[ETH_ADDR_LEN] =
    {0x00, 0x60, 0x6e, 0x90, 0x00, 0xae};
uint8_t  SendPackOk = 0;

uint8_t s_FSMC_Init_Ok = 0;    /* 用于指示FSMC是否初始化 */

//#define printk(...)
#define printk printf

void dm9k_debug_test(void);
void dm9k_udelay(uint16_t time);

static void DM9K_CtrlLinesConfig(void);
static void DM9K_FSMCConfig(void);

/*******************************************************************************
*    函数名: dm9k_udelay
*    参  数: time ： 延迟时间，不精确，us级别
*    返  回: 无
*    功  能: 延迟函数
*/
void dm9k_udelay(uint16_t time)
{
//    uint16_t i,k;

//    for (i = 0; i < time; i++)
//    {
//        for (k = 0; k < 80; k++);
//    }
//    while(time--);
    bsp_DelayUS(time);
}

/*******************************************************************************
*    函数名: ior
*    参  数: reg ：寄存器地址
*    返  回: 无
*    功  能: 读出寄存器的值
*/
uint8_t ior(uint8_t reg)
{
    NET_REG_ADDR = reg;
    return (NET_REG_DATA);
}

/*******************************************************************************
*    函数名: iow
*    参  数: reg ：寄存器地址
*            writedata : 写入的数据
*    返  回: 无
*    功  能: 写DM9000AE寄存器的值
*/
void iow(uint8_t reg, uint8_t writedata)
{
    NET_REG_ADDR = reg;
    NET_REG_DATA = writedata;
}

/*******************************************************************************
*    函数名: dm9k_hash_table
*    参  数: 无
*    返  回: 无
*    功  能: 设置 DM9000A MAC 、 广播 、 多播 寄存器
*/
void dm9k_hash_table(void)
{
    uint8_t i;

    /* 将MAC地址告诉uip */
    for (i = 0; i < 6; i++)
    {
        uip_ethaddr.addr[i] = DEF_MAC_ADDR[i];
    }

    /* 设置 网卡 MAC 位置，来自於 MyHardware */
    for(i = 0; i < 6; i++)
        iow(DM9000_REG_PAR + i, DEF_MAC_ADDR[i]);

    for(i = 0; i < 8; i++)                                 /* 清除 网卡多播设置 */
        iow(DM9000_REG_MAR + i, 0x00);
    iow(DM9000_REG_MAR + 7, 0x80);                      /* 速设置 广播包 设置 */
}

/*******************************************************************************
*    函数名: dm9k_reset
*    参  数: 无
*    返  回: 无
*    功  能: 对DM9000AE进行软件复位
*/
void dm9k_reset(void)
{
    iow(DM9000_REG_NCR, DM9000_REG_RESET);            /* 对 DM9000A 进行软件重置 */
    dm9k_udelay(10);                                /* delay 10us */
    iow(DM9000_REG_NCR, DM9000_REG_RESET);            /* 对 DM9000A 进行软件重置 */
    dm9k_udelay(10);                                /* delay 10us */

    /* 基本记存器相关设置 */
    iow(DM9000_REG_IMR, DM9000_IMR_OFF);             /* 开启内存自环模式 */
    iow(DM9000_REG_TCR2, DM9000_TCR2_SET);            /* 设置 LED 显示模式1:全双工亮，半双工灭 */

    /* 清除多余资讯 */
    iow(DM9000_REG_NSR, 0x2c);
    iow(DM9000_REG_TCR, 0x00);
    iow(DM9000_REG_ISR, 0x3f);

#ifdef DM9000A_FLOW_CONTROL
    iow(DM9000_REG_BPTR, DM9000_BPTR_SET);            /* 半双工流控设置 */
    iow(DM9000_REG_FCTR, DM9000_FCTR_SET);            /* 全双工流控设置 */
    iow(DM9000_REG_FCR, DM9000_FCR_SET);            /* 开启流控设置 */
#endif

#ifdef DM9000A_UPTO_100M
    /* DM9000A无此寄存器 */
    iow(DM9000_REG_OTCR, DM9000_OTCR_SET);            /* 工作频率到 100Mhz 设置 */
#endif

#ifdef  Rx_Int_enable
    iow(DM9000_REG_IMR, DM9000_IMR_SET);            /* 开启 中断模式 */
#else
    iow(DM9000_REG_IMR, DM9000_IMR_OFF);            /* 关闭 中断模式 */
#endif

    iow(DM9000_REG_RCR, DM9000_RCR_SET);            /* 开启 接收工能 */

    SendPackOk = 0;
}

/*******************************************************************************
*    函数名: dm9k_phy_write
*    参  数: phy_reg ： 寄存器地址
*            writedata ： 写入的数据
*    返  回: 无
*    功  能: 写DM9000A PHY 寄存器
*/
void dm9k_phy_write(uint8_t phy_reg, uint16_t writedata)
{
    /* 设置写入 PHY 寄存器的位置 */
    iow(DM9000_REG_EPAR, phy_reg | DM9000_PHY);

    /* 设置写入 PHY 寄存器的值 */
    iow(DM9000_REG_EPDRH, ( writedata >> 8 ) & 0xff);
    iow(DM9000_REG_EPDRL, writedata & 0xff);

    iow(DM9000_REG_EPCR, 0x0a);                         /* 将资料写入 PHY 寄存器 */
    while(ior(DM9000_REG_EPCR) & 0x01);                    /* 查寻是否执行结束 */
    iow(DM9000_REG_EPCR, 0x08);                         /* 清除写入命令 */
}

/*******************************************************************************
*    函数名: dm9k_initnic
*    参  数: 无
*    返  回: 无
*    功  能: 初始化DM9000AE
*/
void dm9k_initnic(void)
{
    iow(DM9000_REG_NCR, DM9000_REG_RESET);            /* 对 DM9000A 进行软件重置 */
    dm9k_udelay(10);                                /* delay 10us */

    dm9k_hash_table();                                /* 设置 DM9000A MAC 及 多播*/

    dm9k_reset();                                    /* 进行 DM9000A 软件设置 */

    iow(DM9000_REG_GPR, DM9000_PHY_OFF);            /* 关闭 PHY ，进行 PHY 设置*/
    dm9k_phy_write(0x00, 0x8000);                    /* 重置 PHY 的寄存器 */
#ifdef DM9000A_FLOW_CONTROL
    dm9k_phy_write(0x04, 0x01e1 | 0x0400);            /* 设置 自适应模式相容表 */
#else
    dm9k_phy_write(0x04, 0x01e1);                    /* 设置 自适应模式相容表 */
#endif
    //dm9k_phy_write(0x00, 0x1000);                    /* 设置 基本连接模式 */
    /* 连接模式设置
      0x0000 : 固定10M半双工
      0x0100 : 固定10M全双工
      0x2000 : 固定100M半双工
      0x2100 : 固定100M全双工
      0x1000 : 自适应模式
    */
    dm9k_phy_write(0x00, 0x1000);                /* 设置 基本连接模式 */

    iow(DM9000_REG_GPR, DM9000_PHY_ON);                /* 结束 PHY 设置, 开启 PHY */

    //dm9k_debug_test();
}

/*******************************************************************************
*    函数名: dm9k_receive_packet
*    参  数: _uip_buf : 接收缓冲区
*    返  回: > 0 表示接收的数据长度, 0表示没有数据
*    功  能: 读取一包数据
*/
uint16_t dm9k_receive_packet(uint8_t *_uip_buf)
{
    uint16_t ReceiveLength;
    uint16_t *ReceiveData;
    uint8_t  rx_int_count = 0;
    uint8_t  rx_checkbyte;
    uint16_t rx_status, rx_length;
    uint8_t  jump_packet;
    uint16_t i;
    uint16_t calc_len;
    uint16_t calc_MRR;

    do
    {
        ReceiveLength = 0;                                /* 清除接收的长度 */
        ReceiveData = (uint16_t *)_uip_buf;
        jump_packet = 0;                                /* 清除跳包动作 */
        ior(DM9000_REG_MRCMDX);                            /* 读取内存数据，地址不增加 */
        /* 计算内存数据位置 */
        calc_MRR = (ior(DM9000_REG_MRRH) << 8) + ior(DM9000_REG_MRRL);
        rx_checkbyte = ior(DM9000_REG_MRCMDX);            /*  */

        if(rx_checkbyte == DM9000_PKT_RDY)                /* 取 */
        {
            /* 读取封包相关资讯 及 长度 */
            NET_REG_ADDR = DM9000_REG_MRCMD;
            rx_status = NET_REG_DATA;
            rx_length = NET_REG_DATA;

            /* 若收到超过系统可承受的封包，此包跳过 */
            if(rx_length > Max_Ethernet_Lenth)
                jump_packet = 1;

#ifdef Broadcast_Jump
            /* 若收到的广播或多播包超过特定长度，此包跳过 */
            if(rx_status & 0x4000)
            {
                if(rx_length > Max_Broadcast_Lenth)
                    jump_packet = 1;
            }
#endif
            /* 计算下一个包的指针位 , 若接收长度为奇数，需加一对齐偶字节。*/
            /* 若是超过 0x3fff ，则需回归绕到 0x0c00 起始位置 */
            calc_MRR += (rx_length + 4);
            if(rx_length & 0x01) calc_MRR++;
            if(calc_MRR > 0x3fff) calc_MRR -= 0x3400;

            if(jump_packet == 0x01)
            {
                /* 将指针移到下一个包的包头位置 */
                iow (DM9000_REG_MRRH, (calc_MRR >> 8) & 0xff);
                iow (DM9000_REG_MRRL, calc_MRR & 0xff );
                continue;
            }

            /* 开始将内存的资料搬到到系统中，每次移动一个 word */
            calc_len = (rx_length + 1) >> 1;
            for(i = 0 ; i < calc_len ; i++)
                ReceiveData[i] = NET_REG_DATA;

            /* 将包长回报给 TCP/IP 上层，并减去最後 4 BYTE 的 CRC-32 检核码 */
            ReceiveLength = rx_length - 4;

            rx_int_count++;                                /* 累计收包次数 */

#ifdef FifoPointCheck
            if(calc_MRR != ((ior(DM9000_REG_MRRH) << 8) + ior(DM9000_REG_MRRL)))
            {
#ifdef Point_Error_Reset
                dm9k_reset();                                /* 若是指针出错，重置 */
                return ReceiveLength;
#endif
                /*若是指针出错，将指针移到下一个包的包头位置  */
                iow(DM9000_REG_MRRH, (calc_MRR >> 8) & 0xff);
                iow(DM9000_REG_MRRL, calc_MRR & 0xff);
            }
#endif
            return ReceiveLength;
        }
        else
        {
            if(rx_checkbyte == DM9000_PKT_NORDY)        /* 未收到包 */
            {
                iow(DM9000_REG_ISR, 0x3f);                /*  */
            }
            else
            {
                dm9k_reset();                                /* 接收指针出错，重置 */
            }
            return (0);
        }
    }while(rx_int_count < Max_Int_Count);                /* 是否超过最多接收封包计数 */
    return 0;
}

/*******************************************************************************
*    函数名: dm9k_send_packet
*    参  数: p_char : 发送数据缓冲区
*            length : 数据长度
*    返  回: 无
*    功  能: 发送一包数据
*/
void dm9k_send_packet(uint8_t *p_char, uint16_t length)
{
    uint16_t SendLength = length;
    uint16_t *SendData = (uint16_t *) p_char;
    uint16_t i;
    uint16_t calc_len;
    __IO uint16_t calc_MWR;

    /* 检查 DM9000A 是否还在传送中！若是等待直到传送结束 */
    if(SendPackOk == Max_Send_Pack)
    {
        while(ior(DM9000_REG_TCR) & DM9000_TCR_SET)
        {
            dm9k_udelay (5);
        }
        SendPackOk = 0;
    }

    SendPackOk++;                                        /* 设置传送计数 */

#ifdef FifoPointCheck
    /* 计算下一个传送的指针位 , 若接收长度为奇数，需加一对齐偶字节。*/
    /* 若是超过 0x0bff ，则需回归绕到 0x0000 起始位置 */
    calc_MWR = (ior(DM9000_REG_MWRH) << 8) + ior(DM9000_REG_MWRL);
    calc_MWR += SendLength;
    if(SendLength & 0x01) calc_MWR++;
    if(calc_MWR > 0x0bff) calc_MWR -= 0x0c00;
#endif

    iow(DM9000_REG_TXPLH, (SendLength >> 8) & 0xff);    /* 设置传送封包的长度 */
    iow(DM9000_REG_TXPLL, SendLength & 0xff);

    /* 开始将系统的资料搬到到内存中，每次移动一个 word */
    NET_REG_ADDR = DM9000_REG_MWCMD;
    calc_len = (SendLength + 1) >> 1;
    for(i = 0; i < calc_len; i++)
        NET_REG_DATA = SendData[i];

    iow(DM9000_REG_TCR, DM9000_TCR_SET);                /* 进行传送 */

#ifdef FifoPointCheck
    if(calc_MWR != ((ior(DM9000_REG_MWRH) << 8) + ior(DM9000_REG_MWRL)))
    {
#ifdef Point_Error_Reset
        /* 若是指针出错，等待此一封包送完 , 之後进行重置 */
        while(ior(DM9000_REG_TCR) & DM9000_TCR_SET) dm9k_udelay (5);
        dm9k_reset();
        return;
#endif
        /*若是指针出错，将指针移到下一个传送包的包头位置  */
        iow(DM9000_REG_MWRH, (calc_MWR >> 8) & 0xff);
        iow(DM9000_REG_MWRL, calc_MWR & 0xff);
    }
#endif
    return;
}

/*******************************************************************************
*    函数名: dm9k_interrupt
*    参  数: 无
*    返  回: 无
*    功  能: 中断处理函数 (webserver例程未使用中断)
*/
void  dm9k_interrupt(void)
{
#if 0  /* armfly */    
    uint8_t  save_reg;
    uint8_t  isr_status;

    save_reg = NET_REG_ADDR;                            /* 暂存所使用的位置 */

    iow(DM9000_REG_IMR , DM9000_IMR_OFF);                /* 关闭 DM9000A 中断 */
    isr_status = ior(DM9000_REG_ISR);                    /* 取得中断产生值 */

    if(isr_status & DM9000_RX_INTR)                     /* 检查是否为接收中断 */
        dm9k_receive_packet();                            /* 执行接收处理程序 */

    iow(DM9000_REG_IMR , DM9000_IMR_SET);                /* 开启 DM9000A 中断 */
    NET_REG_ADDR = save_reg;                            /* 回复所使用的位置 */
#endif
}

/*******************************************************************************
*    函数名: dm9k_debug_test
*    参  数: 无
*    返  回: 无
*    功  能: 测试DM9000AE的函数,用于排错
*/
void dm9k_debug_test(void)
{
    uint32_t check_device;
    uint8_t  check_iomode;
    uint8_t  check_reg_fail = 0;
    uint8_t  check_fifo_fail = 0;
    uint16_t i;
    uint16_t j;

    iow(DM9000_REG_NCR, DM9000_REG_RESET);            /* 对 DM9000A 进行软件重置 */
    dm9k_udelay(10);                                /* delay 10us */
    iow(DM9000_REG_NCR, DM9000_REG_RESET);            /* 对 DM9000A 进行软件重置 */
    dm9k_udelay(10);                                /* delay 10us */

    check_device  = ior(DM9000_REG_VID_L);
    check_device |= ior(DM9000_REG_VID_H) << 8;
    check_device |= ior(DM9000_REG_PID_L) << 16;
    check_device |= ior(DM9000_REG_PID_H) << 24;

    if(check_device != 0x90000A46)
    {
        printk("DM9K_DEBUG ==> DEIVCE NOT FOUND, SYSTEM HOLD !!\n");
        while(1);
    }
    else
    {
        printk("DM9K_DEBUG ==> DEIVCE FOUND !!\n");
    }

    check_iomode = ior(DM9000_REG_ISR) >> 6;
    if(check_iomode != DM9000_WORD_MODE)
    {
        printk("DM9K_DEBUG ==> DEIVCE NOT WORD MODE, SYSTEM HOLD !!\n");
        while(1);
    }
    else
    {
        printk("DM9K_DEBUG ==> DEIVCE IS WORD MODE !!\n");
    }

    printk("DM9K_DEBUG ==> REGISTER R/W TEST !!\n");
    NET_REG_ADDR = DM9000_REG_MAR;
    for(i = 0; i < 0x0100; i++)
    {
        NET_REG_DATA = i;
        if(i != (NET_REG_DATA & 0xff))
        {
            printk("             > error W %02x , R %02x \n", i , NET_REG_DATA);
            check_reg_fail = 1;
        }
    }

    if(check_reg_fail)
    {
        printk("DM9K_DEBUG ==> REGISTER R/W FAIL, SYSTEM HOLD !!\n");
        while(1);
    }

    printk("DM9K_DEBUG ==> FIFO R/W TEST !!\n");
    printk("DM9K_DEBUG ==> FIFO WRITE START POINT 0x%02x%02x \n",
            ior(DM9000_REG_MWRH), ior(DM9000_REG_MWRL));

    NET_REG_ADDR = DM9000_REG_MWCMD;
    for(i = 0; i < 0x1000; i++)
        NET_REG_DATA = ((i & 0xff) * 0x0101);

    printk("DM9K_DEBUG ==> FIFO WRITE END POINT 0x%02x%02x \n",
            ior(DM9000_REG_MWRH), ior(DM9000_REG_MWRL));

    if((ior(DM9000_REG_MWRH) != 0x20) || (ior(DM9000_REG_MWRL) != 0x00))
    {
        printk("DM9K_DEBUG ==> FIFO WRITE FAIL, SYSTEM HOLD !!\n");
        while(1);
    }

    ior(DM9000_REG_MRCMDX);
    printk("DM9K_DEBUG ==> FIFO READ START POINT 0x%02x%02x \n",
            ior(DM9000_REG_MRRH), ior(DM9000_REG_MRRL));
    ior(DM9000_REG_MRCMDX);

    NET_REG_ADDR = DM9000_REG_MRCMD;
    for(i = 0; i < 0x1000; i++)
    {
        j = NET_REG_DATA;

        if(((i & 0xff) * 0x0101) != j)
        {
            //printk("             > error W %04x , R %04x \n",
            //        ((i & 0xff) * 0x0101) , j);
            check_fifo_fail = 1;
        }
    }

    printk("DM9K_DEBUG ==> FIFO READ END POINT 0x%02x%02x \n",
            ior(DM9000_REG_MRRH), ior(DM9000_REG_MRRL));

    if((ior(DM9000_REG_MRRH) != 0x20) || (ior(DM9000_REG_MRRL) != 0x00))
    {
        printk("DM9K_DEBUG ==> FIFO WRITE FAIL, SYSTEM HOLD !!\n");
        while(1);
    }

        if(check_fifo_fail)
    {
        printk("DM9K_DEBUG ==> FIFO R/W DATA FAIL, SYSTEM HOLD !!\n");
        while(1);
    }

    printk("DM9K_DEBUG ==> PACKET SEND & INT TEST !! \n");
    iow(DM9000_REG_NCR, DM9000_REG_RESET);
    dm9k_udelay(10);
    iow(DM9000_REG_NCR, DM9000_REG_RESET);
    dm9k_udelay(10);

    iow(DM9000_REG_IMR, DM9000_IMR_OFF | DM9000_TX_INTR);

    iow(DM9000_REG_TXPLH, 0x01);
    iow(DM9000_REG_TXPLL, 0x00);

    do
    {
        iow(DM9000_REG_ISR, DM9000_TX_INTR);
        printk("DM9K_DEBUG ==> INT PIN IS OFF\n");

        NET_REG_ADDR = DM9000_REG_MWCMD;
        for(i = 0; i < (0x0100 / 2); i++)
        {
            if(i < 3)
                NET_REG_DATA = 0xffff;
            else
                NET_REG_DATA = i * 0x0101;
        }

        printk("DM9K_DEBUG ==> PACKET IS SEND \n");
        iow(DM9000_REG_TCR, DM9000_TCR_SET);

        while(ior(DM9000_REG_TCR) & DM9000_TCR_SET) dm9k_udelay (5);
        if(ior(DM9000_REG_ISR) & DM9000_TX_INTR)
            printk("DM9K_DEBUG ==> INT PIN IS ACTIVE \n");
        else
            printk("DM9K_DEBUG ==> INT PIN IS NOT ACTIVE \n");

        for(i = 0; i < 10; i++)
            dm9k_udelay(1000);

    }while(1);
}

/*******************************************************************************
*    函数名: etherdev_init
*    参  数: 无
*    返  回: 无
*    功  能: uIP 接口函数,初始化网卡
*/
void etherdev_init(void)
{
    DM9K_CtrlLinesConfig();
    DM9K_FSMCConfig();

    s_FSMC_Init_Ok = 1;

    dm9k_initnic();
}

/*******************************************************************************
*    函数名: etherdev_send
*    参  数: p_char : 数据缓冲区
*            length : 数据长度
*    返  回: 无
*    功  能: uIP 接口函数,发送一包数据
*/
void etherdev_send(uint8_t *p_char, uint16_t length)
{
    dm9k_send_packet(p_char, length);
}

uint16_t etherdev_read(uint8_t *p_char)
{
    return dm9k_receive_packet(p_char);
}

/*******************************************************************************
*    函数名: etherdev_chkmedia
*    参  数: p_char : 数据缓冲区
*            length : 数据长度
*    返  回: 无
*    功  能: uIP 接口函数, 检测网络连接状态
*/
void etherdev_chkmedia(void)
{
//    uint8_t status;

    while(!(ior(DM9000_REG_NSR) & DM9000_PHY))
    {
        dm9k_udelay(2000);
    }
}


/*******************************************************************************
*    函数名: etherdev_poll
*    参  数: 无
*    返  回: 无
*    功  能: uIP 接口函数, 采用查询方式接收一个IP包
*/
/*
                              etherdev_poll()

    This function will read an entire IP packet into the uip_buf.
    If it must wait for more than 0.5 seconds, it will return with
    the return value 0. Otherwise, when a full packet has been read
    into the uip_buf buffer, the length of the packet is returned.
*/
uint16_t etherdev_poll(void)
{
    uint16_t bytes_read = 0;
#if 0

    /* tick_count threshold should be 12 for 0.5 sec bail-out
        One second (24) worked better for me, but socket recycling
        is then slower. I set UIP_TIME_WAIT_TIMEOUT 60 in uipopt.h
        to counter this. Retransmission timing etc. is affected also. */
    while ((!(bytes_read = etherdev_read())) && (timer0_tick() < 12)) continue;

    timer0_reset();

#endif
    return bytes_read;
}

/*******************************************************************************
*    函数名: dm9k_ReadID
*    参  数: 无
*    返  回: 无
*    功  能: 读取芯片ID
*/
uint32_t dm9k_ReadID(void)
{
    uint8_t vid1,vid2,pid1,pid2;

    if (s_FSMC_Init_Ok == 0)
    {
        DM9K_CtrlLinesConfig();
        DM9K_FSMCConfig();

        s_FSMC_Init_Ok = 1;
    }
    vid1 = ior(DM9000_REG_VID_L) & 0xFF;
    vid2 = ior(DM9000_REG_VID_H) & 0xFF;
    pid1 = ior(DM9000_REG_PID_L) & 0xFF;
    pid2 = ior(DM9000_REG_PID_H) & 0xFF;

    return (vid2 << 24) | (vid1 << 16) | (pid2 << 8) | pid1;
}

/*******************************************************************************
*    函数名: DM9000AE_CtrlLinesConfig
*    参  数: 无
*    返  回: 无
*    功  能: 配置DM9000AE控制口线，FSMC管脚设置为复用功能
*/
static void DM9K_CtrlLinesConfig(void)
{
    /*

    PD0/FMC_D2
    PD1/FMC_D3
    PD4/FMC_NOE        ---- 读控制信号，OE = Output Enable ， N 表示低有效
    PD5/FMC_NWE        -XX- 写控制信号，AD7606 只有读，无写信号
    PD8/FMC_D13
    PD9/FMC_D14
    PD10/FMC_D15
    PD14/FMC_D0
    PD15/FMC_D1

    PE7/FMC_D4
    PE8/FMC_D5
    PE9/FMC_D6
    PE10/FMC_D7
    PE11/FMC_D8
    PE12/FMC_D9
    PE13/FMC_D10
    PE14/FMC_D11
    PE15/FMC_D12
    
    PG0/FMC_A10        --- 和主片选FMC_NE2一起译码
    PG1/FMC_A11        --- 和主片选FMC_NE2一起译码
    PG9/FMC_NE2        --- 主片选（OLED, 74HC574, DM9000, AD    606）    
    
    PF0/FMC_A0      ---- RS
    */    

    GPIO_InitTypeDef gpio_init_structure;

    /* 使能 GPIO时钟 */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    /* 使能FMC时钟 */
    __HAL_RCC_FMC_CLK_ENABLE();

    /* 设置 GPIOD 相关的IO为复用推挽输出 */
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = GPIO_AF12_FMC;
    
    /* 配置GPIOD */
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 |
                                GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
                                GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &gpio_init_structure);

    /* 配置GPIOE */
    gpio_init_structure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                                GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                                GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init_structure);

    /* 配置GPIOG */
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOG, &gpio_init_structure);
    
    /* 配置GPIOH */
    gpio_init_structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
                        | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH, &gpio_init_structure);

    /* 配置GPIOI */
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6
                        | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOI, &gpio_init_structure);

    /* 配置GPIOF */
    gpio_init_structure.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOF, &gpio_init_structure);
}

/*******************************************************************************
*    函数名: DM9K_FSMCConfig
*    参  数: 无
*    返  回: 无
*    功  能: 配置FSMC并口访问时序
*/
static void DM9K_FSMCConfig(void)
{
    SRAM_HandleTypeDef hsram = {0};
    FMC_NORSRAM_TimingTypeDef SRAM_Timing = {0};

    hsram.Instance  = FMC_NORSRAM_DEVICE;
    hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;

    SRAM_Timing.AddressSetupTime       = 4;
    SRAM_Timing.AddressHoldTime        = 1;
    SRAM_Timing.DataSetupTime          = 2;
    SRAM_Timing.BusTurnAroundDuration  = 3;        /* 1不行 */
    SRAM_Timing.CLKDivision            = 2;
    SRAM_Timing.DataLatency            = 2;
    SRAM_Timing.AccessMode             = FMC_ACCESS_MODE_A;    

    hsram.Init.NSBank             = FMC_NORSRAM_BANK1;
    hsram.Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType         = FMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_32;    /* 32位总线宽度 */
    hsram.Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WaitSignalActive   = FMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal         = FMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;
    hsram.Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;

    /* 初始化SRAM控制器 */
    if (HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
    {
        /* 初始化错误 */
        Error_Handler(__FILE__, __LINE__);
    }
}

