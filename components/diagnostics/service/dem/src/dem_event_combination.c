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

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
static FUNC(uint8, DEM_CODE) calc_test_not_completed_bits(dem_event_status_rec_type dtc_status)
{
	uint8 ret = dtc_status;

	if (DEM_IS_MASK_SET(dtc_status, DEM_TEST_FAILED_SINCE_LAST_CLEAR, dem_event_status_rec_type)) {
		DEM_CLEAR_MASK(ret, DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR, dem_event_status_rec_type);
	}

	if (DEM_IS_MASK_SET(dtc_status, DEM_TEST_FAILED_THIS_OPERATION_CYCLE, dem_event_status_rec_type)) {
		DEM_CLEAR_MASK(ret, DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type);
	}

	return ret;
}
#endif

static FUNC(uint8, DEM_CODE) get_combined_event_end_index(uint8 group_index)
{
	return g_dem_event_combined_groups[group_index].end_combined_event_id_index;
}

static FUNC(uint8, DEM_CODE) get_max_number_event_combined_group(void)
{
	return DEM_MAX_NUMBER_EVENT_COMBINED_GROUP;
}
#endif

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
FUNC(uint8, DEM_CODE) dem_get_combined_event_start_index(uint8 group_index)
{
	return g_dem_event_combined_groups[group_index].start_combined_event_id_index;
}

FUNC(uint8, DEM_CODE) dem_get_event_combined_group_index(dem_event_id_type event_id)
{
	const dem_config_set_type *dem_config_set = dem_get_config_set();

	return dem_config_set->event_parameter[event_id - 1u].event_combined_groups_index;
}

FUNC(boolean, DEM_CODE) dem_is_combined_event(dem_event_id_type event_id)
{
	if (dem_get_event_combined_group_index(event_id) != DEM_EVENT_COMBINED_INVALID_GROUP_INDEX) {
		return TRUE;
	}

	return FALSE;
}

FUNC(dem_event_id_type, DEM_CODE) dem_get_master_event(dem_event_id_type event_id)
{
	dem_event_id_type master_event_id = event_id;
	uint8 group_index = dem_get_event_combined_group_index(event_id);

	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		master_event_id = g_dem_combined_event_id_table[g_dem_event_combined_groups[group_index]
									.start_combined_event_id_index];
	}

	return master_event_id;
}

FUNC(uint8, DEM_CODE) dem_get_event_combined_group_uds_status(uint8 group_index)
{
	return g_dem_event_combined_group_status[group_index];
}

FUNC(void, DEM_CODE) dem_set_event_combined_group_uds_status(uint8 group_index, uint8 uds_status)
{
	if (group_index < get_max_number_event_combined_group()) {
		g_dem_event_combined_group_status[group_index] = uds_status;
	}
}

FUNC(dem_event_id_type, DEM_CODE) dem_get_event_combined_group_event_id(uint8 group_index, uint8 combined_event_index)
{
	(void)group_index;

	return g_dem_combined_event_id_table[combined_event_index];
}

FUNC(uint8, DEM_CODE) dem_calculate_event_combine_group_uds_status(uint8 group_index)
{
	uint8 combined_uds_status = 0u;

	for (uint8 combined_event_index = dem_get_combined_event_start_index(group_index);
	     combined_event_index < get_combined_event_end_index(group_index); combined_event_index++) {
		dem_event_id_type event_id = dem_get_event_combined_group_event_id(group_index, combined_event_index);
		dem_event_info_rec_type *event_info_rec = NULL;

		dem_lookup_event_info_rec(event_id, &event_info_rec);

		if (NULL == event_info_rec) {
			continue;
		}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
		if (FALSE == event_info_rec->is_available) {
			continue;
		}
#endif
		DEM_SET_MASK(combined_uds_status, event_info_rec->event_status_rec, uint8);
	}

#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
	combined_uds_status = calc_test_not_completed_bits((dem_event_status_rec_type)combined_uds_status);
#endif

	return combined_uds_status;
}

FUNC(void, DEM_CODE) dem_handle_event_combined_group_status_changed(uint8 group_index)
{
	uint8 event_combined_group_uds_status = dem_calculate_event_combine_group_uds_status(group_index);

	dem_set_event_combined_group_uds_status(group_index, event_combined_group_uds_status);
}

FUNC(boolean, DEM_CODE) dem_event_combination_on_storage_clear_allowed(uint8 group_index)
{
	boolean clear_allowed = TRUE;

	for (uint8 combined_event_index = dem_get_combined_event_start_index(group_index);
	     combined_event_index < get_combined_event_end_index(group_index); combined_event_index++) {
		dem_event_id_type sub_event_id =
			dem_get_event_combined_group_event_id(group_index, combined_event_index);
		dem_event_info_rec_type *event_info_rec = NULL;
		const dem_event_parameter_type *sub_event_param = NULL;

		dem_lookup_event_param(sub_event_id, &sub_event_param);
		dem_lookup_event_info_rec(sub_event_id, &event_info_rec);
		if ((NULL == sub_event_param) || (NULL == event_info_rec) ||
#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
		    (FALSE == event_info_rec->is_available) ||
#endif
		    (dem_clear_event_allowed(sub_event_param) == FALSE)) {
			clear_allowed = FALSE;
			break;
		}
	}

	return clear_allowed;
}

#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
FUNC(boolean, DEM_CODE) dem_clear_combined_event(uint8 group_index, boolean reset_event_record, boolean delete_dtc_data)
{
	uint8 combined_uds_status = 0u;
	boolean data_deleted = FALSE;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_event_parameter_type *sub_event_param = NULL;

	for (uint8 combined_event_index = dem_get_combined_event_start_index(group_index);
	     combined_event_index < get_combined_event_end_index(group_index); combined_event_index++) {
		dem_event_id_type sub_event_id =
			dem_get_event_combined_group_event_id(group_index, combined_event_index);
		dem_lookup_event_param(sub_event_id, &sub_event_param);
		dem_lookup_event_info_rec(sub_event_id, &event_info_rec);

		if ((NULL == sub_event_param) || (NULL == event_info_rec)) {
			continue;
		}
#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
		if (FALSE == event_info_rec->is_available) {
			continue;
		}
#endif
		if (TRUE == delete_dtc_data) {
			if (dem_delete_dtc_data(sub_event_param) == TRUE) {
				data_deleted = TRUE;
			}
		}

		if (TRUE == reset_event_record) {
			dem_reset_event_info_rec(sub_event_param);
		} else {
			DEM_CLEAR_MASK(event_info_rec->event_status_rec,
				       DEM_CONFIRMED_DTC | DEM_PENDING_DTC, dem_event_status_rec_type);
		}
	}
	combined_uds_status = dem_calculate_event_combine_group_uds_status(group_index);
	dem_set_event_combined_group_uds_status(group_index, combined_uds_status);

	return data_deleted;
}
#endif

FUNC(sint8, DEM_CODE) dem_get_event_combined_group_fdc(dem_event_id_type event_id)
{
	sint8 max_fault_detection_counter = 0;
	uint8 group_index = dem_get_event_combined_group_index(event_id);

	for (uint8 combined_event_index = dem_get_combined_event_start_index(group_index);
	     combined_event_index < get_combined_event_end_index(group_index); combined_event_index++) {
		dem_event_id_type sub_event_id =
			dem_get_event_combined_group_event_id(group_index, combined_event_index);
		sint8 sub_event_fault_detection_counter = 0;
		dem_event_info_rec_type *event_info_rec = NULL;
		const dem_event_parameter_type *sub_event_param = NULL;

		dem_lookup_event_param(sub_event_id, &sub_event_param);
		dem_lookup_event_info_rec(sub_event_id, &event_info_rec);

		if ((NULL == sub_event_param) || (NULL == event_info_rec)) {
			continue;
		}
#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
		if (FALSE == event_info_rec->is_available) {
			continue;
		}
#endif

		(void)dem_get_fdc_internal(sub_event_id, &sub_event_fault_detection_counter);
		if (max_fault_detection_counter < sub_event_fault_detection_counter) {
			max_fault_detection_counter = sub_event_fault_detection_counter;
		}
	}

	return max_fault_detection_counter;
}

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_calculate_combined_group_dtc_suppressed_status(uint8 group_index)
{
	boolean suppressed = TRUE;

	for (uint8 combined_event_index = dem_get_combined_event_start_index(group_index);
	     combined_event_index < get_combined_event_end_index(group_index); combined_event_index++) {
		dem_event_id_type sub_event_id =
			dem_get_event_combined_group_event_id(group_index, combined_event_index);
		dem_event_info_rec_type *event_info_rec = NULL;

		dem_lookup_event_info_rec(sub_event_id, &event_info_rec);

		if (NULL == event_info_rec) {
			continue;
		}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
		if (FALSE == event_info_rec->is_available) {
			continue;
		}
#endif

		suppressed = FALSE;
		break;
	}

	return suppressed;
}
#endif

#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"
