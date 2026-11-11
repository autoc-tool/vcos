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

#include "dem_internal.h"
#include "Compiler.h"

#define DEM_START_SEC_MULTI_APP_BSS
#include "dem_memmap.h"

static VAR(uint16, DEM_MULTI_APP_BSS) dem_opcycle_pending_cycle_state;
static volatile VAR(uint16, DEM_MULTI_APP_BSS) dem_opcycle_cycle_flag;

#define DEM_STOP_SEC_MULTI_APP_BSS
#include "dem_memmap.h"

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

#if (DEM_SUPPORT_CYCLES_COUNTER == STD_ON)
static inline FUNC(void, DEM_CODE) handle_cycle_counters_end(dem_event_info_rec_type *event_info_rec)
{
#if (DEM_CFG_DATA_CYCLES_SINCE_LAST_FAILED == STD_ON)
	if (event_info_rec->cycles_since_last_failed_start == (boolean)TRUE) {
		if (event_info_rec->cycles_since_last_failed < DEM_CYCLE_CNTR_MAX) {
			event_info_rec->cycles_since_last_failed++;
		}
	}
#endif

#if (DEM_CFG_DATA_CYCLES_SINCE_FIRST_FAILED == STD_ON)
	if (event_info_rec->cycles_since_first_failed_start == (boolean)TRUE) {
		if (event_info_rec->cycles_since_first_failed < DEM_CYCLE_CNTR_MAX) {
			event_info_rec->cycles_since_first_failed++;
		}
	}
#endif

#if (DEM_CFG_DATA_FAILED_CYCLES == STD_ON)
	if ((event_info_rec->failed_cycles_start == (boolean)TRUE) &&
	    DEM_IS_MASK_SET(event_info_rec->event_status_rec, DEM_TEST_FAILED_THIS_OPERATION_CYCLE,
			    dem_event_status_rec_type)) {
		if (event_info_rec->failed_cycles < DEM_CYCLE_CNTR_MAX) {
			event_info_rec->failed_cycles++;
		}
	}
#endif
}
#endif

static FUNC(void, DEM_CODE) opcycle_start(dem_opcycle_id_type opcycle_id)
{
	dem_event_status_rec_type old_status;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = old_combined_status;
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
#endif

	g_dem_opcycle_state_list[opcycle_id] = DEM_CYCLE_STATE_START;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		event_id = g_dem_event_info_rec[i].event_id;
		lookup_event_param(event_id, &event_param);

		if (NULL == event_param) {
			continue;
		}

		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
		if ((event_param->event_class_ref->opcycle_ref == opcycle_id) &&
		    (TRUE == g_dem_event_info_rec[i].is_available)) {
#else
		if (event_param->event_class_ref->opcycle_ref == opcycle_id) {
#endif
			old_status = g_dem_event_info_rec[i].event_status_rec;
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			group_index = dem_get_event_combined_group_index(g_dem_event_info_rec[i].event_id);
			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				old_combined_status = dem_get_event_combined_group_uds_status(group_index);
			}
#endif
			DEM_CLEAR_MASK(g_dem_event_info_rec[i].event_status_rec,
				     DEM_TEST_FAILED_THIS_OPERATION_CYCLE, dem_event_status_rec_type);
			DEM_SET_MASK(g_dem_event_info_rec[i].event_status_rec,
				     DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type);

			dem_reset_debounce_counter(&g_dem_event_info_rec[i]);

			if (old_status != g_dem_event_info_rec[i].event_status_rec) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
				if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
					/* event状态发生改变，更新事件组状态 */
					dem_handle_event_combined_group_status_changed(group_index);
					new_combined_status = dem_get_event_combined_group_uds_status(group_index);
					if (old_combined_status != new_combined_status) {
						dem_notify_dtc_status_changed(event_param, old_combined_status,
									  new_combined_status);
					}
				} else {
					dem_notify_dtc_status_changed(event_param, old_status,
								  g_dem_event_info_rec[i].event_status_rec);
				}
#else
				{
					dem_notify_dtc_status_changed(event_param, old_status,
								  g_dem_event_info_rec[i].event_status_rec);
				}
#endif
				dem_notify_event_status_change(event_param, old_status,
							   g_dem_event_info_rec[i].event_status_rec);
			}
#if (DEM_CALLBACK_INIT_M_FOR_E_SUPPORT == STD_ON)
			if (NULL != event_param->callback_init_mfor_e) {
				(void)event_param->callback_init_mfor_e(DEM_INIT_RESTART_MONITOR);
			}
#endif
		}

		if (event_param->event_class_ref->failure_cycle_ref == opcycle_id) {
			g_dem_event_info_rec[i].passed_during_failure_cycle = FALSE;
			g_dem_event_info_rec[i].failed_during_failure_cycle = FALSE;
		}
#if (DEM_SUPPORT_AGING == STD_ON)
		if (event_param->event_class_ref->aging_cycle_ref == opcycle_id) {
			g_dem_event_info_rec[i].passed_during_aging_cycle = FALSE;
			g_dem_event_info_rec[i].failed_during_aging_cycle = FALSE;
		}
#endif
	}
#if defined(DEM_USE_INDICATORS)
	dem_indicator_opcycle_start(opcycle_id);
#endif
}

static FUNC(boolean, DEM_CODE)
	handle_opcycle_end(dem_opcycle_id_type opcycle_id, dem_event_info_rec_type *event_info,
				boolean *permanent_memory_updated, uint16 *nof_erased_permanent_dtcs,
				boolean *clear_obd_evt_mem)
{
	dem_event_status_rec_type old_status = event_info->event_status_rec;
	boolean store_evt_mem = FALSE;
	const dem_event_parameter_type *event_param = NULL;

#if defined(DEM_USE_INDICATORS)
	(void)dem_indicator_opcycle_end(opcycle_id, event_info);
#endif
#if (DEM_SUPPORT_CYCLES_COUNTER == STD_ON)
	handle_cycle_counters_end(event_info);
#endif
#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
	if (dem_handle_permanent_dtc_erase(event_info, opcycle_id) == TRUE) {
		(*nof_erased_permanent_dtcs)++;
		(*permanent_memory_updated) = TRUE;
	}
	dem_handle_perm_status_on_opcycle_end(opcycle_id, event_info);
#else
	(void)permanent_memory_updated;
	(void)nof_erased_permanent_dtcs;
#endif

	event_info->fdc_this_opcycle = (boolean)FALSE;

	lookup_event_param(event_info->event_id, &event_param);

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
	if (DEM_EMISSION_REL_DTCS_KIND == event_param->event_dtc_kind) {
		if (DEM_IS_MASK_SET(event_info->event_status_rec, DEM_PENDING_DTC, dem_event_status_rec_type) &&
		    DEM_IS_MASK_UNSET(event_info->event_status_rec, DEM_CONFIRMED_DTC, dem_event_status_rec_type) &&
		    DEM_IS_MASK_UNSET(event_info->event_status_rec, DEM_TEST_FAILED_THIS_OPERATION_CYCLE |
				      DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type)) {
			DEM_CLEAR_MASK(event_info->event_status_rec, DEM_PENDING_DTC, dem_event_status_rec_type);
			*clear_obd_evt_mem = TRUE;
		}
	}
#endif

	if (DEM_IS_MASK_UNSET(event_info->event_status_rec, DEM_TEST_FAILED_THIS_OPERATION_CYCLE |
			      DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE, dem_event_status_rec_type)) {
		if (event_param->event_class_ref->opcycle_ref == opcycle_id) {
			DEM_CLEAR_MASK(event_info->event_status_rec, DEM_PENDING_DTC,
				       dem_event_status_rec_type);
			if (old_status != event_info->event_status_rec) {
				store_evt_mem = TRUE;
			}
		}
	}

	if ((event_param->event_class_ref->failure_cycle_ref == opcycle_id) &&
	    event_info->passed_during_failure_cycle && !event_info->failed_during_failure_cycle) {
		if (0u != event_info->failure_counter) {
			event_info->failure_counter = 0;
			store_evt_mem = TRUE;
		}
	}

	return store_evt_mem;
}

static FUNC(void, DEM_CODE) opcycle_end(dem_opcycle_id_type opcycle_id)
{
	dem_event_status_rec_type old_status;
	dem_primary_mem_type *event_rec = NULL;
	boolean permanent_memory_updated = FALSE;
	uint16 nof_erased_permanent_dtcs = 0;
	dem_event_id_type event_id = DEM_EVENT_ID_NULL;
	const dem_event_parameter_type *event_param = NULL;

#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
	dem_event_status_rec_type old_combined_status = 0u;
	dem_event_status_rec_type new_combined_status = old_combined_status;
	uint8 group_index = DEM_EVENT_COMBINED_INVALID_GROUP_INDEX;
#endif

	g_dem_opcycle_state_list[opcycle_id] = DEM_CYCLE_STATE_END;

	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		boolean store_evt_mem = FALSE;
		boolean clear_obd_evt_mem = FALSE;

		event_id = g_dem_event_info_rec[i].event_id;
		if (DEM_EVENT_ID_NULL == event_id) {
			continue;
		}

		lookup_event_param(event_id, &event_param);
		if (NULL == event_param) {
			continue;
		}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
		if (FALSE == g_dem_event_info_rec[i].is_available) {
			continue;
		}
#endif
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
		group_index = dem_get_event_combined_group_index(g_dem_event_info_rec[i].event_id);
		if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
			old_combined_status = dem_get_event_combined_group_uds_status(group_index);
		}
#endif
		old_status = g_dem_event_info_rec[i].event_status_rec;
		store_evt_mem = handle_opcycle_end(opcycle_id, &g_dem_event_info_rec[i], &permanent_memory_updated,
							&nof_erased_permanent_dtcs, &clear_obd_evt_mem);
		if (old_status != g_dem_event_info_rec[i].event_status_rec) {
#if (DEM_SUPPORT_EVENTCOMBINATION == STD_ON)
			if (DEM_EVENT_COMBINED_INVALID_GROUP_INDEX != group_index) {
				/* event状态发生改变，更新事件组状态 */
				dem_handle_event_combined_group_status_changed(group_index);
				new_combined_status = dem_get_event_combined_group_uds_status(group_index);
				if (old_combined_status != new_combined_status) {
					dem_notify_dtc_status_changed(event_param, old_combined_status,
								  new_combined_status);
				}
			} else {
				dem_notify_dtc_status_changed(event_param, old_status,
							  g_dem_event_info_rec[i].event_status_rec);
			}
#else
			{
				dem_notify_dtc_status_changed(event_param, old_status,
							  g_dem_event_info_rec[i].event_status_rec);
			}
#endif
			dem_notify_event_status_change(event_param, old_status,
						       g_dem_event_info_rec[i].event_status_rec);
		}

#if (DEM_CFG_ENABLE_OBDII == STD_ON)
		dem_clear_obd_storage(event_param, clear_obd_evt_mem);
#endif

		if (FALSE == store_evt_mem) {
			continue;
		}

		if (DEM_EMISSION_REL_DTCS_KIND != event_param->event_dtc_kind) {
			if (dem_store_event_mem(event_param, &g_dem_event_info_rec[i], FALSE, &event_rec) == E_OK) {
				dem_notify_event_storage_changed(event_param);
			}
		}
	}
#if ((DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON) && (DEM_OBD_SUPPORT == STD_ON))
	if (TRUE == permanent_memory_updated) {
		dem_transfer_event_to_permanent(nof_erased_permanent_dtcs);
#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
		dem_nvm_set_permanent_block_changed(TRUE);
#else
		dem_nvm_set_permanent_block_changed(FALSE);
#endif
	}
#endif

	return;
}

static FUNC(uint8, DEM_CODE) get_current_opcycle_status(uint8 cycle_id)
{
	return (uint8)((dem_is_opcycle_started(cycle_id) == TRUE) ? DEM_PENDING_CYCLE_STATE_STARTED :
								    DEM_PENDING_CYCLE_STATE_STOPPED);
}

/**
 * @brief 处理操作周期状态转换
 *
 * 根据新旧状态执行周期转换：
 * - 启动->启动: 重启周期（先结束再启动）
 * - 停止->启动: 直接启动
 * - 启动->停止: 结束周期
 * - 停止->停止: 无操作
 *
 * @param opcycle_id                 操作周期ID
 * @param operation_queue_cycle_state 状态位域（最低位：1-启动，0-停止）
 * @return E_OK-成功; E_NOT_OK-失败
 */
static FUNC(std_return_type, DEM_CODE)
	handle_opcycle_impl(const uint8 opcycle_id, const uint16 operation_queue_cycle_state)
{
	std_return_type ret = E_OK;

	/* 提取新状态：检查最低位 */
	uint8 new_status = (uint8)(DEM_IS_MASK_SET(operation_queue_cycle_state, 0x01u, uint16) ?
					   DEM_PENDING_CYCLE_STATE_STARTED :
						 DEM_PENDING_CYCLE_STATE_STOPPED);

	/* 获取当前状态 */
	uint8 old_status = get_current_opcycle_status(opcycle_id);

	/* 执行状态转换 */
	if (DEM_PENDING_CYCLE_STATE_STARTED == new_status) {
		if (DEM_PENDING_CYCLE_STATE_STARTED == old_status) {
			/* 重启：先结束再启动 */
			ret = dem_handle_set_opcycle_state(opcycle_id, DEM_CYCLE_STATE_END);
			if (E_OK == ret) {
				ret = dem_handle_set_opcycle_state(opcycle_id, DEM_CYCLE_STATE_START);
			}
		} else {
			/* 直接启动 */
			ret = dem_handle_set_opcycle_state(opcycle_id, DEM_CYCLE_STATE_START);
		}
	} else {
		if (DEM_PENDING_CYCLE_STATE_STARTED == old_status) {
			/* 结束运行中的周期 */
			ret = dem_handle_set_opcycle_state(opcycle_id, DEM_CYCLE_STATE_END);
		}
	/* 已停止则无需操作 */
	}
	return ret;
}

/**
 * @brief 批量处理待处理的操作周期状态变更
 *
 * 从全局队列获取待处理的操作周期请求并逐个处理。
 * 使用位域管理多个周期：每位对应一个周期，通过位移遍历。
 *
 * @note 处理前会清空全局队列，避免重复处理
 *       最多支持16个操作周期（uint16位数）
 */
static FUNC(void, DEM_CODE) handle_opcycles(void)
{
	uint8 opcycle_id = 0u;
	uint16 operation_queue_cycle_state = 0u;
	uint16 operation_queue_cycle_flag = 0u;
	std_return_type ret = E_OK;

	/* 获取并清空全局队列 */
	operation_queue_cycle_state = dem_opcycle_pending_cycle_state;    /* 周期状态信息 */
	operation_queue_cycle_flag = dem_opcycle_cycle_flag;              /* 待处理周期标志 */
	dem_opcycle_pending_cycle_state = 0u;                             /* 清空队列 */
	dem_opcycle_cycle_flag = 0u;

	/* 遍历所有待处理周期 */
	while (operation_queue_cycle_flag != 0u) {
		/* 检查当前周期是否需要处理 */
		if (DEM_IS_MASK_SET(operation_queue_cycle_flag, 0x01u, uint16)) {
			/* 处理周期状态转换 */
			ret = handle_opcycle_impl(opcycle_id, operation_queue_cycle_state);

			if (E_OK != ret) {
				/* 报告错误但继续处理 */
				DEM_DET_REPORT(DEM_DEM_OPERATIONCYCLEPROCESSCYCLES_ID_DEM_E_UNEXPECTED_EXECUTION);
				(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "errno:%u",
					       DEM_DEM_OPERATIONCYCLEPROCESSCYCLES_ID_DEM_E_UNEXPECTED_EXECUTION);
			}
		}
		/* 移动到下一个周期 */
		++opcycle_id;
		operation_queue_cycle_flag = SHIFT_RIGHT_ONE_U16(operation_queue_cycle_flag);    /* 右移标志位 */
		operation_queue_cycle_state = SHIFT_RIGHT_ONE_U16(operation_queue_cycle_state);  /* 右移标志位 */

	}
}

FUNC(boolean, DEM_CODE) dem_is_failure_cycle_started(const dem_event_parameter_type *event_param)
{
	return is_opcycle_started(event_param->event_class_ref->failure_cycle_ref);
}

#if (DEM_SUPPORT_CYCLES_COUNTER == STD_ON)
FUNC(void, DEM_CODE) dem_handle_cycle_counters(dem_event_info_rec_type *event_info_rec, boolean event_is_stored)
{
	if (event_is_stored == (boolean)TRUE) {
#if (DEM_CFG_DATA_CYCLES_SINCE_LAST_FAILED == STD_ON)
		event_info_rec->cycles_since_last_failed = (uint8)0u;
#endif
	} else {
#if (DEM_CFG_DATA_CYCLES_SINCE_LAST_FAILED == STD_ON)
		event_info_rec->cycles_since_last_failed_start = TRUE;
		event_info_rec->cycles_since_last_failed = (uint8)0u;
#endif
#if (DEM_CFG_DATA_CYCLES_SINCE_FIRST_FAILED == STD_ON)
		event_info_rec->cycles_since_first_failed_start = TRUE;
		event_info_rec->cycles_since_first_failed = (uint8)0u;
#endif
#if (DEM_CFG_DATA_FAILED_CYCLES == STD_ON)
		event_info_rec->failed_cycles_start = TRUE;
		event_info_rec->failed_cycles = (uint8)0u;
#endif
	}
}
#endif

FUNC(std_return_type, DEM_CODE)
dem_handle_set_opcycle_state(dem_opcycle_id_type opcycle_id, dem_opcycle_state_type cycle_state)
{
	std_return_type ret = E_OK;

	if (opcycle_id >= DEM_OPERATION_CYCLE_ID_ENDMARK) {
		DEM_DET_REPORT(DEM_DEM_SETOPERATIONCYCLESTATE_ID_DEM_E_PARAM_DATA);
		return E_NOT_OK;
	}

	if (DEM_CYCLE_STATE_START == cycle_state) {
		opcycle_start(opcycle_id);
		(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "%u Operation Cycle Start \r\n", opcycle_id);
	} else if (DEM_CYCLE_STATE_END == cycle_state) {
		if (g_dem_opcycle_state_list[opcycle_id] != DEM_CYCLE_STATE_END) {
			opcycle_end(opcycle_id);
			(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_DEBUG, "%u Operation Cycle End \r\n", opcycle_id);
#if (DEM_SUPPORT_AGING == STD_ON) && (DEM_USE_MEMORY_FUNCTIONS == STD_ON)
			(void)dem_handle_aging(opcycle_id);
#endif
		}
	} else {
		DEM_DET_REPORT(DEM_DEM_SETOPERATIONCYCLESTATE_ID_DEM_E_PARAM_DATA);
		ret = E_NOT_OK;
	}

	return ret;
}

FUNC(void, DEM_CODE) dem_set_pending_opcycle(uint8 cycle_id, uint8 status)
{
	uint16 mask = (uint16)1 << cycle_id;

	DEM_SET_MASK(dem_opcycle_cycle_flag, mask, uint16);

	if (DEM_PENDING_CYCLE_STATE_STOPPED == status) {
		DEM_CLEAR_MASK(dem_opcycle_pending_cycle_state, mask, uint16);
	} else {
		DEM_SET_MASK(dem_opcycle_pending_cycle_state, mask, uint16);
	}
}

FUNC(void, DEM_CODE) dem_precess_opcycle_main_function(void)
{
	if (0U != dem_opcycle_cycle_flag) {
		handle_opcycles();
	}
}

FUNC(void, DEM_CODE) dem_init_opcycle_state_list(void)
{
	for (uint8 i = 0u; i < DEM_OPERATION_CYCLE_ID_ENDMARK; i++) {
		g_dem_opcycle_state_list[i] = DEM_CYCLE_STATE_END;
	}
}

FUNC(void, DEM_CODE) dem_init_auto_start_opcycle(void)
{
	for (uint8 i = 0u; i < DEM_OPERATION_CYCLE_ID_ENDMARK; i++) {
		if (g_dem_general_set->opcycle[i].opcycle_autostart == (boolean)TRUE) {
			(void)dem_handle_set_opcycle_state(g_dem_general_set->opcycle[i].opcycle_id,
						DEM_CYCLE_STATE_START);
		}
	}
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"