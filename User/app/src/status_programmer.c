/*
*********************************************************************************************************
*
*    模块名称 : 脱机编程器
*    文件名称 : status_programmer.c
*    版    本 : V1.0
*    说    明 : 脱机编程器
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2019-10-06 armfly  正式发布
*
*    Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"

#define GROUP_NUM      20      /* 最大分组个数 */
#define FILE_NUM       50      /* 每个分组下面最大文件个数 */

/*
    界面规划:
*/

/* 三个按钮 */
#define BTN1_X     (240 - BTN1_W - 5)
#define BTN1_Y     80
#define BTN1_H     28 
#define BTN1_W     80 
#define BTN1_TEXT  "选择文件"

#define BTN2_X     BTN1_X
#define BTN2_Y     BTN1_Y + BTN1_H + 5 
#define BTN2_H     BTN1_H 
#define BTN2_W     BTN1_W 
#define BTN2_TEXT  "开始烧录"

#define BTN3_X     BTN1_X
#define BTN3_Y     BTN1_Y + 2 * (BTN1_H + 5)
#define BTN3_H     BTN1_H 
#define BTN3_W     BTN1_W 
#define BTN3_TEXT  "输出电压"

/* CPU型号 */
#define TEXT1_X    5
#define TEXT1_Y    BTN1_Y

/* 文件大小 */
#define TEXT2_X    TEXT1_X
#define TEXT2_Y    TEXT1_Y + 20

/* 烧写次数 */
#define TEXT3_X    TEXT1_X
#define TEXT3_Y    TEXT2_Y + 20

/* TVCC电压 */
#define TEXT4_X    TEXT1_X
#define TEXT4_Y    TEXT3_Y + 20

/* 供电电流 */
#define TEXT5_X    TEXT1_X
#define TEXT5_Y    TEXT4_Y + 20

/* 烧录时间 */
#define PROG_TIME_X     160
#define PROG_TIME_Y     240 - 50

/* 烧录步骤提示 */
#define PROG_STEP_X     5
#define PROG_STEP_Y     240 - 50

/* 烧录进度条 */
#define PROGRESS_X      5
#define PROGRESS_Y      240 - 30

enum
{
	PS_MAIN = 0,
	PS_GROUP,
	PS_FILE,
	PS_TVCC,	
	
	PS_EXIT,
};

typedef struct 
{
    uint32_t Time;
    uint8_t Step;
    uint8_t Progress;  
    
    uint32_t BeginAddr;
    uint32_t SectorCount;
    uint32_t SectorIdx;

    uint32_t PageCount;
    uint32_t PageIdx;    
}OFFLINE_PROG_T;

OFFLINE_PROG_T g_tProg;
static uint8_t s_ProgStatus = PS_MAIN;    /* 编程器子状态 */

static void ProgStatusMain(void);
static void ProgStatusGroup(void);
static void ProgStatusFile(void);
static void ProgStatusTvcc(void);

static void DispGroupList(uint16_t _usIndex);
static void DispFileList(uint16_t _usIndex);

BUTTON_T btn1, btn2, btn3;

/*
*********************************************************************************************************
*    函 数 名: status_ProgInit
*    功能说明: 脱机编程器预览界面 
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_ProgInit(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;

    DispHeader("烧录器");
    DispHelpBar("长按S键开始烧录",
                "");      
    fRefresh = 1;
    while (g_MainStatus == MS_PROG_INIT)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;
        }

        bsp_Idle();
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_DOWN_S:    /* S键按下 */
                break;

            case KEY_UP_S:      /* S键释放 */
                g_MainStatus = NextStatus(g_MainStatus);
                break;

            case KEY_LONG_DOWN_S:    /* S键长按 */
                g_MainStatus = MS_PROG_WORK;
                break;

            case KEY_DOWN_C:    /* C键按下 */
                break;

            case KEY_UP_C:      /* C键释放 */
                g_MainStatus = LastStatus(g_MainStatus);
                break;

            case KEY_LONG_DOWN_C:    /* C键长按 */
                break;

            default:
                break;
            }
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: status_ProgWork
*    功能说明: 脱机编程器烧录状态
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_ProgWork(void)
{
    s_ProgStatus = 0;
    while (g_MainStatus == MS_PROG_WORK)
    {
        switch (s_ProgStatus)
        {
        case PS_MAIN: 
            ProgStatusMain();      	/* 烧录器主界面 */
            break;

        case PS_GROUP:
            ProgStatusGroup();   	/* 选择分组 */
            break;

        case PS_FILE:
            ProgStatusFile();      	/* 选择文件 */
            break;		

        case PS_TVCC:
            ProgStatusTvcc();		/* 选择输出电压 */
            break;
        }
    }
}


// 烧录demo演示
uint8_t PG_DetectIC(void)
{
    bsp_DelayMS(500);
    return 1;
}

uint8_t PG_EraseSector(uint32_t _Addr)
{
    bsp_DelayMS(500);
    return 1;
}

uint8_t PG_ProgPage(uint32_t _Addr)
{
    bsp_DelayMS(20);
    return 1;
}

uint8_t PG_VerifyData(uint32_t _Addr)
{
    bsp_DelayMS(10);
    return 1;
}
enum
{
    PG_STEP_IDLE = 0,
    PG_STEP_DETECT,
    PG_STEP_ERASE,
    PG_STEP_PROG,
    PG_STEP_VERIFY,
    
    PG_STEP_DETECT_ERR,
    PG_STEP_ERASE_ERR,
    PG_STEP_PROG_ERR,
    PG_STEP_VERIFY_ERR,
    
    PG_STEP_SUCCESS,
};

#define SECTOR_COUNT_DEMO   10
#define PAGE_COUNT_DEMO     200

// 烧录进度demo演示
void ProgTask(uint8_t _cmd)
{
    FONT_T tFont16;
    static uint8_t s_state = 0;
    static uint8_t fRefresh = 0;
    static uint8_t s_retry = 0;
    const char *strStep[] = 
    {
        "空闲",
        "检测IC...",
        "正在擦除...",
        "正在编程...",
        "正在校验...",
        
        "未检测到IC",
        "擦除失败",
        "编程失败",
        "校验失败",
        "烧录成功",
    };

    if (_cmd == 1)
    {
        s_state = 0;    /* 待机 */
        g_tProg.Step = PG_STEP_IDLE;
        g_tProg.Time = bsp_GetRunTime();
        g_tProg.Progress = 0;        
        fRefresh = 1;
    }
    else if (_cmd == 2)  
    {
        s_state = 1;    /* 开始烧录 */
    }    
    
    /* 设置字体参数 */
    {
        tFont16.FontCode = FC_ST_16;          /* 字体代码 16点阵 */
        tFont16.FrontColor = INFO_NAME_COLOR; /* 字体颜色 */
        tFont16.BackColor = FORM_BACK_COLOR;  /* 文字背景颜色 */
        tFont16.Space = 0;                    /* 文字间距，单位 = 像素 */ 
    } 
    
    if (fRefresh == 1)
    {
        char buf[48];
        uint32_t t1;
        
        fRefresh = 0;
        
        t1 = bsp_CheckRunTime(g_tProg.Time);
        
        /* 烧录时间 */
        sprintf(buf, "%6dms", t1);
        LCD_DispStr(PROG_TIME_X,    PROG_TIME_Y,    buf, &tFont16);

        /* 烧录步骤提示 */
        LCD_DispStrEx(PROG_STEP_X, PROG_STEP_Y,  (char *)strStep[g_tProg.Step], &tFont16, 120, 0);
        
        /* 烧录进度条 */
        tFont16.BackColor = CL_MASK;
        DispProgressBar(PROGRESS_X, PROGRESS_Y, 24, 230, "", g_tProg.Progress, &tFont16);     
        tFont16.BackColor = FORM_BACK_COLOR;  /* 文字背景颜色 */
    }
    
    if (bsp_CheckTimer(0))
    {
        fRefresh = 1;
    }
    
    switch (s_state)
    {
        case 0:     /* 待机 */
            ;
            break;
        
        case 1:     /* 启动 */
            g_tProg.Step = PG_STEP_DETECT;
            g_tProg.Time = bsp_GetRunTime();
            g_tProg.Progress = 0;
            fRefresh = 1;
            s_retry = 0;
            bsp_StartAutoTimer(0, 1000);
            s_state++;       
            break;        
        
        case 2:     /* 检测IC */
            if (PG_DetectIC() == 1)
            {
                g_tProg.Step = PG_STEP_ERASE;
                g_tProg.SectorIdx = 0;
                g_tProg.SectorCount = SECTOR_COUNT_DEMO;
                s_retry = 0;
                s_state++;
            }
            else
            {
                if (++s_retry > 30)
                {
                    g_tProg.Step = PG_STEP_DETECT_ERR;
                    s_state = 99;   /* 失败 */
                }
            }
            fRefresh = 1;            
            break;
        
        case 3:     /* 正在擦除 */
            if (PG_EraseSector(g_tProg.SectorIdx) == 1)
            {
                if (++g_tProg.SectorIdx == g_tProg.SectorCount)
                {                
                    g_tProg.Step = PG_STEP_PROG;
                    g_tProg.PageCount = PAGE_COUNT_DEMO;
                    g_tProg.PageIdx = 0;
                    s_state++;
                }
                g_tProg.Progress = 100 * g_tProg.SectorIdx / g_tProg.SectorCount;
                s_retry = 0;
            }
            else
            {
                if (++s_retry > 3)
                {
                    g_tProg.Step = PG_STEP_ERASE;
                    s_state = 99;   /* 失败 */
                }
            }       
            fRefresh = 1;
            break;

        case 4:     /* 正在编程 */
            if (PG_ProgPage(g_tProg.PageIdx) == 1)
            {
                if (++g_tProg.PageIdx == g_tProg.PageCount)
                {                
                    g_tProg.Step = PG_STEP_VERIFY;
                    
                    g_tProg.PageIdx = 0;
                    s_state++;
                }
                g_tProg.Progress = 100 * g_tProg.PageIdx / g_tProg.PageCount;
                s_retry = 0;
            }
            else
            {
                if (++s_retry > 3)
                {
                    g_tProg.Step = PG_STEP_PROG_ERR;
                    s_state = 99;   /* 失败 */
                }
            }
            fRefresh = 1;            
            break;  

        case 5:     /* 正在校验 */
            if (PG_VerifyData(g_tProg.PageIdx) == 1)
            {
                if (++g_tProg.PageIdx == g_tProg.PageCount)
                {                
                    g_tProg.Step = PG_STEP_SUCCESS;
                    s_state = 100;
                }
                g_tProg.Progress = 100 * g_tProg.PageIdx / g_tProg.PageCount;
                s_retry = 0;
            }
            else
            {
                if (++s_retry > 3)
                {
                    g_tProg.Step = PG_STEP_VERIFY_ERR;
                    s_state = 99;   /* 失败 */
                }
            }  
            fRefresh = 1;            
            break;      
        
        case 99:      /* 失败 */
            BEEP_Start(5, 50, 3); /* 鸣叫50ms，停10ms， 1次 */
            bsp_StopTimer(0);
            s_state = 0;
            fRefresh = 1;
            break;
            
        case 100:     /* 成功 */
            BEEP_Start(5, 1, 1); /* 鸣叫50ms，停10ms， 1次 */
            bsp_StopTimer(0);
            s_state = 0;
            fRefresh = 1;           
            break;             
    }
}

/*
*********************************************************************************************************
*    函 数 名: ProgStatusMain
*    功能说明: 烧录主界面
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void ProgStatusMain(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    FONT_T tFont16;
    FONT_T tFont24;
	static uint8_t cursor = 0;

    /* 设置字体参数 */
    {
        tFont16.FontCode = FC_ST_16;          /* 字体代码 16点阵 */
        tFont16.FrontColor = INFO_NAME_COLOR; /* 字体颜色 */
        tFont16.BackColor = CL_MASK;  /* 文字背景颜色 */
        tFont16.Space = 0;                    /* 文字间距，单位 = 像素 */

        tFont24.FontCode = FC_ST_24;          /* 字体代码 16点阵 */
        tFont24.FrontColor = INFO_VALUE_COLOR; /* 字体颜色 */
        tFont24.BackColor = CL_MASK;  /* 文字背景颜色 */
        tFont24.Space = 0;                    /* 文字间距，单位 = 像素 */        
    }    

    LCD_ClrScr(FORM_BACK_COLOR); 
    
    DispBox(5, 5, 64, 230, CL_WHITE);
    LCD_DispStr(5 + 3,      5 + 3,      "0105", &tFont24);  
    LCD_DispStr(5 + 3 + 64, 5 + 3,      "数码管产品", &tFont24);  
    LCD_DispStr(5 + 3,      5 + 3 + 30, "LED-485-034(北京)", &tFont24);  

    /* CPU型号 */
    LCD_DispStr(TEXT1_X,    TEXT1_Y,    "STM32H743XI", &tFont16);

    /* 文件大小 */
    LCD_DispStr(TEXT2_X,    TEXT2_Y,    "文件大小: 2168789B", &tFont16);

    /* 烧录次数 */
    LCD_DispStr(TEXT3_X,    TEXT3_Y,    "烧录次数: 5 / 1000", &tFont16);    

    /* TVCC电压 */
    LCD_DispStr(TEXT4_X,    TEXT4_Y,    "TVCC电压: 3.300V", &tFont16);

    /* TVCC电流 */
    LCD_DispStr(TEXT5_X,    TEXT5_Y,    "TVCC电流: 20mA", &tFont16); 
        
    ProgTask(1);    /* 刷新一下烧录信息 */
    
    fRefresh = 1;
    while (s_ProgStatus == PS_MAIN)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

			{
				btn1.Left = BTN1_X;
				btn1.Top = BTN1_Y;
				btn1.Height = BTN1_H;
				btn1.Width = BTN1_W;
				btn1.pCaption = "开始烧录";
				btn1.Font =  &tFont16;
				btn1.Focus = 0;

				btn2.Left = BTN2_X;
				btn2.Top = BTN2_Y;
				btn2.Height = BTN2_H;
				btn2.Width = BTN2_W;
				btn2.pCaption = "选择文件";
				btn2.Font =  &tFont16;
				btn2.Focus = 0;

				btn3.Left = BTN3_X;
				btn3.Top = BTN3_Y;
				btn3.Height = BTN3_H;
				btn3.Width = BTN3_W;
				btn3.pCaption = "TVCC设置";
				btn3.Font =  &tFont16;
				btn3.Focus = 0;
				
				if (cursor == 0) btn1.Focus = 1;
				else if (cursor == 1) btn2.Focus = 1;
				else if (cursor == 2) btn3.Focus = 1;
				
				LCD_DrawButton(&btn1);
				LCD_DrawButton(&btn2);
				LCD_DrawButton(&btn3);
			} 			
        }

        bsp_Idle();
        
        ProgTask(0);
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S:      /* S键释放 - 移动按钮焦点*/    
				if (++cursor == 3)
				{
					cursor = 0;
				}
				fRefresh = 1;
                break;

            case KEY_UP_C:      /* C键释放 - 确认执行按钮功能 */ 
				if (cursor == 0)
				{
					/* 烧录一次 */
                    ProgTask(2);
				}			
                break;

            case KEY_LONG_DOWN_S:     /* S键长按 - 确认后，闪烁，修改参数 */      
				if (cursor == 0)
				{
					/* 连续烧录 */
				}
				else if (cursor == 1)
				{
                    PlayKeyTone();
					s_ProgStatus = PS_GROUP;
				}
				else if (cursor == 2)
				{
                    PlayKeyTone();
					s_ProgStatus = PS_TVCC;
				}	                
                break;
            
            case KEY_LONG_DOWN_C:    /* C键长按 */
                g_MainStatus = MS_PROG_INIT;
                s_ProgStatus = PS_EXIT;
                break;

            default:
                break;
            }
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: ProgStatusGroup
*    功能说明: 选择分组
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void ProgStatusGroup(void)
{
	uint8_t ucKeyCode;
    uint8_t fRefresh;	
    static uint8_t s_cursor = 0;
	
    DispHeader("请选择分组");

    bsp_SetKeyParam(KID_S, KEY_LONG_TIME, 5);   /* 启动按键连发功能 */
    bsp_SetKeyParam(KID_C, KEY_LONG_TIME, 5);
    fRefresh = 1;
    while (s_ProgStatus == PS_GROUP)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

			DispGroupList(s_cursor);
            DispHeaderSn(s_cursor);
        }

        bsp_Idle();
        
        ucKeyCode = bsp_GetKey();   /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S:          /* S键释放 - 向上移动 */    
                if (s_cursor > 0)
                {
                    s_cursor--;
                }
                else
                {
                    s_cursor = GROUP_NUM;
                }   
                fRefresh = 1;                
                break;

            case KEY_AUTO_C:
            case KEY_UP_C:          /* C键释放 - 向下移动 */ 	
                if (++s_cursor > GROUP_NUM)
                {
                    s_cursor = 0;
                }
                fRefresh = 1;                
                break;

            case KEY_LONG_DOWN_S:    /* S键长按 */
                PlayKeyTone();                
                if (s_cursor == GROUP_NUM)
                {
                    s_ProgStatus = PS_MAIN;
                }
                else
                {
                    s_ProgStatus = PS_FILE;
                }
                break;           
            
            case KEY_LONG_DOWN_C:    /* C键长按 */
                break;
            
            default:
                break;
            }
        }
    }	
    bsp_SetKeyParam(KID_S, KEY_LONG_TIME, 0);   /* 关闭按键连发功能 */
    bsp_SetKeyParam(KID_C, KEY_LONG_TIME, 0);    
}

/*
*********************************************************************************************************
*    函 数 名: ProgStatusFile
*    功能说明: 选择文件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void ProgStatusFile(void)
{
	uint8_t ucKeyCode;
    uint8_t fRefresh;	
    static uint8_t s_cursor = 0;
    uint8_t ucRollDir = 1;
	
    DispHeader("请选择文件");

    bsp_SetKeyParam(KID_S, KEY_LONG_TIME, 5);   /* 启动按键连发功能 */
    bsp_SetKeyParam(KID_C, KEY_LONG_TIME, 5);
    fRefresh = 1;
    while (s_ProgStatus == PS_FILE)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;

			DispFileList(s_cursor);
            DispHeaderSn(s_cursor);
        }

        bsp_Idle();
        
        ucKeyCode = bsp_GetKey();       /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S:              /* S键释放 - 向上移动 */    
                if (s_cursor > 0)
                {
                    s_cursor--;
                }
                else
                {
                    s_cursor = FILE_NUM;
                }   
                ucRollDir = 0;
                fRefresh = 1;                
                break;

            case KEY_UP_C:              /* C键释放 - 向下移动 */ 	
                ucRollDir = 1;                
            case KEY_AUTO_C:            /* C键连续 */       
                if (ucRollDir == 1)
                {
                    if (++s_cursor > FILE_NUM)
                    {
                        s_cursor = 0;
                    }
                }
                else
                {
                    if (s_cursor > 0)
                    {
                        s_cursor--;
                    }
                    else
                    {
                        s_cursor = FILE_NUM;
                    }                   
                }
                fRefresh = 1;                
                break;      

            case KEY_LONG_DOWN_S:       /* S键长按 */
                PlayKeyTone();
                if (s_cursor == FILE_NUM)
                {
                     s_ProgStatus = PS_GROUP;
                }
                else
                {
                    s_ProgStatus = PS_MAIN;
                }                
                break;
            
            case KEY_LONG_DOWN_C:       /* C键长按 */
                break;

            default:
                break;
            }
        }
    }	
    bsp_SetKeyParam(KID_S, KEY_LONG_TIME, 0);   /* 关闭按键连发功能 */
    bsp_SetKeyParam(KID_C, KEY_LONG_TIME, 0);    
}

/*
*********************************************************************************************************
*    函 数 名: ProgStatusTvcc
*    功能说明: 选择电压
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void ProgStatusTvcc(void)
{
	uint8_t ucKeyCode;
    uint8_t fRefresh;	
    char buf[48];
	
	DispHeader("请选择TVCC输出");
    DispHelpBar("长按S确认",
                "长按C返回");  

    sprintf(buf, "关闭");
    DispParamBar(0, "TVCC :", buf, 1);
    
    fRefresh = 1;
    while (s_ProgStatus == PS_TVCC)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;			
        }

        bsp_Idle();
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
            case KEY_UP_S:      /* S键释放 - 移动按钮焦点*/    
                break;

            case KEY_UP_C:      /* C键释放 - 确认执行按钮功能 */ 			
                break;

            case KEY_LONG_DOWN_S:    /* S键长按 - 确认后，闪烁，修改参数 */
				s_ProgStatus = PS_MAIN;
                break;
            
            case KEY_LONG_DOWN_C:    /* C键长按 */
                s_ProgStatus = PS_MAIN;
                break;

            default:
                break;
            }
        }
    }	
}

/*
*********************************************************************************************************
*    函 数 名: DispGroupList
*    功能说明: 分组显示函数
*    形    参: _usIndex 当前选中的 0-20
*    返 回 值: 无
*********************************************************************************************************
*/
const char * TestGroupName[] = 
{
    "STM8S系列",
    "STM32系列",
    "开发板",
    "无线IO模块",
    "TFT显示器",    
    "数码管",      
    "工控板",      
    "安富莱专用", 
    "临时程序", 
    "中国电信专用", 
    
    "STM8S系列",
    "STM32系列",
    "开发板",
    "",
    "TFT显示器",    
    "",      
    "工控板",      
    "安富莱专用", 
    "", 
    "", 

    "---返回---"
};
static void DispGroupList(uint16_t _usIndex)
{
    uint16_t i;
    uint16_t x,y;
    char buf[64];
    uint8_t sn;
    FONT_T tFont;   
    uint16_t ColorList[7] = {
        LIST_COLOR1,
        LIST_COLOR1, 
        LIST_COLOR1,
        LIST_COLOR_ACTIVE,
        LIST_COLOR1,
        LIST_COLOR1,  
        LIST_COLOR1
    };
      
    
    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_24;          /* 字体代码 16点阵 */
        tFont.FrontColor = LIST_ITEM_COLOR; /* 字体颜色 */
        tFont.BackColor = CL_MASK;          /* 文字背景颜色 */
        tFont.Space = 0;                    /* 文字间距，单位 = 像素 */
    }    
    
    x = 0;
    y = 34;
    if (_usIndex >= 3)
    {
        sn = _usIndex - 3;
    }
    else
    {
        sn = GROUP_NUM + _usIndex - 2;
    }
    for (i = 0; i < 7; i++)
    {
        LCD_Fill_Rect(x, y, 28, 240, ColorList[i]);
        
        sprintf(buf, "%02d %s", sn, TestGroupName[sn]);
        LCD_DispStr(x, y + 2, buf, &tFont);
        y += 30;        
        
        if (++sn > GROUP_NUM)
        {
            sn = 0;
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: DispFileList
*    功能说明: 显示文件列表
*    形    参: _usIndex 当前选中的 0-49
*    返 回 值: 无
*********************************************************************************************************
*/
const char * TestFileName[] = 
{
    "LED-485-034 V1.0",
    "STM32-V4 V1.3",
    "开发板V7 emWin",
    "RC602-Boot V2.34",
    "P01北京专用V0.9",
    "数码管V001",   
    "工控板Demo",
    "P01专用V2.0", 
    "V5 emWin V0.8", 
    "---", 
    
    "LED-485-034 V1.0",
    "STM32-V4 V1.3",
    "开发板V7 emWin",
    "RC602-Boot V2.34",
    "P01北京专用V0.9",
    "数码管V001",   
    "工控板Demo",
    "P01专用V1.0", 
    "V5 emWin V0.8", 
    "---",  

    "LED-485-034 V1.0",
    "STM32-V4 V1.3",
    "开发板V7 emWin",
    "RC602-Boot V2.34",
    "P01北京专用V0.9",
    "数码管V001",   
    "工控板Demo",
    "P01专用V1.0", 
    "V5 emWin V0.8", 
    "---", 
    
    "LED-485-034 V1.0",
    "STM32-V4 V1.3",
    "开发板V7 emWin",
    "RC602-Boot V2.34",
    "P01北京专用V0.9",
    "数码管V001",   
    "工控板Demo",
    "P01专用V1.0", 
    "V5 emWin V0.8", 
    "---", 

    "LED-485-034 V1.0",
    "STM32-V4 V1.3",
    "开发板V7 emWin",
    "RC602-Boot V2.34",
    "P01北京专用V0.9",
    "数码管V001",   
    "工控板Demo",
    "P01专用V1.0", 
    "V5 emWin V0.8", 
    "---",

    "---返回---"
};
static void DispFileList(uint16_t _usIndex)
{
    uint16_t i;
    uint16_t x,y;
    char buf[64];
    uint8_t sn;
    FONT_T tFont;   
    uint16_t ColorList[7] = {
        LIST_COLOR1,
        LIST_COLOR1, 
        LIST_COLOR1,
        LIST_COLOR_ACTIVE,
        LIST_COLOR1,
        LIST_COLOR1,  
        LIST_COLOR1
    };
          
    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_24;          /* 字体代码 16点阵 */
        tFont.FrontColor = LIST_ITEM_COLOR; /* 字体颜色 */
        tFont.BackColor = CL_MASK;          /* 文字背景颜色 */
        tFont.Space = 0;                    /* 文字间距，单位 = 像素 */
    }    
    
    x = 0;
    y = 34;
    if (_usIndex >= 3)
    {
        sn = _usIndex - 3;
    }
    else
    {
        sn = FILE_NUM + _usIndex - 2;
    }
    for (i = 0; i < 7; i++)
    {
        LCD_Fill_Rect(x, y, 28, 240, ColorList[i]);
        
        sprintf(buf, "%02d %s", sn, TestFileName[sn]);
        LCD_DispStr(x, y + 2, buf, &tFont);
        y += 30;        
        
        if (++sn > FILE_NUM)
        {
            sn = 0;
        }
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
