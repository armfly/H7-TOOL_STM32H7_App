/*
*********************************************************************************************************
*
*	模块名称 : RS485测试界面
*	文件名称 : form_rs485.c
*	版    本 : V1.0
*	说    明 : 驱动安富莱LED-485系列数码管显示屏。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2014-10-15 armfly  首发
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_rs485.h"

/* 定义界面结构 */
typedef struct
{
  FONT_T FontBlack; /* 静态的文字 */
  FONT_T FontBlue;  /* 变化的文字字体 */
  FONT_T FontBtn;   /* 按钮的字体 */
  FONT_T FontBox;   /* 分组框标题字体 */

  GROUP_T Box1;
  GROUP_T Box2;
  GROUP_T Box3;

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

  BUTTON_T BtnBright[8]; /* 设置亮度 */

  LABEL_T LabelOldAddr;
  LABEL_T LabelNewAddr;
  EDIT_T EditOldAddr;
  EDIT_T EditNewAddr;
  BUTTON_T BtnSetAddr;

  LABEL_T LblInfo1;
  LABEL_T LblInfo2;
} FormRS485_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 8
#define BOX1_H 100
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_TEXT "RS485 LED数码管"

/* 第1个框内的按钮 */
#define BTN1_H 32
#define BTN1_W 100
#define BTN1_X (BOX1_X + 10)
#define BTN1_Y (BOX1_Y + 20)
#define BTN1_TEXT "ASCII协议"

#define BTN2_H BTN1_H
#define BTN2_W BTN1_W
#define BTN2_X (BTN1_X + BTN1_W + 10)
#define BTN2_Y BTN1_Y
#define BTN2_TEXT "Modbus协议"

#define BTN3_H BTN1_H
#define BTN3_W BTN1_W
#define BTN3_X BTN1_X
#define BTN3_Y (BTN1_Y + BTN1_H + 10)
#define BTN3_TEXT "读设备型号"

#define BTN4_H BTN1_H
#define BTN4_W BTN1_W
#define BTN4_X (BTN1_X + BTN1_W + 10)
#define BTN4_Y (BTN1_Y + BTN1_H + 10)
#define BTN4_TEXT "读固件版本"

#define BTN5_H BTN1_H
#define BTN5_W BTN1_W
#define BTN5_X (BTN1_X + 2 * (BTN1_W + 10))
#define BTN5_Y (BTN1_Y + BTN1_H + 10)
#define BTN5_TEXT "测试应答"

#define BTN6_H BTN1_H
#define BTN6_W BTN1_W
#define BTN6_X (BTN1_X + 3 * (BTN1_W + 10))
#define BTN6_Y (BTN1_Y + BTN1_H + 10)
#define BTN6_TEXT "读亮度参数"

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

/* 第2个框 */
#define BOX2_X BOX1_X
#define BOX2_Y (BOX1_Y + BOX1_H + 10)
#define BOX2_H 60
#define BOX2_W (g_LcdWidth - 2 * BOX1_X)
#define BOX2_TEXT "设置亮度"

#define BTNB_H 32
#define BTNB_W 45
#define BTNB_X (BOX2_X + 10)
#define BTNB_Y (BOX2_Y + 20)

/* 第3个框 */
#define BOX3_X BOX1_X
#define BOX3_Y (BOX2_Y + BOX2_H + 10)
#define BOX3_H 72
#define BOX3_W (g_LcdWidth - 2 * BOX1_X)
#define BOX3_TEXT "修改485地址"

#define LABEL_OLDADDR_X (BOX3_X + 10)
#define LABEL_OLDADDR_Y (BOX3_Y + 20)
#define LABEL_OLDADDR_TEXT "当前地址:"

#define EDIT_OLDADDR_X (LABEL_OLDADDR_X + 80)
#define EDIT_OLDADDR_Y LABEL_OLDADDR_Y
#define EDIT_OLDADDR_H 20
#define EDIT_OLDADDR_W 50

#define LABEL_NEWADDR_X LABEL_OLDADDR_X
#define LABEL_NEWADDR_Y (LABEL_OLDADDR_Y + 30)
#define LABEL_NEWADDR_TEXT "  新地址:"

#define EDIT_NEWADDR_X EDIT_OLDADDR_X
#define EDIT_NEWADDR_Y LABEL_NEWADDR_Y
#define EDIT_NEWDDR_H EDIT_OLDADDR_H
#define EDIT_NEWADDR_W EDIT_OLDADDR_W

#define BTN_SETADDR_H 50
#define BTN_SETADDR_W 100
#define BTN_SETADDR_X (EDIT_NEWADDR_X + EDIT_NEWADDR_W + 10)
#define BTN_SETADDR_Y EDIT_OLDADDR_Y
#define BTN_SETADDR_TEXT "修改485地址"

#define LBL_INFO1_X (BOX3_X + 270)
#define LBL_INFO1_Y (BOX3_Y + 11)
#define LBL_INFO1_TEXT "摇杆上下键: 修改新地址"

#define LBL_INFO2_X LBL_INFO1_X
#define LBL_INFO2_Y (LBL_INFO1_Y + 20)
#define LBL_INFO2_TEXT "摇杆左右键: 修改当前地址"

/* 按钮 */
/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 80
#define BTN_RET_X (g_LcdWidth - BTN_RET_W - 8)
#define BTN_RET_Y (g_LcdHeight - BTN_RET_H - 4)
#define BTN_RET_TEXT "返回"

static void InitFormRS485(void);
static void DispFormRS485(void);

static void DispLabelAddr(uint8_t _addr1, uint8_t _addr2);
static void DispLabelBaud(uint32_t _Baud);
static void DispLabelRx(uint8_t *_buf, uint8_t _len);

FormRS485_T *FormRS485;
/*
*********************************************************************************************************
*	函 数 名: FormMainRS485
*	功能说明: RS485测试主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void FormMainRS485(void)
{
  uint8_t ucKeyCode; /* 按键代码 */
  uint8_t ucTouch;   /* 触摸事件 */
  uint8_t fQuit = 0;
  int16_t tpX, tpY;
  FormRS485_T form;
  uint8_t OldAddr;
  uint8_t NewAddr;
  uint32_t baud;
  uint32_t i;
  uint32_t count = 0;
  uint8_t fUpdateCount = 1;
  uint8_t fUpdateAddr = 1;
  uint8_t fLed888 = 0;

  FormRS485 = &form;

  InitFormRS485();

  DispFormRS485();

  OldAddr = 1;
  NewAddr = 1;
  baud = 9600;

  DispLabelBaud(baud);

  /* 进入主程序循环体 */
  while (fQuit == 0)
  {
    bsp_Idle();

    MODH_Poll();

    if (fUpdateAddr == 1)
    {
      fUpdateAddr = 0;

      DispLabelAddr(OldAddr, NewAddr);
    }

    if (fUpdateCount == 1)
    {
      fUpdateCount = 0;

      LED485_DispNumberA(OldAddr, count); /* 命令LED数码管显示计数值 */
    }

    ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
    if (ucTouch != TOUCH_NONE)
    {
      switch (ucTouch)
      {
      case TOUCH_DOWN: /* 触笔按下事件 */
        if (LCD_ButtonTouchDown(&FormRS485->BtnRet, tpX, tpY))
        {
          //fQuit = 1;
        }
        else if (LCD_ButtonTouchDown(&FormRS485->Btn1, tpX, tpY))
        {
          LED485_SetProtAscii(OldAddr); /* 设置为ASCII协议 */
        }
        else if (LCD_ButtonTouchDown(&FormRS485->Btn2, tpX, tpY))
        {
          LED485_SetProtRTU(OldAddr); /* 设置为Modbus RTU 协议 */
        }
        else if (LCD_ButtonTouchDown(&FormRS485->Btn3, tpX, tpY))
        {
          LED485_ReadModel(OldAddr); /* 读设备型号 */
        }
        else if (LCD_ButtonTouchDown(&FormRS485->Btn4, tpX, tpY))
        {
          LED485_ReadVersion(OldAddr); /* 读固件版本 */
        }
        else if (LCD_ButtonTouchDown(&FormRS485->Btn5, tpX, tpY))
        {
          LED485_TestOk(OldAddr); /* 测试OK应答 */
        }
        else if (LCD_ButtonTouchDown(&FormRS485->Btn6, tpX, tpY))
        {
          LED485_ReadBright(OldAddr); /* 读亮度参数 */
        }
        else if (LCD_ButtonTouchDown(&FormRS485->BtnSetAddr, tpX, tpY))
        {
          /* 修改地址 */
          LED485_ModifyAddrA(OldAddr, NewAddr);
        }
        else
        {
          for (i = 0; i < 8; i++)
          {
            if (LCD_ButtonTouchDown(&FormRS485->BtnBright[i], tpX, tpY))
            {
              LED485_SetBrightA(OldAddr, i); /* 设置亮度参数(ASCII协议) */
            }
          }
        }
        break;

      case TOUCH_RELEASE: /* 触笔释放事件 */
        if (LCD_ButtonTouchRelease(&FormRS485->BtnRet, tpX, tpY))
        {
          fQuit = 1; /* 返回 */
        }
        else
        {
          LCD_ButtonTouchRelease(&FormRS485->BtnRet, tpX, tpY);
          LCD_ButtonTouchRelease(&FormRS485->Btn1, tpX, tpY);
          LCD_ButtonTouchRelease(&FormRS485->Btn2, tpX, tpY);
          LCD_ButtonTouchRelease(&FormRS485->Btn3, tpX, tpY);
          LCD_ButtonTouchRelease(&FormRS485->Btn4, tpX, tpY);
          LCD_ButtonTouchRelease(&FormRS485->Btn5, tpX, tpY);
          LCD_ButtonTouchRelease(&FormRS485->Btn6, tpX, tpY);
          LCD_ButtonTouchRelease(&FormRS485->BtnSetAddr, tpX, tpY);
          for (i = 0; i < 8; i++)
          {
            LCD_ButtonTouchRelease(&FormRS485->BtnBright[i], tpX, tpY);
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
      case MSG_485_RX_NOT_RTU: /* 接收到485设备的应答 */
        DispLabelRx(g_tModH.AppRxBuf, g_tModH.AppRxCount);
        break;

      case KEY_DOWN_K1: /* K1键 + 1*/
        count++;
        fUpdateCount = 1;
        break;

      case KEY_DOWN_K2: /* K2键 - 1 */
        if (count > 0)
        {
          count--;
        }
        fUpdateCount = 1;
        break;

      case KEY_DOWN_K3: /* K3键 - 清0 */
        count = 0;
        fUpdateCount = 1;
        break;

      case JOY_DOWN_U: /* 摇杆UP键按下 */
        NewAddr++;
        fUpdateAddr = 1;
        break;

      case JOY_DOWN_D: /* 摇杆DOWN键按下 */
        NewAddr--;
        fUpdateAddr = 1;
        break;

      case JOY_DOWN_L: /* 摇杆LEFT键按下 */
        OldAddr++;
        fUpdateAddr = 1;
        break;

      case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
        OldAddr--;
        fUpdateAddr = 1;
        break;

      case JOY_DOWN_OK: /* 摇杆OK键按下 */
        /* 自动测试 */
        if (fLed888 == 0)
        {
          fLed888 = 1;
          LED485_DispStrA(OldAddr, "8.8.8.");
        }
        else if (fLed888 == 1)
        {
          fLed888 = 2;
          LED485_DispStrA(OldAddr, "8.8.8.8");
        }
        else
        {
          fLed888 = 0;
          LED485_DispStrA(OldAddr, "   ");
        }
        break;

      default:
        break;
      }
    }
  }
}

/*
*********************************************************************************************************
*	函 数 名: InitFormRS485
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormRS485(void)
{
  /* 分组框标题字体 */
  FormRS485->FontBox.FontCode = FC_ST_16;
  FormRS485->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
  FormRS485->FontBox.FrontColor = CL_BLACK;
  FormRS485->FontBox.Space = 0;

  /* 字体1 用于静止标签 */
  FormRS485->FontBlack.FontCode = FC_ST_16;
  FormRS485->FontBlack.BackColor = CL_MASK; /* 透明色 */
  FormRS485->FontBlack.FrontColor = CL_BLACK;
  FormRS485->FontBlack.Space = 0;

  /* 字体2 用于变化的文字 */
  FormRS485->FontBlue.FontCode = FC_ST_16;
  FormRS485->FontBlue.BackColor = CL_BTN_FACE;
  FormRS485->FontBlue.FrontColor = CL_BLUE;
  FormRS485->FontBlue.Space = 0;

  /* 按钮字体 */
  FormRS485->FontBtn.FontCode = FC_ST_16;
  FormRS485->FontBtn.BackColor = CL_MASK; /* 透明背景 */
  FormRS485->FontBtn.FrontColor = CL_BLACK;
  FormRS485->FontBtn.Space = 0;

  /* 分组框 */
  FormRS485->Box1.Left = BOX1_X;
  FormRS485->Box1.Top = BOX1_Y;
  FormRS485->Box1.Height = BOX1_H;
  FormRS485->Box1.Width = BOX1_W;
  FormRS485->Box1.pCaption = BOX1_TEXT;
  FormRS485->Box1.Font = &FormRS485->FontBox;

  FormRS485->Box2.Left = BOX2_X;
  FormRS485->Box2.Top = BOX2_Y;
  FormRS485->Box2.Height = BOX2_H;
  FormRS485->Box2.Width = BOX2_W;
  FormRS485->Box2.pCaption = BOX2_TEXT;
  FormRS485->Box2.Font = &FormRS485->FontBox;

  /* 静态标签 */
  FormRS485->Label1.Left = LABEL1_X;
  FormRS485->Label1.Top = LABEL1_Y;
  FormRS485->Label1.MaxLen = 0;
  FormRS485->Label1.pCaption = LABEL1_TEXT;
  FormRS485->Label1.Font = &FormRS485->FontBlack;

  FormRS485->Label3.Left = LABEL3_X;
  FormRS485->Label3.Top = LABEL3_Y;
  FormRS485->Label3.MaxLen = 0;
  FormRS485->Label3.pCaption = LABEL3_TEXT;
  FormRS485->Label3.Font = &FormRS485->FontBlack;

  FormRS485->Label5.Left = LABEL5_X;
  FormRS485->Label5.Top = LABEL5_Y;
  FormRS485->Label5.MaxLen = 0;
  FormRS485->Label5.pCaption = LABEL5_TEXT;
  FormRS485->Label5.Font = &FormRS485->FontBlack;

  /* 动态标签 */
  FormRS485->Label2.Left = LABEL2_X;
  FormRS485->Label2.Top = LABEL2_Y;
  FormRS485->Label2.MaxLen = 0;
  FormRS485->Label2.pCaption = LABEL2_TEXT;
  FormRS485->Label2.Font = &FormRS485->FontBlue;

  FormRS485->Label4.Left = LABEL4_X;
  FormRS485->Label4.Top = LABEL4_Y;
  FormRS485->Label4.MaxLen = 0;
  FormRS485->Label4.pCaption = LABEL4_TEXT;
  FormRS485->Label4.Font = &FormRS485->FontBlue;

  FormRS485->Label6.Left = LABEL6_X;
  FormRS485->Label6.Top = LABEL6_Y;
  FormRS485->Label6.MaxLen = 0;
  FormRS485->Label6.pCaption = LABEL6_TEXT;
  FormRS485->Label6.Font = &FormRS485->FontBlue;

  /* 按钮 */
  FormRS485->BtnRet.Left = BTN_RET_X;
  FormRS485->BtnRet.Top = BTN_RET_Y;
  FormRS485->BtnRet.Height = BTN_RET_H;
  FormRS485->BtnRet.Width = BTN_RET_W;
  FormRS485->BtnRet.pCaption = BTN_RET_TEXT;
  FormRS485->BtnRet.Font = &FormRS485->FontBtn;
  FormRS485->BtnRet.Focus = 0;

  FormRS485->Btn1.Left = BTN1_X;
  FormRS485->Btn1.Top = BTN1_Y;
  FormRS485->Btn1.Height = BTN1_H;
  FormRS485->Btn1.Width = BTN1_W;
  FormRS485->Btn1.pCaption = BTN1_TEXT;
  FormRS485->Btn1.Font = &FormRS485->FontBtn;
  FormRS485->Btn1.Focus = 0;

  FormRS485->Btn2.Left = BTN2_X;
  FormRS485->Btn2.Top = BTN2_Y;
  FormRS485->Btn2.Height = BTN2_H;
  FormRS485->Btn2.Width = BTN2_W;
  FormRS485->Btn2.pCaption = BTN2_TEXT;
  FormRS485->Btn2.Font = &FormRS485->FontBtn;
  FormRS485->Btn2.Focus = 0;

  FormRS485->Btn3.Left = BTN3_X;
  FormRS485->Btn3.Top = BTN3_Y;
  FormRS485->Btn3.Height = BTN3_H;
  FormRS485->Btn3.Width = BTN3_W;
  FormRS485->Btn3.pCaption = BTN3_TEXT;
  FormRS485->Btn3.Font = &FormRS485->FontBtn;
  FormRS485->Btn3.Focus = 0;

  FormRS485->Btn4.Left = BTN4_X;
  FormRS485->Btn4.Top = BTN4_Y;
  FormRS485->Btn4.Height = BTN4_H;
  FormRS485->Btn4.Width = BTN4_W;
  FormRS485->Btn4.pCaption = BTN4_TEXT;
  FormRS485->Btn4.Font = &FormRS485->FontBtn;
  FormRS485->Btn4.Focus = 0;

  FormRS485->Btn5.Left = BTN5_X;
  FormRS485->Btn5.Top = BTN5_Y;
  FormRS485->Btn5.Height = BTN5_H;
  FormRS485->Btn5.Width = BTN5_W;
  FormRS485->Btn5.pCaption = BTN5_TEXT;
  FormRS485->Btn5.Font = &FormRS485->FontBtn;
  FormRS485->Btn5.Focus = 0;

  FormRS485->Btn6.Left = BTN6_X;
  FormRS485->Btn6.Top = BTN6_Y;
  FormRS485->Btn6.Height = BTN6_H;
  FormRS485->Btn6.Width = BTN6_W;
  FormRS485->Btn6.pCaption = BTN6_TEXT;
  FormRS485->Btn6.Font = &FormRS485->FontBtn;
  FormRS485->Btn6.Focus = 0;

  {
    uint8_t i;
    char *BrightStr[8] = {
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
    };

    for (i = 0; i < 8; i++)
    {
      FormRS485->BtnBright[i].Left = BTNB_X + (BTNB_W + 10) * i;
      FormRS485->BtnBright[i].Top = BTNB_Y;
      FormRS485->BtnBright[i].Height = BTNB_H;
      FormRS485->BtnBright[i].Width = BTNB_W;
      FormRS485->BtnBright[i].pCaption = BrightStr[i];
      FormRS485->BtnBright[i].Font = &FormRS485->FontBtn;
      FormRS485->BtnBright[i].Focus = 0;
    }
  }

  {
    FormRS485->Box3.Left = BOX3_X;
    FormRS485->Box3.Top = BOX3_Y;
    FormRS485->Box3.Height = BOX3_H;
    FormRS485->Box3.Width = BOX3_W;
    FormRS485->Box3.pCaption = BOX3_TEXT;
    FormRS485->Box3.Font = &FormRS485->FontBox;

    FormRS485->LabelOldAddr.Left = LABEL_OLDADDR_X;
    FormRS485->LabelOldAddr.Top = LABEL_OLDADDR_Y;
    FormRS485->LabelOldAddr.MaxLen = 0;
    FormRS485->LabelOldAddr.pCaption = LABEL_OLDADDR_TEXT;
    FormRS485->LabelOldAddr.Font = &FormRS485->FontBlack;

    FormRS485->LabelNewAddr.Left = LABEL_NEWADDR_X;
    FormRS485->LabelNewAddr.Top = LABEL_NEWADDR_Y;
    FormRS485->LabelNewAddr.MaxLen = 0;
    FormRS485->LabelNewAddr.pCaption = LABEL_NEWADDR_TEXT;
    FormRS485->LabelNewAddr.Font = &FormRS485->FontBlack;

    FormRS485->EditOldAddr.Left = EDIT_OLDADDR_X;
    FormRS485->EditOldAddr.Top = EDIT_OLDADDR_Y;
    FormRS485->EditOldAddr.Height = EDIT_OLDADDR_H;
    FormRS485->EditOldAddr.Width = EDIT_OLDADDR_W;
    FormRS485->EditOldAddr.pCaption = FormRS485->EditOldAddr.Text;
    FormRS485->EditOldAddr.Font = &FormRS485->FontBtn;

    FormRS485->EditNewAddr.Left = EDIT_NEWADDR_X;
    FormRS485->EditNewAddr.Top = EDIT_NEWADDR_Y;
    FormRS485->EditNewAddr.Height = EDIT_NEWDDR_H;
    FormRS485->EditNewAddr.Width = EDIT_NEWADDR_W;
    FormRS485->EditNewAddr.pCaption = FormRS485->EditNewAddr.Text;
    FormRS485->EditNewAddr.Font = &FormRS485->FontBtn;

    FormRS485->BtnSetAddr.Left = BTN_SETADDR_X;
    FormRS485->BtnSetAddr.Top = BTN_SETADDR_Y;
    FormRS485->BtnSetAddr.Height = BTN_SETADDR_H;
    FormRS485->BtnSetAddr.Width = BTN_SETADDR_W;
    FormRS485->BtnSetAddr.pCaption = BTN_SETADDR_TEXT;
    FormRS485->BtnSetAddr.Font = &FormRS485->FontBtn;
    FormRS485->BtnSetAddr.Focus = 0;

    FormRS485->EditOldAddr.Text[0] = 0;
    FormRS485->EditNewAddr.Text[0] = 0;

    FormRS485->LblInfo1.Left = LBL_INFO1_X;
    FormRS485->LblInfo1.Top = LBL_INFO1_Y;
    FormRS485->LblInfo1.MaxLen = 0;
    FormRS485->LblInfo1.pCaption = LBL_INFO1_TEXT;
    FormRS485->LblInfo1.Font = &FormRS485->FontBlack;

    FormRS485->LblInfo2.Left = LBL_INFO2_X;
    FormRS485->LblInfo2.Top = LBL_INFO2_Y;
    FormRS485->LblInfo2.MaxLen = 0;
    FormRS485->LblInfo2.pCaption = LBL_INFO2_TEXT;
    FormRS485->LblInfo2.Font = &FormRS485->FontBlack;
  }
}

/*
*********************************************************************************************************
*	函 数 名: DispFormRS485
*	功能说明: 显示所有的静态控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormRS485(void)
{
  LCD_ClrScr(CL_BTN_FACE);

  /* 分组框 */
  LCD_DrawGroupBox(&FormRS485->Box1);
  LCD_DrawGroupBox(&FormRS485->Box2);

  LCD_DrawLabel(&FormRS485->Label1);
  LCD_DrawLabel(&FormRS485->Label3);
  LCD_DrawLabel(&FormRS485->Label5);

  LCD_DrawLabel(&FormRS485->Label2);
  LCD_DrawLabel(&FormRS485->Label4);
  LCD_DrawLabel(&FormRS485->Label6);

  /* 按钮 */
  LCD_DrawButton(&FormRS485->Btn1);
  LCD_DrawButton(&FormRS485->Btn2);
  LCD_DrawButton(&FormRS485->Btn3);
  LCD_DrawButton(&FormRS485->Btn4);
  LCD_DrawButton(&FormRS485->Btn5);
  LCD_DrawButton(&FormRS485->Btn6);

  {
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
      LCD_DrawButton(&FormRS485->BtnBright[i]);
    }
  }

  LCD_DrawGroupBox(&FormRS485->Box3);
  LCD_DrawLabel(&FormRS485->LabelOldAddr);
  LCD_DrawLabel(&FormRS485->LabelNewAddr);
  LCD_DrawEdit(&FormRS485->EditOldAddr);
  LCD_DrawEdit(&FormRS485->EditNewAddr);
  LCD_DrawButton(&FormRS485->BtnSetAddr);
  LCD_DrawLabel(&FormRS485->LblInfo1);
  LCD_DrawLabel(&FormRS485->LblInfo2);

  LCD_DrawButton(&FormRS485->BtnRet);
}

/*
*********************************************************************************************************
*	函 数 名: DispLabelAddr
*	功能说明: 显示485地址
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispLabelAddr(uint8_t _addr1, uint8_t _addr2)
{
  char buf[10];

  sprintf(buf, "%d", _addr1);

  /* 动态标签 */
  FormRS485->Label2.pCaption = buf;
  LCD_DrawLabel(&FormRS485->Label2);

  sprintf(FormRS485->EditOldAddr.Text, "%d", _addr1);
  LCD_DrawEdit(&FormRS485->EditOldAddr);

  sprintf(FormRS485->EditNewAddr.Text, "%d", _addr2);
  LCD_DrawEdit(&FormRS485->EditNewAddr);
}

/*
*********************************************************************************************************
*	函 数 名: DispLabelTx
*	功能说明: 显示发送的数据
*	形    参: _Baud 波特率
*			  _
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispLabelBaud(uint32_t _Baud)
{
  char buf[10];

  sprintf(buf, "%d", _Baud);

  /* 动态标签 */
  FormRS485->Label4.pCaption = buf;
  LCD_DrawLabel(&FormRS485->Label4);
}

/*
*********************************************************************************************************
*	函 数 名: DispLabelRx
*	功能说明: 显示接收到的数据
*	形    参: _buf 要发送的数据
*			  _
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispLabelRx(uint8_t *_buf, uint8_t _len)
{
  char buf[16];

  _buf[_len] = 0;
  sprintf(buf, "%d | %s", _len, _buf);

  /* 动态标签 */
  FormRS485->Label6.pCaption = buf;
  LCD_DrawLabel(&FormRS485->Label6);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
