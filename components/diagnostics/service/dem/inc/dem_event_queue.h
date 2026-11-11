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

#ifndef DEM_EVENT_QUEUE_H
#define DEM_EVENT_QUEUE_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg.h"
#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_EVENTQUEUE_RESET_TESTFAILED_FLAG ((uint8)(0x80u)) ///< 队列重置测试失败
#define DEM_EVENTQUEUE_EVENT_LAST_FAILED_STATUS_MASK  ((uint8)(0x40u)) ///< 置位则表示Fail,否则Pass
#define DEM_EVENTQUEUE_EVENT_STATUS_NUM_MASK ((uint8)(0x0Fu)) ///< 队列中存储的变化的状态个数位置掩码
#define DEM_EVENTQUEUE_QUEUED_STATUS_MAX_NUM ((uint8)10u) ///< 队列中存储的变化的状态个数最大值
#define DEM_MAINFUNC_HANDLE_EVENT_STATUS_KINDS ((uint8)2u) ///< 主要处理的事件状态种类（Fail和Pass）

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 * @brief          主周期中处理event_queue中元素的函数 - 周期处理所有satellite上报的故障
 * @details        从环形队列中取出当前所有的信息，依次处理
 * @reentrant      FALSE
 * @synchronous    FALSE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_precess_event_queue_main_function(void);

/*!*********************************************************************************************************************
 * @brief          事件信息入队
 * @details        事件信息入队
 * @param[in]      satellite_id 卫星id
 * @param[in]      event_id 事件id
 * @param[in]      event_status 事件状态
 * @retval         E_OK 事件信息入队成功
 * @retval         E_NOT_OK 事件信息入队失败
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_push_event_info_to_queue(dem_satellite_id_type satellite_id, dem_event_id_type event_id,
				     dem_event_status_type event_status);

/*!*********************************************************************************************************************
 * @brief          获取初始化队列状态
 * @details        获取初始化队列状态
 * @retval         0-255 状态
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_queue_status_type, DEM_CODE) dem_get_initial_queue_status(void);

/*!*********************************************************************************************************************
 * @brief          计算队列状态
 * @details        计算队列状态
 * @param[in]      queue_status 队列状态
 * @param[in]      event_status 事件状态
 * @retval         0-255 队列状态
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_queue_status_type, DEM_CODE)
dem_calculate_queue_status(const dem_event_queue_status_type queue_status,
				       const dem_event_status_type event_status);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_EVENT_QUEUE_H */
