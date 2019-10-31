/*
*********************************************************************************************************
*
*	模块名称 : 汉字点阵字库。有效显示区 高16x宽15, 最右一列留白
*	文件名称 : hz16.c
*	版    本 : V1.0
*	说    明 : 只包含本程序用到汉字字库
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-09-08 armfly  ST固件库V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
  FLASH中内嵌小字库，只包括本程序用到的汉字点阵
  每行点阵数据，头2字节是汉子的内码，后面32字节是24点阵汉子的字模数据。
*/

unsigned char const g_Ascii24[] =
    {
        0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x10, 0x80, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, // 0 //
        0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0,
        0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x10, 0x80, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x06, 0x00, 0x0E, 0x00, 0x1E, 0x00, 0x26, 0x00, // 1 //
        0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00,
        0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x1F, 0x80, 0x30, 0xC0, 0x20, 0xC0, 0x00, 0xC0, // 2 //
        0x00, 0xC0, 0x01, 0x80, 0x01, 0x80, 0x03, 0x00, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x0C, 0x00,
        0x0C, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x1F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x01, 0x80, 0x03, 0x00, 0x02, 0x00, 0x06, 0x00, // 3 //
        0x0C, 0x00, 0x08, 0x00, 0x18, 0x00, 0x06, 0x00, 0x03, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
        0x01, 0x80, 0x01, 0x80, 0x41, 0x80, 0x3F, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0x00, 0x03, 0x00, 0x02, 0x00, 0x06, 0x00, // 4 //
        0x04, 0x00, 0x0C, 0x00, 0x09, 0x80, 0x19, 0x80, 0x11, 0x80, 0x31, 0x80, 0x21, 0x80, 0x61, 0x80,
        0x7F, 0xE0, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, // 5 //
        0x30, 0x00, 0x30, 0x00, 0x3C, 0x00, 0x03, 0x00, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
        0x01, 0x80, 0x01, 0x80, 0x41, 0x80, 0x3F, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x08, 0x00, 0x18, 0x00, // 6 //
        0x10, 0x00, 0x30, 0x00, 0x36, 0x00, 0x39, 0x00, 0x31, 0x80, 0x31, 0x80, 0x31, 0x80, 0x31, 0x80,
        0x31, 0x80, 0x31, 0x80, 0x31, 0x80, 0x11, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x00, // 7 //
        0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00,
        0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x11, 0x00, 0x31, 0x80, 0x31, 0x80, 0x31, 0x80, // 8 //
        0x31, 0x80, 0x31, 0x80, 0x11, 0x00, 0x0E, 0x00, 0x11, 0x00, 0x31, 0x80, 0x31, 0x80, 0x31, 0x80,
        0x31, 0x80, 0x31, 0x80, 0x31, 0x80, 0x11, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x11, 0x00, 0x31, 0x80, 0x31, 0x80, 0x31, 0x80, // 9 //
        0x31, 0x80, 0x31, 0x80, 0x31, 0x80, 0x31, 0x80, 0x13, 0x80, 0x0D, 0x80, 0x01, 0x80, 0x01, 0x80,
        0x03, 0x00, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // . //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x18, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x41, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x0B, 0x00, 0x0B, 0x00, 0x0B, 0x00, 0x11, 0x80, // A //
        0x11, 0x80, 0x11, 0x80, 0x11, 0x80, 0x3F, 0xC0, 0x20, 0xC0, 0x20, 0xC0, 0x20, 0xC0, 0x40, 0x60,
        0x40, 0x60, 0x40, 0x60, 0xF1, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x6D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0xC0, // m //
        0xF7, 0x60, 0x66, 0x60, 0x66, 0x60, 0x66, 0x60, 0x66, 0x60, 0x66, 0x60, 0x66, 0x60, 0x66, 0x60,
        0x66, 0x60, 0x66, 0x60, 0xF6, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x43, 0x00, 0x00, 0x0F, 0x20, 0x18, 0xE0, 0x30, 0x60, 0x30, 0x20, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, // C //
        0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20,
        0x30, 0x40, 0x18, 0xC0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x4D, 0x00, 0x00, 0xE0, 0x70, 0x60, 0x60, 0x70, 0xE0, 0x70, 0xE0, 0x50, 0xE0, 0x50, 0xE0, 0x59, 0x60, // M //
        0x59, 0x60, 0x49, 0x60, 0x49, 0x60, 0x4D, 0x60, 0x4E, 0x60, 0x46, 0x60, 0x46, 0x60, 0x46, 0x60,
        0x40, 0x60, 0x40, 0x60, 0xE0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x4F, 0x00, 0x00, 0x06, 0x00, 0x1F, 0x80, 0x30, 0xC0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, // O //
        0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
        0x30, 0xC0, 0x1F, 0x80, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x41, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x0B, 0x00, 0x0B, 0x00, 0x0B, 0x00, 0x11, 0x80, // A //
        0x11, 0x80, 0x11, 0x80, 0x11, 0x80, 0x3F, 0xC0, 0x20, 0xC0, 0x20, 0xC0, 0x20, 0xC0, 0x40, 0x60,
        0x40, 0x60, 0x40, 0x60, 0xF1, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x42, 0x00, 0x00, 0xFF, 0x00, 0x31, 0x80, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, // B //
        0x31, 0x80, 0x3F, 0x00, 0x31, 0xC0, 0x30, 0xE0, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60,
        0x30, 0xE0, 0x31, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x43, 0x00, 0x00, 0x0F, 0x20, 0x18, 0xE0, 0x30, 0x60, 0x30, 0x20, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, // C //
        0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20,
        0x30, 0x40, 0x18, 0xC0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // a //
        0x19, 0x80, 0x30, 0xC0, 0x30, 0xC0, 0x03, 0xC0, 0x0C, 0xC0, 0x18, 0xC0, 0x30, 0xC0, 0x30, 0xC0,
        0x31, 0xC0, 0x3A, 0xD0, 0x1C, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x64, 0x00, 0x80, 0x03, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0D, 0x80, // d //
        0x33, 0x80, 0x61, 0x80, 0x61, 0x80, 0x61, 0x80, 0x61, 0x80, 0x61, 0x80, 0x61, 0x80, 0x61, 0x80,
        0x61, 0x80, 0x33, 0xA0, 0x0C, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x68, 0x10, 0x00, 0x70, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x33, 0x80, // h //
        0x34, 0xC0, 0x38, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0,
        0x30, 0xC0, 0x30, 0xC0, 0xF9, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x80, // n //
        0x74, 0xC0, 0x38, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0,
        0x30, 0xC0, 0x30, 0xC0, 0xF9, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xC0, // r //
        0x3D, 0xE0, 0x0E, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x0C, 0x00,
        0x0C, 0x00, 0x0C, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40, // u //
        0x71, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0,
        0x31, 0xC0, 0x32, 0xC0, 0x1C, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //   //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // : //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x18, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x18, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x44, 0x00, 0x00, 0x7E, 0x00, 0x31, 0x80, 0x30, 0xC0, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, // D //
        0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60,
        0x30, 0xC0, 0x31, 0x80, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x47, 0x00, 0x00, 0x1E, 0x40, 0x31, 0xC0, 0x20, 0xC0, 0x60, 0x40, 0x60, 0x40, 0x60, 0x00, 0x60, 0x00, // G //
        0x60, 0x00, 0x60, 0x00, 0x63, 0xF0, 0x60, 0xC0, 0x60, 0xC0, 0x60, 0xC0, 0x60, 0xC0, 0x60, 0xC0,
        0x60, 0xC0, 0x21, 0x40, 0x1E, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x49, 0x00, 0x00, 0x1F, 0x80, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, // I //
        0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00,
        0x06, 0x00, 0x06, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x50, 0x00, 0x00, 0xFF, 0x80, 0x30, 0xC0, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, 0x30, 0x60, // P //
        0x30, 0xC0, 0x3F, 0x80, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x30, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x46, 0x00, 0x00, 0xFF, 0xE0, 0x30, 0x60, 0x30, 0x20, 0x30, 0x20, 0x30, 0x00, 0x30, 0x80, 0x30, 0x80, // F //
        0x31, 0x80, 0x3F, 0x80, 0x31, 0x80, 0x30, 0x80, 0x30, 0x80, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x30, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x57, 0x00, 0x00, 0xF0, 0x70, 0x62, 0x20, 0x62, 0x20, 0x62, 0x20, 0x63, 0x20, 0x63, 0x20, 0x23, 0x20, // W //
        0x35, 0x20, 0x35, 0xA0, 0x35, 0xA0, 0x35, 0xA0, 0x35, 0xA0, 0x35, 0xA0, 0x14, 0xA0, 0x18, 0xC0,
        0x18, 0xC0, 0x18, 0xC0, 0x18, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x69, 0x04, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x06, 0x00, // i //
        0x0E, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00,
        0x06, 0x00, 0x06, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x45, 0x00, 0x00, 0xFF, 0xE0, 0x30, 0x60, 0x30, 0x20, 0x30, 0x00, 0x30, 0x00, 0x30, 0x80, 0x30, 0x80, // E //
        0x31, 0x80, 0x3F, 0x80, 0x31, 0x80, 0x30, 0x80, 0x30, 0x80, 0x30, 0x00, 0x30, 0x00, 0x30, 0x20,
        0x30, 0x20, 0x30, 0x60, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x53, 0x00, 0x00, 0x0F, 0x20, 0x30, 0xE0, 0x60, 0x60, 0x60, 0x20, 0x60, 0x20, 0x30, 0x00, 0x18, 0x00, // S //
        0x0C, 0x00, 0x03, 0x00, 0x01, 0x80, 0x00, 0xC0, 0x00, 0x60, 0x00, 0x60, 0x40, 0x60, 0x40, 0x60,
        0x60, 0x40, 0x70, 0x80, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x55, 0x00, 0x00, 0xFC, 0x70, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, // U //
        0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20, 0x30, 0x20,
        0x30, 0x20, 0x18, 0x40, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x48, 0x00, 0x00, 0xF9, 0xF0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, // H //
        0x30, 0xC0, 0x3F, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0, 0x30, 0xC0,
        0x30, 0xC0, 0x30, 0xC0, 0xF9, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x56, 0x00, 0x00, 0xF0, 0x70, 0x60, 0x20, 0x60, 0x20, 0x30, 0x40, 0x30, 0x40, 0x30, 0x40, 0x30, 0x40, // V //
        0x18, 0x80, 0x18, 0x80, 0x18, 0x80, 0x19, 0x00, 0x0D, 0x00, 0x0D, 0x00, 0x0D, 0x00, 0x06, 0x00,
        0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // - //
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x4B, 0x00, 0x00, 0xF9, 0xF0, 0x60, 0x40, 0x60, 0x80, 0x61, 0x00, 0x61, 0x00, 0x62, 0x00, 0x64, 0x00, // K //
        0x68, 0x00, 0x7C, 0x00, 0x6C, 0x00, 0x66, 0x00, 0x66, 0x00, 0x63, 0x00, 0x63, 0x00, 0x61, 0x80,
        0x61, 0x80, 0x60, 0xC0, 0xF8, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x30, 0x00, // > //
        0x0C, 0x00, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x30, 0x00, 0xC0, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
        0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        /* 最后一行必须用0xFF,0xFF结束，这是字库数组结束标志 */
        0xFF, 0xFF};

unsigned char const g_Ascii24_VarWidth[] =
    {
        // 土耳其C大写 //
        0x43, 0x11, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XF0, 0X00, 0X0F, 0XFC, 0X00, 0X1F, 0XFE, 0X00, 0X3C, 0X1F, 0X00, 0X38, 0X06,
        0X00, 0X78, 0X00, 0X00, 0X70, 0X00, 0X00, 0X70, 0X00, 0X00, 0X70, 0X00, 0X00, 0X70, 0X00, 0X00,
        0X70, 0X00, 0X00, 0X78, 0X00, 0X00, 0X38, 0X06, 0X00, 0X3C, 0X1F, 0X00, 0X1F, 0XFE, 0X00, 0XCF,
        0XFC, 0X00, 0XFF, 0XF0, 0X00, 0X7F, 0XC0, 0X00,

        // 土耳其c小写 //
        0x63, 0x0D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0F, 0X80,
        0X00, 0X1F, 0XE0, 0X00, 0X3F, 0XE0, 0X00, 0X78, 0XF0, 0X00, 0X70, 0X60, 0X00, 0X70, 0X00, 0X00,
        0X70, 0X00, 0X00, 0X70, 0X00, 0X00, 0X70, 0X60, 0X00, 0X78, 0XF0, 0X00, 0X3F, 0XE0, 0X00, 0X9F,
        0XE0, 0X00, 0XFF, 0X80, 0X00, 0X7E, 0X00, 0X00,

        // 土耳其G大写 //
        0x47, 0x13, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0E, 0X0E, 0X00, 0X07, 0XFC, 0X00, 0X03, 0XF8, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XF8, 0X00, 0X0F, 0XFE, 0X00, 0X1F, 0XFF, 0X00, 0X3E, 0X0F, 0X80, 0X38, 0X03,
        0X00, 0X78, 0X00, 0X00, 0X70, 0X00, 0X00, 0X70, 0X00, 0X00, 0X70, 0X3F, 0X80, 0X70, 0X3F, 0X80,
        0X70, 0X3F, 0X80, 0X78, 0X03, 0X80, 0X38, 0X03, 0X80, 0X3E, 0X0F, 0X80, 0X1F, 0XFF, 0X80, 0X0F,
        0XFE, 0X00, 0X03, 0XF8, 0X00, 0X00, 0X00, 0X00,

        // 土耳其g小写 //
        0x67, 0x0F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1C, 0X1C, 0X00, 0X0F, 0XF8, 0X00, 0X07, 0XF0, 0X00, 0X00,
        0X00, 0X00, 0X0F, 0X38, 0X00, 0X1F, 0XF8, 0X00, 0X3F, 0XF8, 0X00, 0X38, 0X78, 0X00, 0X70, 0X38,
        0X00, 0X70, 0X38, 0X00, 0X70, 0X38, 0X00, 0X70, 0X38, 0X00, 0X70, 0X38, 0X00, 0X38, 0X78, 0X00,
        0X3F, 0XF8, 0X00, 0X1F, 0XF8, 0X00, 0X0F, 0X38, 0X00, 0X00, 0X38, 0X00, 0X70, 0X78, 0X00, 0X7F,
        0XF0, 0X00, 0X3F, 0XF0, 0X00, 0X1F, 0XC0, 0X00,

        // 土耳其I大写 //
        0x49, 0x07, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00,
        0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00,
        0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38,
        0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 土耳其i小写 //
        0x69, 0x07, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X38, 0X00,
        0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00,
        0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38,
        0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 土耳其O大写 //
        0x4F, 0x12, 0X00, 0X00, 0X00, 0X06, 0X18, 0X00, 0X0F, 0X3C, 0X00, 0X0F, 0X3C, 0X00, 0X06, 0X18, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XF0, 0X00, 0X0F, 0XFC, 0X00, 0X1F, 0XFE, 0X00, 0X3E, 0X1F, 0X00, 0X38, 0X07,
        0X00, 0X78, 0X07, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80,
        0X70, 0X03, 0X80, 0X78, 0X07, 0X80, 0X38, 0X07, 0X00, 0X3E, 0X1F, 0X00, 0X1F, 0XFE, 0X00, 0X0F,
        0XFC, 0X00, 0X03, 0XF0, 0X00, 0X00, 0X00, 0X00,

        // 土耳其o小写 //
        0x6F, 0x0F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0C,
        0X60, 0X00, 0X1E, 0XF0, 0X00, 0X1E, 0XF0, 0X00, 0X0C, 0X60, 0X00, 0X00, 0X00, 0X00, 0X07, 0XC0,
        0X00, 0X1F, 0XF0, 0X00, 0X3F, 0XF8, 0X00, 0X3C, 0X78, 0X00, 0X78, 0X3C, 0X00, 0X70, 0X1C, 0X00,
        0X70, 0X1C, 0X00, 0X70, 0X1C, 0X00, 0X78, 0X3C, 0X00, 0X3C, 0X78, 0X00, 0X3F, 0XF8, 0X00, 0X1F,
        0XF0, 0X00, 0X07, 0XC0, 0X00, 0X00, 0X00, 0X00,

        // 土耳其S大写 //
        0x53, 0x10, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X07, 0XE0, 0X00, 0X1F, 0XF8, 0X00, 0X1F, 0XF8, 0X00, 0X3C, 0X3C, 0X00, 0X38, 0X1C,
        0X00, 0X3C, 0X00, 0X00, 0X3F, 0X80, 0X00, 0X1F, 0XF0, 0X00, 0X07, 0XFC, 0X00, 0X00, 0XFC, 0X00,
        0X00, 0X1E, 0X00, 0X70, 0X0E, 0X00, 0X78, 0X0E, 0X00, 0X3C, 0X1E, 0X00, 0X3F, 0XFC, 0X00, 0X9F,
        0XF8, 0X00, 0XFF, 0XF0, 0X00, 0X7F, 0X80, 0X00,

        // 土耳其s小写 //
        0x73, 0x0D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1F, 0X80,
        0X00, 0X3F, 0XE0, 0X00, 0X70, 0XE0, 0X00, 0X70, 0X70, 0X00, 0X7C, 0X00, 0X00, 0X7F, 0X80, 0X00,
        0X3F, 0XE0, 0X00, 0X07, 0XF0, 0X00, 0X00, 0XF0, 0X00, 0X70, 0X70, 0X00, 0X38, 0X70, 0X00, 0XBF,
        0XE0, 0X00, 0XFF, 0X80, 0X00, 0X7E, 0X00, 0X00,

        // 土耳其U大写 //
        0x55, 0x11, 0X00, 0X00, 0X00, 0X06, 0X30, 0X00, 0X0F, 0X78, 0X00, 0X0F, 0X78, 0X00, 0X06, 0X30, 0X00, 0X00,
        0X00, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E,
        0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00,
        0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X3C, 0X1E, 0X00, 0X1F, 0XFC, 0X00, 0X0F,
        0XF8, 0X00, 0X07, 0XF0, 0X00, 0X00, 0X00, 0X00,

        // 土耳其u小写 //
        0x75, 0x0F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0C,
        0X60, 0X00, 0X1E, 0XF0, 0X00, 0X1E, 0XF0, 0X00, 0X0C, 0X60, 0X00, 0X00, 0X00, 0X00, 0X38, 0X38,
        0X00, 0X38, 0X38, 0X00, 0X38, 0X38, 0X00, 0X38, 0X38, 0X00, 0X38, 0X38, 0X00, 0X38, 0X38, 0X00,
        0X38, 0X38, 0X00, 0X38, 0X38, 0X00, 0X38, 0X38, 0X00, 0X3C, 0X78, 0X00, 0X3F, 0XF8, 0X00, 0X1F,
        0XF8, 0X00, 0X0F, 0X38, 0X00, 0X00, 0X00, 0X00,

        // 德语A大写 //
        0x41, 0x11, 0X00, 0X00, 0X00, 0X06, 0X30, 0X00, 0X0F, 0X78, 0X00, 0X0F, 0X78, 0X00, 0X06, 0X30, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XE0, 0X00, 0X03, 0XE0, 0X00, 0X03, 0XE0, 0X00, 0X07, 0X70, 0X00, 0X07, 0X70,
        0X00, 0X0F, 0X78, 0X00, 0X0E, 0X38, 0X00, 0X0E, 0X38, 0X00, 0X1C, 0X1C, 0X00, 0X1C, 0X1C, 0X00,
        0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X70, 0X07, 0X00, 0X70, 0X07, 0X00, 0X70,
        0X07, 0X00, 0XE0, 0X03, 0X80, 0X00, 0X00, 0X00,

        // 德语a小写 //
        0x61, 0x0D, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X18,
        0X60, 0X00, 0X3C, 0XF0, 0X00, 0X3C, 0XF0, 0X00, 0X18, 0X60, 0X00, 0X00, 0X00, 0X00, 0X0F, 0XC0,
        0X00, 0X3F, 0XE0, 0X00, 0X38, 0XF0, 0X00, 0X70, 0X70, 0X00, 0X00, 0X70, 0X00, 0X03, 0XF0, 0X00,
        0X1F, 0XF0, 0X00, 0X3E, 0X70, 0X00, 0X70, 0X70, 0X00, 0X70, 0X70, 0X00, 0X78, 0XF0, 0X00, 0X3F,
        0XF0, 0X00, 0X1F, 0X38, 0X00, 0X00, 0X00, 0X00,

        // 德语B大写 //
        0x42, 0x11, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X0F, 0XF8, 0X00, 0X1F, 0XFC, 0X00, 0X3F, 0XFC, 0X00, 0X38, 0X1E, 0X00, 0X38, 0X0E,
        0X00, 0X38, 0X0E, 0X00, 0X38, 0X1E, 0X00, 0X3F, 0XFC, 0X00, 0X3F, 0XFC, 0X00, 0X3F, 0XFE, 0X00,
        0X38, 0X0F, 0X00, 0X38, 0X07, 0X00, 0X38, 0X07, 0X00, 0X38, 0X0F, 0X00, 0X3B, 0XFE, 0X00, 0X39,
        0XFE, 0X00, 0X38, 0XF8, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙A~大写 ASC:1 //
        0x31, 0x17, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X07, 0X06, 0X00, 0X08, 0X88, 0X00, 0X30, 0X70, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XE0, 0X00, 0X03, 0XE0, 0X00, 0X03, 0XE0, 0X00, 0X07, 0X70, 0X00, 0X07, 0X70,
        0X00, 0X0F, 0X78, 0X00, 0X0E, 0X38, 0X00, 0X0E, 0X38, 0X00, 0X1C, 0X1C, 0X00, 0X1C, 0X1C, 0X00,
        0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X70, 0X07, 0X00, 0X70, 0X07, 0X00, 0X70,
        0X07, 0X00, 0XE0, 0X03, 0X80, 0X00, 0X00, 0X00,

        // 葡萄牙A^大写 ASC:2 //
        0x32, 0x17, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0XC0, 0X00, 0X03, 0X60, 0X00, 0X06, 0X30, 0X00, 0X0C,
        0X18, 0X00, 0X1B, 0XEC, 0X00, 0X03, 0XE0, 0X00, 0X03, 0XE0, 0X00, 0X07, 0X70, 0X00, 0X07, 0X70,
        0X00, 0X0F, 0X78, 0X00, 0X0E, 0X38, 0X00, 0X0E, 0X38, 0X00, 0X1C, 0X1C, 0X00, 0X1C, 0X1C, 0X00,
        0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X70, 0X07, 0X00, 0X70, 0X07, 0X00, 0X70,
        0X07, 0X00, 0XE0, 0X03, 0X80, 0X00, 0X00, 0X00,

        // 葡萄牙a~ ASC:3  //
        0x33, 0x12, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X0C, 0X30, 0X00, 0X12, 0X40, 0X00, 0X61, 0X80, 0X00, 0X00, 0X00, 0X00, 0X0F, 0XC0,
        0X00, 0X3F, 0XE0, 0X00, 0X38, 0XF0, 0X00, 0X70, 0X70, 0X00, 0X00, 0X70, 0X00, 0X03, 0XF0, 0X00,
        0X1F, 0XF0, 0X00, 0X3E, 0X70, 0X00, 0X70, 0X70, 0X00, 0X70, 0X70, 0X00, 0X78, 0XF0, 0X00, 0X3F,
        0XF0, 0X00, 0X1F, 0X38, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙O~ ASC:4  //
        0x34, 0x18, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X07, 0X06, 0X00, 0X08, 0X88, 0X00, 0X30, 0X70, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XF0, 0X00, 0X0F, 0XFC, 0X00, 0X1F, 0XFE, 0X00, 0X3E, 0X1F, 0X00, 0X38, 0X07,
        0X00, 0X78, 0X07, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80,
        0X70, 0X03, 0X80, 0X78, 0X07, 0X80, 0X38, 0X07, 0X00, 0X3E, 0X1F, 0X00, 0X1F, 0XFE, 0X00, 0X0F,
        0XFC, 0X00, 0X03, 0XF0, 0X00, 0X00, 0X00, 0X00,

        // p //
        0x70, 0x0F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3F, 0XC0,
        0X00, 0X3F, 0XF0, 0X00, 0X3F, 0XF0, 0X00, 0X38, 0X78, 0X00, 0X38, 0X38, 0X00, 0X38, 0X38, 0X00,
        0X38, 0X78, 0X00, 0X3F, 0XF0, 0X00, 0X3F, 0XF0, 0X00, 0X3F, 0XC0, 0X00, 0X38, 0X00, 0X00, 0X38,
        0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00,

        // g  用:的ASC//
        0x3A, 0x13, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0F, 0X70,
        0X00, 0X3F, 0XF0, 0X00, 0X7F, 0XF0, 0X00, 0X78, 0X70, 0X00, 0X70, 0X70, 0X00, 0X78, 0X70, 0X00,
        0X7F, 0XF0, 0X00, 0X3F, 0XF0, 0X00, 0X1F, 0X70, 0X00, 0X00, 0X70, 0X00, 0X60, 0XF0, 0X00, 0X7F,
        0XF0, 0X00, 0X3F, 0XE0, 0X00, 0X0F, 0XC0, 0X00,

        // 葡萄牙a` ASC:5  //
        0x35, 0x12, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X01, 0XC0, 0X00, 0X03, 0X80, 0X00, 0X07, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0F, 0XC0,
        0X00, 0X3F, 0XE0, 0X00, 0X38, 0XF0, 0X00, 0X70, 0X70, 0X00, 0X00, 0X70, 0X00, 0X03, 0XF0, 0X00,
        0X1F, 0XF0, 0X00, 0X3E, 0X70, 0X00, 0X70, 0X70, 0X00, 0X70, 0X70, 0X00, 0X78, 0XF0, 0X00, 0X3F,
        0XF0, 0X00, 0X1F, 0X38, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙I` ASC:6  //
        0x36, 0x07, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0E, 0X00, 0X00, 0X1C, 0X00, 0X00, 0X38, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00,
        0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00,
        0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38,
        0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙O` ASC:7  //
        0x37, 0x12, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X70, 0X00, 0X00, 0XE0, 0X00, 0X01, 0XC0, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XF0, 0X00, 0X0F, 0XFC, 0X00, 0X1F, 0XFE, 0X00, 0X3E, 0X1F, 0X00, 0X38, 0X07,
        0X00, 0X78, 0X07, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80, 0X70, 0X03, 0X80,
        0X70, 0X03, 0X80, 0X78, 0X07, 0X80, 0X38, 0X07, 0X00, 0X3E, 0X1F, 0X00, 0X1F, 0XFE, 0X00, 0X0F,
        0XFC, 0X00, 0X03, 0XF0, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙U` ASC:8 //
        0x38, 0x11, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X70, 0X00, 0X00, 0XE0, 0X00, 0X01, 0XC0, 0X00, 0X00,
        0X00, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E,
        0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00,
        0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X38, 0X0E, 0X00, 0X3C, 0X1E, 0X00, 0X1F, 0XFC, 0X00, 0X0F,
        0XF8, 0X00, 0X07, 0XF0, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙A` ASC:9 //
        0x39, 0x13, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X70, 0X00, 0X00, 0XE0, 0X00, 0X01, 0XC0, 0X00, 0X00,
        0X00, 0X00, 0X03, 0XE0, 0X00, 0X03, 0XE0, 0X00, 0X03, 0XE0, 0X00, 0X07, 0X70, 0X00, 0X07, 0X70,
        0X00, 0X0F, 0X78, 0X00, 0X0E, 0X38, 0X00, 0X0E, 0X38, 0X00, 0X1C, 0X1C, 0X00, 0X1C, 0X1C, 0X00,
        0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X70, 0X07, 0X00, 0X70, 0X07, 0X00, 0X70,
        0X07, 0X00, 0XE0, 0X03, 0X80, 0X00, 0X00, 0X00,

        // 葡萄牙E` ASC:A //
        0x3A, 0x10, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X70, 0X00, 0X00, 0XE0, 0X00, 0X01, 0XC0, 0X00, 0X00,
        0X00, 0X00, 0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00,
        0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X3F, 0XFC, 0X00, 0X3F, 0XFC, 0X00, 0X3F, 0XFC, 0X00,
        0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X38, 0X00, 0X00, 0X3F, 0XFE, 0X00, 0X3F,
        0XFE, 0X00, 0X3F, 0XFE, 0X00, 0X00, 0X00, 0X00,

        /* 最后一行必须用0xFF,0xFF结束，这是字库数组结束标志 */
        0xFF, 0xFF

};

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
