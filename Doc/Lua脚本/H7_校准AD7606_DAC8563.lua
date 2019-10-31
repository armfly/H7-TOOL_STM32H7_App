
beep()
print("H7_校准AD7606_DAC8563.lua")
write_tvcc_dac(47)	--3.3V接口电平
delayms(500)
extio_start()
print("扩展IO板已启动")

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
  local X1 = {75,		75,		74,		72,		73,		74,		73,		71}
  local Y1 = {0,		0,		0,		0,		0,		0,		0,		0}
  local X2 = {29417,	29362,	29520,	29396,	29407,	29407,	29407,	29407}
  local Y2 = {8.999,	8.999,	8.999,	8.999,	8.999,	8.999,	8.999,	8.999}
  local adc
  local volt
  
  adc = extio_get_adc(ch)
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
  
  adc = extio_get_adc(ch)
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
  extio_set_dac(0, dac)
end
