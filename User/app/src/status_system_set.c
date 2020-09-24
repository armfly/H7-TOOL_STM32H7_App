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
#include "file_lib.h"

const uint8_t *g_MenuSys_Text[] =
{
    " 1 硬件信息",
    " 2 参数设置",
    " 3 ESP32固件升级",
    " 4 USB eMMC磁盘",
    " 5 数据维护",
    " 6 重启",
    /* 结束符号, 用于菜单函数自动识别菜单项个数 */
    "&"
};

MENU_T g_tMenuSys;

ALIGN_32BYTES(uint8_t qspi_file_buf[1024]);
ALIGN_32BYTES(uint8_t qspi_read_buf[1024]);

static void FM_WirteFont(void);
static void FM_WirteBoot(void);
static void FM_WirteQspiApp(void);


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
    static uint8_t s_enter_sub_menu = 0;
    uint8_t ResetReq = 0;

    DispHeader2(90, "系统设置");

    if (s_enter_sub_menu == 0)
    {
        g_tMenuSys.Left = MENU_LEFT;
        g_tMenuSys.Top = MENU_TOP;
        g_tMenuSys.Height = MENU_HEIGHT;
        g_tMenuSys.Width = MENU_WIDTH;
        g_tMenuSys.LineCap = MENU_CAP;
        g_tMenuSys.ViewLine = 8;
        g_tMenuSys.Font.FontCode = FC_ST_24;
//        g_tMenuSys.Font.FrontColor = CL_BLACK;        /* 字体颜色 */
//        g_tMenuSys.Font.BackColor = FORM_BACK_COLOR;    /* 文字背景颜色 */
        g_tMenuSys.Font.Space = 0;
        g_tMenuSys.RollBackEn = 1;  /* 允许回滚 */        
        g_tMenuSys.GBK = 0;
        g_tMenuSys.ActiveBackColor = 0;   /* 选中行背景色ID */
        LCD_InitMenu(&g_tMenuSys, (char **)g_MenuSys_Text); /* 初始化菜单结构 */
    }
    LCD_DispMenu(&g_tMenuSys);

    fRefresh = 1;
    while (g_MainStatus == MS_SYSTEM_SET)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

            if (g_tMenuSys.Cursor == 0)
            {
                ;
            }
        }
        
        bsp_Idle();
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
                case KEY_UP_S: /* S键 上 */                   
                    LCD_MoveUpMenu(&g_tMenuSys);
                    break;

                case KEY_LONG_DOWN_S: /* S键 上 */
                    PlayKeyTone();
                    s_enter_sub_menu = 1;

                    if (g_tMenuSys.Cursor == 0)
                    {
                        g_MainStatus = MS_HARD_INFO;
                    }
                    else if (g_tMenuSys.Cursor == 1)
                    {
                        g_MainStatus = MS_MODIFY_PARAM;
                    }
                    else if (g_tMenuSys.Cursor == 2)
                    {
                        g_MainStatus = MS_ESP32_TEST;
                    }
                    else if (g_tMenuSys.Cursor == 3)
                    {
                        g_MainStatus = MS_USB_EMMC;
                    }    
                    else if (g_tMenuSys.Cursor == 4)
                    {
                        g_MainStatus = MS_FILE_MANAGE;
                    }
                    else if (g_tMenuSys.Cursor == 5)    /* 重启 */
                    {
                        ResetReq = 1;   
                    }                     
                    break;

                case KEY_LONG_UP_S:     /* 长按弹起 */
                    if (ResetReq == 1)
                    {
                        /* 复位进入APP */
                        *(uint32_t *)0x20000000 = 0;
                        NVIC_SystemReset(); /* 复位CPU */
                    }                     
                    break;
                    
                case KEY_UP_C: /* C键 下 */
                    LCD_MoveDownMenu(&g_tMenuSys);
                    break;

                case KEY_LONG_DOWN_C: /* C键长按 */
                    PlayKeyTone();
                    s_enter_sub_menu = 0;
                                
                    g_MainStatus = MS_EXTEND_MENU1;
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
    char buf[128];
    uint8_t line = 0;

    DispHeader2(91, "硬件信息");

    /* 检测CPU ID */
    {
        uint32_t id[3];

        bsp_GetCpuID(id);

        sprintf(buf, "STM32H750IBK6  %dMHz", SystemCoreClock / 1000000);
        DispInfoBar16(line++, "CPU:", buf);

        sprintf(buf, "%08X %08X %08X", id[0], id[1], id[2]);
        DispInfoBar16(line++, "  ", buf);
    }

    /* 显示TFT控制器型号和屏幕分辨率 */
    {
        sprintf(buf, "ST7789 / 240x240");
        DispInfoBar16(line++, "LCD:", buf);
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
            sprintf(buf, "%d.%02dGB Ok", kb / (1024 * 1024), (kb % (1024 * 1024)) / 10000);
            DispInfoBar16(line++, "eMMC:", buf);
        }
        else
        {
            sprintf(buf, "Error, BlockSize %d", CardInfo.LogBlockSize);
            DispInfoBar16Ex(line++, "eMMC:", buf, CL_RED);
        }
    }

    /* 测试I2C设备 */
    {
        if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
        {
            sprintf(buf, "24C16 Ok (0x%02X)", EE_DEV_ADDR);
            DispInfoBar16(line++, "EEPROM:", buf);
        }
        else
        {
            sprintf(buf, "24C16 Err (0x%02X)", EE_DEV_ADDR);
            DispInfoBar16Ex(line++, "EEPROM:", buf, CL_RED);
        }

        if (i2c_CheckDevice(MCP4018_SLAVE_ADDRESS) == 0)
        {
            sprintf(buf, "MCP4018 Ok (0x%02X)", MCP4018_SLAVE_ADDRESS);
            DispInfoBar16(line++, "POT:", buf);
        }
        else
        {
            sprintf(buf, "MCP4018 Err (0x%02X)", MCP4018_SLAVE_ADDRESS);
            DispInfoBar16Ex(line++, "POT:", buf, CL_RED);
        }
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
            sprintf(buf, "W25Q256 Ok, %08X", id);
            DispInfoBar16(line++, "QSPI:", buf);
        }
        else
        {
            sprintf(buf, "QSPI : W25Q256 Err, %08X", id);
            DispInfoBar16Ex(line++, "QSPI:", buf, CL_RED);
        }
    }

    /* 以太网MAC */
    {
        uint32_t PhyID[2];

        /*
            PhyID[0].15:0  = OUI:3:18
            PhyID[1].15:10 = OUI.24:19
       + 
       、【开模 
            PhyID[1].9:4 = Model Number
            PhyID[1].3:0 = Revision Number
        */
        ETH_PHY_IO_ReadReg(0, 2, &PhyID[0]);
        ETH_PHY_IO_ReadReg(0, 3, &PhyID[1]);

        if (((PhyID[1] >> 4) & 0x2F) == 0x0F)
        {
            sprintf(buf, "LAN8720 Ok");
            DispInfoBar16(line++, "Eth Phy:", buf);
        }
        else
        {
            sprintf(buf, "LAN8720 Error");
            DispInfoBar16Ex(line++, "Eth Phy:", buf, CL_RED);
        }
    }

    /* WiFi MAC */
    {
        //uint16_t mac[6];

        //ESP32_GetMac(&mac);
    }
    
    /* 固件版本 */
    {
        sprintf(buf, "%d.%02X", BOOT_VERSION >> 8, BOOT_VERSION & 0xFF);
        DispInfoBar16(line++, "Boot Ver:", buf);  

        sprintf(buf, "%d.%02X", APP_VERSION >> 8, APP_VERSION & 0xFF);
        DispInfoBar16(line++, "App Ver:", buf);  
    }

    bsp_StartAutoTimer(0, 1000);
    while (g_MainStatus == MS_HARD_INFO)
    {
        bsp_Idle();

        /* 显示时钟 */
        if (bsp_CheckTimer(0))
        {
            FONT_T tFont;
            uint16_t x, y;

            tFont.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
            tFont.FrontColor = CL_WHITE;            /* 字体颜色 */
            tFont.BackColor = HEAD_BACK_COLOR;      /* 文字背景颜色 */
            tFont.Space = 0;                        /* 文字间距，单位 = 像素 */

            RTC_ReadClock();    /* 读时钟，结果在 g_tRTC */

            x = 5;
            y = LCD_GetHeight() - 20;

            sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d",
                            g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
            LCD_DispStr(x, y, buf, &tFont);
        }

        ucKeyCode = bsp_GetKey();   /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
                case KEY_UP_C:      /* C键 下 */
                    break;

                case KEY_LONG_DOWN_C:    /* C键长按 */
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
*    函 数 名: status_ModifyParam
*    功能说明: 修改常用参数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
#define PARAM_NUM       4
void status_ModifyParam(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh = 1;
    uint8_t fSaveParam = 0;
    uint8_t cursor = 0;
    char buf[48];
    uint8_t ucIgnoreKey = 1; 
    uint8_t active;

    DispHeader2(92, "参数设置");
    DispHelpBar("长按S键选择参数",
                "短按S、C键修改参数值");
    
    while (g_MainStatus == MS_MODIFY_PARAM)
    {
        bsp_Idle();

        if (fRefresh == 1)
        {
            fRefresh = 0;
            
            /* 第1个参数 */
            {
                if (cursor == 0)
                {
                    active = 1;       
                }
                else 
                {
                    active = 0;
                }
                if (g_tParam.KeyToneEnable == 0)
                {               
                    sprintf(buf, "关闭");
                    DispParamBar(0, "1 按键音:", buf, active);
                }
                else
                {
                    sprintf(buf, "打开");
                    DispParamBar(0, "1 按键音:", buf, active);
                }                
            }
            
            /* 第2个参数 - 界面风格  */
            {
                if (cursor == 1)
                {
                    active = 1;       
                }
                else 
                {
                    active = 0;
                }
                sprintf(buf, "%3d", g_tParam.UIStyle);
                DispParamBar(1, "2 主 题:", buf, active);
            }   

            /* 第3个参数 - 屏保  */
            {
                uint16_t min;
                
                if (cursor == 2)
                {
                    active = 1;       
                }
                else 
                {
                    active = 0;
                }
                
                min = GetSleepTimeMinute();
                if (min == 0)
                {
                    sprintf(buf, "关闭");
                }
                else
                {
                    sprintf(buf, "%3d 分钟", min);
                }
                DispParamBar(2, "3 屏 保:", buf, active);
            }      
            
            /* 第3个参数 - 文件列表字体  */
            {
                if (cursor == 3)
                {
                    active = 1;       
                }
                else 
                {
                    active = 0;
                }
                
                if (g_tParam.FileListFont24 == 1)
                {
                    sprintf(buf, "24点阵");
                }
                else
                {
                    sprintf(buf, "16点阵");
                }
                DispParamBar(3, "4 列表字体:", buf, active);
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
                    else if (cursor == 2)
                    {
                        if (g_tParam.LcdSleepTime > 0)
                        {
                            g_tParam.LcdSleepTime--;
                        }
                        else
                        {
                            g_tParam.LcdSleepTime = 4;
                        }
                    }  
                    else if (cursor == 3)
                    {
                        if (g_tParam.FileListFont24 == 0)
                        {
                            g_tParam.FileListFont24 = 1;
                        } 
                        else
                        {
                            g_tParam.FileListFont24 = 0;
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
                    else if (cursor == 2)
                    {
                        if (++g_tParam.LcdSleepTime > 4)
                        {
                            g_tParam.LcdSleepTime = 0;
                        } 
                    }         
                    else if (cursor == 3)
                    {
                        if (g_tParam.FileListFont24 == 0)
                        {
                            g_tParam.FileListFont24 = 1;
                        } 
                        else
                        {
                            g_tParam.FileListFont24 = 0;
                        }
                    }                 
                    fRefresh = 1;
                    fSaveParam = 1;
                    break;

                case KEY_LONG_DOWN_S:        /* S键长按 - 选择参数 */
                    if (++cursor >= PARAM_NUM)
                    {
                        cursor = 0;
                    }
                    ucIgnoreKey = 1;
                    fRefresh = 1;
                    break;

                case KEY_LONG_DOWN_C:        /* C键长按 - 返回 */
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
    uint8_t isp_flag = 0;

    DispHeader2(92, "ESP32固件升级");
    DispHelpBar("S键切换模式",
                "");
    
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
                DispInfoBar16(1, "当前模式:", "AT指令 ");  
            }
            else
            {
                DispInfoBar16(1, "当前模式:", "ISP升级");  
            }
        }

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
                case KEY_UP_S: /* S键 弹起 */
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
                    fRefresh = 1;
                    break;

                case KEY_LONG_DOWN_S: /* S键 上 */
                    break;

                case KEY_LONG_DOWN_C: /* C键长按 */
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

/*
*********************************************************************************************************
*    函 数 名: status_UsbEMMC
*    功能说明: USB虚拟磁盘，eMMC磁盘
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_UsbEMMC(void)
{
    uint8_t ucKeyCode;          /* 按键代码 */
    
    DispHeader2(93, "USB eMMC磁盘");
    DispHelpBar("请在电脑操作eMMC文件",
                "");
    
    usbd_CloseCDC();    
    usbd_OpenMassStorage();

    DispInfoBar16(1, "USB模式:", "虚拟磁盘");      

    while (g_MainStatus == MS_USB_EMMC)
    {
        bsp_Idle();

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
                case KEY_UP_S:      /* S键 弹起 */
                    break;

                case KEY_UP_C:      /* C键 下 */
                    break;

                case KEY_LONG_DOWN_S:    /* S键 上 */
                    break;

                case KEY_LONG_DOWN_C:    /* C键长按 */
                    PlayKeyTone();
                    g_MainStatus = MS_SYSTEM_SET;
                    break;

                default:
                    break;
            }
        }
    }

	usbd_CloseMassStorage();

    usbd_OpenCDC2(COM_USB_PC);   /* 映射到串口8. 和PC软件联机 */
}

/*
*********************************************************************************************************
*    函 数 名: status_FileManage
*    功能说明: 文件管理，维护。写字库，刷boot。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
/* 按钮坐标定义 */
#define BTN1_X     5
#define BTN1_Y     50
#define BTN1_H     28 
#define BTN1_W     100 
#define BTN1_TEXT  "写QSPI字库"

#define BTN2_X     BTN1_X
#define BTN2_Y     BTN1_Y + BTN1_H + 5 
#define BTN2_H     BTN1_H 
#define BTN2_W     BTN1_W 
#define BTN2_TEXT  "刷新boot"

#define BTN3_X     BTN1_X
#define BTN3_Y     BTN1_Y + 2 * (BTN1_H + 5)
#define BTN3_H     BTN1_H 
#define BTN3_W     BTN1_W 
#define BTN3_TEXT  "写QSPI App"

#define BTN4_X     (5 + 120)
#define BTN4_Y     BTN1_Y
#define BTN4_H     BTN1_H 
#define BTN4_W     BTN1_W 
#define BTN4_TEXT  "----"

#define BTN5_X     BTN4_X
#define BTN5_Y     BTN1_Y + BTN1_H + 5 
#define BTN5_H     BTN1_H 
#define BTN5_W     BTN1_W 
#define BTN5_TEXT  "----"

#define BTN6_X     BTN4_X
#define BTN6_Y     BTN1_Y + 2 * (BTN1_H + 5)
#define BTN6_H     BTN1_H 
#define BTN6_W     BTN1_W 
#define BTN6_TEXT  "----"

#define BTN_NUM     6

void status_FileManage(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    FONT_T tFont16;
    uint8_t cursor = 0;
    BUTTON_T btn1, btn2, btn3, btn4, btn5, btn6;
    
    DispHeader2(94, "数据维护");
//    DispHelpBar("S、C键短按切换焦点",
//                "S键长按执行");
    
    /* 设置字体参数 */
    {
        tFont16.FontCode = FC_ST_16;          /* 字体代码 16点阵 */
        tFont16.FrontColor = INFO_NAME_COLOR; /* 字体颜色 */
        tFont16.BackColor = CL_MASK;          /* 文字背景颜色 */
        tFont16.Space = 0;                    /* 文字间距，单位 = 像素 */
    } 
    
    fRefresh = 1;
    while (g_MainStatus == MS_FILE_MANAGE)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

			{
				btn1.Left = BTN1_X;
				btn1.Top = BTN1_Y;
				btn1.Height = BTN1_H;
				btn1.Width = BTN1_W;
				btn1.pCaption = BTN1_TEXT;
				btn1.Font =  &tFont16;
				btn1.Focus = 0;

				btn2.Left = BTN2_X;
				btn2.Top = BTN2_Y;
				btn2.Height = BTN2_H;
				btn2.Width = BTN2_W;
				btn2.pCaption = BTN2_TEXT;
				btn2.Font =  &tFont16;
				btn2.Focus = 0;

				btn3.Left = BTN3_X;
				btn3.Top = BTN3_Y;
				btn3.Height = BTN3_H;
				btn3.Width = BTN3_W;
				btn3.pCaption = BTN3_TEXT;
				btn3.Font =  &tFont16;
				btn3.Focus = 0;

				btn4.Left = BTN4_X;
				btn4.Top = BTN4_Y;
				btn4.Height = BTN4_H;
				btn4.Width = BTN4_W;
				btn4.pCaption = BTN4_TEXT;
				btn4.Font =  &tFont16;
				btn4.Focus = 0;
                
				btn5.Left = BTN5_X;
				btn5.Top = BTN5_Y;
				btn5.Height = BTN5_H;
				btn5.Width = BTN5_W;
				btn5.pCaption = BTN5_TEXT;
				btn5.Font =  &tFont16;
				btn5.Focus = 0;

				btn6.Left = BTN6_X;
				btn6.Top = BTN6_Y;
				btn6.Height = BTN6_H;
				btn6.Width = BTN6_W;
				btn6.pCaption = BTN6_TEXT;
				btn6.Font =  &tFont16;
				btn6.Focus = 0;
				
				if (cursor == 0) btn1.Focus = 1;
				else if (cursor == 1) btn2.Focus = 1;
				else if (cursor == 2) btn3.Focus = 1;
                else if (cursor == 3) btn4.Focus = 1;
                else if (cursor == 4) btn5.Focus = 1;
                else if (cursor == 5) btn6.Focus = 1;
				
				LCD_DrawButton(&btn1);
				LCD_DrawButton(&btn2);
				LCD_DrawButton(&btn3);
                LCD_DrawButton(&btn4);
                LCD_DrawButton(&btn5);
                LCD_DrawButton(&btn6);
			} 			
        }

        bsp_Idle();
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
                case KEY_UP_S:      /* S键释放 - 移动按钮焦点*/    
                    if (++cursor == BTN_NUM)
                    {
                        cursor = 0;
                    }
                    fRefresh = 1;
                    break;

               case KEY_UP_C:      /* C键释放 - 确认执行按钮功能 */ 		
                    if (cursor > 0)
                    {
                        cursor--;
                    }             
                    else
                    {
                        cursor = BTN_NUM - 1;
                    }
                    fRefresh = 1;
                    break;

                case KEY_LONG_DOWN_S:       /* S键长按 - 确认后，闪烁，修改参数 */      
                    if (cursor == 0)
                    {
                        FM_WirteFont();     /* 写字库 */
                    }
                    else if (cursor == 1)
                    {
                        FM_WirteBoot();     /* 写QSPI App */
                    }
                    else if (cursor == 2)
                    {
                        FM_WirteQspiApp();
                    }	                
                    break;
                
                case KEY_LONG_DOWN_C:    /* C键长按 */
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
*    函 数 名: FM_DispFileInfo
*    功能说明: 显示文件操作信息
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void FM_DispFileInfo(char *_str1)
{
    FONT_T tFont;   /* 定义字体结构体变量 */

    tFont.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
    tFont.FrontColor = HELP_TEXT_COLOR;     /* 字体颜色 */
    tFont.BackColor = HELP_BACK_COLOR;      /* 文字背景颜色 */
    tFont.Space = 0;                        /* 文字间距，单位 = 像素 */
    
    LCD_DispStrEx(5, 180, _str1, &tFont, 230, 0);
    ST7789_DrawScreen();    /* 硬件SPI+DMA+刷屏 */    
}

/*
*********************************************************************************************************
*    函 数 名: FM_DispProgress
*    功能说明: 显示文件操作进度和时间
*    形    参: _percent 百分比
*              _time1 时间ms单位
*    返 回 值: 无
*********************************************************************************************************
*/
void FM_DispProgress(uint8_t _percent, uint32_t _time1)
{
    FONT_T tFont;   /* 定义字体结构体变量 */
    static uint8_t s_percent = 200;
    static uint32_t s_ms = 0;    
    char buf[10];    

    tFont.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
    tFont.FrontColor = HELP_TEXT_COLOR;     /* 字体颜色 */
    tFont.BackColor = CL_MASK;      		/* 文字背景颜色 */
    tFont.Space = 0;                        /* 文字间距，单位 = 像素 */
	
    if (s_percent == 0 || s_percent != _percent || s_ms != _time1 / 100)
    {
        /* 100.2 秒 */
        sprintf(buf, "%d.%d 秒", _time1 / 1000, (_time1 % 1000) / 100);
        
        DispProgressBar(5, 200, 24, 240 - 2 * 5, "", _percent, buf, &tFont); 

//        LCD_DispStr(160, 200 + 4, buf, &tFont);
        ST7789_DrawScreen();    /* 硬件SPI+DMA+刷屏 */ 

        s_percent = _percent;
        s_ms = _time1 / 100;
    }    
}

/*
*********************************************************************************************************
*    函 数 名: FM_WirteQSPIFont
*    功能说明: 写字库
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
#define QSPI_WRITE_PAGE_SIZE	256
#define QSPI_ERASE_SECTOR_SIZE	(4 * 1024)
#define QSPI_FONT_ADDRESS		((32 - 2) * 1024 * 1024)
#define QSPI_FILE_SIZE			(2 * 1024 * 1024)
#define QSPI_WRITE_PAGE_SIZE    256
#define QSPI_ERASE_SECTOR_SIZE  (4 * 1024)
#define QSPI_BEGIN_ADDRESS      ((32 - 2) * 1024 * 1024)
#define QSPI_FILE_SIZE          (2 * 1024 * 1024)
static void FM_WirteFont(void)
{
    uint32_t i;
    FRESULT result;
    uint32_t bw;
    char *path = "0:/H7-TOOL/Fonts/GB2312ZK.bin";
    uint32_t FileSize;
    uint32_t OffsetAddr = 0;
    uint8_t percent;
    int32_t time1;

    FM_DispProgress(0, 0); 
    /* 改程序仅在CPU 内部运行时才能执行 */
    if ((uint32_t)FM_WirteFont < 0x08000000 || (uint32_t)FM_WirteFont > 0x08100000)
    {
        FM_DispFileInfo("只能在CPU内部Flash执行");
        return;
    }
        
    result = f_open(&g_file, path, FA_OPEN_EXISTING | FA_READ);
    if (result !=  FR_OK)
    {
        FM_DispFileInfo("没有找到文件GB2312ZK.bin");
        goto quit1;
    }

    /* 获得文件大小 */
    {
        FILINFO fno;

        f_stat(path, &fno);
        FileSize = fno.fsize;

        /* 文件大小 */
        if (FileSize  != 2 * 1024 * 1024)
        {
            FM_DispFileInfo("字库文件长度必须是2MB");
            goto quit1;
        }
    }

    time1 = bsp_GetRunTime();
    /* 擦除 */
    FM_DispFileInfo("正在擦除QSPI Flash字库...");
    OffsetAddr = QSPI_FONT_ADDRESS;
    for (i = 0; i < QSPI_FILE_SIZE / QSPI_ERASE_SECTOR_SIZE; i++)
    {
        QSPI_EraseSector(OffsetAddr);
        OffsetAddr += QSPI_ERASE_SECTOR_SIZE;
        percent = ((i + 1) * 100) / (QSPI_FILE_SIZE / QSPI_ERASE_SECTOR_SIZE);
        FM_DispProgress(percent, bsp_CheckRunTime(time1));        
    }

    /* 写入，每次256字节 */
    FM_DispFileInfo("正在写入...");
    percent = 0;
    FM_DispProgress(percent, bsp_CheckRunTime(time1));    
    OffsetAddr = QSPI_FONT_ADDRESS;
    for (i = 0; i < QSPI_FILE_SIZE / 1024; i++)
    {
        percent = ((i + 1) * 100) / (QSPI_FILE_SIZE / 1024);

        result = f_read(&g_file, &qspi_file_buf, 1024, &bw);
        if (result != FR_OK)
        {
            FM_DispFileInfo("文件读取失败");
            goto quit1;
        }
        
        SCB_InvalidateDCache_by_Addr((uint32_t *)qspi_file_buf,  sizeof(qspi_file_buf));
        
        QSPI_WriteBuffer(qspi_file_buf, OffsetAddr, 256);
        QSPI_WriteBuffer(qspi_file_buf + 256, OffsetAddr + 256, 256);
        QSPI_WriteBuffer(qspi_file_buf + 2 * 256, OffsetAddr + 2 * 256, 256);
        QSPI_WriteBuffer(qspi_file_buf + 3 * 256, OffsetAddr + 3 * 256, 256);

        QSPI_ReadBuffer(qspi_read_buf, OffsetAddr, 1024);
        
        SCB_InvalidateDCache_by_Addr((uint32_t *)qspi_read_buf,  sizeof(qspi_read_buf));
        if (memcmp(qspi_file_buf, qspi_read_buf, 1024) != 0)
        {
            FM_DispFileInfo("写QSPI Flash出错!");
            goto quit1;
        }

        OffsetAddr += 1024;
        percent = ((i + 1) * 100) / (QSPI_FILE_SIZE / 1024);
        FM_DispProgress(percent, bsp_CheckRunTime(time1));
    }

    FM_DispFileInfo("QSPI Flash字库成功!");
    
quit1:
    /* 关闭文件*/
    f_close(&g_file);
}

/*
*********************************************************************************************************
*    函 数 名: FM_WirteQspiApp
*    功能说明: 写QSPI APP
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
#define QSPI_APP_ADDRESS   0
static void FM_WirteQspiApp(void)
{
    uint32_t i;
    FRESULT result;
    uint32_t bw;
    char *path = "0:/H7-TOOL/Firmware/H7-QSPI_APP.bin";
    uint32_t FileSize;
    uint32_t OffsetAddr = 0;
    uint8_t percent;
    int32_t time1;

    FM_DispProgress(0, 0); 
    
    /* 改程序仅在CPU 内部运行时才能执行 */
    if ((uint32_t)FM_WirteFont < 0x08000000 || (uint32_t)FM_WirteFont > 0x08100000)
    {
        FM_DispFileInfo("只能在CPU内部Flash执行");
        return;
    }
        
    result = f_open(&g_file, path, FA_OPEN_EXISTING | FA_READ);
    if (result !=  FR_OK)
    {
        FM_DispFileInfo("没有找到文件H7-QSPI_APP.bin");
        goto quit1;
    }
    
    /* 获得文件大小 */
    {
        FILINFO fno;

        f_stat(path, &fno);
        FileSize = fno.fsize;

        /* 文件大小 */
        if (FileSize  > 30 * 1024 * 1024)
        {
            FM_DispFileInfo("文件长度必须小于30MB");
            goto quit1;
        }
    }

    time1 = bsp_GetRunTime();
    /* 擦除 */
    FM_DispFileInfo("正在擦除QSPI Flash...");
    OffsetAddr = QSPI_APP_ADDRESS;
    for (i = 0; i < (FileSize + QSPI_ERASE_SECTOR_SIZE - 1) / QSPI_ERASE_SECTOR_SIZE; i++)
    {
        QSPI_EraseSector(OffsetAddr);
        OffsetAddr += QSPI_ERASE_SECTOR_SIZE;
        percent = ((i + 1) * 100) / ((FileSize + QSPI_ERASE_SECTOR_SIZE - 1) / QSPI_ERASE_SECTOR_SIZE);
        FM_DispProgress(percent, bsp_CheckRunTime(time1));        
    }

    /* 写入，每次256字节 */
    FM_DispFileInfo("正在写入...");
    percent = 0;
    FM_DispProgress(percent, bsp_CheckRunTime(time1));    
    OffsetAddr = QSPI_APP_ADDRESS;
    for (i = 0; i < (FileSize + 1024 - 1) / 1024; i++)
    {
        percent = ((i + 1) * 100) / ((FileSize + 1024 - 1) / 1024);

        memset(qspi_file_buf, 0, 1024);   /* 末尾填0 */
        result = f_read(&g_file, &qspi_file_buf, 1024, &bw);
        if (result != FR_OK)
        {
            FM_DispFileInfo("文件读取失败");
            goto quit1;
        }
        
        
        SCB_InvalidateDCache_by_Addr((uint32_t *)qspi_file_buf,  sizeof(qspi_file_buf));
        
        QSPI_WriteBuffer(qspi_file_buf, OffsetAddr, 256);
        QSPI_WriteBuffer(qspi_file_buf + 256, OffsetAddr + 256, 256);
        QSPI_WriteBuffer(qspi_file_buf + 2 * 256, OffsetAddr + 2 * 256, 256);
        QSPI_WriteBuffer(qspi_file_buf + 3 * 256, OffsetAddr + 3 * 256, 256);

        QSPI_ReadBuffer(qspi_read_buf, OffsetAddr, 1024);
        
        SCB_InvalidateDCache_by_Addr((uint32_t *)qspi_read_buf,  sizeof(qspi_read_buf));
        if (memcmp(qspi_file_buf, qspi_read_buf, 1024) != 0)
        {
            FM_DispFileInfo("写QSPI Flash出错!");
            goto quit1;
        }

        OffsetAddr += 1024;
        percent = ((i + 1) * 100) / ((FileSize + 1024 - 1) / 1024);
        FM_DispProgress(percent, bsp_CheckRunTime(time1));
    }

    FM_DispFileInfo("写QSPI Flash成功!");
    
quit1:
    /* 关闭文件*/
    f_close(&g_file);
}

/*
*********************************************************************************************************
*    函 数 名: FM_WirteBoot
*    功能说明: 写BOOT
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
#define BOOT_BEGIN_ADDRESS  0x08000000
static void FM_WirteBoot(void)
{
    uint32_t i;
    FRESULT result;
    uint32_t bw;
    char *path = "0:/H7-TOOL/Firmware/H7-BOOT.bin";
    uint32_t FileSize;
    uint32_t OffsetAddr = 0;
    uint8_t percent;
    int32_t time1;
    
    FM_DispProgress(0, 0); 
    result = f_open(&g_file, path, FA_OPEN_EXISTING | FA_READ);
    if (result !=  FR_OK)
    {
        FM_DispFileInfo("没有找到文件H7-BOOT.bin");
        goto quit1;
    }

    /* 获得文件大小 */
    {
        FILINFO fno;

        f_stat(path, &fno);
        FileSize = fno.fsize;

        /* 文件大小 */
        if (FileSize  > 128 * 1024)
        {
            FM_DispFileInfo("文件长度大于128KB");
            goto quit1;
        }
    }
    
    /* 简单判断下文件是否boot固件 */
    {
        result = f_read(&g_file, &qspi_file_buf, 512, &bw);
        if (result != FR_OK)
        {
            FM_DispFileInfo("文件读取失败");
            goto quit1;
        }
        
        SCB_InvalidateDCache_by_Addr((uint32_t *)qspi_file_buf, 512);
        // 0x2408EDF8 0x080002AD
        // 0x20020000 0x080002AD
        if ((qspi_file_buf[3] == 0x24 || qspi_file_buf[3] == 0x20) 
            && (qspi_file_buf[7] == 0x08) && (qspi_file_buf[6] == 0x00))
        {
            /* 合法固件 */;
        }
        else
        {
            FM_DispFileInfo("不是固件文件");
            goto quit1;
        }     

        /* 还原文件指针到开头 */
        f_lseek(&g_file, 0);
    }
    
    time1 = bsp_GetRunTime();
    
    /* 擦除 */
    FM_DispFileInfo("正在擦除Flash...");
    percent = 0;
    FM_DispProgress(percent, bsp_CheckRunTime(time1)); 
    OffsetAddr = BOOT_BEGIN_ADDRESS;
    if (bsp_EraseCpuFlash(BOOT_BEGIN_ADDRESS) != HAL_OK)
    {
        percent = 100;
        FM_DispProgress(percent, bsp_CheckRunTime(time1)); 
    }
        
    /* 写入，每次1024字节 */
    FM_DispFileInfo("正在写入...");
    percent = 0;
    FM_DispProgress(percent, bsp_CheckRunTime(time1));    
    OffsetAddr = BOOT_BEGIN_ADDRESS;
    for (i = 0; i < (FileSize + 1023) / 1024; i++)
    {
        percent = ((i + 1) * 100) / ((FileSize + 1023) / 1024);
        
        memset(qspi_file_buf, 0, 1024);   /* 末尾填0 */
        result = f_read(&g_file, &qspi_file_buf, 1024, &bw);
        if (result != FR_OK)
        {
            FM_DispFileInfo("文件读取失败");
            goto quit1;
        }
        
        SCB_InvalidateDCache_by_Addr((uint32_t *)qspi_file_buf,  sizeof(qspi_file_buf));
        
        if (bsp_WriteCpuFlash(OffsetAddr, qspi_file_buf, 1024) == 0)		/* 每次写入1024个字节 */
        {
            if (memcmp((uint8_t *)OffsetAddr, qspi_file_buf, 1024) != 0)
            {
                FM_DispFileInfo("写CPU Flash出错!");
                goto quit1;
            }            
        }
        else
        {
            FM_DispFileInfo("写CPU Flash出错!");
            goto quit1;           
        }
        
        OffsetAddr += 1024;
        percent = ((i + 1) * 100) / ((FileSize + 1023) / 1024);
        FM_DispProgress(percent, bsp_CheckRunTime(time1));
    }

    FM_DispFileInfo("Boot写入成功!");
    
quit1:
    /* 关闭文件*/
    f_close(&g_file);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
