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

#ifndef DEM_INDICATOR_H
#define DEM_INDICATOR_H

#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(DEM_USE_INDICATORS)
#define INDICATOR_FAILED_DURING_FAILURE_CYCLE 1u
#define INDICATOR_PASSED_DURING_FAILURE_CYCLE (1u << 1u)
#define INDICATOR_FAILED_DURING_HEALING_CYCLE (1u << 2u)
#define INDICATOR_PASSED_DURING_HEALING_CYCLE (1u << 3u)
#endif

#define IS_VALID_INDICATOR_ID(_x) ((_x) < DEM_NOF_INDICATORS)

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if defined(DEM_USE_INDICATORS)
FUNC_P2VAR(const dem_indicator_attribute_type *const, AUTOMATIC, DEM_CODE)
dem_get_event_indicator_attr_base(const dem_event_class_type *event_class);
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
FUNC(boolean, DEM_CODE) dem_is_emission_event_ref_to_mil(const dem_event_parameter_type *event_param);
#endif

#if defined(DEM_USE_INDICATORS)
FUNC(boolean, DEM_CODE) dem_reset_indicator_counters(const dem_event_parameter_type *event_param);

FUNC(boolean, DEM_CODE)
dem_indicator_fail_fulfilled(const dem_event_parameter_type *event_param,
			 const dem_indicator_attribute_type *indicator_attribute);

FUNC(boolean, DEM_CODE) dem_is_indicator_fulfilled(const dem_event_parameter_type *event_param);

FUNC(void, DEM_CODE) dem_indicator_opcycle_start(dem_opcycle_id_type opcycle_id);

FUNC(boolean, DEM_CODE)
dem_indicator_opcycle_end(dem_opcycle_id_type opcycle_id, dem_event_info_rec_type *event_info);

FUNC(boolean, DEM_CODE)
dem_handle_indicators(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_info,
		  dem_event_status_type event_status);
#endif

#if ((DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON) && defined(DEM_USE_INDICATORS)) ||                                       \
	(DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)

FUNC(boolean, DEM_CODE) dem_is_event_activates_mil(const dem_event_parameter_type *event_param);
#endif

#if defined(DEM_USE_INDICATORS)
FUNC(void, DEM_CODE) dem_init_indicator_infos(const dem_indicator_type *indicator_ptr);

FUNC(std_return_type, DEM_CODE) dem_get_event_indicator_off_fulfilled(dem_event_id_type event_id, boolean *fulfilled);

FUNC(std_return_type, DEM_CODE) dem_get_event_indicator_on_fulfilled(dem_event_id_type event_id, boolean *fulfilled);
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_INDICATOR_H */