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

#if (DEM_FF_DATA_IN_PRI_MEM)
static FUNC(std_return_type, DEM_CODE)
	get_non_obd_ff_recnum(const dem_event_parameter_type *event_param, uint8 *nof_stored)
{
	dem_event_info_rec_type *event_info_rec = NULL;

	lookup_event_info_rec(event_param->event_id, &event_info_rec);
	if ((NULL == event_info_rec)) {
		return E_NOT_OK;
	}
	*nof_stored = event_info_rec->stored_ff_recnum;

	return E_OK;
}
#endif

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
static FUNC(std_return_type, DEM_CODE)
	get_next_ff_recnum(const dem_event_parameter_type *event_param, uint8 *recnum, uint8 nof_stored)
{
	std_return_type ret = E_OK;
	const dem_ff_recnum_class_type *ff_recnum_class = dem_get_event_ff_recnum_class(event_param);

	if (nof_stored < event_param->max_number_ff_records) {
		if (DEM_FF_RECNUM_CALCULATED == g_dem_general_set->ff_recnum_type) {
			*recnum = nof_stored + 1u;
		} else if (NULL != ff_recnum_class) {
			*recnum = ff_recnum_class->ff_recnum[nof_stored];
		} else {
			ret = E_NOT_OK;
		}
	} else {
		if (event_param->max_number_ff_records > 1u) {
			if (DEM_FF_RECNUM_CALCULATED == g_dem_general_set->ff_recnum_type) {
				*recnum = event_param->max_number_ff_records;
			} else if (NULL != ff_recnum_class) {
				*recnum = ff_recnum_class->ff_recnum[event_param->max_number_ff_records - 1u];
			} else {
				ret = E_NOT_OK;
			}
		} else {
			ret = E_NOT_OK;
		}
	}

	return ret;
}
#endif

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
static FUNC(void, DEM_CODE) set_obd_ff_visibility(uint8 ff_index, boolean visible)
{
	g_dem_obd_ff_mem_data[ff_index].is_visible = visible;
}

#if (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON)
static FUNC(uint8, DEM_CODE) get_obd_ff_displace_index(dem_event_id_type event_id)
{
	/* 替换算法后续实现，暂不支持 */
	(void)event_id;

	return DEM_CFG_GLOBAL_FREEZEFRAME_SIZE;
}
#endif

static FUNC(uint8, DEM_CODE) get_obd_ff_index(dem_event_id_type event_id)
{
	dem_event_id_type ff_event_id;
	uint8 ff_index = DEM_CFG_GLOBAL_FREEZEFRAME_SIZE;
	int i; /* 使用有符号类型 */

	/* 从最后一个索引开始向前遍历 */
	for (i = (int)DEM_CFG_GLOBAL_FREEZEFRAME_SIZE - 1; i >= 0; i--) {
		ff_event_id = dem_get_obd_ff_event_id((uint8)i);

		if (event_id == ff_event_id) {
			ff_index = (uint8)i;
			break;
		} else if (DEM_EVENT_ID_NULL == ff_event_id) {
			ff_index = (uint8)i;
		} else {
			/* do nothing*/
		}
	}

#if (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON)
	/* 法规里固定当有失火相关的故障发生时，需要替换obd冻结帧*/
	if (DEM_CFG_GLOBAL_FREEZEFRAME_SIZE == ff_index) {
		ff_index = get_obd_ff_displace_index(event_id);
	}
#endif

	return ff_index;
}

static FUNC(void, DEM_CODE) set_obd_ff_data(const dem_event_parameter_type *event_param, uint8 ff_index)
{
	uint16 store_index = 0;
	uint8 *obd_ff_data = NULL;
	const dem_ff_class_type *ff_local = NULL;
	const dem_config_set_type *dem_config_set = dem_get_config_set();

	ff_local = dem_config_set->global_obd_ff_class_ref;

	obd_ff_data = dem_get_obd_ff_data(ff_index);
	(void)memset(obd_ff_data, 0, DEM_CFG_MAX_SIZE_OBDFREEZEFRAME);

	if (NULL != ff_local) {
		if (NULL != ff_local->ff_id_class_ref) {
			dem_get_pid_data_for_ff(ff_local->ff_id_class_ref, obd_ff_data, &store_index, event_param);
		}
	} else {
		dem_set_obd_ff_event_id(ff_index, DEM_EVENT_ID_NULL);
	}

	if (store_index != 0u) {
		set_obd_ff_visibility(ff_index, TRUE);
	}
}

static FUNC(void, DEM_CODE)
	store_obd_ff(const dem_event_parameter_type *event_param, uint8 ff_index)
{
	dem_event_id_type event_id = event_param->event_id;

	dem_set_obd_ff_event_id(ff_index, event_id);
	set_obd_ff_data(event_param, ff_index);

#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_obd_ff_block_changed(TRUE);
#else
	dem_nvm_set_obd_ff_block_changed(FALSE);
#endif
}
#endif

FUNC_P2CONST(dem_ff_class_type, AUTOMATIC, DEM_CODE)
dem_get_event_ff_class(const dem_event_parameter_type *event_param)
{
	const dem_ff_class_type *ff_class_list = NULL;

	if ((NULL == event_param) || (DEM_INVALID_U16_REF_INDEX == event_param->ff_class_ref_index)) {
		return NULL;
	}

	ff_class_list = dem_get_ff_class_list();
	if (NULL == ff_class_list) {
		return NULL;
	}

	return &(ff_class_list[event_param->ff_class_ref_index]);
}

FUNC_P2CONST(dem_ff_recnum_class_type, AUTOMATIC, DEM_CODE)
dem_get_event_ff_recnum_class(const dem_event_parameter_type *event_param)
{
	const dem_ff_recnum_class_type *ff_recnum_class_list = NULL;

	if ((NULL == event_param) || (DEM_INVALID_U16_REF_INDEX == event_param->ff_recnum_class_ref_index)) {
		return NULL;
	}

	ff_recnum_class_list = dem_get_ff_recnum_class_list();
	if (NULL == ff_recnum_class_list) {
		return NULL;
	}

	return &(ff_recnum_class_list[event_param->ff_recnum_class_ref_index]);
}

FUNC(uint16, DEM_CODE) dem_get_did_data_size(const dem_pid_or_did_type *const did_class)
{
	uint8 did_data_size = 0;
	uint8 data_elem_start = did_class->data_class_ref_start;
	uint8 data_elem_end = did_class->data_class_ref_start + did_class->data_class_num;

	for (uint8 i = data_elem_start; i < data_elem_end; i++) {
		uint8 mapped_idx = g_dem_cfg_helper->did_data_elem_map[i];
		const dem_data_elem_class_type *data_elem_ref = &g_dem_config_set->data_elem[mapped_idx];

		did_data_size += data_elem_ref->data_size;
	}

	return did_data_size;
}

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
FUNC(void, DEM_CODE)
dem_get_did_data_for_ff(const dem_pid_or_did_type *const *did_class_ptr, uint8 *data_ptr, uint16 *store_index_ptr,
			const dem_event_parameter_type *event_param)
{
	const dem_pid_or_did_type *const *ff_id_class_ref;
	uint16 store_index = 0;
	uint16 record_size = 0;

	ff_id_class_ref = did_class_ptr;

	for (uint16 i = 0; ((i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && (!ff_id_class_ref[i]->dem_eol)); i++) {
		record_size = dem_get_did_data_size(ff_id_class_ref[i]);
		if ((store_index + record_size) > DEM_MAX_SIZE_FF_DATA) {
			DEM_DET_REPORT(DEM_DEM_GET_FREEZEFRAME_ID_DEM_E_FF_TOO_BIG);
			break;
		}

		uint8 data_elem_start = ff_id_class_ref[i]->data_class_ref_start;
		uint8 data_elem_end = ff_id_class_ref[i]->data_class_ref_start + ff_id_class_ref[i]->data_class_num;

		for (uint8 data_elem_idx = data_elem_start; data_elem_idx < data_elem_end; data_elem_idx++) {
			uint8 mapped_idx = g_dem_cfg_helper->did_data_elem_map[data_elem_idx];
			const dem_data_elem_class_type *data_elem_ref = &g_dem_config_set->data_elem[mapped_idx];

			if (NULL != data_elem_ref->data_elem_read_fnc) {
				(void)data_elem_ref->data_elem_read_fnc(&(data_ptr[store_index]));
			} else if (DEM_NO_ELEMENT != data_elem_ref->internal_data_type) {
				dem_get_internal_element(event_param, data_elem_ref->internal_data_type,
							 &(data_ptr[store_index]), data_elem_ref->data_size);
			} else {
				(void)memset(&(data_ptr[store_index]), DEM_FREEZEFRAME_DEFAULT_VALUE,
					     data_elem_ref->data_size);
				DEM_DET_REPORT(DEM_DEM_GET_FREEZEFRAME_ID_DEM_E_NODATAAVAILABLE);
			}
			store_index += data_elem_ref->data_size;
		}
	}

	*store_index_ptr = store_index;
}

FUNC(std_return_type, DEM_CODE)
dem_get_next_ff_recnum(const dem_event_parameter_type *event_param, uint8 *recnum, dem_ff_kind_type ff_kind,
		   dem_dtc_origin_type origin)
{
	std_return_type ret = E_OK;
	uint8 nof_stored = 0;
	(void)origin;

	if (DEM_FREEZE_FRAME_NON_OBD == ff_kind) {
		if (event_param->max_number_ff_records != 0u) {
			if (get_non_obd_ff_recnum(event_param, &nof_stored) == E_OK) {
				ret = get_next_ff_recnum(event_param, recnum, nof_stored);
			}
		} else {
			ret = E_NOT_OK;
		}
	} else {
		*recnum = 0;
	}

	return ret;
}

FUNC(void, DEM_CODE)
dem_get_ff_data(const dem_event_parameter_type *event_param, dem_ff_rec_type *ff, dem_ff_kind_type ff_kind,
		dem_dtc_origin_type origin)
{
	uint8 recnum = 0;
	uint16 store_index = 0;
	const dem_ff_class_type *ff_local = NULL;

	(void)memset(ff, 0, sizeof(dem_ff_rec_type));

	if ((DEM_FREEZE_FRAME_NON_OBD == ff_kind) && (DEM_INVALID_U16_REF_INDEX != event_param->ff_class_ref_index)) {
		ff_local = dem_get_event_ff_class(event_param);
	} else if ((DEM_FREEZE_FRAME_OBD == ff_kind) && (NULL != g_dem_config_set->global_obd_ff_class_ref)) {
		ff_local = g_dem_config_set->global_obd_ff_class_ref;
	} else {
		ff_local = NULL;
	}

	if ((NULL != ff_local) && (NULL != ff_local->ff_id_class_ref)) {
		uint8 *data_ptr = &ff->data[0];

		switch (ff_kind) {
			case DEM_FREEZE_FRAME_NON_OBD:
				dem_get_did_data_for_ff(ff_local->ff_id_class_ref, data_ptr, &store_index, event_param);
				break;
			case DEM_FREEZE_FRAME_OBD:
				dem_get_pid_data_for_ff(ff_local->ff_id_class_ref, data_ptr, &store_index, event_param);
				break;
			default:
				break;
		}
	}

	if ((store_index != 0u) && (dem_get_next_ff_recnum(event_param, &recnum, ff_kind, origin) == E_OK)) {
		ff->event_id = event_param->event_id;
		ff->data_size = store_index;
		ff->recnum = recnum;
		ff->kind = ff_kind;
	} else {
		ff->event_id = DEM_EVENT_ID_NULL;
		ff->data_size = store_index;
	}
}
#endif

FUNC(boolean, DEM_CODE)
dem_lookup_ff_data_size(uint8 recnum, dem_ff_class_type const *const *ff_class_ptr, uint16 *data_size)
{
	(void)recnum;
	boolean data_size_found = FALSE;
	uint16 i = 0u;

	if ((NULL != *ff_class_ptr) && (NULL != data_size)) {
		*data_size = 0;
		for (i = 0; (i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) &&
			    (TRUE != (*ff_class_ptr)->ff_id_class_ref[i]->dem_eol); i++) {
			*data_size += ((uint16)(dem_get_did_data_size((*ff_class_ptr)->ff_id_class_ref[i])) + 2u);
		}
		data_size_found = TRUE;
	}

	return data_size_found;
}

FUNC(boolean, DEM_CODE)
dem_get_ff_record(dem_event_id_type event_id, uint8 recnum, dem_ff_class_type const *ff_data_record_class,
		  dem_dtc_origin_type dtc_origin, uint8 *dest_buffer, uint16 *buf_size)
{
	boolean ff_found = FALSE;
	uint16 store_index = 0;
	uint16 record_size = 0;
	uint16 ff_data_index = 0;
	dem_ff_rec_type ff_local = { 0 };
	dem_ff_rec_type *ff = &ff_local;

	const dem_pid_or_did_type *const *ff_id_class_ref = ff_data_record_class->ff_id_class_ref;

	ff_found = dem_get_stored_ff(event_id, recnum, dtc_origin, &ff);
	if (FALSE == ff_found) {
		return FALSE;
	}

	dest_buffer[0] = recnum;
	dest_buffer[1] = 0;
	for (uint8 ff_num = 0; FALSE == ff_id_class_ref[ff_num]->dem_eol; ff_num++) {
		dest_buffer[1]++;
	}
	store_index = 2u;

	for (uint16 i = 0; ((i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && (!ff_id_class_ref[i]->dem_eol)); i++) {
		record_size = dem_get_did_data_size(ff_id_class_ref[i]);

		if (((store_index + record_size + 2u) > (*buf_size)) ||
			((ff_data_index + record_size) > DEM_MAX_SIZE_FF_DATA)) {
			DEM_DET_REPORT(DEM_DEM_GET_FREEZEFRAME_ID_DEM_E_FF_TOO_BIG);
			break;
		}
		dest_buffer[store_index] = (uint8)((ff_id_class_ref[i]->identifier & 0xff00u) >> 8u);
		dest_buffer[store_index + 1u] = (uint8)(ff_id_class_ref[i]->identifier & 0x00ffu);
		(void)memcpy(&dest_buffer[store_index + 2u], &ff->data[ff_data_index], record_size);
		ff_data_index += record_size;
		store_index += (2u + record_size);
	}
	*buf_size = store_index;

	return ff_found;
}

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(void, DEM_CODE)
dem_set_obd_ff_event_id(uint8 ff_index, dem_event_id_type event_id)
{
	g_dem_obd_ff_mem_data[ff_index].event_id = event_id;
}

FUNC(dem_event_id_type, DEM_CODE) dem_get_obd_ff_event_id(uint8 ff_index)
{
	return g_dem_obd_ff_mem_data[ff_index].event_id;
}

FUNC_P2VAR(uint8, AUTOMATIC, DEM_CODE) dem_get_obd_ff_data(uint8 ff_index)
{
	return &(g_dem_obd_ff_mem_data[ff_index].ff_data[0]);
}

FUNC(boolean, DEM_CODE)
dem_store_obd_ff(const dem_event_parameter_type *event_param)
{
	dem_prestore_entry_type *prestored_data = NULL;
	uint8 ff_index = 0u;

	ff_index = get_obd_ff_index(event_param->event_id);
	if (DEM_CFG_GLOBAL_FREEZEFRAME_SIZE == ff_index) {
		/* 找不到位置存储，返回存储失败结果 */
		return FALSE;
	}

#if (DEM_ENABLE_PRESTORED_FF == STD_ON)
	prestored_data = dem_is_ff_prestored(event_param);
	if (NULL == prestored_data) {
		/* 没有预存储条目时捕捉冻结帧信息 */
		store_obd_ff(event_param, ff_index);
	} else {
		dem_process_prestored_obd_ff(prestored_data, ff_index, event_param);
	}
#else
	(void)prestored_data;
	store_obd_ff(event_param, ff_index);
#endif

	return TRUE;
}

FUNC(boolean, DEM_CODE) dem_clear_obd_ff(dem_event_id_type event_id)
{
	boolean ret = TRUE;

	for (uint8 ff_index = 0u; ff_index < DEM_CFG_GLOBAL_FREEZEFRAME_SIZE; ff_index++) {
		if (event_id == g_dem_obd_ff_mem_data[ff_index].event_id) {
			(void)memset(&g_dem_obd_ff_mem_data[ff_index], 0u, sizeof(dem_obd_ff_mem_type));
			g_dem_obd_ff_mem_data[ff_index].event_id = DEM_EVENT_ID_NULL;

#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
			dem_nvm_set_obd_ff_block_changed(TRUE);
#else
			dem_nvm_set_obd_ff_block_changed(FALSE);
#endif
		}
	}

	return ret;
}

FUNC(boolean, DEM_CODE) dem_clear_all_obd_ff(void)
{
	boolean ret = TRUE;

	for (uint8 ff_index = 0u; ff_index < DEM_CFG_GLOBAL_FREEZEFRAME_SIZE; ff_index++) {
		(void)memset(&g_dem_obd_ff_mem_data[ff_index], 0u, sizeof(dem_obd_ff_mem_type));
		g_dem_obd_ff_mem_data[ff_index].event_id = DEM_EVENT_ID_NULL;
	}
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
	dem_nvm_set_obd_ff_block_changed(TRUE);
#else
	dem_nvm_set_obd_ff_block_changed(FALSE);
#endif

	return ret;
}
#endif

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
FUNC(void, DEM_CODE)
dem_get_pid_data_for_ff(const dem_pid_or_did_type *const *pid_class_ptr, uint8 *data_ptr, uint16 *store_index_ptr,
			const dem_event_parameter_type *event_param)
{
	uint16 store_index = 0;
#if (DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA > 0)
	const dem_pid_or_did_type *const *ff_id_class_ref;
	uint16 record_size = 0;

	ff_id_class_ref = pid_class_ptr;

	for (uint16 i = 0; ((i < DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA) && (!ff_id_class_ref[i]->dem_eol)); i++) {
		record_size = dem_get_did_data_size(ff_id_class_ref[i]);
		if ((store_index + record_size) > DEM_MAX_SIZE_FF_DATA) {
			DEM_DET_REPORT(DEM_DEM_GET_FREEZEFRAME_ID_DEM_E_FF_TOO_BIG);
			break;
		}

		uint8 data_elem_start = ff_id_class_ref[i]->data_class_ref_start;
		uint8 data_elem_end = ff_id_class_ref[i]->data_class_ref_start + ff_id_class_ref[i]->data_class_num;

		for (uint8 data_elem_idx = data_elem_start; data_elem_idx < data_elem_end; data_elem_idx++) {
			uint8 mapped_idx = g_dem_cfg_helper->did_data_elem_map[data_elem_idx];

			const dem_data_elem_class_type *data_elem_ref = &g_dem_config_set->data_elem[mapped_idx];

			if (NULL != data_elem_ref->data_elem_read_fnc) {
				(void)data_elem_ref->data_elem_read_fnc(&(data_ptr[store_index]));
			} else if (DEM_NO_ELEMENT != data_elem_ref->internal_data_type) {
				dem_get_internal_element(event_param, data_elem_ref->internal_data_type,
							 &(data_ptr[store_index]), data_elem_ref->data_size);
			} else {
				(void)memset(&(data_ptr[store_index]), DEM_FREEZEFRAME_DEFAULT_VALUE,
					     data_elem_ref->data_size);
				DEM_DET_REPORT(DEM_DEM_GET_FREEZEFRAME_ID_DEM_E_NODATAAVAILABLE);
			}
			store_index += data_elem_ref->data_size;
		}
	}
#else
	(void)data_ptr;
	(void)pid_class_ptr;
	(void)event_param;
#endif
	*store_index_ptr = store_index;

	return;
}
#endif

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(boolean, DEM_CODE) dem_delete_all_obd_ff(void)
{
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	boolean clear_allowed = TRUE;

	for (uint8 ff_index = 0u; ff_index < DEM_CFG_GLOBAL_FREEZEFRAME_SIZE; ff_index++) {
		event_id = g_dem_obd_ff_mem_data[ff_index].event_id;

		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

		clear_allowed = dem_is_event_cleard_allowed(event_id);
		if (FALSE == clear_allowed) {
			continue;
		}

#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
		dem_nvm_set_obd_ff_block_changed(TRUE);
#else
		dem_nvm_set_obd_ff_block_changed(FALSE);
#endif
		(void)memset((void *)(&g_dem_obd_ff_mem_data[ff_index]), 0u, sizeof(dem_obd_ff_mem_type));
	}

	return TRUE;
}
#endif

FUNC(boolean, DEM_CODE)
dem_check_obdff_storage_condition(dem_event_status_rec_type old_status, dem_event_status_rec_type status)
{
	if ((DEM_IS_MASK_UNSET(old_status, DEM_CONFIRMED_DTC, dem_event_status_rec_type) &&
	     DEM_IS_MASK_SET(status, DEM_CONFIRMED_DTC, dem_event_status_rec_type)) ||
	    (DEM_IS_MASK_UNSET(old_status, DEM_PENDING_DTC, dem_event_status_rec_type) &&
	     DEM_IS_MASK_SET(status, DEM_PENDING_DTC, dem_event_status_rec_type))) {
		return TRUE;
	} else {
		return FALSE;
	}
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"