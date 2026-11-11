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

#define DEM_START_SEC_MULTI_APP_BSS
#include "dem_memmap.h"

static VAR(uint8, DEM_MULTI_APP_BSS) g_dem_master_state;
#if (DEM_USE_CONFIGPTR == STD_ON)
static P2CONST(dem_config_type, DEM_MULTI_APP_BSS, dem_var_class) g_dem_config_ptr;
static VAR(uint8, DEM_MULTI_APP_BSS) g_dem_config_ptr_state;
#endif

#define DEM_STOP_SEC_MULTI_APP_BSS
#include "dem_memmap.h"

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

FUNC(dem_state_type, DEM_CODE) dem_get_master_state(void)
{
	return g_dem_master_state;
}

FUNC(void, DEM_CODE) dem_set_master_state(dem_state_type state)
{
	g_dem_master_state = state;
}

FUNC(boolean, DEM_CODE) dem_is_master_in_state(const dem_state_type state)
{
	if (g_dem_master_state == state) {
		return TRUE;
	}

	return FALSE;
}

FUNC(boolean, DEM_CODE) dem_is_master_at_least_in_state(const dem_state_type state)
{
	if (g_dem_master_state >= state) {
		return TRUE;
	}

	return FALSE;
}

FUNC(boolean, DEM_CODE) dem_is_satellite_in_state(const dem_satellite_id_type satellite_id, const dem_state_type state)
{
	if (dem_get_satellite_state(satellite_id) == state) {
		return TRUE;
	}

	return FALSE;
}

FUNC(boolean, DEM_CODE)
dem_is_satellite_at_least_in_state(const dem_satellite_id_type satellite_id, const dem_state_type state)
{
	if (dem_get_satellite_state(satellite_id) >= state) {
		return TRUE;
	}

	return FALSE;
}

FUNC(boolean, DEM_CODE) dem_is_all_satellites_at_least_in_state(const dem_state_type state)
{
	for (dem_satellite_id_type satellite_id = 0u; satellite_id < DEM_MAX_NUMBER_SATELLITE; satellite_id++) {
		if (dem_is_satellite_at_least_in_state(satellite_id, state) == FALSE) {
			return FALSE;
		}
	}

	return TRUE;
}

FUNC(boolean, DEM_CODE) dem_is_event_satellite_in_state(const dem_event_id_type event_id, const dem_state_type state)
{
	const dem_event_parameter_type *event_param = NULL;

	dem_lookup_event_param(event_id, &event_param);
	if (NULL == event_param) {
		return FALSE;
	}

	return dem_is_satellite_in_state(event_param->satellite_id, state);
}

#if (DEM_USE_CONFIGPTR == STD_ON)
FUNC_P2CONST(dem_config_type, AUTOMATIC, DEM_CODE) dem_get_global_config_ptr(void)
{
	return g_dem_config_ptr;
}

FUNC(void, DEM_CODE) dem_set_global_config_ptr(const dem_config_type *config_ptr)
{
	g_dem_config_ptr = config_ptr;
}

FUNC(const dem_config_ptr_state_type, DEM_CODE) dem_get_global_config_ptr_state(void)
{
	return g_dem_config_ptr_state;
}

FUNC(void, DEM_CODE) dem_set_global_config_ptr_state(dem_config_ptr_state_type global_config_ptr_state)
{
	g_dem_config_ptr_state = global_config_ptr_state;
}

FUNC(boolean, DEM_CODE) dem_is_global_cfg_ptr_in_state(dem_config_ptr_state_type global_config_ptr_state)
{
	if (dem_get_global_config_ptr_state() == global_config_ptr_state) {
		return TRUE;
	}

	return FALSE;
}

FUNC(boolean, DEM_CODE) dem_is_global_cfg_ptr_at_least_in_state(dem_config_ptr_state_type global_config_ptr_state)
{
	if (dem_get_global_config_ptr_state() >= global_config_ptr_state) {
		return TRUE;
	}

	return FALSE;
}
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"