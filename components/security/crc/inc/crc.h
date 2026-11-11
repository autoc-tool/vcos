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

#ifndef CRC_H
#define CRC_H

#include "crc_cfg.h"
#include "crc_types.h"
#include "Platform_Types.h"
#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CRC_START_SEC_CODE
#include "crc_memmap.h"

#ifdef CRC_8_Mode
/*!********************************************************************************************************************
 * @brief          CRC8计算
 * @details        针对 8-bit SAE J1850标准的CRC8计算
 * param[in]       data_ptr         输入数据
 * param[in]       len              输入长度
 * param[in]       start_val        初始值
 * param[in]       is_first_call    是否首次
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Crc_00031
 *********************************************************************************************************************/
FUNC(uint8, CRC_CODE) crc_calculate_8(const uint8* data_ptr, uint32 len, uint8 start_val, boolean is_first_call);
#endif

#ifdef CRC_8_8H2FMode
/*!********************************************************************************************************************
 * @brief          CRC8H2F计算
 * @details        针对 8-bit 0x2F J1850标准的CRC8计算
 * param[in]       data_ptr         输入数据
 * param[in]       len              输入长度
 * param[in]       start_val        初始值
 * param[in]       is_first_call    是否首次
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Crc_00043
 *********************************************************************************************************************/
FUNC(uint8, CRC_CODE) crc_calculate_8_h2f(const uint8* data_ptr, uint32 len, uint8 start_val, boolean is_first_call);
#endif

#ifdef CRC_16_Mode
/*!*********************************************************************************************************************
 * @brief          CRC16计算
 * @details        针对 16-bit CCITT-FALSE J1850标准的CRC8计算
 * param[in]       data_ptr         输入数据
 * param[in]       len              输入长度
 * param[in]       start_val        初始值
 * param[in]       is_first_call    是否首次
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Crc_00019
 *********************************************************************************************************************/
FUNC(uint16, CRC_CODE) crc_calculate_16(const uint8 *data_ptr, uint32 len, uint16 start_val, boolean is_first_call);
#endif

#ifdef CRC_16ARC_Mode
/*!*********************************************************************************************************************
 * @brief          CRC16ARC计算
 * @details        针对 16-bit ARC J1850标准的CRC8计算
 * param[in]       data_ptr         输入数据
 * param[in]       len              输入长度
 * param[in]       start_val        初始值
 * param[in]       is_first_call    是否首次
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Crc_00071
 *********************************************************************************************************************/
FUNC(uint16, CRC_CODE) crc_calculate_16_arc(const uint8 *data_ptr, uint32 len,
			      uint16 start_val,
			      boolean is_first_call);
#endif

 #ifdef CRC_32_Mode
/*!********************************************************************************************************************
 * @brief          CRC32计算
 * @details        针对 32-bit Ethernet标准的CRC32计算
 * param[in]       data_ptr        输入数据
 * param[in]       len             输入长度
 * param[in]       start_val       初始值
 * param[in]       is_first_call   是否首次
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Crc_00020
 *********************************************************************************************************************/
FUNC(uint32, CRC_CODE) crc_calculate_32(const uint8 *data_ptr, uint32 len,
			   uint32 start_val, boolean is_first_call);
#endif

#ifdef CRC_32P4_Mode
/*!********************************************************************************************************************
 * @brief          CRC32P4计算
 * @details        针对 32-bit 0xF4ACFB13标准的CRC32计算
 * param[in]       data_ptr        输入数据
 * param[in]       len             输入长度
 * param[in]       start_val       初始值
 * param[in]       is_first_call   是否首次
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Crc_00058
 *********************************************************************************************************************/
FUNC(uint32, CRC_CODE) crc_calculate_32_p4(const uint8 *data_ptr, uint32 len,
			   uint32 start_val, boolean is_first_call);
#endif


#ifdef CRC_64_Mode
/*!********************************************************************************************************************
 * @brief          CRC32P4计算
 * @details        针对 64-bit 标准的CRC64计算
 * param[in]       data_ptr        输入数据
 * param[in]       len             输入长度
 * param[in]       start_val       初始值
 * param[in]       is_first_call   是否首次
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_Crc_00061
 *********************************************************************************************************************/
FUNC(uint64, CRC_CODE) crc_calculate_64(const uint8 *data_ptr, uint32 len,
				uint64 start_val, boolean is_first_call);

#endif
#define CRC_STOP_SEC_CODE
#include "crc_memmap.h"
#ifdef __cplusplus
}
#endif

#endif
