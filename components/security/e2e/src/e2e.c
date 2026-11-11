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
#include "e2e.h"

#if (CONFIG_SECURITY_CRC == 1)
#include "crc.h"
#endif

/* definition of functions */
#define E2E_START_SEC_CODE
#include "e2e_memmap.h"
/***********************************************************************************************************************
 *  e2e_crc_calculate_crc8h2f()
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE)
e2e_crc_calculate_crc8h2f(const uint8 *crc_data_ptr, uint32 crc_length, uint8 crc_start_value8,
			  boolean crc_is_first_call)
{
#if CONFIG_SECURITY_CRC == 1
	return (uint8)crc_calculate_8_h2f(crc_data_ptr, crc_length, crc_start_value8, crc_is_first_call);
#else
	return 0u;
#endif
}

/***********************************************************************************************************************
 *  e2e_crc_calculate_crc32p4()
 **********************************************************************************************************************/
FUNC(uint32, E2E_CODE)
e2e_crc_calculate_crc32p4(const uint8 *crc_data_ptr, uint32 crc_length, uint32 crc_start_value32p4,
			  boolean crc_is_first_call)
{
#if CONFIG_SECURITY_CRC == 1
	return (uint32)crc_calculate_32_p4(crc_data_ptr, crc_length, crc_start_value32p4, crc_is_first_call);
#else
	return 0u;
#endif
}

/***********************************************************************************************************************
 *  e2e_crc_calculate_crc16()
 **********************************************************************************************************************/
FUNC(uint16, E2E_CODE)
e2e_crc_calculate_crc16(const uint8 *crc_data_ptr, uint32 crc_length, uint16 crc_start_value16,
			boolean crc_is_first_call)
{
#if CONFIG_SECURITY_CRC == 1
	return (uint16)crc_calculate_16(crc_data_ptr, crc_length, crc_start_value16, crc_is_first_call);
#else
	return 0u;
#endif
}

/***********************************************************************************************************************
 *  e2e_crc8u8array()
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE) e2e_crc8u8array(const uint8 *e2e_data_ptr, uint32 e2e_array_length, uint8 e2e_start_value)
{
#if CONFIG_SECURITY_CRC == 1
	return crc_calculate_8(e2e_data_ptr, e2e_array_length, e2e_start_value ^ 0xFFu, FALSE) ^ 0xFFu;
#else
	return 0u;
#endif
}

/***********************************************************************************************************************
 *  e2e_change_bit_endianness()
 **********************************************************************************************************************/
FUNC(uint8, E2E_CODE) e2e_change_bit_endianness(uint8 value)
{
	uint8 result = 0;

	result |= (value & 0x01u) << 7;
	result |= (value & 0x02u) << 5;
	result |= (value & 0x04u) << 3;
	result |= (value & 0x08u) << 1;
	result |= (value & 0x10u) >> 1;
	result |= (value & 0x20u) >> 3;
	result |= (value & 0x40u) >> 5;
	result |= (value & 0x80u) >> 7;
	return result;
}

#define E2E_STOP_SEC_CODE
#include "e2e_memmap.h"