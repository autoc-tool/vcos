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

#ifndef DEM_DEBOUNCE_H
#define DEM_DEBOUNCE_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
FUNC(void, DEM_CODE) dem_time_based_debounce_main_function(dem_satellite_id_type satellite_id);
#endif
#endif

FUNC(void, DEM_CODE) dem_reset_debounce_counter(dem_event_info_rec_type *event_info_rec);

FUNC(std_return_type, DEM_CODE)
dem_get_fdc_internal(dem_event_id_type event_id, sint8 *fault_detection_counter);

#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON) || (DEM_SUPPORT_CURRENT_FDC == STD_ON)
FUNC(sint8, DEM_CODE) dem_get_default_uds_fdc(dem_event_id_type event_id);
#endif

FUNC(dem_event_status_type, DEM_CODE)
dem_process_debounce(const dem_event_parameter_type *event_param, dem_event_status_type reported_event_status,
			dem_event_info_rec_type *event_info);

FUNC(std_return_type, DEM_CODE)
dem_get_debouncing_state(dem_event_id_type event_id, dem_debouncing_state_type *debouncing_state);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_DEBOUNCE_H */
