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

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
static FUNC(uint8, DEM_CODE) get_event_available_mask(uint16 index);
static FUNC(void, DEM_CODE) set_event_available_mask(uint16 index, uint8 available_mask);
#endif

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
static FUNC_P2CONST(dem_enable_condition_group_type, AUTOMATIC, DEM_CODE)
	get_enable_condition_group(const dem_event_class_type *event_class)
{
	const dem_enable_condition_group_type *enable_condition_group_list = NULL;

	if ((NULL == event_class) || (DEM_INVALID_U16_REF_INDEX == event_class->enable_condition_group_ref_index)) {
		return NULL;
	}

	enable_condition_group_list = dem_get_enable_condition_group_list();
	if (NULL == enable_condition_group_list) {
		return NULL;
	}

	return &(enable_condition_group_list[event_class->enable_condition_group_ref_index]);
}

static FUNC_P2VAR(const dem_enable_condition_type *const, AUTOMATIC, DEM_CODE)
	get_enable_condition_base(const dem_enable_condition_group_type *enable_condition_group)
{
	const dem_enable_condition_type *const *enable_condition_group_class = NULL;

	if (NULL == enable_condition_group) {
		return NULL;
	}

	enable_condition_group_class = dem_get_enable_condition_group_class_list();
	if (NULL == enable_condition_group_class) {
		return NULL;
	}

	return &(enable_condition_group_class[enable_condition_group->enable_condition_base_index]);
}
#endif

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
static FUNC(uint8, DEM_CODE) get_event_available_mask(uint16 index)
{
	if (index < DEM_CFG_EVENT_AVAILABLE_DATA_LENGTH) {
		return g_dem_event_available_mem_data[index];
	} else {
		return (uint8)0u;
	}
}

static FUNC(void, DEM_CODE) set_event_available_mask(uint16 index, uint8 available_mask)
{
	if (index < DEM_CFG_EVENT_AVAILABLE_DATA_LENGTH) {
		g_dem_event_available_mem_data[index] = available_mask;
	}
}
#endif

static FUNC(void, DEM_CODE)
	handle_fault_confirmation(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_info_rec)
{
	dem_event_id_type master_event_id = dem_get_master_event_id(event_info_rec->event_id);

	if (dem_is_failure_cycle_started(event_param) && !event_info_rec->failed_during_failure_cycle) {
		if (event_info_rec->failure_counter < DEM_FAILURE_CNTR_MAX) {
			event_info_rec->failure_counter++;
			event_info_rec->error_status_changed = TRUE;
		}

		if (dem_is_fulfill_confirm(event_param, event_info_rec)) {
			if (master_event_id == event_info_rec->event_id) {
				DEM_SET_MASK(event_info_rec->event_status_rec, DEM_CONFIRMED_DTC,
					     dem_event_status_rec_type);
			}
			event_info_rec->error_status_changed = TRUE;
		}
		event_info_rec->failed_during_failure_cycle = TRUE;
	}
}

static FUNC(void, DEM_CODE)
	process_on_event_failed(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_info)
{
	if (DEM_IS_MASK_UNSET(event_info->event_status_rec, DEM_TEST_FAILED, dem_event_status_rec_type)) {
#if (DEM_CFG_DATA_OCCCTR == STD_ON)
		if (event_info->occctr < DEM_OCCURENCE_COUNTER_MAX) {
			event_info->occctr++;
		}
#endif
		event_info->error_status_changed = TRUE;
	}

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
	// 在诊断仪介入后，在清除永久故障码前上报了故障，则回到清除方法一来清除
	dem_change_perm_status_on_failed(event_param);
#endif

#if (DEM_SUPPORT_AGING == STD_ON)
	if (is_opcycle_started(event_param->event_class_ref->aging_cycle_ref)) {
		event_info->failed_during_aging_cycle = TRUE;
	}

	dem_handle_aging_cnt_on_failed(event_param, event_info);
#endif
	handle_fault_confirmation(event_param, event_info);

	DEM_SET_MASK(event_info->event_status_rec, DEM_TEST_FAILED | DEM_TEST_FAILED_THIS_OPERATION_CYCLE |
		     DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_PENDING_DTC, dem_event_status_rec_type);
	DEM_CLEAR_MASK(event_info->event_status_rec, DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR |
		       DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type);
}

static FUNC(void, DEM_CODE)
	process_on_event_passed(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_info)
{
	if (DEM_IS_MASK_SET(event_info->event_status_rec, DEM_TEST_FAILED, dem_event_status_rec_type)) {
		event_info->error_status_changed = TRUE;
	}

	DEM_CLEAR_MASK(event_info->event_status_rec, DEM_TEST_FAILED | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR |
		       DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type);

#if (DEM_SUPPORT_AGING == STD_ON)
	if (is_opcycle_started(event_param->event_class_ref->aging_cycle_ref)) {
		event_info->passed_during_aging_cycle = TRUE;
	}
#endif

	if (is_opcycle_started(event_param->event_class_ref->failure_cycle_ref)) {
		event_info->passed_during_failure_cycle = TRUE;
	}
}

static FUNC(void, DEM_CODE)
	dem_process_event_status(const dem_event_status_type event_status, const dem_event_parameter_type *event_param,
				 dem_event_info_rec_type *event_info)
{
	if (DEM_EVENT_STATUS_PASSED == event_status) {
		process_on_event_passed(event_param, event_info);
	} else if (DEM_EVENT_STATUS_FAILED == event_status) {
		process_on_event_failed(event_param, event_info);
	} else {
		/* do nothing */
	}

#if defined(DEM_USE_INDICATORS)
	if (dem_handle_indicators(event_param, event_info, event_status)) {
		event_info->indicator_data_changed = TRUE;
	}
#endif

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
	if (event_info->error_status_changed && (DEM_EVENT_STATUS_FAILED == event_status)) {
		dem_set_event_timestamp(event_info);
	}
#endif

	return;
}

static FUNC(void, DEM_CODE)
	notify_event_changed(boolean event_data_updated, const dem_event_parameter_type *event_param,
			     dem_event_status_rec_type old_status,
			     dem_event_status_rec_type new_status,
			     const dem_event_info_rec_type *event_info_rec)
{
	(void)event_info_rec;

	if (TRUE == event_data_updated) {
		dem_notify_event_storage_changed(event_param);
	}
	if (old_status != new_status) {
		dem_notify_event_status_change(event_param, old_status, new_status);
	}
	if (old_status != new_status) {
		dem_notify_dtc_status_changed(event_param, old_status, new_status);
	}

	return;
}

static FUNC_P2VAR(dem_event_info_rec_type, AUTOMATIC, DEM_CODE) get_event_info_rec(dem_event_id_type event_id)
{
	dem_event_info_rec_type *event_info = NULL;

	lookup_event_info_rec(event_id, &event_info);

	return event_info;
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
static FUNC(boolean, DEM_CODE) enable_conditions_set(const dem_event_class_type *event_class)
{
	boolean conditions_set = TRUE;
	uint8 enable_condition_num = 0u;
	const dem_enable_condition_group_type *enable_condition_group_ptr = get_enable_condition_group(event_class);

	const dem_enable_condition_type *const *enable_condition_base =
		get_enable_condition_base(enable_condition_group_ptr);

	if ((NULL != enable_condition_group_ptr) && (NULL != enable_condition_base)) {
		enable_condition_num = enable_condition_group_ptr->nof_enable_conditions;

		for (uint8 i = 0; i < enable_condition_num; i++) {
			if (!g_dem_enable_conditions[enable_condition_base[i]->enable_condition_id]) {
				conditions_set = FALSE;
				break;
			}
		}
	}

	return conditions_set;
}
#endif

FUNC(dem_event_id_type, DEM_CODE) dem_get_master_event_id(dem_event_id_type event_id)
{
	dem_event_id_type master_event_id = event_id;

#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
	uint8 group_index = dem_get_event_combined_group_index(event_id);

	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		uint8 combined_start_event_index = dem_get_combined_event_start_index(group_index);

		master_event_id = dem_get_event_combined_group_event_id(group_index, combined_start_event_index);
	}
#endif

	return master_event_id;
}

FUNC(boolean, DEM_CODE) dem_is_master_event(dem_event_id_type event_id)
{
	boolean ret_code = TRUE;
	dem_event_id_type master_event_id = dem_get_master_event_id(event_id);

	if (master_event_id != event_id) {
		ret_code = FALSE;
	}

	return ret_code;
}

FUNC(dem_event_status_rec_type, DEM_CODE) dem_get_event_status_data(dem_event_id_type event_id)
{
	return g_dem_status_mem_data.event_status[event_id - 1u];
}

FUNC(uint8, DEM_CODE) dem_get_event_trip_cnt(dem_event_id_type event_id)
{
	return g_dem_status_mem_data.trip_cnt[event_id - 1u];
}

FUNC(void, DEM_CODE) dem_reset_event_status_data(dem_event_id_type event_id)
{
	if (DEM_IS_VALID_EVENT_ID(event_id)) {
		dem_set_event_status_data(event_id, DEM_DEFAULT_EVENT_STATUS);
		dem_set_event_trip_cnt(event_id, 0u);
		dem_set_event_status_data_blcok_changed(event_id);
	}
}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_get_event_available_data(dem_event_id_type event_id)
{
	uint16 index = event_id / 8u;
	uint8 mask = get_event_available_mask(index);

	if (DEM_IS_MASK_SET(mask, DEM_BIT(event_id % 8u), uint8)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

FUNC(void, DEM_CODE) dem_set_event_available_data(dem_event_id_type event_id, boolean available_status)
{
	uint16 index = event_id / 8u;
	uint8 available_mask = 0u;
	uint8 mask = get_event_available_mask(index);

	if (available_status) {
		DEM_SET_MASK(available_mask, mask | DEM_BIT(event_id % 8u), uint8);
	} else {
		DEM_SET_MASK(available_mask, mask & DEM_BIT_MASK(event_id % 8u, DEM_1_BYTE), uint8);
	}

	set_event_available_mask(index, available_mask);
	dem_nvm_set_event_available_data_blcok_changed(FALSE);
}
#endif

FUNC(boolean, DEM_CODE)
dem_is_fulfill_confirm(const dem_event_parameter_type *event_param, const dem_event_info_rec_type *event_info)
{
	if ((DEM_OPERATION_CYCLE_ID_ENDMARK != event_param->event_class_ref->failure_cycle_ref) &&
	    (event_info->failure_counter >= event_param->event_class_ref->failure_cycle_counter_threshold)) {
		return TRUE;
	}

	return FALSE;
}

FUNC(void, DEM_CODE)
dem_notify_event_status_change(const dem_event_parameter_type *event_param, dem_event_status_rec_type old_status,
			       dem_event_status_rec_type new_status)
{
	if (NULL != event_param) {
#if (DEM_CALLBACK_EVENT_UDS_STATUS_CHANGED_SUPPORT == STD_ON)
		if (NULL != event_param->callback_event_status_changed) {
			(void)event_param->callback_event_status_changed(old_status, new_status);
		}
#endif
#if (DEM_GENERAL_EVENT_STATUS_CB == STD_ON) && (CONFIG_MODULE_RTE == 1)
		(void)rte_call_general_cb_status_evt_event_status_changed(event_param->event_id, old_status,
									  new_status);
#endif
	}
	(void)old_status;
	(void)new_status;
}

FUNC(void, DEM_CODE)
dem_handle_event_status(const dem_event_id_type event_id, const dem_event_status_type event_status,
			boolean need_notify)
{
	const dem_event_parameter_type *event_param = NULL;
	dem_event_info_rec_type *event_info_rec = NULL;
	boolean event_data_updated = FALSE;
	boolean error_status_changed = FALSE;
	boolean old_fdc_this_opcycle = FALSE;
	dem_event_status_rec_type old_status = 0u;
	dem_event_status_rec_type new_status = 0u;
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = 0u;
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
#endif

	lookup_event_param(event_id, &event_param);
	if (NULL == event_param) { // 找不到event对应的静态配置，结束
		return;
	}
	lookup_event_info_rec(event_id, &event_info_rec);
	if (NULL == event_info_rec) {
		// 找不到event对应的status管理表节点，结束
		return;
	}
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
	group_index = dem_get_event_combined_group_index(event_id);
	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		old_combined_status = dem_get_event_combined_group_uds_status(group_index);
	}
#endif
	old_status = event_info_rec->event_status_rec;
	old_fdc_this_opcycle = event_info_rec->fdc_this_opcycle;
	dem_process_event_status(event_status, event_param, event_info_rec);
	new_status = event_info_rec->event_status_rec;
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		if (old_status != new_status) {
			/* 如果event状态发生改变，计算事件组的状态 */
			new_combined_status = dem_calculate_event_combine_group_uds_status(group_index);
			/* 组合事件根据组合dtc的状态转换作为事件存储的触发源 */

			if (DEM_IS_MASK_SET(old_combined_status, DEM_TEST_FAILED, dem_event_status_rec_type) !=
			    DEM_IS_MASK_SET(new_combined_status, DEM_TEST_FAILED, dem_event_status_rec_type)) {
				error_status_changed = TRUE;
				/* 组合dtc由dtc_group状态来触发扩展数据和冻结帧的存储 */
				old_status = old_combined_status;
				new_status = new_combined_status;
			}
		}
	} else {
#endif
		if (TRUE == event_info_rec->error_status_changed) {
			error_status_changed = TRUE;
		}
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
	}
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
	if (((TRUE == error_status_changed)) && dem_is_storage_conditions_set(event_param->event_class_ref)) {
#else
	if (TRUE == error_status_changed) {
#endif
		event_data_updated = dem_process_event_storage(event_info_rec, event_param, old_status,
									       new_status, old_fdc_this_opcycle);
	}

#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
	if ((DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) && (old_status != new_status)) {
		/* 如果event状态发生改变，更新dtc组的状态 */
		uint8 event_combined_group_uds_status = dem_calculate_event_combine_group_uds_status(group_index);

		dem_set_event_combined_group_uds_status(group_index, event_combined_group_uds_status);
	}
#endif

	if (TRUE == need_notify) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
		if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
			notify_event_changed(event_data_updated, event_param, old_combined_status,
								new_combined_status, event_info_rec);
		} else {
			notify_event_changed(event_data_updated, event_param, old_status,
								event_info_rec->event_status_rec, event_info_rec);
		}
#else
		{
			notify_event_changed(event_data_updated, event_param, old_status,
								event_info_rec->event_status_rec, event_info_rec);
		}
#endif
	}

	return;
}

FUNC(void, DEM_CODE) dem_reset_event_info_rec(const dem_event_parameter_type *event_param)
{
	dem_event_info_rec_type *event_info = NULL;

	lookup_event_info_rec(event_param->event_id, &event_info);

	if (NULL != event_info) {
		dem_reset_debounce_counter(event_info);
		event_info->event_status_rec = DEM_DEFAULT_EVENT_STATUS;
		event_info->error_status_changed = (boolean)FALSE;
#if (DEM_CFG_DATA_OCCCTR == STD_ON)
		event_info->occctr = 0u;
#endif
		event_info->failure_counter = 0u;
		event_info->failed_during_failure_cycle = (boolean)FALSE;
		event_info->passed_during_failure_cycle = (boolean)FALSE;
#if (DEM_SUPPORT_AGING == STD_ON)
		event_info->aging_cnt = 0u;
#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
		event_info->aged_cnt = 0u;
#endif
		event_info->failed_during_aging_cycle = (boolean)FALSE;
		event_info->passed_during_aging_cycle = (boolean)FALSE;
#endif
		event_info->timestamp = 0u;
#if (DEM_CFG_DATA_CYCLES_SINCE_LAST_FAILED == STD_ON)
		event_info->cycles_since_last_failed = (uint8)0u;
		event_info->cycles_since_last_failed_start = (boolean)FALSE;
#endif
#if (DEM_CFG_DATA_CYCLES_SINCE_FIRST_FAILED == STD_ON)
		event_info->cycles_since_first_failed = (uint8)0u;
		event_info->cycles_since_first_failed_start = (boolean)FALSE;
#endif
#if (DEM_CFG_DATA_FAILED_CYCLES == STD_ON)
		event_info->failed_cycles = (uint8)0u;
		event_info->failed_cycles_start = (boolean)FALSE;
#endif
		event_info->fdc_this_opcycle = (boolean)FALSE;
		event_info->ext_data_stored = (boolean)FALSE;
#if (DEM_FF_DATA_IN_PRI_MEM)
		event_info->stored_ff_recnum = 0u;
#endif
	}
}

FUNC(boolean, DEM_CODE) dem_is_passive_event(dem_event_id_type event_id)
{
	boolean event_passive = FALSE;
	dem_event_status_rec_type event_status;

	if (E_OK == dem_get_event_status(event_id, &event_status)) {
		event_passive = DEM_IS_MASK_UNSET(event_status,
						  DEM_TEST_FAILED | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE,
						  dem_event_status_rec_type);
	}

	return event_passive;
}

FUNC(uint8, DEM_CODE) dem_get_event_priority(dem_event_id_type event_id)
{
	uint8 priority = 0xFFu;
	const dem_event_parameter_type *event_param = NULL;

	lookup_event_param(event_id, &event_param);
	if (NULL != event_param) {
		priority = event_param->event_class_ref->event_priority;
	}

	return priority;
}

FUNC(boolean, DEM_CODE) dem_is_event_cleard_allowed(dem_event_id_type event_id)
{
	boolean clear_allowed = TRUE;
	const dem_event_parameter_type *event_param = NULL;

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	if (dem_is_combined_event(event_id) == TRUE) {
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
		uint8 group_index = dem_get_event_combined_group_index(event_id);

		clear_allowed = dem_event_combination_on_storage_clear_allowed(group_index);
#endif
	} else {
		lookup_event_param(event_id, &event_param);
		clear_allowed = dem_clear_event_allowed(event_param);
	}
#else
	{
		lookup_event_param(event_id, &event_param);
		clear_allowed = dem_clear_event_allowed(event_param);
	}
#endif

	return clear_allowed;
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
FUNC(std_return_type, DEM_CODE)
dem_get_event_enable_condition_group_status(dem_event_id_type event_id, boolean *enable_condition_group_status)
{
	std_return_type ret = E_OK;
	boolean status = FALSE;
	const dem_event_parameter_type *event_param = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_GETEVENTSTATUS_ID_DEM_E_UNINIT,
			E_NOT_OK);

	if (NULL == enable_condition_group_status) {
		return E_NOT_OK;
	}

	lookup_event_param(event_id, &event_param);
	if ((NULL != event_param) && (NULL != event_param->event_class_ref)) {
		status = enable_conditions_set(event_param->event_class_ref);
	} else {
		ret = E_NOT_OK;
	}
	*enable_condition_group_status = status;

	return ret;
}
#endif

FUNC(boolean, DEM_CODE) dem_is_event_allow_process(const dem_event_parameter_type *event_param)
{
	if (TRUE == dem_is_dtc_setting_disabled(event_param)) {
		return FALSE;
	}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
	if (FALSE == enable_conditions_set(event_param->event_class_ref)) {
		return FALSE;
	}
#endif

	return TRUE;
}

FUNC(std_return_type, DEM_CODE)
dem_handle_set_event_status(dem_event_id_type event_id, dem_event_status_type event_status)
{
	const dem_event_parameter_type *event_param = NULL;
	dem_event_info_rec_type *event_info_rec = NULL;
	dem_satellite_id_type satellite_id;
	dem_event_status_type evt_sta_after_deb;

	lookup_event_param(event_id, &event_param);
	if (NULL == event_param) {
		return E_NOT_OK;
	}
	lookup_event_info_rec(event_id, &event_info_rec);
	if (NULL == event_info_rec) {
		return E_NOT_OK;
	}
#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if (FALSE == event_info_rec->is_available) {
		return E_NOT_OK;
	}
#endif
	if (is_opcycle_started(event_param->event_class_ref->opcycle_ref) == FALSE) {
		return E_NOT_OK;
	}
	if (dem_is_event_allow_process(event_param) == FALSE) {
		return E_NOT_OK;
	}

	satellite_id = dem_get_event_satellite_id(event_id);
	evt_sta_after_deb = dem_process_debounce(event_param, event_status, event_info_rec);

	if (DEM_EVENT_STATUS_INVALID == evt_sta_after_deb) {
		return E_NOT_OK;
	}

	if ((DEM_EVENT_STATUS_PREFAILED == evt_sta_after_deb) || (DEM_EVENT_STATUS_PREPASSED == evt_sta_after_deb)) {
		/* 去抖尚未成熟，等待后续上报继续去抖 */
		return E_OK;
	}

	/* 去抖后状态成熟，加入事件队列等待master处理 */
	(void)dem_push_event_info_to_queue(satellite_id, event_id, evt_sta_after_deb);

	return E_OK;
}

FUNC(std_return_type, DEM_CODE) dem_handle_reset_event_status(dem_event_id_type event_id)
{
	std_return_type ret_val = E_NOT_OK;
	const dem_event_parameter_type *event_param;
	dem_event_info_rec_type *event_info_rec = NULL;
	dem_satellite_id_type satellite_id;

	lookup_event_param(event_id, &event_param);
	if (NULL == event_param) {
		return E_NOT_OK;
	}

	lookup_event_info_rec(event_id, &event_info_rec);
	if (NULL == event_info_rec) {
		return E_NOT_OK;
	}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if (FALSE == event_info_rec->is_available) {
		return E_NOT_OK;
	}
#endif
	if (DEM_IS_MASK_UNSET(event_info_rec->event_status_rec, DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE,
			      dem_event_status_rec_type)) {
		return E_NOT_OK;
	}

	satellite_id = dem_get_event_satellite_id(event_id);
	ret_val = dem_push_event_info_to_queue(satellite_id, event_id, DEM_EVENTQUEUE_RESET_TESTFAILED_FLAG);
	if (E_OK != ret_val) {
		return E_NOT_OK;
	}

	return E_OK;
}

FUNC(std_return_type, DEM_CODE) dem_reset_test_failed(const dem_event_id_type event_id)
{
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = old_combined_status;
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
#endif
	dem_event_status_rec_type old_status = 0u;
	dem_event_info_rec_type *event_info = NULL;
	const dem_event_parameter_type *event_param = NULL;

	lookup_event_info_rec(event_id, &event_info);
	if (NULL == event_info) {
		return E_NOT_OK;
	}
	lookup_event_param(event_id, &event_param);
	if (NULL == event_param) {
		return E_NOT_OK;
	}

	old_status = event_info->event_status_rec;
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	group_index = dem_get_event_combined_group_index(event_info->event_id);
	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		old_combined_status = dem_get_event_combined_group_uds_status(group_index);
	}
#endif
	DEM_CLEAR_MASK(event_info->event_status_rec, DEM_TEST_FAILED, dem_event_status_rec_type);

	dem_reset_debounce_counter(event_info);

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

	return E_OK;
}

FUNC(std_return_type, DEM_CODE)
dem_get_event_status(dem_event_id_type event_id, dem_event_status_rec_type *event_status_rec)
{
	std_return_type ret = E_OK;
	dem_event_info_rec_type *event_info = NULL;

	event_info = get_event_info_rec(event_id);

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if ((event_info != NULL) && event_info->is_available) {
#else
	if (event_info != NULL) {
#endif
		*event_status_rec = event_info->event_status_rec;
	} else {
		*event_status_rec =
			DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
		ret = E_NOT_OK;
	}

	return ret;
}

FUNC(void, DEM_CODE) dem_init_event_infos(const dem_event_parameter_type *event_param_list)
{
	const dem_event_parameter_type *event_param = NULL;
	dem_event_info_rec_type *event_info = NULL;

	for (uint16 i = 0u; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_param = &event_param_list[i];
		lookup_event_info_rec(event_param->event_id, &event_info);
		if (NULL != event_info) {
			event_info->event_id = event_param->event_id;
#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON) || (DEM_SUPPORT_CURRENT_FDC == STD_ON)
			sint8 start_uds_fdc = dem_get_default_uds_fdc(event_param->event_id);
#if (DEM_SUPPORT_CURRENT_FDC == STD_ON)
			event_info->uds_fdc = start_uds_fdc;
#endif
#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON)
			event_info->max_uds_fdc = start_uds_fdc;
#endif
#endif
#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
			event_info->is_available = event_param->event_available;

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
			const dem_dtc_class_type *dtc_class = NULL;
			boolean suppressed = (!event_param->event_available);
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			uint8 group_index = dem_get_event_combined_group_index(event_param->event_id);

			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				suppressed = dem_calculate_combined_group_dtc_suppressed_status(group_index);
			}
#endif
			dtc_class = dem_get_event_dtc_class(event_param);
			if ((NULL != dtc_class) && (dtc_class->dtc_index < DEM_NOF_DTCS)) {
				g_dem_dtc_suppressed[dtc_class->dtc_index].suppressed_by_event = suppressed;
			}
#endif
#endif
		}
	}
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
FUNC(void, DEM_CODE) dem_init_enable_conditions(void)
{
	const dem_enable_condition_type *enable_condition = g_dem_config_set->enable_condition;

	for (uint16 i = 0u; i < DEM_NUM_ENABLECONDITIONS; i++) {
		g_dem_enable_conditions[enable_condition->enable_condition_id] =
			enable_condition->enable_condition_status;
		enable_condition++;
	}
}

FUNC(uint8, DEM_CODE) dem_get_enable_condition_state(uint8 id)
{
	uint8 state = 0xFFu;

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
	if (id < DEM_NUM_ENABLECONDITIONS) {
		if (g_dem_enable_conditions[id] == TRUE) {
			state = 1;
		} else {
			state = 0;
		}
	}
#endif

	return state;
}
#endif

FUNC(boolean, DEM_CODE) dem_clear_event_allowed(const dem_event_parameter_type *event_param)
{
	boolean clear_allowed = TRUE;

#if (DEM_CALLBACK_CLEAR_EVENT_ALLOWED_SUPPORT == STD_ON)
	if (NULL != event_param->callback_clear_event_allowed) {
		if (E_OK != event_param->callback_clear_event_allowed(clear_allowed)) {
			clear_allowed = FALSE;
		}
	}
#endif

	return clear_allowed;
}

#if (DEM_OBD_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_is_emission_related_event(const dem_event_parameter_type *event_param)
{
	if (DEM_EMISSION_REL_DTCS_KIND == event_param->event_dtc_kind) {
		return TRUE;
	}

	return FALSE;
}
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"