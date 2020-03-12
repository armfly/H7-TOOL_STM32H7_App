--以下快捷方式将显示在PC软件界面(STM8)
--F01=自动编程,start_prog()
--F03=擦除flash,erase_chip(0x08000)
--F04=擦除eeprom,erase_chip(0x04000)
--F05=打印flash,print_flash(0x08000,1024,32,0x08000)
--F06=打印eeprom,print_flash(0x04000,256,32,0x04000)
--F07=打印UID,print_flash(UID_ADDR,12)
--F08=打印内核id,print_core_id()
--F09=修改RAM,pg_write_mem(0, "1234")
--F10=读回RAM,print_flash(0, 16)
--F12=硬件复位,pg_reset()
--F13=设置读保护, set_read_protect(1)
--F14=解除读保护, set_read_protect(0)
--F15=打印Option Bytes,print_option_bytes()

--下面的注释将显示在H7-TOOL液晶屏
Note01 = "STM8S208 测试程序"

beep()

--配置芯片接口和参数（全局变量)
function cofig_chip1(void)	
	CHIP_TYPE = "SWIM"		--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C" 	
	STM8_SERIAL = "STM8S"	--选择2个系列: "STM8S" 或 "STM8L"	
	FLASH_BLOCK_SIZE = 128	--定义BLOCK SIZE, 只有64和128两种 
	FLASH_ADDRESS = 0x008000	--定义FLASH起始地址
	FLASH_SIZE = 64 * 1024  --定义FLASH总容量
	EEPROM_ADDRESS = 0x004000 	--定义FLASH起始地址(STM8S和STM8L不同）
	EEPROM_SIZE = 2 * 1024  --定义EEPROM容量
	UID_ADDR = 0x48CD		--UID地址，不同的CPU不同   		
	
	--任务列表，可以任意追加
	--数据文件和lua文件同目录.支持../上级目录,也可以写绝对路径
	TaskList = {
		"0:/H7-TOOL/Programmer/User/TestBin/64K_55.bin",	--数据文件 (""表示忽略)
		0x008000,											--目标地址 (0x008000 Flash)
		
		"0:/H7-TOOL/Programmer/User/TestBin/512.bin",		--数据文件 (""表示忽略)
		0x004000,											--目标地址 (0x004000 EEPROM)	
	}	

	TVCC_VOLT = 3.3			--定义CPU供电电压TVCC
	
	--1表示整片擦除后编程 0表示按BLOCK编程，未用BLOCK保持原状
	ERASE_CHIP_ENABLE = 0
	
	--编程结束后复位 0表示不复位  1表示硬件复位
	RESET_AFTER_COMPLETE = 0

	--OPTION BYTES 配置（STM8S208x8)
	OB_ENABLE	= 0 				--1表示编程完毕后写OPTION BYTES
	--地址组中的FFFFFFFF表示原始数据中插入上个字节的反码 FFFFFFFE表示原始数据中插入前2个字节的反码
	OB_ADDRESS     = "4800 4801 FFFF 4803 FFFF 4805 FFFF 4807 FFFF 4809 FFFF 480B FFFF 480D FFFF 487E FFFF"
	SECURE_ENABLE  = 0				--选择加密还是不加密	
	
	OB_SECURE_OFF  = "00 00 00 00 00 00 00 00 00"	--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
	OB_SECURE_ON   = "AA 00 00 00 00 00 00 00 00"	--SECURE_ENABLE = 1时，编程完毕后写入该值
	
	OB_RDP_OFF     = "00 00 00 00 00 00 00 00 00"	--OPTION缺省值,用于解除保护
	OB_RDP_ON      = "AA 00 00 00 00 00 00 00 0"	--读保护的值，用于解除保护		
	
	pg_reload_var()			--用于更新c程序的全局变量
end

-------------------------------------------------------
-- 下面的代码一般无需更改(STM8和ARM芯片相同）
-------------------------------------------------------

cofig_chip1()				--执行一次给全局变量赋初值

--编程入口
function start_prog(void)	
	local err = ""
	local core_id
	local str
	
	cofig_chip1()		--配置烧录参数			
	set_tvcc(0)			--断电
	delayms(20)
	set_tvcc(TVCC_VOLT)	--设置TVCC电压	
	core_id = pg_detect_ic()
	if (core_id > 0) then
		str = string.format("core_id = 0x%08X", core_id)
		print(str)	

		--核对core id
		if (CHECK_MCU_ID == 1) then
			if (core_id ~= MCU_ID) then
				err = "MCU ID不正确"
				pg_print_text(err)
				goto quit
			end
		end
		
		if (CHIP_TYPE == "SWD") then
			err = swd_start_prog()	--编程ARM (SWD)
		else
			err = swim_start_prog()	--编程STM8 (SWD)
		end
		if (err ~= "OK") then goto quit end		
		goto quit
	end
	
	err = "未检测到IC"	

::quit::
	if (err == "OK") then
		beep() --成功叫1次
		pg_print_text("编程成功")
	else
		beep(5, 5, 3) --失败叫3次
		if (err ~= "error") then
			pg_print_text(err)	
		end
	end

	return err
end

--判断芯片移除（用于连续烧录）
function CheckChipRemove(void)
	local core_id
	
	core_id = pg_detect_ic()
	if (core_id == 0) then
		return "removed"
	end
	
	return "no"
end

--判断芯片插入（用于连续烧录）
function CheckChipInsert(void)
	cofig_chip1()
	core_id = pg_detect_ic()
	if (core_id > 0) then
		return "inserted"
	end
	
	return "no"
end

-------------------------------------------------------
-- 下面的代码用于烧录产品和程序UID加密
-------------------------------------------------------

--开始修正数据文件，动态填充SN UID USR数据
function fix_data_begin(void)
	--产品编码（滚码）设置（固定4字节）
	SN_ENABLE = 0				--1表示启用   0表示不启用
	SN_SAVE_ADDR = 0			--产品序号保存地址
	SN_INIT_VALUE = 1			--产品序号初始值
	SN_LITTLE_ENDIN = 1			--0表示大端，1表示小端
	SN_DATA = ""				--序号内容，由后面的 sn_new() 函数生成
	SN_LEN = 0					--序号长度
	
	--UID加密存储设置（只针对有UID的MCU）
	UID_ENABLE = 0	       		--1表示启用加密  0表示不启用
	UID_BYTES = 12             	--UID长度
	UID_SAVE_ADDR = 0 			--加密结果FLASH存储地址
	UID_DATA = ""				--加密数据内容，由后面的 uid_encrypt() 函数生成
	UID_LEN = 0					--数据长度
	
	--用户自定义数据（可填充生产日期，客户编号等数据，要求数据连续）
	USR_ENABLE = 0	       		--1表示启用   0表示不启用
	USR_SAVE_ADDR = 0 			--自定义数据存储地址
	USR_DATA = ""				--自定义数据内容，由后面的 make_user_data() 函数生成
	USR_LEN = 0					--数据长度
	
	local str
	local re
		
	--读文件中上次SN并生成新的SN数据
	if (SN_ENABLE == 1) then
		SN_DATA = sn_new()	--根据上次SN生4成新的SN
		SN_LEN = string.len(SN_DATA)
		str = "new sn  = "..bin2hex(SN_DATA) print(str)
	end
	
	--读UID（unique device identifier) 并生成UID加密数据
	re,mcu_uid = pg_read_mem(UID_ADDR, UID_BYTES)
	if (re == 1) then
		str = "uid original = "..bin2hex(mcu_uid)
		print(str)

		if (UID_ENABLE == 1) then
			UID_DATA = uid_encrypt(mcu_uid)
			UID_LEN = string.len(UID_DATA)
			str = "uid encrypt  = "..bin2hex(UID_DATA)
			print(str)
		end
	end

	--动态生成用户区数据
	if (USR_ENABLE == 1) then
		USR_DATA = make_user_data()
		USR_LEN = string.len(USR_DATA)
		str = "user data  = "..USR_DATA
		print(str)
	end	
end

--结束修正数据文件
function fix_data_end(void)
	SN_ENABLE = 0
	UID_ENABLE = 0	
	USR_ENABLE = 0
end	

--产品序号SN生成函数 （last_sn是一个UINT32整数滚码）输出是二进制字符串
function sn_new(void)
	local bin = {}
	local out = {}
	local sn1

	sn1 = pg_read_sn()	--读上次SN （必须是整数）
	str = string.format("last sn = %d", sn1) print(str)		
	if (sn1 == nil) then
		sn1 = SN_INIT_VALUE
	else
		sn1 = sn1 + 1	--序号步长=1
	end
	
	pg_write_sn(sn1)	--编程成功后才会保存本次SN

	--拼接为二进制串返回
	if (SN_LITTLE_ENDIN == 1) then
		s =    string.char(sn1)
		s = s..string.char(sn1 >> 8)
		s = s..string.char(sn1 >> 16)
		s = s..string.char(sn1 >> 24)	
	else
		s =    string.char(sn1 >> 24)
		s = s..string.char(sn1 >> 16)
		s = s..string.char(sn1 >> 8)
		s = s..string.char(sn1)
	end
	return s;
end

--UID加密函数，用户可自行修改
--	&	按位与
--	|	按位或
--	~	按位异或
--	>>	右移
--	<<	左移
--	~	按位非
function uid_encrypt(uid)
	local bin = {}
	local out = {}
	local i

	--将二进制的uid字符串转换为lua整数数组
	for i = 1,12,1 do
		bin[i] = tonumber(string.byte(uid, i,i))
	end

	--进行逻辑运算 用户可自行修改为保密算法
	out[1] = bin[1] ~ (bin[5] >> 1) ~ (bin[9]  >> 1)
	out[2] = bin[2] ~ (bin[6] >> 1) ~ (bin[10] >> 2)
	out[3] = bin[3] ~ (bin[7] >> 1) ~ (bin[11] >> 3)
	out[4] = bin[4] ~ (bin[8] >> 2) ~ (bin[12] >> 4)

    out[1] = out[4] ~ 0x12
    out[2] = out[3] ~ 0x34
    out[3] = out[1] ~ 0x56
    out[4] = out[2] ~ 0x78

	--拼接为二进制串返回
	s =    string.char(out[1])
	s = s..string.char(out[2])
	s = s..string.char(out[3])
	s = s..string.char(out[4])
	return s
end

--动态生成用户区数据USR
function make_user_data(uid)
	s = os.date("%Y-%m-%d %H:%M:%S")	--返回ASCII字符串 2020-01-21 23:25:01
	s = s.." aaa"..string.char(0)
	return s
end

-------------------------------------------------------
-- 下面的代码一般无需修改
-------------------------------------------------------

--开始编程SWD接口芯片
--供电-擦除全片-动态生成SN、UID加密数据、用户数据
--编程文件(自动擦除、编程、校验)
--写OPTION BYTES
function swd_start_prog(void)
	local err = "OK"
	local re
	local core_id
	local uid_bin
	local last_sn
	local str
	local mcu_uid
	local ob_data
	local i	
		
	--判断读保护和写保护，如果保护了则执行解锁操作
	if (AUTO_REMOVE_PROTECT == 1) then
		local remove_protect	

		print("检查读写保护...")			
		remove_protect = 0;		
		for i = 1, #OB_WRP_ADDRESS, 1 do
			local wrp
			
			re,ob_data = pg_read_mem(OB_WRP_ADDRESS[i], 1)
			if (re == 0) then
				pg_print_text("  已保护，设置读保护")	
				remove_protect = 1	
				break
			else
				wrp = tonumber(string.byte(ob_data,1,1))					
				str = string.format("  0x%08X ： 0x%02X & 0x%02X == 0x%02X", OB_WRP_ADDRESS[i], wrp, OB_WRP_MASK[i], OB_WRP_VALUE[i])
				print(str)
				if ((wrp & OB_WRP_MASK[i]) ~= OB_WRP_VALUE[i]) then
					pg_print_text("  已保护，设置读保护")	
					err = set_read_protect(1)		--设置读保护
					--if (err ~= "OK") then goto quit end  --这个地方不要退出（STM32F051)
					remove_protect = 1	
					break	
				end			
			end			
		end	
		
		if (remove_protect == 1) then
			pg_print_text("正在解除保护...")			
			err = set_read_protect(0)		--解除读保护，内部有复位操作
			if (err ~= "OK") then goto quit end	
		else
			print("  无保护")			
		end
	end
	
	for i = 1, #TaskList, 3 do		
		if (TaskList[i] ~= "") then
			print("------------------------")
			str = string.format("FLM : %s", TaskList[i])  print(str)
			str = string.format("Data: %s", TaskList[i + 1]) print(str)
			str = string.format("Addr: 0x%08X", TaskList[i + 2]) print(str)
			
			pg_print_text("编程文件")	
			--加载flash算法文件
			re = pg_load_algo_file( TaskList[i], AlgoRamAddr, AlgoRamSize)
			if (re == 0) then
				err = "加载flash算法失败"  goto quit
			end
			
			fix_data_begin()				--开始动态填充SN UID USR数据
			re = pg_prog_file(TaskList[i + 1], TaskList[i + 2])
			fix_data_end()					--结束动态填充SN UID USR数据
			if (re == 0) then
				err = "error" goto quit 	--pg_prog_file内部已显示出错信息
			end
		end
	end
		
	--写OPTION BYTES (读保护也在内）
	if (OB_ENABLE == 1) then
		print("------------------------")
		str = string.format("FLM : %s", AlgoFile_OPT)  print(str)		
		pg_print_text("编程OPTION BYTES")		
		re = pg_load_algo_file(AlgoFile_OPT, AlgoRamAddr, AlgoRamSize)
		if (re == 0) then
			err = "加载OPT算法失败"  goto quit
		end	
	
		if (SECURE_ENABLE == 0) then
			str = string.format("OB_SECURE_OFF : %s", OB_SECURE_OFF)  print(str)	
			re = pg_prog_buf_ob(OB_ADDRESS, OB_SECURE_OFF)
		else
			str = string.format("OB_SECURE_ON  : %s", OB_SECURE_ON)  print(str)	
			re = pg_prog_buf_ob(OB_ADDRESS, OB_SECURE_ON)
		end
		if (re == 0) then
			err = "写OPTION BYTES失败"  goto quit
			goto quit
		else
			
		end
	end

	--复位
	if (RESET_AFTER_COMPLETE == 1) then
		pg_reset()
	end

::quit::
	return err
end

--开始编程,步骤：
--供电-擦除全片-动态生成SN、UID加密数据、用户数据
--编程文件(自动擦除、编程、校验)
--写OPTION BYTES
function swim_start_prog(void)
	local err = "OK"
	local re
	local uid_bin
	local last_sn
	local str
	local mcu_uid
	local i	
	local bin
	local ff
	
	--设置TVCC电压
	set_tvcc(TVCC_VOLT)
	delayms(20)
	
	pg_init()

	--先设置读保护，再解除读保护。自动擦除全片。
	pg_print_text("擦除全片")
	re = pg_prog_buf_ob(OB_ADDRESS, OB_RDP_ON)
	if (re == 0) then
		err = "加锁失败"  goto quit
		goto quit	
	end	
	pg_init()	--加锁后复位后生效	
	re = pg_prog_buf_ob(OB_ADDRESS, OB_RDP_OFF)
	if (re == 0) then
		err = "解锁失败"  goto quit
		goto quit	
	end		

	--编程文件（查空、擦除、编程、校验）
	for i = 1, #TaskList, 2 do		
		if (TaskList[i] ~= "") then	
			print("------------------------")
			str = string.format("File : %s", TaskList[i])  print(str)		
			fix_data_begin()			--开始动态填充SN UID USR数据				
			re = pg_prog_file(TaskList[i], TaskList[i + 1])
			fix_data_end()				--结束动态填充SN UID USR数据
			if (re == 0) then
				err = "编程失败"  goto quit
				goto quit
			end	
		end
	end
	
	--写OPTION BYTES (读保护也在内）

	if (OB_ENABLE == 1) then
		print("------------------------")
		pg_print_text("写option bytes")
		if (SECURE_ENABLE == 0) then
			str = string.format("OB_SECURE_OFF : %s", OB_SECURE_OFF)  print(str)	
			re = pg_prog_buf_ob(OB_ADDRESS, OB_SECURE_OFF)
		else
			str = string.format("OB_SECURE_ON  : %s", OB_SECURE_ON)  print(str)	
			re = pg_prog_buf_ob(OB_ADDRESS, OB_SECURE_ON)
		end
		if (re == 0) then
			err = "写OPTION BYTES失败"  goto quit
			goto quit
		else
			
		end		
	end

	--复位
	if (RESET_AFTER_COMPLETE == 1) then
		pg_reset()
	end

::quit::
	return err
end

--二进制字符串转换可见的hex字符串
function bin2hex(s)
	s = string.gsub(s,"(.)", function (x) return string.format("%02X ",string.byte(x)) end)
	return s
end

--打印内存数据
function print_flash(addr, len, width, dispaddr)
	local re
	local bin
	local str
	local core_id

	--设置TVCC电压
	set_tvcc(TVCC_VOLT)
	delayms(20)

	pg_init()
		
	re,bin = pg_read_mem(addr, len)
	if (re == 1) then
		str = string.format("address = 0x%08X, len = %d", addr, len)
		print(str)	
		if (width == nil) then
			print_hex(bin,16)
		else		
			if (dispaddr == nil) then
				print_hex(bin,width)	
			else
				print_hex(bin,width, dispaddr)	
			end		
		end
	else
		str = "error"
		print(str)
	end
end

--打印OPTION BYTES
function print_option_bytes(void)
	local re
	local bin
	
	print("Option bytes Address:")
	print(OB_ADDRESS)

	print("Option bytes data:")
	re,obin = pg_read_ob(OB_ADDRESS)
	if (re == 1) then
		print_hex(obin)
	else
		print("error")
	end
end

--设置读保护 0 表示解除读保护，1表示启用读保护
function set_read_protect(on)
	local re
	local err = "OK"
	local time1
	local time2
	local str
	
	if (on == 1) then
		print("启用读保护...")
	else
		print("关闭读保护...")
	end
	
	pg_reset()
	
	time1 = get_runtime()
	
	--设置TVCC电压
	set_tvcc(TVCC_VOLT)
	delayms(20)
	
	--检测IC,打印内核ID
	local core_id = pg_detect_ic()
	if (core_id == 0) then
		err = "未检测到IC"  print(err) return err
	else
		str = string.format("core_id = 0x%08X", core_id)
		print(str)
	end
	
	if (CHIP_TYPE == "SWD") then
		if (AlgoFile_OPT == "") then
			err = "没有OPT算法文件"  print(err) return err	
		end
	
		--加载flash算法文件
		re = pg_load_algo_file(AlgoFile_OPT, AlgoRamAddr, AlgoRamSize)
		if (re == 0) then
			err = "加载flash算法失败"  print(err) return err
		end
	else 
		if (CHIP_TYPE == "SWIM") then
			
		else
			print("不支持该功能")
			return "err"
		end	
	end
	
	if (on == 0) then
		re = pg_prog_buf_ob(OB_ADDRESS, OB_RDP_OFF)
		if (re == 0) then		--部分芯片需要2次操作来确认是否成功 （STM32L051)
			print("断电200ms")  
			set_tvcc(0)			--断电
			delayms(200)		--延迟200ms
			set_tvcc(TVCC_VOLT) --上电
			print("重新上电")	
			pg_reset()		
			re = pg_prog_buf_ob(OB_ADDRESS, OB_RDP_OFF)
		end	
	else 
		if (on == 1) then
			re = pg_prog_buf_ob(OB_ADDRESS, OB_RDP_ON)
		end
	end
	if (re == 0) then
		err = "写OPTION BYTES失败"
	end
	time2 = get_runtime()

	if (err == "OK") then
		print("写入成功")
		str = string.format("执行时间 = %d ms", time2 - time1); 
		print(str)		
	else
		
	end	

	return err
end

--解锁芯片=擦除芯片
function erase_chip(FlashAddr)
	local re
	local err = "OK"
	local time1
	local time2
	local str
	
	print("开始擦除flash..")

	--检测IC,打印内核ID
	local core_id = pg_detect_ic()
	if (core_id == 0) then
		err = "未检测到IC"  print(str) return err
	else
		str = string.format("core_id = 0x%08X", core_id)
		print(str)
	end
	
	if (CHIP_TYPE == "SWD") then
		--加载flash算法文件
		re = pg_load_algo_file(AlgoFile_FLASH, AlgoRamAddr, AlgoRamSize)
		if (re == 0) then
			err = "加载flash算法失败"  print(str) return err
		end
	else 
		if (CHIP_TYPE == "SWIM") then
			if (FlashAddr == 0x08000) then
				str = string.format("开始擦除flash. 地址 : 0x%X 长度 : %dKB ", FlashAddr, FLASH_SIZE / 1024)
			else
				str = string.format("开始擦除flash. 地址 : 0x%X 长度 : %dKB ", FlashAddr, EEPROM_SIZE / 1024)
			end
			print(str)	
		else
			print("未知接口")	
		end	
	end
		
	time1 = get_runtime()
	
	--设置TVCC电压
	set_tvcc(TVCC_VOLT)
	delayms(20)
	
	pg_init()

	re = pg_erase_chip(FlashAddr)
	if (re == 1) then
		pg_print_text("擦除成功")		
	else
		pg_print_text("擦除失败")		
		err = "err"
	end
	time2 = get_runtime()

	str = string.format("执行时间 = %d ms", time2 - time1); 
	print(str)
	return err
end

--显示内核id,
function print_core_id(void)
	local core_id
	local str

	set_tvcc(TVCC_VOLT)

	--检测IC,打印内核ID
	core_id = pg_detect_ic()
	if (core_id == 0) then
		print("未检测到IC")
	else
		str = string.format("core_id = 0x%08X", core_id) print(str)
	end
end

--擦除CPU片外QSPI FLASH
function erase_chip_qspi(void)
	local core_id
	local str
	local addr
	local i
	local nSector
	local percent
	local time1
	local time2
	
	print("开始擦除QSPI Flash...")

	time1 = get_runtime()

	cofig_chip1()		--配置烧录参数			
	set_tvcc(0)			--断电
	delayms(20)
	set_tvcc(TVCC_VOLT)	--设置TVCC电压	
	
	core_id = pg_detect_ic()
	if (core_id > 0) then
		str = string.format("swd : core_id = 0x%08X", core_id)
		print(str)
	end
		
	--加载flash算法文件
	re = pg_load_algo_file(AlgoFile_QSPI, AlgoRamAddr, AlgoRamSize)
	
	addr = 0x90000000
	nSector = 32 * 1024 / 64
	for i = 1, nSector,1 do
		pg_erase_sector(addr)
		
		percent = 100 * i / nSector;
		str = string.format("erase 0x%08X, %0.2f%%", addr, percent)
		print(str)
		addr = addr + 64 * 1024
	end
	
	time2 = check_runtime(time1)
	str = string.format("擦除结束  %0.3f 秒", time2 / 1000)
	print(str)
end	

--擦除CPU片内 FLASH
function erase_chip_mcu(void)
	local core_id
	local str
	local addr
	local i
	local nSector
	local percent
	local time1
	local time2
	
	time1 = get_runtime()

	cofig_chip1()		--配置烧录参数			
	set_tvcc(0)			--断电
	delayms(20)
	set_tvcc(TVCC_VOLT)	--设置TVCC电压	
	
	erase_chip(FLASH_ADDRESS)	

	time2 = check_runtime(time1)
	str = string.format("擦除结束  %0.3f 秒", time2 / 1000)
	print(str)
end	

---------------------------结束-----------------------------------

