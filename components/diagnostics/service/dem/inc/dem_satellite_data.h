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

#ifndef DEM_SATELLITE_DATA_H
#define DEM_SATELLITE_DATA_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 *  @brief          获取卫星数据初始值
 *  @details        获取卫星数据初始值
 *  @retval         0-255 卫星数据初始值
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_data_type, DEM_CODE) dem_get_satellite_data_init_value(void);

/*!*********************************************************************************************************************
 *  @brief          设置卫星数据队列状态
 *  @details        设置监控状态
 *  @param[in]      satellite_data_value 卫星数据值
 *  @param[in]      queue_status 队列状态
 *  @retval         0-255 卫星数据队列状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_data_type, DEM_CODE)
dem_set_satellite_data_queue_status(const dem_satellite_data_type satellite_data_value,
				    const dem_event_queue_status_type queue_status);

/*!*********************************************************************************************************************
 *  @brief          获取卫星数据队列状态
 *  @details        获取卫星数据队列状态
 *  @param[in]      satellite_data_value 卫星数据值
 *  @retval         0-255 卫星数据队列状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_queue_status_type, DEM_CODE)
dem_get_satellite_data_queue_status(const dem_satellite_data_type satellite_data_value);

/*!*********************************************************************************************************************
 *  @brief          获取卫星数据指针
 *  @details        获取卫星数据指针
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      satellite_event_id 卫星事件id
 *  @retval         dem_satellite_data_type 卫星数据指针
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC_P2VAR(dem_satellite_data_type, AUTOMATIC, DEM_CODE)
dem_get_satellite_data_ptr(const dem_satellite_id_type satellite_id,
			   const dem_satellite_event_id_type satellite_event_id);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_SATELLITE_DATA_H */
