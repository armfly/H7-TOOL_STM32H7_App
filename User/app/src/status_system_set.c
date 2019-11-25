/*
*********************************************************************************************************
*
*    模块名称 : 系统设置主程序
*    文件名称 : status_system_set.c
*    版    本 : V1.0
*    说    明 : 提供一个菜单选择子功能.
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2018-12-06  armfly  正式发布
*
*    Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

#include "status_system_set.h"
#include "lcd_menu.h"
#include "wifi_if.h"
#include "usb_if.h"

const uint8_t *g_Menu1_Text[] =
        {
                " 1 硬件信息",
                " 2 参数设置",
                " 3 ESP32固件升级",

                /* 结束符号, 用于菜单函数自动识别菜单项个数 */
                "&"};

MENU_T g_tMenu1;

/*
*********************************************************************************************************
*    函 数 名: status_SystemSetMain
*    功能说明: 系统设置状态. 菜单选择
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_SystemSetMain(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    uint8_t ucFirstKey = 1;
    static uint8_t s_enter_sub_menu = 0;

    DispHeader("系统设置");

    if (s_enter_sub_menu == 0)
    {
        g_tMenu1.Left = MENU_LEFT;
        g_tMenu1.Top = 30;
        g_tMenu1.Height = MENU_HEIGHT;
        g_tMenu1.Width = MENU_WIDTH;
        g_tMenu1.LineCap = 2;
        g_tMenu1.ViewLine = 8;
        g_tMenu1.Font.FontCode = FC_ST_24;                            /* 字体代码 16点阵 */
                                                                                                        //        g_tMenu1.Font.FrontColor = CL_BLACK;        /* 字体颜色 */
                                                                                                        //        g_tMenu1.Font.BackColor = FORM_BACK_COLOR;    /* 文字背景颜色 */
        g_tMenu1.Font.Space = 0;                                                /* 文字间距，单位 = 像素 */
        LCD_InitMenu(&g_tMenu1, (char **)g_Menu1_Text); /* 初始化菜单结构 */
    }
    LCD_DispMenu(&g_tMenu1);

    fRefresh = 1;
    while (g_MainStatus == MS_SYSTEM_SET)
    {
        bsp_Idle();

        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

            if (g_tMenu1.Cursor == 0)
            {
                ;
            }
        }

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S: /* S键 上 */
                if (ucFirstKey == 1)
                {
                    ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
                    break;
                }
                LCD_MoveUpMenu(&g_tMenu1);
                break;

            case KEY_LONG_S: /* S键 上 */
                PlayKeyTone();
                s_enter_sub_menu = 1;

                if (g_tMenu1.Cursor == 0)
                {
                    g_MainStatus = MS_HARD_INFO;
                }
                else if (g_tMenu1.Cursor == 1)
                {
                    g_MainStatus = MS_MODIFY_PARAM;
                }
                else if (g_tMenu1.Cursor == 2)
                {
                    g_MainStatus = MS_ESP32_TEST;
                }
                break;

            case KEY_UP_C: /* C键 下 */
                if (ucFirstKey == 1)
                {
                    ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
                    break;
                }
                LCD_MoveDownMenu(&g_tMenu1);
                break;

            case KEY_LONG_C: /* C键长按 */
                PlayKeyTone();
                s_enter_sub_menu = 0;
                g_MainStatus = MS_LINK_MODE;
                break;

            default:
                break;
            }
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: status_HardInfo
*    功能说明: 硬件信息测试
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
extern int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
void status_HardInfo(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    FONT_T tFont;             /* 定义字体结构体变量 */
    uint16_t x, y;
    uint16_t usLineCap = 18;
    char buf[128];

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE;            /* 字体颜色 */
        tFont.BackColor = FORM_BACK_COLOR;      /* 文字背景颜色 */
        tFont.Space = 0;                        /* 文字间距，单位 = 像素 */
    }

    LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

    x = 5;
    y = 3;
    tFont.BackColor = CL_BLUE;                      /* 文字背景颜色 */
    LCD_DispStr(x, y, "H7-TOOL硬件信息", &tFont);   /* 在(8,3)坐标处显示一串汉字 */
    y += usLineCap;

    tFont.BackColor = FORM_BACK_COLOR;      /* 文字背景颜色 */
    
    /* 检测CPU ID */
    {
        uint32_t id[3];

        bsp_GetCpuID(id);

        sprintf(buf, "CPU : STM32H750IBK6  %dMHz", SystemCoreClock / 1000000);
        LCD_DispStr(x, y, buf, &tFont);
        y += usLineCap;

        sprintf(buf, "  %08X %08X %08X", id[0], id[1], id[2]);
        LCD_DispStr(x, y, buf, &tFont);
        y += usLineCap;
    }

    /* 显示TFT控制器型号和屏幕分辨率 */
    {
        LCD_DispStr(x, y, "LCD : ST7789 / 240x240", &tFont);
        y += usLineCap;
    }

    /* 测试eMMC */
    {
        BSP_MMC_CardInfo CardInfo;

        BSP_MMC_Init();
        BSP_MMC_GetCardInfo(&CardInfo);

        /* CardInfo.LogBlockSize = 512 */
        if (CardInfo.LogBlockSize == 512)
        {
            uint32_t kb;

            kb = CardInfo.LogBlockNbr / 2;
            sprintf(buf, "eMMC : %d.%02dGB Ok", kb / (1024 * 1024), (kb % (1024 * 1024)) / 10000);
            LCD_DispStr(x, y, buf, &tFont);
        }
        else
        {
            sprintf(buf, "eMMC : Error, BlockSize %d", CardInfo.LogBlockSize);
            tFont.FrontColor = CL_RED;
            LCD_DispStr(x, y, buf, &tFont);
            tFont.FrontColor = CL_WHITE;
        }
        y += usLineCap;
    }

    /* 测试I2C设备 */
    {
        if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
        {
            sprintf(buf, "EEPROM : 24C16 Ok (0x%02X)", EE_DEV_ADDR);
            LCD_DispStr(x, y, buf, &tFont);
        }
        else
        {
            sprintf(buf, "EEPROM : 24C16 Err (0x%02X)", EE_DEV_ADDR);
            tFont.FrontColor = CL_RED;
            LCD_DispStr(x, y, buf, &tFont);
            tFont.FrontColor = CL_WHITE;
        }
        y += usLineCap;

        if (i2c_CheckDevice(MCP4018_SLAVE_ADDRESS) == 0)
        {
            sprintf(buf, "POT : MCP4018 Ok (0x%02X)", MCP4018_SLAVE_ADDRESS);
            LCD_DispStr(x, y, buf, &tFont);
        }
        else
        {
            sprintf(buf, "POT : MCP4018 Err (0x%02X)", MCP4018_SLAVE_ADDRESS);
            tFont.FrontColor = CL_RED;
            LCD_DispStr(x, y, buf, &tFont);
            tFont.FrontColor = CL_WHITE;
        }
        y += usLineCap;
    }

    /* QSPI检测 */
    {
        uint32_t id;
        char name[32];

        bsp_InitQSPI_W25Q256();

        id = QSPI_ReadID();
        if (id == W25Q64_ID)
        {
            strcpy(name, "W25Q64");
        }
        else if (id == W25Q128_ID)
        {
            strcpy(name, "W25Q128");
        }
        else if (id == W25Q256_ID)
        {
            strcpy(name, "W25Q256");
        }
        else
        {
            strcpy(name, "UNKNOW");
        }

        /* 检测串行Flash OK */
        if (id == W25Q256_ID)
        {
            sprintf(buf, "QSPI : W25Q256 Ok, %08X", id);
            LCD_DispStr(x, y, buf, &tFont);
        }
        else
        {
            sprintf(buf, "QSPI : W25Q256 Err, %08X", id);
            tFont.FrontColor = CL_RED;
            LCD_DispStr(x, y, buf, &tFont);
            tFont.FrontColor = CL_WHITE;
        }
        y += usLineCap;
    }

    /* 以太网MAC */
    {
        uint32_t PhyID[2];

        /*
            PhyID[0].15:0  = OUI:3:18
            PhyID[1].15:10 = OUI.24:19
        
            PhyID[1].9:4 = Model Number
            PhyID[1].3:0 = Revision Number
        */
        ETH_PHY_IO_ReadReg(0, 2, &PhyID[0]);
        ETH_PHY_IO_ReadReg(0, 3, &PhyID[1]);

        if (((PhyID[1] >> 4) & 0x2F) == 0x0F)
        {
            sprintf(buf, "Eth Phy : LAN8720 Ok");
            LCD_DispStr(x, y, buf, &tFont);
        }
        else
        {
            sprintf(buf, "Eth Phy : LAN8720 Error");
            tFont.FrontColor = CL_RED;
            LCD_DispStr(x, y, buf, &tFont);
            tFont.FrontColor = CL_WHITE;
        }
        y += usLineCap;
    }

    /* WiFi MAC */
    {
        //uint16_t mac[6];

        //ESP32_GetMac(&mac);
    }
    
    /* 固件版本 */
    {
        tFont.FrontColor = CL_YELLOW;       /* 字体颜色 */
        
        sprintf(buf, "Boot Ver : %d.%02X", BOOT_VERSION >> 8, BOOT_VERSION & 0xFF);
        LCD_DispStr(x, y, buf, &tFont);   

        y += usLineCap;
        
        sprintf(buf, "App Ver : %d.%02X", APP_VERSION >> 8, APP_VERSION & 0xFF);
        LCD_DispStr(x, y, buf, &tFont);
    }

    bsp_StartAutoTimer(0, 1000);
    while (g_MainStatus == MS_HARD_INFO)
    {
        bsp_Idle();

        /* 显示时钟 */
        if (bsp_CheckTimer(0))
        {
            uint16_t x, y;

            tFont.FontCode = FC_ST_16;      /* 字体代码 16点阵 */
            tFont.FrontColor = CL_WHITE;    /* 字体颜色 */
            tFont.BackColor = CL_BLUE;      /* 文字背景颜色 */
            tFont.Space = 0;                /* 文字间距，单位 = 像素 */

            RTC_ReadClock(); /* 读时钟，结果在 g_tRTC */

            x = 5;
            y = LCD_GetHeight() - 20;

            sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d",
                            g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
            LCD_DispStr(x, y, buf, &tFont);
        }

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_C: /* C键 下 */
                break;

            case KEY_LONG_C: /* C键长按 */
                PlayKeyTone();
                g_MainStatus = MS_SYSTEM_SET;
                break;

            default:
                break;
            }
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: status_UsbUart1
*    功能说明: USB虚拟串口，映射到硬件串口
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_UsbUart1(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    FONT_T tFont;             /* 定义字体结构体变量 */

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;                 /* 字体代码 16点阵 */
        tFont.FrontColor = CL_BLACK;             /* 字体颜色 */
        tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
        tFont.Space = 0;                                     /* 文字间距，单位 = 像素 */

        LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

        LCD_DispStr(5, 3, "USB虚拟串口", &tFont);
    }

    usbd_CloseCDC();
    usbd_OpenCDC(COM1); /* 映射到串口1 */

    while (g_MainStatus == MS_USB_UART1)
    {
        bsp_Idle();

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S: /* S键 弹起 */
                break;

            case KEY_UP_C: /* C键 下 */
                break;

            case KEY_LONG_S: /* S键 上 */
                break;

            case KEY_LONG_C: /* C键长按 */
                PlayKeyTone();
                g_MainStatus = MS_SYSTEM_SET;
                break;

            default:
                break;
            }
        }
    }

    usbd_CloseCDC();
    usbd_OpenCDC(COM_USB_PC); /* 映射到串口8. 和PC软件联机 */
}


/*
*********************************************************************************************************
*    函 数 名: status_ModifyParam
*    功能说明: 修改常用参数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
#define PARAM_NUM       2
void status_ModifyParam(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    FONT_T tFont;             /* 定义字体结构体变量 */
    uint8_t fRefresh = 1;
    uint8_t fSaveParam = 0;
    uint8_t cursor = 0;
    char buf[48];
    uint8_t ucIgnoreKey = 1;    

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;          /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE;        /* 字体颜色 */
        tFont.BackColor = FORM_BACK_COLOR;  /* 文字背景颜色 */
        tFont.Space = 0;                    /* 文字间距，单位 = 像素 */

        LCD_ClrScr(FORM_BACK_COLOR);        /* 清屏，背景蓝色 */

        LCD_DispStr(5, 3, "设置参数", &tFont);
    }

    {
        FONT_T tFont1;   /* 定义字体结构体变量 */

        tFont1.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
        tFont1.FrontColor = HELP_TEXT_COLOR;     /* 字体颜色 */
        tFont1.BackColor = HELP_BACK_COLOR;      /* 文字背景颜色 */
        tFont1.Space = 0;                        /* 文字间距，单位 = 像素 */

        LCD_DispStr(5, 240 - 40, "长按S键选择参数", &tFont1);
        LCD_DispStr(5, 240 - 20, "短按S、C键修改参数值", &tFont1);
    }    
    
    while (g_MainStatus == MS_MODIFY_PARAM)
    {
        bsp_Idle();

        if (fRefresh == 1)
        {
            uint16_t x = 10;
            uint16_t y = 30;
            
            fRefresh = 0;
            
            /* 第1个参数 */
            {
                if (cursor == 0)
                {
                    tFont.BackColor = CL_RED;
                }
                else 
                {
                    tFont.BackColor = FORM_BACK_COLOR;
                }
                if (g_tParam.KeyToneEnable == 0)
                {               
                    LCD_DispStr(x, y, "按键音 : 关闭", &tFont);
                }
                else
                {
                    LCD_DispStr(x, y, "按键音 : 打开", &tFont);
                }
            }

            y += 20;
            
            /* 第2个参数 - 界面风格  */
            {
                if (cursor == 1)
                {
                    tFont.BackColor = CL_RED;
                }
                else 
                {
                    tFont.BackColor = FORM_BACK_COLOR;
                }
                sprintf(buf, "界面风格 : %3d", g_tParam.UIStyle);
                LCD_DispStr(x, y, buf, &tFont);
            }          
                      
        }
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S:      /* S键 弹起 */
                if (ucIgnoreKey == 1)
                {
                    ucIgnoreKey = 0;
                    break;
                }
                
                if (cursor == 0)
                {
                    if (g_tParam.KeyToneEnable == 0)
                    {
                        g_tParam.KeyToneEnable = 1;
                    }
                    else
                    {
                        g_tParam.KeyToneEnable = 0;
                    }     
                }
                else if (cursor == 1)
                {
                    g_tParam.UIStyle++;
                    if (g_tParam.UIStyle >= UI_STYLE_NUM)
                    {
                        g_tParam.UIStyle = 0;
                    }  
                }                
                fRefresh = 1;
                fSaveParam = 1;
                break;

            case KEY_UP_C:      /* C键 下 */
                if (cursor == 0)
                {                
                    if (g_tParam.KeyToneEnable == 0)
                    {
                        g_tParam.KeyToneEnable = 1;
                    }
                    else
                    {
                        g_tParam.KeyToneEnable = 0;
                    }
                }
                else if (cursor == 1)
                {
                    if (g_tParam.UIStyle > 0)
                    {
                        g_tParam.UIStyle--;
                    }     
                    else
                    {
                        g_tParam.UIStyle = UI_STYLE_NUM;
                    }
                }                 
                fRefresh = 1;
                fSaveParam = 1;
                break;

            case KEY_LONG_S:        /* S键长按 - 选择参数 */
                if (++cursor >= PARAM_NUM)
                {
                    cursor = 0;
                }
                ucIgnoreKey = 1;
                fRefresh = 1;
                break;

            case KEY_LONG_C:        /* C键长按 - 返回 */
                PlayKeyTone();
                g_MainStatus = MS_SYSTEM_SET;
                break;

            default:
                break;
            }
        }
    }
    
    if (fSaveParam == 1)
    {
        SaveParam();    /* 保存参数 */
    }
}

/*
*********************************************************************************************************
*    函 数 名: status_ESP32Test
*    功能说明: esp32测试
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_ESP32Test(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    FONT_T tFont; /* 定义字体结构体变量 */
    uint8_t ucFirstKey = 1;
    uint8_t isp_flag = 0;

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;                 /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE;             /* 字体颜色 */
        tFont.BackColor = FORM_BACK_COLOR; /* 文字背景颜色 */
        tFont.Space = 0;                                     /* 文字间距，单位 = 像素 */

        LCD_ClrScr(FORM_BACK_COLOR); /* 清屏，背景蓝色 */

        LCD_DispStr(5, 3, "ESP32模块测试", &tFont);
    }

    usbd_CloseCDC();
    usbd_OpenCDC(COM4);

    //bsp_InitESP32();

    ESP32_EnterISP();
    isp_flag = 1;

    wifi_state = WIFI_STOP;

    fRefresh = 1;
    while (g_MainStatus == MS_ESP32_TEST)
    {
        bsp_Idle();

        if (fRefresh == 1) /* 刷新整个界面 */
        {
            fRefresh = 0;

            if (isp_flag == 0)
            {
                LCD_DispStr(5, 60, "当前模式:  AT", &tFont);
            }
            else
            {
                LCD_DispStr(5, 60, "当前模式: ISP", &tFont);
            }
        }

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S: /* S键 弹起 */
                if (ucFirstKey == 1)
                {
                    ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
                    break;
                }
                PlayKeyTone();

                if (isp_flag == 0)
                {
                    isp_flag = 1;

                    ESP32_EnterISP();
                }
                else
                {
                    isp_flag = 0;

                    ESP32_EnterAT();
                }
                fRefresh = 1;
                break;

            case KEY_UP_C: /* C键 下 */
                if (ucFirstKey == 1)
                {
                    ucFirstKey = 0; /* 丢弃第1个按键弹起事件 */
                    break;
                }

                fRefresh = 1;
                break;

            case KEY_LONG_S: /* S键 上 */
                break;

            case KEY_LONG_C: /* C键长按 */
                PlayKeyTone();
                g_MainStatus = MS_SYSTEM_SET;
                break;

            default:
                break;
            }
        }
    }
    usbd_CloseCDC();
    usbd_OpenCDC(COM_USB_PC); /* 启用USB虚拟串口 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
