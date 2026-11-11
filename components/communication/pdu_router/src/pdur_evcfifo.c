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
#include "pdur.h"
#include "pdur_evcfifo.h"
#include <string.h>
#include "pdur_spinlock.h"
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_fifo_idx_increment(uint16 *index, uint16 size)
{
	if ((NULL == index) || ((*index) >= size)) {
		return;
	}

	if ((*index) == (size - 1u)) {
		(*index) = 0u;
	} else {
		(*index)++;
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_fifo_is_empty(uint16 write_idx, uint16 read_idx)
{
	return (read_idx == write_idx);
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_evc_fifo_is_full(uint16 write_idx, uint16 read_idx, uint16 size)
{
	uint16 write_idx_temp = write_idx;

	pdur_fifo_idx_increment(&write_idx_temp, size);
	return (write_idx_temp == read_idx);
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_evc_fifo_init(void)
{
	const pdur_evc_fifo_desc_type *const *pdur_evc_fifo_desc_ref_table = pdur_get_evc_fifo_desc_ref_table();
	if (pdur_evc_fifo_desc_ref_table != NULL) {
		for (uint16 core_id = 0u; core_id < PDUR_EVC_FIFO_CORE_MAX_NUM; core_id++) {
			if (pdur_evc_fifo_desc_ref_table[core_id] == NULL) {
				continue;
			}
			if (pdur_evc_fifo_desc_ref_table[core_id]->fifo != NULL) {
				pdur_evc_fifo_desc_ref_table[core_id]->fifo->write_idx = 0u;
				pdur_evc_fifo_desc_ref_table[core_id]->fifo->read_idx = 0u;
			}
		}
	}
	const pdur_evc_fifo_desc_type *const *pdur_if_evc_fifo_desc_ref_table = pdur_get_if_evc_fifo_desc_ref_table();
	if (pdur_if_evc_fifo_desc_ref_table != NULL) {
		for (uint16 core_id = 0u; core_id < PDUR_IF_EVC_FIFO_CORE_MAX_NUM; core_id++) {
			if (pdur_if_evc_fifo_desc_ref_table[core_id] == NULL) {
				continue;
			}
			if (pdur_if_evc_fifo_desc_ref_table[core_id]->fifo != NULL) {
				pdur_if_evc_fifo_desc_ref_table[core_id]->fifo->write_idx = 0u;
				pdur_if_evc_fifo_desc_ref_table[core_id]->fifo->read_idx = 0u;
			}
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
FUNC(Std_ReturnType, PDUR_CODE)
pdur_evc_fifo_push(pdur_core_id_type core_id, const pdur_evc_data_type *evc_data, pdur_evc_fifo_sync_type sync_type)
{
	Std_ReturnType ret_val = E_NOT_OK;

	if (evc_data == NULL) {
		return ret_val;
	}
	const pdur_evc_fifo_desc_type *const *pdur_evc_fifo_desc_ref_table = NULL;
	if (sync_type == PDUR_ASYNC_EVCFIFO) {
		if (core_id >= PDUR_EVC_FIFO_CORE_MAX_NUM) {
			return E_NOT_OK;
		}
		pdur_evc_fifo_desc_ref_table = pdur_get_evc_fifo_desc_ref_table();
	} else if (sync_type == PDUR_SYNC_EVCFIFO) {
		if (core_id >= PDUR_IF_EVC_FIFO_CORE_MAX_NUM) {
			return E_NOT_OK;
		}
		pdur_evc_fifo_desc_ref_table = pdur_get_if_evc_fifo_desc_ref_table();
	} else {
		return E_NOT_OK;
	}
	const pdur_evc_fifo_desc_type *const p_evc_fifo_desc = pdur_evc_fifo_desc_ref_table[core_id];

	if (p_evc_fifo_desc == NULL) {
		return E_NOT_OK;
	}
	if (p_evc_fifo_desc->fifo == NULL) {
		return E_NOT_OK;
	}
	Std_ReturnType spinlock_res = pdur_get_evc_fifo_spin_lock(p_evc_fifo_desc);

	if (spinlock_res != E_OK) {
		return E_NOT_OK;
	}
	if (pdur_evc_fifo_is_full(p_evc_fifo_desc->fifo->write_idx, p_evc_fifo_desc->fifo->read_idx,
				  p_evc_fifo_desc->size) == (boolean)FALSE) {
		(void)memcpy(&(p_evc_fifo_desc->data[p_evc_fifo_desc->fifo->write_idx]), evc_data,
			     sizeof(pdur_evc_data_type));
		pdur_fifo_idx_increment(&(p_evc_fifo_desc->fifo->write_idx), p_evc_fifo_desc->size);
		ret_val = E_OK;
	} else {
		ret_val = E_NOT_OK;
	}
	(void)pdur_release_evc_fifo_spin_lock(p_evc_fifo_desc);
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE)
pdur_evc_fifo_pop(pdur_core_id_type core_id, pdur_evc_data_type *evc_data, pdur_evc_fifo_sync_type sync_type)
{
	Std_ReturnType ret_val;

	if (evc_data == NULL) {
		return E_NOT_OK;
	}
	const pdur_evc_fifo_desc_type *const *pdur_evc_fifo_desc_ref_table = NULL;
	if (sync_type == PDUR_ASYNC_EVCFIFO) {
		if (core_id >= PDUR_EVC_FIFO_CORE_MAX_NUM) {
			return E_NOT_OK;
		}
		pdur_evc_fifo_desc_ref_table = pdur_get_evc_fifo_desc_ref_table();
	} else if (sync_type == PDUR_SYNC_EVCFIFO) {
		if (core_id >= PDUR_IF_EVC_FIFO_CORE_MAX_NUM) {
			return E_NOT_OK;
		}
		pdur_evc_fifo_desc_ref_table = pdur_get_if_evc_fifo_desc_ref_table();
	} else {
		return E_NOT_OK;
	}
	const pdur_evc_fifo_desc_type *const p_evc_fifo_desc = pdur_evc_fifo_desc_ref_table[core_id];

	if (p_evc_fifo_desc == NULL) {
		return E_NOT_OK;
	}
	if (p_evc_fifo_desc->fifo == NULL) {
		return E_NOT_OK;
	}
	Std_ReturnType spinlock_res = pdur_get_evc_fifo_spin_lock(p_evc_fifo_desc);

	if (spinlock_res != E_OK) {
		return E_NOT_OK;
	}
	if (pdur_fifo_is_empty(p_evc_fifo_desc->fifo->write_idx, p_evc_fifo_desc->fifo->read_idx) == (boolean)FALSE) {
		(void)memcpy(evc_data, &(p_evc_fifo_desc->data[p_evc_fifo_desc->fifo->read_idx]),
			     sizeof(pdur_evc_data_type));
		pdur_fifo_idx_increment(&(p_evc_fifo_desc->fifo->read_idx), p_evc_fifo_desc->size);
		ret_val = E_OK;
	} else {
		ret_val = E_NOT_OK;
	}
	(void)pdur_release_evc_fifo_spin_lock(p_evc_fifo_desc);
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif
