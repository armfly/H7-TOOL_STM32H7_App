
-------------------------------------------------------
-- 文件名 : STM8S207_208.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	DeviceList = {
		"STM8S207R6",  32 * 1024, 1024,
		"STM8S207C6",  32 * 1024, 1024,
		"STM8S207S6",  32 * 1024, 1024,
		"STM8S207K6",  32 * 1024, 1024,
		"STM8S207M8",  64 * 1024, 2048,
		"STM8S207K8",  64 * 1024, 1024,
		"STM8S207C8",  64 * 1024, 1536,
		"STM8S207S8",  64 * 1024, 1536,
		"STM8S207R8",  64 * 1024, 1536,
		"STM8S207SB", 128 * 1024, 1536,
		"STM8S207MB", 128 * 1024, 2048,
		"STM8S207RB", 128 * 1024, 2048,
		"STM8S207CB", 128 * 1024, 2048,

		"STM8S208S6",  32 * 1024, 1536,
		"STM8S208R6",  32 * 1024, 2048,
		"STM8S208C6",  32 * 1024, 2048,
		"STM8S208R8",  64 * 1024, 2048,
		"STM8S208C8",  64 * 1024, 2048,
		"STM8S208S8",  64 * 1024, 1536,
		"STM8S208SB", 128 * 1024, 1536,
		"STM8S208MB", 128 * 1024, 2048,
		"STM8S208RB", 128 * 1024, 2048,
		"STM8S208CB", 128 * 1024, 2048,
	}

	CHIP_TYPE = "SWIM"			--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C"

	STM8_SERIAL = "STM8S"		--选择2个系列: "STM8S" 或 "STM8L"

	FLASH_ADDRESS = 0x008000	--定义FLASH起始地址

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

	UID_ADDR = 0x48CD			--UID地址，不同的CPU不同

	OB_ADDRESS     = "4800 4801 FFFF 4803 FFFF 4805 FFFF 4807 FFFF 4809 FFFF 480B FFFF 480D FFFF 487E FFFF"

	OB_SECURE_OFF  = "00 00 00 00 00 00 00 00 00"	--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
	OB_SECURE_ON   = "AA 00 00 00 00 00 00 00 00"	--SECURE_ENABLE = 1时，编程完毕后写入该值
end

---------------------------结束-----------------------------------
