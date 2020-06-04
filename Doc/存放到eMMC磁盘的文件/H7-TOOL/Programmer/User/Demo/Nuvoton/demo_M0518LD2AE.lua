--以下快捷方式将显示在PC软件界面-------------
--F01=自动编程,start_prog()
--F03=擦除MCU flash,erase_chip_mcu()
--F04=擦除QSPI flash,erase_chip_qspi()
--F05=擦除EEPROM, erase_chip_eeprom()
--F06=打印内核id,print_core_id()
--F07=打印UID,MCU_ReadUID()
--F08=打印Flash,print_flash(FLASH_ADDRESS,1024,32,FLASH_ADDRESS)
--F09=读EEPROM,print_flash(EEPROM_ADDRESS, 256)
--F10=读回RAM,print_flash(RAM_ADDRESS, 256)
--F12=硬件复位,pg_reset()
--F13=设置读保护,set_read_protect(1)
--F14=解除读保护,set_read_protect(0)
--F15=打印CONFIG, print_option_bytes()
--F16=掉电1秒再上电,set_tvcc(0) delayms(1000) set_tvcc(TVCC_VOLT)

--选择芯片系列----------------------------------
--dofile("0:/H7-TOOL/Programmer/Device/ST/STM32F0xx/STM32F0xxx4.lua")
function config_cpu(void)
	CHIP_TYPE = "SWD"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C", "UART"

--          <algorithm  name  ="Flash/M0518_CFG.FLM"         start="0x00300000"  size="0x00000004"                   default="0"/>
--          <algorithm  name  ="Flash/M0518_LD_4.FLM"        start="0x00100000"  size="0x1000"                   default="0"/>
--          <algorithm  name  ="Flash/M0518_AP_68.FLM"        start="0x00000000"  size="0x11000"                   default="1"/>

	AlgoFile_FLASH = "0:/H7-TOOL/Programmer/Device/Nuvoton/FLM/M0518_AP_68.FLM"
	AlgoFile_OTP   = ""
	AlgoFile_OPT   = "0:/H7-TOOL/Programmer/Device/Nuvoton/FLM/M0518_CFG.FLM"
	AlgoFile_QSPI  = ""

	FLASH_ADDRESS = 0x00000000		--CPU内部FLASH起始地址

	RAM_ADDRESS = 0x20000000		--CPU内部RAM起始地址

	--Flash算法文件加载内存地址和大小
	AlgoRamAddr = RAM_ADDRESS
	AlgoRamSize = 4 * 1024

	MCU_ID = 0x0BB11477

	UID_ADDR = 0x1FFFF7AC	   	--UID地址，不同的CPU不同
	UID_BYTES = 12

	--缺省校验模式
	VERIFY_MODE = 0				-- 0:读回校验, 1:软件CRC32校验, 其他:扩展硬件CRC(需要单片机支持）

	ERASE_CHIP_TIME = 5000		--全片擦除时间ms（仅用于进度指示)

	OB_ADDRESS     = "00300000 00300001 00300002 00300003 00300004 00300005 00300006 00300007"

	OB_SECURE_OFF  = "FF FF FF FF FF FF FF FF"	--SECURE_ENABLE = 0时，编程完毕后写入该值(解除加密)
	OB_SECURE_ON   = "FD FF FF FF FF FF FF FF"	--SECURE_ENABLE = 1时，编程完毕后写入该值(芯片加密)

	--判断读保护和写保护的条件(WRP = Write protection)
	OB_WRP_ADDRESS   = {0x00300000} 	--内存地址
	OB_WRP_MASK  	 = {0x20}		--读出数据与此数相与
	OB_WRP_VALUE 	 = {0x20}		--相与后与此数比较，相等表示没有保护

	--启用MCU专有的解除读保护指令
	MCU_REMOVE_PROTECT = 1		--1表示需要专门的解除保护的函数
	MCU_READ_OPTION = 1			--1表示需要专门的读选项字的函数

	MCU_Init()
end

--新唐寄存器定义
FMC_ISPCON = 0x5000C000
FMC_ISPADR = 0x5000C004
FMC_ISPDAT = 0x5000C008
FMC_ISPCMD = 0x5000C00C
FMC_ISPTRG = 0x5000C010
--FMC_DFBADR = 0x5000C014
--FMC_FATCON = 0x5000C018
--FMC_ISPSTA = 0x5000C040

ISPCMD_READ        = 0x00
ISPCMD_PROGRAM     = 0x21
ISPCMD_PAGE_ERASE  = 0x22
ISPCMD_VECMAP      = 0x2e
ISPCMD_READ_UID    = 0x04
ISPCMD_READ_CID    = 0x0B
ISPCMD_READ_DID    = 0x0C

FMC_ISPCON_ISPFF_Msk = 0x40

g_apromSize = 32 * 1024
g_dataFlashAddr = 0
g_dataFlashSize = 0

function MCU_Init(void)
	--SYS_UnlockReg()
	local i

	for i = 1,1000, 1 do
		if (pg_read32(0x50000100) == 1) then
			break
		end
		pg_write32(0x50000100, 0x59)
		pg_write32(0x50000100, 0x16)
		pg_write32(0x50000100, 0x88)
	end


	--
	--pg_write32(0x50000204, pg_read32(0x50000204) | 0x04)

	--FMC_Open()
	pg_write32(FMC_ISPCON, pg_read32(FMC_ISPCON) | 0x39)
end


function MCU_ReadUID(void)
	local id
	local i,j
	local s = ""
	local str

	for i = 1, 3, 1 do
		pg_write32(FMC_ISPCMD, ISPCMD_READ_UID)
	 	pg_write32(FMC_ISPADR, (i - 1) * 4)
	 	pg_write32(FMC_ISPTRG, 0x01)

	 	for j = 1, 1000, 1 do
	 		if ((pg_read32(FMC_ISPTRG) & 0x01) == 0) then
	 			break
	 		end
		end

	 	id = pg_read32(FMC_ISPDAT)

		s = s..string.char(id & 0xFF)
		s = s..string.char((id >> 8) & 0xFF)
		s = s..string.char((id >> 16) & 0xFF)
		s = s..string.char((id >> 24) & 0xFF)
	end

	str = "uid  = "..bin2hex(s)  print(str)

	return s
end

function FMC_Read_User(address)
	local reg

	pg_write32(FMC_ISPCMD, ISPCMD_READ)		--ISPCMD = ISP_Read
 	pg_write32(FMC_ISPADR, address)	--ISPADR = address
 	pg_write32(FMC_ISPDAT, 0) 		--ISPDAT = 0

 	pg_write32(FMC_ISPTRG, 0x01)	--ISPTRG = ISPGO

	reg = pg_read32(FMC_ISPCON)		--read FISPCON
	if ((reg & 0x40) > 0) then
		pg_write32(FMC_ISPCON, reg)
		return -1
	end
 	return pg_read32(FMC_ISPDAT)	--read ISPDAT
end

function FMC_Write_User(u32Addr, u32Data)
    local Reg;

    pg_write32(FMC_ISPCMD, ISPCMD_PROGRAM)	--ISPCMD = FMC_ISPCMD_PROGRAM
	pg_write32(FMC_ISPADR, u32Addr)	--ISPADR = u32Addr
	pg_write32(FMC_ISPDAT, u32Data)	--ISPDAT = u32Data

	pg_write32(FMC_ISPTRG, 0x01)	--ISPTRG = ISPGO

	--Check ISPFF flag to know whether erase OK or fail
	reg = pg_read32(FMC_ISPCON)		--read FISPCON
	if ((reg & 0x40) > 0) then
		pg_write32(FMC_ISPCON, reg)
		return -1
	else
		return 0
	end
end

function FMC_Erase_User(address)
	local reg

	pg_write32(FMC_ISPCMD, ISPCMD_PAGE_ERASE)	--ISPCMD = FMC_ISPCMD_PAGE_ERASE
 	pg_write32(FMC_ISPADR, address)	--ISPADR = address
 	pg_write32(FMC_ISPTRG, 0x01)	--ISPTRG = ISPGO

 	for j = 1, 1000, 1 do
 		if ((pg_read32(FMC_ISPTRG) & 0x01) == 0) then
 			break
 		end
 		delayms(1)
	end

	--Check ISPFF flag to know whether erase OK or fail
	reg = pg_read32(FMC_ISPCON)		--read FISPCON
	if ((reg & 0x40) > 0) then
		pg_write32(FMC_ISPCON, reg)
		return -1
	else
		return 0
	end
end

function EraseAP(addr_start, addr_end)
	while (addr_start < addr_end) do
		 if (FMC_Erase_User(addr_start)  == -1) then
		 	print("EraseAP failed", addr_start, addr_end)
		 	return -1
		 end
		 addr_start = addr_start + 512
	end
	print("EraseAP Ok", addr_start, addr_end)
	return 0
end

function UpdateConfig(cfg0, cfg1)
    --FMC_ENABLE_CFG_UPDATE()
    pg_write32(FMC_ISPCON, pg_read32(FMC_ISPCON) | 0x10)

    if (FMC_Erase_User(0x00300000) == -1) then
    	print("FMC_Erase_User failed", 0x00300000)
    end
    FMC_Write_User(0x00300000, cfg0);
    FMC_Write_User(0x00300004, cfg1);
end

--芯片专有的解除保护函数
function MCU_ReadOptionsByte(addr, bytes)
	local cfg1, cfg2
	local re
	local data = 0
	local idx

	cfg1 = FMC_Read_User(0x00300000)
	cfg2 = FMC_Read_User(0x00300004)

	if (cfg1 == -1 or cfg2 == -1) then
		re = 0
		return 0,0
	end


	if (addr >= 0x00300000 and addr <= 0x00300003) then
		idx = addr - 0x00300000
		data = (cfg1 >> (idx * 8)) & 0xFF
	else
		if (addr >= 0x00300004 and addr <= 0x00300007) then
			idx = addr - 0x00300004
			data = (cfg2 >> (idx * 8)) & 0xFF
		else
			return 0,0
		end
	end
	return 1, data
end

--芯片专有的解除保护函数
function MCU_RemoveProtect(void)
	local i

	pg_write32(FMC_ISPCMD, 0x26)
 	pg_write32(FMC_ISPADR, 0)
 	pg_write32(FMC_ISPDAT, 0)
 	pg_write32(FMC_ISPTRG, 0x01)

 	for i = 1, 5000, 1 do
 		if ((pg_read32(FMC_ISPTRG) & 0x01) == 0) then
 			break
 		end
 		delayms(1)
	end
end

--Supports 32K/64K (APROM)
function GetApromSize(void)
	local size = 0xA000, data;

    data = FMC_Read_User(size)

    if (data < 0) then
        return 32 * 1024;
    else
        return 64 * 1024;
   	end
end



function GetDataFlashInfo(void)
    local uData;

	CONFIG0_DFEN = 0x01
	CONFIG0_DFVSEN = 0x04

    g_apromSize = GetApromSize()

    g_dataFlashSizee = 0;

--   	Note: DFVSEN = 1, DATA Flash Size is 4K bytes
--             DFVSEN = 0, DATA Flash Size is based on CONFIG1
	uData = FMC_Read_User(0x00300000)

    if ((uData & CONFIG0_DFVSEN) > 0) then
        g_dataFlashAddr = 0x1F000
        g_dataFlashSize = 4 * 1024
   	else
   		if ((uData & CONFIG0_DFEN) > 0) then
	        g_apromSize = g_apromSize + 4096
	        g_dataFlashAddr = g_apromSize
	        g_dataFlashSize = 0
	    else
			g_apromSize = g_apromSize + 4096
	        uData = FMC_Read_User(0x00300004)
	        uData = uData & 0x000FFE00

			if (uData > g_apromSize + 4096) then
				uData = g_apromSize
	        end

	        g_dataFlashAddr = uData
	        g_dataFlashSize = g_apromSize - uData
	        g_apromSize = g_apromSize - g_dataFlashSize
	    end
	end
end

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

	--如果解除读保护后，必须断电才能生效，则添加如下代码
	REMOVE_RDP_POWEROFF = 0
	POWEROFF_TIME1 = 0	--写完OB延迟时间 2000ms
	POWEROFF_TIME2 = 100	--断电时间 100ms
	POWEROFF_TIME3 = 20	--上电后等待时间 100ms

	--编程任务列表，可以任意追加
	--算法文件名和数据文件名支持绝对路径和相对路径，相对路径时和lua文件同目录，支持../上级目录
	TaskList = {
		AlgoFile_FLASH,							--算法文件
		"0:/H7-TOOL/Programmer/User/TestBin/64k_55.bin",  	--数据文件
		FLASH_ADDRESS,							--目标地址
	}

	--定义CPU供电电压TVCC
	TVCC_VOLT = 3.3

	--SWD时钟延迟，0不延迟，值越大速度越慢
	if (MULTI_MODE == 0) then
		SWD_CLOCK_DELAY = 0		--单路编程
	else
		SWD_CLOCK_DELAY = 0		--多路编程，根据实际板子调节，和CPU主频、电缆长度有关
	end

	--1表示整片擦除，0表示按扇区擦除. 有些CPU整片擦除速度快很多，有些慢很多
	ERASE_CHIP_ENABLE = 0

	RESET_TYPE = 0				-- 0表示软件复位  1表示硬件复位

	--是否核对CPU内核ID
	CHECK_MCU_ID = 0

	VERIFY_MODE = 0				--校验模式: 0:自动(FLM提供校验函数或读回) 1:读回  2:软件CRC32  3:STM32硬件CRC32

	--编程结束后复位 0表示不复位  1表示硬件复位
	RESET_AFTER_COMPLETE = 1

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

MULTI_MODE = pg_read_c_var("MultiProgMode")

config_chip1()				--执行一次给全局变量赋初值

config_fix_data()			--动态填充SN UID USR数据

---------------------------结束-----------------------------------
