/*
*********************************************************************************************************
*
*    模块名称 : 测试WIFI模块
*    文件名称 : wifi_test.c
*    版    本 : V1.0
*    说    明 : 测试串口WiFi模块. 使用串口超级终端工具可以操作本例子。为了测试AT指令
*                SecureCRT ，需要配置为: 菜单选项 -> 会话选项 -> 左侧栏终端 -> 仿真 -> 模式
*                    右侧窗口，当前模式中勾选"新行模式"
*    修改记录 :
*        版本号  日期       作者    说明
*        v1.0    2015-07-16 armfly  首发
*
*    Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_wifi.h"
#include "num_pad.h"

#define AP_MAX_NUM 30

/* 定义界面结构 */
typedef struct
{
    FONT_T FontBlack; /* 静态的文字 */
    FONT_T FontBlue;    /* 变化的文字字体 */
    FONT_T FontRed;
    FONT_T FontBtn; /* 按钮的字体 */
    FONT_T FontBox; /* 分组框标题字体 */

    GROUP_T Box1;

    LABEL_T Label1;
    LABEL_T Label2;
    LABEL_T Label3;
    LABEL_T Label4;
    LABEL_T Label5;
    LABEL_T Label6;
    LABEL_T Label7;
    LABEL_T Label8;

    LABEL_T Label9;

    EDIT_T Edit1; /* WIFI密码 */

    BUTTON_T Btn1; /* 列举AP */
    BUTTON_T Btn2; /* 加入AP */
    BUTTON_T Btn3; /*   */
    BUTTON_T Btn4; /*   */

    BUTTON_T BtnRet; /* 返回 */

    WIFI_AP_T APList[AP_MAX_NUM]; /* AP 列表 */

} FormWIFI_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H (g_LcdHeight - BOX1_Y - 10)
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_TEXT "ESP8266 WiFi模块测试程序."

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 60
#define BTN_RET_X ((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define BTN_RET_Y ((BOX1_Y + BOX1_H) - BTN_RET_H - 4)
#define BTN_RET_TEXT "返回"

#define LABEL1_X (BOX1_X + 6)
#define LABEL1_Y (g_LcdHeight - 28)
#define LABEL1_TEXT "--- "

/* 按钮 */
#define BTN1_H 32
#define BTN1_W 120
#define BTN1_X (g_LcdWidth - BTN1_W - 10)
#define BTN1_Y 20
#define BTN1_TEXT "列举AP"

/* Edit */
#define EDIT1_X BTN1_X
#define EDIT1_Y (BTN1_Y + (BTN1_H + 2))
#define EDIT1_H BTN1_H
#define EDIT1_W BTN1_W

#define LABEL2_X EDIT1_X - 45
#define LABEL2_Y EDIT1_Y + 4
#define LABEL2_TEXT "密码:"

#define BTN2_H BTN1_H
#define BTN2_W BTN1_W
#define BTN2_X BTN1_X
#define BTN2_Y (BTN1_Y + (BTN1_H + 2) * 2)
#define BTN2_TEXT "加入AP"

#define BTN3_H BTN1_H
#define BTN3_W BTN1_W
#define BTN3_X BTN1_X
#define BTN3_Y (BTN1_Y + (BTN1_H + 2) * 3)
#define BTN3_TEXT "查看本机IP"

#define BTN4_H BTN1_H
#define BTN4_W BTN1_W
#define BTN4_X BTN1_X
#define BTN4_Y (BTN1_Y + (BTN1_H + 2) * 4)
#define BTN4_TEXT "创建TCP服务"

static void InitFormWIFI(void);
static void DispFormWIFI(void);
static void DispInfoWiFi(char *_str);

FormWIFI_T *FormWIFI;

int16_t g_APCount = 0;
uint8_t g_TCPServerOk = 0;

static void ScanAP(void);
static void DispAP(void);
void AnlyzeHostCmd(void);

/*
*********************************************************************************************************
*    函 数 名: TestWIFI
*    功能说明: 测试串口WiFi模块
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
void TestWIFI(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t ucTouch;     /* 触摸事件 */
    uint8_t fQuit = 0;
    int16_t tpX, tpY;
    uint8_t ucValue;
    uint8_t fRefresh = 0;
    FormWIFI_T form;

    FormWIFI = &form;

    LCD_ClrScr(CL_BTN_FACE);

    InitFormWIFI();
    DispFormWIFI();

    bsp_InitESP8266();

    //WiFiDispHelp();

    DispInfoWiFi("【1】正在给ESP8266模块上电...(波特率: 74880bsp)");
    printf("\r\n【1】正在给ESP8266模块上电...(波特率: 74880bsp)\r\n");

    ESP8266_PowerOn();

    DispInfoWiFi("【2】上电完成。波特率: 115200bsp");
    printf("\r\n【2】上电完成。波特率: 115200bsp\r\n");

    //
    DispInfoWiFi("【3】测试AT指令");
    ESP8266_SendAT("AT");
    if (ESP8266_WaitResponse("OK", 50) == 1)
    {
        DispInfoWiFi("【3】模块应答AT成功");
        bsp_DelayMS(1000);
    }
    else
    {
        DispInfoWiFi("【3】模块无应答, 请按K3键修改模块的波特率为115200");
        bsp_DelayMS(1000);
    }

    g_TCPServerOk = 0;

    /* 进入主程序循环体 */
    while (fQuit == 0)
    {
        bsp_Idle();

        if (g_TCPServerOk == 1)
        {
            AnlyzeHostCmd();
        }
        else
        {
            /* 从WIFI收到的数据发送到串口1 */
            if (comGetChar(COM_ESP8266, &ucValue))
            {
                comSendChar(COM1, ucValue);
                continue;
            }
            /* 将串口1的数据发送到MG323模块 */
            if (comGetChar(COM1, &ucValue))
            {
                comSendChar(COM_ESP8266, ucValue);
                continue;
            }
        }

        if (fRefresh)
        {
            fRefresh = 0;

            LCD_ClrScr(CL_BTN_FACE);
            DispFormWIFI(); /* 刷新所有控件 */
            DispAP();
        }

        ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
        if (ucTouch != TOUCH_NONE)
        {
            switch (ucTouch)
            {
            case TOUCH_DOWN: /* 触笔按下事件 */
                LCD_ButtonTouchDown(&FormWIFI->BtnRet, tpX, tpY);
                LCD_ButtonTouchDown(&FormWIFI->Btn1, tpX, tpY);
                LCD_ButtonTouchDown(&FormWIFI->Btn2, tpX, tpY);
                LCD_ButtonTouchDown(&FormWIFI->Btn3, tpX, tpY);
                LCD_ButtonTouchDown(&FormWIFI->Btn4, tpX, tpY);

                /* 编辑框 */
                if (TOUCH_InRect(tpX, tpY, EDIT1_X, EDIT1_Y, EDIT1_H, EDIT1_W))
                {
                    {
                        uint8_t len = 30;

                        if (InputNumber(NUMPAD_STR, "输入WiFi密码", &len, (void *)FormWIFI->Edit1.Text))
                        {
                            ;
                        }
                        fRefresh = 1;
                    }
                }
                break;

            case TOUCH_RELEASE: /* 触笔释放事件 */
                if (LCD_ButtonTouchRelease(&FormWIFI->BtnRet, tpX, tpY))
                {
                    fQuit = 1; /* 返回 */
                }
                else if (LCD_ButtonTouchRelease(&FormWIFI->Btn1, tpX, tpY))
                {
                    ScanAP(); /* 扫描AP */
                    DispAP();
                }
                else if (LCD_ButtonTouchRelease(&FormWIFI->Btn2, tpX, tpY))
                {
                    int32_t sn = 0;
                    char buf[64];

                    if (InputInt("选择AP序号", 0, 20, &sn))
                    {
                        LCD_ClrScr(CL_BTN_FACE);
                        DispFormWIFI(); /* 刷新所有控件 */
                        DispAP();

                        if (sn > 1)
                        {
                            sn--;
                        }

                        sprintf(buf, "正在加入AP... 请等待 %s", FormWIFI->APList[sn].ssid);
                        DispInfoWiFi(buf);
                        /* 加入AP 超时时间 10000ms，10秒 */
                        if (ESP8266_JoinAP(FormWIFI->APList[sn].ssid, FormWIFI->Edit1.Text, 15000))
                        {
                            DispInfoWiFi("接入AP成功");
                        }
                        else
                        {
                            DispInfoWiFi("接入AP失败");
                        }
                    }
                    else
                    {
                        LCD_ClrScr(CL_BTN_FACE);
                        DispFormWIFI(); /* 刷新所有控件 */
                        DispAP();
                    }
                }
                else if (LCD_ButtonTouchRelease(&FormWIFI->Btn3, tpX, tpY)) /* 查看本机IP */
                {
                    char ip[20];
                    char mac[32];
                    char buf[128];

                    if (ESP8266_GetLocalIP(ip, mac) == 1)
                    {
                        sprintf(buf, "%s, %s", ip, mac);
                        DispInfoWiFi(buf);
                    }
                    else
                    {
                        DispInfoWiFi("查询IP失败");
                    }
                }
                else if (LCD_ButtonTouchRelease(&FormWIFI->Btn4, tpX, tpY)) /* 拨打10086 */
                {
                    if (g_TCPServerOk == 0)
                    {
                        if (ESP8266_CreateTCPServer(1000) == 1)
                        {
                            DispInfoWiFi("正在监听1000端口...");
                            g_TCPServerOk = 1;
                        }
                        else
                        {
                            DispInfoWiFi("创建TCP服务失败!");
                        }
                    }
                    else
                    {
                        ESP8266_CloseTcpUdp(0);
                        g_TCPServerOk = 0;
                        DispInfoWiFi("关闭当前TCP连接!");
                    }
                }
                break;
            }
        }

        /* 处理按键事件 */
        ucKeyCode = bsp_GetKey();
        if (ucKeyCode > 0)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_DOWN_K1: /* K1键按下 */
                //ESP8266_SendAT("AT+CWLAP");    /* 列举AP */
                break;

            case KEY_DOWN_K2: /* K2键按下 */
                //ESP8266_SendAT("AT+CWJAP=\"Tenda_5BD8A8\",\"123456887af\"");    /* 加入某个WIFI 网络 */
                //ESP8266_JoinAP("Tenda_5BD8A8", "123456887af");
                break;

            case KEY_DOWN_K3:                                             /* K3键-9600波特率切换到115200 */
                comSetBaud(COM_ESP8266, 9600);             /* 模块缺省是9600bps */
                ESP8266_SendAT("AT+CIOBAUD=115200"); /* 按 9600bps 发送指令切换为 115200 */
                ESP8266_WaitResponse("OK", 2000);         /* 这个 OK 是模块按 9600 应答的 */
                comSetBaud(COM_ESP8266, 115200);         /* 切换STM32的波特率为 115200 */

                /* 切换为 Station模式 */
                bsp_DelayMS(100);
                ESP8266_SendAT("AT+CWMODE=1");
                ESP8266_WaitResponse("OK", 2000);
                bsp_DelayMS(1500);
                ESP8266_SendAT("AT+RST");
                break;

            case JOY_DOWN_U: /* 摇杆上键， AT+CIFSR获取本地IP地址 */
                ESP8266_SendAT("AT+CIFSR");
                break;

            case JOY_DOWN_D: /* 摇杆DOWN键 AT+CIPSTATUS获得IP连接状态 */
                ESP8266_SendAT("AT+CIPSTATUS");
                break;

            case JOY_DOWN_L: /* 摇杆LEFT键按下   AT+CIPSTART 建立TCP连接. 访问www,armfly.com http服务端口 */
                ESP8266_SendAT("AT+CIPSTART=\"TCP\",\"WWW.ARMFLY.COM\",80");
                break;

            case JOY_DOWN_R: /* 摇杆RIGHT键按下  AT+CIPCLOSE关闭当前的TCP或UDP连接  */
                ESP8266_SendAT("AT+CIPCLOSE");
                break;

            case JOY_DOWN_OK: /* 摇杆OK键按下  */
                //printf("\r\n进入固件升级模式\r\n");
                break;

            default:
                break;
            }
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: AnlyzeHostCmd
*    功能说明: 分析TCP客户端发来的数据
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/

void AnlyzeHostCmd(void)
{
    uint8_t cmd_buf[2048];
    //uint8_t *cmd_buf;
    uint8_t cmd_len;
    static uint8_t s_test = 0;

    //cmd_buf = (uint8_t *)EXT_SRAM_ADDR;    /* 指向外部SRAM */
    cmd_len = ESP8266_RxNew(cmd_buf);
    if (cmd_len == 0)
    {
        return;
    }

    if (s_test == 1)
    {
        if ((cmd_len == 1) && (memcmp(cmd_buf, "A", 1)) == 0)
        {
            ESP8266_SendTcpUdp(cmd_buf, 1300);
        }
    }
    else if (s_test == 2)
    {
        ESP8266_SendTcpUdp("OK", 2);
    }

    if ((cmd_len == 6) && (memcmp(cmd_buf, "txtest", 6)) == 0)
    {
        s_test = 1;
        ESP8266_SendTcpUdp(cmd_buf, 1300);
    }
    else if ((cmd_len == 6) && (memcmp(cmd_buf, "rxtest", 6)) == 0)
    {
        ESP8266_SendTcpUdp("OK", 2);
        s_test = 2;
    }
    else if ((cmd_len == 4) && (memcmp(cmd_buf, "stop", 4)) == 0)
    {
        s_test = 0;
    }
    else
    {
        if (cmd_len == 7)
        {
            if (memcmp(cmd_buf, "ledon 1 ", 7) == 0)
            {
                bsp_LedOn(1);
                ESP8266_SendTcpUdp("led1 on", 7);
            }
            else if (memcmp(cmd_buf, "ledon 2 ", 7) == 0)
            {
                bsp_LedOn(2);
                ESP8266_SendTcpUdp("led2 on", 7);
            }
            else if (memcmp(cmd_buf, "ledon 3 ", 7) == 0)
            {
                bsp_LedOn(3);
                ESP8266_SendTcpUdp("led3 on", 7);
            }
            else if (memcmp(cmd_buf, "ledon 4 ", 7) == 0)
            {
                bsp_LedOn(4);
                ESP8266_SendTcpUdp("led4 on", 7);
            }
        }
        else if (cmd_len == 8)
        {
            if (memcmp(cmd_buf, "ledoff 1", 8) == 0)
            {
                bsp_LedOff(1);
                ESP8266_SendTcpUdp("led1 off", 8);
            }
            else if (memcmp(cmd_buf, "ledoff 2", 8) == 0)
            {
                bsp_LedOff(2);
                ESP8266_SendTcpUdp("led2 off", 8);
            }
            else if (memcmp(cmd_buf, "ledoff 3", 8) == 0)
            {
                bsp_LedOff(3);
                ESP8266_SendTcpUdp("led3 off", 8);
            }
            else if (memcmp(cmd_buf, "ledoff 4", 8) == 0)
            {
                bsp_LedOff(4);
                ESP8266_SendTcpUdp("led4 off", 8);
            }
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: ScanAP
*    功能说明: 扫描AP，并显示出来.
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void ScanAP(void)
{
    /* 扫描 AP列表, 返回AP个数 */
    g_APCount = ESP8266_ScanAP(FormWIFI->APList, AP_MAX_NUM);
}

/*
*********************************************************************************************************
*    函 数 名: DispAP
*    功能说明: 显示扫描到的AP
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispAP(void)
{
    char buf[48];
    FONT_T tFont;
    uint16_t x, y;

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;         /* 字体代码 16点阵 */
        tFont.FrontColor = CL_GREY;         /* 字体颜色 */
        tFont.BackColor = CL_BTN_FACE; /* 文字背景颜色 */
        tFont.Space = 0;                             /* 文字间距，单位 = 像素 */
    }

    x = 10;
    y = 25;

    LCD_Fill_Rect(x, y, 234, 280, CL_BTN_FACE);

    sprintf(buf, "共扫描到%d个WiFi AP", g_APCount);
    DispInfoWiFi(buf);

    {
        uint8_t i;
        uint8_t m;
        //const char *ecn_name[5] =
        //{
        //    "OPEN", "WEP", "WPA_PSK", "WPA2_PSK", "WPA_WPA2_PSK"
        //};
        m = g_APCount;
        if (m > 13)
        {
            m = 13;
        }
        for (i = 0; i < m; i++)
        {
            //sprintf(buf, "  %02d=%s, rssi=%d, [%s]\r\n", i+1, g_APList[i].ssid, g_APList[i].rssi,
            //    ecn_name[g_APList[i].ecn]);

            sprintf(buf, "%02d=%s, %ddBm", i + 1, FormWIFI->APList[i].ssid, FormWIFI->APList[i].rssi);

            LCD_DispStr(x, y, buf, &tFont);
            y += 17;
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: InitFormWIFI
*    功能说明: 初始化控件属性
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void InitFormWIFI(void)
{
    /* 分组框标题字体 */
    FormWIFI->FontBox.FontCode = FC_ST_16;
    FormWIFI->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
    FormWIFI->FontBox.FrontColor = CL_BLACK;
    FormWIFI->FontBox.Space = 0;

    /* 字体1 用于静止标签 */
    FormWIFI->FontBlack.FontCode = FC_ST_16;
    FormWIFI->FontBlack.BackColor = CL_BTN_FACE; /* 透明色 */
    FormWIFI->FontBlack.FrontColor = CL_BLACK;
    FormWIFI->FontBlack.Space = 0;

    /* 字体2 用于变化的文字 */
    FormWIFI->FontBlue.FontCode = FC_ST_16;
    FormWIFI->FontBlue.BackColor = CL_BTN_FACE;
    FormWIFI->FontBlue.FrontColor = CL_BLUE;
    FormWIFI->FontBlue.Space = 0;

    FormWIFI->FontRed.FontCode = FC_ST_16;
    FormWIFI->FontRed.BackColor = CL_BTN_FACE;
    FormWIFI->FontRed.FrontColor = CL_RED;
    FormWIFI->FontRed.Space = 0;

    /* 按钮字体 */
    FormWIFI->FontBtn.FontCode = FC_ST_16;
    FormWIFI->FontBtn.BackColor = CL_MASK; /* 透明背景 */
    FormWIFI->FontBtn.FrontColor = CL_BLACK;
    FormWIFI->FontBtn.Space = 0;

    /* 分组框 */
    FormWIFI->Box1.Left = BOX1_X;
    FormWIFI->Box1.Top = BOX1_Y;
    FormWIFI->Box1.Height = BOX1_H;
    FormWIFI->Box1.Width = BOX1_W;
    FormWIFI->Box1.pCaption = BOX1_TEXT;
    FormWIFI->Box1.Font = &FormWIFI->FontBox;

    /* 静态标签 */
    FormWIFI->Label1.Left = LABEL1_X;
    FormWIFI->Label1.Top = LABEL1_Y;
    FormWIFI->Label1.MaxLen = 0;
    FormWIFI->Label1.pCaption = LABEL1_TEXT;
    FormWIFI->Label1.Font = &FormWIFI->FontBlack;

    FormWIFI->Label2.Left = LABEL2_X;
    FormWIFI->Label2.Top = LABEL2_Y;
    FormWIFI->Label2.MaxLen = 0;
    FormWIFI->Label2.pCaption = LABEL2_TEXT;
    FormWIFI->Label2.Font = &FormWIFI->FontBlack;

    /* 按钮 */
    FormWIFI->BtnRet.Left = BTN_RET_X;
    FormWIFI->BtnRet.Top = BTN_RET_Y;
    FormWIFI->BtnRet.Height = BTN_RET_H;
    FormWIFI->BtnRet.Width = BTN_RET_W;
    FormWIFI->BtnRet.pCaption = BTN_RET_TEXT;
    FormWIFI->BtnRet.Font = &FormWIFI->FontBtn;
    FormWIFI->BtnRet.Focus = 0;

    FormWIFI->Btn1.Left = BTN1_X;
    FormWIFI->Btn1.Top = BTN1_Y;
    FormWIFI->Btn1.Height = BTN1_H;
    FormWIFI->Btn1.Width = BTN1_W;
    FormWIFI->Btn1.pCaption = BTN1_TEXT;
    FormWIFI->Btn1.Font = &FormWIFI->FontBtn;
    FormWIFI->Btn1.Focus = 0;

    FormWIFI->Btn2.Left = BTN2_X;
    FormWIFI->Btn2.Top = BTN2_Y;
    FormWIFI->Btn2.Height = BTN2_H;
    FormWIFI->Btn2.Width = BTN2_W;
    FormWIFI->Btn2.pCaption = BTN2_TEXT;
    FormWIFI->Btn2.Font = &FormWIFI->FontBtn;
    FormWIFI->Btn2.Focus = 0;

    FormWIFI->Btn3.Left = BTN3_X;
    FormWIFI->Btn3.Top = BTN3_Y;
    FormWIFI->Btn3.Height = BTN3_H;
    FormWIFI->Btn3.Width = BTN3_W;
    FormWIFI->Btn3.pCaption = BTN3_TEXT;
    FormWIFI->Btn3.Font = &FormWIFI->FontBtn;
    FormWIFI->Btn3.Focus = 0;

    FormWIFI->Btn4.Left = BTN4_X;
    FormWIFI->Btn4.Top = BTN4_Y;
    FormWIFI->Btn4.Height = BTN4_H;
    FormWIFI->Btn4.Width = BTN4_W;
    FormWIFI->Btn4.pCaption = BTN4_TEXT;
    FormWIFI->Btn4.Font = &FormWIFI->FontBtn;
    FormWIFI->Btn4.Focus = 0;

    /* 编辑框 */
    FormWIFI->Edit1.Left = EDIT1_X;
    FormWIFI->Edit1.Top = EDIT1_Y;
    FormWIFI->Edit1.Height = EDIT1_H;
    FormWIFI->Edit1.Width = EDIT1_W;
    sprintf(FormWIFI->Edit1.Text, "123456887af");
    FormWIFI->Edit1.pCaption = FormWIFI->Edit1.Text;
    FormWIFI->Edit1.Font = &FormWIFI->FontBtn;
}

/*
*********************************************************************************************************
*    函 数 名: DispInfoWiFi
*    功能说明: 显示信息
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispInfoWiFi(char *_str)
{
    FormWIFI->Label1.pCaption = _str;
    LCD_DrawLabel(&FormWIFI->Label1);
}

/*
*********************************************************************************************************
*    函 数 名: DispFormWIFI
*    功能说明: 显示所有的控件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispFormWIFI(void)
{
    //LCD_ClrScr(CL_BTN_FACE);

    /* 分组框 */
    LCD_DrawGroupBox(&FormWIFI->Box1);

    /* 标签 */
    LCD_DrawLabel(&FormWIFI->Label1);
    LCD_DrawLabel(&FormWIFI->Label2);

    /* 按钮 */
    LCD_DrawButton(&FormWIFI->BtnRet);
    LCD_DrawButton(&FormWIFI->Btn1);
    LCD_DrawButton(&FormWIFI->Btn2);
    LCD_DrawButton(&FormWIFI->Btn3);
    LCD_DrawButton(&FormWIFI->Btn4);

    /* 编辑框 */
    LCD_DrawEdit(&FormWIFI->Edit1);

    /* 动态标签 */
    LCD_DrawLabel(&FormWIFI->Label2);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
