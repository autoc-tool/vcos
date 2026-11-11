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

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
static FUNC(boolean, DEM_CODE) is_event_ref_to_mil(const dem_event_parameter_type *event_param)
{
	boolean is_ref_mil = FALSE;

#if defined(DEM_USE_INDICATORS)
	const dem_indicator_attribute_type *const *ind_attr_ptr =
		dem_get_event_indicator_attr_base(event_param->event_class_ref);

	if (NULL != ind_attr_ptr) {
		for (; FALSE == (*ind_attr_ptr)->dem_eol; ind_attr_ptr++) {
			if (DEM_MIL_INIDICATOR_ID == (*ind_attr_ptr)->indicator_ref->indicator_id) {
				is_ref_mil = TRUE;
				break;
			}
		}
	}
#else
	(void)event_param;
#endif

	return is_ref_mil;
}
#endif

#if defined(DEM_USE_INDICATORS)
static inline FUNC(uint8, DEM_CODE)
get_indicator_failure_threshold(const dem_event_parameter_type *event_param,
				const dem_indicator_attribute_type *ind_attr)
{
	if (DEM_FAILURE_CYCLE_INDICATOR == ind_attr->indicator_failure_cycle_source) {
		return ind_attr->indicator_failure_cycle_threshold;
	}

	return event_param->event_class_ref->failure_cycle_counter_threshold;
}

static inline FUNC(dem_opcycle_id_type, DEM_CODE)
get_indicator_failure_cycle(const dem_event_parameter_type *event_param,
			    const dem_indicator_attribute_type *ind_attr)
{
	if (DEM_FAILURE_CYCLE_INDICATOR == ind_attr->indicator_failure_cycle_source) {
		return ind_attr->indicator_failure_cycle;
	}

	return event_param->event_class_ref->failure_cycle_ref;
}

static inline FUNC(dem_opcycle_id_type, DEM_CODE)
get_indicator_healing_cycle(const dem_indicator_attribute_type *ind_attr)
{
	return ind_attr->indicator_healing_cycle;
}

static FUNC(boolean, DEM_CODE) is_indicator_failure_cycle_started(const dem_event_parameter_type *event_param,
								  const dem_indicator_attribute_type *ind_attr)
{
	dem_opcycle_id_type fail_cyc = get_indicator_failure_cycle(event_param, ind_attr);

	return is_opcycle_started(fail_cyc);
}

static FUNC(boolean, DEM_CODE) is_indicator_healing_cycle_started(const dem_indicator_attribute_type *ind_attr)
{
	dem_opcycle_id_type heal_cyc = get_indicator_healing_cycle(ind_attr);

	return is_opcycle_started(heal_cyc);
}

static FUNC(boolean, DEM_CODE) indicator_off_criteria_fulfilled(const dem_event_parameter_type *event_param)
{
	return (FALSE == dem_is_indicator_fulfilled(event_param));
}

static FUNC(void, DEM_CODE) handle_healing_counter_on_failed(const dem_event_parameter_type *event_param,
							     const dem_indicator_attribute_type *ind_attr_ptr)
{
	dem_indicator_info_type *ind_info = &g_dem_indicator_mem_data[ind_attr_ptr->indicator_buffer_index];

#if (DEM_HEALING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_CYCLE == STD_ON)
	if (is_indicator_failure_cycle_started(event_param, ind_attr_ptr)) {
		ind_info->healing_counter = 0;
	}
#elif defined(DEM_HEALING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_OR_HEALING_CYCLE)
	if (is_indicator_failure_cycle_started(event_param, ind_attr_ptr) ||
	    is_indicator_healing_cycle_started(ind_attr_ptr)) {
		ind_info->healing_counter = 0;
	}
#else
	(void)event_param;
	ind_info->healing_counter = 0;
#endif

#if (DEM_OBD_SUPPORT == STD_ON)
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_indicator_status_block_changed(TRUE);
#else
	dem_nvm_set_indicator_status_block_changed(FALSE);
#endif
#endif
}
#endif

#if defined(DEM_USE_INDICATORS)
FUNC_P2VAR(const dem_indicator_attribute_type *const, AUTOMATIC, DEM_CODE)
dem_get_event_indicator_attr_base(const dem_event_class_type *event_class)
{
	const dem_indicator_attribute_type *const *indicator_attribute_list = NULL;

	if ((NULL == event_class) || (DEM_INVALID_U16_REF_INDEX == event_class->indicator_attribute_index)) {
		return NULL;
	}

	indicator_attribute_list = dem_get_indicator_attribute_group_list();
	if (NULL == indicator_attribute_list) {
		return NULL;
	}

	return &indicator_attribute_list[event_class->indicator_attribute_index];
}
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(boolean, DEM_CODE) dem_is_emission_event_ref_to_mil(const dem_event_parameter_type *event_param)
{
	if ((DEM_EMISSION_REL_DTCS_KIND == event_param->event_dtc_kind) && is_event_ref_to_mil(event_param)) {
		return TRUE;
	}

	return FALSE;
}
#endif

#if defined(DEM_USE_INDICATORS)
FUNC(boolean, DEM_CODE) dem_reset_indicator_counters(const dem_event_parameter_type *event_param)
{
	boolean is_changed = FALSE;

	const dem_indicator_attribute_type *const *ind_attr_ptr =
		dem_get_event_indicator_attr_base(event_param->event_class_ref);

	if (NULL == ind_attr_ptr) {
		return FALSE;
	}

	for (; FALSE == (*ind_attr_ptr)->dem_eol; ind_attr_ptr++) {
		dem_indicator_info_type *ind_info = &g_dem_indicator_mem_data[(*ind_attr_ptr)->indicator_buffer_index];

		if ((0u != ind_info->failure_counter) || (0u != ind_info->healing_counter)) {
			is_changed = TRUE;
		}
		ind_info->failure_counter = 0;
		ind_info->healing_counter = 0;
		ind_info->opcycle_status = 0;
	}

#if (DEM_OBD_SUPPORT == STD_ON)
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_indicator_status_block_changed(TRUE);
#else
	dem_nvm_set_indicator_status_block_changed(FALSE);
#endif
#endif

	return is_changed;
}

FUNC(boolean, DEM_CODE)
dem_indicator_fail_fulfilled(const dem_event_parameter_type *event_param,
			     const dem_indicator_attribute_type *indicator_attribute)
{
	boolean fulfilled = FALSE;
	uint8 threshold = get_indicator_failure_threshold(event_param, indicator_attribute);
	dem_opcycle_id_type fail_cyc = get_indicator_failure_cycle(event_param, indicator_attribute);

	if ((fail_cyc < DEM_OPERATION_CYCLE_ID_ENDMARK) &&
	    (g_dem_indicator_mem_data[indicator_attribute->indicator_buffer_index].failure_counter >= threshold)) {
		fulfilled = TRUE;
	}

	return fulfilled;
}

FUNC(boolean, DEM_CODE) dem_is_indicator_fulfilled(const dem_event_parameter_type *event_param)
{
	boolean fulfilled = FALSE;

	const dem_indicator_attribute_type *const *ind_attr_ptr =
		dem_get_event_indicator_attr_base(event_param->event_class_ref);

	if (NULL != ind_attr_ptr) {
		for (; (FALSE == (*ind_attr_ptr)->dem_eol) && (FALSE == fulfilled); ind_attr_ptr++) {
			fulfilled = dem_indicator_fail_fulfilled(event_param, *ind_attr_ptr);
		}
	}

	return fulfilled;
}

FUNC(void, DEM_CODE) dem_indicator_opcycle_start(dem_opcycle_id_type opcycle_id)
{
	dem_opcycle_id_type fail_cyc;
	dem_opcycle_id_type heal_cyc;

	const dem_indicator_attribute_type *const *ind_attr_ptr = NULL;

	const dem_event_parameter_type *event_param_list = g_dem_config_set->event_parameter;

	for (uint16 i = 0u; i < DEM_MAX_NUMBER_EVENT; i++) {
		ind_attr_ptr = dem_get_event_indicator_attr_base(event_param_list[i].event_class_ref);
		if (NULL == ind_attr_ptr) {
			continue;
		}

		for (; FALSE == (*ind_attr_ptr)->dem_eol; ind_attr_ptr++) {
			dem_indicator_info_type *ind_info =
				&g_dem_indicator_mem_data[(*ind_attr_ptr)->indicator_buffer_index];

			fail_cyc = get_indicator_failure_cycle(&event_param_list[i], *ind_attr_ptr);
			heal_cyc = get_indicator_healing_cycle(*ind_attr_ptr);

			if (opcycle_id == fail_cyc) {
				DEM_CLEAR_MASK(ind_info->opcycle_status,INDICATOR_FAILED_DURING_FAILURE_CYCLE |
					       INDICATOR_PASSED_DURING_FAILURE_CYCLE, uint8);
			}
			if (opcycle_id == heal_cyc) {
				DEM_CLEAR_MASK(ind_info->opcycle_status, INDICATOR_FAILED_DURING_HEALING_CYCLE |
					       INDICATOR_PASSED_DURING_HEALING_CYCLE, uint8);
			}
		}
	}
}

FUNC(boolean, DEM_CODE)
dem_indicator_opcycle_end(dem_opcycle_id_type opcycle_id, dem_event_info_rec_type *event_info)
{
	dem_opcycle_id_type fail_cyc;
	dem_opcycle_id_type heal_cyc;
	boolean counter_changed = FALSE;
	uint8 healing_counter_old = 0;
	uint8 failure_counter_old = 0;
	const dem_event_parameter_type *event_param = NULL;

	const dem_indicator_attribute_type *const *ind_attr_ptr = NULL;

	lookup_event_param(event_info->event_id, &event_param);
	if (NULL != event_param) {
		ind_attr_ptr = dem_get_event_indicator_attr_base(event_param->event_class_ref);
	}

	if (NULL == ind_attr_ptr) {
		return FALSE;
	}

	for (; FALSE == (*ind_attr_ptr)->dem_eol; ind_attr_ptr++) {
		dem_indicator_info_type *ind_info = &g_dem_indicator_mem_data[(*ind_attr_ptr)->indicator_buffer_index];

		healing_counter_old = ind_info->healing_counter;
		failure_counter_old = ind_info->failure_counter;

		heal_cyc = get_indicator_healing_cycle(*ind_attr_ptr);
		if ((opcycle_id == heal_cyc) &&
		    DEM_IS_MASK_UNSET(ind_info->opcycle_status, INDICATOR_FAILED_DURING_HEALING_CYCLE, uint8) &&
		    DEM_IS_MASK_SET(ind_info->opcycle_status, INDICATOR_PASSED_DURING_HEALING_CYCLE, uint8) &&
		    dem_indicator_fail_fulfilled(event_param, *ind_attr_ptr)) {
			if (ind_info->healing_counter < DEM_INDICATOR_CNTR_MAX) {
				ind_info->healing_counter++;
			}
			if (ind_info->healing_counter >= (*ind_attr_ptr)->indicator_healing_cycle_threshold) {
				ind_info->failure_counter = 0;
			}
		}

		fail_cyc = get_indicator_failure_cycle(event_param, *ind_attr_ptr);
		if ((opcycle_id == fail_cyc) &&
		    DEM_IS_MASK_UNSET(ind_info->opcycle_status, INDICATOR_FAILED_DURING_FAILURE_CYCLE, uint8) &&
		    DEM_IS_MASK_SET(ind_info->opcycle_status, INDICATOR_PASSED_DURING_FAILURE_CYCLE, uint8) &&
		    (!dem_indicator_fail_fulfilled(event_param, *ind_attr_ptr))) {
			ind_info->failure_counter = 0;
		}
		if ((healing_counter_old != ind_info->healing_counter) ||
		    (failure_counter_old != ind_info->failure_counter)) {
			counter_changed = TRUE;
		}
	}

	if (indicator_off_criteria_fulfilled(event_param)) {
		DEM_CLEAR_MASK(event_info->event_status_rec, DEM_WARNING_INDICATOR_REQUESTED,
			       dem_event_status_rec_type);
	}
#if (DEM_OBD_SUPPORT == STD_ON)
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_indicator_status_block_changed(TRUE);
#else
	dem_nvm_set_indicator_status_block_changed(FALSE);
#endif
#endif


	return counter_changed;
}

FUNC(boolean, DEM_CODE)
dem_handle_indicators(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_info,
		  dem_event_status_type event_status)
{
	boolean info_change = FALSE;

	const dem_indicator_attribute_type *const *ind_attr_ptr =
		dem_get_event_indicator_attr_base(event_param->event_class_ref);

	if (NULL == ind_attr_ptr) {
		return FALSE;
	}

	for (; FALSE == (*ind_attr_ptr)->dem_eol; ind_attr_ptr++) {
		dem_indicator_info_type *ind_info = &g_dem_indicator_mem_data[(*ind_attr_ptr)->indicator_buffer_index];

		if (DEM_EVENT_STATUS_PASSED == event_status) {
			if (is_indicator_healing_cycle_started(*ind_attr_ptr)) {
				DEM_SET_MASK(ind_info->opcycle_status, INDICATOR_PASSED_DURING_HEALING_CYCLE, uint8);
			}
			if (is_indicator_failure_cycle_started(event_param, *ind_attr_ptr)) {
				DEM_SET_MASK(ind_info->opcycle_status, INDICATOR_PASSED_DURING_FAILURE_CYCLE, uint8);
			}
		} else if (DEM_EVENT_STATUS_FAILED == event_status) {
			if (is_indicator_healing_cycle_started(*ind_attr_ptr)) {
				DEM_SET_MASK(ind_info->opcycle_status, INDICATOR_FAILED_DURING_HEALING_CYCLE, uint8);
			}
			if (is_indicator_failure_cycle_started(event_param, *ind_attr_ptr)) {
				if (DEM_IS_MASK_UNSET(ind_info->opcycle_status, INDICATOR_FAILED_DURING_FAILURE_CYCLE,
						      uint8) &&
				    (ind_info->failure_counter < DEM_INDICATOR_CNTR_MAX)) {
					ind_info->failure_counter++;
					info_change = TRUE;
				}
				DEM_SET_MASK(ind_info->opcycle_status, INDICATOR_FAILED_DURING_FAILURE_CYCLE, uint8);
			}

			handle_healing_counter_on_failed(event_param, *ind_attr_ptr);
		} else {
			/* do nothing */
		}
	}

	if (dem_is_indicator_fulfilled(event_param)) {
		DEM_SET_MASK(event_info->event_status_rec, DEM_WARNING_INDICATOR_REQUESTED, dem_event_status_rec_type);
	}
#if (DEM_OBD_SUPPORT == STD_ON)
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_indicator_status_block_changed(TRUE);
#else
	dem_nvm_set_indicator_status_block_changed(FALSE);
#endif
#endif

	return info_change;
}
#endif

#if ((DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON) && defined(DEM_USE_INDICATORS)) ||                                       \
	(DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_is_event_activates_mil(const dem_event_parameter_type *event_param)
{
	boolean fulfilled = FALSE;

#if defined(DEM_USE_INDICATORS)
	const dem_indicator_attribute_type *const *ind_attr_ptr =
		dem_get_event_indicator_attr_base(event_param->event_class_ref);

	if (NULL == ind_attr_ptr) {
		return FALSE;
	}

	for (; FALSE == (*ind_attr_ptr)->dem_eol; ind_attr_ptr++) {
		if (DEM_MIL_INIDICATOR_ID == (*ind_attr_ptr)->indicator_ref->indicator_id) {
			fulfilled = dem_indicator_fail_fulfilled(event_param, *ind_attr_ptr);
			if (TRUE == fulfilled) {
				break;
			}
		}
	}
#else
	(void)event_param;
#endif

	return fulfilled;
}
#endif

#if defined(DEM_USE_INDICATORS)
FUNC(void, DEM_CODE) dem_init_indicator_infos(const dem_indicator_type *indicator_ptr)
{
	uint16 i = 0u;

	if (NULL != indicator_ptr) {
		for (i = 0u; i < DEM_NOF_EVENT_INDICATORS; i++) {
			g_dem_indicator_mem_data[i].internal_indicator_id = indicator_ptr[i].indicator_id;
			g_dem_indicator_mem_data[i].failure_counter = 0;
			g_dem_indicator_mem_data[i].healing_counter = 0;
			g_dem_indicator_mem_data[i].opcycle_status = 0;
		}
	}
}

FUNC(std_return_type, DEM_CODE) dem_get_event_indicator_off_fulfilled(dem_event_id_type event_id, boolean *fulfilled)
{
	std_return_type ret = E_NOT_OK;
	const dem_event_parameter_type *event_param = NULL;

	if ((dem_get_master_state() == DEM_STATE_INIT) && DEM_IS_VALID_EVENT_ID(event_id)) {
		lookup_event_param(event_id, &event_param);
		if (NULL != event_param) {
			*fulfilled = indicator_off_criteria_fulfilled(event_param);
			ret = E_OK;
		}
	}

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_get_event_indicator_on_fulfilled(dem_event_id_type event_id, boolean *fulfilled)
{
	std_return_type ret = E_NOT_OK;
	const dem_event_parameter_type *event_param = NULL;

	if ((dem_get_master_state() == DEM_STATE_INIT) && DEM_IS_VALID_EVENT_ID(event_id)) {
		lookup_event_param(event_id, &event_param);
		if (NULL != event_param) {
			*fulfilled = dem_is_indicator_fulfilled(event_param);
			ret = E_OK;
		}
	}

	return ret;
}
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"