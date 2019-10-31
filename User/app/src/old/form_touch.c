/*
*********************************************************************************************************
*
*	模块名称 : 触摸测试界面。
*	文件名称 : touch_test.c
*	版    本 : V1.1
*	说    明 : 测试屏是否有缺陷，触摸是否准确
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-01-01 armfly  正式发布
*		V1.1    2014-09-06 armfly  增加图片显示速度测试功能
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h" /* printf函数定向输出到串口，所以必须包含这个文件 */

#include "form_main_menu.h"
#include "images.h"
#include "param.h"
#include "ff.h"       /* FatFS 文件系统头文件 */
#include "usbh_usr.h" /* usb host u盘支持 */
#include "ff_gen_drv.h"
#include "sd_diskio_dma.h"
#include "usbh_diskio.h"
#include "nand_diskio.h"

//#define PIC_DISK	FS_NAND		/* 选择缺省读取的磁盘： FS_NAND, FS_SD, FS_USB */
#define PIC_PATH "/Pic" /* MP3文件存放的缺省文件夹， 根目录下的PIC */

/* 定义界面结构 */
typedef struct
{
  FONT_T FontBlack; /* 静态的文字 */
  FONT_T FontBlue;  /* 变化的文字字体 */

  BUTTON_T Btn1; /* 拨号 */
  BUTTON_T Btn2; /* 挂机 */
  BUTTON_T Btn3; /* 切换音频通道 */

  BUTTON_T BtnRet; /* 返回 */

  char strHardInfo[32]; /* 模块硬件信息 */
  uint8_t ucNetStatus;  /* 网络状态 */
  uint8_t ucAudioCh;    /* 当前音频通道 0， 1 */
  uint8_t ucEarVolume;  /* 耳机音量 0 - 5 */
  int16_t ucMicGain;    /* MIC音量  -12：最小增益  12：最大增益  13：静音*/

} FormGPRS_T;

/* 返回按钮的坐标(屏幕右下角) */
#define BUTTON_RET_H 32
#define BUTTON_RET_W 60
#define BUTTON_RET_X (g_LcdWidth - BUTTON_RET_W - 4)
#define BUTTON_RET_Y (g_LcdHeight - BUTTON_RET_H - 4)

static LABEL_T Label1; /* 显示PS2设备状态 */
static LABEL_T Label2; /* 显示PS2键盘和鼠标发出的键值 */
static LABEL_T Label3; /* 显示摇杆，红外，按钮事件 */

static FONT_T FontLabel;

/* 图片地址 RA8875图片芯片偏移地址 */
const uint32_t g_RA8875Addr_480x272[3] =
    {
        0x00200000, /* 480x272 沙滩背景 */
        0x0023FC00, /* 480x272 树叶背景 */
        0x0027F800, /* 480x272 水珠背景 */
};
const uint32_t g_RA8875Addr_800x480[3] =
    {
        0x002BF400, /* 800x480 沙滩背景 */
        0x0037AC00, /* 800x480 树叶背景 */
        0x00436400  /* 800x480 水珠背景 */
};

/* NOR FLASH中存储的图片地址 */
const uint32_t g_NorAddr_480x272[3] =
    {
        (0x64000000 + 0x00200000), /* 480x272 沙滩背景 */
        (0x64000000 + 0x0023FC00), /* 480x272 树叶背景 */
        (0x64000000 + 0x0027F800), /* 480x272 水珠背景 */
};
const uint32_t g_NorAddr_800x480[3] =
    {
        (0x64000000 + 0x002BF400), /* 800x480 沙滩背景 */
        (0x64000000 + 0x0037AC00), /* 800x480 树叶背景 */
        (0x64000000 + 0x00436400)  /* 800x480 水珠背景 */
};

/* NOR FLASH, PIC目录下 存储的图片文件名 */
const char *g_NandFileName_480x272[3] =
    {
        "480272_1.bin",
        "480272_2.bin",
        "480272_3.bin",
        //"480x272_4.bin",
        //"480x272_5.bin",
        //"480x272_6.bin"
};

const char *g_NandFileName_800x480[3] =
    {
        "800480_1.bin",
        "800480_2.bin",
        "800480_3.bin",
        //"800x480_4.bin",
        //"800x480_5.bin",
        //"800x480_6.bin"
};

typedef struct
{
  uint32_t code;
  char *str;
} KB_STR_T;

static const KB_STR_T s_KeyNameTab[] =
    {
        {0xEEEEEEEE, ""},

        {KB_A, "A"},
        {KB_B, "B"},
        {KB_C, "C"},
        {KB_D, "D"},
        {KB_E, "E"},
        {KB_F, "F"},
        {KB_G, "G"},
        {KB_H, "H"},
        {KB_I, "I"},
        {KB_J, "J"},
        {KB_K, "K"},
        {KB_L, "L"},
        {KB_M, "M"},
        {KB_N, "N"},
        {KB_O, "O"},
        {KB_P, "P"},
        {KB_Q, "Q"},
        {KB_R, "R"},
        {KB_S, "S"},
        {KB_T, "T"},
        {KB_U, "U"},
        {KB_V, "V"},
        {KB_W, "W"},
        {KB_X, "X"},
        {KB_Y, "Y"},
        {KB_Z, "Z"},
        {KB_0, "0"},
        {KB_1, "1"},
        {KB_2, "2"},
        {KB_3, "3"},
        {KB_4, "4"},
        {KB_5, "5"},
        {KB_6, "6"},
        {KB_7, "7"},
        {KB_8, "8"},
        {KB_9, "9"},
        {KB_PIE, "`"},  /* 撇，键盘左上角 */
        {KB_SUB, "-"},  /* 中杠，减号 */
        {KB_EQU, "="},  /* 等号 */
        {KB_FXG, "\\"}, /* 反斜杠 */
        {KB_BKSP, "Backspace"},
        {KB_SPACE, "Space"},
        {KB_TAB, "Tab"},
        {KB_CAPS, "CapsLk"},
        {KB_L_SHFT, "Shift Left"},
        {KB_L_CTRL, "Ctrl Left"},
        {KB_L_GUI, "GUI Left"},
        {KB_L_ALT, "Alt Left"},
        {KB_R_SHFT, "Shift Right"},
        {KB_R_CTRL, "Ctrl Right"},
        {KB_R_GUI, "Gui Right"},
        {KB_R_ALT, "Alt Right"},
        {KB_APPS, "Apps"},
        {KB_ENTER, "Enter"},
        {KB_ESC, "ESC"},
        {KB_F1, "F1"},
        {KB_F2, "F2"},
        {KB_F3, "F3"},
        {KB_F4, "F4"},
        {KB_F5, "F5"},
        {KB_F6, "F6"},
        {KB_F7, "F7"},
        {KB_F8, "F8"},
        {KB_F9, "F9"},
        {KB_F10, "F10"},
        {KB_F11, "F11"},
        {KB_F12, "F12"},
        {KB_PRNT_SCRN, "Print Screen/SysRq"},
        {KB_SCROLL, "Scroll Lock"},
        {KB_PAUSE, "Pause/Break"},
        {KB_ZZKH, "["},
        {KB_INSERT, "Insert"},
        {KB_HOME, "Home"},
        {KB_PGUP, "Page Up"},
        {KB_DELETE, "Delete"},
        {KB_END, "End"},
        {KB_PGDN, "Page Down"},
        {KB_U_ARROW, "Up Arrow"},
        {KB_L_ARROW, "Left Arrow"},
        {KB_D_ARROW, "Down Arrow"},
        {KB_R_ARROW, "Right Arrow"},
        {KB_NUM, "Num Lock"},
        {KB_KP_DIV, "KP /"},  /* 小键盘除号  KP 表示小键盘 */
        {KB_KP_MULT, "KP *"}, /* 小键盘乘号 */
        {KB_KP_SUB, "KP -"},  /* - */
        {KB_KP_ADD, "KP +"},
        {KB_KP_ENTER, "KP Enter"},
        {KB_KP_DOT, "KP ."}, /* 小数点 */
        {KB_KP_0, "KP 0"},
        {KB_KP_1, "KP 0"},
        {KB_KP_2, "KP 0"},
        {KB_KP_3, "KP 0"},
        {KB_KP_4, "KP 0"},
        {KB_KP_5, "KP 0"},
        {KB_KP_6, "KP 0"},
        {KB_KP_7, "KP 0"},
        {KB_KP_8, "KP 0"},
        {KB_KP_9, "KP 0"},
        {KB_YZKH, "]"},      /* ] 右中括号 */
        {KB_SEMICOLON, ";"}, /* ; 分号 */
        {KB_QUOTES, "'"},    /* 单引号 */
        {KB_COMMA, ","},     /* 逗号 */
        {KB_DOT, "."},       /* 小数点 */
        {KB_DIV, "/"},       /* 除号 */

        {0, ""} /* 查表结束标志 */
};

static void InitFormTouch(void);
static const char *GetNameOfKey(uint32_t _code);
static void DispPic(void);
uint8_t ReadFileDispPic(char *_strFileName, uint16_t _usX, uint16_t _usY, uint16_t usHeight, uint16_t usWidth);

static uint8_t s_pic = 0;

/*
*********************************************************************************************************
*	函 数 名: TestTouch
*	功能说明: 触摸测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TestTouch(void)
{
  uint8_t ucKeyCode;      /* 按键代码 */
  uint32_t uiPS2Msg;      /* PS2键盘鼠标消息 */
  uint8_t ucTouch;        /* 触摸事件 */
  uint8_t fRefresh;       /* 刷屏请求标志,1表示需要刷新 */
  FONT_T tFont, tFontBtn; /* 定义一个字体结构体变量，用于设置字体参数 */
  char buf[64];
  uint16_t usAdcX, usAdcY;
  int16_t tpX, tpY;
  uint8_t fQuit = 0;

  BUTTON_T tBtn;

  s_pic = 0;

  LCD_ClrScr(CL_BLUE); /* 清屏，背景蓝色 */

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;   /* 字体代码 16点阵 */
    tFont.FrontColor = CL_WHITE; /* 字体颜色 */
    tFont.BackColor = CL_BLUE;   /* 文字背景颜色 */
    tFont.Space = 0;             /* 文字间距，单位 = 像素 */
  }

  {
    tFontBtn.FontCode = FC_ST_16;
    tFontBtn.BackColor = CL_MASK; /* 背景透明 */
    tFontBtn.FrontColor = CL_BLACK;
    tFontBtn.Space = 0;
  }

  LCD_SetBackLight(g_tParam.ucBackLight); /* 设置背光亮度 */

  InitFormTouch();

#if 1
  bsp_InitPS2();
  PS2_StartWork();

  /* 初始化PS2设备. 键盘可以不配置. 鼠标必须配置后才会输出数据包 */
  {
    uint8_t ucDevType;

    ucDevType = PS2_GetDevceType();
    if (ucDevType == PS2_MOUSE)
    {
      Label1.pCaption = "检测到PS2鼠标";
      LCD_DrawLabel(&Label1);

      PS2_InitMouse();
    }
    else if (ucDevType == PS2_KEYBOARD)
    {
      Label1.pCaption = "检测到PS2键盘";
      LCD_DrawLabel(&Label1);

      PS2_InitKeyboard();
    }
    else
    {
      Label1.pCaption = "未检测到PS2设备";
      LCD_DrawLabel(&Label1);
    }
  }
#endif

  IRD_StartWork(); /* 开始红外解码 */

  usbh_OpenMassStorage(); /* 打开USB HOST， 支持U盘 */

  fRefresh = 1; /* 1表示需要刷新LCD */
  while (fQuit == 0)
  {
    bsp_Idle();

    usbh_Poll(); /* 支持U盘 */

    if (fRefresh)
    {
      fRefresh = 0;

      /* 实时刷新触摸ADC采样值和转换后的坐标 */
      {
        LCD_DispStr(5, 3, "测试触摸屏、按键、摇杆、LED灯、PS/S键盘鼠标 红外遥控器", &tFont); /* 在(8,3)坐标处显示一串汉字 */

        /* 显示TFT控制器型号和屏幕分辨率 */
        //LCD_GetChipDescribe(buf);	/* 读取TFT驱动芯片型号 */
        if (g_TouchType == CT_GT811)
        {
          strcpy(buf, "STM32H743 + GT811");
        }
        else if (g_TouchType == CT_GT911)
        {
          strcpy(buf, "STM32H743 + GT911");
        }
        else if (g_TouchType == CT_FT5X06)
        {
          strcpy(buf, "STM32H743 + FT5X06");
        }
        else if (g_TouchType == CT_STMPE811)
        {
          strcpy(buf, "STM32H743 + STMPE811");
        }
        else
        {
          strcpy(buf, "STM32H743 + NoTouch");
        }
        sprintf(&buf[strlen(buf)], "   %d x %d", LCD_GetWidth(), LCD_GetHeight());
        LCD_DispStr(5, 23, buf, &tFont); /* 在(8,3)坐标处显示一串汉字 */

        /* 读取并显示当前X轴和Y轴的ADC采样值 */
        usAdcX = TOUCH_ReadAdcX();
        usAdcY = TOUCH_ReadAdcY();
        sprintf(buf, "触摸ADC值 X = %4d, Y = %4d   ", usAdcX, usAdcY);
        LCD_DispStr(5, 60, buf, &tFont);

        /* 读取并显示当前触摸坐标 */
        tpX = TOUCH_GetX();
        tpY = TOUCH_GetY();
        sprintf(buf, "触摸坐标  X = %4d, Y = %4d   ", tpX, tpY);
        LCD_DispStr(5, 80, buf, &tFont);

        /* 在触笔所在位置显示一个小圈 */
        if ((tpX > 0) && (tpY > 0))
        {
          LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
        }
      }

      /* 显示图标阵列 */
      {
        tBtn.Font = &tFontBtn;

        tBtn.Left = BUTTON_RET_X;
        tBtn.Top = BUTTON_RET_Y;
        tBtn.Height = BUTTON_RET_H;
        tBtn.Width = BUTTON_RET_W;
        tBtn.Focus = 0; /* 失去焦点 */
        tBtn.pCaption = "返回";
        LCD_DrawButton(&tBtn);
      }

      /* 在屏幕边沿绘制2个矩形框(用于检测面板边缘像素是否正常) */
      LCD_DrawRect(0, 0, LCD_GetHeight(), LCD_GetWidth(), CL_WHITE);
      LCD_DrawRect(2, 2, LCD_GetHeight() - 4, LCD_GetWidth() - 4, CL_YELLOW);

      /* 显示背光值 */
      sprintf(buf, "当前背光亮度: %3d", g_tParam.ucBackLight);
      LCD_DispStr(5, 130, buf, &tFont);

      //if (g_ChipID == IC_8875)
      {
        /* 3.5寸屏不支持 */
        LCD_DispStr(5, LCD_GetHeight() - 20, "按K1 K2 键显示图片", &tFont);
      }
    }

    ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
    if (ucTouch != TOUCH_NONE)
    {
      switch (ucTouch)
      {
      case TOUCH_DOWN:  /* 触笔按下事件 */
        BEEP_KeyTone(); /* 按键提示音 */

        if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
        {
          tBtn.Font = &tFontBtn;

          tBtn.Left = BUTTON_RET_X;
          tBtn.Top = BUTTON_RET_Y;
          tBtn.Height = BUTTON_RET_H;
          tBtn.Width = BUTTON_RET_W;
          tBtn.Focus = 1; /* 焦点 */
          tBtn.pCaption = "返回";
          LCD_DrawButton(&tBtn);
        }

        /* 在触笔所在位置显示一个小圈 */
        if ((tpX > 0) && (tpY > 0))
        {
          LCD_DrawCircle(tpX, tpY, 3, CL_RED);
        }
        break;

      case TOUCH_MOVE: /* 触笔移动事件 */
        /* 实时刷新触摸ADC采样值和转换后的坐标 */
        {
          /* 读取并显示当前X轴和Y轴的ADC采样值 */
          usAdcX = TOUCH_ReadAdcX();
          usAdcY = TOUCH_ReadAdcY();
          sprintf(buf, "触摸ADC值 X = %4d, Y = %4d   ", usAdcX, usAdcY);
          LCD_DispStr(5, 60, buf, &tFont);

          /* 读取并显示当前触摸坐标 */
          //tpX = TOUCH_GetX();
          //tpY = TOUCH_GetY();
          sprintf(buf, "触摸坐标  X = %4d, Y = %4d   ", tpX, tpY);
          LCD_DispStr(5, 80, buf, &tFont);

          /* 在触笔所在位置显示一个小圈 */
          if ((tpX > 0) && (tpY > 0))
          {
            LCD_DrawCircle(tpX, tpY, 2, CL_YELLOW);
          }
        }
        break;

      case TOUCH_RELEASE: /* 触笔释放事件 */
        /* 在触笔所在位置显示一个小圈 */
        if ((tpX > 0) && (tpY > 0))
        {
          LCD_DrawCircle(tpX, tpY, 4, CL_WHITE);
        }

        if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
        {
          tBtn.Font = &tFontBtn;

          tBtn.Left = BUTTON_RET_X;
          tBtn.Top = BUTTON_RET_Y;
          tBtn.Height = BUTTON_RET_H;
          tBtn.Width = BUTTON_RET_W;
          tBtn.Focus = 1; /* 焦点 */
          tBtn.pCaption = "返回";
          LCD_DrawButton(&tBtn);

          SaveParam(); /* 保存参数 */
          fQuit = 1;
          break; /* 返回 */
        }
        else /* 按钮失去焦点 */
        {
          tBtn.Font = &tFontBtn;

          tBtn.Left = BUTTON_RET_X;
          tBtn.Top = BUTTON_RET_Y;
          tBtn.Height = BUTTON_RET_H;
          tBtn.Width = BUTTON_RET_W;
          tBtn.Focus = 0; /* 焦点 */
          tBtn.pCaption = "返回";
          LCD_DrawButton(&tBtn);
        }
      }
    }

    ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
    if (ucKeyCode != KEY_NONE)
    {
      /* 遥控按键发提示音 */
      if (ucKeyCode >= IR_KEY_STRAT)
      {
        BEEP_KeyTone(); /* 按键提示音 */
      }

      /* 有键按下 */
      switch (ucKeyCode)
      {
      case KEY_DOWN_K1: /* K1键按下 */
        bsp_LedOn(1);   /* 点亮LED1 */
        {
          if (s_pic > 0)
          {
            s_pic--;
          }
          DispPic(); /* 显示图片 */
        }
        break;

      case KEY_UP_K1:  /* K1键松开 */
        bsp_LedOff(1); /* 熄灭LED1 */
        break;

      case KEY_DOWN_K2: /* K2键按下 */
        bsp_LedOn(2);   /* 点亮LED2 */
        {
          if (s_pic < 14)
          {
            s_pic++;
          }
          DispPic(); /* 显示上一张图片 */
        }
        break;

      case KEY_UP_K2: /* K2键松开 */
        //Label3.pCaption = "K2键松开";
        //LCD_DrawLabel(&Label3);
        bsp_LedOff(2); /* 熄灭LED2 */
        break;

      case KEY_DOWN_K3: /* K3键按下 */
        Label3.pCaption = "K3键按下";
        LCD_DrawLabel(&Label3);
        bsp_LedOn(3); /* 点亮LED3 */
        break;

      case KEY_UP_K3: /* K3键松开 */
        Label3.pCaption = "K3键松开";
        LCD_DrawLabel(&Label3);
        bsp_LedOff(3); /* 熄灭LED3 */
        break;

      case JOY_DOWN_U: /* 摇杆UP键按下 */
        Label3.pCaption = "摇杆上键按下";
        LCD_DrawLabel(&Label3);

        if (g_tParam.ucBackLight < 255)
        {
          g_tParam.ucBackLight++;
        }
        LCD_SetBackLight(g_tParam.ucBackLight); /* 设置背光亮度 */
        fRefresh = 1;
        break;

      case JOY_DOWN_D: /* 摇杆DOWN键按下 */
        Label3.pCaption = "摇杆下键按下";
        LCD_DrawLabel(&Label3);
        if (g_tParam.ucBackLight > 0)
        {
          g_tParam.ucBackLight--;
        }
        LCD_SetBackLight(g_tParam.ucBackLight); /* 设置背光亮度 */
        fRefresh = 1;
        break;

      case JOY_DOWN_L: /* 摇杆LEFT键按下 */
        Label3.pCaption = "摇杆左键按下";
        LCD_DrawLabel(&Label3);
        break;

      case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
        Label3.pCaption = "摇杆右键按下";
        LCD_DrawLabel(&Label3);
        bsp_DelayMS(500);
        break;

      case JOY_DOWN_OK: /* 摇杆OK键按下 */
        Label3.pCaption = "摇杆OK键按下";
        LCD_DrawLabel(&Label3);
        bsp_LedOn(4); /* 点亮LED4 */
        break;

      case JOY_UP_OK: /* 摇杆OK键松开 */
        Label3.pCaption = "摇杆OK键松开";
        LCD_DrawLabel(&Label3);
        bsp_LedOff(4); /* 熄灭LED4 */
        break;

      /* 下面是红外遥控器的事件 */
      case IR_KEY_POWER:
        Label3.pCaption = "IR Power";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_MENU:
        Label3.pCaption = "IR Menu";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_TEST:
        Label3.pCaption = "IR Test";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_UP:
        Label3.pCaption = "IR +";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_RETURN:
        Label3.pCaption = "IR Return";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_LEFT:
        Label3.pCaption = "IR Left";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_OK:
        Label3.pCaption = "IR Ok";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_RIGHT:
        Label3.pCaption = "IR Right";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_0:
        Label3.pCaption = "IR 0";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_DOWN:
        Label3.pCaption = "IR -";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_C:
        Label3.pCaption = "IR C";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_1:
        Label3.pCaption = "IR 1";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_2:
        Label3.pCaption = "IR 2";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_3:
        Label3.pCaption = "IR 3";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_4:
        Label3.pCaption = "IR 4";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_5:
        Label3.pCaption = "IR 5";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_6:
        Label3.pCaption = "IR 6";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_7:
        Label3.pCaption = "IR 7";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_8:
        Label3.pCaption = "IR 8";
        LCD_DrawLabel(&Label3);
        break;

      case IR_KEY_9:
        Label3.pCaption = "IR 9";
        LCD_DrawLabel(&Label3);
        break;

      default:
        if (ucKeyCode >= IR_KEY_STRAT)
        {
          sprintf(buf, "IR 无法识别 %02X %02X %02X %02X", g_tIR.RxBuf[0], g_tIR.RxBuf[1],
                  g_tIR.RxBuf[2], g_tIR.RxBuf[3]);

          Label3.pCaption = buf;
          LCD_DrawLabel(&Label3);
        }
        break;
      }
    }

    /* PS2按键检测由中断服务程序实现，我们只需要调用PS2_GetMsg读取键值即可。 */
    uiPS2Msg = PS2_GetMsg();
    if (uiPS2Msg != PS2_NONE)
    {
      //sprintf(buf, "%X\r\n", uiPS2Msg);
      //Label1.pCaption = buf;
      //LCD_DrawLabel(&Label1);

      if (uiPS2Msg == 0xAA)
      {
        Label1.pCaption = "检测到PS2键盘";
        LCD_DrawLabel(&Label1);

        PS2_InitKeyboard();
      }
      if (uiPS2Msg == 0xAA00)
      {
        Label1.pCaption = "检测到PS2鼠标";
        LCD_DrawLabel(&Label1);

        PS2_InitMouse();
      }

      /* 打印PS2鼠标数据包解码结果 */
      if (PS2_IsMousePacket(uiPS2Msg))
      {
        MOUSE_PACKET_T mouse;

        PS2_DecodeMouse(uiPS2Msg, &mouse);

        sprintf(buf, "X=%4d, Y=%4d, Z=%2d, 左键=%d, 中键=%d, 右键=%d",
                mouse.Xmove, mouse.Ymove, mouse.Zmove,
                mouse.BtnLeft, mouse.BtnMid, mouse.BtnRight);

        Label2.pCaption = buf;
        LCD_DrawLabel(&Label2);
      }
      else /* 作为PS2键盘数据包处理 */
      {
        sprintf(buf, "%s", GetNameOfKey(uiPS2Msg));
        Label2.pCaption = buf;
        LCD_DrawLabel(&Label2);
      }
    }
  }

  PS2_StopWork(); /* 停止PS2中断 */
  IRD_StopWork(); /* 停止红外解码 */

  usbh_CloseMassStorage(); /* 关闭 USB HOST - U盘 */
}

/*
*********************************************************************************************************
*	函 数 名: DispPic
*	功能说明: 依次显示NAND Flash图片（3张）、SRAM单色图片（3张）、SD卡图片（3张）、U盘图片（3张）并计算刷屏时间
*	形    参: 无
*	返 回 值: 字符串指针
*********************************************************************************************************
*/
static void DispPic(void)
{
  char buf[64];

  int32_t time1, time2;
  FONT_T tFont;

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;   /* 字体代码 16点阵 */
    tFont.FrontColor = CL_WHITE; /* 字体颜色 */
    tFont.BackColor = CL_BLUE;   /* 文字背景颜色 */
    tFont.Space = 0;             /* 文字间距，单位 = 像素 */
  }

  /* K1键切换背景图片 */
  if (s_pic <= 2) /* NAND FLASH 中的图片3张 */
  {
    uint16_t color;

    if (s_pic == 0)
    {
      color = CL_RED;
    }
    else if (s_pic == 1)
    {
      color = CL_GREEN;
    }
    else
    {
      color = CL_BLUE;
    }

    time1 = bsp_GetRunTime();

    /* 
      DMA2D填充太快，只有几个ms，因此用示波器测量执行时间。
      使用 J6排针的第10脚 --- 5.0V扩展输出口最高bit
      开始绘制前设置 Y50_7 = 1; 完毕后设置 Y50_7 = 0.
      2015-11-19 测试结果:  (已经将DMA2D库函数就地展开进行优化)
      
      480 * 272 屏 : 1.84ms
      800 * 480 屏 : 3.68ms
    */
    HC574_SetPin(Y50_7, 1); /* Y50_7 = 1 */

    if (LCD_GetWidth() == 480) /* 4.3寸屏 480x272 */
    {
      LCD_Fill_Rect(0, 0, g_LcdHeight, g_LcdWidth, color);
    }
    else /* 5寸和7寸 800*480 屏 */
    {
      LCD_Fill_Rect(0, 0, g_LcdHeight, g_LcdWidth, color);
    }

    HC574_SetPin(Y50_7, 0); /* Y50_7 = 0, */

    time2 = bsp_GetRunTime();
    sprintf(buf, "图片%d DMA2D单色填充, 显示时间: %4dms", s_pic + 1, time2 - time1);
    LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);
  }
  else if (s_pic >= 3 && s_pic <= 5) /* SDRAM 中单色图片3张 */
  {
    uint16_t *p1 = (uint16_t *)SDRAM_APP_BUF;

    /* 填充1个单色区域 */
    {
      uint32_t i;
      uint16_t *p = (uint16_t *)SDRAM_APP_BUF;
      uint16_t color;

      if (s_pic == 3)
      {
        color = CL_RED;
      }
      else if (s_pic == 4)
      {
        color = CL_GREEN;
      }
      else
      {
        color = CL_BLUE;
      }

      for (i = 0; i < g_LcdHeight * g_LcdWidth; i++)
      {
        *p++ = color;
      }
    }

    time1 = bsp_GetRunTime();
    if (LCD_GetWidth() == 480) /* 4.3寸屏 480x272 */
    {
      LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, p1);
    }
    else /* 5寸和7寸 800*480 屏 */
    {
      LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, p1);
    }
    time2 = bsp_GetRunTime();
    sprintf(buf, "图片%d SDRAM, 显示时间: %4dms", s_pic + 1, time2 - time1);
    LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);
  }
  else if (s_pic >= 6 && s_pic <= 8) /* SD卡中的图片3张 */
  {
    /* 访问Fatfs用到的全局变量 */
    FATFS fs;
    char FileName[64];
    uint8_t err = 0;
    char DiskPath[4]; /* 保存FatFS 磁盘路径 */

    /* 每次读1行 */
    FATFS_LinkDriver(&SD_Driver, DiskPath);

    if (f_mount(&fs, DiskPath, 0) == FR_OK)
    {
      time1 = bsp_GetRunTime();

      if (LCD_GetWidth() == 480) /* 4.3寸屏 480x272 */
      {
        sprintf(FileName, "%sPic/%s", DiskPath, g_NandFileName_480x272[s_pic - 6]);
      }
      else
      {
        sprintf(FileName, "%sPic/%s", DiskPath, g_NandFileName_800x480[s_pic - 6]);
      }
      if (ReadFileDispPic(FileName, 0, 0, g_LcdHeight, g_LcdWidth) == 0)
      {
        err = 1;
      }

      time2 = bsp_GetRunTime();
    }
    else
    {
      err = 1;
    }

    if (err == 1)
    {
      LCD_ClrScr(CL_BLUE);
      sprintf(buf, "图片%d SD卡文件错误: %s", s_pic + 1, FileName);
    }
    else
    {
      sprintf(buf, "图片%d SD卡, 显示时间: %dms", s_pic + 1, time2 - time1);
    }
    LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);

    /* 卸载文件系统 */
    f_mount(NULL, DiskPath, 0);

    FATFS_UnLinkDriver(DiskPath);
  }
  else if (s_pic >= 9 && s_pic <= 11) /* U盘中的图片3张 */
  {
    /* 访问Fatfs用到的全局变量 */
    FATFS fs;
    char FileName[64];
    uint8_t err = 0;
    char DiskPath[4]; /* 保存FatFS 磁盘路径 */

    /* 每次读1行 */
    FATFS_LinkDriver(&USBH_Driver, DiskPath);

    /* 每次读1行 */
    if (f_mount(&fs, DiskPath, 0) == FR_OK)
    {
      time1 = bsp_GetRunTime();

      if (LCD_GetWidth() == 480) /* 4.3寸屏 480x272 */
      {
        sprintf(FileName, "%sPic/%s", DiskPath, g_NandFileName_480x272[s_pic - 9]);
      }
      else
      {
        sprintf(FileName, "%sPic/%s", DiskPath, g_NandFileName_800x480[s_pic - 9]);
      }
      if (ReadFileDispPic(FileName, 0, 0, g_LcdHeight, g_LcdWidth) == 0)
      {
        err = 1;
      }

      time2 = bsp_GetRunTime();
    }
    else
    {
      printf("f_mount文件系统失败");
      err = 1;
    }

    if (err == 1)
    {
      LCD_ClrScr(CL_BLUE);
      sprintf(buf, "图片%d U盘文件错误:%s", s_pic + 1, FileName);
    }
    else
    {
      sprintf(buf, "图片%d U盘, 显示时间: %dms", s_pic + 1, time2 - time1);
    }
    LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);

    /* 卸载文件系统 */
    f_mount(NULL, DiskPath, 0);

    FATFS_UnLinkDriver(DiskPath);
  }
  else if (s_pic >= 12 && s_pic <= 14) /* NAND FALSH的图片3张 */
  {
    /* 访问Fatfs用到的全局变量 */
    FATFS fs;
    char FileName[64];
    uint8_t err = 0;
    char DiskPath[4]; /* 保存FatFS 磁盘路径 */

    /* 每次读1行 */
    FATFS_LinkDriver(&nand_Driver, DiskPath);

    /* 每次读1行 */
    if (f_mount(&fs, DiskPath, 0) == FR_OK)
    {
      time1 = bsp_GetRunTime();

      if (LCD_GetWidth() == 480) /* 4.3寸屏 480x272 */
      {
        sprintf(FileName, "%sPic/%s", DiskPath, g_NandFileName_480x272[s_pic - 12]);
      }
      else
      {
        sprintf(FileName, "%sPic/%s", DiskPath, g_NandFileName_800x480[s_pic - 12]);
      }
      if (ReadFileDispPic(FileName, 0, 0, g_LcdHeight, g_LcdWidth) == 0)
      {
        err = 1;
      }

      time2 = bsp_GetRunTime();
    }
    else
    {
      printf("f_mount文件系统失败");
      err = 1;
    }

    if (err == 1)
    {
      LCD_ClrScr(CL_BLUE);
      sprintf(buf, "图片%d NAND Flash文件错误:%s", s_pic + 1, FileName);
    }
    else
    {
      sprintf(buf, "图片%d NAND Flash, 显示时间: %dms", s_pic + 1, time2 - time1);
    }
    LCD_DispStrEx(5, 5, buf, &tFont, 300, ALIGN_LEFT);

    /* 卸载文件系统 */
    f_mount(NULL, DiskPath, 0);

    FATFS_UnLinkDriver(DiskPath);
  }
}

/*
*********************************************************************************************************
*	函 数 名: ReadFileDispPic
*	功能说明: 读图片文件，并显示。 在调用本函数前，请确保 磁盘已经执行 f_mount
*	形    参: _strFileName : 文件全名，含磁盘号和全路径
*	返 回 值: 0 表示错误， 1表示成功
*********************************************************************************************************
*/
uint8_t ReadFileDispPic(char *_strFileName, uint16_t _usX, uint16_t _usY, uint16_t usHeight, uint16_t usWidth)
{
  /* 访问Fatfs用到的全局变量 */
  //	FATFS   fs;
  FIL file;
  uint32_t bw;
  //	uint16_t data[800];
  uint16_t *pSDRAM = (uint16_t *)SDRAM_APP_BUF;

  if (usWidth > 800)
  {
    return 0;
  }

#if 1
  /* 打开文件 */
  {
    FRESULT result;

    //			uint16_t *p = (uint16_t *)(SDRAM_APP_BUF + g_LcdHeight * g_LcdWidth * 2);

    result = f_open(&file, _strFileName, FA_OPEN_EXISTING | FA_READ);
    if (result != FR_OK)
    {
      printf("Open file Error, %s\r\n", _strFileName);
      goto err_ret;
    }

    memset((uint8_t *)pSDRAM, 0x51, usWidth * usHeight * 2);

    f_lseek(&file, 0); /* 修改文件当前指针到文件头, 从头开始读。 可以不做，缺省就是0 */
    f_read(&file, pSDRAM, usWidth * usHeight * 2, &bw);

    if (bw != usWidth * usHeight * 2)
    {
      printf("Open file Error, %s\r\n", _strFileName);
      goto err_ret;
    }

    /* 关闭文件*/
    f_close(&file);

    LCD_DrawBMP(_usX, _usY, usHeight, usWidth, pSDRAM);
  }

#else
  /* 打开文件 */
  {
    FRESULT result;
    uint16_t i;

    result = f_open(&file, _strFileName, FA_OPEN_EXISTING | FA_READ);
    if (result != FR_OK)
    {
      printf("Open file Error, %s\r\n", _strFileName);
      goto err_ret;
    }

    // f_lseek(&file, 0);	/* 修改文件当前指针到文件头, 从头开始读。 可以不做，缺省就是0 */

    for (i = 0; i < usHeight; i++)
    {
      f_read(&file, &data, usWidth * 2, &bw);
      if (bw <= 0)
      {
        goto err_ret;
      }

      LCD_DrawBMP(_usX, _usY + i, 1, usWidth, data);
    }
  }
#endif

  /* 关闭文件*/
  f_close(&file);
  return 1; /* 返回OK */

err_ret:
  /* 关闭文件*/
  f_close(&file);
  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispKeyBoard
*	功能说明: 显示按键键名字
*	形    参: 无
*	返 回 值: 字符串指针
*********************************************************************************************************
*/
static const char *GetNameOfKey(uint32_t _code)
{
  uint16_t i = 0;

  while (1)
  {
    if (s_KeyNameTab[i].code == 0)
    {
      break;
    }

    if (_code == s_KeyNameTab[i].code)
    {
      return s_KeyNameTab[i].str;
    }
    i++;
  }

  return "";
}

/*
*********************************************************************************************************
*	函 数 名: InitFormTouch
*	功能说明: 显示按键键名字
*	形    参: 无
*	返 回 值: 字符串指针
*********************************************************************************************************
*/
static void InitFormTouch(void)
{
  FontLabel.FontCode = FC_ST_16;
  FontLabel.BackColor = CL_BLUE;
  FontLabel.FrontColor = CL_WHITE;
  FontLabel.Space = 0;

  Label3.Left = 5;
  Label3.Top = 110;
  Label3.MaxLen = 0;
  Label3.pCaption = "";
  Label3.Font = &FontLabel;

  Label1.Left = 5;
  Label1.Top = 150;
  Label1.MaxLen = 0;
  Label1.pCaption = "";
  Label1.Font = &FontLabel;

  Label2.Left = 5;
  Label2.Top = 170;
  Label2.MaxLen = 0;
  Label2.pCaption = "";
  Label2.Font = &FontLabel;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
