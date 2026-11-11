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

static FUNC(boolean, DEM_CODE)
	is_dtc_status_mask_match(dem_event_status_rec_type filter_mask, const dem_event_info_rec_type *event_info_rec)
{
	if ((DEM_DTC_STATUS_MASK_ALL == filter_mask) ||
	    DEM_IS_MASK_SET(event_info_rec->event_status_rec, filter_mask, dem_event_status_rec_type)) {
		return TRUE;
	}

	return FALSE;
}

static FUNC(boolean, DEM_CODE)
	is_dtc_in_dtc_group(dem_dtc_group_id_type group_id, const dem_event_parameter_type *event_param)
{
	const dem_dtc_class_type *dtc_class = dem_get_event_dtc_class(event_param);
	const dem_group_of_dtc_type *dtc_groups = NULL;
	uint32 dtc_group_lower = 0u;
	uint32 dtc_group_upper = 0u;

	if (NULL == dtc_class) {
		// 无相关dtc，结束
		return FALSE;
	}
	if (dem_is_dtc_available(dtc_class) == FALSE) {
		// 相关dtc被抑制，结束
		return FALSE;
	}
	dtc_groups = g_dem_config_set->group_of_dtc;
	dtc_group_lower = dtc_groups[group_id].dem_group_dtcs;
	dtc_group_upper = dtc_groups[group_id + 1u].dem_group_dtcs;

	if (dtc_group_upper > 0xFFFE00u) {
		dtc_group_upper = 0xFFFE00u; // sws_dem_01059 规定了group_id为[0x000100u, 0xFFFE00]范围
	}

	// dtc_group是uds的概念，除了0x000000清除all_obddtc和0xFFFFFF清除all_dtc，其他都只有udsdtc
	return (dtc_class->uds_dtc >= dtc_group_lower) && (dtc_class->uds_dtc < dtc_group_upper);
}

static FUNC(boolean, DEM_CODE)
is_dtc_severity_mask_match(dem_dtc_severity_type dtc_severity_mask, const dem_event_parameter_type *event_param)
{
	const dem_dtc_class_type *dtc_class = dem_get_event_dtc_class(event_param);

	return (NULL != dtc_class) &&
	       DEM_IS_MASK_SET(dtc_class->dtc_severity, dtc_severity_mask, dem_dtc_severity_type);
}

static FUNC(std_return_type, DEM_CODE)
clear_event_dtc(const dem_event_parameter_type *event_param, const dem_event_info_rec_type *event_status,
	  dem_dtc_origin_type dtc_origin, boolean delete_dtc_data)
{
	boolean data_deleted = FALSE;
	dem_event_status_rec_type old_status;
	dem_event_status_rec_type new_status;

	/* 0x14明确规定输入为dem_dtc_origin_primary_memory */
	if (event_param->event_class_ref->event_destination != dtc_origin) {
		return DEM_WRONG_DTCORIGIN;
	}

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	if (dem_is_combined_event(event_param->event_id) == TRUE) {
#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
		uint8 group_index = dem_get_event_combined_group_index(event_param->event_id);
		boolean clear_allowed = dem_event_combination_on_storage_clear_allowed(group_index);

		old_status = dem_get_event_combined_group_uds_status(group_index);

		if (TRUE == clear_allowed) {
			data_deleted = dem_clear_combined_event(group_index, TRUE, delete_dtc_data);
		}
		new_status = dem_get_event_combined_group_uds_status(group_index);
#endif
	} else {
		old_status = event_status->event_status_rec;
		if (dem_clear_event_allowed(event_param) == FALSE) {
			return DEM_CLEAR_FAILED;
		}
		if (TRUE == delete_dtc_data) {
			data_deleted = dem_delete_dtc_data(event_param);
		}
		dem_reset_event_info_rec(event_param);
		new_status = event_status->event_status_rec;
	}
#else
	{
		old_status = event_status->event_status_rec;
		if (dem_clear_event_allowed(event_param) == FALSE) {
			return DEM_CLEAR_FAILED;
		}
		if (TRUE == delete_dtc_data) {
			data_deleted = dem_delete_dtc_data(event_param);
		}
		dem_reset_event_info_rec(event_param);
		new_status = event_status->event_status_rec;
	}
#endif

#if defined(DEM_USE_INDICATORS)
	(void)dem_reset_indicator_counters(event_param);
#endif
	if (TRUE == data_deleted) {
		dem_notify_event_storage_changed(event_param);
	}

	if (old_status != new_status) {
		dem_notify_event_status_change(event_param, old_status, event_status->event_status_rec);
		dem_notify_dtc_status_changed(event_param, old_status, event_status->event_status_rec);
	}

#if (DEM_CALLBACK_INIT_M_FOR_E_SUPPORT == STD_ON)
	if (NULL != event_param->callback_init_mfor_e) {
		(void)event_param->callback_init_mfor_e(DEM_INIT_CLEAR_MONITOR);
	}
#endif

	return E_OK;
}

FUNC_P2CONST(dem_dtc_class_type, AUTOMATIC, DEM_CODE)
dem_get_event_dtc_class(const dem_event_parameter_type *event_param)
{
	const dem_dtc_class_type *dtc_class_list = NULL;

	if ((NULL == event_param) || (DEM_INVALID_U16_REF_INDEX == event_param->dtc_class_ref_index)) {
		return NULL;
	}

	dtc_class_list = dem_get_dtc_class_list();
	if (NULL == dtc_class_list) {
		return NULL;
	}

	return &(dtc_class_list[event_param->dtc_class_ref_index]);
}

FUNC(boolean, DEM_CODE) dem_is_dtc_available(const dem_dtc_class_type *dtc_class)
{
	uint16 dtc_index = 0u;

	if (NULL == dtc_class) {
		return FALSE;
	}

	dtc_index = dtc_class->dtc_index;

	if (dtc_index >= DEM_NOF_DTCS) {
		return FALSE;
	}

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
	if ((TRUE == g_dem_dtc_suppressed[dtc_index].suppressed_by_dtc) ||
	    (TRUE == g_dem_dtc_suppressed[dtc_index].suppressed_by_event)) {
		return FALSE;
	}
#endif

	return TRUE;
}

FUNC(boolean, DEM_CODE)
dem_is_dtc_format_match(const dem_event_parameter_type *event_param, dem_dtc_format_type dtc_format)
{
	boolean ret = FALSE;
	const dem_dtc_class_type *dtc_class = dem_get_event_dtc_class(event_param);

	if (NULL != dtc_class) {
		ret = (((DEM_DTC_FORMAT_UDS == dtc_format) && (DEM_NO_DTC != dtc_class->uds_dtc)) ||
		       ((DEM_DTC_FORMAT_OBD == dtc_format) && (DEM_NO_OBDDTC != dtc_class->obd_dtc)));
	}

	return ret;
}

FUNC(boolean, DEM_CODE)
dem_is_dtc_origin_match(dem_dtc_origin_type dtc_origin, const dem_event_parameter_type *event_param,
		 boolean allow_permanent_memory)
{
	boolean is_match = FALSE;

	if ((DEM_DTC_ORIGIN_PERMANENT_MEMORY == dtc_origin) && (TRUE == allow_permanent_memory)) {
#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
		if (FALSE == dem_is_emission_event_ref_to_mil(event_param)) {
			return FALSE;
		}

		for (uint8 i = 0; i < DEM_MAX_NUMBER_EVENT_PERM_MEM; i++) {
			if (dem_get_permanent_event(i) == event_param->event_id) {
				return TRUE;
			}
		}
#endif
	} else {
		is_match = (event_param->event_class_ref->event_destination == dtc_origin);
	}

	return is_match;
}

FUNC(void, DEM_CODE)
dem_notify_dtc_status_changed(const dem_event_parameter_type *event_param, dem_event_status_rec_type old_status,
			      dem_event_status_rec_type new_status)
{ // 当前不支持多event对单dtc，所以dtc状态和相关event状态同步改变
	const dem_dtc_class_type *dtc_class = dem_get_event_dtc_class(event_param);

	if (NULL == dtc_class) {
		return;
	}

	if (dem_is_dtc_available(dtc_class) == FALSE) {
		return;
	}

#if (dem_get_size_of_dtc_sta_chg_cbk_list() > 0)
	uint32 dtc = dtc_class->uds_dtc;

	// cbk是和client相关联的，但是sws上面说任意dtc有变化时会调用所有的cbd
	for (uint8 i = 0u; i < dem_get_size_of_dtc_sta_chg_cbk_list(); i++) {
		if (g_dem_cfg_helper->dtc_status_changed_cbk_map[i] == NULL) {
			continue;
		}
		(void)g_dem_cfg_helper->dtc_status_changed_cbk_map[i](dtc, old_status, new_status);
	}
#endif

	return;
}

FUNC(boolean, DEM_CODE) dem_lookup_event_of_uds_dtc(uint32 dtc, dem_event_info_rec_type **event_info_rec)
{
	boolean found = FALSE;
	uint16 i = 0;
	const dem_dtc_class_type *dtc_class = NULL;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;

	*event_info_rec = NULL;

	for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

		lookup_event_param(event_id, &event_param);
		dtc_class = dem_get_event_dtc_class(event_param);
		if (NULL == dtc_class) {
			continue;
		}

		if ((dtc_class->uds_dtc == dtc) && (dem_is_dtc_available(dtc_class))) {
			*event_info_rec = &g_dem_event_info_rec[i];
			found = TRUE;
			break;
		}
	}

	return found;
}

FUNC(boolean, DEM_CODE)
dem_is_dtc_match_filter(const dem_dtc_filter_type *dtc_filter, const dem_event_info_rec_type *event_info_rec)
{
	boolean dtc_match = (boolean)FALSE;
	const dem_dtc_class_type *dtc_class = NULL;
	const dem_event_parameter_type *event_param = NULL;

	if (NULL == dtc_filter) {
		return dtc_match;
	}

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	if (dem_is_master_event(event_info_rec->event_id) == FALSE) {
		return dtc_match;
	}
#endif

	if (is_dtc_status_mask_match(dtc_filter->dtc_status_mask, event_info_rec)) {
		lookup_event_param(event_info_rec->event_id, &event_param);
		if ((NULL != event_param) &&
		    (dem_is_dtc_origin_match(dtc_filter->dtc_origin, event_param, TRUE) == TRUE)) {
			if ((dtc_filter->filter_with_severity == (boolean)FALSE) ||
			    ((dtc_filter->filter_with_severity == (boolean)TRUE) &&
			     (is_dtc_severity_mask_match(dtc_filter->dtc_severity_mask, event_param)))) {
				if (dtc_filter->filter_for_fault_detection_counter == (boolean)FALSE) {
					dtc_class = dem_get_event_dtc_class(event_param);
					if ((NULL != dtc_class) && dem_is_dtc_available(dtc_class) &&
					    dem_is_dtc_format_match(event_param, dtc_filter->dtc_format)) {
						dtc_match = TRUE;
					}
				}
			}
		}
	}

	return dtc_match;
}

FUNC(boolean, DEM_CODE) dem_is_dtc_setting_disabled(const dem_event_parameter_type *event_param)
{
	const dem_dtc_class_type *dtc_class = NULL;

	if (FALSE == g_dem_disable_dtc_set.setting_disabled) {
		return FALSE;
	}

	dtc_class = dem_get_event_dtc_class(event_param);
	if (dem_is_dtc_available(dtc_class) == FALSE) {
		return FALSE;
	}

	return TRUE;
}

FUNC(std_return_type, DEM_CODE) dem_clear_dtc_process_single_dtc(const dem_dtc_selector_data_type *selector_ptr)
{
	std_return_type ret = DEM_WRONG_DTC;
	dem_dtc_origin_type dtc_origin = selector_ptr->request.dtc_origin;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}
		if (selector_ptr->selection.event_id != event_id) {
			continue;
		}

		lookup_event_param(event_id, &event_param);
		if (NULL == event_param) {
			DEM_DET_REPORT(DEM_DEM_CLEARDTC_ID_DEM_E_UNEXPECTED_EXECUTION);
			continue;
		}

		dtc_class = dem_get_event_dtc_class(event_param);
		if (NULL == dtc_class) {
			continue;
		}
		if (dem_is_dtc_available(dtc_class) == FALSE) {
			// 确认当前操作的dtc是否被抑制，被抑制报错结束
			DEM_DET_REPORT(DEM_DEM_CLEARDTC_ID_DEM_E_UNEXPECTED_EXECUTION);
			return DEM_CLEAR_FAILED;
		}
		// 清除
		ret = clear_event_dtc(event_param, &g_dem_event_info_rec[i], dtc_origin, TRUE);
		break;
	}

	return ret;
}

FUNC(std_return_type, DEM_CODE) dem_clear_dtc_process_group(const dem_dtc_selector_data_type *selector_ptr)
{
	std_return_type final_ret = E_OK;
	std_return_type ret = DEM_WRONG_DTC;
	dem_dtc_origin_type dtc_origin = selector_ptr->request.dtc_origin;
	dem_dtc_group_id_type group_id = selector_ptr->selection.group_id;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}
		lookup_event_param(event_id, &event_param);
		if (NULL == event_param) {
			DEM_DET_REPORT(DEM_DEM_CLEARDTC_ID_DEM_E_UNEXPECTED_EXECUTION);
			continue;
		}
		if (is_dtc_in_dtc_group(group_id, event_param) == FALSE) {
			// 确认当前event对应的dtc是否属于该dtc_group，不是则观察下一个
			continue;
		}
		// 清除
		ret = clear_event_dtc(event_param, &g_dem_event_info_rec[i], dtc_origin, TRUE);
		if (E_OK != ret) { // 任意失败都要返回失败
			final_ret = ret;
		}
	}

	return (E_OK != final_ret) ? final_ret : ret;
}

FUNC(std_return_type, DEM_CODE)
dem_clear_dtc_process_all_dtc(const dem_dtc_selector_data_type *selector_ptr)
{
	(void)selector_ptr;
	std_return_type final_ret = E_OK;
	std_return_type ret = DEM_WRONG_DTCORIGIN;
	dem_dtc_origin_type dtc_origin = DEM_DTC_ORIGIN_INVALID;
	const dem_event_info_rec_type *event_info = NULL;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}
		event_info = &g_dem_event_info_rec[i];

		lookup_event_param(event_id, &event_param);
		if (NULL == event_param) {
			DEM_DET_REPORT(DEM_DEM_CLEARDTC_ID_DEM_E_UNEXPECTED_EXECUTION);
			continue;
		}

		if (DEM_INVALID_U16_REF_INDEX == event_param->dtc_class_ref_index) {
			continue;
		}

		dtc_origin = event_param->event_class_ref->event_destination;
		ret = clear_event_dtc(event_param, event_info, dtc_origin, FALSE);
		if (E_OK != ret) { // 任意失败都要返回失败
			final_ret = ret;
		}
	}

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	(void)dem_delete_all_dtc_data_in_memory(DEM_DTC_ORIGIN_PRIMARY_MEMORY, g_dem_pri_mem_data,
						DEM_MAX_NUMBER_EVENT_PRI_MEM);
#endif

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
	dem_delete_all_obd_ff();
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
	if (E_OK == final_ret) { // 清除全部dtc完成后重置memory_overflow标志位
		dem_set_overflow_indication(dtc_origin, FALSE);
	}
#endif
#endif

	return (E_OK != final_ret) ? final_ret : ret;
}

FUNC(std_return_type, DEM_CODE) dem_clear_dtc_process_emission(const dem_dtc_selector_data_type *selector_ptr)
{
	(void)selector_ptr;
	std_return_type final_ret = E_OK;
	std_return_type ret = DEM_WRONG_DTCORIGIN;
	dem_dtc_origin_type dtc_origin;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}
		lookup_event_param(event_id, &event_param);
		dtc_class = dem_get_event_dtc_class(event_param);
		if (NULL == dtc_class) {
			DEM_DET_REPORT(DEM_DEM_CLEARDTC_ID_DEM_E_UNEXPECTED_EXECUTION);
			continue;
		}
		if (DEM_NO_OBDDTC == dtc_class->obd_dtc) {
			continue;
		}
		// 清除
		dtc_origin = event_param->event_class_ref->event_destination;
		ret = clear_event_dtc(event_param, &g_dem_event_info_rec[i], dtc_origin, TRUE);
		if (E_OK != ret) { // 任意失败都要返回失败
			final_ret = ret;
		}
	}

	return (E_OK != final_ret) ? final_ret : ret;
}

#if (DEM_CLEAR_DTC_NOTIFICATION_START == STD_ON)
FUNC(void, DEM_CODE)
dem_clear_dtc_notification_start(uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	const dem_config_set_type *config = dem_get_config_set();

	if (NULL != config->clear_dtc_notification_start_fnc) {
		for (uint8 i = 0; config->clear_dtc_notification_start_fnc[i] != NULL; i++) {
			(void)config->clear_dtc_notification_start_fnc[i](dtc, dtc_format, dtc_origin);
		}
	}
}
#endif

#if (DEM_CLEAR_DTC_NOTIFICATION_FINISH == STD_ON)
FUNC(void, DEM_CODE)
dem_clear_dtc_notification_finish(uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	const dem_config_set_type *config = dem_get_config_set();

	if (NULL != config->clear_dtc_notification_finish_fnc) {
		for (uint8 i = 0; config->clear_dtc_notification_finish_fnc[i] != NULL; i++) {
			(void)config->clear_dtc_notification_finish_fnc[i](dtc, dtc_format, dtc_origin);
		}
	}
}
#endif

FUNC(std_return_type, DEM_CODE)
dem_is_dtc_match_format(uint32 dtc, dem_dtc_format_type dtc_format, const dem_dtc_class_type *dtc_class_ptr)
{
	std_return_type ret = E_NOT_OK;

	if ((DEM_DTC_FORMAT_UDS == dtc_format) && (dtc == dtc_class_ptr->uds_dtc)) {
		ret = E_OK;
	} else if ((DEM_DTC_FORMAT_OBD == dtc_format) && (DEM_TO_OBD_FORMAT(dtc_class_ptr->obd_dtc) == dtc)) {
		ret = E_OK;
	} else {
		/* do nothing */
	}

	return ret;
}

FUNC(boolean, DEM_CODE) dem_get_dtc_setting_status(void)
{
	return g_dem_disable_dtc_set.setting_disabled;
}

FUNC(boolean, DEM_CODE) dem_delete_dtc_data(const dem_event_parameter_type *event_param)
{
	boolean data_deleted = FALSE;
	dem_primary_mem_type *event_buffer = NULL;
	uint32 event_buffer_size = 0u;
	dem_dtc_origin_type origin = DEM_DTC_ORIGIN_NOT_USED;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
	if (DEM_DTC_ORIGIN_PRIMARY_MEMORY == event_param->event_class_ref->event_destination) {
		event_buffer = g_dem_pri_mem_data;
		event_buffer_size = DEM_MAX_NUMBER_EVENT_PRI_MEM;
		origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
	}
#endif

	if (NULL == event_buffer) {
		DEM_DET_REPORT(DEM_DEM_CLEARDTC_ID_DEM_E_NOT_IMPLEMENTED_YET);
		return FALSE;
	}

	data_deleted = dem_delete_event_mem(event_param, event_buffer, event_buffer_size, origin);

	return data_deleted;
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"