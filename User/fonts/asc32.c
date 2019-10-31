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

unsigned char const g_Ascii32[] =
    {
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0xC0, 0x01, 0xC0, 0x03, 0xC0, 0x0F, 0xC0, 0x01, 0xC0, // 1 //
        0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0,
        0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0, 0x01, 0xC0,
        0x03, 0xE0, 0x0F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x32, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x07, 0x38, 0x0E, 0x1C, 0x1C, 0x0E, 0x38, 0x0E, 0x38, 0x0E, // 2 //
        0x38, 0x0E, 0x30, 0x0E, 0x00, 0x0E, 0x00, 0x1C, 0x00, 0x1C, 0x00, 0x38, 0x00, 0x70, 0x00, 0x60,
        0x00, 0xC0, 0x01, 0x80, 0x03, 0x00, 0x06, 0x00, 0x0C, 0x00, 0x18, 0x00, 0x38, 0x02, 0x30, 0x02,
        0x30, 0x06, 0x3F, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x33, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x0E, 0x38, 0x1C, 0x1C, 0x1C, 0x1C, 0x38, 0x1C, 0x38, 0x1C, // 3 //
        0x18, 0x1C, 0x00, 0x1C, 0x00, 0x38, 0x00, 0x38, 0x00, 0x70, 0x01, 0xF0, 0x00, 0x38, 0x00, 0x1C,
        0x00, 0x0E, 0x00, 0x0E, 0x00, 0x0E, 0x20, 0x0E, 0x70, 0x0E, 0x70, 0x0E, 0x30, 0x0E, 0x38, 0x1C,
        0x0E, 0x38, 0x07, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x78, 0x00, 0xB8, 0x00, 0xB8, 0x01, 0x38, 0x01, 0x38, // 4 //
        0x03, 0x38, 0x03, 0x38, 0x06, 0x38, 0x06, 0x38, 0x06, 0x38, 0x0C, 0x38, 0x0C, 0x38, 0x0C, 0x38,
        0x18, 0x38, 0x18, 0x38, 0x30, 0x38, 0x30, 0x38, 0x7F, 0xFF, 0x00, 0x38, 0x00, 0x38, 0x00, 0x38,
        0x00, 0x7C, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x45, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFC, 0x18, 0x0C, 0x18, 0x04, 0x18, 0x04, 0x18, 0x00, 0x18, 0x00, // E //
        0x18, 0x00, 0x18, 0x00, 0x18, 0x08, 0x18, 0x08, 0x18, 0x18, 0x1F, 0xF8, 0x18, 0x18, 0x18, 0x08,
        0x18, 0x08, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x04, 0x18, 0x04,
        0x18, 0x0C, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x50, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xF0, 0x18, 0x38, 0x18, 0x1C, 0x18, 0x0C, 0x18, 0x0C, 0x18, 0x0C, // P //
        0x18, 0x0C, 0x18, 0x0C, 0x18, 0x0C, 0x18, 0x1C, 0x18, 0x38, 0x1F, 0xF0, 0x18, 0x00, 0x18, 0x00,
        0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
        0x3C, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x53, 0x00, 0x00, 0x00, 0x00, 0x03, 0xE4, 0x0E, 0x74, 0x1C, 0x1C, 0x18, 0x0C, 0x30, 0x06, 0x30, 0x02, // S //
        0x30, 0x02, 0x38, 0x00, 0x18, 0x00, 0x1E, 0x00, 0x0F, 0x00, 0x03, 0xC0, 0x01, 0xE0, 0x00, 0x70,
        0x00, 0x38, 0x00, 0x1C, 0x00, 0x0C, 0x40, 0x0C, 0x40, 0x0C, 0x60, 0x0C, 0x30, 0x1C, 0x38, 0x38,
        0x2E, 0x70, 0x27, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x54, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFC, 0x31, 0x8C, 0x21, 0x84, 0x61, 0x86, 0x41, 0x82, 0x01, 0x80, // T //
        0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
        0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80,
        0x03, 0xC0, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        /* 最后一行必须用0xFF,0xFF结束，这是字库数组结束标志 */
        0xFF, 0xFF

};

unsigned char const g_Ascii32_VarWidth[] =
    {
        // 土耳其C大写 //
        0x43, 0x17, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFE, 0X00, 0X00,
        0X03, 0XFF, 0X80, 0X00, 0X07, 0XFF, 0XE0, 0X00, 0X0F, 0XFF, 0XE0, 0X00, 0X1F, 0X83, 0XF0, 0X00,
        0X3E, 0X00, 0XF8, 0X00, 0X3C, 0X00, 0XF8, 0X00, 0X3C, 0X00, 0X60, 0X00, 0X78, 0X00, 0X00, 0X00,
        0X78, 0X00, 0X00, 0X00, 0X78, 0X00, 0X00, 0X00, 0X78, 0X00, 0X00, 0X00, 0X78, 0X00, 0X00, 0X00,
        0X78, 0X00, 0X00, 0X00, 0X78, 0X00, 0X60, 0X00, 0X7C, 0X00, 0X78, 0X00, 0X3C, 0X00, 0XF8, 0X00,
        0X3E, 0X01, 0XF0, 0X00, 0X1F, 0X83, 0XF0, 0X00, 0X0F, 0XFF, 0XE0, 0X00, 0X07, 0XFF, 0XC0, 0X00,
        0X03, 0XFF, 0X80, 0X00, 0X18, 0XFE, 0X00, 0X00, 0X0F, 0XF0, 0X00, 0X00, 0X07, 0XE0, 0X00, 0X00,

        //// 土耳其c小写 //
        //0x63,0x12,  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X03,0XF8,0X00,0X00,0X0F,0XFE,0X00,0X00,0X1F,0XFF,0X00,0X00,
        //			0X3E,0X1F,0X00,0X00,0X3C,0X0F,0X80,0X00,0X78,0X06,0X00,0X00,0X78,0X00,0X00,0X00,
        //			0X78,0X00,0X00,0X00,0X78,0X00,0X00,0X00,0X78,0X00,0X00,0X00,0X78,0X00,0X00,0X00,
        //			0X78,0X06,0X00,0X00,0X3C,0X0F,0X80,0X00,0X3E,0X1F,0X00,0X00,0X1F,0XFF,0X00,0X00,
        //			0X0F,0XFE,0X00,0X00,0X43,0XF8,0X00,0X00,0X3F,0XC0,0X00,0X00,0X1F,0XC0,0X00,0X00,

        // 土耳其G大写 //
        0x47, 0x19, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0F, 0X80, 0X7C, 0X00, 0X07, 0XFF, 0XF8, 0X00,
        0X03, 0XFF, 0XF0, 0X00, 0X00, 0XFF, 0XC0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7F, 0X80, 0X00,
        0X03, 0XFF, 0XE0, 0X00, 0X07, 0XFF, 0XF8, 0X00, 0X0F, 0XFF, 0XF8, 0X00, 0X1F, 0X80, 0XFC, 0X00,
        0X3F, 0X00, 0X7E, 0X00, 0X3C, 0X00, 0X3E, 0X00, 0X3C, 0X00, 0X18, 0X00, 0X78, 0X00, 0X00, 0X00,
        0X78, 0X00, 0X00, 0X00, 0X78, 0X00, 0X00, 0X00, 0X78, 0X07, 0XFE, 0X00, 0X78, 0X07, 0XFE, 0X00,
        0X78, 0X07, 0XFE, 0X00, 0X78, 0X07, 0XFE, 0X00, 0X3C, 0X00, 0X1E, 0X00, 0X3E, 0X00, 0X1E, 0X00,
        0X3F, 0X00, 0X1E, 0X00, 0X1F, 0XC0, 0XFE, 0X00, 0X0F, 0XFF, 0XFE, 0X00, 0X07, 0XFF, 0XFC, 0X00,
        0X03, 0XFF, 0XF0, 0X00, 0X00, 0X7F, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        //// 土耳其g小写 //
        //0x67,0x13, 0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X7C,0X03,0XE0,0X00,0X3F,0XFF,0XC0,0X00,0X1F,0XFF,0X80,0X00,0X07,0XFE,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X07,0XC7,0X80,0X00,0X0F,0XF7,0X80,0X00,0X1F,0XFF,0X80,0X00,
        //			0X3E,0X1F,0X80,0X00,0X3C,0X0F,0X80,0X00,0X78,0X07,0X80,0X00,0X78,0X07,0X80,0X00,
        //			0X78,0X07,0X80,0X00,0X78,0X07,0X80,0X00,0X78,0X07,0X80,0X00,0X78,0X07,0X80,0X00,
        //			0X78,0X0F,0X80,0X00,0X3C,0X0F,0X80,0X00,0X3E,0X1F,0X80,0X00,0X1F,0XFF,0X80,0X00,
        //			0X0F,0XF7,0X80,0X00,0X07,0XC7,0X80,0X00,0X00,0X07,0X80,0X00,0X30,0X07,0X80,0X00,
        //			0X3C,0X0F,0X00,0X00,0X1F,0XFF,0X00,0X00,0X1F,0XFE,0X00,0X00,0X07,0XF8,0X00,0X00,

        // 土耳其I大写 //
        0x49, 0x08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        //// 土耳其i小写 //
        //0x69,0x0A, 0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,
        //			0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,
        //			0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,
        //			0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,
        //			0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,
        //			0X1E,0X00,0X00,0X00,0X1E,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,

        // 土耳其O大写 //
        0x4F, 0x18, 0X01, 0X81, 0X80, 0X00, 0X03, 0XC3, 0XC0, 0X00, 0X07, 0XE7, 0XE0, 0X00, 0X07, 0XE7, 0XE0, 0X00,
        0X03, 0XC3, 0XC0, 0X00, 0X01, 0X81, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0X00, 0X00,
        0X03, 0XFF, 0XC0, 0X00, 0X07, 0XFF, 0XE0, 0X00, 0X0F, 0XFF, 0XF0, 0X00, 0X1F, 0X81, 0XF8, 0X00,
        0X3E, 0X00, 0X7C, 0X00, 0X3C, 0X00, 0X3C, 0X00, 0X7C, 0X00, 0X3E, 0X00, 0X78, 0X00, 0X1E, 0X00,
        0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00,
        0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X7C, 0X00, 0X3E, 0X00, 0X3C, 0X00, 0X3C, 0X00,
        0X3E, 0X00, 0X7C, 0X00, 0X1F, 0X81, 0XF8, 0X00, 0X0F, 0XFF, 0XF0, 0X00, 0X07, 0XFF, 0XE0, 0X00,
        0X03, 0XFF, 0XC0, 0X00, 0X00, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        //// 土耳其o小写 //
        //0x6F,0x13, 0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X06,0X0C,0X00,0X00,0X0F,0X1E,0X00,0X00,
        //			0X1F,0XBF,0X00,0X00,0X1F,0XBF,0X00,0X00,0X0F,0X1E,0X00,0X00,0X06,0X0C,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X03,0XF8,0X00,0X00,0X0F,0XFE,0X00,0X00,0X1F,0XFF,0X00,0X00,
        //			0X3E,0X0F,0X80,0X00,0X3C,0X07,0X80,0X00,0X7C,0X07,0XC0,0X00,0X78,0X03,0XC0,0X00,
        //			0X78,0X03,0XC0,0X00,0X78,0X03,0XC0,0X00,0X78,0X03,0XC0,0X00,0X78,0X03,0XC0,0X00,
        //			0X7C,0X07,0XC0,0X00,0X3C,0X07,0X80,0X00,0X3E,0X0F,0X80,0X00,0X1F,0XFF,0X00,0X00,
        //			0X0F,0XFE,0X00,0X00,0X03,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,

        // 土耳其S大写 //
        0x53, 0x15, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X03, 0XFC, 0X00, 0X00,
        0X0F, 0XFF, 0X00, 0X00, 0X1F, 0XFF, 0X80, 0X00, 0X1F, 0XFF, 0X80, 0X00, 0X3E, 0X07, 0XC0, 0X00,
        0X3C, 0X03, 0XC0, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3E, 0X00, 0X00, 0X00, 0X3F, 0X80, 0X00, 0X00,
        0X1F, 0XF0, 0X00, 0X00, 0X0F, 0XFE, 0X00, 0X00, 0X07, 0XFF, 0X80, 0X00, 0X01, 0XFF, 0XC0, 0X00,
        0X00, 0X3F, 0XC0, 0X00, 0X00, 0X07, 0XE0, 0X00, 0X00, 0X01, 0XE0, 0X00, 0X78, 0X01, 0XE0, 0X00,
        0X7C, 0X01, 0XE0, 0X00, 0X3E, 0X03, 0XE0, 0X00, 0X3F, 0XFF, 0XC0, 0X00, 0X1F, 0XFF, 0XC0, 0X00,
        0X0F, 0XFF, 0X80, 0X00, 0X33, 0XFE, 0X00, 0X00, 0X3F, 0XF0, 0X00, 0X00, 0X1F, 0XF0, 0X00, 0X00,

        //// 土耳其s小写 //
        //0x73,0x12, 0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X07,0XF0,0X00,0X00,0X1F,0XFC,0X00,0X00,0X1F,0XFC,0X00,0X00,
        //			0X3C,0X3E,0X00,0X00,0X3C,0X18,0X00,0X00,0X3C,0X00,0X00,0X00,0X3F,0X80,0X00,0X00,
        //			0X1F,0XF0,0X00,0X00,0X0F,0XFC,0X00,0X00,0X03,0XFE,0X00,0X00,0X00,0X7F,0X00,0X00,
        //			0X18,0X0F,0X00,0X00,0X7C,0X0F,0X00,0X00,0X3E,0X1F,0X00,0X00,0X3F,0XFE,0X00,0X00,
        //			0X1F,0XFC,0X00,0X00,0X67,0XF0,0X00,0X00,0X7F,0XC0,0X00,0X00,0X3F,0XC0,0X00,0X00,

        // 土耳其U大写 //
        0x55, 0x16, 0X07, 0X03, 0X80, 0X00, 0X0F, 0X87, 0XC0, 0X00, 0X1F, 0XCF, 0XE0, 0X00, 0X1F, 0XCF, 0XE0, 0X00,
        0X0F, 0X87, 0XC0, 0X00, 0X07, 0X03, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3E, 0X01, 0XF0, 0X00, 0X1F, 0X03, 0XE0, 0X00, 0X1F, 0XFF, 0XE0, 0X00, 0X0F, 0XFF, 0XC0, 0X00,
        0X07, 0XFF, 0X80, 0X00, 0X01, 0XFE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        //// 土耳其u小写 //
        //0x75,0x13, 0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X0E,0X0E,0X00,0X00,0X1F,0X1F,0X00,0X00,
        //			0X3F,0XBF,0X80,0X00,0X3F,0XBF,0X80,0X00,0X1F,0X1F,0X00,0X00,0X0E,0X0E,0X00,0X00,
        //			0X00,0X00,0X00,0X00,0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,
        //			0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,
        //			0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,0X3C,0X07,0X80,0X00,
        //			0X3C,0X07,0X80,0X00,0X3C,0X0F,0X80,0X00,0X3E,0X1F,0X80,0X00,0X1F,0XFF,0X80,0X00,
        //			0X1F,0XF7,0X80,0X00,0X07,0XC7,0X80,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,

        // 德语A大写 //
        0x41, 0x17, 0X01, 0X83, 0X00, 0X00, 0X03, 0XC7, 0X80, 0X00, 0X07, 0XEF, 0XC0, 0X00, 0X07, 0XEF, 0XC0, 0X00,
        0X03, 0XC7, 0X80, 0X00, 0X01, 0X83, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7C, 0X00, 0X00,
        0X00, 0XFE, 0X00, 0X00, 0X00, 0XFE, 0X00, 0X00, 0X01, 0XFF, 0X00, 0X00, 0X01, 0XEF, 0X00, 0X00,
        0X01, 0XEF, 0X00, 0X00, 0X03, 0XEF, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00,
        0X07, 0X83, 0XC0, 0X00, 0X07, 0X83, 0XC0, 0X00, 0X0F, 0X83, 0XE0, 0X00, 0X0F, 0X01, 0XE0, 0X00,
        0X0F, 0X01, 0XE0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X3F, 0XFF, 0XF8, 0X00,
        0X3F, 0XFF, 0XF8, 0X00, 0X3C, 0X00, 0X78, 0X00, 0X7C, 0X00, 0X7C, 0X00, 0X78, 0X00, 0X3C, 0X00,
        0X78, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X1E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 德语a小写 //
        0x61, 0x12, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0C, 0X18, 0X00, 0X00, 0X1E, 0X3C, 0X00, 0X00,
        0X3F, 0X7E, 0X00, 0X00, 0X3F, 0X7E, 0X00, 0X00, 0X1E, 0X3C, 0X00, 0X00, 0X0C, 0X18, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X07, 0XF8, 0X00, 0X00, 0X0F, 0XFE, 0X00, 0X00, 0X1F, 0XFE, 0X00, 0X00,
        0X3C, 0X1F, 0X00, 0X00, 0X18, 0X0F, 0X00, 0X00, 0X00, 0X0F, 0X00, 0X00, 0X00, 0X3F, 0X00, 0X00,
        0X07, 0XFF, 0X00, 0X00, 0X1F, 0XFF, 0X00, 0X00, 0X3F, 0XCF, 0X00, 0X00, 0X7C, 0X0F, 0X00, 0X00,
        0X78, 0X0F, 0X00, 0X00, 0X78, 0X1F, 0X00, 0X00, 0X7C, 0X3F, 0X00, 0X00, 0X3F, 0XFF, 0X00, 0X00,
        0X3F, 0XF7, 0X00, 0X00, 0X0F, 0XC7, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 德语B大写 //
        0x42, 0x16, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X07, 0XFF, 0X00, 0X00,
        0X0F, 0XFF, 0XC0, 0X00, 0X1F, 0XFF, 0XE0, 0X00, 0X3F, 0XFF, 0XF0, 0X00, 0X3C, 0X01, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X01, 0XE0, 0X00,
        0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XFF, 0XC0, 0X00, 0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XFF, 0XF0, 0X00,
        0X3C, 0X01, 0XF0, 0X00, 0X3C, 0X00, 0X78, 0X00, 0X3C, 0X00, 0X78, 0X00, 0X3C, 0X00, 0X78, 0X00,
        0X3C, 0X00, 0X78, 0X00, 0X3D, 0X00, 0XF8, 0X00, 0X3D, 0XFF, 0XF0, 0X00, 0X3C, 0XFF, 0XF0, 0X00,
        0X3C, 0X7F, 0XE0, 0X00, 0X3C, 0X1F, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙A~大写 ASC:1 //
        0x31, 0x17, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X01, 0XE0, 0X70, 0X00, 0X03, 0XE0, 0XC0, 0X00,
        0X06, 0X1F, 0X80, 0X00, 0X1C, 0X0F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7C, 0X00, 0X00,
        0X00, 0XFE, 0X00, 0X00, 0X00, 0XFE, 0X00, 0X00, 0X01, 0XFF, 0X00, 0X00, 0X01, 0XEF, 0X00, 0X00,
        0X01, 0XEF, 0X00, 0X00, 0X03, 0XEF, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00,
        0X07, 0X83, 0XC0, 0X00, 0X07, 0X83, 0XC0, 0X00, 0X0F, 0X83, 0XE0, 0X00, 0X0F, 0X01, 0XE0, 0X00,
        0X0F, 0X01, 0XE0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X3F, 0XFF, 0XF8, 0X00,
        0X3F, 0XFF, 0XF8, 0X00, 0X3C, 0X00, 0X78, 0X00, 0X7C, 0X00, 0X7C, 0X00, 0X78, 0X00, 0X3C, 0X00,
        0X78, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X1E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙A^大写 ASC:2 //
        0x32, 0x17, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X6C, 0X00, 0X00,
        0X00, 0XC6, 0X00, 0X00, 0X01, 0X83, 0X00, 0X00, 0X03, 0X01, 0X80, 0X00, 0X00, 0X7C, 0X00, 0X00,
        0X00, 0XFE, 0X00, 0X00, 0X00, 0XFE, 0X00, 0X00, 0X01, 0XFF, 0X00, 0X00, 0X01, 0XEF, 0X00, 0X00,
        0X01, 0XEF, 0X00, 0X00, 0X03, 0XEF, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00,
        0X07, 0X83, 0XC0, 0X00, 0X07, 0X83, 0XC0, 0X00, 0X0F, 0X83, 0XE0, 0X00, 0X0F, 0X01, 0XE0, 0X00,
        0X0F, 0X01, 0XE0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X3F, 0XFF, 0XF8, 0X00,
        0X3F, 0XFF, 0XF8, 0X00, 0X3C, 0X00, 0X78, 0X00, 0X7C, 0X00, 0X7C, 0X00, 0X78, 0X00, 0X3C, 0X00,
        0X78, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X1E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙a~ ASC:3  //
        0x33, 0x12, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X07, 0X81, 0X80, 0X00, 0X0F, 0XC3, 0X00, 0X00, 0X18, 0X66, 0X00, 0X00, 0X30, 0X3C, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X07, 0XF8, 0X00, 0X00, 0X0F, 0XFE, 0X00, 0X00, 0X1F, 0XFE, 0X00, 0X00,
        0X3C, 0X1F, 0X00, 0X00, 0X18, 0X0F, 0X00, 0X00, 0X00, 0X0F, 0X00, 0X00, 0X00, 0X3F, 0X00, 0X00,
        0X07, 0XFF, 0X00, 0X00, 0X1F, 0XFF, 0X00, 0X00, 0X3F, 0XCF, 0X00, 0X00, 0X7C, 0X0F, 0X00, 0X00,
        0X78, 0X0F, 0X00, 0X00, 0X78, 0X1F, 0X00, 0X00, 0X7C, 0X3F, 0X00, 0X00, 0X3F, 0XFF, 0X00, 0X00,
        0X3F, 0XF7, 0X00, 0X00, 0X0F, 0XC7, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙O~ ASC:4  //
        0x34, 0x18, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X03, 0XE0, 0X1E, 0X00, 0X07, 0XF0, 0X3E, 0X00,
        0X7C, 0X1F, 0XE0, 0X00, 0X78, 0X0F, 0XC0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0X00, 0X00,
        0X03, 0XFF, 0XC0, 0X00, 0X07, 0XFF, 0XE0, 0X00, 0X0F, 0XFF, 0XF0, 0X00, 0X1F, 0X81, 0XF8, 0X00,
        0X3E, 0X00, 0X7C, 0X00, 0X3C, 0X00, 0X3C, 0X00, 0X7C, 0X00, 0X3E, 0X00, 0X78, 0X00, 0X1E, 0X00,
        0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00,
        0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X7C, 0X00, 0X3E, 0X00, 0X3C, 0X00, 0X3C, 0X00,
        0X3E, 0X00, 0X7C, 0X00, 0X1F, 0X81, 0XF8, 0X00, 0X0F, 0XFF, 0XF0, 0X00, 0X07, 0XFF, 0XE0, 0X00,
        0X03, 0XFF, 0XC0, 0X00, 0X00, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙a` ASC:5  //
        0x35, 0x12, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X00, 0X70, 0X00, 0X00, 0X00, 0XE0, 0X00, 0X00, 0X01, 0XC0, 0X00, 0X00,
        0X00, 0X00, 0X00, 0X00, 0X07, 0XF8, 0X00, 0X00, 0X0F, 0XFE, 0X00, 0X00, 0X1F, 0XFE, 0X00, 0X00,
        0X3C, 0X1F, 0X00, 0X00, 0X18, 0X0F, 0X00, 0X00, 0X00, 0X0F, 0X00, 0X00, 0X00, 0X3F, 0X00, 0X00,
        0X07, 0XFF, 0X00, 0X00, 0X1F, 0XFF, 0X00, 0X00, 0X3F, 0XCF, 0X00, 0X00, 0X7C, 0X0F, 0X00, 0X00,
        0X78, 0X0F, 0X00, 0X00, 0X78, 0X1F, 0X00, 0X00, 0X7C, 0X3F, 0X00, 0X00, 0X3F, 0XFF, 0X00, 0X00,
        0X3F, 0XF7, 0X00, 0X00, 0X0F, 0XC7, 0X80, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙I` ASC:6  //
        0x36, 0x08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0E, 0X00, 0X00, 0X00,
        0X1C, 0X00, 0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙O` ASC:7  //
        0x37, 0x18, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0E, 0X00, 0X00,
        0X00, 0X1C, 0X00, 0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XFF, 0X00, 0X00,
        0X03, 0XFF, 0XC0, 0X00, 0X07, 0XFF, 0XE0, 0X00, 0X0F, 0XFF, 0XF0, 0X00, 0X1F, 0X81, 0XF8, 0X00,
        0X3E, 0X00, 0X7C, 0X00, 0X3C, 0X00, 0X3C, 0X00, 0X7C, 0X00, 0X3E, 0X00, 0X78, 0X00, 0X1E, 0X00,
        0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00,
        0X78, 0X00, 0X1E, 0X00, 0X78, 0X00, 0X1E, 0X00, 0X7C, 0X00, 0X3E, 0X00, 0X3C, 0X00, 0X3C, 0X00,
        0X3E, 0X00, 0X7C, 0X00, 0X1F, 0X81, 0XF8, 0X00, 0X0F, 0XFF, 0XF0, 0X00, 0X07, 0XFF, 0XE0, 0X00,
        0X03, 0XFF, 0XC0, 0X00, 0X00, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙U` ASC:8 //
        0x38, 0x16, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1C, 0X00, 0X00,
        0X00, 0X38, 0X00, 0X00, 0X00, 0X70, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X3C, 0X00, 0XF0, 0X00,
        0X3E, 0X01, 0XF0, 0X00, 0X1F, 0X03, 0XE0, 0X00, 0X1F, 0XFF, 0XE0, 0X00, 0X0F, 0XFF, 0XC0, 0X00,
        0X07, 0XFF, 0X80, 0X00, 0X01, 0XFE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙A` ASC:9 //
        0x39, 0x17, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0E, 0X00, 0X00,
        0X00, 0X1C, 0X00, 0X00, 0X00, 0X38, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X7C, 0X00, 0X00,
        0X00, 0XFE, 0X00, 0X00, 0X00, 0XFE, 0X00, 0X00, 0X01, 0XFF, 0X00, 0X00, 0X01, 0XEF, 0X00, 0X00,
        0X01, 0XEF, 0X00, 0X00, 0X03, 0XEF, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00, 0X03, 0XC7, 0X80, 0X00,
        0X07, 0X83, 0XC0, 0X00, 0X07, 0X83, 0XC0, 0X00, 0X0F, 0X83, 0XE0, 0X00, 0X0F, 0X01, 0XE0, 0X00,
        0X0F, 0X01, 0XE0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X1F, 0XFF, 0XF0, 0X00, 0X3F, 0XFF, 0XF8, 0X00,
        0X3F, 0XFF, 0XF8, 0X00, 0X3C, 0X00, 0X78, 0X00, 0X7C, 0X00, 0X7C, 0X00, 0X78, 0X00, 0X3C, 0X00,
        0X78, 0X00, 0X3C, 0X00, 0XF0, 0X00, 0X1E, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        // 葡萄牙E` ASC:A //
        0x3A, 0x15, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X38, 0X00, 0X00,
        0X00, 0X70, 0X00, 0X00, 0X00, 0XE0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3F, 0XFF, 0XE0, 0X00,
        0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XFF, 0XE0, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3F, 0XFF, 0XC0, 0X00, 0X3F, 0XFF, 0XC0, 0X00, 0X3F, 0XFF, 0XC0, 0X00, 0X3F, 0XFF, 0XC0, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00,
        0X3C, 0X00, 0X00, 0X00, 0X3C, 0X00, 0X00, 0X00, 0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XFF, 0XE0, 0X00,
        0X3F, 0XFF, 0XE0, 0X00, 0X3F, 0XFF, 0XE0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,

        /* 最后一行必须用0xFF,0xFF结束，这是字库数组结束标志 */
        0xFF, 0xFF

};

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
