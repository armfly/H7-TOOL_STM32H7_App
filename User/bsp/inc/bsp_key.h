/*
*********************************************************************************************************
*
*	模块名称 : 按键驱动模块
*	文件名称 : bsp_key.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

/* 根据应用程序的功能重命名按键宏 */
#define KEY_DOWN_S KEY_1_DOWN
#define KEY_UP_S KEY_1_UP
#define KEY_LONG_S KEY_1_LONG

#define KEY_DOWN_C KEY_2_DOWN
#define KEY_UP_C KEY_2_UP
#define KEY_LONG_C KEY_2_LONG

/* 按键ID, 主要用于bsp_KeyState()函数的入口参数 */
typedef enum
{
	KID_S = 0,
	KID_C,
} KEY_ID_E;

/*
	按键滤波时间50ms, 单位10ms。
	只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
	即使按键电路不做硬件滤波，该滤波机制也可以保证可靠地检测到按键事件
*/
#define KEY_FILTER_TIME 5
#define KEY_LONG_TIME 50 /* 单位10ms， 持续1秒，认为长按事件 */

/*
	每个按键对应1个全局的结构体变量。
*/
typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数 */
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */

	uint8_t Count;			 /* 滤波器计数器 */
	uint16_t LongCount;	/* 长按计数器 */
	uint16_t LongTime;	 /* 按键按下持续时间, 0表示不检测长按 */
	uint8_t State;			 /* 按键当前状态（按下还是弹起） */
	uint8_t RepeatSpeed; /* 连续按键周期 */
	uint8_t RepeatCount; /* 连续按键计数器 */
} KEY_T;

/*
	定义键值代码, 必须按如下次序定时每个键的按下、弹起和长按事件

	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2) 编译器可帮我们避免键值重复。
*/
typedef enum
{
	KEY_NONE = 0, /* 0 表示按键事件 */

	KEY_1_DOWN, /* 1键按下 */
	KEY_1_UP,		/* 1键弹起 */
	KEY_1_LONG, /* 1键长按 */

	KEY_2_DOWN, /* 2键按下 */
	KEY_2_UP,		/* 2键弹起 */
	KEY_2_LONG, /* 2键长按 */
} KEY_ENUM;

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE 10
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE]; /* 键值缓冲区 */
	uint8_t Read;								/* 缓冲区读指针1 */
	uint8_t Write;							/* 缓冲区写指针 */
	uint8_t Read2;							/* 缓冲区读指针2 */
} KEY_FIFO_T;

/* 供外部调用的函数声明 */
void bsp_InitKey(void);
void bsp_KeyScan10ms(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
uint8_t bsp_GetKey2(void);
uint8_t bsp_GetKeyState(KEY_ID_E _ucKeyID);
void bsp_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t _RepeatSpeed);
void bsp_ClearKey(void);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
