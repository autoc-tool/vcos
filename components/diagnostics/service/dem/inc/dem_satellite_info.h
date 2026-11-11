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

#ifndef DEM_SATELLITE_INFO_H
#define DEM_SATELLITE_INFO_H

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
 *  @brief          设置卫星状态信息初始化状态
 *  @details        设置卫星状态信息初始化状态
 *  @param[in]      satellite_info 卫星信息
 *  @param[in]      state 输入初始化状态
 *  @retval         dem_satellite_status_info_type 卫星状态信息初始化状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_status_info_type, DEM_CODE)
dem_set_satellite_info_state(const dem_satellite_status_info_type satellite_info, const uint8 state);

/*!*********************************************************************************************************************
 *  @brief          重置事件队列更新
 *  @details        重置事件队列更新
 *  @param[in]      satellite_info 卫星信息
 *  @retval         dem_satellite_status_info_type 卫星状态信息
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_status_info_type, DEM_CODE)
dem_reset_event_queue_updated(const dem_satellite_status_info_type satellite_info);

/*!*********************************************************************************************************************
 *  @brief          设置卫星状态信息事件队列更新
 *  @details        设置卫星状态信息事件队列更新
 *  @param[in]      satellite_info 卫星信息
 *  @retval         dem_satellite_status_info_type 卫星状态信息
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_status_info_type, DEM_CODE)
dem_set_event_queue_updated(const dem_satellite_status_info_type satellite_info);

/*!*********************************************************************************************************************
 *  @brief          获取卫星状态信息初始化状态
 *  @details        获取卫星状态信息初始化状态
 *  @param[in]      satellite_info 卫星信息
 *  @retval         dem_satellite_status_info_type 卫星状态信息初始化状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(uint8, DEM_CODE) dem_get_satellite_info_state(const dem_satellite_status_info_type satellite_info);

/*!*********************************************************************************************************************
 *  @brief          获取事件队列更新
 *  @details        获取事件队列更新
 *  @param[in]      satellite_info 卫星信息
 *  @retval         TURE 事件队列已更新
 *  @retval         FALSE 事件队列未更新
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_get_event_queue_updated(const dem_satellite_status_info_type satellite_info);

/*!*********************************************************************************************************************
 *  @brief          获取卫星信息的状态
 *  @details        获取卫星信息的状态
 *  @param[in]      satellite_id 卫星id
 *  @retval         dem_atomic_word_type 处理类型指针
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC_P2VAR(dem_atomic_word_type, AUTOMATIC, DEM_CODE)
dem_get_satellite_status(const dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 *  @brief          获取卫星的事件计数
 *  @details        获取卫星的事件计数
 *  @param[in]      satellite_id 卫星id
 *  @retval         0-65535 事件计数
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_event_id_type, DEM_CODE) dem_get_satellite_event_count(const dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 *  @brief          获取卫星数据
 *  @details        获取卫星数据
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      satellite_event_id 卫星事件id
 *  @retval         dem_satellite_data_type 卫星数据
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_satellite_data_type, DEM_CODE)
dem_get_satellite_data(const dem_satellite_id_type satellite_id, const dem_satellite_event_id_type satellite_event_id);

/*!*********************************************************************************************************************
 *  @brief          获取事件应用id
 *  @details        获取事件应用id
 *  @param[in]      event_id 事件id
 *  @retval         0-255 事件应用id
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(ApplicationType, DEM_CODE) dem_get_event_application_id(const dem_event_id_type event_id);

/*!*********************************************************************************************************************
 *  @brief          是否是单卫星
 *  @details        是否是单卫星
 *  @retval         TURE 是单卫星
 *  @retval         FALSE 不是单卫星
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_single_satellite(void);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_SATELLITE_INFO_H */
