
-------------------------------------------------------
-- 文件名 : STM8S103_003.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	DeviceList = {
		"STM8S103F2",  4 * 1024, 640,
		"STM8S103F3",  8 * 1024, 640,
		"STM8S103K3",  8 * 1024, 640,

		"STM8S003K3",  8 * 1024, 128,
		"STM8S003F3",  8 * 1024, 128,

		"STM8S903F3",  8 * 1024, 640,
		"STM8S903K3",  8 * 1024, 640,

		"STM8S001J3",  8 * 1024, 128,
	}

	CHIP_TYPE = "SWIM"			--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C"

	STM8_SERIAL = "STM8S"		--选择2个系列: "STM8S" 或 "STM8L"

	FLASH_ADDRESS  = 0x008000	--定义FLASH起始地址

	EEPROM_ADDRESS = 0x004000 	--定义FLASH起始地址(STM8S和STM8L不同）

	for i = 1, #DeviceList, 3 do
		if (CHIP_NAME == DeviceList[i]) then
			FLASH_SIZE  = DeviceList[i + 1]	--FLASH总容量
			EEPROM_SIZE = DeviceList[i + 2]	--EEPROM容量

			--定义BLOCK SIZE, 只有64和128两种
			if (FLASH_SIZE <= 8 * 1024) then
				FLASH_BLOCK_SIZE = 64
			else
				FLASH_BLOCK_SIZE = 128
			end
			break
		end
	end

	UID_ADDR = 0x4865			--UID地址 103 903 = 0x4865， 001 003未查到

	OB_ADDRESS     = "4800 4801 FFFF 4803 FFFF 4805 FFFF 4807 FFFF 4809 FFFF "

	OB_SECURE_OFF  = "00 00 00 00 00 00"	--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
	OB_SECURE_ON   = "AA 00 00 00 00 00"	--SECURE_ENABLE = 1时，编程完毕后写入该值
end

---------------------------结束-----------------------------------
