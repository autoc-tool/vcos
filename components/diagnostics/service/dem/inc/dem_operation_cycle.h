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

#ifndef DEM_OPERATION_CYCLE_H
#define DEM_OPERATION_CYCLE_H

#include "dem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#define DEM_CYCLE_CNTR_MAX 0xFFu

FUNC(boolean, DEM_CODE) dem_is_failure_cycle_started(const dem_event_parameter_type *event_param);

#if (DEM_SUPPORT_CYCLES_COUNTER == STD_ON)
FUNC(void, DEM_CODE) dem_handle_cycle_counters(dem_event_info_rec_type *event_info_rec, boolean event_is_stored);
#endif

FUNC(std_return_type, DEM_CODE)
dem_handle_set_opcycle_state(dem_opcycle_id_type opcycle_id, dem_opcycle_state_type cycle_state);

FUNC(void, DEM_CODE) dem_set_pending_opcycle(uint8 cycle_id, uint8 status);

FUNC(void, DEM_CODE) dem_precess_opcycle_main_function(void);

FUNC(void, DEM_CODE) dem_init_opcycle_state_list(void);

FUNC(void, DEM_CODE) dem_init_auto_start_opcycle(void);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_OPERATION_CYCLE_H */