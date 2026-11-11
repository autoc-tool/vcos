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
#ifndef BSWM_TYPES_H
#define BSWM_TYPES_H

#include "Std_Types.h"
#include "bswm_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  Global Type & Structure Definition */
/*!*********************************************************************************************************************
 * @brief  定义BswM 用户类型
 **********************************************************************************************************************/
typedef uint8 bswm_user_t;

/*!*********************************************************************************************************************
 * @brief   定义BswM 模式类型
 **********************************************************************************************************************/
typedef uint8 bswm_mode_t;

/*!*********************************************************************************************************************
 * @brief   定义Timer 状态类型
 **********************************************************************************************************************/
typedef uint8 bswm_timer_state_t;

/*!*********************************************************************************************************************
 * @brief  定义Timer 值类型
 **********************************************************************************************************************/
typedef uint32 bswm_timer_value_t;

/*!*********************************************************************************************************************
 * @brief  定义BswM初始化状态类型
 **********************************************************************************************************************/
typedef enum {
	BSWM_UNINIT, ///<  未初始化
	BSWM_INIT ///<  已初始化
} bswm_init_status_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM PDUR执行类型
 **********************************************************************************************************************/
typedef enum {
	BSWM_PDUR_DISABLE = 0, ///<  失能PDUR
	BSWM_PDUR_ENABLE ///<  使能PDUR
} bswm_pdur_action_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM NM执行类型
 **********************************************************************************************************************/
typedef enum {
	BSWM_NM_DISABLE = 0, ///<  失能Nm
	BSWM_NM_ENABLE ///<  使能Nm
} bswm_nm_action_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 动作类型类型
 **********************************************************************************************************************/
typedef enum {
	BSWM_ATOMIC = 0, ///<  原子
	BSWM_LIST, ///<  列表
	BSWM_RULE ///<  规则
} bswm_action_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 动作执行方式类型
 **********************************************************************************************************************/
typedef enum {
	BSWM_CONDITION = 0, ///<  条件
	BSWM_TRIGGER ///<  触发
} bswm_execution_t;

/*!*********************************************************************************************************************
 *  @brief 定义Rule状态类型
 **********************************************************************************************************************/
typedef enum {
	BSWM_FALSE = 0, ///<  错误
	BSWM_TRUE, ///<  正确
	BSWM_UNDEFINED ///<  未定义
} bswm_rule_state_t;
/*!*********************************************************************************************************************
 * @brief 动作列表项定义
 **********************************************************************************************************************/
typedef struct {
	const boolean stop_if_fail; ///<  是否失败时终止执行
	const boolean need_reinit_pdu_group; ///<  是否重初始化PduGroupSwitch
	const uint8 timer_id; ///<  定时器ID
	const uint8 referred_lin_sched; ///< Lin调度引用
	const sint16 rule_idx_ref; ///< 引用的规则索引
	const sint32 *pdu_path_group_ref; ///< Pdu路由路径组引用ID
	const sint32 *inactive_pdu_group; ///< 禁用的Pdu组引用ID
	const sint32 *active_pdu_group; ///< 启用的Pdu组引用ID
	const bswm_action_t action_t; ///<  执行类型
	const bswm_nm_action_t nm_action; ///< Nm执行类型
	const bswm_timer_value_t timer_value; ///< 定时器值
	const bswm_pdur_action_t pdu_route_type; ///< Pdu路由执行类型
	const uint32 atomic_idx; ///< 原子动作索引
	const sint32 list_idx_ref; ///< 引用的列表索引
	const sint32 dem_fail_report_ref; ///< 报告失败到DEM的引用ID
} bswm_action_list_item_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 规则类型
 **********************************************************************************************************************/
typedef struct {
	const uint32 logic_expr_idx; ///<  逻辑表达式索引
	const sint32 false_list_idx; ///<  动作列表为假的索引
	const sint32 true_list_idx; ///<  动作列表为真的索引
	const bswm_rule_state_t initial_state; ///<  规则状态初始值
} bswm_rule_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 规则配置类型
 **********************************************************************************************************************/
typedef struct {
	const sint16 *immediate_rule_set; ///<  立即执行规则
	const sint16 *deferred_rule_set; ///<  延迟规则
	const bswm_rule_t *rule_list; ///<  规则数据
	const uint32 rules_count; ///<  规则数量
} bswm_rule_config_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 动作列表类型
 **********************************************************************************************************************/
typedef struct {
	const bswm_action_list_item_t *const *items; ///<  执行列表项指针数组
	const uint32 items_count; ///<  执行列表项数量
	const bswm_execution_t exec_mode; ///<  执行类型
} bswm_action_list_t;

/*!*********************************************************************************************************************
 * @brief 定义BswM 全局配置类型
 **********************************************************************************************************************/
typedef struct {
	const bswm_rule_config_t *rule_cfg; ///<  规则配置
	const bswm_action_list_t *action_list_data; ///<  动作列表数据
	const uint32 *mode_request_data; ///<  模式请求数据
} bswm_config_type;

#ifdef __cplusplus
}
#endif

#endif /* BSWM_TYPES_H */