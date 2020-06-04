
-------------------------------------------------------
-- 文件名 : STM8L151_152.lua
-- 版  本 : V1.0  2020-04-28
-- 说  明 :
-------------------------------------------------------
function config_cpu(void)
	DeviceList = {
		"STM8L151F2",  4 * 1024, 256,
		"STM8L151G2",  4 * 1024, 256,
		"STM8L151K2",  4 * 1024, 256,
		"STM8L151C2",  4 * 1024, 256,

		"STM8L151F3",  8 * 1024, 256,
		"STM8L151G3",  8 * 1024, 256,
		"STM8L151K3",  8 * 1024, 256,
		"STM8L151C3",  8 * 1024, 256,

		"STM8L151G4",  16 * 1024, 1 * 1024,
		"STM8L151G4",  16 * 1024, 1 * 1024,
		"STM8L151C4",  16 * 1024, 1 * 1024,

		"STM8L151G6",  32 * 1024, 1 * 1024,
		"STM8L151G6",  32 * 1024, 1 * 1024,
		"STM8L151C6",  32 * 1024, 1 * 1024,

		"STM8L151C8",  64 * 1024, 2 * 1024,
		"STM8L151K8",  64 * 1024, 2 * 1024,
		"STM8L151R8",  64 * 1024, 2 * 1024,
		"STM8L151M8",  64 * 1024, 2 * 1024,
		"STM8L151R6",  32 * 1024, 2 * 1024,

		"STM8L152C4",  16 * 1024, 1 * 1024,
		"STM8L152C6",  32 * 1024, 1 * 1024,
		"STM8L152C8",  64 * 1024, 2 * 1024,
		"STM8L152K4",  16 * 1024, 1 * 1024,
		"STM8L152K6",  32 * 1024, 1 * 1024,
		"STM8L152K8",  64 * 1024, 2 * 1024,
		"STM8L152R8",  64 * 1024, 2 * 1024,
		"STM8L152M8",  64 * 1024, 2 * 1024,
		"STM8L152R6",  32 * 1024, 2 * 1024,

		"STM8L162R8",  64 * 1024, 2 * 1024,
		"STM8L162M6",  64 * 1024, 2 * 1024,

		"STM8L050J3",  8 * 1024,  256,
		"STM8L051F3",  8 * 1024,  256,
		"STM8L052R8",  64 * 1024, 256,
		"STM8L052C6",  32 * 1024, 256,
	}

	CHIP_TYPE = "SWIM"			--指定器件接口类型: "SWD", "SWIM", "SPI", "I2C"

	STM8_SERIAL = "STM8L"		--选择2个系列: "STM8S" 或 "STM8L"

	FLASH_ADDRESS = 0x008000	--定义FLASH起始地址

	EEPROM_ADDRESS = 0x001000 	--定义EEPROM起始地址(STM8S和STM8L不同）

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

	UID_ADDR = 0x4926			--UID地址，不同的CPU不同

	OB_ADDRESS     = "4800 4802 4807 4808 4809 480A 480B 480C"

	OB_SECURE_OFF  = "AA 00 00 00 00 00 00 00 00"	--SECURE_ENABLE = 0时，编程完毕后写入该值 (不含反码字节）
	OB_SECURE_ON   = "00 00 00 00 00 00 00 00 00"	--SECURE_ENABLE = 1时，编程完毕后写入该值

	MCU_REMOVE_PROTECT = 1		--1表示使用 MCU_RemoveProtect() 解除保护
end

-- STM8L05x/15x, medium density STM8L05x/15x and STM8AL31xx/STM8AL3Lxx and high density STM8L05x/15x/16x microcontrollers,
-- 需要操作2次写入才能解除保护
function MCU_RemoveProtect(void)
	pg_prog_buf_ob("4800", "AA")
	delayms(5)
	pg_prog_buf_ob("4800", "AA")
	delayms(5)
	pg_reset()
end

---------------------------结束-----------------------------------
