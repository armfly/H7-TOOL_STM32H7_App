--以下快捷方式将显示在PC软件界面-------------
--F01=自动编程,start_prog()
--F03=擦除MCU flash,erase_chip_mcu()
--F04=擦除QSPI flash,erase_chip_qspi()
--F05=擦除EEPROM, erase_chip_eeprom()
--F06=打印内核id,print_core_id()
--F07=打印UID,print_flash(UID_ADDR,12)
--F08=打印Flash,print_flash(FLASH_ADDRESS,1024,32,FLASH_ADDRESS)
--F09=读EEPROM,print_flash(EEPROM_ADDRESS, 256)
--F10=读回RAM,print_flash(RAM_ADDRESS, 256)
--F12=硬件复位,pg_reset()
--F13=设置读保护,set_read_protect(1)
--F14=解除读保护,set_read_protect(0)
--F15=打印Option Bytes,print_option_bytes()

--选择芯片系列----------------------------------
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F401xx_128.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F401xx_256.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F401xx_384.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F401xx_512.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F40xxx_41xxx_512.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F40xxx_41xxx_1024.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F410xx_412xx_128.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F410xx_412xx_256.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F410xx_412xx_512.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F410xx_412xx_1024.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F411xx_256.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F411xx_512.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F413xx_423xx_1024.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F413xx_423xx_1536.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F42xxx_43xxx_512.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F42xxx_43xxx_1024.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F42xxx_43xxx_1536.lua")
dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F42xxx_43xxx_2048.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F446xx_256.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F446xx_512.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F469xx_479xx_512.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F469xx_479xx_1024.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F4xx/STM32F469xx_479xx_2048.lua")

--UID加密和产品序号处理文件
dofile("0:/H7-TOOL/Programmer/LuaLib/fix_data.lua")

--公共lua子程序
dofile("0:/H7-TOOL/Programmer/LuaLib/prog_lib.lua")

--下面的注释将显示在H7-TOOL液晶屏
Note01 = "测试程序"

beep()

--配置芯片接口和参数
function config_chip1(void)

	config_cpu()

	--编程任务列表，可以任意追加
	--算法文件名和数据文件名支持绝对路径和相对路径，相对路径时和lua文件同目录，支持../上级目录
	TaskList = {
		AlgoFile_FLASH,							--算法文件
		"0:/H7-TOOL/Programmer/User/TestBin/2M_55.bin",  	--数据文件
		0x08000000,								--目标地址
	}

	--定义CPU供电电压TVCC
	TVCC_VOLT = 3.3

	--SWD时钟延迟，0不延迟，值越大速度越慢
	if (MULTI_MODE == 0) then
		SWD_CLOCK_DELAY = 0		--单路编程
	else
		SWD_CLOCK_DELAY = 1		--多路编程，根据实际板子调节，和CPU主频、电缆长度有关
	end

	--1表示整片擦除，0表示按扇区擦除. 有些CPU整片擦除速度快很多，有些慢很多
	ERASE_CHIP_ENABLE = 1

	RESET_TYPE = 0				-- 0表示软件复位  1表示硬件复位

	--是否核对CPU内核ID
	CHECK_MCU_ID = 0

	VERIFY_MODE = 0				--校验模式: 0:自动(FLM提供校验函数或读回) 1:读回  2:软件CRC32  3:STM32硬件CRC32

	--编程结束后复位 0表示不复位  1表示硬件复位
	RESET_AFTER_COMPLETE = 0

	AUTO_REMOVE_PROTECT = 1		--1表示自动解除读保护和写保护

	--OPTION BYTES 配置
	OB_ENABLE	= 0 				--1表示编程完毕后写OPTION BYTES
	SECURE_ENABLE  = 0				--选择加密还是不加密

	pg_reload_var()				--用于更新c程序的全局变量
end

--动态填充SN UID USR数据
function config_fix_data(void)
	SN_ENABLE = 0				--1表示启用   0表示不启用
	SN_SAVE_ADDR = 0			--产品序号保存地址

	UID_ENABLE = 0	       		--1表示启用加密函数1  0表示不启用
	UID_SAVE_ADDR = 0 			--加密结果FLASH存储地址

	USR_ENABLE = 0	       		--1表示启用   0表示不启用
	USR_SAVE_ADDR = 0 			--自定义数据存储地址
end

config_chip1()				--执行一次给全局变量赋初值

config_fix_data()			--动态填充SN UID USR数据

MULTI_MODE = pg_read_c_var("MultiProgMode")

---------------------------结束-----------------------------------
