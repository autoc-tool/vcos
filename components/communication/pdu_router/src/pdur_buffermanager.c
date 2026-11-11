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
#include "pdur_internal.h"
#include "pdur_buffermanager.h"
#include "pdur_bufferpoolcfg.h"
#include "pdur_spinlock.h"
#if defined(VCOS_OS_CONFIG_MEMPOOL) && (VCOS_OS_CONFIG_MEMPOOL == 1)
#include "Os_MemPool_Cfg.h"
#endif
/* local macro definition */

/* local enumeration definition */

/* local type & structure definition */

/* global variable definition */

/* local variable definition */

/* local function declaration */
/*! \brief          遍历链表，释放特定pdu_id对应的buffer_manager - 观察并处理首个节点
 *  \param[in]      src_pdu_id 期望释放的buffer_manager中存储的pdu_id
 *  \param[in]      is_queue_first 期望释放队列的首元素还是尾元素
 *  \details        头节点逻辑特殊，特别处理
 *  \return         E_OK - 释放成功
 *                  E_NOT_OK - 释放失败
 */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE)
pdur_free_buf_mager_by_src_pdu_id_header_node(PduIdType src_pdu_id, boolean is_queue_first);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/*! \brief          遍历链表，释放特定pdu_id对应的buffer_manager - 观察并处理首个节点之外的其他节点
 *  \details        其他节点可以统一逻辑处理
 *  \param[in]      src_pdu_id 期望释放的buffer_manager中存储的pdu_id
 *  \param[in]      is_queue_first 期望释放队列的首元素还是尾元素
 *  \return         E_OK - 释放成功
 *                  E_NOT_OK - 释放失败
 */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE)
pdur_free_buf_mager_by_src_pdu_id_ordinary_node(PduIdType src_pdu_id, boolean is_queue_first);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/*! \brief          在queue_depth配置为1或者当前queue中无元素时调用该接口分配
 *  \param[in]      src_pdu_id 用来申请缓存的pdu_id
 *  \details        头节点逻辑特殊，特别处理
 *  \return         NULL - 申请失败
 *                  node_ptr - 申请成功
 */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_alloc_buf_mager_first_element_in_queue(PduIdType src_pdu_id, PduLengthType tp_sdu_length);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/*! \brief          在queue_depth配置为大于1且当前queue中有元素时调用该接口分配
 *  \param[in]      src_pdu_id 用来申请缓存的pdu_id
 *  \details        头节点逻辑特殊，特别处理
 *  \return         NULL - 申请失败
 *                  node_ptr - 申请成功
 */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_alloc_buf_mager_another_element_in_queue(PduIdType src_pdu_id, PduLengthType tp_sdu_length);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/* local function implementation */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE)
pdur_free_buf_mager_by_src_pdu_id_header_node(PduIdType src_pdu_id, boolean is_queue_first)
{
	pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();
	uint8 buf_mager_num = pdur_get_buffer_manager_len();
	pdur_buffer_manager_type *node_to_free = &buf_mager_list[buf_mager_busy_header->head_node_idx];

	if (pdur_get_buf_mager_queue_length(src_pdu_id) == 1u) {
		buf_mager_busy_header->head_node_idx = node_to_free->next_node_idx;
	} else if (is_queue_first == (boolean)TRUE) {
		pdur_buffer_manager_type *second_node_in_queue = &buf_mager_list[node_to_free->queue_next_node_idx];

		buf_mager_busy_header->head_node_idx = second_node_in_queue->self_node_idx;
		second_node_in_queue->next_node_idx = node_to_free->next_node_idx;
	} else {
		pdur_buffer_manager_type *before_last_node = &buf_mager_list[node_to_free->self_node_idx];
		pdur_buffer_manager_type *last_node = &buf_mager_list[node_to_free->queue_next_node_idx];

		while (last_node->queue_next_node_idx != buf_mager_num) {
			before_last_node = last_node;
			last_node = &buf_mager_list[last_node->queue_next_node_idx];
		}
		node_to_free = last_node;
		before_last_node->queue_next_node_idx = buf_mager_num;
	}
	pdur_buffer_free(node_to_free);
	return E_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE)
pdur_free_buf_mager_by_src_pdu_id_ordinary_node(PduIdType src_pdu_id, boolean is_queue_first)
{
	const pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();
	uint8 buf_mager_num = pdur_get_buffer_manager_len();
	uint8 busy_buffer_manager_idx = buf_mager_busy_header->head_node_idx;
	pdur_buffer_manager_type *node_before_node_to_free = &buf_mager_list[busy_buffer_manager_idx];

	if (node_before_node_to_free->next_node_idx == buf_mager_num) {
		return E_NOT_OK;
	}
	pdur_buffer_manager_type *node_to_free = NULL;

	do {
		node_to_free = &buf_mager_list[node_before_node_to_free->next_node_idx];
		if (node_to_free->src_pdu_id == src_pdu_id) {
			if (pdur_get_buf_mager_queue_length(src_pdu_id) == 1u) {
				node_before_node_to_free->next_node_idx = node_to_free->next_node_idx;
			} else if (is_queue_first == (boolean)TRUE) {
				pdur_buffer_manager_type *second_node_in_queue =
					&buf_mager_list[node_to_free->queue_next_node_idx];
				node_before_node_to_free->next_node_idx = second_node_in_queue->self_node_idx;
				second_node_in_queue->next_node_idx = node_to_free->next_node_idx;
			} else {
				pdur_buffer_manager_type *before_last_node =
					&buf_mager_list[node_to_free->self_node_idx];
				pdur_buffer_manager_type *last_node =
					&buf_mager_list[node_to_free->queue_next_node_idx];
				while (last_node->queue_next_node_idx != buf_mager_num) {
					before_last_node = last_node;
					last_node = &buf_mager_list[last_node->queue_next_node_idx];
				}
				node_to_free = last_node;
				before_last_node->queue_next_node_idx = buf_mager_num;
			}
			break;
		}
		node_before_node_to_free = node_to_free;
	} while (node_before_node_to_free->next_node_idx != buf_mager_num);
	if (node_before_node_to_free == node_to_free) {
		return E_NOT_OK;
	}
	pdur_buffer_free(node_to_free);
	return E_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_alloc_buf_mager_first_element_in_queue(PduIdType src_pdu_id, PduLengthType tp_sdu_length)
{
	pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	pdur_buffer_manager_type *applied_buf_mager_node_ptr = pdur_buffer_alloc(tp_sdu_length);

	if (applied_buf_mager_node_ptr == NULL) {
		return NULL;
	}
	Std_ReturnType lock_result = pdur_get_buf_mngr_busy_list_spin_lock();

	if (lock_result != E_OK) {
		pdur_buffer_free(applied_buf_mager_node_ptr);
		return NULL;
	}
	applied_buf_mager_node_ptr->next_node_idx = buf_mager_busy_header->head_node_idx;
	buf_mager_busy_header->head_node_idx = applied_buf_mager_node_ptr->self_node_idx;
	applied_buf_mager_node_ptr->src_pdu_id = src_pdu_id;
	(void)pdur_rel_buf_mngr_busy_list_spin_lock();
	return applied_buf_mager_node_ptr;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_alloc_buf_mager_another_element_in_queue(PduIdType src_pdu_id, PduLengthType tp_sdu_length)
{
	uint8 buf_mager_num = pdur_get_buffer_manager_len();
	pdur_buffer_manager_type *applied_buf_mager_node_ptr = pdur_buffer_alloc(tp_sdu_length);

	if (applied_buf_mager_node_ptr == NULL) {
		return NULL;
	}
	pdur_buffer_manager_type *last_buf_mager_in_queue = pdur_find_last_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);

	if (last_buf_mager_in_queue == NULL) {
		pdur_buffer_free(applied_buf_mager_node_ptr);
		return NULL;
	}
	Std_ReturnType lock_result = pdur_get_buf_mngr_busy_list_spin_lock();

	if (lock_result != E_OK) {
		pdur_buffer_free(applied_buf_mager_node_ptr);
		return NULL;
	}
	applied_buf_mager_node_ptr->next_node_idx = buf_mager_num;
	applied_buf_mager_node_ptr->queue_next_node_idx = buf_mager_num;
	last_buf_mager_in_queue->queue_next_node_idx = applied_buf_mager_node_ptr->self_node_idx;
	applied_buf_mager_node_ptr->src_pdu_id = src_pdu_id;
	(void)pdur_rel_buf_mngr_busy_list_spin_lock();
	return applied_buf_mager_node_ptr;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(uint8, PDUR_CODE) pdur_get_buf_mager_queue_length(PduIdType src_pdu_id)
{
	const pdur_buffer_manager_type *applied_buffer_manager_addr = pdur_find_buf_mager_by_src_pdu_id(src_pdu_id);
	const pdur_buffer_manager_type *buf_mager_list = pdur_get_buffer_manager_list_addr();

	if ((applied_buffer_manager_addr == NULL) || (buf_mager_list == NULL)) {
		return 0u;
	}
	uint8 length = 1u;
	uint8 buf_mager_num = pdur_get_buffer_manager_len();

	while (applied_buffer_manager_addr->queue_next_node_idx != buf_mager_num) {
		length++;
		applied_buffer_manager_addr = &buf_mager_list[applied_buffer_manager_addr->queue_next_node_idx];
	}
	return length;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/* global function implementation */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_check_buf_mag_with_pdu_id_existed_in_busy_list(PduIdType src_pdu_id)
{
	const pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	const pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();

	if (buf_mager_list == NULL) {
		return (boolean)FALSE;
	}
	uint8 buf_mager_num = pdur_get_buffer_manager_len();
	uint8 head_node_idx = buf_mager_busy_header->head_node_idx;

	if (head_node_idx == buf_mager_num) {
		return (boolean)FALSE;
	}
	const pdur_buffer_manager_type *node_ptr = &buf_mager_list[head_node_idx];

	do {
		if (node_ptr->src_pdu_id == src_pdu_id) {
			return (boolean)TRUE;
		}
		if (node_ptr->next_node_idx == buf_mager_num) {
			break;
		}
		node_ptr = &buf_mager_list[node_ptr->next_node_idx];
	} while ((boolean)TRUE);
	return (boolean)FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_alloc_buffer_manager_by_src_pdu_id(PduIdType src_pdu_id, PduLengthType tp_sdu_length)
{
	pdur_buffer_manager_type *alloc_buf_mager = NULL;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	uint8 queue_depth = pdur_config_ptr->routing_paths[src_pdu_id].pdur_queue_depth;

	if (queue_depth == 1u) {
		if (pdur_check_buf_mag_with_pdu_id_existed_in_busy_list(src_pdu_id) == (boolean)FALSE) {
			alloc_buf_mager = pdur_alloc_buf_mager_first_element_in_queue(src_pdu_id, tp_sdu_length);
		}
	} else {
		if (pdur_check_buf_mag_with_pdu_id_existed_in_busy_list(src_pdu_id) == (boolean)TRUE) {
			if (pdur_get_buf_mager_queue_length(src_pdu_id) < queue_depth) {
				alloc_buf_mager =
					pdur_alloc_buf_mager_another_element_in_queue(src_pdu_id, tp_sdu_length);
			}
		} else {
			alloc_buf_mager = pdur_alloc_buf_mager_first_element_in_queue(src_pdu_id, tp_sdu_length);
		}
	}
	return alloc_buf_mager;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE) pdur_find_buf_mager_by_src_pdu_id(PduIdType src_pdu_id)
{
	pdur_buffer_manager_type *applied_buffer_manager_addr = NULL;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[src_pdu_id];
	uint16 tx_buffer_idx = routing_path_ptr->pdur_tx_buffer_idx;

	if (tx_buffer_idx != PDUR_NO_BUFFER) {
		if (tx_buffer_idx < pdur_get_tx_buffer_manager_len()) {
			applied_buffer_manager_addr = pdur_get_tx_buffer_manager_list_addr();
			return &applied_buffer_manager_addr[tx_buffer_idx];
		}
	}
	const pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();

	if (buf_mager_list == NULL) {
		return NULL;
	}
	uint8 buf_mager_num = pdur_get_buffer_manager_len();
	uint8 applied_buffer_manager_idx = buf_mager_busy_header->head_node_idx;

	if (applied_buffer_manager_idx == buf_mager_num) {
		return NULL;
	}
	applied_buffer_manager_addr = &buf_mager_list[applied_buffer_manager_idx];
	do {
		if (applied_buffer_manager_addr->src_pdu_id == src_pdu_id) {
			return applied_buffer_manager_addr;
		}
		if (applied_buffer_manager_addr->next_node_idx == buf_mager_num) {
			break;
		}
		applied_buffer_manager_addr = &buf_mager_list[applied_buffer_manager_addr->next_node_idx];
	} while ((boolean)TRUE);
	return NULL;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_free_buf_mager_by_src_pdu_id(PduIdType src_pdu_id, boolean is_queue_first)
{
	Std_ReturnType ret_val = E_NOT_OK;
	const pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	const pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();

	if (buf_mager_list == NULL) {
		return ret_val;
	}
	uint8 buf_mager_num = pdur_get_buffer_manager_len();
	Std_ReturnType lock_result = E_NOT_OK;
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	const pdur_routing_path_type *routing_path_ptr = &pdur_config_ptr->routing_paths[src_pdu_id];
	uint16 tx_buffer_idx = routing_path_ptr->pdur_tx_buffer_idx;

	if (tx_buffer_idx != PDUR_NO_BUFFER) {
		return E_OK;
	}
	lock_result = pdur_get_buf_mngr_busy_list_spin_lock();
	if (lock_result == E_OK) {
		uint8 busy_buffer_manager_idx = buf_mager_busy_header->head_node_idx;

		if (busy_buffer_manager_idx == buf_mager_num) {
			(void)pdur_rel_buf_mngr_busy_list_spin_lock();
			return ret_val;
		}
		if (buf_mager_list[busy_buffer_manager_idx].src_pdu_id == src_pdu_id) {
			ret_val = pdur_free_buf_mager_by_src_pdu_id_header_node(src_pdu_id, is_queue_first);
		} else {
			ret_val = pdur_free_buf_mager_by_src_pdu_id_ordinary_node(src_pdu_id, is_queue_first);
		}
		(void)pdur_rel_buf_mngr_busy_list_spin_lock();
	} else {
		return E_NOT_OK;
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_free_all_buf_mager_by_src_pdu_id(PduIdType src_pdu_id)
{
	while (pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id) != NULL) {
		(void)pdur_free_buf_mager_by_src_pdu_id(src_pdu_id, (boolean)TRUE);
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_pdu_id_buffer_manager_init(pdur_buffer_manager_type *applied_buffer_manager_addr)
{
	uint8 buf_mager_num = pdur_get_buffer_manager_len();

	if (applied_buffer_manager_addr == NULL) {
		return;
	}
	for (uint8 itr = 0u; itr < PDUR_MAX_GW_DESTINATIONS; itr++) {
		applied_buffer_manager_addr->routing_state[itr] = PDUR_ROUTING_IDLE;
		applied_buffer_manager_addr->buffer_read_offset[itr] = 0u;
	}
	applied_buffer_manager_addr->buffer_write_offset = 0u;
	applied_buffer_manager_addr->src_pdu_id = PDUR_INVALID_PDU_ID;
	applied_buffer_manager_addr->pdu_length = 0u;
	applied_buffer_manager_addr->next_node_idx = buf_mager_num;
	applied_buffer_manager_addr->queue_next_node_idx = buf_mager_num;
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/* global fuction implementation */
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_buffer_manager_init(void)
{
	uint8 buf_manage_header_num = pdur_get_buffer_manager_header_len();
	uint8 buf_manager_num = pdur_get_buffer_manager_len();

	if ((buf_manage_header_num == 0u) || (buf_manager_num == 0u)) {
		return;
	}
	pdur_buffer_manager_header_type *const buf_mager_pool_header = pdur_get_buffer_manager_header_list_addr();
	pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	pdur_buffer_manager_type *const buf_manager_list = pdur_get_buffer_manager_list_addr();

	buf_mager_busy_header->head_node_idx = buf_manager_num;
	for (uint8 bufferlinklist = 0u; bufferlinklist < buf_manage_header_num; bufferlinklist++) {
		buf_mager_pool_header[bufferlinklist].head_node_idx = buf_manager_num;
	}
	for (uint16 buffer_manager_idx = 0u; buffer_manager_idx < buf_manager_num; buffer_manager_idx++) {
		for (uint8 itr = 0u; itr < PDUR_MAX_GW_DESTINATIONS; itr++) {
			buf_manager_list[buffer_manager_idx].routing_state[itr] = PDUR_ROUTING_IDLE;
			buf_manager_list[buffer_manager_idx].buffer_read_offset[itr] = 0u;
		}
		buf_manager_list[buffer_manager_idx].buffer_write_offset = 0u;
		buf_manager_list[buffer_manager_idx].src_pdu_id = PDUR_INVALID_PDU_ID;
		buf_manager_list[buffer_manager_idx].pdu_length = 0u;
		buf_manager_list[buffer_manager_idx].queue_next_node_idx = buf_manager_num;
		uint8 pool_idx_of_block = buf_manager_list[buffer_manager_idx].pool_idx;

		buf_manager_list[buffer_manager_idx].next_node_idx =
			buf_mager_pool_header[pool_idx_of_block].head_node_idx;
		buf_mager_pool_header[pool_idx_of_block].head_node_idx =
			buf_manager_list[buffer_manager_idx].self_node_idx;
		if (pool_idx_of_block == PDUR_BUFFER_POOL_MEMPOOL_IDX) {
#if defined(VCOS_OS_CONFIG_MEMPOOL) && (VCOS_OS_CONFIG_MEMPOOL == 1)
#ifdef PduR_MemPoolFree
			if (buf_manager_list[buffer_manager_idx].buf_addr != NULL) {
				pdur_mem_pool_free((void *)buf_manager_list[buffer_manager_idx].buf_addr, 1u);
			}
#endif
#endif
			buf_manager_list[buffer_manager_idx].buf_addr = NULL;
			buf_manager_list[buffer_manager_idx].buf_length = 0u;
		}
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(uint8, PDUR_CODE) pdur_get_num_of_buffer_manager_busy_node(void)
{
	const pdur_buffer_manager_header_type *const buf_mager_busy_header = pdur_get_buffer_manager_busy_header_addr();
	const pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();
	uint8 num_of_busy_node = 0u;

	if (buf_mager_list == NULL) {
		return 0u;
	}
	uint8 buf_mager_num = pdur_get_buffer_manager_len();
	uint8 node_idx = buf_mager_busy_header->head_node_idx;

	if (node_idx == buf_mager_num) {
		return 0u;
	}
	const pdur_buffer_manager_type *node_addr = &buf_mager_list[node_idx];

	do {
		num_of_busy_node++;
		if (node_addr->next_node_idx == buf_mager_num) {
			break;
		}
		node_addr = &buf_mager_list[node_addr->next_node_idx];
	} while ((boolean)TRUE);
	return num_of_busy_node;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_find_last_buf_mager_in_queue_by_src_pdu_id(PduIdType src_pdu_id)
{
	pdur_buffer_manager_type *applied_buffer_manager_addr = pdur_find_buf_mager_by_src_pdu_id(src_pdu_id);

	if (applied_buffer_manager_addr == NULL) {
		return NULL;
	}
	pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();
	uint8 buf_mager_num = pdur_get_buffer_manager_len();

	while (applied_buffer_manager_addr->queue_next_node_idx != buf_mager_num) {
		applied_buffer_manager_addr = &buf_mager_list[applied_buffer_manager_addr->queue_next_node_idx];
	}
	return applied_buffer_manager_addr;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE)
pdur_find_first_buf_mager_in_queue_by_src_pdu_id(PduIdType src_pdu_id)
{
	return pdur_find_buf_mager_by_src_pdu_id(src_pdu_id);
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_check_if_new_buf_alloc_allowed_in_queue(PduIdType src_pdu_id)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	uint8 queue_depth = pdur_config_ptr->routing_paths[src_pdu_id].pdur_queue_depth;
	uint8 queue_length = pdur_get_buf_mager_queue_length(src_pdu_id);

	if (queue_length == 0u) {
		return (boolean)TRUE;
	} else if (queue_length >= queue_depth) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_CHECKQUEUEALLOW_FULL);
		return (boolean)FALSE;
	} else {
		const pdur_buffer_manager_type *applied_buffer_manager_addr =
			pdur_find_last_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);
		for (uint8 dest_pdu_idx = 0u;
		     dest_pdu_idx < pdur_config_ptr->routing_paths[src_pdu_id].pdur_destinatn_num; dest_pdu_idx++) {
			if ((applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_DIRECT_RX_READY) ||
			    (applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_DIRECT_RX_ONGOING)) {
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_CHECKQUEUEALLOW_DIRECT);
				return (boolean)FALSE;
			}
			if ((applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) ||
			    (applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) ||
			    (applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_READY) ||
			    (applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_READY) ||
			    (applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_ONGOING)) {
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_CHECKQUEUEALLOW_ONTHEFLY);
				return (boolean)FALSE;
			}
			if ((applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_DIAG_UPPERLAYER_RX_READY) ||
			    (applied_buffer_manager_addr->routing_state[dest_pdu_idx] ==
			     PDUR_DIAG_UPPERLAYER_RX_ONGOING)) {
				PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_CHECKQUEUEALLOW_UPPER);
				return (boolean)FALSE;
			}
		}
	}
	return (boolean)TRUE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_check_if_buf_mager_transmit_allowed_in_queue(PduIdType src_pdu_id, uint8 dest_pdu_idx)
{
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();
	uint8 pdur_direct_gateway = pdur_config_ptr->routing_paths[src_pdu_id].pdur_direct_gateway;
	const pdur_buffer_manager_type *first_node = pdur_find_first_buf_mager_in_queue_by_src_pdu_id(src_pdu_id);

	if (first_node == NULL) {
		return (boolean)FALSE;
	}
	if (pdur_direct_gateway == PDUR_GATEWAY_DIRECT) {
		if (first_node->routing_state[dest_pdu_idx] == PDUR_GATEWAY_DIRECT_RX_FINISH) {
			return (boolean)TRUE;
		}
	} else {
		if (first_node->buffer_write_offset < pdur_config_ptr->routing_paths[src_pdu_id].pdur_tp_threshld) {
			if (first_node->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_WAIT) {
				return (boolean)TRUE;
			}
		} else {
			if ((first_node->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_READY_TX_WAIT) ||
			    (first_node->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_ONGOING_TX_WAIT) ||
			    (first_node->routing_state[dest_pdu_idx] == PDUR_GATEWAY_ONTHEFLY_RX_FINISH_TX_WAIT)) {
				return (boolean)TRUE;
			}
		}
	}
	if (first_node->routing_state[dest_pdu_idx] == PDUR_DIAG_UPPERLAYER_RX_FINISH) {
		return (boolean)TRUE;
	}
	return (boolean)FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC_P2VAR(pdur_buffer_manager_type, AUTOMATIC, PDUR_CODE) pdur_buffer_alloc(PduLengthType buffer_length)
{
	pdur_buffer_manager_type *applied_block_addr = NULL;
	pdur_buffer_manager_header_type *const buf_mager_pool_header = pdur_get_buffer_manager_header_list_addr();
	pdur_buffer_manager_type *const buf_mager_list = pdur_get_buffer_manager_list_addr();
	uint8 buf_pool_num = pdur_get_buffer_manager_header_len();
	uint8 buf_block_num = pdur_get_buffer_manager_len();

	for (uint8 buf_pool_idx = 0u; buf_pool_idx < buf_pool_num; buf_pool_idx++) {
		if (buf_mager_pool_header[buf_pool_idx].block_length != PDUR_BUFFER_LENGTH_INVALID) {
			if (buf_mager_pool_header[buf_pool_idx].block_length < buffer_length) {
				continue;
			}
		}
		if (buf_mager_pool_header[buf_pool_idx].head_node_idx >= buf_block_num) {
			continue;
		}
		uint8 applied_block_idx = buf_mager_pool_header[buf_pool_idx].head_node_idx;
		Std_ReturnType lock_result = pdur_get_buffer_pool_spin_lock(buf_pool_idx);

		if (lock_result != E_OK) {
			continue;
		}
		if (buf_mager_pool_header[buf_pool_idx].head_node_idx != applied_block_idx) {
			(void)pdur_release_buffer_pool_spin_lock(buf_pool_idx);
			continue;
		}
		applied_block_addr = &buf_mager_list[applied_block_idx];
		buf_mager_pool_header[buf_pool_idx].head_node_idx = applied_block_addr->next_node_idx;
		(void)pdur_release_buffer_pool_spin_lock(buf_pool_idx);
		applied_block_addr->next_node_idx = buf_block_num;
		if (applied_block_addr->buf_addr != NULL) {
			(void)memset(applied_block_addr->buf_addr, 0, applied_block_addr->buf_length);
		} else {
#if defined(VCOS_OS_CONFIG_MEMPOOL) && (VCOS_OS_CONFIG_MEMPOOL == 1)
#ifdef PduR_MemPoolAlloc
			applied_block_addr->buf_addr =
				(uint8 *)pdur_mem_pool_alloc(OS_MEMPOOLTYPE_GLOBAL, buffer_length, 1u);
#endif
#endif
			if (applied_block_addr->buf_addr == NULL) {
				(void)pdur_get_buffer_pool_spin_lock(buf_pool_idx);
				applied_block_addr->next_node_idx = buf_mager_pool_header[buf_pool_idx].head_node_idx;
				buf_mager_pool_header[buf_pool_idx].head_node_idx = applied_block_addr->self_node_idx;
				(void)pdur_release_buffer_pool_spin_lock(buf_pool_idx);
				return NULL;
			}
			applied_block_addr->buf_length = buffer_length;
		}
		break;
	}
	return applied_block_addr;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_buffer_free(pdur_buffer_manager_type *buf_mager)
{
	if (buf_mager == NULL) {
		return;
	}
	pdur_buffer_manager_header_type *const buf_mager_pool_header = pdur_get_buffer_manager_header_list_addr();
	Std_ReturnType lock_result = pdur_get_buffer_pool_spin_lock(buf_mager->pool_idx);

	if (lock_result != E_OK) {
		return;
	}
	if (buf_mager->pool_idx == PDUR_BUFFER_POOL_MEMPOOL_IDX) {
#if defined(VCOS_OS_CONFIG_MEMPOOL) && (VCOS_OS_CONFIG_MEMPOOL == 1)
#ifdef PduR_MemPoolFree
		pdur_mem_pool_free((void *)buf_mager->buf_addr, 1u);
		buf_mager->buf_addr = NULL;
		buf_mager->buf_length = 0u;
#endif
#endif
	}
	pdur_pdu_id_buffer_manager_init(buf_mager);
	buf_mager->next_node_idx = buf_mager_pool_header[buf_mager->pool_idx].head_node_idx;
	buf_mager_pool_header[buf_mager->pool_idx].head_node_idx = buf_mager->self_node_idx;
	(void)pdur_release_buffer_pool_spin_lock(buf_mager->pool_idx);
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
