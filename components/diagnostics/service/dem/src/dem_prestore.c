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

#if (DEM_ENABLE_PRESTORED_FF == STD_ON)
/***********************************************************************************************************************
 *  dem_clear_prestored_ff_entry()
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_clear_prestored_ff_entry(dem_prestore_entry_type *entry)
{
	memset(entry, 0, sizeof(dem_prestore_entry_type));
	entry->event_id = DEM_EVENT_ID_NULL;

	return E_OK;
}

/***********************************************************************************************************************
 *  dem_is_ff_prestored()
 **********************************************************************************************************************/
FUNC_P2VAR(dem_prestore_entry_type, AUTOMATIC, DEM_CODE)
dem_is_ff_prestored(const dem_event_parameter_type *event_param)
{
	if ((NULL == event_param) || (NULL == event_param->event_class_ref) ||
	    (FALSE == event_param->event_class_ref->ff_prestorage_supported)) {
		return NULL;
	}

	return dem_get_prestore_ff_entry(event_param->event_id);
}

/***********************************************************************************************************************
 *  dem_process_prestored_ff()
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_process_prestored_ff(dem_prestore_entry_type *prestored_data, dem_ff_rec_type *ff,
			 const dem_event_parameter_type *event_param, dem_ff_kind_type ff_kind,
			 dem_dtc_origin_type origin)
{
	uint8 recnum = 0;

	if (dem_get_next_ff_recnum(event_param, &recnum, ff_kind, origin) == E_OK) {
		ff->event_id = event_param->event_id;
		ff->data_size = prestored_data->data_size;
		ff->recnum = recnum;
		ff->kind = ff_kind;
		(void)memcpy(&ff->data[0], &prestored_data->data[0], prestored_data->data_size);
		(void)dem_clear_prestored_ff_entry(prestored_data);
	} else {
		ff->event_id = DEM_EVENT_ID_NULL;
		ff->data_size = 0;
	}

	return;
}

/***********************************************************************************************************************
 *  dem_get_prestore_ff_entry_for_updating()
 **********************************************************************************************************************/
FUNC_P2VAR(dem_prestore_entry_type, AUTOMATIC, DEM_CODE)
dem_get_prestore_ff_entry_for_updating(const dem_event_id_type event_id)
{
	dem_prestore_entry_type *prestore_buf = dem_get_prestorage_buffer();
	dem_prestore_entry_type *entry = dem_get_prestore_ff_entry(event_id);

	if (NULL == entry) {
		// 当前队列中不存在该event_id，找一个空的
		for (uint32 i = 0u; i < DEM_MAX_NUMBER_PRESTORED_FF; i++) {
			if (DEM_EVENT_ID_NULL == prestore_buf[i].event_id) {
				prestore_buf[i].event_id = event_id;
				entry = &prestore_buf[i];
				break;
			}
		}
	}

	return entry;
}

/***********************************************************************************************************************
 *  dem_collect_prestored_ff_data()
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_collect_prestored_ff_data(dem_prestore_entry_type *entry, const dem_event_parameter_type *event_param)
{
	std_return_type ret = E_OK;

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
	uint16 store_index = 0;
	const dem_ff_class_type *ff_class = NULL;

	const dem_pid_or_did_type *const *did_ptr = NULL;
	const dem_pid_or_did_type *const *pid_ptr = NULL;
	const dem_config_set_type *config_set = dem_get_config_set();
	uint8 *data_ptr = &entry->data[0];

	if (DEM_DTC_KIND_ALL_DTCS == event_param->event_dtc_kind) {
		ff_class = dem_get_event_ff_class(event_param);
		did_ptr = ff_class->ff_id_class_ref;
		if (NULL != did_ptr) {
			dem_get_did_data_for_ff(did_ptr, data_ptr, &store_index, event_param);
		}
	} else {
		ff_class = config_set->global_obd_ff_class_ref;
		pid_ptr = ff_class->ff_id_class_ref;
		if (NULL != pid_ptr) {
			dem_get_pid_data_for_ff(pid_ptr, data_ptr, &store_index, event_param);
		}
	}
	entry->data_size = store_index;
#else
	(void)entry;
	(void)event_param;
	ret = E_NOT_OK;
#endif

	return ret;
}

/***********************************************************************************************************************
 *  dem_get_prestore_ff_entry()
 **********************************************************************************************************************/
FUNC_P2VAR(dem_prestore_entry_type, AUTOMATIC, DEM_CODE)
dem_get_prestore_ff_entry(const dem_event_id_type event_id)
{
	dem_prestore_entry_type *entry = NULL;
	dem_prestore_entry_type *prestore_buf = dem_get_prestorage_buffer();

	for (uint32 i = 0u; i < DEM_MAX_NUMBER_PRESTORED_FF; i++) {
		if (prestore_buf[i].event_id == event_id) {
			entry = &prestore_buf[i];
			break;
		}
	}

	return entry;
}

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
/***********************************************************************************************************************
 *  dem_process_prestored_obd_ff()
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_process_prestored_obd_ff(dem_prestore_entry_type *prestored_data, uint8 obd_ff_index,
			     const dem_event_parameter_type *event_param)
{
	uint8 *obd_ff_data = dem_get_obd_ff_data(obd_ff_index);

	dem_set_obd_ff_event_id(obd_ff_index, event_param->event_id);
	(void)memcpy(obd_ff_data, &prestored_data->data[0], prestored_data->data_size);
	(void)dem_clear_prestored_ff_entry(prestored_data);

	return;
}
#endif

#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"
