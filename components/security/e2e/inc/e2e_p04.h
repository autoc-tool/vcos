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

#ifndef E2E_P04_H
#define E2E_P04_H

#include "e2e_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 定义e2e p04校验状态类型 */
#define E2E_P04_STATUS_OK                        0x08u
#define E2E_P04_STATUS_OKSOMELOST                0x09u
#define E2E_P04_STATUS_REPEATED                  0x10u
#define E2E_P04_STATUS_WRONGSEQUENCE             0x20u
#define E2E_P04_STATUS_ERROR                     0x40u
#define E2E_P04_STATUS_NONEWDATA                 0x80u

/*!*********************************************************************************************************************
 * @brief   定义e2e p04配置信息类型
 **********************************************************************************************************************/
typedef struct {
	uint32 data_id;
	uint16 offset;
	uint16 min_data_length;
	uint16 max_data_length;
	uint8 max_delta_counter;
} e2e_p04_config_type;

/*!*********************************************************************************************************************
 * @brief   定义e2e p04保护状态类型
 **********************************************************************************************************************/
typedef struct {
	uint16 counter;
} e2e_p04_protect_state_type;

/*!*********************************************************************************************************************
 * @brief   定义e2e p04校验状态类型
 **********************************************************************************************************************/
typedef struct {
	uint16 counter;
	uint8 e2e_status;
} e2e_p04_check_state_type;

#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/*!*********************************************************************************************************************
 * @brief          E2E profile 4的保护函数
 * @details        使用e2e profile 4保护要传输的数组/缓冲区
 * @param[in]      config_ptr     指向静态配置的指针
 * @param[out]     state_ptr      指向通信状态的指针
 * @param[out]     data_ptr       指向要保护的数据的指针
 * @param[in]      length        数据数组长度
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_INPUTERR_WRONG 参数错误
 * @retval         E2E_E_INTERR 内部库错误
 * @retval         E2E_E_OK 保护成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p04_protect(const e2e_p04_config_type *config_ptr, e2e_p04_protect_state_type *state_ptr, uint8 *data_ptr,
	uint16 length);

/*!*********************************************************************************************************************
 * @brief          E2E profile 4的保护初始化函数
 * @details        通过重置计数器来初始化保护状态
 * @param[out]     state_ptr 指向通信状态的指针，用于记录保护状态
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_OK 保护成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p04_protect_init(e2e_p04_protect_state_type *state_ptr);

/*!*********************************************************************************************************************
 * @brief          E2E profile 2的检查函数
 * @details        使用e2e profile 2对接收到的数据进行检查。包括crc计算、处理计数器和数据id
 * @param[in]      config_ptr     指向配置文件的指针，用于配置检查规则
 * @param[out]     state_ptr      指向通信状态的指针，用于记录检查状态
 * @param[out]     data_ptr       指向要检查的数据的指针
 * @param[in]      length        数据数组长度
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_INPUTERR_WRONG 参数错误
 * @retval         E2E_E_OK 检查成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p04_check(const e2e_p04_config_type *config_ptr, e2e_p04_check_state_type *state_ptr, const uint8 *data_ptr,
	     uint16 length);

/*!*********************************************************************************************************************
 * @brief          初始化p04 check方法
 * @details        初始化p04 check方法
 * @param[out]     state_ptr 状态结构体指针
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_OK 保护成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p04_check_init(e2e_p04_check_state_type *state_ptr);

/*!*********************************************************************************************************************
 * @brief          将p04check方法的结果转换为状态机的状态
 * @details        将p04check方法的结果转换为状态机的状态
 * @param[in]      check_return             本次校验函数返回结果
 * @param[in]      check_status             本次校验函数校验结果
 * @retval         E2E_P_OK                正常状态
 * @retval         E2E_P_REPEATED          重复
 * @retval         E2E_P_NONEWDATA         未收到新数据
 * @retval         E2E_P_WRONGSEQUENCE     顺序错误
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE) e2e_p04_map_status_to_sm(Std_ReturnType check_return, uint8 check_status);

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* E2E_P04_H */
