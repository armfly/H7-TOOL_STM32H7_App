/*
*********************************************************************************************************
*
*    模块名称 : GUI配色模块
*    文件名称 : ui_def.h
*    版    本 : V1.0
*
*    Copyright (C), 2018-2025, 安富莱电子 www.armfly.com
*
********************************************************************************************************
*/

#ifndef __UI_DEF_H_
#define __UI_DEF_H_

/*
    RGB(32, 98, 230)    橙色
    
    RGB(5, 54, 131)     深蓝
    
    RGB(0, 186, 38)     粉绿
    
    RGB(0, 179, 254)    淡蓝
    
    RGB(215, 235, 118)  淡草绿
*/

#define UI_STYLE_NUM            3                   /* 界面风格个数 */

#define FORM_BACK_COLOR         RGB(20, 60, 60)     /* 背景颜色 */

#define HEAD_TEXT_COLOR         RGB(20, 20, 20)     /* 标题文字颜色 */
#define HEAD_BACK_COLOR         RGB(254, 124, 36)   /* 标题背景颜色 */

#define VALUE_TEXT_COLOR        RGB(60, 60, 60)     /* 测量值文字颜色 */
#define VALUE_BACK_COLOR        RGB(215, 235, 118)  /* 测量值背景颜色 */

#define HELP_TEXT_COLOR         RGB(150, 150, 150)  /* 帮助文字颜色 */
#define HELP_BACK_COLOR         FORM_BACK_COLOR     /* 帮助文字背景颜色 */

#define INFO_TEXT_COLOR         RGB(200, 200, 200)  /* 联机模式正文文字颜色 */
#define INFO_BACK_COLOR         FORM_BACK_COLOR     /* 联机模式正文文字背景颜色 */

#define CL_MENU_TEXT1           CL_WHITE            /* 菜单文字颜色 */
#define CL_MENU_BACK1           RGB(5, 54, 131)     /* 菜单文字背景颜色 */
#define CL_MENU_TEXT2           CL_BLACK            /* 光标选中文字颜色 */
#define CL_MENU_BACK2           CL_YELLOW           /* 光标选中文字背景颜色 */

#define SETTING_TIMEOUT         30                  /* 菜单按键超时 */
#define LCD_WAKE_UP_TIMEOUT     180                 /* LCD背光唤醒超时 3分钟 */

void FillTopBar(uint16_t _color);
void FillBottomBar(uint16_t _color);
void FillMidRect(uint16_t _color);
void DispStrInTopBar(char *_str);
void DispStrInBottomBar(char *_str);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
