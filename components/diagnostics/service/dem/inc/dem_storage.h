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

#ifndef DEM_STORAGE_H
#define DEM_STORAGE_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_SUPPORTED_ORIGIN(_x) (DEM_DTC_ORIGIN_PRIMARY_MEMORY == (_x))

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(boolean, DEM_CODE)
dem_delete_event_mem(const dem_event_parameter_type *event_param, dem_primary_mem_type *event_memory,
		 uint32 event_memory_size, dem_dtc_origin_type origin);
#endif

FUNC(void, DEM_CODE) dem_set_event_status_data_blcok_changed(dem_event_id_type event_id);

FUNC(void, DEM_CODE)
dem_set_event_status_data(dem_event_id_type event_id, dem_event_status_rec_type event_status_rec);

FUNC(void, DEM_CODE) dem_set_event_trip_cnt(dem_event_id_type event_id, uint8 trip_cnt);

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(void, DEM_CODE) dem_set_event_timestamp(dem_event_info_rec_type *event_info);
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(boolean, DEM_CODE)
dem_handle_permanent_dtc_erase(const dem_event_info_rec_type *event_info_rec, dem_opcycle_id_type opcycle_id);

FUNC(void, DEM_CODE) dem_merge_permanent_storage(void);
#endif /* DEM_USE_PERMANENT_MEMORY_SUPPORT */

FUNC(void, DEM_CODE) dem_notify_event_storage_changed(const dem_event_parameter_type *event_param);

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(void, DEM_CODE) dem_change_perm_status_on_failed(const dem_event_parameter_type *event_param);
#endif

FUNC(boolean, DEM_CODE)
dem_process_event_storage(dem_event_info_rec_type *event_info_rec,
					  const dem_event_parameter_type *event_param,
					  dem_event_status_rec_type old_status, dem_event_status_rec_type new_status,
					  boolean old_fdc_this_opcycle);

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
FUNC(void, DEM_CODE) dem_set_overflow_indication(dem_dtc_origin_type origin, boolean overflow);

FUNC(std_return_type, DEM_CODE) dem_get_overflow_indication(dem_dtc_origin_type origin, boolean *overflow);
#endif
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(void, DEM_CODE) dem_set_event_block_changed(dem_dtc_origin_type origin, dem_event_id_type event_id);
#endif /* DEM_USE_MEMORY_FUNCTIONS */

FUNC(boolean, DEM_CODE)
dem_store_ext_data(const dem_event_parameter_type *event_param, boolean ext_data_stored,
		   dem_primary_mem_type *event_rec);

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE)
dem_delete_all_dtc_data_in_memory(dem_dtc_origin_type origin, dem_primary_mem_type *event_memory,
				  uint32 event_buffer_size);
#endif

FUNC(std_return_type, DEM_CODE)
dem_store_event_mem(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_status,
		    boolean failed_now, dem_primary_mem_type **event_rec);

FUNC(boolean, DEM_CODE)
dem_lookup_ff_data_recnum_param(uint8 recnum, const dem_event_parameter_type *event_param,
			    dem_ff_class_type const **ff_class_ptr);

FUNC(boolean, DEM_CODE)
dem_get_stored_ff(dem_event_id_type event_id, uint8 recnum, dem_dtc_origin_type dtc_origin, dem_ff_rec_type **ff);

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
FUNC(boolean, DEM_CODE) dem_is_storage_conditions_set(const dem_event_class_type *event_class);
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(void, DEM_CODE) dem_transfer_event_to_permanent(uint16 nof_entries);
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(void, DEM_CODE)
dem_handle_perm_status_on_opcycle_end(dem_opcycle_id_type opcycle_id, const dem_event_info_rec_type *event_info);
#endif

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
FUNC(void, DEM_CODE) dem_clear_obd_storage(const dem_event_parameter_type *event_param, boolean clear_obd_evt_mem);
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
FUNC(void, DEM_CODE) dem_init_storage_conditions(void);

FUNC(uint8, DEM_CODE) dem_get_storage_condition_state(uint8 id);
#endif

#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(void, DEM_CODE) dem_merge_storage_to_running_mem(dem_dtc_origin_type origin);
#endif /* DEM_USE_MEMORY_FUNCTIONS */

FUNC(void, DEM_CODE) dem_clear_storage_mem(void);

FUNC(dem_return_get_ext_data_record_by_dtc_type, DEM_CODE)
dem_get_ext_data_record(const dem_ext_data_record_class_type *ext_data_record_class,
					 dem_dtc_origin_type dtc_origin, uint8 *dest_buffer, uint16 *buf_size,
					 const dem_event_info_rec_type *event_rec, uint8 ext_data_number);

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
#if (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(boolean, DEM_CODE) dem_is_event_stored_in_memory(dem_event_id_type event_id);
#endif
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
/* ****************************************************************************
 * dem_mem_permanent_get_event
 *****************************************************************************/
FUNC(dem_event_id_type, DEM_CODE) dem_get_permanent_event(uint8 index);

/* ****************************************************************************
 * dem_mem_permanent_set_status
 *****************************************************************************/
FUNC(void, DEM_CODE) dem_set_permanent_status(uint8 index, uint8 status);
#endif

#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
FUNC(void, DEM_CODE) dem_set_aged_data_blcok_changed(const dem_event_parameter_type *event_param);
#endif

FUNC(void, DEM_CODE) dem_init_dtc_record_disabled(void);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_STORAGE_H */