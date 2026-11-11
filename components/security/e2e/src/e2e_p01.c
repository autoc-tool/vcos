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
#include "e2e_p01.h"
#include "e2e.h"
#include "e2e_internal.h"
#include "e2e_types.h"
/* local macro definition */
#define E2E_P01_MAX_COUNTER_VALUE 14u

/* local fuction declaration */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/**********************************************************************************************************************
 *  e2e_p01_protect_verify_inputs()
 *********************************************************************************************************************/
/*!
 * \brief      e2e_p01_protect的输入数据检查函数
 * \details    检查输入指针参数是否有效，以及计数器值是否在范围内
 * \param[in]  config_ptr 指向配置数据结构的指针
 * \param[in]  state_ptr  指向状态数据结构的指针
 * \param[in]  data_ptr   指向要保护的数据的指针
 * \return     标准返回值，表示输入是否有效
 * \pre        -
 * \context    TASK
 * \reentrant  FALSE
 * \synchronous TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_protect_verify_inputs(const e2e_p01_config_type *config_ptr,
				const e2e_p01_protect_state_type *state_ptr, const uint8 *data_ptr);

/**********************************************************************************************************************
 *  e2e_p01_protect_counter()
 *********************************************************************************************************************/
/*!
 * \brief      e2e_p01_protect的计数计算函数
 * \details    检查输入指针参数是否有效，以及计数器值是否在范围内
 * \param[in]  config_ptr 指向配置数据结构的指针
 * \param[in]  state_ptr  指向状态数据结构的指针
 * \param[in]  data_ptr   指向要保护的数据的指针
 * \return     void
 * \pre        -
 * \context    TASK
 * \reentrant  FALSE
 * \synchronous TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_protect_counter(const e2e_p01_config_type *config_ptr,
	const e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr);

/**********************************************************************************************************************
 *  e2e_p01_protect_nibble_data_id()
 *********************************************************************************************************************/
/*!
 * \brief      e2e_p01_protect的计数计算函数
 * \details    检查输入指针参数是否有效，以及计数器值是否在范围内
 * \param[in]  config_ptr 指向配置数据结构的指针
 * \param[in]  state_ptr  指向状态数据结构的指针
 * \param[in]  data_ptr   指向要保护的数据的指针
 * \return     void
 * \pre        -
 * \context    TASK
 * \reentrant  FALSE
 * \synchronous TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_protect_nibble_data_id(const e2e_p01_config_type *config_ptr,
		const e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr);

/**********************************************************************************************************************
 *  e2e_p01_protect_crc()
 *********************************************************************************************************************/
/*!
 * \brief      e2e_p01_protect的计数计算函数
 * \details    检查输入指针参数是否有效，以及计数器值是否在范围内
 * \param[in]  config_ptr 指向配置数据结构的指针
 * \param[in]  state_ptr  指向状态数据结构的指针
 * \param[in]  data_ptr   指向要保护的数据的指针
 * \return     void
 * \pre        -
 * \context    TASK
 * \reentrant  FALSE
 * \synchronous TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_protect_crc(const e2e_p01_config_type *config_ptr,
	const e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr);

/**********************************************************************************************************************
 *  e2e_p01_check_verify_inputs()
 *********************************************************************************************************************/
/*!
 * \brief      e2e_p01_check的输入数据检查函数
 * \details    检查输入指针参数是否有效，并检查计数器值是否在范围内
 * \param[in]  config_ptr 指向配置数据结构的指针
 * \param[in]  state_ptr  指向状态数据结构的指针
 * \param[in]  data_ptr   指向要检查的数据的指针
 * \return     标准返回值，表示输入是否有效
 * \pre        -
 * \context    TASK
 * \reentrant  FALSE
 * \synchronous TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p01_check_verify_inputs(const e2e_p01_config_type *config_ptr, const e2e_p01_check_state_type *state_ptr,
				    const uint8 *data_ptr);

/**********************************************************************************************************************
 *  e2e_p01_compute_crc()
 *********************************************************************************************************************/
/*!
 * \brief      计算给定数据的crc，包括data_id
 * \details    根据data_id_mode计算给定数据的crc，包括data_id
 * \param[in]  data_ptr   指向要保护的数据的指针
 * \param[in]  config_ptr 指向配置数据结构的指针
 * \param[in]  counter   序列计数器的值
 * \return     uint8     计算得到的crc
 * \pre        -
 * \context    TASK
 * \reentrant  FALSE
 * \synchronous TRUE
 */
E2E_STATIC FUNC(uint16, E2E_CODE)
	e2e_p01_compute_crc(const uint8 *data_ptr, const e2e_p01_config_type *config_ptr, uint8 counter);

/**********************************************************************************************************************
 *  e2e_p01_check_delta_counter()
 *********************************************************************************************************************/
/*!
 * \brief      对增量计数器进行进一步检查
 * \details    对增量计数器进行进一步检查
 * \param[in]  config_ptr       指向配置数据结构的指针
 * \param[in]  state_ptr        指向状态数据结构的指针
 * \param[in]  received_counter 接收到的计数器值
 * \pre        -
 * \context    TASK
 * \reentrant  FALSE
 * \synchronous TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_check_delta_counter(const e2e_p01_config_type *config_ptr,
						       e2e_p01_check_state_type *state_ptr, uint8 received_counter);

/**********************************************************************************************************************
 *  e2e_p01_check_delta_counter_acceptable()
 *********************************************************************************************************************/
/*!
 * \brief          根据计数差值更新状态结构体 - 计数差值可接受分支
 * \param[in]      config_ptr  指向配置数据结构的指针
 * \param[in,out]  state_ptr   指向状态数据结构的指针
 * \param[in]      delta_counter  当前本地和接收计数之间的差值
 * \param[in]      received_counter  接收计数之间的差值
 * \pre            -
 * \context        TASK
 * \reentrant      FALSE
 * \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p01_check_delta_counter_acceptable(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr,
		uint8 delta_counter, uint8 received_counter);

/**********************************************************************************************************************
 *  e2e_p01_check_delta_counter_unacceptable()
 *********************************************************************************************************************/
/*!
 * \brief          根据计数差值更新状态结构体 - 计数差值不可接受分支
 * \param[in]      config_ptr  指向配置数据结构的指针
 * \param[in,out]  state_ptr   指向状态数据结构的指针
 * \param[in]      received_counter  接收计数之间的差值
 * \pre            -
 * \context        TASK
 * \reentrant      FALSE
 * \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p01_check_delta_counter_unacceptable(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr,
		uint8 received_counter);

/**********************************************************************************************************************
 *  e2e_p01_check_calc_recv_data_id()
 *********************************************************************************************************************/
/*!
 * \brief          定义e2e保护校验函数计算recv_data_id
 * \details        定义e2e保护校验函数计算recv_data_id
 * \param[in]      config_ptr  指向配置数据结构的指针
 * \param[in,out]  data_ptr   指向数据的指针
 * \pre            -
 * \context        TASK
 * \reentrant      FALSE
 * \synchronous    TRUE
 */
E2E_STATIC FUNC(uint8, E2E_CODE)
	e2e_p01_check_calc_recv_data_id(const e2e_p01_config_type *config_ptr, const uint8 *data_ptr);

/**********************************************************************************************************************
 *  e2e_p01_check_calc_recv_cnt()
 *********************************************************************************************************************/
/*!
 * \brief          定义e2e保护校验函数计算recv_cnt
 * \details        定义e2e保护校验函数计算recv_cnt
 * \param[in]      config_ptr  指向配置数据结构的指针
 * \param[in,out]  data_ptr   指向数据的指针
 * \pre            -
 * \context        TASK
 * \reentrant      FALSE
 * \synchronous    TRUE
 */

E2E_STATIC FUNC(uint8, E2E_CODE)
	e2e_p01_check_calc_recv_cnt(const e2e_p01_config_type *config_ptr, const uint8 *data_ptr);

/**********************************************************************************************************************
 *  e2e_p01_check_check_crc_and_first_data()
 *********************************************************************************************************************/
/*!
 * \brief          检查收到的crc和本地计算的crc值是否正确，
					检查当前包是否为第一次收到的包
 * \param[in]      config_ptr  指向配置数据结构的指针
 * \param[in,out]  state_ptr   指向状态数据结构的指针
 * \param[in]      data_ptr   接收报文内容指针
 * \param[in,out]  received_counter   接收的计数值
 * \pre            -
 * \context        TASK
 * \reentrant      FALSE
 * \synchronous    TRUE
 */
E2E_STATIC FUNC(boolean, E2E_CODE)
	e2e_p01_check_check_crc_and_first_data(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr,
		const uint8 *data_ptr, uint8 received_counter);

/**********************************************************************************************************************
 *  e2e_p01_compute_crc_data_id()
 *********************************************************************************************************************/
/*!
 * \brief          根据配置决定参与CRC计算的DataId
 * \details
 * \param[in,out]  data_id_ptr   DataId的出参
 * \param[in,out]  crc_len_ptr   CrcLen的出参
 * \param[in]      config_ptr    配置数据
 * \param[in]      counter       当前RollingCounter
 * \context        TASK
 * \reentrant      FALSE
 * \synchronous    TRUE
 */

E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p01_compute_crc_data_id(uint8 *data_id_ptr, uint8 *crc_len_ptr, const e2e_p01_config_type *config_ptr,
		uint8 counter);

/**********************************************************************************************************************
 *  e2e_p01_compute_crc_calc()
 *********************************************************************************************************************/
/*!
 * \brief          调用接口计算CRC
 * \details
 * \param[in]      data_ptr      实际Pdu数据
 * \param[in,out]  data_id_ptr   DataId的出参
 * \param[in,out]  crc_len_ptr   CrcLen的出参
 * \param[in]      config_ptr    配置数据
 * \context        TASK
 * \reentrant      FALSE
 * \synchronous    TRUE
 */

E2E_STATIC FUNC(uint16, E2E_CODE)
	e2e_p01_compute_crc_calc(const uint8 *data_ptr, uint8 *data_id_ptr, uint8 crc_len,
		const e2e_p01_config_type *config_ptr);

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

/**********************************************************************************************************************
 *  LOCAL FUNCTIONS
 **********************************************************************************************************************/

#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/**********************************************************************************************************************
 *  e2e_p01_protect_verify_inputs()
 *********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_protect_verify_inputs(const e2e_p01_config_type *config_ptr,
				const e2e_p01_protect_state_type *state_ptr, const uint8 *data_ptr)
{
	Std_ReturnType ret = E2E_E_OK;

	if (config_ptr == NULL) {
		ret = E2E_E_INPUTERR_NULL;
	} else if (state_ptr == NULL) {
		ret = E2E_E_INPUTERR_NULL;
	} else if (data_ptr == NULL) {
		ret = E2E_E_INPUTERR_NULL;
	} else {
		if (state_ptr->counter > E2E_P01_MAX_COUNTER_VALUE) {
			ret = E2E_E_INPUTERR_WRONG;
		}
		if (config_ptr->data_id_mode >= E2E_P01_DATAID_INVALID) {
			ret = E2E_E_INPUTERR_WRONG;
		}
	}
	return ret;
}

/**********************************************************************************************************************
 *  e2e_p01_protect_counter()
 *********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_protect_counter(const e2e_p01_config_type *config_ptr,
			const e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr)
{
	uint16 offset = config_ptr->counter_offset >> 3u;

	if ((config_ptr->counter_offset & 0x07u) == 0u) {
		data_ptr[offset] = (data_ptr[offset] & 0xf0u) | state_ptr->counter;
	} else {
		data_ptr[offset] = (data_ptr[offset] & 0x0fu) | ((state_ptr->counter << 4u) & 0xf0u);
	}
	return;
}

/**********************************************************************************************************************
 *  e2e_p01_protect_nibble_data_id()
 *********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_protect_nibble_data_id(const e2e_p01_config_type *config_ptr,
	const e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr)
{
	uint8 offset;

	if (config_ptr->data_id_mode != E2E_P01_DATAID_NIBBLE) {
		return;
	}
	offset = config_ptr->data_id_nibble_offset >> 3u;
	if ((config_ptr->data_id_nibble_offset & 0x07u) == 0u) {
		data_ptr[offset] = ((data_ptr[offset] & 0xf0u) | ((uint8)(config_ptr->data_id >> 8u) & 0x0fu));
	} else {
		data_ptr[offset] = ((data_ptr[offset] & 0x0fu) | ((uint8)(config_ptr->data_id >> 4u) & 0xf0u));
	}
	return;
}

/**********************************************************************************************************************
 *  e2e_p01_protect_crc()
 *********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_protect_crc(const e2e_p01_config_type *config_ptr,
	const e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr)
{
	uint8 offset = config_ptr->crc_offset >> 3u;

	data_ptr[offset] = (uint8) e2e_p01_compute_crc(data_ptr, config_ptr, state_ptr->counter);
	return;
}

/**********************************************************************************************************************
 *  e2e_p01_check_verify_inputs()
 *********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p01_check_verify_inputs(const e2e_p01_config_type *config_ptr, const e2e_p01_check_state_type *state_ptr,
				    const uint8 *data_ptr)
{
	Std_ReturnType ret;

	if (config_ptr == NULL) {
		ret = E2E_E_INPUTERR_NULL;
	} else if (state_ptr == NULL) {
		ret = E2E_E_INPUTERR_NULL;
	} else if (data_ptr == NULL) {
		ret = E2E_E_INPUTERR_NULL;
	} else if (state_ptr->last_valid_counter > E2E_P01_MAX_COUNTER_VALUE) {
		ret = E2E_E_INPUTERR_WRONG;
	} else if (state_ptr->max_delta_counter > E2E_P01_MAX_COUNTER_VALUE) {
		ret = E2E_E_INPUTERR_WRONG;
	} else if (state_ptr->sync_counter > config_ptr->sync_counter_init) {
		ret = E2E_E_INPUTERR_WRONG;
	} else if (state_ptr->no_update_counter > E2E_P01_MAX_COUNTER_VALUE) {
		ret = E2E_E_INPUTERR_WRONG;
	} else if (config_ptr->data_id_mode >= E2E_P01_DATAID_INVALID) {
		ret = E2E_E_INPUTERR_WRONG;
	} else {
		ret = E2E_E_OK;
	}
	return ret;
}

/**********************************************************************************************************************
 *  e2e_p01_compute_crc_data_id()
 *********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_p01_compute_crc_data_id(uint8 *data_id_ptr, uint8 *crc_len_ptr, const e2e_p01_config_type *config_ptr,
	uint8 counter)
{
	uint8 data_id_mode = config_ptr->data_id_mode;
	uint16 data_id = config_ptr->data_id;

	data_id_ptr[0] = (uint8)data_id;
	data_id_ptr[1] = 0u;
	if (data_id_mode == E2E_P01_DATAID_BOTH) {
		data_id_ptr[1] = (uint8)(data_id >> 8u);
		*crc_len_ptr = 2u;
	} else if (data_id_mode == E2E_P01_DATAID_LOW) {
		*crc_len_ptr = 1u;
	} else if (data_id_mode == E2E_P01_DATAID_NIBBLE) {
		*crc_len_ptr = 2u;
	} else if (data_id_mode == E2E_P01_DATAID_ALT) {
		*crc_len_ptr = 1u;
		if ((counter & 0x01u) != 0u) {
			data_id_ptr[0] = (uint8)(data_id >> 8u);
		}
	} else {
		*crc_len_ptr = 0u;
	}
	return;
}

/**********************************************************************************************************************
 *  e2e_p01_compute_crc_calc()
 *********************************************************************************************************************/
E2E_STATIC FUNC(uint16, E2E_CODE)
	e2e_p01_compute_crc_calc(const uint8 *data_ptr, uint8 *data_id_ptr, uint8 crc_len,
		const e2e_p01_config_type *config_ptr)
{
	uint16 crc_offset = config_ptr->crc_offset >> 3u;
	uint8 crc = e2e_crc8u8array(data_id_ptr, crc_len, 0xffu);
	uint16 data_len = config_ptr->data_length >> 3u;

	if (crc_offset > 0u) {
		crc = e2e_crc8u8array(data_ptr, crc_offset, crc);
	}
	if (crc_offset < (data_len - 1u)) {
		crc = e2e_crc8u8array(&data_ptr[crc_offset + 1u], ((uint32)data_len - (uint32)crc_offset - 1u), crc);
	}
	return (uint16)(crc ^ 0xffu);
}

/**********************************************************************************************************************
 *  e2e_p01_compute_crc()
 *********************************************************************************************************************/
E2E_STATIC FUNC(uint16, E2E_CODE)
	e2e_p01_compute_crc(const uint8 *data_ptr, const e2e_p01_config_type *config_ptr, uint8 counter)
{
	uint8 crc_length = 0u;
	uint8 local_data_id[2];
	uint16 crc;

	e2e_p01_compute_crc_data_id(local_data_id, &crc_length, config_ptr, counter);
	crc = e2e_p01_compute_crc_calc(data_ptr, local_data_id, crc_length, config_ptr);
	return crc;
}

/**********************************************************************************************************************
 *  e2e_p01_check_delta_counter()
 *********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p01_check_delta_counter(const e2e_p01_config_type *config_ptr,
	e2e_p01_check_state_type *state_ptr, uint8 received_counter)
{
	uint8 delta_counter = (uint8)(received_counter - state_ptr->last_valid_counter) & 0x0fu;

	if (delta_counter == 0u) {
		if (state_ptr->no_update_counter < E2E_P01_MAX_COUNTER_VALUE) {
			state_ptr->no_update_counter++;
		}
		state_ptr->e2e_status = E2E_P01_STATUS_REPEATED;
	} else if (delta_counter <= state_ptr->max_delta_counter) {
		e2e_p01_check_delta_counter_acceptable(config_ptr, state_ptr, delta_counter, received_counter);
	} else {
		e2e_p01_check_delta_counter_unacceptable(config_ptr, state_ptr, received_counter);
	}
	return;
}

/**********************************************************************************************************************
 *  e2e_p01_check_delta_counter_acceptable()
 *********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p01_check_delta_counter_acceptable(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr,
		uint8 delta_counter, uint8 received_counter)
{
	state_ptr->max_delta_counter = config_ptr->max_delta_counter_init;
	state_ptr->last_valid_counter = received_counter;
	state_ptr->lost_data = delta_counter - 1u;
	if (state_ptr->no_update_counter > config_ptr->max_no_update_counter) {
		state_ptr->sync_counter = config_ptr->sync_counter_init;
		state_ptr->e2e_status = E2E_P01_STATUS_SYNC;
	} else if (state_ptr->sync_counter > 0u) {
		state_ptr->sync_counter--;
		state_ptr->e2e_status = E2E_P01_STATUS_SYNC;
	} else if (delta_counter == 1u) {
		state_ptr->e2e_status = E2E_P01_STATUS_OK;
	} else {
		state_ptr->e2e_status = E2E_P01_STATUS_OKSOMELOST;
	}
	state_ptr->no_update_counter = 0u;
	return;
}

/**********************************************************************************************************************
 *  e2e_p01_check_delta_counter_unacceptable()
 *********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p01_check_delta_counter_unacceptable(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr,
		uint8 received_counter)
{
	state_ptr->no_update_counter = 0u;
	state_ptr->sync_counter = config_ptr->sync_counter_init;
	if (config_ptr->sync_counter_init > 0u) {
		state_ptr->max_delta_counter = config_ptr->max_delta_counter_init;
		state_ptr->last_valid_counter = received_counter;
	}
	state_ptr->e2e_status = E2E_P01_STATUS_WRONGSEQUENCE;
	return;
}

/**********************************************************************************************************************
 *  e2e_p01_check_calc_recv_data_id()
 *********************************************************************************************************************/
E2E_STATIC FUNC(uint8, E2E_CODE)
	e2e_p01_check_calc_recv_data_id(const e2e_p01_config_type *config_ptr, const uint8 *data_ptr)
{
	uint8 received_data_id_nibble = 0;
	uint16 computed_offset = config_ptr->data_id_nibble_offset >> 3u;

	if ((config_ptr->data_id_nibble_offset & 0x7u) == 0u) {
		received_data_id_nibble = data_ptr[computed_offset] & 0x0fu;
	} else {
		received_data_id_nibble = (data_ptr[computed_offset] & 0xf0u) >> 4u;
	}
	return received_data_id_nibble;
}

/**********************************************************************************************************************
 *  e2e_p01_check_calc_recv_cnt()
 *********************************************************************************************************************/
E2E_STATIC FUNC(uint8, E2E_CODE)
	e2e_p01_check_calc_recv_cnt(const e2e_p01_config_type *config_ptr, const uint8 *data_ptr)
{
	uint8 received_counter;
	uint16 computed_offset = config_ptr->counter_offset >> 3u;

	if ((config_ptr->counter_offset & 0x07u) == 0u) {
		received_counter = data_ptr[computed_offset] & 0x0fu;
	} else {
		received_counter = (data_ptr[computed_offset] & 0xf0u) >> 4u;
	}
	return received_counter;
}

/**********************************************************************************************************************
 *  e2e_p01_check_check_crc_and_first_data()
 *********************************************************************************************************************/
E2E_STATIC FUNC(boolean, E2E_CODE)
	e2e_p01_check_check_crc_and_first_data(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr,
		const uint8 *data_ptr, uint8 received_counter)
{
	boolean ret = FALSE;
	uint16 computed_offset = config_ptr->crc_offset >> 3u;
	uint8 received_crc = data_ptr[computed_offset];
	uint16 computed_crc = e2e_p01_compute_crc(data_ptr, config_ptr, received_counter);

	if (received_crc != computed_crc) {
		state_ptr->e2e_status = E2E_P01_STATUS_WRONGCRC;
	} else if (state_ptr->is_wait_for_first_data == TRUE) {
		state_ptr->is_wait_for_first_data = FALSE;
		state_ptr->max_delta_counter = config_ptr->max_delta_counter_init;
		state_ptr->last_valid_counter = received_counter;
		state_ptr->e2e_status = E2E_P01_STATUS_INITIAL;
	} else {
		ret = TRUE;
	}
	return ret;
}

/**********************************************************************************************************************
 *  GLOBAL FUNCTIONS
 **********************************************************************************************************************/

/**********************************************************************************************************************
 *  e2e_p01_protect()
 *********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_protect(const e2e_p01_config_type *config_ptr, e2e_p01_protect_state_type *state_ptr, uint8 *data_ptr)
{
	Std_ReturnType ret = e2e_p01_protect_verify_inputs(config_ptr, state_ptr, data_ptr);

	if (ret != E2E_E_OK) {
		return ret;
	}
	e2e_p01_protect_counter(config_ptr, state_ptr, data_ptr);
	e2e_p01_protect_nibble_data_id(config_ptr, state_ptr, data_ptr);
	e2e_p01_protect_crc(config_ptr, state_ptr, data_ptr);
	state_ptr->counter = (state_ptr->counter + 1u) % (E2E_P01_MAX_COUNTER_VALUE + 1u);
	return ret;
}

/**********************************************************************************************************************
 *  e2e_p01_protect_init()
 *********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_protect_init(e2e_p01_protect_state_type *state_ptr)
{
	Std_ReturnType ret = E2E_E_INPUTERR_NULL;

	if (state_ptr != NULL) {
		state_ptr->counter = 0u;
		ret = E2E_E_OK;
	}
	return ret;
}

/**********************************************************************************************************************
 *  e2e_p01_check()  定义e2e保护校验函数，用于检查p01配置文件
 *********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_check(const e2e_p01_config_type *config_ptr, e2e_p01_check_state_type *state_ptr, const uint8 *data_ptr)
{
	Std_ReturnType ret = e2e_p01_check_verify_inputs(config_ptr, state_ptr, data_ptr);

	if (ret != E2E_E_OK) {
		return ret;
	}
	if (state_ptr->max_delta_counter < E2E_P01_MAX_COUNTER_VALUE) {
		state_ptr->max_delta_counter++;
	}
	if (state_ptr->is_new_data_available == FALSE) {
		if (state_ptr->no_update_counter < E2E_P01_MAX_COUNTER_VALUE) {
			state_ptr->no_update_counter++;
		}
		state_ptr->e2e_status = E2E_P01_STATUS_NONEWDATA;
		return ret;
	}
	if (config_ptr->data_id_mode == E2E_P01_DATAID_NIBBLE) {
		uint8 received_data_id_nibble = e2e_p01_check_calc_recv_data_id(config_ptr, data_ptr);
		uint8 data_id_nibble = (config_ptr->data_id >> 8u) & 0x0fu;

		if (received_data_id_nibble != data_id_nibble) {
			state_ptr->e2e_status = E2E_P01_STATUS_WRONGCRC;
			return ret;
		}
	}
	uint8 received_counter = e2e_p01_check_calc_recv_cnt(config_ptr, data_ptr);
	boolean check_delta_counter;

	check_delta_counter = e2e_p01_check_check_crc_and_first_data(config_ptr, state_ptr, data_ptr,received_counter);
	if (check_delta_counter == TRUE) {
		e2e_p01_check_delta_counter(config_ptr, state_ptr, received_counter);
	}
	return ret;
}

/**********************************************************************************************************************
 *  e2e_p01_check_init()
 *********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p01_check_init(e2e_p01_check_state_type *state_ptr)
{
	if (state_ptr == NULL) {
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->last_valid_counter = 0u;
	state_ptr->max_delta_counter = 0u;
	state_ptr->is_wait_for_first_data = TRUE;
	state_ptr->is_new_data_available = TRUE;
	state_ptr->lost_data = 0u;
	state_ptr->e2e_status = E2E_P01_STATUS_NONEWDATA;
	state_ptr->no_update_counter = 0u;
	state_ptr->sync_counter = 0u;
	return E2E_E_OK;
}

/**********************************************************************************************************************
 *  e2e_p01_map_status_to_sm()
 *********************************************************************************************************************/
FUNC(uint8, E2E_CODE)
e2e_p01_map_status_to_sm(Std_ReturnType check_return, uint8 check_status, boolean profile_behavior)
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