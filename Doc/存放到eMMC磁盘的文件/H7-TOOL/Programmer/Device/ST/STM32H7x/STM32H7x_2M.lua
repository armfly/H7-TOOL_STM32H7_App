-------------------------------------------------------
-- 文件名 : STM32H7x2M.lua
-- 版  本 : V1.0  2020-04-23 armfly
-- 说  明 : STM32H743、STM32H750, 2MB Flash算法
-------------------------------------------------------
function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

	AlgoFile_FLASH = "0:/H7-TOOL/Programmer/Device/ST/STM32H7x/STM32H7x_2M.FLM"
	AlgoFile_OTP   = ""			--OTP算法文件
	AlgoFile_OPT   = ""			--Option Bytes 算法文件
	AlgoFile_QSPI  = "0:/H7-TOOL/Programmer/Device/ST/STM32H7x/STM32H7XX_H7-TOOL_W25Q256.FLM"	--片外QSPI算法文件

	FLASH_ADDRESS = 0x08000000		--CPU内部FLASH起始地址

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = 0x20000000
	AlgoRamSize = 64*1024

	MCU_ID = 0x6BA02477

	UID_ADDR = 0x1FF1E800	   	--UID地址，不同的CPU不同
	UID_BYTES = 12

	--缺省校验模式
	VERIFY_MODE = 0				-- 0:读回校验, 1:软件CRC32校验, 其他:扩展硬件CRC(需要单片机支持）

	ERASE_CHIP_TIME = 11000		--全片擦除时间ms（仅用于进度指示)

	OB_ADDRESS     = "5200201C 5200201D 5200201E 5200201F "..
					 "52002028 52002029 5200202A 5200202B "..
					 "52002030 52002031 52002032 52002033 "..
					 "52002038 "..
					 "52002040 52002041 52002042 52002043 "..
					 "52002128 52002129 5200212A 5200212B "..
					 "52002130 52002131 52002132 52002133 "..
					 "52002138"

	OB_SECURE_OFF  = "F0AAC603  FF000000 FF000000 FF 00080000 FF000000 FF000000 FF"	--SECURE_ENABLE = 0时，编程完毕后写入该值
	OB_SECURE_ON   = "F000C603  FF000000 FF000000 FF 00080000 FF000000 FF000000 FF"	--SECURE_ENABLE = 1时，编程完毕后写入该值

	--判断读保护和写保护的条件(WRP = Write protection)
	OB_WRP_ADDRESS   = {0x5200201D, 0x52002038,0x52002138} 	--内存地址
	OB_WRP_MASK  	 = {0xFF, 0xFF, 0xFF}					--读出数据与此数相与
	OB_WRP_VALUE 	 = {0xAA, 0xFF, 0xFF}					--相与后与此数比较，相等表示没有写保护
end

---------------------------结束-----------------------------------
