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

#define DEM_SATELLITE_CAS_MAX_COUNT 10000

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

static FUNC(boolean, DEM_CODE) sync_set_satellite_info(const dem_satellite_id_type satellite_id,
						       const dem_satellite_status_info_type old_satellite_status_info,
						       const dem_satellite_status_info_type new_satellite_status_info)
{
	boolean ret = TRUE;
	dem_atomic_word_type *satellite_status = NULL;

	if (satellite_id < DEM_SATELLITEINFO_SIZE) {
		satellite_status = dem_get_satellite_status(satellite_id);
		ret = dem_atomic_cmp_swap(satellite_status, old_satellite_status_info.raw,
					  new_satellite_status_info.raw);
	}

	return ret;
}

static FUNC(void, DEM_CODE) set_satellite_data(const dem_satellite_id_type satellite_id,
					       const dem_satellite_event_id_type satellite_event_id,
					       const dem_satellite_data_type new_satellite_data)
{
	if ((satellite_id < DEM_SATELLITEINFO_SIZE) &&
	    (satellite_event_id < (dem_get_satellite_event_count(satellite_id)))) {
		g_dem_satellite_info[satellite_id].satellite_data[satellite_event_id] = new_satellite_data;
	}
}

static FUNC(boolean, DEM_CODE) sync_set_satellite_data(const dem_satellite_id_type satellite_id,
						       const dem_satellite_event_id_type satellite_event_id,
						       const dem_satellite_data_type old_satellite_data,
						       const dem_satellite_data_type new_satellite_data)
{
	return dem_satellite_sync_set_satellite_data(satellite_id, satellite_event_id, old_satellite_data,
						     new_satellite_data);
}

static FUNC(dem_satellite_status_info_type, DEM_CODE)
	get_satellite_status_info(const dem_satellite_id_type satellite_id)
{
	(void)satellite_id;
	dem_satellite_status_info_type ret;
	dem_atomic_word_type *satellite_status = dem_get_satellite_status(satellite_id);

	ret.raw = (dem_atomic_word_type)(*satellite_status);

	return ret;
}

static FUNC(ApplicationType, DEM_CODE) get_current_application_id(void)
{
	ApplicationType ret = (ApplicationType)0;

#if (DEM_SUPPORT_MULTIPLE_SATELLITES == STD_ON)
	ret = os_get_current_app_id();
#endif

	return ret;
}

static FUNC(ApplicationType, DEM_CODE) get_satellite_application_id(dem_satellite_id_type satellite_id)
{
	return (ApplicationType)g_dem_satellite_info[satellite_id].application_id;
}

/**
 * @brief 使用CAS机制设置卫星事件队列更新标志
 *
 * @details 该函数通过CAS（Compare-And-Swap）原子操作来安全地标记指定卫星的事件队列已更新。在并发环境下确保
 *          数据一致性，防止多个任务同时修改导致的状态不一致问题。
 *
 * @param[in] satellite_id  需要设置事件队列更新标志的卫星ID
 *
 * @return boolean
 *         - TRUE:  事件队列更新标志设置成功
 *         - FALSE: 在最大重试次数内未能成功设置
 *
 * @note 函数最多重试 DEM_SATELLITE_CAS_MAX_COUNT 次，通常用于通知卫星有新的事件需要处理
 */
static FUNC(boolean, DEM_CODE) dem_satellite_set_event_queue_updated_cas(const dem_satellite_id_type satellite_id)
{
	dem_satellite_status_info_type old_satellite_status_info;
	dem_satellite_status_info_type new_satellite_status_info;

	/* CAS重试循环：确保在并发环境下安全更新事件队列标志 */
	for (int i = 0; i < DEM_SATELLITE_CAS_MAX_COUNT; i++) {
		old_satellite_status_info = get_satellite_status_info(satellite_id);
		new_satellite_status_info = dem_set_event_queue_updated(old_satellite_status_info);

		if (TRUE ==
		    sync_set_satellite_info(satellite_id, old_satellite_status_info, new_satellite_status_info)) {
			/* 成功设置事件队列更新标志 */
			return TRUE;
		}
	}
	/* 达到最大重试次数，可能存在高并发竞争 */
	return FALSE;
}

/**
 * @brief 使用CAS（Compare-And-Swap）机制设置卫星状态
 *
 * @details 该函数通过CAS原子操作来安全地更新指定卫星的状态。在多线程/多核环境下，使用CAS机制保证数据一致性，
 *          避免并发修改导致的数据竞争问题。
 *
 * @param[in] satellite_id  需要设置状态的卫星ID
 * @param[in] state        要设置的新状态值
 *
 * @return boolean
 *         - TRUE:  状态设置成功
 *         - FALSE: 在最大重试次数内未能成功设置状态
 *
 * @note 函数最多重试 DEM_SATELLITE_CAS_MAX_COUNT 次，如果在此期间其他任务不断修改卫星状态，可能导致设置失败
 */
static FUNC(boolean, DEM_CODE)
	dem_set_satellite_state_cas(const dem_satellite_id_type satellite_id, const dem_state_type state)
{
	dem_satellite_status_info_type old_satellite_status_info;
	dem_satellite_status_info_type new_satellite_status_info;

	/*
	 * CAS操作循环：通过比较和交换来保证数据一致性
     * 如果在读取和写入之间数据被其他任务修改，
     * sync_set_satellite_info会返回FALSE，需要重新尝试
     */
	for (int i = 0; i < DEM_SATELLITE_CAS_MAX_COUNT; i++) {
		old_satellite_status_info = get_satellite_status_info(satellite_id);
		new_satellite_status_info = dem_set_satellite_info_state(old_satellite_status_info, state);

		if (TRUE ==
		    sync_set_satellite_info(satellite_id, old_satellite_status_info, new_satellite_status_info)) {
			return TRUE;
		}
	}

	/* 超过最大重试次数，设置失败
     * 可能原因：
     * 1. 系统负载过高，竞争激烈
     * 2. 其他任务持续修改该卫星状态
     */
	return FALSE;
}

/**
 * @brief 使用CAS机制清除卫星事件队列更新标志
 *
 * @details 该函数通过CAS（Compare-And-Swap）原子操作来安全地清除指定卫星的事件队列更新标志。
 *          清除该标志通常表示卫星已经处理完队列中的事件。函数会返回清除前的卫星状态信息，
 *          便于调用者了解清除操作前的状态。
 *
 * @param[in] satellite_id  需要清除事件队列更新标志的卫星ID
 *
 * @return dem_satellite_status_info_type 清除操作前的卫星状态信息
 *         - 成功时：返回清除标志前的原始状态信息
 *         - 失败时：返回最后一次读取的状态信息（可能不是最新的）
 *
 * @note 函数最多重试 DEM_SATELLITE_CAS_MAX_COUNT 次
 *       如果操作失败，会记录错误日志但仍会返回最后读取的状态
 *
 * @warning 返回值在CAS失败时可能不反映实际的当前状态
 */
static FUNC(dem_satellite_status_info_type, DEM_CODE)
	dem_clear_event_queue_updated_cas(dem_satellite_id_type satellite_id)
{
	dem_satellite_status_info_type old_satellite_status_info;
	dem_satellite_status_info_type new_satellite_status_info;
	/* CAS重试循环：确保在并发环境下安全清除事件队列更新标志 */
	for (int i = 0; i < DEM_SATELLITE_CAS_MAX_COUNT; i++) {
		old_satellite_status_info = get_satellite_status_info(satellite_id);
		new_satellite_status_info = dem_reset_event_queue_updated(old_satellite_status_info);

		if (TRUE ==
		    sync_set_satellite_info(satellite_id, old_satellite_status_info, new_satellite_status_info)) {
			/* CAS操作成功，返回清除前的原始状态 */
			return old_satellite_status_info;
		}
	}

	/* 达到最大重试次数仍未成功，记录错误日志。这可能表明系统存在高并发竞争或其他异常情况 */
	(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_ERROR, "Dem satellite %u set clear update failed \r\n", satellite_id);

	/* 即使操作失败，仍返回最后一次读取的状态信息（注意：可能不是最新状态） */
	return old_satellite_status_info;
}

FUNC(void, DEM_CODE)
dem_satellite_set_event_queue_updated(const dem_satellite_id_type satellite_id)
{
	boolean update_success = FALSE;

	update_success = dem_satellite_set_event_queue_updated_cas(satellite_id);

	if (!update_success) {
		(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_ERROR, "Dem satellite %u set event queue updated failed \r\n",
			       satellite_id);
	}
}

FUNC(dem_satellite_data_type, DEM_CODE)
dem_update_satellite_data(const dem_satellite_data_type old_satellite_data, dem_event_status_type event_status)
{
	dem_satellite_data_type new_satellite_data = old_satellite_data;
	dem_event_queue_status_type old_queue_status = dem_get_satellite_data_queue_status(old_satellite_data);
	dem_event_queue_status_type new_queue_status = dem_calculate_queue_status(old_queue_status, event_status);

	new_satellite_data = dem_set_satellite_data_queue_status(new_satellite_data, new_queue_status);

	return new_satellite_data;
}

FUNC(boolean, DEM_CODE)
dem_is_queue_status_updated(const dem_satellite_data_type old_satellite_data,
			    const dem_satellite_data_type new_satellite_data)
{
	dem_event_queue_status_type old_queue_status = dem_get_satellite_data_queue_status(old_satellite_data);
	dem_event_queue_status_type new_queue_status = dem_get_satellite_data_queue_status(new_satellite_data);

	if ((DEM_EVENTQUEUE_INITIAL_STATE == old_queue_status) && (DEM_EVENTQUEUE_INITIAL_STATE != new_queue_status)) {
		return TRUE;
	}

	return FALSE;
}

FUNC(void, DEM_CODE) dem_satellite_memory_init(const dem_satellite_id_type satellite_id)
{
	dem_atomic_word_type *satellite_status = NULL;
	dem_satellite_status_info_type satellite_info;

	(void)memset(&satellite_info, 0, sizeof(satellite_info));
	satellite_info.data.state = DEM_STATE_UNINIT;

	if (satellite_id < DEM_SATELLITEINFO_SIZE) {
		satellite_status = dem_get_satellite_status(satellite_id);
		*satellite_status = satellite_info.raw;
	} else {
		DEM_REPORT_ERROR_NO_RV(DEM_DEM_SATELLITECOREMEMORYINIT_ID_DEM_E_PARAM_DATA);
	}
}

FUNC(void, DEM_CODE) dem_satellite_pre_init_internal(const dem_satellite_id_type satellite_id)
{
	dem_atomic_word_type *satellite_status = NULL;
	dem_satellite_status_info_type satellite_info;
	dem_satellite_data_type data = dem_get_satellite_data_init_value();

	for (dem_satellite_event_id_type event_id = 0u; event_id < dem_get_satellite_event_count(satellite_id);
	     ++event_id) {
		set_satellite_data(satellite_id, event_id, data);
		/* 已经放在master里面对去抖做了初始化 */
	}

	satellite_info = get_satellite_status_info(satellite_id);
	satellite_info = dem_set_satellite_info_state(satellite_info, DEM_INIT_PREINIT);

	satellite_status = dem_get_satellite_status(satellite_id);
	*satellite_status = satellite_info.raw;
}

FUNC(void, DEM_CODE)
dem_set_satellite_state(const dem_satellite_id_type satellite_id, const dem_state_type state)
{
	boolean set_stat_success = FALSE;

	set_stat_success = dem_set_satellite_state_cas(satellite_id, state);

	if (!set_stat_success) {
		(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_ERROR, "Dem satellite %u set state failed \r\n", satellite_id);
	}
}

FUNC(boolean, DEM_CODE)
dem_satellite_sync_set_satellite_data(const dem_satellite_id_type satellite_id,
				      const dem_satellite_event_id_type satellite_event_id,
				      const dem_satellite_data_type old_satellite_data,
				      const dem_satellite_data_type new_satellite_data)
{
	boolean ret = (boolean)FALSE;
	dem_satellite_data_type *satellite_data = NULL;

	if ((satellite_id < DEM_SATELLITEINFO_SIZE) &&
	    (satellite_event_id < (dem_get_satellite_event_count(satellite_id)))) {
		satellite_data = dem_get_satellite_data_ptr(satellite_id, satellite_event_id);
		ret = dem_atomic_cmp_swap(&(satellite_data->raw), old_satellite_data.raw, new_satellite_data.raw);
	} else {
		DEM_REPORT_ERROR_RV(DEM_DEM_SATELLITESATPREINIT_ID_DEM_E_PARAM_DATA, TRUE);
	}

	return ret;
}

FUNC(boolean, DEM_CODE) dem_clear_event_queue_updated(dem_satellite_id_type satellite_id)
{
	dem_satellite_status_info_type satellite_status_info = dem_clear_event_queue_updated_cas(satellite_id);

	return dem_get_event_queue_updated(satellite_status_info);
}

FUNC(dem_event_queue_status_type, DEM_CODE)
dem_get_event_queue_status(const dem_satellite_id_type satellite_id,
			   const dem_satellite_event_id_type satellite_event_id)
{
	dem_satellite_data_type old_satellite_data = dem_get_satellite_data(satellite_id, satellite_event_id);

	return dem_get_satellite_data_queue_status(old_satellite_data);
}

FUNC(dem_event_queue_status_type, DEM_CODE)
dem_handle_event_queue_status(const dem_satellite_id_type satellite_id,
			      const dem_satellite_event_id_type satellite_event_id)
{
	dem_satellite_data_type old_satellite_data;
	dem_satellite_data_type new_satellite_data;
	dem_event_queue_status_type old_event_queue_status;
	int counter;

	for (counter = 0; counter < DEM_SATELLITE_CAS_MAX_COUNT; counter++) {
		old_satellite_data = dem_get_satellite_data(satellite_id, satellite_event_id);
		old_event_queue_status = dem_get_satellite_data_queue_status(old_satellite_data);
		new_satellite_data =
			dem_set_satellite_data_queue_status(old_satellite_data, dem_get_initial_queue_status());

		if (TRUE ==
		    sync_set_satellite_data(satellite_id, satellite_event_id, old_satellite_data, new_satellite_data)) {
			break;
		}
	}

	if (counter == DEM_SATELLITE_CAS_MAX_COUNT) {
		(void)VCOS_LOG(DEM_MODULE_ID, VCOS_LOG_ERROR, "Dem satellite %u handle event queue status failed \r\n",
			       satellite_id);
	}
	return old_event_queue_status;
}

FUNC(uint8, DEM_CODE) dem_get_satellite_state(dem_satellite_id_type satellite_id)
{
	dem_satellite_status_info_type satellite_info = get_satellite_status_info(satellite_id);

	return dem_get_satellite_info_state(satellite_info);
}

FUNC(boolean, DEM_CODE) dem_current_partition_is_master(void)
{
#if (DEM_SUPPORT_MULTIPLE_SATELLITES == STD_ON)
	if (os_get_current_app_id() != (ApplicationType)DEM_MASTER_APPLICATION_ID) {
		return FALSE;
	}
#endif

	return TRUE;
}

FUNC(dem_satellite_id_type, DEM_CODE) dem_get_current_satellite_id(void)
{
	dem_satellite_id_type satellite_id = (dem_satellite_id_type)0;

#if (DEM_SUPPORT_MULTIPLE_SATELLITES == STD_ON)
	ApplicationType current_application = get_current_application_id();

	for (satellite_id = 0u; satellite_id < DEM_MAX_NUMBER_SATELLITE; ++satellite_id) {
		if (get_satellite_application_id(satellite_id) == current_application) {
			break;
		}
	}
#endif

	if (satellite_id < DEM_MAX_NUMBER_SATELLITE) {
		return satellite_id;
	}

	DEM_REPORT_ERROR_RV(DEM_DEM_GETCURRENTAPPLICATIONID_ID_DEM_E_WRONG_CONDITION, 0u);
}

FUNC(boolean, DEM_CODE) dem_check_event_application_id(const dem_event_id_type event_id)
{
	boolean ret = (boolean)TRUE;

#if (DEM_SUPPORT_MULTIPLE_SATELLITES == STD_ON)
	ret = ((boolean)(get_current_application_id() == dem_get_event_application_id(event_id)));
#else
	(void)event_id;
#endif

	return ret;
}

FUNC(dem_satellite_id_type, DEM_CODE) dem_get_event_satellite_id(const dem_event_id_type event_id)
{
	const dem_event_parameter_type *event_param = NULL;

	dem_lookup_event_param(event_id, &event_param);
	if (NULL == event_param) {
		return (dem_satellite_id_type)0;
	}

	return event_param->satellite_id;
}

FUNC(dem_satellite_event_id_type, DEM_CODE) dem_get_satellite_event_id(const dem_event_id_type event_id)
{
	const dem_event_parameter_type *event_param = NULL;

	dem_lookup_event_param(event_id, &event_param);
	if (NULL == event_param) {
		return (dem_satellite_id_type)0;
	}

	return event_param->satellite_event_idx;
}

FUNC(dem_event_id_type, DEM_CODE)
dem_satellite_get_event_id(const dem_satellite_id_type satellite_id,
			   const dem_satellite_event_id_type satellite_event_id)
{
	return g_dem_satellite_event_table[g_dem_satellite_info[satellite_id].event_table_ind_start_idx +
					   satellite_event_id];
}

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"