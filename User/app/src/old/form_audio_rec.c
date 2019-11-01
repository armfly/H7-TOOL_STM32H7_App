/*
*********************************************************************************************************
*
*    模块名称 : 录音演示程序。
*    文件名称 : audio_rec.c
*    版    本 : V1.2
*    说    明 : 演示I2S录音和回放功能。通过串口超级终端作为操作界面。
*    修改记录 :
*        版本号  日期       作者    说明
*        v1.0    2013-02-01 armfly  首发
*        V1.1    2014-11-04 armfly  去掉LED1指示灯的控制。因为这个GPIO将用于3.5寸触摸屏。
*        V1.2    2015-01-08 armfly  修改StartPlay(void)函数，根据扬声器设置决定是否打扬声器
*
*    Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
    安富莱开发板配置的I2S音频CODEC芯片为WM8978。

    操作提示：
        [TAMPER]键     = 选择音频格式
        [WAKEUP]键     = 开始录音
        [USER]键       = 开始回放
        摇杆上/下键    = 调节音量
        摇杆左/右键    = 调节MIC增益
        摇杆OK键       = 终止录放

    录音时数据保存在安富莱STM32开发板外扩的2MB SRAM中，缺省使用8K采样率，16bit格式。1MB可以保存64秒录音。
    放音按相同的格式，从外部SRAM中读取数据。
        
    WM8978支持的采样率 : 
        Support for 8, 11.025, 12, 16, 22.05, 24, 32, 44.1 and 48kHz sample rates
        
*/

#include "bsp.h"
#include "form_audio_rec.h"

#define STR_Help1 "摇杆上/下键 = 调节耳机音量"
#define STR_Help2 "摇杆左/右键 = 调节麦克风增益"
#define STR_Help3 "K1键       = 选择音频格式"

/* 返回按钮的坐标(屏幕右下角) */
#define BUTTON_RET_H 32
#define BUTTON_RET_W 60
#define BUTTON_RET_X (g_LcdWidth - BUTTON_RET_W - 4)
#define BUTTON_RET_Y (g_LcdHeight - BUTTON_RET_H - 4)
#define BUTTON_RET_TEXT "返回"

#define BUTTON_REC_H 32
#define BUTTON_REC_W 74
#define BUTTON_REC_X 5
#define BUTTON_REC_Y 180
#define BUTTON_REC_TEXT "开始录音"

#define BUTTON_PLAY_H 32
#define BUTTON_PLAY_W 74
#define BUTTON_PLAY_X (BUTTON_REC_X + BUTTON_REC_W + 10)
#define BUTTON_PLAY_Y BUTTON_REC_Y
#define BUTTON_PLAY_TEXT "开始放音"

#define BUTTON_STOP_H 32
#define BUTTON_STOP_W 74
#define BUTTON_STOP_X (BUTTON_PLAY_X + BUTTON_PLAY_W + 10)
#define BUTTON_STOP_Y BUTTON_REC_Y
#define BUTTON_STOP_TEXT "停止"

/* 以下是检查框 */
#define CHECK_SPK_X BUTTON_REC_X
#define CHECK_SPK_Y (BUTTON_REC_Y + BUTTON_REC_H + 5)
#define CHECK_SPK_H CHECK_BOX_H
#define CHECK_SPK_W (CHECK_BOX_W + 5 * 16) /* 决定触摸有效区域 */
#define CHECK_SPK_TEXT "打开扬声器"

REC_T g_tRec;

/* 音频格式切换列表(可以自定义) */
#define FMT_COUNT 9 /* 音频格式数组元素个数 */
uint32_t g_FmtList[FMT_COUNT][3] =
        {
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_8K},
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_11K},
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_16K},
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_22K},

                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_32K},
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_44K},
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_48K},
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_96K},
                {I2S_STANDARD_PHILIPS, SAI_DATASIZE_16, I2S_AUDIOFREQ_192K},
};

/* 定义录音放音缓冲区 */
#define REC_MEM_ADDR SDRAM_APP_BUF
#define REC_MEM_SIZE (2 * 1024 * 1024)

/* 仅允许本文件内调用的函数声明 */
static void DispStatus(void);

static void StartPlay(void);
static void StartRecord(void);
static void StopRec(void);

/*
*********************************************************************************************************
*    函 数 名: RecorderDemo
*    功能说明: 录音机主程序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void RecorderDemo(void)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t ucTouch;     /* 触摸事件 */
    uint8_t fRefresh;    /* 刷屏请求标志,1表示需要刷新 */
    uint8_t fQuit = 0;
    FONT_T tFont, tFontBtn, tFontChk; /* 定义一个字体结构体变量，用于设置字体参数 */

    char buf[128];
    uint16_t x, y;
    uint16_t usLineCap = 18;

    int16_t tpX, tpY;
    BUTTON_T tBtn;
    CHECK_T tCheck;

    LCD_ClrScr(CL_BLUE); /* 清屏，背景蓝色 */

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE; /* 字体颜色 */
        tFont.BackColor = CL_BLUE;     /* 文字背景颜色 */
        tFont.Space = 0;                         /* 文字间距，单位 = 像素 */

        /* 按钮字体 */
        tFontBtn.FontCode = FC_ST_16;
        tFontBtn.BackColor = CL_MASK; /* 透明色 */
        tFontBtn.FrontColor = CL_BLACK;
        tFontBtn.Space = 0;

        /* CheckBox 字体 */
        tFontChk.FontCode = FC_ST_16;
        tFontChk.BackColor = CL_MASK; /* 透明色 */
        tFontChk.FrontColor = CL_YELLOW;
        tFontChk.Space = 0;
    }

    x = 5;
    y = 3;
    LCD_DispStr(x, y, "WM8978录音机", &tFont); /* 在(8,3)坐标处显示一串汉字 */
    y += usLineCap;

    /* 测试I2C设备 */
    {
        if (i2c_CheckDevice(WM8978_SLAVE_ADDRESS) == 0)
        {
            sprintf(buf, "WM8978 Ok (0x%02X)", WM8978_SLAVE_ADDRESS);
            LCD_DispStr(x, y, buf, &tFont);
        }
        else
        {
            sprintf(buf, "WM8978 Err (0x%02X)", WM8978_SLAVE_ADDRESS);

            tFont.FrontColor = CL_RED;
            LCD_DispStr(x, y, buf, &tFont);
            tFont.FrontColor = CL_WHITE;
        }
        y += usLineCap;

        tFont.FrontColor = CL_YELLOW;
        LCD_DispStr(x, y, STR_Help1, &tFont);
        y += usLineCap;

        LCD_DispStr(x, y, STR_Help2, &tFont);
        y += usLineCap;

        LCD_DispStr(x, y, STR_Help3, &tFont);
        y += usLineCap;

        tFont.FrontColor = CL_WHITE;
    }

    /* 配置外部SRAM接口已经在 bsp.c 中的 bsp_Init() 函数执行过 */

    /* 检测WM8978芯片，此函数会自动配置CPU的GPIO */
    if (!wm8978_Init())
    {
        bsp_DelayMS(500);
        return;
    }

    /* 显示按钮 */
    {
        tBtn.Font = &tFontBtn;

        tBtn.Left = BUTTON_RET_X;
        tBtn.Top = BUTTON_RET_Y;
        tBtn.Height = BUTTON_RET_H;
        tBtn.Width = BUTTON_RET_W;
        tBtn.Focus = 0; /* 未选中 */
        tBtn.pCaption = BUTTON_RET_TEXT;
        LCD_DrawButton(&tBtn);

        tBtn.Left = BUTTON_REC_X;
        tBtn.Top = BUTTON_REC_Y;
        tBtn.Height = BUTTON_REC_H;
        tBtn.Width = BUTTON_REC_W;
        tBtn.Focus = 0; /* 失去焦点 */
        tBtn.pCaption = BUTTON_REC_TEXT;
        LCD_DrawButton(&tBtn);

        tBtn.Left = BUTTON_PLAY_X;
        tBtn.Top = BUTTON_PLAY_Y;
        tBtn.Height = BUTTON_PLAY_H;
        tBtn.Width = BUTTON_PLAY_W;
        tBtn.Focus = 0; /* 失去焦点 */
        tBtn.pCaption = BUTTON_PLAY_TEXT;
        LCD_DrawButton(&tBtn);

        tBtn.Left = BUTTON_STOP_X;
        tBtn.Top = BUTTON_STOP_Y;
        tBtn.Height = BUTTON_STOP_H;
        tBtn.Width = BUTTON_STOP_W;
        tBtn.Focus = 0; /* 失去焦点 */
        tBtn.pCaption = BUTTON_STOP_TEXT;
        LCD_DrawButton(&tBtn);

        /* 显示检查框 */
        tCheck.Font = &tFontChk;

        tCheck.Left = CHECK_SPK_X;
        tCheck.Top = CHECK_SPK_Y;
        tCheck.Height = CHECK_SPK_H;
        tCheck.Width = CHECK_SPK_W;
        if (g_tRec.ucSpkOutEn == 1)
        {
            tCheck.Checked = 1;
        }
        else
        {
            tCheck.Checked = 0;
        }
        tCheck.pCaption = CHECK_SPK_TEXT;
        LCD_DrawCheckBox(&tCheck);
    }

    /* 初始化全局变量 */
    g_tRec.ucVolume = 52;    /* 缺省音量 */
    g_tRec.ucMicGain = 34; /* 缺省PGA增益 */

    fRefresh = 1;

    g_tRec.ucFmtIdx = 1;                                            /* 缺省音频格式(16Bit, 16KHz) */
    g_tRec.pAudio = (int16_t *)SDRAM_APP_BUF; /* 通过 init16_t * 型指针访问外部SRAM */

    /* 清零录音缓冲区. SRAM容量2M字节 */
    //    {
    //        int i;

    //        for (i = 0 ; i < 1 * 1024 * 1024; i++)
    //        {
    //            g_tRec.pAudio[i] = 0;
    //        }
    //    }

    /* 生成正弦波数组 */
    AUDIO_MakeSine16bit((int16_t *)REC_MEM_ADDR, 1000, 16000, 16000 * 5);

    g_tRec.ucStatus = STA_IDLE; /* 首先进入空闲状态 */

    /* 进入主程序循环体 */
    while (fQuit == 0)
    {
        bsp_Idle();

        AUDIO_Poll();

        /* 集中处理显示界面刷新 */
        if (fRefresh == 1)
        {
            fRefresh = 0;
            DispStatus(); /* 显示当前状态，频率，音量等 */
        }

        ucTouch = TOUCH_GetKey(&tpX, &tpY); /* 读取触摸事件 */
        if (ucTouch != TOUCH_NONE)
        {
            switch (ucTouch)
            {
            case TOUCH_DOWN: /* 触笔按下事件 */
                if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
                {
                    tBtn.Left = BUTTON_RET_X;
                    tBtn.Top = BUTTON_RET_Y;
                    tBtn.Height = BUTTON_RET_H;
                    tBtn.Width = BUTTON_RET_W;
                    tBtn.Focus = 1; /* 焦点 */
                    tBtn.pCaption = BUTTON_RET_TEXT;
                    LCD_DrawButton(&tBtn);
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_REC_X, BUTTON_REC_Y, BUTTON_REC_H, BUTTON_REC_W))
                {
                    tBtn.Left = BUTTON_REC_X;
                    tBtn.Top = BUTTON_REC_Y;
                    tBtn.Height = BUTTON_REC_H;
                    tBtn.Width = BUTTON_REC_W;
                    tBtn.Focus = 1; /* 焦点 */
                    tBtn.pCaption = BUTTON_REC_TEXT;
                    LCD_DrawButton(&tBtn);
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_PLAY_X, BUTTON_PLAY_Y, BUTTON_PLAY_H, BUTTON_PLAY_W))
                {
                    tBtn.Left = BUTTON_PLAY_X;
                    tBtn.Top = BUTTON_PLAY_Y;
                    tBtn.Height = BUTTON_PLAY_H;
                    tBtn.Width = BUTTON_PLAY_W;
                    tBtn.Focus = 1; /* 焦点 */
                    tBtn.pCaption = BUTTON_PLAY_TEXT;
                    LCD_DrawButton(&tBtn);
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_STOP_X, BUTTON_STOP_Y, BUTTON_STOP_H, BUTTON_STOP_W))
                {
                    tBtn.Left = BUTTON_STOP_X;
                    tBtn.Top = BUTTON_STOP_Y;
                    tBtn.Height = BUTTON_STOP_H;
                    tBtn.Width = BUTTON_STOP_W;
                    tBtn.Focus = 1; /* 焦点 */
                    tBtn.pCaption = BUTTON_STOP_TEXT;
                    LCD_DrawButton(&tBtn);
                }
                else if (TOUCH_InRect(tpX, tpY, CHECK_SPK_X, CHECK_SPK_Y, CHECK_SPK_H, CHECK_SPK_W))
                {
                    if (g_tRec.ucSpkOutEn)
                    {
                        g_tRec.ucSpkOutEn = 0;
                        tCheck.Checked = 0;

                        /* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 */
                        /* 在 StartPlay() 函数内部配置 */
                    }
                    else
                    {
                        g_tRec.ucSpkOutEn = 1;
                        tCheck.Checked = 1;

                        /* 配置WM8978芯片，输入为AUX接口(收音机)，输出为耳机 和 扬声器 */
                        /* 在 StartPlay() 函数内部配置 */
                    }

                    tCheck.Left = CHECK_SPK_X;
                    tCheck.Top = CHECK_SPK_Y;
                    tCheck.Height = CHECK_SPK_H;
                    tCheck.Width = CHECK_SPK_W;
                    tCheck.pCaption = CHECK_SPK_TEXT;
                    LCD_DrawCheckBox(&tCheck);
                }
                break;

            case TOUCH_RELEASE: /* 触笔释放事件 */
                if (TOUCH_InRect(tpX, tpY, BUTTON_RET_X, BUTTON_RET_Y, BUTTON_RET_H, BUTTON_RET_W))
                {
                    tBtn.Font = &tFontBtn;

                    tBtn.Left = BUTTON_RET_X;
                    tBtn.Top = BUTTON_RET_Y;
                    tBtn.Height = BUTTON_RET_H;
                    tBtn.Width = BUTTON_RET_W;
                    tBtn.Focus = 1; /* 焦点 */
                    tBtn.pCaption = BUTTON_RET_TEXT;
                    LCD_DrawButton(&tBtn);

                    fQuit = 1; /* 返回 */
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_REC_X, BUTTON_REC_Y, BUTTON_REC_H, BUTTON_REC_W))
                {
                    tBtn.Font = &tFontBtn;

                    tBtn.Left = BUTTON_REC_X;
                    tBtn.Top = BUTTON_REC_Y;
                    tBtn.Height = BUTTON_REC_H;
                    tBtn.Width = BUTTON_REC_W;
                    tBtn.Focus = 0; /* 失去焦点 */
                    tBtn.pCaption = BUTTON_REC_TEXT;
                    LCD_DrawButton(&tBtn);

                    StartRecord(); /* 开始录音 */
                    fRefresh = 1;
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_PLAY_X, BUTTON_PLAY_Y, BUTTON_PLAY_H, BUTTON_PLAY_W))
                {
                    tBtn.Font = &tFontBtn;

                    tBtn.Left = BUTTON_PLAY_X;
                    tBtn.Top = BUTTON_PLAY_Y;
                    tBtn.Height = BUTTON_PLAY_H;
                    tBtn.Width = BUTTON_PLAY_W;
                    tBtn.Focus = 0; /* 失去焦点 */
                    tBtn.pCaption = BUTTON_PLAY_TEXT;
                    LCD_DrawButton(&tBtn);

                    StartPlay(); /* 开始放音 */
                    fRefresh = 1;
                }
                else if (TOUCH_InRect(tpX, tpY, BUTTON_STOP_X, BUTTON_STOP_Y, BUTTON_STOP_H, BUTTON_STOP_W))
                {
                    tBtn.Left = BUTTON_STOP_X;
                    tBtn.Top = BUTTON_STOP_Y;
                    tBtn.Height = BUTTON_STOP_H;
                    tBtn.Width = BUTTON_STOP_W;
                    tBtn.Focus = 0; /* 失去焦点 */
                    tBtn.pCaption = BUTTON_STOP_TEXT;
                    LCD_DrawButton(&tBtn);

                    StopRec(); /* 停止录音和放音 */
                    fRefresh = 1;
                }
                else /* 按钮失去焦点 */
                {
                    tBtn.Font = &tFontBtn;

                    tBtn.Focus = 0; /* 未选中 */

                    tBtn.Left = BUTTON_RET_X;
                    tBtn.Top = BUTTON_RET_Y;
                    tBtn.Height = BUTTON_RET_H;
                    tBtn.Width = BUTTON_RET_W;
                    tBtn.pCaption = BUTTON_RET_TEXT;
                    LCD_DrawButton(&tBtn);

                    tBtn.Left = BUTTON_REC_X;
                    tBtn.Top = BUTTON_REC_Y;
                    tBtn.Height = BUTTON_REC_H;
                    tBtn.Width = BUTTON_REC_W;
                    tBtn.pCaption = BUTTON_REC_TEXT;
                    LCD_DrawButton(&tBtn);

                    tBtn.Left = BUTTON_PLAY_X;
                    tBtn.Top = BUTTON_PLAY_Y;
                    tBtn.Height = BUTTON_PLAY_H;
                    tBtn.Width = BUTTON_PLAY_W;
                    tBtn.pCaption = BUTTON_PLAY_TEXT;
                    LCD_DrawButton(&tBtn);

                    tBtn.Left = BUTTON_STOP_X;
                    tBtn.Top = BUTTON_STOP_Y;
                    tBtn.Height = BUTTON_STOP_H;
                    tBtn.Width = BUTTON_STOP_W;
                    tBtn.pCaption = BUTTON_STOP_TEXT;
                    LCD_DrawButton(&tBtn);
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
            case KEY_DOWN_K1: /* K1键切换音频格式，在下次开始录音和放音时有效 */
                if (++g_tRec.ucFmtIdx >= FMT_COUNT)
                {
                    g_tRec.ucFmtIdx = 0;
                }
                fRefresh = 1;
                break;

            case KEY_DOWN_K2: /* K2键按下，录音 */
                StartRecord();
                fRefresh = 1;
                break;

            case KEY_DOWN_K3: /* K3键按下，放音 */
                StartPlay();
                fRefresh = 1;
                break;

            case JOY_DOWN_U: /* 摇杆UP键按下 */
                if (g_tRec.ucVolume <= VOLUME_MAX - VOLUME_STEP)
                {
                    g_tRec.ucVolume += VOLUME_STEP;
                    wm8978_SetEarVolume(g_tRec.ucVolume);
                    wm8978_SetSpkVolume(g_tRec.ucVolume);
                    fRefresh = 1;
                }
                break;

            case JOY_DOWN_D: /* 摇杆DOWN键按下 */
                if (g_tRec.ucVolume >= VOLUME_STEP)
                {
                    g_tRec.ucVolume -= VOLUME_STEP;
                    wm8978_SetEarVolume(g_tRec.ucVolume);
                    wm8978_SetSpkVolume(g_tRec.ucVolume);
                    fRefresh = 1;
                }
                break;

            case JOY_DOWN_L: /* 摇杆LEFT键按下 */
                if (g_tRec.ucMicGain >= GAIN_STEP)
                {
                    g_tRec.ucMicGain -= GAIN_STEP;
                    wm8978_SetMicGain(g_tRec.ucMicGain);
                    fRefresh = 1;
                }
                break;

            case JOY_DOWN_R: /* 摇杆RIGHT键按下 */
                if (g_tRec.ucMicGain <= GAIN_MAX - GAIN_STEP)
                {
                    g_tRec.ucMicGain += GAIN_STEP;
                    wm8978_SetMicGain(g_tRec.ucMicGain);
                    fRefresh = 1;
                }
                break;

            case JOY_DOWN_OK: /* 摇杆OK键按下 */
                StopRec();            /* 停止录音和放音 */
                fRefresh = 1;
                break;

            default:
                break;
            }
        }

        /* 处理消息 */
        {
            MSG_T msg;

            if (bsp_GetMsg(&msg))
            {
                switch (msg.MsgCode)
                {
                case MSG_WM8978_DMA_END:
                    StopRec(); /* 停止录音和放音 */
                    fRefresh = 1;
                    break;

                default:
                    break;
                }
            }
        }
    }

    StopRec(); /* 停止录音和放音 */
}

/*
*********************************************************************************************************
*    函 数 名: StartPlay
*    功能说明: 配置WM8978和STM32的I2S开始放音。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void StartPlay(void)
{
    /* 如果已经再录音和放音状态，则需要先停止再开启 */
    AUDIO_Stop();    /* 停止I2S录音和放音 */
    wm8978_Init(); /* 复位WM8978到复位状态 */

    bsp_DelayMS(20); /* 延迟一段时间，等待I2S中断结束 */

    g_tRec.ucStatus = STA_PLAYING; /* 放音状态 */

    g_tRec.uiCursor = 0; /* 数据指针复位为0，从头开始放音 */

    /* 配置WM8978芯片，输入为DAC，输出为耳机 */
    if (g_tRec.ucSpkOutEn == 1)
    {
        wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON); /* 打开扬声器  */
    }
    else
    {
        wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON); /* 关闭扬声器 */
    }

    /* 调节音量，左右相同音量 */
    wm8978_SetEarVolume(g_tRec.ucVolume);
    wm8978_SetSpkVolume(g_tRec.ucVolume);

    /* 配置WM8978音频接口为飞利浦标准I2S接口，16bit */
    wm8978_CfgAudioIF(I2S_STANDARD_PHILIPS, 16);

    /* 配置STM32的I2S音频接口(比如飞利浦标准I2S接口，16bit， 8K采样率), 开始放音 */
    AUDIO_Init(1, I2S_STANDARD_PHILIPS, g_FmtList[g_tRec.ucFmtIdx][1], g_FmtList[g_tRec.ucFmtIdx][2]);

    {
        int16_t *pWave = (int16_t *)REC_MEM_ADDR;

        AUDIO_Play(pWave, AUDIO_GetRecordSampleCount());
    }
}

/*
*********************************************************************************************************
*    函 数 名: StartRecord
*    功能说明: 配置WM8978和STM32的I2S开始录音。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void StartRecord(void)
{
    /* 如果已经再录音和放音状态，则需要先停止再开启 */
    AUDIO_Stop();    /* 停止I2S录音和放音 */
    wm8978_Init(); /* 复位WM8978到复位状态 */

    bsp_DelayMS(20); /* 延迟一段时间，等待I2S中断结束 */

    g_tRec.ucStatus = STA_RECORDING; /* 录音状态 */

    g_tRec.uiCursor = 0; /* 数据指针复位为0，从头开始录音 */

    /* 配置WM8978芯片，输入为Mic，输出为耳机 */
    //wm8978_CfgAudioPath(MIC_LEFT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
    //wm8978_CfgAudioPath(MIC_RIGHT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
    wm8978_CfgAudioPath(MIC_LEFT_ON | MIC_RIGHT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);

    /* 调节放音音量，左右相同音量 */
    wm8978_SetEarVolume(g_tRec.ucVolume);

    /* 设置MIC通道增益 */
    wm8978_SetMicGain(g_tRec.ucMicGain);

    /* 配置WM8978音频接口为飞利浦标准I2S接口，16bit */
    wm8978_CfgAudioIF(I2S_STANDARD_PHILIPS, 16);

    /* 配置STM32的I2S音频接口(比如飞利浦标准I2S接口，16bit， 8K采样率), 开始放音 */
    AUDIO_Init(3, I2S_STANDARD_PHILIPS, g_FmtList[g_tRec.ucFmtIdx][1], g_FmtList[g_tRec.ucFmtIdx][2]);

    {
        int16_t *pWave1 = (int16_t *)REC_MEM_ADDR;
        int16_t *pWave2 = (int16_t *)(REC_MEM_ADDR + 1 * 1024 * 1024);

        AUDIO_Play(pWave2, 1 * 1024 * 1024);
        AUDIO_Record(pWave1, 1 * 1024 * 1024);
    }
}

/*
*********************************************************************************************************
*    函 数 名: StopRec
*    功能说明: 停止录音和放音
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void StopRec(void)
{
    g_tRec.ucStatus = STA_IDLE; /* 待机状态 */
    AUDIO_Stop();                                /* 停止I2S录音和放音 */
    wm8978_Init();                            /* 复位WM8978到复位状态 */
}

/*
*********************************************************************************************************
*    函 数 名: DispStatus
*    功能说明: 显示当前状态
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void DispStatus(void)
{
    FONT_T tFont;
    char buf[128];
    uint16_t x, y;

    /* 设置字体参数 */
    {
        tFont.FontCode = FC_ST_16;     /* 字体代码 16点阵 */
        tFont.FrontColor = CL_WHITE; /* 字体颜色 */
        tFont.BackColor = CL_BLUE;     /* 文字背景颜色 */
        tFont.Space = 0;                         /* 文字间距，单位 = 像素 */
    }

    x = 5;
    y = 100;
    /* 显示当前音频格式 */
    sprintf(buf, "音频格式: Philips,16Bit,%d.%dkHz     ",
                    g_FmtList[g_tRec.ucFmtIdx][2] / 1000,
                    (g_FmtList[g_tRec.ucFmtIdx][2] % 1000) / 100);

    sprintf(&buf[strlen(buf)], "录音时长: %d.%03d秒    ",
                    (REC_MEM_SIZE / 2) / g_FmtList[g_tRec.ucFmtIdx][2],
                    (((REC_MEM_SIZE / 2) * 1000) / g_FmtList[g_tRec.ucFmtIdx][2]) % 1000);

    LCD_DispStr(x, y, buf, &tFont);
    y += 18;

    sprintf(buf, "麦克风增益 = %d ", g_tRec.ucMicGain);
    sprintf(&buf[strlen(buf)], "耳机音量 = %d         ", g_tRec.ucVolume);
    LCD_DispStr(x, y, buf, &tFont);
    y += 18;

    if (g_tRec.ucStatus == STA_IDLE)
    {
        sprintf(buf, "状态 = 空闲    ");
    }
    else if (g_tRec.ucStatus == STA_RECORDING)
    {
        sprintf(buf, "状态 = 正在录音");
    }
    else if (g_tRec.ucStatus == STA_PLAYING)
    {
        sprintf(buf, "状态 = 正在回放");
    }
    LCD_DispStr(x, y, buf, &tFont);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
