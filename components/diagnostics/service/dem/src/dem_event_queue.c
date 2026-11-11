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

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

static inline FUNC(boolean, DEM_CODE) is_reset_test_failed_requested(dem_event_queue_status_type queue_status)
{
	if (DEM_IS_MASK_SET(queue_status, DEM_EVENTQUEUE_RESET_TESTFAILED_FLAG, dem_event_queue_status_type)) {
		return TRUE;
	}

	return FALSE;
}

static inline FUNC(dem_event_status_type, DEM_CODE)
get_event_last_queued_status(dem_event_queue_status_type queue_status)
{
	if (DEM_IS_MASK_SET(queue_status, DEM_EVENTQUEUE_EVENT_LAST_FAILED_STATUS_MASK, dem_event_queue_status_type)) {
		return DEM_EVENT_STATUS_FAILED;
	}

	return DEM_EVENT_STATUS_PASSED;
}

static inline FUNC(dem_event_status_type, DEM_CODE) get_event_status_reverse(dem_event_status_type event_status)
{
	if (DEM_EVENT_STATUS_FAILED == event_status) {
		return DEM_EVENT_STATUS_PASSED;
	}

	return DEM_EVENT_STATUS_FAILED;
}

static inline FUNC(void, DEM_CODE) set_event_last_queued_status(dem_event_queue_status_type *queue_status,
							 dem_event_status_type event_status)
{
	if (DEM_EVENT_STATUS_FAILED == event_status) {
		DEM_SET_MASK(*queue_status, DEM_EVENTQUEUE_EVENT_LAST_FAILED_STATUS_MASK,
			     dem_event_queue_status_type);
	} else if (DEM_EVENT_STATUS_PASSED == event_status) {
		DEM_CLEAR_MASK(*queue_status, DEM_EVENTQUEUE_EVENT_LAST_FAILED_STATUS_MASK,
			       dem_event_queue_status_type);
	} else {
		/* do nothing */
	}
}

static inline FUNC(uint8, DEM_CODE) get_event_queued_status_num(dem_event_queue_status_type queue_status)
{
	return queue_status & DEM_EVENTQUEUE_EVENT_STATUS_NUM_MASK;
}

static inline FUNC(void, DEM_CODE) set_event_queued_status_num(dem_event_queue_status_type *queue_status, uint8 num)
{
	DEM_CLEAR_MASK(*queue_status, DEM_EVENTQUEUE_EVENT_STATUS_NUM_MASK, dem_event_queue_status_type);
	DEM_SET_MASK(*queue_status, num & DEM_EVENTQUEUE_EVENT_STATUS_NUM_MASK, dem_event_queue_status_type);
}

/* 处理事件队列 */
static FUNC(void, DEM_CODE) process_queued_status(const dem_event_queue_status_type queue_status,
						  const dem_event_id_type event_id)
{
	boolean need_notify = TRUE;
	uint8 status_num = 0u;
	dem_event_status_type first_status = DEM_EVENT_STATUS_PASSED;
	dem_event_status_type last_status = DEM_EVENT_STATUS_PASSED;
	dem_event_status_type curr_status = DEM_EVENT_STATUS_PASSED;

	if (is_reset_test_failed_requested(queue_status) == TRUE) {
		(void)dem_reset_test_failed(event_id);
	}

	last_status = get_event_last_queued_status(queue_status);
	status_num = get_event_queued_status_num(queue_status);

	if (0u == status_num) {
		return;
	}

	if (1u == status_num) {
		dem_handle_event_status(event_id, last_status, need_notify);
		return ;
	}

	if ((status_num % DEM_MAINFUNC_HANDLE_EVENT_STATUS_KINDS) == 0) {
		first_status = get_event_status_reverse(last_status);
	} else {
		first_status = last_status;
	}
	curr_status = first_status;

	for (uint8 i = 0; i < status_num; i++) {
		dem_handle_event_status(event_id, curr_status, need_notify);
		curr_status = get_event_status_reverse(curr_status);
	}
}

static FUNC(void, DEM_CODE) process_satellite_queued_status(const dem_satellite_id_type satellite_id)
{
	dem_satellite_event_id_type satellite_event_id;

	for (satellite_event_id = 0u; satellite_event_id < dem_get_satellite_event_count(satellite_id);
	     ++satellite_event_id) {
		/* 事件队列状态发生改变时，需要进行处理 */
		if (dem_get_event_queue_status(satellite_id, satellite_event_id) != DEM_EVENTQUEUE_INITIAL_STATE) {
			dem_event_queue_status_type queue_status;
			dem_event_id_type event_id;

			event_id = dem_satellite_get_event_id(satellite_id, satellite_event_id);
			queue_status = dem_handle_event_queue_status(satellite_id, satellite_event_id);

			if (DEM_EVENTQUEUE_INITIAL_STATE != queue_status) {
				process_queued_status(queue_status, event_id);
			}
		}
	}
}

/* 周期处理所有satellite上报的故障 */
FUNC(void, DEM_CODE) dem_precess_event_queue_main_function(void)
{
	for (dem_satellite_id_type satellite_id = 0u; satellite_id < DEM_MAX_NUMBER_SATELLITE; ++satellite_id) {
		if (dem_clear_event_queue_updated(satellite_id) == TRUE) {
			process_satellite_queued_status(satellite_id);
		}
	}
}

/* 将故障状态更新到satellite_data等待master处理 */
FUNC(std_return_type, DEM_CODE)
dem_push_event_info_to_queue(dem_satellite_id_type satellite_id, dem_event_id_type event_id,
			     dem_event_status_type event_status)
{
	dem_satellite_event_id_type satellite_event_id = dem_get_satellite_event_id(event_id);
	dem_satellite_data_type old_satellite_data;
	dem_satellite_data_type new_satellite_data;

	do {
		old_satellite_data = dem_get_satellite_data(satellite_id, satellite_event_id);
		new_satellite_data = dem_update_satellite_data(old_satellite_data, event_status);
	} while (dem_satellite_sync_set_satellite_data(satellite_id, satellite_event_id, old_satellite_data,
						       new_satellite_data) == FALSE);

	if (dem_is_queue_status_updated(old_satellite_data, new_satellite_data) == TRUE) {
		dem_satellite_set_event_queue_updated(satellite_id);
	}

	return E_OK;
}

FUNC(dem_event_queue_status_type, DEM_CODE) dem_get_initial_queue_status(void)
{
	return DEM_EVENTQUEUE_INITIAL_STATE;
}

FUNC(dem_event_queue_status_type, DEM_CODE)
dem_calculate_queue_status(const dem_event_queue_status_type queue_status,
			   const dem_event_status_type event_status)
{
	dem_event_queue_status_type new_queue_status = queue_status;
	uint8 status_num = 0u;
	dem_event_status_type last_status = DEM_EVENT_STATUS_PASSED;

	switch(event_status) {
		case DEM_EVENTQUEUE_RESET_TESTFAILED_FLAG:
			DEM_SET_MASK(new_queue_status, DEM_EVENTQUEUE_RESET_TESTFAILED_FLAG,
				     dem_event_queue_status_type);
			break;
		case DEM_EVENT_STATUS_PASSED:
		case DEM_EVENT_STATUS_FAILED: // fall through
			last_status = get_event_last_queued_status(queue_status);
			status_num = get_event_queued_status_num(queue_status);
			if ((last_status != event_status) || (status_num == 0u)) {
				set_event_last_queued_status(&new_queue_status, event_status);
				if (status_num < DEM_EVENTQUEUE_QUEUED_STATUS_MAX_NUM) {
					set_event_queued_status_num(&new_queue_status, status_num + 1);
				}
			}
			break;
		default:
			break;
	}

	return new_queue_status;
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"