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

#ifndef PDUR_SPINLOCK_H
#define PDUR_SPINLOCK_H

#include "pdur_types.h"
#include "pdur_evcfifo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          尝试获取buffer_manager的自旋锁
 * @param[in]      void
 * @retval         E_OK                      获取成功
 * @retval         E_NOT_OK                  获取失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_buf_mngr_busy_list_spin_lock(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          尝试释放buffer_manager的自旋锁
 * @param[in]      void
 * @retval         E_OK                      获取成功
 * @retval         E_NOT_OK                  获取失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_rel_buf_mngr_busy_list_spin_lock(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          尝试获取buffer_pool的自旋锁
 * @param[in]      buf_pool_idx                缓存池编号
 * @retval         E_OK                      获取成功
 * @retval         E_NOT_OK                  获取失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_buffer_pool_spin_lock(uint8 buf_pool_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          尝试释放buffer_pool的自旋锁
 * @param[in]      buf_pool_idx                缓存池编号
 * @retval         E_OK                      获取成功
 * @retval         E_NOT_OK                  获取失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_buffer_pool_spin_lock(uint8 buf_pool_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          尝试获取evc_fifo的自旋锁
 * @param[in]      p_evc_fifo_desc              evc管理指针
 * @retval         E_OK                      获取成功
 * @retval         E_NOT_OK                  获取失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_get_evc_fifo_spin_lock(const pdur_evc_fifo_desc_type *const p_evc_fifo_desc);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          尝试释放evc_fifo的自旋锁
 * @param[in]      p_evc_fifo_desc              evc管理指针
 * @retval         E_OK                      获取成功
 * @retval         E_NOT_OK                  获取失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_evc_fifo_spin_lock(const pdur_evc_fifo_desc_type *const p_evc_fifo_desc);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#ifdef __cplusplus
}
#endif

#endif
