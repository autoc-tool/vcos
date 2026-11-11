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

#ifndef DEM_ATOMIC_H
#define DEM_ATOMIC_H

#include "dem_types.h"
#include "dem_lcfg.h"
#include "dem_lcfg_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 * @brief          默认同步比较与交换
 * @details        默认同步比较与交换
 * @param[in]      address_ptr 地址指针
 * @param[in]      old_value 旧值
 * @param[in]      new_value 新值
 * @retval         TRUE  默认同步比较与交换成功
 * @retval         FALSE 默认同步比较与交换失败
 * @reentrant      TRUE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(boolean, DEM_CODE)
dem_atomic_cmp_swap(volatile dem_atomic_word_type *address_ptr, const dem_atomic_word_type old_value,
		    const dem_atomic_word_type new_value);

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_ATOMIC_H */
