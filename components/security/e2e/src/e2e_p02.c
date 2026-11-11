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
#include "e2e_p02.h"
#include "e2e_internal.h"
#include "e2e.h"
#include "e2e_types.h"

/* local macro definition */
#define E2E_P02_MAX_COUNTER_VALUE 15u
#define E2E_P02_DELTA_COUNTER_RENEW 16u
#define MAX_P02_DATA_LENGTH_IN_BITS (8u * 256u)
#define MIN_P02_DATA_LENGTH_IN_BITS (8u * 2u)
#define E2E_P02_RECV_CNT_MASK 0x0fu
#define E2E_P02_HIGH_NIBBLE_MASK 0xf0u
#define E2E_P02_CRC_OFFSET 0u
#define E2E_P02_RECV_CNT_OFFSET 1u

/* local fuction declaration */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_p02_calculate_delta_counter()
 **********************************************************************************************************************/
/*! \brief          根据received_counter和last_valid_counter计算delta_counter
 *  \details        无
 *  \param[in]      received_counter         本次收到报文中携带的计数器值
 *  \param[in]      last_valid_counter        e2e中保存的上次报文对应的计数器值
 *  \return         delta_counter            两个入参的差值
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(uint8, E2E_CODE) e2e_p02_calculate_delta_counter(uint8 received_counter, uint8 last_valid_counter);

/***********************************************************************************************************************
 *  e2e_check_config()
 **********************************************************************************************************************/
/*! \brief          根据received_counter和last_valid_counter计算delta_counter
 *  \details        无
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \return         E2E_E_INPUTERR_NULL     入参存在空指针
 *                  E2E_E_INPUTERR_WRONG    入参错误
 *                  E2E_E_OK                正常
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE) e2e_check_config(const e2e_p02_config_type *config_ptr);

/***********************************************************************************************************************
 *  e2e_p02_handle_wrong_sequence()
 **********************************************************************************************************************/
/*! \brief          实现二校验wrong_sequence分支后处理
 *  \details        参见: autosar_prs_e2e_protocol - figure 6.25
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      received_counter         本次收到报文中携带的计数器值
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p02_handle_wrong_sequence(const e2e_p02_config_type *config_ptr,
						e2e_p02_check_state_type *state_ptr, uint8 received_counter);

/***********************************************************************************************************************
 *  e2e_p02_handle_ok_or_some_lost()
 **********************************************************************************************************************/
/*! \brief          实现二校验ok_or_some_lost分支后处理
 *  \details        参见: autosar_prs_e2e_protocol - figure 6.26
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      received_counter         本次收到报文中携带的计数器值
 *  \param[in]      delta_counter            received_counter与last_valid_counter的差值
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p02_handle_ok_or_some_lost(const e2e_p02_config_type *config_ptr, e2e_p02_check_state_type *state_ptr,
				uint8 received_counter, uint8 delta_counter);

/***********************************************************************************************************************
 *  e2e_process_counter()
 **********************************************************************************************************************/
/*! \brief          实现二校验流程处理计数器过程
 *  \details        参见: autosar_prs_e2e_protocol - figure 6.19 - e2e_p02__process_counter
					参见: autosar_prs_e2e_protocol - figure 6.22
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      received_counter         本次收到报文中携带的计数器值
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_process_counter(const e2e_p02_config_type *config_ptr,
						e2e_p02_check_state_type *state_ptr, uint8 received_counter);

/***********************************************************************************************************************
 *  e2e_p02_check_state_and_data()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p02_check_state_and_data(const e2e_p02_config_type *config_ptr, const e2e_p02_check_state_type *state_ptr,
			const uint8 *data_ptr);

/***********************************************************************************************************************
 *  e2e_p02_check_no_new_or_repeated_data_counter()
 **********************************************************************************************************************/
/*! \brief          实现二校验流程检查是否为新数据分支
 *  \details
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p02_check_no_new_or_repeated_data_counter(e2e_p02_check_state_type *state_ptr);

/***********************************************************************************************************************
 *  e2e_p02_check_crc()
 **********************************************************************************************************************/
/*! \brief          实现二校验流程校验crc
 *  \details        参见: autosar_prs_e2e_protocol figure 6.19
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      received_counter         本次收到报文中携带的计数器值
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p02_check_crc(const e2e_p02_config_type *config_ptr,
						e2e_p02_check_state_type *state_ptr, const uint8 *data_ptr);

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

/* local fuction implementation */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_p02_calculate_delta_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(uint8, E2E_CODE) e2e_p02_calculate_delta_counter(uint8 received_counter, uint8 last_valid_counter)
{
	uint8 delta_counter;

	if (received_counter >= last_valid_counter) {
		delta_counter = received_counter - last_valid_counter;
	} else {
		delta_counter = E2E_P02_DELTA_COUNTER_RENEW + received_counter - last_valid_counter;
	}
	return delta_counter;
}

/***********************************************************************************************************************
 *  e2e_check_config()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE) e2e_check_config(const e2e_p02_config_type *config_ptr)
{
	if (config_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	if (((config_ptr->data_length % 8u) != 0u) || (config_ptr->data_length < MIN_P02_DATA_LENGTH_IN_BITS) ||
		(config_ptr->data_length > MAX_P02_DATA_LENGTH_IN_BITS)) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (config_ptr->offset > (MAX_P02_DATA_LENGTH_IN_BITS - MIN_P02_DATA_LENGTH_IN_BITS)) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p02_protect()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p02_protect(const e2e_p02_config_type *config_ptr, e2e_p02_protect_state_type *state_ptr, uint8 *data_ptr)
{
	Std_ReturnType return_value = e2e_check_config(config_ptr);

	if (return_value != E2E_E_OK) {
		return return_value;
	}
	if ((state_ptr == NULL) || (data_ptr == NULL)) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->counter = (state_ptr->counter + 1u) % (E2E_P02_MAX_COUNTER_VALUE + 1u);
	data_ptr[E2E_P02_RECV_CNT_OFFSET] = (data_ptr[E2E_P02_RECV_CNT_OFFSET] & E2E_P02_HIGH_NIBBLE_MASK) |
					(state_ptr->counter & E2E_P02_RECV_CNT_MASK);

	uint8 data_id = config_ptr->data_id_list[state_ptr->counter];
	uint8 data_length_in_byte = (uint8)((config_ptr->data_length / 8u) -
					1u);
	uint8 calc_crc =
		e2e_crc_calculate_crc8h2f(&data_ptr[E2E_P02_RECV_CNT_OFFSET], data_length_in_byte, 0x00u, TRUE);

		calc_crc = e2e_crc_calculate_crc8h2f(&data_id, 1u, calc_crc, FALSE);
	data_ptr[E2E_P02_CRC_OFFSET] = calc_crc;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p02_handle_wrong_sequence()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p02_handle_wrong_sequence(const e2e_p02_config_type *config_ptr,
							    e2e_p02_check_state_type *state_ptr, uint8 received_counter)
{
	state_ptr->no_update_counter = 0u;
	state_ptr->sync_counter = config_ptr->sync_counter_init;
	if (state_ptr->sync_counter > 0u) {
		state_ptr->max_delta_counter = config_ptr->max_delta_counter_init;
		state_ptr->last_valid_counter = received_counter;
	}
	state_ptr->e2e_status = E2E_P02_STATUS_WRONGSEQUENCE;
	return;
}

/***********************************************************************************************************************
 *  e2e_p02_handle_ok_or_some_lost()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p02_handle_ok_or_some_lost(const e2e_p02_config_type *config_ptr, e2e_p02_check_state_type *state_ptr,
				      uint8 received_counter, uint8 delta_counter)
{
	state_ptr->max_delta_counter = config_ptr->max_delta_counter_init;
	state_ptr->last_valid_counter = received_counter;
	state_ptr->lost_data = delta_counter - 1u;
	if (state_ptr->no_update_counter <= config_ptr->max_no_update_counter) {
		if (state_ptr->sync_counter > 0u) {
			state_ptr->sync_counter--;
			state_ptr->e2e_status = E2E_P02_STATUS_SYNC;
		} else {
			if (delta_counter == 1u) {
				state_ptr->e2e_status = E2E_P02_STATUS_OK;
			} else {
				state_ptr->e2e_status = E2E_P02_STATUS_OKSOMELOST;
			}
		}
	} else {
		state_ptr->sync_counter = config_ptr->sync_counter_init;
		state_ptr->e2e_status = E2E_P02_STATUS_SYNC;
	}
	state_ptr->no_update_counter = 0u;
	return;
}

/***********************************************************************************************************************
 *  e2e_process_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_process_counter(const e2e_p02_config_type *config_ptr,
						   e2e_p02_check_state_type *state_ptr, uint8 received_counter)
{
	uint8 delta_counter = e2e_p02_calculate_delta_counter(received_counter, state_ptr->last_valid_counter);

	if (delta_counter == 0u) {
		if (state_ptr->no_update_counter < E2E_P02_MAX_COUNTER_VALUE) {
			state_ptr->no_update_counter++;
		}
		state_ptr->e2e_status = E2E_P02_STATUS_REPEATED;
	} else if (delta_counter > state_ptr->max_delta_counter) {
		e2e_p02_handle_wrong_sequence(config_ptr, state_ptr, received_counter);
	} else {
		e2e_p02_handle_ok_or_some_lost(config_ptr, state_ptr, received_counter, delta_counter);
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_p02_check_state_and_data()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p02_check_state_and_data(const e2e_p02_config_type *config_ptr, const e2e_p02_check_state_type *state_ptr,
				const uint8 *data_ptr)
{
	if ((state_ptr == NULL) || (data_ptr == NULL)) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	if ((state_ptr->last_valid_counter > E2E_P02_MAX_COUNTER_VALUE) ||
	    (state_ptr->max_delta_counter > E2E_P02_MAX_COUNTER_VALUE) ||
	    (state_ptr->sync_counter > config_ptr->sync_counter_init) ||
	    (state_ptr->no_update_counter > E2E_P02_MAX_COUNTER_VALUE)) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p02_check_no_new_or_repeated_data_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p02_check_no_new_or_repeated_data_counter(e2e_p02_check_state_type *state_ptr)
{
	if (state_ptr->no_update_counter < E2E_P02_MAX_COUNTER_VALUE) { /* 只允许增加到15 */
		state_ptr->no_update_counter++;
	}
	state_ptr->e2e_status = E2E_P02_STATUS_NONEWDATA;
	return;
}

/***********************************************************************************************************************
 *  e2e_p02_check_crc()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p02_check_crc(const e2e_p02_config_type *config_ptr,
						e2e_p02_check_state_type *state_ptr, const uint8 *data_ptr)
{
	uint8 received_counter = data_ptr[E2E_P02_RECV_CNT_OFFSET] & E2E_P02_RECV_CNT_MASK;
	uint8 data_id = config_ptr->data_id_list[received_counter];
	uint8 data_length_in_byte = (uint8)((config_ptr->data_length / 8u) - 1u);
	uint8 calc_crc = e2e_crc_calculate_crc8h2f(&data_ptr[E2E_P02_RECV_CNT_OFFSET], data_length_in_byte, 0x00u, TRUE);

	calc_crc = e2e_crc_calculate_crc8h2f(&data_id, 1u, calc_crc, FALSE);
	if (calc_crc != data_ptr[E2E_P02_CRC_OFFSET]) {
		state_ptr->e2e_status = E2E_P02_STATUS_WRONGCRC;
	} else if (state_ptr->is_wait_for_first_data == TRUE) {
		state_ptr->is_wait_for_first_data = FALSE;
		state_ptr->max_delta_counter = config_ptr->max_delta_counter_init;
		state_ptr->last_valid_counter = received_counter;
		state_ptr->e2e_status = E2E_P02_STATUS_INITIAL;
	} else {
		e2e_process_counter(config_ptr, state_ptr, received_counter);
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_p02_check()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p02_check(const e2e_p02_config_type *config_ptr, e2e_p02_check_state_type *state_ptr, const uint8 *data_ptr)
{
	Std_ReturnType return_value = E2E_E_OK;

	return_value = e2e_check_config(config_ptr);
	if (return_value != E2E_E_OK) {
		return return_value;
	}
	return_value = e2e_p02_check_state_and_data(config_ptr, state_ptr, data_ptr);
	if (return_value != E2E_E_OK) {
		return return_value;
	}
	if (state_ptr->max_delta_counter < E2E_P02_MAX_COUNTER_VALUE) {
		state_ptr->max_delta_counter++;
	}
	if (state_ptr->is_new_data_available == FALSE) {
		e2e_p02_check_no_new_or_repeated_data_counter(state_ptr);
	} else {
		e2e_p02_check_crc(config_ptr, state_ptr, data_ptr);
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p02_protect_init()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p02_protect_init(e2e_p02_protect_state_type* state_ptr)
{
	if (state_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->counter = 0u;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p02_check_init()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p02_check_init(e2e_p02_check_state_type *state_ptr)
{
	if (state_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P02_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->last_valid_counter = 0u;
	state_ptr->max_delta_counter = 0u;
	state_ptr->is_wait_for_first_data = TRUE;
	state_ptr->is_new_data_available = FALSE;
	state_ptr->lost_data = 0u;
	state_ptr->e2e_status = E2E_P02_STATUS_NONEWDATA;
	state_ptr->sync_counter = 0u;
	state_ptr->no_update_counter = 0u;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p02_map_status_to_sm()
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE) e2e_p02_map_status_to_sm(Std_ReturnType check_return, uint8 check_status,
						boolean profile_behavior)
{
	uint8 ret_value = E2E_P_ERROR;

	if (check_return != E2E_E_OK) {
		return ret_value;
	}
	ret_value = check_status & E2E_P_MAP_MASK;
	if (profile_behavior == FALSE) {
		ret_value = ret_value ^ E2E_P_XOR_MASK;
	}
	return ret_value;
}

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"