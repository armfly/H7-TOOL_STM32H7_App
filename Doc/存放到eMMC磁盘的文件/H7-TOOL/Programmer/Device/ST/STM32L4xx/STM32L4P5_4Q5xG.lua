
-------------------------------------------------------
-- 文件名 : STM32L4P5_4Q5xG.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

	AlgoFile_FLASH =  "0:/H7-TOOL/Programmer/Device/ST/STM32L4xx/STM32L4P5xx_1M.FLM"
	AlgoFile_EEPROM = ""
	AlgoFile_OTP   = ""
	AlgoFile_OPT   = "0:/H7-TOOL/Programmer/Device/ST/STM32L4xx/STM32L4Rx_DB_OPT.FLM"
	AlgoFile_QSPI  = ""

	FLASH_ADDRESS = 0x08000000		--CPU内部FLASH起始地址
	FLASH_SIZE = 1024 * 1024		--芯片实际Flash Size小于FLM中的定义的size

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = 0x20000000
	AlgoRamSize = 2 * 1024

	MCU_ID = 0x2BA01477

	UID_ADDR = 0x1FFF7590	   	--UID地址，不同的CPU不同
	UID_BYTES = 12

	ERASE_CHIP_TIME = 2000		--全片擦除时间ms（仅用于进度指示)

	--地址组中的FFFFFFFF表示原始数据中插入上个字节的反码 FFFFFFFE表示原始数据中插入前2个字节的反码
	OB_ADDRESS     = "1FF00000 1FF00001 1FF00002 1FF00003 1FF00008 1FF00009 1FF0000A 1FF0000B "
				   .."1FF00010 1FF00011 1FF00012 1FF00013 1FF00018 1FF00019 1FF0001A 1FF0001B "
 				   .."1FF00020 1FF00021 1FF00022 1FF00023 "

				   .."1FF01008 1FF01009 1FF0100A 1FF0100B "
				   .."1FF01010 1FF01011 1FF01012 1FF01013 "
				   .."1FF01018 1FF01019 1FF0101A 1FF0101B "
				   .."1FF01020 1FF01021 1FF01022 1FF01023 "

	OB_SECURE_OFF  = "AA 70 EF 0F FF FF FF FF 00 00 FF FF FF FF 00 FF "
				   .."FF FF 00 FF "		--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
				   .."FF FF FF FF 00 00 FF FF FF FF 00 FF FF FF 00 FF "
	OB_SECURE_ON   = "00 70 EF 0F FF FF FF FF 00 00 FF FF FF FF 00 FF "
				   .."FF FF 00 FF "		--SECURE_ENABLE = 1时，编程完毕后写入该值
				   .."FF FF FF FF 00 00 FF FF FF FF 00 FF FF FF 00 FF "

	--判断读保护和写保护的条件(WRP = Write protection)
	OB_WRP_ADDRESS   = {0x1FF00000,
						0x1FF00008, 0x1FF00010, 0x1FF00018, 0x1FF0001A, 0x1FF00020, 0x1FF00022,
						0x1FF01008, 0x1FF01010, 0x1FF01018, 0x1FF0101A, 0x1FF01020, 0x1FF01022,
						} 	--内存地址

	OB_WRP_MASK  	 = {0xFF,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}		--读出数据与此数相与
	OB_WRP_VALUE 	 = {0xAA,
						0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
						0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}		--相与后与此数比较，相等表示没有保护
end

---------------------------结束-----------------------------------
