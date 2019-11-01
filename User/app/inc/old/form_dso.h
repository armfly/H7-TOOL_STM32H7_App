/*
*********************************************************************************************************
*
*    模块名称 : 双通道示波器程序
*    文件名称 : form_dso.c
*
*    Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __FORM_DSO_H
#define __FORM_DSO_H

#define SAMPLE_COUNT (1 * 1024) /* 采样深度，即最大样本个数 */
/*
    示波器相关的数据结构
*/
typedef struct
{
    //    uint16_t Ch1Buf[SAMPLE_COUNT];     /* 通道1数据缓冲区 */
    //    uint16_t Ch2Buf[SAMPLE_COUNT];     /* 通道2数据缓冲区 */

    uint16_t *Ch1Buf; /* 通道1数据缓冲区 */
    uint16_t *Ch2Buf; /* 通道2数据缓冲区 */

    uint32_t TimeBaseIdHold; /* 暂停时的时基 */

    uint32_t TimeBaseId; /* 时基索引, 查表可得到 us单位的时基 */
    uint32_t SampleFreq; /* 采样频率，单位Hz */
    uint32_t TimeBase;     /* 时基 查表可得到 us单位的时基 */

    uint8_t Ch1AttId;                /* CH1 衰减倍数索引 */
    uint8_t Ch2AttId;                /* CH2 衰减倍数索引 */
    int32_t Ch1Attenuation; /* 波形1幅度衰减系数(原始数据x10后，再除以这个数)  */
    int32_t Ch2Attenuation; /* 波形2幅度衰减系数(原始数据x10后，再除以这个数)  */
    uint16_t Ch1VScale;            /* 通道1垂直分度值mV单位 */
    uint16_t Ch2VScale;            /* 通道2垂直分度值mV单位 */

    uint32_t TriggerLevel;    /* 触发电平(ADC采样结果比较值) */
    uint32_t TriggerUpEdge; /* 1表示触发模式上跳沿，0表示下跳沿 */

    int16_t Ch1VOffset; /* 通道1 GND线位置, 可以为负数 */
    int16_t Ch2VOffset; /* 通道1 GND线位置, 可以为负数 */

    uint8_t ActiveCH;        /* 当前焦点通道 1表示CH1, 2表示CH2 */
    uint8_t AdjustMode; /* 当前调节模式，0表示调节幅度，1表示调节偏移 */

    /* 使用2个缓冲区完成波形的擦除和重现 */
    uint16_t xCh1Buf1[310]; /* 波形数据，坐标数组 */
    uint16_t yCh1Buf1[310]; /* 波形数据，坐标数组 */
    uint16_t xCh1Buf2[310]; /* 波形数据，坐标数组 */
    uint16_t yCh1Buf2[310]; /* 波形数据，坐标数组 */

    uint16_t xCh2Buf1[310]; /* 波形数据，坐标数组 */
    uint16_t yCh2Buf1[310]; /* 波形数据，坐标数组 */
    uint16_t xCh2Buf2[310]; /* 波形数据，坐标数组 */
    uint16_t yCh2Buf2[310]; /* 波形数据，坐标数组 */
    uint8_t BufUsed;                /* 0表示当前用Buf1， 1表示用Buf2 */

    uint8_t HoldEn; /* 波形采集暂停标志 1表示暂停，0表示运行 */

    uint8_t CH1_DC; /* 通道1 AC/DC的设置 */
    uint8_t CH2_DC; /* 通道1 AC/DC的设置 */

    uint8_t CH1_Gain; /* 通道1 增益的设置 */
    uint8_t CH2_Gain; /* 通道1 增益的设置 */
} DSO_T;

/* 
    定义支持的采样频率
*/
typedef enum
{
    SR_1K = 0, /*  */

} SAMPLE_RATE_E;

void InitDSO(void);
void SetSampRate(uint32_t freq);
void DsoMain(void);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
