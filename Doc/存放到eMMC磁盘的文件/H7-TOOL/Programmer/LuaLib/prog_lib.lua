-------------------------------------------------------
-- 文件名 : prog_lib.lua
-- 版  本 : V1.1  2020-06-03
-- 说  明 :脱机编程共用函数库
-------------------------------------------------------

--编程入口
function start_prog(void)
	return prog_or_erase(0)
end

--擦除入口
function erase_chip_mcu(void)
	return prog_or_erase(1)
end

--擦除入口
function erase_chip_eeprom(void)
	return prog_or_erase(2)
end

--编程或者擦除公共函数
function prog_or_erase(mode)
	local err = ""
	local str

	if (MULTI_MODE == 0) then
		print("单路烧录")
	end
	if (MULTI_MODE == 1) then
		print("多路烧录 1路")
	end
	if (MULTI_MODE == 2) then
		print("多路烧录 1-2路")
	end

	if (MULTI_MODE == 3) then
		print("多路烧录 1-3路")
	end
	if (MULTI_MODE == 4) then
		print("多路烧录 1-4路")
	end

	config_chip1()		--配置烧录参数

	if (CHIP_TYPE == "SWD") then
		print("SWCLK时钟延迟: ", SWD_CLOCK_DELAY)
	else if (CHIP_TYPE == "SWIM") then
		print("CHIP_NAME = "..CHIP_NAME)
		print(" flash  size = ", FLASH_SIZE)
		print(" eeprom size = ", EEPROM_SIZE)
		if (FLASH_SIZE == nil or EEPROM_SIZE == nil) then
			err = "chip name is invalid"
			goto quit
		end
	end
	end

--	set_tvcc(0)			--断电
--	delayms(20)
	set_tvcc(TVCC_VOLT)	--设置TVCC电压
--	delayms(20)

	if (MULTI_MODE > 0) then
		local id1
		local id2
		local id3
		local id4
		local i

		id1,id2,id3,id4 = pg_detect_ic()
		str = string.format("core_id: = 0x%08X 0x%08X 0x%08X 0x%08X", id1, id2, id3, id4)
		print(str)
		if ((MULTI_MODE == 1 and id1 > 0) or
			(MULTI_MODE == 2 and id1 > 0 and id2 > 0) or
			(MULTI_MODE == 3 and id1 > 0 and id2 > 0 and id3 > 0) or
			(MULTI_MODE == 4 and id1 > 0 and id2 > 0 and id3 > 0 and id4 > 0)) then
			if (mode == 0) then --编程
				if (CHIP_TYPE == "SWD") then
					err = swd_start_prog()	--编程ARM (SWD)
				else
					err = swim_start_prog()	--编程STM8 (SWD)
				end
			else	--擦除
				if (mode == 1) then
					err = erase_chip(FLASH_ADDRESS)	--擦除CPU Flash
				else
					if (EEPROM_ADDRESS ~= nil) then
						err = erase_chip(EEPROM_ADDRESS)	--擦除CPU EEPROM
					else
						print("MCU未配置EEPROM")
					end
				end
			end
			if (err ~= "OK") then goto quit end
			goto quit
		end

		err = "未检测到IC"

		if (MULTI_MODE >= 1) then
			if (id1 == 0) then
				err = err.." #1"
			end
		end

		if (MULTI_MODE >= 2)then
			if (id2 == 0) then
				err = err.." #2"
			end
		end

		if (MULTI_MODE >= 3) then
			if (id3 == 0) then
				err = err.." #3"
			end
		end

		if (MULTI_MODE >= 4) then
			if (id3 == 0) then
				err = err.." #4"
			end
		end
	else
		local core_id

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

			if (mode == 0) then --编程
				if (CHIP_TYPE == "SWD") then
					err = swd_start_prog()	--编程ARM (SWD)
				else
					err = swim_start_prog()	--编程STM8 (SWD)
				end
			else	--擦除
				if (mode == 1) then
					err = erase_chip(FLASH_ADDRESS)	--擦除CPU Flash
				else
					if (EEPROM_ADDRESS ~= nil) then
						err = erase_chip(EEPROM_ADDRESS)	--擦除CPU EEPROM
					else
						print("MCU未配置EEPROM")
					end
				end
			end
			if (err ~= "OK") then goto quit end
			goto quit
		end

		err = "未检测到IC"
	end

::quit::
	if (err == "OK") then
		beep() --成功叫1次

		if (MULTI_MODE == 0) then
			pg_print_text("编程成功")
		end

		if (MULTI_MODE == 1) then
			pg_print_text("编程成功 1路")
		end
		if (MULTI_MODE == 2) then
			pg_print_text("编程成功 1-2路")
		end
		if (MULTI_MODE == 3) then
			pg_print_text("编程成功 1-3路")
		end
		if (MULTI_MODE == 4) then
			pg_print_text("编程成功 1-4路")
		end
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
	if (MULTI_MODE > 0) then
		local id1
		local id2
		local id3
		local id4

		id1,id2,id3,id4 = pg_detect_ic()
		if (MULTI_MODE == 1) then
			if (id1 == 0) then
				return "removed"
			end
		end
		if (MULTI_MODE == 2) then
			if (id1 == 0 and id2 == 0) then
				return "removed"
			end
		end
		if (MULTI_MODE == 3) then
			if (id1 == 0 and id2 == 0 and id3 == 0) then
				return "removed"
			end
		end
		if (MULTI_MODE == 4) then
			if (id1 == 0 and id2 == 0 and id3 == 0 and id4 == 0) then
				return "removed"
			end
		end
	else
		local core_id

		core_id = pg_detect_ic()
		if (core_id == 0) then
			return "removed"
		end
	end

	return "no"
end

--判断芯片插入（用于连续烧录）
function CheckChipInsert(void)
	config_chip1()

	if (MULTI_MODE > 0) then
		local id
		local id1
		local id2
		local id3
		local id4

		id1,id2,id3,id4 = pg_detect_ic()
		if (MULTI_MODE == 1) then
			if (id1 > 0) then
				return "inserted"
			end
		end
		if (MULTI_MODE == 2) then
			if (id1 > 0 and id2 > 0) then
				return "inserted"
			end
		end
		if (MULTI_MODE == 3) then
			if (id1 > 0 and id2 > 0 and id3 > 0) then
				return "inserted"
			end
		end
		if (MULTI_MODE == 4) then
			if (id1 > 0 and id2 > 0 and id3 > 0 and id4 > 0) then
				return "inserted"
			end
		end
	else
		local core_id

		core_id = pg_detect_ic()
		if (core_id > 0) then
			return "inserted"
		end
	end

	return "no"
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

			if (MCU_READ_OPTION == 1) then
				re,ob_data = MCU_ReadOptionsByte(OB_WRP_ADDRESS[i], 1)
			else
				re,ob_data = pg_read_mem(OB_WRP_ADDRESS[i], 1)
			end
			if (re == 0) then
				print("  读寄存器失败")
				pg_print_text("  已保护，设置读保护")
				remove_protect = 1
				break
			else
				if (MCU_READ_OPTION == 1) then
					wrp = ob_data
				else
					wrp = tonumber(string.byte(ob_data,1,1))
				end
				str = string.format("  0x%08X ： 0x%02X & 0x%02X == 0x%02X", OB_WRP_ADDRESS[i], wrp, OB_WRP_MASK[i], OB_WRP_VALUE[i])
				if ((wrp & OB_WRP_MASK[i]) ~= OB_WRP_VALUE[i]) then
					str = str.."(已保护)"

					--pg_print_text("  已保护，设置读保护")
					--err = set_read_protect(1)		--设置读保护(如果只是写保护呢?)
					--if (err ~= "OK") then goto quit end  --这个地方不要退出（STM32F051)
					remove_protect = 1
					--break
				end
				print(str)
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

	fix_data_begin()				--开始动态填充SN UID USR数据

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

			re = pg_prog_file(TaskList[i + 1], TaskList[i + 2])

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

	if (ERASE_CHIP_ENABLE == 1) then
		--先设置读保护，再解除读保护。自动擦除全片。
		pg_print_text("擦除全片")
		--先加保护再解除保护，达到清空芯片的目的
		set_read_protect(1)
		pg_init()
		set_read_protect(0)
	end

	--动态填充SN UID USR数据
	fix_data_begin()

	--编程文件（查空、擦除、编程、校验）
	for i = 1, #TaskList, 2 do
		if (TaskList[i] ~= "") then
			print("------------------------")
			str = string.format("File : %s", TaskList[i])  print(str)
			re = pg_prog_file(TaskList[i], TaskList[i + 1])
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
	local bin1
	local bin2
	local bin3
	local bin4
	local str
	local core_id

	--设置TVCC电压
	set_tvcc(TVCC_VOLT)
	delayms(20)

	pg_init()

	if (MULTI_MODE > 0) then
		re,bin1,bin2,bin3,bin4 = pg_read_mem(addr, len)
		if (re == 1) then
			if (MULTI_MODE >= 1) then
				str = string.format("#1 address = 0x%08X, len = %d", addr, len)
				print(str)
				if (width == nil) then
					print_hex(bin1,16)
				else
					if (dispaddr == nil) then
						print_hex(bin1,width)
					else
						print_hex(bin1,width, dispaddr)
					end
				end
				delayms(5)
			end

			if (MULTI_MODE >= 2) then
				str = string.format("#2 address = 0x%08X, len = %d", addr, len)
				print(str)
				if (width == nil) then
					print_hex(bin2,16)
				else
					if (dispaddr == nil) then
						print_hex(bin2,width)
					else
						print_hex(bin2,width, dispaddr)
					end
				end
				delayms(5)
			end

			if (MULTI_MODE >= 3) then
				str = string.format("#3 address = 0x%08X, len = %d", addr, len)
				print(str)
				if (width == nil) then
					print_hex(bin3,16)
				else
					if (dispaddr == nil) then
						print_hex(bin3,width)
					else
						print_hex(bin3,width, dispaddr)
					end
				end
				delayms(5)
			end

			if (MULTI_MODE == 4) then
				str = string.format("#4 address = 0x%08X, len = %d", addr, len)
				print(str)
				if (width == nil) then
					print_hex(bin4,16)
				else
					if (dispaddr == nil) then
						print_hex(bin4,width)
					else
						print_hex(bin4,width, dispaddr)
					end
				end
				delayms(5)
			end
		else
			str = "error"
			print(str)
		end
	else
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
end

--打印OPTION BYTES
function print_option_bytes(void)
	if (MULTI_MODE > 0) then
		local re
		local bin1
		local bin2
		local bin3
		local bin4

		print("Option bytes Address:")
		print(OB_ADDRESS)

		re,bin1,bin2,bin3,bin4 = pg_read_ob(OB_ADDRESS)
		if (re == 1) then
			if (MULTI_MODE >= 1) then
				print("#1 Option bytes data:")
				print_hex(bin1)
			end

			if (MULTI_MODE >= 2) then
				print("#2 Option bytes data:")
				print_hex(bin2)
			end

			if (MULTI_MODE >= 3) then
				print("#3 Option bytes data:")
				print_hex(bin3)
			end

			if (MULTI_MODE >= 4) then
				print("#4 Option bytes data:")
				print_hex(bin4)
			end
		else
			print("error")
		end
	else
		local re
		local bin

		print("Option bytes Address:")
		print(OB_ADDRESS)

		print("Option bytes data:")
		re,bin = pg_read_ob(OB_ADDRESS)
		if (re == 1) then
			print_hex(bin)
		else
			print("error")
		end
	end
end

--设置读保护 0 表示解除读保护，1表示启用读保护
function set_read_protect(on)
	local re
	local err = "OK"
	local time1
	local time2
	local str

	if (REMOVE_RDP_POWEROFF == nil) then
		REMOVE_RDP_POWEROFF = 0
	end

	if (on == 1) then
		print("启用读保护...")
	else
		print("关闭读保护...")
	end

	time1 = get_runtime()

--	--设置TVCC电压
--	set_tvcc(TVCC_VOLT)
--	delayms(20)
--
--	--检测IC,打印内核ID
--	local core_id = pg_detect_ic()
--	if (core_id == 0) then
--		err = "未检测到IC"  print(err) return err
--	else
--		str = string.format("core_id = 0x%08X", core_id)
--		print(str)
--	end

	if (CHIP_TYPE == "SWD") then

		--新唐专用的解除保护
		if (MCU_REMOVE_PROTECT == 1) then
			if (on == 0) then
				print("MCU_RemoveProtect()")
				MCU_RemoveProtect()
			end
		end

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
			--STM8专用的解除保护
			if (MCU_REMOVE_PROTECT == 1) then
				if (on == 0) then
					print("MCU_RemoveProtect()")
					MCU_RemoveProtect()
				end
			end
		else
			print("不支持该功能")
			return "err"
		end
	end

	if (on == 0) then
		local i

		print("OB_SECURE_OFF = ", OB_SECURE_OFF)
		for i = 1, 3, 1 do
			re = pg_prog_buf_ob(OB_ADDRESS, OB_SECURE_OFF)
			if (re == 0) then		--部分芯片需要2次操作来确认是否成功 （STM32L051)
				if (REMOVE_RDP_POWEROFF > 0) then
					print("  prog ob failed", i)
					delayms(POWEROFF_TIME1)
					print("  断电")
					set_tvcc(0)			--断电
					delayms(POWEROFF_TIME2)
					set_tvcc(TVCC_VOLT) --上电
					pg_reset()
					delayms(POWEROFF_TIME3)
					core_id = pg_detect_ic()	--MM32复位后必须读一次ID才能访问内存
				else
					print("  prog ob failed", i)
					delayms(100)
					pg_reset()
					delayms(100)
					core_id = pg_detect_ic()	--MM32复位后必须读一次ID才能访问内存
				end
			else	--成功
				if (REMOVE_RDP_POWEROFF > 0) then
					delayms(POWEROFF_TIME1)
					print("  断电")
					set_tvcc(0)			--断电
					delayms(POWEROFF_TIME2)
					set_tvcc(TVCC_VOLT) --上电
					pg_reset()
					delayms(POWEROFF_TIME3)
					core_id = pg_detect_ic()	--MM32复位后必须读一次ID才能访问内存
				else
					pg_reset()
					delayms(100)
				end
				core_id = pg_detect_ic()	--MM32复位后必须读一次ID才能访问内存
				break
			end
		end
	else
		if (on == 1) then
			print("OB_SECURE_OFF = ", OB_SECURE_ON)
			re = pg_prog_buf_ob(OB_ADDRESS, OB_SECURE_ON)
		end
	end

	if (re == 0) then
		err = "写OPTION BYTES失败"
	end
	time2 = get_runtime()

	if (err == "OK") then
		print("写Option Bytes成功")
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

	pg_print_text("擦除...")

	if (CHIP_TYPE == "SWD") then
		if (FlashAddr == FLASH_ADDRESS) then
			re = pg_load_algo_file(AlgoFile_FLASH, AlgoRamAddr, AlgoRamSize)	--加载flash算法文件
		else
			re = pg_load_algo_file(AlgoFile_EEPROM, AlgoRamAddr, AlgoRamSize)	--加载eeprom算法文件
		end
		if (re == 0) then
			err = "加载flash算法失败"  print(str) return err
		end
	else
		if (CHIP_TYPE == "SWIM") then
			if (FlashAddr == FLASH_ADDRESS) then
				str = string.format("开始擦除flash. 地址 : 0x%X 长度 : %dKB ", FlashAddr, FLASH_SIZE / 1024)
			else
				str = string.format("开始擦除eeprom. 地址 : 0x%X 长度 : %dB ", FlashAddr, EEPROM_SIZE)
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
	local id
	local id1
	local id2
	local id3
	local id4
	local str

	set_tvcc(TVCC_VOLT)

	if (MULTI_MODE > 0) then
		id1,id2,id3,id4 = pg_detect_ic()
		str = string.format("core_id1 = 0x%08X", id1) print(str) delayms(5)
		str = string.format("core_id2 = 0x%08X", id2) print(str) delayms(5)
		str = string.format("core_id3 = 0x%08X", id3) print(str) delayms(5)
		str = string.format("core_id4 = 0x%08X", id4) print(str) delayms(5)
	else
		core_id = pg_detect_ic()
		if (core_id == 0) then
			print("未检测到IC")
		else
			str = string.format("core_id = 0x%08X", core_id) print(str)
		end
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

	if (AlgoFile_QSPI == nil or AlgoFile_QSPI == "") then
		print("未配置QSPI Flash")
		return
	end

	print("开始擦除QSPI Flash...")

	time1 = get_runtime()

	config_chip1()		--配置烧录参数
	--set_tvcc(0)			--断电
	--delayms(20)
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

---------------------------结束-----------------------------------
