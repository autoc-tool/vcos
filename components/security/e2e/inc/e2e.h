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

#ifndef E2E_H
#define E2E_H

#include "Std_Types.h"
#include "Platform_Types.h"
#include <string.h>
#include "e2e_p01.h"
#include "e2e_p02.h"
#include "e2e_p04.h"
#include "e2e_p05.h"
#include "e2e_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define e2e_get_version_info(_vi) STD_GET_VERSION_INFO(_vi, E2E) ///< 获取版本信息接口

/*!*********************************************************************************************************************
 * @brief          为了与crc模块解耦封装crc函数8h2f
 * @details        为了与crc模块解耦封装crc函数8h2f
 * @param[in]      crc_data_ptr crc数据指针
 * @param[in]      crc_length  crc数据长度
 * @param[in]      crc_start_value8 crc起始值8
 * @param[in]      crc_is_first_call crc是否首次调用
 * @retval         0-255 计算crc值
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
uint8 e2e_crc_calculate_crc8h2f(const uint8 *crc_data_ptr, uint32 crc_length, uint8 crc_start_value8,
				boolean crc_is_first_call);

/*!*********************************************************************************************************************
 * @brief          为了与crc模块解耦封装crc函数32p4
 * @details        为了与crc模块解耦封装crc函数32p4
 * @param[in]      crc_data_ptr crc数据指针
 * @param[in]      crc_length  crc数据长度
 * @param[in]      crc_start_value32p4 crc起始值32
 * @param[in]      crc_is_first_call crc是否首次调用
 * @retval         0-2^32-1 计算crc值
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
uint32 e2e_crc_calculate_crc32p4(const uint8 *crc_data_ptr, uint32 crc_length, uint32 crc_start_value32p4,
				 boolean crc_is_first_call);

/*!*********************************************************************************************************************
 * @brief          为了与crc模块解耦封装crc函数16
 * @details        为了与crc模块解耦封装crc函数16
 * @param[in]      crc_data_ptr crc数据指针
 * @param[in]      crc_length  crc数据长度
 * @param[in]      crc_start_value16 crc起始值16
 * @param[in]      crc_is_first_call crc是否首次调用
 * @retval         0-2^16-1 计算crc值
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
uint16 e2e_crc_calculate_crc16(const uint8 *crc_data_ptr, uint32 crc_length, uint16 crc_start_value16,
			       boolean crc_is_first_call);

/*!*********************************************************************************************************************
 * @brief          计算uint8数组数据的8-bit-CRC
 * @details        计算uint8数组数据的8-bit-CRC
 * @param[in]      e2e_data_ptr        指向入参的指针
 * @param[in]      e2e_array_length 数组的长度
 * @param[in]      e2e_start_value  计算初始值
 * @retval         0-255 计算crc数组
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
uint8 e2e_crc8u8array(const uint8 *e2e_data_ptr, uint32 e2e_array_length, uint8 e2e_start_value);

/*!*********************************************************************************************************************
 * @brief          uint8（一字节）大小端变化
 * @details        uint8（一字节）大小端变化
 * @param[in]      value  输入数据
 * @retval         0-255 计算crc数组
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
uint8 e2e_change_bit_endianness(uint8 value);

#ifdef __cplusplus
}
#endif

#endif /* E2E_H */
