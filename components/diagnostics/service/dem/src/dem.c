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

#include "SchM_Dem.h"
#include "dem_internal.h"
#include "Compiler.h"

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

static FUNC(void, DEM_CODE) dem_master_pre_init_internal(const dem_config_type *config_ptr)
{
	DEM_VALIDATE_NO_RV(NULL != config_ptr->config_set, DEM_DEM_MASTERPREINIT_ID_DEM_E_CONFIG_PTR_INVALID);
	DEM_VALIDATE_NO_RV(NULL != config_ptr->cfg_helper, DEM_DEM_MASTERPREINIT_ID_DEM_E_CONFIG_PTR_INVALID);
	DEM_VALIDATE_NO_RV(NULL != config_ptr->general, DEM_DEM_MASTERPREINIT_ID_DEM_E_CONFIG_PTR_INVALID);

	g_dem_config_set = config_ptr->config_set;
	g_dem_cfg_helper = config_ptr->cfg_helper;
	g_dem_general_set = config_ptr->general;

	dem_init_event_infos(g_dem_config_set->event_parameter);

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
	dem_init_enable_conditions();
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
	dem_init_storage_conditions();
#endif

#if defined(DEM_USE_INDICATORS)
	dem_init_indicator_infos(g_dem_general_set->indicator);
#endif

	dem_init_opcycle_state_list();
	dem_init_auto_start_opcycle();

	dem_init_dtc_record_disabled();

#if (CONFIG_MODULE_NVM == 1) && (DEM_USE_NVM == STD_ON)
	dem_nvm_init();
#endif
}

static FUNC(void, DEM_CODE) dem_master_init_internal(void)
{
	const dem_event_parameter_type *event_param = NULL;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		g_dem_event_info_rec[i].error_status_changed = FALSE;
	}
	if (dem_get_master_state() != DEM_INIT_PREINIT) {
		(void)dem_handle_set_opcycle_state(DEM_ACTIVE, DEM_CYCLE_STATE_START);

	} else if (g_dem_mgmt_mem_data.cfg_id != (uint16)DEM_CURRENT_CONFIGURATION_ID) {
		/* dem配置发生改变，需要清除nvram中的数据 */
		dem_clear_storage_mem();
	} else {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
		dem_merge_storage_to_running_mem(DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
		dem_merge_permanent_storage();
#endif
	}

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		if (TRUE == g_dem_event_info_rec[i].error_status_changed) {
			lookup_event_param(g_dem_event_info_rec[i].event_id, &event_param);
			dem_notify_event_storage_changed(event_param);
			g_dem_event_info_rec[i].error_status_changed = FALSE;
		}
	}

	dem_init_filter();
	g_dem_disable_dtc_set.setting_disabled = FALSE;
	dem_init_ff_record_filter();

	dem_set_master_state(DEM_STATE_INIT);
}

FUNC(void, DEM_CODE) dem_master_pre_init(const dem_config_type *config_ptr)
{
	DEM_VALIDATE_NO_RV(NULL != config_ptr, DEM_DEM_MASTERPREINIT_ID_DEM_E_CONFIG_PTR_INVALID);
	if (dem_is_master_in_state(DEM_STATE_UNINIT) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERPREINIT_ID_DEM_E_WRONG_CONDITION);
	}

#if (DEM_USE_CONFIGPTR == STD_ON)
	/* 如果配置指针状态不是dem_configptr_uninitialized，说明已经做过预初始化 */
	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_UNINITIALIZED) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERPREINIT_ID_DEM_E_WRONG_CONDITION);
	}

	dem_set_global_config_ptr(config_ptr);
	dem_set_global_config_ptr_state(DEM_CONFIGPTR_PREINITIALIZED);
#endif

	for (dem_satellite_id_type satellite_id = 0u; satellite_id < DEM_MAX_NUMBER_SATELLITE; satellite_id++) {
		dem_satellite_memory_init(satellite_id);
	}

	dem_master_pre_init_internal(config_ptr);
	dem_set_master_state(DEM_INIT_PREINIT);
	(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_INFO, "Dem Master successfully preInit \r\n");
}

FUNC(void, DEM_CODE) dem_satellite_pre_init(dem_satellite_id_type satellite_id)
{
	if (satellite_id >= DEM_SATELLITEINFO_SIZE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITESATPREINIT_ID_DEM_E_PARAM_DATA);
	}
#if (DEM_USE_CONFIGPTR == STD_ON)
	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_PREINITIALIZED) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEPREINIT_ID_DEM_E_WRONG_CONDITION);
	}
#endif
	if (dem_is_master_in_state(DEM_INIT_PREINIT) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEPREINIT_ID_DEM_E_WRONG_CONDITION);
	}
	if (dem_is_satellite_in_state(satellite_id, DEM_STATE_UNINIT) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEPREINIT_ID_DEM_E_WRONG_CONDITION);
	}

	dem_satellite_pre_init_internal(satellite_id);
	(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_INFO, "Dem Satellite %u successfully preInit \r\n", satellite_id);
}

FUNC(void, DEM_CODE) dem_pre_init(const dem_config_type *config_ptr)
{
	/* 多核情况下直接单独调用dem_master_pre_init和dem_satellite_pre_init做初始化 */
	DEM_VALIDATE_NO_RV(dem_is_single_satellite() == TRUE, DEM_DEM_PREINIT_ID_DEM_E_WRONG_CONDITION);

	/* 单satellite情况下才会调用dem_pre_init()将master和core0下的satellite初始化 */
	dem_master_pre_init(config_ptr);
	dem_satellite_pre_init(DEM_DEFAULT_SINGLE_SATELLITE_ID);
}

FUNC(void, DEM_CODE) dem_master_init(const dem_config_type *config_ptr)
{
	if ((dem_is_master_in_state(DEM_INIT_PREINIT) == FALSE) &&
	    (dem_is_master_in_state(DEM_STATE_SHUTDOWN) == FALSE)) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERINIT_ID_DEM_E_WRONG_CONDITION);
	}
	if (dem_is_all_satellites_at_least_in_state(DEM_INIT_PREINIT) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERINIT_ID_DEM_E_WRONG_CONDITION);
	}

#if (DEM_USE_CONFIGPTR == STD_ON)
	if ((dem_is_master_in_state(DEM_STATE_SHUTDOWN) == TRUE) && (NULL != config_ptr) &&
	    (config_ptr != dem_get_global_config_ptr())) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERINIT_ID_DEM_E_WRONG_CONDITION);
	}

	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_UNINITIALIZED) == TRUE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERINIT_ID_DEM_E_WRONG_CONDITION);
	}

	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_PREINITIALIZED) != TRUE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERINIT_ID_DEM_E_WRONG_CONDITION);
	}

	if (NULL == config_ptr) {
		dem_set_global_config_ptr_state(DEM_CONFIGPTR_INITIALIZED);
	} else {
		dem_set_global_config_ptr(config_ptr);
		dem_set_global_config_ptr_state(DEM_CONFIGPTR_INITIALIZED);
	}

	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_INITIALIZED) == TRUE)
#endif
	{
		dem_master_init_internal();
	}
	(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_INFO, "Dem Master successfully init \r\n");
}

FUNC(void, DEM_CODE) dem_satellite_init(dem_satellite_id_type satellite_id)
{
#if (DEM_USE_CONFIGPTR == STD_ON)
	if (dem_current_partition_is_master() == TRUE) {
		if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_INITIALIZED) == FALSE) {
			DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEINIT_ID_DEM_E_WRONG_CONDITION);
		}
	}
#endif
	if (dem_current_partition_is_master() == TRUE) {
		if (dem_is_master_in_state(DEM_STATE_INIT) == FALSE) {
			DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEINIT_ID_DEM_E_WRONG_CONDITION);
		}
	}
	if (dem_is_satellite_in_state(satellite_id, DEM_INIT_PREINIT) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEINIT_ID_DEM_E_WRONG_CONDITION);
	}

	dem_set_satellite_state(satellite_id, DEM_STATE_INIT);
	(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_INFO, "Dem Satellite %u successfully init \r\n", satellite_id);
}

FUNC(void, DEM_CODE) dem_init(const dem_config_type *config_ptr)
{
	/* 单satellite情况下才会调用dem_init()将master和core0下的satellite初始化 */
	DEM_VALIDATE_NO_RV(dem_is_single_satellite() == TRUE, DEM_DEM_INIT_ID_DEM_E_WRONG_CONDITION);

	boolean is_master_shutdown = dem_is_master_in_state(DEM_STATE_SHUTDOWN);

	dem_master_init(config_ptr);
	if (FALSE == is_master_shutdown) {
		dem_satellite_init(DEM_DEFAULT_SINGLE_SATELLITE_ID);
	}
}

FUNC(void, DEM_CODE) dem_shutdown(void)
{
	DEM_VALIDATE_NO_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_SHUTDOWN_ID_DEM_E_UNINIT);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	/* dem_shut_down时将所有处于start状态的操作周期关闭并处理对应的计数器等信息 */
	for (uint8 opcycle_indx = 0u; opcycle_indx < DEM_OPERATION_CYCLE_ID_ENDMARK; opcycle_indx++) {
		if (g_dem_opcycle_state_list[opcycle_indx] == DEM_CYCLE_STATE_START) {
			(void)dem_handle_set_opcycle_state(opcycle_indx, DEM_CYCLE_STATE_END);
		}
	}

	dem_set_master_state(DEM_STATE_SHUTDOWN);

#if (DEM_USE_CONFIGPTR == STD_ON)
	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_INITIALIZED) == TRUE) {
		dem_set_global_config_ptr_state(DEM_CONFIGPTR_PREINITIALIZED);
	}
#endif

#if (CONFIG_MODULE_NVM == 1) && (DEM_USE_NVM == STD_ON)
	dem_nvm_shutdown();
#endif

	DEM_EXIT_EXCLUSIVE_AREA_0();

	(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_INFO, "Dem shutdown \r\n");
}

FUNC(void, DEM_CODE) dem_init_memory(void)
{
	for (dem_satellite_id_type satellite_id = 0u; satellite_id < DEM_MAX_NUMBER_SATELLITE; ++satellite_id) {
		dem_satellite_memory_init(satellite_id);
	}

	dem_set_master_state(DEM_STATE_UNINIT);
	dem_set_global_config_ptr_state(DEM_CONFIGPTR_UNINITIALIZED);
}

FUNC(void, DEM_CODE) Dem_MasterMainFunction(void)
{
	/* 非完全初始化状态不做处理 */
	if (dem_is_master_in_state(DEM_STATE_INIT) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_MASTERMAINFUNCTION_ID_DEM_E_UNINIT);
	}

	dem_precess_event_queue_main_function();

	dem_precess_opcycle_main_function();

#if (CONFIG_MODULE_NVM == 1) && (DEM_USE_NVM == STD_ON)
	dem_nvm_main_function();
#endif /* DEM_USE_MEMORY_FUNCTIONS */
}

FUNC(void, DEM_CODE) Dem_SatelliteMainFunction(void)
{
	dem_satellite_id_type satellite_id = dem_get_current_satellite_id();

#if (DEM_USE_CONFIGPTR == STD_ON)
	if (dem_is_global_cfg_ptr_at_least_in_state(DEM_CONFIGPTR_PREINITIALIZED) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEMAINFUNCTION_ID_DEM_E_UNINIT);
	}
#endif

	if (dem_is_satellite_at_least_in_state(satellite_id, DEM_INIT_PREINIT) == FALSE) {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITEMAINFUNCTION_ID_DEM_E_UNINIT);
	}

#if (DEM_USE_CONFIGPTR == STD_ON)
	if ((dem_is_satellite_at_least_in_state(satellite_id, DEM_STATE_INIT) == TRUE) &&
	    (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_INITIALIZED) == TRUE)) {
#else
	if (dem_is_satellite_at_least_in_state(satellite_id, DEM_STATE_INIT) == TRUE) {
#endif
#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
		dem_time_based_debounce_main_function(satellite_id);
#endif
#endif
	}
}

/* 单核情况下调用Dem_MainFunction */
FUNC(void, DEM_CODE) Dem_MainFunction(void)
{
	Dem_SatelliteMainFunction();
	Dem_MasterMainFunction();
}

FUNC(std_return_type, DEM_CODE)
dem_get_indicator_status(uint8 indicator_id, dem_indicator_status_type *indicator_status)
{
	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_GETINDICATORSTATUS_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV(NULL != indicator_status, DEM_DEM_GETINDICATORSTATUS_ID_DEM_E_PARAM_POINTER, E_NOT_OK);

#if defined(DEM_USE_INDICATORS)
	std_return_type ret = E_NOT_OK;
	uint8 curr_prio = 0xff;
	const dem_indicator_type *ind_config = NULL;
	const dem_event_parameter_type *event_param = NULL;

	const dem_indicator_attribute_type *const *ind_attr_ptr = NULL;

	DEM_VALIDATE_RV(DEM_NOF_INDICATORS > indicator_id, DEM_DEM_GETINDICATORSTATUS_ID_DEM_E_PARAM_CONFIG, E_NOT_OK);

	ind_config = &g_dem_general_set->indicator[indicator_id];
	*indicator_status = DEM_INDICATOR_OFF;

	DEM_ENTER_EXCLUSIVE_AREA_0();

	for (uint8 indx = 0; indx < ind_config->event_list_size; indx++) {
		event_param = NULL;
		ind_attr_ptr = NULL;
		lookup_event_param(*(ind_config->event_list[indx]), &event_param);
		if (NULL != event_param) {
			ind_attr_ptr = dem_get_event_indicator_attr_base(event_param->event_class_ref);
		}

		if (NULL == ind_attr_ptr) {
			continue;
		}

		while (!(*ind_attr_ptr)->dem_eol) {
			if ((*ind_attr_ptr)->indicator_ref->indicator_id != indicator_id) {
				ind_attr_ptr++;
				continue;
			}

			ret = E_OK;
			if (dem_indicator_fail_fulfilled(event_param, *ind_attr_ptr)) {
				if (event_param->event_class_ref->event_priority < curr_prio) {
					*indicator_status = (*ind_attr_ptr)->indicator_behaviour;
					curr_prio = event_param->event_class_ref->event_priority;
				}
			}

			ind_attr_ptr++;
		}
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
#else
	(void)indicator_id;
	DEM_DET_REPORT(DEM_DEM_GETINDICATORSTATUS_ID_DEM_E_PARAM_CONFIG);

	return E_NOT_OK;
#endif
}

FUNC(std_return_type, DEM_CODE) dem_set_event_status(dem_event_id_type event_id, dem_event_status_type event_status)
{
	DEM_VALIDATE_RV(IS_VALID_EVENT_STATUS(event_status), DEM_DEM_SETEVENTSTATUS_ID_DEM_E_PARAM_DATA, E_NOT_OK);

#if (DEM_USE_CONFIGPTR == STD_ON)
	if (dem_is_global_cfg_ptr_at_least_in_state(DEM_CONFIGPTR_PREINITIALIZED) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_SETEVENTSTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}
#endif

	if (dem_is_master_at_least_in_state(DEM_INIT_PREINIT) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_SETEVENTSTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	if ((dem_is_event_satellite_in_state(event_id, DEM_INIT_PREINIT) == FALSE) &&
	    (dem_is_event_satellite_in_state(event_id, DEM_STATE_INIT) == FALSE)) {
		DEM_REPORT_ERROR_RV(DEM_DEM_SETEVENTSTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	/* 检查event_id对应的application_id是否为当前application_id */
	if (dem_check_event_application_id(event_id) == FALSE) {
		return E_NOT_OK;
	}

	if (dem_is_master_in_state(DEM_STATE_SHUTDOWN) == TRUE) {
		return E_NOT_OK;
	}

	return dem_handle_set_event_status(event_id, event_status);
}

FUNC(std_return_type, DEM_CODE) dem_reset_event_status(dem_event_id_type event_id)
{
	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_RESETEVENTSTATUS_ID_DEM_E_UNINIT,
			E_NOT_OK);

#if (DEM_USE_CONFIGPTR == STD_ON)
	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_INITIALIZED) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETEVENTSTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}
#endif
	if (dem_is_satellite_in_state(dem_get_event_satellite_id(event_id), DEM_STATE_INIT) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETEVENTSTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}
	if (dem_check_event_application_id(event_id) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETEVENTSTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	/* 只处理位于master同核下的satellite预存储 */
	if (dem_current_partition_is_master() == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETEVENTSTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	return dem_handle_reset_event_status(event_id);
}

FUNC(std_return_type, DEM_CODE)
dem_get_event_uds_status(dem_event_id_type event_id, dem_uds_status_byte_type *uds_status_byte)
{
	std_return_type ret = E_NOT_OK;
	dem_event_status_rec_type *event_status_rec = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_GETEVENTSTATUS_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV(NULL != uds_status_byte, DEM_DEM_GETEVENTSTATUS_ID_DEM_E_PARAM_POINTER, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	event_status_rec = (dem_event_status_rec_type *)uds_status_byte;
	ret = dem_get_event_status(event_id, event_status_rec);

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE)
dem_get_fdc(dem_event_id_type event_id, sint8 *fault_detection_counter)
{
	std_return_type ret = E_OK;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETFAULTDETECTIONCOUNTER_ID_DEM_E_UNINIT, E_NOT_OK);

#if (DEM_DEBOUNCE_DISABLE == STD_ON)
	(void)event_id;
	(void)fault_detection_counter;
	ret = E_NOT_OK;
#else
	DEM_ENTER_EXCLUSIVE_AREA_0();

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	uint8 group_index = dem_get_event_combined_group_index(event_id);

	if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
		*fault_detection_counter = dem_get_event_combined_group_fdc(group_index);
	} else {
		ret = dem_get_fdc_internal(event_id, fault_detection_counter);
	}
#else
	{
		ret = dem_get_fdc_internal(event_id, fault_detection_counter);
	}
#endif

	DEM_EXIT_EXCLUSIVE_AREA_0();
#endif

	return ret;
}

FUNC(std_return_type, DEM_CODE)
dem_set_opcycle_state(dem_opcycle_id_type opcycle_id, dem_opcycle_state_type cycle_state)
{
	std_return_type ret = E_OK;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_SETOPERATIONCYCLESTATE_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(dem_current_partition_is_master() == TRUE,
			DEM_DEM_SETOPERATIONCYCLESTATE_ID_DEM_E_WRONG_CONDITION, E_NOT_OK);
	DEM_VALIDATE_RV(DEM_ACTIVE != opcycle_id, DEM_DEM_SETOPERATIONCYCLESTATE_ID_DEM_E_UNEXPECTED_EXECUTION,
			E_NOT_OK);
	DEM_VALIDATE_RV(opcycle_id < DEM_OPERATION_CYCLE_ID_ENDMARK,
			DEM_DEM_SETOPERATIONCYCLESTATE_ID_DEM_E_UNEXPECTED_EXECUTION, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	dem_set_pending_opcycle(opcycle_id,
				      ((DEM_CYCLE_STATE_START == cycle_state) ? DEM_PENDING_CYCLE_STATE_STARTED :
										DEM_PENDING_CYCLE_STATE_STOPPED));

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_restart_opcycle(uint8 opcycle_id)
{
	return dem_set_opcycle_state(opcycle_id, DEM_CYCLE_STATE_START);
}

FUNC(std_return_type, DEM_CODE)
dem_get_dtc_of_event(dem_event_id_type event_id, dem_dtc_format_type dtc_format, uint32 *dtc_of_event)
{
	std_return_type ret = DEM_E_NO_DTC_AVAILABLE;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_GETDTCOFEVENT_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV(IS_VALID_DTC_FORMAT(dtc_format), DEM_DEM_GETDTCOFEVENT_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	lookup_event_param(event_id, &event_param);
	lookup_event_info_rec(event_id, &event_info_rec);

	if ((NULL == event_param) || (NULL == event_info_rec)) {
		return E_NOT_OK;
	}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if (FALSE == event_info_rec->is_available) {
		return E_NOT_OK;
	}
#endif
	dtc_class = dem_get_event_dtc_class(event_param);
	if (NULL == dtc_class) {
		return E_NOT_OK;
	}

	DEM_ENTER_EXCLUSIVE_AREA_0();

	if (dem_is_dtc_format_match(event_param, dtc_format)) {
		*dtc_of_event = (DEM_DTC_FORMAT_UDS == dtc_format) ?
					dtc_class->uds_dtc :
					DEM_TO_OBD_FORMAT(dtc_class->obd_dtc);
		ret = E_OK;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_get_status_of_dtc(uint8 client_id, dem_event_status_rec_type *dtc_status)
{
	std_return_type ret = E_NOT_OK;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_selector_data_type *selector_ptr = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_GETSTATUSOFDTC_ID_DEM_E_UNINIT,
			E_NOT_OK);

	selector_ptr = dem_client_id_get_selection_result(client_id);

	DEM_VALIDATE_RV(((NULL != selector_ptr) && (TRUE == selector_ptr->dtc_selector_is_set)),
			DEM_DEM_GETSTATUSOFDTC_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(DEM_SELECTED_SELECTION_DTC == selector_ptr->selector_state,
			DEM_DEM_GETSTATUSOFDTC_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(IS_SUPPORTED_ORIGIN(selector_ptr->request.dtc_origin),
			DEM_DEM_GETSTATUSOFDTC_ID_DEM_E_PARAM_DATA, DEM_WRONG_DTCORIGIN);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	if (dem_lookup_event_of_uds_dtc(selector_ptr->request.dtc, &event_info_rec) == TRUE) {
		lookup_event_param(event_info_rec->event_id, &event_param);
		if (dem_is_dtc_origin_match(selector_ptr->request.dtc_origin, event_param, FALSE) == TRUE) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			uint8 group_index = dem_get_event_combined_group_index(event_info_rec->event_id);

			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
				*dtc_status = dem_calculate_event_combine_group_uds_status(group_index) &
					      DEM_DTC_STATUS_AVAILABILITY_MASK;
				ret = E_OK;
#endif
			} else {
				*dtc_status = event_info_rec->event_status_rec & DEM_DTC_STATUS_AVAILABILITY_MASK;
				ret = E_OK;
			}
#else
			{
				*dtc_status = event_info_rec->event_status_rec & DEM_DTC_STATUS_AVAILABILITY_MASK;
				ret = E_OK;
			}
#endif
		} else {
			ret = DEM_WRONG_DTC;
		}
	} else {
		ret = DEM_WRONG_DTC;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE)
dem_get_dtc_status_availability_mask(uint8 client_id, uint8 *dtc_status_mask)
{
	(void)client_id;

	*dtc_status_mask = DEM_DTC_STATUS_AVAILABILITY_MASK;

	return E_OK;
}

FUNC(std_return_type, DEM_CODE)
dem_set_dtc_filter(uint8 client_id, uint8 dtc_status_mask, dem_dtc_format_type dtc_format,
		   dem_dtc_origin_type dtc_origin, boolean filter_with_severity,
		   dem_dtc_severity_type dtc_severity_mask, boolean filter_for_fault_detection_counter)
{
	std_return_type ret = E_OK;
	uint8 dtc_status_availability_mask = 0u;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_SETDTCFILTER_ID_DEM_E_UNINIT,
			DEM_PENDING);

	DEM_VALIDATE_RV((DEM_DTC_ORIGIN_PRIMARY_MEMORY == dtc_origin) ||
				(DEM_DTC_ORIGIN_PERMANENT_MEMORY == dtc_origin),
			DEM_DEM_SETDTCFILTER_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	DEM_VALIDATE_RV(((filter_with_severity == (boolean)FALSE) ||
			 ((filter_with_severity == (boolean)TRUE) &&
			  DEM_IS_MASK_UNSET(dtc_severity_mask,
				~(DEM_SEVERITY_MAINTENANCE_ONLY | DEM_SEVERITY_CHECK_AT_NEXT_HALT |
				  DEM_SEVERITY_CHECK_IMMEDIATELY), dem_dtc_severity_type))),
			DEM_DEM_SETDTCFILTER_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	DEM_VALIDATE_RV((filter_for_fault_detection_counter == (boolean)TRUE) ||
				(filter_for_fault_detection_counter == (boolean)FALSE),
			DEM_DEM_SETDTCFILTER_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	DEM_VALIDATE_RV(IS_VALID_DTC_FORMAT(dtc_format), DEM_DEM_SETDTCFILTER_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	(void)dem_get_dtc_status_availability_mask(client_id, &dtc_status_availability_mask);

	if (DEM_IS_MASK_UNSET(dtc_status_mask, dtc_status_availability_mask, uint8) &&
	    (DEM_DTC_STATUS_MASK_ALL != dtc_status_mask)) {
		ret = E_NOT_OK;
	} else {
		dem_dtc_filter_type *dtc_filter_ptr = dem_get_filter_by_client_id(client_id);

		DEM_VALIDATE_RV(NULL != dtc_filter_ptr, DEM_DEM_SETDTCFILTER_ID_DEM_E_WRONG_CLIENT, E_NOT_OK);

		dtc_filter_ptr->dtc_status_mask = dtc_status_mask & dtc_status_availability_mask;
		dtc_filter_ptr->dtc_origin = dtc_origin;
		dtc_filter_ptr->filter_with_severity = filter_with_severity;
		dtc_filter_ptr->dtc_severity_mask = dtc_severity_mask;
		dtc_filter_ptr->filter_for_fault_detection_counter = filter_for_fault_detection_counter;
		dtc_filter_ptr->fault_index = DEM_MAX_NUMBER_EVENT;
		dtc_filter_ptr->dtc_format = dtc_format;
		dtc_filter_ptr->dtc_filter_is_set = TRUE;
	}
	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_get_number_of_filtered_dtc(uint8 client_id, uint16 *number_of_filtered_dtc)
{
	uint16 i = 0u;
	uint16 number_of_faults = 0u;
	std_return_type ret = E_OK;
	const dem_dtc_filter_type *dtc_filter = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETNUMBEROFFILTEREDDTC_ID_DEM_E_UNINIT, DEM_PENDING);
	DEM_VALIDATE_RV(NULL != number_of_filtered_dtc, DEM_DEM_GETNUMBEROFFILTEREDDTC_ID_DEM_E_PARAM_POINTER,
			E_NOT_OK);

	dtc_filter = dem_get_filter_by_client_id(client_id);

	DEM_VALIDATE_RV(((NULL != dtc_filter) && (TRUE == dtc_filter->dtc_filter_is_set)),
			DEM_DEM_GETNUMBEROFFILTEREDDTC_ID_DEM_E_WRONG_CONDITION, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		if (DEM_EVENT_ID_NULL != g_dem_event_info_rec[i].event_id) {
			if (dem_is_dtc_match_filter(dtc_filter, &g_dem_event_info_rec[i]) == TRUE) {
				number_of_faults++;
			}
		}
	}

	*number_of_filtered_dtc = number_of_faults;

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE)
dem_get_next_filtered_dtc(uint8 client_id, uint32 *dtc, dem_event_status_rec_type *dtc_status)
{
	std_return_type ret = E_OK;
	boolean dtc_found = FALSE;
	uint32 last_event_timestamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
	uint32 current_event_timestamp = 0u;
	dem_dtc_filter_type *dtc_filter = NULL;
	const dem_dtc_class_type *dtc_class = NULL;
	const dem_event_parameter_type *event_param = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_GETNEXTFILTEREDDTC_ID_DEM_E_UNINIT,
			DEM_PENDING);
	DEM_VALIDATE_RV(NULL != dtc, DEM_DEM_GETNEXTFILTEREDDTC_ID_DEM_E_PARAM_POINTER, DEM_NO_SUCH_ELEMENT);
	DEM_VALIDATE_RV(NULL != dtc_status, DEM_DEM_GETNEXTFILTEREDDTC_ID_DEM_E_PARAM_POINTER, DEM_NO_SUCH_ELEMENT);

	dtc_filter = dem_get_filter_by_client_id(client_id);

	DEM_VALIDATE_RV(((NULL != dtc_filter) && (TRUE == dtc_filter->dtc_filter_is_set)),
			DEM_DEM_GETNEXTFILTEREDDTC_ID_DEM_E_WRONG_CONDITION, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	if (DEM_MAX_NUMBER_EVENT != dtc_filter->fault_index) {
		last_event_timestamp = g_dem_event_info_rec[dtc_filter->fault_index].timestamp;
	}

	/* 先根据event发生时间查找失败过的dtc */
	if ((DEM_MAX_NUMBER_EVENT == dtc_filter->fault_index) ||
	    (0u != g_dem_event_info_rec[dtc_filter->fault_index].timestamp)) {
		for (uint16 buffer_index = DEM_MAX_NUMBER_EVENT - 1u; buffer_index != 0xFFFFu; buffer_index--) {
			if (DEM_EVENT_ID_NULL == g_dem_event_info_rec[buffer_index].event_id) {
				continue;
			}
			if (dem_is_dtc_match_filter(dtc_filter, &g_dem_event_info_rec[buffer_index]) == FALSE) {
				continue;
			}
			if ((g_dem_event_info_rec[buffer_index].timestamp >= last_event_timestamp) ||
			    (g_dem_event_info_rec[buffer_index].timestamp <= current_event_timestamp)) {
				continue;
			}

			current_event_timestamp = g_dem_event_info_rec[buffer_index].timestamp;
			dtc_found = TRUE;
			dtc_filter->fault_index = buffer_index;
		}
	}

	if (TRUE == dtc_found) {
		lookup_event_param(g_dem_event_info_rec[dtc_filter->fault_index].event_id, &event_param);
		dtc_class = dem_get_event_dtc_class(event_param);

		if (NULL == dtc_class) {
			ret = DEM_NO_SUCH_ELEMENT;
		} else if (DEM_DTC_FORMAT_UDS == dtc_filter->dtc_format) {
			*dtc = dtc_class->uds_dtc;
		} else {
			*dtc = DEM_TO_OBD_FORMAT(dtc_class->obd_dtc);
		}
		*dtc_status = g_dem_event_info_rec[dtc_filter->fault_index].event_status_rec &
			      DEM_DTC_STATUS_AVAILABILITY_MASK;
	} else {
		/* 之后查找余下没有失败过的dtc */
		if ((DEM_MAX_NUMBER_EVENT != dtc_filter->fault_index) &&
		    (0u != g_dem_event_info_rec[dtc_filter->fault_index].timestamp)) {
			dtc_filter->fault_index = DEM_MAX_NUMBER_EVENT;
		}
		while ((FALSE == dtc_found) && (0u != dtc_filter->fault_index)) {
			dtc_filter->fault_index--;
			if (DEM_EVENT_ID_NULL == g_dem_event_info_rec[dtc_filter->fault_index].event_id) {
				continue;
			}

			if ((dem_is_dtc_match_filter(dtc_filter, &g_dem_event_info_rec[dtc_filter->fault_index]) !=
			     TRUE) ||
			    (0u != g_dem_event_info_rec[dtc_filter->fault_index].timestamp)) {
				continue;
			}
			lookup_event_param(g_dem_event_info_rec[dtc_filter->fault_index].event_id, &event_param);

			dtc_class = dem_get_event_dtc_class(event_param);
			if (NULL == dtc_class) {
				continue;
			} else if (DEM_DTC_FORMAT_UDS == dtc_filter->dtc_format) {
				*dtc = dtc_class->uds_dtc;
			} else {
				*dtc = DEM_TO_OBD_FORMAT(dtc_class->obd_dtc);
			}
			*dtc_status = g_dem_event_info_rec[dtc_filter->fault_index].event_status_rec &
				      DEM_DTC_STATUS_AVAILABILITY_MASK;
			dtc_found = TRUE;
		}
	}

	if (TRUE == dtc_found) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
		uint8 group_index =
			dem_get_event_combined_group_index(g_dem_event_info_rec[dtc_filter->fault_index].event_id);
		if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
			*dtc_status = dem_calculate_event_combine_group_uds_status(group_index) &
				      DEM_DTC_STATUS_AVAILABILITY_MASK;
#endif
		}
#endif
	} else {
		dtc_filter->fault_index = DEM_MAX_NUMBER_EVENT;
		ret = DEM_NO_SUCH_ELEMENT;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(dem_dtc_translation_format_type, DEM_CODE) dem_get_translation_type(uint8 client_id)
{
	(void)client_id;

	return DEM_TYPE_OF_DTC_SUPPORTED;
}

FUNC(std_return_type, DEM_CODE) dem_clear_dtc(uint8 client_id)
{
	std_return_type ret = E_NOT_OK;
	const dem_dtc_selector_data_type *selector_ptr = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_CLEARDTC_ID_DEM_E_UNINIT,
			DEM_CLEAR_FAILED);

	selector_ptr = dem_client_id_get_selection_result(client_id);

	DEM_VALIDATE_RV(((NULL != selector_ptr) && (TRUE == selector_ptr->dtc_selector_is_set)),
			DEM_DEM_CLEARDTC_ID_DEM_E_WRONG_CONFIGURATION, E_NOT_OK);
	DEM_VALIDATE_RV(IS_VALID_DTC_FORMAT(selector_ptr->request.dtc_format), DEM_DEM_CLEARDTC_ID_DEM_E_PARAM_DATA,
			DEM_CLEAR_FAILED);

#if (DEM_CLEAR_DTC_NOTIFICATION_START == STD_ON)
	(void)dem_clear_dtc_notification_start(selector_ptr->request.dtc, selector_ptr->request.dtc_format,
					       selector_ptr->request.dtc_origin);
#endif

	DEM_ENTER_EXCLUSIVE_AREA_0();

	switch (selector_ptr->selector_state) {
		case DEM_SELECTED_SELECTION_DTC:
			ret = dem_clear_dtc_process_single_dtc(selector_ptr);
			break;
		case  DEM_SELECTED_SELECTION_GROUP_DTC:
			ret = dem_clear_dtc_process_group(selector_ptr);
			break;
		case DEM_SELECTED_ALL_DTCS:
			ret = dem_clear_dtc_process_all_dtc(selector_ptr);
			break;
		case  DEM_SELECTED_SELECTION_EMISSION_DTC:
			ret = dem_clear_dtc_process_emission(selector_ptr);
			break;
		case  DEM_SELECTED_SELECTION_NO_MATCH_DTC:
			ret = E_NOT_OK;
			break;
		case  DEM_SELECTED_SELECTION_WRONG_ORIGIN:
			ret = DEM_WRONG_DTCORIGIN;
			break;
		default:
			DEM_DET_REPORT(DEM_DEM_CLEARDTC_ID_DEM_E_UNEXPECTED_EXECUTION);
			break;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

#if (DEM_CLEAR_DTC_NOTIFICATION_FINISH == STD_ON)
	(void)dem_clear_dtc_notification_finish(selector_ptr->request.dtc, selector_ptr->request.dtc_format,
						selector_ptr->request.dtc_origin);
#endif

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_disable_dtc_setting(uint8 client_id)
{
	(void)client_id;
	std_return_type ret = E_OK;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_DISABLEDTCSETTING_ID_DEM_E_UNINIT,
			DEM_PENDING);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	g_dem_disable_dtc_set.dtc_group = DEM_DTC_GROUP_ALL_DTCS;
	g_dem_disable_dtc_set.setting_disabled = TRUE;

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_enable_dtc_setting(uint8 client_id)
{
	(void)client_id;
	std_return_type ret = E_OK;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_ENABLEDTCSETTING_ID_DEM_E_UNINIT,
			DEM_PENDING);

	g_dem_disable_dtc_set.setting_disabled = FALSE;

	return ret;
}

FUNC(dem_return_get_ext_data_record_by_dtc_type, DEM_CODE)
dem_get_ext_data_record_by_dtc(uint32 dtc, dem_dtc_origin_type dtc_origin, uint8 ext_data_number, uint8 *dest_buffer,
			       uint16 *buf_size)
{
	dem_return_get_ext_data_record_by_dtc_type ret = DEM_RECORD_WRONG_DTC;
	uint16 ed_data_size = 0u;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_ext_data_record_class_type *ext_data_record_class = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETEXTENDEDDATARECORDBYDTC_ID_DEM_E_UNINIT, DEM_RECORD_WRONG_DTC);
	DEM_VALIDATE_RV((NULL != dest_buffer), DEM_DEM_GETEXTENDEDDATARECORDBYDTC_ID_DEM_E_PARAM_POINTER,
			DEM_RECORD_WRONG_DTC);
	DEM_VALIDATE_RV((NULL != buf_size), DEM_DEM_GETEXTENDEDDATARECORDBYDTC_ID_DEM_E_PARAM_POINTER,
			DEM_RECORD_WRONG_DTC);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	if (ext_data_number > DEM_HIGHEST_EXT_DATA_REC_NUM) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_RECORD_NUMBER;
	}
	if (dem_lookup_event_of_uds_dtc(dtc, &event_info_rec) == FALSE) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_RECORD_WRONG_DTC;
	}
	lookup_event_param(event_info_rec->event_id, &event_param);

	if (dem_is_dtc_origin_match(dtc_origin, event_param, FALSE) == FALSE) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_RECORD_WRONG_DTCORIGIN;
	}
	if (dem_lookup_ext_data_recnum_param(ext_data_number, event_param, &ext_data_record_class) == FALSE) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_RECORD_NUMBER;
	}

	ed_data_size = dem_get_ext_data_record_data_size(ext_data_record_class);
	if (*buf_size < ed_data_size) {
		DEM_DET_REPORT(DEM_DEM_GETEXTENDEDDATARECORDBYDTC_ID_DEM_E_PARAM_LENGTH);

		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_RECORD_BUFFERSIZE;
	}
	ret = dem_get_ext_data_record(ext_data_record_class, dtc_origin, dest_buffer, buf_size,
						       event_info_rec, ext_data_number);

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(dem_return_get_ff_data_by_dtc_type, DEM_CODE)
dem_get_ff_data_by_dtc(uint32 dtc, dem_dtc_origin_type dtc_origin, uint8 recnum, uint8 *dest_buffer, uint16 *buf_size)
{
	uint16 ff_data_size = 0;
	dem_return_get_ff_data_by_dtc_type ret = DEM_GET_FFDATABYDTC_WRONG_DTC;
	dem_event_info_rec_type *event_info_rec = NULL;
	dem_ff_class_type const *ff_data_record_class = NULL;
	const dem_event_parameter_type *event_param = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETFREEZEFRAMEDATABYDTC_ID_DEM_E_UNINIT, DEM_GET_ID_PENDING);
	DEM_VALIDATE_RV((NULL != dest_buffer), DEM_DEM_GETFREEZEFRAMEDATABYDTC_ID_DEM_E_PARAM_POINTER,
			DEM_GET_FFDATABYDTC_WRONG_DTC);
	DEM_VALIDATE_RV((NULL != buf_size), DEM_DEM_GETFREEZEFRAMEDATABYDTC_ID_DEM_E_PARAM_POINTER,
			DEM_GET_FFDATABYDTC_WRONG_DTC);

	if (recnum > DEM_HIGHEST_FF_REC_NUM) {
		return DEM_GET_FFDATABYDTC_RECORDNUMBER;
	}

	DEM_ENTER_EXCLUSIVE_AREA_0();

	if (FALSE == dem_lookup_event_of_uds_dtc(dtc, &event_info_rec)) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_GET_FFDATABYDTC_WRONG_DTC;
	}

	lookup_event_param(event_info_rec->event_id, &event_param);

	if (FALSE == dem_is_dtc_origin_match(dtc_origin, event_param, FALSE)) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
	}

	if (FALSE == dem_lookup_ff_data_recnum_param(recnum, event_param, &ff_data_record_class)) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		return DEM_GET_FFDATABYDTC_RECORDNUMBER;
	}

	(void)dem_lookup_ff_data_size(recnum, &ff_data_record_class, &ff_data_size);
	if (*buf_size >= ff_data_size) {
		if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == dtc_origin) {
			if (dem_get_ff_record(event_info_rec->event_id, recnum, ff_data_record_class,
						dtc_origin,dest_buffer, buf_size) == FALSE) {
				*buf_size = 0;
			}
			ret = DEM_GET_FFDATABYDTC_OK;
		} else {
			ret = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
			DEM_DET_REPORT(DEM_DEM_GETFREEZEFRAMEDATABYDTC_ID_DEM_E_NOT_IMPLEMENTED_YET);
		}
	} else {
		DEM_DET_REPORT(DEM_DEM_GETFREEZEFRAMEDATABYDTC_ID_DEM_E_PARAM_LENGTH);
		ret = DEM_GET_FFDATABYDTC_BUFFERSIZE;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_set_ff_record_filter(uint8 client_id, dem_dtc_format_type dtc_format)
{
	dem_ff_record_filter_type *ff_record_filter = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_SETFREEZEFRAMERECORDFILTER_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(IS_VALID_DTC_FORMAT(dtc_format), DEM_DEM_SETFREEZEFRAMERECORDFILTER_ID_DEM_E_PARAM_DATA,
			E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	ff_record_filter = dem_get_ff_record_filter_by_client_id(client_id);
	if (NULL == ff_record_filter) {
		DEM_EXIT_EXCLUSIVE_AREA_0();
		DEM_REPORT_ERROR_RV(DEM_DEM_SETFREEZEFRAMERECORDFILTER_ID_DEM_E_WRONG_CONDITION, E_NOT_OK);
	}

	ff_record_filter->dtc_format = dtc_format;
	ff_record_filter->event_index = 0;
	ff_record_filter->ff_index = 0;
	ff_record_filter->ff_rec_filter_is_set = TRUE;

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return E_OK;
}

FUNC(std_return_type, DEM_CODE)
dem_get_number_of_ff_records(uint8 client_id, uint16 *number_of_filtered_records)
{
	uint16 numberof_records = 0;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETNUMBEROFFREEZEFRAMERECORDS_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV((NULL != number_of_filtered_records),
			DEM_DEM_GETNUMBEROFFREEZEFRAMERECORDS_ID_DEM_E_PARAM_POINTER, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;
	const dem_ff_record_filter_type *ff_record_filter = dem_get_ff_record_filter_by_client_id(client_id);

	if ((NULL == ff_record_filter) || (FALSE == ff_record_filter->ff_rec_filter_is_set)) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		DEM_REPORT_ERROR_RV(DEM_DEM_GETNUMBEROFFREEZEFRAMERECORDS_ID_DEM_E_WRONG_CONDITION, E_NOT_OK);
	}

	for (uint32 i = 0; i < DEM_MAX_NUMBER_EVENT_PRI_MEM; i++) {
		event_id = g_dem_pri_mem_data[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

		lookup_event_info_rec(event_id, &event_info_rec);
		lookup_event_param(event_id, &event_param);

		if ((NULL != event_info_rec) && (NULL != event_param) &&
		    dem_is_dtc_format_match(event_param, ff_record_filter->dtc_format)) {
			dtc_class = dem_get_event_dtc_class(event_param);
			if ((NULL != dtc_class) && dem_is_dtc_available(dtc_class)) {
				numberof_records += event_info_rec->stored_ff_recnum;
			}
		}
	}
#endif
	*number_of_filtered_records = numberof_records;

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return E_OK;
}

FUNC(std_return_type, DEM_CODE) dem_get_next_filtered_record(uint8 client_id, uint32 *dtc, uint8 *recnum)
{
	std_return_type ret = E_NOT_OK;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETNEXTFILTEREDRECORD_ID_DEM_E_UNINIT, DEM_PENDING);
	DEM_VALIDATE_RV(NULL != dtc, DEM_DEM_GETNEXTFILTEREDRECORD_ID_DEM_E_PARAM_POINTER, DEM_NO_SUCH_ELEMENT);
	DEM_VALIDATE_RV(NULL != recnum, DEM_DEM_GETNEXTFILTEREDRECORD_ID_DEM_E_PARAM_POINTER, DEM_NO_SUCH_ELEMENT);

	DEM_ENTER_EXCLUSIVE_AREA_0();

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
	boolean found = FALSE;
	const dem_dtc_class_type *dtc_class = NULL;
	const dem_event_parameter_type *event_param = NULL;
	dem_ff_record_filter_type *ff_record_filter = dem_get_ff_record_filter_by_client_id(client_id);

	if ((NULL == ff_record_filter) || (FALSE == ff_record_filter->ff_rec_filter_is_set)) {
		DEM_EXIT_EXCLUSIVE_AREA_0();

		DEM_REPORT_ERROR_RV(DEM_DEM_GETNEXTFILTEREDRECORD_ID_DEM_E_WRONG_CONDITION, E_NOT_OK);
	}

	for (uint32 i = ff_record_filter->event_index; i < DEM_MAX_NUMBER_EVENT_PRI_MEM; i++) {
		for (uint16 j = ff_record_filter->ff_index; j < DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS; j++) {
			lookup_event_param(g_dem_pri_mem_data[i].event_id, &event_param);
			if ((NULL == event_param) ||
			    (FALSE == dem_is_dtc_format_match(event_param, ff_record_filter->dtc_format))) {
				continue;
			}

			dtc_class = dem_get_event_dtc_class(event_param);
			if ((NULL != dtc_class) && dem_is_dtc_available(dtc_class)) {
				*recnum = (uint8)(ff_record_filter->ff_index + 1u);
				*dtc = (DEM_DTC_FORMAT_UDS == ff_record_filter->dtc_format) ?
						dtc_class->uds_dtc :
						DEM_TO_OBD_FORMAT(dtc_class->obd_dtc);
				if (j < (DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS - 1u)) {
					ff_record_filter->ff_index = j + 1u;
				} else {
					ff_record_filter->event_index = i + 1u;
					ff_record_filter->ff_index = 0u;
				}

				found = TRUE;
				ret = E_OK;
				break;
			}
		}
		if (found) {
			break;
		}
	}
#endif

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
FUNC(std_return_type, DEM_CODE) dem_set_enable_condition(uint8 condition_id, boolean condition_fulfilled)
{
	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_SETENABLECONDITION_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV((condition_id < DEM_NUM_ENABLECONDITIONS),
			DEM_DEM_SETENABLECONDITION_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	g_dem_enable_conditions[condition_id] = condition_fulfilled;

	return E_OK;
}
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
FUNC(std_return_type, DEM_CODE) dem_set_storage_condition(uint8 condition_id, boolean condition_fulfilled)
{
	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_SETSTORAGECONDITION_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV((condition_id < DEM_NUM_STORAGECONDITIONS),
			DEM_DEM_SETSTORAGECONDITION_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	g_dem_storage_conditions[condition_id] = condition_fulfilled;

	return E_OK;
}
#endif

#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
FUNC(std_return_type, DEM_CODE)
dem_get_event_memory_overflow(uint8 client_id, dem_dtc_origin_type dtc_origin, boolean *overflow_indication)
{
	(void)client_id;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETEVENTMEMORYOVERFLOW_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(NULL != overflow_indication, DEM_DEM_GETEVENTMEMORYOVERFLOW_ID_DEM_E_PARAM_POINTER, E_NOT_OK);

	return dem_get_overflow_indication(dtc_origin, overflow_indication);
}
#endif

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
FUNC(std_return_type, DEM_CODE) dem_set_dtc_suppression(uint8 client_id, boolean suppression_status)
{
	std_return_type ret = E_NOT_OK;
	dem_dtc_format_type dtc_format = DEM_DTC_FORMAT_UDS;
	uint32 dtc = 0u;
	const dem_dtc_selector_data_type *selector_ptr = NULL;
	const dem_dtc_class_type *dtc_class_list = NULL;
	const dem_dtc_class_type *dtc_class_ptr = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_SETDTCSUPPRESSION_ID_DEM_E_UNINIT,
			E_NOT_OK);

	selector_ptr = dem_client_id_get_selection_result(client_id);
	DEM_VALIDATE_RV(((NULL != selector_ptr) && (TRUE == selector_ptr->dtc_selector_is_set)),
			DEM_DEM_SETDTCSUPPRESSION_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	dtc = selector_ptr->request.dtc;
	dtc_format = selector_ptr->request.dtc_format;

	DEM_VALIDATE_RV(IS_VALID_DTC_FORMAT(dtc_format), DEM_DEM_SETDTCSUPPRESSION_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	dtc_class_list = g_dem_config_set->dtc_class;
	dtc_class_ptr = NULL;
	for (uint16 i = 0u; i < DEM_NOF_DTCS; i++) {
		dtc_class_ptr = &dtc_class_list[i];
		if (dem_is_dtc_match_format(dtc, dtc_format, dtc_class_ptr) == E_OK) {
			g_dem_dtc_suppressed[dtc_class_ptr->dtc_index].suppressed_by_dtc = suppression_status;
			ret = E_OK;
			break;
		}
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_get_dtc_suppression(uint8 client_id, boolean *suppression_status)
{
	std_return_type ret = E_NOT_OK;
	dem_dtc_format_type dtc_format = DEM_DTC_FORMAT_UDS;
	uint32 dtc = 0u;
	const dem_dtc_selector_data_type *selector_ptr = NULL;
	const dem_dtc_class_type *dtc_class_list = NULL;
	const dem_dtc_class_type *dtc_class_ptr = NULL;

	DEM_VALIDATE_RV(dem_is_master_in_state(DEM_STATE_INIT) == TRUE,
			DEM_DEM_SETDTCSUPPRESSION_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(dem_current_partition_is_master() == TRUE, DEM_DEM_SETDTCSUPPRESSION_ID_DEM_E_WRONG_CONDITION,
			E_NOT_OK);

	selector_ptr = dem_client_id_get_selection_result(client_id);
	DEM_VALIDATE_RV(((NULL != selector_ptr) && (TRUE == selector_ptr->dtc_selector_is_set)),
			DEM_DEM_SETDTCSUPPRESSION_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	dtc = selector_ptr->request.dtc;
	dtc_format = selector_ptr->request.dtc_format;
	DEM_VALIDATE_RV(IS_VALID_DTC_FORMAT(dtc_format), DEM_DEM_SETDTCSUPPRESSION_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	dtc_class_list = g_dem_config_set->dtc_class;
	dtc_class_ptr = NULL;
	for (uint16 i = 0u; i < DEM_NOF_DTCS; i++) {
		dtc_class_ptr = &dtc_class_list[i];
		if (dem_is_dtc_match_format(dtc, dtc_format, dtc_class_ptr) == E_OK) {
			if ((TRUE == g_dem_dtc_suppressed[dtc_class_ptr->dtc_index].suppressed_by_dtc) ||
			    (TRUE == g_dem_dtc_suppressed[dtc_class_ptr->dtc_index].suppressed_by_event)) {
				*suppression_status = TRUE;
			} else {
				*suppression_status = FALSE;
			}
			ret = E_OK;
			break;
		}
	}

	return ret;
}
#endif

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
FUNC(std_return_type, DEM_CODE) dem_set_event_available(dem_event_id_type event_id, boolean available_status)
{
	std_return_type ret = E_NOT_OK;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;
	dem_event_status_rec_type old_status;
	dem_event_info_rec_type *event_info_rec = NULL;

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = 0u;
#endif

	DEM_VALIDATE_RV(((dem_is_master_at_least_in_state(DEM_INIT_PREINIT) != FALSE) &&
			 (dem_get_master_state() != DEM_STATE_SHUTDOWN)),
			DEM_DEM_SETEVENTAVAILABLE_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(DEM_IS_VALID_EVENT_ID(event_id), DEM_DEM_SETEVENTAVAILABLE_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	if (dem_current_partition_is_master() == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_SETEVENTAVAILABLE_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	DEM_ENTER_EXCLUSIVE_AREA_0();

	lookup_event_info_rec(event_id, &event_info_rec);
	lookup_event_param(event_id, &event_param);

	if ((NULL != event_info_rec) && (NULL != event_param) && (TRUE == event_param->event_available) &&
	    DEM_IS_MASK_UNSET(event_info_rec->event_status_rec, DEM_TEST_FAILED, dem_event_status_rec_type) &&
	    ((dem_get_master_state() == DEM_INIT_PREINIT) ||
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	     !dem_is_event_stored_in_memory(dem_get_master_event(event_id))
#else
	     !dem_is_event_stored_in_memory(event_id)
#endif
		     )) {
		if (event_info_rec->is_available != available_status) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			uint8 group_index = dem_get_event_combined_group_index(event_id);

			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				old_combined_status = dem_get_event_combined_group_uds_status(group_index);
			}
#endif
			event_info_rec->is_available = available_status;
			dem_set_event_available_data(event_info_rec->event_id, available_status);
			old_status = event_info_rec->event_status_rec;
			if (!available_status) {
				event_info_rec->event_status_rec = 0x00;
			} else {
				event_info_rec->event_status_rec = 0x50;
			}

			if (old_status != event_info_rec->event_status_rec) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
				if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
					/* event状态发生改变，更新事件组状态 */
					dem_handle_event_combined_group_status_changed(group_index);
					new_combined_status = dem_get_event_combined_group_uds_status(group_index);
					if (old_combined_status != new_combined_status) {
						dem_notify_dtc_status_changed(event_param, old_combined_status,
									  new_combined_status);
					}
				} else {
					dem_notify_dtc_status_changed(event_param, old_status,
								  event_info_rec->event_status_rec);
				}
#else
				{
					dem_notify_dtc_status_changed(event_param, old_status,
								  event_info_rec->event_status_rec);
				}
#endif
				dem_notify_event_status_change(event_param, old_status,
							       event_info_rec->event_status_rec);
			}

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
			boolean suppressed = (!available_status);
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			group_index = dem_get_event_combined_group_index(event_id);
			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				suppressed = dem_calculate_combined_group_dtc_suppressed_status(group_index);
			}
#endif
			dtc_class = dem_get_event_dtc_class(event_param);
			if (NULL != dtc_class) {
				g_dem_dtc_suppressed[dtc_class->dtc_index].suppressed_by_event = suppressed;
			}
#endif
		}
		ret = E_OK;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}
#endif

FUNC(std_return_type, DEM_CODE)
dem_get_monitor_status(dem_event_id_type event_id, dem_monitor_status_type *monitor_status)
{
	dem_event_status_rec_type event_status = 0u;
	dem_event_info_rec_type *event_info_rec = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_GETMONITORSTATUS_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV((NULL != monitor_status), DEM_DEM_GETMONITORSTATUS_ID_DEM_E_PARAM_POINTER, E_NOT_OK);
	DEM_VALIDATE_RV(DEM_IS_VALID_EVENT_ID(event_id), DEM_DEM_GETMONITORSTATUS_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	lookup_event_info_rec(event_id, &event_info_rec);
	if (NULL == event_info_rec) {
		return E_NOT_OK;
	}

	*monitor_status = 0u;

	event_status = event_info_rec->event_status_rec;

	if (DEM_IS_MASK_SET(event_status, DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type)) {
		DEM_SET_MASK(*monitor_status, DEM_MONITOR_TNCTOC_STATUS, dem_monitor_status_type);
	}
	if (DEM_IS_MASK_SET(event_status, DEM_TEST_FAILED, dem_event_status_rec_type)) {
		DEM_SET_MASK(*monitor_status, DEM_MONITOR_TF_STATUS, dem_monitor_status_type);
	}

	return E_OK;
}

FUNC(dem_opcycle_state_type, DEM_CODE) dem_get_opcycle_state(uint8 opcycle_id)
{
	dem_opcycle_state_type state = 0xFFu;

	if (opcycle_id < DEM_OPERATION_CYCLE_ID_ENDMARK) {
		state = g_dem_opcycle_state_list[opcycle_id];
	}

	return state;
}

FUNC(std_return_type, DEM_CODE)
dem_reset_event_debounce_status(dem_event_id_type event_id, dem_debounce_reset_status_type debounce_reset_status)
{
	(void)debounce_reset_status;
	std_return_type ret = E_OK;
	boolean check_res = FALSE;
	dem_event_info_rec_type *event_info = NULL;

#if (DEM_USE_CONFIGPTR == STD_ON)
	check_res = dem_is_global_cfg_ptr_at_least_in_state(DEM_CONFIGPTR_PREINITIALIZED);
	if (FALSE == check_res) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETDEBOUNCESTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}
#endif

	if (dem_is_master_at_least_in_state(DEM_INIT_PREINIT) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETDEBOUNCESTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	check_res = dem_is_master_in_state(DEM_STATE_SHUTDOWN);
	if (TRUE == check_res) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETDEBOUNCESTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	check_res = dem_is_satellite_in_state(dem_get_event_satellite_id(event_id), DEM_INIT_PREINIT);
	if (FALSE == check_res) {
		check_res = dem_is_satellite_in_state(dem_get_event_satellite_id(event_id), DEM_STATE_INIT);
		if (FALSE == check_res) {
			DEM_REPORT_ERROR_RV(DEM_DEM_RESETDEBOUNCESTATUS_ID_DEM_E_UNINIT, E_NOT_OK);
		}
	}

	check_res = dem_check_event_application_id(event_id);
	if (FALSE == check_res) {
		DEM_REPORT_ERROR_RV(DEM_DEM_RESETDEBOUNCESTATUS_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	}

	lookup_event_info_rec(event_id, &event_info);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	if (NULL != event_info) {
		dem_reset_debounce_counter(event_info);
	} else {
		ret = E_NOT_OK;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

/***********************************************************************************************************************
 *  dem_select_dtc()
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_select_dtc(uint8 client_id, uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_SELECTDTC_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV(dtc <= DEM_DTC_GROUP_ALL_DTCS, DEM_DEM_SELECTDTC_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(dtc_format < DEM_DTC_FORMAT_INVALID, DEM_DEM_SELECTDTC_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(dtc_origin < DEM_DTC_ORIGIN_INVALID, DEM_DEM_SELECTDTC_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(DEM_DTC_ORIGIN_NOT_USED != dtc_origin, DEM_DEM_SELECTDTC_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	dem_client_id_select_dtc(client_id, dtc, dtc_format, dtc_origin);

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return E_OK;
}

#if defined(DEM_USE_INDICATORS)
FUNC(std_return_type, DEM_CODE) dem_set_wir_status(dem_event_id_type event_id, boolean wir_status)
{
	std_return_type ret = E_NOT_OK;
	dem_event_status_rec_type old_status = 0u;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_event_parameter_type *event_param = NULL;

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = old_combined_status;
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
#endif

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT, DEM_DEM_SETWIRSTATUS_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV(DEM_IS_VALID_EVENT_ID(event_id), DEM_DEM_SETWIRSTATUS_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	lookup_event_info_rec(event_id, &event_info_rec);
	lookup_event_param(event_id, &event_param);

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if ((NULL == event_param) || (NULL == event_info_rec) || (FALSE == event_info_rec->is_available)) {
#else
	if ((NULL == event_param) || (NULL == event_info_rec)) {
#endif
		return E_NOT_OK;
	}

	DEM_ENTER_EXCLUSIVE_AREA_0();

	if (dem_is_event_allow_process(event_param) == (boolean)TRUE) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
		group_index = dem_get_event_combined_group_index(event_id);
		if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
			old_combined_status = dem_get_event_combined_group_uds_status(group_index);
		}
#endif
		old_status = event_info_rec->event_status_rec;

		if (wir_status == (boolean)TRUE) {
			DEM_SET_MASK(event_info_rec->event_status_rec, DEM_WARNING_INDICATOR_REQUESTED,
				     dem_event_status_rec_type);
		} else if (dem_is_indicator_fulfilled(event_param) == (boolean)FALSE) {
			DEM_CLEAR_MASK(event_info_rec->event_status_rec,
				       DEM_WARNING_INDICATOR_REQUESTED, dem_event_status_rec_type);
		} else {
			/* do nothing */
		}

		if (old_status != event_info_rec->event_status_rec) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				/* event状态发生改变，更新事件组状态 */
				dem_handle_event_combined_group_status_changed(group_index);
				new_combined_status = dem_get_event_combined_group_uds_status(group_index);
				if (old_combined_status != new_combined_status) {
					dem_notify_dtc_status_changed(event_param, old_combined_status,
								  new_combined_status);
				}
			} else {
				dem_notify_dtc_status_changed(event_param, old_status,
							      event_info_rec->event_status_rec);
			}
#else
			{
				dem_notify_dtc_status_changed(event_param, old_status,
							      event_info_rec->event_status_rec);
			}
#endif
			dem_notify_event_status_change(event_param, old_status, event_info_rec->event_status_rec);
		}
		ret = E_OK;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}
#endif

FUNC(std_return_type, DEM_CODE)
dem_get_debouncing_of_event(dem_event_id_type event_id, dem_debouncing_state_type *debouncing_state)
{
	std_return_type ret = E_OK;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETDEBOUNCINGOFEVENT_ID_DEM_E_UNINIT, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	ret = dem_get_debouncing_state(event_id, debouncing_state);

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

#if (DEM_ENABLE_PRESTORED_FF == STD_ON)
FUNC(std_return_type, DEM_CODE) dem_prestore_ff(dem_event_id_type event_id)
{
	std_return_type ret = E_OK;
	dem_prestore_entry_type *entry = NULL;
	const dem_config_set_type *config_set = NULL;
	const dem_event_parameter_type *event_param_list = NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_ref = NULL;
	const dem_event_class_type *event_class_ref = NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(event_id > DEM_EVENT_ID_NULL, DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(event_id <= DEM_EVENT_ID_LAST_VALID_ID, DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_PARAM_DATA,
			E_NOT_OK);

#if (DEM_USE_CONFIGPTR == STD_ON)
	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_INITIALIZED) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);
	}
#endif
	if (dem_is_satellite_in_state(dem_get_event_satellite_id(event_id), DEM_STATE_INIT) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);
	}
	if (dem_check_event_application_id(event_id) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	}

	/* 只处理位于master同核下的satellite预存储 */
	if (dem_current_partition_is_master() == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	}

	config_set = dem_get_config_set();
	event_param_list = config_set->event_parameter;
	event_param = &event_param_list[event_id - 1u];
	dtc_ref = dem_get_event_dtc_class(event_param);
	event_class_ref = event_param->event_class_ref;

	DEM_VALIDATE_RV(((NULL != dtc_ref) && (DEM_NO_DTC != dtc_ref->uds_dtc)),
			DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	DEM_VALIDATE_RV(TRUE == event_class_ref->ff_prestorage_supported,
			DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	// 校验event_id对应的event当前是否available
	DEM_VALIDATE_RV(TRUE == event_param->event_available, DEM_DEM_PRESTOREFREEZEFRAME_ID_DEM_E_PARAM_DATA,
			E_NOT_OK);
#endif

	DEM_ENTER_EXCLUSIVE_AREA_0();

	entry = dem_get_prestore_ff_entry_for_updating(event_id);
	if (NULL != entry) {
		ret = dem_collect_prestored_ff_data(entry, event_param);
	} else {
		ret = E_NOT_OK;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_clear_prestored_ff(dem_event_id_type event_id)
{
	std_return_type ret = E_OK;
	const dem_config_set_type *config_set = NULL;
	const dem_event_parameter_type *event_param_list = NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_ref = NULL;
	const dem_event_class_type *event_class_ref = NULL;
	dem_prestore_entry_type *entry = NULL;

	// 校验dem是否已经初始化
	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);

	if (dem_is_global_cfg_ptr_in_state(DEM_CONFIGPTR_INITIALIZED) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	if (dem_is_satellite_in_state(dem_get_event_satellite_id(event_id), DEM_STATE_INIT) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);
	}

	if (dem_check_event_application_id(event_id) == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	}

	/* 只处理位于master同核下的satellite预存储 */
	if (dem_current_partition_is_master() == FALSE) {
		DEM_REPORT_ERROR_RV(DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	}

	DEM_VALIDATE_RV(event_id > DEM_EVENT_ID_NULL, DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(event_id <= DEM_EVENT_ID_LAST_VALID_ID, DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_PARAM_DATA,
			E_NOT_OK);

	config_set = dem_get_config_set();
	event_param_list = config_set->event_parameter;
	event_param = &event_param_list[event_id - 1u];
	dtc_ref = dem_get_event_dtc_class(event_param);
	event_class_ref = event_param->event_class_ref;

	DEM_VALIDATE_RV(((NULL != dtc_ref) && (DEM_NO_DTC != dtc_ref->uds_dtc)),
			DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	if (FALSE == event_class_ref->ff_prestorage_supported) {
		return E_NOT_OK;
	}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	// 校验event_id对应的event当前是否available
	DEM_VALIDATE_RV(TRUE == event_param->event_available, DEM_DEM_CLEARPRESTOREDFREEZEFRAME_ID_DEM_E_PARAM_DATA,
			E_NOT_OK);
#endif

	DEM_ENTER_EXCLUSIVE_AREA_0();

	entry = dem_get_prestore_ff_entry(event_id);
	if (NULL != entry) {
		ret = dem_clear_prestored_ff_entry(entry);
	} else {
		ret = E_NOT_OK;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}
#endif // DEM_ENABLE_PRESTORED_FF == STD_ON

FUNC(std_return_type, DEM_CODE)
dem_get_dtc_of_obd_ff(uint8 frame_number, uint32 *dtc, dem_dtc_format_type dtc_format)
{
#if (DEM_OBD_SUPPORT == STD_ON)
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;
	std_return_type ret = E_NOT_OK;
	uint8 obd_ff_index = 0u;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_GETDTCOFOBDFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(dem_current_partition_is_master() == TRUE, DEM_DEM_GETDTCOFOBDFREEZEFRAME_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV(dtc != NULL, DEM_DEM_GETDTCOFOBDFREEZEFRAME_ID_DEM_E_PARAM_POINTER, E_NOT_OK);
	DEM_VALIDATE_RV(frame_number == 0u, DEM_DEM_GETDTCOFOBDFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);
	DEM_VALIDATE_RV(DEM_DTC_FORMAT_OBD == dtc_format, DEM_DEM_GETDTCOFOBDFREEZEFRAME_ID_DEM_E_PARAM_DATA, E_NOT_OK);

	DEM_ENTER_EXCLUSIVE_AREA_0();

	event_id = dem_get_obd_ff_event_id(obd_ff_index);
	dem_lookup_event_param(event_id, &event_param);
	if (NULL != event_param) {
		dtc_class = dem_get_event_dtc_class(event_param);
		if ((NULL != dtc_class) && (dem_is_dtc_available(dtc_class) == TRUE) &&
		    (dem_is_dtc_format_match(event_param, dtc_format) == TRUE)) {
			*dtc = DEM_TO_OBD_FORMAT(dtc_class->obd_dtc);
			ret = E_OK;
		} else {
			/* do nothing */
		}
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
#else
	(void)frame_number;
	(void)dtc;
	(void)dtc_format;

	return E_NOT_OK;
#endif
}

FUNC(std_return_type, DEM_CODE)
dem_read_data_of_obd_ff(uint8 pid, uint8 data_elem_index_of_pid, uint8 *dest_buffer, uint8 *buf_size)
{
#if (DEM_OBD_SUPPORT == STD_ON)
	(void)data_elem_index_of_pid;
	std_return_type ret = E_NOT_OK;

	DEM_VALIDATE_RV(dem_get_master_state() == DEM_STATE_INIT,
			DEM_DEM_READDATAOFOBDFREEZEFRAME_ID_DEM_E_UNINIT, E_NOT_OK);
	DEM_VALIDATE_RV(dem_current_partition_is_master() == TRUE, DEM_DEM_READDATAOFOBDFREEZEFRAME_ID_DEM_E_UNINIT,
			E_NOT_OK);
	DEM_VALIDATE_RV(NULL != dest_buffer, DEM_DEM_READDATAOFOBDFREEZEFRAME_ID_DEM_E_PARAM_POINTER, E_NOT_OK);
	DEM_VALIDATE_RV(NULL != buf_size, DEM_DEM_READDATAOFOBDFREEZEFRAME_ID_DEM_E_PARAM_POINTER, E_NOT_OK);

#if (DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA > 0)
	const uint8 *obd_ff_data = NULL;
	const dem_ff_class_type *ff_class;
	boolean pid_found = FALSE;
	uint8 offset = 0;
	uint8 pid_data_size = 0;
	uint8 obd_ff_index = 0u;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;
	const dem_config_set_type *config_set = dem_get_config_set();
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;

	DEM_ENTER_EXCLUSIVE_AREA_0();

	ff_class = config_set->global_obd_ff_class_ref;
	event_id = dem_get_obd_ff_event_id(obd_ff_index);
	dem_lookup_event_param(event_id, &event_param);
	if (NULL != event_param) {
		dtc_class = dem_get_event_dtc_class(event_param);
		if ((NULL == dtc_class) || (dem_is_dtc_available(dtc_class) == FALSE) ||
		    (dem_is_dtc_format_match(event_param, DEM_DTC_FORMAT_OBD) == FALSE)) {
			DEM_EXIT_EXCLUSIVE_AREA_0();

			return E_NOT_OK;
		}

		obd_ff_data = dem_get_obd_ff_data(obd_ff_index);
	}

	if ((NULL != obd_ff_data) && (DEM_FREEZE_FRAME_OBD == ff_class->ff_kind) &&
	    (NULL != ff_class->ff_id_class_ref)) {
		for (uint16 i = 0; (i < DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA) &&
				   ((ff_class->ff_id_class_ref[i]->dem_eol) == FALSE); i++) {
			if ((uint8)(ff_class->ff_id_class_ref[i]->identifier) == pid) {
				pid_data_size =
					(uint8)dem_get_did_data_size(ff_class->ff_id_class_ref[i]);
				pid_found = TRUE;
				break;
			} else {
				offset += (uint8)dem_get_did_data_size(ff_class->ff_id_class_ref[i]);
			}
		}
	}

	if ((TRUE == pid_found) && ((*buf_size) >= pid_data_size) &&
		((offset + pid_data_size) <= DEM_CFG_MAX_SIZE_OBDFREEZEFRAME)) {
		(void)memcpy(dest_buffer, &obd_ff_data[offset], pid_data_size);
		*buf_size = pid_data_size;
		ret = E_OK;
	}

	DEM_EXIT_EXCLUSIVE_AREA_0();
#else
	(void)pid;
	(void)memset(dest_buffer, 0, *buf_size);
#endif

	return ret;
#else
	(void)pid;
	(void)data_elem_index_of_pid;
	(void)dest_buffer;
	(void)buf_size;

	return E_NOT_OK;
#endif
}

#if (DEM_OBD_SUPPORT == STD_ON) && (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
/* ****************************************************************************
 * dem_set_all_perm_status_passive
 *****************************************************************************/
FUNC(void, DEM_CODE) dem_set_all_perm_status_passive(void)
{
	DEM_ENTER_EXCLUSIVE_AREA_0();

	for (uint8 index = 0u; index < DEM_MAX_NUMBER_EVENT_PERM_MEM; index++) {
		if (DEM_EVENT_ID_NULL != g_dem_perm_mem_data.perm_data_entry[index].event_id) {
			dem_set_permanent_status(index, DEM_PERMANENT_DTC_PASSIVE);
		}
	}
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_permanent_block_changed(TRUE);
#else
	dem_nvm_set_permanent_block_changed(FALSE);
#endif

	DEM_EXIT_EXCLUSIVE_AREA_0();
}
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"
