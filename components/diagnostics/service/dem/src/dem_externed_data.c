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

static FUNC_P2VAR(const dem_ext_data_record_class_type *const, AUTOMATIC, DEM_CODE)
	get_event_extend_data_record_base(const dem_event_parameter_type *event_param)
{
	const dem_ext_data_class_type *extern_data_class_list = NULL;

	if ((NULL == event_param) || (DEM_INVALID_U16_REF_INDEX == event_param->ext_data_class_ref_index)) {
		return NULL;
	}

	extern_data_class_list = dem_get_ext_data_class_list();
	if (NULL == extern_data_class_list) {
		return NULL;
	}

	return extern_data_class_list[event_param->ext_data_class_ref_index].ext_data_record_class_ref;
}

#if (DEM_EXT_DATA_IN_PRI_MEM) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
static FUNC(boolean, DEM_CODE)
	get_ext_data_content(const dem_event_parameter_type *event_param,
			     const dem_ext_data_record_class_type *ext_data_record,
			     uint16 *store_index, uint8 *ext_data, boolean ext_data_stored)
{
	boolean b_stored_data = FALSE;
	uint8 data_elem_start = ext_data_record->data_class_ref_start;
	uint8 data_elem_end = ext_data_record->data_class_ref_start + ext_data_record->data_class_num;

	if ((DEM_UPDATE_RECORD_YES == ext_data_record->update_rule) ||
	    ((DEM_UPDATE_RECORD_NO == ext_data_record->update_rule) || !ext_data_stored)) {
		for (uint8 i = data_elem_start; i < data_elem_end; i++) {
			uint8 mapped_idx = g_dem_cfg_helper->ext_data_data_elem_map[i];
			const dem_data_elem_class_type *data_elem_ref = &g_dem_config_set->data_elem[mapped_idx];

			if (NULL != data_elem_ref->data_elem_read_fnc) {
				if (data_elem_ref->data_elem_read_fnc(&ext_data[*store_index]) != E_OK) {
					(void)memset(&ext_data[*store_index], 0xFF, data_elem_ref->data_size);
				}
			} else if (DEM_NO_ELEMENT != data_elem_ref->internal_data_type) {
				dem_get_internal_element(event_param, data_elem_ref->internal_data_type,
							 &ext_data[*store_index], data_elem_ref->data_size);
			} else {
				/* do nothing */
			}
			*store_index += data_elem_ref->data_size;
		}
		b_stored_data = TRUE;
	}

	return b_stored_data;
}
#endif

FUNC(void, DEM_CODE)
dem_get_internal_element(const dem_event_parameter_type *event_param, dem_internal_data_elem_type element_type,
			 uint8 *buf, uint16 size)
{
	dem_event_info_rec_type *event_info_rec = NULL;

	if ((NULL == buf) || (0u == size)) {
		return;
	}
	(void)memset(buf, 0, size);

	lookup_event_info_rec(event_param->event_id, &event_info_rec);

	if ((NULL == event_info_rec) || (DEM_EVENT_ID_NULL == event_info_rec->event_id)) {
		return;
	}

	switch (element_type) {
#if (DEM_SUPPORT_CURRENT_FDC == STD_ON)
		case DEM_CURRENT_FDC:
			buf[size - 1u] = (uint8)event_info_rec->uds_fdc;
			break;
#endif
#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON)
		case DEM_MAX_FDC_DURING_CURRENT_CYCLE:
			buf[size - 1u] = (uint8)event_info_rec->max_uds_fdc;
			break;
#endif
#if (DEM_CFG_DATA_OCCCTR == STD_ON)
		case DEM_OCCCTR:
			if (1u == size) {
				buf[0] = (uint8)MIN(event_info_rec->occctr, 0xFFu);
			} else {
				buf[size - 2u] = (uint8)((event_info_rec->occctr & 0xFF00u) >> 8u);
				buf[size - 1u] = (uint8)(event_info_rec->occctr & 0x00FFu);
			}
			break;
#endif
#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
		case DEM_OVFLIND: {
			boolean ovflw = FALSE;

			if (E_OK == dem_get_overflow_indication(event_param->event_class_ref
								->event_destination, &ovflw)) {
				buf[size - 1u] = (uint8)(ovflw ? 1 : 0);
			} else {
				buf[size - 1u] = 0;
			}
		} break;
#endif
#if (DEM_SUPPORT_AGING == STD_ON)
		case DEM_AGINGCTR:
			if (event_param->event_class_ref->aging_allowed) {
				buf[size - 1u] = event_info_rec->aging_cnt;
			}
			break;
#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
		case DEM_AGED_COUNTER:
			if (event_param->event_class_ref->aging_allowed) {
				buf[size - 1u] = event_info_rec->aged_cnt;
			}
			break;
#endif
#endif
#if (DEM_CFG_DATA_CYCLES_SINCE_FIRST_FAILED == STD_ON)
		case DEM_CYCLES_SINCE_FIRST_FAILED:
			buf[size - 1u] = event_info_rec->cycles_since_first_failed;
			break;
#endif
#if (DEM_CFG_DATA_CYCLES_SINCE_LAST_FAILED == STD_ON)
		case DEM_CYCLES_SINCE_LAST_FAILED:
			buf[size - 1u] = event_info_rec->cycles_since_last_failed;
			break;
#endif
#if (DEM_CFG_DATA_FAILED_CYCLES == STD_ON)
		case DEM_FAILED_CYCLES:
			buf[size - 1u] = event_info_rec->failed_cycles;
			break;
#endif
		case DEM_FAULT_PENDING_COUNTER:
			buf[size - 1u] = event_info_rec->failure_counter;
			break;
		default:
			break;
	}
}

FUNC(uint16, DEM_CODE)
dem_get_ext_data_record_data_size(const dem_ext_data_record_class_type *ext_data_record_class)
{
	uint8 ed_data_size = 0;
	uint8 data_elem_start = ext_data_record_class->data_class_ref_start;
	uint8 data_elem_end = ext_data_record_class->data_class_ref_start + ext_data_record_class->data_class_num;

	for (uint8 i = data_elem_start; i < data_elem_end; i++) {
		uint8 mapped_idx = g_dem_cfg_helper->ext_data_data_elem_map[i];
		const dem_data_elem_class_type *data_elem_ref = &g_dem_config_set->data_elem[mapped_idx];

		ed_data_size += data_elem_ref->data_size;
	}

	return ed_data_size;
}

#if (DEM_EXT_DATA_IN_PRI_MEM) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(boolean, DEM_CODE)
dem_get_ext_data_content(const dem_event_parameter_type *event_param, boolean ext_data_stored,
			 dem_primary_mem_type *event_rec)
{
	boolean b_stored_data = FALSE;
	uint8 *ext_data_cur_pos = NULL;
	const dem_ext_data_record_class_type *ext_data_record = NULL;

	const dem_ext_data_record_class_type *const *extern_data_record_base = NULL;

	extern_data_record_base = get_event_extend_data_record_base(event_param);
	if (NULL == extern_data_record_base) {
		return b_stored_data;
	}

#if (DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA > 0)
	for (uint16 i = 0u; i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA; i++) {
		uint16 store_index = 0;
		uint16 record_size = 0;

		ext_data_cur_pos = &event_rec->ext_data[i][0];
		ext_data_record = extern_data_record_base[i];

		if (NULL == ext_data_record) {
			return b_stored_data;
		}

		record_size = dem_get_ext_data_record_data_size(ext_data_record);
		if ((store_index + record_size) > DEM_MAX_SIZE_EXT_DATA) {
			DEM_DET_REPORT(DEM_DEM_GET_EXTENDED_DATA_ID_DEM_E_EXT_DATA_TOO_BIG);
			return b_stored_data; // 即使超出，前面部分复制成功的话，也可以返回true
		}
		b_stored_data = get_ext_data_content(event_param, ext_data_record, &store_index,
									     ext_data_cur_pos, ext_data_stored);
	}
#else
	(void)ext_data_stored;
	(void)event_rec;
	(void)ext_data_cur_pos;
#endif

	return b_stored_data;
}
#endif

FUNC(boolean, DEM_CODE)
dem_lookup_ext_data_recnum_param(uint8 ext_data_number, const dem_event_parameter_type *event_param,
				 const dem_ext_data_record_class_type **ext_data_rec_class_ptr)
{
	boolean recnum_found = FALSE;
	const dem_ext_data_record_class_type *const *extern_data_record_base = NULL;
	const dem_ext_data_record_class_type *ext_data_record = NULL;

	extern_data_record_base = get_event_extend_data_record_base(event_param);
	if (NULL == extern_data_record_base) {
		return recnum_found;
	}
#if (DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA > 0)
	for (uint32 i = 0u; i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA; i++) {
		ext_data_record = extern_data_record_base[i];

		if (NULL == ext_data_record) {
			continue;
		}
		if (ext_data_record->recnum == ext_data_number) {
			*ext_data_rec_class_ptr = ext_data_record;
			recnum_found = TRUE;
			break;
		}
	}
#else
	(void)ext_data_number;
	(void)ext_data_rec_class_ptr;
	(void)ext_data_record;
#endif

	return recnum_found;
}

#if (DEM_EXT_DATA_IN_PRI_MEM)
FUNC(uint8, DEM_CODE)
dem_lookup_ext_data_record_index(const dem_primary_mem_type *event_rec, uint8 recnum)
{
	uint8 ext_data_record_index = DEM_MAX_RECORD_NUMBERS_IN_EXD_REC_NUM_CLASS;
	const dem_event_parameter_type *event_param = NULL;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_ext_data_record_class_type *const *extern_data_record_base = NULL;
	const dem_ext_data_record_class_type *ext_data_record = NULL;

	lookup_event_param(event_rec->event_id, &event_param);
	lookup_event_info_rec(event_rec->event_id, &event_info_rec);
	if ((NULL == event_param) || (NULL == event_info_rec) || (FALSE == event_info_rec->ext_data_stored)) {
		return ext_data_record_index;
	}

	extern_data_record_base = get_event_extend_data_record_base(event_param);
	if (NULL == extern_data_record_base) {
		return ext_data_record_index;
	}

#if (DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA > 0)
	for (uint8 i = 0u; i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA; i++) {
		ext_data_record = extern_data_record_base[i];
		if (NULL == ext_data_record) {
			continue;
		}
		if (ext_data_record->recnum == recnum) {
			ext_data_record_index = i;
			break;
		}
	}
#else
	(void)recnum;
#endif

	return ext_data_record_index;
}
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"