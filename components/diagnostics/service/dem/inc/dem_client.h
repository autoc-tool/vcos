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

#ifndef DEM_CLIENT_H
#define DEM_CLIENT_H

/* including file */
#include "dem_types.h"
#include "dem_lcfg.h"
#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 * @brief          用户选择dtc
 * @details        用户选择dtc
 * @param[in]      client_id  用户id
 * @param[in]      DTC DTC
 * @param[in]      dtc_format dtc格式
 * @param[in]      dtc_origin dtc来源
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_client_id_select_dtc(uint8 client_id, uint32 DTC, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);

/*!*********************************************************************************************************************
 * @brief          获取用户选择结果
 * @details        none
 * @param[in]      client_id 用户id
 * @retval         selector的指针
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC_P2VAR(dem_dtc_selector_data_type, AUTOMATIC, DEM_CODE) dem_client_id_get_selection_result(uint8 client_id);

/*!*********************************************************************************************************************
 * @brief          设置用户filter
 * @details        设置用户filter
 * @param[in]      client_id 用户id
 * @param[in]      dtc_status_mask dtc状态掩码
 * @param[in]      dtc_format dtc格式
 * @param[in]      dtc_origin dtc来源
 * @param[in]      filter_with_severity 严重性过滤
 * @param[in]      dtc_severity_mask dtc严重等级掩码
 * @param[in]      filter_for_fault_detection_counter - fdc过滤
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_client_id_set_filter(uint8 client_id, dem_event_status_rec_type dtc_status_mask, dem_dtc_format_type dtc_format,
			 dem_dtc_origin_type dtc_origin, boolean filter_with_severity,
			 dem_dtc_severity_type dtc_severity_mask, boolean filter_for_fault_detection_counter);

/*!*********************************************************************************************************************
 * @brief          初始化所有的filter
 * @details        初始化所有的filter
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_init_filter(void);

/*!*********************************************************************************************************************
 * @brief          初始化所有的冻结帧记录filter
 * @details        初始化所有的冻结帧记录filter
 * @reentrant      TRUE/FALSE
 * @synchronous    TRUE/FALSE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_init_ff_record_filter(void);

/*!*********************************************************************************************************************
 * @brief          根据client_id获取对应的filter
 * @details        根据client_id获取对应的filter
 * @param[in]      client_id 用户id
 * @retval         filter的地址
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC_P2VAR(dem_dtc_filter_type, AUTOMATIC, DEM_CODE) dem_get_filter_by_client_id(uint8 client_id);

/*!*********************************************************************************************************************
 * @brief          根据client_id获取对应的冻结帧记录filter
 * @details        根据client_id获取对应的冻结帧记录filter
 * @param[in]      client_id 用户id
 * @retval         filter的地址
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC_P2VAR(dem_ff_record_filter_type, AUTOMATIC, DEM_CODE)
dem_get_ff_record_filter_by_client_id(uint8 client_id);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_CLIENT_H */
