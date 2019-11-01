/*
*********************************************************************************************************
*
*    模块名称 : 编程器接口文件
*    文件名称 : prog_if.c
*    版    本 : V1.0
*    说    明 : 
*
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2019-03-19  armfly  正式发布
*
*    Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "param.h"
#include "modbus_slave.h"
#include "prog_if.h"

#define PG_INST_MAX_LEN 32 * 1024
static uint16_t s_prog_buf[PG_INST_MAX_LEN]; /* 脚本程序数据 */
static uint32_t s_prog_len;                                     /* 程序长度 */
static uint32_t s_prog_pc = 0;                             /* 程序指针 */
static uint8_t s_prog_param_len = 0;                 /* 指令参数长度 */
static uint8_t s_prog_state = 0;                         /* 指令执行状态，0表示取cmd，1表示取参数，2表示等待执行 */
static uint8_t s_prog_cmd = 0;
uint8_t s_prog_ack_buf[2 * 1024];
uint16_t s_prog_ack_len;

static uint8_t s_prog_run_flag = 0;

void PG_AnalyzeInst(void);
uint8_t PG_AnalyzeI2C(uint16_t _inst);

/*
*********************************************************************************************************
*    函 数 名: PG_ReadInst
*    功能说明: 读取一条指令或数据
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
uint16_t PG_ReadInst(void)
{
    uint16_t inst;

    if (s_prog_pc < PG_INST_MAX_LEN)
    {
        inst = s_prog_buf[s_prog_pc];
        s_prog_pc++;

        if (s_prog_pc >= s_prog_len)
        {
            s_prog_run_flag = 2;
        }
    }
    else
    {
        inst = 0;
    }

    return inst;
}

/*
*********************************************************************************************************
*    函 数 名: PG_Run
*    功能说明: 运行指令
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void PG_Poll(void)
{
    if (s_prog_run_flag == 1)
    {
        PG_AnalyzeInst();
    }
}

/*
*********************************************************************************************************
*    函 数 名: PG_Stop
*    功能说明: 停止程序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void PG_Stop(void)
{
    s_prog_run_flag = 0;
    s_prog_pc = 0;
}

/*
*********************************************************************************************************
*    函 数 名: PG_WaitRunCompleted
*    功能说明: 执行小程序，并等待指令结束
*    形    参: _usTimeout,等待时间ms，最大65秒.
*    返 回 值: 0 表示超时，1表示OK
*********************************************************************************************************
*/
uint8_t PG_WaitRunCompleted(uint16_t _usTimeout)
{
    int32_t time1;
    uint8_t re;

    s_prog_run_flag = 1;
    s_prog_pc = 0;
    s_prog_ack_len = 0;

    time1 = bsp_GetRunTime();
    while (1)
    {
        bsp_Idle();

        if (s_prog_run_flag == 2)
        {
            re = 1;
            break;
        }

        if (bsp_CheckRunTime(time1) > _usTimeout)
        {
            re = 0;
            break;
        }
    }
    return re;
}
/*
*********************************************************************************************************
*    函 数 名: PG_Install
*    功能说明: 安装程序
*    形    参: 
*        _addr : 偏移地址
*        _buf :  缓冲区
*        _len :  程序长度
*    返 回 值: 无
*********************************************************************************************************
*/
void PG_Install(uint16_t _addr, uint8_t *_buf, uint16_t _len, uint16_t _total_len)
{
    uint32_t i;

    for (i = 0; i < _len; i++)
    {
        if (_addr < PG_INST_MAX_LEN)
        {
            s_prog_buf[_addr++] = (_buf[2 * i] << 8) + _buf[2 * i + 1];
        }
    }

    s_prog_len = _total_len;
}

void PG_AnalyzeInst(void)
{
    uint16_t inst;

    if (s_prog_state == 0)
    {
        inst = PG_ReadInst();
        s_prog_cmd = inst;
    }
    else if (s_prog_state == 1)
    {
        inst = PG_ReadInst();
    }
    else if (s_prog_state == 2)
    {
        inst = 0;
    }

    switch (s_prog_cmd >> 8)
    {
    case DEV_SYS:
        break;

    case DEV_GPIO:
        break;

    case DEV_TIM:
        break;

    case DEV_DAC:
        break;

    case DEV_ADC:
        break;

    case DEV_I2C:
        PG_AnalyzeI2C(inst);
        break;

    case DEV_SPI:
        break;

    case DEV_UART:
        break;

    case DEV_485:
        break;

    case DEV_CAN:
        break;

    case DEV_SWD:
        break;
    }
}

uint8_t PG_AnalyzeI2C(uint16_t _inst)
{
    static uint16_t s_len = 0;
    static uint16_t s_cmd = 0;

    if (s_prog_state == 0) /* 取指令 */
    {
        switch (_inst)
        {
        case I2C_START:
            i2c_Start();
            break;

        case I2C_STOP:
            i2c_Stop();
            break;

        case I2C_SEND_BYTE:
            s_prog_state = 1;
            s_prog_param_len = 1;
            s_len = 0;
            break;

        case I2C_SEND_BYTES:
            s_prog_state = 1;
            s_prog_param_len = PG_ReadInst(); /* 取长度字段 */
            s_len = 0;
            break;

        case I2C_READ_BYTES:
            s_prog_state = 1;
            s_prog_ack_len = PG_ReadInst(); /* 取长度字段 */
            s_len = 0;
            break;
        }
        s_cmd = _inst;
    }
    else if (s_prog_state == 1) /* 执行指令 */
    {
        switch (s_cmd)
        {
        case I2C_SEND_BYTE:
            i2c_SendByte(_inst >> 8);
            i2c_WaitAck();
            s_prog_state = 2; /* 执行完毕 */
            break;

        case I2C_SEND_BYTES: /* 此状态会进入 s_prog_param_len 次数 */
            if (s_len < s_prog_param_len)
            {
                i2c_SendByte(_inst >> 8);
                i2c_WaitAck();
                s_len++;
            }
            else /* 执行完毕 */
            {
                s_prog_state = 2;
            }
            break;

        case I2C_READ_BYTES:
            if (s_len <= s_prog_ack_len)
            {
                s_prog_ack_buf[s_len] = i2c_ReadByte(); /* 读1个字节 */
                s_len++;
                if (s_len == s_prog_ack_len)
                {
                    i2c_Ack(); /* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
                }
                else
                {
                    i2c_Ack(); /* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
                }
            }
            else
            {
                s_prog_state = 2; /* 异常分之 */
            }
            break;
        }
    }
    else if (s_prog_state == 2)
    {
        s_prog_state = 0;
        //        switch (s_cmd)
        //        {
        //            case I2C_SEND_BYTE:
        //                /* 等待数据传输完毕 */
        //                s_prog_state = 0;
        //                break;
        //
        //            case I2C_SEND_BYTES:
        //                /* 等待数据传输完毕 */
        //                s_prog_state = 0;
        //                break;
        //        }
    }

    return 0;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
