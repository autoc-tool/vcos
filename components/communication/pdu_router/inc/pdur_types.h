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

#ifndef PDUR_TYPES_H
#define PDUR_TYPES_H

#include "Std_Types.h"
#include "comstack_cfg.h"
#include "comstack_types.h"
#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief    定义pdur核id类型
 **********************************************************************************************************************/
typedef uint8 pdur_core_id_type;

/*!*********************************************************************************************************************
 * @brief    定义pdur模块类型
 **********************************************************************************************************************/
typedef uint8 pdur_module_type;

/*!*********************************************************************************************************************
 * @brief    定义pdur路由路径组id类型
 **********************************************************************************************************************/
typedef uint16 pdur_routing_path_group_id_type;

/*!*********************************************************************************************************************
 * @brief    定义pdur状态类型
 **********************************************************************************************************************/
typedef uint8 pdur_state_type;

#define PDUR_UNINIT ((pdur_state_type)0x00u)
#define PDUR_ONLINE ((pdur_state_type)0x01u)
#define PDUR_REDUCED ((pdur_state_type)0x02u)

/*!*********************************************************************************************************************
 * @brief    定义pdur数据提供类型
 **********************************************************************************************************************/
typedef uint8 pdur_data_provision_type;

#define PDUR_NO_PROVISION ((pdur_data_provision_type)0x00u)
#define PDUR_TRIGGERTRANSMIT ((pdur_data_provision_type)0x01u)
#define PDUR_DIRECT ((pdur_data_provision_type)0x02u)

/*!*********************************************************************************************************************
 * @brief    定义pdur目的报文数据处理类型
 **********************************************************************************************************************/
typedef uint8 pdur_dest_pdu_data_processing_type;

#define IMMEDIATE ((pdur_dest_pdu_data_processing_type)0x00u)
#define DEFERRED ((pdur_dest_pdu_data_processing_type)0x01u)

/*!*********************************************************************************************************************
 * @brief    定义pdur目的报文数据处理类型
 **********************************************************************************************************************/
typedef uint8 pdur_dest_pdu_len_strategy_type;

#define PDUR_STRATEGY_UNUSED ((pdur_dest_pdu_data_processing_type)0x00u)
#define PDUR_STRATEGY_SHORTEN ((pdur_dest_pdu_data_processing_type)0x01u)
#define PDUR_STRATEGY_DISCARD ((pdur_dest_pdu_data_processing_type)0x02u)
#define PDUR_STRATEGY_IGNORE ((pdur_dest_pdu_data_processing_type)0x03u)

/*!*********************************************************************************************************************
 * @brief    定义pdur缓存池模块名称类型
 **********************************************************************************************************************/
typedef uint32 pdur_buffer_pool_module_name_type;

#define PDUR_BUFFER_POOL_DEFAULT ((pdur_buffer_pool_module_name_type)0x03u)

/*!*********************************************************************************************************************
 * @brief    定义pdur目的报文类型
 **********************************************************************************************************************/
typedef struct {
	const pdur_module_type dest_module;
	const pdur_core_id_type pdur_dest_pdu_core;
	const uint8 data_provision;
	const uint8 idx_num;
	const uint8 start_idx;
	const pdur_dest_pdu_len_strategy_type pdu_len_strategy;
	const PduIdType dest_pdu_id;
	const PduLengthType pdu_len_in_ecuc;
} pdur_dest_pdu_type;

/*!*********************************************************************************************************************
 * @brief    定义pdur路由路径类型
 **********************************************************************************************************************/
typedef struct {
	const pdur_module_type src_module;
	const uint8 pdur_destinatn_num;
	const uint8 pdur_direct_gateway;
	const uint8 pdur_queue_depth;
	const uint16 pdur_tx_buffer_idx;
	const pdur_core_id_type pdur_src_pdu_core;
	const PduLengthType pdur_tp_threshld;
	const PduIdType src_pdu_id;
	const pdur_dest_pdu_type *const *pdur_dest_pdus;
} pdur_routing_path_type;

/*!*********************************************************************************************************************
 * @brief    定义pdur配置id类型
 **********************************************************************************************************************/
typedef uint8 pdur_pb_config_id_type;

/*!*********************************************************************************************************************
 * @brief    定义pdur路由路径组类型
 **********************************************************************************************************************/
typedef struct {
	const boolean enabled_at_init;
	const uint8 start_idx;
	const uint8 idx_num;
} pdur_routing_path_group_type;

/*!*********************************************************************************************************************
 * @brief    定义pdur配置类型
 **********************************************************************************************************************/
typedef struct {
	pdur_pb_config_id_type pdur_configuration_id;
	PduIdType n_routing_paths;
	const pdur_routing_path_type *routing_paths;
	const pdur_routing_path_group_type *routing_path_groups_init_value_list;
	const PduIdType *routing_path_groups_dest_pdu_list;
	const pdur_routing_path_group_id_type *routing_path_group_list;
} pdur_pb_config_type;

#ifdef __cplusplus
}
#endif

#endif
