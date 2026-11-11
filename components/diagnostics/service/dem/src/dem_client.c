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

/***********************************************************************************************************************
 *  dem_selector_init_all_dtc()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE) dem_selector_init_all_dtc(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
						      dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);

/***********************************************************************************************************************
 *  dem_selector_init_emission_dtc()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_selector_init_emission_dtc(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
				       dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);

/***********************************************************************************************************************
 *  dem_selector_init_group()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_selector_init_group(dem_dtc_selector_data_type *selector_ptr, uint32 dtc, dem_dtc_format_type dtc_format,
				dem_dtc_origin_type dtc_origin, dem_dtc_group_id_type group_id);

/***********************************************************************************************************************
 *  dem_selector_init_event_id()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_selector_init_event_id(dem_dtc_selector_data_type *selector_ptr, uint32 dtc, dem_dtc_format_type dtc_format,
				   dem_dtc_origin_type dtc_origin, dem_event_id_type event_id);

/***********************************************************************************************************************
 *  dem_selector_init_invalid()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE) dem_selector_init_invalid(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
						      dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);

/***********************************************************************************************************************
 *  dem_get_number_of_clients()
 **********************************************************************************************************************/
static FUNC(uint8, DEM_CODE) dem_get_number_of_clients(void);

/***********************************************************************************************************************
 *  dem_get_selector_by_client_id()
 **********************************************************************************************************************/
static FUNC(dem_dtc_selector_data_type *, DEM_CODE) dem_get_selector_by_client_id(uint8 client_id);

/***********************************************************************************************************************
 *  dem_set_selector()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE) dem_set_selector(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
					     dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);

/***********************************************************************************************************************
 *  dem_is_all_dtc_selected()
 **********************************************************************************************************************/
static FUNC(boolean, DEM_CODE) dem_is_all_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format);

/***********************************************************************************************************************
 *  dem_is_emission_dtc_selected()
 **********************************************************************************************************************/
static FUNC(boolean, DEM_CODE) dem_is_emission_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format);

/***********************************************************************************************************************
 *  dem_is_group_dtc_selected()
 **********************************************************************************************************************/
static FUNC(dem_dtc_group_id_type, DEM_CODE)
	dem_is_group_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);

/***********************************************************************************************************************
 *  dem_is_single_dtc_selected()
 **********************************************************************************************************************/
static FUNC(dem_event_id_type, DEM_CODE) dem_is_single_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format);

/***********************************************************************************************************************
 *  dem_set_selector_selection_data()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_set_selector_selection_data(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
					dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin,
					dem_dtc_selector_selection_type dtc_selection_type,
					dem_dtc_selector_result_type dtc_selection_result);

/***********************************************************************************************************************
 *  dem_get_number_of_clients()
 **********************************************************************************************************************/
static FUNC(uint8, DEM_CODE) dem_get_number_of_clients(void)
{
	uint8 cnum = dem_get_size_of_client_id_list();

	return cnum;
}

/***********************************************************************************************************************
 *  dem_set_selector()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE) dem_set_selector(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
					     dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	boolean is_all_dtc = FALSE;
	boolean is_emssion_dtc = FALSE;
	dem_dtc_group_id_type group_id = 0u;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;

	is_all_dtc = dem_is_all_dtc_selected(dtc, dtc_format);
	if (is_all_dtc == (boolean)TRUE) { // 按照dtc_group处理，但是是alldtc，结束
		dem_selector_init_all_dtc(selector_ptr, dtc, dtc_format, dtc_origin);
		return;
	}

	is_emssion_dtc = dem_is_emission_dtc_selected(dtc, dtc_format);
	if (is_emssion_dtc == (boolean)TRUE) { // 按照dtc_group处理，但是是emission_dtc，结束
		dem_selector_init_emission_dtc(selector_ptr, dtc, dtc_format, dtc_origin);
		return;
	}

	// 如果group存在，则返回对应的group_id（group按照dtc号码从小到大排列）
	group_id = dem_is_group_dtc_selected(dtc, dtc_format, dtc_origin);
	if (group_id != 0xFFu) { // 按照dtc_group处理，结束
		dem_selector_init_group(selector_ptr, dtc, dtc_format, dtc_origin,
					group_id); // 按照group_id把selector置为选择了group
		return;
	}

	event_id = dem_is_single_dtc_selected(dtc, dtc_format); // selector置为
	if (DEM_EVENT_ID_NULL != event_id) { // 按照单个dtc处理，结束
		dem_selector_init_event_id(selector_ptr, dtc, dtc_format, dtc_origin, event_id);
		return;
	}

	dem_selector_init_invalid(selector_ptr, dtc, dtc_format, dtc_origin); // 没有成功初始化selector

	return;
}

/***********************************************************************************************************************
 *  dem_is_all_dtc_selected()
 **********************************************************************************************************************/
static FUNC(boolean, DEM_CODE) dem_is_all_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format)
{
	boolean is_all_dtc = (boolean)FALSE;

	if (DEM_DTC_FORMAT_UDS == dtc_format) {
		if (dtc == DEM_DTC_GROUP_ALL_DTCS) {
			is_all_dtc = (boolean)TRUE; // 确认是所有的dtc被选中
		}
	}

	return is_all_dtc;
}

/***********************************************************************************************************************
 *  dem_is_emission_dtc_selected()
 **********************************************************************************************************************/
static FUNC(boolean, DEM_CODE) dem_is_emission_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format)
{
	boolean is_emission_dtc = (boolean)FALSE;

	if (DEM_DTC_FORMAT_OBD == dtc_format) {
		if (dtc == DEM_EMISSION_GROUP_REL_DTCS) {
			is_emission_dtc = (boolean)TRUE; // 确认是所有排放相关dtc被选中
		}
	}

	return is_emission_dtc;
}

/***********************************************************************************************************************
 *  dem_is_group_dtc_selected()
 **********************************************************************************************************************/
static FUNC(dem_dtc_group_id_type, DEM_CODE)
	dem_is_group_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	(void)dtc_origin;
	const dem_config_set_type *config = dem_get_config_set();
	dem_dtc_group_id_type group_id = DEM_UNKNOWN_GROUP_ID;

	if (DEM_DTC_FORMAT_UDS != dtc_format) { // 只有uds有dtc_group
		return DEM_UNKNOWN_GROUP_ID;
	}

	for (uint8 i = 0; config->group_of_dtc[i].dem_eol == (boolean)FALSE; ++i) {
		if (dtc == config->group_of_dtc[i].dem_group_dtcs) {
			group_id = (dem_dtc_group_id_type)i;
			break;
		}
	}

	return group_id;
}

/***********************************************************************************************************************
 *  dem_is_single_dtc_selected()
 **********************************************************************************************************************/
static FUNC(dem_event_id_type, DEM_CODE) dem_is_single_dtc_selected(uint32 dtc, dem_dtc_format_type dtc_format)
{
	const dem_event_parameter_type *event_param = NULL;
	const dem_dtc_class_type *dtc_class = NULL;
	const dem_config_set_type *config_set = dem_get_config_set();
	const dem_event_parameter_type *event_parameter_list = config_set->event_parameter;
	dem_event_id_type single_event_id = DEM_EVENT_ID_NULL;

	if (NULL == event_parameter_list) {
		return single_event_id;
	}

	for (uint16 i = 0u; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_param = &event_parameter_list[i];
		dtc_class = dem_get_event_dtc_class(event_param);
		if (NULL == dtc_class) {
			continue;
		}

		if (((DEM_DTC_FORMAT_UDS == dtc_format) && (dtc == dtc_class->uds_dtc)) ||
		    ((DEM_DTC_FORMAT_OBD == dtc_format) && (dtc == DEM_TO_OBD_FORMAT(dtc_class->obd_dtc)))) {
			single_event_id = event_param->event_id;
			break;
		}
	}

	return single_event_id;
}

/***********************************************************************************************************************
 *  dem_selector_init_all_dtc()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE) dem_selector_init_all_dtc(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
						      dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	dem_dtc_selector_selection_type dtc_selector;
	dem_dtc_selector_result_type dtc_selection_result;

	dtc_selector.group_id = DEM_UNKNOWN_GROUP_ID; // 既然是给alldtc，这里直接写DEM_UNKNOWN_GROUP_ID
	dtc_selection_result = DEM_SELECTED_ALL_DTCS;
	dem_set_selector_selection_data(selector_ptr, dtc, dtc_format, dtc_origin, dtc_selector, dtc_selection_result);

	return;
}

/***********************************************************************************************************************
 *  dem_selector_init_emission_dtc()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_selector_init_emission_dtc(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
				       dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	dem_dtc_selector_selection_type dtc_selector;
	dem_dtc_selector_result_type dtc_selection_result;

	dtc_selector.group_id = DEM_UNKNOWN_GROUP_ID;
	dtc_selection_result =  DEM_SELECTED_SELECTION_EMISSION_DTC;
	dem_set_selector_selection_data(selector_ptr, dtc, dtc_format, dtc_origin, dtc_selector, dtc_selection_result);

	return;
}

/***********************************************************************************************************************
 *  dem_selector_init_group()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_selector_init_group(dem_dtc_selector_data_type *selector_ptr, uint32 dtc, dem_dtc_format_type dtc_format,
				dem_dtc_origin_type dtc_origin, dem_dtc_group_id_type group_id)
{
	dem_dtc_selector_selection_type dtc_selector;
	dem_dtc_selector_result_type dtc_selection_result;

	dtc_selector.group_id = group_id;
	dtc_selection_result =  DEM_SELECTED_SELECTION_GROUP_DTC;
	dem_set_selector_selection_data(selector_ptr, dtc, dtc_format, dtc_origin, dtc_selector, dtc_selection_result);

	return;
}

/***********************************************************************************************************************
 *  dem_selector_init_event_id()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_selector_init_event_id(dem_dtc_selector_data_type *selector_ptr, uint32 dtc, dem_dtc_format_type dtc_format,
				   dem_dtc_origin_type dtc_origin, dem_event_id_type event_id)
{
	dem_dtc_selector_selection_type dtc_selector;
	dem_dtc_selector_result_type dtc_selection_result;

	dtc_selector.event_id = event_id;
	dtc_selection_result = DEM_SELECTED_SELECTION_DTC;
	dem_set_selector_selection_data(selector_ptr, dtc, dtc_format, dtc_origin, dtc_selector, dtc_selection_result);

	return;
}

/***********************************************************************************************************************
 *  dem_selector_init_invalid()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE) dem_selector_init_invalid(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
						      dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	dem_dtc_selector_selection_type dtc_selector;

	dtc_selector.event_id = DEM_EVENT_ID_NULL;

	dem_set_selector_selection_data(selector_ptr, dtc, dtc_format, dtc_origin, dtc_selector,
					 DEM_SELECTED_SELECTION_NO_MATCH_DTC);

	return;
}

/***********************************************************************************************************************
 *  dem_set_selector_selection_data()
 **********************************************************************************************************************/
static FUNC(void, DEM_CODE)
	dem_set_selector_selection_data(dem_dtc_selector_data_type *selector_ptr, uint32 dtc,
					dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin,
					dem_dtc_selector_selection_type dtc_selection_type,
					dem_dtc_selector_result_type dtc_selection_result)
{
	selector_ptr->request.dtc = dtc;
	selector_ptr->request.dtc_format = dtc_format;
	selector_ptr->request.dtc_origin = dtc_origin;
	selector_ptr->selection = dtc_selection_type;
	selector_ptr->selector_state = dtc_selection_result;
	selector_ptr->dtc_selector_is_set = TRUE;

	return;
}

/***********************************************************************************************************************
 *  dem_get_selector_by_client_id()
 **********************************************************************************************************************/
static FUNC_P2VAR(dem_dtc_selector_data_type, AUTOMATIC, DEM_CODE) dem_get_selector_by_client_id(uint8 client_id)
{
	dem_dtc_selector_data_type *selector_ptr = dem_get_client_selector_list();
	const dem_general_type *general = dem_get_general_ptr();

	for (uint8 i = 0u; i < dem_get_number_of_clients(); ++i) {
		if (general->client[i].client_id == client_id) {
			return &selector_ptr[i];
		}
	}

	return NULL;
}

/***********************************************************************************************************************
 *  dem_get_filter_by_client_id()
 **********************************************************************************************************************/
FUNC_P2VAR(dem_dtc_filter_type, AUTOMATIC, DEM_CODE) dem_get_filter_by_client_id(uint8 client_id)
{
	dem_dtc_filter_type *filter_ptr = dem_get_client_filter_list();
	const dem_general_type *general = dem_get_general_ptr();

	for (uint8 i = 0u; i < dem_get_number_of_clients(); ++i) {
		if (general->client[i].client_id == client_id) {
			return &filter_ptr[i];
		}
	}

	return NULL;
}

/***********************************************************************************************************************
 *  dem_get_ff_record_filter_by_client_id()
 **********************************************************************************************************************/
FUNC_P2VAR(dem_ff_record_filter_type, AUTOMATIC, DEM_CODE)
dem_get_ff_record_filter_by_client_id(uint8 client_id)
{
	dem_ff_record_filter_type *ff_record_filter = dem_get_client_ff_record_filter_list();
	const dem_general_type *general = dem_get_general_ptr();

	for (uint8 i = 0u; i < dem_get_number_of_clients(); ++i) {
		if (general->client[i].client_id == client_id) {
			return &ff_record_filter[i];
		}
	}

	return NULL;
}

/***********************************************************************************************************************
 *  dem_client_id_select_dtc()
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_client_id_select_dtc(uint8 client_id, uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin)
{
	dem_dtc_selector_data_type *selector_ptr = dem_get_selector_by_client_id(client_id);

	if (NULL == selector_ptr) { // 空指针，没有找到对应的selector
		return;
	}
	dem_set_selector(selector_ptr, dtc, dtc_format, dtc_origin); // 找到selector

	return;
}

/***********************************************************************************************************************
 *  dem_client_id_get_selection_result()
 **********************************************************************************************************************/
FUNC_P2VAR(dem_dtc_selector_data_type, AUTOMATIC, DEM_CODE) dem_client_id_get_selection_result(uint8 client_id)
{
	return dem_get_selector_by_client_id(client_id);
}

/***********************************************************************************************************************
 *  dem_client_id_set_filter()
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_client_id_set_filter(uint8 client_id, dem_event_status_rec_type dtc_status_mask, dem_dtc_format_type dtc_format,
			 dem_dtc_origin_type dtc_origin, boolean filter_with_severity,
			 dem_dtc_severity_type dtc_severity_mask, boolean filter_for_fault_detection_counter)
{
	dem_dtc_filter_type *filter_ptr = dem_get_filter_by_client_id(client_id);

	filter_ptr->dtc_status_mask = dtc_status_mask;
	filter_ptr->dtc_origin = dtc_origin;
	filter_ptr->filter_with_severity = filter_with_severity;
	filter_ptr->dtc_severity_mask = dtc_severity_mask;
	filter_ptr->filter_for_fault_detection_counter = filter_for_fault_detection_counter;
	filter_ptr->fault_index = DEM_MAX_NUMBER_EVENT;
	filter_ptr->dtc_format = dtc_format;
	filter_ptr->dtc_filter_is_set = TRUE;

	return;
}

/***********************************************************************************************************************
 *  dem_init_filter()
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_init_filter(void)
{
	uint8 size_of_list = dem_get_size_of_filter_list();
	dem_dtc_filter_type *filter_ptr = dem_get_client_filter_list();

	for (uint8 i = 0; i < size_of_list; i++) {
		filter_ptr[i].dtc_status_mask = DEM_DTC_STATUS_MASK_ALL;
		filter_ptr[i].dtc_origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
		filter_ptr[i].filter_with_severity = (boolean)FALSE;
		filter_ptr[i].dtc_severity_mask = DEM_SEVERITY_NO_SEVERITY;
		filter_ptr[i].filter_for_fault_detection_counter = (boolean)FALSE;
		filter_ptr[i].fault_index = DEM_MAX_NUMBER_EVENT;
	}

	return;
}

/***********************************************************************************************************************
 *  dem_init_ff_record_filter()
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_init_ff_record_filter(void)
{
	uint8 size_of_list = dem_get_size_of_ff_record_filter_list();
	dem_ff_record_filter_type *ff_record_filter = dem_get_client_ff_record_filter_list();

	for (uint8 i = 0; i < size_of_list; i++) {
		ff_record_filter[i].event_index = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
		ff_record_filter[i].ff_index = DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS;
		ff_record_filter[i].dtc_format = 0xff;
	}

	return;
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"
