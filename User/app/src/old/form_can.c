/*
*********************************************************************************************************
*
*    模块名称 : CAN网络测试界面
*    文件名称 : form_can.c
*    版    本 : V1.0
*    说    明 : 两个开发板之间进行CAN网络互通测试
*    修改记录 :
*        版本号  日期       作者    说明
*        v1.0    2015-08-09 armfly  首发
*
*    Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_can.h"
#include "can_network.h"

/* 定义界面结构 */
typedef struct
{
    FONT_T FontBlack; /* 静态的文字 */
    FONT_T FontBlue;    /* 变化的文字字体 */
    FONT_T FontBtn;        /* 按钮的字体 */
    FONT_T FontBox;        /* 分组框标题字体 */

    GROUP_T Box1;

    LABEL_T Label1;
    LABEL_T Label2;
    LABEL_T Label3;
    LABEL_T Label4;
    LABEL_T Label5;
    LABEL_T Label6;

    BUTTON_T BtnRet;

    BUTTON_T Btn1;
    BUTTON_T Btn2;
    BUTTON_T Btn3;
    BUTTON_T Btn4;
    BUTTON_T Btn5;
    BUTTON_T Btn6;

    LABEL_T LblInfo1;
    LABEL_T LblInfo2;
} FormCAN_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H 100
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_TEXT "CAN网络测试"

/* 第1个框内的按钮 */
#define BTN1_H 32
#define BTN1_W 100
#define BTN1_X (BOX1_X + 10)
#define BTN1_Y (BOX1_Y + 20)
#define BTN1_TEXT "点亮LED2"

#define BTN2_H BTN1_H
#define BTN2_W BTN1_W
#define BTN2_X (BTN1_X + BTN1_W + 10)
#define BTN2_Y BTN1_Y
#define BTN2_TEXT "关闭LED2"

#define BTN3_H BTN1_H
#define BTN3_W BTN1_W
#define BTN3_X BTN1_X
#define BTN3_Y (BTN1_Y + BTN1_H + 10)
#define BTN3_TEXT "蜂鸣2声"

#define BTN4_H BTN1_H
#define BTN4_W BTN1_W
#define BTN4_X (BTN1_X + BTN1_W + 10)
#define BTN4_Y (BTN1_Y + BTN1_H + 10)
#define BTN4_TEXT "蜂鸣3声"

#define BTN5_H BTN1_H
#define BTN5_W BTN1_W
#define BTN5_X (BTN1_X + 2 * (BTN1_W + 10))
#define BTN5_Y (BTN1_Y + BTN1_H + 10)
#define BTN5_TEXT "蜂鸣20声"

#define BTN6_H BTN1_H
#define BTN6_W BTN1_W
#define BTN6_X (BTN1_X + 3 * (BTN1_W + 10))
#define BTN6_Y (BTN1_Y + BTN1_H + 10)
#define BTN6_TEXT "停止蜂鸣"

#define LABEL1_X (BTN5_X + 10)
#define LABEL1_Y BTN2_Y
#define LABEL1_TEXT "地址: "

#define LABEL2_X (LABEL1_X + 48)
#define LABEL2_Y LABEL1_Y
#define LABEL2_TEXT "0"

#define LABEL3_X (LABEL2_X + 32)
#define LABEL3_Y LABEL1_Y
#define LABEL3_TEXT "波特率: "

#define LABEL4_X (LABEL3_X + 64)
#define LABEL4_Y (LABEL3_Y)
#define LABEL4_TEXT "0"

#define LABEL5_X (LABEL1_X)
#define LABEL5_Y (LABEL1_Y + 20)
#define LABEL5_TEXT "接收:"

#define LABEL6_X (LABEL5_X + 48)
#define LABEL6_Y LABEL5_Y
#define LABEL6_TEXT " "

#define LBL_INFO1_X (BOX1_X)
#define LBL_INFO1_Y (BOX1_Y + BOX1_H + 10)
#define LBL_INFO1_TEXT "请将两个开发板的CAN接口互联"

#define LBL_INFO2_X LBL_INFO1_X
#define LBL_INFO2_Y (LBL_INFO1_Y + 20)
#define LBL_INFO2_TEXT "可以互相控制对方的LED2和蜂鸣器"

/* 按钮 */
/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 80
#define BTN_RET_X (g_LcdWidth - BTN_RET_W - 8)
#define BTN_RET_Y (g_LcdHeight - BTN_RET_H - 4)
#define BTN_RET_TEXT "返回"

static void InitFormCAN(void);
static void DispFormCAN(void);

static void DispLabelBaud(uint32_t _Baud);
static void DispLabelRx(uint8_t *_buf, uint8_t _len);

FormCAN_T *FormCAN;

/*
*********************************************************************************************************
*    函 数 名: FormMainCAN
*    功能说明: CAN测试主程序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void FormMainCAN(void)
{
    uint8_t ucTouch; /* 触摸事件 */
    uint8_t fQuit = 0;
    int16_t tpX, tpY;
    FormCAN_T form;
    uint32_t baud;

    FormCAN = &form;

    InitFormCAN();
    DispFormCAN();

    baud = 500000;
    DispLabelBaud(baud);

    can_Init(); /* 初始化STM32 CAN硬件 */

    /* 进入主程序循环体 */
    bsp_StartAutoTimer(2, 10);
    while (fQuit == 0)
    {
        bsp_Idle();

        //        MODBUS_Poll();

        if (bsp_CheckTimer(2))
        {
            can_LedOn(1, 1);
        }

        ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
        if (ucTouch != TOUCH_NONE)
        {
            switch (ucTouch)
            {
            case TOUCH_DOWN: /* 触笔按下事件 */
                if (LCD_ButtonTouchDown(&FormCAN->BtnRet, tpX, tpY))
                {
                    //fQuit = 1;
                }
                else if (LCD_ButtonTouchDown(&FormCAN->Btn1, tpX, tpY))
                {
                    can_LedOn(1, 2);
                }
                else if (LCD_ButtonTouchDown(&FormCAN->Btn2, tpX, tpY))
                {
                    can_LedOff(1, 2);
                }
                else if (LCD_ButtonTouchDown(&FormCAN->Btn3, tpX, tpY))
                {
                    can_BeepCtrl(1, 2); /* 控制蜂鸣器鸣叫2声 */
                }
                else if (LCD_ButtonTouchDown(&FormCAN->Btn4, tpX, tpY))
                {
                    can_BeepCtrl(1, 3); /* 控制蜂鸣器鸣叫3声 */
                }
                else if (LCD_ButtonTouchDown(&FormCAN->Btn5, tpX, tpY))
                {
                    can_BeepCtrl(1, 20); /* 控制蜂鸣器鸣叫20声 */
                }
                else if (LCD_ButtonTouchDown(&FormCAN->Btn6, tpX, tpY))
                {
                    can_BeepCtrl(1, 0); /* 控制蜂鸣器停止鸣叫 */
                }
                break;

            case TOUCH_RELEASE: /* 触笔释放事件 */
                if (LCD_ButtonTouchRelease(&FormCAN->BtnRet, tpX, tpY))
                {
                    fQuit = 1; /* 返回 */
                }
                else
                {
                    LCD_ButtonTouchRelease(&FormCAN->BtnRet, tpX, tpY);
                    LCD_ButtonTouchRelease(&FormCAN->Btn1, tpX, tpY);
                    LCD_ButtonTouchRelease(&FormCAN->Btn2, tpX, tpY);
                    LCD_ButtonTouchRelease(&FormCAN->Btn3, tpX, tpY);
                    LCD_ButtonTouchRelease(&FormCAN->Btn4, tpX, tpY);
                    LCD_ButtonTouchRelease(&FormCAN->Btn5, tpX, tpY);
                    LCD_ButtonTouchRelease(&FormCAN->Btn6, tpX, tpY);
                }
                break;
            }
        }

        /* 处理应用层消息 */
        {
            MSG_T msg;

            if (bsp_GetMsg(&msg))
            {
                switch (msg.MsgCode)
                {
                case MSG_CAN1_RX: /* 接收到CAN设备的应答 */
                    DispLabelRx(g_Can1RxData, g_Can1RxHeader.DataLength);
                    can1_Analyze();
                    break;

                case MSG_CAN2_RX: /* 接收到CAN设备的应答 */
                    DispLabelRx(g_Can2RxData, g_Can2RxHeader.DataLength);
                    can1_Analyze();
                    break;
                }
            }
        }
    }

    can_DeInit(); /* 关闭CAN时钟 */
}

/*
*********************************************************************************************************
*    函 数 名: InitFormCAN
*    功能说明: 初始化控件属性
*    形    参：无
*    返 回 值: 无
*********************************************************************************************************
*/
static void InitFormCAN(void)
{
    /* 分组框标题字体 */
    FormCAN->FontBox.FontCode = FC_ST_16;
    FormCAN->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
    FormCAN->FontBox.FrontColor = CL_BLACK;
    FormCAN->FontBox.Space = 0;

    /* 字体1 用于静止标签 */
    FormCAN->FontBlack.FontCode = FC_ST_16;
    FormCAN->FontBlack.BackColor = CL_MASK; /* 透明色 */
    FormCAN->FontBlack.FrontColor = CL_BLACK;
    FormCAN->FontBlack.Space = 0;

    /* 字体2 用于变化的文字 */
    FormCAN->FontBlue.FontCode = FC_ST_16;
    FormCAN->FontBlue.BackColor = CL_BTN_FACE;
    FormCAN->FontBlue.FrontColor = CL_BLUE;
    FormCAN->FontBlue.Space = 0;

    /* 按钮字体 */
    FormCAN->FontBtn.FontCode = FC_ST_16;
    FormCAN->FontBtn.BackColor = CL_MASK; /* 透明背景 */
    FormCAN->FontBtn.FrontColor = CL_BLACK;
    FormCAN->FontBtn.Space = 0;

    /* 分组框 */
    FormCAN->Box1.Left = BOX1_X;
    FormCAN->Box1.Top = BOX1_Y;
    FormCAN->Box1.Height = BOX1_H;
    FormCAN->Box1.Width = BOX1_W;
    FormCAN->Box1.pCaption = BOX1_TEXT;
    FormCAN->Box1.Font = &FormCAN->FontBox;

    /* 静态标签 */
    FormCAN->Label1.Left = LABEL1_X;
    FormCAN->Label1.Top = LABEL1_Y;
    FormCAN->Label1.MaxLen = 0;
    FormCAN->Label1.pCaption = LABEL1_TEXT;
    FormCAN->Label1.Font = &FormCAN->FontBlack;

    FormCAN->Label3.Left = LABEL3_X;
    FormCAN->Label3.Top = LABEL3_Y;
    FormCAN->Label3.MaxLen = 0;
    FormCAN->Label3.pCaption = LABEL3_TEXT;
    FormCAN->Label3.Font = &FormCAN->FontBlack;

    FormCAN->Label5.Left = LABEL5_X;
    FormCAN->Label5.Top = LABEL5_Y;
    FormCAN->Label5.MaxLen = 0;
    FormCAN->Label5.pCaption = LABEL5_TEXT;
    FormCAN->Label5.Font = &FormCAN->FontBlack;

    /* 动态标签 */
    FormCAN->Label2.Left = LABEL2_X;
    FormCAN->Label2.Top = LABEL2_Y;
    FormCAN->Label2.MaxLen = 0;
    FormCAN->Label2.pCaption = LABEL2_TEXT;
    FormCAN->Label2.Font = &FormCAN->FontBlue;

    FormCAN->Label4.Left = LABEL4_X;
    FormCAN->Label4.Top = LABEL4_Y;
    FormCAN->Label4.MaxLen = 0;
    FormCAN->Label4.pCaption = LABEL4_TEXT;
    FormCAN->Label4.Font = &FormCAN->FontBlue;

    FormCAN->Label6.Left = LABEL6_X;
    FormCAN->Label6.Top = LABEL6_Y;
    FormCAN->Label6.MaxLen = 0;
    FormCAN->Label6.pCaption = LABEL6_TEXT;
    FormCAN->Label6.Font = &FormCAN->FontBlue;

    /* 按钮 */
    FormCAN->BtnRet.Left = BTN_RET_X;
    FormCAN->BtnRet.Top = BTN_RET_Y;
    FormCAN->BtnRet.Height = BTN_RET_H;
    FormCAN->BtnRet.Width = BTN_RET_W;
    FormCAN->BtnRet.pCaption = BTN_RET_TEXT;
    FormCAN->BtnRet.Font = &FormCAN->FontBtn;
    FormCAN->BtnRet.Focus = 0;

    FormCAN->Btn1.Left = BTN1_X;
    FormCAN->Btn1.Top = BTN1_Y;
    FormCAN->Btn1.Height = BTN1_H;
    FormCAN->Btn1.Width = BTN1_W;
    FormCAN->Btn1.pCaption = BTN1_TEXT;
    FormCAN->Btn1.Font = &FormCAN->FontBtn;
    FormCAN->Btn1.Focus = 0;

    FormCAN->Btn2.Left = BTN2_X;
    FormCAN->Btn2.Top = BTN2_Y;
    FormCAN->Btn2.Height = BTN2_H;
    FormCAN->Btn2.Width = BTN2_W;
    FormCAN->Btn2.pCaption = BTN2_TEXT;
    FormCAN->Btn2.Font = &FormCAN->FontBtn;
    FormCAN->Btn2.Focus = 0;

    FormCAN->Btn3.Left = BTN3_X;
    FormCAN->Btn3.Top = BTN3_Y;
    FormCAN->Btn3.Height = BTN3_H;
    FormCAN->Btn3.Width = BTN3_W;
    FormCAN->Btn3.pCaption = BTN3_TEXT;
    FormCAN->Btn3.Font = &FormCAN->FontBtn;
    FormCAN->Btn3.Focus = 0;

    FormCAN->Btn4.Left = BTN4_X;
    FormCAN->Btn4.Top = BTN4_Y;
    FormCAN->Btn4.Height = BTN4_H;
    FormCAN->Btn4.Width = BTN4_W;
    FormCAN->Btn4.pCaption = BTN4_TEXT;
    FormCAN->Btn4.Font = &FormCAN->FontBtn;
    FormCAN->Btn4.Focus = 0;

    FormCAN->Btn5.Left = BTN5_X;
    FormCAN->Btn5.Top = BTN5_Y;
    FormCAN->Btn5.Height = BTN5_H;
    FormCAN->Btn5.Width = BTN5_W;
    FormCAN->Btn5.pCaption = BTN5_TEXT;
    FormCAN->Btn5.Font = &FormCAN->FontBtn;
    FormCAN->Btn5.Focus = 0;

    FormCAN->Btn6.Left = BTN6_X;
    FormCAN->Btn6.Top = BTN6_Y;
    FormCAN->Btn6.Height = BTN6_H;
    FormCAN->Btn6.Width = BTN6_W;
    FormCAN->Btn6.pCaption = BTN6_TEXT;
    FormCAN->Btn6.Font = &FormCAN->FontBtn;
    FormCAN->Btn6.Focus = 0;

    {
        FormCAN->LblInfo1.Left = LBL_INFO1_X;
        FormCAN->LblInfo1.Top = LBL_INFO1_Y;
        FormCAN->LblInfo1.MaxLen = 0;
        FormCAN->LblInfo1.pCaption = LBL_INFO1_TEXT;
        FormCAN->LblInfo1.Font = &FormCAN->FontBlack;

        FormCAN->LblInfo2.Left = LBL_INFO2_X;
        FormCAN->LblInfo2.Top = LBL_INFO2_Y;
        FormCAN->LblInfo2.MaxLen = 0;
        FormCAN->LblInfo2.pCaption = LBL_INFO2_TEXT;
        FormCAN->LblInfo2.Font = &FormCAN->FontBlack;
    }
}

/*
*********************************************************************************************************
*    函 数 名: DispFormCAN
*    功能说明: 显示所有的静态控件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispFormCAN(void)
{
    LCD_ClrScr(CL_BTN_FACE);

    /* 分组框 */
    LCD_DrawGroupBox(&FormCAN->Box1);

    LCD_DrawLabel(&FormCAN->Label1);
    LCD_DrawLabel(&FormCAN->Label3);
    LCD_DrawLabel(&FormCAN->Label5);

    LCD_DrawLabel(&FormCAN->Label2);
    LCD_DrawLabel(&FormCAN->Label4);
    LCD_DrawLabel(&FormCAN->Label6);

    /* 按钮 */
    LCD_DrawButton(&FormCAN->Btn1);
    LCD_DrawButton(&FormCAN->Btn2);
    LCD_DrawButton(&FormCAN->Btn3);
    LCD_DrawButton(&FormCAN->Btn4);
    LCD_DrawButton(&FormCAN->Btn5);
    LCD_DrawButton(&FormCAN->Btn6);

    LCD_DrawLabel(&FormCAN->LblInfo1);
    LCD_DrawLabel(&FormCAN->LblInfo2);

    LCD_DrawButton(&FormCAN->BtnRet);
}

/*
*********************************************************************************************************
*    函 数 名: DispLabelTx
*    功能说明: 显示发送的数据
*    形    参: _Baud 波特率
*              _
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispLabelBaud(uint32_t _Baud)
{
    char buf[10];

    sprintf(buf, "%d", _Baud);

    /* 动态标签 */
    FormCAN->Label4.pCaption = buf;
    LCD_DrawLabel(&FormCAN->Label4);
}

/*
*********************************************************************************************************
*    函 数 名: DispLabelTx
*    功能说明: 显示发送的数据
*    形    参: _buf 要发送的数据
*              _
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispLabelRx(uint8_t *_buf, uint8_t _len)
{
    char disp_buf[32];
    uint8_t len;

    len = _len;
    if (len > sizeof(disp_buf) / 3)
    {
        len = sizeof(disp_buf) / 3;
    }

    HexToAscll(_buf, disp_buf, len);

    /* 动态标签 */
    FormCAN->Label6.pCaption = disp_buf;
    LCD_DrawLabel(&FormCAN->Label6);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
