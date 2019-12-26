/*
*********************************************************************************************************
*                                      
*    模块名称 : 文件操作接口函数
*    文件名称 : file_lib.h
*
*********************************************************************************************************
*/

#ifndef __FILE_LIB_H
#define __FILE_LIB_H

#include "ff.h" /* FatFS文件系统模块*/
#include "ff_gen_drv.h"
#include "sd_diskio_dma.h"

/* 静态分配，单个目录下最多100个文件 */
#define FILE_MAX_NUM        100
#define FILE_NAME_MAX_LEN   32      /* 32字符, 只显示前面16字符 */

typedef struct
{
    /* 文件类型（目录或文件）列表数组，静态分配 */
    uint8_t Type[FILE_MAX_NUM];
    char Name[FILE_MAX_NUM][FILE_NAME_MAX_LEN + 1];
    uint8_t Count;   
    char Path[200];     
}FILE_LIST_T;

extern FILE_LIST_T g_tFileList;


extern FATFS fs;
extern FIL g_file;
extern char FsReadBuf[1024];
extern char FsWriteBuf[1024];

extern DIR DirInf;
extern FILINFO FileInf;
extern char DiskPath[4];   /* SD卡逻辑驱动路径，比盘符0，就是"0:/" */

void FileSystemLoad(void);
void FileSystemUnLoad(void);
uint8_t CreateNewFile(char *_FileName);
uint8_t CloseFile(FIL *_file);
uint8_t DeleteFile(char *_Path);
uint8_t ReadFile(FIL* fp, void* buff, UINT btr, UINT* br);
uint8_t WriteFile(FIL* fp, const void* buff, UINT btw, UINT* bw);
uint8_t OpenFile(FIL* fp, char *_Path);
void ListDir(char *_Path);
uint32_t ReadFileToMem(char *_Path, char *_Buff, uint32_t _MaxLen);
#endif
