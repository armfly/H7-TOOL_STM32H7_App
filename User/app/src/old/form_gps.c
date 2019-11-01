/*
*********************************************************************************************************
*
*    模块名称 : GPS定位模块测试程序
*    文件名称 : gps_test.c
*    版    本 : V1.0
*    说    明 : 测试MPU-6050, HCM5833L, BMP085, BH1750
*    修改记录 :
*        版本号  日期       作者    说明
*        v1.0    2013-02-01 armfly  首发
*
*    Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_gps.h"

/* 定义界面结构 */
typedef struct
{
    FONT_T FontBlack; /* 静态的文字 */
    FONT_T FontBlue;    /* 变化的文字字体 */
    FONT_T FontRed;        /* 红色字体 */
    FONT_T FontBtn;        /* 按钮的字体 */
    FONT_T FontBox;        /* 分组框标题字体 */

    GROUP_T Box1;

    LABEL_T Label1;
    LABEL_T Label2; /* 纬度 */
    LABEL_T Label3;
    LABEL_T Label4; /* 经度 */
    LABEL_T Label5;
    LABEL_T Label6; /* 速度 */
    LABEL_T Label7;
    LABEL_T Label8; /* 海拔 */

    LABEL_T Label9;
    LABEL_T Label10; /* 状态 */

    BUTTON_T BtnRet;
} FormGPS_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H (g_LcdHeight - BOX1_Y - 10)
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_TEXT "GPS定位模块测试程序"

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 60
#define BTN_RET_X ((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define BTN_RET_Y ((BOX1_Y + BOX1_H) - BTN_RET_H - 4)
#define BTN_RET_TEXT "返回"

#define LABEL1_X (BOX1_X + 6)
#define LABEL1_Y (BOX1_Y + 20)
#define LABEL1_TEXT "纬度 : "

#define LABEL2_X (LABEL1_X + 64)
#define LABEL2_Y LABEL1_Y
#define LABEL2_TEXT "0000.0000"

#define LABEL3_X (LABEL1_X)
#define LABEL3_Y (LABEL1_Y + 20)
#define LABEL3_TEXT "经度 : "

#define LABEL4_X (LABEL3_X + 64)
#define LABEL4_Y (LABEL3_Y)
#define LABEL4_TEXT "00000.0000"

#define LABEL5_X (LABEL1_X)
#define LABEL5_Y (LABEL1_Y + 20 * 2)
#define LABEL5_TEXT "速度 : "

#define LABEL6_X (LABEL5_X + 64)
#define LABEL6_Y LABEL5_Y
#define LABEL6_TEXT "0.0KM"

#define LABEL7_X (LABEL1_X)
#define LABEL7_Y (LABEL1_Y + 20 * 3)
#define LABEL7_TEXT "海拔 : "

#define LABEL8_X (LABEL7_X + 64)
#define LABEL8_Y LABEL7_Y
#define LABEL8_TEXT "0.0M"

#define LABEL9_X (LABEL1_X)
#define LABEL9_Y (LABEL1_Y + 20 * 5)
#define LABEL9_TEXT "硬件 : "

#define LABEL10_X (LABEL9_X + 64)
#define LABEL10_Y LABEL9_Y
#define LABEL10_TEXT ""

static void InitFormGPS(void);
static void DispGPSInitFace(void);
static void DispGPSStatus(void);

FormGPS_T *FormGPS;

/*
*********************************************************************************************************
*    函 数 名: TestGPS
*    功能说明: 测试GPS模块。
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
void TestGPS(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t ucTouch;     /* 触摸事件 */
    uint8_t fQuit = 0;
    int16_t tpX, tpY;
    FormGPS_T form;

    FormGPS = &form;

    bsp_InitGPS();

    InitFormGPS();

    DispGPSInitFace();

    bsp_StartAutoTimer(0, 1000); /* 每秒定时显示GPS状态 */

    bsp_StartTimer(1, 3000); /* 3秒超时没有收到GPS数据，则认为串口连接失败 */

    /* 进入主程序循环体 */
    while (fQuit == 0)
    {
        bsp_Idle();

        gps_pro();

        if (bsp_CheckTimer(0))
        {
            DispGPSStatus();
        }

        /* 这段代码用于判断CPU是否能够收到GPS模块返回的数据 */
        {
            if (bsp_CheckTimer(1))
            {
                FormGPS->Label10.Font = &FormGPS->FontRed;
                FormGPS->Label10.pCaption = "未检测到GPS模块";
                LCD_DrawLabel(&FormGPS->Label10);
            }
            if (g_tGPS.UartOk == 1) /* 串口通信正常的标志, 如果以后收到了校验合格的命令串则设置为1 */
            {
                bsp_StartTimer(1, 3000); /* 3秒超时没有收到GPS数据，则认为串口连接失败 */

                FormGPS->Label10.Font = &FormGPS->FontBlue;
                FormGPS->Label10.pCaption = "检测到GPS模块. 串口数据收发OK";
                LCD_DrawLabel(&FormGPS->Label10);

                g_tGPS.UartOk = 0;
            }
        }

        ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
        if (ucTouch != TOUCH_NONE)
        {
            switch (ucTouch)
            {
            case TOUCH_DOWN: /* 触笔按下事件 */
                if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
                {
                    FormGPS->BtnRet.Focus = 1;
                    LCD_DrawButton(&FormGPS->BtnRet);
                }
                break;

            case TOUCH_RELEASE: /* 触笔释放事件 */
                if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
                {
                    FormGPS->BtnRet.Focus = 0;
                    LCD_DrawButton(&FormGPS->BtnRet);
                    fQuit = 1; /* 返回 */
                }
                else /* 按钮失去焦点 */
                {
                    FormGPS->BtnRet.Focus = 0;
                    LCD_DrawButton(&FormGPS->BtnRet);
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
            case KEY_DOWN_K1: /* K1键 */
                break;

            case KEY_DOWN_K2: /* K2键按下 */
                break;

            case KEY_DOWN_K3: /* K3键按下 */
                break;

            case JOY_DOWN_U: /* 摇杆UP键按下 */
                break;

            case JOY_DOWN_D: /* 摇杆DOWN键按下 */
                break;

            case JOY_DOWN_L: /* 摇杆LEFT键按下 */
                break;

            case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
                break;

            case JOY_DOWN_OK: /* 摇杆OK键按下 */
                break;

            default:
                break;
            }
        }
    }

    bsp_StopTimer(0);
}

/*
*********************************************************************************************************
*    函 数 名: DispGPSStatus
*    功能说明: 显示GPS状态
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispGPSStatus(void)
{
    char buf[128];

    /* 纬度 */
    if (g_tGPS.NS == 'S')
    {
        sprintf(buf, "南纬 %02d°%02d.%04d'=%02d.%06d°", g_tGPS.WeiDu_Du,
                        g_tGPS.WeiDu_Fen / 10000, g_tGPS.WeiDu_Fen % 10000,
                        g_tGPS.WeiDu_Du, gps_FenToDu(g_tGPS.WeiDu_Fen));

        sprintf(&buf[strlen(buf)], "=%02d°%02d'%02d\"", g_tGPS.WeiDu_Du,
                        g_tGPS.WeiDu_Fen / 10000, gps_FenToMiao(g_tGPS.WeiDu_Fen));
    }
    else
    {
        sprintf(buf, "北纬 %02d°%02d.%04d'=%02d.%06d°", g_tGPS.WeiDu_Du,
                        g_tGPS.WeiDu_Fen / 10000, g_tGPS.WeiDu_Fen % 10000,
                        g_tGPS.WeiDu_Du, gps_FenToDu(g_tGPS.WeiDu_Fen));

        sprintf(&buf[strlen(buf)], "=%02d°%02d'%02d\"", g_tGPS.WeiDu_Du,
                        g_tGPS.WeiDu_Fen / 10000, gps_FenToMiao(g_tGPS.WeiDu_Fen));
    }
    FormGPS->Label2.pCaption = buf;
    LCD_DrawLabel(&FormGPS->Label2);

    /* 经度 */
    if (g_tGPS.EW == 'E')
    {
        sprintf(buf, "东经 %03d°%02d.%04d'=%03d.%06d°", g_tGPS.JingDu_Du,
                        g_tGPS.JingDu_Fen / 10000, g_tGPS.JingDu_Fen % 10000,
                        g_tGPS.JingDu_Du, gps_FenToDu(g_tGPS.JingDu_Fen));

        sprintf(&buf[strlen(buf)], "=%03d°%02d'%02d\"", g_tGPS.JingDu_Du,
                        g_tGPS.WeiDu_Fen / 10000, gps_FenToMiao(g_tGPS.JingDu_Fen));
    }
    else
    {
        sprintf(buf, "西经 %03d°%02d.%04d'=%03d.%06d°", g_tGPS.JingDu_Du,
                        g_tGPS.JingDu_Fen / 10000, g_tGPS.JingDu_Fen % 10000,
                        g_tGPS.JingDu_Du, gps_FenToDu(g_tGPS.JingDu_Fen));

        sprintf(&buf[strlen(buf)], "=%03d°%02d'%02d\"", g_tGPS.JingDu_Du,
                        g_tGPS.JingDu_Fen / 10000, gps_FenToMiao(g_tGPS.JingDu_Fen));
    }
    FormGPS->Label4.pCaption = buf;
    LCD_DrawLabel(&FormGPS->Label4);

    /* 速度 */
    sprintf(buf, "%5d.%d KM/h", g_tGPS.SpeedKM / 10, g_tGPS.SpeedKM % 10);
    FormGPS->Label6.pCaption = buf;
    LCD_DrawLabel(&FormGPS->Label6);

    /* 海拔 */
    sprintf(buf, "%5d.%d M", g_tGPS.Altitude / 10, g_tGPS.Altitude % 10);
    FormGPS->Label8.pCaption = buf;
    LCD_DrawLabel(&FormGPS->Label8);
}

/*
*********************************************************************************************************
*    函 数 名: InitFormGPS
*    功能说明: 初始化GPS初始界面控件
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void InitFormGPS(void)
{
    /* 分组框标题字体 */
    FormGPS->FontBox.FontCode = FC_ST_16;
    FormGPS->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
    FormGPS->FontBox.FrontColor = CL_BLACK;
    FormGPS->FontBox.Space = 0;

    /* 字体1 用于静止标签 */
    FormGPS->FontBlack.FontCode = FC_ST_16;
    FormGPS->FontBlack.BackColor = CL_MASK; /* 透明色 */
    FormGPS->FontBlack.FrontColor = CL_BLACK;
    FormGPS->FontBlack.Space = 0;

    /* 字体2 用于变化的文字 */
    FormGPS->FontBlue.FontCode = FC_ST_16;
    FormGPS->FontBlue.BackColor = CL_BTN_FACE;
    FormGPS->FontBlue.FrontColor = CL_BLUE;
    FormGPS->FontBlue.Space = 0;

    FormGPS->FontRed.FontCode = FC_ST_16;
    FormGPS->FontRed.BackColor = CL_BTN_FACE;
    FormGPS->FontRed.FrontColor = CL_RED;
    FormGPS->FontRed.Space = 0;

    /* 按钮字体 */
    FormGPS->FontBtn.FontCode = FC_ST_16;
    FormGPS->FontBtn.BackColor = CL_MASK; /* 透明背景 */
    FormGPS->FontBtn.FrontColor = CL_BLACK;
    FormGPS->FontBtn.Space = 0;

    /* 分组框 */
    FormGPS->Box1.Left = BOX1_X;
    FormGPS->Box1.Top = BOX1_Y;
    FormGPS->Box1.Height = BOX1_H;
    FormGPS->Box1.Width = BOX1_W;
    FormGPS->Box1.pCaption = BOX1_TEXT;
    FormGPS->Box1.Font = &FormGPS->FontBox;

    /* 静态标签 */
    FormGPS->Label1.Left = LABEL1_X;
    FormGPS->Label1.Top = LABEL1_Y;
    FormGPS->Label1.MaxLen = 0;
    FormGPS->Label1.pCaption = LABEL1_TEXT;
    FormGPS->Label1.Font = &FormGPS->FontBlack;

    FormGPS->Label3.Left = LABEL3_X;
    FormGPS->Label3.Top = LABEL3_Y;
    FormGPS->Label3.MaxLen = 0;
    FormGPS->Label3.pCaption = LABEL3_TEXT;
    FormGPS->Label3.Font = &FormGPS->FontBlack;

    FormGPS->Label5.Left = LABEL5_X;
    FormGPS->Label5.Top = LABEL5_Y;
    FormGPS->Label5.MaxLen = 0;
    FormGPS->Label5.pCaption = LABEL5_TEXT;
    FormGPS->Label5.Font = &FormGPS->FontBlack;

    FormGPS->Label7.Left = LABEL7_X;
    FormGPS->Label7.Top = LABEL7_Y;
    FormGPS->Label7.MaxLen = 0;
    FormGPS->Label7.pCaption = LABEL7_TEXT;
    FormGPS->Label7.Font = &FormGPS->FontBlack;

    FormGPS->Label9.Left = LABEL9_X;
    FormGPS->Label9.Top = LABEL9_Y;
    FormGPS->Label9.MaxLen = 0;
    FormGPS->Label9.pCaption = LABEL9_TEXT;
    FormGPS->Label9.Font = &FormGPS->FontBlack;

    /* 动态标签 */
    FormGPS->Label2.Left = LABEL2_X;
    FormGPS->Label2.Top = LABEL2_Y;
    FormGPS->Label2.MaxLen = 0;
    FormGPS->Label2.pCaption = LABEL2_TEXT;
    FormGPS->Label2.Font = &FormGPS->FontBlue;

    FormGPS->Label4.Left = LABEL4_X;
    FormGPS->Label4.Top = LABEL4_Y;
    FormGPS->Label4.MaxLen = 0;
    FormGPS->Label4.pCaption = LABEL4_TEXT;
    FormGPS->Label4.Font = &FormGPS->FontBlue;

    FormGPS->Label6.Left = LABEL6_X;
    FormGPS->Label6.Top = LABEL6_Y;
    FormGPS->Label6.MaxLen = 0;
    FormGPS->Label6.pCaption = LABEL6_TEXT;
    FormGPS->Label6.Font = &FormGPS->FontBlue;

    FormGPS->Label8.Left = LABEL8_X;
    FormGPS->Label8.Top = LABEL8_Y;
    FormGPS->Label8.MaxLen = 0;
    FormGPS->Label8.pCaption = LABEL8_TEXT;
    FormGPS->Label8.Font = &FormGPS->FontBlue;

    FormGPS->Label10.Left = LABEL10_X;
    FormGPS->Label10.Top = LABEL10_Y;
    FormGPS->Label10.MaxLen = 0;
    FormGPS->Label10.pCaption = LABEL10_TEXT;
    FormGPS->Label10.Font = &FormGPS->FontBlue;

    /* 按钮 */
    FormGPS->BtnRet.Left = BTN_RET_X;
    FormGPS->BtnRet.Top = BTN_RET_Y;
    FormGPS->BtnRet.Height = BTN_RET_H;
    FormGPS->BtnRet.Width = BTN_RET_W;
    FormGPS->BtnRet.pCaption = BTN_RET_TEXT;
    FormGPS->BtnRet.Font = &FormGPS->FontBtn;
    FormGPS->BtnRet.Focus = 0;
}

/*
*********************************************************************************************************
*    函 数 名: DispGPSInitFace
*    功能说明: 显示所有的静态控件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispGPSInitFace(void)
{
    LCD_ClrScr(CL_BTN_FACE);

    /* 分组框 */
    LCD_DrawGroupBox(&FormGPS->Box1);

    /* 静态标签 */
    LCD_DrawLabel(&FormGPS->Label1);
    LCD_DrawLabel(&FormGPS->Label3);
    LCD_DrawLabel(&FormGPS->Label5);
    LCD_DrawLabel(&FormGPS->Label7);
    LCD_DrawLabel(&FormGPS->Label9);

    /* 动态标签 */
    LCD_DrawLabel(&FormGPS->Label2);
    LCD_DrawLabel(&FormGPS->Label4);
    LCD_DrawLabel(&FormGPS->Label6);
    LCD_DrawLabel(&FormGPS->Label8);
    LCD_DrawLabel(&FormGPS->Label10);

    /* 按钮 */
    LCD_DrawButton(&FormGPS->BtnRet);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
