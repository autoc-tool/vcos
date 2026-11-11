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

#ifndef DEM_STATE_H
#define DEM_STATE_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 *  @brief          获取主状态
 *  @details        获取主状态
 *  @retval         0-255 主状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_state_type, DEM_CODE) dem_get_master_state(void);

/*!*********************************************************************************************************************
 *  @brief          设置主状态
 *  @details        设置主状态
 *  @param[in]      state 状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_set_master_state(dem_state_type state);

/*!*********************************************************************************************************************
 *  @brief          主是否处于参数state状态
 *  @details        主是否处于参数state状态
 *  @param[in]      state 状态
 *  @retval         TRUE 主处于参数state状态
 *  @retval         FALSE 主未处于参数state状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_master_in_state(const dem_state_type state);

/*!*********************************************************************************************************************
 *  @brief          主是否至少处于参数state状态
 *  @details        主是否至少处于参数state状态
 *  @param[in]      state 状态
 *  @retval         TRUE 主至少处于参数state状态
 *  @retval         FALSE 主不处于参数state状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_master_at_least_in_state(const dem_state_type state);

/*!*********************************************************************************************************************
 *  @brief          卫星是否处于参数state状态
 *  @details        卫星是否处于参数state状态
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      state 状态
 *  @retval         TRUE 卫星处于参数state状态
 *  @retval         FALSE 卫星未处于参数state状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_satellite_in_state(const dem_satellite_id_type satellite_id, const dem_state_type state);

/*!*********************************************************************************************************************
 *  @brief          卫星是否至少处于参数state状态
 *  @details        卫星是否至少处于参数state状态
 *  @param[in]      satellite_id 卫星id
 *  @param[in]      state 状态
 *  @retval         TRUE 卫星至少处于参数state状态
 *  @retval         FALSE 卫星未处于参数state状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE)
dem_is_satellite_at_least_in_state(const dem_satellite_id_type satellite_id, const dem_state_type state);

/*!*********************************************************************************************************************
 *  @brief          所有卫星是否至少处于参数state状态
 *  @details        所有卫星是否至少处于参数state状态
 *  @param[in]      state 状态
 *  @retval         TRUE 所有卫星至少处于参数state状态
 *  @retval         FALSE 所有卫星未处于参数state状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_all_satellites_at_least_in_state(const dem_state_type state);

/*!*********************************************************************************************************************
 *  @brief          事件所在的卫星是否处于参数state状态
 *  @details        事件所在的卫星是否处于参数state状态
 *  @param[in]      event_id 事件id
 *  @param[in]      state 状态
 *  @retval         TRUE 事件所在的卫星处于参数state状态
 *  @retval         FALSE 事件所在的卫星未处于参数state状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_event_satellite_in_state(const dem_event_id_type event_id, const dem_state_type state);

#if (DEM_USE_CONFIGPTR == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          获取全局配置指针
 *  @details        获取全局配置指针
 *  @retval         dem_config_type 配置指针
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC_P2CONST(dem_config_type, AUTOMATIC, DEM_CODE) dem_get_global_config_ptr(void);

/*!*********************************************************************************************************************
 *  @brief          设置全局配置指针
 *  @details        设置全局配置指针
 *  @param[in]      config_ptr 配置指针
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_set_global_config_ptr(const dem_config_type *config_ptr);

/*!*********************************************************************************************************************
 *  @brief          获取全局配置指针状态
 *  @details        获取全局配置指针状态
 *  @retval         0-255 全局配置指针状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(const dem_config_ptr_state_type, DEM_CODE) dem_get_global_config_ptr_state(void);

/*!*********************************************************************************************************************
 *  @brief          设置全局配置指针状态
 *  @details        设置全局配置指针状态
 *  @param[in]      global_config_ptr_state 全局配置指针状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_set_global_config_ptr_state(dem_config_ptr_state_type global_config_ptr_state);

/*!*********************************************************************************************************************
 *  @brief          全局配置指针是否处于参数对应的状态
 *  @details        全局配置指针是否处于参数对应的状态
 *  @retval         TRUE 全局配置指针处于该状态
 *  @retval         FALSE 全局配置指针未处于该状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_global_cfg_ptr_in_state(dem_config_ptr_state_type global_config_ptr_state);

/*!*********************************************************************************************************************
 *  @brief          全局配置指针是否至少处于参数对应的状态
 *  @details        全局配置指针是否至少处于参数对应的状态
 *  @param[in]      global_config_ptr_state 全局配置指针状态
 *  @retval         TRUE 全局配置指针至少处于该状态
 *  @retval         FALSE 全局配置指针未处于该状态
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE) dem_is_global_cfg_ptr_at_least_in_state(dem_config_ptr_state_type global_config_ptr_state);
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_STATE_H */
