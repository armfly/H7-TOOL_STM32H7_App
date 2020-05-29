--F01=测试RC602,AutoTestRC608(8)
--F02=识别+测试602,AutoDetect()
--F03=RC402 校准+检测, AutoCalibTest()
--F04=RC402 校准模拟量,Calib402()
--F05=RC402 自动检测,AutoTestRC202()
--F06=RC402 配置为0-20mA,ConfigChanRange(0)
--F07=RC402 配置0-10V,ConfigChanRange(1)

--xF05=读6通道模拟量,ReadAllChan(0)
--xF07=设置通道1电压,SetCh1VoltMV(5000)
--xF08=设置通道2电压,SetCh2VoltMV(10000)
--xF09=设置通道1电流,SetCh1CurrMA(4.0)
--xF10=设置通道2电流,SetCh2CurrMA(19.0)
--xF11=读NTC温度,ReadTemp()

--xF13=测试0-10V线性度,TestChVolt(1)
--xF14=测试0-20mA线性度,TestChCurr(1)

print("无线IO模块程序已加载")

local Addr485 = 1
local COMx = 1
local TimeOut = 1000
local RC604Addr	= 250
local RC302Addr	= 251

local RC202_NORMAL	= 0xC202
local RD202_20MA    = 0x202A
local RD202_18B20   = 0x202B
local RD202_PULSE	= 0x202C

local REG03_AO_VOLT_01 = 0x0420		--RC302 V1输出电压 mV
local REG03_AO_CURR_01 = 0x0440		--RC302 输出电流 uA

local CALIB_VOTL1 = 4000		--500mV  校准电压1
local CALIB_VOTL2 = 9500	--95000mV 校准电压2

local CALIB_CURR1 = 4000	--4mA  校准电流1
local CALIB_CURR2 = 19000	--19mA 校准电流2

local REG06_CALIB_KEY = 0x2FF0		--校准开关。写入 0x55AA 允许写校准参数。其他值禁止写入
local REG06_CALIB_SET_RANGE = 0x2FF1		--临时设置模拟通道量程档位， 高字节通道号，低字节是档位

local CALIB_REG_STEP 	= 0x100   	--通道寄存器步距
local RANGE_REG_STEP 	= 12		--量程寄存器步距
local REG03_CC01_K0 	= 0x3000	--通道1 电压第1档  K值
	local CC01_K_0 		= 0		--K值
	local CC01_B_0 		= 2		--B值
	local CC01_T_5K_0 	= 4		--校准时常温温度值 - NTC上拉电阻
	local CC01_COEF_K_0 = 6		--温度补偿系数K
	local CC01_COEF_B_0 = 8		--温度补偿系数B
	local CC01_ZERO_ADC_0 = 10		--输入悬空ADC

	local CC01_K_1 		= 12		--K值
	local CC01_B_1	 	= 14		--B值
	local CC01_T_5K_1 	= 16		--校准时常温温度值
	local CC01_COEF_K_1	 = 18		--校准时满位ADC
	local CC01_COEF_B_1	 = 20		--温度系数 0=不修正
	local CC01_ZERO_ADC_1 = 22		--输入悬空ADC

local REG03_CC02_K0	= 0x3100		--通道2 电压第1档  K值

--
--模拟通道修正后的值 output ， int32 形式. 32个通道，共64个寄存器
local REG04_CH01_OUT_I = 0x0010		--通道1-32 输出值，有符号32位整数，高2字节在前，2个寄存器

--模拟通道修正后的值 output ， float 形式. 32个通道，共64个寄存器
local REG04_CH01_OUT_F = 0x0050		--通道1-32 输出值，浮点数，高2字节在前，2个寄存器

--通道1 - 8 ADC值 有效值
local REG04_CH01_ADC_RMS = 0x0090		--通道1-32 ADC值(NTC是均值，交流电流时RMS值）, 浮点数，高2字节在前，2个寄存器

--通道1 - 8 测得数据值的float形式
local REG04_CH01_VALUE	= 0x00D0		--通道1-32 测量值（电阻 电压 电流），浮点数，高2字节在前，2个寄存器

local REG04_CH01_ADC_AVG = 0x0110		--通道1-32 ADC均值

local REG04_CH01_SIGNAL_TYPE = 0x0150		--通道1-32 信号类型（连续分配，方便读取），高字节bit0表示启用 禁用，低字节表示类型


local REG03_CF01_EN = 0x2000		--通道使能 0表示不使能  1表示使能
	local CF01_EN			= 0x00		--通道使能 0表示不使能  1表示使能
	local CF01_SIGN_TYPE		= 0x01		--（保留）输入信号类型 统一编码 （电流，电压，热电阻）
	local CF01_FILT_MODE		= 0x02		--滤波模式 0 保留不用
	local CF01_FILT_TIME		= 0x03		--滤波时间 单位0.1s
	local CF01_CORR_K			= 0x04		--线性修正 y=kx+b 之k， 浮点数，4字节
	local CF01_CORR_B			= 0x06		--线性修正 y=kx+b 之b， 浮点数，4字节
	local CF01_LIMIT_LOWER	= 0x08		--量程下限，浮点数
	local CF01_LIMIT_UPPER	= 0x0A		--量程上限，浮点数
	local CF01_CUTOFF			= 0x0C		--小信号切除万分比  .*/
	local CF01_ALARM_EN		= 0x0D		--报警使能 1表示使能 0 禁止
	local CF01_ALARM_LOWER	= 0x0E		--报警下限 浮点数
	local CF01_ALARM_UPPER	= 0x10		--报警上限
	local CF01_ALARM_BACK		= 0x12		--报警回差 浮点数
	local CF01_ALARM_ACTION	= 0x14		--报警时设置 Y1-Y24 哪个继电器状态为1。允许多个通道映射到一个继电器输出（总报警标记）*/
	local CF01_TEMP_CORR		= 0x15		--热电偶冷端温度修正。 有符号16bit整数 0.1摄氏度


uart_cfg(COMx, 9600, 0, 8, 1)
beep()

--失败
function PrintError(void)
	print("-------测试失败----------")
	beep(5,5,3)
end

--成功
function PrintOk(void)
	print("-------测试成功----------")
	beep()
end

--自动校准+测试
function AutoCalibTest(void)
	if (Calib402() == 0) then
		print("")
		AutoTestRC202()
	end
end

--读模拟量输出
function ReadAllChan(n)
	local err, y1,y2,y3,y4,y5,y6,y7
	local str

	err,y1,y2,y3,y4,y5,y6,y7 = modbus_read_float(COMx,TimeOut,Addr485,REG04_CH01_OUT_F,7)

	if (err == 0) then
		str = string.format("%d %f, %f, %f, %f, %f, %f, %f", n, y1,y2,y3,y4,y5,y6,y7) print(str)
	else
		str = string.format("错误代码 = %d", err) print(str)
	end
end

--读NTC温度
function ReadTemp(void)
	local err, y1
	local str

	err, y1 = modbus_read_float(COMx,TimeOut,Addr485,REG04_CH01_OUT_F + 12,1)
	if (err == 0) then
		str = string.format("温度 = %f ℃", y1) print(str)
	else
		str = string.format("错误代码 = %d", err) print(str)
	end
end

--给CH1输入电压
function SetCh1VoltMV(volt)
	local err, y1
	local str

	--切换继电器,通道1电压
	err = modbus_write_do(COMx,TimeOut,RC604Addr,1,1,0,0,0)

	--设置DAC电压 - 校准点1
	err = err + modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_VOLT_01, volt)
	if (err == 0) then
		str = string.format("CH1 电压 = %d mV", volt) print(str)
	else
		str = string.format("错误代码 = %d", err) print(str)
	end
end

--给CH2输入电压
function SetCh2VoltMV(volt)
	local err, y1
	local str

	--切换继电器,通道2电压
	err = modbus_write_do(COMx,TimeOut,RC604Addr,1,0,1,0,0)

	--设置DAC电压 - 校准点1
	err = err + modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_VOLT_01, volt)
	if (err == 0) then
		str = string.format("CH2 电压 = %d mV", volt) print(str)
	else
		str = string.format("错误代码 = %d", err) print(str)
	end
end

--给CH1输入电流
function SetCh1CurrMA(curr)
	local err, y1
	local str

	--切换继电器,通道1电压
	err = modbus_write_do(COMx,TimeOut,RC604Addr,1,0,0,1,0)

	--设置DAC电压 - 校准点1
	err = err + modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_CURR_01, curr * 1000)
	if (err == 0) then
		str = string.format("CH1 电流 = %f mA", curr) print(str)
	else
		str = string.format("错误代码 = %d", err) print(str)
	end
end

--给CH2输入电流
function SetCh2CurrMA(curr)
	local err, y1
	local str

	--切换继电器,通道1电压
	err = modbus_write_do(COMx,TimeOut,RC604Addr,1,0,0,0,1)

	--设置DAC电压 - 校准点1
	err = err + modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_CURR_01, curr * 1000)
	if (err == 0) then
		str = string.format("CH2 电流 = %f mA", curr) print(str)
	else
		str = string.format("错误代码 = %d", err) print(str)
	end
end

--配置通道参数为电流模式
function ConfigChanRange(_rg)
	local err, y1
	local str

	--通道1
	err = modbus_write_u16(COMx,TimeOut,Addr485,REG03_CF01_EN + CF01_SIGN_TYPE, _rg)
	if (err > 0) then goto quit end

	--通道2
	err = err + modbus_write_u16(COMx,TimeOut,Addr485,REG03_CF01_EN + CF01_SIGN_TYPE + 0x20, _rg)
	if (err > 0) then goto quit end

::quit::
	if (err == 0) then
		if (_rg == 0) then
			str = string.format("配置2个通道为0-20mA成功") print(str)
		else
			str = string.format("配置2个通道为0-10V成功") print(str)
		end
	else
		str = string.format("错误代码 = %d", err) print(str)
	end
end

--检查温度范围， 返回1表示OK
function Check18B20(temp1, temp2)
	local diff

	if (temp1 < 5 or temp2 < 5 or temp1 > 40  or temp2 > 40) then
		return 0
	end

	if (temp1 > temp2) then
		diff = temp1 - temp2
	else
		diff = temp2 - temp1
	end

	if (diff > 2) then
		return 0
	else
		return 1
	end
end

--自动测试
function AutoTestRC202(void)
	local err,y1,y2,y3,y4
	local str
	local time1,time2
	local Model
	local RegAddr

	print("-------开始测试------------------------------------")
	time1 = get_runtime()

	--读硬件型号和版本
	err,y1,y2 = modbus_read_u16(COMx,TimeOut,Addr485,0x9000,2)
	if (err > 0) then goto quit end
	str = string.format("型号 = %04X 固件版本 = V%X.%02X", y1, y2 >> 8, y2 & 0xFF) print(str)

	Model = y1

	if (Model == RC202_NORMAL) then
		str = string.format("已识别机型: RC602")  print(str)
	else
		if (Model == RD202_20MA) then
			str = string.format("已识别机型: RD202-20MA")  print(str)
		else
			if (Model == RD202_18B20) then
				str = string.format("已识别机型: RD202-18B20")  print(str)
			else
				if (Model == RD202_PULSE) then
					str = string.format("已识别机型: RD202-PULSE")  print(str)
				else
					if (Model == 0xC402) then
						str = string.format("已识别机型: RC402")  print(str)
					else
						str = string.format("未知机型")  print(str)
						goto quit
					end
				end
			end
		end
	end

	--测试DS18B20功能
	if (Model == RC202_NORMAL or Model == RD202_18B20) then
		str = string.format("\r\n【1】测试DS18B20功能")  print(str)
		if (Model == RC202_NORMAL) then
			RegAddr = REG04_CH01_OUT_F + 8
		else
			RegAddr = REG04_CH01_OUT_Fl
		end
		err,y1,y2 = modbus_read_float(COMx,TimeOut,Addr485,RegAddr,2)
		if (err > 0) then
			goto quit
		end

		if (Check18B20(y1,y2) == 1) then
			str = string.format(" %0.1f℃, %0.1f℃  -- OK ", y1, y2)  print(str)
		else
			str = string.format(" %0.1f℃, %0.1f℃  -- Err", y1, y2)  print(str)
			err = 9
			goto quit
		end
	end

	--测试脉冲功能
	if (Model == RC202_NORMAL or Model == RD202_PULSE) then
		str = string.format("\r\n【2】测试脉冲功能")  print(str)
		if (Model == RC202_NORMAL) then
			RegAddr = REG04_CH01_OUT_F + 4
		else
			RegAddr = REG04_CH01_OUT_F
		end
		err,y1,y2 = modbus_read_float(COMx,TimeOut,Addr485,RegAddr,2)
		if (err > 0) then
			goto quit
		end

		if (y1 > 90 and y2 > 90 and y1 < 110 and y2 < 110 ) then
			str = string.format(" %0.2fHz, %0.2fHz  -- OK ", y1, y2)  print(str)
		else
			str = string.format(" %0.2fHz, %0.2fHz   -- Err", y1, y2)  print(str)
			err = 9
			goto quit
		end
	end

	--测试模拟量检测功能
	if (Model == RC202_NORMAL or Model == RD202_20MA or Model == 0xC402) then
		str = string.format("\r\n【4】测试模拟量功能")  print(str)

		ConfigChanRange(1)	--切换到电压档位

		--设置DAC电压
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_VOLT_01, 5000)
		if (err > 0) then goto quit end
		--切换继电器
		err = modbus_write_do(COMx,TimeOut,RC604Addr,1, 1, 0,0,0)
		if (err > 0) then goto quit end
		delayms(1000)
		err,y1 = modbus_read_float(COMx,TimeOut,Addr485, REG04_CH01_OUT_F, 1)
		if (err > 0) then goto quit end
		if (math.abs(y1 - 5.0) < 0.02) then
			str = string.format(" CH1 5.000V, %0.3fV  -- OK ", y1)   print(str)
		else
			str = string.format(" CH1 5.000V, %0.3fV  -- Err ", y1)  print(str)
			err = 9
			goto quit
		end

		--设置DAC电压
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_VOLT_01, 6000)
		if (err > 0) then goto quit end
		--切换继电器
		err = modbus_write_do(COMx,TimeOut,RC604Addr,1, 0, 1,0,0)
		if (err > 0) then goto quit end
		delayms(1000)
		err,y1 = modbus_read_float(COMx,TimeOut,Addr485, REG04_CH01_OUT_F + 2, 1)
		if (err > 0) then goto quit end
		if (math.abs(y1 - 6.0) < 0.02) then
			str = string.format(" CH2 6.000V, %0.3fV  -- OK ", y1)   print(str)
		else
			str = string.format(" CH2 6.000V, %0.3fV  -- Err ", y1)  print(str)
			err = 9
			goto quit
		end

		print("")
		ConfigChanRange(0) --切换到电流档位

		--设置DAC电流
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_CURR_01, 12000)
		if (err > 0) then goto quit end
		--切换继电器
		err = modbus_write_do(COMx,TimeOut,RC604Addr,1, 0, 0,1,0)
		if (err > 0) then goto quit end
		delayms(1000)
		err,y1 = modbus_read_float(COMx,TimeOut,Addr485, REG04_CH01_OUT_F, 1)
		if (err > 0) then goto quit end
		if (math.abs(y1 - 12.0) < 0.02) then
			str = string.format(" CH1 12.000mA, %0.3fmA  -- OK ", y1)   print(str)
		else
			str = string.format(" CH1 12.000mA, %0.3fmA  -- Err ", y1)  print(str)
			err = 9
			goto quit
		end

		--设置DAC电流
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_CURR_01, 10000)
		if (err > 0) then goto quit end
		--切换继电器
		err = modbus_write_do(COMx,TimeOut,RC604Addr,1, 0, 0,0,1)
		if (err > 0) then goto quit end
		delayms(1000)
		err,y1 = modbus_read_float(COMx,TimeOut,Addr485, REG04_CH01_OUT_F + 2, 1)
		if (err > 0) then goto quit end
		if (math.abs(y1 - 10.0) < 0.02) then
			str = string.format(" CH2 10.000mA, %0.3fmA  -- OK ", y1)   print(str)
		else
			str = string.format(" CH2 10.000mA, %0.3fmA  -- Err ", y1)  print(str)
			err = 9
			goto quit
		end
	end

	--测试继电器功能
	if (Model == RC202_NORMAL or Model == RD202_PULSE) then
		str = string.format("\r\n【3】测试继电器功能")  print(str)

		err = modbus_write_do(COMx,TimeOut,Addr485,1,0,0)
		if (err > 0) then goto quit end
		delayms(100)
		err,y1,y2 = modbus_read_di(COMx,TimeOut,RC604Addr,1,2)
		if (y1 == 0 and y2 == 0) then
			str = string.format(" y1 = 0(%d)， y2 = 0(%d)  -- Ok ", y1, y2)  print(str)
		else
			str = string.format(" y1 = 0(%d)， y2 = 0(%d)  -- Err ", y1, y2)  print(str)
			err = 9
			goto quit
		end

		err = modbus_write_do(COMx,TimeOut,Addr485,1,1,0)
		if (err > 0) then goto quit end
		delayms(100)
		err,y1,y2 = modbus_read_di(COMx,TimeOut,RC604Addr,1,2)
		if (y1 == 1 and y2 == 0) then
			str = string.format(" y1 = 1(%d)， y2 = 0(%d)  -- Ok ", y1, y2)  print(str)
		else
			str = string.format(" y1 = 1(%d)， y2 = 0(%d)  -- Err ", y1, y2)  print(str)
			err = 9
			goto quit
		end

		err = modbus_write_do(COMx,TimeOut,Addr485,1,0,1)
		if (err > 0) then goto quit end
		delayms(100)
		err,y1,y2 = modbus_read_di(COMx,TimeOut,RC604Addr,1,2)
		if (y1 == 0 and y2 == 1) then
			str = string.format(" y1 = 0(%d)， y2 = 1(%d)  -- Ok ", y1, y2)  print(str)
		else
			str = string.format(" y1 = 0(%d)， y2 = 1(%d)  -- Err ", y1, y2)  print(str)
			err = 9
			goto quit
		end

		err = modbus_write_do(COMx,TimeOut,Addr485,1,0,0)
		if (err > 0) then goto quit end
	end

::quit::
	if (err == 0) then
		PrintOk()
		time2 = get_runtime()
		time1 = (time2 - time1) / 1000
		str = string.format("  执行时间 = %0.1f 秒", time1)	   print(str)
	else
		if (err == 6) then
			print("RS485通信错误")
		else
			str = string.format("错误代码 = %d", err)
			print(str)
		end
		PrintError()
	end
end

function AutoTestRC608(void)
	local err,y1,y2,y3,y4
	local str
	local time1,time2
	local Model
	local SoftVer
	local RFHard
	local RFSoft
	local ModelStr = "xxxxx"
	local RegAddr
	local ch_num

	print("\r\n-------开始测试---------")
	time1 = get_runtime()

	uart_clear_rx(COMx)

	err =  modbus_write_u16(COMx, 1000, 1, 0x1007, 1)

	--读硬件型号和版本
	err,Model,SoftVer = modbus_read_u16(COMx,TimeOut,Addr485,0x9000,2)
	if (err > 0) then goto quit end

	uart_clear_rx(COMx)
	err, RFHard, RFSoft = modbus_read_u16(COMx,TimeOut,Addr485,0x9009,2)
	if (err > 0) then goto quit end

	if (Model == 0xC602) then
		ModelStr = "RC602"
		ch_num = 2
	else
		if (Model == 0xC604) then
			ModelStr = "RC604"
			ch_num = 4
		else
			if (Model == 0xC608) then
				ModelStr = "RC608"
				ch_num = 8
			else
				if (Model == 0xC612) then
					ModelStr = "RC612"
					ch_num = 12
				else
					if (Model == 0xC616) then
						ModelStr = "RC616"
						ch_num = 16
					else
						if (Model == 0xC632) then
							ModelStr = "RC632"
							ch_num = 32
						else
							ch_num = -1
						end
					end
				end
			end
		end
	end

	if (RFHard == 0) then ModelStr = ModelStr.."-XXX" end
	if (RFHard == 1) then ModelStr = ModelStr.."-485" end
	if (RFHard == 2) then ModelStr = ModelStr.."-232" end
	if (RFHard == 0x30) then ModelStr = ModelStr.."-2K" end
	if (RFHard == 0x31) then ModelStr = ModelStr.."-8K" end

	if (RFHard == 0x32) then
		if ((RFSoft & 0xFF) == 0x14) then
			ModelStr = ModelStr.."-3K"
		else
			if ((RFSoft & 0xFF) == 0x1E) then
				ModelStr = ModelStr.."-7K"
			else
				ModelStr = ModelStr.."-xx"
			end
		end
	end

	if (RFHard == 0x82) then ModelStr = ModelStr.."-WiFi" end
	if (RFHard == 0x83) then ModelStr = ModelStr.."-NBIoT" end

	str = string.format("型号: %s  V%X.%02X", ModelStr, SoftVer >> 8, SoftVer & 0xFF) print(str)

	if (ch_num > 0) then
		TestRC608(ch_num)
	else

	end

	if (RFHard == 0x30 or RFHard == 0x32) then
		delayms(500)
		Test433M()
	end

::quit::
	if (err == 0) then
		time2 = get_runtime()
		time1 = (time2 - time1) / 1000
		str = string.format("  执行时间 = %0.1f 秒", time1)	   print(str)
	else
		if (err == 6) then
			print("RS485通信错误")
		end
		PrintError()
	end
end

--自动识别型号
function AutoDetect(void)
	local err,y1
	local i

	err = 0
	err = err + modbus_write_u16(COMx, 1000, Addr485, 0x9100, 4)
	if (err > 0) then goto quit end
	delayms(100)

	for i=1,50,1 do
		err,y1 = modbus_read_u16(COMx,1000, Addr485,0x9100, 1)
		if (err == 0) then
			if (y1 == 0) then
				break
			end
		end

		delayms(100)
	end

::quit::
	if (err > 0) then
		print("-------识别失败----------")
		beep(5,5,3)
	else
		print("-------识别OK----------")
		AutoTestRC608()
	end
end

--自动测试RC602 604 608 612 616 632
function TestRC608(ch_num)
	local err,y1,y2,y3,y4,y5,y6,y7,y8
	local str
	local time1,time2
	local Model
	local RegAddr
	local S = {0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0}
	local Y = {0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0}
	local i
	local terr = 0
	local strerr = ""

	time1 = get_runtime()

	uart_clear_rx(COMx)
	err = modbus_write_do(COMx,TimeOut,Addr485,1, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0)
	if (err > 0) then goto quit end
	delayms(60)
	for i = 0, ch_num - 1, 1 do
		RegAddr = 1 + i

		uart_clear_rx(COMx)
		err, y1 = modbus_read_di(COMx,TimeOut,RC604Addr, RegAddr, 1)
		if (err > 0) then goto quit end

		uart_clear_rx(COMx)
		err = modbus_write_do(COMx,TimeOut,Addr485,RegAddr, 1)
		if (err > 0) then goto quit end
		delayms(60)
		uart_clear_rx(COMx)
		err, y2 = modbus_read_di(COMx,TimeOut,RC604Addr, RegAddr, 1)
		if (err > 0) then goto quit end

		if (y1 == 0 and y2 == 1) then
			--str = string.format(" Y%d  -- Ok ", i + 1)  print(str)
		else
			--str = string.format(" Y%d  -- Err %d,%d", i + 1, y1, y2)  print(str)
			err = 9
			terr = terr + 1
			strerr = strerr..string.format("Y%d ", i + 1)
		end
	end

	print("")
	uart_clear_rx(COMx)
	err = modbus_write_do(COMx,TimeOut,RC604Addr,1, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0)
	if (err > 0) then goto quit end
	delayms(60)
	for i = 0, ch_num - 1, 1 do
		RegAddr = 1 + i
		uart_clear_rx(COMx)
		err, y1 = modbus_read_di(COMx,TimeOut,Addr485, RegAddr, 1)
		if (err > 0) then goto quit end

		uart_clear_rx(COMx)
		err = modbus_write_do(COMx,TimeOut,RC604Addr,RegAddr, 1)
		if (err > 0) then goto quit end
		delayms(60)
		uart_clear_rx(COMx)
		err, y2 = modbus_read_di(COMx,TimeOut,Addr485, RegAddr, 1)
		if (err > 0) then goto quit end

		if (y1 == 0 and y2 == 1) then
			--str = string.format(" X%d  -- Ok ", i + 1)  print(str)
		else
			--str = string.format(" X%d  -- Err %d,%d", i + 1, y1, y2)  print(str)
			err = 9
			terr = terr + 1
			strerr = strerr..string.format("X%d ", i + 1)
		end
	end

::quit::
	if (err == 0 and terr == 0) then
		print("-------IO测试OK----------")
		beep()
	else
		if (err == 6) then
			print("RS485通信错误")
		else
			if (terr > 0) then
				str = "错误:"..strerr	print(str)
			else
				str = string.format(" 错误代码 = %d", terr)	print(str)
			end
		end

		print("-------IO测试失败----------")
		beep(5,5,3)
	end
end

--检查433M射频功能
function Test433M(void)
	local err,y1,y2,y3,y4
	local str
	local COM_TTL = 7
	local rx_len = 0
	local rx_str = ""

	err = 0
	err = err + modbus_write_u16(COMx, 1000, 1, 0x1007, 1)

	gpio_cfg(0, 5)	-- 配置D0为UART功能
	gpio_cfg(1, 5)	-- 配置D1为UART功能
	uart_cfg(COM_TTL, 9600, 0, 8, 1)

	--D2 D3 = M0 M1 = 1 进入指令模式
	gpio_cfg(2,1)
	gpio_cfg(3,1)
	gpio_write(2,1)
	gpio_write(3,1)

	delayms(20)

	uart_clear_rx(COM_TTL)

	--0001 1111    0000 0000
	uart_send(COM_TTL, "\xC2\xFF\xFF\x1F\x17\x00")
	rx_len, rx_str = uart_recive(COM_TTL, 6, 100)
	print_hex(rx_len, rx_str)

	--M0 M1 = 0 进入工作状态
	gpio_write(2,0)
	gpio_write(3,0)
	delayms(20)

	err = err + modbus_write_do(COM_TTL,1000, 1, 1, 1)
	delayms(100)
	err = err + modbus_write_do(COM_TTL,1000, 1, 1, 0)

	err = err + modbus_write_u16(COMx, 1000, 1, 0x1007, 3)

	if (err > 0) then
		print("-------433M测试失败----------")
		beep(5,5,3)
	else
		print("-------433M测试OK----------")
		beep()
	end
end

------------------------------------------------------

--检查一个值是否在公差范围 1表示err 0表示ok
function check_err(data, mid, diff)
	local re
	local dd

	if (mid < 0) then
		dd = -mid * diff
	else
		dd = mid * diff
	end

	if ((data >= mid - dd) and  (data <= mid + dd)) then
		re = 0
	else
		re = 1
	end
	return re
end

--校准  返回0表示0K, 1表示错误
function Calib402(void)
	local Model
	local err,y1,y2,y3,y4
	local x1,x2,K,B
	local str
	local time1,time2
	local RegAddr
	local adc1
	local adc2
	local Temp
	local i
	local VoltAdc1 = 1202
	local VoltAdc2 = 2860
	local CurrAdc1 = 615
	local CurrAdc2 = 2923
	local DIFF = 0.05

	print("-------开始校准------------------------------------")
	time1 = get_runtime()

	err,y1,y2 = modbus_read_u16(COMx,TimeOut,RC604Addr,0x9000,2)
	if (err > 0) then
		str = string.format("未找到工装RC604") print(str)
		goto quit
	end

	err,y1,y2 = modbus_read_u16(COMx,TimeOut,RC302Addr,0x9000,2)
	if (err > 0) then
		str = string.format("未找到工装RC302") print(str)
		goto quit
	end

	--初始化基本参数和模拟通道配置参数
	err = modbus_write_u16(COMx,2000,Addr485,0x9100,0x5A50)
	if (err > 0) then
		str = string.format("初始化基本参数和模拟通道配置参数失败") print(str)
		goto quit
	else
		str = string.format("初始化基本参数和模拟通道配置参数 ok") print(str)
	end

	--初始化模拟量校准参数
	err = modbus_write_u16(COMx,3000,Addr485,0x9100,0x5AA0)
	if (err > 0) then
		str = string.format("初始化模拟量校准参数失败") print(str)
		goto quit
	else
		str = string.format("初始化模拟量校准参数 ok") print(str)
	end
	print("")

	--读硬件型号和版本
	err,y1,y2 = modbus_read_u16(COMx,TimeOut,Addr485,0x9000,2)
	if (err > 0) then goto quit end
	str = string.format("型号 = %04X 固件版本 = V%X.%02X", y1, y2 >> 8, y2 & 0xFF) print(str)

	Model = y1

	if (Model == RC202_NORMAL) then
		str = string.format("已识别机型: RC602")  print(str)
	else
		if (Model == RD202_20MA) then
			str = string.format("已识别机型: RD202-20MA")  print(str)
		else
			if (Model == RD202_18B20) then
				str = string.format("已识别机型: RD202-18B20")  print(str)
			else
				if (Model == RD202_PULSE) then
					str = string.format("已识别机型: RD202-PULSE")  print(str)
				else
					if (Model == 0xC402) then
						str = string.format("已识别机型: RC402")  print(str)
					else
						str = string.format("未知机型")  print(str)
						goto quit
					end
				end
			end
		end
	end

	if (Model ~= RC202_NORMAL and Model ~= RD202_20MA and Model ~= 0xC402) then
		str = string.format(" 该型号无需模拟量校准")  print(str)
		goto quit
	end

	err = modbus_write_u16(COMx,TimeOut,Addr485,REG06_CALIB_KEY, 0x55AA)
	if (err > 0) then goto quit end
	--print("校准开关已打开")

	--print("--校准零位...")
		--切换继电器
		err = modbus_write_do(COMx,TimeOut,RC604Addr,1,0,0,0,0)
		if (err > 0) then goto quit end

		delayms(1000)

		err,y1,y2 = modbus_read_float(COMx,TimeOut,Addr485,REG04_CH01_ADC_AVG, 2)
		if (err > 0) then goto quit end
		if (check_err(y1, 0.5, 1) == 1) then
			str = string.format(" CH1 零位ADC = %f --- Err", y1) print(str)
			err = 9
			goto quit
		end
		if (check_err(y2, 0.5, 1) == 1) then
			str = string.format(" CH2 零位ADC = %f --- Err", y1) print(str)
			err = 9
			goto quit
		end

		--写通道1零位ADC,2个量程ADC一样
		RegAddr = 	REG03_CC01_K0 + CC01_ZERO_ADC_0;
		err = modbus_write_float(COMx,TimeOut,Addr485, RegAddr, y1)
		if (err > 0) then goto quit end

		RegAddr = 	REG03_CC01_K0 + CC01_ZERO_ADC_0 + RANGE_REG_STEP;
		err = modbus_write_float(COMx,TimeOut,Addr485, RegAddr, y1)
		if (err > 0) then goto quit end

		--写通道2零位ADC,2个量程ADC一样
		RegAddr = 	REG03_CC01_K0 + CALIB_REG_STEP + CC01_ZERO_ADC_0;
		err = modbus_write_float(COMx,TimeOut,Addr485, RegAddr, y2)
		if (err > 0) then goto quit end

		RegAddr = 	REG03_CC01_K0 + CALIB_REG_STEP + CC01_ZERO_ADC_0 + RANGE_REG_STEP;
		err = modbus_write_float(COMx,TimeOut,Addr485, RegAddr, y2)
		if (err > 0) then goto quit end

		str = string.format(" CH1 CH2 零位ADC = %f, %f --OK", y1, y2) print(str)

	--读取NTC温度
	err, Temp = modbus_read_float(COMx,TimeOut,Addr485,REG04_CH01_OUT_F + 12,1)
	if (err > 0) then goto quit end
	str = string.format(" NTC温度 = %f ℃", Temp) print(str)

	--校准2个通道的电压档位
	for i = 0, 1, 1 do

		str = string.format("\r\n校准通道%d电压，第1点 %dmV, 第2点 %dmV", i + 1, CALIB_VOTL1, CALIB_VOTL2)
		print(str)

		--切换继电器 电压
		err = modbus_write_do(COMx,TimeOut,RC604Addr,1, 1 - i, i,0,0)
		if (err > 0) then
			goto quit
		end

		--设置DAC电压 - 校准点1
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_VOLT_01, CALIB_VOTL1)
		if (err > 0) then
			goto quit
		end

		delayms(1000)

		err,y1 = modbus_read_float(COMx,TimeOut, Addr485,REG04_CH01_ADC_RMS + 2 * i, 2)
		if (err > 0) then goto quit end
		--str = string.format("  RMS_ADC = %f", y1) print(str)

		--保存第1点adc
		adc1 = y1
		if (check_err(adc1, VoltAdc1, DIFF) == 1) then
			str = string.format(" CH%d电压Adc1 = %f -- Err (ok = %f)", i + 1, y1, VoltAdc1) print(str)
			err = 9
			goto quit
		else
			str = string.format(" CH%d电压Adc1 = %f -- Ok", i + 1, y1) print(str)
		end

		--str = string.format("\r\n校准通道%d电压，第2点 %dmV", i + 1, CALIB_VOTL2) print(str)
		--设置DAC电压 - 校准点2
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_VOLT_01, CALIB_VOTL2)
		if (err > 0) then
			goto quit
		end

		delayms(1000)

		err,y1 = modbus_read_float(COMx,TimeOut,Addr485,REG04_CH01_ADC_RMS + 2 * i, 2)
		if (err > 0) then goto quit end
		--str = string.format("  RMS_ADC = %f", y1) print(str)

		--保存第2点adc
		adc2 = y1
		if (check_err(adc2, VoltAdc2, DIFF) == 1) then
			str = string.format(" CH%d电压Adc2 = %f -- Err (ok = %f)", i + 1, y1, VoltAdc2) print(str)
			err = 9
			goto quit
		else
			str = string.format(" CH%d电压Adc2 = %f -- Ok", i + 1, y1) print(str)
		end

		--计算K,B
		x1 = adc1
		y1 = CALIB_VOTL1 / 1000
		x2 = adc2
		y2 = CALIB_VOTL2 / 1000

		K = (y2 - y1) / (x2 - x1);
		B = (x2 * y1 - x1 * y2)/(x2 - x1);

		str = string.format(" X1 = %f, X2 = %f, X2 = %f, Y2 = %f",x1,y1,x2,y2) print(str)
		str = string.format(" K = %f, B = %f", K, B) print(str)

		RegAddr = REG03_CC01_K0 + i * CALIB_REG_STEP + RANGE_REG_STEP + CC01_K_0
		err = modbus_write_float(COMx,TimeOut,Addr485, RegAddr, K,B,Temp,0,0)
		if (err > 0) then goto quit end
		str = string.format(" 通道%d电压档校准参数写入成功", i + 1) print(str)
	end

	--校准电流档位
	print("")

	for i = 0, 1, 1 do
		--校准通道1电流
		str = string.format("\r\n校准通道%d电流，第1点 %fmA  第2点 %fmA", i + 1, CALIB_CURR1 / 1000, CALIB_CURR2 / 1000)
		print(str)

		--切换继电器 电压
		err = modbus_write_do(COMx,TimeOut,RC604Addr,1, 0, 0, 1 - i, i)
		if (err > 0) then
			goto quit
		end

		--设置DAC电压 - 校准点1
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_CURR_01, CALIB_CURR1)
		if (err > 0) then
			goto quit
		end

		delayms(1000)

		err,y1 = modbus_read_float(COMx,TimeOut, Addr485,REG04_CH01_ADC_RMS + 2 * i, 2)
		if (err > 0) then goto quit end
		--str = string.format("  RMS_ADC = %f", y1) print(str)

		--保存第1点adc
		adc1 = y1
		if (check_err(adc1, CurrAdc1, DIFF) == 1) then
			str = string.format("  CH%d电流Adc1 = %f -- Err (ok = %f)", i + 1, y1, CurrAdc1) print(str)
			err = 9
			goto quit
		else
			str = string.format("  CH%d电流Adc1 = %f -- Ok", i + 1, y1) print(str)
		end

		--str = string.format("\r\n校准通道%d电流，第1点 %duA", i + 1, CALIB_CURR2)	print(str)
		--设置DAC电压 - 校准点2
		err = modbus_write_u16(COMx,TimeOut,RC302Addr,REG03_AO_CURR_01, CALIB_CURR2)
		if (err > 0) then
			goto quit
		end

		delayms(1000)

		err,y1 = modbus_read_float(COMx,TimeOut,Addr485,REG04_CH01_ADC_RMS + 2 * i, 2)
		if (err > 0) then goto quit end
		--str = string.format("  RMS_ADC = %f", y1) print(str)

		--保存第2点adc
		adc2 = y1
		if (check_err(adc2, CurrAdc2, DIFF) == 1) then
			str = string.format("  CH%d电流Adc2 = %f -- Err (ok = %f)", i + 1, y1, CurrAdc2) print(str)
			err = 9
			goto quit
		else
			str = string.format("  CH%d电流Adc2 = %f -- Ok", i + 1, y1) print(str)
		end

		--计算K,B
		x1 = adc1
		y1 = CALIB_CURR1 / 1000
		x2 = adc2
		y2 = CALIB_CURR2 / 1000

		K = (y2 - y1) / (x2 - x1);
		B = (x2 * y1 - x1 * y2)/(x2 - x1);

		str = string.format(" X1 = %f, X2 = %f, X2 = %f, Y2 = %f",x1,y1,x2,y2) print(str)
		str = string.format(" K = %f, B = %f", K, B) print(str)

		RegAddr = REG03_CC01_K0 + i * CALIB_REG_STEP + 0 * RANGE_REG_STEP + CC01_K_0
		err = modbus_write_float(COMx,TimeOut,Addr485, RegAddr, K,B,Temp,0,0)
		if (err > 0) then goto quit end
		str = string.format("--通道%d电流档校准参数写入成功", i + 1) print(str)
	end

::quit::
	if (err == 0) then
		print("-------校准成功------------------------------------")
		time2 = get_runtime()
		time1 = (time2 - time1) / 1000
		str = string.format("  执行时间 = %0.1f 秒", time1)	   print(str)
		return 0
	else
		if (err == 6) then
			print("RS485通信错误")
		else
			str = string.format("错误代码 = %d", err)
			print(str)
		end
		PrintError()
		return 1
	end
end
