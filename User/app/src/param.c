/*
*********************************************************************************************************
*
*	模块名称 : 应用程序参数模块
*	文件名称 : param.c
*	版    本 : V1.0
*	说    明 : 读取和保存应用程序的参数
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-01-01 armfly  正式发布
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "param.h"
#include "modbus_reg_addr.h"

PARAM_T g_tParam; /* 基本参数 */
CALIB_T g_tCalib; /* 校准参数 */
VAR_T g_tVar;     /* 全局变量 */

void LoadCalibParam(void);

/*
*********************************************************************************************************
*	函 数 名: LoadParam
*	功能说明: 从eeprom读参数到g_tParam
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LoadParam(void)
{

  /* 读取EEPROM中的参数 */
  ee_ReadBytes((uint8_t *)&g_tParam, PARAM_ADDR, sizeof(PARAM_T));

  if (sizeof(PARAM_T) > PARAM_SIZE)
  {
    /* 基本参数分配空间不足 */
    while (1)
      ;
  }

  if (g_tParam.ParamVer != PARAM_VER)
  {
    InitBaseParam();
  }

  bsp_GetCpuID(g_tVar.CPU_Sn); /* 读取CPU ID */

  /* 自动生成以太网MAC */
  g_tVar.MACaddr[0] = 0xC8;
  g_tVar.MACaddr[1] = 0xF4;
  g_tVar.MACaddr[2] = 0x8D;
  g_tVar.MACaddr[3] = g_tVar.CPU_Sn[0] >> 16;
  g_tVar.MACaddr[4] = g_tVar.CPU_Sn[0] >> 8;
  g_tVar.MACaddr[5] = g_tVar.CPU_Sn[0] >> 0;

  LoadCalibParam();
}

/*
*********************************************************************************************************
*	函 数 名: SaveParam
*	功能说明: 将全局变量g_tParam 写入到eeprom
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void SaveParam(void)
{
  /* 将全局的参数变量保存到EEPROM */
  ee_WriteBytes((uint8_t *)&g_tParam, PARAM_ADDR, sizeof(PARAM_T));
}

/*
*********************************************************************************************************
*	函 数 名: InitBaseParam
*	功能说明: 初始化基本参数为缺省值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitBaseParam(void)
{
  g_tParam.UpgradeFlag = 0;      /*升级标记,0x55AAA55A表示需要更新APP，0xFFFF表示更新完毕*/
  g_tParam.ParamVer = PARAM_VER; /* 参数区版本控制（可用于程序升级时，决定是否对参数区进行升级） */

  g_tParam.DispDir = 3; /* 显示方向 */

  g_tParam.Addr485 = 1;

  g_tParam.LocalIPAddr[0] = 192; /* 本机IP地址 */
  g_tParam.LocalIPAddr[1] = 168;
  g_tParam.LocalIPAddr[2] = 1;
  g_tParam.LocalIPAddr[3] = 211;

  g_tParam.NetMask[0] = 255; /* 子网掩码 */
  g_tParam.NetMask[1] = 255;
  g_tParam.NetMask[2] = 255;
  g_tParam.NetMask[3] = 0;

  g_tParam.Gateway[0] = 192; /* 网关 */
  g_tParam.Gateway[1] = 168;
  g_tParam.Gateway[2] = 1;
  g_tParam.Gateway[3] = 1;

  g_tParam.LocalTCPPort = 30010; /* 本机TCP端口和UDP端口号，相同 */

  g_tParam.RemoteIPAddr[0] = 192; /* 远端(前置）IP地址 */
  g_tParam.RemoteIPAddr[1] = 168;
  g_tParam.RemoteIPAddr[2] = 1;
  g_tParam.RemoteIPAddr[3] = 213;

  g_tParam.RemoteTcpPort = 30000; /* 远端（前置）TCP端口 */

  g_tParam.WorkMode = 0; /* 工作模式 保留 */

  g_tParam.APSelfEn = 0;               /* 0作为客户端，1作为AP */
  memset(g_tParam.AP_SSID, 0, 32 + 1); /* AP名字 */
  memset(g_tParam.AP_PASS, 0, 16 + 1); /* AP密码 */
  g_tParam.WiFiIPAddr[0] = 192;        /* 静态IP地址  */
  g_tParam.WiFiIPAddr[1] = 168;
  g_tParam.WiFiIPAddr[2] = 1;
  g_tParam.WiFiIPAddr[3] = 105;

  g_tParam.WiFiNetMask[0] = 255; /* 子网掩码 255.255.255.0 */
  g_tParam.WiFiNetMask[1] = 255;
  g_tParam.WiFiNetMask[2] = 255;
  g_tParam.WiFiNetMask[3] = 0;

  g_tParam.WiFiGateway[0] = 192; /* 网关 192.168.1.1 */
  g_tParam.WiFiGateway[1] = 168;
  g_tParam.WiFiGateway[2] = 1;
  g_tParam.WiFiGateway[3] = 1;

  g_tParam.DHCPEn = 0; /* DHCP使能  */

  g_tParam.TestWord = 0; /*　测试单元，用于检测eepromg功能 */
  g_tParam.NtcType = 0;  /* NTC热敏电阻类型 0 = 10K_B3950，1 = 100K_B3950 */

  SaveParam();
}

/*
*********************************************************************************************************
*	函 数 名: LoadCalibParam
*	功能说明: 将全局变量g_tParam 写入到CPU内部Flash
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LoadCalibParam(void)
{
  if (sizeof(g_tCalib) > PARAM_CALIB_SIZE)
  {
    /* 校准参数分配空间不足 */
    while (1)
      ;
  }

  /* 读取EEPROM中的参数 */
  ee_ReadBytes((uint8_t *)&g_tCalib, PARAM_CALIB_ADDR, sizeof(g_tCalib));

  if (g_tCalib.CalibVer != CALIB_VER)
  {
    g_tCalib.CalibVer = CALIB_VER;

    InitCalibParam(); /* 初始化校准参数 */
  }
}

/*
*********************************************************************************************************
*	函 数 名: WriteParamUint16
*	功能说明: 写参数，16bit
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void WriteParamUint16(uint16_t _addr, uint16_t _value)
{
  ee_WriteBytes((uint8_t *)&_value, PARAM_CALIB_ADDR + _addr, 2);
}

/*
*********************************************************************************************************
*	函 数 名: SaveCalibParam
*	功能说明: 将全局变量g_tCalib写入到eeprom
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void SaveCalibParam(void)
{
  /* 将全局的参数变量保存到EEPROM */
  ee_WriteBytes((uint8_t *)&g_tCalib, PARAM_CALIB_ADDR, sizeof(g_tCalib));
}

/*
*********************************************************************************************************
*	函 数 名: InitCalibParam
*	功能说明: 初始化校准参数为缺省值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitCalibParam(void)
{
  uint8_t i;

  g_tCalib.CalibVer = CALIB_VER;
  for (i = 0; i < 8; i++)
  {
    g_tCalib.CH1[i].x1 = 0;
    g_tCalib.CH1[i].y1 = -14.3678 / (1 << i); /* -14.3678V */
    g_tCalib.CH1[i].x2 = 65535;
    g_tCalib.CH1[i].y2 = 14.3678 / (1 << i); /* +14.3678V */

    g_tCalib.CH2[i].x1 = 0;
    g_tCalib.CH2[i].y1 = -14.3678 / (1 << i); /* -14.3678V */
    g_tCalib.CH2[i].x2 = 65535;
    g_tCalib.CH2[i].y2 = 14.3678 / (1 << i); /* +14.3678V */
  }

  g_tCalib.LoadVolt.x1 = 0;
  g_tCalib.LoadVolt.y1 = 0;
  g_tCalib.LoadVolt.x2 = 9246.938;
  g_tCalib.LoadVolt.y2 = 4.943; /*  */

  g_tCalib.LoadCurr[0].x1 = 1321.703; /* 负载电流 小量程 */
  g_tCalib.LoadCurr[0].y1 = 0;
  g_tCalib.LoadCurr[0].x2 = 30302.359;
  g_tCalib.LoadCurr[0].y2 = 55.24; /*   */
  g_tCalib.LoadCurr[0].x3 = 36213.906;
  g_tCalib.LoadCurr[0].y3 = 66.15; /*   */
  g_tCalib.LoadCurr[0].x4 = 44583.625;
  g_tCalib.LoadCurr[0].y4 = 81.58; /* 65535 = 123.934mA */

  g_tCalib.LoadCurr[1].x1 = 187.047; /* 负载电流 大量程 */
  g_tCalib.LoadCurr[1].y1 = 0;
  g_tCalib.LoadCurr[1].x2 = 2936.438;
  g_tCalib.LoadCurr[1].y2 = 52.60; /*  */
  g_tCalib.LoadCurr[1].x3 = 7807.172;
  g_tCalib.LoadCurr[1].y3 = 145.2; /*  */
  g_tCalib.LoadCurr[1].x4 = 11930.469;
  g_tCalib.LoadCurr[1].y4 = 221.8; /*  */

  g_tCalib.TVCCVolt.x1 = 0;
  g_tCalib.TVCCVolt.y1 = 0;
  g_tCalib.TVCCVolt.x2 = 65535;
  g_tCalib.TVCCVolt.y2 = 6.25; /* 最高测量6.25V */

  g_tCalib.TVCCCurr.x1 = 50;
  g_tCalib.TVCCCurr.y1 = 0;
  g_tCalib.TVCCCurr.x2 = 8423.875;
  g_tCalib.TVCCCurr.y2 = 55.36; /*   */
  g_tCalib.TVCCCurr.x3 = 12319.656;
  g_tCalib.TVCCCurr.y3 = 81.2; /*   */
  g_tCalib.TVCCCurr.x4 = 18746.266;
  g_tCalib.TVCCCurr.y4 = 124; /*  */

  //g_tCalib.RefResistor = 5.1;
  g_tCalib.NtcRes.x1 = 0;
  g_tCalib.NtcRes.y1 = 100;
  g_tCalib.NtcRes.x2 = 20;
  g_tCalib.NtcRes.y2 = 300;
  g_tCalib.NtcRes.x3 = 43452.312;
  g_tCalib.NtcRes.y3 = 9.979;
  g_tCalib.NtcRes.x4 = 622207.797;
  g_tCalib.NtcRes.y4 = 99.94;

  g_tCalib.TVCCSet.x1 = 127;
  g_tCalib.TVCCSet.y1 = 1265;
  g_tCalib.TVCCSet.x2 = 34;
  g_tCalib.TVCCSet.y2 = 4687;

  g_tCalib.Dac10V.x1 = 500;
  g_tCalib.Dac10V.y1 = -9302; /*  */
  g_tCalib.Dac10V.x2 = 1500;
  g_tCalib.Dac10V.y2 = -3278; /*  */
  g_tCalib.Dac10V.x3 = 2500;
  g_tCalib.Dac10V.y3 = 2753; /*  */
  g_tCalib.Dac10V.x4 = 3500;
  g_tCalib.Dac10V.y4 = 8783; /*  */

  g_tCalib.Dac20mA.x1 = 500;
  g_tCalib.Dac20mA.y1 = 2504;
  g_tCalib.Dac20mA.x2 = 1500;
  g_tCalib.Dac20mA.y2 = 7682; /*  */
  g_tCalib.Dac20mA.x3 = 2500;
  g_tCalib.Dac20mA.y3 = 12867; /*  */
  g_tCalib.Dac20mA.x4 = 3500;
  g_tCalib.Dac20mA.y4 = 18050; /*  */

  /* 下面2个电压暂时未用到 */
  g_tCalib.USBVolt.x1 = 0;
  g_tCalib.USBVolt.y1 = 0;
  g_tCalib.USBVolt.x2 = 65535;
  g_tCalib.USBVolt.y2 = 6.25; /* 最高测量6.25V */

  g_tCalib.ExtPowerVolt.x1 = 0;
  g_tCalib.ExtPowerVolt.y1 = 0;
  g_tCalib.ExtPowerVolt.x2 = 65535;
  g_tCalib.ExtPowerVolt.y2 = 6.25; /* 最高测量6.25V */

  SaveCalibParam();
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
