/*
*********************************************************************************************************
*
*	模块名称 : 摄像头模块程序。
*	文件名称 : camera_test.c
*	版    本 : V1.1
*	说    明 : 使用STM32F429的DCMI摄像头接口，显示图像
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2013-02-01 armfly  首发 STM32F407
*		v1.1    2015-10-17 armfly  移植到STM32F429
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_camera.h"

/* 定义界面结构 */
typedef struct
{
  FONT_T FontBlack; /* 静态的文字 */
  FONT_T FontBlue;  /* 变化的文字字体 蓝色 */
  FONT_T FontRed;   /* 变化的文字字体 红色 */
  FONT_T FontBtn;   /* 按钮的字体 */
  FONT_T FontBox;   /* 分组框标题字体 */

  GROUP_T Box1;

  LABEL_T Label1;
  LABEL_T Label2; /* 状态 */
  LABEL_T Label3;
  LABEL_T Label4; /* 状态 */
  LABEL_T Label5;
  LABEL_T Label6; /* 状态 */

  BUTTON_T Btn1; /* 打开摄像头 */
  BUTTON_T Btn2; /* 暂停 */

  BUTTON_T BtnRet;

} FormCAM_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR CL_BTN_FACE

/* 4个框的坐标和大小 */
#define BOX1_X 5
#define BOX1_Y 5
#define BOX1_H (g_LcdHeight - BOX1_Y - 10)
#define BOX1_W (g_LcdWidth - 2 * BOX1_X)
#define BOX1_TEXT "摄像头0V7670测试程序"

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H 32
#define BTN_RET_W 60
#define BTN_RET_X ((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define BTN_RET_Y ((BOX1_Y + BOX1_H) - BTN_RET_H - 4)
#define BTN_RET_TEXT "返回"

#define BTN1_H 32
#define BTN1_W 100
#define BTN1_X (BOX1_X + 330)
#define BTN1_Y (BOX1_Y + 100)
#define BTN1_TEXT "打开摄像头"

#define BTN2_H 32
#define BTN2_W 100
#define BTN2_X BTN1_X
#define BTN2_Y (BTN1_Y + BTN1_H + 10)
#define BTN2_TEXT "关闭摄像头"

/* 标签 */
#define LABEL1_X (BOX1_X + 330)
#define LABEL1_Y (BOX1_Y + 20)
#define LABEL1_TEXT "Chip ID : "

#define LABEL2_X (LABEL1_X + 80)
#define LABEL2_Y LABEL1_Y
#define LABEL2_TEXT "--"

#define LABEL3_X (LABEL1_X)
#define LABEL3_Y (LABEL1_Y + 20)
#define LABEL3_TEXT "状态1   : "

#define LABEL4_X (LABEL3_X + 80)
#define LABEL4_Y (LABEL3_Y)
#define LABEL4_TEXT "--"

#define LABEL5_X (LABEL1_X)
#define LABEL5_Y (LABEL1_Y + 20 * 2)
#define LABEL5_TEXT "状态2   : "

#define LABEL6_X (LABEL5_X + 80)
#define LABEL6_Y (LABEL5_Y)
#define LABEL6_TEXT "--"

/* 摄像显示窗口位置和大小 */
#define PHOTO_X 10
#define PHOTO_Y 22
#define PHOTO_H 240
#define PHOTO_W 320

static void InitFormCam(void);
static void DispCamInitFace(void);

FormCAM_T *FormCam;

/* Buffer location should aligned to cache line size (32 bytes) */
#define CAN_BUF_SIZE (320 * 265 * 2)
ALIGN_32BYTES(uint16_t s_CamCache[CAN_BUF_SIZE]);

/*
*********************************************************************************************************
*	函 数 名: TestCamera
*	功能说明: 测试摄像头
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestCamera(void)
{
  uint8_t ucKeyCode; /* 按键代码 */
  uint8_t ucTouch;   /* 触摸事件 */
  uint8_t fQuit = 0;
  int16_t tpX, tpY;
  uint16_t usChipID;
  char buf[128];
  uint8_t fStop = 0;

  FormCAM_T form;
  uint8_t fRefresh;

  FormCam = &form;

  InitFormCam();

  bsp_InitCamera();
  DispCamInitFace();

  if (i2c_CheckDevice(OV7670_SLAVE_ADDRESS) == 0)
  {
    usChipID = OV_ReadID();
    sprintf(buf, "0x%04X", usChipID);

    FormCam->Label2.Font = &FormCam->FontBlue;
    FormCam->Label2.pCaption = buf;
  }
  else
  {
    sprintf(buf, "None  ");

    FormCam->Label2.Font = &FormCam->FontRed;
    FormCam->Label2.pCaption = buf;
  }
  LCD_DrawLabel(&FormCam->Label2);

  LCD_DrawRect(PHOTO_X - 1, PHOTO_Y - 1, PHOTO_H + 2, PHOTO_W + 2, CL_RED);

  fRefresh = 1;

  /* 进入主程序循环体 */
  while (fQuit == 0)
  {
    bsp_Idle();

    if (fRefresh)
    {
      fRefresh = 0;

      //LCD_DrawLabel(&FormCam->Label6);
    }

    if (g_tCam.CaptureOk == 1)
    {
      g_tCam.CaptureOk = 0;

      /* 开始绘图 ,图片在 s_CamCache */
      SCB_CleanInvalidateDCache();
      LCD429_DrawBMP(PHOTO_X, PHOTO_Y, PHOTO_H, PHOTO_W, (uint16_t *)s_CamCache);

      if (fStop == 0)
      {
        bsp_StartTimer(0, 50); /* 启动定时器 */
      }
    }

    if (bsp_CheckTimer(0))
    {
      {
        //g_tTP.Enable = 0;

        //RA8875_StartDirectDraw(PHOTO_X, PHOTO_Y, PHOTO_H, PHOTO_W);
        CAM_Start((uint32_t)s_CamCache);
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
          FormCam->BtnRet.Focus = 1;
          LCD_DrawButton(&FormCam->BtnRet);
        }
        else if (TOUCH_InRect(tpX, tpY, BTN1_X, BTN1_Y, BTN1_H, BTN1_W))
        {
          FormCam->Btn1.Focus = 1;
          LCD_DrawButton(&FormCam->Btn1);
        }
        else if (TOUCH_InRect(tpX, tpY, BTN2_X, BTN2_Y, BTN2_H, BTN2_W))
        {
          FormCam->Btn2.Focus = 1;
          LCD_DrawButton(&FormCam->Btn2);
        }
        break;

      case TOUCH_MOVE: /* 触笔移动事件 */
        break;

      case TOUCH_RELEASE: /* 触笔释放事件 */
        if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
        {
          FormCam->BtnRet.Focus = 0;
          LCD_DrawButton(&FormCam->BtnRet);
          fQuit = 1; /* 返回 */
        }
        else if (TOUCH_InRect(tpX, tpY, BTN1_X, BTN1_Y, BTN1_H, BTN1_W)) /* 打开摄像头 */
        {
          FormCam->Btn1.Focus = 0;
          LCD_DrawButton(&FormCam->Btn1);

          {
            if (i2c_CheckDevice(OV7670_SLAVE_ADDRESS) == 0)
            {
              usChipID = OV_ReadID();
              sprintf(buf, "0x%04X", usChipID);

              FormCam->Label2.Font = &FormCam->FontBlue;
              FormCam->Label2.pCaption = buf;
            }
            else
            {
              sprintf(buf, "None  ");

              FormCam->Label2.Font = &FormCam->FontRed;
              FormCam->Label2.pCaption = buf;
            }
            LCD_DrawLabel(&FormCam->Label2);
          }

          {
            //g_tTP.Enable = 0;

            //RA8875_StartDirectDraw(PHOTO_X, PHOTO_Y, PHOTO_H, PHOTO_W);
            CAM_Start((uint32_t)s_CamCache); /* 摄像头DMA的目标地址设置为显存 */
          }
          fStop = 0;
          fRefresh = 1;
        }
        else if (TOUCH_InRect(tpX, tpY, BTN2_X, BTN2_Y, BTN2_H, BTN2_W))
        {
          FormCam->Btn2.Focus = 0;
          LCD_DrawButton(&FormCam->Btn2);

          bsp_StopTimer(0); /* 停止自动定时器 */

          fStop = 1;
          fRefresh = 1;
        }
        else /* 按钮失去焦点 */
        {
          FormCam->BtnRet.Focus = 0;
          LCD_DrawButton(&FormCam->BtnRet);

          FormCam->Btn1.Focus = 0;
          LCD_DrawButton(&FormCam->Btn1);

          FormCam->Btn2.Focus = 0;
          LCD_DrawButton(&FormCam->Btn2);
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
        CAM_Stop();     /* 停止摄像头DMA */
        g_tTP.Enable = 1;
        break;

      default:
        break;
      }
    }
  }

  bsp_StopTimer(0); /* 停止自动定时器 */

  CAM_Stop();
  //	RA8875_QuitDirectDraw();
  g_tTP.Enable = 1;
}

/*
*********************************************************************************************************
*	函 数 名: InitFormCam
*	功能说明: 初始化GPS初始界面控件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormCam(void)
{
  /* 分组框标题字体 */
  FormCam->FontBox.FontCode = FC_ST_16;
  FormCam->FontBox.BackColor = CL_BTN_FACE; /* 和背景色相同 */
  FormCam->FontBox.FrontColor = CL_BLACK;
  FormCam->FontBox.Space = 0;

  /* 字体1 用于静止标签 */
  FormCam->FontBlack.FontCode = FC_ST_16;
  FormCam->FontBlack.BackColor = CL_MASK; /* 透明色 */
  FormCam->FontBlack.FrontColor = CL_BLACK;
  FormCam->FontBlack.Space = 0;

  /* 字体2 用于变化的文字 */
  FormCam->FontBlue.FontCode = FC_ST_16;
  FormCam->FontBlue.BackColor = CL_BTN_FACE;
  FormCam->FontBlue.FrontColor = CL_BLUE;
  FormCam->FontBlue.Space = 0;

  /* 字体3 用于变化的文字 */
  FormCam->FontRed.FontCode = FC_ST_16;
  FormCam->FontRed.BackColor = CL_BTN_FACE;
  FormCam->FontRed.FrontColor = CL_RED;
  FormCam->FontRed.Space = 0;

  /* 按钮字体 */
  FormCam->FontBtn.FontCode = FC_ST_16;
  FormCam->FontBtn.BackColor = CL_MASK; /* 透明背景 */
  FormCam->FontBtn.FrontColor = CL_BLACK;
  FormCam->FontBtn.Space = 0;

  /* 分组框 */
  FormCam->Box1.Left = BOX1_X;
  FormCam->Box1.Top = BOX1_Y;
  FormCam->Box1.Height = BOX1_H;
  FormCam->Box1.Width = BOX1_W;
  FormCam->Box1.pCaption = BOX1_TEXT;
  FormCam->Box1.Font = &FormCam->FontBox;

  /* 静态标签 */
  FormCam->Label1.Left = LABEL1_X;
  FormCam->Label1.Top = LABEL1_Y;
  FormCam->Label1.MaxLen = 0;
  FormCam->Label1.pCaption = LABEL1_TEXT;
  FormCam->Label1.Font = &FormCam->FontBlack;

  FormCam->Label3.Left = LABEL3_X;
  FormCam->Label3.Top = LABEL3_Y;
  FormCam->Label3.MaxLen = 0;
  FormCam->Label3.pCaption = LABEL3_TEXT;
  FormCam->Label3.Font = &FormCam->FontBlack;

  FormCam->Label5.Left = LABEL5_X;
  FormCam->Label5.Top = LABEL5_Y;
  FormCam->Label5.MaxLen = 0;
  FormCam->Label5.pCaption = LABEL5_TEXT;
  FormCam->Label5.Font = &FormCam->FontBlack;

  /* 动态标签 */
  FormCam->Label2.Left = LABEL2_X;
  FormCam->Label2.Top = LABEL2_Y;
  FormCam->Label2.MaxLen = 0;
  FormCam->Label2.pCaption = LABEL2_TEXT;
  FormCam->Label2.Font = &FormCam->FontBlue;

  FormCam->Label4.Left = LABEL4_X;
  FormCam->Label4.Top = LABEL4_Y;
  FormCam->Label4.MaxLen = 0;
  FormCam->Label4.pCaption = LABEL4_TEXT;
  FormCam->Label4.Font = &FormCam->FontBlue;

  FormCam->Label6.Left = LABEL6_X;
  FormCam->Label6.Top = LABEL6_Y;
  FormCam->Label6.MaxLen = 0;
  FormCam->Label6.pCaption = LABEL6_TEXT;
  FormCam->Label6.Font = &FormCam->FontBlue;

  /* 按钮 */
  FormCam->BtnRet.Left = BTN_RET_X;
  FormCam->BtnRet.Top = BTN_RET_Y;
  FormCam->BtnRet.Height = BTN_RET_H;
  FormCam->BtnRet.Width = BTN_RET_W;
  FormCam->BtnRet.pCaption = BTN_RET_TEXT;
  FormCam->BtnRet.Font = &FormCam->FontBtn;
  FormCam->BtnRet.Focus = 0;

  FormCam->Btn1.Left = BTN1_X;
  FormCam->Btn1.Top = BTN1_Y;
  FormCam->Btn1.Height = BTN1_H;
  FormCam->Btn1.Width = BTN1_W;
  FormCam->Btn1.pCaption = BTN1_TEXT;
  FormCam->Btn1.Font = &FormCam->FontBtn;
  FormCam->Btn1.Focus = 0;

  FormCam->Btn2.Left = BTN2_X;
  FormCam->Btn2.Top = BTN2_Y;
  FormCam->Btn2.Height = BTN2_H;
  FormCam->Btn2.Width = BTN2_W;
  FormCam->Btn2.pCaption = BTN2_TEXT;
  FormCam->Btn2.Font = &FormCam->FontBtn;
  FormCam->Btn2.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispUSBInitFace
*	功能说明: 显示所有的控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispCamInitFace(void)
{
  LCD_ClrScr(CL_BTN_FACE);

  /* 分组框 */
  LCD_DrawGroupBox(&FormCam->Box1);

  /* 标签 */
  LCD_DrawLabel(&FormCam->Label1);
  LCD_DrawLabel(&FormCam->Label2);
  LCD_DrawLabel(&FormCam->Label3);
  LCD_DrawLabel(&FormCam->Label4);
  LCD_DrawLabel(&FormCam->Label5);
  LCD_DrawLabel(&FormCam->Label6);

  /* 按钮 */
  LCD_DrawButton(&FormCam->BtnRet);
  LCD_DrawButton(&FormCam->Btn1);
  LCD_DrawButton(&FormCam->Btn2);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
