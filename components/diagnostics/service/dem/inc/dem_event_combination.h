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

#ifndef DEM_EVENT_COMBINATION_H
#define DEM_EVENT_COMBINATION_H

#include "dem.h"
#include "dem_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          计算组合组dtc抑制状态
 * @details        计算组合组dtc抑制状态
 * @param[in]      group_index  组索引
 * @retval         TRUE/FALSE  抑制/不抑制
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_calculate_combined_group_dtc_suppressed_status(uint8 group_index);
#endif

/*!*********************************************************************************************************************
 * @brief          获取开始组合事件索引
 * @details        获取开始组合事件索引
 * @param[in]      group_index  组索引
 * @retval         0-255  开始组合事件索引
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(uint8, DEM_CODE) dem_get_combined_event_start_index(uint8 group_index);

/*!*********************************************************************************************************************
 * @brief          获取组合事件组索引
 * @details        获取组合事件组索引
 * @param[in]      event_id  事件id
 * @retval         0-255  组合事件组索引
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(uint8, DEM_CODE) dem_get_event_combined_group_index(dem_event_id_type event_id);

/*!*********************************************************************************************************************
 * @brief          事件是否已组合
 * @details        事件是否已组合
 * @param[in]      event_id  事件id
 * @retval         TRUE 事件已组合
 * @retval         FALSE 事件未组合
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_combined_event(dem_event_id_type event_id);

/*!*********************************************************************************************************************
 * @brief          获取主事件
 * @details        获取主事件
 * @param[in]      event_id  事件id
 * @retval         0-65535 事件id
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_id_type, DEM_CODE) dem_get_master_event(dem_event_id_type event_id);

/*!*********************************************************************************************************************
 * @brief          获取事件组合组uds状态
 * @details        获取事件组合组uds状态
 * @param[in]      group_index  组索引
 * @retval         0-255 事件组合组uds状态
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(uint8, DEM_CODE) dem_get_event_combined_group_uds_status(uint8 group_index);

/*!*********************************************************************************************************************
 * @brief          设置事件组合组uds状态
 * @details        设置事件组合组uds状态
 * @param[in]      group_index  组索引
 * @param[in]      uds_status  uds状态
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_set_event_combined_group_uds_status(uint8 group_index, uint8 uds_status);

/*!*********************************************************************************************************************
 * @brief          获取事件组合组事件id
 * @details        获取事件组合组事件id
 * @param[in]      group_index  组索引
 * @param[in]      combined_event_index  组合事件索引
 * @retval         0-65535 事件组合组事件id
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_event_id_type, DEM_CODE) dem_get_event_combined_group_event_id(uint8 group_index, uint8 combined_event_index);

/*!*********************************************************************************************************************
 * @brief          计算事件组合组uds状态
 * @details        计算事件组合组uds状态
 * @param[in]      group_index  组索引
 * @retval         0-255 事件组合组uds状态
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(uint8, DEM_CODE) dem_calculate_event_combine_group_uds_status(uint8 group_index);

/*!*********************************************************************************************************************
 * @brief          处理事件组合组状态改变
 * @details        处理事件组合组状态改变
 * @param[in]      group_index  组索引
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_handle_event_combined_group_status_changed(uint8 group_index);

/*!*********************************************************************************************************************
 * @brief          是否允许清除事件组合储存
 * @details        是否允许清除事件组合储存
 * @param[in]      group_index  组索引
 * @retval         TRUE 允许
 * @retval         FALSE 不允许
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_event_combination_on_storage_clear_allowed(uint8 group_index);

#if (DEM_SUPPORT_EVENTCOMBINATION_ONSTORAGE == STD_ON)
/*!*********************************************************************************************************************
 * @brief          清除事件组合
 * @details        清除事件组合
 * @param[in]      group_index  组索引
 * @param[in]      reset_event_record  重置事件记录
 * @retval         TRUE 允许
 * @retval         FALSE 不允许
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE)
dem_clear_combined_event(uint8 group_index, boolean reset_event_record, boolean delete_dtc_data);
#endif

/*!*********************************************************************************************************************
 * @brief          获取事件组合组fdc
 * @details        获取事件组合组fdc
 * @param[in]      event_id  事件id
 * @retval         -128-127 状态
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(sint8, DEM_CODE) dem_get_event_combined_group_fdc(dem_event_id_type event_id);

#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_EVENT_COMBINATION_H */
