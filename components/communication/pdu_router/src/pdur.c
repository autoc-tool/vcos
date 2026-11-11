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
#include <string.h>
#include "pdur.h"
#include "pdur_routingcfg.h"
#include "pdur_evcfifo.h"
#include "pdur_buffermanager.h"
#include "pdur_bufferpoolcfg.h"
#include "pdur_internal.h"
#if defined(VCOS_MODULE_CONFIG_VCOSMTNCE) && (VCOS_MODULE_CONFIG_VCOSMTNCE == 1)
#include "vcosMtnce_Cfg.h"
#endif
#if defined(VCOS_MODULE_CONFIG_SHELL) && (VCOS_MODULE_CONFIG_SHELL == 1)
#include "Shell.h"
#endif
#include "SchM_PduR.h"
#if (PDUR_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#include "vcos_log.h"
#include "pdur_vcos_log.h"

#define PDUR_START_SEC_MULTI_APP_BSS
#include "pdur_memmap.h"
static VAR(pdur_state_type, PDUR_MULTI_APP_BSS) pdur_pdur_state;
static P2CONST(pdur_pb_config_type, PDUR_MULTI_APP_BSS, PDUR_CONST) pdur_pdur_config;
#define PDUR_STOP_SEC_MULTI_APP_BSS
#include "pdur_memmap.h"

#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_routing_path_group_status_init(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

/*!  \brief         pdur多核路由发送
 *  \details        pdur多核路由发送
 *
 *  \param[in]      pdur_multi_core_evc_data   多核evc数据
 *  \param[in]      core_id                 核索引
 *  \return
 *
 *  \context        pdur
 *
 */
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
/*! \brief          fifo出队后调用dest对应的下层发送接口
 *  \param[in]      pdur_multi_core_evc_data 从fifo中拿出的数据
 *  \return         void
 */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_multi_core_route_transmit(pdur_evc_data_type pdur_multi_core_evc_data);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/*! \brief          fifo出队后调用dest对应的上层接收接口
 *  \param[in]      pdur_multi_core_evc_data 从fifo中拿出的数据
 *  \return         void
 */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_multi_core_upper_layer(pdur_evc_data_type pdur_multi_core_evc_data);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_init(const pdur_pb_config_type *config_ptr)
{
	if (config_ptr == NULL) {
		return;
	}
	if (pdur_get_pdur_state() != PDUR_UNINIT) {
		return;
	}
	pdur_set_pdur_config(config_ptr);
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
	pdur_evc_fifo_init();
#endif
	pdur_buffer_manager_init();

#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
	pdur_routing_path_group_status_init();
#endif
	pdur_set_pdur_state(PDUR_ONLINE);
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(pdur_pb_config_id_type, PDUR_CODE) pdur_get_configuration_id(void)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (pdur_get_pdur_state() == PDUR_UNINIT) {
		return PDUR_INVALID_CONFIGID;
	}
	return pdur_config_ptr->pdur_configuration_id;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_internal_interrupt_service_routine(void)
{
	if (pdur_get_pdur_state() == PDUR_UNINIT) {
		return;
	}
	pdur_core_id_type core_id = (pdur_core_id_type)GetCoreID();

	if (core_id >= PDUR_IF_EVC_FIFO_CORE_MAX_NUM) {
		return;
	}
	pdur_evc_data_type pdur_multi_core_evc_data;
	uint32 evc_fifo_pop_ret_val = E_NOT_OK;
	(void)memset(&pdur_multi_core_evc_data, 0, sizeof(pdur_evc_data_type));
	const pdur_evc_fifo_desc_type *const *pdur_evc_fifo_desc_ref_table = pdur_get_if_evc_fifo_desc_ref_table();
	const pdur_evc_fifo_desc_type *p_evc_fifo_desc = pdur_evc_fifo_desc_ref_table[core_id];

	if (p_evc_fifo_desc == NULL) {
		return;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	while (pdur_fifo_is_empty(p_evc_fifo_desc->fifo->write_idx, p_evc_fifo_desc->fifo->read_idx) ==
	       (boolean)FALSE) {
		evc_fifo_pop_ret_val = pdur_evc_fifo_pop(core_id, &pdur_multi_core_evc_data, PDUR_SYNC_EVCFIFO);
		if (evc_fifo_pop_ret_val != E_OK) {
			(void)pdur_lo_tp_transmit_failed_process(pdur_multi_core_evc_data.src_pdu_id,
								 pdur_multi_core_evc_data.dest_pdu_idx);
			continue;
		}
		const pdur_dest_pdu_type *destination =
			pdur_config_ptr->routing_paths[pdur_multi_core_evc_data.src_pdu_id]
				.pdur_dest_pdus[pdur_multi_core_evc_data.dest_pdu_idx];
		const PduIdType src_pdu_id = pdur_multi_core_evc_data.src_pdu_id;
		pdur_buffer_manager_type *pdur_buffer_manager_ptr =
			pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
		if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
			continue;
		}
		PduInfoType pdu_info;

		pdu_info.SduDataPtr = pdur_buffer_manager_ptr->buf_addr;
		pdu_info.SduLength = pdur_buffer_manager_ptr->pdu_length;
		Std_ReturnType ret_val = E_OK;
		pdur_dest_pdu_len_strategy_type handle_strategy =
			pdur_check_and_handle_dest_pdu_len_strategy(destination, &pdu_info);
		if (handle_strategy == PDUR_STRATEGY_DISCARD) {
			ret_val = E_NOT_OK;
		} else if (handle_strategy == PDUR_STRATEGY_SHORTEN) {
			pdu_info.SduLength = destination->pdu_len_in_ecuc;
		} else {
			if (destination->pdu_len_in_ecuc > pdur_buffer_manager_ptr->buf_length) {
				pdu_info.SduLength = pdur_buffer_manager_ptr->buf_length;
			} else {
				pdu_info.SduLength = destination->pdu_len_in_ecuc;
			}
		}
		if (ret_val == E_OK) {
			ret_val = pdur_route_transmit(destination, &pdu_info);
		}
		if ((ret_val == E_OK) && (destination->data_provision == PDUR_TRIGGERTRANSMIT)) {
			pdur_buffer_manager_ptr->routing_state[pdur_multi_core_evc_data.dest_pdu_idx] =
				PDUR_GATEWAY_IF_TT_TX_TRIGGERTRANSMIT;
		} else {
			pdur_buffer_manager_ptr->routing_state[pdur_multi_core_evc_data.dest_pdu_idx] =
				PDUR_ROUTING_IDLE;
		}
		pdur_check_pending_dst_cnt_reached(pdur_multi_core_evc_data.src_pdu_id, (boolean)TRUE);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_internal_main_function(void)
{
	if (pdur_get_pdur_state() == PDUR_UNINIT) {
		return;
	}
	pdur_core_id_type core_id = (pdur_core_id_type)GetCoreID();

	if (core_id >= PDUR_EVC_FIFO_CORE_MAX_NUM) {
		return;
	}
	pdur_evc_data_type pdur_multi_core_evc_data;
	uint32 evc_fifo_pop_ret_val = E_NOT_OK;
	(void)memset(&pdur_multi_core_evc_data, 0, sizeof(pdur_evc_data_type));
	const pdur_evc_fifo_desc_type *const *pdur_evc_fifo_desc_ref_table = pdur_get_evc_fifo_desc_ref_table();
	const pdur_evc_fifo_desc_type *p_evc_fifo_desc = pdur_evc_fifo_desc_ref_table[core_id];

	if (p_evc_fifo_desc == NULL) {
		return;
	}
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	while (pdur_fifo_is_empty(p_evc_fifo_desc->fifo->write_idx, p_evc_fifo_desc->fifo->read_idx) ==
	       (boolean)FALSE) {
		evc_fifo_pop_ret_val = pdur_evc_fifo_pop(core_id, &pdur_multi_core_evc_data, PDUR_ASYNC_EVCFIFO);
		if (evc_fifo_pop_ret_val != E_OK) {
			(void)pdur_lo_tp_transmit_failed_process(pdur_multi_core_evc_data.src_pdu_id,
								 pdur_multi_core_evc_data.dest_pdu_idx);
			continue;
		}
		const pdur_dest_pdu_type *destination =
			pdur_config_ptr->routing_paths[pdur_multi_core_evc_data.src_pdu_id]
				.pdur_dest_pdus[pdur_multi_core_evc_data.dest_pdu_idx];
		if (pdur_is_up_module(destination->dest_module)) {
			pdur_multi_core_upper_layer(pdur_multi_core_evc_data);
		} else {
			pdur_multi_core_route_transmit(pdur_multi_core_evc_data);
		}
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_multi_core_upper_layer(pdur_evc_data_type pdur_multi_core_evc_data)
{
	const PduIdType src_pdu_id = pdur_multi_core_evc_data.src_pdu_id;
	const uint8 dest_pdu_idx = pdur_multi_core_evc_data.dest_pdu_idx;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_dest_pdu_type *destination = pdur_config_ptr->routing_paths[src_pdu_id].pdur_dest_pdus[dest_pdu_idx];
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
		return;
	}
	PduInfoType pdu_info;
	PduLengthType buf_size = 0u;

	pdu_info.SduDataPtr = pdur_buffer_manager_ptr->buf_addr;
	pdu_info.SduLength = pdur_buffer_manager_ptr->pdu_length;
	BufReq_ReturnType ret_val = pdur_route_start_of_reception(destination, NULL, pdu_info.SduLength, &buf_size);

	if (ret_val == BUFREQ_OK) {
		ret_val = pdur_route_copy_rx_data(destination, &pdu_info, &buf_size);
	}
	if (ret_val == BUFREQ_OK) {
		pdur_route_tp_rx_indication(destination, NTFRSLT_OK);
	} else {
		pdur_route_tp_rx_indication(destination, NTFRSLT_E_NOT_OK);
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_MULTICOREUPPERLAYER_RETURN_NOK);
	}
	pdur_buffer_manager_ptr->buffer_write_offset = pdu_info.SduLength;
	(void)pdur_lo_tp_transmit_failed_process(src_pdu_id, dest_pdu_idx);
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
static FUNC(void, PDUR_CODE) pdur_multi_core_route_transmit(pdur_evc_data_type pdur_multi_core_evc_data)
{
	const PduIdType src_pdu_id = pdur_multi_core_evc_data.src_pdu_id;
	const uint8 dest_pdu_idx = pdur_multi_core_evc_data.dest_pdu_idx;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_dest_pdu_type *destination = pdur_config_ptr->routing_paths[src_pdu_id].pdur_dest_pdus[dest_pdu_idx];
	PduInfoType pdu_info;
	pdur_buffer_manager_type *pdur_buffer_manager_ptr =
		pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
	if ((pdur_buffer_manager_ptr == NULL) || (pdur_buffer_manager_ptr->buf_addr == NULL)) {
		return;
	}
	pdu_info.SduDataPtr = pdur_buffer_manager_ptr->buf_addr;
	pdu_info.SduLength = pdur_buffer_manager_ptr->pdu_length;
	if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_READY;
	} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY;
	} else if (pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_WAIT) {
		pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_READY;
	} else {
	}
	if (pdur_route_transmit(destination, &pdu_info) != E_OK) {
		(void)pdur_lo_tp_transmit_failed_process(src_pdu_id, dest_pdu_idx);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_enable_routing(pdur_routing_path_group_id_type id)
{
	(void)id;
	if (pdur_get_pdur_state() == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ENABLEROUTING_NOT_INIT);
		return;
	}
#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
	if (id < PDUR_ROUTING_GROUP_NUM) {
		pdur_set_routing_path_group_status(id, (boolean)TRUE);
		(void)VCOS_LOG(PDUR_MODULE_ID, VCOS_LOG_INFO, PDUR_DLT_CHECK_ID11, id);
	} else {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ENABLEROUTING_ID_INVALID);
	}
#endif
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_disable_routing(pdur_routing_path_group_id_type id, boolean initialize)
{
	(void)id;
	(void)initialize;
	if (pdur_get_pdur_state() == PDUR_UNINIT) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_DISABLEROUTING_NOT_INIT);
		return;
	}
#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (id < PDUR_ROUTING_GROUP_NUM) {
		pdur_set_routing_path_group_status(id, (boolean)FALSE);
		const pdur_routing_path_group_type *rpg_config =
			&pdur_config_ptr->routing_path_groups_init_value_list[id];
		(void)VCOS_LOG(PDUR_MODULE_ID, VCOS_LOG_INFO, PDUR_DLT_CHECK_ID12, id);
		const PduIdType *dest_pdu_id_ptr =
			&pdur_config_ptr->routing_path_groups_dest_pdu_list[rpg_config->start_idx];
		pdur_buffer_manager_type *pdur_buffer_manager_ptr = NULL;

		for (uint32 idx = 0u; idx < rpg_config->idx_num; idx++) {
			PduIdType source_pdu_id = pdur_get_src_pdu_id_by_dest_pdu_id(dest_pdu_id_ptr[idx]);

			if (source_pdu_id >= pdur_config_ptr->n_routing_paths) {
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_DISABLEROUTING_PDU_ID_INVALID);
				return;
			}
			pdur_buffer_manager_ptr = pdur_find_buf_mager_by_src_pdu_id(source_pdu_id);
			if (NULL == pdur_buffer_manager_ptr) {
				continue;
			}
			for (uint8 dest_pdu_idx = 0u; dest_pdu_idx < PDUR_MAX_GW_DESTINATIONS; dest_pdu_idx++) {
				pdur_buffer_manager_ptr->routing_state[dest_pdu_idx] = PDUR_ROUTING_IDLE;
			}
			pdur_free_all_buf_mager_by_src_pdu_id(source_pdu_id);
		}
	} else {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_DISABLEROUTING_ID_INVALID);
	}
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_routing_path_group_status_init(void)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	for (pdur_routing_path_group_id_type routing_path_group_idx = 0u;
	     routing_path_group_idx < PDUR_ROUTING_GROUP_NUM; routing_path_group_idx++) {
		pdur_set_routing_path_group_status(
			routing_path_group_idx,
			pdur_config_ptr->routing_path_groups_init_value_list[routing_path_group_idx].enabled_at_init);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(pdur_state_type, PDUR_CODE) pdur_get_pdur_state(void)
{
	return pdur_pdur_state;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_set_pdur_state(pdur_state_type state)
{
	pdur_pdur_state = state;
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2CONST(pdur_pb_config_type, AUTOMATIC, PDUR_CODE) pdur_get_pdur_config(void)
{
	return pdur_pdur_config;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_set_pdur_config(const pdur_pb_config_type *config)
{
	pdur_pdur_config = config;
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
