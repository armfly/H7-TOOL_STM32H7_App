/**
 * @file    SWD_flash.c
 * @brief   Program target flash through SWD
 */
#include "swd_host.h"
#include "SWD_flash.h"
#include "prog_if.h"
#include "elf_file.h"

extern const program_target_t flash_algo;


error_t target_flash_init(uint32_t flash_start)
{
//    DAP_Data.clock_delay = 5;
//    DAP_Data.fast_clock = 0;
    
    if (0 == swd_set_target_state_hw(RESET_PROGRAM)) {
        return ERROR_RESET;
    }
    
//    DAP_Data.clock_delay = 1;
//    DAP_Data.fast_clock = 1;    

    // Download flash programming algorithm to target and initialise.
    if (0 == swd_write_memory(flash_algo.algo_start, (uint8_t *)flash_algo.algo_blob, flash_algo.algo_size)) {
        return ERROR_ALGO_DL;
    }

    g_tProg.FLMFuncTimeout = 500;    /* 超时 */  
    
    if (0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.init, flash_start, 0, 0, 0)) {
        return ERROR_INIT;
    }

    return ERROR_SUCCESS;
}

error_t target_flash_uninit(void)
{
    g_tProg.FLMFuncTimeout = 200;       /* 函数执行超时 */
    
    swd_set_target_state_hw(RESET_RUN);

    swd_off();
    return ERROR_SUCCESS;
}

error_t target_flash_program_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
    /*
        FLM中的超时3秒, 如果是解除读保护，这个时间就不够，因此不要在此赋值
        g_tProg.FLMFuncTimeout = g_tFLM.Device.toProg;  // page编程超时
    */
    g_tProg.FLMFuncTimeout = 60 * 1000;
    while (size > 0) {
        uint32_t write_size = size > flash_algo.program_buffer_size ? flash_algo.program_buffer_size : size;

        // Write page to buffer
        if (!swd_write_memory(flash_algo.program_buffer, (uint8_t *)buf, write_size)) {
            return ERROR_ALGO_DATA_SEQ;
        }
    

        // Run flash programming
        if (!swd_flash_syscall_exec(&flash_algo.sys_call_s,
                                    flash_algo.program_page,
                                    addr,
                                    flash_algo.program_buffer_size,
                                    flash_algo.program_buffer,
                                    0)) {
            return ERROR_WRITE;
        }
        
		addr += write_size;
		buf  += write_size;
		size -= write_size;
    }

    return ERROR_SUCCESS;
}

/* 校验函数 */
error_t target_flash_verify_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{    
    g_tProg.FLMFuncTimeout = 3000;
    while (size > 0) 
    {
        uint32_t write_size = size > flash_algo.program_buffer_size ? flash_algo.program_buffer_size : size;

        // Write page to buffer
        if (!swd_write_memory(flash_algo.program_buffer, (uint8_t *)buf, write_size)) {
            return ERROR_ALGO_DATA_SEQ;
        }
    
        // Run verify programming
        if (swd_flash_syscall_exec_ex(&flash_algo.sys_call_s,
                                    flash_algo.verify,
                                    addr,
                                    flash_algo.program_buffer_size,
                                    flash_algo.program_buffer,
                                    0) != addr + size) 
        {
            return ERROR_WRITE;
        }
        
		addr += write_size;
		buf  += write_size;
		size -= write_size;
    }

    return ERROR_SUCCESS;
}

/* 查空函数, 1表示不空需要擦除 */
uint8_t target_flash_check_blank(uint32_t addr, uint32_t size)
{    
        // Run verify programming
    if (1 == swd_flash_syscall_exec(&flash_algo.sys_call_s,
                                flash_algo.check_blank,
                                addr,
                                size,
                                0xFF,
                                0)) {
        return 1;
    }

    return 0;
}

/* 计算flash crc32 */
uint32_t target_flash_cacul_crc32(uint32_t addr, uint32_t size, uint32_t ini_value)
{    
    uint32_t crc;
    
        // Run verify programming
    crc = swd_flash_syscall_exec_ex(&flash_algo.sys_call_s,
                                flash_algo.cacul_crc32,
                                addr,
                                size,
                                ini_value,
                                0);
    return crc;
}

error_t target_flash_erase_sector(uint32_t addr)
{   
    g_tProg.FLMFuncTimeout = 60 * 1000;
    
    if (0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.erase_sector, addr, 0, 0, 0)) {
        return ERROR_ERASE_SECTOR;
    }

    return ERROR_SUCCESS;
}

error_t target_flash_erase_chip(void)
{
    error_t status = ERROR_SUCCESS;  
    
    g_tProg.FLMFuncTimeout = 60 * 1000;
    
    if (0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.erase_chip, 0, 0, 0, 0)) 
    {
        return ERROR_ERASE_ALL;
    }

    return status;
}
