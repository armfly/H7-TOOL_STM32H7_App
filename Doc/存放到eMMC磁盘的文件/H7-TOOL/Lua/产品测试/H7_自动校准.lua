--F01=自动校准,test_calib()
--F02=---校准TVCC---,calib_tvcc_volt()
--F03=---校准DAC电压和电流---,calib_dac()
--F04=---校准示波器电压---,calib_ch1ch2()
--F05=---校准TVCC电流和高侧电流,calib_curr()
--F06=---校准NTC---,calib_ntc()

beep()
print("脚本版本 V1.04 2019-11-28")

local str  --用于打印

--根据2点方程求值
function cacul(x1,y1,x2,y2,x)
	local ff

	if (x2 == x1) then
		ff = 0
	else
		ff = y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
	end
	return ff
end

--返回0-7通道的电压 V
function ad7606_volt(ch)
	local X1 = {39,		39,		39,		36,		73,		74,		73,		71}
	local Y1 = {0,		0,		0,		0,		0,		0,		0,		0}
	local X2 = {29417,	29362,	29520,	29396,	29407,	29407,	29407,	29407}
	local Y2 = {8.999,	8.999,	8.999,	8.999,	8.999,	8.999,	8.999,	8.999}
	local adc
	local volt

	adc = ex_adc(ch)
	volt = cacul(X1[ch+1], Y1[ch+1], X2[ch+1], Y2[ch+1], adc)
	return volt
end

--返回0-7通道的电流 mA
function ad7606_curr(ch)
	local X1 = {75,		75,		74,		72,		73,		74,		73,		71}
	local Y1 = {0,		0,		0,		0,		0,		0,		0,		0}
	local X2 = {29417,	29362,	29520,	29396,	29407,	29812,	31786,	29017}
	local Y2 = {8.999,	8.999,	8.999,	8.999,	8.999,	454.64,	19.482,	88.900}
	local adc
	local curr

	adc = ex_adc(ch)
	curr = cacul(X1[ch+1], Y1[ch+1], X2[ch+1], Y2[ch+1], adc)
	return curr
end

--设置DAC8563输出电压，单位V, 浮点
function dac8563_volt(volt)
	local X1 = 1000
	local Y1 = -9.8551
	local X2 = 32767
	local Y2 = -0.003030
	local X3 = 64000
	local Y3 = 9.6802
	local dac

	if (volt < Y2) then
		dac = cacul(Y1,X1,Y2,X2,volt)
	else
		dac = cacul(Y2,X2,Y3,X3,volt)
	end
	ex_dac(0, dac)
end

--启动示波器，ADC采集，多通道模式
function start_dso(void)
	print("启动多通道低速测量模式")
	write_reg16(0x01FF, 2) --多通道低速测量
	write_reg16(0x0200, 1) -- CH1选DC耦合
	write_reg16(0x0201, 1) -- CH2选DC耦合
	write_reg16(0x0202, 0) -- CH1通道增益0档，不放大
	write_reg16(0x0203, 0) -- CH2通道增益0档，不放大
	write_reg16(0x0204, 0) -- CH1通道直流偏值，未用
	write_reg16(0x0205, 0) -- CH2通道直流偏值，未用
	write_reg16(0x0206, 12) --采样频率1M
	write_reg16(0x0207, 0) --采样深度1K
	write_reg16(0x0208, 0) --触发电平
	write_reg16(0x0209, 50) --触发位置
	write_reg16(0x020A, 0) --触发模式 0=自动
	write_reg16(0x020B, 0) --触发通道CH1
	write_reg16(0x020C, 0) --触发边沿
	write_reg16(0x020D, 2) --通道使能
	write_reg16(0x020E, 1) --开始采集
end


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

--关闭所有的继电器
function colse_all_y(void)
	return ch
end

--成功叫一声，失败叫三声
function disp_result(err)
	if (err == 0) then
		print("\n*****测试通过*****")
		beep()
	else
		print("\n*****测试失败*****")
		beep()
		delayms(100)
		beep()
		delayms(100)
		beep()
	end
end

--校准初始化
function calib_init(void)
	write_tvcc_dac(43) --设置TVCC大概3.3V
	delayms(100)
	ex_start()	--会关闭全部继电器
	start_dso()	--启动多通道低速采集模式
	write_reg16(0xBFFF, 1)	--打开校准开关
end

--校准TVCC设置电压和采样电压
function calib_tvcc_volt(void)
	local adc
	local volt
	local err

	close_all()
	ex_dout(20,1)
	ex_dout(21,1)

	err = 0
	--先校准TVCC设置电压和检测电压
	print("")
	print("---校准TVCC---")

	---第1档
	write_tvcc_dac(36)	--4.4V左右
	delayms(1500)
	volt = ad7606_volt(0) --读取AD7606 TVCC电压
	if (check_err(volt, 4.416, 0.05)==1) then
		str = string.format("SET = 36  实际电压 = %f err", volt) print(str) delayms(5)
		err = err + 1
	else
		str = string.format("SET = 36  实际电压 = %f ok", volt) print(str) delayms(5)
		write_regfloat(0xC0C0, 36)
		write_regfloat(0xC0C2, volt)
	end

	adc = read_adc(4) --读cpu adc tvcc电压
	if (check_err(adc, 46368, 0.1)==1) then
		print("TVCC Volt ADC     =", adc, "err") delayms(5)
		err = err + 1
	else
		print("TVCC Volt ADC     =", adc, "ok") delayms(5)
		write_regfloat(0xC0AC, adc)
		write_regfloat(0xC0AE, volt)
	end

	adc = read_adc(2) --读cpu adc 高侧电压
	if (check_err(adc, 8117, 0.1)==1) then
		print("HighSide Volt ADC =", adc, "err") delayms(5)
		err = err + 1
	else
		print("HighSide Volt ADC =", adc, "ok") delayms(5)
		write_regfloat(0xC084, adc)
		write_regfloat(0xC086, volt)
	end

	---第2档
	print("")
	write_tvcc_dac(100)
	delayms(1500)
	volt = ad7606_volt(0) --读取AD7606 TVCC 电压
	if (check_err(volt, 1.59, 0.1)==1) then
		str = string.format("SET = 100  实际电压 = %f err", volt) print(str) delayms(5)
		err = err + 1
	else
		str = string.format("SET = 100  实际电压 = %f ok", volt) print(str) delayms(5)
		write_regfloat(0xC0C4, 100)
		write_regfloat(0xC0C6, volt)
	end

	adc = read_adc(4) --读cpu adc tvcc电压
	if (check_err(adc, 16679, 0.1)==1) then
		print("TVCC Volt ADC     =", adc, "err") delayms(5)
		err = err + 1
	else
		print("TVCC Volt ADC     =", adc, "ok") delayms(5)
		write_regfloat(0xC0A8, adc)
		write_regfloat(0xC0AA, volt)
	end

	adc = read_adc(2) --读cpu adc 高侧电压
	if (check_err(adc, 2879, 0.1)==1) then
		print("HighSide Volt ADC =", adc, "err") delayms(5)
		err = err + 1
	else
		print("HighSide Volt ADC =", adc, "ok") delayms(5)
		write_regfloat(0xC080, adc)
		write_regfloat(0xC082, volt)
	end

	--恢复TVCC电压3.3，后面就不动了
	write_tvcc_dac(47)
	return err
end

--校准CPU的DAC输出电压和电流
function calib_dac(void)
	local i
	local err
	local volt
	local curr
	local dac
	local dac_tab = {500, 1500, 2500, 3500}
	local volt_mid = {-9.176, -3.235, 2.711, 8.661}
	local curr_mid = {2.592, 7.723, 12.857, 17.992}
	local volt_err = 0.1
	local curr_err = 0.1

	close_all()
	ex_dout(0,1)
	ex_dout(19,1)

	err = 0
	dac_on()	--打开DAC电源，设置为直流输出
	delayms(100)

	print("")
	print("---校准DAC电压和电流---")
	for i = 0, 3, 1 do
		dac = dac_tab[i + 1]
		dac_write(dac)	--CPU DAC输出
		delayms(500)

		volt = ad7606_volt(1)
		if (check_err(volt, volt_mid[i + 1], volt_err) == 1) then
			str = string.format("DAC电压 = %d  %f err", dac, volt) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("DAC电压 = %d  %f ok", dac, volt) print(str) delayms(5)
			write_reg16(0xC0C8 + i * 2,  dac)
			write_reg16(0xC0C9 + i * 2,  volt * 1000)
		end

		curr = ad7606_curr(6)  --20mA电流
		if (check_err(curr, curr_mid[i + 1], curr_err) == 1) then
			str = string.format("DAC电流 = %d  %f err", dac, curr) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("DAC电流 = %d  %f ok", dac, curr) print(str) delayms(5)
			write_reg16(0xC0D0 + i * 2,  dac)
			write_reg16(0xC0D1 + i * 2,  curr * 1000)
		end
	end
	return err
end

--校准示波器功能
function calib_ch1ch2(void)
	local i
	local err
	local volt
	local adc
	local ch_tab = {9.0, 6.0, 3.0, 1.5, 0.75, 0.375, 0.170, 0.09}
	local zero_mid = {32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768}
	local full_mid = {53641, 60506, 60629, 60686, 60857, 60915, 58393, 60166}
	local adc_err1 = 0.22
	local adc_err2 = 0.12

	close_all()
	ex_dout(4,1)

	print("")
	print("---校准示波器电压---")	delayms(5)
	err = 0
	print("悬空校准零位") delayms(5)
	for i = 0, 7, 1 do
		write_reg16(0x0202, i) -- CH1通道增益0档，不放大
		write_reg16(0x0203, i) -- CH2通道增益0档，不放大

		if (i == 0) then
			delayms(1200)
		else
			delayms(1200)
		end
		adc = read_adc(0)
		if (check_err(adc, zero_mid[i + 1], adc_err1)==1) then
			str = string.format("  CH1 %d adc = %f err", i, adc)
			print(str)
			err = err + 1
		else
			str = string.format("  CH1 %d adc = %f ok", i, adc) print(str) delayms(5)

			write_regfloat(0xC000 + 8 * i, adc)
			write_regfloat(0xC002 + 8 * i, 0)
		end

		adc = read_adc(1)
		if (check_err(adc, zero_mid[i + 1], adc_err1)==1) then
			str = string.format("  CH2 %d adc = %f err", i, adc) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("  CH2 %d adc = %f ok", i, adc) print(str) delayms(5)
			write_regfloat(0xC040 + 8 * i, adc)
			write_regfloat(0xC042 + 8 * i, 0)
		end
	end

	if (err > 0) then
		goto quit
	end

	close_all()
	ex_dout(12,1)
	ex_dout(13,1)
	ex_dout(14,1)
	delayms(1000)

	print("校准满位")
	for i = 0, 7, 1 do
		write_reg16(0x0202, i) -- CH1通道增益0档，不放大
		write_reg16(0x0203, i) -- CH2通道增益0档，不放大

		dac8563_volt(ch_tab[i + 1])

		delayms(1000)
		adc = read_adc(0)
		volt = ad7606_volt(1)
		if (check_err(adc, full_mid[i + 1], adc_err2)==1) then
			str = string.format("  CH1 %d volt = %f  adc = %f err", i, volt, adc) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("  CH1 %d volt = %f  adc = %f ok", i, volt, adc) print(str) delayms(5)
			write_regfloat(0xC004 + 8 * i, adc)
			write_regfloat(0xC006 + 8 * i, volt)
		end

		adc = read_adc(1)
		volt = ad7606_volt(1)
		if (check_err(adc, full_mid[i + 1], adc_err2)==1) then
			str = string.format("  CH2 %d volt = %f  adc = %f err", i, volt, adc) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("  CH2 %d volt = %f  adc = %f ok", i, volt, adc) print(str) delayms(5)
			write_regfloat(0xC044 + 8 * i, adc)
			write_regfloat(0xC046 + 8 * i, volt)
		end
	end

::quit::
	return err
end

--校准tvcc电流和高侧电流
function calib_curr(void)
	local i
	local err
	local curr
	local adc
	local set_tabe1 = {0, 0.15 * 10, 0.3*10, 0.4*10}
	local high_mid1 = {160, 7700, 15330, 20296}
	local tvcc_mid = {477, 21458, 42877, 56882}
	local adc_err1 = {0.6,0.1,0.1,0.1}

	local set_tabe2 = {0, 0.03 * 50, 0.05*50, 0.09 * 50}
	local high_mid2 = {1300, 16421, 26873, 48841}
	local adc_err2 = {0.9,0.1,0.1,0.1}
	local volt

	print("")
	print("---校准TVCC电流和高侧电流---")
	err = 0

	close_all()
	ex_dout(20,1)
	ex_dout(21,1)

	--TVCC电流和高侧电流相同。先测1.2A量程
	print("---1.2A---")	 delayms(5)
	write_reg16(0x0211, 1) --1.2A量程 HIGH_SIDE
	for i = 0, 3, 1 do
		volt = set_tabe1[i+1]
		str = string.format("输出电压 = %fV", volt) print(str) delayms(5)
		if (volt == 0) then
			ex_dout(20,0)
			ex_dout(21,0)

			ex_dout(22,0)
			ex_dout(23,0)
			set_tvcc(5)	--设置TVCC输出电压5v 校准0位
			delayms(1000)
		else
			ex_dout(20,1)
			ex_dout(21,1)

			set_tvcc(volt)	--设置TVCC输出电压。负载电阻为10欧
			ex_dout(23,0)
			ex_dout(22,1)  --选择10欧负载
		end

		delayms(1000)

		if (volt == 0) then
			curr = 0
		else
			curr = ad7606_curr(5)	--10欧负载，大电流
		end

		adc = read_adc(3)	--3=高侧电流cpu ADC
		if (check_err(adc, high_mid1[i + 1], adc_err1[i + 1])==1) then
			str = string.format("  高侧电流 = %fmA  adc = %f err", curr, adc) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("  高侧电流 = %fmA  adc = %f ok", curr, adc) print(str) delayms(5)
			write_regfloat(0xC098 + 4 * i, adc)
			write_regfloat(0xC09A + 4 * i, curr)
		end

		adc = read_adc(5)	--5=tvcc电流
		if (check_err(adc, tvcc_mid[i + 1], adc_err1[i + 1])==1) then
			str = string.format("  TVCC电流 = %fmA  adc = %f err", curr, adc) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("  TVCC电流 = %fmA  adc = %f ok", curr, adc) print(str) delayms(5)
			write_regfloat(0xC0B0 + 4 * i, adc)
			write_regfloat(0xC0B2 + 4 * i, curr)
		end
	end

	--高侧电流相同。测高侧电流100量程
	print("")
	print("---120mA---")	 delayms(5)
	write_reg16(0x0211, 0) --120mA量程 HIGH_SIDE
	for i = 0, 3, 1 do
		volt = set_tabe2[i+1]
		str = string.format("输出电压 = %fV", volt) print(str) delayms(5)
		if (volt == 0) then
			ex_dout(22,0)
			ex_dout(23,0)
		else
			set_tvcc(volt)	--设置TVCC输出电压。负载电阻为10欧
			ex_dout(22,0)
			ex_dout(23,1)--选择50欧负载
		end

		delayms(1000)

		if (volt == 0) then
			curr = 0
		else
			curr = ad7606_curr(7)	--50欧负载，大电流
		end

		adc = read_adc(3)	--3=高侧电流cpu ADC
		if (check_err(adc, high_mid2[i + 1], adc_err2[i + 1])==1) then
			str = string.format("  高侧电流 = %fmA  adc = %f err", curr, adc) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("  高侧电流 = %fmA  adc = %f ok", curr, adc) print(str) delayms(5)
			write_regfloat(0xC088 + 4 * i, adc)
			write_regfloat(0xC08A + 4 * i, curr)
		end
	end

	return err
end

--校准NTC
function calib_ntc(void)
	local i
	local err
	local adc
	local ref = {0.0003, 0.0222, 9.9732, 99.958}
	local Y = {5,6,16,17}
	local adc_mid = {105, 283, 43376, 62376}
	local adc_err = {0.6, 0.7, 0.1, 0.1}

	print("")
	print("---校准NTC---")
	err = 0

	for i=0,3,1 do
		close_all()
		ex_dout(Y[i+1],1)
		delayms(1000)
		adc = read_adc(6)	--6=NTC adc
		if (check_err(adc, adc_mid[i+1], adc_err[i+1])==1) then
			str = string.format("  电阻 = %f欧  adc = %f err", ref[i+1], adc) print(str) delayms(5)
			err = err + 1
		else
			str = string.format("  电阻 = %f欧  adc = %f ok", ref[i+1], adc) print(str) delayms(5)
			write_regfloat(0xC0D8+4*i, adc)
			write_regfloat(0xC0DA+4*i, ref[i+1])
		end
	end
	return err
end

--关闭所有的继电器
function close_all(void)
	local i

	for i=0,23,1 do
		ex_dout(i, 0)
	end
end

--测试主函数
function test_calib(void)
	local err
	local time1
	local time2

	time1 = get_runtime()

	print("\r\n----检查时钟----")
	time2 = os.time()
	print(os.date())
	print(time2)
	if (time2 < 1574937149) then
		print("*****时钟错误*****")
		goto quit
	else
		print("时钟OK > 1574937149")
	end

	err = 0

	err = err + calib_tvcc_volt() --校准TVCC电压
	if (err > 0) then
		goto quit
	end

	err = err + calib_dac() --校准DAC
	if (err > 0) then
		goto quit
	end

	err = err + calib_ch1ch2() --校准示波器
	if (err > 0) then
		goto quit
	end

	err = err + calib_curr() --校准TVCC电流和高侧电流
	if (err > 0) then
		goto quit
	end

	err = err + calib_ntc() --校准ntc
	if (err > 0) then
		goto quit
	end

	save_param()	--保存参数到eeprom

::quit::
	disp_result(err)  delayms(5)

	close_all()

	time2 = get_runtime()
	str = string.format("测试时间 : %f 秒",(time2 - time1) / 1000) print(str) delayms(5)
end

	calib_init()
