/*
*********************************************************************************************************
*
*	模块名称 : GUI配色模块
*	文件名称 : gui_conf.h
*	版    本 : V1.0
*
*	Copyright (C), 2018-2025, 安富莱电子 www.armfly.com
*
********************************************************************************************************
*/

#ifndef _GUI_CONF_H_
#define _GUI_CONF_H_

#define FORM_BACK_COLOR CL_BLUE
#define HEAD_BAR_COLOR RGB(5, 54, 131)

#define SETTING_TIMEOUT 30      /* 菜单按键超时 */
#define LCD_WAKE_UP_TIMEOUT 180 /* LCD背光唤醒超时 3分钟 */

#define MAIN_BACK_COLOR RGB(5, 54, 131)
#define MAIN_TEXT_COLOR CL_WHITE

#define WIN_GRID_COLOR CL_WHITE /* 波形窗口栅格颜色 */

#define WIN_BACK_COLOR MAIN_BACK_COLOR /* 波形窗口背景颜色 */

#define WIN_WAVE_COLOR CL_YELLOW /* 波形线条颜色 */

#define SURGE_WIN_BACK_COLOR RGB(193, 227, 148) /* SURGE窗口背景色 */
#define SURGE_TITLE_TEXT_COLOR RGB(0, 0, 0)     /* SURGE标题文字颜色 */
#define SURGE_REC_TEXT1_COLOR RGB(0, 0, 0)      /* SURGE 记录第1行文字颜色 */
#define SURGE_REC_TEXT2_COLOR RGB(73, 65, 73)   /* SURGE 记录内容行文字颜色 */

#define ALARM_WIN_BACK_COLOR RGB(255, 189, 89) /* ALARM 窗口背景色 */
#define ALARM_TITLE_TEXT_COLOR RGB(0, 0, 0)    /* ALARM 标题文字颜色 */
#define ALARM_REC_TEXT1_COLOR RGB(0, 0, 0)     /* ALARM 记录第1行文字颜色 */
#define ALARM_REC_TEXT2_COLOR RGB(73, 65, 73)  /* ALARM 记录内容行文字颜色 */

#define CLOCK_WIN_BACK_COLOR RGB(224, 243, 199) /* 时钟窗口背景颜色 */
#define CLOCK_TEXT_COLOR RGB(73, 65, 73)        /* 时钟文字颜色 */

/* IO界面配色 */
#define IO_WIN_BACK_COLOR RGB(178, 191, 221)
#define IO_WIN_TEXT_COLOR RGB(51, 51, 51)
#define IO_DISABLE_TEXT_COLOR RGB(100, 100, 100) /* 通道无效的颜色 */

#define RES_WIN_BACK_COLOR RGB(252, 252, 252) /* 电阻仪联机状态窗口背景颜色 */
#define RES_TEXT_COLOR RGB(73, 65, 73)        /* 电阻仪联机状态窗口文字颜色 */
#define RES_ERR_TEXT_COLOR RGB(255, 0, 0)     /* 错误时文本颜色 */

#define DEL_REC_WIN_BACK_COLOR RGB(224, 243, 199) /* 删除记录窗口背景颜色 */
#define DEL_REC_TEXT_COLOR RGB(255, 0, 0)         /* 删除文本颜色 */

#define TRIGER_WIN_BACK_COLOR RGB(224, 243, 199) /* 触发电流窗口背景颜色 */
#define TRIGER_TEXT_COLOR RGB(73, 65, 73)        /* 触发电流文本颜色 */

#define BAUD_WIN_BACK_COLOR TRIGER_WIN_BACK_COLOR /* 波特率窗口背景 */
#define BAUD_TEXT_COLOR TRIGER_TEXT_COLOR         /* 波特率文字 */

#define ADDR_WIN_BACK_COLOR TRIGER_WIN_BACK_COLOR /* 地址窗口文字 */
#define ADDR_TEXT_COLOR TRIGER_TEXT_COLOR         /* 地址文字 */

#define TEMP_WIN_BACK_COLOR RGB(224, 243, 199) /* 温度窗口背景颜色 */
#define TEMP_TEXT_COLOR RGB(73, 65, 73)        /* 温度电流文本颜色 */

/* 菜单配色 */
#define MENU_WIN_BACK_COLOR IO_WIN_BACK_COLOR /* 菜单窗口背景色 */
#define MENU_TEXT_COLOR IO_WIN_TEXT_COLOR     /* 菜单文字颜色 */

#define TOP_BAR_BACK_COLOR MAIN_BACK_COLOR /* 底部BAR背景颜色 */
#define TOP_BAR_TEXT_COLOR MAIN_TEXT_COLOR /* 底部BAR文字颜色 */

#define BOTTOM_BAR_BACK_COLOR MAIN_BACK_COLOR /* 底部BAR背景颜色 */
#define BOTTOM_BAR_TEXT_COLOR MAIN_TEXT_COLOR /* 底部BAR文字颜色 */

/* 定义波形窗口的大小和偏移位置 */
#define DOT_Y 4 /* 每小点间隔5像素 */
#define DOT_X 4 /* 每小点间隔10像素 */

#define GRID_Y (DOT_Y * 10) /* Y轴，大栅格25像素，内分5个小点 */
#define GRID_X (DOT_X * 10) /* X轴，大栅格50像素, 内分5个小点 */

#define WIN_LEFT 26
#define WIN_TOP 18

#define WIN_WIDTH (GRID_X * 8)
#define WIN_HEIGHT (GRID_Y * 6)

/*　通道窗口- 用于清屏 */
#define IO_RECT1_LEFT 0
#define IO_RECT1_TOP 0
#define IO_RECT1_WIDTH WIN_WIDTH + WIN_LEFT + 1
#define IO_RECT1_HEIGHT WIN_HEIGHT + WIN_TOP + 32

/*　通道窗口- 用于清屏 */
#define IO_RECT2_LEFT IO_RECT1_WIDTH
#define IO_RECT2_TOP TRIGER_TOP + TRIGER_HEIGHT + 1
#define IO_RECT2_WIDTH TRIGER_WIDTH
#define IO_RECT2_HEIGHT 40

/*　通道窗口- 用于显示信息区 */
#define IO_WIN_LEFT 5
#define IO_WIN_TOP (WIN_TOP - 1)
#define IO_WIN_WIDTH (SURGE_LEFT - 10)
#define IO_WIN_HEIGHT 225

#define IO_TEXT_LEFT (IO_WIN_LEFT + 10)
#define IO_TEXT_TOP (IO_WIN_TOP + 10)

#define IO_TEXT_LEFT2 (IO_WIN_LEFT + 5)
#define IO_TEXT_LEFT3 (IO_WIN_LEFT + (IO_WIN_WIDTH - 10) / 2)

#define IO_TEXT_Y_CAP 24

/* 接地电阻 */
#define RES_VALUE_LEFT 5
#define RES_VALUE_TOP 252

#define RES_INFO_LEFT RES_VALUE_LEFT
#define RES_INFO_TOP 272

/* 定义SURGE 窗口的大小和偏移位置 */
#define SURGE_LEFT (WIN_LEFT + WIN_WIDTH + 5)
#define SURGE_TOP (WIN_TOP - 1)
#define SURGE_WIDTH 125
#define SURGE_HEIGHT 134

#define SURGE_LINE_CAP1 15
#define SURGE_LINE_CAP2 14

/* 定义 ALARM窗口的大小和偏移位置 */
#define ALARM_LEFT SURGE_LEFT
#define ALARM_TOP (SURGE_TOP + SURGE_HEIGHT + 4)
#define ALARM_WIDTH SURGE_WIDTH
#define ALARM_HEIGHT 74

#define TRIGER_LEFT 370
#define TRIGER_TOP 250
#define TRIGER_WIDTH 105
#define TRIGER_HEIGHT 20

#define ADDR_LEFT TRIGER_LEFT
#define ADDR_TOP (TRIGER_TOP + 23)
#define ADDR_WIDTH TRIGER_WIDTH
#define ADDR_HEIGHT 20

#define BAUD_LEFT TRIGER_LEFT
#define BAUD_TOP (TRIGER_TOP + 23 * 2)
#define BAUD_WIDTH TRIGER_WIDTH
#define BAUD_HEIGHT 20

#define CLOCK_LEFT 5
#define CLOCK_TOP (320 - 24)
#define CLOCK_WIDTH 160
#define CLOCK_HEIGHT 20

#define RESLINK_LEFT 5
#define RESLINK_TOP (IO_WIN_TOP + IO_WIN_HEIGHT + 5)
#define RESLINK_WIDTH 160
#define RESLINK_HEIGHT 20

#define TMODE_LEFT (CLOCK_LEFT + CLOCK_WIDTH + 10)
#define TMODE_TOP CLOCK_TOP
#define TMODE_WIDTH 20
#define TMODE_HEIGHT 20

#define WMODE_LEFT (TMODE_LEFT + TMODE_WIDTH + 10)
#define WMODE_TOP CLOCK_TOP
#define WMODE_WIDTH 20
#define WMODE_HEIGHT 20

// -125.1℃
#define TEMP_LEFT (WMODE_LEFT + WMODE_WIDTH + 20)
#define TEMP_TOP (CLOCK_TOP + 3)
#define TEMP_WIDTH 64
#define TEMP_HEIGHT 16

#define TOP_BAR_LEFT 0
#define TOP_BAR_TOP 0
#define TOP_BAR_HEIGHT 24
#define TOP_BAR_WIDTH 480

#define BOTTOM_BAR_LEFT 0
#define BOTTOM_BAR_TOP (320 - BOTTOM_BAR_HEIGHT)
#define BOTTOM_BAR_HEIGHT 24
#define BOTTOM_BAR_WIDTH 480

/* 系统参数，右边参数区 */
#define SYS_BOX_LEFT 150
#define SYS_BOX_TOP TOP_BAR_HEIGHT
#define SYS_BOX_HEIGHT (320 - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT)
#define SYS_BOX_WIDTH (480 - SYS_BOX_LEFT)
#define SYS_BOX_BACK_COLOR IO_WIN_BACK_COLOR

void FillTopBar(uint16_t _color);
void FillBottomBar(uint16_t _color);
void FillMidRect(uint16_t _color);
void DispStrInTopBar(char *_str);
void DispStrInBottomBar(char *_str);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
