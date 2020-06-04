--LuaSubFile=1  --表示该文件作为子文件加载（不要修改这一行）
-------------------------------------------------------
-- 文件名 : fix_data.lua
-- 版  本 : V1.0  2020-04-23
-- 说  明 : 用于烧录产品序号、程序UID加密字符、自定义字符串
-------------------------------------------------------

--开始修正数据文件，动态填充SN UID USR数据
function fix_data_begin(void)

	config_fix_data()	--在用户lua程序部分实现这个函数

	--产品编码（滚码）设置（固定4字节）
	--SN_ENABLE = 0				--1表示启用   0表示不启用
	--SN_SAVE_ADDR = 0			--产品序号保存地址

	SN_INIT_VALUE = 1			--产品序号初始值
	SN_LITTLE_ENDIN = 1			--0表示大端，1表示小端
	SN_DATA = ""				--序号内容，由后面的 sn_new() 函数生成
	SN_LEN = 0					--序号长度

	SN_DATA1 = ""
	SN_DATA2 = ""
	SN_DATA3 = ""
	SN_DATA4 = ""

	--UID加密存储设置（只针对有UID的MCU）
	--UID_ENABLE = 0	       		--1表示启用加密  0表示不启用
	--UID_SAVE_ADDR = 0 			--加密结果FLASH存储地址
	--UID_BYTES = 12             	--UID长度
	UID_DATA = ""				--加密数据内容，由后面的 uid_encrypt() 函数生成
	UID_LEN = 0					--数据长度

	UID_DATA1 = ""
	UID_DATA2 = ""
	UID_DATA3 = ""
	UID_DATA4 = ""

	--用户自定义数据（可填充生产日期，客户编号等数据，要求数据连续）
	--USR_ENABLE = 0	       		--1表示启用   0表示不启用
	--USR_SAVE_ADDR = 0 			--自定义数据存储地址
	USR_DATA = ""				--自定义数据内容，由后面的 make_user_data() 函数生成
	USR_LEN = 0					--数据长度

	local str
	local re

	--读文件中上次SN并生成新的SN数据
	if (SN_ENABLE == 1) then
		if (MULTI_MODE > 0) then
			SN_DATA1 = sn_new()	--根据上次SN生成新的SN
			SN_DATA2 = sn_new()	--根据上次SN生成新的SN
			SN_DATA3 = sn_new()	--根据上次SN生成新的SN
			SN_DATA4 = sn_new()	--根据上次SN生成新的SN
			SN_LEN = string.len(SN_DATA1)

			str = "new sn1 = "..bin2hex(SN_DATA1) print(str)
			str = "new sn2 = "..bin2hex(SN_DATA2) print(str)
			str = "new sn3 = "..bin2hex(SN_DATA3) print(str)
			str = "new sn4 = "..bin2hex(SN_DATA4) print(str)
		else
			SN_DATA = sn_new()	--根据上次SN生成新的SN
			SN_LEN = string.len(SN_DATA)
			str = "new sn  = "..bin2hex(SN_DATA) print(str)
		end
	end

	--读UID（unique device identifier) 并生成UID加密数据
	if (MULTI_MODE > 0) then
		local uid1,uid2,uid3,uid4

		re,uid1,uid2,uid3,uid4 = pg_read_mem(UID_ADDR, UID_BYTES)
		if (re == 1) then
			str = "uid1 = "..bin2hex(uid1) print(str) delayms(5)
			str = "uid2 = "..bin2hex(uid2) print(str) delayms(5)
			str = "uid3 = "..bin2hex(uid3) print(str) delayms(5)
			str = "uid4 = "..bin2hex(uid4) print(str) delayms(5)

			if (UID_ENABLE == 1) then
				UID_DATA1 = uid_encrypt(uid1)
				UID_DATA2 = uid_encrypt(uid2)
				UID_DATA3 = uid_encrypt(uid3)
				UID_DATA4 = uid_encrypt(uid4)
				UID_LEN = string.len(UID_DATA1)

				str = "uid encrypt 1 = "..bin2hex(UID_DATA1)  print(str) delayms(5)
				str = "uid encrypt 2 = "..bin2hex(UID_DATA2)  print(str) delayms(5)
				str = "uid encrypt 3 = "..bin2hex(UID_DATA3)  print(str) delayms(5)
				str = "uid encrypt 4 = "..bin2hex(UID_DATA4)  print(str) delayms(5)
			end
		end
	else
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
	end

	--动态生成用户区数据
	if (USR_ENABLE == 1) then
		USR_DATA = make_user_data()
		USR_LEN = string.len(USR_DATA)
		str = "user data  = "..USR_DATA
		print(str)
	end

	--通知C程序更新变量
	pg_reload_var("UidSnUsr")
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
	local s

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
		s =    string.char(sn1 & 0xFF)
		s = s..string.char((sn1 >> 8) & 0xFF)
		s = s..string.char((sn1 >> 16) & 0xFF)
		s = s..string.char((sn1 >> 24) & 0xFF)
	else
		s =    string.char((sn1 >> 24) & 0xFF)
		s = s..string.char((sn1 >> 16) & 0xFF)
		s = s..string.char((sn1 >> 8) & 0xFF)
		s = s..string.char(sn1 & 0xFF)
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
	s =    string.char(out[1] & 0xFF)
	s = s..string.char(out[2] & 0xFF)
	s = s..string.char(out[3] & 0xFF)
	s = s..string.char(out[4] & 0xFF)
	return s
end

--动态生成用户区数据USR
function make_user_data(uid)
	s = os.date("%Y-%m-%d %H:%M:%S")	--返回ASCII字符串 2020-01-21 23:25:01
	s = s.." aaa"..string.char(0)
	return s
end

---------------------------结束-----------------------------------
