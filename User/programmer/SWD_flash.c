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

    /*
        FLM算法函数 Iint Init (unsigned long adr, unsigned long clk, unsigned long fnc) 
        
        Initialize Flash Programming Functions
        Parameter:      adr:  Device Base Address
        clk:  Clock Frequency (Hz)
        fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
        Return Value:   0 - OK,  1 - Failed
    */    
    
    g_tProg.FLMFuncTimeout = 500;    /* 超时 */  
    
    LoadAlgoToTarget();
    
    if (0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.init, flash_start, 0, 1, 0)) {
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
    if ( flash_algo.program_page == 0)
    {
        return ERROR_SUCCESS;
    }
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
    if ( flash_algo.verify == 0)
    {
        return ERROR_SUCCESS;
    }
    
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
    if ( flash_algo.check_blank == 0)
    {
        return ERROR_SUCCESS;
    }
    
    // Run verify programming
    if (1 == swd_flash_syscall_exec(&flash_algo.sys_call_s,
                                flash_algo.check_blank,
                                addr,
                                size,
                                g_tFLM.Device.valEmpty,     /* 空值，多半为0xFF,  STM32L051为0x00 */
                                0)) {
        return 1;
    }

    return 0;
}

/* 计算flash crc32 */
uint32_t target_flash_cacul_crc32(uint32_t addr, uint32_t size, uint32_t ini_value)
{    
    uint32_t crc;
   
    if ( flash_algo.cacul_crc32 == 0)
    {
        return 0;
    }       
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
    if ( flash_algo.erase_sector == 0)
    {
        return ERROR_SUCCESS;
    } 
    
    g_tProg.FLMFuncTimeout = 60 * 1000;
    
    if (0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.erase_sector, addr, 0, 0, 0)) {
        return ERROR_ERASE_SECTOR;
    }

    return ERROR_SUCCESS;
}

error_t target_flash_erase_chip(void)
{   
    error_t status = ERROR_SUCCESS;  

    if ( flash_algo.erase_chip == 0)
    {
        return ERROR_SUCCESS;
    }     
    
    g_tProg.FLMFuncTimeout = 60 * 1000;
    
    if (0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.erase_chip, 0, 0, 0, 0)) 
    {
        return ERROR_ERASE_ALL;
    }

    return status;
}

/* 读取外部SPI FLASH的id */
uint32_t target_flash_read_extid(uint32_t addr)
{    
    uint32_t id;
//   
//    if ( flash_algo.read_extid == 0)
//    {
//        return 0;
//    }       

//    id = swd_flash_syscall_exec_ex(&flash_algo.sys_call_s,
//                                flash_algo.read_extid,
//                                addr,
//                                0,
//                                0,
//                                0);

    g_tProg.FLMFuncTimeout = 500;    /* 超时 */  
    
//    if (0 == swd_flash_syscall_exec(&flash_algo.sys_call_s, flash_algo.init, flash_start, 0, 0, 0)) {
//        return ERROR_INIT;
//    }
    id = swd_flash_syscall_exec_ex(&flash_algo.sys_call_s,
                                flash_algo.read_extid,
                                addr,
                                0,
                                0,
                                0);
    return id;    
    return id;
}
