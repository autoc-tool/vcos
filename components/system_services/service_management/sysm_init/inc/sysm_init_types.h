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

#ifndef ECUM_TYPES_H
#define ECUM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"

#if (VCOS_MODULE_CONFIG_RTE == 1)
#include "Rte_Type.h"
#endif

/*!*********************************************************************************************************************
 * @brief  EcuM 所有可能的状态
 **********************************************************************************************************************/
#ifndef ECUM_MAINSTATE_MASK
#define ECUM_MAINSTATE_MASK (0xF0u) ///< 主状态掩码
#endif

#ifndef ECUM_SUBSTATE_MASK
#define ECUM_SUBSTATE_MASK (0x0Fu) ///< 子状态掩码
#endif

#ifndef ECUM_STATE_STARTUP
#define ECUM_STATE_STARTUP (0x10u) ///< ECUM_STATE_STARTUP
#endif

#ifndef ECUM_STATE_STARTUP_ONE
#define ECUM_STATE_STARTUP_ONE (0x11u) ///< ECUM_STATE_STARTUP_ONE
#endif

#ifndef ECUM_STATE_STARTUP_TWO
#define ECUM_STATE_STARTUP_TWO (0x12u) ///< ECUM_STATE_STARTUP_TWO
#endif

#ifndef ECUM_STATE_WAKEUP
#define ECUM_STATE_WAKEUP (0x20u) ///< ECUM_STATE_WAKEUP
#endif

#ifndef ECUM_STATE_WAKEUP_ONE
#define ECUM_STATE_WAKEUP_ONE (0x21u) ///< ECUM_STATE_WAKEUP_ONE
#endif

#ifndef ECUM_STATE_WAKEUP_VALIDATION
#define ECUM_STATE_WAKEUP_VALIDATION (0x22u) ///< ECUM_STATE_WAKEUP_VALIDATION
#endif

#ifndef ECUM_STATE_WAKEUP_REACTION
#define ECUM_STATE_WAKEUP_REACTION (0x23u) ///< ECUM_STATE_WAKEUP_REACTION
#endif

#ifndef ECUM_STATE_WAKEUP_TWO
#define ECUM_STATE_WAKEUP_TWO (0x24u) ///< ECUM_STATE_WAKEUP_TWO
#endif

#ifndef ECUM_STATE_WAKEUP_WAKESLEEP
#define ECUM_STATE_WAKEUP_WAKESLEEP (0x25u) ///< ECUM_STATE_WAKEUP_WAKESLEEP
#endif

#ifndef ECUM_STATE_RUN
#define ECUM_STATE_RUN (0x30u) ///< ECUM_STATE_RUN
#endif

#ifndef ECUM_STATE_APP_RUN
#define ECUM_STATE_APP_RUN (0x32u) ///< ECUM_STATE_APP_RUN
#endif

#ifndef ECUM_STATE_APP_POST_RUN
#define ECUM_STATE_APP_POST_RUN (0x33u) ///< ECUM_STATE_APP_POST_RUN
#endif

#ifndef ECUM_STATE_SHUTDOWN
#define ECUM_STATE_SHUTDOWN (0x40u) ///< ECUM_STATE_SHUTDOWN
#endif

#ifndef ECUM_STATE_PREP_SHUTDOWN
#define ECUM_STATE_PREP_SHUTDOWN (0x44u) ///< ECUM_STATE_PREP_SHUTDOWN
#endif

#ifndef ECUM_STATE_GO_SLEEP
#define ECUM_STATE_GO_SLEEP (0x49u) ///< ECUM_STATE_GO_SLEEP
#endif

#ifndef ECUM_STATE_GO_OFF_ONE
#define ECUM_STATE_GO_OFF_ONE (0x4Du) ///< ECUM_STATE_GO_OFF_ONE
#endif

#ifndef ECUM_STATE_GO_OFF_TWO
#define ECUM_STATE_GO_OFF_TWO (0x4Eu) ///< ECUM_STATE_GO_OFF_TWO
#endif

#ifndef ECUM_STATE_SLEEP
#define ECUM_STATE_SLEEP (0x50u) ///< ECUM_STATE_SLEEP
#endif

/*!*********************************************************************************************************************
 * @brief  Enum literals for ecum_shutdown_target_type
 **********************************************************************************************************************/
#define ECUM_SHUTDOWN_TARGET_SLEEP (0x0u) ///< ECUM_SHUTDOWN_TARGET_SLEEP
#define ECUM_SHUTDOWN_TARGET_RESET (0x1u) ///< ECUM_SHUTDOWN_TARGET_RESET
#define ECUM_SHUTDOWN_TARGET_OFF (0x2u) ///< ECUM_SHUTDOWN_TARGET_OFF

/*!*********************************************************************************************************************
 * @brief  Enum literals for ecum_boot_target_type
 **********************************************************************************************************************/
#define ECUM_BOOT_TARGET_APP (0u) ///< ECUM_BOOT_TARGET_APP
#define ECUM_BOOT_TARGET_OEM_BOOTLOADER (1u) ///< ECUM_BOOT_TARGET_OEM_BOOTLOADER
#define ECUM_BOOT_TARGET_SYS_BOOTLOADER (2u) ///< ECUM_BOOT_TARGET_SYS_BOOTLOADER

/*!*********************************************************************************************************************
 * @brief  定义非法模式/频道
 **********************************************************************************************************************/
#define ECUM_SLEEP_MODE_WDGM_MODE_ILL (0xff) ///< ECUM_SLEEP_MODE_WDGM_MODE_ILL
#define ECUM_COMM_CHANNEL_ILL (0xffu) ///< ECUM_COMM_CHANNEL_ILL
#define ECUM_VALIDATION_TIMEOUT_ILL (0xffffuL) ///< ECUM_VALIDATION_TIMEOUT_ILL

/*!*********************************************************************************************************************
 * @brief  Run Request 状态定义
 **********************************************************************************************************************/
#define ECUM_RUNSTATUS_UNKNOWN (0u) ///< ECUM_RUNSTATUS_UNKNOWN
#define ECUM_RUNSTATUS_REQUESTED (1u) ///< ECUM_RUNSTATUS_REQUESTED
#define ECUM_RUNSTATUS_RELEASED (2u) ///< ECUM_RUNSTATUS_RELEASED

/*!*********************************************************************************************************************
 * @brief  唤醒源状态的定义
 * SWS_EcuM_04041
 **********************************************************************************************************************/
typedef enum {
	ECUM_WKSTATUS_NONE = 0, ///<  没有探测到唤醒事件
	ECUM_WKSTATUS_PENDING = 1, ///<  唤醒事件已被探测到但未验证
	ECUM_WKSTATUS_VALIDATED = 2, ///<  唤醒事件已被验证
	ECUM_WKSTATUS_EXPIRED = 3, ///<  唤醒事件还没被验证并且事件已经过期
	ECUM_WKSTATUS_DISABLED = 4 ///<  唤醒源已被失能且不会探测到唤醒事件
} ecum_wakeup_status_t;

/*!*********************************************************************************************************************
 * @brief ecum_reset_type 定义
 * SWS_EcuM_04044
 **********************************************************************************************************************/
typedef uint8 ecum_reset_t;

/*!*********************************************************************************************************************
 * @brief 定义EcuM_RunStatusType，表示ECU的运行状态
 **********************************************************************************************************************/
typedef uint8 ecum_run_status_t;

/*!*********************************************************************************************************************
 * @brief 定义EcuM_StateType，表示ECU的状态
 **********************************************************************************************************************/
typedef uint8 ecum_state_t;

/*!*********************************************************************************************************************
 * @brief 定义EcuM_ResetType下的重置类型宏定义
 **********************************************************************************************************************/
#define ECUM_RESET_MCU (0u) ///< MCU重置
#define ECUM_RESET_WDG (1u) ///< 看门狗重置
#define ECUM_RESET_IO (2u) ///< 输入/输出重置

/*!*********************************************************************************************************************
 * @brief  唤醒源类型的定义 SWS_EcuM_04040, SWS_EcuMf_00049
 **********************************************************************************************************************/
typedef uint32 ecum_wakeup_source_t;

/*!*********************************************************************************************************************
 * @brief  Rte 定义的数据类型
 **********************************************************************************************************************/
// #ifndef Rte_TypeDef_EcuM_UserType
typedef uint8 ecum_user_t; ///<  用户定义类型
// #endif
// #ifndef Rte_TypeDef_EcuM_TimeType
typedef uint32 ecum_time_t; ///<  时间定义类型
// #endif
// #ifndef Rte_TypeDef_EcuM_BootTargetType
typedef uint8 ecum_boot_target_t; ///<  引导目标类型
// #endif
// #ifndef Rte_TypeDef_EcuM_ShutdownCauseType
typedef uint8 ecum_shutdown_cause_t; ///<  关机原因类型
// #endif
// #ifndef Rte_TypeDef_EcuM_ShutdownModeType
typedef uint16 ecum_shutdown_mode_t; ///<  关机模式类型
// #endif
// #ifndef Rte_TypeDef_EcuM_ShutdownTargetType
typedef uint8 ecum_shutdown_target_t; ///<  关机目标类型
// #endif

#ifdef __cplusplus
}
#endif

#endif /* ECUM_TYPES_H  */