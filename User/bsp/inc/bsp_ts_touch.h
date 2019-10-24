/*
*********************************************************************************************************
*
*	模块名称 : 电阻式触摸板驱动模块
*	文件名称 : bsp_touch.h
*	版    本 : V1.6
*	说    明 : 头文件
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_TOUCH_H
#define __BSP_TOUCH_H

/* 触摸板类型 */
enum
{
	CT_FT5X06 = 0,		/* FT系列电容触摸IC */
	CT_GT811,			/* 7寸电容触摸GT811 800 * 480 */
	CT_GT911,			/* 7寸电容触摸GT911 800 * 480 */
	
	CT_STMPE811			/* 电阻触摸 */
};

/* LCD面板类型 */
enum
{
	LCD_35_480X320 = 0,	/* 3.5寸 480 * 320 */	
	LCD_43_480X272,		/* 4.3寸 480 * 272 */
	LCD_50_480X272,		/* 5.0寸 480 * 272 */
	LCD_50_800X480,		/* 5.0寸 480 * 272 */
	LCD_70_800X480,		/* 7.0寸 800 * 480 */	
	LCD_70_1024X600,	/* 7.0寸 1024 * 600 */		
};

#define CALIB_POINT_COUNT	2		/* 2 = 2点校准； 4 = 四点校准 */

#define TOUCH_FIFO_SIZE		20

typedef struct
{
	/* 2点校准 和 4点校准 */
//	uint16_t usAdcX1;	/* 左上角 */
//	uint16_t usAdcY1;
//	uint16_t usAdcX2;	/* 右下角 */
//	uint16_t usAdcY2;
//	uint16_t usAdcX3;	/* 左下角 */
//	uint16_t usAdcY3;
//	uint16_t usAdcX4;	/* 右上角 */
//	uint16_t usAdcY4;
//	
//	uint16_t usLcdX1;	/* 左上角 */
//	uint16_t usLcdY1;
//	uint16_t usLcdX2;	/* 右下角 */
//	uint16_t usLcdY2;
//	uint16_t usLcdX3;	/* 左下角 */
//	uint16_t usLcdY3;
//	uint16_t usLcdX4;	/* 右上角 */
//	uint16_t usLcdY4;	

//	uint16_t XYChange;	/* X, Y 是否交换  */

	uint16_t usMaxAdc;	/* 触摸板最大ADC值，用于有效点判断. 最小ADC = 0  */
	uint16_t usAdcNowX;
	uint16_t usAdcNowY;

	uint8_t Enable;		/* 触摸检测使能标志 */

	uint8_t Event[TOUCH_FIFO_SIZE];	/* 触摸事件 */
	int16_t XBuf[TOUCH_FIFO_SIZE];	/* 触摸坐标缓冲区 */
	int16_t YBuf[TOUCH_FIFO_SIZE];	/* 触摸坐标缓冲区 */
	uint8_t Read;					/* 缓冲区读指针 */
	uint8_t Write;					/* 缓冲区写指针 */
}TOUCH_T;

/* 保存的电阻屏校准参数 */
typedef struct
{
	uint8_t CalibPointCount;	/* 校准点数，2表示2点校准，4表示4点校准 */	
	
	uint8_t TouchDirection;	/* 屏幕方向 0-3  0表示横屏，1表示横屏180° 2表示竖屏 3表示竖屏180° */
	
	uint8_t XYChange;		/* X, Y 是否交换， 1表示i切换，0表示不切换  */
	
	uint16_t usAdcX1;	/* 左上角 */
	uint16_t usAdcY1;
	uint16_t usAdcX2;	/* 右下角 */
	uint16_t usAdcY2;
	uint16_t usAdcX3;	/* 左下角 */
	uint16_t usAdcY3;
	uint16_t usAdcX4;	/* 右上角 */
	uint16_t usAdcY4;
	
	uint16_t usLcdX1;	/* 校准时，屏幕坐标 */
	uint16_t usLcdY1;	/* 校准时，屏幕坐标 */
	uint16_t usLcdX2;	/* 校准时，屏幕坐标 */
	uint16_t usLcdY2;	/* 校准时，屏幕坐标 */
	uint16_t usLcdX3;	/* 校准时，屏幕坐标 */
	uint16_t usLcdY3;	/* 校准时，屏幕坐标 */
	uint16_t usLcdX4;	/* 校准时，屏幕坐标 */
	uint16_t usLcdY4;	/* 校准时，屏幕坐标 */	

}TP_CALIB_PARAM_T;

/* 触摸事件 */
enum
{
	TOUCH_NONE = 0,		/* 无触摸 */
	TOUCH_DOWN = 1,		/* 按下 */
	TOUCH_MOVE = 2,		/* 移动 */
	TOUCH_RELEASE = 3	/* 释放 */
};

/* 供外部调用的函数声明 */
void TOUCH_InitHard(void);
uint8_t TOUCH_PenInt(void);
void TOUCH_Calibration(uint8_t _PointCount);
uint16_t TOUCH_ReadAdcX(void);
uint16_t TOUCH_ReadAdcY(void);
int16_t TOUCH_GetX(void);
int16_t TOUCH_GetY(void);
uint8_t TOUCH_GetKey(int16_t *_pX, int16_t *_pY);
void TOUCH_CelarFIFO(void);
uint8_t TOUCH_InRect(uint16_t _usX, uint16_t _usY,
uint16_t _usRectX, uint16_t _usRectY, uint16_t _usRectH, uint16_t _usRectW);
void TOUCH_Scan(void);
int32_t TOUCH_Abs(int32_t x);
void TOUCH_PutKey(uint8_t _ucEvent, uint16_t _usX, uint16_t _usY);
void TOUCH_CapScan(void);
void bsp_DetectLcdType(void);

extern TOUCH_T g_tTP;
extern uint8_t g_TouchType;
extern uint8_t g_LcdType;

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
