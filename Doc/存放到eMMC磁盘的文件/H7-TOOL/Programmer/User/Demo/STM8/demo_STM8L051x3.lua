--以下快捷方式将显示在PC软件界面(STM8)
--F01=自动编程,start_prog()
--F03=擦除flash,erase_chip(FLASH_ADDRESS)
--F04=擦除eeprom,erase_chip(EEPROM_ADDRESS)
--F05=打印flash,print_flash(FLASH_ADDRESS,512,16,FLASH_ADDRESS)
--F06=打印eeprom,print_flash(EEPROM_ADDRESS,256,16,EEPROM_ADDRESS)
--F07=打印UID,print_flash(UID_ADDR,12)
--F08=打印内核id,print_core_id()
--F09=修改RAM,pg_write_mem(0, "1234")
--F10=读回RAM,print_flash(0, 16)
--F12=硬件复位,pg_reset()
--F13=设置读保护, set_read_protect(1)
--F14=解除读保护, set_read_protect(0)
--F15=打印Option Bytes,print_option_bytes()

--下面的注释将显示在H7-TOOL液晶屏
Note01 = "测试程序"

beep()

CHIP_NAME = "STM8L051F3"

--dofile("0:/H7-TOOL/Programmer/Device/ST/STM8S/STM8S103_903_003_001.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM8S/STM8S105_005_007.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM8S/STM8S207_208.lua")

--CHIP_NAME = "STM8L151C8"
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM8L/STM8L101.lua")
dofile("0:/H7-TOOL/Programmer/Device/ST/STM8L/STM8L151_152_05x_162.lua")

--CHIP_NAME = "STM8AF52AA"
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM8A/STM8AF6226_F6223_F6213.lua")
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM8A/STM8AF52xx_F62xx_F63xx.lua")

--UID加密和产品序号处理文件
dofile("0:/H7-TOOL/Programmer/LuaLib/fix_data.lua")

--公共lua子程序
dofile("0:/H7-TOOL/Programmer/LuaLib/prog_lib.lua")

--配置芯片接口和参数
function config_chip1(void)

	config_cpu()

	--如果解除读保护后，必须断电才能生效，则添加如下代码
	REMOVE_RDP_POWEROFF = 0
	POWEROFF_TIME1 = 0		--写完OB延迟时间 2000ms
	POWEROFF_TIME2 = 100	--断电时间 100ms
	POWEROFF_TIME3 = 20		--上电后等待时间 100ms

	--编程任务列表，可以任意追加
	--数据文件名支持绝对路径和相对路径，相对路径时和lua文件同目录，支持../上级目录
	TaskList = {
		"0:/H7-TOOL/Programmer/User/TestBin/8K_5A.bin",	--数据文件 (""表示忽略)
		0x008000,										--目标地址 (0x008000 Flash)

--		"0:/H7-TOOL/Programmer/User/TestBin/128.bin",	--数据文件 (""表示忽略)
--		EEPROM_ADDRESS,									--目标地址 (0x004000 EEPROM)
	}

	--定义CPU供电电压TVCC
	TVCC_VOLT = 3.3

	--1表示整片擦除，0表示按扇区擦除. 有些CPU整片擦除速度快很多，有些慢很多
	ERASE_CHIP_ENABLE = 1

	RESET_TYPE = 0				-- 0表示软件复位  1表示硬件复位

	--是否核对CPU内核ID
	CHECK_MCU_ID = 0

	--编程结束后复位 0表示不复位  1表示硬件复位
	RESET_AFTER_COMPLETE = 0

	AUTO_REMOVE_PROTECT = 1		--1表示自动解除读保护和写保护

	--OPTION BYTES 配置
	OB_ENABLE	= 0 			--1表示编程完毕后写OPTION BYTES
	SECURE_ENABLE  = 0			--选择加密还是不加密

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
