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

#ifndef DEM_INTERNAL_H
#define DEM_INTERNAL_H

#include "dem.h"
#include "dem_lcfg_internal.h"
#include "dem_age.h"
#include "dem_atomic.h"
#include "dem_client.h"
#include "dem_debounce.h"
#include "dem_dtc.h"
#include "dem_event_combination.h"
#include "dem_event_process.h"
#include "dem_event_queue.h"
#include "dem_externed_data.h"
#include "dem_freeze_frame.h"
#include "dem_indicator.h"
#include "dem_nvm.h"
#include "dem_operation_cycle.h"
#include "dem_prestore.h"
#include "dem_satellite.h"
#include "dem_satellite_data.h"
#include "dem_satellite_info.h"
#include "dem_state.h"
#include "dem_storage.h"
#include "rtfw_atomic.h"
#include "SchM_Dem.h"

#if (CONFIG_MODULE_RTE == 1)
#include "Rte_Type.h"
#endif
#include "vcos_log.h"
#if (DEM_DEV_ERROR_DET_SUPPORT == STD_ON) && (CONFIG_MODULE_DET == 1)
#include "Det.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_1_BYTE 1 ///< 1个字节
#define DEM_2_BYTE 2 ///< 2个字节
#define DEM_4_BYTE 4 ///< 4个字节

#define DEM_MASK_1_BYTE 0xffu ///< 1个字节掩码
#define DEM_MASK_2_BYTE 0xffffu ///< 2个字节掩码
#define DEM_MASK_4_BYTE 0xfffffffful ///< 4个字节掩码

#define DEM_MASK(size) DEM_MASK_##size##_BYTE ///< 掩码大小
#define DEM_BIT(pos) (0x01ul << (pos)) ///< 比特
#define DEM_BIT_MASK(pos, size) (DEM_MASK(size) ^ DEM_BIT(pos)) ///< 比特掩码
#define DEM_N_BIT(n, pos) ((DEM_MASK(1) >> (8U - (n))) << (pos)) ///< n比特

#define DEM_IS_MASK_SET(src, mask, type) ((((type)(src)) & ((type)(mask))) != 0)
#define DEM_IS_MASK_FULL_SET(src, mask, type) ((((type)(src)) & ((type)(mask))) == ((type)(mask)))
#define DEM_IS_MASK_UNSET(src, mask, type) ((((type)(src)) & ((type)(mask))) == 0u)
#define DEM_SET_MASK(src, mask, type) ((src) |= ((type)(mask)))
#define DEM_CLEAR_MASK(src, mask, type) ((src) &= ((type)(~((type)(mask)))))

#define DEM_SATELLITEINFO_EVENT_QUEUE_STATUS_POS (0u) ///< 事件队列状态起始
#define DEM_SATELLITEINFO_EVENT_QUEUE_STATUS_SIZE (1u) ///< 事件队列状态大小

#define DEM_SATELLITEINFO_EVENT_QUEUE_STATUS                                                                           \
	DEM_N_BIT(DEM_SATELLITEINFO_EVENT_QUEUE_STATUS_SIZE, DEM_SATELLITEINFO_EVENT_QUEUE_STATUS_POS) ///< 队列状态

#define DEM_UNKNOWN_GROUP_ID 0xFFu ///< 未知组id

#define DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT (UINT32_MAX) ///< 最大重排时间戳
#define DEM_MAX_TIMESTAMP_FOR_PRE_INIT ((uint32)(UINT32_MAX / 2u)) ///< 最大预初始化时间戳

#define SHIFT_RIGHT_ONE_U16(value) ((uint16)((value) >> 1U))

#if (DEM_DEV_ERROR_DET_SUPPORT == STD_ON) && (CONFIG_MODULE_DET == 1)
#define DEM_DET_REPORT(errid)                                                                                          \
	do {                                                                                                           \
		(void)det_report_error_with_info((uint32)(errid), NULL_PTR, 0);                                        \
	} while (0) ///< 错误信息上报
#else
#define DEM_DET_REPORT(errid) ///< 错误信息不上报
#endif

#ifndef DEM_VALIDATE_RV
#define DEM_VALIDATE_RV(_exp, errid, _rv)                                                                              \
	do {                                                                                                           \
		if (!(_exp)) {                                                                                         \
			DEM_DET_REPORT(errid);                                                                         \
			VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "errno:%u", errid);                                    \
			return _rv;                                                                                    \
		}                                                                                                      \
	} while (0) ///< 有返回值校验
#endif

#ifndef DEM_VALIDATE_NO_RV
#define DEM_VALIDATE_NO_RV(_exp, errid)                                                                                \
	do {                                                                                                           \
		if (!(_exp)) {                                                                                         \
			DEM_DET_REPORT(errid);                                                                         \
			VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "errno:%u", errid);                                    \
			return;                                                                                        \
		}                                                                                                      \
	} while (0) ///< 有返回值校验
#endif

#ifndef DEM_REPORT_ERROR_RV
#define DEM_REPORT_ERROR_RV(errid, _rv)                                                                                \
	do {                                                                                                           \
		DEM_DET_REPORT(errid);                                                                                 \
		VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "errno:%u", errid);                                            \
		return _rv;                                                                                            \
	} while (0) ///< 有返回报告错误
#endif

#ifndef DEM_REPORT_ERROR_NO_RV
#define DEM_REPORT_ERROR_NO_RV(errid)                                                                                  \
	do {                                                                                                           \
		DEM_DET_REPORT(errid);                                                                                 \
		VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "errno:%u", errid);                                            \
		return;                                                                                                \
	} while (0) ///< 无返回报告错误
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b)) ///< 最小值
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b)) ///< 最大值
#endif

#define DEM_IS_VALID_EVENT_ID(_x) (((_x) > 0u) && ((_x) <= DEM_EVENT_ID_LAST_VALID_ID))

#define DEM_ENTER_EXCLUSIVE_AREA_0 SchM_Enter_DEM_EXCLUSIVE_AREA_0
#define DEM_EXIT_EXCLUSIVE_AREA_0 SchM_Exit_DEM_EXCLUSIVE_AREA_0

#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
/*!*********************************************************************************************************************
 * @brief          永久dtc类型
 **********************************************************************************************************************/
typedef struct {
	uint32 uds_dtc; ///< uds的dtc
	uint16 obd_dtc; ///< obd的dtc
} permanent_dtc_type;
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

static inline FUNC(void, DEM_CODE)
	lookup_event_info_rec(dem_event_id_type event_id, dem_event_info_rec_type **const event_info_rec)
{
	*event_info_rec = (DEM_IS_VALID_EVENT_ID(event_id) ? &g_dem_event_info_rec[event_id - 1u] : NULL);
}

static inline FUNC(void, DEM_CODE)
	lookup_event_param(dem_event_id_type event_id, const dem_event_parameter_type **const event_id_param)
{
	*event_id_param = (DEM_IS_VALID_EVENT_ID(event_id) ? &g_dem_config_set->event_parameter[event_id - 1u] : NULL);
}

static inline FUNC(boolean, DEM_CODE) is_opcycle_started(dem_opcycle_id_type opcycle)
{
	if ((opcycle < DEM_OPERATION_CYCLE_ID_ENDMARK) &&
	    (g_dem_opcycle_state_list[opcycle] == DEM_CYCLE_STATE_START)) {
		return TRUE;
	}

	return FALSE;
}

static inline FUNC_P2CONST(dem_config_set_type, AUTOMATIC, DEM_CODE) dem_get_config_set(void)
{
	return g_dem_config_set;
}

static inline FUNC_P2CONST(dem_general_type, AUTOMATIC, DEM_CODE) dem_get_general_ptr(void)
{
	return g_dem_general_set;
}

#define dem_is_opcycle_started is_opcycle_started
#define dem_lookup_event_param lookup_event_param
#define dem_lookup_event_info_rec lookup_event_info_rec

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_INTERNAL_H */
