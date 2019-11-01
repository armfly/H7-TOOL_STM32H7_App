/*
*********************************************************************************************************
*
*    模块名称 : 独立按键驱动模块 (外部输入IO)
*    文件名称 : bsp_key.c
*    版    本 : V1.3
*    说    明 : 扫描独立按键，具有软件滤波机制，具有按键FIFO。可以检测如下事件：
*                (1) 按键按下
*                (2) 按键弹起
*                (3) 长按键
*                (4) 长按时自动连发
*
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2013-02-01 armfly  正式发布
*        V1.1    2013-06-29 armfly  增加1个读指针，用于bsp_Idle() 函数读取系统控制组合键（截屏）
*                                   增加 K1 K2 组合键 和 K2 K3 组合键，用于系统控制
*        V1.2    2016-01-25 armfly  针对P02工控板更改. 调整gpio定义方式，更加简洁
*        V1.3    2018-11-26 armfly  s_tBtn结构赋初值0
*
*    Copyright (C), 2016-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

#define HARD_KEY_NUM 2                             /* 实体按键个数 */
#define KEY_COUNT (HARD_KEY_NUM + 0) /* 2个独立建 + 0个组合按键 */

/* 使能GPIO时钟 */
#define ALL_KEY_GPIO_CLK_ENABLE() \
    {                               \
        __HAL_RCC_GPIOF_CLK_ENABLE(); \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    };

/* 依次定义GPIO */
typedef struct
{
    GPIO_TypeDef *gpio;
    uint16_t pin;
    uint8_t ActiveLevel; /* 激活电平 */
} X_GPIO_T;

/* GPIO和PIN定义 */
static const X_GPIO_T s_gpio_list[HARD_KEY_NUM] = {
        {GPIOF, GPIO_PIN_2, 0}, /* S键 */
        {GPIOG, GPIO_PIN_0, 0}, /* C键 */
};

/* 定义一个宏函数简化后续代码 
    判断GPIO引脚是否有效按下
*/
static KEY_T s_tBtn[KEY_COUNT] = {0};
static KEY_FIFO_T s_tKey; /* 按键FIFO变量,结构体 */

static void bsp_InitKeyVar(void);
static void bsp_InitKeyHard(void);
static void bsp_DetectKey(uint8_t i);

#define KEY_PIN_ACTIVE(id)

/*
*********************************************************************************************************
*    函 数 名: KeyPinActive
*    功能说明: 判断按键是否按下
*    形    参: 无
*    返 回 值: 返回值1 表示按下(导通），0表示未按下（释放）
*********************************************************************************************************
*/
static uint8_t KeyPinActive(uint8_t _id)
{
    uint8_t level;

    if ((s_gpio_list[_id].gpio->IDR & s_gpio_list[_id].pin) == 0)
    {
        level = 0;
    }
    else
    {
        level = 1;
    }

    if (level == s_gpio_list[_id].ActiveLevel)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
*********************************************************************************************************
*    函 数 名: IsKeyDownFunc
*    功能说明: 判断按键是否按下。单键和组合键区分。单键事件不允许有其他键按下。
*    形    参: 无
*    返 回 值: 返回值1 表示按下(导通），0表示未按下（释放）
*********************************************************************************************************
*/
static uint8_t IsKeyDownFunc(uint8_t _id)
{
    /* 实体单键 */
    if (_id < HARD_KEY_NUM)
    {
        uint8_t i;
        uint8_t count = 0;
        uint8_t save = 255;

        /* 判断有几个键按下 */
        for (i = 0; i < HARD_KEY_NUM; i++)
        {
            if (KeyPinActive(i))
            {
                count++;
                save = i;
            }
        }

        if (count == 1 && save == _id)
        {
            return 1; /* 只有1个键按下时才有效 */
        }

        return 0;
    }

    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: bsp_InitKey
*    功能说明: 初始化按键. 该函数被 bsp_Init() 调用。
*    形    参:  无
*    返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitKey(void)
{
    bsp_InitKeyVar();    /* 初始化按键变量 */
    bsp_InitKeyHard(); /* 初始化按键硬件 */
}

/*
*********************************************************************************************************
*    函 数 名: bsp_InitKeyHard
*    功能说明: 配置按键对应的GPIO
*    形    参:  无
*    返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitKeyHard(void)
{
    GPIO_InitTypeDef gpio_init;
    uint8_t i;

    /* 第1步：打开GPIO时钟 */
    ALL_KEY_GPIO_CLK_ENABLE();

    /* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
    gpio_init.Mode = GPIO_MODE_INPUT;                         /* 设置输入 */
    gpio_init.Pull = GPIO_NOPULL;                                 /* 上下拉电阻不使能 */
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* GPIO速度等级 */

    for (i = 0; i < HARD_KEY_NUM; i++)
    {
        gpio_init.Pin = s_gpio_list[i].pin;
        HAL_GPIO_Init(s_gpio_list[i].gpio, &gpio_init);
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_InitKeyVar
*    功能说明: 初始化按键变量
*    形    参:  无
*    返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitKeyVar(void)
{
    uint8_t i;

    /* 对按键FIFO读写指针清零 */
    s_tKey.Read = 0;
    s_tKey.Write = 0;
    s_tKey.Read2 = 0;

    /* 给每个按键结构体成员变量赋一组缺省值 */
    for (i = 0; i < KEY_COUNT; i++)
    {
        s_tBtn[i].LongTime = KEY_LONG_TIME;         /* 长按时间 0 表示不检测长按键事件 */
        s_tBtn[i].Count = KEY_FILTER_TIME / 2; /* 计数器设置为滤波时间的一半 */
        s_tBtn[i].State = 0;                                     /* 按键缺省状态，0为未按下 */
        s_tBtn[i].RepeatSpeed = 0;                         /* 按键连发的速度，0表示不支持连发 */
        s_tBtn[i].RepeatCount = 0;                         /* 连发计数器 */
    }

    /* 如果需要单独更改某个按键的参数，可以在此单独重新赋值 */

    /* 摇杆上下左右，支持长按1秒后，自动连发 */
    bsp_SetKeyParam(KID_S, 100, 6);
    bsp_SetKeyParam(KID_C, 100, 6);
}

/*
*********************************************************************************************************
*    函 数 名: bsp_PutKey
*    功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*    形    参:  _KeyCode : 按键代码
*    返 回 值: 无
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
    s_tKey.Buf[s_tKey.Write] = _KeyCode;

    if (++s_tKey.Write >= KEY_FIFO_SIZE)
    {
        s_tKey.Write = 0;
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_GetKey
*    功能说明: 从按键FIFO缓冲区读取一个键值。
*    形    参: 无
*    返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
    uint8_t ret;

    if (s_tKey.Read == s_tKey.Write)
    {
        return KEY_NONE;
    }
    else
    {
        ret = s_tKey.Buf[s_tKey.Read];

        if (++s_tKey.Read >= KEY_FIFO_SIZE)
        {
            s_tKey.Read = 0;
        }
        return ret;
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_GetKey2
*    功能说明: 从按键FIFO缓冲区读取一个键值。独立的读指针。
*    形    参:  无
*    返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t bsp_GetKey2(void)
{
    uint8_t ret;

    if (s_tKey.Read2 == s_tKey.Write)
    {
        return KEY_NONE;
    }
    else
    {
        ret = s_tKey.Buf[s_tKey.Read2];

        if (++s_tKey.Read2 >= KEY_FIFO_SIZE)
        {
            s_tKey.Read2 = 0;
        }
        return ret;
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_GetKeyState
*    功能说明: 读取按键的状态
*    形    参:  _ucKeyID : 按键ID，从0开始
*    返 回 值: 1 表示按下， 0 表示未按下
*********************************************************************************************************
*/
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID)
{
    return s_tBtn[_ucKeyID].State;
}

/*
*********************************************************************************************************
*    函 数 名: bsp_SetKeyParam
*    功能说明: 设置按键参数
*    形    参：_ucKeyID : 按键ID，从0开始
*            _LongTime : 长按事件时间
*             _RepeatSpeed : 连发速度
*    返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t _RepeatSpeed)
{
    s_tBtn[_ucKeyID].LongTime = _LongTime;             /* 长按时间 0 表示不检测长按键事件 */
    s_tBtn[_ucKeyID].RepeatSpeed = _RepeatSpeed; /* 按键连发的速度，0表示不支持连发 */
    s_tBtn[_ucKeyID].RepeatCount = 0;                         /* 连发计数器 */
}

/*
*********************************************************************************************************
*    函 数 名: bsp_ClearKey
*    功能说明: 清空按键FIFO缓冲区
*    形    参：无
*    返 回 值: 按键代码
*********************************************************************************************************
*/
void bsp_ClearKey(void)
{
    s_tKey.Read = s_tKey.Write;
}

/*
*********************************************************************************************************
*    函 数 名: bsp_DetectKey
*    功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*    形    参: IO的id， 从0开始编码
*    返 回 值: 无
*********************************************************************************************************
*/
static void bsp_DetectKey(uint8_t i)
{
    KEY_T *pBtn;

    pBtn = &s_tBtn[i];
    if (IsKeyDownFunc(i))
    {
        if (pBtn->Count < KEY_FILTER_TIME)
        {
            pBtn->Count = KEY_FILTER_TIME;
        }
        else if (pBtn->Count < 2 * KEY_FILTER_TIME)
        {
            pBtn->Count++;
        }
        else
        {
            if (pBtn->State == 0)
            {
                pBtn->State = 1;

                /* 发送按钮按下的消息 */
                bsp_PutKey((uint8_t)(3 * i + 1));
            }

            if (pBtn->LongTime > 0)
            {
                if (pBtn->LongCount < pBtn->LongTime)
                {
                    /* 发送按钮持续按下的消息 */
                    if (++pBtn->LongCount == pBtn->LongTime)
                    {
                        /* 键值放入按键FIFO */
                        bsp_PutKey((uint8_t)(3 * i + 3));
                    }
                }
                else
                {
                    if (pBtn->RepeatSpeed > 0)
                    {
                        if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
                        {
                            pBtn->RepeatCount = 0;
                            /* 常按键后，每隔10ms发送1个按键 */
                            bsp_PutKey((uint8_t)(3 * i + 1));
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (pBtn->Count > KEY_FILTER_TIME)
        {
            pBtn->Count = KEY_FILTER_TIME;
        }
        else if (pBtn->Count != 0)
        {
            pBtn->Count--;
        }
        else
        {
            if (pBtn->State == 1)
            {
                pBtn->State = 0;

                /* 发送按钮弹起的消息 */
                bsp_PutKey((uint8_t)(3 * i + 2));
            }
        }

        pBtn->LongCount = 0;
        pBtn->RepeatCount = 0;
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_DetectFastIO
*    功能说明: 检测高速的输入IO. 1ms刷新一次
*    形    参: IO的id， 从0开始编码
*    返 回 值: 无
*********************************************************************************************************
*/
static void bsp_DetectFastIO(uint8_t i)
{
    KEY_T *pBtn;

    pBtn = &s_tBtn[i];
    if (IsKeyDownFunc(i))
    {
        if (pBtn->State == 0)
        {
            pBtn->State = 1;

            /* 发送按钮按下的消息 */
            bsp_PutKey((uint8_t)(3 * i + 1));
        }

        if (pBtn->LongTime > 0)
        {
            if (pBtn->LongCount < pBtn->LongTime)
            {
                /* 发送按钮持续按下的消息 */
                if (++pBtn->LongCount == pBtn->LongTime)
                {
                    /* 键值放入按键FIFO */
                    bsp_PutKey((uint8_t)(3 * i + 3));
                }
            }
            else
            {
                if (pBtn->RepeatSpeed > 0)
                {
                    if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
                    {
                        pBtn->RepeatCount = 0;
                        /* 常按键后，每隔10ms发送1个按键 */
                        bsp_PutKey((uint8_t)(3 * i + 1));
                    }
                }
            }
        }
    }
    else
    {
        if (pBtn->State == 1)
        {
            pBtn->State = 0;

            /* 发送按钮弹起的消息 */
            bsp_PutKey((uint8_t)(3 * i + 2));
        }

        pBtn->LongCount = 0;
        pBtn->RepeatCount = 0;
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_KeyScan10ms
*    功能说明: 扫描所有按键。非阻塞，被systick中断周期性的调用，10ms一次
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void bsp_KeyScan10ms(void)
{
    uint8_t i;

    for (i = 0; i < KEY_COUNT; i++)
    {
        bsp_DetectKey(i);
    }
}

/*
*********************************************************************************************************
*    函 数 名: bsp_KeyScan1ms
*    功能说明: 扫描所有按键。非阻塞，被systick中断周期性的调用，1ms一次.
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void bsp_KeyScan1ms(void)
{
    uint8_t i;

    for (i = 0; i < KEY_COUNT; i++)
    {
        bsp_DetectFastIO(i);
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
