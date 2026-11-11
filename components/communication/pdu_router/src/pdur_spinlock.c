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

/* including file */
#include "pdur_spinlock.h"
#include "pdur_types.h"
#include "pdur_bufferpoolcfg.h"
#include "pdur_evcfifo.h"
#include "pdur_cfg.h"
#include "pdur_internal.h"
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
#include "Os.h"
#include "Spinlock_Cfg.h"
#endif
#endif

/*!*********************************************************************************************************************
 * @brief          获取缓存自旋锁
 * @details        获取缓存自旋锁
 * @param[in]      spinlock_id     自旋锁编号
 * @retval         E_OK           获取缓存自旋锁成功
 * @retval         E_NOT_OK       获取缓存自旋锁失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_buffer_spin_lock(SpinlockIdType spinlock_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

/*!*********************************************************************************************************************
 * @brief          释放缓存自旋锁
 * @details        释放缓存自旋锁
 * @param[in]      spinlock_id     自旋锁编号
 * @retval         E_OK           释放缓存自旋锁成功
 * @retval         E_NOT_OK       释放缓存自旋锁失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_buffer_spin_lock(SpinlockIdType spinlock_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_buffer_spin_lock(SpinlockIdType spinlock_id)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	uint8 pdur_try_spin_lock_time = 0u;
	try_to_get_spinlock_type lock_result = TRYTOGETSPINLOCK_NOSUCCESS;

	do {
		(void)try_to_get_spinlock(spinlock_id, &lock_result);
		if (lock_result == TRYTOGETSPINLOCK_SUCCESS) {
			return E_OK;
		} else {
			pdur_try_spin_lock_time++;
		}
	} while (pdur_try_spin_lock_time < PDUR_SPINLOCK_RETRY_COUNTER);
	PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_GETSPINLOCK_GET_FAILED);
	return E_NOT_OK;
#else
	(void)spinlock_id;
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_buffer_spin_lock(SpinlockIdType spinlock_id)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	uint8 pdur_try_spin_lock_time = 0u;
	status_type result;

	do {
		result = release_spinlock(spinlock_id);
		if (result == E_OK) {
			return E_OK;
		} else {
			pdur_try_spin_lock_time++;
		}
	} while (pdur_try_spin_lock_time < PDUR_SPINLOCK_RETRY_COUNTER);
	PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_RELEASESPINLOCK_GET_FAILED);
	return E_NOT_OK;
#else
	(void)spinlock_id;
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_buf_mngr_busy_list_spin_lock(void)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	return pdur_get_buffer_spin_lock((SpinlockIdType)PDUR_BUFFER_MANAGER_BUSYLIST_SPINLOCK_ID);
#else
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_rel_buf_mngr_busy_list_spin_lock(void)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	return pdur_release_buffer_spin_lock((SpinlockIdType)PDUR_BUFFER_MANAGER_BUSYLIST_SPINLOCK_ID);
#else
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_buffer_pool_spin_lock(uint8 buf_pool_idx)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	const pdur_buffer_manager_header_type *const buf_pool_list = pdur_get_buffer_manager_header_list_addr();

	if (buf_pool_list == NULL) {
		return E_NOT_OK;
	}
	uint8 buf_pool_num = pdur_get_buffer_manager_header_len();

	if (buf_pool_idx >= buf_pool_num) {
		return E_NOT_OK;
	}
	return pdur_get_buffer_spin_lock(buf_pool_list[buf_pool_idx].spinlock_id);
#else
	(void)buf_pool_idx;
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_buffer_pool_spin_lock(uint8 buf_pool_idx)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	const pdur_buffer_manager_header_type *const buf_pool_list = pdur_get_buffer_manager_header_list_addr();

	if (buf_pool_list == NULL) {
		return E_NOT_OK;
	}
	uint8 buf_pool_num = pdur_get_buffer_manager_header_len();

	if (buf_pool_idx >= buf_pool_num) {
		return E_NOT_OK;
	}
	return pdur_release_buffer_spin_lock(buf_pool_list[buf_pool_idx].spinlock_id);
#else
	(void)buf_pool_idx;
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_evc_fifo_spin_lock(const pdur_evc_fifo_desc_type *const p_evc_fifo_desc)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	if (p_evc_fifo_desc == NULL) {
		return E_NOT_OK;
	}
	return pdur_get_buffer_spin_lock(p_evc_fifo_desc->spinlock_id);
#else
	(void)p_evc_fifo_desc;
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_evc_fifo_spin_lock(const pdur_evc_fifo_desc_type *const p_evc_fifo_desc)
{
#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#if defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1)
	if (p_evc_fifo_desc == NULL) {
		return E_NOT_OK;
	}
	return pdur_release_buffer_spin_lock(p_evc_fifo_desc->spinlock_id);
#else
	(void)p_evc_fifo_desc;
	return E_OK;
#endif
#else
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
