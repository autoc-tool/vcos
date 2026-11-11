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

#include "nvm_fsm.h"
#include "nvm_crc.h"
#include "nvm_internal.h"
#include "nvm_queue.h"
#include "memif.h"
#include "nvm.h"
#include "vcos_log.h"
#include "nvm_vcos_log.h"

#define NVM_START_SEC_CONST
#include "nvm_memmap.h"

CONST(nvm_request_result_type, NVM_CONST)
Nvm_ResultMap[MEMIF_BLOCK_INVALID + 1u] = { [MEMIF_JOB_OK] = NVM_REQ_OK,
					    [MEMIF_JOB_FAILED] = NVM_REQ_NOT_OK,
					    [MEMIF_JOB_PENDING] = NVM_REQ_NOT_OK,
					    [MEMIF_JOB_CANCELED] = NVM_REQ_NOT_OK,
					    [MEMIF_BLOCK_INCONSISTENT] = NVM_REQ_INTEGRITY_FAILED,
					    [MEMIF_BLOCK_INVALID] = NVM_REQ_NV_INVALIDATED };
#define NVM_STOP_SEC_CONST
#include "nvm_memmap.h"

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"

static FUNC(void, NVM_CODE) nvm_get_result_and_update(uint8 dev_id)
{
	nvm_current_block_info.last_result = Nvm_ResultMap[(uint32)memif_get_job_result(dev_id)];
}

static FUNC(boolean, NVM_CODE) nvm_event_crc_match(void)
{
	boolean checksum_ok = TRUE;

	if (nvm_current_block_info.Descriptor_pt->block_crc_type != NVM_CRC_OFF) {
		nvm_read_crc_from_buffer(nvm_work_buffer, nvm_current_block_info.Descriptor_pt,
					 nvm_current_block_info.mngmt_ptr);
		checksum_ok = nvm_verify_read_crc(nvm_work_buffer, nvm_current_block_info.Descriptor_pt,
						  nvm_current_block_info.mngmt_ptr);
	}

	if (checksum_ok == FALSE) {
		(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_ERROR, NVM_DLT_CRC_FAILED,
			       nvm_current_block_info.Descriptor_pt->nvram_block_identifier);
	}
	return checksum_ok;
}

static FUNC(boolean, NVM_CODE) nvm_event_crc_compare(void)
{
	boolean checksum_ok = FALSE;
	const nvm_block_descriptor_type *block_ptr = nvm_current_block_info.Descriptor_pt;

	if ((block_ptr->block_crc_type != NVM_CRC_OFF) && ((block_ptr->flags & NVM_CRC_COMP_MECHANISM_ON) != 0u)) {
		checksum_ok = nvm_check_crc(nvm_work_buffer, block_ptr, nvm_current_block_info.mngmt_ptr);
	}
	return checksum_ok;
}

static FUNC(boolean, NVM_CODE) nvm_event_true(void)
{
	return TRUE;
}

static FUNC(boolean, NVM_CODE) nvm_event_nv_busy(void)
{
	boolean ret_val = FALSE;
	uint8 dev_id = nvm_current_block_info.Descriptor_pt->nvram_device_id;

	if (MEMIF_BUSY == memif_get_status(dev_id)) {
		return TRUE;
	}

	if (NVM_REQ_PENDING == nvm_current_block_info.last_result) {
		nvm_get_result_and_update(dev_id);
	}
	return ret_val;
}

static FUNC(boolean, NVM_CODE) nvm_event_cancel_write_all(void)
{
	return ((nvm_api_flags & NVM_SET_CANCEL_WRITE_ALL_FLAG) != 0u);
}

static FUNC(boolean, NVM_CODE) nvm_event_sub_task_running(void)
{
	return (boolean)(NVM_FSM_FSM_FINISHED != nvm_subtask_state);
}

static FUNC(boolean, NVM_CODE) nvm_event_write_all_end(void)
{
	if ((nvm_api_flags & NVM_SET_CANCEL_WRITE_ALL_FLAG) != 0u) {
		return TRUE;
	}

	if (nvm_current_job.job_block_id == 0u) {
		nvm_subtask_state = NVM_FSM_FSM_FINISHED;
		return TRUE;
	}

	return FALSE;
}

static FUNC(boolean, NVM_CODE) nvm_event_read_all_done(void)
{
	if (nvm_current_job.job_block_id >= (NVM_NUM_OF_NVRAM_BLOCKS - 1u)) {
		nvm_subtask_state = NVM_FSM_FSM_FINISHED;
		return TRUE;
	}

	return FALSE;
}

static FUNC(boolean, NVM_CODE) nvm_event_have_write_all_job(void)
{
	return (boolean)((nvm_api_flags & NVM_SET_WRITE_ALL_FLAG) != 0u);
}

static FUNC(boolean, NVM_CODE) nvm_event_last_result_ok(void)
{
	return (boolean)(NVM_REQ_OK == nvm_current_block_info.last_result);
}

static FUNC(boolean, NVM_CODE) nvm_event_have_job(void)
{
	boolean ret = TRUE;

	if (nvm_event_high_prio_job() == TRUE) {
		return TRUE;
	}

	if (nvm_event_normal_prio_job() == FALSE) /* No Single Block Job */
	{
		if ((nvm_api_flags & (NVM_SET_WRITE_ALL_FLAG | NVM_SET_READ_ALL_FLAG)) ==
		    0u) /* And No Multi Block Job */
		{
			ret = FALSE;
		}
	}
	return ret;
}

static FUNC(boolean, NVM_CODE) nvm_event_redundant_block(void)
{
	return (boolean)(
		(nvm_current_block_info.Descriptor_pt->block_management_type == NVM_BLOCK_REDUNDANT) &&
		(nvm_current_block_info.nv_block_id == nvm_current_block_info.Descriptor_pt->nv_block_base_number));
}

static FUNC(boolean, NVM_CODE) nvm_event_read_retry(void)
{
	return (boolean)(nvm_current_block_info.retry_times <
			 nvm_current_block_info.Descriptor_pt->max_num_of_read_retries);
}

static FUNC(boolean, NVM_CODE) nvm_event_write_retry(void)
{
	boolean ret_val = (boolean)(
		(nvm_current_block_info.last_result != NVM_REQ_OK) &&
		(nvm_current_block_info.retry_times < nvm_current_block_info.Descriptor_pt->max_num_of_write_retries));
	return ret_val;
}

#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"

#define NVM_START_SEC_CONST
#include "nvm_memmap.h"
CONST(nvm_event_ptr_t, NVM_CONST)
nvm_event_table[NVM_EVENT_TRUE + 1] = { &nvm_event_have_job,	     &nvm_event_high_prio_job,
					&nvm_event_normal_prio_job,  &nvm_event_have_write_all_job,
					&nvm_event_nv_busy,	     &nvm_event_crc_match,
					&nvm_event_crc_compare,	     &nvm_event_last_result_ok,
					&nvm_event_cancel_write_all, &nvm_event_sub_task_running,
					&nvm_event_redundant_block,  &nvm_event_read_retry,
					&nvm_event_write_retry,	     &nvm_event_write_all_end,
					&nvm_event_read_all_done,    &nvm_event_true };

#define NVM_STOP_SEC_CONST
#include "nvm_memmap.h"
