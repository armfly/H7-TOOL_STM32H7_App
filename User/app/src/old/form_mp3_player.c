/*
*********************************************************************************************************
*
*	模块名称 : MP3播放器界面
*	文件名称 : "mp3_player.h"
*	版    本 : V1.1
*	说    明 : 测试VS1053 MP3模块
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2013-02-01 armfly  首发
*		V1.1    2015-10-17 armfly  f_opendir后必须f_closedir
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "form_mp3_player.h"
#include "ff.h"				/* FatFS 文件系统头文件 */
#include "ff_gen_drv.h"
#include "sd_diskio_dma.h"

/* 自动播放指定磁盘指定目录下的MP3文件 */
#define MP3_FOLDER	"/Music"			/* MP3文件夹, 不含磁盘盘符 */

/* 定义界面结构 */
typedef struct
{
	FONT_T FontBlack;	/* 静态的文字 */
	FONT_T FontBlue;	/* 变化的文字字体 */
	FONT_T FontBtn;		/* 按钮的字体 */
	FONT_T FontBox;		/* 分组框标题字体 */

	GROUP_T Box1;

	LABEL_T Label1;	LABEL_T Label2;
	LABEL_T Label3; LABEL_T Label4;
	LABEL_T Label5; LABEL_T Label6;
	LABEL_T Label7; LABEL_T Label8;

	LABEL_T Label9;
	BUTTON_T BtnRet;
}FormMP3_T;

/* 窗体背景色 */
#define FORM_BACK_COLOR		CL_BTN_FACE

/* 框的坐标和大小 */
#define BOX1_X	5
#define BOX1_Y	8
#define BOX1_H	(g_LcdHeight - BOX1_Y - 10)
#define BOX1_W	(g_LcdWidth -  2 * BOX1_X)
#define BOX1_TEXT	"MP3模块测试程序"

/* 返回按钮的坐标(屏幕右下角) */
#define BTN_RET_H	32
#define BTN_RET_W	60
#define	BTN_RET_X	((BOX1_X + BOX1_W) - BTN_RET_W - 4)
#define	BTN_RET_Y	((BOX1_Y  + BOX1_H) - BTN_RET_H - 4)
#define	BTN_RET_TEXT	"返回"

#define LABEL1_X  	(BOX1_X + 6)
#define LABEL1_Y	(BOX1_Y + 20)
#define LABEL1_TEXT	"芯片型号 :"

	#define LABEL2_X  	(LABEL1_X + 90)
	#define LABEL2_Y	LABEL1_Y
	#define LABEL2_TEXT	"  "

#define LABEL3_X  	(LABEL1_X)
#define LABEL3_Y	(LABEL1_Y + 20)
#define LABEL3_TEXT	"输出音量 :"

	#define LABEL4_X  	(LABEL3_X + 90)
	#define LABEL4_Y	(LABEL3_Y)
	#define LABEL4_TEXT	" "

#define LABEL5_X  	(LABEL1_X)
#define LABEL5_Y	(LABEL1_Y + 20 * 2)
#define LABEL5_TEXT	"播放进度 : "

	#define LABEL6_X  	(LABEL5_X + 90)
	#define LABEL6_Y	LABEL5_Y
	#define LABEL6_TEXT	" "

#define LABEL7_X  	(LABEL1_X)
#define LABEL7_Y	(LABEL1_Y + 20 * 3)
#define LABEL7_TEXT	"文件名   : "

	#define LABEL8_X  	(LABEL7_X + 90)
	#define LABEL8_Y	LABEL7_Y
	#define LABEL8_TEXT	"---"


#define LABEL9_X  	LABEL1_X
#define LABEL9_Y	120
#define LABEL9_TEXT	"请将MP3文件放到SD卡Music目录下"

#define SONG_LIST_MAX	10			/* 歌曲列表最大数目 */


static void InitFormMP3(void);
static void DispFormMP3(void);
static void FillSongList(void);
static void MP3HardInit(void);
static void ViewDir(char *_path);
static uint8_t Mp3Pro(void);
static void Mp3DispStatus(void);

FormMP3_T *FormMP3;

MP3_T g_tMP3;
PLAY_LIST_T g_tPlayList[SONG_LIST_MAX];

/* 访问Fatfs用到的全局变量 */
FATFS   g_fs;
FIL 	g_Mp3File;

char DiskPath[4]; /* 保存FatFS 磁盘路径 */

/*
*********************************************************************************************************
*	函 数 名: ReserveFunc
*	功能说明: 保留功能。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void MP3Player(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t ucTouch;		/* 触摸事件 */
	uint8_t fQuit = 0;
	int16_t tpX, tpY;
	FormMP3_T form;
	uint8_t ucRefresh;
	uint8_t ucNextSong;

	FormMP3= &form;

	InitFormMP3();
	DispFormMP3();
	MP3HardInit();		/* 配置VS1053B硬件和WM8978硬件 */
	
	//FATFS_LinkDriver(&USBH_Driver, DiskPath);
	FATFS_LinkDriver(&SD_Driver, DiskPath);
	
	/* 挂载文件系统 */
	if (f_mount(&g_fs, DiskPath, 0) != FR_OK)
	{
		printf("f_mount文件系统失败");
	}

#if 1
	/* 打印NAND Flash 根目录和MP3目录下的文件 */
	{
		char path[60];

		sprintf(path, "%s", DiskPath);		/* 根目录 */
		ViewDir(path);

		sprintf(path, "%s%s", DiskPath, MP3_FOLDER);	/* 音乐目录 */
		ViewDir(path);
	}
#endif
	
	FillSongList();		/* 搜索NAND Flash 根目录下的MP3文件，并填充到播放列表数组 */

	ucRefresh = 1;
	g_tMP3.ucPauseEn = 0;	/* 缺省开始播放 */

	if (g_tMP3.ListCount > 0)
	{
		ucNextSong = 1;			/* 定位下一首歌曲的标志 */
	}
	else
	{
		ucNextSong = 0;
	}
	g_tMP3.ListIndex = 0;
	bsp_StartAutoTimer(0, 100);		/* 100ms 刷新进度条 */
	/* 进入主程序循环体 */
	while (fQuit == 0)
	{
		bsp_Idle();

		/* 打开歌曲文件 */
		if (ucNextSong == 1)
		{
			ucNextSong = 0;

			/* 关闭上一个文件*/
			f_close(&g_Mp3File);

			printf("\r\n\r\n");		/* 换一行显示 */

			/* 打开MUSIC目录下的mp3 文件 */
			{
				char FileName[256];
				FRESULT result;

				sprintf(FileName, "%s%s/%s", DiskPath, MP3_FOLDER, g_tPlayList[g_tMP3.ListIndex].FileName);
				result = f_open(&g_Mp3File, FileName, FA_OPEN_EXISTING | FA_READ);
				if (result !=  FR_OK)
				{
					printf("Open MP3 file Error, %s\r\n", g_tPlayList[g_tMP3.ListIndex].FileName);
					FormMP3->Label8.pCaption = "打开MP3文件失败";
					LCD_DrawLabel(&FormMP3->Label8);
				}
				else
				{
					printf("正在播放: %s\r\n", g_tPlayList[g_tMP3.ListIndex].FileName);

					/* 显示文件名 */
					FormMP3->Label8.pCaption = g_tPlayList[g_tMP3.ListIndex].FileName;
					LCD_DrawLabel(&FormMP3->Label8);
				}
			}

			g_tMP3.uiProgress = 0;	/* 进度 */
		}

		if (g_tMP3.ucPauseEn == 0)
		{
			if (Mp3Pro() == 1)
			{
				/* 歌曲播放完毕，自动切换到下一首歌 */

				if (g_tMP3.ListCount > 0)			
				{
					/* 模拟一个摇杆右键按下，执行换歌操作 */
					bsp_PutKey(JOY_DOWN_R);
				}
			}
		}

		/* 刷新状态栏 */
		if ((ucRefresh == 1) || (bsp_CheckTimer(0)))
		{
			ucRefresh = 0;
			Mp3DispStatus();		/* 显示当前状态，音量等 */
		}

		ucTouch = TOUCH_GetKey(&tpX, &tpY);	/* 读取触摸事件 */
		if (ucTouch != TOUCH_NONE)
		{
			switch (ucTouch)
			{
				case TOUCH_DOWN:		/* 触笔按下事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormMP3->BtnRet.Focus = 1;
						LCD_DrawButton(&FormMP3->BtnRet);
					}
					break;

				case TOUCH_RELEASE:		/* 触笔释放事件 */
					if (TOUCH_InRect(tpX, tpY, BTN_RET_X, BTN_RET_Y, BTN_RET_H, BTN_RET_W))
					{
						FormMP3->BtnRet.Focus = 0;
						LCD_DrawButton(&FormMP3->BtnRet);
						fQuit = 1;	/* 返回 */
					}
					else	/* 按钮失去焦点 */
					{
						FormMP3->BtnRet.Focus = 0;
						LCD_DrawButton(&FormMP3->BtnRet);
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
				case KEY_DOWN_K1:			/* K1键按下 */
					if (g_tMP3.ucPauseEn == 0)
					{
						g_tMP3.ucPauseEn = 1;
					}
					else
					{
						g_tMP3.ucPauseEn = 0;
					}
					ucRefresh = 1;
					break;

				case KEY_DOWN_K2:			/* K2键按下 */
					f_lseek(&g_Mp3File, 0);	/* 修改文件当前指针到文件头, 从头开始播放 */
					g_tMP3.uiProgress = 0;	/* 进度 */
					ucRefresh = 1;
					break;

				case KEY_DOWN_K3:			/* K3键按下 */
					if (g_tMP3.ucMuteOn == 1)
					{
						g_tMP3.ucMuteOn = 0;
						VS1053_SetBASS(0, 0, 0, 0);		/* 原音,低音不增强 */
					}
					else
					{
						g_tMP3.ucMuteOn = 1;
						VS1053_SetBASS(0, 0, 10, 100);	/* 设置低音增强(截止频率100Hz)，高音不变 */
					}
					ucRefresh = 1;
					break;

				case JOY_DOWN_D:		/* 摇杆DOWN键按下 */
					if (g_tMP3.ucVolume > VS_VOL_MIN)
					{
						g_tMP3.ucVolume--;
						VS1053_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case JOY_DOWN_U:		/* 摇杆UP键按下 */
					if (g_tMP3.ucVolume < VS_VOL_MAX)
					{
						g_tMP3.ucVolume++;
						VS1053_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case JOY_DOWN_L:		/* 摇杆LEFT键按下 */
					if (g_tMP3.ListIndex > 0)
					{
						g_tMP3.ListIndex--;		/* 前一首歌 */
					}
					else
					{
						g_tMP3.ListIndex = g_tMP3.ListCount - 1;	/* 循环 */
					}
					ucRefresh = 1;
					ucNextSong = 1;	 /* 打开下一首歌曲 */
					break;

				case JOY_DOWN_R:		/* 摇杆RIGHT键按下 */
					if (g_tMP3.ListIndex < g_tMP3.ListCount - 1)
					{
						g_tMP3.ListIndex++;	/* 下一首歌 */
					}
					else
					{
						g_tMP3.ListIndex = 0;	/* 循环 */
					}
					ucRefresh = 1;
					ucNextSong = 1;	 /* 打开下一首歌曲 */
					break;

				case JOY_DOWN_OK:		/* 摇杆OK键按下 */
					ucRefresh = 1;
					break;

				default:
					break;
			}
		}
	}

	bsp_StopTimer(0);	/* 关闭自动定时器 */

	/* 关闭文件*/
	f_close(&g_Mp3File);

	/* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
	
	FATFS_UnLinkDriver(DiskPath);	/* 卸载驱动 */
}

/*
*********************************************************************************************************
*	函 数 名: InitFormMP3
*	功能说明: 初始化控件属性
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitFormMP3(void)
{
	/* 分组框标题字体 */
	FormMP3->FontBox.FontCode = FC_ST_16;
	FormMP3->FontBox.BackColor = CL_BTN_FACE;	/* 和背景色相同 */
	FormMP3->FontBox.FrontColor = CL_BLACK;
	FormMP3->FontBox.Space = 0;

	/* 字体1 用于静止标签 */
	FormMP3->FontBlack.FontCode = FC_ST_16;
	FormMP3->FontBlack.BackColor = CL_MASK;		/* 透明色 */
	FormMP3->FontBlack.FrontColor = CL_BLACK;
	FormMP3->FontBlack.Space = 0;

	/* 字体2 用于变化的文字 */
	FormMP3->FontBlue.FontCode = FC_ST_16;
	FormMP3->FontBlue.BackColor = CL_BTN_FACE;
	FormMP3->FontBlue.FrontColor = CL_BLUE;
	FormMP3->FontBlue.Space = 0;

	/* 按钮字体 */
	FormMP3->FontBtn.FontCode = FC_ST_16;
	FormMP3->FontBtn.BackColor = CL_MASK;		/* 透明背景 */
	FormMP3->FontBtn.FrontColor = CL_BLACK;
	FormMP3->FontBtn.Space = 0;

	/* 分组框 */
	FormMP3->Box1.Left = BOX1_X;
	FormMP3->Box1.Top = BOX1_Y;
	FormMP3->Box1.Height = BOX1_H;
	FormMP3->Box1.Width = BOX1_W;
	FormMP3->Box1.pCaption = BOX1_TEXT;
	FormMP3->Box1.Font = &FormMP3->FontBox;

	/* 静态标签 */
	FormMP3->Label1.Left = LABEL1_X;
	FormMP3->Label1.Top = LABEL1_Y;
	FormMP3->Label1.MaxLen = 0;
	FormMP3->Label1.pCaption = LABEL1_TEXT;
	FormMP3->Label1.Font = &FormMP3->FontBlack;

	FormMP3->Label3.Left = LABEL3_X;
	FormMP3->Label3.Top = LABEL3_Y;
	FormMP3->Label3.MaxLen = 0;
	FormMP3->Label3.pCaption = LABEL3_TEXT;
	FormMP3->Label3.Font = &FormMP3->FontBlack;

	FormMP3->Label5.Left = LABEL5_X;
	FormMP3->Label5.Top = LABEL5_Y;
	FormMP3->Label5.MaxLen = 0;
	FormMP3->Label5.pCaption = LABEL5_TEXT;
	FormMP3->Label5.Font = &FormMP3->FontBlack;

	FormMP3->Label7.Left = LABEL7_X;
	FormMP3->Label7.Top = LABEL7_Y;
	FormMP3->Label7.MaxLen = 0;
	FormMP3->Label7.pCaption = LABEL7_TEXT;
	FormMP3->Label7.Font = &FormMP3->FontBlack;

	FormMP3->Label9.Left = LABEL9_X;
	FormMP3->Label9.Top = LABEL9_Y;
	FormMP3->Label9.MaxLen = 0;
	FormMP3->Label9.pCaption = LABEL9_TEXT;
	FormMP3->Label9.Font = &FormMP3->FontBlack;

	/* 动态标签 */
	FormMP3->Label2.Left = LABEL2_X;
	FormMP3->Label2.Top = LABEL2_Y;
	FormMP3->Label2.MaxLen = 0;
	FormMP3->Label2.pCaption = LABEL2_TEXT;
	FormMP3->Label2.Font = &FormMP3->FontBlue;

	FormMP3->Label4.Left = LABEL4_X;
	FormMP3->Label4.Top = LABEL4_Y;
	FormMP3->Label4.MaxLen = 0;
	FormMP3->Label4.pCaption = LABEL4_TEXT;
	FormMP3->Label4.Font = &FormMP3->FontBlue;

	FormMP3->Label6.Left = LABEL6_X;
	FormMP3->Label6.Top = LABEL6_Y;
	FormMP3->Label6.MaxLen = 0;
	FormMP3->Label6.pCaption = LABEL6_TEXT;
	FormMP3->Label6.Font = &FormMP3->FontBlue;

	FormMP3->Label8.Left = LABEL8_X;
	FormMP3->Label8.Top = LABEL8_Y;
	FormMP3->Label8.MaxLen = 0;
	FormMP3->Label8.pCaption = LABEL8_TEXT;
	FormMP3->Label8.Font = &FormMP3->FontBlue;

	/* 按钮 */
	FormMP3->BtnRet.Left = BTN_RET_X;
	FormMP3->BtnRet.Top = BTN_RET_Y;
	FormMP3->BtnRet.Height = BTN_RET_H;
	FormMP3->BtnRet.Width = BTN_RET_W;
	FormMP3->BtnRet.pCaption = BTN_RET_TEXT;
	FormMP3->BtnRet.Font = &FormMP3->FontBtn;
	FormMP3->BtnRet.Focus = 0;
}

/*
*********************************************************************************************************
*	函 数 名: DispFormMP3
*	功能说明: 显示所有的静态控件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispFormMP3(void)
{
	LCD_ClrScr(CL_BTN_FACE);

	/* 分组框 */
	LCD_DrawGroupBox(&FormMP3->Box1);

	/* 静态标签 */
	LCD_DrawLabel(&FormMP3->Label1);
	LCD_DrawLabel(&FormMP3->Label3);
	LCD_DrawLabel(&FormMP3->Label5);
	LCD_DrawLabel(&FormMP3->Label7);

	LCD_DrawLabel(&FormMP3->Label9);	

	/* 动态标签 */
	LCD_DrawLabel(&FormMP3->Label2);
	LCD_DrawLabel(&FormMP3->Label4);
	LCD_DrawLabel(&FormMP3->Label6);
	LCD_DrawLabel(&FormMP3->Label8);

	/* 按钮 */
	LCD_DrawButton(&FormMP3->BtnRet);
}


/*
*********************************************************************************************************
*	函 数 名: MP3HardInit
*	功能说明: 配置MP3播放相关的硬件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MP3HardInit(void)
{
	/* 配置VS1053硬件 */
	{
		VS1053_Init();
		
		/* 等待芯片内部操作完成 */
		if (VS1053_WaitTimeOut())
		{
			/* 如果没有插VS1053B模块，DREQ口线将返回低电平，这是一种异常情况 */
			FormMP3->Label2.pCaption = "没有检测到MP3模块硬件";
			LCD_DrawLabel(&FormMP3->Label2);		/* 显示芯片型号 */

			return;
		}

		
		VS1053_SoftReset();

		/* 打印MP3解码芯片型号 */
		{
			char *pModel;

			switch (VS1053_ReadChipID())
			{
				case VS1001:
					pModel = "VS1001";
					break;

				case VS1011:
					pModel = "VS1011";
					break;

				case VS1002:
					pModel = "VS1002";
					break;

				case VS1003:
					pModel = "VS1003";
					break;

				case VS1053:
					pModel = "VS1053";
					break;

				case VS1033:
					pModel = "VS1033";
					break;

				case VS1103:
					pModel = "VS1103";
					break;

				default:
					pModel = "unknow";
					break;
			}
			FormMP3->Label2.pCaption = pModel;
			LCD_DrawLabel(&FormMP3->Label2);		/* 显示芯片型号 */
		}

		g_tMP3.ucVolume = 200; 			/* 缺省音量,越大声音越小 */
		VS1053_SetVolume(g_tMP3.ucVolume);

		VS1053_SetBASS(0, 0, 0, 0);		/* 高频和低音不增强 */
	}

	/* 配置WM8978音频通道, 可以将VS1053B的输出音频接入WM8978的Line输入插座， 驱动板子上的扬声器放音 */
	{
		/* bsp.c 中已经初始化I2C总线 */
		wm8978_Init();		/* 复位WM8978到复位状态 */

		wm8978_SetSpkVolume(0);	/* 临时静音扬声器 */

		/* 配置WM8978芯片，输入为LINE IN，输出为耳机和扬声器 */
		wm8978_CfgAudioPath(LINE_ON, EAR_LEFT_ON | EAR_RIGHT_ON | SPK_ON);
		/* 调节放音音量，左右相同音量 */
		wm8978_SetEarVolume(30);	/* 设置耳机音量，最大63 */
		wm8978_SetSpkVolume(60);	/* 设置扬声器音量，最大63 */
		wm8978_SetLineGain(6);		/* 设置Line输入增益， 0-7 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: FillSongList
*	功能说明: 填充歌曲播放列表。搜索NAND Flash根目录下的歌曲，最多10个
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void FillSongList(void)
{
	/* 访问Fatfs用到的全局变量 */
	FRESULT result;
	FILINFO FileInf;
	DIR DirInf;
	char path[128];

	/* 打开歌曲目录 */
	sprintf(path, "%s%s", DiskPath, MP3_FOLDER);
	result = f_opendir(&DirInf, path); 	/* path可以带盘符，最后一个字符不能是/ */
	if (result != FR_OK)
	{
		printf("Open Root Directory Error (%d)\r\n", result);
	}

	g_tMP3.ListIndex = 0;
	g_tMP3.ListCount = 0;	/* 歌曲个数 */
	for(;;)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')	/* 表示目录 */
		{
			continue;
		}

		if (FileInf.fattrib != AM_DIR)
		{
			uint8_t Len;
			//altname   fname
			Len = strlen(FileInf.altname);
			if (Len >= 5)
			{
				if (memcmp(&FileInf.altname[Len - 3], "MP3", 3) == 0)
				{
					/* 复制MP3文件名到播放列表 */
					strcpy(g_tPlayList[g_tMP3.ListCount].FileName, FileInf.altname);
					g_tPlayList[g_tMP3.ListCount].FileSize = FileInf.fsize;
					g_tMP3.ListCount++;		/* 歌曲个数 */

					/* 如果MP3文件已填满，则退出 */
					if (g_tMP3.ListCount > SONG_LIST_MAX)
					{
						break;
					}

				}
			}
		}
	}

	if (g_tMP3.ListCount == 0)
	{
		printf("没有在根目录下找到 MP3 文件\r\n");
	}
}

/*
*********************************************************************************************************
*	函 数 名: Mp3Pro
*	功能说明: MP3文件播放，在主程序while循环中调用. 每次向VS105B发送32字节。
*	形    参: 无
*	返 回 值: 0 表示正常播放; 1 表示文件播放完毕,主程序据此切换到下一首歌曲
*********************************************************************************************************
*/
static uint8_t Mp3Pro(void)
{
	uint32_t bw,i;
	char buf[32];

	/* 如果VS1003空闲，则写入新的数据 */
	if (VS1053_ReqNewData())
	{
		f_read(&g_Mp3File, &buf, 32, &bw);
		if (bw <= 0)
		{
			return 1;
		}

		/* 计算进度 */
		g_tMP3.uiProgress += bw;

		VS1053_PreWriteData();	/* 写数据准备，设置好片选 */
		for (i = 0; i < bw; i++)
		{
			VS1053_WriteData(buf[i]);
		}
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: Mp3DispStatus
*	功能说明: 显示当前状态
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void Mp3DispStatus(void)
{
	char buf[5];

	/* 显示音量 */
	sprintf(buf, "%3d ", g_tMP3.ucVolume);
	FormMP3->Label4.pCaption = buf;
	LCD_DrawLabel(&FormMP3->Label4);

	/* 显示进度 */
	sprintf(buf, "%3d%%", g_tMP3.uiProgress * 100 / g_tPlayList[g_tMP3.ListIndex].FileSize);
	FormMP3->Label6.pCaption = buf;
	LCD_DrawLabel(&FormMP3->Label6);
}

/*
*********************************************************************************************************
*	函 数 名: ViewDir
*	功能说明: 显示根目录下的文件名
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ViewDir(char *_path)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;

	/* 打开根文件夹 - 用完后需要 f_closedir  */
	result = f_opendir(&DirInf, _path); /* 1: 表示盘符 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	printf("\r\n当前目录：%s\r\n", _path);

	/* 读取当前文件夹下的文件和目录 */
	printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)目录  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)文件  ", FileInf.fattrib);
		}

		/* 打印文件大小, 最大4G */
		printf(" %10d", FileInf.fsize);

		printf("  %s |", FileInf.altname);	/* 短文件名 */

		printf("  %s\r\n", (char *)FileInf.fname);	/* 长文件名 */
	}
	
	f_closedir(&DirInf);	/*　关闭打开的目录 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
