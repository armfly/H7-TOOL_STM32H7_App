/*
*********************************************************************************************************
*
*	模块名称 : ft5x06电容触摸芯片驱动程序
*	文件名称 : bsp_ts_ft5x06.h
*	说    明 : 头文件
*	版    本 : V1.0
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*********************************************************************************************************
*/

#ifndef _BSP_TS_FT5X06_H
#define _BSP_TS_FT5X06_H

/* I2C总线，器件ID */
#define FT5X06_I2C_ADDR       0x70

#define FT5X06_TOUCH_POINTS   5		/* 支持的触摸点数 */

/* 寄存器地址 */
#define FT5X06_REG_FW_VER     0xA6		/* 固件版本 */
#define FT5X06_REG_POINT_RATE 0x88		/* 速率 */
#define FT5X06_REG_THGROUP    0x80		/* 门槛 */

/*Chip Device Type*/
#define IC_FT5X06       0	/* x=2,3,4 */
#define IC_FT5606       1	/* ft5506/FT5606/FT5816 */
#define IC_FT5316       2	/* ft5x16 */
#define IC_FT6208       3	/* ft6208 */
#define IC_FT6x06       4	/* ft6206/FT6306 */
#define IC_FT5x06i      5	/* ft5306i */
#define IC_FT5x36       6	/* ft5336/ft5436/FT5436i */

/*register address*/
#define TS_DEVICE_MODE           0x00
#define GEST_ID               0x01
#define TD_STATUS             0x02
#define TOUCH1_XH             0x03
#define TOUCH1_XL             0x04
#define TOUCH1_YH             0x05
#define TOUCH1_YL             0x06
#define TOUCH2_XH             0x09
#define TOUCH2_XL             0x0A
#define TOUCH2_YH             0x0B
#define TOUCH2_YL             0x0C
#define TOUCH3_XH             0x0F
#define TOUCH3_XL             0x10
#define TOUCH3_YH             0x11
#define TOUCH3_YL             0x12

#define FTS_REG_CHIP_ID       0xA3	/* chip ID */
#define FTS_REG_FW_VER        0xA6	/* FW  version */
#define FTS_REG_VENDOR_ID     0xA8	/* TP vendor ID */
#define FTS_REG_POINT_RATE    0x88	/* report rate */

#define CFG_POINT_READ_BUF  (3 + 6 * (FT5X06_TOUCH_POINTS))    // 33字节

#if 0  /* 以为内容摘抄自 Linux驱动 */
struct Upgrade_Info 
{
	uint8_t CHIP_ID;
	uint8_t FTS_NAME[20];
	uint8_t TPD_MAX_POINTS;
	uint8_t AUTO_CLB;
	uint8_t delay_aa;			/* delay of write FT_UPGRADE_AA */
	uint8_t delay_55;			/* delay of write FT_UPGRADE_55 */
	uint8_t upgrade_id_1;		/* upgrade id 1 */
	uint8_t upgrade_id_2;		/* upgrade id 2 */
	uint8_t delay_readid;		/* delay of read id */
	uint8_t delay_earse_flash;	/* delay of earse flash */
};

struct Upgrade_Info fts_updateinfo[] =
{
	{0x55,"FT5x06",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x03, 10, 2000},

	{0x08,"FT5606",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 10, 0x79, 0x06, 100, 2000},
	
	{0x0a,"FT5x16",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x07, 10, 1500},

	{0x06,"FT6x06",TPD_MAX_POINTS_2,AUTO_CLB_NONEED,100, 30, 0x79, 0x08, 10, 2000},

	{0x36,"FT6x36",TPD_MAX_POINTS_2,AUTO_CLB_NONEED,10, 10, 0x79, 0x18, 10, 2000},
	{0x55,"FT5x06i",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x03, 10, 2000},
	{0x14,"FT5336",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x13,"FT3316",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x12,"FT5436i",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x11,"FT5336i",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x54,"FT5x46",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,2, 2, 0x54, 0x2c, 10, 2000},
	{0x58,"FT5x22",TPD_MAX_POINTS_5,AUTO_CLB_NONEED,2, 2, 0x58, 0x2c, 20, 2000},
	{0x59,"FT5x26",TPD_MAXPOINTS_10,AUTO_CLB_NONEED,30, 50, 0x79, 0x10, 1, 2000},
};
#endif

typedef struct
{
	uint8_t ChipID;
	uint8_t Enable;
	uint8_t TimerCount;
	
	uint8_t Count;			/* 几个点按下 */
	
	uint16_t X[FT5X06_TOUCH_POINTS];
	uint16_t Y[FT5X06_TOUCH_POINTS];	
	uint8_t id[FT5X06_TOUCH_POINTS];
	uint8_t Event[FT5X06_TOUCH_POINTS];
}FT5X06_T;

void FT5X06_InitHard(void);
uint8_t FT5X06_ReadID(void);
uint16_t FT5X06_ReadVersion(void);
void FT5X06_Scan(void);
void FT5X06_Timer1ms(void);


extern FT5X06_T g_tFT5X06;

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
