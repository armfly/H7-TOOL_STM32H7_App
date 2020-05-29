
-------------------------------------------------------
-- 文件名 : STM32L47x_48x_49x_4AxxG.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

	AlgoFile_FLASH =  "0:/H7-TOOL/Programmer/Device/ST/STM32L4xx/STM32L4xx_1024.FLM"
	AlgoFile_EEPROM = ""
	AlgoFile_OTP   = ""
	AlgoFile_OPT   = "0:/H7-TOOL/Programmer/Device/ST/STM32L4xx/STM32L4xx_DB_OPT.FLM"
	AlgoFile_QSPI  = ""

	FLASH_ADDRESS = 0x08000000		--CPU内部FLASH起始地址
	FLASH_SIZE = 1024 * 1024			--芯片实际Flash Size小于FLM中的定义的size

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = 0x20000000
	AlgoRamSize = 2 * 1024

	MCU_ID = 0x2BA01477

	UID_ADDR = 0x1FFF7590	   	--UID地址，不同的CPU不同
	UID_BYTES = 12

	ERASE_CHIP_TIME = 2000		--全片擦除时间ms（仅用于进度指示)

	--地址组中的FFFFFFFF表示原始数据中插入上个字节的反码 FFFFFFFE表示原始数据中插入前2个字节的反码
	OB_ADDRESS     = "1FFF7800 1FFF7801 1FFF7802 1FFF7803 1FFF7808 1FFF7809 1FFF780A 1FFF780B "
				   .."1FFF7810 1FFF7811 1FFF7812 1FFF7813 1FFF7818 1FFF7819 1FFF781A 1FFF781B "
 				   .."1FFF7820 1FFF7821 1FFF7822 1FFF7823 "

				   .."1FFFF808 1FFFF809 1FFFF80A 1FFFF80B "
				   .."1FFFF810 1FFFF811 1FFFF812 1FFFF813 "
				   .."1FFFF818 1FFFF819 1FFFF81A 1FFFF81B "
				   .."1FFFF820 1FFFF821 1FFFF822 1FFFF823 "

	OB_SECURE_OFF  = "AA 70 EF 0F FF FF FF FF 00 00 FF FF FF FF 00 FF "
				   .."FF FF 00 FF "		--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
				   .."FF FF FF FF 00 00 FF FF FF FF 00 FF FF FF 00 FF "
	OB_SECURE_ON   = "00 70 EF 0F FF FF FF FF 00 00 FF FF FF FF 00 FF "
				   .."FF FF 00 FF "		--SECURE_ENABLE = 1时，编程完毕后写入该值
				   .."FF FF FF FF 00 00 FF FF FF FF 00 FF FF FF 00 FF "

	--判断读保护和写保护的条件(WRP = Write protection)
	OB_WRP_ADDRESS   = {0x1FFF7800,
						0x1FFF7808, 0x1FFF7810, 0x1FFF7818, 0x1FFF781A, 0x1FFF7820, 0x1FFF7822,
						0x1FFFF808, 0x1FFFF810, 0x1FFFF818, 0x1FFFF81A, 0x1FFFF820, 0x1FFFF822,
						} 	--内存地址

	OB_WRP_MASK  	 = {0xFF,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}		--读出数据与此数相与
	OB_WRP_VALUE 	 = {0xAA,
						0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
						0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00}		--相与后与此数比较，相等表示没有保护
end

---------------------------结束-----------------------------------
