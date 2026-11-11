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

#ifndef DEM_AGE_H
#define DEM_AGE_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg.h"
#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_SUPPORT_AGING == STD_ON) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
FUNC(std_return_type, DEM_CODE) dem_handle_aging(dem_opcycle_id_type opcycle_id);
#endif

#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
FUNC(uint8, DEM_CODE) dem_get_event_aged_cnt(uint16 event_aged_data_index);
FUNC(void, DEM_CODE) dem_set_event_aged_cnt(const dem_event_parameter_type *event_param, uint8 aged_cnt);
#endif

#if (DEM_SUPPORT_AGING == STD_ON)
FUNC(void, DEM_CODE)
dem_handle_aging_cnt_on_failed(const dem_event_parameter_type *event_param, dem_event_info_rec_type *event_info);
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_AGE_H */