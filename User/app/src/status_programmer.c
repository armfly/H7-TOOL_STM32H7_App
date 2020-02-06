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
#include "file_lib.h"
#include "lua_if.h"
#include "prog_if.h"

/* 三个按钮 */
#define BTN1_X     (240 - BTN1_W - 5)
#define BTN1_Y     80
#define BTN1_H     28 
#define BTN1_W     80 

#define BTN2_X     BTN1_X
#define BTN2_Y     BTN1_Y + BTN1_H + 8 
#define BTN2_H     BTN1_H 
#define BTN2_W     BTN1_W 

#define BTN3_X     BTN1_X
#define BTN3_Y     BTN1_Y + 2 * (BTN1_H + 8)
#define BTN3_H     BTN1_H 
#define BTN3_W     BTN1_W 

#define TEXT_WIDTH  144

/* 文件大小 */
#define TEXT1_X    5
#define TEXT1_Y    BTN1_Y

/* 本次次数 */
#define TEXT2_X    TEXT1_X
#define TEXT2_Y    TEXT1_Y + 20

/* 累积次数 */
#define TEXT3_X    TEXT1_X
#define TEXT3_Y    TEXT2_Y + 20

/* TVCC电压 */
#define TEXT4_X    TEXT1_X
#define TEXT4_Y    TEXT3_Y + 20

/* 供电电压*/
#define TEXT_VOLT_X     5
#define TEXT_VOLT_Y     TEXT4_Y + 25
#define TEXT_VOLT_W     60

/* 供电电流 */
#define TEXT_CURR_X     TEXT_VOLT_X + TEXT_VOLT_W + 5
#define TEXT_CURR_Y     TEXT_VOLT_Y
#define TEXT_CURR_W     60

/* 烧录时间 */
#define PROG_TIME_X     160
#define PROG_TIME_Y     240 - 50

/* 烧录步骤提示 */
#define PROG_STEP_X     5
#define PROG_STEP_Y     240 - 50

/* 烧录进度条 */
#define PROGRESS_X      5
#define PROGRESS_Y      240 - 30



//static void DispGroupList(uint16_t _usIndex);
//static void DispFileList(uint16_t _usIndex);

//static void FindLuaNote(char *_strNote1, char *_strNote2, uint16_t _MaxLen);
void DispProgVoltCurrent(void);
void DispProgProgress(char *_str, float _progress);
static void DispProgCounter(void);

extern void sysTickInit(void);
extern uint8_t swd_init_debug(void);
extern uint8_t swd_read_idcode(uint32_t *id);

/*
*********************************************************************************************************
*    函 数 名: status_ProgSelectFile
*    功能说明: 脱机编程器界面，选择文件。 - 文件浏览器
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_ProgSelectFile(void)
{
    uint8_t re;
    
    re = SelectFile(PROG_USER_DIR, MS_PROG_SELECT_FILE, MS_PROG_WORK, "*.lua");
    
    if (re == 1)
    {
        /* 保存缺省lua文件路径到autorun.ini文件 */
        SaveProgAutorunFile(g_tFileList.Path);
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
extern void PG_PrintText(char *_str);
void status_ProgWork(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    BUTTON_T btn1, btn2, btn3;
    FONT_T tFontNote;
    FONT_T tFontText;
    FONT_T tFontBtn;
    //static uint8_t cursor = 0;   
    uint8_t cursor = 0;  
//    static uint8_t s_first = 0;
    uint8_t fRunOnce = 0; 
    uint8_t ucAutoState = 0;
    uint8_t ucDetectCount = 0;    /* 连续多少次没有检测到芯片则认为目标板已拔出 */
    int32_t iDetectTime;
    
//    if (s_first == 0)
//    {
//        s_first = 1;
//        
//        strcpy(g_tProg.FilePath, "");   /* 初始路径 */
//    }
//    else
//    {
//        strcpy(g_tProg.FilePath, g_tFileList.Path);
//    }
    
    /* 读autorun.ini文件，获得缺省lua文件路径 */
    LoadProgAutorunFile(g_tProg.FilePath, sizeof(g_tProg.FilePath));
    
    /* 设置字体参数 */
    {
        tFontNote.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
        tFontNote.FrontColor = INFO_VALUE_COLOR;    /* 字体颜色 */
        tFontNote.BackColor = CL_WHITE;             /* 文字背景颜色 */      
        tFontNote.Space = 0;                        /* 文字间距，单位 = 像素 */

        tFontText.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
        tFontText.FrontColor = INFO_NAME_COLOR;     /* 字体颜色 */
        tFontText.BackColor = FORM_BACK_COLOR;      /* 文字背景颜色 */      
        tFontText.Space = 0;                        /* 文字间距，单位 = 像素 */      

        tFontBtn.FontCode = FC_ST_16;               /* 字体代码 16点阵 */
        tFontBtn.FrontColor = INFO_NAME_COLOR;      /* 字体颜色 */
        tFontBtn.BackColor = CL_MASK;               /* 文字背景颜色 */      
        tFontBtn.Space = 0;                         /* 文字间距，单位 = 像素 */        
    }    

    LCD_ClrScr(FORM_BACK_COLOR); 
    
    {                
        s_lua_read_len = ReadFileToMem(g_tProg.FilePath, 0, s_lua_prog_buf, LUA_PROG_LEN_MAX);  /* 读lua文件 */
        
        //DispBox(5, 5, 56, 230, CL_WHITE);
        {
            /* 绘制边框 */
            LCD_DrawRoundRect(5, 5, 56, 230, 3, MEAS_BODER_COLOR);
            
            /* 填充矩形 */
            LCD_FillRoundRect(5 + 1, 5 + 1, 64, 230 - 2, 3, CL_WHITE);            
        }      
        
        if (s_lua_read_len > 0)
        {              
            const char *pNote1;
            const char *pDataFileName;
            uint32_t ulDataFileSize = 0;
            char *p;
            char buf[32];   

            uint8_t i;
            uint16_t len;
            uint8_t line = 0;
            
            lua_DownLoadFile(g_tProg.FilePath);  /* 重新初始化lua环境，并装载lua文件 */
            
            /* 从lua文件中获得注释字符串Note01 */
            lua_getglobal(g_Lua, "Note01");    
            if (lua_isstring(g_Lua, -1))
            {
                pNote1 = lua_tostring(g_Lua, -1); 
            }
            else
            {
                pNote1 = "";
            }
            
            /* 从lua文件中获得程序文件名 */
            lua_getglobal(g_Lua, "DataFile_0");  
            if (lua_isstring(g_Lua, -1))
            {
                pDataFileName = lua_tostring(g_Lua, -1);
                
                {
                    char path[256];
                    
                    /* 传入的文件名是相对路径 */
                    if (strlen(pDataFileName) + strlen(PROG_USER_DIR) > sizeof(path))
                    {
                        /* 文件名过长 */
                    }
                
                    GetDirOfFileName(g_tProg.FilePath, path);
                    strcat(path, "/");
                    strcat(path, pDataFileName);
    
                    FixFileName(path);  /* 去掉路径中的..上级目录 */
                    
                    ulDataFileSize = GetFileSize(path);     /* 获得程序文件的大小，bin */
                }
            }
            else
            {
                pDataFileName = "";
            }        
            
            LCD_SetEncode(ENCODE_GBK);            
            
            /* 显示lua文件名，两行最多56字符。 需要处理过长的文件名，每行只能显示28字符 */
            p = &g_tProg.FilePath[strlen(PROG_USER_DIR) + 1];
            for (i = 0; i < 2; i++)
            {
                len = 0;
                while (1)
                {
                    buf[len] = *p++;
                    if (buf[len] == 0)
                    {
                        LCD_DispStr(5 + 3,      5 + 3 + 20 * line,      buf, &tFontNote);   
                        i = 2;
                        break;
                    }
                    len++;
                    
                    if (buf[len] >= 0x80)   /* 汉字需要双字节处理,自动断行 */
                    {
                        buf[len] = *p++;
                        len++;
                    }      

                    if (len > 27)
                    {
                        buf[len] = 0;
                        LCD_DispStr(5 + 3,      5 + 3 + 20 * line++,      buf, &tFontNote);   
                        break;
                    }
                }
            }            
            
            LCD_DispStr(5 + 3,      5 + 3 + 40, (char *)pNote1, &tFontNote);  /* 显示lua中的注释 */        
            
            LCD_SetEncode(ENCODE_UTF8);
            
            sprintf(buf, "程序大小: %d", ulDataFileSize);
            LCD_DispStrEx(TEXT1_X,    TEXT1_Y, buf, &tFontText,  TEXT_WIDTH, ALIGN_LEFT);
            
            /* 读ini配置文件 */
            ReadProgIniFile(g_tProg.FilePath, &g_tProgIni);
            DispProgCounter();
        }
        else
        {
            LCD_DispStr(5 + 3,      5 + 3,      "未选择文件", &tFontText);   
            LCD_DispStr(5 + 3,      5 + 3 + 30, "", &tFontText);                
        }
    }
    
    g_tProg.AutoStart = 0;
    fRefresh = 1;
    while (g_MainStatus == MS_PROG_WORK)
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
				btn1.Font =  &tFontBtn;
				btn1.Focus = 0;

				btn2.Left = BTN2_X;
				btn2.Top = BTN2_Y;
				btn2.Height = BTN2_H;
				btn2.Width = BTN2_W;
				btn2.pCaption = "选择文件";
				btn2.Font =  &tFontBtn;
				btn2.Focus = 0;

				btn3.Left = BTN3_X;
				btn3.Top = BTN3_Y;
				btn3.Height = BTN3_H;
				btn3.Width = BTN3_W;
				btn3.pCaption = "烧录设置";
				btn3.Font =  &tFontBtn;
				btn3.Focus = 0;
				
				if (cursor == 0) btn1.Focus = 1;
				else if (cursor == 1) btn2.Focus = 1;
				else if (cursor == 2) btn3.Focus = 1;
				
				LCD_DrawButton(&btn1);
				LCD_DrawButton(&btn2);
				LCD_DrawButton(&btn3);
			} 			
        }

        DispProgVoltCurrent();

        bsp_Idle();                        
        
        if (fRunOnce == 1)
        {
            fRunOnce = 0;
            
            /* 读ini配置文件 */
            ReadProgIniFile(g_tProg.FilePath, &g_tProgIni);
            DispProgCounter();
            
            /* 判断剩余次数 */
            {
                ReadProgIniFile(g_tProg.FilePath, &g_tProgIni); /* 读ini文件，每个lua对应一个ini文件，用于记录编程次数 */
                
                if (g_tProgIni.Locked == 1)
                {
                    continue;
                }
                
                if (g_tProgIni.ProgramLimit == 1 && g_tProgIni.ProgramLimit <= g_tProgIni.ProgrammedCount)
                {                  
                    continue;
                }
            }            
            
            g_tProg.Time = bsp_GetRunTime();    /* 记录开始时间 */
            g_tProg.Err = 100;
            g_tProg.EraseChipTime1 = 0;
            g_tProg.EraseChipTime2 = 0;
            if (g_Lua > 0)
            {
                const char *ret_str;
                /* 
                    执行lua中的函数start_prog()。会阻塞只到编程完毕。  
                    lua_if.c 中的钩子函数LuaYeildHook()实现长按S键终止lua执行。
                    界面的绘制由编程函数内部负责刷新
                */
                PG_PrintText("开始烧录..\r\n");
                
                lua_do("ret_str = start_prog()");   /* 执行编程，阻塞只到编程完毕 */
                
                lua_getglobal(g_Lua, "ret_str");    
                if (lua_isstring(g_Lua, -1))
                {
                    ret_str = lua_tostring(g_Lua, -1); 
                }
                else
                {
                    ret_str = "";
                }
                //ua_pop(g_Lua, 1);
                if (strcmp(ret_str, "OK") == 0)
                {
                    g_tProg.Err = 0;
                }
                else
                {
                     g_tProg.Err = 1;
                }
                
                /* 编程完毕 */
                {
                    char buf[32];
                    
                    if (g_tProg.Err == 0)
                    {
                        g_tProg.NowProgCount++;

                        sprintf(buf, "本次计数: %d", g_tProg.NowProgCount);
                        LCD_DispStrEx(TEXT2_X,    TEXT2_Y, buf, &tFontText,  TEXT_WIDTH, ALIGN_LEFT);                        
                    }
                }
                
                if (g_tProg.Err == 0)
                {
                    g_tProgIni.ProgrammedCount++;       /* 已编程计数器加1 */
                    
                    WriteProgIniFile(g_tProg.FilePath, &g_tProgIni);    /* 写入ini文件 */
                    
                    /* 读ini配置文件 */
                    ReadProgIniFile(g_tProg.FilePath, &g_tProgIni);
                    DispProgCounter();      /* 显示烧录计数器 */

                    /* 连续自动烧录 */
                    if (g_tProg.AutoStart == 1)
                    {
                        ucAutoState = 1;
                        
                        iDetectTime = bsp_GetRunTime();
                        
                        PG_PrintText("烧录成功,等待移除");
                    }                    
                }  
                else    /* 烧录失败 */
                {
                    /* 连续自动烧录 */
                    if (g_tProg.AutoStart == 1)
                    {
                        ucAutoState = 1;
                        
                        iDetectTime = bsp_GetRunTime();
                        
                        PG_PrintText("烧录失败,等待移除");
                    }
                }
            }
            else
            {
                BEEP_Start(5, 5, 3);    /* 错误提示音 */
  
                LCD_DispStrEx(PROG_STEP_X, PROG_STEP_Y,  "请选择文件", &tFontText, 120, 0);
            }
        }
        
        if (ucAutoState == 1)
        {
            uint32_t id;

            if (bsp_CheckRunTime(iDetectTime) > 50)
            {
                iDetectTime = bsp_GetRunTime();
                
                sysTickInit();          /* 这是DAP驱动中的初始化函数,全局变量初始化 */
                swd_init_debug();       /* 进入swd debug状态 */
                
                if (swd_read_idcode(&id) == 0)  /* 0 = 出错 */
                {
                    if (++ucDetectCount > 3)      /* 连续3次读取失败 */
                    {
                        ucDetectCount = 0;
                        
                        ucAutoState = 2;
                        
                        if (g_tProg.Err == 0)
                        {
                            PG_PrintText("连续烧录,等待插入");
                        }
                        else
                        {
                            PG_PrintText("上次失败,等待插入");
                        }                       
                        
                        bsp_PutKey(KEY_DB_S);       /* 任意发一个本状态无用的按键消息，避免背光超时关闭 */
                    }
                }
                else
                {
                    ucDetectCount = 0;
                }
            }
        }
        else if (ucAutoState == 2)
        {
            uint32_t id;

            if (bsp_CheckRunTime(iDetectTime) > 50)
            {
                iDetectTime = bsp_GetRunTime();
                
                sysTickInit();          /* 这是DAP驱动中的初始化函数,全局变量初始化 */
                swd_init_debug();       /* 进入swd debug状态 */                
                if (swd_read_idcode(&id) != 0)  /* 不等于0表示检测OK */
                {
                    if (++ucDetectCount > 6)      /* 连续6次 300ms */
                    {
                        ucAutoState = 0;
                        fRunOnce = 1;
                    }
                }
                else
                {
                    ucDetectCount = 0;
                }
            }
        }
                    
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
                        g_tProg.AutoStart = 0;
                        fRunOnce = 1;
                    }			
                    break;

                case KEY_LONG_DOWN_S:     /* S键长按 - 确认后，闪烁，修改参数 */      
                    if (cursor == 0)
                    {
                        g_tProg.AutoStart = 1;  /* 连续烧录 */
                        fRunOnce = 1;
                    }
                    else if (cursor == 1)
                    {
                        PlayKeyTone();
                        g_MainStatus = MS_PROG_SELECT_FILE;
                    }
                    else if (cursor == 2)
                    {
                        PlayKeyTone();
                        g_MainStatus = MS_PROG_SETTING;
                    }	                
                    break;
                
                case KEY_LONG_DOWN_C:    /* C键长按 */
                    g_MainStatus = MS_EXTEND_MENU1;
                    break;

                default:
                    break;
            }
        }
    }
}

/* 
char s_lua_prog_buf[LUA_PROG_LEN_MAX + 1];
uint32_t s_lua_prog_len;

根据LUA文件查找程序注释

--Note01=数码管产品
--Note02=LED-485-034(V2.1)
*/
//void FindLuaNote(char *_strNote1, char *_strNote2, uint16_t _MaxLen)
//{
//	/* 解析文件，填充按钮提示栏 */
//	{
//        uint16_t i,j;
//		char *pText;
//		char *p;
//		char head[12];
//        char *pOut[2];

//        pOut[0] = _strNote1;
//        pOut[1] = _strNote2;
//        
//        _strNote1[0] = 0;
//        _strNote2[0] = 0;
//		pText = s_lua_prog_buf;
//		for (i = 0; i < 2; i++)
//		{
//			sprintf(head, "--Note%02d=", i + 1);

//			p = strstr(pText, head);
//			if (p > 0)
//			{
//				p += 9;
//				for (j = 0; j < _MaxLen; j++)
//				{
//					pOut[i][j] = p[j];
//					if (p[j] == 0 || p[j] == 0x0D || p[j] == 0x0A)
//					{
//						pOut[i][j] = 0;
//						break;
//					}
//				}
//				pOut[i][j] = 0;
//			}
//			else
//			{
//				strcpy(pOut[i], "----");
//			}
//		}
//	}
//    
//    /* 找到数据文件名，并计算文件大小 */
//    ;
//}

/*
*********************************************************************************************************
*    函 数 名: ProgFinishedCallBack
*    功能说明: 显示编程进度
*    形    参:  _ErrCode 编程结果
*    返 回 值: 无
*********************************************************************************************************
*/
void ProgFinishedCallBack(uint8_t _ErrCode)
{
    g_tProg.Err = _ErrCode;
}

/*
*********************************************************************************************************
*    函 数 名: DispProgProgress
*    功能说明: 显示编程进度
*    形    参:  _str :  文本提示. 0表示不刷新显示
*               _progress : 进度百分比， -1表示不刷新
*    返 回 值: 无
*********************************************************************************************************
*/
void DispProgProgress(char *_str, float _progress)
{    
    FONT_T tFont16;
    
    /* 设置字体参数 */
    {
        tFont16.FontCode = FC_ST_16;          /* 字体代码 16点阵 */
        tFont16.FrontColor = INFO_NAME_COLOR; /* 字体颜色 */
        tFont16.BackColor = FORM_BACK_COLOR;  /* 文字背景颜色 */
        tFont16.Space = 0;                    /* 文字间距，单位 = 像素 */ 
    } 
          
    {
        char buf[48];
        uint32_t t1;     
        
        if (g_tProg.Time > 0)  
        {
            t1 = bsp_CheckRunTime(g_tProg.Time);
            
            /* 烧录时间 */
            sprintf(buf, "%3d.%02d 秒", t1 / 1000, (t1 % 1000) / 10);
            LCD_DispStr(PROG_TIME_X,    PROG_TIME_Y,    buf, &tFont16);
        }
        
        /* 烧录步骤提示 */
        if (_str > 0)
        {
            LCD_SetEncode(ENCODE_GBK);
            LCD_DispStrEx(PROG_STEP_X, PROG_STEP_Y,  _str, &tFont16, 144, 0);
            LCD_SetEncode(ENCODE_UTF8);
        }
        
        /* 烧录进度条 */
        if (_progress > 0)
        {
            tFont16.BackColor = CL_MASK;        
            DispProgressBar(PROGRESS_X, PROGRESS_Y, 24, 230, "", _progress, &tFont16);             
            tFont16.BackColor = FORM_BACK_COLOR;  /* 文字背景颜色 */
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: status_ProgSetting
*    功能说明: 选择烧录器参数配置
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void status_ProgSetting(void)
{
	uint8_t ucKeyCode;
    uint8_t fRefresh;	
    char buf[48];
	
	DispHeader("烧录器参数");
    DispHelpBar("长按S确认",
                "长按C返回");  

    sprintf(buf, "关闭");
    DispParamBar(0, "TVCC :", buf, 1);
    
    fRefresh = 1;
    while (g_MainStatus == MS_PROG_SETTING)
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
				g_MainStatus = MS_PROG_WORK;
                break;
            
            case KEY_LONG_DOWN_C:    /* C键长按 */
                g_MainStatus = MS_PROG_WORK;
                break;

            default:
                break;
            }
        }
    }	
}

/*
*********************************************************************************************************
*    函 数 名: DispProgCounter
*    功能说明: 显示烧录次数等信息
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispProgCounter(void)
{
    char buf[32];
    int32_t count;  
    FONT_T tFontText;

    tFontText.FontCode = FC_ST_16;              /* 字体代码 16点阵 */
    tFontText.FrontColor = INFO_NAME_COLOR;     /* 字体颜色 */
    tFontText.BackColor = FORM_BACK_COLOR;      /* 文字背景颜色 */      
    tFontText.Space = 0;                        /* 文字间距，单位 = 像素 */       

    sprintf(buf, "本次计数: %d", g_tProg.NowProgCount);
    LCD_DispStrEx(TEXT2_X,    TEXT2_Y, buf, &tFontText,  TEXT_WIDTH, ALIGN_LEFT);

    /* 显示剩余次数 */
    if (g_tProgIni.Locked == 1)
    {
        sprintf(buf, "剩余次数: 已锁死");
    }
    else 
    {
        count = g_tProgIni.ProgramLimit - g_tProgIni.ProgrammedCount;
    }
    if (g_tProgIni.ProgramLimit == 0 || count <= 0)
    {
        sprintf(buf, "剩余次数: 不限制");
    }
    else
    {
        sprintf(buf, "剩余次数: %d", count);
    }         
    LCD_DispStrEx(TEXT3_X,    TEXT3_Y, buf, &tFontText,  TEXT_WIDTH, ALIGN_LEFT); 

    
    sprintf(buf, "累积次数: %d", g_tProgIni.ProgrammedCount);
         
    LCD_DispStrEx(TEXT4_X,    TEXT4_Y, buf, &tFontText,  TEXT_WIDTH, ALIGN_LEFT);     
}

/*
*********************************************************************************************************
*    函 数 名: DispProgVoltCurrent
*    功能说明: 显示TVCC电压电流。 每隔300ms刷新一次。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void DispProgVoltCurrent(void)
{
    FONT_T tFont16;
    char buf[32];
    static int32_t s_last_time = 0;
    
    if (bsp_CheckRunTime(s_last_time) < 300)
    {
        return;
    }
    
    s_last_time = bsp_GetRunTime();
    
    /* 设置字体参数 */
    {
        tFont16.FontCode = FC_ST_16;    /* 字体代码 16点阵 */
        tFont16.FrontColor = CL_WHITE;  /* 字体颜色 */
        tFont16.BackColor = HEAD_SN_COLOR;    /* 文字背景颜色 */
        tFont16.Space = 0;              /* 文字间距，单位 = 像素 */ 
    } 
    
    sprintf(buf, "%0.3fV", g_tVar.TVCCVolt);
    LCD_DispStrEx(TEXT_VOLT_X, TEXT_VOLT_Y, buf, &tFont16, TEXT_VOLT_W, 1);  

    sprintf(buf, "%0.0fmA", g_tVar.TVCCCurr);
    LCD_DispStrEx(TEXT_CURR_X, TEXT_CURR_Y, buf, &tFont16, TEXT_CURR_W, 1);         
}    

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
