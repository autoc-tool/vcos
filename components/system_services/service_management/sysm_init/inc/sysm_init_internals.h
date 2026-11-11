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

#ifndef ECUM_INTERNALS_H
#define ECUM_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif
/*!*********************************************************************************************************************
 * @brief          Including File
 **********************************************************************************************************************/
#include "sysm_init.h"
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
#include "det.h"

/*!*********************************************************************************************************************
 * @brief          Development errors should be reported to DET module
 **********************************************************************************************************************/
#define ecum_validate(_exp, _errId)                                \
do {                                                           \
	if (!(_exp)) {                                                 \
	    (void)det_report_error_with_info(_errId, NULL_PTR, 0);     \
	}                                                              \
} while (0)

#define ecum_validate_rv(_exp, _errId, _rv)                     \
do {                                                        \
	if (!(_exp)) {                                          \
	    (void)det_report_error_with_info(_errId, NULL_PTR, 0); \
	    return _rv;                                         \
	}                                                       \
} while(0)

#define ecum_validate_no_rv(_exp, _errId)                   \
do {                                                    \
	if (!(_exp)) {                                          \
	    (void)det_report_error_with_info(_errId, NULL_PTR, 0); \
	    return;                                             \
	}                                                       \
} while(0)

#define ecum_det_report_error(_errId)                       \
do {                                                    \
	(void)det_report_error_with_info(_errId, NULL_PTR, 0);  \
} while(0)

#define ecum_get_errorid(_serviceId, _errorCode)             = \
	((((uint32)ECUM_MODULE_ID) << 16u) | (((uint32)_serviceId) << 8u) | ((uint32)_errorCode))

#else
#define ecum_validate(_exp, _errId)

#define ecum_validate_rv(_exp, _errId, _rv)          \
do{                                                  \
	if (!(_exp)) {                                   \
		return _rv;                                  \
	}                                                \
} while(0)

#define ecum_validate_no_rv(_exp, _errId)                \
do{                                                      \
	if (!(_exp)) {                                       \
		return _rv;                                      \
	}                                                    \
} while(0)

#define ecum_det_report_error(_errId)

#define ecum_get_errorid(_serviceId, _errorCode)
#endif

/*!*********************************************************************************************************************
 * @brief          Get TimeTick
 **********************************************************************************************************************/
#if ((CONFIG_RT_FRAMEWORK == 1u) && (EcuM_INITPHASEDOT == 1u))
#define ecum_get_timmertick(_TickNum) up_timer_value((clock_t *)&_TickNum)
float32 ecum_calculate_time(uint32 ecum_t1, uint32 ecum_t0);
#else
#define ecum_get_timmertick(_TickNum)
#define ecum_calculate_time(_t1, _t0)
#endif

/*!*********************************************************************************************************************
 * @brief          get user wksource ID
 **********************************************************************************************************************/
#define ecum_wk_source_id(i) ((i) + ECUM_WKSOURCE_SYSTEM_CNT - ECUM_WKSOURCE_USER_CNT)

/*!*********************************************************************************************************************
 * @brief          core numbs for sync during STARTUP Two phash
 **********************************************************************************************************************/
#define ECUM_STARTSYNC (((uint64)(1u) << (ECUM_NUMBER_OF_CORES)) - (1u))

#define ECUM_COMBINEDWAKEUPS (ecum_get_pending_wakeup_events() | ecum_get_validated_wakeup_events())
#define WAKEUP_SOURCE_TEST ECUM_WKSOURCE_CAN_1
#define ECUM_WKSOURCE_NONE (ecum_wakeup_source_t)(0x00000000uL) ///< 无唤醒源
#define ECUM_WKSOURCE_ALL_SOURCES (ecum_wakeup_source_t)(0xFFFFFFFFuL) ///< 所有唤醒源

/*!*********************************************************************************************************************
 * @brief  EcuM core_id
 **********************************************************************************************************************/
typedef uint8 ecum_core_id_t;

/*!*********************************************************************************************************************
 * @brief  EcuM  全局结构体
 **********************************************************************************************************************/
typedef struct {
	boolean initiated; ///<  是否初始化
	boolean kill_all_request; ///<  清除所有请求
	boolean kill_all_postrun_request; ///<  清除所有Postrun请求
	ecum_shutdown_target_t shutdown_target; ///<  ShutdownTarget
	ecum_app_mode_t app_mode; ///<  AppMode
	ecum_state_t current_state; ///<  EcuM State
	ecum_config_t *config; ///<  模块配置
	uint32 run_requests; ///<  Run Requests
	uint32 postrun_requests; ///<  PostRun Requests
	ecum_shutdown_cause_t shutdown_cause; ///<  ShutdownCause
	ecum_shutdown_mode_t sleep_mode; ///<  关机模式
	// 由EcuM_SetWakeupEvent设置的事件
	uint32 wakeup_events; ///<  挂起的唤醒事件
	uint32 wakeup_timer; ///<  唤醒定时器
	// 由EcuM_ValidateWakeupEvent设置的事件
	uint32 valid_events; ///<  已验证事件
	uint32 validation_timer; ///<  验证定时器
} ecum_global_t;

extern ecum_global_t ecum_world;
/*!*********************************************************************************************************************
 * @brief          StartupTwo起止时间
 **********************************************************************************************************************/
extern uint32 ecum_startup_two_start_time;
extern uint32 ecum_bswm_end_time_value;
extern uint32 ecum_rte_end_time_value;
extern uint32 ecum_rte_sync_time_value;
/*!*********************************************************************************************************************
 * @brief          EcuM各个初始化阶段起始时间点
 **********************************************************************************************************************/
extern uint32 ecum_init_start_time;
extern uint32 ecum_init_one_start_time;
extern uint32 ecum_init_two_start_time;
extern uint32 ecum_init_three_start_time;
/*!*********************************************************************************************************************
 * @brief          EcuM各个初始化阶段结束时间点
 **********************************************************************************************************************/
extern uint32 ecum_init_zero_end_time;
extern uint32 ecum_init_one_end_time;
extern uint32 ecum_init_two_end_time;
extern uint32 ecum_init_three_end_time;

/*!*********************************************************************************************************************
 * @brief          定义不同通信通道名称的枚举类型
 **********************************************************************************************************************/
typedef enum {
	ALL_COM, ///<  所有通道
	ALL_WO_LIN, ///<  除了 Lin 的其他通道
	ONLY_LIN ///<  只有 Lin 通道
} ecum_comm_communication_groups_t;

#define ECUM_START_SEC_CODE
#include "ecum_memmap.h"

#define ECUM_STOP_SEC_CODE
#include "ecum_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* ECUM_INTERNALS_H */
