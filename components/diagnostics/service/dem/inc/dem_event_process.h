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

#ifndef DEM_EVENT_PROCESS_H
#define DEM_EVENT_PROCESS_H

#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_DEFAULT_EVENT_STATUS (DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)
#define DEM_FAILURE_CNTR_MAX 0xFFu
#define DEM_OCCURENCE_COUNTER_MAX 0xFFFFu

#define IS_VALID_EVENT_STATUS(_x)                                                                                      \
	((DEM_EVENT_STATUS_PREPASSED == (_x)) || (DEM_EVENT_STATUS_PASSED == (_x)) ||                                  \
	 (DEM_EVENT_STATUS_PREFAILED == (_x)) || (DEM_EVENT_STATUS_FAILED == (_x)))

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

FUNC(dem_event_id_type, DEM_CODE) dem_get_master_event_id(dem_event_id_type event_id);
FUNC(boolean, DEM_CODE) dem_is_master_event(dem_event_id_type event_id);

FUNC(dem_event_status_rec_type, DEM_CODE) dem_get_event_status_data(dem_event_id_type event_id);

FUNC(uint8, DEM_CODE) dem_get_event_trip_cnt(dem_event_id_type event_id);

FUNC(void, DEM_CODE) dem_reset_event_status_data(dem_event_id_type event_id);

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_get_event_available_data(dem_event_id_type event_id);

FUNC(void, DEM_CODE) dem_set_event_available_data(dem_event_id_type event_id, boolean available_status);
#endif

FUNC(boolean, DEM_CODE)
dem_is_fulfill_confirm(const dem_event_parameter_type *event_param, const dem_event_info_rec_type *event_info);

FUNC(void, DEM_CODE)
dem_notify_event_status_change(const dem_event_parameter_type *event_param, dem_event_status_rec_type old_status,
			   dem_event_status_rec_type new_status);

FUNC(void, DEM_CODE)
dem_handle_event_status(const dem_event_id_type event_id, const dem_event_status_type event_status,
			boolean notify_event_changed);

FUNC(void, DEM_CODE) dem_reset_event_info_rec(const dem_event_parameter_type *event_param);

FUNC(boolean, DEM_CODE) dem_is_passive_event(dem_event_id_type event_id);

FUNC(uint8, DEM_CODE) dem_get_event_priority(dem_event_id_type event_id);
FUNC(boolean, DEM_CODE) dem_is_event_cleard_allowed(dem_event_id_type event_id);

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
FUNC(std_return_type, DEM_CODE)
dem_get_event_enable_condition_group_status(dem_event_id_type event_id, boolean *enable_condition_group_status);
#endif

FUNC(boolean, DEM_CODE) dem_is_event_allow_process(const dem_event_parameter_type *event_param);

FUNC(std_return_type, DEM_CODE)
dem_handle_set_event_status(dem_event_id_type event_id, dem_event_status_type event_status);

FUNC(std_return_type, DEM_CODE) dem_handle_reset_event_status(dem_event_id_type event_id);

FUNC(std_return_type, DEM_CODE) dem_reset_test_failed(const dem_event_id_type event_id);

FUNC(std_return_type, DEM_CODE)
dem_get_event_status(dem_event_id_type event_id, dem_event_status_rec_type *event_status_rec);

FUNC(void, DEM_CODE) dem_init_event_infos(const dem_event_parameter_type *event_param_list);

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
FUNC(void, DEM_CODE) dem_init_enable_conditions(void);

FUNC(uint8, DEM_CODE) dem_get_enable_condition_state(uint8 id);
#endif

FUNC(boolean, DEM_CODE) dem_clear_event_allowed(const dem_event_parameter_type *event_param);

#if (DEM_OBD_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_is_emission_related_event(const dem_event_parameter_type *event_param);
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_EVENT_PROCESS_H */
