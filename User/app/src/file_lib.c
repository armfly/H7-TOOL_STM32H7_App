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
#include "lcd_menu.h"
#include "main.h"
#include "prog_if.h"

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
char FsReadBuf[8*1024];
#pragma location = 0x24003000
char FsWriteBuf[8*1024] = {"FatFS Write Demo \r\n www.armfly.com \r\n"};
#pragma location = 0x24004000
uint8_t g_TestBuf[BUF_SIZE];
#elif defined(__CC_ARM)
__attribute__((section(".RAM_D1"))) FATFS fs;
__attribute__((section(".RAM_D1"))) FIL g_file;
ALIGN_32BYTES(__attribute__((section(".RAM_D1"))) char FsReadBuf[16*1024]);
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

uint8_t *g_MenuLua_Text[FILE_MAX_NUM + 1] =
{
    /* 结束符号, 用于菜单函数自动识别菜单项个数 */
    "&"
};

MENU_T g_tMenuLua;

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
*              _offset : 文件偏移地址
*              _Buff : 目标缓冲区
*              _MaxLen : 最大长度
*    返 回 值: 读到的数据长度。 0 表示失败。
*********************************************************************************************************
*/
uint32_t ReadFileToMem(char *_Path, uint32_t _offset, char *_Buff, uint32_t _MaxLen)
{
    FRESULT re;
    uint32_t br;
       
    re = f_open(&g_file, _Path, FA_OPEN_EXISTING | FA_READ);
    if (re != FR_OK)
    {
        return 0;
    }
 
    if (_offset > 0)
    {
        f_lseek(&g_file, _offset);
    }
    
    re = f_read(&g_file, _Buff, _MaxLen,  &br);
    f_close(&g_file);
    
    SCB_InvalidateDCache_by_Addr((uint32_t *)_Buff,  _MaxLen);
    
    return br;
}

/*
*********************************************************************************************************
*    函 数 名: GetFileSize
*    功能说明: 得到文件长度
*    形    参: _Path : 文件路径+文件名*             
*    返 回 值: 文件长度。0表示失败。
*********************************************************************************************************
*/
uint32_t GetFileSize(char *_Path)
{
    FRESULT re;
    uint32_t size;
       
    re = f_open(&g_file, _Path, FA_OPEN_EXISTING | FA_READ);
    if (re != FR_OK)
    {
        return 0;
    }
 
    size =  f_size(&g_file);
    
    f_close(&g_file);
    
    return size;
}

/*
*********************************************************************************************************
*    函 数 名: ini_ReadString
*    功能说明: 读ini文件中的字符串变量。简化版，不支持[]分组，需要保证每个ini参数名唯一.  字符串内容不支持引号
*    形    参: _IniFileBuf : 文件内容缓冲区
*              _ParamName : 参数名(区分大小写）
*              _pOutString : 输出结果
*    返 回 值: 无
*********************************************************************************************************
*/
void ini_ReadString(const char *_IniBuf, const char *_ParamName, char *_OutBuff, int32_t _BuffSize)
{
    /*
    Count = -1
    ProductSN = 0
    Path = "1234"
    */    
    char *p;
    
    p = strstr(_IniBuf, _ParamName);
    if (p == 0)
    {
        _OutBuff[0] = 0;
        return;
    }
    
    p = strstr(p, "=");
    if (p == 0)
    {
        _OutBuff[0] = 0;
        return;
    }

    p = strstr( p++, "\"");
    if (p == 0)
    {
        _OutBuff[0] = 0;
        return;
    }
    p++;
    
    while (1)
    {
        if (*p == 0 || *p == '"')
        {
            *_OutBuff = 0;
            break;
        }
        else
        {
            *_OutBuff++ = *p++;
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: ini_WriteString
*    功能说明: 写ini文件中的字符串变量。简化版，不支持[]分组，需要保证每个ini参数名唯一.  字符串内容不支持引号
*    形    参: _IniFileBuf : 文件内容缓冲区
*              _ParamName : 参数名(区分大小写）
*              _OutBuff : 输出结果
*              _BuffSize : 输出缓冲区大小
*    返 回 值: 无
*********************************************************************************************************
*/
void ini_WriteString(const char *_IniBuf, const char *_ParamName, const char *_NewStr, uint32_t _IniBufSize)
{
    /*
    Count = -1
    ProductSN = 0
    Path = "1234"
    */    
    char *p;
    char *p1, *p2, *p3, *p4;
    int len, len2;
    int i;
    
    p = strstr(_IniBuf, _ParamName);
    if (p == 0)
    {
        return;
    }
    
    p = strstr(p, "=");
    if (p == 0)
    {
        return;
    }    
    p1 = p + 1;

    p2 = strstr(p, "\r");
    if (p2 == 0)
    {
        return;
    }
    
    len = strlen(_NewStr);    
    if (p2 - p1 > len + 2)  /* 将后面的字符串前移 */
    {
        memcpy(p1, " \"", 2); p1 += 2;
        memcpy(p1, _NewStr, len); p1 += len;
        memcpy(p1, "\"", 1); p1 += 1;
        
        while(1)
        {
            *p1++ = *p2++;
            
            if (*p2 == 0)
            {
                break;
            }
        }
    }
    else    /* 将后面的字符串后移，挪出空间存放新字符串  p1   p2   尾部 p3 --- p4 */
    {
        len2 = strlen(p2);
        p3 = len2 + p2;
        p4 = (p1 + len + 2) + len2;
        
        if (p4 - _IniBuf > _IniBufSize)
        {
            return;
        }
        
        for (i = 0; i < len2; i++)
        {
            *p4-- = *p3--; 
        }     

        memcpy(p1, " \"", 2); p1 += 2;
        memcpy(p1, _NewStr, len); p1 += len;
        memcpy(p1, "\"", 1); p1 += 1;        
    }
}

/*
*********************************************************************************************************
*    函 数 名: ini_ReadString
*    功能说明: 读ini文件中的整数变量。简化版，不支持[]分组，需要保证每个ini参数名唯一
*    形    参: _IniFileBuf : 文件内容缓冲区
*              _ParamName : 参数名(区分大小写）
*    返 回 值: 读到的整数值。如果没找到则返回0
*********************************************************************************************************
*/
int32_t ini_ReadInteger(const char *_IniBuf, const char *_ParamName)
{
    /*
    Count = -1
    ProductSN = 0
    Path = "1234"
    */    
    char *p;
    
    p = strstr(_IniBuf, _ParamName);
    if (p == 0)
    {
        return 0;
    }
    
    p = strstr(p, "=");
    if (p == 0)
    {
        return 0;
    }
    
    return str_to_int3(p + 1);
}

/*
*********************************************************************************************************
*    函 数 名: ini_WriteInteger
*    功能说明: 写ini文件中的整数变量。简化版，不支持[]分组，需要保证每个ini参数名唯一.  字符串内容不支持引号
*    形    参: _IniFileBuf : 文件内容缓冲区
*              _ParamName : 参数名(区分大小写）
*              _OutBuff : 输出结果
*              _BuffSize : 输出缓冲区大小
*    返 回 值: 无
*********************************************************************************************************
*/
void ini_WriteInteger(const char *_IniBuf, const char *_ParamName, int _IntValue, uint32_t _IniBufSize)
{
    /*
    Count = -1
    ProductSN = 0
    Path = "1234"
    */    
    char *p;
    char *p1, *p2, *p3, *p4;
    int len, len2;
    int i;
    char NewStr[32];
    
    p = strstr(_IniBuf, _ParamName);
    if (p == 0)
    {
        return;
    }
    
    p = strstr(p, "=");
    if (p == 0)
    {
        return;
    }    
    p1 = p + 1;

    p2 = strstr(p, "\r");
    if (p2 == 0)
    {
        return;
    }
    
    sprintf(NewStr, " %d", _IntValue);
    
    len = strlen(NewStr);    
    if (p2 - p1 > len)  /* 将后面的字符串前移 */
    {
        memcpy(p1, NewStr, len); p1 += len;
        
        while(1)
        {
            *p1++ = *p2++;
            
            if (*p2 == 0)
            {
                break;
            }
        }
    }
    else    /* 将后面的字符串后移，挪出空间存放新字符串  p1   p2   尾部 p3 --- p4 */
    {
        len2 = strlen(p2);
        p3 = len2 + p2;
        p4 = (p1 + len + 0) + len2;
        
        for (i = 0; i < len2; i++)
        {
            *p4-- = *p3--; 
        }     

        memcpy(p1, NewStr, len); p1 += len;       
    }
}

/*
*********************************************************************************************************
*    函 数 名: ReadProgIniFile
*    功能说明: 读ini文件.  使用了全局变量 FsReadBuf[]
*    形    参: _Path : 文件路径+文件名 .lua文件，后缀改为.ini作为计数文件            
*    返 回 值: -1表示错误
*********************************************************************************************************
*/
int32_t ReadProgIniFile(char *_LuaPath, PROG_INI_T *pIni)
{
    FRESULT re;
    uint32_t FileSize;

    char path[256];
    uint16_t len;            
                
    len = strlen(_LuaPath);
    
    if (len > sizeof(path))
    {
        /* 文件名过长 */
        goto err_quit;
    }
    
    /* 将 *.lua 文件修改为 *.txt */
    memcpy(path, _LuaPath, len);
    path[len - 3] = 'i';
    path[len - 2] = 'n';
    path[len - 1] = 'i';    
    path[len] = 0;
    
    /*　打开ini文件 */
    re = f_open(&g_file, path, FA_OPEN_EXISTING | FA_READ);
    if (re != FR_OK)
    {
        goto err_quit;
    }
 
    /* 将ini文件全部内容读到内存 16KB */
    re = f_read(&g_file, FsReadBuf, sizeof(FsReadBuf),  &FileSize);
    if (re != FR_OK || FileSize == 0)
    {
        f_close(&g_file);
        goto err_quit;
    }   
    FsReadBuf[FileSize] = 0;
    
    f_close(&g_file);
    
    /* 解析 Locked */
    pIni->Locked = ini_ReadInteger(FsReadBuf, "Locked");
    
    /* 解析 ProgramLimit */
    pIni->ProgramLimit = ini_ReadInteger(FsReadBuf, "ProgramLimit");

    /* 解析 ProgrammedCount */
    pIni->ProgrammedCount = ini_ReadInteger(FsReadBuf, "ProgrammedCount");    
    
    /* 解析 ProductSN */
    pIni->ProductSN = ini_ReadInteger(FsReadBuf, "ProductSN");  

    /* 上次编程总时间 */
    pIni->LastTotalTime = ini_ReadInteger(FsReadBuf, "LastTotalTime");

    /* 擦除整片的时间 */
    pIni->LastEraseChipTime = ini_ReadInteger(FsReadBuf, "LastEraseChipTime");   

    return 1;

err_quit:
    pIni->Locked = 0;
    pIni->ProductSN = 0;
    pIni->ProgramLimit = 0;
    pIni->ProgrammedCount = 0;
    pIni->LastTotalTime = 0;
    pIni->LastEraseChipTime = 0;
    return -1;
}

/*
*********************************************************************************************************
*    函 数 名: WriteProgIniFile
*    功能说明: 写ini文件.  使用了全局变量 FsReadBuf[]
*    形    参: _Path : 文件路径+文件名 .lua文件，后缀改为.ini作为计数文件
*              _pIni : ini文件变量
*    返 回 值: -1表示错误  1表示OK
*********************************************************************************************************
*/
int32_t WriteProgIniFile(char *_LuaPath, PROG_INI_T *_pIni)
{
    FRESULT re;
    uint32_t FileSize;

    char path[256];
    uint16_t len;            
                
    len = strlen(_LuaPath);
    
    if (len > sizeof(path))
    {
        /* 文件名过长 */
        goto err_quit;
    }
    
    /* 将 *.lua 文件修改为 *.txt */
    memcpy(path, _LuaPath, len);
    path[len - 3] = 'i';
    path[len - 2] = 'n';
    path[len - 1] = 'i';    
    path[len] = 0;
    
    sprintf(FsReadBuf, "Locked = %d\r\n", _pIni->Locked);
    sprintf(&FsReadBuf[strlen(FsReadBuf)], "ProgramLimit = %d\r\n", _pIni->ProgramLimit);
    sprintf(&FsReadBuf[strlen(FsReadBuf)], "ProgrammedCount = %d\r\n", _pIni->ProgrammedCount);
    sprintf(&FsReadBuf[strlen(FsReadBuf)], "ProductSN = %d\r\n", _pIni->ProductSN);    
    sprintf(&FsReadBuf[strlen(FsReadBuf)], "LastTotalTime = %d\r\n", _pIni->LastTotalTime);
    sprintf(&FsReadBuf[strlen(FsReadBuf)], "LastEraseChipTime = %d\r\n", _pIni->LastEraseChipTime);
    
    /*　打开ini文件准备写 */
    re = f_open(&g_file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (re != FR_OK)
    {
        goto err_quit;
    }

    re = f_write(&g_file, FsReadBuf, strlen(FsReadBuf),  &FileSize);
    if (re != FR_OK)
    {
        goto err_quit;
    }    
       
    f_close(&g_file);   
    return 0;

err_quit:
    f_close(&g_file);   
    return -1;
}

/*
*********************************************************************************************************
*    函 数 名: LoadProgAutorunFile
*    功能说明: 读Autorun.ini文件.  使用了全局变量 FsReadBuf[]
*    形    参: _OutPath : 读到的结果保存在这个缓冲区         
*    返 回 值: -1表示错误
*********************************************************************************************************
*/
void LoadProgAutorunFile(char *_OutBuff, uint32_t _BuffSize)
{
    FRESULT re;
    uint32_t FileSize;

    /*　打开ini文件 */
    re = f_open(&g_file, PROG_AUTORUN_FILE, FA_OPEN_EXISTING | FA_READ);
    if (re != FR_OK)
    {
        goto err_quit;
    }
 
    /* 将ini文件全部内容读到内存 16KB */
    re = f_read(&g_file, FsReadBuf, sizeof(FsReadBuf),  &FileSize);
    if (re != FR_OK || FileSize == 0)
    {
        goto err_quit;
    }   
    FsReadBuf[FileSize] = 0;
    
    f_close(&g_file);

    ini_ReadString(FsReadBuf, "DefaultLuaFile", _OutBuff, _BuffSize);
    return;
 
err_quit:    
    f_close(&g_file); 
    _OutBuff[0] = 0;
    return;
}

/*
*********************************************************************************************************
*    函 数 名: SaveProgAutorunFile
*    功能说明: 更新修改Autorun.ini文件.  使用了全局变量 FsReadBuf[]
*    形    参: _OutPath : 读到的结果保存在这个缓冲区         
*    返 回 值: -1表示错误
*********************************************************************************************************
*/
void SaveProgAutorunFile(const char *_NewStr)
{
    FRESULT re;
    uint32_t FileSize;

//    /*　打开ini文件 */
//    re = f_open(&g_file, PROG_AUTORUN_FILE, FA_OPEN_EXISTING | FA_READ);
//    if (re != FR_OK)
//    {
//        goto err_quit;
//    }
// 
//    /* 将ini文件全部内容读到内存 16KB */
//    re = f_read(&g_file, FsReadBuf, sizeof(FsReadBuf),  &FileSize);
//    if (re != FR_OK || FileSize == 0)
//    {
//        f_close(&g_file);
//        goto err_quit;
//    }   
//    FsReadBuf[FileSize] = 0;
//    
//    f_close(&g_file);

//    ini_WriteString(FsReadBuf, "DefaultLuaFile", _NewStr, sizeof(FsReadBuf));


    sprintf(FsReadBuf, "DefaultLuaFile = \"%s\"", _NewStr);

    /*　打开ini文件准备写 */
    re = f_open(&g_file, PROG_AUTORUN_FILE, FA_WRITE | FA_CREATE_ALWAYS);
    if (re != FR_OK)
    {
        goto err_quit;
    }

    re = f_write(&g_file, FsReadBuf, strlen(FsReadBuf),  &FileSize);
    if (re != FR_OK)
    {

        goto err_quit;
    }     
        
err_quit:
    f_close(&g_file);
    return;
}

/*
*********************************************************************************************************
*    函 数 名: GetFileNamePostfix
*    功能说明: 获得文件名的后缀，不包括小数点。  1234.lua 则返回 "lua". 
*    形    参: _Path : 完整的路径带文件名
*              _Dir : 去掉文件名后的路径，不带斜杠
*    返 回 值: 无
*********************************************************************************************************
*/
void GetDirOfFileName(char *_Path, char *_Dir)
{
    uint16_t len;
    uint8_t i;
    
    len = strlen(_Path);
    
    for (i = len; i > 0; i--)
    {
        if (_Path[i] == '/')
        {
            len = i;
            break;
        }
    }
    
    for (i = 0; i < len; i++)
    {
        _Dir[i] = _Path[i];
    }
    _Dir[i] = 0;
}


/*
*********************************************************************************************************
*    函 数 名: GetFileNamePostfix
*    功能说明: 获得文件名的后缀，不包括小数点。  1234.lua 则返回 "lua". 
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
char *GetFileNamePostfix(char *_Path)
{
    uint16_t len;
    uint8_t i;
    
    len = strlen(_Path);
    
    for (i = len; i > 0; i--)
    {
        if (_Path[i] == '.')
        {
            break;
        }
    }
    if (i == 0)
    {
        return &_Path[len];
    }
    return &_Path[i];
}

/*
*********************************************************************************************************
*    函 数 名: CheckFileNamePostfix
*    功能说明: 判断文件名后缀是否在过滤器内
*    形    参: _Path 文件名
*              _Filter 过滤器 *.lua|*.txt  必须全部小写
*    返 回 值: 1表示在过滤器内， 0表示不在
*********************************************************************************************************
*/
uint8_t CheckFileNamePostfix(char *_Path, char *_Filter)
{
    char FixName[10];
    char *p;
    
    p = GetFileNamePostfix(_Path);          /* 获得后缀名，不包括小数点 */
    strncpy(FixName, p, sizeof(FixName) - 1);   
    
    strlwr(FixName);    /* 全部转换为小写字母 */
    
    p = strstr(_Filter, FixName);
    if (p > 0)
    {
        return 1;
    }
    
    return 0;    
}

/*
*********************************************************************************************************
*    函 数 名: FixFileName
*    功能说明: 修正文件名中..上级目录. 0:H7-TOOL/User/../Dir1 转换为 0:H7-TOOL/Dir1
*    形    参: _Path 文件名
*              _Filter 过滤器 *.lua|*.txt  必须全部小写
*    返 回 值: 无
*********************************************************************************************************
*/
void FixFileName(char *_Path)
{
    int32_t i,j;
    int32_t len;
    
    len = strlen(_Path);
    
    for (i = 0; i < len - 3; i++)
    {
        if (_Path[i] == '/' &&_Path[i + 1] == '.' && _Path[i + 2] == '.')
        {
            for (j = i - 1; j > 0; j--)
            {
                if (_Path[j] == '/')
                {
                    strcpy(&_Path[j], &_Path[i + 3]);
                    
                    len = strlen(_Path);                    
                    i = 0;  /* 重新开始处理 */
                    break;
                }
            }
        }
    }
    
}

/*
*********************************************************************************************************
*    函 数 名: ListDir
*    功能说明: lua界面，列出文件. 保存到 g_FileNameList
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void ListDir(char *_Path, char *_Filter)
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
            g_tFileList.Type[g_tFileList.Count] = 0;    /* 目录 */
        }
        else
        {
            g_tFileList.Type[g_tFileList.Count] = 1;    /* 文件 */
            
            /* 过滤掉不符合后缀要求的文件名 */
            if (CheckFileNamePostfix(FileInf.fname, _Filter) == 0)                
            {
                continue;
            }
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

/*
*********************************************************************************************************
*    函 数 名: SelectFile
*    功能说明: l文件浏览器。 用于Lua小程序和脱机编程器
*    形    参:  _InitPath :  初始路径
*               _MainStatus : 运行时的状态字
*               _RetStatus : 返回时的状态字
*               _Filter : 过滤器，*.lua
*    返 回 值: 0 表示返回， 1表示用户按了确认
*********************************************************************************************************
*/
uint8_t SelectFile(char *_InitPath, uint16_t _MainStatus, uint16_t _RetStatus, char *_Filter)
{
    uint8_t ucKeyCode; /* 按键代码 */
    uint8_t fRefresh;
    uint8_t fListDir;
    uint32_t i;
    uint8_t RetValue = 0;
    
    DispHeader("文件浏览器");
    
    LCD_DispMenu(&g_tMenuLua);

    strcpy(g_tFileList.Path, _InitPath);
    
    fRefresh = 0;
    fListDir = 1;
    while (g_MainStatus == _MainStatus)
    {
        if (fRefresh) /* 刷新整个界面 */
        {
            fRefresh = 0;
            
            LCD_ClearMenu(&g_tMenuLua);
            LCD_DispMenu(&g_tMenuLua);
        }

        if (fListDir == 1)
        {
            fListDir = 0;
            
            ListDir(g_tFileList.Path, _Filter);
            
            g_tMenuLua.Left = MENU_LEFT;
            g_tMenuLua.Top = MENU_TOP;
            g_tMenuLua.Height = MENU_HEIGHT;
            g_tMenuLua.Width = MENU_WIDTH;
            if (g_tParam.FileListFont24 == 1)   /* 24点阵显示文件列表 */
            {
                g_tMenuLua.LineCap = MENU_CAP;            
                g_tMenuLua.ViewLine = 7;
                g_tMenuLua.Font.FontCode = FC_ST_24;
            }
            else    /* 16点阵显示文件列表 */
            {
                g_tMenuLua.LineCap = MENU_CAP;            
                g_tMenuLua.ViewLine = 10;
                g_tMenuLua.Font.FontCode = FC_ST_16;               
            }
            g_tMenuLua.Font.Space = 0;            
            g_tMenuLua.RollBackEn = 1;
            g_tMenuLua.GBK = 1;
            
            for (i = 0; i < g_tFileList.Count; i++)
            {
                g_MenuLua_Text[i] = (uint8_t *)g_tFileList.Name[i];
            }
            g_MenuLua_Text[i] = "&";
                            
            LCD_InitMenu(&g_tMenuLua, (char **)g_MenuLua_Text); /* 初始化菜单结构 */
            
            if (g_tFileList.Count >= 2)
            {
                g_tMenuLua.Cursor = 1;
            }
            else
            {
                g_tMenuLua.Cursor  = 0;     /* 光标初始位置设置为返回上级 */
            }
            
            fRefresh = 1;
        }
        
        bsp_Idle();
        
        ucKeyCode = bsp_GetKey(); /* 读取键值, 无键按下时返回 KEY_NONE = 0 */
        if (ucKeyCode != KEY_NONE)
        {
            /* 有键按下 */
            switch (ucKeyCode)
            {
                case KEY_UP_S:          /* S键 上 */
                    LCD_MoveUpMenu(&g_tMenuLua);
                    break;

                case KEY_LONG_DOWN_S:   /* S键 长按 */      
                    if (g_tMenuLua.Cursor == 0)     /* 返回上级 */
                    {
                        uint8_t len;
                        
                        /* 0:/H7-TOOL/Lua */
                        len = strlen(g_tFileList.Path);
                        if (strcmp(g_tFileList.Path, _InitPath) == 0)   /* 已经在根目录了，退出 */
                        {
                            g_MainStatus = _RetStatus;
                        }
                        else    /* 还在子目录 */
                        {                        
                            for (i = len - 1; i > 2; i--)
                            {
                                if (g_tFileList.Path[i] == '/')
                                {
                                    g_tFileList.Path[i] = 0;
                                    fListDir = 1;
                                    break;
                                }
                            }
                        }
                    }
                    else 
                    {
                        if (g_tFileList.Type[g_tMenuLua.Cursor] == 0)   /* 目录 */
                        {
                            /* 路径后面追加目录 */
                            strcat(g_tFileList.Path, "/");
                            strcat(g_tFileList.Path, g_tFileList.Name[g_tMenuLua.Cursor]);
                            fListDir = 1;
                        }
                        else    /* 文件 */
                        {
                            uint16_t len;
                            char *p;
                            char ext_name[5];   /* 扩展名 */
                            
                            /* 支持 1.lua，  prog.lua */       
                            p = g_tFileList.Name[g_tMenuLua.Cursor];
                            len = strlen(p);
                            if (len >= 5)
                            {
                                memcpy(ext_name, &p[len - 4], 4);
                                ext_name[4] = 0;
                                strlwr(ext_name);   /* 转换为小写 */
                                if (strcmp(ext_name, ".lua") == 0)
                                {
                                    if (_RetStatus == MS_PROG_WORK)
                                    {                   
                                        g_MainStatus = MS_PROG_WORK;
                                        RetValue = 1;
                                    }
                                    else
                                    {
                                        g_MainStatus = MS_LUA_EXEC_FILE;
                                        RetValue = 1;
                                    }
                                    strcat(g_tFileList.Path, "/");
                                    strcat(g_tFileList.Path, g_tFileList.Name[g_tMenuLua.Cursor]);                                    
                                }
                            }
                        }
                    }
                    break;

                case KEY_UP_C:          /* C键 下 */
                    LCD_MoveDownMenu(&g_tMenuLua);
                    break;

                case KEY_LONG_DOWN_C:   /* C键长按 */
                    g_MainStatus = _RetStatus;
                    break;

                default:
                    break;
            }
        }
    } 
    return RetValue;    
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
