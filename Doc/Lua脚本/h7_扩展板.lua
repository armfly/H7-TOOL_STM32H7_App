beep()
--测试转接板，循环点灯
function test_ledout(void)
	local i
	local err
	local fmc
	local flag
	
	print("")
	print("----开始测试转接板(输出+FMC总线输入)----")

--设置TVCC输出
	write_tvcc_dac(47)

--设置D7-D0为输出
	gpio_cfg(0, 1)
	gpio_cfg(1, 1)
	gpio_cfg(2, 1)
	gpio_cfg(3, 1)
	gpio_cfg(4, 1)
	gpio_cfg(5, 1)
	gpio_cfg(6, 1)
	gpio_cfg(7, 1)
--全灭
	for i=0,7,1 do
		gpio_write(i, 0)		
	end
--循环点亮
	flag = 1
	err = 0
	for i=0,7,1 do
		gpio_write(i, 1) 	
		delayms(100)	
		fmc = read_bus() % 256
		printhex(fmc, 1)
		if (fmc ~= (flag)) then
			err = err + 1
		end	
		flag = flag * 2
		gpio_write(i, 0) 	
		delayms(100)
	end	
--全部点亮
	for i=0,7,1 do
		gpio_write(i, 1)	
	end

--成功叫一声，失败叫三声
	if (err == 0) then
		print("测试通过")
		beep()
	else
		print("测试失败")
		beep()
		delayms(100)
		beep()
		delayms(100)
		beep()	
	end
end

--测试扩展板继电器
function test_extio_open_do(void)
	local i
	
	print("依次打开24个继电器 - 开始")
	beep()
	extio_start()
	for i=0,23,1 do	
		print(i)
		extio_set_do(i, 1)
		delayms(500)
	end
	print("依次打开24个继电器 - 结束")
end

--测试扩展板继电器
function test_extio_close_do(void)
	local i
	
	print("依次关闭24个继电器 - 开始")
	beep()
	extio_start()
	for i=0,23,1 do	
		print(i)
		extio_set_do(i, 1)
		delayms(500)
	end
	print("依次关闭24个继电器 - 结束")
end

--测试扩展板DI
function test_extio_di(void)
	local i
	
	print("测试扩展板DI")
	beep()
	for i=0,15,1 do	
		print(extio_get_di(i))
	end
end

--测试扩展板ADC
function test_extio_adc(void)
	local i
	
	print("测试扩展板ADC")
	beep()
	for i=0,7,1 do	
		print(extio_get_adc(i))
	end
end

--测试转接板，循环点灯
function test_swd(void)
	local err
	local id
	local str
	
	print("")
	print("----开始测SWD功能----")
	err = 0
	swd_init(3.3)     --配置SWD，3.3V电压
	id = swd_getid()  --读ID
	printhex(id,4)	
	if (id ~= 0x0BB11477) then 
		err = err + 1
	end
	
	swd_write(0x20000000, "12345678")
	str = swd_read(0x20000000, 8)
	print(str)
	if (str ~= "12345678") then
		err = err + 1
	end
	
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
