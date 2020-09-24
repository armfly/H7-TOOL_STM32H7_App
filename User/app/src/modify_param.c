/*
*********************************************************************************************************
*
*    模块名称 : 修改参数模块
*    文件名称 : modify_param.c
*    版    本 : V1.0
*    说    明 : 修改参数的公共函数
*    修改记录 :
*        版本号  日期       作者    说明
*        v1.0    2020-09-20 armfly  发布
*
*    Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "fonts.h"
#include "ui_def.h"
#include "main.h"
#include "lcd_menu.h"
#include "modify_param.h"

/* 多行文本框 */
#define MEMO_X     5
#define MEMO_Y     5 
#define MEMO_H     (240 - MEMO_Y - 0)  
#define MEMO_W     (240 - 2 * MEMO_X)

/* 菜单 */
#define MENU_ITEM_COUNT_MAX    20  /* 菜单项最大数量 */
char g_MenuBuf[20][32];

char *g_MenuParam_Text[MENU_ITEM_COUNT_MAX + 1];

MENU_T g_tMenuParam;

static void UartMonDispParam(void);
static void UartMonAdjustParam(uint8_t _index, int16_t _adj);

/*
*********************************************************************************************************
*    函 数 名: ModifyParam
*    功能说明: 系统设置状态. 菜单选择
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void ModifyParam(uint16_t _MainStatus)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    static uint8_t s_enter_sub_menu = 0;
    uint8_t fQuit = 0;
    uint8_t ucModifyStatus = 0;

    DispHeader2(90, "设置参数");
    
    /* 动态菜单项初始化指针 */
    {
        uint8_t i;
        
        for (i = 0; i < MENU_ITEM_COUNT_MAX; i++)
        {
            g_MenuParam_Text[i] = g_MenuBuf[i];
        }
    }

    if (_MainStatus == MS_MONITOR_UART)
    {
        UartMonDispParam();
    }
            
    if (s_enter_sub_menu == 0)
    {
        g_tMenuParam.Left = MENU_LEFT;
        g_tMenuParam.Top = MENU_TOP;
        g_tMenuParam.Height = MENU_HEIGHT;
        g_tMenuParam.Width = MENU_WIDTH;
        g_tMenuParam.LineCap = MENU_CAP;
        g_tMenuParam.ViewLine = 8;
        g_tMenuParam.Font.FontCode = FC_ST_24;
//        g_tMenuParam.Font.FrontColor = CL_BLACK;        /* 字体颜色 */
//        g_tMenuParam.Font.BackColor = FORM_BACK_COLOR;    /* 文字背景颜色 */
        g_tMenuParam.Font.Space = 0;
        g_tMenuParam.RollBackEn = 1;  /* 允许回滚 */        
        g_tMenuParam.GBK = 0;
        g_tMenuParam.ActiveBackColor = 0;   /* 选中行背景色ID */        
        LCD_InitMenu(&g_tMenuParam, (char **)g_MenuParam_Text); /* 初始化菜单结构 */
    }

    fRefresh = 1;
    while (fQuit == 0)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

            if (_MainStatus == MS_MONITOR_UART)
            {
                UartMonDispParam();
                LCD_DispMenu(&g_tMenuParam);
            }
        }
        
        bsp_Idle();
        
        if (ucModifyStatus == 0)
        {        
            ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
            if (ucKeyCode != KEY_NONE)
            {
                /* 有键按下 */
                switch (ucKeyCode)
                {
                    case KEY_UP_S: /* S键 上 */                   
                        LCD_MoveUpMenu(&g_tMenuParam);
                        break;

                    case KEY_LONG_DOWN_S: /* S键 上 */
                        PlayKeyTone();
                        ucModifyStatus = 1;
                    
                        g_tMenuParam.ActiveBackColor = 1;   /* 选中行背景色ID */   
                        fRefresh = 1;
                        break;

                    case KEY_LONG_UP_S:     /* 长按弹起 */                    
                        break;
                        
                    case KEY_UP_C: /* C键 下 */
                        LCD_MoveDownMenu(&g_tMenuParam);
                        break;

                    case KEY_LONG_DOWN_C: /* C键长按 */
                        PlayKeyTone();
                        s_enter_sub_menu = 0;
                        fQuit = 1;
                        break;

                    default:
                        break;
                }
            }
        }
        else    /* 修改状态 */
        {
            ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
            if (ucKeyCode != KEY_NONE)
            {
                /* 有键按下 */
                switch (ucKeyCode)
                {
                    case KEY_UP_S:          /* S键 上 */                   
                        UartMonAdjustParam(g_tMenuParam.Cursor, -1);
                        fRefresh = 1;
                        break;

                    case KEY_LONG_DOWN_S:   /* S键 上 */
                        break;

                    case KEY_LONG_UP_S:     /* 长按弹起 */                    
                        break;
                        
                    case KEY_UP_C:          /* C键 下 */
                        UartMonAdjustParam(g_tMenuParam.Cursor, 1);
                        fRefresh = 1;
                        break;

                    case KEY_LONG_DOWN_C:   /* C键长按 */
                        SaveParam();
                        g_tMenuParam.ActiveBackColor = 0;   /* 选中行背景色ID */  
                        ucModifyStatus = 0;
                        fRefresh = 1;
                        break;

                    default:
                        break;
                }
            }           
        }
    }
}

#define UART_MON_PARAM_COUTN  7
const char *UartMonParam0[] = {"1200", "2400", "4700", "9600", "19200", "38400", "57600", "115200"};
const char *UartMonParam1[] = {"无校验", "奇校验", "偶校验"};
const char *UartMonParam2[] = {"关闭", "启用"};
const char *UartMonParam3[] = {"12点阵", "16点阵"};
const char *UartMonParam4[] = {"关闭", "启用"};
const char *UartMonParam5[] = {"关闭", "启用"};
const char *UartMonParam6[] = {"无协议", "Modbus RTU"};
const PARAM_LIST_T UartMonParamList[UART_MON_PARAM_COUTN] = 
{ 
    /*  数据类型,      名称          可选列表,      最小值, 最大值, 缺省值 */
    {   0,         "波特率:",     UartMonParam0,   0,      7,     7},
    {   0,         "奇偶校验:",   UartMonParam1,   0,      2,     0},
    {   0,         "自动换行:",   UartMonParam2,   0,      1,     1},
    {   0,         "字体:",       UartMonParam3,   0,      1,     1},
    {   0,         "HEX显示:",    UartMonParam4,   0,      1,     0},
    {   0,         "时间戳:",     UartMonParam5,   0,      1,     0},
    {   0,         "协议:",       UartMonParam6,   0,      1,     0},    
};

/*
*********************************************************************************************************
*    函 数 名: MonDispReadParam
*    功能说明: 读参数
*    形    参: _index : 参数索引
*    返 回 值: 无
*********************************************************************************************************
*/
static int32_t MonDispReadParam(uint8_t _index)
{
    int32_t value = 0;
    
    if (_index == 0) value = g_tParam.UartMonBaud;
    if (_index == 1) value = g_tParam.UartMonParit;
    if (_index == 2) value = g_tParam.UartMonWordWrap;
    if (_index == 3) value = g_tParam.UartMonFont;
    if (_index == 4) value = g_tParam.UartMonHex;
    if (_index == 5) value = g_tParam.UartMonTimeStamp;
    if (_index == 6) value = g_tParam.UartMonProxy;
    
    return value;
}

/*
*********************************************************************************************************
*    函 数 名: MonDispWriteParam
*    功能说明: 修改参数
*    形    参: _index : 参数索引
*              _value : 参数值
*    返 回 值: 无
*********************************************************************************************************
*/
static void MonDispWriteParam(uint8_t _index, int32_t _value)
{   
    if (_index == 0)  g_tParam.UartMonBaud = _value;
    else if (_index == 1) g_tParam.UartMonParit = _value;
    else if (_index == 2) g_tParam.UartMonWordWrap = _value;
    else if (_index == 3) g_tParam.UartMonFont = _value;
    else if (_index == 4) g_tParam.UartMonHex = _value;
    else if (_index == 5) g_tParam.UartMonTimeStamp = _value;
    else if (_index == 6) g_tParam.UartMonProxy = _value;
}

/*
*********************************************************************************************************
*    函 数 名: UartMonDispParam
*    功能说明: 显示参数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void UartMonDispParam(void)
{
    uint8_t i;
    int32_t min, max, def;
    int32_t now;    
    
    for (i = 0; i < UART_MON_PARAM_COUTN; i++)
    {
        min = UartMonParamList[i].MinValue;
        max = UartMonParamList[i].MaxValue;
        def = UartMonParamList[i].DefaultValue;
        
        now = MonDispReadParam(i);
        
        if (now < min || now > max)
        {
            now = def;
        }

        sprintf(g_MenuBuf[i], "%s%s", UartMonParamList[i].ParamName, UartMonParamList[i].ParamItems[now]);
    }

    sprintf(g_MenuBuf[i], "&");     /* 结束符 */
}

/*
*********************************************************************************************************
*    函 数 名: UartMonAdjustParam
*    功能说明: 修改参数值
*    形    参: _index : 参数序号
*              _adj : 调节大小
*    返 回 值: 无
*********************************************************************************************************
*/
static void UartMonAdjustParam(uint8_t _index, int16_t _adj)
{
    int32_t min, max, def;
    int32_t OldValue, NewValue;
    
    if (_index >= UART_MON_PARAM_COUTN)
    {
        return;
    }
    
    min = UartMonParamList[_index].MinValue;
    max = UartMonParamList[_index].MaxValue;
    def = UartMonParamList[_index].DefaultValue;
    
    OldValue = MonDispReadParam(_index);
    
    if (OldValue < min || OldValue > max)
    {
        NewValue = def;
    }
    else
    {
        NewValue = OldValue + _adj;
        
        if (NewValue > max)
        {
            NewValue = min;
        }
        if (NewValue < min)
        {
            NewValue = max;
        }
    }
   
    MonDispWriteParam(_index, NewValue);
}

/*
*********************************************************************************************************
*    函 数 名: UartMonCheckParam
*    功能说明: 检查参数合法性并修正
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void UartMonCheckParam(void)
{
    uint8_t i;
    int32_t min, max, def;
    int32_t now;    
    
    for (i = 0; i < UART_MON_PARAM_COUTN; i++)
    {
        min = UartMonParamList[i].MinValue;
        max = UartMonParamList[i].MaxValue;
        def = UartMonParamList[i].DefaultValue;
        
        now = MonDispReadParam(i);
        
        if (now < min || now > max)
        {
            now = def;
        }

        MonDispWriteParam(i, now);
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
