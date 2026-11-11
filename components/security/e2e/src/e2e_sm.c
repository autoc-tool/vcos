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

/* including file */
#include "e2e_sm.h"
#include "e2e_internal.h"
#include "e2e_types.h"

/* local fuction declaration */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_sm_add_status()
 **********************************************************************************************************************/
/*! \brief          将最新的验证状态添加到状态机内
 *  \details        无
 *  \param[in]      profile_status        本次状态
 *  \param[inout]   state_ptr             指向当前状态机状态结构体的指针
 *  \param[in]      window_size           窗口大小
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_sm_add_status(uint8 profile_status,e2e_sm_check_state_type *state_ptr,
	uint8 window_size);

/***********************************************************************************************************************
 *  e2e_sm_update_counter()
 **********************************************************************************************************************/
/*! \brief          更新状态机内的计数器情况
 *  \details        无
 *  \param[inout]   state_ptr             指向当前状态机状态结构体的指针
 *  \param[in]      window_size           窗口大小
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_sm_update_counter(e2e_sm_check_state_type *state_ptr, uint8 window_size);

/***********************************************************************************************************************
 *  e2e_sm_check_handle_no_data()
 **********************************************************************************************************************/
/*! \brief          检查是否应该跳出nodata状态
 *  \details        无
 *  \param[in]      profile_status        本次状态
 *  \param[inout]   state_ptr             指向当前状态机状态结构体的指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_no_data(uint8 profile_status, e2e_sm_check_state_type *state_ptr);

/***********************************************************************************************************************
 *  e2e_sm_check_handle_init()
 **********************************************************************************************************************/
/*! \brief          检查是否应该跳出init状态
 *  \details        无
 *  \param[in]      config_ptr            指向当前状态机配置结构体的指针
 *  \param[inout]   state_ptr             指向当前状态机状态结构体的指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_init(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr,
	uint8 profile_status);

/***********************************************************************************************************************
 *  e2e_sm_check_handle_valid()
 **********************************************************************************************************************/
/*! \brief          检查是否应该跳出valid状态
 *  \details        无
 *  \param[in]      config_ptr            指向当前状态机配置结构体的指针
 *  \param[inout]   state_ptr             指向当前状态机状态结构体的指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_valid(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr,
	uint8 profile_status);

/***********************************************************************************************************************
 *  e2e_sm_check_handle_invalid()
 **********************************************************************************************************************/
/*! \brief          检查是否应该跳出invalid状态
 *  \details        无
 *  \param[in]      config_ptr            指向当前状态机配置结构体的指针
 *  \param[inout]   state_ptr             指向当前状态机状态结构体的指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_invalid(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr,
	uint8 profile_status);

/***********************************************************************************************************************
 *  e2e_sm_check_params()
 **********************************************************************************************************************/
/*! \brief          校验check函数的入参
 *  \details        无
 *  \param[in]      profile_status        本次状态
 *  \param[in]      config_ptr            指向当前状态机配置结构体的指针
 *  \param[inout]   state_ptr             指向当前状态机状态结构体的指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_sm_check_params(uint8 profile_status, const e2e_sm_config_type *config_ptr,
			const e2e_sm_check_state_type *state_ptr);

/***********************************************************************************************************************
 *  e2e_sm_check_params_check_profile_status()
 **********************************************************************************************************************/
/*! \brief          校验profile_statue入参的正确性
 *  \details        none
 *  \param[in]      profile_status
 *  \param[out]     none
 *  \return         E2E_E_OK - 校验正常 / others - 校验异常
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_sm_check_params_check_profile_status(uint8 profile_status);

/***********************************************************************************************************************
 *  e2e_sm_check_params_check_config_ptr()
 **********************************************************************************************************************/
/*! \brief          校验config_ptr入参的正确性
 *  \details        none
 *  \param[in]      config_ptr
 *  \param[out]     none
 *  \return         E2E_E_OK - 校验正常 / others - 校验异常
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE) e2e_sm_check_params_check_config_ptr(const e2e_sm_config_type *config_ptr);

/***********************************************************************************************************************
 *  e2e_sm_check_params_check_state_ptr()
 **********************************************************************************************************************/
/*! \brief          校验state_ptr入参的正确性
 *  \details        none
 *  \param[in]      config_ptr
 *  \param[in]      state_ptr
 *  \param[out]     none
 *  \return         E2E_E_OK - 校验正常 / others - 校验异常
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE) e2e_sm_check_params_check_state_ptr(const e2e_sm_config_type *config_ptr,
									      const e2e_sm_check_state_type *state_ptr);
#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

/* local fuction implementation */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_sm_add_status()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_add_status(uint8 profile_status, e2e_sm_check_state_type *state_ptr, uint8 window_size)
{
	state_ptr->profile_status_window[state_ptr->window_top_index] = profile_status;
	if (state_ptr->window_top_index >= (window_size - 1u)) {
		state_ptr->window_top_index = 0u;
	} else {
		state_ptr->window_top_index++;
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_sm_update_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_sm_update_counter(e2e_sm_check_state_type *state_ptr, uint8 window_size)
{
	state_ptr->error_count = 0u;
	state_ptr->ok_count = 0u;
	for (uint8 itr = 0u; itr < window_size; itr++) {
		if (state_ptr->profile_status_window[itr] == E2E_P_ERROR) {
			state_ptr->error_count++;
			continue;
		}
		if (state_ptr->profile_status_window[itr] == E2E_P_OK) {
			state_ptr->ok_count++;
			continue;
		}
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_sm_check_handle_no_data()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_no_data(uint8 profile_status, e2e_sm_check_state_type *state_ptr)
{
	if ((profile_status != E2E_P_ERROR) && (profile_status != E2E_P_NONEWDATA)) {
		state_ptr->sm_state = E2E_SM_INIT;
	} else {
		state_ptr->sm_state = E2E_SM_NODATA;
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_sm_check_handle_init()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_init(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr, uint8 profile_status)
{
	e2e_sm_add_status(profile_status, state_ptr, config_ptr->window_size);
	e2e_sm_update_counter(state_ptr, config_ptr->window_size);
	if ((state_ptr->error_count <= config_ptr->max_error_state_init) &&
	    (state_ptr->ok_count >= config_ptr->min_ok_state_init)) {
		state_ptr->sm_state = E2E_SM_VALID;
	} else if (state_ptr->error_count > config_ptr->max_error_state_init) {
		state_ptr->sm_state = E2E_SM_INVALID;
	} else {
		state_ptr->sm_state = E2E_SM_INIT;
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_sm_check_handle_valid()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_valid(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr,
	uint8 profile_status)
{
	e2e_sm_add_status(profile_status, state_ptr, config_ptr->window_size);
	e2e_sm_update_counter(state_ptr, config_ptr->window_size);
	if ((state_ptr->error_count <= config_ptr->max_error_state_valid) &&
	    (state_ptr->ok_count >= config_ptr->min_ok_state_valid)) {
		state_ptr->sm_state = E2E_SM_VALID;
	} else {
		state_ptr->sm_state = E2E_SM_INVALID;
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_sm_check_handle_invalid()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_sm_check_handle_invalid(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr,
	uint8 profile_status)
{
	e2e_sm_add_status(profile_status, state_ptr, config_ptr->window_size);
	e2e_sm_update_counter(state_ptr, config_ptr->window_size);
	if ((state_ptr->error_count <= config_ptr->max_error_state_invalid) &&
	    (state_ptr->ok_count >= config_ptr->min_ok_state_invalid)) {
		state_ptr->sm_state = E2E_SM_VALID;
	} else {
		state_ptr->sm_state = E2E_SM_INVALID;
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_sm_check_params_check_profile_status()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE) e2e_sm_check_params_check_profile_status(uint8 profile_status)
{
	Std_ReturnType ret = E2E_E_OK;
	if ((profile_status != E2E_P_OK) &&
		(profile_status != E2E_P_REPEATED) &&
		(profile_status != E2E_P_WRONGSEQUENCE) &&
		(profile_status != E2E_P_ERROR) &&
		(profile_status != E2E_P_NOTAVAILABLE) &&
		(profile_status != E2E_P_NONEWDATA)) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		ret = E2E_E_INPUTERR_WRONG;
	}
	return ret;
}

/***********************************************************************************************************************
 *  e2e_sm_check_params_check_config_ptr()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE) e2e_sm_check_params_check_config_ptr(const e2e_sm_config_type *config_ptr)
{
	if (config_ptr->window_size == 0u) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (config_ptr->min_ok_state_init > config_ptr->window_size) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (config_ptr->max_error_state_init > config_ptr->window_size) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (config_ptr->min_ok_state_valid > config_ptr->window_size) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (config_ptr->max_error_state_valid > config_ptr->window_size) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (config_ptr->min_ok_state_invalid > config_ptr->window_size) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (config_ptr->max_error_state_invalid > config_ptr->window_size) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_sm_check_params_check_state_ptr()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE) e2e_sm_check_params_check_state_ptr(const e2e_sm_config_type *config_ptr,
									      const e2e_sm_check_state_type *state_ptr)
{
	Std_ReturnType ret = E2E_E_OK;

	if (state_ptr->profile_status_window == NULL) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		ret = E2E_E_INPUTERR_WRONG;
	} else if (state_ptr->window_top_index >= config_ptr->window_size) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		ret = E2E_E_INPUTERR_WRONG;
	} else if (state_ptr->sm_state > E2E_SM_INVALID) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		ret = E2E_E_INPUTERR_WRONG;
	} else {
		ret = E2E_E_OK;
	}
	return ret;
}

/***********************************************************************************************************************
 *  e2e_sm_check_params()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_sm_check_params(uint8 profile_status, const e2e_sm_config_type *config_ptr,
			    const e2e_sm_check_state_type *state_ptr)
{
	Std_ReturnType ret = E2E_E_OK;
	ret = e2e_sm_check_params_check_profile_status(profile_status);
	if (ret != E2E_E_OK) {
		return ret;
	}
	ret = e2e_sm_check_params_check_config_ptr(config_ptr);
	if (ret != E2E_E_OK) {
		return ret;
	}
	ret = e2e_sm_check_params_check_state_ptr(config_ptr, state_ptr);
	if (ret != E2E_E_OK) {
		return ret;
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_sm_check()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_sm_check(uint8 profile_status, const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr)
{
	Std_ReturnType ret;

	if ((state_ptr == NULL) || (config_ptr == NULL)) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	ret = e2e_sm_check_params(profile_status, config_ptr, state_ptr);
	if (ret != E2E_E_OK) {
		return ret;
	} else if (state_ptr->sm_state == E2E_SM_DEINIT) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_WRONGSTATE);
		ret = E2E_E_WRONGSTATE;
	} else if (state_ptr->sm_state == E2E_SM_NODATA) {
		e2e_sm_check_handle_no_data(profile_status, state_ptr);
	} else if (state_ptr->sm_state == E2E_SM_INIT) {
		e2e_sm_check_handle_init(config_ptr, state_ptr, profile_status);
	} else if (state_ptr->sm_state == E2E_SM_VALID) {
		e2e_sm_check_handle_valid(config_ptr, state_ptr, profile_status);
	} else if (state_ptr->sm_state == E2E_SM_INVALID) {
		e2e_sm_check_handle_invalid(config_ptr, state_ptr, profile_status);
	} else {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INTERR);
		ret = E2E_E_INTERR;
	}
	return ret;
}

/***********************************************************************************************************************
 *  e2e_sm_check_init()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_sm_check_init(const e2e_sm_config_type *config_ptr, e2e_sm_check_state_type *state_ptr)
{
	if ((state_ptr == NULL) || (config_ptr == NULL)) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	if ((state_ptr->profile_status_window == NULL) && (config_ptr->window_size > 0u)) {
		E2E_DET_REPORT(E2E_E2E_SM_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	for (uint8 i = 0u; i < config_ptr->window_size; i++) {
		state_ptr->profile_status_window[i] = E2E_P_NOTAVAILABLE;
	}
	state_ptr->ok_count = 0u;
	state_ptr->error_count = 0u;
	state_ptr->window_top_index = 0u;
	state_ptr->sm_state = E2E_SM_NODATA;

	return E2E_E_OK;
}

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"
