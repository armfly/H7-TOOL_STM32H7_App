#ifndef __SWD_FLASH_H__
#define __SWD_FLASH_H__

#include <stdint.h>

#include "error.h"

error_t target_flash_enter_debug_program(void);

error_t target_flash_init(uint32_t flash_start);
error_t target_flash_uninit(void);
error_t target_flash_program_page(uint32_t addr, const uint8_t *buf, uint32_t size);
error_t target_flash_verify_page(uint32_t addr, const uint8_t *buf, uint32_t size);
error_t target_flash_erase_sector(uint32_t addr);
error_t target_flash_erase_chip(void);
uint8_t target_flash_check_blank(uint32_t addr, uint32_t size);
uint32_t target_flash_cacul_crc32(uint32_t addr, uint32_t size, uint32_t ini_value);
uint8_t target_flash_check_blank_ex(uint32_t addr, uint32_t size);
uint32_t target_flash_cacul_crc32_ex(uint32_t addr, uint32_t size, uint32_t ini_value);

uint32_t target_flash_read_extid(uint32_t addr);

uint8_t LoadCheckBlankAlgoToTarget(void);
uint8_t LoadCheckCRCAlgoToTarget(void);

#endif // __SWD_FLASH_H__
