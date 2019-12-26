--F01=测试RS232,TestUart()
--F02=测试D0-D1,TestGPIO()
--F03=每秒蜂鸣10秒,TestBeep()

--成功叫一声，失败叫三声
function TestErr(void)
	print("测试失败")
	beep()
	delayms(100)
	beep()
	delayms(100)
	beep()	
end

function TestOk(void)
	print("测试通过")
	beep()
end

function TestBeep(void)
	local i
	local str
	
	for i = 1,10,1 do
		str = string.format("第%d次", i);
		print(str)
		beep()
		delayms(1000)
	end
end

--测试串口硬件功能
function TestUart(void)
	local COM = 1
	local Parity = 0
	local DataBits = 8
	local StopBits = 1
	local tx_str = "H7-TOOL"
	local rx_str
	local rx_len
	
	print("测试RS232串口")
	
	uart_cfg(COM, 9600, Parity, DataBits, StopBits)
	
	uart_send(COM, tx_str)
	rx_len, rx_str = uart_recive(COM, 32, 100)
	print(rx_len, rx_str)
			
	if (rx_str == tx_str) then
		TestOk()
	else
		TestErr()
	end
end

--测试GPIO功能,D0和D1短接后测试可通过
function TestGPIO(void)
	local y
	local err = 0
	
	print("")
	print("测试GPIO")
	
	gpio_cfg(0, 1)
	gpio_cfg(1, 0)

	-- D0 = 0
	gpio_write(0, 0) 
	delayms(100)	
	y = gpio_read(1) 	
	if (y ~= 0) then
		err = err + 1
	end

	-- D0 = 1
	gpio_write(0, 1) 
	delayms(100)	
	y = gpio_read(1) 	
	if (y ~= 1) then
		err = err + 1
	end	
				
	if (err == 0) then
		TestOk()
	else
		TestErr()
	end
end
