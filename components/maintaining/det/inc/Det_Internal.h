/*!**************************************************************************************
 * @file        Det_Internal.h
 * @brief       Det 内部配置数据类型头文件
 * @details     主要包含 Det 内部配置的数据定义和类型
 * @date        2023-3-17
 * @version     V1.0
 * @copyright   Copyright (C) 2015-2024 Lixiang Group Holding Limited
 ****************************************************************************************/

#ifndef DET_INTERNAL_H
#define DET_INTERNAL_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!**************************************************************************************
 * @brief 定义Det hook callout 函数类型
 ****************************************************************************************/
typedef Std_ReturnType (*Det_CalloutTableType)(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

#define  DET_START_SEC_CONST
#include "det_memmap.h"
/* 运行时错误回调 */
#if (DET_ENABLE_RUNTIME_ERROR_CALLOUT == STD_ON)
/*!**************************************************************************************
 * @brief 定义Det hook Det_RuntimeErrorCalloutTable 函数指针数组
 ****************************************************************************************/
extern FUNC(const Det_CalloutTableType, DET_CONST) Det_RuntimeErrorCalloutTable[DET_RUNTIMEERR_CALLOUT_TABLE_NUMBER];
#endif

/* 瞬态错误回调 */
#if (DET_ENABLE_TRANSIENT_FAULT_CALLOUT == STD_ON)
/*!**************************************************************************************
 * @brief 定义Det hook Det_TransientFaultCalloutTable 函数指针数组
 ****************************************************************************************/
extern FUNC(const Det_CalloutTableType, DET_CONST) Det_TransientFaultCalloutTable[DET_TRANSIENT_FAULT_CALLOUT_NUMBER];
#endif

/* 开发错误回调 */
#if (DET_ENABLE_ERROR_HOOK == STD_ON)
/*!**************************************************************************************
 * @brief 定义Det hook Det_ErrorHookTable 函数指针数组
 ****************************************************************************************/
extern FUNC(const Det_CalloutTableType, DET_CONST) Det_ErrorHookTable[DET_ERROR_HOOK_TABLE_NUMBER];
#endif
#define  DET_STOP_SEC_CONST
#include "det_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /*DET_INTERNAL_H*/
