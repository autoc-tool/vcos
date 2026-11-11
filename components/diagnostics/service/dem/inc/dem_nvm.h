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

#ifndef DEM_NVM_H
#define DEM_NVM_H

#include "dem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 * @brief          设置事件块改变
 * @details        设置事件块改变
 * @param[in]      origin 来源
 * @param[in]      event_id 事件id
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_nvm_set_event_block_changed(dem_dtc_origin_type origin, dem_event_id_type event_id, boolean immediate_storage);

/*!*********************************************************************************************************************
 * @brief          设置事件块改变
 * @details        设置事件块改变
 * @param[in]      origin 来源
 * @param[in]      event_buff_index 事件缓存索引
 * @param[in]      event_id 事件id
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_nvm_set_event_block_changed_with_indx(dem_dtc_origin_type origin, uint16 event_buff_index,
					  dem_event_id_type event_id, boolean immediate_storage);

/*!*********************************************************************************************************************
 * @brief          设置管理块改变
 * @details        设置管理块改变
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_set_mgmt_blcok_changed(boolean immediate_storage);

#if (DEM_CFG_DTC_AGED_COUNTER == STD_ON)
/*!*********************************************************************************************************************
 * @brief          设置老化数据块改变
 * @details        设置老化数据块改变
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_set_aged_data_blcok_changed(boolean immediate_storage);
#endif

/*!*********************************************************************************************************************
 * @brief          设置事件状态数据块改变
 * @details        设置事件状态数据块改变
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_set_event_status_data_blcok_changed(boolean immediate_storage);

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          设置事件可用数据块改变
 * @details        设置事件可用数据块改变
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_set_event_available_data_blcok_changed(boolean immediate_storage);
#endif

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
/*!*********************************************************************************************************************
 * @brief          设置永久数据块改变
 * @details        设置永久数据块改变
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_set_permanent_block_changed(boolean immediate_storage);

/*!*********************************************************************************************************************
 * @brief          设置obd冻结帧块改变
 * @details        设置obd冻结帧块改变
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_set_obd_ff_block_changed(boolean immediate_storage);

/*!*********************************************************************************************************************
 * @brief          设置indicator status块改变
 * @details        设置indicator status冻结帧块改变
 * @param[in]      immediate_storage 立即储存
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_set_indicator_status_block_changed(boolean immediate_storage);
#endif

#if (CONFIG_MODULE_NVM == 1) && (DEM_USE_NVM == STD_ON)
/*!*********************************************************************************************************************
 * @brief         dem nvm初始化
 * @details        dem nvm初始化
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_init(void);
/*!*********************************************************************************************************************
 * @brief         dem nvm主函数
 * @details        dem nvm主函数
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_main_function(void);
/*!*********************************************************************************************************************
 * @brief         dem nvm关闭
 * @details        dem nvm关闭
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_nvm_shutdown(void);
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_NVM_H */
