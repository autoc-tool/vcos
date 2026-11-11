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
#ifndef BSWM_INTERNAL_H
#define BSWM_INTERNAL_H

#include "bswm_types.h"
#if (CONFIG_MODULE_COM == 1)
#include "com.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

#define BSWM_STACK_MAX (7u) ///<  栈最大深度

/*!*********************************************************************************************************************
 * @brief   BswM 内部使用的状态类型
 **********************************************************************************************************************/
typedef struct {
	uint32 item_id; ///< 列表中项目id
	sint32 list_id; ///< 动作列表id
} bswm_action_id_t;

typedef struct {
	uint8 sp; ///< 栈顶指针
	bswm_action_id_t action_states[BSWM_STACK_MAX]; ///< 动作索引栈
} bswm_execute_stack_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 执行类型
 **********************************************************************************************************************/
typedef struct {
	bswm_timer_state_t timer_state; ///<  定时器状态
	bswm_timer_value_t timer_value; ///<  定时器值
} bswm_timer_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 执行类型
 **********************************************************************************************************************/
typedef enum {
	BSWM_TIMER_START = 0, ///<  启动定时器
	BSWM_TIMER_STOP ///<  关闭定时器
} bswm_timer_action_t;

typedef boolean (*logical_expression_func_t)(void);
typedef Std_ReturnType (*atomic_action_func_t)(const bswm_action_list_item_t *item);
/*!*********************************************************************************************************************
 * @brief   核心 ID 类型
 **********************************************************************************************************************/
typedef uint8 bswm_core_id_t;

/*!*********************************************************************************************************************
 * @brief   各个核心数组起止index
 **********************************************************************************************************************/
typedef struct {
	uint8 begin;
	uint8 end;
} bswm_array_index;

/*!*********************************************************************************************************************
 * @brief   BswM 内部类型
 **********************************************************************************************************************/
typedef struct {
#if (BSWM_NUMBER_OF_RULES_MAX > 0)
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	bswm_rule_state_t *bswm_rule_states;
	boolean bswm_request_processing_ongoing;
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	boolean *bswm_delayed_request;
	boolean bswm_pdu_group_switch_action_performed; /* @req BswM0128 */
	boolean bswm_com_ipdu_initialize;
	boolean bswm_dm_control_performed;
#endif
#if (BSWM_TIMERS_NUM > 0)
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	bswm_timer_t *timers;
	bswm_array_index timer_index;
#endif

#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
	com_ipg_vec bswm_com_ipdu_group_vector;
#endif

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
	com_ipg_vec bswm_dm_control_group_vector;
#endif
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	uint32 *bswm_mode_req_mirrors;
	bswm_init_status_t init_status; ///<  BswM 初始化状态
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	const atomic_action_func_t *bswm_atomic_actions;
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	const logical_expression_func_t *bswm_logical_expressions;
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	const bswm_rule_config_t *bswm_rule_config_ptr;
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	const bswm_config_type *bswm_config_ptr;
	// coverity[misra_c_2012_rule_5_9_violation:SUPPRESS]
	const bswm_rule_t *bswm_rules;
} bswm_internal_t;

/*!*********************************************************************************************************************
 * @brief   BswM 内部类型
 **********************************************************************************************************************/
extern bswm_internal_t *bswm_multicore[BSWM_MAX_SUPPORT_CORENUM];

#if (BSWM_DEV_ERROR_DETECT == STD_ON)
#include "det.h"

#define bswm_det_report_with_coreid(errorId, coreid)                                                                   \
	(void)det_report_error_with_info((errorId), (uint8 *)&(coreid)), 1U) ///< BswM 实例核心错误报告

#define bswm_det_reporterror(errorId) (void)det_report_error_with_info((errorId), NULL_PTR, 0) ///<  Det 错误信息报告

///<  BswM 有返回值校验
#define bswm_validate(expression, errorId, ret)                                                                        \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			bswm_det_reporterror(errorId);                                                                 \
			return ret;                                                                                    \
		}                                                                                                      \
	} while (0)

///<  BswM 无返回值校验
#define bswm_validate_norv(expression, errorId)                                                                        \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			bswm_det_reporterror(errorId);                                                                 \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

///<  BswM 多核无返回值校验
#define bswm_validate_withcoreid_norv(expression, coreid, errorId)                                                     \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			bswm_det_report_with_coreid(errorId, coreid);                                                  \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

///<  BswM 多核有返回值校验
#define bswm_validate_withcoreid(expression, coreid, errorId, ret)                                                     \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			bswm_det_report_with_coreid(errorId, coreid);                                                  \
			return ret;                                                                                    \
		}                                                                                                      \
	} while (0)

#else
#define bswm_det_reporterror(...) ///<  Det 错误信息报告

///<  BswM 有返回值校验
#define bswm_validate(expression, errorId, ret)                                                                        \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			return ret;                                                                                    \
		}                                                                                                      \
	} while (0)

#define bswm_validate_norv(expression, errorId)                                                                        \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

///<  BswM 无返回值校验
#define bswm_validate_withcoreid_norv(expression, coreid, errorId)                                                     \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

///<  BswM 多核有返回值校验
#define bswm_validate_withcoreid(expression, coreid, errorId, ret)                                                     \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			return ret;                                                                                    \
		}                                                                                                      \
	} while (0)
#endif

#define bswm_validate_pointer_withcoreid_norv(pointer, coreid, errorId)                                                \
	bswm_validate_withcoreid_norv((pointer != NULL), (coreid), (errorId)) ///<  BswM 传入指针无返回值校验

#define bswm_validate_pointer_withcoreid(pointer, coreid, errorId, ret)                                                \
	bswm_validate_withcoreid((pointer != NULL), (coreid), (errorId), (ret)) ///<  BswM 传入指针有返回值校验

#define bswm_validate_init(errorId, ptr)                                                                               \
	bswm_validate((ptr->init_status == BSWM_INIT), (errorId), E_NOT_OK) ///<  BswM 初始化有返回值校验

#define bswm_validate_init_norv(errorId, ptr)                                                                          \
	bswm_validate_norv((ptr->init_status == BSWM_INIT), (errorId)) ///<  BswM 初始化无返回值校验

#define bswm_validate_requestuser(expression, errorId)                                                                 \
	bswm_validate(expression, (errorId), E_NOT_OK) ///<  BswM 用户请求有返回值校验

#define bswm_validate_requestuser_norv(expression, errorId)                                                            \
	bswm_validate_norv(expression, (errorId)) ///<  BswM 用户请求无返回值校验

#define bswm_validate_requestmode(expression, errorId)                                                                 \
	bswm_validate(expression, (errorId), E_NOT_OK) ///<  BswM 模式请求有返回值校验

#define bswm_validate_requestmode_norv(expression, errorId)                                                            \
	bswm_validate_norv(expression, (errorId)) ///<  BswM 模式请求无返回值校验

#define bswm_validate_pointer_norv(pointer, errorId)                                                                   \
	bswm_validate_norv((pointer != NULL), (errorId)) ///<  BswM 传入指针无返回值校验

#define bswm_validate_cfgpointer_norv(pointer, errorId)                                                                \
	bswm_validate_norv((pointer != NULL), (errorId)) ///<  BswM 配置指针无返回值校验

#define bswm_validate_cfgparamrange_norv(expression, errorId)                                                          \
	bswm_validate_norv(expression, (errorId)) ///<  BswM 配置参数无返回值校验

/* Timer Status */
#define BSWM_TIMER_STOPPED ((uint8)0x00u) ///< 计时器停止
#define BSWM_TIMER_STARTED ((uint8)0x01u) ///< 计时器开始
#define BSWM_TIMER_EXPIRED ((uint8)0x02u) ///< 计时器过期

#define bswm_set_timer_state(Index, Value) (timers[(Index)].timer_state = (Value)) ///< 设置计时器状态
#define bswm_set_timer_value(Index, Value) (timers[(Index)].timer_value = (Value)) ///< 设置计时器值
#define bswm_get_timer_state(Index) (timers[Index].timer_state) ///< 获取计时器状态
#define bswm_get_timer_value(Index) (timers[Index].timer_value) ///< 获取计时器值

#if (BSWM_MULTICORE_USAGE == STD_ON)
#include "Os.h"
#define bswm_get_core_id() ((bswm_core_id_t)GetCoreID())
#else
#define bswm_get_core_id() 0
#endif /* if (WDGM_MULTICORE_USAGE == STD_ON) */

#if (CONFIG_MODULE_DEM == 1)
#define bswm_report_error_status(eventID, error) dem_set_event_status((dem_event_id_type)eventID, error);
#else ///< Dem 设置事件状态
#define bswm_report_error_status(eventID, error)
#endif

#define BSWM_START_SEC_CODE
#include "bswm_memmap.h"
#if (BSWM_PDUGROUPSWITCH_ACTIONS > 0)
/*!*********************************************************************************************************************
 * @brief          执行PDU组切换动作
 * @details        根据配置执行PduGroup的启用或禁用动作
 * @param[in]      item                    指向BswM_ActionListItemType结构，包含了PduGroup切换相关的配置数据
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00001
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_internal_pdu_group_switch_action(const bswm_action_list_item_t *item);
#endif

#if (BSWM_DEADLINEMONITORING_ACTIONS > 0)
/*!*********************************************************************************************************************
 * @brief          执行Deadline Monitoring控制动作
 * @details        根据BswM的决策触发Deadline Monitoring的控制操作
 * @param[in]      item                    指向BswM_ActionListItemType结构，包含了Deadline Monitoring控制相关的配置数据
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00002
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_internal_dm_control_action(const bswm_action_list_item_t *item);
#endif

#if (BSWM_PDUROUTERCONTROL_ACTIONS > 0)
/*!*********************************************************************************************************************
 * @brief          执行PDU路由控制动作
 * @details        根据BswM的决策激活或停止PDU路由操作
 * @param[in]      item     指向BswM_ActionListItemType结构，包含了PDU路由控制相关的配置数据
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_internal_pdu_router_control_action(const bswm_action_list_item_t *item);
#endif

#if (BSWM_NMCONTROL_ACTIONS > 0)
/*!*********************************************************************************************************************
 * @brief          执行网络管理控制操作
 * @details        根据BswM的决策执行网络管理器(Nm)的控制指令
 * @param[in]      item                    指向BswM_ActionListItemType结构，包含了网络管理控制相关的配置数据
 * @param[in]      NetworkHandle           网络的句柄，用于指定哪个网络管理器进行操作
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_nm_control(const bswm_action_list_item_t *item, const uint8 NetworkHandle);
#endif

/*!*********************************************************************************************************************
 * @brief          更新BswM定时器
 * @details        用于周期性更新或重置BswM定时器
 * @param[in]      item     指向BswM_ActionListItemType结构，包含了定时器更新操作相关的配置数据
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_update_timer(const bswm_action_list_item_t *item);
#define BSWM_STOP_SEC_CODE
#include "bswm_memmap.h"

extern bswm_timer_t timers[BSWM_TIMERS_NUM];
#ifdef __cplusplus
}
#endif

#endif /* BSWM_INTERNAL_H */
