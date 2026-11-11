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

#ifndef DEM_DTC_H
#define DEM_DTC_H

#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_NO_DTC_DISABLED 0xFFFFFFFFul
#define IS_VALID_DTC_FORMAT(_x) ((DEM_DTC_FORMAT_UDS == (_x)) || (DEM_DTC_FORMAT_OBD == (_x)))
#define DEM_TO_OBD_FORMAT(_x) (((uint32)(_x)) << 8u) ///< 转换成OBD格式

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

FUNC_P2CONST(dem_dtc_class_type, AUTOMATIC, DEM_CODE)
dem_get_event_dtc_class(const dem_event_parameter_type *event_param);

FUNC(boolean, DEM_CODE) dem_is_dtc_available(const dem_dtc_class_type *dtc_class);

FUNC(boolean, DEM_CODE)
dem_is_dtc_format_match(const dem_event_parameter_type *event_param, dem_dtc_format_type dtc_format);

FUNC(boolean, DEM_CODE)
dem_is_dtc_origin_match(dem_dtc_origin_type dtc_origin, const dem_event_parameter_type *event_param,
		 boolean allow_permanent_memory);

FUNC(void, DEM_CODE)
dem_notify_dtc_status_changed(const dem_event_parameter_type *event_param, dem_event_status_rec_type old_status,
			  dem_event_status_rec_type new_status);

FUNC(boolean, DEM_CODE) dem_lookup_event_of_uds_dtc(uint32 dtc, dem_event_info_rec_type **event_info_rec);

FUNC(boolean, DEM_CODE)
dem_is_dtc_match_filter(const dem_dtc_filter_type *dtc_filter, const dem_event_info_rec_type *event_info_rec);

FUNC(boolean, DEM_CODE) dem_is_dtc_setting_disabled(const dem_event_parameter_type *event_param);

FUNC(std_return_type, DEM_CODE) dem_clear_dtc_process_single_dtc(const dem_dtc_selector_data_type *selector_ptr);

FUNC(std_return_type, DEM_CODE) dem_clear_dtc_process_group(const dem_dtc_selector_data_type *selector_ptr);

FUNC(std_return_type, DEM_CODE)
dem_clear_dtc_process_all_dtc(const dem_dtc_selector_data_type *selector_ptr);

FUNC(std_return_type, DEM_CODE) dem_clear_dtc_process_emission(const dem_dtc_selector_data_type *selector_ptr);

#if (DEM_CLEAR_DTC_NOTIFICATION_START == STD_ON)
FUNC(void, DEM_CODE)
dem_clear_dtc_notification_start(uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);
#endif

#if (DEM_CLEAR_DTC_NOTIFICATION_FINISH == STD_ON)
FUNC(void, DEM_CODE)
dem_clear_dtc_notification_finish(uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);
#endif

FUNC(std_return_type, DEM_CODE)
dem_is_dtc_match_format(uint32 dtc, dem_dtc_format_type dtc_format, const dem_dtc_class_type *dtc_class_ptr);

FUNC(boolean, DEM_CODE) dem_get_dtc_setting_status(void);

FUNC(boolean, DEM_CODE) dem_delete_dtc_data(const dem_event_parameter_type *event_param);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_DTC_H */
