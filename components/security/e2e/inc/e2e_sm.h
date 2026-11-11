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

#ifndef E2E_SM_H
#define E2E_SM_H

#include "e2e.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 定义状态机状态类型 */
#define E2E_SM_VALID                        0x00u
#define E2E_SM_DEINIT                       0x01u
#define E2E_SM_NODATA                       0x02u
#define E2E_SM_INIT                         0x03u
#define E2E_SM_INVALID                      0x04u
/*!*********************************************************************************************************************
 * @brief   定义状态机配置类型
 **********************************************************************************************************************/
typedef struct {
	uint8 window_size;
	uint8 min_ok_state_init;
	uint8 max_error_state_init;
	uint8 min_ok_state_valid;
	uint8 max_error_state_valid;
	uint8 min_ok_state_invalid;
	uint8 max_error_state_invalid;
} e2e_sm_config_type;

/*!*********************************************************************************************************************
 * @brief   定义状态机校验状态类型
 **********************************************************************************************************************/
typedef struct {
	uint8 *profile_status_window;
	uint8 sm_state;
	uint8 window_top_index;
	uint8 ok_count;
	uint8 error_count;
} e2e_sm_check_state_type;

#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/*!*********************************************************************************************************************
 * @brief          检查本次profile调用的结果，并根据结果更新状态机状态
 * @param[in]      profile_status 本次状态
 * @param[in]      config_ptr     指向配置文件的指针，用于配置检查规则
 * @param[out]     state_ptr      指向通信状态的指针，用于记录检查状态
 * @retval         E2E_E_INTERR         函数运行异常
 * @retval         E2E_E_INPUTERR_NULL 指针参数为空
 * @retval         E2E_E_INPUTERR_WRONG 参数错误
 * @retval         E2E_E_OK 检查成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_sm_check(uint8 profile_status, const e2e_sm_config_type *config_ptr,
		e2e_sm_check_state_type *state_ptr);

/*!*********************************************************************************************************************
 * @brief          初始化状态机数据，将状态切回deinit
 * @details        初始化状态机数据，将状态切回deinit
 * @param[in]      config_ptr            指向当前状态机配置结构体的指针
 * @param[in]      state_ptr             指向当前状态机状态结构体的指针
 * @retval         E2E_E_OK             函数正常返回
 * @retval         E2E_E_WRONGSTATE     状态机未初始化
 * @retval         E2E_E_INPUTERR_NULL  入参有空指针
 * @retval         E2E_E_INTERR         函数运行异常
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_sm_check_init(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr);

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* E2E_SM_H */
