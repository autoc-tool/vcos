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

#include "memif.h"

#include "Std_Types.h"

#define MEMIF_START_SEC_CONST
#include "memif_memmap.h"

extern CONST(memif_abs_api_type, MEMIF_CONST) memif_hw_abstraction[MEMIF_NUMBER_OF_DEVICES];
#define MEMIF_STOP_SEC_CONST
#include "memif_memmap.h"

#define MEMIF_START_SEC_CODE
#include "memif_memmap.h"

FUNC(Std_ReturnType, MEMIF_CODE) memif_check_device_id(uint8 device_index);

FUNC(Std_ReturnType, MEMIF_CODE) memif_check_device_id(uint8 device_index)
{
	Std_ReturnType ret_val = E_OK;

	if (device_index >= MEMIF_NUMBER_OF_DEVICES) {
		ret_val = E_NOT_OK;
	}
	return ret_val;
}

FUNC(Std_ReturnType, MEMIF_CODE)
memif_read(uint8 device_index, uint16 block_number, uint16 block_offset, uint8 *data_buffer_ptr, uint16 length)
{
	Std_ReturnType ret_val = E_NOT_OK;

	if (memif_check_device_id(device_index) == E_OK) {
		ret_val = memif_hw_abstraction[device_index].read(block_number, block_offset, data_buffer_ptr, length);
	}

	return ret_val;
}

FUNC(Std_ReturnType, MEMIF_CODE) memif_write(uint8 device_index, uint16 block_number, uint8 *data_buffer_ptr)
{
	Std_ReturnType ret_val = E_NOT_OK;

	if (memif_check_device_id(device_index) == E_OK) {
		ret_val = memif_hw_abstraction[device_index].write(block_number, data_buffer_ptr);
	}

	return ret_val;
}

FUNC(Std_ReturnType, MEMIF_CODE) memif_invalidate_block(uint8 device_index, uint16 block_number)
{
	Std_ReturnType ret_val = E_NOT_OK;

	if (memif_check_device_id(device_index) == E_OK) {
		ret_val = memif_hw_abstraction[device_index].invalidate_block(block_number);
	}

	return ret_val;
}

FUNC(Std_ReturnType, MEMIF_CODE) memif_erase_immediate_block(uint8 device_index, uint16 block_number)
{
	Std_ReturnType ret_val = E_NOT_OK;

	if (memif_check_device_id(device_index) == E_OK) {
		ret_val = memif_hw_abstraction[device_index].erase_immediate_block(block_number);
	}

	return ret_val;
}

FUNC(void, MEMIF_CODE) memif_cancel(uint8 device_index)
{
	if (memif_check_device_id(device_index) == E_OK) {
		memif_hw_abstraction[device_index].cancel();
	}
}

FUNC(MemIf_StatusType, MEMIF_CODE) memif_get_status(uint8 dev_idx)
{
	/* 状态优先级映射表：UNINIT > BUSY > BUSY_INTERNAL > IDLE */
	static const MemIf_StatusType priority_map[] = {
		[MEMIF_IDLE] = 0u, [MEMIF_BUSY_INTERNAL] = 1u, [MEMIF_BUSY] = 2u, [MEMIF_UNINIT] = 3u
	};

	if ((dev_idx != MEMIF_BROADCAST_ID) && (dev_idx >= MEMIF_NUMBER_OF_DEVICES)) {
		return MEMIF_IDLE;
	}

	if (dev_idx != MEMIF_BROADCAST_ID) {
		return memif_hw_abstraction[dev_idx].get_status();
	}

	MemIf_StatusType aggregate = MEMIF_IDLE;
	for (uint8 i = 0u; i < MEMIF_NUMBER_OF_DEVICES; ++i) {
		const MemIf_StatusType current = memif_hw_abstraction[i].get_status();
		aggregate = priority_map[current] > priority_map[aggregate] ? current : aggregate;
	}

	return aggregate;
}

FUNC(MemIf_JobResultType, MEMIF_CODE) memif_get_job_result(uint8 device_index)
{
	MemIf_JobResultType ret_val = MEMIF_JOB_FAILED;

	if (memif_check_device_id(device_index) == E_OK) {
		ret_val = memif_hw_abstraction[device_index].get_job_result();
	}

	return ret_val;
}

FUNC(void, MEMIF_CODE) memif_set_mode(MemIf_ModeType mode)
{
	uint8 index;
	// switch all underlying modules into the requested mode
	for (index = 0; index < MEMIF_NUMBER_OF_DEVICES; index++) {
		memif_hw_abstraction[index].set_mode(mode);
	}
}

#define MEMIF_STOP_SEC_CODE
#include "memif_memmap.h"
