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
#include "e2e_p04.h"
#include "e2e_internal.h"
#include "e2e.h"
#include "e2e_types.h"

/* local macro definition */
#define MAX_P04_COUNTER_VALUE 0xffffu
#define MAX_P04_DATA_LENGTH_IN_BITS 0x8000u
#define MIN_P04_DATA_LENGTH_IN_BITS 0x60u
#define E2E_CRC_CALC_START_VALUE 0xffffffffu
/* local enumeration definition */

/* local type & structure definition */

/* global variable definition */

/* local variable definition */

/* local function declaration */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs_is_null()
 **********************************************************************************************************************/
/*! \brief          检查输入的三个指针是否为空指针
 *  \details        无
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      state_ptr                状态结构体指针
 *  \param[in]      data_ptr                 数据数组指针
 *  \return         Std_ReturnType          检查结果
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs_is_null(const e2e_p04_config_type *config_ptr,
					  const e2e_p04_protect_state_type *state_ptr, const uint8 *data_ptr);

/***********************************************************************************************************************
 *  e2e_p04_verify_check_inputs_is_null()
 **********************************************************************************************************************/
/*! \brief          检查输入的两个指针是否为空指针
 *  \details        无
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      state_ptr                状态结构体指针
 *  \return         Std_ReturnType          检查结果
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_check_inputs_is_null(const e2e_p04_config_type *config_ptr,
		const e2e_p04_check_state_type *state_ptr);

/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs_len_is_valid()
 **********************************************************************************************************************/
/*! \brief          检查输入的三个指针是否为空指针
 *  \details        无
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      length                  数据数组长度
 *  \return         Std_ReturnType          检查结果
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs_len_is_valid(const e2e_p04_config_type *config_ptr, uint16 length);

/***********************************************************************************************************************
 *  e2e_p04_verify_check_inputs_len_is_valid()
 **********************************************************************************************************************/
/*! \brief          检查输入的三个指针是否为空指针
 *  \details        无
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      length                  数据数组长度
 *  \return         Std_ReturnType          检查结果
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p04_verify_check_inputs_len_is_valid(const e2e_p04_config_type *config_ptr, const uint8 *data_ptr, uint16 length);

/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs_offset_is_valid()
 **********************************************************************************************************************/
/*! \brief          检查输入的中的偏移量是否为空指针
 *  \details        无
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      length                  数据数组长度
 *  \return         Std_ReturnType          检查结果
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs_offset_is_valid(const e2e_p04_config_type *config_ptr, uint16 length);

/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs()
 **********************************************************************************************************************/
/*! \brief          检查输入函数
 *  \details        无
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      state_ptr                状态结构体指针
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      length                  数据数组长度
 *  \return         Std_ReturnType          检查结果
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs(const e2e_p04_config_type *config_ptr, const e2e_p04_protect_state_type *state_ptr,
				const uint8 *data_ptr, uint16 length);

/***********************************************************************************************************************
 *  e2e_p04_write_length()
 **********************************************************************************************************************/
/*! \brief          将长度信息写入数据数组
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \param[in]      length                  数据数组长度
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p04_write_length(uint8 *data_ptr, uint16 offset, uint16 length);

/***********************************************************************************************************************
 *  e2e_p04_write_counter()
 **********************************************************************************************************************/
/*! \brief          将计数器信息写入数据数组
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \param[in]      state_ptr                状态结构体指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p04_write_counter(uint8 *data_ptr, uint16 offset, const e2e_p04_protect_state_type *state_ptr);

/***********************************************************************************************************************
 *  e2e_p04_write_data_id()
 **********************************************************************************************************************/
/*! \brief          将data_id数据写入数据数组
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \param[in]      config_ptr               数据数组长度
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p04_write_data_id(uint8 *data_ptr, uint16 offset,
	const e2e_p04_config_type *config_ptr);

/***********************************************************************************************************************
 *  e2e_p04_compute_crc()
 **********************************************************************************************************************/
/*! \brief          根据数据计算crc
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \param[in]      length                  数据数组长度
 *  \return         uint32                  计算得到的四字节crc值
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(uint32, E2E_CODE) e2e_p04_compute_crc(const uint8 *data_ptr, uint16 offset, uint16 length);

/***********************************************************************************************************************
 *  e2e_p04_compute_and_write_crc()
 **********************************************************************************************************************/
/*! \brief          根据数据计算crc并写入数据数组
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \param[in]      length                  数据数组长度
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p04_compute_and_write_crc(uint8 *data_ptr, uint16 offset, uint16 length);

/***********************************************************************************************************************
 *  e2e_p04_increment_counter()
 **********************************************************************************************************************/
/*! \brief          更新计数器
 *  \details        无
 *  \param[in]      state_ptr                状态结构体指针
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE) e2e_p04_increment_counter(e2e_p04_protect_state_type *state_ptr);

/***********************************************************************************************************************
 *  e2e_p04_read_length()
 **********************************************************************************************************************/
/*! \brief          读取接收数据中的长度信息
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \return         uint16                  计算得到的两字节length值
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(uint16, E2E_CODE) e2e_p04_read_length(const uint8 *data_ptr, uint16 offset);

/***********************************************************************************************************************
 *  e2e_p04_read_counter()
 **********************************************************************************************************************/
/*! \brief          读取接收数据中的计数器信息
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \return         uint16                  计算得到的两字节counter值
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(uint16, E2E_CODE) e2e_p04_read_counter(const uint8 *data_ptr, uint16 offset);

/***********************************************************************************************************************
 *  e2e_p04_read_data_id()
 **********************************************************************************************************************/
/*! \brief          读取接收数据中的data_id信息
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \return         uint32                  计算得到的四字节data_id值
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(uint32, E2E_CODE) e2e_p04_read_data_id(const uint8 *data_ptr, uint16 offset);

/***********************************************************************************************************************
 *  e2e_p04_read_crc()
 **********************************************************************************************************************/
/*! \brief          读取接收数据中的crc信息
 *  \details        无
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      offset                  数据偏移量
 *  \return         uint32                  计算得到的四字节crc值
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(uint32, E2E_CODE) e2e_p04_read_crc(const uint8 *data_ptr, uint16 offset);

/***********************************************************************************************************************
 *  e2e_p04_calculate_delta_counter()
 **********************************************************************************************************************/
/*! \brief          计算delta_counter
 *  \details        无
 *  \param[in]      state_ptr                状态结构体指针
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      received_counter         本次接收到的计数值
 *  \return         void
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p04_calculate_delta_counter(e2e_p04_check_state_type *state_ptr, const e2e_p04_config_type *config_ptr,
				       uint16 received_counter);

/***********************************************************************************************************************
 *  e2e_p04_check_read_rcv_data()
 **********************************************************************************************************************/
/*! \brief          根据接收信息计算并校验counter、data_id、CRC，并返回delta_counter和crc校验结果
 *  \details        无
 *  \param[in]      config_ptr               配置结构体指针
 *  \param[in]      state_ptr                状态结构体指针
 *  \param[in]      data_ptr                 数据数组指针
 *  \param[in]      length                  数据数组长度
 *  \param[in]      received_counter         收到计数器的容器指针
 *  \return         boolean                 校验crc的结果
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(boolean, E2E_CODE)
	e2e_p04_check_read_rcv_data(const e2e_p04_config_type *config_ptr, e2e_p04_check_state_type *state_ptr,
				   const uint8 *data_ptr, uint16 length, uint16 *received_counter);

/***********************************************************************************************************************
 *  e2e_p04_verify_check_inputs()
 **********************************************************************************************************************/
/*! \brief          p04check方法的入参校验
 *  \details        none
 *  \param[in]      is_new_data_available            本轮调用中是否收到新数据
 *  \param[in]      config_ptr                   配置结构体指针
 *  \param[in]      state_ptr                    状态结构体指针
 *  \param[in]      data_ptr                     数据数组指针
 *  \param[in]      length                      数据数组长度
 *  \return         Std_ReturnType              标准返回值
 *  \context        ANY
 *  \reentrant      TRUE
 *  \synchronous    TRUE
 */
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_check_inputs(boolean *is_new_data_available, const e2e_p04_config_type *config_ptr,
				   const e2e_p04_check_state_type *state_ptr, const uint8 *data_ptr, uint16 length);
#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"

/* local function implementation */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs_is_null()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs_is_null(const e2e_p04_config_type *config_ptr,
					  const e2e_p04_protect_state_type *state_ptr, const uint8 *data_ptr)
{
	Std_ReturnType ret_val = E2E_E_OK;

	if ((config_ptr == NULL) || (state_ptr == NULL) || (data_ptr == NULL)) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_NULL);
		ret_val = E2E_E_INPUTERR_NULL;
	}
	return ret_val;
}

/***********************************************************************************************************************
 *  e2e_p04_verify_check_inputs_is_null()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_check_inputs_is_null(const e2e_p04_config_type *config_ptr,
		const e2e_p04_check_state_type *state_ptr)
{
	Std_ReturnType ret_val = E2E_E_OK;

	if ((config_ptr == NULL) || (state_ptr == NULL)) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_NULL);
		ret_val = E2E_E_INPUTERR_NULL;
	}
	return ret_val;
}

/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs_len_is_valid()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs_len_is_valid(const e2e_p04_config_type *config_ptr, uint16 length)
{
	Std_ReturnType ret_val = E2E_E_OK;

	/* check input parameters values */
	if ((config_ptr->min_data_length < MIN_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->max_data_length < MIN_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->min_data_length > MAX_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->max_data_length > MAX_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->min_data_length > config_ptr->max_data_length)) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_WRONG);
		ret_val = E2E_E_INPUTERR_WRONG;
	} else if (((length * 8u) < config_ptr->min_data_length) || ((length * 8u) > config_ptr->max_data_length)) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_WRONG);
		ret_val = E2E_E_INPUTERR_WRONG;
	} else {
		/* empty */
	}
	return ret_val;
}

/***********************************************************************************************************************
 *  e2e_p04_verify_check_inputs_len_is_valid()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
e2e_p04_verify_check_inputs_len_is_valid(const e2e_p04_config_type *config_ptr, const uint8 *data_ptr, uint16 length)
{
	Std_ReturnType ret_val = E2E_E_OK;

	if ((config_ptr->min_data_length < MIN_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->max_data_length < MIN_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->min_data_length > MAX_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->max_data_length > MAX_P04_DATA_LENGTH_IN_BITS) ||
	    (config_ptr->min_data_length > config_ptr->max_data_length)) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_WRONG);
		ret_val = E2E_E_INPUTERR_WRONG;
	} else if (((data_ptr == NULL) && (length != 0u)) || ((data_ptr != NULL) && (length == 0u))) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_WRONG);
		ret_val = E2E_E_INPUTERR_WRONG;
	} else {
		/* empty */
	}
	return ret_val;
}

/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs_offset_is_valid()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs_offset_is_valid(const e2e_p04_config_type *config_ptr, uint16 length)
{
	Std_ReturnType ret_val = E2E_E_OK;

	if (((config_ptr->offset % 8u) != 0u) ||
	    (config_ptr->offset > (MAX_P04_DATA_LENGTH_IN_BITS - MIN_P04_DATA_LENGTH_IN_BITS)) ||
	    (config_ptr->offset > ((length * 8u) - MIN_P04_DATA_LENGTH_IN_BITS))) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_WRONG);
		ret_val = E2E_E_INPUTERR_WRONG;
	}
	return ret_val;
}

/***********************************************************************************************************************
 *  e2e_p04_verify_check_inputs_offset_is_valid()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_check_inputs_offset_is_valid(const e2e_p04_config_type *config_ptr, uint16 length)
{
	Std_ReturnType ret_val = E2E_E_OK;

	if (((config_ptr->offset % 8u) != 0u) ||
	    (config_ptr->offset > ((MAX_P04_DATA_LENGTH_IN_BITS) - (MIN_P04_DATA_LENGTH_IN_BITS))) ||
	    (config_ptr->offset > ((length * 8u) - (MIN_P04_DATA_LENGTH_IN_BITS)))) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_WRONG);
		ret_val = E2E_E_INPUTERR_WRONG;
	} else if (((length * 8u) < config_ptr->min_data_length) || ((length * 8u) > config_ptr->max_data_length)) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_WRONG);
		ret_val = E2E_E_INPUTERR_WRONG;
	} else {
		/* empty */
	}
	return ret_val;
}

/***********************************************************************************************************************
 *  e2e_p04_verify_protect_inputs()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_protect_inputs(const e2e_p04_config_type *config_ptr, const e2e_p04_protect_state_type *state_ptr,
				     const uint8 *data_ptr, uint16 length)
{
	Std_ReturnType check_result = E2E_E_OK;

	check_result = e2e_p04_verify_protect_inputs_is_null(config_ptr, state_ptr, data_ptr);
	if (check_result != E2E_E_OK) {
		return check_result;
	}
	check_result = e2e_p04_verify_protect_inputs_len_is_valid(config_ptr, length);
	if (check_result != E2E_E_OK) {
		return check_result;
	}
	check_result = e2e_p04_verify_protect_inputs_offset_is_valid(config_ptr, length);
	if (check_result != E2E_E_OK) {
		return check_result;
	}
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p04_verify_check_inputs()
 **********************************************************************************************************************/
E2E_STATIC FUNC(Std_ReturnType, E2E_CODE)
	e2e_p04_verify_check_inputs(boolean *is_new_data_available, const e2e_p04_config_type *config_ptr,
				   const e2e_p04_check_state_type *state_ptr, const uint8 *data_ptr, uint16 length)
{
	Std_ReturnType check_result = E2E_E_OK;
	*is_new_data_available = FALSE;
	check_result = e2e_p04_verify_check_inputs_is_null(config_ptr, state_ptr);
	if (check_result != E2E_E_OK) {
		return check_result;
	}
	check_result = e2e_p04_verify_check_inputs_len_is_valid(config_ptr, data_ptr, length);
	if (check_result != E2E_E_OK) {
		return check_result;
	}
	if (data_ptr == NULL) {
		return check_result;
	}
	check_result = e2e_p04_verify_check_inputs_offset_is_valid(config_ptr, length);
	if (check_result != E2E_E_OK) {
		return check_result;
	}
	*is_new_data_available = TRUE;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p04_write_length()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p04_write_length(uint8 *data_ptr, uint16 offset, uint16 length)
{
	data_ptr[offset] = e2e_change_bit_endianness((uint8)(((length & 0xff00u) >> 8u) & 0xffu));
	data_ptr[offset + 1u] = e2e_change_bit_endianness((uint8)((length & 0x00ffu) & 0xffu));
	return;
}

/***********************************************************************************************************************
 *  e2e_p04_write_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p04_write_counter(uint8 *data_ptr, uint16 offset, const e2e_p04_protect_state_type *state_ptr)
{
	data_ptr[offset + 2u] = e2e_change_bit_endianness((uint8)(((state_ptr->counter & 0xff00u) >> 8u) & 0xffu));
	data_ptr[offset + 3u] = e2e_change_bit_endianness((uint8)((state_ptr->counter & 0x00ffu) & 0xffu));
}

/***********************************************************************************************************************
 *  e2e_p04_write_length()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
e2e_p04_write_data_id(uint8 *data_ptr, uint16 offset, const e2e_p04_config_type *config_ptr)
{
	data_ptr[offset + 4u] =
		e2e_change_bit_endianness((uint8)(((config_ptr->data_id & 0xff000000u) >> 24u) & 0xffu));
	data_ptr[offset + 5u] =
		e2e_change_bit_endianness((uint8)(((config_ptr->data_id & 0x00ff0000u) >> 16u) & 0xffu));
	data_ptr[offset + 6u] =
		e2e_change_bit_endianness((uint8)(((config_ptr->data_id & 0x0000ff00u) >> 8u) & 0xffu));
	data_ptr[offset + 7u] = e2e_change_bit_endianness((uint8)(((config_ptr->data_id & 0x000000ffu)) & 0xffu));
	return;
}

/***********************************************************************************************************************
 *  e2e_p04_compute_crc()
 **********************************************************************************************************************/
E2E_STATIC FUNC(uint32, E2E_CODE) e2e_p04_compute_crc(const uint8 *data_ptr, uint16 offset, uint16 length)
{
	uint32 offset_bytes = (uint32)offset + (uint32)8u;
	uint32 crc = e2e_crc_calculate_crc32p4(&data_ptr[0], offset_bytes, E2E_CRC_CALC_START_VALUE, TRUE);

	if ((offset + 12u) < length) {
		offset_bytes = (uint32)((length - offset) - 12u);
		crc = e2e_crc_calculate_crc32p4(&data_ptr[offset + 12u], offset_bytes, crc, FALSE);
	}
	return crc;
}

/***********************************************************************************************************************
 *  e2e_p04_compute_and_write_crc()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p04_compute_and_write_crc(uint8 *data_ptr, uint16 offset, uint16 length)
{
	uint32 crc = e2e_p04_compute_crc(data_ptr, offset, length);

	data_ptr[offset + 8u] = e2e_change_bit_endianness((uint8)(((crc & 0xff000000u) >> 24u) & 0xffu));
	data_ptr[offset + 9u] = e2e_change_bit_endianness((uint8)(((crc & 0x00ff0000u) >> 16u) & 0xffu));
	data_ptr[offset + 10u] = e2e_change_bit_endianness((uint8)(((crc & 0x0000ff00u) >> 8u) & 0xffu));
	data_ptr[offset + 11u] = e2e_change_bit_endianness((uint8)(((crc & 0x000000ffu)) & 0xffu));
	return;
}

/***********************************************************************************************************************
 *  e2e_p04_increment_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE) e2e_p04_increment_counter(e2e_p04_protect_state_type *state_ptr)
{
	if (state_ptr->counter == MAX_P04_COUNTER_VALUE) {
		state_ptr->counter = 0u;
	} else {
		state_ptr->counter += 1u;
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_p04_read_length()
 **********************************************************************************************************************/
E2E_STATIC FUNC(uint16, E2E_CODE) e2e_p04_read_length(const uint8 *data_ptr, uint16 offset)
{
	uint16 received_length = 0u;

	received_length |= (((uint16)e2e_change_bit_endianness(data_ptr[offset]) << 8u) & 0xff00u);
	received_length |= (((uint16)e2e_change_bit_endianness(data_ptr[offset + 1u])) & 0x00ffu);
	return received_length;
}

/***********************************************************************************************************************
 *  e2e_p04_read_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(uint16, E2E_CODE) e2e_p04_read_counter(const uint8 *data_ptr, uint16 offset)
{
	uint16 received_counter = 0u;

	received_counter |= (((uint16)e2e_change_bit_endianness(data_ptr[offset + 2u]) << 8u) & 0xff00u);
	received_counter |= (((uint16)e2e_change_bit_endianness(data_ptr[offset + 3u])) & 0x00ffu);
	return received_counter;
}

/***********************************************************************************************************************
 *  e2e_p04_read_data_id()
 **********************************************************************************************************************/
E2E_STATIC FUNC(uint32, E2E_CODE) e2e_p04_read_data_id(const uint8 *data_ptr, uint16 offset)
{
	uint32 received_data_id = 0u;

	received_data_id |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 4u]) << 24u) & 0xff000000u);
	received_data_id |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 5u]) << 16u) & 0x00ff0000u);
	received_data_id |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 6u]) << 8u) & 0x0000ff00u);
	received_data_id |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 7u])) & 0x000000ffu);
	return received_data_id;
}

/***********************************************************************************************************************
 *  e2e_p04_read_crc()
 **********************************************************************************************************************/
E2E_STATIC FUNC(uint32, E2E_CODE) e2e_p04_read_crc(const uint8 *data_ptr, uint16 offset)
{
	uint32 received_crc = 0u;

	received_crc |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 8u]) << 24u) & 0xff000000u);
	received_crc |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 9u]) << 16u) & 0x00ff0000u);
	received_crc |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 10u]) << 8u) & 0x0000ff00u);
	received_crc |= (((uint32)e2e_change_bit_endianness(data_ptr[offset + 11u])) & 0x000000ffu);
	return received_crc;
}

/***********************************************************************************************************************
 *  e2e_p04_calculate_delta_counter()
 **********************************************************************************************************************/
E2E_STATIC FUNC(void, E2E_CODE)
	e2e_p04_calculate_delta_counter(e2e_p04_check_state_type *state_ptr, const e2e_p04_config_type *config_ptr,
				       uint16 received_counter)
{
	uint16 delta_counter = received_counter - state_ptr->counter;

	if (delta_counter == 0u) {
		state_ptr->e2e_status = E2E_P04_STATUS_REPEATED;
	} else if (delta_counter == 1u) {
		state_ptr->e2e_status = E2E_P04_STATUS_OK;
	} else if (delta_counter <= config_ptr->max_delta_counter) {
		state_ptr->e2e_status = E2E_P04_STATUS_OKSOMELOST;
	} else {
		state_ptr->e2e_status = E2E_P04_STATUS_WRONGSEQUENCE;
	}
	return;
}

/***********************************************************************************************************************
 *  e2e_p04_check_read_rcv_data()
 **********************************************************************************************************************/
E2E_STATIC FUNC(boolean, E2E_CODE)
	e2e_p04_check_read_rcv_data(const e2e_p04_config_type *config_ptr, e2e_p04_check_state_type *state_ptr,
				   const uint8 *data_ptr, uint16 length, uint16 *received_counter)
{
	boolean is_error = FALSE;
	uint16 offset = config_ptr->offset / 8u;
	uint16 received_length = e2e_p04_read_length(data_ptr, offset);
	uint32 received_data_id = e2e_p04_read_data_id(data_ptr, offset);
	uint32 received_crc = e2e_p04_read_crc(data_ptr, offset);
	uint32 computed_crc = e2e_p04_compute_crc(data_ptr, offset, length);

	*received_counter = e2e_p04_read_counter(data_ptr, offset);
	if ((received_crc != computed_crc) || (received_data_id != config_ptr->data_id) ||
	    (received_length != length)) {
		state_ptr->e2e_status = E2E_P04_STATUS_ERROR;
		is_error = TRUE;
	}
	return is_error;
}

/* global function implementation */
/***********************************************************************************************************************
 *  e2e_p04_protect()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p04_protect(const e2e_p04_config_type *config_ptr, e2e_p04_protect_state_type *state_ptr, uint8 *data_ptr,
	uint16 length)
{
	uint16 offset;
	Std_ReturnType return_value = e2e_p04_verify_protect_inputs(config_ptr, state_ptr, data_ptr, length);

	if (E2E_E_OK != return_value) {
		return return_value;
	}
	offset = config_ptr->offset / 8u;
	e2e_p04_write_length(data_ptr, offset, length);
	e2e_p04_write_counter(data_ptr, offset, state_ptr);
	e2e_p04_write_data_id(data_ptr, offset, config_ptr);
	e2e_p04_compute_and_write_crc(data_ptr, offset, length);
	e2e_p04_increment_counter(state_ptr);
	return return_value;
}

/***********************************************************************************************************************
 *  e2e_p04_protect_init()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p04_protect_init(e2e_p04_protect_state_type *state_ptr)
{
	if (state_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->counter = 0u;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p04_check()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE)
e2e_p04_check(const e2e_p04_config_type *config_ptr, e2e_p04_check_state_type *state_ptr, const uint8 *data_ptr,
	     uint16 length)
{
	boolean is_new_data_available;
	uint16 received_counter;
	boolean is_error;
	Std_ReturnType return_value = e2e_p04_verify_check_inputs(&is_new_data_available, config_ptr, state_ptr, data_ptr,
		length);
	if (return_value != E2E_E_OK) {
		return return_value;
	}
	if (is_new_data_available == FALSE) {
		state_ptr->e2e_status = E2E_P04_STATUS_NONEWDATA;
		return E2E_E_OK;
	}
	is_error = e2e_p04_check_read_rcv_data(config_ptr, state_ptr, data_ptr, length, &received_counter);
	if (is_error) {
		return E2E_E_OK;
	}
	e2e_p04_calculate_delta_counter(state_ptr, config_ptr, received_counter);
	state_ptr->counter = received_counter;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p04_check_init()
 **********************************************************************************************************************/
FUNC(Std_ReturnType, E2E_CODE) e2e_p04_check_init(e2e_p04_check_state_type *state_ptr)
{
	if (state_ptr == NULL) {
		E2E_DET_REPORT(E2E_E2E_P04_CHECK_E2E_E_INPUTERR_NULL);
		return E2E_E_INPUTERR_NULL;
	}
	state_ptr->e2e_status = E2E_P04_STATUS_ERROR;
	state_ptr->counter = 0xffff;
	return E2E_E_OK;
}

/***********************************************************************************************************************
 *  e2e_p04_map_status_to_sm()
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE) e2e_p04_map_status_to_sm(Std_ReturnType check_return, uint8 check_status)
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