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
#include "Compiler.h"

#define DEM_AGING_CNTR_MAX 0xFFu
#define DEM_AGED_CNTR_MAX 0xFFu

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_SUPPORT_AGING == STD_ON) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(boolean, DEM_CODE) age_event(dem_event_info_rec_type *event_info, boolean *event_deleted)
{
	boolean updated_memory = FALSE;
	const dem_event_parameter_type *event_param = NULL;
	uint8 event_status = event_info->event_status_rec;
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	uint8 group_index = dem_get_event_combined_group_index(event_info->event_id);

	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		event_status = dem_get_event_combined_group_uds_status(group_index);
	}

	/* 对于事件组合，只处理master event */
	if (dem_get_master_event(event_info->event_id) != event_info->event_id) {
		return updated_memory;
	}
#endif
	if (DEM_IS_MASK_UNSET(event_status, DEM_CONFIRMED_DTC, uint8)) {
		return updated_memory;
	}

	lookup_event_param(event_info->event_id, &event_param);
	if (NULL == event_param) {
		return updated_memory;
	}

	if (event_info->passed_during_aging_cycle && !event_info->failed_during_aging_cycle &&
	    DEM_IS_MASK_UNSET(event_status, DEM_TEST_FAILED, uint8)) {
		if (event_info->aging_cnt < DEM_AGING_CNTR_MAX) {
			event_info->aging_cnt++;
			updated_memory = TRUE;
		}
		if (event_info->aging_cnt >= event_param->event_class_ref->aging_cycle_counter_threshold) {
			event_info->aging_cnt = 0u;
#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
			if (event_info->aged_cnt < DEM_AGED_CNTR_MAX) {
				event_info->aged_cnt++;
			}
#endif

#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				if ((dem_event_combination_on_storage_clear_allowed(group_index) == TRUE) &&
				    (dem_clear_combined_event(group_index, FALSE, TRUE) == TRUE)) {
					updated_memory = TRUE;
					*event_deleted = TRUE;
				}
			} else {
#endif
				if (dem_delete_dtc_data(event_param) == TRUE) {
					DEM_CLEAR_MASK(event_info->event_status_rec,
						       DEM_CONFIRMED_DTC | DEM_PENDING_DTC, dem_event_status_rec_type);
					updated_memory = TRUE;
					*event_deleted = TRUE;
					(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "EventId:%u has been aged \r\n",
						       event_info->event_id);
				}
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
			}
#endif

			event_info->failure_counter = 0;
		}
	} else if ((TRUE == event_info->failed_during_aging_cycle) && (0u != event_info->aging_cnt)) {
		event_info->aging_cnt = 0;
		updated_memory = TRUE;
	} else {
		/* do nothing.. */
	}

	return updated_memory;
}

static FUNC(void, DEM_CODE) process_age_event(dem_event_info_rec_type *event_info)
{
	boolean event_deleted = FALSE;
	boolean updated_memory = FALSE;
	dem_primary_mem_type *event_rec = NULL;
	const dem_event_parameter_type *event_param = NULL;
	dem_event_status_rec_type old_status = event_info->event_status_rec;
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = old_combined_status;
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
#endif

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	group_index = dem_get_event_combined_group_index(event_info->event_id);
	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		old_combined_status = dem_get_event_combined_group_uds_status(group_index);
	}
#endif
	updated_memory = age_event(event_info, &event_deleted);

	lookup_event_param(event_info->event_id, &event_param);

	if (old_status != event_info->event_status_rec) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
		if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
			/* event状态发生改变，更新事件组状态 */
			dem_handle_event_combined_group_status_changed(group_index);
			new_combined_status = dem_get_event_combined_group_uds_status(group_index);
			if (old_combined_status != new_combined_status) {
				dem_notify_dtc_status_changed(event_param, old_combined_status, new_combined_status);
			}
		} else {
			dem_notify_dtc_status_changed(event_param, old_status, event_info->event_status_rec);
		}
#else
		{
			dem_notify_dtc_status_changed(event_param, old_status, event_info->event_status_rec);
		}
#endif
		dem_notify_event_status_change(event_param, old_status, event_info->event_status_rec);
	}
	if (FALSE == updated_memory) {
		return;
	}
	if (FALSE == event_deleted) {
		if (E_OK == dem_store_event_mem(event_param, event_info, FALSE, &event_rec)) {
			dem_notify_event_storage_changed(event_param);
		}
	} else {
		dem_notify_event_storage_changed(event_param);
#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
		dem_set_event_block_changed(event_param->event_class_ref->event_destination, event_info->event_id);
#endif
	}

	return;
}

FUNC(std_return_type, DEM_CODE) dem_handle_aging(dem_opcycle_id_type opcycle_id)
{
	uint16 i = 0;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	dem_dtc_origin_type origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
	const dem_event_parameter_type *event_param = NULL;

	if (opcycle_id >= DEM_OPERATION_CYCLE_ID_ENDMARK) {
		DEM_DET_REPORT(DEM_DEM_SETOPERATIONCYCLESTATE_ID_DEM_E_PARAM_DATA);
		return E_NOT_OK;
	}

	for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}
		lookup_event_param(event_id, &event_param);
		if (NULL == event_param) {
			continue;
		}
		if (NULL == event_param->event_class_ref) {
			continue;
		}
		if (FALSE == event_param->event_class_ref->aging_allowed) {
			continue;
		}
		if (event_param->event_class_ref->aging_cycle_ref != opcycle_id) {
			continue;
		}

		origin = event_param->event_class_ref->event_destination;
		if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == origin) {
			process_age_event(&g_dem_event_info_rec[i]);
		}
	}

	return E_OK;
}
#endif

#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
FUNC(uint8, DEM_CODE) dem_get_event_aged_cnt(uint16 event_aged_data_index)
{
	if (event_aged_data_index < DEM_INVALID_ADEG_DATA_INDEX) {
		return g_dem_aged_mem_data.aged_cnt[event_aged_data_index];
	}

	return 0u;
}

FUNC(void, DEM_CODE) dem_set_event_aged_cnt(const dem_event_parameter_type *event_param, uint8 aged_cnt)
{
	if (NULL == event_param) {
		return;
	}

	uint16 event_aged_data_index = event_param->event_aged_data_index;

	if (event_aged_data_index < DEM_INVALID_ADEG_DATA_INDEX) {
		g_dem_aged_mem_data.aged_cnt[event_aged_data_index] = aged_cnt;
		dem_set_aged_data_blcok_changed(event_param);
	}
}
#endif

#if (DEM_SUPPORT_AGING == STD_ON)
FUNC(void, DEM_CODE)
dem_handle_aging_cnt_on_failed(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_info)
{
#if (DEM_AGING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_CYCLE == STD_ON)
	if (is_opcycle_started(event_param->event_class_ref->failure_cycle_ref)) {
		event_info->aging_cnt = 0;
	}
#elif (DEM_AGING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_OR_AGING_CYCLE == STD_ON)
	if (is_opcycle_started(event_param->event_class_ref->failure_cycle_ref) ||
	    is_opcycle_started(event_param->event_class_ref->aging_cycle_ref)) {
		event_info->aging_cnt = 0;
	}
#else
	(void)event_param;
	event_info->aging_cnt = 0;
#endif
}
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"