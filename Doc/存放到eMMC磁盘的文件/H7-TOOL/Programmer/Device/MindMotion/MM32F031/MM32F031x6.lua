
-------------------------------------------------------
-- 文件名 : MM32F031x6.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

	AlgoFile_FLASH = "0:/H7-TOOL/Programmer/Device/MindMotion/MM32F031/MM32xx_32.FLM"
	AlgoFile_OTP   = ""
	AlgoFile_OPT   = "0:/H7-TOOL/Programmer/Device/MindMotion/OPT/MM32_M0_npq_OPT.FLM"
	AlgoFile_QSPI  = ""

	FLASH_ADDRESS = 0x08000000		--CPU内部FLASH起始地址

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = RAM_ADDRESS
	AlgoRamSize = 4 * 1024

	MCU_ID = 0x0BB11477

	UID_ADDR = 0x1FFFF7E8	   	--UID地址，不同的CPU不同
	UID_BYTES = 12

	--缺省校验模式
	VERIFY_MODE = 0				-- 0:读回校验, 1:软件CRC32校验, 其他:扩展硬件CRC(需要单片机支持）

	--地址组中的FFFFFFFF表示原始数据中插入一个取反的字节
	OB_ADDRESS     = "1FFFF800 FFFFFFFF 1FFFF802 FFFFFFFF 1FFFF804 FFFFFFFF 1FFFF806 FFFFFFFF 1FFFF808 FFFFFFFF 1FFFF80A FFFFFFFF 1FFFF80C FFFFFFFF 1FFFF80E FFFFFFFF "
			       .."1FFE0000 1FFE0001 1FFE0002 1FFE0003 "
	OB_SECURE_OFF  = "A5 FF FF FF FF FF FF FF FF FF FF FF "	--SECURE_ENABLE = 0时，编程完毕后写入该值
	OB_SECURE_ON   = "A5 FF FF FF FF FF FF FF 80 7F 00 FF "	--SECURE_ENABLE = 1时，编程完毕后写入该值

	--判断读保护和写保护的条件(WRP = Write protection)
	OB_WRP_ADDRESS   = {0x1FFFF800, 0x1FFFF808, 0x1FFFF80A, 0x1FFFF80C, 0x1FFFF80E,		0x1FFE0000, 0x1FFE0002} 	--内存地址
	OB_WRP_MASK  	 = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}		--读出数据与此数相与
	OB_WRP_VALUE 	 = {0xA5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}		--相与后与此数比较，相等表示没有保护
end

---------------------------结束-----------------------------------
