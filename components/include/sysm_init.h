/*
 * Copyright (c) 2025 Li Auto Inc. and its affiliates
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECUM_H
#define ECUM_H
#include "sysm_init_cfg.h"
#include "sysm_init_types.h"
#include "sysm_init_cbk.h"

#if (CONFIG_MODULE_COM == 1)
#include "comstack_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ECUM_MODULE_ID (10u) ///< 模块 ID
#define ECUM_VENDOR_ID (60u) ///< 供应商 ID

#define ECUM_SW_MAJOR_VERSION (3u) ///< 软件主版本号
#define ECUM_SW_MINOR_VERSION (1u) ///< 软件次版本号
#define ECUM_SW_PATCH_VERSION (0u) ///< 软件补丁版本号

#define ECUM_AR_MAJOR_VERSION (4u) ///< 自动重构主版本号
#define ECUM_AR_MINOR_VERSION (3u) ///< 自动重构次版本号
#define ECUM_AR_PATCH_VERSION (0u) ///< 自动重构补丁版本号

#define ECUM_CALLOUT_SW_MAJOR_VERSION (3u) ///< 回调软件主版本号
#define ECUM_CALLOUT_SW_MINOR_VERSION (0u) ///< 回调软件次版本号
#define ECUM_CALLOUT_SW_PATCH_VERSION (0u) ///< 回调软件补丁版本号

/*!*********************************************************************************************************************
 * @brief          错误代码
 **********************************************************************************************************************/
#define ECUM_E_UNINIT (0x10u) ///< 未初始化
#define ECUM_E_SERVICE_DISABLED (0x11u) ///< 服务失能
#define ECUM_E_NULL_POINTER (0x12u) ///< 空指针
#define ECUM_E_INVALID_PAR (0x13u) ///< 无效参数
#define ECUM_E_MULTIPLE_RUN_REQUESTS (0x14u) ///< 多个 RUN 请求
#define ECUM_E_MISMATCHED_RUN_RELEASE (0x15u) ///< 不匹配的运行版本
#define ECUM_E_STATE_PAR_OUT_OF_RANGE (0x16u) ///< 状态超出范围
#define ECUM_E_UNKNOWN_WAKEUP_SOURCE (0x17u) ///< 未知唤醒源
#define ECUM_E_TIMERERROR (0x18u) ///< 计时器错误
#define ECUM_E_INVALID_SHUTDOWN_TARGET (0x19u) ///< 无效的ShutdownTarget
#define ECUM_E_INVALID_CURRENT_STATE (0x1au) ///< 无效的当前状态

/*!*********************************************************************************************************************
 * @brief          服务 ID
 **********************************************************************************************************************/
#define ECUM_REQUESTRUN_ID (0x03u) ///< Request RUN
#define ECUM_RELEASERUN_ID (0x04u) ///< Release RUN
#define ECUM_SELECTSHUTDOWNTARGET_ID (0x06u) ///< SelectShutdownTarget
#define ECUM_GETSTATE_ID (0x07u) ///< Get State
#define ECUM_GETSHUTDOWNTARGET_ID (0x09u) ///< GetShutdownTarget
#define ECUM_REQUESTPOSTRUN_ID (0x0au) ///< Request PostRUN
#define ECUM_RELEASEPOSTRUN_ID (0x0bu) ///< Release PostRUN
#define ECUM_SETWAKEUPEVENT_ID (0x0cu) ///< Set Wakeupevent
#define ECUM_SELECTAPPMODE_ID (0x0fu) ///< Select AppMode
#define ECUM_GETAPPMODE_ID (0x11u) ///< Get AppMode
#define ECUM_SELECT_BOOTARGET_ID (0x12u) ///< Select BootTarget
#define ECUM_GET_BOOTARGET_ID (0x13u) ///< Get BootTarget
#define ECUM_VALIDATE_WAKEUP_EVENT_ID (0x14u) ///< Validate Wakeupevent
#define ECUM_GETSTATUSOFWAKEUPSOURCE_ID (0x17u) ///< Get Status of Wakeupsource
#define ECUM_MAINFUNCTION_ID (0x18u) ///< MainFunction
#define ECUM_STARTUPTWO_ID (0x1au) ///< StartupTwo
#define ECUM_SELECTSHUTDOWNCAUSE_ID (0x1bu) ///< SelectShutdownCause
#define ECUM_GETSHUTDOWNCAUSE_ID (0x1cu) ///< GetShutdownCause
#define ECUM_GODOWN_ID (0x1fu) ///< Godown
#define ECUM_GOHALT_ID (0x20u) ///< Gohalt
#define ECUM_GOPOLL_ID (0x21u) ///< Gopoll
#define ECUM_GETVERSIONINFO_ID (0x00u) ///< Get Versioninfo

/*!*********************************************************************************************************************
 * @brief          每个内核的状态
 **********************************************************************************************************************/
#define ECUM_CORESTATUS_OFF (0x00u) ///< OFF
#define ECUM_CORESTATUS_SLEEP (0x01u) ///< SLEEP
#define ECUM_CORESTATUS_RUN (0x02u) ///< RUN

#define ecum_set_core_status_value(partitionIdx, value) (*(ecum_core_status[partitionIdx]) = value)
#define ecum_get_core_status_value(partitionIdx) (*(ecum_core_status[partitionIdx]))

#define ECUM_AR_VERSION ((ECUM_AR_MAJOR_VERSION * 10000) + (ECUM_AR_MINOR_VERSION * 100) + ECUM_AR_PATCH_VERSION)

#define ECUM_START_SEC_CODE
#include "ecum_memmap.h"
#if (ECUM_VERSION_INFO_API == STD_ON)
/*!*********************************************************************************************************************
 * @brief          获取 EcuM 版本信息
 * @details        获取 EcuM 模块的软件版本信息。此接口将返回模块的软件版本信息。
 * @param[out]     version_info 指向要填充的版本信息结构的指针
 * @reentrant      TRUE 可重入
 * @synchronous    TRUE 同步
 * @swsid          SWS_EcuM_02813
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_get_version_info(Std_VersionInfoType *version_info);
#endif

/*!*********************************************************************************************************************
 * @brief          不需要 OS 支持的相关模块初始化
 * @details        程序主函数调用此函数来初始化不需要 OS 支持的相关模块并启动 OS。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02811
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) platform_init(void);

/*!*********************************************************************************************************************
 * @brief          需要 OS 支持的相关模块初始化
 * @details        OS 启动后在任务中调用此函数完成需要 OS 支持的相关 BSW 模块初始化。
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02838
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) system_init(void);

/*!*********************************************************************************************************************
 * @brief          实现 ECU 最终的关闭或重启
 * @details        此函数通常被 OS shutdown hook 调用，最终完成 ECU 重启或关机。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02812
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_shutdown(void);

/*!*********************************************************************************************************************
 * @brief          切换 EcuM 的状态
 * @details        此函数由 BswM 调用来指示 EcuM 状态的切换。
 * @param[in]      state 要切换到的 EcuM 状态。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_04122
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_set_state(ecum_state_t state);

/*!*********************************************************************************************************************
 * @brief          获取 EcuM 当前的状态
 * @details        此函数可以被调用来获取 EcuM 当前的状态。
 * @param[out]     state 指向存储 EcuM 当前状态的变量的指针。
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           获取 EcuM 状态成功。
 * @retval         E_NOT_OK       获取 EcuM 状态失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_get_state(ecum_state_t *state);

/*!*********************************************************************************************************************
 * @brief          请求 EcuM RUN 状态
 * @details        用户调用此函数请求 RUN 状态。
 * @param[in]      user 请求 RUN 状态的用户标识。
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           请求 EcuM RUN 状态成功。
 * @retval         E_NOT_OK       请求 EcuM RUN 状态失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_04124
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_request_run(ecum_user_t user);

/*!*********************************************************************************************************************
 * @brief          释放 EcuM RUN 状态的请求
 * @details        用户调用此函数释放先前通过调用 EcuM_RequestRUN 完成的 RUN 请求。
 * @param[in]      user 释放 RUN 请求的用户标识。
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           释放 EcuM RUN 状态成功。
 * @retval         E_NOT_OK       释放 EcuM RUN 状态失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_04127
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_release_run(ecum_user_t user);

/*!*********************************************************************************************************************
 * @brief          请求 EcuM POST_RUN 状态
 * @details        用户调用此函数请求 POST_RUN 状态
 * @param[in]      user
 * @retval         E_OK           请求 EcuM POST_RUN 状态成功
 * @retval                 E_NOT_OK       请求 EcuM POST_RUN 状态失败
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_04128
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_request_post_run(ecum_user_t user);

/*!*********************************************************************************************************************
 * @brief          释放 EcuM POST_RUN 状态的请求
 * @details        用户调用此函数释放先前通过调用 EcuM_RequestPOST_RUN 完成的 POST_RUN 请求
 * @param[in]      user
 * @retval         E_OK           释放 EcuM POST_RUN 状态成功
 * @retval         E_NOT_OK       释放 EcuM POST_RUN 状态失败
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_04128
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_release_post_run(ecum_user_t user);

/*!*********************************************************************************************************************
 * @brief          选择对应的 ShutdownTarget
 * @details        此函数用于选择不同的 ShutdownTarget 和 sleepMode。
 * @param[in]      shutdownTarget ShutdownTarget类型
 * @param[in]      sleepMode Shutdown模式
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           选择 ShutdownTarget 成功。
 * @retval         E_NOT_OK       选择 ShutdownTarget 失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02822
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE)
ecum_select_shutdown_target(ecum_shutdown_target_t shutdownTarget, ecum_shutdown_mode_t sleepMode);

/*!*********************************************************************************************************************
 * @brief          获取 ShutdownTarget
 * @details        此函数用于获取当前的 ShutdownTarget 和 sleepMode。
 * @param[out]     shutdownTarget 指向存储 ShutdownTarget 的变量的指针
 * @param[out]     sleepMode 指向存储 sleepMode 的变量的指针
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           获取 ShutdownTarget 成功。
 * @retval         E_NOT_OK       获取 ShutdownTarget 失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02822
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE)
ecum_get_shutdown_target(ecum_shutdown_target_t *shutdownTarget, ecum_shutdown_mode_t *sleepMode);

/*!*********************************************************************************************************************
 * @brief          获取上一次的 ShutdownTarget
 * @details        此函数用于获取上一次的 ShutdownTarget 和 sleepMode。
 * @param[out]     shutdownTarget 指向存储上一次 ShutdownTarget 的变量的指针
 * @param[out]     sleepMode 指向存储上一次 sleepMode 的变量的指针
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           获取上一次的 ShutdownTarget 成功。
 * @retval         E_NOT_OK       获取上一次的 ShutdownTarget 失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02825
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE)
ecum_get_last_shutdown_target(ecum_shutdown_target_t *shutdownTarget, ecum_shutdown_mode_t *sleepMode);

/*!*********************************************************************************************************************
 * @brief          选择 ShutdownCause
 * @details        此函数用于选择 ShutdownCause。
 * @param[in]      target ShutdownCause
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           选择 ShutdownCause 成功。
 * @retval         E_NOT_OK       选择 ShutdownCause 失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_04050
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_select_shutdown_cause(ecum_shutdown_cause_t target);

/*!*********************************************************************************************************************
 * @brief          获取 ShutdownCause
 * @details        此函数用于获取当前 ShutdownCause。
 * @param[out]     shutdownCause 指向存储 ShutdownCause 的变量的指针
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           获取 ShutdownCause 成功。
 * @retval         E_NOT_OK       获取 ShutdownCause 失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_04051
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_get_shutdown_cause(ecum_shutdown_cause_t *shutdownCause);

/*!*********************************************************************************************************************
 * @brief          选择 BootTarget
 * @details        此函数用于选择一个 BootTarget。
 * @param[in]      target BootTarget类型
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02835
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_select_boot_target(ecum_boot_target_t target);

/*!*********************************************************************************************************************
 * @brief          获取 BootTarget
 * @details        此函数用于获取当前 BootTarget。
 * @param[out]     target 指向存储 BootTarget 的变量的指针
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           获取 BootTarget 成功。
 * @retval         E_NOT_OK       获取 BootTarget 失败。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02836
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_get_boot_target(ecum_boot_target_t *target);

/*!*********************************************************************************************************************
 * @brief          获取当前挂起的唤醒事件
 * @details        此函数用于获取当前被挂起的待验证的唤醒事件。
 * @return         ecum_wakeup_source_t 当前挂起的唤醒事件。
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02827
 **********************************************************************************************************************/
FUNC(ecum_wakeup_source_t, ECUM_CODE) ecum_get_pending_wakeup_events(void);

/*!*********************************************************************************************************************
 * @brief          清除当前所有挂起/已验证的唤醒事件
 * @details        此函数用于清除当前所有挂起/已验证的唤醒事件。
 * @param[in]      source 唤醒源
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02828
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_clear_wakeup_event(ecum_wakeup_source_t source);

/*!*********************************************************************************************************************
 * @brief          获取当前已验证的唤醒事件
 * @details        此函数用于获取当前已验证的唤醒事件。
 * @return         ecum_wakeup_source_t 当前已验证的唤醒事件。
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02830
 **********************************************************************************************************************/
FUNC(ecum_wakeup_source_t, ECUM_CODE) ecum_get_validated_wakeup_events(void);

/*!*********************************************************************************************************************
 * @brief          获取当前唤醒源的状态
 * @details        此函数用于获取当前唤醒源的状态。
 * @param[in]      sources 唤醒源
 * @return         ecum_wakeup_status_t 唤醒源状态。
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02830
 **********************************************************************************************************************/
FUNC(ecum_wakeup_status_t, ECUM_CODE) ecum_get_status_of_wakeup_source(ecum_wakeup_source_t sources);

/*!*********************************************************************************************************************
 * @brief          根据唤醒源设置对应的唤醒事件
 * @details        此函数用于根据唤醒源设置对应的唤醒事件。
 * @param[in]      sources 唤醒源
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02826
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_set_wakeup_event(ecum_wakeup_source_t sources);

/*!*********************************************************************************************************************
 * @brief          根据唤醒源验证对应的唤醒事件
 * @details        此函数用于根据唤醒源验证对应的唤醒事件。
 * @param[in]      sources 唤醒源
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EcuM_02829
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_validate_wakeup_event(ecum_wakeup_source_t sources);

/*!*********************************************************************************************************************
 * @brief          EcuM 关机流程
 * @details        BswM 将调用此函数使 ECU 进入关机流程。
 * @param[in]      caller 调用者
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           EcuM 关机流程成功。
 * @retval         E_NOT_OK       EcuM 关机流程失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_go_down(uint16 caller);

/*!*********************************************************************************************************************
 * @brief          EcuM Halt 休眠流程
 * @details        BswM 将调用此函数使 ECU 进入 Halt 休眠流程。
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           EcuM Halt 休眠流程成功。
 * @retval         E_NOT_OK       EcuM Halt 休眠流程失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_go_halt(void);

/*!*********************************************************************************************************************
 * @brief          EcuM Poll 休眠流程
 * @details        BswM 将调用此函数使 ECU 进入 Poll 休眠流程。
 * @return         Std_ReturnType 操作结果。
 * @retval         E_OK           EcuM Poll 休眠流程成功。
 * @retval         E_NOT_OK       EcuM Poll 休眠流程失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ECUM_CODE) ecum_go_poll(void);

/*!*********************************************************************************************************************
 * @brief          EcuM 进入 RUN 模式
 * @details        当所有 BSW 模块初始化完成，EcuM 将调用此函数进入 RUN 模式
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_enter_run_mode(void);

/*!*********************************************************************************************************************
 * @brief          设置当前 EcuM 状态
 * @details        当 EcuM 发生模式切换，将使用此函数完成状态切换
 * @param[in]      state
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) set_current_state(ecum_state_t state);

/*!*********************************************************************************************************************
 * @brief          判断当前是否有用户存在用户 RUN 请求
 * @details        检查当前是否有用户提出 RUN 状态请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, ECUM_CODE) has_run_requests(void);

/*!*********************************************************************************************************************
 * @brief          判断当前是否有用户存在用户 POST RUN 请求
 * @details        检查当前是否有用户提出 POST RUN 状态请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, ECUM_CODE) has_post_run_requests(void);

/*!*********************************************************************************************************************
 * @brief          启动唤醒源验证定时器
 * @details        当存在待验证的唤醒源时，通过此函数启动定时器以进行验证
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_set_validation_timer(void);

/*!*********************************************************************************************************************
 * @brief          EcuM 校验唤醒源
 * @details        EcuM 校验唤醒源
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ECUM_CODE) ecum_internal_validation(void);
#define ECUM_STOP_SEC_CODE
#include "ecum_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* ECUM_H */