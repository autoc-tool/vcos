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

/* 获取一个monitor为初始状态的satellite_data */
FUNC(dem_satellite_data_type, DEM_CODE) dem_get_satellite_data_init_value(void)
{
	dem_satellite_data_type satellite_data;

	satellite_data.raw = 0u;

	return satellite_data;
}

/* 设置satellite_data_value下的事件队列状态 */
FUNC(dem_satellite_data_type, DEM_CODE)
dem_set_satellite_data_queue_status(const dem_satellite_data_type satellite_data_value,
				    const dem_event_queue_status_type queue_status)
{
	dem_satellite_data_type satellite_data = satellite_data_value;

	satellite_data.data.queue_status = queue_status;

	return satellite_data;
}

/* 获取satellite_data_value下的事件队列状态 */
FUNC(dem_event_queue_status_type, DEM_CODE)
dem_get_satellite_data_queue_status(const dem_satellite_data_type satellite_data_value)
{
	return satellite_data_value.data.queue_status;
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"