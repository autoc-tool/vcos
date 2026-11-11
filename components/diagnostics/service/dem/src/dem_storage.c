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

#define IS_SUPPORTED_ORIGIN(_x) (DEM_DTC_ORIGIN_PRIMARY_MEMORY == (_x))

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
/* ****************************************************************************
 * get_permanent_status
 *****************************************************************************/
static FUNC(uint8, DEM_CODE) get_permanent_status(uint8 index)
{
	return g_dem_perm_mem_data.perm_data_entry[index].status;
}

/* ****************************************************************************
 * set_permanent_event
 *****************************************************************************/
static FUNC(void, DEM_CODE) set_permanent_event(uint8 index, dem_event_id_type event_id)
{
	g_dem_perm_mem_data.perm_data_entry[index].event_id = event_id;
}

/* ****************************************************************************
 * remove_permanent_entry
 *****************************************************************************/
static FUNC(void, DEM_CODE) remove_permanent_entry(uint8 index)
{
	set_permanent_event(index, DEM_EVENT_ID_NULL);
	dem_set_permanent_status(index, DEM_PERMANENT_DTC_UNUSED);
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_permanent_block_changed(TRUE);
#else
	dem_nvm_set_permanent_block_changed(FALSE);
#endif
}

/* ****************************************************************************
 * find_permanent_entry
 *****************************************************************************/
static FUNC(uint8, DEM_CODE) find_permanent_entry(dem_event_id_type event_id)
{
	uint8 index = 0u;

	for (index = 0; index < DEM_MAX_NUMBER_EVENT_PERM_MEM; index++) {
		if (event_id == dem_get_permanent_event(index)) {
			break;
		}
	}

	return index;
}
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
static FUNC_P2CONST(dem_storage_condition_group_type, AUTOMATIC, DEM_CODE)
	get_storage_condition_group(const dem_event_class_type *event_class)
{
	const dem_storage_condition_group_type *storage_condition_group_list = NULL;

	if ((NULL == event_class) || (DEM_INVALID_U16_REF_INDEX == event_class->storage_condition_group_ref_index)) {
		return NULL;
	}

	storage_condition_group_list = dem_get_storage_condition_group_list();
	if (NULL == storage_condition_group_list) {
		return NULL;
	}

	return &(storage_condition_group_list[event_class->storage_condition_group_ref_index]);
}

static FUNC_P2VAR(const dem_storage_condition_type *const, AUTOMATIC, DEM_CODE)
	get_storage_condition_base(const dem_storage_condition_group_type *storage_condition_group)
{
	const dem_storage_condition_type *const *storage_condition_group_class = NULL;

	if (NULL == storage_condition_group) {
		return NULL;
	}

	storage_condition_group_class = dem_get_storage_condition_group_class_list();
	if (NULL == storage_condition_group_class) {
		return NULL;
	}

	return &(storage_condition_group_class[storage_condition_group->storage_condition_base_index]);
}
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(boolean, DEM_CODE) is_storaged_entry_valid(dem_event_id_type event_id, dem_dtc_origin_type origin)
{
	const dem_event_parameter_type *event_id_param = NULL;
	dem_event_info_rec_type *event_info_rec = NULL;

	lookup_event_param(event_id, &event_id_param);

	if (NULL == event_id_param) {
		return FALSE;
	}

	lookup_event_info_rec(event_id, &event_info_rec);

	if ((NULL == event_info_rec) || (FALSE == dem_is_dtc_origin_match(origin, event_id_param, FALSE))) {
		return FALSE;
	}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if (FALSE == event_info_rec->is_available) {
		return FALSE;
	}
#endif

	return TRUE;
}
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
static FUNC(boolean, DEM_CODE) store_dtc_permanent_memory(dem_event_id_type event_id)
{
	uint8 index = find_permanent_entry(event_id);

	if (index == DEM_MAX_NUMBER_EVENT_PERM_MEM) {
		index = find_permanent_entry(DEM_EVENT_ID_NULL);
	}

	if (index != DEM_MAX_NUMBER_EVENT_PERM_MEM) {
		set_permanent_event(index, event_id);
		dem_set_permanent_status(index, DEM_PERMANENT_DTC_ACTIVE);
		return TRUE;
	}

	return FALSE;
}

static FUNC(boolean, DEM_CODE) delete_dtc_permanent_memory(dem_event_id_type event_id)
{
	for (uint8 i = 0; i < DEM_MAX_NUMBER_EVENT_PERM_MEM; i++) {
		if (event_id == dem_get_permanent_event(i)) {
			remove_permanent_entry(i);
			return TRUE;
		}
	}

	return FALSE;
}

static FUNC(boolean, DEM_CODE) is_dtc_permanent_storage_condition_fulfilled(const dem_event_parameter_type *event_param,
									    dem_event_status_rec_type event_status)
{
	if (DEM_IS_MASK_SET(event_status, DEM_CONFIRMED_DTC, dem_event_status_rec_type) &&
	    (TRUE == dem_is_emission_event_ref_to_mil(event_param)) &&
	    (TRUE == dem_is_event_activates_mil(event_param))) {
		return TRUE;
	}

	return FALSE;
}

static FUNC(boolean, DEM_CODE) handle_dtc_store_permanent(const dem_event_parameter_type *event_param,
							  dem_event_status_rec_type event_status)
{
	boolean res = FALSE;

	if (is_dtc_permanent_storage_condition_fulfilled(event_param, event_status)) {
		res = store_dtc_permanent_memory(event_param->event_id);
	}

	return res;
}
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
static FUNC(void, DEM_CODE)
	process_permanent_storage_on_event_failed(const dem_event_parameter_type *event_param,
						  const dem_event_info_rec_type *event_info_rec)
{
	if (handle_dtc_store_permanent(event_param, event_info_rec->event_status_rec) == TRUE) {
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
		dem_nvm_set_permanent_block_changed(TRUE);
#else
		dem_nvm_set_permanent_block_changed(FALSE);
#endif
	}

	return;
}
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(boolean, DEM_CODE) is_ext_data_stored(const dem_primary_mem_type *event_rec)
{
#if (DEM_EXT_DATA_IN_PRI_MEM)
	for (uint8 i = 0u; i < DEM_MAX_RECORD_NUMBERS_IN_EXD_REC_NUM_CLASS; i++) {
		for (uint8 j = 0u; j < DEM_MAX_SIZE_EXT_DATA; j++) {
			if (event_rec->ext_data[i][j] != 0u) {
				return TRUE;
			}
		}
	}
#endif
	return FALSE;

}

static FUNC(void, DEM_CODE) update_event_info_rec_from_storage(const dem_primary_mem_type *event_rec)
{
	dem_event_info_rec_type *event_info = NULL;
	const dem_event_parameter_type *event_param = NULL;
	dem_event_status_rec_type event_status_rec = dem_get_event_status_data(event_rec->event_id);
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
	uint8 event_combined_group_uds_status = 0;
#endif

	lookup_event_info_rec(event_rec->event_id, &event_info);
	lookup_event_param(event_rec->event_id, &event_param);

	if ((NULL == event_info) || (NULL == event_param)) {
		return;
	}

	DEM_SET_MASK(event_info->event_status_rec, event_status_rec & (DEM_PENDING_DTC | DEM_CONFIRMED_DTC),
		     dem_event_status_rec_type);

	if (dem_is_fulfill_confirm(event_param, event_info)) {
		DEM_SET_MASK(event_info->event_status_rec, DEM_CONFIRMED_DTC, dem_event_status_rec_type);
	}

	DEM_SET_MASK(event_info->event_status_rec, event_status_rec & DEM_TEST_FAILED_SINCE_LAST_CLEAR,
		     dem_event_status_rec_type);

	if (DEM_IS_MASK_UNSET(event_info->event_status_rec, event_status_rec & DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR,
			      dem_event_status_rec_type)) {
		DEM_CLEAR_MASK(event_info->event_status_rec, DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR,
			       dem_event_status_rec_type);
	}

#if defined(DEM_USE_INDICATORS)
	if (dem_is_indicator_fulfilled(event_param)) {
		DEM_SET_MASK(event_info->event_status_rec, DEM_WARNING_INDICATOR_REQUESTED,
			     dem_event_status_rec_type);
	}
#endif

#if (DEM_CFG_DATA_OCCCTR == STD_ON)
	event_info->occctr = event_rec->occctr;
#endif

	event_info->failure_counter += dem_get_event_trip_cnt(event_rec->event_id);

#if (DEM_SUPPORT_AGING == STD_ON)
	event_info->aging_cnt += (uint8)(event_rec->aging_cnt);
#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
	event_info->aged_cnt += dem_get_event_aged_cnt(event_param->event_aged_data_index);
#endif
#endif
	event_info->ext_data_stored = is_ext_data_stored(event_rec);

#if (DEM_FF_DATA_IN_PRI_MEM)
	for (uint8 i = 0; i < DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS; i++) {
		for (uint8 j = 0; j < DEM_MAX_SIZE_FF_DATA; j++) {
			if ((event_rec->ff_data[i][j] != 0u) &&
				(event_info->stored_ff_recnum < DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS)) {
				event_info->stored_ff_recnum++;
				break;
			}
		}
	}
#endif

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	group_index = dem_get_event_combined_group_index(event_rec->event_id);
	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		event_combined_group_uds_status = dem_calculate_event_combine_group_uds_status(group_index);
		dem_set_event_combined_group_uds_status(group_index, event_combined_group_uds_status);
	}
#endif

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
	if (DEM_IS_MASK_UNSET(event_info->event_status_rec, DEM_TEST_FAILED_THIS_OPERATION_CYCLE,
			      dem_event_status_rec_type)) {
		event_info->timestamp = event_rec->timestamp;
	}
#endif

	return;
}
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
static FUNC(uint8, DEM_CODE) get_memory_overflow(void)
{
	return g_dem_mgmt_mem_data.memory_overflow;
}

static FUNC(void, DEM_CODE) set_memory_overflow(uint8 memory_overflow)
{
	g_dem_mgmt_mem_data.memory_overflow = memory_overflow;
}
#endif
#endif

static FUNC(boolean, DEM_CODE) is_event_dtc_disabled_update_record(const dem_event_parameter_type *event_param)
{
	boolean disabled = FALSE;
	const dem_dtc_class_type *dtc_class = dem_get_event_dtc_class(event_param);

	if ((DEM_NO_DTC_DISABLED != g_dem_dtc_record_disabled.dtc) && (NULL != event_param) && (NULL != dtc_class) &&
	    (DEM_NO_DTC != dtc_class->uds_dtc) && (g_dem_dtc_record_disabled.dtc == dtc_class->uds_dtc) &&
	    (g_dem_dtc_record_disabled.origin == event_param->event_class_ref->event_destination)) {
		disabled = TRUE;
	}

	return disabled;
}

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM && (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON))
static FUNC(boolean, DEM_CODE) is_event_holds_obd_ff(const dem_event_parameter_type *event_param)
{
	boolean ret = FALSE;

	for (uint32 i = 0; i < DEM_MAX_NUMBER_EVENT_PRI_MEM; i++) {
		for (uint32 j = 0; (j < DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS) &&
				   (event_param->event_id == g_dem_pri_mem_data[i].event_id);
		     j++) {
			if ((event_param->event_id == g_dem_pri_mem_data[i].ff_record[j].event_id) &&
			    (DEM_FREEZE_FRAME_OBD == g_dem_pri_mem_data[i].ff_record[j].kind)) {
				ret = TRUE;
				break;
			}
		}
	}

	return ret;
}
#endif

static FUNC(boolean, DEM_CODE) is_event_displacement_prohibited(const dem_event_parameter_type *candidate_event_param,
								const dem_event_parameter_type *event_param)
{
#if (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON)
	dem_event_status_rec_type event_status;

	(void)dem_get_event_status(candidate_event_param->event_id, &event_status);

	if (FALSE == dem_is_emission_related_event(candidate_event_param)) {
		return FALSE;
	}

	if (DEM_IS_MASK_SET(event_status, DEM_PENDING_DTC, dem_event_status_rec_type)) {
		return TRUE;
	}

#if defined(DEM_USE_INDICATORS)
	if (TRUE == dem_is_event_activates_mil(candidate_event_param)) {
		return TRUE;
	}
#endif

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
	if (is_event_holds_obd_ff(candidate_event_param) && (candidate_event_param->event_class_ref->event_priority <=
							 event_param->event_class_ref->event_priority)) {
		return TRUE;
	}
#endif

	return FALSE;
#else
	(void)candidate_event_param;
	(void)event_param;

	return FALSE;
#endif
}

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC_P2VAR(dem_primary_mem_type, AUTOMATIC, DEM_CODE)
	get_event_for_displacement(const dem_event_parameter_type *event_param,
				   dem_primary_mem_type *event_buffer, uint32 buffer_size,
				   uint16 *stored_event_buff_index)
{
	uint8 event_prio = 0xFFu;
	boolean event_passive = FALSE;
	boolean passive_candidate_found = FALSE;
	boolean active_candidate_found = FALSE;
	boolean event_data_removal_prohibited = FALSE;
	boolean prohibit_remove = FALSE;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	uint32 remove_index = 0;
	uint32 oldest_passive_timestamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
	uint32 oldest_active_timestamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
	const dem_event_parameter_type *candidate_event_param = NULL;
	dem_primary_mem_type *event_rec_to_remove = NULL;

	for (uint32 i = 0; i < buffer_size; i++) {
		event_id = event_buffer[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

		lookup_event_param(event_id, &candidate_event_param);
		if (DEM_NO_DTC_DISABLED != g_dem_dtc_record_disabled.dtc) {
			event_data_removal_prohibited =
				is_event_dtc_disabled_update_record(candidate_event_param);
		} else {
			event_data_removal_prohibited = FALSE;
		}

		if (TRUE == event_data_removal_prohibited) {
			continue;
		}

		prohibit_remove = is_event_displacement_prohibited(candidate_event_param, event_param);
		if (TRUE == prohibit_remove) {
			continue;
		}

		event_prio = dem_get_event_priority(event_id);
		event_passive = dem_is_passive_event(event_id);
		if (event_passive && (event_param->event_class_ref->event_priority <= event_prio) &&
			(oldest_passive_timestamp > event_buffer[i].timestamp)) {
			oldest_passive_timestamp = event_buffer[i].timestamp;
			remove_index = i;
			passive_candidate_found = TRUE;
		}

		if ((!passive_candidate_found) &&
		    (event_param->event_class_ref->event_priority < event_prio) &&
		    (oldest_active_timestamp > event_buffer[i].timestamp)) {
			oldest_active_timestamp = event_buffer[i].timestamp;
			remove_index = i;
			active_candidate_found = TRUE;
		}
	}

	if (passive_candidate_found || active_candidate_found) {
		event_rec_to_remove = &event_buffer[remove_index];
		*stored_event_buff_index = (uint16)remove_index;
	}

	return event_rec_to_remove;
}
#endif

static FUNC(void, DEM_CODE) swap_event_rec(dem_primary_mem_type *rec_src, dem_primary_mem_type *rec_dst)
{
	dem_primary_mem_type rec_tmp;

	(void)memcpy(&rec_tmp, rec_src, sizeof(dem_primary_mem_type));
	(void)memcpy(rec_src, rec_dst, sizeof(dem_primary_mem_type));
	(void)memcpy(rec_dst, &rec_tmp, sizeof(dem_primary_mem_type));
}

static FUNC(void, DEM_CODE) rearrange_event_timestamp(uint32 *timestamp)
{
	uint32 index = 0;
	uint32 i = 0;
	uint32 j = 0;
	uint32 k = 0;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	const uint32 event_buffer_sizes[1] = { DEM_MAX_NUMBER_EVENT_ENTRY_PRI };
	dem_primary_mem_type *event_buffers[1] = { g_dem_pri_mem_data };
	uint32 nof_ori = 1u;
#else
	const uint32 event_buffer_sizes[1] = { 0 };
	dem_primary_mem_type *event_buffers[1] = { NULL };
	uint32 nof_ori = 0;
#endif

	for (index = 0; index < nof_ori; index++) {
		dem_primary_mem_type *event_buffer = event_buffers[index];
		uint32 event_buffer_size = event_buffer_sizes[index];

		for (i = 0; i < event_buffer_size; i++) {
			if (DEM_EVENT_ID_NULL == event_buffer[i].event_id) {
				continue;
			}

			for (j = (event_buffer_size - 1u); j > i; j--) {
				if (DEM_EVENT_ID_NULL == event_buffer[j].event_id) {
					continue;
				}

				if (event_buffer[i].timestamp > event_buffer[j].timestamp) {
					swap_event_rec(&event_buffer[i], &event_buffer[j]);
				}
			}
			event_buffer[i].timestamp = k;
			k++;
		}
	}

	*timestamp = k;
}

#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */

#if (DEM_EXT_DATA_IN_PRI_MEM) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(boolean, DEM_CODE)
	store_ext_data_in_mem(const dem_event_parameter_type *event_param, dem_dtc_origin_type origin,
			      boolean ext_data_stored, dem_primary_mem_type *event_rec)
{
	boolean b_stored_data = FALSE;

	b_stored_data = dem_get_ext_data_content(event_param, ext_data_stored, event_rec);

	if (TRUE == b_stored_data) {
#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
		dem_set_event_block_changed(origin, event_param->event_id);
#endif
	}

	return b_stored_data;
}
#endif /* DEM_EXT_DATA_IN_PRI_MEM */

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
static FUNC(std_return_type, DEM_CODE)
	lookup_and_remove_event_for_displacement(const dem_event_parameter_type *event_param,
						 dem_primary_mem_type *event_buffer, uint32 buffer_size,
						 dem_primary_mem_type **event_rec, dem_dtc_origin_type origin,
						 uint16 *stored_event_buff_index)
{
	(void)origin;
	std_return_type ret = E_NOT_OK;
	dem_event_id_type event_to_remove = DEM_EVENT_ID_NULL;
	dem_primary_mem_type *event_rec_to_remove = NULL;
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = old_combined_status;
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
#endif
	const dem_event_parameter_type *remove_event_param = NULL;
	dem_event_info_rec_type *event_info = NULL;

	event_rec_to_remove =
		get_event_for_displacement(event_param, event_buffer, buffer_size, stored_event_buff_index);
	if (NULL != event_rec_to_remove) {
		event_to_remove = event_rec_to_remove->event_id;
		(void)memset(event_rec_to_remove, 0, sizeof(dem_primary_mem_type));
		(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "EventId:%u has been displaced \r\n",
			       event_rec_to_remove->event_id);
		*event_rec = event_rec_to_remove;
		ret = E_OK;

		lookup_event_info_rec(event_to_remove, &event_info);
		if (NULL != event_info) {
			dem_event_status_rec_type old_status = event_info->event_status_rec;
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			group_index = dem_get_event_combined_group_index(event_to_remove);
			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				old_combined_status = dem_get_event_combined_group_uds_status(group_index);
			}
#endif
			DEM_CLEAR_MASK(event_info->event_status_rec, DEM_CONFIRMED_DTC, dem_event_status_rec_type);

			event_info->failure_counter = 0;
			event_info->failed_during_failure_cycle = FALSE;
			event_info->passed_during_failure_cycle = FALSE;
#if (DEM_SUPPORT_AGING == STD_ON)
			event_info->aging_cnt = 0;
			event_info->failed_during_aging_cycle = FALSE;
			event_info->passed_during_aging_cycle = FALSE;
#endif
			lookup_event_param(event_info->event_id, &remove_event_param);

			if (old_status != event_info->event_status_rec) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
				if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
					dem_handle_event_combined_group_status_changed(group_index);
					new_combined_status = dem_get_event_combined_group_uds_status(group_index);
					if (old_combined_status != new_combined_status) {
						dem_notify_dtc_status_changed(remove_event_param, old_combined_status,
									  new_combined_status);
					}
				} else {
					dem_notify_dtc_status_changed(remove_event_param, old_status,
								  event_info->event_status_rec);
				}
#else
				{
					dem_notify_dtc_status_changed(remove_event_param, old_status,
								  event_info->event_status_rec);
				}
#endif
				dem_notify_event_status_change(remove_event_param, old_status,
							   event_info->event_status_rec);
			}
		} else {
			lookup_event_param(event_to_remove, &remove_event_param);
		}
		if (NULL != remove_event_param) {
#if defined(DEM_USE_INDICATORS)
			(void)dem_reset_indicator_counters(remove_event_param);
#endif
			dem_notify_event_storage_changed(remove_event_param);
		}
	}

	return ret;
}
#endif

static FUNC(std_return_type, DEM_CODE)
	store_event(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_status,
			dem_primary_mem_type *buffer, uint32 buffer_size, dem_dtc_origin_type origin,
			boolean failed_now, dem_primary_mem_type **event_rec)
{
	boolean stored_position_found = FALSE;
	boolean free_position_found = FALSE;
	dem_primary_mem_type *stored_event_rec = NULL;
	dem_primary_mem_type *free_event_rec = NULL;
	std_return_type ret = E_OK;
	uint16 old_aging_cnt = 0u;
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	boolean immediate_storage = FALSE;
#endif
	uint16 stored_event_buff_index = DEM_EVENT_BUFF_INVAILD_INDEX;
	dem_event_id_type master_event_id = dem_get_master_event_id(event_status->event_id);
	dem_event_status_rec_type store_event_status = event_status->event_status_rec;
	const dem_dtc_class_type *dtc_class = NULL;

	for (uint16 i = 0; i < buffer_size; i++) {
		if (buffer[i].event_id == master_event_id) {
			stored_event_rec = &buffer[i];
			*event_rec = stored_event_rec;
			stored_position_found = TRUE;
			stored_event_buff_index = i;
			break;
		} else if ((TRUE != free_position_found) && (DEM_EVENT_ID_NULL == buffer[i].event_id)) {
			/* 找到第一个空闲位置 */
			free_event_rec = &buffer[i];
			free_position_found = TRUE;
			stored_event_buff_index = i;
		} else {
			/* do nothing */
		}
	}

	if (FALSE == stored_position_found) {
		if (FALSE == free_position_found) {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
			if (lookup_and_remove_event_for_displacement(event_param, buffer, buffer_size, event_rec,
								     origin, &stored_event_buff_index) != E_OK) {
				/* 查找不到替换位置时将overflow标识上 */
#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
				dem_set_overflow_indication(event_param->event_class_ref->event_destination, TRUE);
				(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG,
					       "Overflow! EventId:%u storage failure \r\n", event_status->event_id);
#endif
			}
#else
#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
			dem_set_overflow_indication(event_param->event_class_ref->event_destination, TRUE);
			(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "Overflow! EventId:%u storage failure \r\n",
				       event_status->event_id);
#endif
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
		} else {
			*event_rec = free_event_rec;
		}
	}

	if (master_event_id != event_status->event_id) {
		store_event_status = event_status->event_status_rec | (dem_event_status_rec_type)(DEM_CONFIRMED_DTC);
	}

	if (NULL != *event_rec) {
		/* 事件组合存储的event为唯一设置confirmed_dtc位的event */
		(*event_rec)->event_id = master_event_id;
#if (DEM_CFG_DATA_OCCCTR == STD_ON)
		(*event_rec)->occctr = event_status->occctr;
#endif
		dem_set_event_status_data(event_status->event_id, store_event_status);
		dem_set_event_trip_cnt(event_status->event_id, event_status->failure_counter);
#if (DEM_SUPPORT_AGING == STD_ON)
		old_aging_cnt = (*event_rec)->aging_cnt;
		(*event_rec)->aging_cnt = event_status->aging_cnt;
#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
		dem_set_event_aged_cnt(event_param, event_status->aged_cnt);
#endif
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
		(*event_rec)->timestamp = event_status->timestamp;
#endif

#if (DEM_SUPPORT_CYCLES_COUNTER == STD_ON)
		if (TRUE == failed_now) {
			dem_handle_cycle_counters(event_status, stored_position_found);
		}
#endif

#if (DEM_SUPPORT_AGING == STD_ON)
		if ((TRUE == failed_now) || (old_aging_cnt != (*event_rec)->aging_cnt)) {
#else
		if (TRUE == failed_now) {
#endif
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
			dtc_class = dem_get_event_dtc_class(event_param);
			if ((NULL != dtc_class) && (TRUE == dtc_class->immediate_nv_storage)) {
				immediate_storage = TRUE;
			}
			dem_nvm_set_event_block_changed_with_indx(origin, stored_event_buff_index,
								  event_param->event_id, immediate_storage);
#else
			(void)failed_now;
			(void)dtc_class;
			dem_nvm_set_event_block_changed_with_indx(origin, stored_event_buff_index,
								  event_param->event_id, FALSE);
#endif
		}
	} else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
		DEM_DET_REPORT(DEM_DEM_STORE_EVENT_MEM_ID_DEM_E_MEM_EVENT_BUFF_FULL);
#endif
		ret = E_NOT_OK;
	}

	return ret;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */

static FUNC(boolean, DEM_CODE) lookup_ext_data_rec(dem_event_id_type event_id, uint8 **ext_data,
						   dem_dtc_origin_type origin, uint8 ext_data_number)
{
	boolean ed_found = FALSE;
	dem_primary_mem_type *event_buffer = NULL;
	uint32 event_buffer_size = 0;

	*ext_data = NULL;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_PRI_MEM
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == origin) {
		event_buffer = g_dem_pri_mem_data;
		event_buffer_size = DEM_MAX_NUMBER_EVENT_PRI_MEM;
	}
#endif

	if (NULL == event_buffer) {
		return FALSE;
	}

#if (DEM_EXT_DATA_IN_PRI_MEM)
	for (uint8 i = 0; (i < event_buffer_size) && (!ed_found); i++) {
		if (event_buffer[i].event_id != event_id) {
			continue;
		}
#if (DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA > 0)
		for (uint8 j = 0u; j < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA; j++) {
			uint8 ext_data_record_index =
				dem_lookup_ext_data_record_index(&event_buffer[i], ext_data_number);
			if (DEM_MAX_RECORD_NUMBERS_IN_EXD_REC_NUM_CLASS == ext_data_record_index) {
				continue;
			}
			*ext_data = &event_buffer[i].ext_data[ext_data_record_index][0];
			ed_found = TRUE;
			break;
		}
#else
		(void)ext_data;
		(void)ext_data_number;
#endif
	}
#else
	(void)event_buffer_size;
	(void)event_id;
#endif

	return ed_found;
}

#if ((DEM_USE_MEMORY_FUNCTIONS == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
static FUNC(boolean, DEM_CODE) store_ff_data_mem(const dem_event_parameter_type *event_param, const dem_ff_rec_type *ff,
						 dem_primary_mem_type *event_rec, dem_dtc_origin_type origin)
{
	boolean ff_updated = FALSE;
	dem_event_info_rec_type *event_info_rec = NULL;
	dem_event_id_type event_id = dem_get_master_event_id(event_param->event_id);

	lookup_event_info_rec(event_id, &event_info_rec);
	if (NULL == event_info_rec) {
		return FALSE;
	}

	if (event_info_rec->stored_ff_recnum < DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS) {
		(void)memcpy(&event_rec->ff_data[event_info_rec->stored_ff_recnum][0], &(ff->data[0]),
			     DEM_MAX_SIZE_FF_DATA * sizeof(ff->data[0]));
#if (DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS == 2u)
		if (event_info_rec->stored_ff_recnum == 0u) {
			(void)memcpy(&event_rec->ff_data[DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS - 1u][0],
				     &(ff->data[0]), DEM_MAX_SIZE_FF_DATA * sizeof(ff->data[0]));
		}
#endif
		ff_updated = TRUE;
	} else {
		(void)memcpy(&event_rec->ff_data[DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS - 1u][0],
			     &(ff->data[0]), DEM_MAX_SIZE_FF_DATA * sizeof(ff->data[0]));
		ff_updated = TRUE;
	}

	if (ff_updated) {
		if (event_info_rec->stored_ff_recnum < DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS) {
			event_info_rec->stored_ff_recnum++;
#if (DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS == 2u)
			event_info_rec->stored_ff_recnum = DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS;
#endif
		}
		dem_set_event_block_changed(origin, event_id);
	}

	return ff_updated;
}
#endif

#if (DEM_FF_DATA_IN_PRI_MEM)
static FUNC(boolean, DEM_CODE) store_ff(const dem_event_parameter_type *event_param, dem_ff_rec_type *ff,
					dem_ff_kind_type ff_kind, dem_primary_mem_type *event_rec)
{
	boolean ret = FALSE;
	dem_prestore_entry_type *prestored_data = NULL;

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == event_param->event_class_ref->event_destination) {
#if (DEM_ENABLE_PRESTORED_FF == STD_ON)
		prestored_data = dem_is_ff_prestored(event_param);
		if ((NULL == prestored_data) ||
			(DEM_EMISSION_REL_DTCS_KIND == event_param->event_dtc_kind)) {
			dem_get_ff_data(event_param, ff, ff_kind, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
		} else {
			dem_process_prestored_ff(prestored_data, ff, event_param, ff_kind,
							DEM_DTC_ORIGIN_PRIMARY_MEMORY);
		}
#else
		dem_get_ff_data(event_param, ff, ff_kind, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif
		if (DEM_EVENT_ID_NULL != ff->event_id) {
			if (DEM_FREEZE_FRAME_NON_OBD == ff->kind) {
				ret = store_ff_data_mem(event_param, ff, event_rec,
							DEM_DTC_ORIGIN_PRIMARY_MEMORY);
			}
		}
	}
#endif

	return ret;
}
#endif

static FUNC(void, DEM_CODE) get_storage_conditions(boolean event_failed_now, boolean event_confirmed_now,
						   boolean event_fdc_this_opcycle_now, boolean event_data_updated,
						   boolean *store_ff_data, boolean *store_ext_data)
{
#if (DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE == STD_ON)
	*store_ext_data = event_data_updated;
#elif (DEM_EXTENDED_DATA_CAPTURE_TESTFAILED == STD_ON)
	*store_ext_data = event_failed_now;
#else
	*store_ext_data = FALSE;
#endif

#if (DEM_FREEZE_FRAME_CAPTURE_ON_TEST_FAILED == STD_ON) || (DEM_FREEZE_FRAME_CAPTURE_ON_PENDING == STD_ON)
	*store_ff_data = event_failed_now;
#elif (DEM_FREEZE_FRAME_CAPTURE_ON_CONFIRMED == STD_ON)
	*store_ff_data = event_confirmed_now;
#elif (DEM_FREEZE_FRAME_CAPTURE_ON_FDC_THRESHOLD == STD_ON)
	*store_ff_data = event_fdc_this_opcycle_now;
#else
	*store_ff_data = FALSE;
	(void)event_failed_now;
	(void)event_data_updated;
#endif
}

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
static FUNC(boolean, DEM_CODE) is_stored_in_permanent_memory(const dem_event_parameter_type *event_param)
{
	boolean is_stored = FALSE;

	for (uint8 i = 0; i < DEM_MAX_NUMBER_EVENT_PERM_MEM; i++) {
		if (event_param->event_id == dem_get_permanent_event(i)) {
			is_stored = TRUE;
			break;
		}
	}

	return is_stored;
}
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(void, DEM_CODE)
	merge_event_info_records(dem_primary_mem_type *event_buffer, uint32 event_buffer_size,
					 uint32 *evt_timestamp, dem_dtc_origin_type origin)
{
	uint32 i = 0u;

	for (i = 0; i < event_buffer_size; i++) {
		if (is_storaged_entry_valid(event_buffer[i].event_id, origin) == FALSE) {
			if (DEM_EVENT_ID_NULL != event_buffer[i].event_id) {
				dem_set_event_block_changed(origin, event_buffer[i].event_id);
			}
			(void)memset(&event_buffer[i], 0, sizeof(dem_primary_mem_type));
		} else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_OFF)
			update_event_info_rec_from_storage(&event_buffer[i]);
#endif
		}
	}

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
	rearrange_event_timestamp(evt_timestamp);

	for (i = 0; i < event_buffer_size; i++) {
		if (DEM_EVENT_ID_NULL != event_buffer[i].event_id) {
			update_event_info_rec_from_storage(&event_buffer[i]);
		}
	}
#else
	(void)evt_timestamp;
#endif
}

#if defined(DEM_USE_INDICATORS)
static FUNC(void, DEM_CODE) merge_indicator_infos(const dem_indicator_type *indicator_ptr)
{
	uint16 i = 0u;

	if (NULL != indicator_ptr) {
		for (i = 0u; i < DEM_NOF_EVENT_INDICATORS; i++) {
			g_dem_indicator_mem_data[i].opcycle_status = 0;
		}
	}
}
#endif
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(void, DEM_CODE) restore_buffer(dem_dtc_origin_type origin)
{
	dem_primary_mem_type *event_buffer = NULL;
	uint32 event_buffer_size = 0u;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == origin) {
		event_buffer = g_dem_pri_mem_data;
		event_buffer_size = DEM_MAX_NUMBER_EVENT_PRI_MEM;
	}
#endif

	if (NULL == event_buffer) {
		return;
	}

	for (uint16 i = 0; i < event_buffer_size; i++) {
		if (DEM_EVENT_ID_NULL != event_buffer[i].event_id) {
			dem_set_event_block_changed(origin, event_buffer[i].event_id);
		}
		(void)memset(&event_buffer[i], 0, sizeof(dem_primary_mem_type));
	}
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
static FUNC(void, DEM_CODE) clear_permanent_mem(void)
{
	(void)memset(&g_dem_perm_mem_data, 0, sizeof(g_dem_perm_mem_data));
	g_dem_perm_mem_data.cfg_id = DEM_CURRENT_PERMANENT_CONFICURATION_ID;
	dem_nvm_set_permanent_block_changed(TRUE);
}
#endif

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(boolean, DEM_CODE) is_in_event_memory(const dem_event_parameter_type *event_param)
{
	boolean found = FALSE;
	uint16 mem_size = 0;
	const dem_primary_mem_type *mem_event_buffer = NULL;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == event_param->event_class_ref->event_destination) {
		mem_event_buffer = g_dem_pri_mem_data;
		mem_size = DEM_MAX_NUMBER_EVENT_ENTRY_PRI;
	}
#endif

	if (NULL == mem_event_buffer) {
		return FALSE;
	}

	for (uint16 i = 0; i < mem_size; i++) {
		if (event_param->event_id == mem_event_buffer[i].event_id) {
			found = TRUE;
			break;
		}
	}

	return found;
}
#endif
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(boolean, DEM_CODE)
dem_delete_event_mem(const dem_event_parameter_type *event_param, dem_primary_mem_type *event_memory,
		 uint32 event_memory_size, dem_dtc_origin_type origin)
{
	dem_event_id_type event_id = event_param->event_id;
	boolean event_id_found = FALSE;
	uint16 i = 0u;

	for (i = 0; i < event_memory_size; i++) {
		if (event_memory[i].event_id == event_id) {
			event_id_found = TRUE;
			break;
		}
	}

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
	(void)dem_clear_obd_ff(event_id);
#endif

	if (event_id_found) {
		dem_set_event_block_changed(origin, event_id);
		(void)memset(&event_memory[i], 0u, sizeof(dem_primary_mem_type));

		dem_reset_event_status_data(event_id);
	}

	return event_id_found;
}
#endif

FUNC(void, DEM_CODE) dem_set_event_status_data_blcok_changed(dem_event_id_type event_id)
{
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	const dem_event_parameter_type *event_id_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;

	lookup_event_param(event_id, &event_id_param);
	if (NULL == event_id_param) {
		return;
	}

	dtc_class = dem_get_event_dtc_class(event_id_param);
	if (NULL == dtc_class) {
		return;
	}

	if (TRUE == dtc_class->immediate_nv_storage) {
		dem_nvm_set_event_status_data_blcok_changed(TRUE);
	} else {
		dem_nvm_set_event_status_data_blcok_changed(FALSE);
	}
#else
	(void)event_id;

	dem_nvm_set_event_status_data_blcok_changed(FALSE);
#endif
}

FUNC(void, DEM_CODE)
dem_set_event_status_data(dem_event_id_type event_id, dem_event_status_rec_type event_status_rec)
{
	if (event_id > 0u) {
		g_dem_status_mem_data.event_status[event_id - 1u] = event_status_rec;
	}
	dem_set_event_status_data_blcok_changed(event_id);
}

FUNC(void, DEM_CODE) dem_set_event_trip_cnt(dem_event_id_type event_id, uint8 trip_cnt)
{
	g_dem_status_mem_data.trip_cnt[event_id - 1u] = trip_cnt;
	dem_set_event_status_data_blcok_changed(event_id);
}

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(void, DEM_CODE) dem_set_event_timestamp(dem_event_info_rec_type *event_info)
{
	if (g_dem_event_timestamp >= DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT) {
		rearrange_event_timestamp(&g_dem_event_timestamp);
	}
	event_info->timestamp = g_dem_event_timestamp;
	g_dem_event_timestamp++;
}
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(boolean, DEM_CODE)
dem_handle_permanent_dtc_erase(const dem_event_info_rec_type *event_info_rec, dem_opcycle_id_type opcycle_id)
{
	boolean mil_heal_ok = TRUE;
	boolean perm_mem_updated = FALSE;
	uint8 status = DEM_PERMANENT_DTC_UNUSED;
	uint8 index = 0u;
	const dem_event_parameter_type *event_param = NULL;

	lookup_event_param(event_info_rec->event_id, &event_param);
	if (NULL == event_param) {
		return FALSE;
	}

	index = find_permanent_entry(event_param->event_id);
	if (index != DEM_MAX_NUMBER_EVENT_PERM_MEM) {
		status = get_permanent_status(index);
	}

	// permanent清除逻辑一：关联mil灯的状态，等待三个驾驶循环的老化
	if (dem_is_emission_event_ref_to_mil(event_param) && (DEM_PERMANENT_DTC_ACTIVE == status)) {
		if (FALSE == dem_is_event_activates_mil(event_param)) {
#if defined(DEM_USE_INDICATORS)
			const dem_indicator_attribute_type *const *ind_attr_ptr =
				dem_get_event_indicator_attr_base(event_param->event_class_ref);

			if (NULL != ind_attr_ptr) {
				while ((FALSE == (*ind_attr_ptr)->dem_eol) && (FALSE != mil_heal_ok)) {
					if (DEM_MIL_INIDICATOR_ID != (*ind_attr_ptr)->indicator_ref->indicator_id) {
						ind_attr_ptr++;
						continue;
					}

					if ((opcycle_id != (*ind_attr_ptr)->indicator_healing_cycle) ||
					    (0u != (g_dem_indicator_mem_data[(*ind_attr_ptr)->indicator_buffer_index]
							    .opcycle_status &
						    INDICATOR_FAILED_DURING_HEALING_CYCLE)) ||
					    (0u == (g_dem_indicator_mem_data[(*ind_attr_ptr)->indicator_buffer_index]
							    .opcycle_status &
						    INDICATOR_PASSED_DURING_HEALING_CYCLE))) {
						mil_heal_ok = FALSE;
					}
					ind_attr_ptr++;
				}
			}
#endif
			if (TRUE == mil_heal_ok) {
				perm_mem_updated = delete_dtc_permanent_memory(event_param->event_id);
				(void)dem_reset_indicator_counters(event_param);
			}
		}
	} else if (DEM_PERMANENT_DTC_CLEARNOW == status) {
		// permanent清除逻辑二
		if ((opcycle_id == event_param->event_class_ref->opcycle_ref) &&
		    DEM_IS_MASK_UNSET(event_info_rec->event_status_rec, DEM_TEST_FAILED_THIS_OPERATION_CYCLE |
				      DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type)) {
			perm_mem_updated = delete_dtc_permanent_memory(event_param->event_id);
			(void)dem_reset_indicator_counters(event_param);
		}
	} else {
		return perm_mem_updated;
	}

	return perm_mem_updated;
}

FUNC(void, DEM_CODE) dem_merge_permanent_storage(void)
{
	boolean new_data_stored = FALSE;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

		lookup_event_param(event_id, &event_param);
		if (NULL == event_param) {
			continue;
		}

		if (TRUE == handle_dtc_store_permanent(event_param, g_dem_event_info_rec[i].event_status_rec)) {
			new_data_stored = TRUE;
		}
	}

	if (TRUE == new_data_stored) {
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
		dem_nvm_set_permanent_block_changed(TRUE);
#else
		dem_nvm_set_permanent_block_changed(FALSE);
#endif
	}
}
#endif /* DEM_USE_PERMANENT_MEMORY_SUPPORT */

FUNC(void, DEM_CODE) dem_notify_event_storage_changed(const dem_event_parameter_type *event_param)
{
#if (DEM_CALLBACK_EVENT_DATA_CHANGED_SUPPORT == STD_ON)
	if ((NULL != event_param) && (NULL != event_param->callback_event_data_changed)) {
		(void)event_param->callback_event_data_changed();
	}
#endif

#if (DEM_GENERAL_EVENT_DATA_CB == STD_ON)
	(void)rte_call_general_cb_data_evt_event_data_changed(event_param->event_id);
#endif

#if ((DEM_CALLBACK_EVENT_DATA_CHANGED_SUPPORT != STD_ON) && (DEM_GENERAL_EVENT_DATA_CB != STD_ON))
	(void)event_param;
#endif
}

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(void, DEM_CODE) dem_change_perm_status_on_failed(const dem_event_parameter_type *event_param)
{
	uint8 permanent_state;
	uint8 index = find_permanent_entry(event_param->event_id);

	if (index != DEM_MAX_NUMBER_EVENT_PERM_MEM) {
		permanent_state = get_permanent_status(index);
		if ((DEM_PERMANENT_DTC_PASSIVE == permanent_state) || (DEM_PERMANENT_DTC_CLEARNOW == permanent_state)) {
			dem_set_permanent_status(index, DEM_PERMANENT_DTC_ACTIVE);
		}
	}
}
#endif

FUNC(boolean, DEM_CODE)
dem_process_event_storage(dem_event_info_rec_type *event_info_rec,
			  const dem_event_parameter_type *event_param,
			  dem_event_status_rec_type old_status, dem_event_status_rec_type new_status,
			  boolean old_fdc_this_opcycle)
{
	std_return_type event_store_status = E_OK;
	boolean event_fdc_this_opcycle_now = (boolean)FALSE;
	boolean event_failed_now = (DEM_IS_MASK_SET(new_status, DEM_TEST_FAILED, dem_event_status_rec_type) &&
				    DEM_IS_MASK_UNSET(old_status, DEM_TEST_FAILED, dem_event_status_rec_type));
	boolean event_confirmed_now = (DEM_IS_MASK_SET(new_status, DEM_CONFIRMED_DTC, dem_event_status_rec_type) &&
				       DEM_IS_MASK_UNSET(old_status, DEM_CONFIRMED_DTC, dem_event_status_rec_type));
	boolean store_ext_data = (boolean)FALSE;
	boolean store_ff_data = (boolean)FALSE;
	boolean event_data_updated = (boolean)FALSE;
	dem_primary_mem_type *event_rec = NULL;
#if (DEM_FF_DATA_IN_PRI_MEM)
	dem_ff_rec_type ff_local;
#endif

	if ((TRUE == event_failed_now) && (FALSE == old_fdc_this_opcycle)) {
		event_info_rec->fdc_this_opcycle = (boolean)TRUE;
		event_fdc_this_opcycle_now = (boolean)TRUE;
	}
	event_store_status = dem_store_event_mem(event_param, event_info_rec, event_failed_now, &event_rec);
	event_data_updated = (E_OK == event_store_status);

	get_storage_conditions(event_failed_now, event_confirmed_now, event_fdc_this_opcycle_now, event_data_updated,
			       &store_ff_data, &store_ext_data);

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
	/*永久内存一般和obd绑定*/
	if (event_failed_now || event_confirmed_now) {
		process_permanent_storage_on_event_failed(event_param, event_info_rec);
	}
#endif

	if ((NULL != event_rec) && (FALSE == is_event_dtc_disabled_update_record(event_param))) {
		if (store_ext_data && (DEM_INVALID_U16_REF_INDEX != event_param->ext_data_class_ref_index)) {
			if (dem_store_ext_data(event_param, event_info_rec->ext_data_stored, event_rec)) {
				event_data_updated = TRUE;
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
				dem_event_id_type master_event_id = dem_get_master_event_id(event_info_rec->event_id);

				if (master_event_id != event_info_rec->event_id) {
					dem_event_info_rec_type *master_event_info_rec = NULL;

					lookup_event_info_rec(master_event_id, &master_event_info_rec);
					if (NULL != master_event_info_rec) {
						master_event_info_rec->ext_data_stored = (boolean)TRUE;
					}
				} else {
					event_info_rec->ext_data_stored = (boolean)TRUE;
				}
#else
				event_info_rec->ext_data_stored = (boolean)TRUE;
#endif
			}
		}
		if (store_ff_data) {
			if (DEM_INVALID_U16_REF_INDEX != event_param->ff_class_ref_index) {
#if (DEM_FF_DATA_IN_PRI_MEM)
				if (store_ff(event_param, &ff_local, DEM_FREEZE_FRAME_NON_OBD,
							  event_rec)) {
					event_data_updated = TRUE;
				}
#endif
			}
		}
		if ((DEM_EMISSION_REL_DTCS_KIND == event_param->event_dtc_kind) && event_data_updated &&
		    dem_check_obdff_storage_condition(old_status, new_status)) {
#if (DEM_CFG_ENABLE_OBDII == STD_ON)
			if (dem_store_obd_ff(event_param)) {
				event_data_updated = TRUE;
			}
#endif
		}
	}

	if (E_NOT_OK == event_store_status) {
		// 转储不成功的话，强制设置以保证bit3为0
		DEM_CLEAR_MASK(event_info_rec->event_status_rec, DEM_CONFIRMED_DTC, dem_event_status_rec_type);
	} else {
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
		dem_event_id_type master_event_id = dem_get_master_event_id(event_info_rec->event_id);

		if (master_event_id != event_info_rec->event_id) {
			/* 如果是事件存储组合，则将master_event_id对应的bit3置为1 */
			dem_event_info_rec_type *master_event_info_rec = NULL;

			lookup_event_info_rec(master_event_id, &master_event_info_rec);
			if (NULL != master_event_info_rec) {
				DEM_SET_MASK(master_event_info_rec->event_status_rec,DEM_CONFIRMED_DTC,
					     dem_event_status_rec_type);
			}
		}
#endif
	}

	return event_data_updated;
}

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
FUNC(void, DEM_CODE) dem_set_overflow_indication(dem_dtc_origin_type origin, boolean overflow)
{
	uint8 old_overflow = 0u;
	uint8 new_overflow = 0u;

	old_overflow = get_memory_overflow();
	if (TRUE == overflow) {
		DEM_SET_MASK(new_overflow, old_overflow | (1u << origin), uint8);
	} else {
		DEM_SET_MASK(new_overflow, old_overflow & (~(1u << origin)), uint8);
	}

	if (old_overflow != new_overflow) {
		set_memory_overflow(new_overflow);
		dem_nvm_set_mgmt_blcok_changed(FALSE);
	}
}

FUNC(std_return_type, DEM_CODE) dem_get_overflow_indication(dem_dtc_origin_type origin, boolean *overflow)
{
	uint8 memory_overflow = 0u;

	memory_overflow = get_memory_overflow();

	*overflow = (DEM_IS_MASK_SET(memory_overflow, (1u << origin), uint8) ? TRUE : FALSE);

	return E_OK;
}
#endif
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(void, DEM_CODE) dem_set_event_block_changed(dem_dtc_origin_type origin, dem_event_id_type event_id)
{
	const dem_event_parameter_type *event_param = NULL;
	dem_event_info_rec_type *event_info_rec = NULL;

	lookup_event_param(event_id, &event_param);
	lookup_event_info_rec(event_id, &event_info_rec);
	if ((dem_get_master_state() == DEM_INIT_PREINIT) || (NULL == event_param) ||
	    (NULL == event_info_rec)) {
		return;
	}

#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	boolean immediate_storage = FALSE;
	const dem_dtc_class_type *dtc_class = dem_get_event_dtc_class(event_param);

	if ((NULL != dtc_class) && (TRUE == dtc_class->immediate_nv_storage)) {
		immediate_storage = TRUE;
	}
	dem_nvm_set_event_block_changed(origin, event_id, immediate_storage);
#else
	dem_nvm_set_event_block_changed(origin, event_id, FALSE);
#endif
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */

FUNC(boolean, DEM_CODE)
dem_store_ext_data(const dem_event_parameter_type *event_param, boolean ext_data_stored,
		   dem_primary_mem_type *event_rec)
{
	boolean ret = FALSE;

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_PRI_MEM)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == event_param->event_class_ref->event_destination) {
		ret = store_ext_data_in_mem(event_param, DEM_DTC_ORIGIN_PRIMARY_MEMORY, ext_data_stored,
						event_rec);
	}
#endif

	return ret;
}

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE)
dem_delete_all_dtc_data_in_memory(dem_dtc_origin_type origin, dem_primary_mem_type *event_memory,
				  uint32 event_buffer_size)
{
	const dem_event_parameter_type *event_param = NULL;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	boolean clear_allowed = TRUE;

	for (uint16 i = 0; i < event_buffer_size; i++) {
		event_id = event_memory[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

		clear_allowed = dem_is_event_cleard_allowed(event_id);
		if (FALSE == clear_allowed) {
			continue;
		}

		dem_set_event_block_changed(origin, event_id);
		(void)memset((void *)(&event_memory[i]), 0u, sizeof(dem_primary_mem_type));

		dem_reset_event_status_data(event_id);

		lookup_event_param(event_id, &event_param);
		if (NULL != event_param) {
			dem_notify_event_storage_changed(event_param);
		}
	}

	return TRUE;
}
#endif

FUNC(std_return_type, DEM_CODE)
dem_store_event_mem(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_status,
		    boolean failed_now, dem_primary_mem_type **event_rec)
{
	std_return_type ret = E_NOT_OK;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == event_param->event_class_ref->event_destination) {
		ret = store_event(event_param, event_status, g_dem_pri_mem_data,
					DEM_MAX_NUMBER_EVENT_ENTRY_PRI, DEM_DTC_ORIGIN_PRIMARY_MEMORY, failed_now,
					event_rec);
	}
#endif

	return ret;
}

FUNC(boolean, DEM_CODE)
dem_lookup_ff_data_recnum_param(uint8 recnum, const dem_event_parameter_type *event_param,
			    dem_ff_class_type const **ff_class_ptr)
{
	boolean recnum_found = FALSE;
	const dem_ff_class_type *ff_class = NULL;
	const dem_ff_recnum_class_type *ff_recnum_class = NULL;

	if (DEM_INVALID_U16_REF_INDEX == event_param->ff_class_ref_index) {
		return recnum_found;
	}

	ff_class = dem_get_event_ff_class(event_param);

	if (DEM_FF_RECNUM_CALCULATED == g_dem_general_set->ff_recnum_type) {
		if ((recnum != 0u) && (recnum <= event_param->max_number_ff_records)) {
			recnum_found = TRUE;
			*ff_class_ptr = ff_class;
		}
	} else {
		ff_recnum_class = dem_get_event_ff_recnum_class(event_param);
		if (NULL != ff_recnum_class) {
			for (uint8 i = 0u; (i < event_param->max_number_ff_records); i++) {
				if (ff_recnum_class->ff_recnum[i] == recnum) {
					recnum_found = TRUE;
					*ff_class_ptr = ff_class;
					break;
				}
			}
		}
	}

	return recnum_found;
}

FUNC(boolean, DEM_CODE)
dem_get_stored_ff(dem_event_id_type event_id, uint8 recnum, dem_dtc_origin_type dtc_origin, dem_ff_rec_type **ff)
{
	boolean ff_found = FALSE;
#if (DEM_FF_DATA_IN_PRI_MEM)
	const dem_primary_mem_type *event_buffer = NULL;
	uint32 event_buffer_size = 0;
#endif

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == dtc_origin) {
		event_buffer = g_dem_pri_mem_data;
		event_buffer_size = DEM_MAX_NUMBER_EVENT_PRI_MEM;
	}
#endif

#if (DEM_FF_DATA_IN_PRI_MEM)
	dem_event_info_rec_type *event_info_rec = NULL;

	lookup_event_info_rec(event_id, &event_info_rec);

	if ((NULL != event_buffer) && (NULL != event_info_rec) && (recnum <= event_info_rec->stored_ff_recnum)) {
		for (uint16 i = 0; i < event_buffer_size; i++) {
			if (event_buffer[i].event_id != event_id) {
				continue;
			}
			(void)memcpy(&(*ff)->data[0], &event_buffer[i].ff_data[recnum - 1u][0],
				     DEM_MAX_SIZE_FF_DATA * sizeof((*ff)->data[0]));
			ff_found = TRUE;
		}
	}
#endif

	return ff_found;
}

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_is_storage_conditions_set(const dem_event_class_type *event_class)
{
	boolean conditions_set = TRUE;
	uint8 storage_condition_num = 0u;
	const dem_storage_condition_group_type *storage_condition_group_ptr = get_storage_condition_group(event_class);

	const dem_storage_condition_type *const *storage_condition_base =
		get_storage_condition_base(storage_condition_group_ptr);

	if ((NULL != storage_condition_group_ptr) && (NULL != storage_condition_base)) {
		storage_condition_num = storage_condition_group_ptr->nof_storage_conditions;

		for (uint8 i = 0; i < storage_condition_num; i++) {
			if (!g_dem_storage_conditions[storage_condition_base[i]->storage_condition_id]) {
				conditions_set = FALSE;
				break;
			}
		}
	}

	return conditions_set;
}
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(void, DEM_CODE) dem_transfer_event_to_permanent(uint16 nof_entries)
{
	uint32 oldest_timestamp;
	uint16 store_index = 0;
	boolean candidate_found = FALSE;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;

	for (uint16 cnt = 0; cnt < nof_entries; cnt++) {
		candidate_found = FALSE;
		oldest_timestamp = 0xFFFFFFFFu;

		for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
			event_id = g_dem_event_info_rec[i].event_id;
			if (DEM_EVENT_ID_NULL == event_id) {
				continue;
			}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
			if (FALSE == g_dem_event_info_rec[i].is_available) {
				continue;
			}
#endif
			lookup_event_param(event_id, &event_param);
			if (is_dtc_permanent_storage_condition_fulfilled(event_param,
				g_dem_event_info_rec[i].event_status_rec) &&
			    (FALSE == is_stored_in_permanent_memory(event_param)) &&
			    (g_dem_event_info_rec[i].timestamp < oldest_timestamp)) {
				store_index = i;
				oldest_timestamp = g_dem_event_info_rec[i].timestamp;
				candidate_found = TRUE;
			}
		}

		if (candidate_found) {
			lookup_event_param(g_dem_event_info_rec[store_index].event_id, &event_param);
			(void)handle_dtc_store_permanent(event_param,
							 g_dem_event_info_rec[store_index].event_status_rec);
		} else {
			break;
		}
	}
}
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(void, DEM_CODE)
dem_handle_perm_status_on_opcycle_end(dem_opcycle_id_type opcycle_id, const dem_event_info_rec_type *event_info)
{
	uint8 status = DEM_PERMANENT_DTC_UNUSED;
	uint8 index = find_permanent_entry(event_info->event_id);

	if (index != DEM_MAX_NUMBER_EVENT_PERM_MEM) {
		status = get_permanent_status(index);

		if ((DEM_WARMUP == opcycle_id) && (DEM_PERMANENT_DTC_PASSIVE == status) &&
		    DEM_IS_MASK_UNSET(event_info->event_status_rec, DEM_TEST_FAILED_THIS_OPERATION_CYCLE |
				      DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type)) {
			dem_set_permanent_status(index, DEM_PERMANENT_DTC_CLEARNOW);
		}
	}
}
#endif

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(void, DEM_CODE) dem_clear_obd_storage(const dem_event_parameter_type *event_param, boolean clear_obd_evt_mem)
{
	boolean event_id_found = FALSE;
	dem_primary_mem_type *event_memory = NULL;
	uint8 index = 0u;

	// 按照obd标准，清除未决故障码和确认故障码的逻辑
	if ((DEM_EMISSION_REL_DTCS_KIND == event_param->event_dtc_kind) && (TRUE == clear_obd_evt_mem)) {
		if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == event_param->event_class_ref->event_destination) {
			event_memory = g_dem_pri_mem_data;
			for (index = 0u; index < DEM_MAX_NUMBER_EVENT_PRI_MEM; index++) {
				if (event_memory[index].event_id == event_param->event_id) {
					event_id_found = TRUE;
					break;
				}
			}
			if (event_id_found) {
				dem_set_event_block_changed(DEM_DTC_ORIGIN_PRIMARY_MEMORY, event_param->event_id);
				(void)memset(&event_memory[index], 0u, sizeof(dem_primary_mem_type));
			}
		}
		(void)dem_clear_obd_ff(event_param->event_id);
	}
}
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
FUNC(void, DEM_CODE) dem_init_storage_conditions(void)
{
	const dem_storage_condition_type *storage_condition = g_dem_config_set->storage_condition;

	for (uint16 i = 0u; i < DEM_NUM_STORAGECONDITIONS; i++) {
		g_dem_storage_conditions[storage_condition->storage_condition_id] =
			storage_condition->storage_condition_status;
		storage_condition++;
	}
}

FUNC(uint8, DEM_CODE) dem_get_storage_condition_state(uint8 id)
{
	uint8 state = 0xFFu;

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
	if (id < DEM_NUM_STORAGECONDITIONS) {
		if (g_dem_storage_conditions[id] == TRUE) {
			state = 1;
		} else {
			state = 0;
		}
	}
#endif

	return state;
}
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(void, DEM_CODE) dem_merge_storage_to_running_mem(dem_dtc_origin_type origin)
{
	dem_primary_mem_type *event_buffer = NULL;
	uint32 event_buffer_size = 0u;
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_OFF)
	uint32 g_dem_event_timestamp = 0u;
#endif

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == origin) {
		event_buffer = g_dem_pri_mem_data;
		event_buffer_size = DEM_MAX_NUMBER_EVENT_PRI_MEM;
	}
#endif

	if (NULL == event_buffer) {
		return;
	}

	merge_event_info_records(event_buffer, event_buffer_size, &g_dem_event_timestamp, origin);
#if defined(DEM_USE_INDICATORS)
	merge_indicator_infos(g_dem_general_set->indicator);
#endif
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */

FUNC(void, DEM_CODE) dem_clear_storage_mem(void)
{
	(void)memset(&g_dem_mgmt_mem_data, 0, sizeof(dem_mgmt_mem_type));
	g_dem_mgmt_mem_data.cfg_id = DEM_CURRENT_CONFIGURATION_ID;
	dem_nvm_set_mgmt_blcok_changed(FALSE);

#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
	(void)memset(&g_dem_aged_mem_data, 0, sizeof(g_dem_aged_mem_data));
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_aged_data_blcok_changed(TRUE);
#else
	dem_nvm_set_aged_data_blcok_changed(FALSE);
#endif
#endif

	(void)memset(&g_dem_status_mem_data, 0, sizeof(dem_status_mem_type));
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_event_status_data_blcok_changed(TRUE);
#else
	dem_nvm_set_event_status_data_blcok_changed(FALSE);
#endif

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	(void)memset(&g_dem_event_available_mem_data[0], 0,
		     DEM_CFG_EVENT_AVAILABLE_DATA_LENGTH * sizeof(g_dem_event_available_mem_data[0]));
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_event_available_data_blcok_changed(TRUE);
#else
	dem_nvm_set_event_available_data_blcok_changed(FALSE);
#endif
#endif

#if (DEM_CFG_GLOBAL_DEBOUNCE_STORAGE_EVENT_COUNT > 0)
	(void)memset(&g_dem_cfg_debounce_data[0], 0,
		     DEM_CFG_GLOBAL_DEBOUNCE_STORAGE_EVENT_COUNT * sizeof(g_dem_cfg_debounce_data[0]));
#endif

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	restore_buffer(DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
	if (DEM_CURRENT_PERMANENT_CONFICURATION_ID != g_dem_perm_mem_data.cfg_id) {
		clear_permanent_mem();
	}
	(void)dem_clear_all_obd_ff();
	(void)memset(&g_dem_indicator_mem_data[0], 0, DEM_NOF_EVENT_INDICATORS * sizeof(g_dem_indicator_mem_data[0]));
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_indicator_status_block_changed(TRUE);
#else
	dem_nvm_set_indicator_status_block_changed(FALSE);
#endif
#endif
}

FUNC(dem_return_get_ext_data_record_by_dtc_type, DEM_CODE)
dem_get_ext_data_record(const dem_ext_data_record_class_type *ext_data_record_class,
					 dem_dtc_origin_type dtc_origin, uint8 *dest_buffer, uint16 *buf_size,
					 const dem_event_info_rec_type *event_rec, uint8 ext_data_number)
{
	uint8 *ext_data = NULL;
	uint8 *dest_buffer_ptr = dest_buffer;
	dem_return_get_ext_data_record_by_dtc_type ret = DEM_RECORD_WRONG_DTCORIGIN;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == dtc_origin) {
		if (lookup_ext_data_rec(event_rec->event_id, &ext_data, dtc_origin, ext_data_number) == TRUE) {
			uint16 ed_data_size = dem_get_ext_data_record_data_size(ext_data_record_class);

			(void)memcpy(dest_buffer_ptr, ext_data, ed_data_size);
			*buf_size = ed_data_size;
		} else {
			*buf_size = 0;
		}
		ret = DEM_RECORD_OK;
	}
#endif

	return ret;
}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(boolean, DEM_CODE) dem_is_event_stored_in_memory(dem_event_id_type event_id)
{
#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
	boolean is_stored = FALSE;
	const dem_event_parameter_type *event_param = NULL;

	lookup_event_param(event_id, &event_param);
	if ((NULL != event_param) && (DEM_DTC_ORIGIN_NOT_USED != event_param->event_class_ref->event_destination)) {
		is_stored = is_in_event_memory(event_param);
	}

	return is_stored;
#else
	(void)event_id;

	return FALSE;
#endif
}
#endif
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
/* ****************************************************************************
 * dem_mem_permanent_get_event
 *****************************************************************************/
FUNC(dem_event_id_type, DEM_CODE) dem_get_permanent_event(uint8 index)
{
	return g_dem_perm_mem_data.perm_data_entry[index].event_id;
}

/* ****************************************************************************
 * dem_mem_permanent_set_status
 *****************************************************************************/
FUNC(void, DEM_CODE) dem_set_permanent_status(uint8 index, uint8 status)
{
	g_dem_perm_mem_data.perm_data_entry[index].status = status;
}
#endif

#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
FUNC(void, DEM_CODE) dem_set_aged_data_blcok_changed(const dem_event_parameter_type *event_param)
{
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	const dem_dtc_class_type *dtc_class = dem_get_event_dtc_class(event_param);

	if (NULL == dtc_class) {
		return;
	}
	if (TRUE == dtc_class->immediate_nv_storage) {
		dem_nvm_set_aged_data_blcok_changed(TRUE);
	} else {
		dem_nvm_set_aged_data_blcok_changed(FALSE);
	}
#else
	(void)event_param;
	dem_nvm_set_aged_data_blcok_changed(FALSE);
#endif
}
#endif

FUNC(void, DEM_CODE) dem_init_dtc_record_disabled(void)
{
	g_dem_dtc_record_disabled.dtc = DEM_NO_DTC_DISABLED;
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"