/*!**************************************************************************************
 * @file        Det.h
 * @brief       Det头文件
 * @details     Det模块对外提供的所有接口及类型定义
 * @date        2023-3-17
 * @version     V1.0
 * @copyright   Copyright (C) 2022 理想汽车股份有限公司版权所有.
 ****************************************************************************************/

#ifndef DET_H
#define DET_H

/* Including File */
#include "Std_Types.h"
#include "det_cfg.h"
#include "Det_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Global Macro Definition */

#define DET_MODULE_ID            (15u)        ///< 模块ID
#define DET_VENDOR_ID            (60u)        ///< 制造商ID

/* Implementation version */
#define DET_SW_MAJOR_VERSION     (2u)         ///< Autosar主版本号
#define DET_SW_MINOR_VERSION     (0u)         ///< Autosar次版本号
#define DET_SW_PATCH_VERSION     (0u)         ///< Autosar修订版本号

/* AUTOSAR specification document version */
#define DET_AR_MAJOR_VERSION                (4u)          ///< Autosar主版本号
#define DET_AR_MINOR_VERSION                (4u)          ///< Autosar次版本号
#define DET_AR_RELEASE_REVISION_VERSION     (0u)          ///< Autosar修订版本号
#define DET_AR_RELEASE_MAJOR_VERSION	DET_AR_MAJOR_VERSION    ///< Autosar主版本号
#define DET_AR_RELEASE_MINOR_VERSION	DET_AR_MINOR_VERSION    ///< Autosar次版本号

/* 适配E3 MCAL*/
#define DET_H_AR_RELEASE_MAJOR_VERSION    (4)             ///< Autosar主版本号
#define DET_H_AR_RELEASE_MINOR_VERSION    (3)             ///< Autosar次版本号
#define DET_H_AR_RELEASE_REVISION_VERSION (1)             ///< Autosar修订版本号
#define DET_H_SW_MAJOR_VERSION    (1)                     ///< Autosar主版本号
#define DET_H_SW_MINOR_VERSION    (0)                     ///< Autosar次版本号
#define DET_H_SW_PATCH_VERSION    (0)                     ///< Autosar修订版本号

// 开发错误-ERRID
#define DET_E_PARAM_POINTER           (0x01u)             ///< (Service ID) 空指针

// 开发错误-APIID
#define DET_INIT_SERVICE_ID                 (0x00u)       ///< (Service ID) init api
#define DET_REPORTERROR_SERVICE_ID          (0x01u)       ///< (Service ID) report err api
#define DET_START_SERVICE_ID                (0x02u)       ///< (Service ID) start api
#define DET_GETVERSIONINFO_SERVICE_ID       (0x03u)       ///< (Service ID) getversion api

/* Global Type & Structure Definition */

/*!**************************************************************************************
 * @brief 定义Det 初始化配置类型
 ****************************************************************************************/
typedef uint8 Det_ConfigType;

#define  DET_START_SEC_CODE
#include "det_memmap.h"
/*!**************************************************************************************
 * @brief          初始化 Det
 * @details        将Det 模块初始化为默认值
 * @param[in]      ConfigPtr
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_Det_00008
 ****************************************************************************************/
FUNC(void, DET_CODE) Det_Init(const Det_ConfigType* ConfigPtr);

/*!**************************************************************************************
 * @brief          反初始化 Det
 * @details        将Det 模块初始化为默认值
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_Det_00002
 ****************************************************************************************/
FUNC(void, DET_CODE) Det_DeInit( void );

/*!**************************************************************************************
 * @brief          Det_Start
 * @details        DET_reporterror 等三个函数 在此函数执行后才开始工作
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_Det_00010
 ****************************************************************************************/
FUNC(void, DET_CODE) Det_Start( void );

/*!**************************************************************************************
 * @brief          Det 开发错误上报
 * @details        开发错误上报
 * @param[in]      ModuleId 模块Id号
 * @param[in]      InstanceId 实例Id号，Bsw模块对应的就是多核多Application实例化
 * @param[in]      ApiId 发生错误所在的API Id号
 * @param[in]      ErrorId 发生错误的错误类型号
 * @reentrant      可重入
 * @synchronous    异步
 * @retval         TRUE        消息发送成功
 * @retval         FALSE       消息发送失败
 * @swsid          SWS_Det_00009
 ****************************************************************************************/
FUNC(Std_ReturnType, DET_CODE) Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

/*!**************************************************************************************
 * @brief          Det 运行错误上报
 * @details        运行错误上报，通常是运行时产生的错误，如通信丢包
 * @param[in]      ModuleId 模块Id号
 * @param[in]      InstanceId 实例Id号，Bsw模块对应的就是多核多Application实例化
 * @param[in]      ApiId 发生错误所在的API Id号
 * @param[in]      ErrorId 发生错误的错误类型号
 * @retval         TRUE        消息发送成功
 * @retval         FALSE       消息发送失败
 * @reentrant      可重入
 * @synchronous    异步
 * @swsid          SWS_Det_01001
 ****************************************************************************************/
FUNC(Std_ReturnType, DET_CODE) Det_ReportRuntimeError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

/*!**************************************************************************************
 * @brief          Det 瞬态错误上报
 * @details        瞬态错误上报，例如存储等产生的读写错误
 * @param[in]      ModuleId 模块Id号
 * @param[in]      InstanceId 实例Id号，Bsw模块对应的就是多核多Application实例化
 * @param[in]      ApiId 发生错误所在的API Id号
 * @param[in]      FaultId 发生错误的错误类型号
 * @retval         TRUE        消息发送成功
 * @retval         FALSE       消息发送失败
 * @reentrant      可重入
 * @synchronous    异步
 * @swsid          SWS_Det_01003
 ****************************************************************************************/
FUNC(Std_ReturnType, DET_CODE) Det_ReportTransientFault(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 FaultId);

/*!**************************************************************************************
 * @brief          Det 错误码上报
 * @details        错误码上报，可以携带不大于20字节的额外信息吐出
 * @param[in]      errorId      模块ID
 * @param[in]      info         指针,指向想要发送的数据
 * @param[in]      len          发送的数据长度,最大不超过20字节
 * @retval         TRUE        消息发送成功
 * @retval         FALSE       消息发送失败
 * @reentrant      可重入
 * @synchronous    异步
 ****************************************************************************************/
FUNC(Std_ReturnType, DET_CODE) Det_ReportErrorWithInfo(uint32 errorId, uint8* info,uint8 len);

#if (DET_VERSIONINFO_API == STD_ON)
/*!**************************************************************************************
 * @brief          Det_GetVersionInfo 获取版本号
 * @details        版本号获取
 * @param[out]     vi 版本号结构体指针
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Det_00011
 ****************************************************************************************/
FUNC(void, DET_CODE) Det_GetVersionInfo(Std_VersionInfoType* vi);
#endif

/*!**************************************************************************************
 * @brief          设置令牌桶流量大小
 * @details        设置令牌桶流量大小，设置时自动补满令牌
 * @param[in]      cap 容量
 * @reentrant      可重入
 * @synchronous    同步
 ****************************************************************************************/
FUNC(void, DET_CODE) Det_SetTokenBucketCapacity(sint32 cap);

/*!**************************************************************************************
 * @brief          获取令牌桶Det流量控制阈值大小
 * @details        获取令牌桶Det流量控制阈值大小
 * @reentrant      可重入
 * @synchronous    同步
 * @retval         sint32令牌桶容量大小
 ****************************************************************************************/
FUNC(sint32, DET_CODE) Det_GetTokenBucketCapacity(void);
#define  DET_STOP_SEC_CODE
#include "det_memmap.h"

#ifndef configOS_WEAK
// 不支持weak统一宏的时候，定义为空
#define configOS_WEAK
#endif

#ifdef __cplusplus
}
#endif

#endif       /* DET_H */
