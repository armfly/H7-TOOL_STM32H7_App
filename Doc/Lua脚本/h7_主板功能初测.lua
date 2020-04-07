--F01=主板功能预测试,test_gpio()
--F02=TVCC+NTC测试,test_tvcc()
--F03=示波器电路测试,test_ch1ch2()
--F04=执行01-02-03,test_gpio() test_tvcc() test_ch1ch2()
--F05=测试转接板LED,test_ledout()
--F06=时钟,print(os.date())
--F07=扩展板继电器全开,test_extio_open_do()
--F08=扩展板继电器全关,test_extio_close_do()
--F09=扩展板DI读取,test_extio_di()
--F10=,close_all()

beep()
print("H7-TOOL主板功能初测程序已加载 2020-01-10")

--测试GPIO输入输出
function test_gpio(void)
	local err
	local terr
	local i
	local time1
	
	print("")
	print("----检查时钟----") delayms(5)
	time1 = os.time()
	print(os.date())
	print(time1)
	if (time1 < 1574937149) then 
		print("*****时钟错误*****") delayms(5)
		beep()
		delayms(100)
		beep()
		delayms(100)
		beep()			
	else
		print("时钟OK > 1574937149") delayms(5)
	end

	print("") delayms(5)
	print("----开始测试----") delayms(5)
	err=0
	terr=0
--设置TVCC输出3.3V
	set_tvcc(3.3)

--变换输入输出方向，再测一遍
	gpio_cfg(0, 0)
	gpio_cfg(1, 1)
	gpio_cfg(2, 0)
	gpio_cfg(3, 1)
	gpio_cfg(4, 0)
	gpio_cfg(5, 1)
	gpio_cfg(6, 0)
	gpio_cfg(7, 1)
	gpio_cfg(8, 0)
	gpio_cfg(9, 1)	
	
	gpio_write(1, 1) if (gpio_read(0)==1) then err=0 else err=1 end
	gpio_write(1, 0) if (gpio_read(0)==1) then err=err+1 end
	if (err == 0) then print("D1->D0 ok") else print("D1->D0 Error") terr=terr+1 end
	delayms(5)
	
	gpio_write(3, 1) if (gpio_read(2)==1) then err=0 else err=1 end
	gpio_write(3, 0) if (gpio_read(2)==1) then err=err+1 end
	if (err == 0) then print("D3->D2 ok") else print("D3->D2 Error") terr=terr+1 end
	delayms(5)

	gpio_write(5, 1) if (gpio_read(4)==1) then err=0 else err=1 end
	gpio_write(5, 0) if (gpio_read(4)==1) then err=err+1 end
	if (err == 0) then print("D5->D4 ok") else print("D5->D4 Error") terr=terr+1 end
	delayms(5)
	
	gpio_write(7, 1) if (gpio_read(6)==1) then err=0 else err=1 end
	gpio_write(7, 0) if (gpio_read(6)==1) then err=err+1 end
	if (err == 0) then print("D7->D6 ok") else print("D7->D6 Error") terr=terr+1 end	
	delayms(5)

	gpio_write(9, 1) if (gpio_read(8)==1) then err=0 else err=1 end
	gpio_write(9, 0) if (gpio_read(8)==1) then err=err+1 end
	if (err == 0) then print("D9->D8 ok") else print("D9->D8 Error") terr=terr+1 end	
	delayms(5)
		
--4对GPIO，输入输出自动测试	
	gpio_cfg(0, 1)	-- 配置D0为输出
	gpio_cfg(1, 0)	-- 配置D1未输入
	gpio_cfg(2, 1)
	gpio_cfg(3, 0)
	gpio_cfg(4, 1)
	gpio_cfg(5, 0)
	gpio_cfg(6, 1)
	gpio_cfg(7, 0)
	gpio_cfg(8, 1)
	gpio_cfg(9, 0)		
	
	gpio_write(0, 1) if (gpio_read(1)==1) then err=0 else err=1 end
	gpio_write(0, 0) if (gpio_read(1)==1) then err=err+1 end
	if (err == 0) then print("D0->D1 ok") else print("D0->D1 Error") terr=terr+1 end
	delayms(5)
	
	gpio_write(2, 1) if (gpio_read(3)==1) then err=0 else err=1 end
	gpio_write(2, 0) if (gpio_read(3)==1) then err=err+1 end
	if (err == 0) then print("D2->D3 ok") else print("D2->D3 Error") terr=terr+1 end
	delayms(5)

	gpio_write(4, 1) if (gpio_read(5)==1) then err=0 else err=1 end
	gpio_write(4, 0) if (gpio_read(5)==1) then err=err+1 end
	if (err == 0) then print("D4->D5 ok") else print("D4->D5 Error") terr=terr+1 end
	delayms(5)
	
	gpio_write(6, 1) if (gpio_read(7)==1) then err=0 else err=1 end
	gpio_write(6, 0) if (gpio_read(7)==1) then err=err+1 end
	if (err == 0) then print("D6->D7 ok") else print("D6->D7 Error") terr=terr+1 end
	delayms(5)

	gpio_write(8, 1) if (gpio_read(9)==1) then err=0 else err=1 end
	gpio_write(8, 0) if (gpio_read(9)==1) then err=err+1 end
	if (err == 0) then print("D8->D9 ok") else print("D8->D9 Error") terr=terr+1 end
	
	delayms(5)
			
--测试CAN				
	gpio_cfg(12, 1)	
	gpio_cfg(13, 0)	
	
	err = 0
	for i=0,10,1 do
		gpio_write(12, 0) delayus(1) if (gpio_read(13)==1) then err=err+1 end
		delayus(100)
		gpio_write(12, 1) delayus(1) if (gpio_read(13)==0) then err=err+1 end
		delayus(100)
	end
	if (err == 0) then print("CANTX->CANRX ok") else print("CANTX->CANRX Error", err) terr=terr+1 end

--测试TTL-UART
	gpio_cfg(100, 1)	
	gpio_write(100, 0)
	
	gpio_cfg(10, 1)
	gpio_cfg(11, 0)
	
	err = 0
	for i=0,10,1 do
		gpio_write(10, 0) delayus(10) if (gpio_read(11)==1) then err=err+1 end
		delayus(100)
		gpio_write(10, 1) delayus(10) if (gpio_read(11)==0) then err=err+1 end
		delayus(100)
	end
	if (err == 0) then print("TTL UART ok") else print("TTL UART Error", err) terr=terr+1 end
	
	delayus(5000)
	if (terr > 0) then
		print("*****测试失败 terr = ", terr)
		beep()
		delayms(100)
		beep()
		delayms(100)
		beep()	
	else
		print("*****测试通过*****")
		beep()	
	end
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

function test_ch1ch2(void)
	local err
	local i
	local adc
	local dac
	local errd
--DAC大致关系
--CH1 4095=12.356V  2500=2.75V  2058=95mV
--CH2(电流采样电阻200欧)	
--CH1的8档量程判据
	local dac1 = {2047, -1024, 512, 256, 128, 64, 32, 10}
	local mid1 = {60760, 4666, 60844, 60785, 60634, 60127, 59042, 46390}
	local diff1 = {0.02, 0.2, 0.05, 0.06, 0.10, 0.15, 0.18, 0.5} --公差系数
--CH2的8档量程判据	
	local dac2 = {4095, 1024, 512, 256, 128, 64, 32, 16}
	local mid2 = {43121, 41400, 37714, 37575, 37556, 37494, 37633, 42601}
	local diff2 = {0.2, 0.1, 0.08, 0.08, 0.08, 0.08, 0.12, 0.17}
	local str
		
	print("") delayms(5)
	print("----开始示波器电路----") delayms(5)
	start_dso();
	err = 0
	
	dac_on()	--打开DAC电源，设置为电平模式
	
	print("正在测CH1,DC耦合...") delayms(5)
	for i=1,8,1 do
		write_reg16(0x0202, i-1) -- CH1通道增益0-7
		dac = dac1[i] + 2044
		dac_write(dac) delayms(500)	
		adc = read_adc(0)
		errd = mid1[i] * diff1[i];
		if (adc < mid1[i] - errd  or adc > mid1[i] + errd) then 
			err = err + 1
			--print("dac=", dac) delayms(5)
			--print(" adc=", adc) delayms(5)
			--print(" error") delayms(5)
			str = string.format("dac=%f adc=%f error", dac, adc)
			print(str)  delayms(5)	
		else
			--print("dac=", dac) delayms(5)
			--print(" adc=", adc) delayms(5)
			--print(" ok") delayms(5)
			str = string.format("dac=%f adc=%f ok", dac, adc)
			print(str)  delayms(5)	
		end
	end
	
	print("正在测CH2,DC耦合...") delayms(5)
	for i=1,8,1 do
		write_reg16(0x0203, i-1) -- CH2通道增益0-7
		dac = dac2[i]
		dac_write(dac) delayms(500)	
		adc = read_adc(1)
		errd = mid2[i] * diff2[i];
		if (adc < mid2[i] - errd  or adc > mid2[i] + errd) then 
			err = err + 1
			str = string.format("dac=%f adc=%f error", dac, adc)
			print(str)  delayms(5)	
		else
			str = string.format("dac=%f adc=%f ok", dac, adc)
			print(str)  delayms(5)	
		end
	end

	write_reg16(0x0200, 0) -- CH1耦合AC
	write_reg16(0x0201, 0) -- CH2耦合AC	
	write_reg16(0x0202, 0) -- CH1通道增益0
	write_reg16(0x0203, 0) -- CH2通道增益0
	delayms(2000)
	adc = read_adc(0)
	if (adc < 32768 - 400 or adc > 32768 + 400) then
		print("CH1 AC耦合", adc, "errpr") delayms(5)
		err = err + 1
	else
		print("CH1 AC耦合", adc, "ok") delayms(5)
	end
	
	adc = read_adc(1)
	if (adc < 32733 - 400 or adc > 32750 + 400) then
		print("CH2 AC耦合", adc, " errpr") delayms(5)
		err = err + 1
	else
		print("CH2 AC耦合", adc, " ok") delayms(5)
	end	
	
::quit::
--成功叫一声，失败叫三声
	if (err == 0) then
		print("*****测试通过*****")
		beep()
	else
		print("*****测试失败*****")
		beep()
		delayms(100)
		beep()
		delayms(100)
		beep()	
	end	
end

function test_tvcc(void)
	local err
	local i
	local adc

	local mid1 = {2485, 1539, 13919, 4167, 32887, 28079, 50701}
	local mid2 = {8157, 4809, 46031, 13299, 32887, 28079, 50469}
	local diff1 = {0.2, 0.2, 0.2, 0.3, 0.1, 0.1, 0.1}
	local diff2 = {0.1, 0.1, 0.1, 0.2, 0.1, 0.1, 0.1}
	local name = {"高侧电压", "高侧电流", "TVCC电压", "TVCC电流", "NTC 电阻","12V 电压","USB 电压"}	
	
	print("") delayms(5)
	print("----开始测试TVCC NTC ----") delayms(5)
	start_dso();	
	err = 0
	print("TVCC = 120") delayms(5)
	write_tvcc_dac(120)
	
	delayms(1000)
	
	for i = 1,6,1 do
		adc = read_adc(i+1)
		errd = mid1[i] * diff1[i];
		if (adc < mid1[i] - errd  or adc > mid1[i] + errd) then 
			err = err + 1
			print(name[i], adc, "error") delayms(5)
			print("  正确范围 = ", mid1[i] - errd, mid1[i] + errd) delayms(5)
		else
			print(name[i], adc, "ok") delayms(5)
		end
	end
	
	print("") delayms(5)
	print("TVCC = 36") delayms(5)
	write_tvcc_dac(36)
	delayms(1000)
	for i = 1,6,1 do
		adc = read_adc(i+1)
		errd = mid2[i] * diff2[i];
		if (adc < mid2[i] - errd  or adc > mid2[i] + errd) then 
			err = err + 1
			print(name[i], adc, "error")  delayms(5)
			print("  正确范围 = ", mid2[i] - errd, mid2[i] + errd) delayms(5)
		else
			print(name[i], adc, "ok") delayms(5)
		end
	end

::quit::
--成功叫一声，失败叫三声
	if (err == 0) then
		print("*****测试通过*****")
		beep()
	else
		print("*****测试失败*****")
		beep()
		delayms(100)
		beep()
		delayms(100)
		beep()	
	end		
end
