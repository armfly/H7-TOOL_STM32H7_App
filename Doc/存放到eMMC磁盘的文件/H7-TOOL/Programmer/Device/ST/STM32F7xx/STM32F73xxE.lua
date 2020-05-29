
-------------------------------------------------------
-- 文件名 : STM32F73xxE.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------

function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

	AlgoFile_FLASH = "0:/H7-TOOL/Programmer/Device/ST/STM32F7xx/STM32F7x2_512.FLM"
	AlgoFile_EEPROM = ""
	AlgoFile_OTP   = "0:/H7-TOOL/Programmer/Device/ST/STM32F7xx/STM32F7x2_OTP.FLM"
	AlgoFile_OPT   = "0:/H7-TOOL/Programmer/Device/ST/STM32F7xx/STM32F72x_73x_OPT.FLM"
	AlgoFile_QSPI  = ""

	FLASH_ADDRESS = 0x08000000		--CPU内部FLASH起始地址
	FLASH_SIZE =  512 * 1024		--覆盖FLM中的 Device Size

	--EEPROM_ADDRESS = 0				--CPU内部EEPROM起始地址
	--EEPROM_SIZE = 0

	OTP_ADDRESS		= 0x1FF07800
	OTP_SIZE		= 0x0210

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = RAM_ADDRESS
	AlgoRamSize = 4 * 1024

	MCU_ID = 0x5BA02477

	UID_ADDR = 0x1FF0F420	   	--UID地址，不同的CPU不同
	UID_BYTES = 12

	--缺省校验模式
	VERIFY_MODE = 0				-- 0:读回校验, 1:软件CRC32校验, 其他:扩展硬件CRC(需要单片机支持）

	ERASE_CHIP_TIME = 8000		--全片擦除时间ms（仅用于进度指示)

	--地址组中的FFFFFFFF表示原始数据中插入一个取反的字节
	OB_ADDRESS     = "1FFF0000 1FFF0001 1FFF0008 1FFF0009 1FFF0010 1FFF0011 1FFF0018 1FFF0019"

	OB_SECURE_OFF  = "FF AA FF FF 80 00 40 00"	--SECURE_ENABLE = 0时，编程完毕后写入该值
	OB_SECURE_ON   = "FF 00 FF FF 80 00 40 00"	--SECURE_ENABLE = 1时，编程完毕后写入该值

	--判断读保护和写保护的条件(WRP = Write protection)
	OB_WRP_ADDRESS   = {0x1FFF0001, 0x1FFF0008} 	--内存地址
	OB_WRP_MASK  	 = {0xFF, 0xFF}					--读出数据与此数相与
	OB_WRP_VALUE 	 = {0xAA, 0xFF}					--相与后与此数比较，相等表示没有保护
end

---------------------------结束-----------------------------------
