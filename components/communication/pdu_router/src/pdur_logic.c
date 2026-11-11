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

/* including file */
#include "pdur.h"
#include <string.h>
#include "SchM_PduR.h"
#include "comstack_types.h"
#include "pdur_evcfifo.h"
#include "pdur_buffermanager.h"
#include "pdur_types.h"
#include "pdur_internal.h"
#include "Compiler.h"
#include "pdur_routingcfg.h"
#include "pdur_internal.h"
#include "pdur_bufferpoolcfg.h"
#include "rtfw_isr.h"
#if defined(__NuttX__)
#include "nuttx/arch.h"
#endif
/* 全局宏定义 */

#if (PDUR_ZERO_COST_OPERATION == STD_OFF)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(boolean, PDUR_CODE) pdur_has_tp_module_dest(const pdur_routing_path_type *route);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_check_buffer_status(const pdur_buffer_manager_type *pdur_buffer_manager_ptr,
						    uint16 length);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_calculate_min_buffer_size(PduLengthType *min_buf_size, const PduLengthType *avbl_buf_size);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(uint8, PDUR_CODE) pdur_get_dest_pdu_idx_in_routing_path(PduIdType src_pdu_id, PduIdType dest_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_buffer_and_transfer(PduIdType pdu_id, PduLengthType tp_sdu_length,
							  PduLengthType *buffer_size_ptr, boolean is_up_module_called);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_buffer_and_transfer_on_the_fly(PduIdType pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_same_core_up_module(PduIdType pdu_id, PduLengthType tp_sdu_length,
							  PduLengthType *buffer_size_ptr, boolean *is_up_module_called);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_lo_tp_start_of_reception_post_proc(const pdur_routing_path_type *routing_path_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_tp_rx_indication_result_is_ok(PduIdType pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_tp_rx_indication_result_is_nok(PduIdType pdu_id, NotifResultType result);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_same_core_up_module(PduIdType pdu_id, const PduInfoType *info,
						    PduLengthType *buffer_size_ptr, boolean *is_up_module_called);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_gateway_on_the_fly(PduIdType src_pdu_id, uint8 dest_pdu_idx, const PduInfoType *info,
						   const RetryInfoType *retry, PduLengthType *available_data_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_buffer_and_transfer(PduIdType pdu_id, const PduInfoType *info,
						    PduLengthType *buffer_size_ptr, boolean is_up_module_called);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_buffer_and_transfer_check(PduIdType pdu_id, boolean is_up_module_called,
							  const pdur_buffer_manager_type *pdur_buffer_manager_ptr,
							  PduLengthType data_len);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_buffer_and_transfer_update_state(PduIdType pdu_id, BufReq_ReturnType ret_val,
								 pdur_buffer_manager_type *pdur_buffer_manager_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_direct_gateway(PduIdType src_pdu_id, uint8 dest_pdu_idx, const PduInfoType *info,
					       const RetryInfoType *retry, PduLengthType *available_data_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_routing_dest_pdu_transmit(PduIdType pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_routing_dest_pdu_transmit_call_func(PduIdType pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_multi_core_append_fifo(PduIdType pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_up_module_call_func(PduLengthType tp_sdu_length, PduLengthType *buffer_size_ptr,
							  const pdur_dest_pdu_type *destination);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_tp_rx_indication_result_is_nok_upper(PduIdType pdu_id, NotifResultType result, uint8 dest_pdu_itr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_tp_rx_indication_result_is_nok_lower(PduIdType pdu_id, uint8 dest_pdu_itr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_tp_rx_indication_result_is_ok_upper(PduIdType pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_tp_rx_indication_result_is_ok_lower(PduIdType pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE)
	pdur_transmit_same_core(const PduInfoType *pdu_info, const pdur_dest_pdu_type *destination);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_buffer_and_transfer(PduIdType src_pdu_id, PduIdType dest_pdu_id,
						    const PduInfoType *info, const RetryInfoType *retry,
						    PduLengthType *available_data_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_same_core_up_module(const pdur_routing_path_type *routing_path_ptr,
						    const PduInfoType *info, RetryInfoType *retry,
						    PduLengthType *available_data_ptr, boolean *is_up_module_called);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_buffer_and_transfer_upper_module(PduIdType src_pdu_id, uint8 dest_pdu_idx,
								 const PduInfoType *info,
								 PduLengthType *available_data_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_tp_tx_confirmation_same_core_upper_module(const pdur_routing_path_type *routing_path_ptr,
						       NotifResultType result, boolean *is_up_module_called);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_transmit_buffer_and_transfer(PduIdType pdu_id, uint8 dest_pdu_idx, const PduInfoType *pdu_info,
					  const pdur_routing_path_type *routing_path_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE)
pdur_rx_indication_check_need_buffer(const pdur_routing_path_type *routing_path_ptr, boolean *all_dest_disable);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_rx_indication_copy_data_into_buffer(PduIdType pdu_id, pdur_buffer_manager_type **pdur_buffer_manager_ptr,
						 const PduInfoType *pdu_info);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_trigger_soft_interrupt(PduIdType pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
#define PDUR_START_SEC_GLOBAL_VAR_NO_INIT_UNSPECIFIED
#include "pdur_memmap.h"
VAR(static boolean, PDUR_GLOBAL_VAR_NO_INIT) g_pdur_routing_group_status[PDUR_ROUTING_GROUP_NUM];
#define PDUR_STOP_SEC_GLOBAL_VAR_NO_INIT_UNSPECIFIED
#include "pdur_memmap.h"
#endif
/* global function implementation */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(PduIdType, PDUR_CODE) pdur_get_src_pdu_id_by_dest_pdu_id(PduIdType dest_pdu_id)
{
	PduIdType src_pdu_id;

	if (dest_pdu_id >= PDU_MAX_ROUTING_PATH) {
		src_pdu_id = (dest_pdu_id & (PduIdType)PDUR_PDU_ID_MASK);
	} else {
		src_pdu_id = dest_pdu_id;
	}
#if (pdur_dest_PDUID_MAPPING_SIZE > 0)
	const pdur_dest_pdu_id_mapping_struct_type *dest_pdu_id_mapping_table = pdur_get_dest_pdu_id_mapping();

	if (dest_pdu_id_mapping_table == NULL) {
		return src_pdu_id;
	}
	uint8 dest_pdu_id_index = 0u;

	for (dest_pdu_id_index = 0u; dest_pdu_id_index < pdur_dest_PDUID_MAPPING_SIZE; dest_pdu_id_index++) {
		if (dest_pdu_id_mapping_table[dest_pdu_id_index].dest_pdu_id == dest_pdu_id) {
			break;
		}
	}
	if (dest_pdu_id_index == pdur_dest_PDUID_MAPPING_SIZE) {
		return src_pdu_id;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	for (uint8 src_pdu_id_index = 0u; src_pdu_id_index < PDUR_N_TO_1_MAX_N_NUM; src_pdu_id_index++) {
		if (dest_pdu_id_mapping_table[dest_pdu_id_index].src_pdu_id[src_pdu_id_index] >=
		    pdur_config_ptr->n_routing_paths) {
			continue;
		}
		src_pdu_id = dest_pdu_id_mapping_table[dest_pdu_id_index].src_pdu_id[src_pdu_id_index];
		const pdur_buffer_manager_type *pdur_buffer_manager_ptr =
			pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
		if (pdur_buffer_manager_ptr == NULL) {
			continue;
		}
		const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[src_pdu_id];

		for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
			if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_ROUTING_IDLE) {
				return src_pdu_id;
			}
		}
	}
#endif
	return src_pdu_id;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(uint8, PDUR_CODE) pdur_get_dest_pdu_idx_in_routing_path(PduIdType src_pdu_id, PduIdType dest_pdu_id)
{
#if (PDUR_SRC_PDUID_MAPPING_SIZE > 0)
	uint8 dest_pdu_id_index = 0u;
	const pdur_src_pdu_id_mapping_struct_type *src_pdu_id_mapping_table = pdur_get_src_pdu_id_mapping();

	if (src_pdu_id_mapping_table == NULL) {
		return 0u;
	}
	uint8 src_pdu_id_index = 0u;

	for (; src_pdu_id_index < PDUR_SRC_PDUID_MAPPING_SIZE; src_pdu_id_index++) {
		if (src_pdu_id_mapping_table[src_pdu_id_index].src_pdu_id == src_pdu_id) {
			break;
		}
	}
	if (src_pdu_id_index == PDUR_SRC_PDUID_MAPPING_SIZE) {
		return 0u;
	}
	for (; dest_pdu_id_index < PDUR_MAX_GW_DESTINATIONS; dest_pdu_id_index++) {
		if (dest_pdu_id == src_pdu_id_mapping_table[src_pdu_id_index].dest_pdu_id[dest_pdu_id_index]) {
			return dest_pdu_id_index;
		}
	}
#endif
	return 0u;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE) pdur_lo_tp_transmit_failed_process(PduIdType src_pdu_id, uint8 dest_pdu_idx)
{
	BufReq_ReturnType ret_val = BUFREQ_E_NOT_OK;
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRANSMITFAILEDPROCESS_BUFFER_MANAGER_NULL);
		return BUFREQ_E_NOT_OK;
	}
	if (pdur_buffer_manager_ptr->buf_addr == NULL) {
		for (uint8 itr = 0u; itr < PDUR_MAX_GW_DESTINATIONS; itr++) {
			pdur_buffer_manager_ptr->routing_state[itr] = PDUR_ROUTING_IDLE;
		}
	}
	if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_TX_READY) ||
	    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_TX_ONGOING) ||
	    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_READY) ||
	    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY) ||
	    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_READY) ||
	    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_TX_READY)) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
		ret_val = BUFREQ_OK;
	}
	do {
		pdur_check_pending_dst_cnt_reached(src_pdu_id, (boolean)TRUE);
	} while (pdur_release_buffer_check_transmit(src_pdu_id) == E_NOT_OK);
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_check_buffer_status(const pdur_buffer_manager_type *pdur_buffer_manager_ptr,
						    uint16 length)
{
	if (pdur_buffer_manager_ptr->buf_addr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYRXDATA_BUFFER_BLOCK_NULL);
		return BUFREQ_E_NOT_OK;
	}
	if (pdur_buffer_manager_ptr->buf_length < pdur_buffer_manager_ptr->buffer_write_offset) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYRXDATA_WRITE_OFFSET_ERROR);
		return BUFREQ_E_NOT_OK;
	}
	if (length > (pdur_buffer_manager_ptr->buf_length - pdur_buffer_manager_ptr->buffer_write_offset)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYRXDATA_BUFFER_NOT_ENOUGH);
		return BUFREQ_E_OVFL;
	}
	return BUFREQ_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE) pdur_release_buffer(PduIdType pdu_id, boolean is_queue_first)
{
	Std_ReturnType buffer_free_result = pdur_free_buf_mager_by_src_pdu_id(pdu_id, is_queue_first);

	if (E_OK != buffer_free_result) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RELEASEBUFFER_FREE_BUFFER_BLOCK_FAILED);
		return BUFREQ_E_NOT_OK;
	}
	if (is_queue_first == (boolean)FALSE) {
		return BUFREQ_OK;
	}
	return BUFREQ_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_buffer_check_transmit(PduIdType src_pdu_id)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	uint8 queue_length = pdur_get_buf_mager_queue_length(src_pdu_id);

	if (queue_length == 0u) {
		return E_OK;
	}
	pdur_buffer_manager_type *first_node = pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);

	if (first_node == NULL) {
		return E_OK;
	}
	Std_ReturnType total_ret_val = E_OK;

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < pdur_config_ptr->routing_paths[src_pdu_id].pdur_destinatn_num;
	     dest_pdu_idx++) {
		if (pdur_check_if_buf_mager_transmit_allowed_in_queue(src_pdu_id, dest_pdu_idx) == (boolean)FALSE) {
			continue;
		}
		Std_ReturnType ret_val = E_OK;
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
		if (first_node->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_FINISH) {
			first_node->routing_state[dest_pdu_idx] = PDUR_DIAG_UPPERLAYER_TX_READY;
			ret_val = pdur_multi_core_append_fifo(src_pdu_id, dest_pdu_idx);
			if (ret_val != E_OK) {
				total_ret_val = E_NOT_OK;
				first_node->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RELEASEBUFFER_APPEND_FIFO_UPPER);
			}
			continue;
		}
#endif
		if (first_node->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_RX_FINISH) {
			first_node->routing_state[dest_pdu_idx] = PDUR_GATEWAY_DIRECT_TX_READY;
		}
		ret_val = pdur_routing_dest_pdu_transmit(src_pdu_id, dest_pdu_idx);
		if (ret_val != E_OK) {
			total_ret_val = E_NOT_OK;
			first_node->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
			PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RELEASEBUFFER_TRANSMIT_LOWER);
		}
	}
	return total_ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_transmit(PduIdType pdu_id, const PduInfoType *pdu_info, uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRANSMIT_NOT_INIT);
		return E_NOT_OK;
	}
	if (NULL == pdu_info) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRANSMIT_PDU_INFO_PTR_NULL);
		return E_NOT_OK;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRANSMIT_PDU_ID_INVALID);
		return E_NOT_OK;
	}
	Std_ReturnType total_ret_val = E_NOT_OK;
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_idx];

		if (pdur_check_routing_path_enabled(destination) == (boolean)FALSE) {
			continue;
		}
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
		if (destination->pdur_dest_pdu_core == (pdur_core_id_type)GetCoreID()) {
#endif
			Std_ReturnType ret_val = pdur_transmit_same_core(pdu_info, destination);

			if (ret_val == E_OK) {
				total_ret_val = E_OK;
			} else {
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRANSMIT_SAME_CORE_TRANSMIT_ERROR);
			}
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
		} else {
			BufReq_ReturnType buf_ret_val =
				pdur_transmit_buffer_and_transfer(pdu_id, dest_pdu_idx, pdu_info, routing_path_ptr);
			if (buf_ret_val == BUFREQ_OK) {
				total_ret_val = E_OK;
			} else {
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRANSMIT_BUFFER_TRANSMIT_ERROR);
			}
		}
#endif
	}
	return total_ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE)
	pdur_transmit_same_core(const PduInfoType *pdu_info, const pdur_dest_pdu_type *destination)
{
	Std_ReturnType ret_val = E_NOT_OK;
	pdur_dest_pdu_len_strategy_type handle_strategy =
		pdur_check_and_handle_dest_pdu_len_strategy(destination, pdu_info);
	if (handle_strategy == PDUR_STRATEGY_DISCARD) {
		ret_val = E_NOT_OK;
	} else if (handle_strategy == PDUR_STRATEGY_SHORTEN) {
		PduInfoType pdu_info_temp;
		(void)memcpy(&pdu_info_temp, pdu_info, sizeof(PduInfoType));
		pdu_info_temp.SduLength = destination->pdu_len_in_ecuc;
		ret_val = pdur_route_transmit(destination, &pdu_info_temp);
	} else {
		ret_val = pdur_route_transmit(destination, pdu_info);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_transmit_buffer_and_transfer(PduIdType pdu_id, uint8 dest_pdu_idx, const PduInfoType *pdu_info,
					  const pdur_routing_path_type *routing_path_ptr)
{
	if (pdur_has_tp_module_dest(routing_path_ptr) == (boolean)FALSE) {
		return BUFREQ_OK;
	}
	if (pdur_get_buf_mager_queue_length(pdu_id) != 0u) {
		return BUFREQ_E_BUSY;
	}
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_alloc_buffer_manager_by_src_pdu_id(pdu_id, pdu_info->SduLength);
	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRANSMIT_ALLOC_BUFFER_BUSY);
		return BUFREQ_E_BUSY;
	}
	pdur_buffer_manager_ptr->pdu_length = pdu_info->SduLength;
	pdur_buffer_manager_ptr->src_pdu_id = pdu_id;
	PduLengthType available_data = 0u;
	PduInfoType input_pdu_info;

	input_pdu_info.SduDataPtr = pdur_buffer_manager_ptr->buf_addr;
	input_pdu_info.SduLength = pdu_info->SduLength;
	BufReq_ReturnType ret_val = pdur_route_copy_tx_data(routing_path_ptr, &input_pdu_info, NULL, &available_data);

	if (ret_val != BUFREQ_OK) {
		pdur_route_tp_tx_confirmation(routing_path_ptr, NTFRSLT_E_NOT_OK);
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
		pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)FALSE);
	} else {
		pdur_route_tp_tx_confirmation(routing_path_ptr, NTFRSLT_OK);
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_DIAG_UPPERLAYER_TX_READY;
		pdur_buffer_manager_ptr->buffer_write_offset = pdu_info->SduLength;
		Std_ReturnType s_ret_val = pdur_multi_core_append_fifo(pdu_id, dest_pdu_idx);

		if (s_ret_val != E_OK) {
			ret_val = BUFREQ_E_NOT_OK;
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
			pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)FALSE);
		}
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_tp_rx_indication_result_is_nok_upper(PduIdType pdu_id, NotifResultType result, uint8 dest_pdu_itr)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_itr];
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	if (destination->pdur_dest_pdu_core == (pdur_core_id_type)GetCoreID()) {
#endif
		pdur_route_tp_rx_indication(destination, result);
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	} else {
		pdur_buffer_manager_type *pdur_buffer_manager_ptr =
			pdur_find_last_buf_mager_in_queue_by_src_pdu_id(pdu_id);
		if (pdur_buffer_manager_ptr == NULL) {
			PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_BUFFER_ERROR_NOK_UPPER);
			return;
		}
		pdur_buffer_manager_ptr->routing_state[dest_pdu_itr] = PDUR_ROUTING_IDLE;
	}
#endif
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_tp_rx_indication_result_is_nok_lower(PduIdType pdu_id, uint8 dest_pdu_itr)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	pdur_buffer_manager_type *pdur_buffer_manager_ptr = pdur_find_last_buf_mager_in_queue_by_src_pdu_id(pdu_id);

	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_BUFFER_ERROR_NOK_LOWER);
		return;
	}
	if (routing_path_ptr->pdur_direct_gateway == PDUR_GATEWAY_DIRECT) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_itr] = PDUR_ROUTING_IDLE;
	} else {
		if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_itr] == PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) ||
		    (pdur_buffer_manager_ptr->routing_state[dest_pdu_itr] ==
		     PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT)) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_itr] = PDUR_ROUTING_IDLE;
		} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_itr] == PDUR_ROUTING_IDLE) {
		} else {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_itr] = PDUR_GATEWAY_ONTHEFLY_FAILED;
		}
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_tp_rx_indication_result_is_nok(PduIdType pdu_id, NotifResultType result)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];

	for (uint8 dest_pdu_itr = 0u; dest_pdu_itr < routing_path_ptr->pdur_destinatn_num; dest_pdu_itr++) {
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_itr];

		if (pdur_is_up_module(destination->dest_module) == (boolean)TRUE) {
			pdur_tp_rx_indication_result_is_nok_upper(pdu_id, result, dest_pdu_itr);
		} else {
			pdur_tp_rx_indication_result_is_nok_lower(pdu_id, dest_pdu_itr);
		}
	}
	pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)FALSE);
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_routing_dest_pdu_transmit_call_func(PduIdType pdu_id, uint8 dest_pdu_idx)
{
	PduInfoType up_buffer;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_dest_pdu_type *destination = pdur_config_ptr->routing_paths[pdu_id].pdur_dest_pdus[dest_pdu_idx];
	pdur_buffer_manager_type *pdur_buffer_manager_ptr = pdur_find_first_buf_mager_in_queue_by_src_pdu_id(pdu_id);

	if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
		return E_NOT_OK;
	}
	up_buffer.SduDataPtr = pdur_buffer_manager_ptr->buf_addr;
	up_buffer.SduLength = pdur_buffer_manager_ptr->pdu_length;
	if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_READY;
	} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY;
	} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_WAIT) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_READY;
	} else {
	}
	Std_ReturnType route_state = pdur_route_transmit(destination, &up_buffer);

	if (route_state == E_OK) {
		if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
				PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_ONGOING;
		} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			   PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_READY) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
				PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_ONGOING;
		} else {
		}
	}
	return route_state;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_multi_core_append_fifo(PduIdType pdu_id, uint8 dest_pdu_idx)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_dest_pdu_type *destination = pdur_config_ptr->routing_paths[pdu_id].pdur_dest_pdus[dest_pdu_idx];
	Std_ReturnType ret_val = E_OK;
	pdur_evc_data_type evc_fifo_data;
	(void)memset(&evc_fifo_data, 0, (uint32)sizeof(pdur_evc_data_type));
	evc_fifo_data.src_pdu_id = pdu_id;
	evc_fifo_data.dest_pdu_idx = dest_pdu_idx;
	Std_ReturnType ev_c_fifo_push_retval =
		pdur_evc_fifo_push(destination->pdur_dest_pdu_core, &evc_fifo_data, PDUR_ASYNC_EVCFIFO);
	if (ev_c_fifo_push_retval == E_NOT_OK) {
		ret_val = E_NOT_OK;
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_routing_dest_pdu_transmit(PduIdType pdu_id, uint8 dest_pdu_idx)
{
	Std_ReturnType ret_val = E_OK;
	const pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(pdu_id);
	if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
		return E_NOT_OK;
	}
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_dest_pdu_type *destination = pdur_config_ptr->routing_paths[pdu_id].pdur_dest_pdus[dest_pdu_idx];

	if (destination->pdur_dest_pdu_core == (pdur_core_id_type)GetCoreID()) {
#endif
		ret_val = pdur_routing_dest_pdu_transmit_call_func(pdu_id, dest_pdu_idx);
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	} else {
		ret_val = pdur_multi_core_append_fifo(pdu_id, dest_pdu_idx);
	}
#endif
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_tp_rx_indication_result_is_ok_upper(PduIdType pdu_id, uint8 dest_pdu_idx)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_dest_pdu_type *destination = pdur_config_ptr->routing_paths[pdu_id].pdur_dest_pdus[dest_pdu_idx];
	Std_ReturnType ret_val = E_OK;
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	if (destination->pdur_dest_pdu_core == (pdur_core_id_type)GetCoreID()) {
#endif
		pdur_route_tp_rx_indication(destination, NTFRSLT_OK);
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	} else {
		pdur_buffer_manager_type *pdur_buffer_manager_ptr =
			pdur_find_last_buf_mager_in_queue_by_src_pdu_id(pdu_id);
		if (pdur_buffer_manager_ptr == NULL) {
			PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_BUFFER_ERROR);
			return E_NOT_OK;
		}
		if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_ONGOING) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_DIAG_UPPERLAYER_RX_FINISH;
		}
		if (pdur_get_buf_mager_queue_length(pdu_id) == 1u) {
			ret_val = pdur_multi_core_append_fifo(pdu_id, dest_pdu_idx);
			if (ret_val != E_OK) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
				pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)TRUE);
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_APPEND_FIFO_FAILED);
			} else {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_DIAG_UPPERLAYER_TX_READY;
			}
		}
	}
#endif
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(Std_ReturnType, PDUR_CODE) pdur_tp_rx_indication_result_is_ok_lower(PduIdType pdu_id, uint8 dest_pdu_idx)
{
	pdur_buffer_manager_type *pdur_buffer_manager_ptr = pdur_find_last_buf_mager_in_queue_by_src_pdu_id(pdu_id);
	Std_ReturnType ret_val = E_NOT_OK;

	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_BUFFER_ERROR_LOWER);
		return ret_val;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];

	if (routing_path_ptr->pdur_direct_gateway == PDUR_GATEWAY_DIRECT) {
		if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_RX_ONGOING) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_DIRECT_RX_FINISH;
		}
		if (pdur_get_buf_mager_queue_length(pdu_id) == 1u) {
			if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_RX_FINISH) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_DIRECT_TX_READY;
			}
			ret_val = pdur_routing_dest_pdu_transmit(pdu_id, dest_pdu_idx);
		} else {
			ret_val = E_OK;
		}
	} else if (routing_path_ptr->pdur_tp_threshld > pdur_buffer_manager_ptr->buffer_write_offset) {
		if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_WAIT;
			if (pdur_get_buf_mager_queue_length(pdu_id) == 1u) {
				ret_val = pdur_routing_dest_pdu_transmit(pdu_id, dest_pdu_idx);
			}
		}
	} else {
		if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
		    PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_ONGOING) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
				PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_ONGOING;
		} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			   PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_READY;
		} else {
		}
		ret_val = E_OK;
	}
	if (ret_val != E_OK) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_TRANSMIT_FAILED_LOWER);
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_tp_rx_indication_result_is_ok(PduIdType pdu_id)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	Std_ReturnType total_ret_val = E_OK;

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		Std_ReturnType ret_val = E_OK;
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_idx];

		if (pdur_is_up_module(destination->dest_module)) {
			ret_val = pdur_tp_rx_indication_result_is_ok_upper(pdu_id, dest_pdu_idx);
		} else {
			ret_val = pdur_tp_rx_indication_result_is_ok_lower(pdu_id, dest_pdu_idx);
		}
		if (ret_val != E_OK) {
			total_ret_val = E_NOT_OK;
		}
	}
	if (total_ret_val != E_OK) {
		pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)FALSE);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_rx_indication(PduIdType pdu_id, const PduInfoType *pdu_info, uint8 service_id)
{
	(void)service_id;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RXINDICATION_NOT_INIT);
		return;
	}
	if (pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RXINDICATION_PDU_ID_INVALID);
		return;
	}
	boolean all_dest_disable = (boolean)TRUE;
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	boolean need_buffer = pdur_rx_indication_check_need_buffer(routing_path_ptr, &all_dest_disable);

	if (all_dest_disable == (boolean)TRUE) {
		return;
	}
	pdur_buffer_manager_type *pdur_buffer_manager_ptr = NULL;

	if (need_buffer == (boolean)TRUE) {
		pdur_rx_indication_copy_data_into_buffer(pdu_id, &pdur_buffer_manager_ptr, pdu_info);
	}
	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_idx];

		if (pdur_check_routing_path_enabled(destination) == (boolean)FALSE) {
			continue;
		}
		if (pdur_is_up_module(destination->dest_module)) {
			pdur_route_rx_indication(destination, pdu_info);
		} else if (pdur_is_lo_module(destination->dest_module)) {
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
			if (destination->pdur_dest_pdu_core != (pdur_core_id_type)GetCoreID()) {
				if (pdur_buffer_manager_ptr == NULL) {
					PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RXINDICATION_IF_ROUTING_NO_BUFFER);
					continue;
				}
				Std_ReturnType trigger_ret_val = pdur_trigger_soft_interrupt(pdu_id, dest_pdu_idx);

				if (trigger_ret_val != E_OK) {
					PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RXINDICATION_RX_INTERRUPT_FAILED);
					continue;
				}
				if (destination->data_provision == PDUR_TRIGGERTRANSMIT) {
					pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
						PDUR_GATEWAY_IF_TT_TX_FIFO_APPEND;
				} else {
					pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
						PDUR_GATEWAY_IF_DIRECT_TX_FIFO_APPEND;
				}
			} else
#endif
			{
				pdur_dest_pdu_len_strategy_type handle_strategy =
					pdur_check_and_handle_dest_pdu_len_strategy(destination, pdu_info);
				Std_ReturnType ret_val = E_OK;

				if (handle_strategy == PDUR_STRATEGY_DISCARD) {
					PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RXINDICATION_STRATEGY_DISCARD);
					ret_val = E_NOT_OK;
				} else if (handle_strategy == PDUR_STRATEGY_SHORTEN) {
					PduInfoType pdu_info_temp;
					(void)memcpy(&pdu_info_temp, pdu_info, sizeof(PduInfoType));
					pdu_info_temp.SduLength = destination->pdu_len_in_ecuc;
					ret_val = pdur_route_transmit(destination, &pdu_info_temp);
				} else {
					ret_val = pdur_route_transmit(destination, pdu_info);
				}
				if (destination->data_provision == PDUR_TRIGGERTRANSMIT) {
					if (ret_val == E_OK) {
						pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
							PDUR_GATEWAY_IF_TT_TX_FIFO_APPEND;
					}
				}
			}
		} else {
			/* do nothing */
		}
	}
	if (pdur_buffer_manager_ptr != NULL) {
		pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)TRUE);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_trigger_soft_interrupt(PduIdType pdu_id, uint8 dest_pdu_idx)
{
	pdur_evc_data_type evc_fifo_data;
	(void)memset(&evc_fifo_data, 0, sizeof(pdur_evc_data_type));
	evc_fifo_data.src_pdu_id = pdu_id;
	evc_fifo_data.dest_pdu_idx = dest_pdu_idx;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_idx];
	pdur_core_id_type dest_core_id = destination->pdur_dest_pdu_core;
	Std_ReturnType ev_c_fifo_push_retval = pdur_evc_fifo_push(dest_core_id, &evc_fifo_data, PDUR_SYNC_EVCFIFO);

	if (ev_c_fifo_push_retval == E_NOT_OK) {
		return E_NOT_OK;
	}
	const OS_ISRID_DEF *isr_source_table = pdur_get_isr_table();

	if (isr_source_table == NULL) {
		return E_NOT_OK;
	}
	if (dest_core_id >= pdur_get_isr_table_len()) {
		return E_NOT_OK;
	}
	if (isr_source_table[dest_core_id] == OS_ISR_CNT) {
		return E_NOT_OK;
	}
	const os_isr_cfg_t *isr_cfg = os_isr_cfg_ref_table[isr_source_table[dest_core_id]];

	up_trigger_irq(isr_cfg->arch_isr_cfg.vector_num, (1 << dest_core_id));
	return E_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE)
pdur_rx_indication_check_need_buffer(const pdur_routing_path_type *routing_path_ptr, boolean *all_dest_disable)
{
	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_idx];

		if (pdur_check_routing_path_enabled(destination) == (boolean)FALSE) {
			continue;
		}
		*all_dest_disable = (boolean)FALSE;
		if (pdur_is_up_module(destination->dest_module) == (boolean)TRUE) {
			continue;
		}
		if (destination->data_provision == PDUR_TRIGGERTRANSMIT) {
			return (boolean)TRUE;
		}
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
		if (destination->pdur_dest_pdu_core != (pdur_core_id_type)GetCoreID()) {
			return (boolean)TRUE;
		}
#endif
	}
	return (boolean)FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_rx_indication_copy_data_into_buffer(PduIdType pdu_id, pdur_buffer_manager_type **pdur_buffer_manager_ptr,
						 const PduInfoType *pdu_info)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];

	if (routing_path_ptr->pdur_tx_buffer_idx == PDUR_NO_BUFFER) {
		(*pdur_buffer_manager_ptr) = pdur_alloc_buffer_manager_by_src_pdu_id(pdu_id, pdu_info->SduLength);
		if ((*pdur_buffer_manager_ptr) == NULL) {
			PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RXINDICATION_RX_INDICATION_ALLOC_BUFFER);
			return;
		}
	} else {
		pdur_buffer_manager_type *pdur_buffer_manager_ptr_list = pdur_get_tx_buffer_manager_list_addr();
		(*pdur_buffer_manager_ptr) = &pdur_buffer_manager_ptr_list[routing_path_ptr->pdur_tx_buffer_idx];
	}
	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < PDUR_MAX_GW_DESTINATIONS; dest_pdu_idx++) {
		(*pdur_buffer_manager_ptr)->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
	}
	(void)memcpy((*pdur_buffer_manager_ptr)->buf_addr, pdu_info->SduDataPtr, pdu_info->SduLength);
	(*pdur_buffer_manager_ptr)->pdu_length = pdu_info->SduLength;
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_tp_tx_confirmation_same_core_upper_module(const pdur_routing_path_type *routing_path_ptr,
						       NotifResultType result, boolean *is_up_module_called)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	if (routing_path_ptr->pdur_src_pdu_core == (pdur_core_id_type)GetCoreID())
#endif
	{
		pdur_route_tp_tx_confirmation(routing_path_ptr, result);
		*is_up_module_called = (boolean)TRUE;
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_tp_tx_confirmation_buffer_and_transfer(PduIdType src_pdu_id, PduIdType dest_pdu_id)
{
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPTXCONFIRMATION_ALLOC_BUFFER);
		return;
	}
	uint8 dest_pdu_idx = pdur_get_dest_pdu_idx_in_routing_path(src_pdu_id, dest_pdu_id);

	pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
	do {
		pdur_check_pending_dst_cnt_reached(src_pdu_id, (boolean)TRUE);
	} while (pdur_release_buffer_check_transmit(src_pdu_id) == E_NOT_OK);
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_tp_tx_confirmation(PduIdType dest_pdu_id, NotifResultType result, uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPTXCONFIRMATION_NOT_INIT);
		return;
	}
	PduIdType source_pdu_id = pdur_get_src_pdu_id_by_dest_pdu_id(dest_pdu_id);
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (source_pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPTXCONFIRMATION_PDU_ID_INVALID);
		return;
	}
	boolean is_up_module_called = (boolean)FALSE;
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[source_pdu_id];

	if (pdur_is_up_module(routing_path_ptr->src_module)) {
		pdur_tp_tx_confirmation_same_core_upper_module(routing_path_ptr, result, &is_up_module_called);
	}
	if (pdur_is_lo_module(routing_path_ptr->src_module) || (is_up_module_called == (boolean)FALSE)) {
		pdur_tp_tx_confirmation_buffer_and_transfer(source_pdu_id, dest_pdu_id);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_check_pending_dst_cnt_reached(PduIdType source_pdu_id, boolean is_queue_first)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[source_pdu_id];
	const pdur_buffer_manager_type *pdur_buffer_manager_ptr = NULL;

	if (is_queue_first == (boolean)TRUE) {
		pdur_buffer_manager_ptr = pdur_find_first_buf_mager_in_queue_by_src_pdu_id(source_pdu_id);
	} else {
		pdur_buffer_manager_ptr = pdur_find_last_buf_mager_in_queue_by_src_pdu_id(source_pdu_id);
	}
	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_CHECKPENDINGDSTCNT_ALLOC_BUFFER);
		return;
	}
	boolean release_flag = (boolean)TRUE;

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_ROUTING_IDLE) {
			release_flag = (boolean)FALSE;
		}
	}
	if (release_flag == (boolean)TRUE) {
		(void)pdur_release_buffer(source_pdu_id, is_queue_first);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_tx_confirmation(PduIdType pdu_id, uint8 result, uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TXCONFIRMATION_NOT_INIT);
		return;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	PduIdType src_pdu_id = pdu_id & (PduIdType)0x0fff;

	if (src_pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TXCONFIRMATION_PDU_ID_INVALID);
		return;
	}
	const pdur_routing_path_type *route = &pdur_config_ptr->routing_paths[src_pdu_id];

	if (pdur_is_up_module(route->src_module)) {
		pdur_route_tx_confirmation(route, (NotifResultType)result);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_trigger_transmit(PduIdType pdu_id, PduInfoType *pdu_info, uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRIGGERTRANSMIT_NOT_INIT);
		return E_NOT_OK;
	}
	if (pdu_info == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRIGGERTRANSMIT_PDU_INFO_PTR_NULL);
		return E_NOT_OK;
	}
	PduIdType src_pdu_id = pdur_get_src_pdu_id_by_dest_pdu_id(pdu_id);
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (src_pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRIGGERTRANSMIT_PDU_ID_INVALID);
		return E_NOT_OK;
	}
	Std_ReturnType ret_val = E_NOT_OK;
	const pdur_routing_path_type *route = &pdur_config_ptr->routing_paths[src_pdu_id];
	uint8 dest_pdu_idx = pdur_get_dest_pdu_idx_in_routing_path(src_pdu_id, pdu_id);
	const pdur_dest_pdu_type *destination = route->pdur_dest_pdus[dest_pdu_idx];

	if (pdur_is_up_module(route->src_module)) {
		ret_val = pdur_route_trigger_transmit(route, pdu_info);
		return ret_val;
	}
	if (pdur_is_lo_module(route->src_module)) {
		if (PDUR_TRIGGERTRANSMIT != destination->data_provision) {
			PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRIGGERTRANSMIT_TYPE_ERROR);
			return E_NOT_OK;
		}
		if (pdur_check_routing_path_enabled(destination) == (boolean)FALSE) {
			return E_NOT_OK;
		}
		pdur_buffer_manager_type *pdur_buffer_manager_ptr = pdur_find_buf_mager_by_src_pdu_id(src_pdu_id);

		if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
			PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TRIGGERTRANSMIT_BUFFER_ERROR);
			return E_NOT_OK;
		}
		(void)memcpy((void *)pdu_info->SduDataPtr, (void *)pdur_buffer_manager_ptr->buf_addr,
			     pdu_info->SduLength);
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
		ret_val = E_OK;
	}
	pdur_check_pending_dst_cnt_reached(src_pdu_id, (boolean)TRUE);
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(boolean, PDUR_CODE) pdur_has_tp_module_dest(const pdur_routing_path_type *route)
{
	for (uint32 i = 0; i < route->pdur_destinatn_num; i++) {
		if (pdur_is_tp_module(route->pdur_dest_pdus[i]->dest_module)) {
			return (boolean)TRUE;
		}
	}
	return (boolean)FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE)
pdur_up_change_parameter(PduIdType pdu_id, TPParameterType parameter, uint16 value, uint8 service_id)
{
	(void)pdu_id;
	(void)parameter;
	(void)value;
	(void)service_id;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_calculate_min_buffer_size(PduLengthType *min_buf_size, const PduLengthType *avbl_buf_size)
{
	if (*min_buf_size != 0U) {
		if (*avbl_buf_size < *min_buf_size) {
			*min_buf_size = *avbl_buf_size;
		}
	} else {
		*min_buf_size = *avbl_buf_size;
	}
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_same_core_up_module(PduIdType pdu_id, const PduInfoType *info,
						    PduLengthType *buffer_size_ptr, boolean *is_up_module_called)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	BufReq_ReturnType ret_val = BUFREQ_OK;
	boolean has_up_tp_module = (boolean)FALSE;

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_idx];

		if (pdur_check_routing_path_enabled(destination) == (boolean)FALSE) {
			continue;
		}
		if (pdur_is_up_module(destination->dest_module) == (boolean)FALSE) {
			continue;
		}
		has_up_tp_module = (boolean)TRUE;
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
		if (destination->pdur_dest_pdu_core == (pdur_core_id_type)GetCoreID())
#endif
		{
			PduLengthType buf_size = 0u;

			ret_val = pdur_route_copy_rx_data(destination, info, &buf_size);
			if (ret_val == BUFREQ_OK) {
				pdur_calculate_min_buffer_size(buffer_size_ptr, &buf_size);
			}
			*is_up_module_called = (boolean)TRUE;
		}
		break;
	}
	if (has_up_tp_module == (boolean)FALSE) {
		*is_up_module_called = (boolean)TRUE;
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_buffer_and_transfer(PduIdType pdu_id, const PduInfoType *info,
						    PduLengthType *buffer_size_ptr, boolean is_up_module_called)
{
	pdur_buffer_manager_type *pdur_buffer_manager_ptr = pdur_find_last_buf_mager_in_queue_by_src_pdu_id(pdu_id);

	if (pdur_buffer_manager_ptr == NULL) {
		return BUFREQ_OK;
	}
	BufReq_ReturnType ret_val =
		pdur_lo_tp_copy_rx_data_check_buffer_status(pdur_buffer_manager_ptr, info->SduLength);
	if (ret_val == BUFREQ_OK) {
		ret_val = pdur_lo_tp_copy_rx_data_buffer_and_transfer_check(pdu_id, is_up_module_called,
									    pdur_buffer_manager_ptr, info->SduLength);
	}
	if (ret_val == BUFREQ_OK) {
		ret_val = pdur_buffer_and_transfer_copy_data_into_ring_buffer(pdur_buffer_manager_ptr, info,
									      buffer_size_ptr);
	}
	pdur_lo_tp_copy_rx_data_buffer_and_transfer_update_state(pdu_id, ret_val, pdur_buffer_manager_ptr);
	if (ret_val != BUFREQ_OK) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTECOPYRXDATA_PDUR_E_NULL_POINTER);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_buffer_and_transfer_check(PduIdType pdu_id, boolean is_up_module_called,
							  const pdur_buffer_manager_type *pdur_buffer_manager_ptr,
							  PduLengthType data_len)
{
	(void)data_len;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		if ((pdur_is_up_module(routing_path_ptr->pdur_dest_pdus[dest_pdu_idx]->dest_module)) &&
		    (is_up_module_called == (boolean)FALSE)) {
			if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_READY) ||
			    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_ONGOING)) {
				return BUFREQ_OK;
			}
		} else if (pdur_is_tp_module(routing_path_ptr->pdur_dest_pdus[dest_pdu_idx]->dest_module) ==
			   (boolean)TRUE) {
			if (routing_path_ptr->pdur_direct_gateway == PDUR_GATEWAY_DIRECT) {
				if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
				     PDUR_GATEWAY_DIRECT_RX_READY) ||
				    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
				     PDUR_GATEWAY_DIRECT_RX_ONGOING)) {
					return BUFREQ_OK;
				}
			} else {
				if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_ROUTING_IDLE) &&
				    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] !=
				     PDUR_GATEWAY_ONTHEFLY_FAILED)) {
					return BUFREQ_OK;
				}
			}
		} else {
		}
	}
	return BUFREQ_E_NOT_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE)
	pdur_lo_tp_copy_rx_data_buffer_and_transfer_update_state(PduIdType pdu_id, BufReq_ReturnType ret_val,
								 pdur_buffer_manager_type *pdur_buffer_manager_ptr)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	boolean is_idle_setted = (boolean)FALSE;

	if (ret_val == BUFREQ_OK) {
		for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
			if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_READY) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_DIAG_UPPERLAYER_RX_ONGOING;
			}
			if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_RX_READY) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_DIRECT_RX_ONGOING;
			}
			if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			    PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
					PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT;
			}
			if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			    PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_READY) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
					PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY;
			}
			if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) &&
			    (pdur_buffer_manager_ptr->buffer_write_offset >= routing_path_ptr->pdur_tp_threshld)) {
				Std_ReturnType single_ret_val = E_OK;

				if (pdur_get_buf_mager_queue_length(pdu_id) == 1u) {
					single_ret_val = pdur_routing_dest_pdu_transmit(pdu_id, dest_pdu_idx);
				}
				if (single_ret_val != E_OK) {
					pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
					is_idle_setted = (boolean)TRUE;
					PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYRXDATA_BUFFER_TRANSFER_FAILED);
				}
			}
		}
	} else {
		for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
			if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_READY) ||
			    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_ONGOING) ||
			    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_RX_READY) ||
			    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_RX_ONGOING)) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
				is_idle_setted = (boolean)TRUE;
			}
			if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) ||
			    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT)) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
				is_idle_setted = (boolean)TRUE;
			}
		}
	}
	if (is_idle_setted == (boolean)TRUE) {
		pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)FALSE);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_lo_tp_copy_rx_data(PduIdType pdu_id, const PduInfoType *info, PduLengthType *buffer_size_ptr, uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYRXDATA_NOT_INIT);
		return BUFREQ_E_NOT_OK;
	}
	if ((info == NULL) || (buffer_size_ptr == NULL)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYRXDATA_PDU_INFO_PTR_NULL);
		return BUFREQ_E_NOT_OK;
	}
	if (pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYRXDATA_PDU_ID_INVALID);
		return BUFREQ_E_NOT_OK;
	}
	boolean is_up_module_called = (boolean)FALSE;
	BufReq_ReturnType ret_val =
		pdur_lo_tp_copy_rx_data_same_core_up_module(pdu_id, info, buffer_size_ptr, &is_up_module_called);
	if (ret_val != BUFREQ_OK) {
		return ret_val;
	}
	if ((pdur_has_tp_module_dest(routing_path_ptr) == (boolean)TRUE) || (is_up_module_called == (boolean)FALSE)) {
		ret_val =
			pdur_lo_tp_copy_rx_data_buffer_and_transfer(pdu_id, info, buffer_size_ptr, is_up_module_called);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_lo_tp_rx_indication(PduIdType pdu_id, NotifResultType result, uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_NOT_INIT);
		return;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_TPRXINDICATION_PDU_ID_INVALID);
		return;
	}
	if (result == (NotifResultType)NTFRSLT_OK) {
		pdur_tp_rx_indication_result_is_ok(pdu_id);
	} else {
		pdur_tp_rx_indication_result_is_nok(pdu_id, result);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_same_core_up_module(PduIdType pdu_id, PduLengthType tp_sdu_length,
							  PduLengthType *buffer_size_ptr, boolean *is_up_module_called)
{
	BufReq_ReturnType ret_val = BUFREQ_OK;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	boolean has_up_tp_module = (boolean)FALSE;

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		if (pdur_check_routing_path_enabled(routing_path_ptr->pdur_dest_pdus[dest_pdu_idx]) == (boolean)FALSE) {
			continue;
		}
		if (pdur_is_up_module(routing_path_ptr->pdur_dest_pdus[dest_pdu_idx]->dest_module) == (boolean)FALSE) {
			continue;
		}
		has_up_tp_module = (boolean)TRUE;
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[dest_pdu_idx];
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
		if (destination->pdur_dest_pdu_core == (pdur_core_id_type)GetCoreID())
#endif
		{
			ret_val = pdur_lo_tp_start_of_reception_up_module_call_func(tp_sdu_length, buffer_size_ptr,
										    destination);
			*is_up_module_called = (boolean)TRUE;
		}
		break;
	}
	if (has_up_tp_module == (boolean)FALSE) {
		*is_up_module_called = (boolean)TRUE;
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_up_module_call_func(PduLengthType tp_sdu_length, PduLengthType *buffer_size_ptr,
							  const pdur_dest_pdu_type *destination)
{
	BufReq_ReturnType ret_val = BUFREQ_OK;
	PduLengthType buf_size = 0u;

	ret_val = pdur_route_start_of_reception(destination, NULL, tp_sdu_length, &buf_size);
	if (ret_val == BUFREQ_OK) {
		pdur_calculate_min_buffer_size(buffer_size_ptr, &buf_size);
	} else {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_UP_MODULE_CALL_FAILED);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_buffer_and_transfer_on_the_fly(PduIdType pdu_id, uint8 dest_pdu_idx)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	const pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_last_buf_mager_in_queue_by_src_pdu_id(pdu_id);
	if (pdur_buffer_manager_ptr == NULL) {
		return BUFREQ_E_NOT_OK;
	}
	if (routing_path_ptr->pdur_tp_threshld != 0u) {
		return BUFREQ_OK;
	}
	if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) {
		return BUFREQ_E_NOT_OK;
	}
	Std_ReturnType ret_val_std = E_OK;

	if (pdur_get_buf_mager_queue_length(pdu_id) == 1u) {
		ret_val_std = pdur_routing_dest_pdu_transmit(pdu_id, dest_pdu_idx);
	}
	if (ret_val_std != E_OK) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_ONTHEFLY_TRANSMIT_FAILED);
		return BUFREQ_E_NOT_OK;
	}
	return BUFREQ_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_start_of_reception_buffer_and_transfer(PduIdType pdu_id, PduLengthType tp_sdu_length,
							  PduLengthType *buffer_size_ptr, boolean is_up_module_called)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	BufReq_ReturnType ret_val = BUFREQ_OK;

	if ((pdur_has_tp_module_dest(routing_path_ptr) == (boolean)FALSE) && (is_up_module_called == (boolean)TRUE)) {
		return ret_val;
	}
	if (pdur_check_if_new_buf_alloc_allowed_in_queue(pdu_id) == (boolean)FALSE) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_QUEUE_FULL);
		return BUFREQ_E_BUSY;
	}
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_alloc_buffer_manager_by_src_pdu_id(pdu_id, tp_sdu_length);
	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_BUFFER_MANAGER_NULL);
		return BUFREQ_E_BUSY;
	}
	PduLengthType buf_size = 0u;

	pdur_buffer_manager_ptr->pdu_length = tp_sdu_length;
	pdur_buffer_manager_ptr->src_pdu_id = pdu_id;
	buf_size = pdur_buffer_manager_ptr->buf_length;
	pdur_calculate_min_buffer_size(buffer_size_ptr, &buf_size);
	ret_val = BUFREQ_E_NOT_OK;
	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		if (pdur_check_routing_path_enabled(routing_path_ptr->pdur_dest_pdus[dest_pdu_idx]) == (boolean)FALSE) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
			ret_val = BUFREQ_OK;
		} else if (pdur_is_up_module(routing_path_ptr->pdur_dest_pdus[dest_pdu_idx]->dest_module) ==
			   (boolean)TRUE) {
			if (is_up_module_called == (boolean)TRUE) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
			} else {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_DIAG_UPPERLAYER_RX_READY;
				ret_val = BUFREQ_OK;
			}
		} else if (routing_path_ptr->pdur_direct_gateway == PDUR_GATEWAY_ONTHEFLY) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT;
			BufReq_ReturnType single_ret_val =
				pdur_lo_tp_start_of_reception_buffer_and_transfer_on_the_fly(pdu_id, dest_pdu_idx);
			if (single_ret_val != BUFREQ_OK) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
			} else {
				ret_val = BUFREQ_OK;
			}
		} else {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_DIRECT_RX_READY;
			ret_val = BUFREQ_OK;
		}
	}
	pdur_check_pending_dst_cnt_reached(pdu_id, (boolean)FALSE);
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_lo_tp_start_of_reception_post_proc(const pdur_routing_path_type *routing_path_ptr)
{
	for (uint8 i = 0u; i < routing_path_ptr->pdur_destinatn_num; i++) {
		if (pdur_is_up_module(routing_path_ptr->pdur_dest_pdus[i]->dest_module) == (boolean)FALSE) {
			continue;
		}
		const pdur_dest_pdu_type *destination = routing_path_ptr->pdur_dest_pdus[i];

		pdur_route_tp_rx_indication(destination, NTFRSLT_E_NOT_OK);
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_POST_PROC);
		break;
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_lo_tp_start_of_reception(PduIdType pdu_id, PduLengthType tp_sdu_length, PduLengthType *buffer_size_ptr,
			      uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_NOT_INIT);
		return BUFREQ_E_NOT_OK;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_PDU_ID_INVALID);
		return BUFREQ_E_NOT_OK;
	}
	if (buffer_size_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_STARTOFRECEPTION_PDU_INFO_PTR_NULL);
		return BUFREQ_E_NOT_OK;
	}
	*buffer_size_ptr = 0u;
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[pdu_id];
	BufReq_ReturnType ret_val;
	boolean is_up_module_called = (boolean)FALSE;

	if (pdur_check_all_dest_disabled(routing_path_ptr) == (boolean)TRUE) {
		return BUFREQ_E_NOT_OK;
	}

	ret_val = pdur_lo_tp_start_of_reception_same_core_up_module(pdu_id, tp_sdu_length, buffer_size_ptr,
								    &is_up_module_called);
	if (ret_val != BUFREQ_OK) {
		return ret_val;
	}
	ret_val = pdur_lo_tp_start_of_reception_buffer_and_transfer(pdu_id, tp_sdu_length, buffer_size_ptr,
								    is_up_module_called);
	if ((ret_val != BUFREQ_OK) && (is_up_module_called == (boolean)TRUE)) {
		pdur_lo_tp_start_of_reception_post_proc(routing_path_ptr);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_gateway_on_the_fly(PduIdType src_pdu_id, uint8 dest_pdu_idx, const PduInfoType *info,
						   const RetryInfoType *retry, PduLengthType *available_data_ptr)
{
	BufReq_ReturnType ret_val = BUFREQ_OK;
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_NULL_ONTHEFLY);
		return BUFREQ_E_NOT_OK;
	}
	if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_FAILED) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_STATE_ERROR_ONTHEFLY);
		ret_val = BUFREQ_E_NOT_OK;
	} else if ((retry != NULL) && (retry->TpDataState != TP_DATACONF)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_STATE_ERROR_ONTHEFLY);
		ret_val = BUFREQ_E_NOT_OK;
	} else if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) ||
		   (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) ||
		   (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_WAIT)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_STATE_ERROR_ONTHEFLY);
		ret_val = BUFREQ_E_NOT_OK;
	} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_ROUTING_IDLE) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_STATE_ERROR_ONTHEFLY);
		return BUFREQ_E_NOT_OK;
	} else if (pdur_buffer_and_transfer_check_send_data_enough(pdur_buffer_manager_ptr, info->SduLength,
								   dest_pdu_idx) == (boolean)FALSE) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_NOT_ENOUGH_ONTHEFLY);
		ret_val = BUFREQ_E_BUSY;
	} else {
		ret_val = pdur_buffer_and_transfer_copy_data_from_ring_buffer(pdur_buffer_manager_ptr, info,
									      dest_pdu_idx, available_data_ptr, NULL);
	}
	if (ret_val == BUFREQ_E_NOT_OK) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_FAILED;
	} else {
		if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
				PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_ONGOING;
		} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			   PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_READY) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
				PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_ONGOING;
		} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			   PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
				PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_ONGOING;
		} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] ==
			   PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_WAIT) {
			pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] =
				PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_ONGOING;
		} else {
		}
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_direct_gateway(PduIdType src_pdu_id, uint8 dest_pdu_idx, const PduInfoType *info,
					       const RetryInfoType *retry, PduLengthType *available_data_ptr)
{
	BufReq_ReturnType ret_val = BUFREQ_OK;
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_NULL_LOWER);
		return BUFREQ_E_NOT_OK;
	}
	if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_GATEWAY_DIRECT_TX_READY) &&
	    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_GATEWAY_DIRECT_TX_ONGOING)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_STATE_ERROR_LOWER);
		return BUFREQ_E_NOT_OK;
	}
	pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_DIRECT_TX_ONGOING;
	if (pdur_buffer_and_transfer_check_send_data_enough(pdur_buffer_manager_ptr, info->SduLength, dest_pdu_idx) ==
	    (boolean)FALSE) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_NOT_ENOUGH_LOWER);
		ret_val = BUFREQ_E_NOT_OK;
	} else {
		ret_val = pdur_buffer_and_transfer_copy_data_from_ring_buffer(pdur_buffer_manager_ptr, info,
									      dest_pdu_idx, available_data_ptr, retry);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_buffer_and_transfer_upper_module(PduIdType src_pdu_id, uint8 dest_pdu_idx,
								 const PduInfoType *info,
								 PduLengthType *available_data_ptr)
{
	BufReq_ReturnType ret_val = BUFREQ_OK;
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_NULL_UPPER);
		return BUFREQ_E_NOT_OK;
	}
	if ((pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_DIAG_UPPERLAYER_TX_READY) &&
	    (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] != PDUR_DIAG_UPPERLAYER_TX_ONGOING)) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_STATE_ERROR_UPPER);
		return BUFREQ_E_NOT_OK;
	}
	pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_DIAG_UPPERLAYER_TX_ONGOING;
	if (pdur_buffer_and_transfer_check_send_data_enough(pdur_buffer_manager_ptr, info->SduLength, dest_pdu_idx) ==
	    (boolean)FALSE) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_NOT_ENOUGH_UPPER);
		ret_val = BUFREQ_E_NOT_OK;
	} else {
		ret_val = pdur_buffer_and_transfer_copy_data_from_ring_buffer(pdur_buffer_manager_ptr, info,
									      dest_pdu_idx, available_data_ptr, NULL);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_lo_tp_copy_tx_data(PduIdType dest_pdu_id, const PduInfoType *info, RetryInfoType *retry,
			PduLengthType *available_data_ptr, uint8 service_id)
{
	(void)service_id;
	const pdur_state_type pdur_state = pdur_get_pdur_state();

	if (pdur_state == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_NOT_INIT);
		return BUFREQ_E_NOT_OK;
	}
	if (NULL == info) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTECOPYTXDATA_PDUR_E_NULL_POINTER);
		return BUFREQ_E_NOT_OK;
	}
	PduIdType src_pdu_id = pdur_get_src_pdu_id_by_dest_pdu_id(dest_pdu_id);
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (src_pdu_id >= pdur_config_ptr->n_routing_paths) {
		return BUFREQ_E_NOT_OK;
	}
	BufReq_ReturnType ret_val = BUFREQ_E_NOT_OK;
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[src_pdu_id];
	boolean is_up_module_called = (boolean)FALSE;

	if (pdur_is_up_module(routing_path_ptr->src_module) == (boolean)TRUE) {
		ret_val = pdur_lo_tp_copy_tx_data_same_core_up_module(routing_path_ptr, info, retry, available_data_ptr,
								      &is_up_module_called);
	}
	if ((pdur_is_lo_module(routing_path_ptr->src_module) == (boolean)TRUE) ||
	    (is_up_module_called == (boolean)FALSE)) {
		ret_val = pdur_lo_tp_copy_tx_data_buffer_and_transfer(src_pdu_id, dest_pdu_id, info, retry,
								      available_data_ptr);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_same_core_up_module(const pdur_routing_path_type *routing_path_ptr,
						    const PduInfoType *info, RetryInfoType *retry,
						    PduLengthType *available_data_ptr, boolean *is_up_module_called)
{
	BufReq_ReturnType ret_val = BUFREQ_OK;
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	if (routing_path_ptr->pdur_src_pdu_core == (pdur_core_id_type)GetCoreID())
#endif
	{
		ret_val = pdur_route_copy_tx_data(routing_path_ptr, info, retry, available_data_ptr);
		*is_up_module_called = (boolean)TRUE;
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(BufReq_ReturnType, PDUR_CODE)
	pdur_lo_tp_copy_tx_data_buffer_and_transfer(PduIdType src_pdu_id, PduIdType dest_pdu_id,
						    const PduInfoType *info, const RetryInfoType *retry,
						    PduLengthType *available_data_ptr)
{
	const pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if (pdur_buffer_manager_ptr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_MANAGER_NULL);
		return BUFREQ_E_NOT_OK;
	}
	if (pdur_buffer_manager_ptr->buf_addr == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_COPYTXDATA_BUFFER_BLOCK_NULL);
		return BUFREQ_E_NOT_OK;
	}
	uint8 dest_pdu_idx = pdur_get_dest_pdu_idx_in_routing_path(src_pdu_id, dest_pdu_id);
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[src_pdu_id];
	BufReq_ReturnType ret_val = BUFREQ_OK;

	if (pdur_is_up_module(routing_path_ptr->src_module) == (boolean)TRUE) {
		ret_val = pdur_lo_tp_copy_tx_data_buffer_and_transfer_upper_module(src_pdu_id, dest_pdu_idx, info,
										   available_data_ptr);
	} else if (routing_path_ptr->pdur_direct_gateway == PDUR_GATEWAY_DIRECT) {
		ret_val = pdur_lo_tp_copy_tx_data_direct_gateway(src_pdu_id, dest_pdu_idx, info, retry,
								 available_data_ptr);
	} else {
		ret_val = pdur_lo_tp_copy_tx_data_gateway_on_the_fly(src_pdu_id, dest_pdu_idx, info, retry,
								     available_data_ptr);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_check_all_dest_disabled(const pdur_routing_path_type *routing_path_ptr)
{
	boolean all_disabled = (boolean)TRUE;

	for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < routing_path_ptr->pdur_destinatn_num; dest_pdu_idx++) {
		if (pdur_check_routing_path_enabled(routing_path_ptr->pdur_dest_pdus[dest_pdu_idx]) == (boolean)TRUE) {
			all_disabled = (boolean)FALSE;
			break;
		}
	}
	return all_disabled;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_check_routing_path_enabled(const pdur_dest_pdu_type *destination)
{
#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (destination == NULL) {
		return (boolean)FALSE;
	}
	if (destination->idx_num == 0u) {
		return (boolean)TRUE;
	}
	const pdur_routing_path_group_id_type *routing_path_group_idx =
		&pdur_config_ptr->routing_path_group_list[destination->start_idx];
	for (uint8 idx = 0; idx < destination->idx_num; idx++) {
		if (g_pdur_routing_group_status[routing_path_group_idx[idx]] == (boolean)FALSE) {
			return (boolean)FALSE;
		}
	}
#else
	(void)destination;
#endif
	return (boolean)TRUE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE)
pdur_set_routing_path_group_status(pdur_routing_path_group_id_type id, boolean routing_group_status)
{
	g_pdur_routing_group_status[id] = routing_group_status;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_get_routing_path_group_status(pdur_routing_path_group_id_type id)
{
	return g_pdur_routing_group_status[id];
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#endif
