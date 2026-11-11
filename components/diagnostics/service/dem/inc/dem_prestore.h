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

#ifndef DEM_PRESTORE_H
#define DEM_PRESTORE_H

#include "dem_lcfg.h"
#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_ENABLE_PRESTORED_FF == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          检查预存储冻结帧
 *  @details        检查预存储冻结帧
 *  @param[in]      event_id 事件id
 *  @retval         NULL - 该event不存在预存储冻结帧 / 非null则存在
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC_P2VAR(dem_prestore_entry_type, AUTOMATIC, DEM_CODE)
dem_is_ff_prestored(const dem_event_parameter_type *event_param);

/*!*********************************************************************************************************************
 *  @brief          处理预存储的冻结帧
 *  @details        处理预存储的冻结帧
 *  @param[in]      prestored_data 预存储的数据
 *  @param[in]      ff 冻结帧
 *  @param[in]      event_param 事件参数
 *  @param[in]      ff_kind 冻结帧类型
 *  @param[in]      origin dtc来源
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_process_prestored_ff(dem_prestore_entry_type *prestored_data, dem_ff_rec_type *ff,
			 const dem_event_parameter_type *event_param, dem_ff_kind_type ff_kind,
			 dem_dtc_origin_type origin);

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          处理预存储的obd冻结帧
 *  @details        处理预存储的obd冻结帧
 *  @param[in]      prestored_data 预存储的数据
 *  @param[in]      obd_ff_index obd冻结帧索引
 *  @param[in]      event_param 事件参数
 *  @reentrant      TRUE
 *  @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE)
dem_process_prestored_obd_ff(dem_prestore_entry_type *prestored_data, uint8 obd_ff_index,
			     const dem_event_parameter_type *event_param);
#endif

FUNC(std_return_type, DEM_CODE) dem_clear_prestored_ff_entry(dem_prestore_entry_type *entry_ptr);
FUNC_P2VAR(dem_prestore_entry_type, AUTOMATIC, DEM_CODE)
dem_get_prestore_ff_entry_for_updating(const dem_event_id_type event_id);

FUNC(std_return_type, DEM_CODE)
dem_collect_prestored_ff_data(dem_prestore_entry_type *entry_ptr, const dem_event_parameter_type *event_param);

FUNC_P2VAR(dem_prestore_entry_type, AUTOMATIC, DEM_CODE)
dem_get_prestore_ff_entry(const dem_event_id_type event_id);
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_PRESTORE_H */
