/*
*********************************************************************************************************
*
*	模块名称 : 收音机测试。
*	文件名称 : fm_radio.c
*	版    本 : V1.0
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _FM_RADIO_H_
#define _FM_RADIO_H_

#define FM_RX		0
#define AM_RX		1

#define SI4704		4
#define SI4730		30

/*
	Worldwide FM band support (64–108 MHz)
	Worldwide AM band support (520–1710 kHz)

	中国范围： 88-108兆赫信号调制方式是调频（频率调制）每个频道的频率间隔是0.1兆赫
*/

typedef struct
{
	uint8_t ChipType;	/* 芯片型号代码  */
	
	uint8_t ucMode;		/* AM 或 FM */
	uint8_t ucListType;	/* 电台列表类型。武汉地区或全国 */

	uint8_t ucFMCount;	/* FM 电台个数 */
	uint16_t usFMList[255];	/* FM 电台列表 */
	uint8_t ucIndexFM;	/* 当前电台索引 */

	uint8_t ucAMCount;	/* FM 电台个数 */
	uint16_t usAMList[128];	/* FM 电台列表 */
	uint8_t ucIndexAM;	/* 当前电台索引 */

	uint32_t usFreq;	/* 当前电台频率 */
	uint8_t ucVolume;	/* 音量 */

	uint8_t ucSpkOutEn;	/* 扬声器输出使能 */
	uint8_t ucRssiEn;	/* 信号质量定时刷新 使能 */
}RADIO_T;

void RadioMain(void);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
