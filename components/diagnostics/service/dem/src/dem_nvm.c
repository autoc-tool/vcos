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

#include "dem_internal.h"
#if (CONFIG_MODULE_NVM == 1)
#include "nvm.h"
#include "nvm_cfg.h"
#endif
#include "Compiler.h"

#define DEM_NO_NVM_BLOCK (uint16)0xFFu
#define DEM_NVM_AVALIABLE_BLOCK MIN(DEM_NOF_SUPPORTED_NVM_BLOCKS, DEM_MAX_NUMBER_NVM_BLOCKS)

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

static FUNC(void, DEM_CODE) dem_nvm_set_nvm_block_changed(uint16 block_id, boolean immediate_storage)
{
#if (CONFIG_MODULE_NVM == 1) && (DEM_USE_NVM == STD_ON)
	nvram_block_id_index nvm_block_id = DEM_NO_NVM_BLOCK;

	if (block_id >= (uint8)DEM_MAX_NUMBER_NVM_BLOCKS) {
		return;
	}

	if (NULL != g_dem_nvm_mgr->nvram_block_id) {
		nvm_block_id = g_dem_nvm_mgr->nvram_block_id[block_id];
	}

	if (DEM_NO_NVM_BLOCK != nvm_block_id) {
		g_dem_nvm_block_status[block_id].dirty = (boolean)TRUE;
		if (immediate_storage == (boolean)TRUE) {
			g_dem_nvm_block_status[block_id].pending_write = (boolean)TRUE;
		}
	}
#else
	(void)block_id;
	(void)immediate_storage;
#endif
}

static FUNC(uint16, DEM_CODE) dem_nvm_get_event_storage_index(dem_dtc_origin_type origin, dem_event_id_type event_id)
{
	uint16 event_buff_index = DEM_EVENT_BUFF_INVAILD_INDEX;
	dem_primary_mem_type *event_buff = NULL;
	uint16 event_buffer_size = 0u;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == origin) {
		event_buff = g_dem_pri_mem_data;
		event_buffer_size = DEM_MAX_NUMBER_EVENT_PRI_MEM;
	}
#endif
	if (NULL == event_buff) {
		return DEM_EVENT_BUFF_INVAILD_INDEX;
	}

	for (uint16 i = 0u; i < event_buffer_size; i++) {
		if (event_buff[i].event_id == event_id) {
			event_buff_index = i;
			break;
		}
	}

	return event_buff_index;
}

#if (CONFIG_MODULE_NVM == 1) && (DEM_USE_NVM == STD_ON)
static FUNC(void, DEM_CODE)
	dem_nvm_main_function_set_ram_block_status(nvram_block_id_index nvm_block_id, uint8 block_id)
{
	nvm_request_result_type req_res;

#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
	if (nvm_get_error_status(nvm_block_id, &req_res) != E_OK) {
		return;
	}
	if (NVM_REQ_PENDING == req_res) {
		return;
	}
	if (nvm_set_ram_block_status(nvm_block_id, TRUE) != E_OK) {
		return;
	}
	g_dem_nvm_block_status[block_id].dirty = FALSE;
	if (TRUE == g_dem_nvm_block_status[block_id].pending_write) {
		g_dem_nvm_block_status[block_id].state = DEM_NVM_WRITEBLOCK;
	} else {
		g_dem_nvm_block_status[block_id].state = DEM_NVM_WAIT_DONE;
	}
#else
	(void)nvm_block_id;
	(void)req_res;
	(void)block_id;
#endif

	return;
}

static FUNC(void, DEM_CODE) dem_nvm_main_function_write_block(nvram_block_id_index nvm_block_id, uint8 block_id)
{
	nvm_request_result_type req_res;

	if (nvm_get_error_status(nvm_block_id, &req_res) != E_OK) {
		return;
	}
	if (NVM_REQ_PENDING == req_res) {
		return;
	}

	if (nvm_write_block(nvm_block_id, NULL) == E_OK) {
		g_dem_nvm_block_status[block_id].pending_write = FALSE;
		g_dem_nvm_block_status[block_id].dirty = FALSE;
		g_dem_nvm_block_status[block_id].state = DEM_NVM_WAIT_DONE;
	} else {
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
		g_dem_nvm_block_status[block_id].state = DEM_NVM_WRITEBLOCK_RETRY;
#endif
	}

	return;
}

static FUNC(void, DEM_CODE) dem_nvm_main_function_wait_done(nvram_block_id_index nvm_block_id, uint8 block_id)
{
	nvm_request_result_type req_res;

	if (nvm_get_error_status(nvm_block_id, &req_res) == E_OK) {
		if (NVM_REQ_PENDING != req_res) {
			g_dem_nvm_block_status[block_id].state = DEM_NVM_IDLE;
		}
	}

	return;
}
#endif

FUNC(void, DEM_CODE)
dem_nvm_set_event_block_changed(dem_dtc_origin_type origin, dem_event_id_type event_id, boolean immediate_storage)
{
	uint16 event_buff_index = DEM_EVENT_BUFF_INVAILD_INDEX;
	dem_event_id_type master_event_id = dem_get_master_event_id(event_id);

	event_buff_index = dem_nvm_get_event_storage_index(origin, master_event_id);
	if (DEM_EVENT_BUFF_INVAILD_INDEX != event_buff_index) {
		dem_nvm_set_nvm_block_changed(event_buff_index, immediate_storage);
	}
}

FUNC(void, DEM_CODE)
dem_nvm_set_event_block_changed_with_indx(dem_dtc_origin_type origin, uint16 event_buff_index,
					  dem_event_id_type event_id, boolean immediate_storage)
{
	/* 初始值不进行处理 */
	if (DEM_EVENT_BUFF_INVAILD_INDEX == event_buff_index) {
		return;
	}

	dem_nvm_set_nvm_block_changed(event_buff_index, immediate_storage);
}

FUNC(void, DEM_CODE) dem_nvm_set_mgmt_blcok_changed(boolean immediate_storage)
{
	dem_nvm_set_nvm_block_changed((uint16)DEM_NVMBLOCK_MGMT, immediate_storage);
}

#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
void dem_nvm_set_aged_data_blcok_changed(boolean immediate_storage)
{
	dem_nvm_set_nvm_block_changed((uint16)DEM_AGED_COUNTER_DATA, immediate_storage);
}
#endif

FUNC(void, DEM_CODE) dem_nvm_set_event_status_data_blcok_changed(boolean immediate_storage)
{
	dem_nvm_set_nvm_block_changed((uint16)DEM_NVMBLOCK_STATUS, immediate_storage);
}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
FUNC(void, DEM_CODE) dem_nvm_set_event_available_data_blcok_changed(boolean immediate_storage)
{
	dem_nvm_set_nvm_block_changed((uint16)DEM_NVMBLOCK_AVAILABLE_DATA, immediate_storage);
}
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))

FUNC(void, DEM_CODE) dem_nvm_set_permanent_block_changed(boolean immediate_storage)
{
	dem_nvm_set_nvm_block_changed((uint16)DEM_NVMBLOCK_PERMANENT, immediate_storage);
}

FUNC(void, DEM_CODE) dem_nvm_set_obd_ff_block_changed(boolean immediate_storage)
{
	dem_nvm_set_nvm_block_changed((uint16)(DEM_NVMBLOCK_OBD_FREEZE_FRAME), immediate_storage);
}

FUNC(void, DEM_CODE) dem_nvm_set_indicator_status_block_changed(boolean immediate_storage)
{
	dem_nvm_set_nvm_block_changed((uint16)(DEM_NVMBLOCK_INDICATOR_STATUS), immediate_storage);
}
#endif

#if (CONFIG_MODULE_NVM == 1) && (DEM_USE_NVM == STD_ON)
FUNC(void, DEM_CODE) dem_nvm_init(void)
{
	g_dem_nvm_mgr = dem_get_general_ptr();
	(void)memset(&g_dem_nvm_block_status[0], 0, DEM_NOF_SUPPORTED_NVM_BLOCKS * sizeof(dem_nvm_block_status_type));
}

FUNC(void, DEM_CODE) dem_nvm_main_function(void)
{
#if (DEM_NVM_AVALIABLE_BLOCK > 0)
	nvram_block_id_index nvm_block_id = DEM_NO_NVM_BLOCK;
	boolean should_write_block = TRUE;

#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	g_dem_nvm_storage_time += DEM_TASK_TIME;
	if (g_dem_nvm_storage_time >= g_dem_nvm_storage_time) {
		g_dem_nvm_storage_time = 0;
	} else {
		should_write_block = FALSE;
	}
#endif
	for (uint8 block_id = 0u; block_id < DEM_NVM_AVALIABLE_BLOCK; block_id++) {
		if ((DEM_NVM_IDLE == g_dem_nvm_block_status[block_id].state) &&
		    ((TRUE == g_dem_nvm_block_status[block_id].pending_write) ||
		     (TRUE == g_dem_nvm_block_status[block_id].dirty))) {
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
			g_dem_nvm_block_status[block_id].state = DEM_NVM_SETRAMBLOCKSTATUS;
#else
			g_dem_nvm_block_status[block_id].state = DEM_NVM_WRITEBLOCK;
#endif
		}

		if (NULL != g_dem_nvm_mgr->nvram_block_id) {
			nvm_block_id = g_dem_nvm_mgr->nvram_block_id[block_id];
		}

		if (DEM_NO_NVM_BLOCK == nvm_block_id) {
			continue;
		}

		switch (g_dem_nvm_block_status[block_id].state) {
			case DEM_NVM_SETRAMBLOCKSTATUS:
				dem_nvm_main_function_set_ram_block_status(nvm_block_id, block_id);
				break;
			case DEM_NVM_WRITEBLOCK:
				if (should_write_block) {
					dem_nvm_main_function_write_block(nvm_block_id, block_id);
				}
				break;
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
			case DEM_NVM_WRITEBLOCK_RETRY:
				dem_nvm_main_function_write_block(nvm_block_id, block_id);
				break;
#endif
			case DEM_NVM_WAIT_DONE:
				dem_nvm_main_function_wait_done(nvm_block_id, block_id);
				break;
			default:
				break;
		}
	}
#endif

	return;
}

FUNC(void, DEM_CODE) dem_nvm_shutdown(void)
{
#if (DEM_NVM_AVALIABLE_BLOCK > 0)
	nvm_request_result_type req_res;
	nvram_block_id_index nvm_block_id = DEM_NO_NVM_BLOCK;

	for (uint8 block_id = 0u; block_id < DEM_NVM_AVALIABLE_BLOCK; block_id++) {
		if ((DEM_NVM_IDLE == g_dem_nvm_block_status[block_id].state) &&
		    ((TRUE == g_dem_nvm_block_status[block_id].pending_write) ||
		     (TRUE == g_dem_nvm_block_status[block_id].dirty))) {
			if (NULL != g_dem_nvm_mgr->nvram_block_id) {
				nvm_block_id = g_dem_nvm_mgr->nvram_block_id[block_id];
			}

			if (DEM_NO_NVM_BLOCK == nvm_block_id) {
				continue;
			}

			if (nvm_get_error_status(nvm_block_id, &req_res) != E_OK) {
				continue;
			}
			if (NVM_REQ_PENDING == req_res) {
				continue;
			}
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
			if (nvm_set_ram_block_status(nvm_block_id, TRUE) != E_OK) {
				continue;
			}
#endif
			g_dem_nvm_block_status[block_id].pending_write = FALSE;
			g_dem_nvm_block_status[block_id].dirty = FALSE;
			g_dem_nvm_block_status[block_id].state = DEM_NVM_IDLE;
		}
	}
#endif

	return;
}

#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"