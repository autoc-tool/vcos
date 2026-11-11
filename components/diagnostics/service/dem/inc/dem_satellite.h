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

#ifndef DEM_SATELLITE_H
#define DEM_SATELLITE_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg.h"
#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 *  @brief          设置事件队列更新
 *  @details        设置事件队列更新
 *  @param[in]      satellite_id 卫星id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_satellite_set_event_queue_updated(const dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 *  @brief          更新卫星数据
 *  @details        更新卫星数据
 *  @param[in]      old_satellite_data 旧卫星数据
 *  @param[in]      event_status 事件状态
 *  @retval         dem_satellite_data_type 卫星数据
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_data_type, DEM_CODE)
dem_update_satellite_data(const dem_satellite_data_type old_satellite_data, dem_event_status_type event_status);

/*!*********************************************************************************************************************
 *  @brief          测试队列状态更新
 *  @details        测试队列状态更新
 *  @param[in]      old_satellite_data 旧卫星数据
 *  @param[in]      new_satellite_data 新卫星数据
 *  @retval         TRUE 测试队列状态更新成功
 *  @retval         FALSE 测试队列状态更新失败
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE)
dem_is_queue_status_updated(const dem_satellite_data_type old_satellite_data,
			    const dem_satellite_data_type new_satellite_data);

/*!*********************************************************************************************************************
 *  @brief          卫星内存初始化
 *  @details        卫星内存初始化
 *  @param[in]      satellite_id 卫星id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_satellite_memory_init(const dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 *  @brief          卫星预初始化
 *  @details        卫星预初始化
 *  @param[in]      satellite_id 卫星id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_satellite_pre_init_internal(const dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 *  @brief          设置卫星状态
 *  @details        设置卫星状态
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      state 卫星状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_set_satellite_state(const dem_satellite_id_type satellite_id, const dem_state_type state);

/*!*********************************************************************************************************************
 *  @brief          同步设置卫星数据
 *  @details        同步设置卫星数据
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      satellite_event_id 卫星事件id
 *  @param[in]      old_satellite_data 旧卫星数据
 *  @param[in]      new_satellite_data 新卫星数据
 *  @retval         TRUE 同步设置卫星数据成功
 *  @retval         FALSE 同步设置卫星数据失败
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE)
dem_satellite_sync_set_satellite_data(const dem_satellite_id_type satellite_id,
				      const dem_satellite_event_id_type satellite_event_id,
				      const dem_satellite_data_type old_satellite_data,
				      const dem_satellite_data_type new_satellite_data);

/*!*********************************************************************************************************************
 *  @brief          清除事件队列更新
 *  @details        清除事件队列更新
 *  @param[in]      satellite_id 卫星id
 *  @retval         TRUE 清除事件队列更新成功
 *  @retval         FALSE 清除事件队列更新失败
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_clear_event_queue_updated(dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 *  @brief          获取事件队列状态
 *  @details        获取事件队列状态
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      satellite_event_id 卫星事件id
 *  @retval         0-255 事件队列状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_queue_status_type, DEM_CODE)
dem_get_event_queue_status(const dem_satellite_id_type satellite_id,
			   const dem_satellite_event_id_type satellite_event_id);

/*!*********************************************************************************************************************
 *  @brief          处理事件队列状态
 *  @details        处理事件队列状态
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      satellite_event_id 卫星事件id
 *  @retval         0-255 事件队列状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_queue_status_type, DEM_CODE)
dem_handle_event_queue_status(const dem_satellite_id_type satellite_id,
			      const dem_satellite_event_id_type satellite_event_id);

/*!*********************************************************************************************************************
 *  @brief          获取卫星状态
 *  @details        获取卫星状态
 *  @param[in]      satellite_id 卫星id
 *  @retval         0-255 卫星状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(uint8, DEM_CODE) dem_get_satellite_state(dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 *  @brief          当前分区是主
 *  @details        当前分区是主
 *  @retval         TRUE 当前分区是主
 *  @retval         FALSE 当前分区不是主
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_current_partition_is_master(void);

/*!*********************************************************************************************************************
 *  @brief          获取当前卫星id
 *  @details        获取当前卫星id
 *  @retval         0-255 当前卫星id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_id_type, DEM_CODE) dem_get_current_satellite_id(void);

/*!*********************************************************************************************************************
 *  @brief          检查事件应用id
 *  @details        检查事件应用id
 *  @param[in]      event_id 事件id
 *  @retval         TRUE 检查事件应用id成功
 *  @retval         FALSE 检查事件应用id失败
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_check_event_application_id(const dem_event_id_type event_id);

/*!*********************************************************************************************************************
 *  @brief          获取事件卫星id
 *  @details        获取事件卫星id
 *  @param[in]      event_id 事件id
 *  @retval         0-255 事件卫星id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_id_type, DEM_CODE) dem_get_event_satellite_id(const dem_event_id_type event_id);

/*!*********************************************************************************************************************
 *  @brief          获取卫星事件id
 *  @details        获取卫星事件id
 *  @param[in]      event_id 事件id
 *  @retval         0-255 事件卫星id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_event_id_type, DEM_CODE) dem_get_satellite_event_id(const dem_event_id_type event_id);

/*!*********************************************************************************************************************
 *  @brief          卫星获取事件id
 *  @details        卫星获取事件id
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      satellite_event_id 卫星事件id
 *  @retval         0-255 事件id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_id_type, DEM_CODE)
dem_satellite_get_event_id(const dem_satellite_id_type satellite_id,
			   const dem_satellite_event_id_type satellite_event_id);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_SATELLITE_H */
