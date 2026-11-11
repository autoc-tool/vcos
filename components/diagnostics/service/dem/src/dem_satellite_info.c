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

FUNC(dem_satellite_status_info_type, DEM_CODE)
dem_set_satellite_info_state(const dem_satellite_status_info_type satellite_info, const uint8 state)
{
	dem_satellite_status_info_type ret = satellite_info;

	ret.data.state = state;

	return ret;
}

FUNC(dem_satellite_status_info_type, DEM_CODE)
dem_set_event_queue_updated(const dem_satellite_status_info_type satellite_info)
{
	dem_satellite_status_info_type ret = satellite_info;

	DEM_SET_MASK(ret.data.satellite_status, DEM_SATELLITEINFO_EVENT_QUEUE_STATUS, uint8);

	return ret;
}

/* 重置event的队列状态位，供下一次故障上报处理 */
FUNC(dem_satellite_status_info_type, DEM_CODE)
dem_reset_event_queue_updated(const dem_satellite_status_info_type satellite_info)
{
	dem_satellite_status_info_type ret = satellite_info;

	DEM_CLEAR_MASK(ret.data.satellite_status, DEM_SATELLITEINFO_EVENT_QUEUE_STATUS, uint8);

	return ret;
}

boolean dem_get_event_queue_updated(const dem_satellite_status_info_type satellite_info)
{
	if (DEM_IS_MASK_SET(satellite_info.data.satellite_status, DEM_SATELLITEINFO_EVENT_QUEUE_STATUS, uint8)) {
		return TRUE;
	}

	return FALSE;
}

FUNC(uint8, DEM_CODE) dem_get_satellite_info_state(const dem_satellite_status_info_type satellite_info)
{
	return satellite_info.data.state;
}

FUNC_P2VAR(dem_atomic_word_type, AUTOMATIC, DEM_CODE)
dem_get_satellite_status(const dem_satellite_id_type satellite_id)
{
	return (dem_atomic_word_type *)g_dem_satellite_info[satellite_id].satellite_status;
}

FUNC(dem_satellite_event_id_type, DEM_CODE) dem_get_satellite_event_count(const dem_satellite_id_type satellite_id)
{
	return g_dem_satellite_info[satellite_id].event_count;
}

/* 获取satellite_id下satellite_event_id事件对应的satellite_data */
FUNC(dem_satellite_data_type, DEM_CODE)
dem_get_satellite_data(const dem_satellite_id_type satellite_id, const dem_satellite_event_id_type satellite_event_id)
{
	return g_dem_satellite_info[satellite_id].satellite_data[satellite_event_id];
}

/* 获取satellite_id下satellite_event_id事件对应的satellite_data的指针 */
FUNC(dem_satellite_data_type *, DEM_CODE)
dem_get_satellite_data_ptr(const dem_satellite_id_type satellite_id,
			   const dem_satellite_event_id_type satellite_event_id)
{
	return (dem_satellite_data_type *)(&g_dem_satellite_info[satellite_id].satellite_data[satellite_event_id]);
}

/* 获取event_id配置所在的application_id */
FUNC(ApplicationType, DEM_CODE) dem_get_event_application_id(const dem_event_id_type event_id)
{
	const dem_event_parameter_type *event_param = NULL;

	dem_lookup_event_param(event_id, &event_param);
	if (NULL == event_param) {
		return (ApplicationType)0;
	}

	return (ApplicationType)g_dem_satellite_info[event_param->satellite_id].application_id;
}

/* 检查satellite数量，是否为单satellite */
FUNC(boolean, DEM_CODE) dem_is_single_satellite(void)
{
	if (DEM_MAX_NUMBER_SATELLITE > 1u) {
		return FALSE;
	}

	return TRUE;
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"