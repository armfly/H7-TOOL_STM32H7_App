
-------------------------------------------------------
-- 文件名 : STM8AF52_F62.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	DeviceList = {
		"STM8AF52AA",  128 * 1024, 2 * 1024,
		"STM8AF528A",  64 * 1024, 2 * 1024,

		"STM8AF52A9",  128 * 1024,  2 * 1024,
		"STM8AF5289",  64 * 1024,   2 * 1024,
		"STM8AF5269",  32 * 1024,   1 * 1024,

		"STM8AF52A8",  128 * 1024, 2 * 1024,
		"STM8AF5288",  64 * 1024, 2 * 1024,
		"STM8AF5268",  32 * 1024, 1 * 1024,

		"STM8AF5286",  64 * 1024, 2 * 1024,
		"STM8AF52A6",  128 * 1024, 2 * 1024,

		"STM8AF62AA",  128 * 1024, 2 * 1024,
		"STM8AF628A",  64 * 1024, 2 * 1024,

		"STM8AF62A9",  128 * 1024, 2 * 1024,
		"STM8AF6289",  64 * 1024, 2 * 1024,
		"STM8AF6269",  32 * 1024, 1 * 1024,

		"STM8AF62A8",  128 * 1024, 2 * 1024,
		"STM8AF6288",  64 * 1024, 2 * 1024,

		"STM8AF6286",  64 * 1024, 2 * 1024,
		"STM8AF62A6",  128 * 1024, 2 * 1024,


		"STM8AF6268",  32 * 1024, 1 * 1024,
		"STM8AF6248",  16 * 1024, 512,
		"STM8AF6266",  32 * 1024, 1 * 1024,
		"STM8AF6246",  16 * 1024, 512,

		"STM8AF6368",  32 * 1024, 1 * 1024,
		"STM8AF6348",  16 * 1024, 512,
		"STM8AF6366",  32 * 1024, 1 * 1024,
		"STM8AF6346",  16 * 1024, 512,

		"STM8AF63AA",  128 * 1024, 2 * 1024,
		"STM8AF638A",  64 * 1024, 2 * 1024,

		"STM8AF63A9",  128 * 1024,  2 * 1024,
		"STM8AF6389",  64 * 1024,   2 * 1024,
		"STM8AF6369",  32 * 1024,   1 * 1024,

		"STM8AF63A8",  128 * 1024, 2 * 1024,
		"STM8AF6388",  64 * 1024, 2 * 1024,
		"STM8AF6386",  64 * 1024, 2 * 1024,
		"STM8AF63A6",  128 * 1024, 1 * 1024,
	}

	CHIP_TYPE = "SWIM"			--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C"

	STM8_SERIAL = "STM8S"		--选择2个系列: "STM8S" 或 "STM8L"

	FLASH_BLOCK_SIZE = 128		--定义BLOCK SIZE, 只有64和128两种

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
		end
	end

	UID_ADDR = 0x4865			--UID地址 STM8AF52 F62未查到

	OB_ADDRESS      = "4800 4801 FFFF 4803 FFFF 4805 FFFF 4807 FFFF 4809 FFFF 480B FFFF 480D FFFF"
					.."4810 4811 4812 4813 4814 4815 4816 4817 4818 487E FFFF"
	OB_SECURE_OFF  =  "00 00 00 00 00 00 00 00"	--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
					.."00 00 00 00 00 00 00 00 C7 00"
	OB_SECURE_ON   =  "AA 00 00 00 00 00 00 00"		--SECURE_ENABLE = 1时，编程完毕后写入该值
					.."00 00 00 00 00 00 00 00 C7 00"
end

---------------------------结束-----------------------------------
