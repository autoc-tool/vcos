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
#include "e2e_p05.h"
#include "e2e_internal.h"
#include "e2e.h"
#include "e2e_types.h"

/* local macro definition */
#define MAX_P05_COUNTER_VALUE 0xffu
#define MAX_P05_DATA_LENGTH_IN_BITS 32768u
#define MIN_P05_DATA_LENGTH_IN_BITS 24u

/* local enumeration definition */

/* local type & structure definition */

/* global variable definition */

/* local variable definition */

/* local function declaration */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_p05_check_check_input_pointers()
 **********************************************************************************************************************/
/*! \brief          检查e2e_p05_check函数的入参指针合法性
 *  \details        none
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      data_ptr                 指向实际数据的指针
 *  \param[in]      length                  实际数据的长度
 *  \return         检查结果
 *  \context        ANY|TASK|ISR1|ISR2|KERNEL
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p05_check_check_input_pointers(const e2e_p05_config_type *config_ptr, const e2e_p05_check_state_type *state_ptr,
				const uint8 *data_ptr, uint16 length);

/***********************************************************************************************************************
 *  e2e_p05_check_check_offset_and_length()
 **********************************************************************************************************************/
/*! \brief          检查e2e_p05_check函数的入参长度和偏移量的合法性
 *  \details        none
 *  \param[out]     is_new_data_available        是否收到新数据的标志位
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[in]      data_ptr                 指向实际数据的指针
 *  \param[in]      length                  实际数据的长度
 *  \return         检查结果
 *  \context        ANY|TASK|ISR1|ISR2|KERNEL
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p05_check_check_offset_and_length(boolean *is_new_data_available, const e2e_p05_config_type *config_ptr,
					  const uint8 *data_ptr, uint16 length);

/***********************************************************************************************************************
 *  e2e_p05_calculate_crc()
 **********************************************************************************************************************/
/*! \brief          根据接收报文数据计算crc
 *  \details        none
 *  \param[in]      data_ptr                 指向实际数据的指针
 *  \param[in]      length                  实际数据的长度
 *  \param[in]      offset                  p05方法信息数据在报文中的偏移量
 *  \param[in]      data_id                  p05方法的data_id
 *  \return         计算得到的crc值
 *  \context        ANY|TASK|ISR1|ISR2|KERNEL
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(uint16, E2E_CODE)
e2e_p05_calculate_crc(const uint8 *data_ptr, uint16 length, uint16 offset, uint16 data_id);

/***********************************************************************************************************************
 *  e2e_p05_process_cnt()
 **********************************************************************************************************************/
/*! \brief          处理循环计数器值
 *  \details        none
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      recv_cnt                 本次收到的循环计数值
 *  \return         void
 *  \context        ANY|TASK|ISR1|ISR2|KERNEL
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
e2e_p05_process_cnt(const e2e_p05_config_type *config_ptr, e2e_p05_check_state_type *state_ptr, uint8 recv_cnt);

/***********************************************************************************************************************
 *  e2e_p05_check_new_data_available()
 **********************************************************************************************************************/
/*! \brief          没有收到新数据的处理分支
 *  \details        none
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      data_ptr                 本次收到的数据指针
 *  \param[in]      length                   本次收到的数据总长度
 *  \return         void
 *  \context        ANY|TASK|ISR1|ISR2|KERNEL
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p05_check_new_data_available(const e2e_p05_config_type *config_ptr,
	e2e_p05_check_state_type *state_ptr, const uint8 *data_ptr, uint16 length);

/***********************************************************************************************************************
 *  e2e_p05_check_protect_input()
 **********************************************************************************************************************/
/*! \brief          检查e2e_p05_protect函数的入参
 *  \details        none
 *  \param[in]      config_ptr               指向配置信息的指针
 *  \param[inout]   state_ptr                指向端口和数据通信状态信息的指针
 *  \param[in]      data_ptr                 指向实际数据的指针
 *  \param[in]      length                  实际数据的长度
 *  \return         校验结果
 *  \context        ANY|TASK|ISR1|ISR2|KERNEL
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p05_check_protect_input(const e2e_p05_config_type *config_ptr, const e2e_p05_protect_state_type *state_ptr,
				const uint8 *data_ptr, uint16 length);

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

/* local function implementation */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_p05_check_check_input_pointers()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p05_check_check_input_pointers(const e2e_p05_config_type *config_ptr, const e2e_p05_check_state_type *state_ptr,
				const uint8 *data_ptr, uint16 length)
{
	if (config_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	if (state_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	if ((data_ptr == NULL) && (length != 0u)) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if ((data_ptr != NULL) && (length == 0u)) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p05_check_check_offset_and_length()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p05_check_check_offset_and_length(boolean *is_new_data_available, const e2e_p05_config_type *config_ptr,
					  const uint8 *data_ptr, uint16 length)
{
	uint16 bit_len = 8u * length;

	if (data_ptr == NULL) {
		return E2E_E_OK;
	}
	if ((bit_len != config_ptr->data_length) || (bit_len < MIN_P05_DATA_LENGTH_IN_BITS) ||
		(bit_len > MAX_P05_DATA_LENGTH_IN_BITS)) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (((config_ptr->offset % 8u) != 0u) || (config_ptr->offset > (bit_len - MIN_P05_DATA_LENGTH_IN_BITS))) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	*is_new_data_available = TRUE;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p05_calculate_crc()
 **********************************************************************************************************************/
E2E_STATIC FUNC(uint16, E2E_CODE)
e2e_p05_calculate_crc(const uint8 *data_ptr, uint16 length, uint16 offset, uint16 data_id)
{
	uint16 start_byte_of_cnt = offset + 2u;
	uint16 length_bytes = length - start_byte_of_cnt;
	uint16 crc = 0xFFFFu;
	uint8 data_id_nibble[2];

	data_id_nibble[0] = (data_id) & 0xffu;
	data_id_nibble[1] = (data_id >> 8u) & 0xffu;
	crc = e2e_crc_calculate_crc16(&data_ptr[0], offset, crc, TRUE);
	crc = e2e_crc_calculate_crc16(&data_ptr[start_byte_of_cnt], length_bytes, crc, FALSE);
	crc = e2e_crc_calculate_crc16(&data_id_nibble[0], 2u, crc, FALSE);
	return crc;
}

/***********************************************************************************************************************
 *  e2e_p05_process_cnt()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_p05_process_cnt(const e2e_p05_config_type *config_ptr, e2e_p05_check_state_type *state_ptr, uint8 recv_cnt)
{
	uint8 delta_counter = recv_cnt - state_ptr->counter;

	if (delta_counter == 1u) {
		state_ptr->e2e_status = E2E_P05_STATUS_OK;
	} else if (delta_counter == 0u) {
		state_ptr->e2e_status = E2E_P05_STATUS_REPEATED;
	} else if (delta_counter <= config_ptr->max_delta_counter) {
		state_ptr->e2e_status = E2E_P05_STATUS_OKSOMELOST;
	} else {
		state_ptr->e2e_status = E2E_P05_STATUS_WRONGSEQUENCE;
	}
	state_ptr->counter = recv_cnt;
	return;
}

/***********************************************************************************************************************
 *  e2e_p05_check_new_data_available()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p05_check_new_data_available(const e2e_p05_config_type *config_ptr,
	e2e_p05_check_state_type *state_ptr, const uint8 *data_ptr, uint16 length)
{
	uint16 recv_crc = 0u;
	uint16 calc_crc = 0u;
	uint8 recv_cnt = 0u;
	uint16 offset = config_ptr->offset / 8u;

	recv_crc |= (((uint16)data_ptr[offset]) & 0x00ffu);
	recv_crc |= (((uint16)data_ptr[offset + 1u] << 8u) & 0xff00u);
	calc_crc = e2e_p05_calculate_crc(data_ptr, length, offset, config_ptr->data_id);
	if (recv_crc != calc_crc) {
		state_ptr->e2e_status = E2E_P05_STATUS_ERROR;
	} else {
		recv_cnt = data_ptr[offset + 2u] & 0xffu;
		e2e_p05_process_cnt(config_ptr, state_ptr, recv_cnt);
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_p05_check_protect_input()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p05_check_protect_input(const e2e_p05_config_type *config_ptr, const e2e_p05_protect_state_type *state_ptr,
				const uint8 *data_ptr, uint16 length)
{
	uint16 bit_len = 8u * length;

	if ((config_ptr == NULL) || (state_ptr == NULL) || (data_ptr == NULL)) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	if ((bit_len < MIN_P05_DATA_LENGTH_IN_BITS) || (bit_len > MAX_P05_DATA_LENGTH_IN_BITS) ||
	    (bit_len != config_ptr->data_length)) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	if (((config_ptr->offset % 8u) != 0u) || (config_ptr->offset > (bit_len - MIN_P05_DATA_LENGTH_IN_BITS))) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_WRONG);
		return E2E_E_INPUTERR_WRONG;
	}
	return E2E_E_OK;
}

/* global function implementation */
/***********************************************************************************************************************
 *  e2e_p05_check()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p05_check(const e2e_p05_config_type *config_ptr, e2e_p05_check_state_type *state_ptr, const uint8 *data_ptr,
	uint16 length)
{
	boolean is_new_data_available = FALSE;
	Std_ReturnType ret;

	ret = e2e_p05_check_check_input_pointers(config_ptr, state_ptr, data_ptr, length);
	if (ret != E2E_E_OK) {
		return ret;
	}
	ret = e2e_p05_check_check_offset_and_length(&is_new_data_available, config_ptr, data_ptr, length);
	if (ret != E2E_E_OK) {
		return ret;
	}
	if (is_new_data_available == TRUE) {
		e2e_p05_check_new_data_available(config_ptr, state_ptr, data_ptr, length);
	} else {
		state_ptr->e2e_status = E2E_P05_STATUS_NONEWDATA;
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p05_check_init()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p05_check_init(e2e_p05_check_state_type *state_ptr)
{
	if (state_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->e2e_status = E2E_P05_STATUS_ERROR;
	state_ptr->counter = 0xffu;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p05_protect()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p05_protect(const e2e_p05_config_type *config_ptr, e2e_p05_protect_state_type *state_ptr, uint8 *data_ptr,
	       uint16 length)
{
	uint16 offset;
	uint16 crc;
	Std_ReturnType return_value = e2e_p05_check_protect_input(config_ptr, state_ptr, data_ptr, length);

	if (return_value != E2E_E_OK) {
		return return_value;
	}
	offset = config_ptr->offset / 8u;
	data_ptr[offset + 2u] = state_ptr->counter & 0xffu;
	crc = e2e_p05_calculate_crc(data_ptr, length, offset, config_ptr->data_id);
	data_ptr[offset] = ((crc & 0x00ffu)) & 0xffu;
	data_ptr[offset + 1u] = ((crc & 0xff00u) >> 8u) & 0xffu;
	state_ptr->counter = (state_ptr->counter + 1u) % (MAX_P05_COUNTER_VALUE + 1u);
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p05_protect_init()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p05_protect_init(e2e_p05_protect_state_type *state_ptr)
{
	if (state_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P05_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->counter = 0u;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p05_map_status_to_sm()
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE) e2e_p05_map_status_to_sm(Std_ReturnType check_return, uint8 check_status)
{
	uint8 ret_value = E2E_P_ERROR;

	if (check_return != E2E_E_OK) {
		return ret_value;
	}
	ret_value = check_status & E2E_P_MAP_MASK;
	return ret_value;
}

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"