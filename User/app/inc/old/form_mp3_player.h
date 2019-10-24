/*
*********************************************************************************************************
*
*	模块名称 : 等待开发的程序界面
*	文件名称 : reserve.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __MP3_PLAYER_H
#define __MP3_PLAYER_H

/* 定义一个用于MP3播放器的结构体
便于全局变量操作
*/
typedef struct
{
	uint8_t ucMuteOn;			/* 0 : 静音， 1: 放音 */
	uint8_t ucVolume;			/* 当前音量 */
	uint32_t uiProgress;		/* 当前进度(已读取的字节数) */
	uint8_t ucPauseEn;			/* 暂停使能 */

	uint8_t ListCount;			/* 播放列表的歌曲个数 */
	uint8_t ListIndex;			/* 当前歌曲索引 */
}MP3_T;

/* 歌曲列表 */
typedef struct
{
	char FileName[13];		/* 8+3结构文件名 */
	uint32_t FileSize;		/* 文件长度 */
	char LenFileName[256];	/* 长文件名, 可以用于显示 */
}PLAY_LIST_T;

void MP3Player(void);

#endif

