/*
*********************************************************************************************************
*
*    模块名称 : 文件系统模块
*    文件名称 : file_lib.c
*    版    本 : V1.0
*    说    明 : 提供文件系统相关的API函数。文件系统采用FatFS。
*
*    修改记录 :
*        版本号   日期         作者        说明
*        V1.0    2019-12-12    armfly    正式发布
*
*    Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "file_lib.h"

/*
    1、V7开发板的SD卡接口是用的SDMMC1，而这个接口仅支持AXI SRAM区访问，其它SRAM和TCP均不支持。
    2、AXI SRAM的主频是200MHz，需要Cache配合提速，所以此例子主RAM直接使用性能最高的DTCM。仅DMA
       操作SDIO的地方使用AXI SRAM，下面这几个变量是都需要使用AXI SRAM空间的。
    3、详情在此贴进行了说明：http://forum.armfly.com/forum.php?mod=viewthread&tid=91531
*/
#if defined(__ICCARM__)
#pragma location = 0x24000000
FATFS fs;
#pragma location = 0x24001000
FIL file;
#pragma location = 0x24002000
char FsReadBuf[1024];
#pragma location = 0x24003000
char FsWriteBuf[1024] = {"FatFS Write Demo \r\n www.armfly.com \r\n"};
#pragma location = 0x24004000
uint8_t g_TestBuf[BUF_SIZE];
#elif defined(__CC_ARM)
__attribute__((section(".RAM_D1"))) FATFS fs;
__attribute__((section(".RAM_D1"))) FIL g_file;
ALIGN_32BYTES(__attribute__((section(".RAM_D1"))) char FsReadBuf[1024]);
ALIGN_32BYTES(__attribute__((section(".RAM_D1"))) char FsWriteBuf[1024]) = {"FatFS Write Demo \r\n www.armfly.com \r\n"};
#endif

DIR DirInf;
FILINFO FileInf;
char DiskPath[4];   /* SD卡逻辑驱动路径，比盘符0，就是"0:/" */

/* FatFs API的返回值 */
#if 0
static const char *FR_Table[] =
{
    "FR_OK：成功",                                                /* (0) Succeeded */
    "FR_DISK_ERR：底层硬件错误",                                  /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR：断言失败",                                       /* (2) Assertion failed */
    "FR_NOT_READY：物理驱动没有工作",                             /* (3) The physical drive cannot work */
    "FR_NO_FILE：文件不存在",                                     /* (4) Could not find the file */
    "FR_NO_PATH：路径不存在",                                     /* (5) Could not find the path */
    "FR_INVALID_NAME：无效文件名",                                /* (6) The path name format is invalid */
    "FR_DENIED：由于禁止访问或者目录已满访问被拒绝",              /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST：文件已经存在",                                     /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT：文件或者目录对象无效",                    /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED：物理驱动被写保护",                       /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE：逻辑驱动号无效",                           /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED：卷中无工作区",                               /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM：没有有效的FAT卷",                          /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",                /* (14) The f_mkfs() aborted due to any parameter error */
    "FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",             /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED：由于文件共享策略操作被拒绝",                      /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",                 /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE",      /* (18) Number of open files > _FS_SHARE */
    "FR_INVALID_PARAMETER：参数无效"                              /* (19) Given parameter is invalid */
};
#endif

FILE_LIST_T g_tFileList;

/*
*********************************************************************************************************
*    函 数 名: FileSystemLoad
*    功能说明: 文件系统加载
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void FileSystemLoad(void)
{
    FRESULT result;
    
    /* 注册SD卡驱动 */
    FATFS_LinkDriver(&SD_Driver, DiskPath);

    /* 挂载文件系统 */
    result = f_mount(&fs, DiskPath, 0); /* Mount a logical drive */
    if (result != FR_OK)
    {
        //printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
    }  
}

/*
*********************************************************************************************************
*    函 数 名: FileSystemUnLoad
*    功能说明: 文件系统卸载
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void FileSystemUnLoad(void)
{
    /* 卸载文件系统 */
    f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*    函 数 名: CreateNewFile
*    功能说明: 创建一个新文件，并关闭.
*    形    参: 文件名 _FileName, 必须带全路径
*    返 回 值: 0成功
*********************************************************************************************************
*/
uint8_t CreateNewFile(char *_FileName)
{
    FRESULT result;

    /* 打开文件 */
    result = f_open(&g_file, _FileName, FA_CREATE_ALWAYS | FA_WRITE);
    if (result != FR_OK)
    {
        return result;
    }

    /* 关闭文件*/
    f_close(&g_file);
    
    return result;
}

/*
*********************************************************************************************************
*    函 数 名: CloseFile
*    功能说明: 关闭文件
*    形    参: 文件名 _FileName, 必须带全路径
*    返 回 值: 0成功
*********************************************************************************************************
*/
uint8_t CloseFile(FIL *_file)
{
    /* 关闭文件*/
    return f_close(_file);
}

/*
*********************************************************************************************************
*    函 数 名: DeleteFile
*    功能说明: 删除目录或文件
*    形    参: 文件名 _FileName, 必须带全路径
*    返 回 值: 0成功
*********************************************************************************************************
*/
uint8_t DeleteFile(char *_Path)
{
    FRESULT result;
    
    result = f_unlink(_Path);
    if (result == FR_OK)
    {
        //printf("删除子目录/Dir1/Dir1_1成功\r\n");       
    }
    else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
    {
        //printf("没有发现文件或目录 :%s\r\n", "/Dir1/Dir1_1");
    }
    else
    {
        //printf("删除子目录/Dir1/Dir1_1失败(错误代码 = %d) 文件只读或目录非空\r\n", result);
    }
    
    return result;
}

/*
*********************************************************************************************************
*    函 数 名: ReadFileToMem
*    功能说明: 读取完整的文件到内存
*    形    参: _Path : 文件路径+文件名
*              _Buff : 目标缓冲区
*              _MaxLen : 最大长度
*    返 回 值: 读到的数据长度。 0 表示失败。
*********************************************************************************************************
*/
uint32_t ReadFileToMem(char *_Path, char *_Buff, uint32_t _MaxLen)
{
    FRESULT re;
    uint32_t br;
       
    re = f_open(&g_file, _Path, FA_OPEN_EXISTING | FA_READ);
    if (re != FR_OK)
    {
        return 0;
    }
        
    re = f_read(&g_file, _Buff, _MaxLen,  &br);
    f_close(&g_file);
    
    return br;
}

/*
*********************************************************************************************************
*    函 数 名: WriteFile
*    功能说明: 读取文件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
FRESULT WriteFile(FIL* fp, const void* buff, UINT btw, UINT* bw)
{
    return f_write(fp, buff, btw, bw);
}

/*
*********************************************************************************************************
*    函 数 名: OpenFile
*    功能说明: 打开一个已存在的文件进行读写
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
FRESULT OpenFile(FIL* fp, char *_Path)
{
    //f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);    
    return f_open(fp, _Path, FA_OPEN_EXISTING | FA_READ);
}

/*
*********************************************************************************************************
*    函 数 名: ListDir
*    功能说明: lua界面，列出文件. 保存到 g_FileNameList
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void ListDir(char *_Path)
{
    FRESULT result;
    uint32_t cnt = 0;
    uint32_t i;
    const char strBack[] = {0xA1,0xFB,0xB7,0xB5,0xBB,0xD8,0xC9,0xCF,0xBC,0xB6,0};  /* "←返回上级" */
    
    strcpy(g_tFileList.Name[0], strBack);     /* 此处中文用GBK编码 返回上级  */
    g_tFileList.Count = 1;
    
    /* 打开根文件夹 */
    result = f_opendir(&DirInf, _Path); /* 如果不带参数，则从当前目录开始 */
    if (result != FR_OK)
    {
        //printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
        return;
    }

    printf("info        |  filesize | name1 | name2\r\n");
    for (cnt = 0;; cnt++)
    {
        result = f_readdir(&DirInf, &FileInf); /* 读取目录项，索引会自动下移 */
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
            g_tFileList.Type[g_tFileList.Count] = 0;
        }
        else
        {
            g_tFileList.Type[g_tFileList.Count] = 1;
        }
        
        for (i = 0; i < FILE_NAME_MAX_LEN; i++)
        {
            if (FileInf.fname[i] == 0)
            {
                break;
            }
            g_tFileList.Name[g_tFileList.Count][i] = FileInf.fname[i];
        }
        g_tFileList.Name[g_tFileList.Count][i] = 0;
        g_tFileList.Count++;
    }
    
    f_closedir(&DirInf);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
