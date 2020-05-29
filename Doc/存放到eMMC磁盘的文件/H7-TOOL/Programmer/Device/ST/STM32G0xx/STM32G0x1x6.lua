
-------------------------------------------------------
-- 文件名 : STM32G0x1x6.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

	AlgoFile_FLASH = "0:/H7-TOOL/Programmer/Device/ST/STM32G0xx/STM32G0xx_32.FLM"
	AlgoFile_OTP   = "0:/H7-TOOL/Programmer/Device/ST/STM32G0xx/STM32G0xx_OTP.FLM"
	AlgoFile_OPT   = "0:/H7-TOOL/Programmer/Device/ST/STM32G0xx/STM32G0x1_OPT.FLM"
	AlgoFile_QSPI  = ""

	FLASH_ADDRESS = 0x08000000		--CPU内部FLASH起始地址

	OTP_ADDRESS = 0x1FFF7000		--CPU内部OTP区地址
	OTP_SIZE = 1024

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = RAM_ADDRESS
	AlgoRamSize = 4 * 1024

	MCU_ID = 0x0BC11477

	UID_ADDR = 0x1FFF7590	   	--UID地址，STM32G0 手册中没有UID
	UID_BYTES = 12

	--缺省校验模式
	VERIFY_MODE = 0				-- 0:读回校验, 1:软件CRC32校验, 其他:扩展硬件CRC(需要单片机支持）

	ERASE_CHIP_TIME = 500		--全片擦除时间ms（仅用于进度指示)

	OB_ADDRESS     =  "1FFF7800 1FFF7801 1FFF7802 1FFF7803 "		--FFFF FEAA
					.."1FFF7808 1FFF7809 1FFF780A 1FFF780B "		--FFFF FFFF
					.."1FFF7810 1FFF7811 1FFF7812 1FFF7813 "		--0000 0000
					.."1FFF7818 1FFF7819 1FFF781A 1FFF781B "		--0000 000F
					.."1FFF7820 1FFF7821 1FFF7822 1FFF7823 "		--0000 000F
					.."1FFF7828 1FFF7829 1FFF782A 1FFF782B "		--FFFF FFFF
					.."1FFF7830 1FFF7831 1FFF7832 1FFF7833 "		--0000 0000
					.."1FFF7870 1FFF7871 1FFF7872 1FFF7873 "		--0000 0000

	OB_SECURE_OFF  = "AA FE FF FF  FF FF FF FF  00 00 00 00  0F 00 00 00  0F 00 00 00  FF FF FF FF 00 00 00 00  00 00 00 00 "	--SECURE_ENABLE = 0时，编程完毕后写入该值(解除加密)

	OB_SECURE_ON   = "00 FE FF FF  FF FF FF FF  00 00 00 00  0F 00 00 00  0F 00 00 00  FF FF FF FF 00 00 00 00  00 00 00 00  "	--SECURE_ENABLE = 1时，编程完毕后写入该值(芯片加密)

	--判断读保护和写保护的条件(WRP = Write protection)
	OB_WRP_ADDRESS   = {0x1FFF7800, 0x1FFF7818, 0x1FFF781A, 0x1FFF7820, 0x1FFF7822} 	--内存地址
	OB_WRP_MASK  	 = {0xFF, 0x3F, 0x3F, 0x3F, 0x3F}		--读出数据与此数相与
	OB_WRP_VALUE 	 = {0xAA, 0x1F, 0x00, 0x1F, 0x00}		--相与后与此数比较，相等表示没有保护
end


---------------------------结束-----------------------------------
