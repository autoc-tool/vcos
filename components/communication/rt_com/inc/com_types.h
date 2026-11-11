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

#ifndef COM_TYPES_H
#define COM_TYPES_H

#include "comstack_types.h"
#include "com_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COM_NO_FUNCTION_CALLOUT (0xFFFFu)
#define NO_PDU_REFERENCE (0xFFFFu)
#define COM_GROUPSIGNALREFS_END (0xFFFFu)
#define COM_NO_REF_INDEX (0xFFFFu)
#define COM_NO_COREMAPIDX (0u)

#define COM_SIGNALTYPE_UNSIGNED (FALSE)
#define COM_SIGNALTYPE_SIGNED (TRUE)

/*!*********************************************************************************************************************
 * @brief    定义com核id类型
 **********************************************************************************************************************/
typedef uint8 com_core_id_type;

/*!*********************************************************************************************************************
 *  @brief  ipdu组id类型
 **********************************************************************************************************************/
typedef uint16 com_ipdu_group_id_type;

/*!*********************************************************************************************************************
 *  @brief  信号id类型
 **********************************************************************************************************************/
typedef uint16 com_signal_id_type;

/*!*********************************************************************************************************************
 *  @brief  信号组id类型
 **********************************************************************************************************************/
typedef uint16 com_signal_group_id_type;

/*!*********************************************************************************************************************
 *  @brief  比特位置类型
 **********************************************************************************************************************/
typedef uint16 com_bit_pos_type;

/*!*********************************************************************************************************************
 *  @brief  rx ipdu callout 回调
 **********************************************************************************************************************/
typedef boolean (*com_rx_ipdu_callout_type)(PduIdType pdu_id, const PduInfoType *ipdu_data);

/*!*********************************************************************************************************************
 *  @brief  tx ipdu callout 回调
 **********************************************************************************************************************/
typedef boolean (*com_tx_ipdu_callout_type)(PduIdType pdu_id, PduInfoType *ipdu_data);

/*!*********************************************************************************************************************
 *  @brief  rx signal 通知回调
 **********************************************************************************************************************/
typedef void (*com_rx_signal_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  rx signal超时通知回调
 **********************************************************************************************************************/
typedef void (*com_rx_signal_timeout_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  rx signal无效通知类型
 **********************************************************************************************************************/
typedef void (*com_rx_signal_invalid_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  rx signal错误通知类型
 **********************************************************************************************************************/
typedef void (*com_rx_signal_error_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  tx signal通知类型
 **********************************************************************************************************************/
typedef void (*com_tx_signal_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  tx signal超时通知类型
 **********************************************************************************************************************/
typedef void (*com_tx_signal_timeout_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  tx signal无效通知类型
 **********************************************************************************************************************/
typedef void (*com_tx_signal_invalid_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  tx signal错误通知类型
 **********************************************************************************************************************/
typedef void (*com_tx_signal_error_notification_callout_type)(void);

/*!*********************************************************************************************************************
 *  @brief  清除发送ipdu更新bit类型
 **********************************************************************************************************************/
typedef uint8 com_tx_ipdu_clear_update_bit_type;
#define COM_CONFIRMATION ((com_tx_ipdu_clear_update_bit_type)0u)
#define COM_TRANSMIT ((com_tx_ipdu_clear_update_bit_type)1u)
#define COM_TRIGGERTRANSMIT ((com_tx_ipdu_clear_update_bit_type)2u)

/*!*********************************************************************************************************************
 *  @brief  状态类型
 **********************************************************************************************************************/
typedef uint8 com_status_type;
#define COM_UNINIT ((com_status_type)0u)
#define COM_INIT ((com_status_type)1u)

/*!*********************************************************************************************************************
 *  @brief  信号处理模式类型
 **********************************************************************************************************************/
typedef uint8 com_ipdu_signal_processing_mode;
#define COM_IMMEDIATE ((com_ipdu_signal_processing_mode)0u)
#define COM_DEFERRED ((com_ipdu_signal_processing_mode)1u)

/*!*********************************************************************************************************************
 *  @brief  ipdu方向类型
 **********************************************************************************************************************/
typedef uint8 com_ipdu_direction;
#define COM_RECEIVE ((com_ipdu_direction)0u)
#define COM_SEND ((com_ipdu_direction)1u)

/*!*********************************************************************************************************************
 *  @brief  信号类型
 **********************************************************************************************************************/
typedef uint8 com_signal_type;
#define COM_BOOLEAN ((com_signal_type)0u)
#define COM_UINT8 ((com_signal_type)1u)
#define COM_UINT16 ((com_signal_type)2u)
#define COM_UINT32 ((com_signal_type)3u)

#define COM_SINT8 ((com_signal_type)4u)
#define COM_SINT16 ((com_signal_type)5u)
#define COM_SINT32 ((com_signal_type)6u)
#define COM_FLOAT32 ((com_signal_type)7u)

#define COM_UINT64 ((com_signal_type)8u)
#define COM_SINT64 ((com_signal_type)9u)
#define COM_FLOAT64 ((com_signal_type)10u)
#define COM_UINT8_N ((com_signal_type)11u)
#define COM_UINT8_DYN ((com_signal_type)12u)

/*!*********************************************************************************************************************
 *  @brief  转换属性类型
 **********************************************************************************************************************/
typedef uint8 com_transfer_property_type;
#define COM_PENDING ((com_transfer_property_type)0u)
#define COM_TRIGGERED ((com_transfer_property_type)1u)
#define COM_TRIGGERED_WITHOUT_REPETITION ((com_transfer_property_type)2u)
#define COM_TRIGGERED_ON_CHANGE_WITHOUT_REPETITION ((com_transfer_property_type)3u)
#define COM_TRIGGERED_ON_CHANGE ((com_transfer_property_type)4u)

/*!*********************************************************************************************************************
 *  @brief  发送模式类型
 **********************************************************************************************************************/
typedef uint8 com_tx_mode_mode_type;
#define COM_DIRECT ((com_tx_mode_mode_type)0u)
#define COM_MIXED ((com_tx_mode_mode_type)1u)
#define COM_NONE ((com_tx_mode_mode_type)2u)
#define COM_PERIODIC ((com_tx_mode_mode_type)3u)

/*!*********************************************************************************************************************
 *  @brief  信号大小端类型
 **********************************************************************************************************************/
typedef uint8 com_signal_endianess_type;
#define COM_BIG_ENDIAN ((com_signal_endianess_type)0u)
#define COM_LITTLE_ENDIAN ((com_signal_endianess_type)1u)
#define COM_OPAQUE ((com_signal_endianess_type)2u)

/*!*********************************************************************************************************************
 *  @brief  数据无效动作类型
 **********************************************************************************************************************/
typedef uint8 com_data_invalidt_action_type;
#define INVALID_NOTIFY ((com_data_invalidt_action_type)0U)
#define INVALID_REPLACE ((com_data_invalidt_action_type)1U)

/*!*********************************************************************************************************************
 *  @brief  接收数据超时动作类型
 **********************************************************************************************************************/
typedef uint8 com_rx_data_timeout_action_type;
#define COM_TIMEOUT_DATA_ACTION_NONE ((com_rx_data_timeout_action_type)0U)
#define COM_TIMEOUT_DATA_ACTION_REPLACE ((com_rx_data_timeout_action_type)1U)

/*!*********************************************************************************************************************
 *  @brief  com_self_tx_signal_type
 **********************************************************************************************************************/
#if ((COM_SIG_GATEWAY_ENABLE == STD_ON) || (COM_SIGNAL_GROUP_ENABLE == STD_ON))
typedef struct {
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
	boolean com_signal_updated_gw_routing : 1;
	boolean com_signal_routing_req : 1;
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	void *com_self_shadow_buffer;
#endif
} com_self_tx_signal_type;
#endif

/*!*********************************************************************************************************************
 *  @brief  com_self_rx_signal_type
 **********************************************************************************************************************/
#if ((COM_SIG_GATEWAY_ENABLE == STD_ON) || (COM_SIGNAL_GROUP_ENABLE == STD_ON) ||                                      \
	(COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF))
typedef struct {
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
	boolean com_signal_updated_gw_routing;
	boolean com_signal_routing_req;
#endif
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
	uint32 com_rx_deadline_cnt;
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	void *com_self_shadow_buffer;
#endif
} com_self_rx_signal_type;
#endif

/*!*********************************************************************************************************************
 *  @brief  com_self_tx_ipdu_type
 **********************************************************************************************************************/
typedef struct {
	boolean com_ipdu_started : 1;
	boolean com_ipdu_updated : 1;
	boolean com_txipdu_tx_mode : 1;
	uint8 com_txipdu_repetitions_left_num;
	uint8 com_txipdu_tx_confirmations_num;
	uint32 com_txmode_repet_period_timer;
	uint32 com_txipdu_min_delay_timer;
	uint32 com_txmode_period_timer;
	uint32 com_tx_dm_timer;
	uint32 com_tx_deadline_cnt;
	uint8 *com_ipdu_data_ptr;
} com_self_tx_ipdu_type;

/*!*********************************************************************************************************************
 *  @brief  com_self_rx_ipdu_type
 **********************************************************************************************************************/
typedef struct {
	boolean com_ipdu_started : 1;
	boolean com_ipdu_updated : 1;
	boolean com_rxipdu_dm_ctrl : 1;
	boolean com_self_ipdu_has_ipdu_buff : 1;
	uint8 *com_ipdu_data_ptr;
#if (COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON)
	uint32 com_rx_deadline_cnt;
#endif
} com_self_rx_ipdu_type;

/*!*********************************************************************************************************************
 *  @brief  com_rx_signal_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_self_eol : 1;
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	const boolean com_signal_self_use_update_bit : 1;
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	const boolean com_self_is_signal_group : 1;
	const boolean com_self_is_group_signal : 1;
#endif
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
	const boolean com_sig_gw_routing_req : 1;
#endif
	const boolean com_sig_init_value_is_zero : 1;
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
	const com_data_invalidt_action_type com_data_invalid_action : 1;
#endif
#if (COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE == STD_ON)
	const com_rx_data_timeout_action_type com_rx_data_timeout_action;
#endif
	com_signal_endianess_type com_signal_endianess;
	com_signal_type com_signal_type;
	com_bit_pos_type com_bit_pos;
	com_bit_pos_type com_end_bit_pos;
	uint16 com_bit_size;
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	const com_bit_pos_type com_update_bit_pos;
#endif
	const uint16 com_ipdu_id;
	uint16 com_buff_idx;
#if (COM_ERROR_NOTIFICATION_ENABLE == STD_ON)
	const uint16 com_error_notification;
#endif
	const uint16 com_notification;
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
	const uint16 com_invalid_notification;
#endif
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
	const uint16 com_timeout_notification;
#endif
	const void *com_signal_init_value;
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
	const uint32 com_first_timeout_factor;
	const uint32 com_timeout_factor;
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	const uint16 *com_group_signal;
	const uint8 *com_self_shadow_buffer_mask;
#endif
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
	const void *com_signal_data_invalid_value;
#endif
} com_rx_signal_type;

/*!*********************************************************************************************************************
 *  @brief  com_tx_signal_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_self_eol : 1;
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	const boolean com_signal_self_use_update_bit : 1;
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	const boolean com_self_is_signal_group : 1;
	const boolean com_self_is_group_signal : 1;
#endif
	const boolean com_sig_init_value_is_zero : 1;
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
	const com_data_invalidt_action_type com_data_invalid_action : 1;
#endif
	const com_signal_endianess_type com_signal_endianess : 2;
	const com_signal_type com_signal_type : 4;

	const com_transfer_property_type com_transfer_property : 3;
	const com_bit_pos_type com_bit_pos;
	const com_bit_pos_type com_end_bit_pos;
	const uint16 com_bit_size;
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	const com_bit_pos_type com_update_bit_pos;
#endif
	const uint16 com_ipdu_id;
#if (COM_ERROR_NOTIFICATION_ENABLE == STD_ON)
	const uint16 com_error_notification;
#endif
#if (COM_TXNOTIFICATION == STD_ON)
	const uint16 com_notification;
#endif
#if (COM_TXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
	const uint16 com_timeout_notification;
#endif
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
	const uint16 com_invalid_notification;
#endif
	const void *com_signal_init_value;
#if (COM_TXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
	const uint32 com_first_timeout_factor;
	const uint32 com_timeout_factor;
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	const uint16 *com_group_signal;
	const uint8 *com_self_shadow_buffer_mask;
#endif
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
	const void *com_signal_data_invalid_value;
#endif
} com_tx_signal_type;

/*!*********************************************************************************************************************
 *  @brief  com_tx_mode_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_tx_mode_repetitions_num;
	const com_tx_mode_mode_type com_tx_mode_mode;
	const uint32 com_tx_mode_repetition_period_factor;
	const uint32 com_tx_mode_time_offset_factor;
	const uint32 com_tx_mode_time_period_factor;
} com_tx_mode_type;

/*!*********************************************************************************************************************
 *  @brief  com_ipdu_tx_type
 **********************************************************************************************************************/
typedef struct {
	const boolean enable_retry_failed_transmit_requests;
	const uint8 com_tx_ipdu_unused_areas_default;
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	const com_tx_ipdu_clear_update_bit_type com_tx_ipdu_clear_update_bit;
#endif
	const uint32 com_tx_ipdu_minimum_delay_factor;
	const com_tx_mode_type *com_tx_mode_true;
	const com_tx_mode_type *com_tx_mode_false;
} com_ipdu_tx_type;

/*!*********************************************************************************************************************
 *  @brief  com_ipg_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_self_eol;
	const com_ipdu_group_id_type com_ipg_handle_id;
} com_ipg_type;

/*!*********************************************************************************************************************
 *  @brief  com_rx_ipdu_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_self_eol : 1;
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
	const boolean com_ipdu_gw_routing_req : 1;
#endif
	const boolean com_ipdu_has_signal_group : 1;

	const com_ipdu_signal_processing_mode com_ipdu_signal_processing : 1;
	const PduIdType self_ipdu_outgoing_id;
	const uint16 com_ipdu_size;
	const uint16 signal_start_idx;
	const uint16 signal_end_idx;
#if (COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON)
	const uint16 rx_tout_info_idx;
#endif
	uint16 com_rx_ipdu_callout;
	const com_ipg_type *const com_ipg_refs;
} com_rx_ipdu_type;

/*!*********************************************************************************************************************
 *  @brief  com_tx_ipdu_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_self_eol : 1;

	const com_ipdu_signal_processing_mode com_ipdu_signal_processing : 1;
	const PduIdType self_ipdu_outgoing_id;
	const uint16 com_ipdu_size;
	const uint16 signal_start_idx;
	const uint16 signal_end_idx;
#if (COM_TXTOU_ENABLE == STD_ON) && (COM_TXIPDU_SAME_TIMEOUT_ENABLE == STD_ON)
	const uint16 tx_tout_info_idx;
#endif
	uint16 com_tx_ipdu_callout;
	uint16 com_trigger_transmit_ipdu_callout;
	const com_ipg_type *const com_ipg_refs;
	const com_ipdu_tx_type *com_tx_ipdu;
} com_tx_ipdu_type;

/*!*********************************************************************************************************************
 *  @brief  定义信号网关引用类型
 **********************************************************************************************************************/
typedef uint8 com_gw_signal_ref_type;
#define COM_SIGNAL_REFERENCE ((com_gw_signal_ref_type)0u)
#define COM_GROUP_SIGNAL_REFERENCE ((com_gw_signal_ref_type)1u)

/*!*********************************************************************************************************************
 *  @brief  com_gw_destn_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_self_eol;
	const uint16 com_gw_dest_sig_id;
	const com_gw_signal_ref_type com_gw_dest_sig_ref;
} com_gw_destn_type;

/*!*********************************************************************************************************************
 *  @brief  com_gw_mapping_type
 **********************************************************************************************************************/
typedef struct {
	const uint8 com_self_eol;
	const uint8 com_gw_desitnation_routes_num;
	const uint16 com_gw_src_sig_id;
	const com_gw_signal_ref_type com_gw_src_sig_ref;
	const com_gw_destn_type *com_gw_dest_ref;
} com_gw_mapping_type;

/*!*********************************************************************************************************************
 *  @brief  com_rx_tout_info_type
 **********************************************************************************************************************/
typedef struct {
	const uint16 com_signal_id_ref;
	const uint16 com_ipdu_id_ref;
} com_rx_tout_info_type;

/*!*********************************************************************************************************************
 *  @brief  com_rx_tout_message_info_type
 **********************************************************************************************************************/
typedef struct {
	const uint16 ipdu_id_ref;
	const uint16 start_timeout_noti_idx;
	const uint16 timeout_noti_num;
	const uint32 com_first_timeout_factor;
	const uint32 com_timeout_factor;
} com_rx_tout_message_info_type;

/*!*********************************************************************************************************************
 *  @brief  com_tx_tout_message_info_type
 **********************************************************************************************************************/
typedef struct {
	const uint16 ipdu_id_ref;
	const uint16 start_timeout_noti_idx;
	const uint16 timeout_noti_num;
	const uint32 com_first_timeout_factor;
	const uint32 com_timeout_factor;
} com_tx_tout_message_info_type;

/*!*********************************************************************************************************************
 *  @brief  com_config_type
 **********************************************************************************************************************/
typedef struct {
	const com_gw_mapping_type *com_gw_mapping_ref;
	const com_ipg_type *const com_ipg;
} com_config_type;

/*!*********************************************************************************************************************
 *  @brief  com_main_function_tx_struct_type
 **********************************************************************************************************************/
typedef struct {
	boolean tx_comfirmation_info;
	uint16 start_ipdu_idx_of_main_function_tx;
	uint16 ipdu_num_of_main_function_tx;
} com_main_function_tx_struct_type;

/*!*********************************************************************************************************************
 *  @brief  com_main_function_rx_struct_type
 **********************************************************************************************************************/
typedef struct {
	boolean rx_timeout_info;
	uint16 start_ipdu_idx_of_main_function_rx;
	uint16 ipdu_num_of_main_function_rx;
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON)
	uint16 rx_toutmessag_start_idx;
	uint16 rx_toutmessag_num;
#else
	uint16 rx_tout_signal_start_idx;
	uint16 rx_tout_signal_num;
#endif
	uint16 ipdu_max_sig_num;
	uint16 *rx_notification_record_ref;
} com_main_function_rx_struct_type;

/*!*********************************************************************************************************************
 *  @brief  com_core_map_info_struct_type signal idx
 **********************************************************************************************************************/
typedef struct {
	uint8 core_id;
	uint16 start_tx_ipdu_idx;
	uint16 end_tx_ipdu_idx;
	uint16 start_rx_ipdu_idx;
	uint16 end_rx_ipdu_idx;
	uint8 *tx_buffer_ref;
	uint8 *rx_buffer_ref;
	uint8 *com_rx_sig_buffer_uint8ref;
	uint16 *com_rx_sig_buffer_uint16ref;
	uint32 *com_rx_sig_buffer_uint32ref;
	uint64 *com_rx_sig_buffer_uint64ref;
	uint8 *com_rx_sig_buffer_array_based_ref;
} com_core_map_info_struct_type;

#ifdef __cplusplus
}
#endif

#endif
