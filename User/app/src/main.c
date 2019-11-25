/*
*********************************************************************************************************
*
*    模块名称 : H7-TOOL App主程序
*    文件名称 : main.c
*    版    本 : V1.1
*    说    明 : 
*
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2019-10-01 armfly  正式发布
*        V1.1    2019-11-02 armfly  整理格式。增加功能。
*
*    Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "main.h"

#include "status_link_mode.h"
#include "status_voltage_meter.h"
#include "status_current_meter.h"
#include "status_resistor_meter.h"
#include "status_temp_meter.h"
#include "status_programmer.h"
#include "status_system_set.h"
#include "status_tvcc_power.h"
#include "status_pulse_meter.h"

#include "wifi_if.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "emmc_diskio_dma.h"

#include "lwip_user.h"
#include "lua_if.h"

#include "target_reset.h"
#include "target_config.h"
#include "swd_host.h"

//#include "usbd_user.h"
#include "usb_if.h"

static void DispLogo(void);
uint16_t GetStatusIndex(uint16_t _NowStatus);

uint16_t g_MainStatus;  /* 主状态字 */

/* 主状态切换顺序 */
static const uint16_t StatusOrder[] =
{
    MS_LINK_MODE,        /* 联机状态 */
    MS_VOLTAGE_METER,    /* 电压表 */
    MS_RESISTOR_METER,   /* 电阻表 */
    MS_CURRENT_METER,    /* 高侧电流表 */
    MS_TEMP_METER,       /* 温度表 */
    MS_TVCC_POWER,       /* 微型数控电源 */   
    MS_PROGRAMMER,       /* 脱机下载器 */
    MS_PULSE_METER,      /* 脉冲计 */
};

/*
*********************************************************************************************************
*    函 数 名: main
*    功能说明: c程序入口
*    形    参: 无
*    返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
    bsp_Init();
    LoadParam(); /* 读取应用程序参数, 该函数在param.c */

    PERIOD_Start(&g_tRunLed, 50, 50, 0); /* LED一直闪烁, 非阻塞 */

    DispLogo();
    
    bsp_InitESP32();

    DSO_InitHard();
    DSO_SetDC(1, 1);
    DSO_SetDC(2, 1);
    DSO_SetGain(1, 3);
    DSO_SetGain(2, 3);

    //usbd_OpenMassStorage();
    bsp_SetTVCC(3300);

    /* LwIP 初始化 */
    {
        /* 如果不插网线，此函数执行时间过长 */
        /* 网络参数存在在全局变量 g_tParam.lwip_ip, g_tParam.lwip_net_mask, g_tParam.lwip_gateway */
        lwip_start();

        lwip_pro();
    }

    PERIOD_Stop(&g_tRunLed); /* 停止LED闪烁 */
    
    usbd_Init();    /* 初始化USB协议栈 */
    
    //wifi_state = WIFI_INIT;

    /* 主程序采用状态机实现程序功能切换 */
    g_MainStatus = MS_LINK_MODE; /* 初始状态 = 联机界面 */
    while (1)
    {
        switch (g_MainStatus)
        {
        case MS_LINK_MODE:      /* 联机状态 */
            status_LinkMode();
            break;

        case MS_SYSTEM_SET:     /* 系统设置 */
            status_SystemSetMain();
            break;

        case MS_HARD_INFO:      /* 硬件信息 */
            status_HardInfo();
            break;

        case MS_ESP32_TEST:     /* ESP32模块固件升级 */
            status_ESP32Test();
            break;

        case MS_USB_UART1:      /* USB虚拟串口，映射到硬件UART1， RS485 RS232 */
            status_UsbUart1();
            break;

        case MS_MODIFY_PARAM:   /* 修改参数 */
            status_ModifyParam();
            break;          

        case MS_PROGRAMMER:     /* 脱机下载器 */
            status_Programmer();
            break;

        case MS_VOLTAGE_METER:  /* 电压表 */
            status_VoltageMeter();
            break;

        case MS_CURRENT_METER:  /* 高侧电流表 */
            status_CurrentMeter();
            break;

        case MS_TEMP_METER:     /* 温度表 */
            status_TempMeter();
            break;

        case MS_RESISTOR_METER: /* 电阻表 */
            status_ResistorMeter();
            break;

        case MS_TVCC_POWER:     /* 微型电源 */
            status_TVCCPower();
            break;        
        
        case MS_PULSE_METER:    /* 脉冲计 */
            status_PulseMeter();
            break;  
        
        
        
        
        default:
            g_MainStatus = MS_LINK_MODE;
            break;
        }
    }
}


/*
*********************************************************************************************************
*    函 数 名: NextStatus
*    功能说明: 状态切换, 向后翻
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
uint16_t NextStatus(uint16_t _NowStatus)
{
    uint16_t next;
    uint16_t i;
    uint16_t count;

    count = sizeof(StatusOrder) / 2;

    for (i = 0; i < count; i++)
    {
        if (_NowStatus == StatusOrder[i])
        {
            next = i;
            break;
        }
    }

    if (++next >= count)
    {
        next = 0;
    }

    PlayKeyTone();
    return StatusOrder[next];
}

/*
*********************************************************************************************************
*    函 数 名: LastStatus
*    功能说明: 状态切换, 向前翻
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
uint16_t LastStatus(uint16_t _NowStatus)
{
    uint16_t next;
    uint16_t i;
    uint16_t count;

    count = sizeof(StatusOrder) / 2;

    for (i = 0; i < count; i++)
    {
        if (_NowStatus == StatusOrder[i])
        {
            next = i;
            break;
        }
    }

    if (next > 0)
    {
        next--;
    }
    else
    {
        next = count - 1;
    }
    PlayKeyTone();
    return StatusOrder[next];
}

/*
*********************************************************************************************************
*    函 数 名: GetStatusIndex
*    功能说明: 根据状态字获取菜单序号
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
uint16_t GetStatusIndex(uint16_t _NowStatus)
{
    uint16_t idx;
    uint16_t i;
    uint16_t count;

    count = sizeof(StatusOrder) / 2;

    for (i = 0; i < count; i++)
    {
        if (_NowStatus == StatusOrder[i])
        {
            idx = i;
            break;
        }
    }

    return idx;
}

/*
*********************************************************************************************************
*    函 数 名: DispLogo
*    功能说明: 开机显示版本号
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispLogo(void)
{
    FONT_T tFont; /* 定义字体结构体变量 */

    tFont.FontCode = FC_ST_16;                  /* 字体代码 16点阵 */
    tFont.FrontColor = RGB(200, 200, 200);      /* 字体颜色 */
    tFont.BackColor = FORM_BACK_COLOR;          /* 文字背景颜色 */
    tFont.Space = 0;                            /* 文字间距，单位 = 像素 */

    ST7789_SetDirection(g_tParam.DispDir);

    LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

    /* 显示APP固件版本。版本号放在中断向量表 */
    {
        char buf[64];
        uint16_t x = 5;
        uint16_t y = 3;
        uint16_t line_cap = 20;

        //LCD_DispStr(x, y, "H7-TOOL多功能开发工具", &tFont);
        LCD_DispStr(x, y, "H7-TOOL", &tFont);
        y += line_cap;

        sprintf(buf, "App Ver:%d.%02X",
                        APP_VERSION >> 8, APP_VERSION & 0xFF);
        LCD_DispStr(x, y, buf, &tFont);

        y += line_cap;
        sprintf(buf, "Boot Ver:%d.%02X",
                        BOOT_VERSION >> 8, BOOT_VERSION & 0xFF);
        LCD_DispStr(x, y, buf, &tFont);

        y += line_cap;
        LCD_DispStr(x, y, "正在配置网络...", &tFont);
    }

    LCD_SetBackLight(BRIGHT_DEFAULT); /* 打开背光，设置为缺省亮度 */
}

/*
*********************************************************************************************************
*    函 数 名: DispHeader
*    功能说明: 显示界面标题（抬头第1行）
*    形    参: _str : 标题文字
*    返 回 值: 无
*********************************************************************************************************
*/
void DispHeader(char *_str)
{
    FONT_T tFont;
    char buf[48];
    uint16_t idx = 0;

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_24;          /* 字体代码 16点阵 */
        tFont.FrontColor = HEAD_TEXT_COLOR; /* 字体颜色 */
        tFont.BackColor = HEAD_BACK_COLOR;  /* 文字背景颜色 */
        tFont.Space = 0;                    /* 文字间距，单位 = 像素 */
    }
    
    idx  = GetStatusIndex(g_MainStatus);
    sprintf(buf, "%d.%s", idx, _str);
    
    if (g_MainStatus == MS_SYSTEM_SET)
    {
        LCD_DispStrEx(0, 0, _str, &tFont, 240, ALIGN_CENTER);
    }
    else
    {
        LCD_DispStrEx(0, 0, buf, &tFont, 240, ALIGN_CENTER);
    }

    LCD_Fill_Rect(0, 24, 240 - 24, 240, FORM_BACK_COLOR); /* 清屏  */
}

/*
*********************************************************************************************************
*    函 数 名: DSO_StartMode2
*    功能说明: 启动示波器欧式2，多通道低速扫描模式
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void DSO_StartMode2(void)
{
    WriteRegValue_06H(0x01FF, 2);    /* 多通道低速测量 */
    WriteRegValue_06H(0x0200, 1);    /* CH1选DC耦合 */
    WriteRegValue_06H(0x0201, 1);    /* CH2选DC耦合 */
    WriteRegValue_06H(0x0202, 0);    /* CH1通道增益0档，不放大 */
    WriteRegValue_06H(0x0203, 0);    /* CH2通道增益0档，不放大 */
    WriteRegValue_06H(0x0204, 0);    /* CH1通道直流偏值，未用 */
    WriteRegValue_06H(0x0205, 0);    /* CH2通道直流偏值，未用 */
    WriteRegValue_06H(0x0206, 12);   /* 采样频率1M */
    WriteRegValue_06H(0x0207, 0);    /* 采样深度1K */
    WriteRegValue_06H(0x0208, 0);    /* 触发电平 */
    WriteRegValue_06H(0x0209, 50);   /* 触发位置 */
    WriteRegValue_06H(0x020A, 0);    /* 触发模式 0=自动 */
    WriteRegValue_06H(0x020B, 0);    /* 触发通道CH1 */
    WriteRegValue_06H(0x020C, 0);    /* 触发边沿 */
    WriteRegValue_06H(0x020D, 2);    /* 通道使能 */
    WriteRegValue_06H(0x020E, 1);    /* 开始采集 */
}

/*
*********************************************************************************************************
*    函 数 名: PlayKeyTone
*    功能说明: 播放按键音。通过参数可以关闭
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void PlayKeyTone(void)
{
    if (g_tParam.KeyToneEnable != 0)
    {
        BEEP_KeyTone();
    }
}

/*
*********************************************************************************************************
*    函 数 名: TestFunc
*    功能说明: 临时测试函数。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
#if 0
static void TestFunc(void)
{
#if 0 /* 测试eeprom驱动 */
    
    ee_WriteBytes("12345678",0,  8);
    ee_WriteBytes("87981234",2048 - 8,  8);

#endif

#if 0 /* 测试SWD接口 */
    {
        static uint8_t read_buf[2048];
                
        sysTickInit();
        
        bsp_SetTVCC(3.3 * 1000);    /* 设置接口电平3.3V */
        bsp_DelayUS(100 * 100);        /* 延迟100ms */
        
        swd_init_debug();    /* 进入swd debug状态 */
        

        while (1)
        {
            /******** 此段代码执行时间 5.2ms *************/
                /* 写2048字节到STM32F030内存 */
                memset(read_buf, 0x55, 2048);
                swd_write_memory(0x20000000, read_buf, 2048);
                
                /* 读STM32F030内存2048字节 */
                memset(read_buf, 0, 2048);
                swd_read_memory(0x20000000, read_buf, 2048);   

                swd_read_memory(0x58020000, read_buf, 2048);   
            
            bsp_DelayUS(50000);    /* 延迟1ms */        
        }
    }
#endif

#if 0
    {
        uint8_t ucWaveIdx = 0;
        int16_t volt_min = -10000;
        int16_t volt_max = 10000;
        uint32_t freq = 1000;
        uint16_t duty = 50;
            bsp_InitDAC1();    
        g_tDacWave.VoltRange = 1;
        g_tDacWave.CycleSetting = 0;            
        g_tDacWave.VoltMin = volt_min;
        g_tDacWave.VoltMax = volt_max;
        g_tDacWave.Freq = freq;
        g_tDacWave.Duty = duty;

        g_tDacWave.Type = DAC_WAVE_SIN;            

    
        dac1_StartDacWave();
        
        while(1);
    }
#endif    
}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
