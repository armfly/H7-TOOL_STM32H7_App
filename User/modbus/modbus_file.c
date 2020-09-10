/*
*********************************************************************************************************
*
*    模块名称 : modbus文件操作模块
*    文件名称 : modbus_file.c
*    版    本 : V1.0
*    说    明 : 
*
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2020-08-06 armfly  正式发布
*
*    Copyright (C), 2014-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "main.h"
#include "modbus_file.h"
#include "param.h"
#include "modbus_slave.h"
#include "lua_if.h"
#include "file_lib.h"

static void MODS64_Lua(void);
static void MODS64_EmmcFile(void);
static void MODS64_ListFile(void);
static void MODS64_GetFileMD5(void);
static void MODS64_DeleteFile(void);

static void MODS64_ReadFile(void);
static void MODS64_WriteFile(void);
static void MODS64_MakeDir(void);

/* 64H帧子功能码定义 */
enum
{
    H64_LUA_RUN_WITH_RESET = 0, /* 下载lua后复位lua环境再执行, */
    H64_LUA_RUN_NO_RESET   = 1, /* 下载lua后，不复位lua环境直接执行 */
    
    H64_LIST_FILE          = 2, /* list目录下的文件 */
    H64_GET_FILE_MD5       = 3, /* 读取文件MD5码 */
    H64_DEL_FILE           = 4, /* 删除文件 */
    H64_DEL_DIR            = 5, /* 删除目录和文件 */
    H64_READ_FILE          = 6, /* 读文件数据 */
    H64_WRITE_FILE         = 7, /* 覆盖文件（写新文件） */
    H64_CREATE_FOLDER      = 8, /* 创建文件夹 */
};

#define  H64_FILE_FUNC_END  H64_CREATE_FOLDER

/*
*********************************************************************************************************
*    函 数 名: MODS_64H
*    功能说明: 文件操作通信接口
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void MODS_64H(void)
{
    /*
        主机发送: 小程序数据
            01  ; 站号
            64  ; 功能码
            0000  ; 子功能,
                - 0表示下载lua后复位lua环境再执行, 
                - 1表示下载lua后，不复位lua环境直接执行
                - 2表示 list目录下的文件
                - 3表示读取文件MD5码
                - 4表示删除文件 del
                - 5表示删除目录和文件
                - 6表示读文件数据
                - 7表示覆盖文件（写新文件）    
            0100 0000 ; 总长度 4字节
            0000 0000 : 偏移地址 4字节
            0020 0000 : 本包数据长度 4字节
            xx ... xx : 程序数据，n个
            CCCC      : CRC16
    
        从机应答:
            01  ; 从机地址
            64  ; 功能码    
            0000  ; 子功能
    
            00  ; 执行结果，0表示OK  1表示错误
            CCCC : CRC16
    */
    uint16_t func;      /* 子功能代码 */

    g_tModS.RspCode = RSP_OK;

    if (g_tModS.RxCount < 11)
    {
        g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
        goto err_ret;
    }
    
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
    
    if (func == H64_LUA_RUN_WITH_RESET || func == H64_LUA_RUN_NO_RESET)     /* 下载lua程序 */
    {
        MODS64_Lua();         
    }
    else if (func >= H64_LIST_FILE && func <= H64_FILE_FUNC_END)
    {
        MODS64_EmmcFile();
    }
    else 
    {
        g_tModS.RspCode = RSP_ERR_VALUE; /* 数据值域错误 */
    }
    
err_ret:
    if (g_tModS.RxBuf[0] != 0x00) /* 00广播地址不应答, FF地址应答g_tParam.Addr485 */
    {
        if (g_tModS.RspCode == RSP_OK) /* 正确应答 */
        {
            MODS_SendWithCRC();
        }
        else
        {
            MODS_SendAckErr(g_tModS.RspCode); /* 告诉主机命令错误 */
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_Lua
*    功能说明: 下载和运行lua程序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_Lua(void)
{
    uint16_t func;      /* 子功能代码 */
    uint32_t total_len; /* 程序长度 */
    uint32_t offset_addr;
    uint32_t package_len; /* 本包数据长度 */
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
    total_len = BEBufToUint32(&g_tModS.RxBuf[4]);
    offset_addr = BEBufToUint32(&g_tModS.RxBuf[8]);
    package_len = BEBufToUint32(&g_tModS.RxBuf[12]);
    
    if (func == H64_LUA_RUN_WITH_RESET)  /* 1需要复位lua环境 */
    {
        if (g_Lua > 0)
        {
            lua_DeInit();
        }
        lua_Init();        
    }
    lua_DownLoad(offset_addr, &g_tModS.RxBuf[16], package_len, total_len); /* 将lua程序保存到内存 */

    if (offset_addr + package_len >= total_len)
    {
        lua_do(s_lua_prog_buf);
    }

    g_tModS.TxCount = 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
    g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;                         /* 功能码 */
    g_tModS.TxBuf[g_tModS.TxCount++] = func >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = func;
    g_tModS.TxBuf[g_tModS.TxCount++] = 0x00;                         /* 执行结果 00 */
    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 24;
    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 16;
    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 24;
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 16;
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 24;
    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 16;
    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 0;    
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_EmmcFile
*    功能说明: 操作emmc磁盘文件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_EmmcFile(void)
{
    uint16_t func;      /* 子功能代码 */
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
    
    switch (func)
    {
        case H64_LIST_FILE: /* list目录下的文件 */
            MODS64_ListFile();
            break;
        
        case H64_GET_FILE_MD5:  /* 读取文件MD5码 */
            MODS64_GetFileMD5();
            break;

        case H64_DEL_FILE:      /* 删除文件 */
            MODS64_DeleteFile();
            break;

        case H64_DEL_DIR:       /* 删除目录和文件 */
            break;
        
        case H64_READ_FILE:     /* 读文件数据 */
            MODS64_ReadFile();
            break; 
        
        case H64_WRITE_FILE:    /* 覆盖文件（写新文件） */
            MODS64_WriteFile();    
            break; 

        case H64_CREATE_FOLDER: /* 创建文件夹 */
            MODS64_MakeDir();
            break;
    }
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_ListFile
*    功能说明: 列举目录文件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_ListFile(void)
{
    uint16_t func;                  /* 子功能代码 */
    static uint32_t s_total_len;     /* 数据长度 */
    uint32_t offset_addr;
    uint32_t package_len;   /* 本包数据长度 */
    char *pData;
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
    s_total_len = BEBufToUint32(&g_tModS.RxBuf[4]);
    offset_addr = BEBufToUint32(&g_tModS.RxBuf[8]);
    package_len = BEBufToUint32(&g_tModS.RxBuf[12]);
    
    pData = (char *)&g_tModS.RxBuf[16];
    
    if (offset_addr == 0)
    {
        ListFileToMem(pData, (char *)s_lua_read_buf, LUA_READ_LEN_MAX); /* 文件列表读取到全局变量(最大4K) */
        s_total_len = strlen((char *)s_lua_read_buf);
    }
    
    /* 应答 */
    {        
        uint32_t i;
        
        g_tModS.TxCount = 0;
        g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
        g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;                         /* 功能码 */
        g_tModS.TxBuf[g_tModS.TxCount++] = func >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = func;
        g_tModS.TxBuf[g_tModS.TxCount++] = 0;
        
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 24;
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 16;
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 0;
        
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 24;
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 16;
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 0;   

        g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 24;
        g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 16;
        g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 0;         
        
        for (i = 0; i < package_len; i++)
        {
            g_tModS.TxBuf[g_tModS.TxCount++] =  s_lua_read_buf[offset_addr + i];
        }
    }   
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_GetFileMD5
*    功能说明: 获得文件的MD5码
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_GetFileMD5(void)
{
    uint16_t func;                  /* 子功能代码 */
    uint32_t package_len;
    char *pData;
    char md5[16];
    uint32_t fsize;
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
//    total_len = BEBufToUint32(&g_tModS.RxBuf[4]);
//    offset_addr = BEBufToUint32(&g_tModS.RxBuf[8]);
    package_len = BEBufToUint32(&g_tModS.RxBuf[12]);
    
    pData = (char *)&g_tModS.RxBuf[16];
    pData[package_len] = 0;
    
    fsize = GetFileMD5(pData, md5);
    
    /* 应答 */
    {        
        uint32_t i;
        
        g_tModS.TxCount = 0;
        g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;    /* 本机地址 */
        g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;                /* 功能码 */
        g_tModS.TxBuf[g_tModS.TxCount++] = func >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = func;
        g_tModS.TxBuf[g_tModS.TxCount++] = 0;        
        
        g_tModS.TxBuf[g_tModS.TxCount++] = fsize >> 24;
        g_tModS.TxBuf[g_tModS.TxCount++] = fsize >> 16;
        g_tModS.TxBuf[g_tModS.TxCount++] = fsize >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = fsize >> 0;
        
        for (i = 0; i < 16; i++)
        {
            g_tModS.TxBuf[g_tModS.TxCount++] = md5[i];
        }
    }   
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_DeleteFile
*    功能说明: 删除文件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_DeleteFile(void)
{
    uint16_t func;                  /* 子功能代码 */
    uint32_t package_len;
    char *pData;
    uint8_t error = 0;
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
//    total_len = BEBufToUint32(&g_tModS.RxBuf[4]);
//    offset_addr = BEBufToUint32(&g_tModS.RxBuf[8]);
    package_len = BEBufToUint32(&g_tModS.RxBuf[12]);
    
    pData = (char *)&g_tModS.RxBuf[16];     /* 文件名 */
    pData[package_len] = 0;
    
    error = DeleteFile(pData);
    
    /* 应答 */
    {        
        g_tModS.TxCount = 0;
        g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;    /* 本机地址 */
        g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;                /* 功能码 */
        g_tModS.TxBuf[g_tModS.TxCount++] = func >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = func;
        
        g_tModS.TxBuf[g_tModS.TxCount++] = error;
    }   
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_ReadFile
*    功能说明: 读文件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_ReadFile(void)
{
    uint16_t func;                  /* 子功能代码 */
    static uint32_t s_total_len;     /* 数据长度 */
    uint32_t offset_addr;
    uint32_t package_len;   /* 本包数据长度 */
    char *pData;
    uint32_t bytes;
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
    s_total_len = BEBufToUint32(&g_tModS.RxBuf[4]);
    offset_addr = BEBufToUint32(&g_tModS.RxBuf[8]);
    package_len = BEBufToUint32(&g_tModS.RxBuf[12]);
    
    pData = (char *)&g_tModS.RxBuf[16];
        
    if (offset_addr == 0)
    {
        s_total_len = GetFileSize(pData);
    }
    bytes = ReadFileToMem(pData, offset_addr, FsReadBuf, package_len);         
        
    /* 应答 */
    {        
        uint32_t i;
        
        g_tModS.TxCount = 0;
        g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485; /* 本机地址 */
        g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;                         /* 功能码 */
        g_tModS.TxBuf[g_tModS.TxCount++] = func >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = func;
        g_tModS.TxBuf[g_tModS.TxCount++] = 0;
        
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 24;
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 16;
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = s_total_len >> 0;
        
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 24;
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 16;
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 0;

        g_tModS.TxBuf[g_tModS.TxCount++] = bytes >> 24;
        g_tModS.TxBuf[g_tModS.TxCount++] = bytes >> 16;
        g_tModS.TxBuf[g_tModS.TxCount++] = bytes >> 8;
        g_tModS.TxBuf[g_tModS.TxCount++] = bytes >> 0;         
        
        for (i = 0; i < bytes; i++)
        {
            g_tModS.TxBuf[g_tModS.TxCount++] =  FsReadBuf[i];
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_WriteFile
*    功能说明: 写文件
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_WriteFile(void)
{
    uint16_t func;      /* 子功能代码 */
    uint32_t total_len; /* 程序长度 */
    uint32_t offset_addr;
    uint32_t package_len; /* 本包数据长度 */
    uint8_t err = 0;
    uint8_t *pData;
    static char s_FileName[256 + 1];    
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
    total_len = BEBufToUint32(&g_tModS.RxBuf[4]);
    offset_addr = BEBufToUint32(&g_tModS.RxBuf[8]);
    package_len = BEBufToUint32(&g_tModS.RxBuf[12]);
    
    pData = &g_tModS.RxBuf[16];

    /* 文件大小（4字节) + MD5 (16字节） + 文件名长度 + 文件名字符串 + 文件数据 */
    if (offset_addr == 0)
    {
        uint16_t NameLen;
        uint32_t NewFileSize;
        char *NewMD5;
        uint16_t i;        
        uint32_t OldFileSize;
        char OldMD5[16];
        uint32_t DataLen;

        NewFileSize = BEBufToUint32(pData); pData += 4;
        
        NewMD5 = (char *)pData;  pData += 16;
        
        NameLen = *pData; pData++;        
        for (i = 0; i < NameLen; i++)
        {
            s_FileName[i] = *pData++;            
        }
        s_FileName[NameLen] = 0;
        
//        printf(s_FileName); printf("\r\n");
        
        #if 0   /* 测试代码，用来验证通信速度 */
            err = 1;    
        #else
        OldFileSize = GetFileMD5(s_FileName, OldMD5); /* 获得本地文件的长度和MD5 */
        
        /* 文件长度和MD5码均相等，则不重复写入 */
        if (NewFileSize == OldFileSize && memcmp(OldMD5, NewMD5, 16) == 0)
        {
            err = 1;    /* 1表示文件相等,无需写入 */
        }
        else
        {                       
            DataLen = package_len - NameLen - 21;            
            memcpy(FsReadBuf, (char *)pData, DataLen);            
            if (WriteFile(s_FileName, 0, (char *)FsReadBuf, DataLen) == 0)
            {
                err = 0;    /* 文件写入OK */
            }
            else
            {
                err = 2;    /* 文件写入失败 */
            }
        }
        #endif
    }
    else
    {
        uint32_t DataLen;
        uint32_t offset;
        
        offset = offset_addr;
        DataLen = package_len;
        memcpy(FsReadBuf, (char *)pData, DataLen); 
        if (WriteFile(s_FileName, offset, (char *)FsReadBuf, DataLen) == 0)
        {
            err = 0;    /* 文件写入OK */
        }
        else
        {
            err = 2;    /* 文件写入失败 */
        }        
    }

    g_tModS.TxCount = 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;    /* 本机地址 */
    g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;                /* 功能码 */
    g_tModS.TxBuf[g_tModS.TxCount++] = func >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = func;
    g_tModS.TxBuf[g_tModS.TxCount++] = err;                 /* 执行结果 */ 

    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 24;
    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 16;
    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = total_len >> 0;
    
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 24;
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 16;
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = offset_addr >> 0;

    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 24;
    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 16;
    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = package_len >> 0;     
}

/*
*********************************************************************************************************
*    函 数 名: MODS64_MakeDir
*    功能说明: 创建目录
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void MODS64_MakeDir(void)
{
    uint16_t func;      /* 子功能代码 */
//    uint32_t total_len; /* 程序长度 */
//    uint32_t offset_addr;
//    uint32_t package_len; /* 本包数据长度 */
    uint8_t err = 0;
    char *pDir;
  
    func = BEBufToUint16(&g_tModS.RxBuf[2]);
//    total_len = BEBufToUint32(&g_tModS.RxBuf[4]);
//    offset_addr = BEBufToUint32(&g_tModS.RxBuf[8]);
//    package_len = BEBufToUint32(&g_tModS.RxBuf[12]);
    pDir = (char *)&g_tModS.RxBuf[16];
    
    if (MakeDir(pDir) != 0)      
    {
        err = 3;    /* 目录创建失败 */
    }
    
    g_tModS.TxCount = 0;
    g_tModS.TxBuf[g_tModS.TxCount++] = g_tParam.Addr485;    /* 本机地址 */
    g_tModS.TxBuf[g_tModS.TxCount++] = 0x64;                /* 功能码 */
    g_tModS.TxBuf[g_tModS.TxCount++] = func >> 8;
    g_tModS.TxBuf[g_tModS.TxCount++] = func;
    g_tModS.TxBuf[g_tModS.TxCount++] = err;                 /* 执行结果 00 */    
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
