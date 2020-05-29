
-------------------------------------------------------
-- 文件名 : STM32L011x3.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

	AlgoFile_FLASH =  "0:/H7-TOOL/Programmer/Device/ST/STM32L0xx/STM32L0xx_16.FLM"
	AlgoFile_EEPROM = "0:/H7-TOOL/Programmer/Device/ST/STM32L0xx/STM32L01_2x_EEPROM.FLM"
	AlgoFile_OTP   = ""
	AlgoFile_OPT   = "0:/H7-TOOL/Programmer/Device/ST/STM32L0xx/STM32L0xx_OPT.FLM"
	AlgoFile_QSPI  = ""

	FLASH_ADDRESS = 0x08000000		--CPU内部FLASH起始地址
	FLASH_SIZE = 16 * 1024			--芯片实际Flash Size小于FLM中的定义的size

	EEPROM_ADDRESS = 0x08080000
	EEPROM_SIZE = 512				--芯片实际Flash Size小于FLM中的定义的size

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = 0x20000000
	AlgoRamSize = 2 * 1024

	MCU_ID = 0x0BC11477

	UID_ADDR = 0x1FF80050	   	--UID地址，不同的CPU不同
	UID_BYTES = 12

	--缺省校验模式
	VERIFY_MODE = 0				-- 0:读回校验, 1:软件CRC32校验, 其他:扩展硬件CRC(需要单片机支持）

	ERASE_CHIP_TIME = 500		--全片擦除时间ms（仅用于进度指示)

	--地址组中的FFFFFFFF表示原始数据中插入上个字节的反码 FFFFFFFE表示原始数据中插入前2个字节的反码
	OB_ADDRESS     = "1FF80000 1FF80001 FFFFFFFE FFFFFFFE 1FF80004 1FF80005 FFFFFFFE FFFFFFFE "
				   .."1FF80008 1FF80009 FFFFFFFE FFFFFFFE 1FF8000C 1FF8000D FFFFFFFE FFFFFFFE "
				   .."1FF80010 1FF80011 FFFFFFFE FFFFFFFE"
	OB_SECURE_OFF  = "AA 00 70 80 00 00 00 00 00 00"	--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
	OB_SECURE_ON   = "00 00 70 80 00 00 00 00 00 00"	--SECURE_ENABLE = 1时，编程完毕后写入该值

	OB_WRP_ADDRESS   = {0x1FF80000, 0x1FF80008, 0x1FF80009, 0x1FF8000C, 0x1FF8000D, 0x1FF80010, 0x1FF80011} 	--内存地址
	OB_WRP_MASK  	 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}		--读出数据与此数相与
	OB_WRP_VALUE 	 = {0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}		--相与后与此数比较，相等表示没有保护
end

---------------------------结束-----------------------------------
