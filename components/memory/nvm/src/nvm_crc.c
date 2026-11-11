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

#include "crc.h"
#include "nvm_crc.h"
#include "nvm_internal.h"

/* NA - Not Aligned */
#define WRITE32_NA(address, value)                                                                                     \
	do {                                                                                                           \
		(address)[0] = (uint8)(((value)&0xffu));                                                               \
		(address)[1] = (uint8)(((value) >> 8u) & 0xffu);                                                       \
		(address)[2] = (uint8)(((value) >> 16u) & 0xffu);                                                      \
		(address)[3] = (uint8)(((value) >> 24u) & 0xffu);                                                      \
	} while (0)
#define WRITE16_NA(address, value)                                                                                     \
	do {                                                                                                           \
		(address)[0] = (uint8)(((value)&0xffu));                                                               \
		(address)[1] = (uint8)(((value) >> 8u) & 0xffu);                                                       \
	} while (0)
/* Not aligned reads */
#define READ32_NA(address)                                                                                             \
	(uint32)((((uint32)((address)[0]))) + (((uint32)((address)[1])) << 8u) + (((uint32)((address)[2])) << 16u) +   \
		 (((uint32)((address)[3])) << 24u))

#define READ16_NA(address) (uint16)((((uint16)((address)[0]))) + (((uint16)((address)[1])) << 8u))

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
FUNC(uint8, NVM_CODE) nvm_get_block_crc_len(const nvm_block_descriptor_type *block_ptr)
{
	static const uint8 crc_length_lookup[] = { [NVM_CRC_OFF] = 0u, [NVM_CRC16_ON] = 2u, [NVM_CRC32_ON] = 4u };

	const uint8 type = block_ptr->block_crc_type;
	return (type <= NVM_CRC32_ON) ? crc_length_lookup[type] : 0u;
}

FUNC(void, NVM_CODE)
nvm_write_crc_to_buffer(uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, const nvm_admin_block_type *adm_ptr)
{
	if (block_ptr->block_crc_type == NVM_CRC16_ON) {
		WRITE16_NA((&buf_ptr[block_ptr->nv_block_length]), adm_ptr->ram_crc.crc16);
	} else {
		WRITE32_NA((&buf_ptr[block_ptr->nv_block_length]), adm_ptr->ram_crc.crc32);
	}
}

FUNC(void, NVM_CODE)
nvm_read_crc_from_buffer(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr,
			 nvm_admin_block_type *adm_ptr)
{
	if (block_ptr->block_crc_type == NVM_CRC16_ON) {
		adm_ptr->ram_crc.crc16 = READ16_NA((&buf_ptr[block_ptr->nv_block_length]));
	} else {
		adm_ptr->ram_crc.crc32 = READ32_NA((&buf_ptr[block_ptr->nv_block_length]));
	}
}

FUNC(boolean, NVM_CODE)
nvm_verify_read_crc(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr,
		    const nvm_admin_block_type *adm_ptr)
{
	uint16 crc16 = 0u;

	uint32 crc32 = 0u;
	boolean checksum_ok = FALSE;

	/* Calculate CRC on the data we just read to RAM. Compare with CRC that is located in NV block */
	if (block_ptr->block_crc_type == NVM_CRC16_ON) {
		crc16 = crc_calculate_16(buf_ptr, block_ptr->nv_block_length, 0xffffU, TRUE);
		checksum_ok = (crc16 == adm_ptr->ram_crc.crc16);
	} else {
		crc32 = crc_calculate_32(buf_ptr, block_ptr->nv_block_length, 0xffffffffU, TRUE);
		checksum_ok = (crc32 == adm_ptr->ram_crc.crc32);
	}
	return checksum_ok;
}

FUNC(void, NVM_CODE)
nvm_update_write_crc(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, nvm_admin_block_type *adm_ptr)
{
	uint16 crc16;

	uint32 crc32;

	if (block_ptr->block_crc_type == NVM_CRC16_ON) {
		crc16 = crc_calculate_16(buf_ptr, block_ptr->nv_block_length, 0xffffU, TRUE);
		/* Just save the checksum */
		adm_ptr->ram_crc.crc16 = crc16;
	} else {
		crc32 = crc_calculate_32(buf_ptr, block_ptr->nv_block_length, 0xffffffffU, TRUE);
		adm_ptr->ram_crc.crc32 = crc32;
	}
}

FUNC(boolean, NVM_CODE)
nvm_check_crc(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, const nvm_admin_block_type *adm_ptr)
{
	uint16 crc16;

	uint32 crc32;

	boolean result = FALSE;

	if (adm_ptr->crc_compare_valid != TRUE) {
		return result;
	}
	if (block_ptr->block_crc_type == NVM_CRC16_ON) {
		crc16 = crc_calculate_16(buf_ptr, block_ptr->nv_block_length, 0xffffU, TRUE);
		result = (adm_ptr->crc_compare_val.crc16 == crc16) ? TRUE : FALSE;
	} else {
		crc32 = crc_calculate_32(buf_ptr, block_ptr->nv_block_length, 0xffffffffU, TRUE);
		result = (adm_ptr->crc_compare_val.crc32 == crc32) ? TRUE : FALSE;
	}
	return result;
}

#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
