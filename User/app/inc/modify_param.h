/*
*********************************************************************************************************
*
*    模块名称 : 修改参数组件
*    文件名称 : modify_param.h
*    版    本 : V1.0
*    说    明 : 头文件
*
*    Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _MODIFY_PARAM_H
#define _MODIFY_PARAM_H

/* 参数列表结构 */
typedef struct
{    
    uint8_t DataType;       /* 参数类型 0 = uint16_t */    
    const char *ParamName;        /* 参数名 */    
    const char **ParamItems;       /* 可选的参数项 */  
    int32_t MinValue;       /* 最小值 */
    int32_t MaxValue;       /* 最大值 */
    int32_t DefaultValue;   /* 缺省值 */ 
}PARAM_LIST_T;

void ModifyParam(uint16_t _MainStatus);
void UartMonCheckParam(void);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
