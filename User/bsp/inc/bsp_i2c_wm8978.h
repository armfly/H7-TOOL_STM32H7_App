/*
*********************************************************************************************************
*
*	模块名称 : WM8978音频芯片驱动模块
*	文件名称 : bsp_wm8978.h
*	版    本 : V1.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-03-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_WM8978_H
#define _BSP_WM8978_H

#define WM8978_SLAVE_ADDRESS 0x34 /* WM8978 I2C从机地址 */

/* WM8978 音频输入通道控制选项, 可以选择多路，比如 MIC_LEFT_ON | LINE_ON */
typedef enum
{
  IN_PATH_OFF = 0x00,  /* 无输入 */
  MIC_LEFT_ON = 0x01,  /* LIN,LIP脚，MIC左声道（接板载咪头） */
  MIC_RIGHT_ON = 0x02, /* RIN,RIP脚，MIC右声道（接GPRS模块音频输出） */
  LINE_ON = 0x04,      /* L2,R2 立体声输入 */
  AUX_ON = 0x08,       /* AUXL,AUXR 立体声输入 */
  DAC_ON = 0x10,       /* I2S数据DAC (CPU产生音频信号) */
  ADC_ON = 0x20        /* 输入的音频馈入WM8978内部ADC （I2S录音) */
} IN_PATH_E;

/* WM8978 音频输出通道控制选项, 可以选择多路 */
typedef enum
{
  OUT_PATH_OFF = 0x00, /* 无输出 */
  EAR_LEFT_ON = 0x01,  /* LOUT1 耳机左声道 */
  EAR_RIGHT_ON = 0x02, /* ROUT1 耳机右声道 */
  SPK_ON = 0x04,       /* LOUT2和ROUT2反相输出单声道,接扬声器 */
  OUT3_4_ON = 0x08,    /* OUT3 和 OUT4 输出单声道音频， 接GSM模块的音频输入 */
} OUT_PATH_E;

/* 定义最大音量 */
#define VOLUME_MAX 63 /* 最大音量 */
#define VOLUME_STEP 1 /* 音量调节步长 */

/* 定义最大MIC增益 */
#define GAIN_MAX 63 /* 最大增益 */
#define GAIN_STEP 1 /* 增益步长 */

/* STM32F429的SAI支持的音频采样频率， 8978仅支持48K */
/* 下面的采样频率对应的 MCLK = 192KHz * 256 = 49.152MHz */
#define SAI_AudioFreq_192k ((uint32_t)192000)
#define SAI_AudioFreq_96k ((uint32_t)96000)
#define SAI_AudioFreq_48k ((uint32_t)48000)
#define SAI_AudioFreq_32k ((uint32_t)32000)
#define SAI_AudioFreq_16k ((uint32_t)16000)
#define SAI_AudioFreq_8k ((uint32_t)8000)

/* 下面的采样频率对应的 MCLK = 44.1kHz x256 = 11.2896MHz */
#define SAI_AudioFreq_44_1k ((uint32_t)44100)
#define SAI_AudioFreq_22_05k ((uint32_t)22050)
#define SAI_AudioFreq_11_025k ((uint32_t)11025)

/* 供外部引用的函数声明 */
uint8_t wm8978_Init(void);

void wm8978_CfgAudioIF(uint16_t _usStandard, uint8_t _ucWordLen);
void wm8978_OutMute(uint8_t _ucMute);
void wm8978_PowerDown(void);

void wm8978_CfgAudioPath(uint16_t _InPath, uint16_t _OutPath);

void wm8978_SetMicGain(uint8_t _ucGain);
void wm8978_SetLineGain(uint8_t _ucGain);
void wm8978_SetSpkVolume(uint8_t _ucVolume);
void wm8978_SetEarVolume(uint8_t _ucVolume);
uint8_t wm8978_ReadEarVolume(void);
uint8_t wm8978_ReadSpkVolume(void);

void wm8978_NotchFilter(uint16_t _NFA0, uint16_t _NFA1);

//void AUDIO_Init(uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq);
void AUDIO_Init(uint8_t _ucMode, uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq);
uint32_t AUDIO_Play(int16_t *pBuffer, uint32_t Size);
uint32_t AUDIO_Record(int16_t *pBuffer, uint32_t Size);
void AUDIO_Pause(void);
void AUDIO_Resume(uint32_t Cmd);
void AUDIO_Stop(void);

void AUDIO_MakeSine16bit(int16_t *_outbuf, uint32_t _sin_freq, uint32_t _sample_freq, uint32_t _count);
void AUDIO_Poll(void);
uint32_t AUDIO_GetRecordSampleCount(void);

#endif
