/*
 * copyright (c) 2025 li auto inc. and its affiliates
 * licensed under the apache license, version 2.0(the "License");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#ifndef E2E_P01_H
#define E2E_P01_H

#include "e2e_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 定义e2e p01数据id模式的类型 */
#define E2E_P01_DATAID_BOTH             0u
#define E2E_P01_DATAID_ALT              1u
#define E2E_P01_DATAID_LOW              2u
#define E2E_P01_DATAID_NIBBLE           3u
#define E2E_P01_DATAID_INVALID          4u

/* 定义e2e p01检查状态类型 */
#define E2E_P01_STATUS_OK              0x08u
#define E2E_P01_STATUS_OKSOMELOST      0x09u
#define E2E_P01_STATUS_SYNC            0x0Au
#define E2E_P01_STATUS_REPEATED        0x10u
#define E2E_P01_STATUS_WRONGSEQUENCE   0x20u
#define E2E_P01_STATUS_INITIAL         0x21u
#define E2E_P01_STATUS_WRONGCRC        0x40u
#define E2E_P01_STATUS_NONEWDATA       0x80u

/*!*********************************************************************************************************************
 * @brief   定义e2e p01配置数据的结构体类型
 **********************************************************************************************************************/
typedef struct {
	uint16 counter_offset;
	uint16 crc_offset;
	uint16 data_id;
	uint16 data_id_nibble_offset;
	uint16 data_length;
	uint8 data_id_mode;
	uint8 max_delta_counter_init;
	uint8 max_no_update_counter;
	uint8 sync_counter_init;
} e2e_p01_config_type;

/*!*********************************************************************************************************************
 * @brief   定义e2e p01保护状态类型
 **********************************************************************************************************************/
typedef struct {
	uint8 counter;
} e2e_p01_protect_state_type;

/*!*********************************************************************************************************************
 * @brief   定义e2e p01检查状态类型
 **********************************************************************************************************************/
typedef struct {
	uint8 e2e_status;
	uint8 last_valid_counter;
	uint8 max_delta_counter;
	uint8 lost_data;
	uint8 sync_counter;
	uint8 no_update_counter;
	boolean is_wait_for_first_data;
	boolean is_new_data_available;
} e2e_p01_check_state_type;

#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/*!*********************************************************************************************************************
 * @brief          E2E profile 1的保护函数
 * @details        使用e2e profile 1保护要传输的数组/缓冲区
 * @param[in]      config_ptr     指向静态配置的指针
 * @param[out]     state_ptr      指向通信状态的指针
 * @param[out]     data_ptr       指向要保护的数据的指针
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_INPUTERR_WRONG 参数错误
 * @retval         E2E_E_INTERR 内部库错误
 * @retval         E2E_E_OK 保护成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_protect(const e2e_p01_config_type *config_ptr, e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr);

/*!*********************************************************************************************************************
 * @brief          E2E profile 1的保护初始化函数
 * @details        通过重置计数器来初始化保护状态
 * @param[out]     state_ptr 指向通信状态的指针，用于记录保护状态
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_OK 保护成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p01_protect_init(e2e_p01_protect_state_type *state_ptr);

/*!*********************************************************************************************************************
 * @brief          E2E profile 1的检查函数
 * @details        使用e2e profile 1对接收到的数据进行检查。包括crc计算、处理计数器和数据id
 * @param[in]      config_ptr     指向配置文件的指针，用于配置检查规则
 * @param[out]     state_ptr      指向通信状态的指针，用于记录检查状态
 * @param[out]     data_ptr       指向要检查的数据的指针
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_INPUTERR_WRONG 参数错误
 * @retval         E2E_E_INTERR 内部库错误
 * @retval         E2E_E_OK 检查成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_check(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr, const uint8 *data_ptr);

/*!*********************************************************************************************************************
 * @brief          E2E profile 1的检查初始化函数
 * @details        E2E profile 1的检查初始化函数
 * @param[out]     state_ptr      指向通信状态的指针
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_OK 检查成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p01_check_init(e2e_p01_check_state_type *state_ptr);

/*!*********************************************************************************************************************
 * @brief          E2E profile 1的状态映射函数
 * @details        将profile 1的检查状态映射为通用的检查状态，可由e2e状态机的检查函数使用
 *                  E2E profile 1提供了更精细的状态，但对于e2e状态机来说这并不相关
 * @param[in]     check_return     e2e_p01_check的返回值
 * @param[in]     check_status          通信状态
 * @param[in]     profile_behavior FALSE: 检查具有r4.2之前的传统行为
 *                                TRUE:  检查具有r4.2引入的新的p4/P5/p6配置文件的行为
 * @retval        用于e2elib状态机的标准状态值
 * @reentrant     不可重入
 * @synchronous   同步
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE)
e2e_p01_map_status_to_sm(Std_ReturnType check_return, uint8 check_status, boolean profile_behavior);
#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* E2E_P01_H */
