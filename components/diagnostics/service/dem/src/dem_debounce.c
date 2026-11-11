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

#define DEM_UDS_TEST_FAILED_TRESHOLD ((sint32)127)
#define DEM_UDS_TEST_PASSED_TRESHOLD ((sint32)(-128))
#define DEM_UDS_FDC_RANGE ((sint32)(DEM_UDS_TEST_FAILED_TRESHOLD - (DEM_UDS_TEST_PASSED_TRESHOLD)))

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

static FUNC(dem_event_status_type, DEM_CODE) debounce_none(const dem_event_status_type reported_status)
{
	dem_event_status_type ret = reported_status;

	switch (reported_status) {
		case DEM_EVENT_STATUS_FAILED:
		case DEM_EVENT_STATUS_PASSED:
			break;

		default:
			ret = DEM_EVENT_STATUS_INVALID;
			break;
	}

	return ret;
}

#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON) || (DEM_SUPPORT_CURRENT_FDC == STD_ON)
static inline FUNC(sint8, DEM_CODE)
	local_fdc_to_uds_fdc(sint16 local_fdc, const dem_debounce_counter_based_type *deb_info)
{
	/* [-128, 127] */
	sint32 pd_range = (sint32)((sint32)deb_info->failed_threshold - (sint32)deb_info->passed_threshold);
	sint32 temp = (DEM_UDS_FDC_RANGE * ((sint32)((sint32)local_fdc - (sint32)deb_info->passed_threshold))) +
		      (DEM_UDS_TEST_PASSED_TRESHOLD * pd_range);

	return (sint8)(temp / pd_range);
}
#endif
#endif

#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
static FUNC(std_return_type, DEM_CODE)
get_counter_based_debouncing_of_event(const dem_event_parameter_type *event_param,
				      const dem_event_info_rec_type *event_info_rec,
				      dem_debouncing_state_type *debouncing_state)
{
	std_return_type ret = E_OK;
	const dem_debounce_counter_based_type *deb_info =
		event_param->event_class_ref->debounce_algorithm_class->debounce_algorithm
			.debounce_counter_based;

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if (event_info_rec->is_available == (boolean)FALSE) {
		return E_NOT_OK;
	}
#endif

	if ((0 < event_info_rec->local_fdc) && (event_info_rec->local_fdc < deb_info->failed_threshold)) {
		*debouncing_state = DEM_TEMP_DEFECTIVE;
	} else if (event_info_rec->local_fdc == deb_info->failed_threshold) {
		*debouncing_state = DEM_FINALLY_DEFECTIVE | DEM_TEST_COMPLETE;
	} else if ((deb_info->passed_threshold < event_info_rec->local_fdc) && (event_info_rec->local_fdc < 0)) {
		*debouncing_state = DEM_TEMP_HEALED;
	} else if (event_info_rec->local_fdc == deb_info->passed_threshold) {
		*debouncing_state = DEM_TEST_COMPLETE;
	} else {
		ret = E_NOT_OK;
	}

	return ret;
}

static FUNC(void, DEM_CODE) counter_based_debounce_prefailed(dem_event_info_rec_type *event_info_rec,
							     const dem_debounce_counter_based_type *deb_info)
{
	if (event_info_rec->local_fdc >= deb_info->failed_threshold) {
		return;
	}

	if ((TRUE == deb_info->jump_up) && (event_info_rec->local_fdc < deb_info->jump_up_value)) {
		event_info_rec->local_fdc = deb_info->jump_up_value;
	}
	if (((sint32)event_info_rec->local_fdc + (sint32)deb_info->increment_step_size) <
		deb_info->failed_threshold) {
		event_info_rec->local_fdc += (sint16)(deb_info->increment_step_size);
	} else {
		event_info_rec->local_fdc = deb_info->failed_threshold;
	}

	return;
}

static FUNC(void, DEM_CODE) counter_based_debounce_prepassed(dem_event_info_rec_type *event_info_rec,
								     const dem_debounce_counter_based_type *deb_info)
{
	if (event_info_rec->local_fdc <= deb_info->passed_threshold) {
		return;
	}

	if ((TRUE == deb_info->jump_down) && (event_info_rec->local_fdc > deb_info->jump_down_value)) {
		event_info_rec->local_fdc = deb_info->jump_down_value;
	}
	if (((sint32)event_info_rec->local_fdc - (sint32)deb_info->decrement_step_size) >
		deb_info->passed_threshold) {
		event_info_rec->local_fdc -= (sint16)(deb_info->decrement_step_size);
	} else {
		event_info_rec->local_fdc = deb_info->passed_threshold;
	}

	return;
}

static FUNC(dem_event_status_type, DEM_CODE)
	counter_based_debounce(dem_event_status_type reported_status, dem_event_info_rec_type *event_info_rec)
{
	dem_event_status_type ret = reported_status;
	const dem_event_parameter_type *event_param = NULL;
	const dem_debounce_counter_based_type *deb_info = NULL;

	lookup_event_param(event_info_rec->event_id, &event_param);

	deb_info = event_param->event_class_ref->debounce_algorithm_class->debounce_algorithm
			.debounce_counter_based;

	switch (reported_status) {
		case DEM_EVENT_STATUS_PASSED:
			event_info_rec->local_fdc = deb_info->passed_threshold;
			break;
		case DEM_EVENT_STATUS_FAILED:
			event_info_rec->local_fdc = deb_info->failed_threshold;
			break;
		case DEM_EVENT_STATUS_PREPASSED:
			counter_based_debounce_prepassed(event_info_rec, deb_info);
			break;
		case DEM_EVENT_STATUS_PREFAILED:
			counter_based_debounce_prefailed(event_info_rec, deb_info);
			break;
		default:
			ret = DEM_EVENT_STATUS_INVALID;
			break;
	}

	if (DEM_EVENT_STATUS_INVALID == ret) {
		DEM_DET_REPORT(DEM_DEM_UPDATE_EVENT_STATUS_ID_DEM_E_NOT_IMPLEMENTED_YET);
		return DEM_EVENT_STATUS_INVALID;
	}

	if (event_info_rec->local_fdc >= deb_info->failed_threshold) {
		ret = DEM_EVENT_STATUS_FAILED;
	} else if (event_info_rec->local_fdc <= deb_info->passed_threshold) {
		ret = DEM_EVENT_STATUS_PASSED;
	} else {
		/* do nothing */
	}

#if (DEM_SUPPORT_CURRENT_FDC == STD_ON)
	event_info_rec->uds_fdc = local_fdc_to_uds_fdc(event_info_rec->local_fdc, deb_info);
#endif
#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON)
	event_info_rec->max_uds_fdc =
		MAX((event_info_rec->max_uds_fdc), (local_fdc_to_uds_fdc(event_info_rec->local_fdc, deb_info)));
#endif

	return ret;
}

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
FUNC(std_return_type, DEM_CODE)
get_time_based_debouncing_of_event(const dem_event_parameter_type *event_param,
				   const dem_event_info_rec_type *event_info_rec,
				   dem_debouncing_state_type *debouncing_state)
{
	std_return_type ret = E_OK;
	const dem_debounce_time_based_type *deb_info = event_param->event_class_ref->debounce_algorithm_class
								  ->debounce_algorithm.debounce_time_based;

	if (deb_info->index >= DEM_NOF_TIME_BASE_PREDEB) {
		return E_NOT_OK;
	}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if (event_info_rec->is_available == (boolean)FALSE) {
		return E_NOT_OK;
	}
#endif

	if (TRUE == g_dem_time_base_deb[deb_info->index].data.failure_counting) {
		if ((uint32)event_info_rec->local_fdc < deb_info->time_failed_threshold) {
			*debouncing_state = DEM_TEMP_DEFECTIVE;
		} else {
			*debouncing_state = DEM_FINALLY_DEFECTIVE | DEM_TEST_COMPLETE;
		}
	} else {
		if ((uint32)event_info_rec->local_fdc < deb_info->time_passed_threshold) {
			*debouncing_state = DEM_TEMP_HEALED;
		} else {
			*debouncing_state = DEM_TEST_COMPLETE;
		}
	}

	return ret;
}

static FUNC(sint8, DEM_CODE) get_time_based_fdc(const dem_time_base_debounce_status_type *debounce_timer,
												const dem_debounce_time_based_type *deb_info)
{
	sint64 threshold = 0;
	sint64 value = 0;
	sint8 result = 0;

	if (debounce_timer->data.started == FALSE) {
		return 0;
	}

	if (FALSE == debounce_timer->data.failure_counting) {
		threshold = (sint64)deb_info->time_passed_threshold;

		if (threshold == 0) {
			result = (sint8)DEM_UDS_TEST_PASSED_TRESHOLD;
		} else {
			value = (sint64)debounce_timer->data.debounce_time * DEM_UDS_TEST_PASSED_TRESHOLD / threshold;
			if (value < DEM_UDS_TEST_PASSED_TRESHOLD) {
				value = DEM_UDS_TEST_PASSED_TRESHOLD;
			}
			result = (sint8)value;
		}
	} else {
		threshold = (sint64)deb_info->time_failed_threshold;

		if (threshold == 0) {
			result = (sint8)DEM_UDS_TEST_FAILED_TRESHOLD;
		} else {
			value = (sint64)debounce_timer->data.debounce_time * DEM_UDS_TEST_FAILED_TRESHOLD / threshold;
			if (value > DEM_UDS_TEST_FAILED_TRESHOLD) {
				value = DEM_UDS_TEST_FAILED_TRESHOLD;
			}
			result = (sint8)value;
		}
	}

	return result;
}

static FUNC(dem_event_status_type, DEM_CODE)
time_based_debounce_calc_on_event(dem_event_status_type reported_status,
				  dem_time_base_debounce_status_type *time_based_deb_buf_ptr,
				  const dem_event_parameter_type *event_param,
				  const dem_debounce_time_based_type *deb_info)
{
	dem_event_status_type ret = reported_status;

	time_based_deb_buf_ptr->data.event_id = event_param->event_id;
	time_based_deb_buf_ptr->data.satellite_id = event_param->satellite_id;
	switch (reported_status) {
		case DEM_EVENT_STATUS_PASSED:
			time_based_deb_buf_ptr->data.started = FALSE; /* 直接成熟，定时器关闭 */
			time_based_deb_buf_ptr->data.debounce_time = deb_info->time_passed_threshold;
			time_based_deb_buf_ptr->data.error_reported = TRUE;
			time_based_deb_buf_ptr->data.counter_reset = FALSE;
			break;
		case DEM_EVENT_STATUS_FAILED:
			/* 直接上报failed，直接在后续流程中入队 */
			time_based_deb_buf_ptr->data.started = FALSE; /* 直接成熟，定时器关闭 */
			time_based_deb_buf_ptr->data.debounce_time = deb_info->time_failed_threshold;
			time_based_deb_buf_ptr->data.error_reported = TRUE;
			time_based_deb_buf_ptr->data.counter_reset = FALSE;
			break;
		case DEM_EVENT_STATUS_PREPASSED:
			if ((FALSE == time_based_deb_buf_ptr->data.started) ||
			    (TRUE == time_based_deb_buf_ptr->data.failure_counting)) {
				time_based_deb_buf_ptr->data.failure_counting = FALSE;
				time_based_deb_buf_ptr->data.started = TRUE;
				if (deb_info->time_passed_threshold == 0u) {
					/* 时间去抖门限为零（无去抖） */
					time_based_deb_buf_ptr->data.debounce_time = deb_info->time_passed_threshold;
					time_based_deb_buf_ptr->data.error_reported = TRUE;
					time_based_deb_buf_ptr->data.counter_reset = FALSE;
				} else {
					/* 上报prepassed */
					time_based_deb_buf_ptr->data.debounce_time = 0;
					time_based_deb_buf_ptr->data.error_reported = FALSE;
					time_based_deb_buf_ptr->data.counter_reset = TRUE;
				}
			}
			break;
		case DEM_EVENT_STATUS_PREFAILED:
			if ((FALSE == time_based_deb_buf_ptr->data.started) ||
			    (FALSE == time_based_deb_buf_ptr->data.failure_counting)) {
				time_based_deb_buf_ptr->data.failure_counting = TRUE;
				time_based_deb_buf_ptr->data.started = TRUE;
				if (deb_info->time_failed_threshold == 0u) {
					time_based_deb_buf_ptr->data.debounce_time = deb_info->time_failed_threshold;
					time_based_deb_buf_ptr->data.error_reported = TRUE;
					time_based_deb_buf_ptr->data.counter_reset = FALSE;
				} else {
					/* 上报prefailed */
					time_based_deb_buf_ptr->data.debounce_time = 0;
					time_based_deb_buf_ptr->data.error_reported = FALSE;
					time_based_deb_buf_ptr->data.counter_reset = TRUE;
				}
			}
			break;
		default:
			ret = DEM_EVENT_STATUS_INVALID;
			DEM_DET_REPORT(DEM_DEM_UPDATE_EVENT_STATUS_ID_DEM_E_NOT_IMPLEMENTED_YET);
			break;
	}

	return ret;
}

static FUNC(boolean, DEM_CODE)
dem_atomic_set_time_base_deb(dem_time_base_debounce_status_type *deb_buf_ptr,
			     dem_time_base_debounce_status_type old_deb_buf,
			     dem_time_base_debounce_status_type new_deb_buf)
{
	boolean ret = TRUE;

	if (NULL == deb_buf_ptr) {
		DEM_REPORT_ERROR_RV(DEM_DEM_SATELLITESATPREINIT_ID_DEM_E_PARAM_DATA, TRUE);
	} else {
		ret = ((dem_atomic_cmp_swap((dem_atomic_word_type *)deb_buf_ptr, old_deb_buf.raw[0],
					    new_deb_buf.raw[0])) &&
		       (dem_atomic_cmp_swap(&(((dem_atomic_word_type *)deb_buf_ptr)[1]), old_deb_buf.raw[1],
					    new_deb_buf.raw[1])));
	}

	return ret;
}

static FUNC(dem_event_status_type, DEM_CODE)
	time_based_debounce(dem_event_status_type reported_status, const dem_event_parameter_type *event_param)
{
	dem_event_status_type ret = reported_status;
	dem_time_base_debounce_status_type old_time_based_deb_buf = { { 0 } };
	dem_time_base_debounce_status_type new_time_based_deb_buf = { { 0 } };
	const dem_debounce_time_based_type *deb_info = event_param->event_class_ref->debounce_algorithm_class
								  ->debounce_algorithm.debounce_time_based;

	do {
		old_time_based_deb_buf = g_dem_time_base_deb[deb_info->index];
		new_time_based_deb_buf = old_time_based_deb_buf;
		ret = time_based_debounce_calc_on_event(reported_status, &new_time_based_deb_buf, event_param,
							    deb_info);
		if (DEM_EVENT_STATUS_INVALID == ret) {
			break;
		}
	} while (dem_atomic_set_time_base_deb(&(g_dem_time_base_deb[deb_info->index]), old_time_based_deb_buf,
					      new_time_based_deb_buf) == FALSE);

	return ret;
}

static FUNC(void, DEM_CODE)
	time_based_debounce_main_function_process(dem_time_base_debounce_status_type *deb_buf_ptr,
						  dem_event_info_rec_type *event_info_rec,
						  const dem_debounce_time_based_type *deb_info)
{
	std_return_type push_queue_ret = E_NOT_OK;

	if (FALSE == deb_buf_ptr->data.started) {
		return;
	}
	if (TRUE == deb_buf_ptr->data.counter_reset) {
		deb_buf_ptr->data.counter_reset = FALSE;
		return;
	}

	if (TRUE == deb_buf_ptr->data.failure_counting) {
		if (deb_buf_ptr->data.debounce_time < deb_info->time_failed_threshold) {
			deb_buf_ptr->data.debounce_time += DEM_TASK_TIME;
		}

		if (deb_buf_ptr->data.debounce_time >= deb_info->time_failed_threshold) {
			push_queue_ret = dem_push_event_info_to_queue(deb_buf_ptr->data.satellite_id,
				deb_buf_ptr->data.event_id, DEM_EVENT_STATUS_FAILED);
			if (E_OK == push_queue_ret) {
				deb_buf_ptr->data.error_reported = TRUE;
				deb_buf_ptr->data.started = FALSE;
				deb_buf_ptr->data.debounce_time = 0;
			}
		}
	} else {
		if (deb_buf_ptr->data.debounce_time < deb_info->time_passed_threshold) {
			deb_buf_ptr->data.debounce_time += DEM_TASK_TIME;
		}

		if (deb_buf_ptr->data.debounce_time >= deb_info->time_passed_threshold) {
			push_queue_ret = dem_push_event_info_to_queue(deb_buf_ptr->data.satellite_id,
				deb_buf_ptr->data.event_id, DEM_EVENT_STATUS_PASSED);
			if (E_OK == push_queue_ret) {
				deb_buf_ptr->data.error_reported = TRUE;
				deb_buf_ptr->data.started = FALSE;
				deb_buf_ptr->data.debounce_time = 0;
			}
		}
	}

#if (DEM_SUPPORT_CURRENT_FDC == STD_ON)
	event_info_rec->uds_fdc = get_time_based_fdc(deb_buf_ptr, deb_info);
#endif

#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON)
	sint8 time_based_fdc = get_time_based_fdc(deb_buf_ptr, deb_info);

	event_info_rec->max_uds_fdc = MAX(time_based_fdc, (event_info_rec->max_uds_fdc));
#endif

	return;
}

static FUNC(void, DEM_CODE) time_based_debounce_calc_on_main_function(dem_time_base_debounce_status_type *deb_buf_ptr,
								      dem_satellite_id_type satellite_id)
{
	const dem_event_parameter_type *event_param = NULL;

	const dem_debounce_algorithm_class_type *deb_alg = NULL;

	dem_event_info_rec_type *event_info_rec = NULL;

	if (DEM_EVENT_ID_NULL == deb_buf_ptr->data.event_id) {
		return;
	}
	/* 只处理当前satellite下的事件去抖 */
	if (deb_buf_ptr->data.satellite_id != satellite_id) {
		return;
	}

	dem_lookup_event_param(deb_buf_ptr->data.event_id, &event_param);
	if (NULL == event_param) {
		return;
	}

	dem_lookup_event_info_rec(deb_buf_ptr->data.event_id, &event_info_rec);
	if (NULL == event_info_rec) {
		return;
	}
	deb_alg = event_param->event_class_ref->debounce_algorithm_class;
	if (NULL == deb_alg) {
		return;
	}
	if (DEM_PRE_DEBOUNCE_TIME_BASED != deb_alg->debounce_name) {
		return;
	}
	if (dem_is_opcycle_started(event_param->event_class_ref->opcycle_ref) == FALSE) {
		// 操作循环结束，停止时延去抖
		deb_buf_ptr->data.started = FALSE;
		deb_buf_ptr->data.error_reported = FALSE;
		return;
	}
	time_based_debounce_main_function_process(deb_buf_ptr, event_info_rec,
						  deb_alg->debounce_algorithm.debounce_time_based);
}
#endif
#endif

static FUNC(dem_event_status_type, DEM_CODE)
run_debounce(dem_event_status_type reported_event_status, dem_event_info_rec_type *event_info,
		const dem_event_parameter_type *event_param)
{
	dem_event_status_type event_status = reported_event_status;

#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
	const dem_debounce_algorithm_class_type *deb_alg = event_param->event_class_ref->debounce_algorithm_class;

	if (NULL != deb_alg) {
		switch (deb_alg->debounce_name) {
			case DEM_PRE_DEBOUNCE_COUNTER_BASED:
				event_status = counter_based_debounce(reported_event_status, event_info);
				break;
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
			case DEM_PRE_DEBOUNCE_TIME_BASED:
				event_status = time_based_debounce(reported_event_status, event_param);
				break;
#endif
			default:
				event_status = DEM_EVENT_STATUS_INVALID;
				DEM_DET_REPORT(DEM_DEM_UPDATE_EVENT_STATUS_ID_DEM_E_NOT_IMPLEMENTED_YET);
				break;
		}
	}
#else
	event_status = debounce_none(reported_event_status);
#endif

	return event_status;
}

#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
FUNC(void, DEM_CODE) dem_time_based_debounce_main_function(dem_satellite_id_type satellite_id)
{
	dem_time_base_debounce_status_type old_deb_buf = { { 0 } };
	dem_time_base_debounce_status_type new_deb_buf = { { 0 } };

	for (uint16 i = 0; i < DEM_NOF_TIME_BASE_PREDEB; i++) {
		do {
			old_deb_buf = g_dem_time_base_deb[i];
			new_deb_buf = old_deb_buf;
			time_based_debounce_calc_on_main_function(&new_deb_buf, satellite_id);
		} while (dem_atomic_set_time_base_deb(&(g_dem_time_base_deb[i]), old_deb_buf, new_deb_buf) == FALSE);
	}

	return;
}
#endif
#endif

FUNC(void, DEM_CODE) dem_reset_debounce_counter(dem_event_info_rec_type *event_info_rec)
{
#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON) || (DEM_SUPPORT_CURRENT_FDC == STD_ON)
	sint8 start_fdc = dem_get_default_uds_fdc(event_info_rec->event_id);

#if (DEM_SUPPORT_CURRENT_FDC == STD_ON)
	event_info_rec->uds_fdc = start_fdc;
#endif
#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON)
	event_info_rec->max_uds_fdc = start_fdc;
#endif
#endif
#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
	event_info_rec->local_fdc = 0;
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
	const dem_event_parameter_type *event_param = NULL;
	dem_time_base_debounce_status_type old_time_based_deb_buf = { { 0 } };
	dem_time_base_debounce_status_type new_time_based_deb_buf = { { 0 } };
	const dem_debounce_algorithm_class_type *deb_alg = NULL;

	dem_lookup_event_param(event_info_rec->event_id, &event_param);
	if (NULL == event_param) {
		return;
	}

	deb_alg = event_param->event_class_ref->debounce_algorithm_class;

	if ((NULL == deb_alg) || (DEM_PRE_DEBOUNCE_TIME_BASED != deb_alg->debounce_name)) {
		return;
	}

	do {
		old_time_based_deb_buf = g_dem_time_base_deb[deb_alg->debounce_algorithm.debounce_time_based->index];
		new_time_based_deb_buf = old_time_based_deb_buf;
		new_time_based_deb_buf.data.started = FALSE;
		new_time_based_deb_buf.data.error_reported = FALSE;
		new_time_based_deb_buf.data.debounce_time = 0UL;
	} while (dem_atomic_set_time_base_deb(&(g_dem_time_base_deb[deb_alg->debounce_algorithm.debounce_time_based
						->index]), old_time_based_deb_buf, new_time_based_deb_buf) == FALSE);
#endif
#endif
}

FUNC(std_return_type, DEM_CODE)
dem_get_fdc_internal(dem_event_id_type event_id, sint8 *fault_detection_counter)
{
	std_return_type ret = E_NOT_OK;
	const dem_event_parameter_type *event_param;
	dem_event_info_rec_type *event_info_rec = NULL;

	dem_lookup_event_info_rec(event_id, &event_info_rec);
	dem_lookup_event_param(event_id, &event_param);

	if ((NULL == event_param) || (NULL == event_info_rec)) {
		DEM_DET_REPORT(DEM_DEM_GETFAULTDETECTIONCOUNTER_ID_DEM_E_PARAM_DATA);
		return E_NOT_OK;
	}

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
	if (FALSE == event_info_rec->is_available) {
		DEM_DET_REPORT(DEM_DEM_GETFAULTDETECTIONCOUNTER_ID_DEM_E_PARAM_DATA);
		return E_NOT_OK;
	}
#endif

#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
	const dem_debounce_algorithm_class_type *deb_alg = event_param->event_class_ref->debounce_algorithm_class;

	if (NULL != deb_alg) {
		switch (deb_alg->debounce_name) {
#if (DEM_SUPPORT_CURRENT_FDC == STD_ON)
			case DEM_PRE_DEBOUNCE_COUNTER_BASED:
				*fault_detection_counter = event_info_rec->uds_fdc;
				ret = E_OK;
				break;
#endif

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
			case DEM_PRE_DEBOUNCE_TIME_BASED:
				*fault_detection_counter =
					get_time_based_fdc(&g_dem_time_base_deb[deb_alg->debounce_algorithm
						.debounce_time_based->index],
						deb_alg->debounce_algorithm.debounce_time_based);
				ret = E_OK;
				break;
#endif
			default:
				DEM_DET_REPORT(DEM_DEM_GETFAULTDETECTIONCOUNTER_ID_DEM_E_PARAM_DATA);
				break;
		}
	}
#endif

	return ret;
}

#if (DEM_SUPPORT_MAX_FDC_DURING_CURRENT_CYCLE == STD_ON) || (DEM_SUPPORT_CURRENT_FDC == STD_ON)
FUNC(sint8, DEM_CODE) dem_get_default_uds_fdc(dem_event_id_type event_id)
{
	sint8 uds_fdc = 0;
	const dem_event_parameter_type *event_param = NULL;

	dem_lookup_event_param(event_id, &event_param);
	if (NULL != event_param) {
#if (DEM_DEBOUNCE_DISABLE == STD_OFF)
		const dem_debounce_algorithm_class_type *deb_alg =
			event_param->event_class_ref->debounce_algorithm_class;

		if ((NULL != deb_alg) && (DEM_PRE_DEBOUNCE_COUNTER_BASED == deb_alg->debounce_name)) {
			uds_fdc = local_fdc_to_uds_fdc(0, deb_alg->debounce_algorithm.debounce_counter_based);
		}
#endif
	}

	return uds_fdc;
}
#endif

FUNC(dem_event_status_type, DEM_CODE)
dem_process_debounce(const dem_event_parameter_type *event_param, dem_event_status_type reported_event_status,
		     dem_event_info_rec_type *event_info)
{
	dem_event_status_type event_status = reported_event_status;

	if (NULL == event_info) {
		return DEM_EVENT_STATUS_INVALID;
	}
	event_status = run_debounce(reported_event_status, event_info, event_param);

	if (DEM_EVENT_STATUS_INVALID != event_status) {
		event_info->error_status_changed = FALSE;
#if defined(DEM_USE_INDICATORS)
		event_info->indicator_data_changed = FALSE;
#endif
	}

	return event_status;
}

FUNC(std_return_type, DEM_CODE)
dem_get_debouncing_state(dem_event_id_type event_id, dem_debouncing_state_type *debouncing_state)
{
#if (DEM_DEBOUNCE_DISABLE == STD_ON)
	(void)event_id;
	(void)debouncing_state;

	return E_NOT_OK;
#else
	std_return_type ret = E_OK;
	dem_event_info_rec_type *event_info_rec = NULL;
	const dem_event_parameter_type *event_param = NULL;
	const dem_debounce_algorithm_class_type *deb_alg = NULL;

	lookup_event_param(event_id, &event_param);
	lookup_event_info_rec(event_id, &event_info_rec);

	if ((NULL == event_param) || (NULL == event_info_rec)) {
		return E_OK;
	}

	deb_alg = event_param->event_class_ref->debounce_algorithm_class;

	if (NULL != deb_alg) {
		switch (deb_alg->debounce_name) {
			case DEM_PRE_DEBOUNCE_COUNTER_BASED:
				ret = get_counter_based_debouncing_of_event(event_param, event_info_rec,
									    debouncing_state);
				break;
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
			case DEM_PRE_DEBOUNCE_TIME_BASED:
				ret = get_time_based_debouncing_of_event(event_param, event_info_rec,
									 debouncing_state);
				break;
#endif
			default:
				ret = E_NOT_OK;
				break;
		}
	}

	return ret;
#endif
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"
