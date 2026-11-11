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

#ifndef DEM_FREEZE_FRAME_H
#define DEM_FREEZE_FRAME_H

#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

FUNC_P2CONST(dem_ff_class_type, AUTOMATIC, DEM_CODE)
dem_get_event_ff_class(const dem_event_parameter_type *event_param);

FUNC_P2CONST(dem_ff_recnum_class_type, AUTOMATIC, DEM_CODE)
dem_get_event_ff_recnum_class(const dem_event_parameter_type *event_param);

FUNC(uint16, DEM_CODE) dem_get_did_data_size(const dem_pid_or_did_type *const did_class);

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
FUNC(void, DEM_CODE)
dem_get_did_data_for_ff(const dem_pid_or_did_type *const *did_class_ptr, uint8 *data_ptr, uint16 *store_index_ptr,
			const dem_event_parameter_type *event_param);
FUNC(std_return_type, DEM_CODE)
dem_get_next_ff_recnum(const dem_event_parameter_type *event_param, uint8 *recnum, dem_ff_kind_type ff_kind,
		   dem_dtc_origin_type origin);

FUNC(void, DEM_CODE)
dem_get_ff_data(const dem_event_parameter_type *event_param, dem_ff_rec_type *ff, dem_ff_kind_type ff_kind,
	    dem_dtc_origin_type origin);
#endif

FUNC(boolean, DEM_CODE)
dem_lookup_ff_data_size(uint8 recnum, dem_ff_class_type const *const *ff_class_ptr, uint16 *data_size);

FUNC(boolean, DEM_CODE)
dem_get_ff_record(dem_event_id_type event_id, uint8 recnum, dem_ff_class_type const *ff_data_record_class,
	      dem_dtc_origin_type dtc_origin, uint8 *dest_buffer, uint16 *buf_size);

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(boolean, DEM_CODE)
dem_check_obdff_storage_condition(dem_event_status_rec_type old_status, dem_event_status_rec_type status);

FUNC(void, DEM_CODE)
dem_set_obd_ff_event_id(uint8 ff_index, dem_event_id_type event_id);

FUNC(dem_event_id_type, DEM_CODE) dem_get_obd_ff_event_id(uint8 ff_index);

FUNC_P2VAR(uint8, AUTOMATIC, DEM_CODE) dem_get_obd_ff_data(uint8 ff_index);

FUNC(boolean, DEM_CODE)
dem_store_obd_ff(const dem_event_parameter_type *event_param);

FUNC(boolean, DEM_CODE) dem_clear_obd_ff(dem_event_id_type event_id);

FUNC(boolean, DEM_CODE) dem_clear_all_obd_ff(void);
#endif

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
FUNC(void, DEM_CODE)
dem_get_pid_data_for_ff(const dem_pid_or_did_type *const *pid_class_ptr, uint8 *data_ptr, uint16 *store_index_ptr,
			const dem_event_parameter_type *event_param);
#endif

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(boolean, DEM_CODE) dem_delete_all_obd_ff(void);
#endif

FUNC(boolean, DEM_CODE)
dem_check_obdff_storage_condition(dem_event_status_rec_type old_status, dem_event_status_rec_type status);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_FREEZE_FRAME_H */