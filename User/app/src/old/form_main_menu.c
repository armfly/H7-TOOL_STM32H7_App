/*
*********************************************************************************************************
*
*	模块名称 : 主界面模块。
*	文件名称 : main_menu.c
*	版    本 : V1.0
*	说    明 :
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2012-08-08 armfly  ST固件库V3.5.0版本。
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"    /* printf函数定向输出到串口，所以必须包含这个文件 */
#include "images.h" /* 图标文件 */
#include "form_main_menu.h"
#include "main.h"

#define KEY_INFO "K3:校准 K1:背景 OK:显示方向"

#define ICON_LEFT 16
#define ICON_TOP 25

#define TOP_BAR_H 20           /* 顶部栏高度 */
#define BOTTOM_BAR_H 20        /* 底部栏高度 */
#define CL_BAR RGB(0, 73, 108) /* 顶部栏和底部栏背景颜色 */

extern const unsigned char acRecorder[48 * 48 * 4];
extern const unsigned char acTape[48 * 48 * 4];
extern const unsigned char acCamera[48 * 48 * 4];
extern const unsigned char acChiLun[48 * 48 * 4];
extern const unsigned char acchujiao[48 * 48 * 4];

#define ICONS_COUNT 28
static ICON_T s_tMainIcons[ICONS_COUNT] =
    {
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)achwinfo, "硬件信息"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acLCD, "触摸屏和按键"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acRadio, "收音机"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acRecorder, "录音机"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acchujiao, "MEMS传感器"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acweb, "WEB服务器"},

        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acGPS, "GPS定位模块"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acPhone, "GPRS模块"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acMP3, "MP3模块"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acUSBDisk, "模拟U盘"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acCamera, "摄像头"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "OLED显示器"},

        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acADC, "AD7606模块"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acADC, "AD7705模块"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acADC, "ADS1256模块"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "RS485数码管"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "DAC8501"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "DAC8562"},

        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "预留"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "示波器"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "串口WiFi"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "CAN网络"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "AD9833模块"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "预留4"},

        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "预留5"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "预留6"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "预留7"},
        {ID_ICON, 0, 0, ICON_HEIGHT, ICON_WIDTH, (uint16_t *)acChiLun, "预留8"},

};

/* 图标点击后，切换程序状态 */
static const uint8_t s_RetStatus[ICONS_COUNT] =
    {
        MS_HARDWARE_INFO, /* 硬件信息 */
        MS_TEST_TOUCH,    /* 测试触摸屏 */
        MS_RADIO,         /* 收音机 */
        MS_WM8978_REC,    /* 录音机 */
        MS_MEMS,          /* MEMS 传感器测试（I2C） 三轴陀螺仪，磁力计，气压计 */
        MS_ETH_WEB,       /* 以太网DM9000和DM9161 WEB服务器 */

        MS_GPS,          /* GPS测试 */
        MS_GPRS,         /* GPRS模块测试 */
        MS_MP3,          /* 校准触摸状态 */
        MS_MASS_STORAGE, /* SD卡，NAND模拟U盘 */
        MS_CAMERA,       /* 摄像头 */
        MS_OLED,         /* OLED显示器 */

        MS_AD7606,  /* 测试 AD7606 */
        MS_AD7705,  /* AD7705模块 */
        MS_ADS1256, /* ADS1256模块 */
        MS_RS485,   /* RS485通信 */
        MS_DAC8501, /* DAC8501 模块 */
        MS_DAC8562, /* DAC8562 模块 */

        MS_RA8875, /* RA8875字库 */
        MS_DSO,    /* 示波器 */
        MS_WIFI,   /* 串口WIFI */
        MS_CAN,    /* CAN网络测试 */
        MS_AD9833, /* AD9833模块 */
        MS_RESERVED,

        MS_RESERVED,
        MS_RESERVED,
        MS_RESERVED,
        MS_RESERVED};

static uint8_t s_IconsPageNo = 0; /* 图标页面, 一屏显示不下时，自动排列到第2屏 */
static uint8_t s_IconsPageMax;    /* 图标页面个数 */
static uint8_t s_IconNumPerPage;  /* 每页面最多包含几个图片 */

static int16_t s_IconsCursor = 0; /* 选中的图标， -1表示无选中 */
static uint16_t s_down_x;         /* 触笔按下时的x坐标 ; 用于识别滑动方向 */
static uint16_t s_down_y;         /* 触笔按下时的y坐标 ; 用于识别滑动方向 */

#define STYLE_COUNT 7 /* 颜色方案个数 */

/* 主界面风格 */
typedef struct
{
  uint16_t *pBmp;     /* 背景图片指针，如果是0, 则取单色背景参数 BackColor */
  uint16_t BackColor; /* 背景颜色 */
  uint16_t TextColor; /* 图标文本的颜色 */
} STYLE_T;

/*
  背景图片（RGB565格式，存放在NOR Flash
  NOR Flash 基地址 0x6400 0000

  0x0020 0000   255K字节 480x272_海滩.bin  + 0x3FC00
  0x0023 FC00   255K字节 480x272_树叶.bin
  0x0027 F800   255K字节 480x272_水珠.bin

  0x002B F400   755K字节 800x480_海滩.bin  + 0xBB800
  0x0037 AC00   755K字节 800x480_树叶.bin
  0x0043 6400   755K字节 800x480_水珠.bin
*/

static const STYLE_T s_UI480[STYLE_COUNT] =
    {
        /* 背景图片,		背景颜色,			图标文字颜色 */
        {0, CL_WHITE, CL_BLACK},           /* 白色背景 */
        {0, RGB(0, 78, 152), CL_WHITE},    /* XP桌面蓝背景 */
        {0, RGB(185, 193, 204), CL_BLACK}, /* 灰色背景 */
        {0, RGB(38, 38, 38), CL_WHITE},    /* 深黑色背景 */

        {(uint16_t *)(0x64000000 + 0x00200000), 0, CL_BLACK}, /* 480x272 沙滩背景 */
        {(uint16_t *)(0x64000000 + 0x0023FC00), 0, CL_WHITE}, /* 480x272 树叶背景 */
        {(uint16_t *)(0x64000000 + 0x0027F800), 0, CL_BLACK}, /* 480x272 水珠背景 */
};

static const STYLE_T s_UI800[STYLE_COUNT] =
    {
        /* 背景图片,		背景颜色,			图标文字颜色 */
        {0, CL_WHITE, CL_BLACK},           /* 白色背景 */
        {0, RGB(0, 78, 152), CL_WHITE},    /* XP桌面蓝背景 */
        {0, RGB(185, 193, 204), CL_BLACK}, /* 灰色背景 */
        {0, RGB(38, 38, 38), CL_WHITE},    /* 深黑色背景 */

        {(uint16_t *)(0x64000000 + 0x002BF400), 0, CL_BLACK}, /* 800x480 沙滩背景 */
        {(uint16_t *)(0x64000000 + 0x0037AC00), 0, CL_WHITE}, /* 800x480 树叶背景 */
        {(uint16_t *)(0x64000000 + 0x00436400), 0, CL_BLACK}, /* 800x480 水珠背景 */
};

static uint16_t s_ColorIndex = 0; /* 当前选中的颜色方案序号 */

/* 图标文字颜色 */
#define MAIN_TEXT_COLOR ((g_LcdWidth == 480) ? s_UI480[s_ColorIndex].TextColor : s_UI800[s_ColorIndex].TextColor)

static void DispBackground(void);
static void ArrayIcon(void);
static void DispTopBar(void);
static void DispBottomBar(void);

/*
*********************************************************************************************************
*	函 数 名: MainMenu
*	功能说明: 主界面
*	形    参：无
*	返 回 值: 状态字
*********************************************************************************************************
*/
uint8_t MainMenu(void)
{
#if 0	
  uint8_t ucKeyCode;		/* 按键代码 */
  uint8_t ucTouch;		/* 触摸事件 */
  uint8_t fRefresh;		/* 刷屏请求标志,1表示需要刷新 */
  FONT_T tIconFont;		/* 定义一个字体结构体变量，用于图标文本 */

  int16_t tpX, tpY;
  uint16_t i;

  DispTopBar();		/* 显示顶部栏 */
  DispBottomBar();	/* 显示底部栏 */
  DispBackground();	/* 显示图标背景 */

  /* 设置字体参数 */
  {
    tIconFont.FontCode = FC_ST_12;			/* 字体代码 16点阵 */
    tIconFont.FrontColor = MAIN_TEXT_COLOR;	/* 字体颜色 */
    tIconFont.BackColor = CL_MASK;			/* 文字背景颜色 */
    tIconFont.Space = 1;					/* 文字间距，单位 = 像素 */
  }

  //s_IconsPageNo = 0;	/* 图标页面号，不要清零。 */

  fRefresh = 1;	/* 1表示需要刷新LCD */
  while (g_MainStatus == MS_MAIN_MENU)
  {
    bsp_Idle();

    if (fRefresh)
    {
      fRefresh = 0;

      /* 显示图标阵列 */
      ArrayIcon();	/* 排列图标 */
    }

    ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
    if (ucTouch != TOUCH_NONE)
    {
      switch (ucTouch)
      {
        case TOUCH_DOWN:		/* 触笔按下事件 */
          /* 绘制图标阵列 */
          {
            s_IconsCursor = -1;	/* -1 表示当前没有图标被激活点亮 */

            for (i = s_IconNumPerPage * s_IconsPageNo;
              i < s_IconNumPerPage * s_IconsPageNo + s_IconNumPerPage && i < ICONS_COUNT;
              i++)
            {
              if (TOUCH_InRect(tpX, tpY, s_tMainIcons[i].Left, s_tMainIcons[i].Top,
                 s_tMainIcons[i].Height, s_tMainIcons[i].Width))
              {
                s_IconsCursor = i;
                LCD_DrawIcon32(&s_tMainIcons[i], &tIconFont, 1);	/* 0 表示正常显示， 1表示选中 */
              }
            }
            s_down_x = tpX;
            s_down_y = tpY;
          }
          break;

        case TOUCH_MOVE:		/* 触笔移动事件 */
          if (s_IconsCursor >= 0)
          {
            if (TOUCH_InRect(tpX, tpY, s_tMainIcons[s_IconsCursor].Left, s_tMainIcons[s_IconsCursor].Top,
                 s_tMainIcons[s_IconsCursor].Height, s_tMainIcons[s_IconsCursor].Width))
            {
              ;
            }
            else
            {
              LCD_DrawIcon32(&s_tMainIcons[s_IconsCursor], &tIconFont, 0);	/* 0 表示正常显示， 1表示选中 */
              s_IconsCursor = -1;
            }
          }

          if (s_down_x > 0)
          {
            if (tpX - s_down_x > 50)
            {
              /* 向右滑动 */
              if (s_IconsPageNo > 0)
              {
                s_IconsPageNo--;
                DispBackground();
                ArrayIcon();	/* 排列图标 */
              }
              s_down_x = 0;
            }
            else if (tpX - s_down_x < -50)
            {
              /* 向左滑动 */
              if (s_IconsPageNo < s_IconsPageMax - 1)
              {
                s_IconsPageNo++;
                DispBackground();
                ArrayIcon();	/* 排列图标 */
              }
              s_down_x = 0;
            }

          }
          break;

        case TOUCH_RELEASE:		/* 触笔释放事件 */
          if (s_IconsCursor >= 0)
          {
            /* 按下时的坐标和弹起时坐标都在图标内才算有效点击 */
            if (TOUCH_InRect(tpX, tpY, s_tMainIcons[s_IconsCursor].Left, s_tMainIcons[s_IconsCursor].Top,
                 s_tMainIcons[s_IconsCursor].Height, s_tMainIcons[s_IconsCursor].Width)
              && TOUCH_InRect(s_down_x, s_down_y, s_tMainIcons[s_IconsCursor].Left, s_tMainIcons[s_IconsCursor].Top,
                 s_tMainIcons[s_IconsCursor].Height, s_tMainIcons[s_IconsCursor].Width))
            {
              g_MainStatus = s_RetStatus[s_IconsCursor];	/* 返回程序状态 */
            }
            else
            {
              LCD_DrawIcon32(&s_tMainIcons[s_IconsCursor], &tIconFont, 0);	/* 0 表示正常显示， 1表示选中 */
              s_IconsCursor = -1;
            }
          }
          break;

        default:
          break;
      }
    }

    ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
    if (ucKeyCode != KEY_NONE)
    {
      /* 有键按下 */
      switch (ucKeyCode)
      {
        case  JOY_DOWN_L:	/* 遥杆右键 2014-08-22 */
          return MS_RA8875;	/* 进入RA8875界面 */

        case  JOY_DOWN_U:	/* 遥杆上键 2014-08-22 */
          return MS_TEST_TOUCH;	/* 进入触摸界面 */

        case  JOY_DOWN_OK:	/* 遥感OK键 */
          if (++g_LcdDirection > 3)
          {
            g_LcdDirection = 0;
          }
          /* 显示方向代码 0 横屏正常, 1=横屏180度翻转, 2=竖屏, 3=竖屏180度翻转 */
          LCD_SetDirection(g_LcdDirection);

          DispTopBar();		/* 显示顶部栏 */
          DispBottomBar();	/* 显示底部栏 */
          DispBackground();	/* 显示背景 */
          fRefresh = 1;
          break;

        case  KEY_DOWN_K3:	/* K3键 */
          return MS_CALIBRATION;	/* 进入触摸界面 */
          //break;

        case  KEY_DOWN_K1:	/* K1键 */
          if (++s_ColorIndex >= STYLE_COUNT)
          {
            s_ColorIndex = 0;
          }
          /* 重新绘制背景 */
          {
            DispBackground();	/* 显示背景 */

            /* 设置字体参数 */
            {
              tIconFont.FontCode = FC_ST_12;		/* 字体代码 16点阵 */
              tIconFont.FrontColor = MAIN_TEXT_COLOR;	/* 字体颜色 */
              tIconFont.BackColor = CL_MASK;		/* 文字背景颜色 */
              tIconFont.Space = 1;				/* 文字间距，单位 = 像素 */
            }
          }
          fRefresh = 1;
          break;

        default:
          break;
      }
    }
  }
#endif
  return 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispTopBar
*	功能说明: 显示抬头栏（标题栏）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispTopBar(void)
{
  FONT_T tFont; /* 定义一个字体结构体变量，用于设置字体参数 */

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;   /* 字体代码 16点阵 */
    tFont.FrontColor = CL_WHITE; /* 字体颜色 */
    tFont.BackColor = CL_BAR;    /* 文字背景颜色 */
    tFont.Space = 0;             /* 文字间距，单位 = 像素 */
  }
  LCD_Fill_Rect(0, 0, TOP_BAR_H, g_LcdWidth, CL_BAR);
  LCD_DispStr(5, 2, VER_INFO, &tFont); /* 显示软件版本信息 */
}

/*
*********************************************************************************************************
*	函 数 名: DispBottomBar
*	功能说明: 显示底部栏
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispBottomBar(void)
{
  FONT_T tFont; /* 定义一个字体结构体变量，用于设置字体参数 */

  /* 设置字体参数 */
  {
    tFont.FontCode = FC_ST_16;   /* 字体代码 16点阵 */
    tFont.FrontColor = CL_WHITE; /* 字体颜色 */
    tFont.BackColor = CL_BAR;    /* 文字背景颜色 */
    tFont.Space = 0;             /* 文字间距，单位 = 像素 */
  }
  LCD_Fill_Rect(0, g_LcdHeight - BOTTOM_BAR_H, BOTTOM_BAR_H, g_LcdWidth, CL_BAR);
  LCD_DispStr(5, g_LcdHeight - 18, KEY_INFO, &tFont); /* 显示按键操作提示 */
}

/*
*********************************************************************************************************
*	函 数 名: DispBackground
*	功能说明: 显示界面背景
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispBackground(void)
{
#if 1
  if ((g_LcdWidth == 480) && (g_LcdHeight == 272)) /* 480 x 272 */
  {
    if (s_UI480[s_ColorIndex].pBmp == 0)
    {
      //LCD_ClrScr(s_UI480[s_ColorIndex].BackColor);  	/* 清屏, 填充单色 */
      LCD_Fill_Rect(0, TOP_BAR_H, g_LcdHeight - TOP_BAR_H - BOTTOM_BAR_H, g_LcdWidth, s_UI480[s_ColorIndex].BackColor);
    }
    else
    {
      LCD_DrawBMP(0, TOP_BAR_H, g_LcdHeight - TOP_BAR_H - BOTTOM_BAR_H, g_LcdWidth, s_UI480[s_ColorIndex].pBmp);
    }
  }
  else if ((g_LcdWidth == 480) && (g_LcdHeight == 320)) /* 480 x 320 */
  {
    if (s_UI480[s_ColorIndex].pBmp == 0)
    {
      //LCD_ClrScr(s_UI800[s_ColorIndex].BackColor);  	/* 清屏, 填充单色 */
      LCD_Fill_Rect(0, TOP_BAR_H, g_LcdHeight - TOP_BAR_H - BOTTOM_BAR_H, g_LcdWidth, s_UI800[s_ColorIndex].BackColor);
    }
    else
    {
      LCD_DrawBMP(0, TOP_BAR_H, g_LcdHeight - TOP_BAR_H - BOTTOM_BAR_H, g_LcdWidth, s_UI800[s_ColorIndex].pBmp);
    }
  }
  else /* 800 x 480 */
  {
    if (s_UI800[s_ColorIndex].pBmp == 0)
    {
      //LCD_ClrScr(s_UI800[s_ColorIndex].BackColor);  	/* 清屏, 填充单色 */
      LCD_Fill_Rect(0, TOP_BAR_H, g_LcdHeight - TOP_BAR_H - BOTTOM_BAR_H, g_LcdWidth, s_UI800[s_ColorIndex].BackColor);
    }
    else
    {
      LCD_DrawBMP(0, TOP_BAR_H, g_LcdHeight - TOP_BAR_H - BOTTOM_BAR_H, g_LcdWidth, s_UI800[s_ColorIndex].pBmp);
    }
  }
#else
  if ((g_LcdWidth == 480) && (g_LcdHeight == 272)) /* 480 x 272 */
  {
    if (s_UI480[s_ColorIndex].pBmp == 0)
    {
      LCD_ClrScr(s_UI480[s_ColorIndex].BackColor); /* 清屏, 填充单色 */
    }
    else
    {
      LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, s_UI480[s_ColorIndex].pBmp);
    }
  }
  else if ((g_LcdWidth == 480) && (g_LcdHeight == 320)) /* 480 x 320 */
  {
    if (s_UI480[s_ColorIndex].pBmp == 0)
    {
      LCD_ClrScr(s_UI800[s_ColorIndex].BackColor); /* 清屏, 填充单色 */
    }
    else
    {
      LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, s_UI800[s_ColorIndex].pBmp);
    }
  }
  else /* 800 x 480 */
  {
    if (s_UI800[s_ColorIndex].pBmp == 0)
    {
      LCD_ClrScr(s_UI800[s_ColorIndex].BackColor); /* 清屏, 填充单色 */
    }
    else
    {
      LCD_DrawBMP(0, 0, g_LcdHeight, g_LcdWidth, s_UI800[s_ColorIndex].pBmp);
    }
  }
#endif
}

/*
*********************************************************************************************************
*	函 数 名: ArrayIcon
*	功能说明: 排列图标 s_tMainIcons 的坐标， 全局变量 s_IconsPageNo 表示页号
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ArrayIcon(void)
{
  uint16_t x;
  uint16_t y;
  uint8_t h_count;     /* 水平方向图标排列个数 */
  uint8_t v_count;     /* 垂直方向图标排列个数 */
  uint16_t icon_index; /* 图标索引 */
  uint16_t icons_left; /* 图标阵列 X 坐标 */
  uint16_t icons_top;  /* 图标阵列 Y 坐标 */
  uint8_t m, n;
  FONT_T tIconFont;

  tIconFont.FontCode = FC_ST_12;          /* 字体代码 16点阵 */
  tIconFont.FrontColor = MAIN_TEXT_COLOR; /* 字体颜色 */
  tIconFont.BackColor = CL_MASK;          /* 文字背景颜色 */
  tIconFont.Space = 1;                    /* 文字间距，单位 = 像素 */

  h_count = g_LcdWidth / ICON_STEP_X;
  v_count = (g_LcdHeight - TOP_BAR_H - BOTTOM_BAR_H) / ICON_STEP_Y;

  s_IconNumPerPage = h_count * v_count; /* 每个页面包含的图标个数 */

  /* 计算需要几个图片页面 */
  s_IconsPageMax = (ICONS_COUNT - 1) / (h_count * v_count) + 1;

  icons_left = (g_LcdWidth - (h_count - 1) * ICON_STEP_X - ICON_WIDTH) / 2;
  icons_top = ICON_TOP;

  icon_index = s_IconsPageNo * h_count * v_count;

  x = icons_left;
  y = icons_top;
  for (m = 0; m < v_count; m++)
  {
    x = icons_left;
    y = icons_top + m * ICON_STEP_Y;
    for (n = 0; n < h_count; n++)
    {
      s_tMainIcons[icon_index].Left = x;
      s_tMainIcons[icon_index].Top = y;

      LCD_DrawIcon32(&s_tMainIcons[icon_index], &tIconFont, 0); /* 0 表示正常显示， 1表示选中 */

      icon_index++;
      if (icon_index >= ICONS_COUNT)
      {
        break;
      }
      x += ICON_STEP_X;
    }
    if (icon_index >= ICONS_COUNT)
    {
      break;
    }
  }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
