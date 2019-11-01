/*
*********************************************************************************************************
*
*    模块名称 : 收音机测试。
*    文件名称 : fm_radio.c
*    版    本 : V1.1
*    说    明 : 主要用于测试Si4730收音芯片的功能
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2013-01-01  armfly  正式发布
*        V1.1    2014-09-06  armfly  完善音量调节函数。WM8978和Si47XX芯片的音量同步调节。
*                                    Volume 单词显示错误. 支持Si4704， 增加FM 调谐电容显示。
*
*    Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h" /* printf函数定向输出到串口，所以必须包含这个文件 */
#include "form_fm_radio.h"
#include "param.h"

/* 返回按钮的坐标(屏幕右下角) */
#define BUTTON_RET_H 32
#define BUTTON_RET_W 60
#define BUTTON_RET_X (g_LcdWidth - BUTTON_RET_W - 4)
#define BUTTON_RET_Y (g_LcdHeight - BUTTON_RET_H - 4)

#define BUTTON_RET2_H 32
#define BUTTON_RET2_W 120
#define BUTTON_RET2_X (BUTTON_RET_X - BUTTON_RET2_W - 5)
#define BUTTON_RET2_Y BUTTON_RET_Y
#define BUTTON_RET2_TEXT "保持收音,返回"

#define BUTTON_FM_H 32
#define BUTTON_FM_W 60
#define BUTTON_FM_X 5
#define BUTTON_FM_Y 120

#define BUTTON_AM_H 32
#define BUTTON_AM_W 60
#define BUTTON_AM_X (BUTTON_FM_X + 70)
#define BUTTON_AM_Y BUTTON_FM_Y

#define BUTTON_VOLP_H 32
#define BUTTON_VOLP_W 48
#define BUTTON_VOLP_X 32
#define BUTTON_VOLP_Y 48

#define BUTTON_CHP_H 32
#define BUTTON_CHP_W 48
#define BUTTON_CHP_X 32
#define BUTTON_CHP_Y 48

#define BUTTON_FREQ_ (BUTTON_FM_X + 70)
#define BUTTON__Y BUTTON_FM_Y

/* 以下是检查框 */
#define CHECK_SPK_X BUTTON_FM_X
#define CHECK_SPK_Y (BUTTON_FM_Y + BUTTON_FM_H + 5)
#define CHECK_SPK_H CHECK_BOX_H
#define CHECK_SPK_W (CHECK_BOX_W + 5 * 16) /* 决定触摸有效区域 */
#define CHECK_SPK_TEXT "打开扬声器"

#define CHECK_RSSI_X BUTTON_FM_X
#define CHECK_RSSI_Y (CHECK_SPK_Y + 30)
#define CHECK_RSSI_H CHECK_BOX_H
#define CHECK_RSSI_W (CHECK_BOX_W + 14 * 16) /* 决定触摸有效区域 */
#define CHECK_RSSI_TEXT "显示信号质量（可能引起噪音）"

#define CHECK_LIST_X BUTTON_FM_X
#define CHECK_LIST_Y (CHECK_RSSI_Y + 30)
#define CHECK_LIST_H CHECK_BOX_H
#define CHECK_LIST_W (CHECK_BOX_W + 5 * 16) /* 决定触摸有效区域 */
#define CHECK_LIST_TEXT "选择全国电台列表"

/*
AM873，FM88.4，武汉人民广播电台（新闻综合频率）



    武汉地区FM电台频率表：
    88.4

    FM89.6 武汉广播电台交通台
        90.7
        91.2
    FM91.6 楚天广播电台卫星台
    
    FM92.7 楚天广播电台交通体育台
    FM93.6 武汉广播电台中波台
    FM95.6 中央广播电台中国之声
    96.0==
    FM96.6 湖北广播电台生活频道
    
    FM97.8 中央广播电台经济之声
    
    FM99.8 湖北广播电台经济频道
    FM100.6 长江经济广播电台
    FM101.8 武汉广播电台文艺台
    FM102.6 湖北广播电台妇女儿童频道
    FM103.8 湖北广播电台音乐频道
    FM104.6 湖北广播电台中波频道
    FM105.8 楚天广播电台音乐台
    FM107.8 湖北广播电台交通频道
*/

const uint16_t g_InitListFM[] = {
        8640,
        8840,
        8960,
        9060,
        9120,
        9270,
        9360,
        9440,
        9560,
        9660,
        9780,
        9980,
        10060,
        10180,
        10260,
        10380,
        10460,
        10580,
        10780};

const uint16_t g_InitListAM[] = {
        531,
        540,
        549,
        558,
        603,
        639,
        855,
        873,
        882,
        900,
        909,
        918,
        927,
};

RADIO_T g_tRadio;

static void radio_DispStatus(void);
static void radio_SignalQuality(void);
static void radio_FM_FreqList(uint8_t _ucAll);
static void radio_AM_FreqList(uint8_t _ucAll);
static void radio_LoadParam(void);
static void radio_SaveParam(void);
static void radio_AdjustVolume(uint8_t _ucVolume);

/*
*********************************************************************************************************
*    函 数 名: RadioMain
*    功能说明: 收音机主程序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void RadioMain(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t ucTouch;     /* 触摸事件 */
    uint8_t fRefresh;    /* 刷屏请求标志,1表示需要刷新 */
    uint8_t fQuit = 0;
    FONT_T tFont, tFontBtn, tFontChk; /* 定义一个字体结构体变量，用于设置字体参数 */

    char buf[128];
    uint16_t x, y;
    uint16_t usLineCap = 18;

    int16_t tpX, tpY;
    BUTTON_T tBtn;
    CHECK_T tCheck;

    LCD_ClrScr(CL_BLUE); /* 清屏，背景蓝色 */

    radio_LoadParam(); /* 读取电台和音量参数 */

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE; /* 字体颜色 */
        tFont.BackColor = CL_BLUE;     /* 文字背景颜色 */
        tFont.Space = 0;                         /* 文字间距，单位 = 像素 */

        /* 按钮字体 */
        tFontBtn.FontCode = FC_ST_16;
        tFontBtn.BackColor = CL_MASK; /* 透明色 */
        tFontBtn.FrontColor = CL_BLACK;
        tFontBtn.Space = 0;

        /* CheckBox 字体 */
        tFontChk.FontCode = FC_ST_16;
        tFontChk.BackColor = CL_MASK; /* 透明色 */
        tFontChk.FrontColor = CL_YELLOW;
        tFontChk.Space = 0;
    }

    x = 5;
    y = 3;
    LCD_DispStr(x, y, "Si4730/Si4704 收音机", &tFont); /* 在(8,3)坐标处显示一串汉字 */
    y += usLineCap;

    /* 测试I2C设备 */
    {
        if (i2c_CheckDevice(I2C_ADDR_SI4730_W) == 0)
        {
            sprintf(buf, "Si47xx Ok (0x%02X)", I2C_ADDR_SI4730_W);
            printf("%s\r\n", buf);
            LCD_DispStr(x, y, buf, &tFont);

            wm8978_OutMute(1); /* 先静音输出, 避免上电嘎哒声 */

            SI4730_PowerUp_FM_Revice();
            bsp_DelayMS(10);

            y += usLineCap;
            /* 打印芯片的版本 */
            {
                uint8_t read_buf[8];

                if (SI4730_GetRevision(read_buf))
                {
                    sprintf(buf, "%d-%c%c-%02X%02X-%c%c-%c",             /* Si4730 */
                                    read_buf[0], read_buf[1], read_buf[2], /* 固件版本 */
                                    read_buf[3], read_buf[4],                             /* 补丁ID */
                                    read_buf[5], read_buf[6],                             /* 组件版本 */
                                    read_buf[7]                                                         /* 芯片硬件版本 */
                    );                                                                                         /* 芯片型号标识 */

                    if (strcmp(buf, "30-60-0000-70-D") == 0)
                    {
                        g_tRadio.ChipType = SI4730;

                        LCD_DispStr(x, y, buf, &tFont);
                        LCD_DispStr(x + 130, y, "Si4730", &tFont);
                    }
                    else if (strcmp(buf, "4-60-0000-70-D") == 0 ||
                                     strcmp(buf, "4-20-0000-20-B") == 0)
                    {
                        g_tRadio.ChipType = SI4704;
                        LCD_DispStr(x, y, buf, &tFont);
                        LCD_DispStr(x + 130, y, "Si4704", &tFont);

                        g_tRadio.ucMode = FM_RX;

                        //SI4704_SetFMIntput(1);    /* 切换天线为PCB天线 */
                        // 0 表示FM引脚输入(耳机天线)  1 表示LPI天线输入(PCB天线)
                        SI4704_SetFMIntput(0); /* 切换天线为耳机天线 */
                    }
                    else
                    {
                        tFont.FrontColor = CL_RED;
                        LCD_DispStr(x, y, buf, &tFont);
                        tFont.FrontColor = CL_WHITE;
                    }
                }
            }
        }
        else
        {
            sprintf(buf, "Si47xx Err (0x%02X)", I2C_ADDR_SI4730_W);
            printf("%s\r\n", buf);

            tFont.FrontColor = CL_RED;
            LCD_DispStr(x, y, buf, &tFont);
            tFont.FrontColor = CL_WHITE;
        }
        y += usLineCap;
    }

    /* 显示按钮 */
    {
        tBtn.Font = &tFontBtn;

        tBtn.Left = BUTTON_RET_X;
        tBtn.Top = BUTTON_RET_Y;
        tBtn.Height = BUTTON_RET_H;
        tBtn.Width = BUTTON_RET_W;
        tBtn.Focus = 0; /* 未选中 */
        tBtn.pCaption = "返回";
        LCD_DrawButton(&tBtn);

        tBtn.Left = BUTTON_RET2_X;
        tBtn.Top = BUTTON_RET2_Y;
        tBtn.Height = BUTTON_RET2_H;
        tBtn.Width = BUTTON_RET2_W;
        tBtn.Focus = 0;                                        /* 未选中 */
        tBtn.pCaption = BUTTON_RET2_TEXT; /* 保持收音 返回 */
        LCD_DrawButton(&tBtn);

        if (g_tRadio.ChipType == SI4730)
        {
            tBtn.Left = BUTTON_FM_X;
            tBtn.Top = BUTTON_FM_Y;
            tBtn.Height = BUTTON_FM_H;
            tBtn.Width = BUTTON_FM_W;
            tBtn.Focus = 0; /* 失去焦点 */
            tBtn.pCaption = "FM";
            LCD_DrawButton(&tBtn);

            tBtn.Left = BUTTON_AM_X;
            tBtn.Top = BUTTON_AM_Y;
            tBtn.Height = BUTTON_AM_H;
            tBtn.Width = BUTTON_AM_W;
            tBtn.Focus = 0; /* 失去焦点 */
            tBtn.pCaption = "AM";
            LCD_DrawButton(&tBtn);
        }

        /* 显示检查框 */
        tCheck.Font = &tFontChk;

        tCheck.Left = CHECK_SPK_X;
        tCheck.Top = CHECK_SPK_Y;
        tCheck.Height = CHECK_SPK_H;
        tCheck.Width = CHECK_SPK_W;
        if (g_tRadio.ucSpkOutEn == 1)
        {
            tCheck.Checked = 1;
        }
        else
        {
            tCheck.Checked = 0;
        }
        tCheck.pCaption = CHECK_SPK_TEXT;
        LCD_DrawCheckBox(&tCheck);

        tCheck.Left = CHECK_RSSI_X;
        tCheck.Top = CHECK_RSSI_Y;
        tCheck.Height = CHECK_RSSI_H;
        tCheck.Width = CHECK_RSSI_W;
        tCheck.Checked = 0;
        tCheck.pCaption = CHECK_RSSI_TEXT;

        LCD_DrawCheckBox(&tCheck);

        tCheck.Left = CHECK_LIST_X;
        tCheck.Top = CHECK_LIST_Y;
        tCheck.Height = CHECK_LIST_H;
        tCheck.Width = CHECK_LIST_W;
        if (g_tRadio.ucListType == 1)
        {
            tCheck.Checked = 1;
        }
        else
        {
            tCheck.Checked = 0;
        }
        tCheck.pCaption = CHECK_LIST_TEXT;
        LCD_DrawCheckBox(&tCheck);
    }

    {
        if (g_tRadio.ucSpkOutEn == 0)
        {
            /* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 */
            wm8978_CfgAudioPath(AUX_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
        }
        else
        {
            /* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 和 扬声器 */
            wm8978_CfgAudioPath(AUX_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON);
        }

        //g_tRadio.ucSpkOutEn = 0

        SI4730_PowerDown();
        bsp_DelayMS(10);

        //SI4730_PowerUp_AM_Revice();
        SI4730_PowerUp_FM_Revice();
        bsp_DelayMS(10);

        /* 调节音量 */
        radio_AdjustVolume(g_tRadio.ucVolume);

        tFont.FrontColor = CL_YELLOW;
        LCD_DispStr(x, y, "请操作摇杆换台和调节音量,K2 K3键微调频率", &tFont);
        if (g_tRadio.ChipType == SI4704)
        {
            LCD_DispStr(x, y + 75, "请将耳机插到绿色插座，用作FM天线", &tFont); /* 在(8,3)坐标处显示一串汉字 */
        }
        tFont.FrontColor = CL_WHITE;

        /* 预填武汉地区的电台列表 */
        {
            if (g_tRadio.ucListType == 0) /* 0 表示 武汉地区列表 1 表示全国列表 */
            {
                radio_FM_FreqList(0); /* 预设武汉地区的FM电台列表 */
                radio_AM_FreqList(0); /* 预设武汉地区的AM电台列表 */
                                                            //g_tRadio.ucIndexFM  = 1;
            }
            else
            {
                radio_FM_FreqList(1); /* 预设全国地区的FM电台列表 */
                radio_AM_FreqList(1); /* 预设全国地区的AM电台列表 */
                                                            //g_tRadio.ucIndexAM  = 1;
            }
            //g_tRadio.ucMode = FM_RX;    /* 缺省是FM接收 */
            //g_tRadio.ucMode = AM_RX;    /* 缺省是FM接收 */
        }

        bsp_DelayMS(300); /* 必须延迟一段时间后，才能保证每次上电后，能够选中一个台 */
        if (g_tRadio.ucMode == FM_RX)
        {
            g_tRadio.usFreq = g_tRadio.usFMList[g_tRadio.ucIndexFM];
            SI4730_SetFMFreq(g_tRadio.usFreq);
        }
        else
        {
            g_tRadio.usFreq = g_tRadio.usAMList[g_tRadio.ucIndexAM];
            SI4730_SetAMFreq(g_tRadio.usFreq);
        }

        bsp_DelayMS(100); /* 延迟100ms，避免强烈的嘎哒声 */

        wm8978_OutMute(0);
    }

    fRefresh = 1; /* 1表示需要刷新LCD */
    bsp_StartAutoTimer(0, 1000);
    while (fQuit == 0)
    {
        bsp_Idle();

        if (fRefresh)
        {
            fRefresh = 0;

            radio_DispStatus();
        }

        if (g_tRadio.ucRssiEn)
        {
            if (bsp_CheckTimer(0))
            {
                radio_SignalQuality(); /* 刷新信号质量状态栏 */
            }
        }

        ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
        if (ucTouch != TOUCH_NONE)
        {
            switch (ucTouch)
            {
            case TOUCH_DOWN: /* 触笔按下事件 */
                if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
                {
                    tBtn.Left = BUTTON_RET_X;
                    tBtn.Top = BUTTON_RET_Y;
                    tBtn.Height = BUTTON_RET_H;
                    tBtn.Width = BUTTON_RET_W;
                    tBtn.Focus = 1; /* 焦点 */
                    tBtn.pCaption = "返回";
                    LCD_DrawButton(&tBtn);
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_RET2_X, BUTTON_RET2_Y, BUTTON_RET2_H, BUTTON_RET2_W))
                {
                    tBtn.Left = BUTTON_RET2_X;
                    tBtn.Top = BUTTON_RET2_Y;
                    tBtn.Height = BUTTON_RET2_H;
                    tBtn.Width = BUTTON_RET2_W;
                    tBtn.Focus = 1; /* 焦点 */
                    tBtn.pCaption = BUTTON_RET2_TEXT;
                    LCD_DrawButton(&tBtn);
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_FM_X, BUTTON_FM_Y, BUTTON_FM_H, BUTTON_FM_W))
                {
                    if (g_tRadio.ChipType == SI4730)
                    {
                        tBtn.Left = BUTTON_FM_X;
                        tBtn.Top = BUTTON_FM_Y;
                        tBtn.Height = BUTTON_FM_H;
                        tBtn.Width = BUTTON_FM_W;
                        tBtn.Focus = 1; /* 焦点 */
                        tBtn.pCaption = "FM";
                        LCD_DrawButton(&tBtn);
                    }
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_AM_X, BUTTON_AM_Y, BUTTON_AM_H, BUTTON_AM_W))
                {
                    if (g_tRadio.ChipType == SI4730)
                    {
                        tBtn.Left = BUTTON_AM_X;
                        tBtn.Top = BUTTON_AM_Y;
                        tBtn.Height = BUTTON_AM_H;
                        tBtn.Width = BUTTON_AM_W;
                        tBtn.Focus = 1; /* 焦点 */
                        tBtn.pCaption = "AM";
                        LCD_DrawButton(&tBtn);
                    }
                }
                else if (TOUCH_InRect(tpX, tpY, CHECK_SPK_X, CHECK_SPK_Y, CHECK_SPK_H, CHECK_SPK_W))
                {
                    if (g_tRadio.ucSpkOutEn)
                    {
                        g_tRadio.ucSpkOutEn = 0;
                        tCheck.Checked = 0;

                        /* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 */
                        wm8978_CfgAudioPath(AUX_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
                    }
                    else
                    {
                        g_tRadio.ucSpkOutEn = 1;
                        tCheck.Checked = 1;

                        /* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 和 扬声器 */
                        wm8978_CfgAudioPath(AUX_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON);
                    }

                    tCheck.Left = CHECK_SPK_X;
                    tCheck.Top = CHECK_SPK_Y;
                    tCheck.Height = CHECK_SPK_H;
                    tCheck.Width = CHECK_SPK_W;
                    tCheck.pCaption = CHECK_SPK_TEXT;
                    LCD_DrawCheckBox(&tCheck);
                }
                else if (TOUCH_InRect(tpX, tpY, CHECK_RSSI_X, CHECK_RSSI_Y, CHECK_RSSI_H, CHECK_RSSI_W))
                {
                    if (g_tRadio.ucRssiEn)
                    {
                        g_tRadio.ucRssiEn = 0;
                        tCheck.Checked = 0;

                        /* 清除信号质量显示内容 */
                        LCD_DispStr(5, 100, "                                                     ", &tFont);
                    }
                    else
                    {
                        g_tRadio.ucRssiEn = 1;
                        tCheck.Checked = 1;
                    }

                    tCheck.Left = CHECK_RSSI_X;
                    tCheck.Top = CHECK_RSSI_Y;
                    tCheck.Height = CHECK_RSSI_H;
                    tCheck.Width = CHECK_RSSI_W;
                    tCheck.pCaption = CHECK_RSSI_TEXT;
                    LCD_DrawCheckBox(&tCheck);
                }
                else if (TOUCH_InRect(tpX, tpY, CHECK_LIST_X, CHECK_LIST_Y, CHECK_LIST_H, CHECK_LIST_W))
                {
                    if (g_tRadio.ucListType)
                    {
                        g_tRadio.ucListType = 0; /* 武汉地区电台列表 */
                        radio_FM_FreqList(0);
                        radio_AM_FreqList(0);

                        tCheck.Checked = 0;
                    }
                    else
                    {
                        g_tRadio.ucListType = 1; /* 全国电台列表 */
                        radio_FM_FreqList(1);
                        radio_AM_FreqList(1);

                        tCheck.Checked = 1;
                    }

                    g_tRadio.ucIndexFM = 0;
                    g_tRadio.ucIndexAM = 0;

                    tCheck.Left = CHECK_LIST_X;
                    tCheck.Top = CHECK_LIST_Y;
                    tCheck.Height = CHECK_LIST_H;
                    tCheck.Width = CHECK_LIST_W;

                    tCheck.pCaption = CHECK_LIST_TEXT;
                    LCD_DrawCheckBox(&tCheck);

                    fRefresh = 1;
                }
                break;

            case TOUCH_RELEASE: /* 触笔释放事件 */
                if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
                {
                    tBtn.Font = &tFontBtn;

                    tBtn.Left = BUTTON_RET_X;
                    tBtn.Top = BUTTON_RET_Y;
                    tBtn.Height = BUTTON_RET_H;
                    tBtn.Width = BUTTON_RET_W;
                    tBtn.Focus = 0; /* 焦点 */
                    tBtn.pCaption = "返回";
                    LCD_DrawButton(&tBtn);

                    fQuit = 1; /* 返回 */
                                         //return;
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_RET2_X, BUTTON_RET2_Y, BUTTON_RET2_H, BUTTON_RET2_W))
                {
                    tBtn.Font = &tFontBtn;

                    tBtn.Left = BUTTON_RET2_X;
                    tBtn.Top = BUTTON_RET2_Y;
                    tBtn.Height = BUTTON_RET2_H;
                    tBtn.Width = BUTTON_RET2_W;
                    tBtn.Focus = 0; /* 焦点 */
                    tBtn.pCaption = BUTTON_RET2_TEXT;
                    LCD_DrawButton(&tBtn);

                    fQuit = 2; /* 保留收音 返回 */
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_FM_X, BUTTON_FM_Y, BUTTON_FM_H, BUTTON_FM_W))
                {
                    if (g_tRadio.ChipType == SI4730)
                    {
                        tBtn.Font = &tFontBtn;

                        tBtn.Left = BUTTON_FM_X;
                        tBtn.Top = BUTTON_FM_Y;
                        tBtn.Height = BUTTON_FM_H;
                        tBtn.Width = BUTTON_FM_W;
                        tBtn.Focus = 0; /* 失去焦点 */
                        tBtn.pCaption = "FM";
                        LCD_DrawButton(&tBtn);

                        /* 切换到 FM 状态 */
                        //SI4730_SetOutVolume(0);        /* 先静音，避免切换噶嗒声 */
                        wm8978_OutMute(1);

                        g_tRadio.ucMode = FM_RX;

                        SI4730_PowerDown();
                        bsp_DelayMS(5);
                        SI4730_PowerUp_FM_Revice();
                        bsp_DelayMS(10);

                        SI4730_SetOutVolume(g_tRadio.ucVolume); /* 缺省音量是最大值63, 修改为32 */

                        g_tRadio.usFreq = g_tRadio.usFMList[g_tRadio.ucIndexFM];
                        SI4730_SetFMFreq(g_tRadio.usFreq);

                        bsp_DelayMS(100); /* 延迟100ms，避免强烈的嘎哒声 */

                        wm8978_OutMute(0);

                        fRefresh = 1;
                    }
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_AM_X, BUTTON_AM_Y, BUTTON_AM_H, BUTTON_AM_W))
                {
                    if (g_tRadio.ChipType == SI4730)
                    {
                        tBtn.Left = BUTTON_AM_X;
                        tBtn.Top = BUTTON_AM_Y;
                        tBtn.Height = BUTTON_AM_H;
                        tBtn.Width = BUTTON_AM_W;
                        tBtn.Focus = 0; /* 失去焦点 */
                        tBtn.pCaption = "AM";
                        LCD_DrawButton(&tBtn);

                        /* 切换到 AM 状态 */
                        //SI4730_SetOutVolume(0);        /* 先静音，避免切换噶嗒声 */
                        wm8978_OutMute(1);

                        g_tRadio.ucMode = AM_RX; /* 缺省是AM接收 */
                        SI4730_PowerDown();
                        bsp_DelayMS(5);
                        SI4730_PowerUp_AM_Revice();
                        bsp_DelayMS(10);

                        SI4730_SetOutVolume(g_tRadio.ucVolume); /* 缺省音量是最大值63, 修改为32 */

                        g_tRadio.usFreq = g_tRadio.usAMList[g_tRadio.ucIndexAM];
                        SI4730_SetAMFreq(g_tRadio.usFreq);

                        bsp_DelayMS(100); /* 延迟100ms，避免强烈的嘎哒声 */

                        wm8978_OutMute(0);

                        fRefresh = 1;
                    }
                }
                else /* 按钮失去焦点 */
                {
                    tBtn.Font = &tFontBtn;

                    tBtn.Focus = 0; /* 未选中 */

                    tBtn.Left = BUTTON_RET_X;
                    tBtn.Top = BUTTON_RET_Y;
                    tBtn.Height = BUTTON_RET_H;
                    tBtn.Width = BUTTON_RET_W;
                    tBtn.pCaption = "返回";
                    LCD_DrawButton(&tBtn);

                    tBtn.Left = BUTTON_RET2_X;
                    tBtn.Top = BUTTON_RET2_Y;
                    tBtn.Height = BUTTON_RET2_H;
                    tBtn.Width = BUTTON_RET2_W;
                    tBtn.pCaption = BUTTON_RET2_TEXT;
                    LCD_DrawButton(&tBtn);

                    if (g_tRadio.ChipType == SI4730)
                    {
                        tBtn.Left = BUTTON_FM_X;
                        tBtn.Top = BUTTON_FM_Y;
                        tBtn.Height = BUTTON_FM_H;
                        tBtn.Width = BUTTON_FM_W;
                        tBtn.pCaption = "FM";
                        LCD_DrawButton(&tBtn);

                        tBtn.Left = BUTTON_AM_X;
                        tBtn.Top = BUTTON_AM_Y;
                        tBtn.Height = BUTTON_AM_H;
                        tBtn.Width = BUTTON_AM_W;
                        tBtn.pCaption = "AM";
                        LCD_DrawButton(&tBtn);
                    }
                }
            }
        }

        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /*
                对于按键弹起事件，缺省的bsp_button.c 仅检测了TAMPER、WAKEUP、USER键、摇杆OK键的弹起事件
                如果您的应用程序需要其它键（比如方向键）的弹起事件，您需要简单修改一下bsp_button.c文件
            */
            switch (ucKeyCode)
            {
            case KEY_DOWN_K1: /* K1键按下 */

                break;

            case KEY_DOWN_K2: /* K2键按下 */

                if (g_tRadio.ucMode == FM_RX)
                {
                    g_tRadio.usFreq -= 10;
                    SI4730_SetFMFreq(g_tRadio.usFreq);
                }
                else
                {
                    g_tRadio.usFreq--;
                    SI4730_SetAMFreq(g_tRadio.usFreq);
                }
                fRefresh = 1;
                break;

            case KEY_DOWN_K3: /* K3键按下 */

                if (g_tRadio.ucMode == FM_RX)
                {
                    g_tRadio.usFreq += 10;
                    SI4730_SetFMFreq(g_tRadio.usFreq);
                }
                else
                {
                    g_tRadio.usFreq++;
                    SI4730_SetAMFreq(g_tRadio.usFreq);
                }
                fRefresh = 1;
                break;

            case JOY_DOWN_U: /* 摇杆UP键按下 */
                g_tRadio.ucVolume += VOLUME_STEP;
                if (g_tRadio.ucVolume > VOLUME_MAX)
                {
                    g_tRadio.ucVolume = VOLUME_MAX;
                }
                radio_AdjustVolume(g_tRadio.ucVolume);
                fRefresh = 1;
                break;

            case JOY_DOWN_D: /* 摇杆DOWN键按下 */
                if (g_tRadio.ucVolume > 0)
                {
                    g_tRadio.ucVolume -= VOLUME_STEP;
                }
                else
                {
                    g_tRadio.ucVolume = 0;
                }
                radio_AdjustVolume(g_tRadio.ucVolume);
                fRefresh = 1;
                break;

            case JOY_DOWN_L: /* 摇杆LEFT键按下 */
                if (g_tRadio.ucMode == FM_RX)
                {
                    if (g_tRadio.ucIndexFM > 0)
                    {
                        g_tRadio.ucIndexFM--;
                        g_tRadio.usFreq = g_tRadio.usFMList[g_tRadio.ucIndexFM];
                        SI4730_SetFMFreq(g_tRadio.usFreq);
                        fRefresh = 1;
                    }
                }
                else
                {
                    if (g_tRadio.ucIndexAM > 0)
                    {
                        g_tRadio.ucIndexAM--;
                        g_tRadio.usFreq = g_tRadio.usAMList[g_tRadio.ucIndexAM];
                        SI4730_SetAMFreq(g_tRadio.usFreq);
                        fRefresh = 1;
                    }
                }
                break;

            case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
                if (g_tRadio.ucMode == FM_RX)
                {
                    if (g_tRadio.ucIndexFM < g_tRadio.ucFMCount - 1)
                    {
                        g_tRadio.ucIndexFM++;
                        g_tRadio.usFreq = g_tRadio.usFMList[g_tRadio.ucIndexFM];
                        SI4730_SetFMFreq(g_tRadio.usFreq);
                        fRefresh = 1;
                    }
                }
                else
                {
                    if (g_tRadio.ucIndexAM < g_tRadio.ucAMCount - 1)
                    {
                        g_tRadio.ucIndexAM++;
                        g_tRadio.usFreq = g_tRadio.usAMList[g_tRadio.ucIndexAM];
                        SI4730_SetAMFreq(g_tRadio.usFreq);
                        fRefresh = 1;
                    }
                }
                break;

            case JOY_DOWN_OK: /* 摇杆OK键按下 */
                break;

            default:
                /* 其他的键值不处理 */
                break;
            }
        }
    } //while (fQuit == 0)

    if (fQuit == 1)
    {
        SI4730_PowerDown();
    }

    bsp_StopTimer(0); /* 停止定时器0 */

    radio_SaveParam(); /* 保存电台参数 */
}

/*
*********************************************************************************************************
*    函 数 名: radio_DispStatus
*    功能说明: 显示当前状态
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void radio_DispStatus(void)
{
    char buf[128];
    FONT_T tFont; /* 定义一个字体结构体变量，用于设置字体参数 */
    uint16_t usLineCap = 18;
    uint16_t x, y;

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE; /* 字体颜色 */
        tFont.BackColor = CL_BLUE;     /* 文字背景颜色 */
        tFont.Space = 0;                         /* 文字间距，单位 = 像素 */
    }

    if (g_tRadio.ucMode == FM_RX)
    {
        sprintf(buf, "FM (%3d/%d) 频率=%5d.%dMHz, Volume = %2d    ", g_tRadio.ucIndexFM + 1,
                        g_tRadio.ucFMCount, g_tRadio.usFreq / 100,
                        (g_tRadio.usFreq % 100) / 10, g_tRadio.ucVolume);
    }
    else
    {
        sprintf(buf, "AM (%3d/%d) 频率=%5dKHz, Volume = %2d    ", g_tRadio.ucIndexAM + 1,
                        g_tRadio.ucAMCount, g_tRadio.usFreq, g_tRadio.ucVolume);
    }

    x = 5;
    y = 80;
    LCD_DispStr(x, y, buf, &tFont);
    y += usLineCap;
}

/*
*********************************************************************************************************
*    函 数 名: radio_SignalQuality
*    功能说明: 显示当前信号质量 RSSI  SNR
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void radio_SignalQuality(void)
{
    char buf[128];
    FONT_T tFont; /* 定义一个字体结构体变量，用于设置字体参数 */
    uint8_t read_buf[7];
    uint16_t x, y;

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE; /* 字体颜色 */
        tFont.BackColor = CL_BLUE;     /* 文字背景颜色 */
        tFont.Space = 0;                         /* 文字间距，单位 = 像素 */
    }
    x = 5;
    y = 100;

    if (g_tRadio.ucMode == FM_RX)
    {
        uint8_t rssi, snr, cap;

        SI4730_GetFMSignalQuality(read_buf);
        /*
            CMD      0x23    FM_RSQ_STATUS
            ARG1     0x01    Clear RSQINT
            STATUS   ?0x80   Reply Status. Clear-to-send high.
            RESP1    ?0x00   No blend, SNR high, low, RSSI high or low interrupts.
            RESP2    ?0x01   Soft mute is not engaged, no AFC rail, valid frequency.
            RESP3    ?0xD9   Pilot presence, 89% blend
            RESP4    ?0x2D   RSSI = 45 dBμV
            RESP5    ?0x33   SNR = 51 dB
            RESP6    ?0x00
            RESP7    ?0x00   Freq offset = 0 kHz
        */
        rssi = read_buf[3];
        snr = read_buf[4];

        SI4730_GetFMTuneStatus(read_buf);
        /*
            CMD      0x22     FM_TUNE_STATUS
            ARG1     0x01     Clear STC interrupt.
            STATUS   ?0x80    Reply Status. Clear-to-send high.

            RESP1    ?0x01    Valid Frequency.
            RESP2    ?0x27    Frequency = 0x27F6 = 102.3 MHz
            RESP3    ?0xF6
            RESP4    ?0x2D    RSSI = 45 dBμV
            RESP5    ?0x33    SNR = 51 dB
            RESP6    ?0x00    MULT[7:0]
            RESP7    ?0x00    Antenna tuning capacitor = 0 (range = 0–191)  READANTCAP[7:0] (Si4704/05/06/2x only)
        */
        cap = read_buf[6];

        sprintf(buf, "RSSI = %ddBuV  SNR = %ddB CAP = %d", rssi, snr, cap);

        LCD_DispStrEx(x, y, buf, &tFont, 300, ALIGN_LEFT);
    }
    else
    {
        uint32_t cap;

        /* 读取AM调谐状态 */
        SI4730_GetAMTuneStatus(read_buf);
        /*
            CMD       0x42           AM_TUNE_STATUS
            ARG1      0x01           Clear STC interrupt.
            STATUS    ?0x80          Reply Status. Clear-to-send high.

            RESP1     ?0x01          Channel is valid
            RESP2     ?0x03
            RESP3     ?0xE8          Frequency = 0x03E8 = 1000 kHz
            RESP4     ?0x2A          RSSI = 0x2A = 42d = 42 dBμV
            RESP5     ?0x1A          SNR = 0x1A = 26d = 26 dB
            RESP6     ?0x0D          Value the antenna tuning capacitor is set to.
            RESP7     ?0x95          0x0D95 = 3477 dec.

            电容计算 The tuning capacitance is 95 fF x READANTCAP + 7 pF            
        */
        cap = (read_buf[5] << 8) | read_buf[6];
        cap = (cap * 95) + 7000; /* 实测 342pF -- 10pF */

        /* 读取AM信号质量 */
        SI4730_GetAMSignalQuality(read_buf);

        sprintf(buf, "RSSI = %ddBuV  SNR = %ddB  Tuning Cap. = %d.%03dpF    ",
                        read_buf[3], read_buf[4], cap / 1000, cap % 1000);
        LCD_DispStr(x, y, buf, &tFont);
    }
}

/*
*********************************************************************************************************
*    函 数 名: radio_FM_FreqList
*    功能说明: 预设电台列表
*    形    参：_ucAll = 0 表示武汉地区FM台， 1 表示所有的台，步长0.1M
*    返 回 值: 无
*********************************************************************************************************
*/
static void radio_FM_FreqList(uint8_t _ucAll)
{
    /* 预填武汉地区的电台列表 */
    uint32_t i;

    if (_ucAll == 0)
    {
        g_tRadio.ucFMCount = sizeof(g_InitListFM) / 2;
        for (i = 0; i < g_tRadio.ucFMCount; i++)
        {
            g_tRadio.usFMList[i] = g_InitListFM[i];
        }
    }
    else
    {
        /*
            中国范围： 88-108兆赫信号调制方式是调频（频率调制）每个频道的频率间隔是0.1兆赫
        */
        g_tRadio.ucFMCount = 0;
        for (i = 8800; i <= 10800; i += 10)
        {
            g_tRadio.usFMList[g_tRadio.ucFMCount++] = i;
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: radio_AM_FreqList
*    功能说明: 预设AM电台列表
*    形    参：_ucAll = 0 表示武汉地区AM台， 1 表示所有的台，步长9KHz
*    返 回 值: 无
*********************************************************************************************************
*/
static void radio_AM_FreqList(uint8_t _ucAll)
{
    /* 预填武汉地区的电台列表 */
    uint8_t i;

    if (_ucAll == 0)
    {
        g_tRadio.ucAMCount = sizeof(g_InitListAM) / 2;
        for (i = 0; i < g_tRadio.ucAMCount; i++)
        {
            g_tRadio.usAMList[i] = g_InitListAM[i];
        }
    }
    else
    {
        /*
            步长 9KHz
        */
        g_tRadio.ucAMCount = 120;
        for (i = 0; i < g_tRadio.ucAMCount; i++)
        {
            g_tRadio.usAMList[i] = 531 + i * 9;
        }
        g_tRadio.ucIndexAM = 0;
    }
}

/*
*********************************************************************************************************
*    函 数 名: radio_LoadParam
*    功能说明: 读取电台参数
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void radio_LoadParam(void)
{
    //LoadPara(); 不需要读取，main() 中已读取

    g_tRadio.ucMode = g_tParam.ucRadioMode;                    /* AM 或 FM */
    g_tRadio.ucListType = g_tParam.ucRadioListType; /* 电台列表类型。武汉地区或全国 */
    g_tRadio.ucIndexFM = g_tParam.ucIndexFM;                /* 当前FM电台索引 */
    g_tRadio.ucIndexAM = g_tParam.ucIndexAM;                /* 当前电台索引 */
    g_tRadio.ucVolume = g_tParam.ucRadioVolume;            /* 音量 */
    g_tRadio.ucSpkOutEn = g_tParam.ucSpkOutEn;            /* 扬声器输出使能 */
}

/*
*********************************************************************************************************
*    函 数 名: radio_LoadParam
*    功能说明: 保存当前的电台和音量
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void radio_SaveParam(void)
{
    g_tParam.ucRadioMode = g_tRadio.ucMode;                    /* AM 或 FM */
    g_tParam.ucRadioListType = g_tRadio.ucListType; /* 电台列表类型。武汉地区或全国 */
    g_tParam.ucIndexFM = g_tRadio.ucIndexFM;                /* 当前FM电台索引 */
    g_tParam.ucIndexAM = g_tRadio.ucIndexAM;                /* 当前电台索引 */
    g_tParam.ucRadioVolume = g_tRadio.ucVolume;            /* 音量 */
    g_tParam.ucSpkOutEn = g_tRadio.ucSpkOutEn;            /* 扬声器输出使能 */

    SaveParam();
}

/*
*********************************************************************************************************
*    函 数 名: radio_AdjustVolume
*    功能说明: 调节音量
*    形    参: _ucVolume : 0-63
*    返 回 值: 无
*********************************************************************************************************
*/
static void radio_AdjustVolume(uint8_t _ucVolume)
{
    uint8_t volume;

    wm8978_SetEarVolume(g_tRadio.ucVolume);
    wm8978_SetSpkVolume(g_tRadio.ucVolume);

    if (g_tRadio.ucVolume == 0)
    {
        SI4730_SetOutVolume(0);
    }
    else
    {
        volume = 21 + (g_tRadio.ucVolume * 2) / 3;
        SI4730_SetOutVolume(volume);
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
