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

#include <string.h>

#include "memif.h"
#include "nvm_fsm.h"
#include "nvm.h"
#include "nvm_internal.h"
#include "nvm_fsmact.h"
#include "nvm_crc.h"
#include "nvm_cfg.h"
#include "nvm_queue.h"
#include "SchM_NvM.h"
#include "nvm_vcos_log.h"
#include "vcos_log.h"

#define NVM_START_SEC_PRIVATE_BSS
#include "nvm_memmap.h"
VAR(static nvm_queue_data_t, NVM_PRIVATE_BSS) nvm_queue_entry;
#define NVM_STOP_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

#define NVM_START_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

VAR(test_nvm_retry_t, NVM_PRIVATE_BSS) test_nvm_retry;
#if (NVM_ENABLE_DYNAMIC_CONFIGURATION == STD_ON)
VAR(static boolean, NVM_PRIVATE_BSS) nvm_configid_dismatch;
#endif

#define NVM_STOP_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
static FUNC(boolean, NVM_CODE) nvm_block_need_write(nvm_admin_block_type *adm, const nvm_block_descriptor_type *desc);
static FUNC(boolean, NVM_CODE) nvm_is_bitfield_true(nvm_bitfield_type boolean_bitfield);
static FUNC(void, NVM_CODE) nvm_act_check_config_id(uint16 config_id);
static FUNC(void, NVM_CODE) nvm_execute_restore(void);
static FUNC(void, NVM_CODE) nvm_update_compare_crc_data(nvm_request_result_type job_result);

static FUNC(void, NVM_CODE) nvm_update_admin_block_zero_status(nvm_request_result_type result, uint8 flag)
{
	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	nvm_admin_block_table[0].error_status = result;
	nvm_api_flags &= flag;
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();
}

static FUNC(boolean, NVM_CODE) nvm_block_need_write(nvm_admin_block_type *adm, const nvm_block_descriptor_type *desc)
{
	boolean ret = FALSE;
	boolean is_selected_for_write_all =
		(boolean)(((adm->attributes & (NVM_SET_WRITE_PROTECTION_STATUS | NVM_SET_LOCK_STATUS)) == 0u) &&
			  ((desc->flags & NVM_SELECT_BLOCK_FOR_WRITEALL_ON) != 0u));
	if (is_selected_for_write_all) {
#if (NVM_ENABLE_SET_RAM_BLOCK_STATUS_API == STD_ON)
		if ((desc->flags & NVM_BLOCK_USE_SET_RAM_BLOCK_STATUS_ON) != 0u) {
			ret = (boolean)((adm->attributes & (NVM_CHANGED_STATE_SET | NVM_VALID_STATE_SET)) ==
					(NVM_CHANGED_STATE_SET | NVM_VALID_STATE_SET));
		} else {
			ret = TRUE;
		}
#else
		ret = TRUE;
#endif
	}
	return ret;
}

static FUNC(boolean, NVM_CODE) nvm_is_bitfield_true(nvm_bitfield_type boolean_bitfield)
{
	return (boolean_bitfield != 0u);
}

static FUNC(void, NVM_CODE) nvm_execute_restore(void)
{
	const nvm_block_descriptor_type *desc = nvm_current_block_info.Descriptor_pt;

	if (nvm_current_block_info.ram_addr == NULL) {
		nvm_current_block_info.ram_addr = desc->ram_block_data_address;
	}

	if (desc->rom_block_data_adress != NULL) {
		/* Restore data from Rom */
		(void)memcpy(nvm_current_block_info.ram_addr, desc->rom_block_data_adress, desc->nv_block_length);
	} else {
		/* Restore data from Callback */
		(void)desc->init_block_callback();
	}

	nvm_current_block_info.mngmt_ptr->attributes |= (NVM_CHANGED_STATE_SET | NVM_VALID_STATE_SET);
}

static FUNC(void, NVM_CODE) nvm_act_check_config_id(uint16 config_id)
{
	(void)config_id;
#if (NVM_ENABLE_DYNAMIC_CONFIGURATION == STD_ON)
	if (config_id != nvm_compiled_config_id) {
		nvm_configid_dismatch = TRUE;
		memcpy(nvm_block_descriptor_list[1u].ram_block_data_address, &nvm_compiled_config_id, sizeof(uint16));
		nvm_admin_block_table[1u].attributes |= (NVM_CHANGED_STATE_SET | NVM_VALID_STATE_SET);
	}
#endif
}

static FUNC(void, NVM_CODE) nvm_act_read_nv_finished(void)
{
	nvm_current_block_info.mngmt_ptr->error_status = nvm_current_block_info.last_result;
}

static FUNC(void, NVM_CODE) nvm_act_read_nv_block(void)
{
	const nvm_block_descriptor_type *desc = nvm_current_block_info.Descriptor_pt;

	uint8 crc_len = 0u;
	uint16 length = 0u;
	Std_ReturnType result = E_NOT_OK;

	crc_len = nvm_get_block_crc_len(desc);
	length = desc->nv_block_length + crc_len;

	memset(nvm_work_buffer, 0, sizeof(nvm_work_buffer));

	result = memif_read(desc->nvram_device_id, nvm_current_block_info.nv_block_id, 0u, nvm_work_buffer, length);
	if (result == E_OK) {
		nvm_current_block_info.last_result = NVM_REQ_PENDING;
	} else {
		nvm_current_block_info.last_result = NVM_REQ_NOT_OK;
	}
}

static FUNC(void, NVM_CODE) nvm_act_finish_restore(void)
{
	const nvm_block_descriptor_type *desc = nvm_current_block_info.Descriptor_pt;

	if ((desc->flags & NVM_BLOCK_WRITE_BLOCK_ONCE_ON) != 0u) {
		nvm_current_block_info.mngmt_ptr->attributes &= NVM_CLEAR_WRITE_PROTECTION_STATUS;
	}

	if ((desc->rom_block_data_adress == NULL) && (desc->init_block_callback == NULL)) {
		nvm_current_block_info.internal_flags |= NVM_SET_ERROR_STATUS_FLAG;
		nvm_current_block_info.last_result = NVM_REQ_INTEGRITY_FAILED;
	} else {
		nvm_current_block_info.last_result = NVM_REQ_RESTORED_FROM_ROM;
		nvm_execute_restore();
	}

	nvm_current_block_info.mngmt_ptr->error_status = nvm_current_block_info.last_result;
	nvm_current_block_info.retry_times = 0;
	nvm_block_notification(nvm_current_job.job_block_id, nvm_service_table[nvm_current_job.job_service_id],
			       nvm_current_block_info.mngmt_ptr->error_status);
	nvm_update_compare_crc_data(NVM_REQ_NOT_OK);
}

static FUNC(void, NVM_CODE) nvm_act_restore_block_defaults(void)
{
	nvm_execute_restore();
	nvm_current_block_info.mngmt_ptr->error_status = NVM_REQ_OK;
	nvm_current_block_info.last_result = nvm_current_block_info.mngmt_ptr->error_status;
	nvm_update_compare_crc_data(NVM_REQ_NOT_OK);
}

static FUNC(void, NVM_CODE) nvm_act_finish_read_block(void)
{
	const nvm_block_descriptor_type *desc = nvm_current_block_info.Descriptor_pt;
	nvm_admin_block_type *mngmt_ptr = nvm_current_block_info.mngmt_ptr;
	uint16 config_id = 0;

	/* 1. Get Ram Block */
	if (nvm_current_block_info.ram_addr == NULL) {
		nvm_current_block_info.ram_addr = desc->ram_block_data_address;
		mngmt_ptr->attributes |= NVM_VALID_STATE_SET;
		mngmt_ptr->attributes &= NVM_CHANGED_STATE_CL;
	}
	if ((nvm_current_block_info.ram_addr == NULL) && (desc->cbk_set_mirror_func_pt != NULL)) {
		(void)desc->cbk_set_mirror_func_pt(nvm_work_buffer);
	} else {
		(void)memcpy(nvm_current_block_info.ram_addr, nvm_work_buffer, desc->nv_block_length);
	}

	/* 2. Check WRITE_ONCE Block */
	if ((nvm_current_block_info.Descriptor_pt->flags & NVM_BLOCK_WRITE_BLOCK_ONCE_ON) != 0u) {
		mngmt_ptr->attributes |= NVM_SET_WRITE_PROTECTION_STATUS;
	}

	/* 3. Check NvMConfig Block */
	if ((nvm_current_job.job_block_id == 1u) && ((nvm_api_flags & NVM_SET_READ_ALL_FLAG) != 0u)) {
		(void)memcpy((void *)&config_id, (const void *)nvm_work_buffer, sizeof(uint16));
		nvm_act_check_config_id(config_id);
	}

	nvm_current_block_info.mngmt_ptr->error_status = NVM_REQ_OK;
	nvm_current_block_info.retry_times = 0;
	nvm_block_notification(nvm_current_job.job_block_id, nvm_service_table[nvm_current_job.job_service_id],
			       nvm_current_block_info.mngmt_ptr->error_status);
	nvm_update_compare_crc_data(nvm_current_block_info.last_result);
}

static FUNC(void, NVM_CODE) nvm_act_setup_redundant(void)
{
	nvm_current_block_info.nv_block_id |= 0x01u;
	nvm_current_block_info.retry_times = 0;
}

static FUNC(void, NVM_CODE) nvm_act_write_nv_block(void)
{
	Std_ReturnType result = E_NOT_OK;

	result = memif_write(nvm_current_block_info.Descriptor_pt->nvram_device_id, nvm_current_block_info.nv_block_id,
			     nvm_work_buffer);
	if (result == E_OK) {
		nvm_current_block_info.last_result = NVM_REQ_PENDING;
	} else {
		nvm_current_block_info.last_result = NVM_REQ_NOT_OK;
	}
}

static FUNC(void, NVM_CODE) nvm_act_erase_nv_block(void)
{
	Std_ReturnType result = E_NOT_OK;

	result = memif_erase_immediate_block(nvm_current_block_info.Descriptor_pt->nvram_device_id,
					     nvm_current_block_info.Descriptor_pt->nv_block_base_number);
	if (result == E_OK) {
		nvm_current_block_info.last_result = NVM_REQ_PENDING;
	} else {
		nvm_current_block_info.last_result = NVM_REQ_NOT_OK;
	}
	nvm_update_compare_crc_data(NVM_REQ_NOT_OK);
}

static FUNC(void, NVM_CODE) nvm_act_init_write_block(void)
{
	const nvm_block_descriptor_type *desc = nvm_current_block_info.Descriptor_pt;

	nvm_current_block_info.last_result = NVM_REQ_OK;

	memset(nvm_work_buffer, 0, sizeof(nvm_work_buffer));

	/* 1.Get Ram data */
	if (nvm_current_block_info.ram_addr == NULL) {
		nvm_current_block_info.ram_addr = desc->ram_block_data_address;
	}

	if ((nvm_current_block_info.ram_addr == NULL) && (desc->cbk_get_mirror_func_pt != NULL)) {
		(void)desc->cbk_get_mirror_func_pt(nvm_work_buffer);
	} else {
		(void)memcpy(nvm_work_buffer, nvm_current_block_info.ram_addr, desc->nv_block_length);
	}

	/* 2.Add the CRC to write buffer */
	if (desc->block_crc_type != NVM_CRC_OFF) {
		nvm_update_write_crc(nvm_work_buffer, desc, nvm_current_block_info.mngmt_ptr);
		nvm_write_crc_to_buffer(nvm_work_buffer, desc, nvm_current_block_info.mngmt_ptr);
	}
}

static FUNC(void, NVM_CODE) nvm_act_write_finished(void)
{
	nvm_current_block_info.mngmt_ptr->error_status = nvm_current_block_info.last_result;
	nvm_admin_block_type *mngmt_ptr = nvm_current_block_info.mngmt_ptr;

	if (nvm_current_block_info.last_result == NVM_REQ_OK) {
		mngmt_ptr->attributes &= NVM_CHANGED_STATE_CL;
		if ((nvm_current_block_info.Descriptor_pt->flags & NVM_BLOCK_WRITE_BLOCK_ONCE_ON) != 0u) {
			mngmt_ptr->attributes |= NVM_SET_WRITE_PROTECTION_STATUS;
		}
	} else {
		mngmt_ptr->attributes &= (NVM_CHANGED_STATE_CL & NVM_VALID_STATE_CL);
		nvm_current_block_info.internal_flags |= NVM_SET_ERROR_STATUS_FLAG;
		(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_ERROR, NVM_DLT_WRITEBLOCK_FAILED, nvm_current_job.job_block_id,
			       nvm_current_block_info.last_result);
	}
	nvm_current_block_info.retry_times = 0;
	nvm_block_notification(nvm_current_job.job_block_id, nvm_service_table[nvm_current_job.job_service_id],
			       nvm_current_block_info.mngmt_ptr->error_status);
	nvm_update_compare_crc_data(nvm_current_block_info.mngmt_ptr->error_status);
}

static FUNC(void, NVM_CODE) nvm_act_init_invalidate_block(void)
{
	Std_ReturnType result = E_NOT_OK;

	result = memif_invalidate_block(nvm_current_block_info.Descriptor_pt->nvram_device_id,
					nvm_current_block_info.nv_block_id);
	if (result == E_OK) {
		nvm_current_block_info.last_result = NVM_REQ_PENDING;
	} else {
		nvm_current_block_info.last_result = NVM_REQ_NOT_OK;
	}
	nvm_update_compare_crc_data(NVM_REQ_NOT_OK);
}

static FUNC(void, NVM_CODE) nvm_act_start_write_all(void)
{
	uint16 index = 0u;
	/* WriteAll: begin with last block and end with configuration block */
	nvm_current_job.job_block_id = NVM_NUM_OF_NVRAM_BLOCKS;
	nvm_current_job.job_service_id = NVM_INT_FID_WRITE_ALL;
	nvm_current_block_info.internal_flags &= NVM_CLEAR_ERROR_STATUS_FLAG;
	for (index = 1u; index < NVM_NUM_OF_NVRAM_BLOCKS; index++) {
		if ((nvm_block_descriptor_list[index].ram_block_data_address == NULL) &&
		    (nvm_block_descriptor_list[index].cbk_get_mirror_func_pt == NULL)) {
			nvm_admin_block_table[index].error_status = NVM_REQ_BLOCK_SKIPPED;
			continue;
		}
		if (nvm_block_need_write(&nvm_admin_block_table[index], &nvm_block_descriptor_list[index]) == TRUE) {
			nvm_admin_block_table[index].error_status = NVM_REQ_PENDING;
			nvm_block_notification(nvm_block_descriptor_list[index].nvram_block_identifier, NVM_WRITE_ALL,
					       NVM_REQ_PENDING);
		} else {
			nvm_admin_block_table[index].error_status = NVM_REQ_BLOCK_SKIPPED;
		}
	}
	nvm_subtask_state = NVM_FSM_FSM_FINISHED;
}

static FUNC(void, NVM_CODE) nvm_act_write_next_block(void)
{
	nvm_current_job.job_block_id--;
	nvm_block_id_t current_id = nvm_current_job.job_block_id;

	while ((current_id > 0u) && (nvm_admin_block_table[current_id].error_status == NVM_REQ_BLOCK_SKIPPED)) {
		current_id--;
	}

	if (current_id == 0u) {
		return;
	}

	nvm_current_block_info.Descriptor_pt = &(nvm_block_descriptor_list[current_id]);
	nvm_current_block_info.nv_block_id = nvm_current_block_info.Descriptor_pt->nv_block_base_number;
	nvm_current_block_info.ram_addr = nvm_current_block_info.Descriptor_pt->ram_block_data_address;
	nvm_current_block_info.mngmt_ptr = &(nvm_admin_block_table[current_id]);
	nvm_current_job.job_block_id = current_id;

	nvm_subtask_state = NVM_FSM_WRITE_INIT;
}

static FUNC(void, NVM_CODE) nvm_act_finish_write_all(void)
{
	nvm_request_result_type job_result;
	/* lack error handle */
	if ((nvm_api_flags & NVM_SET_CANCEL_WRITE_ALL_FLAG) != 0u) {
		uint16 index = 0;

		job_result = NVM_REQ_CANCELED;

		/* block[blockIndex] has been Writen successfully */
		for (index = nvm_current_job.job_block_id; index > 0u; index--) {
			if (nvm_admin_block_table[index].error_status == NVM_REQ_PENDING) {
				nvm_admin_block_table[index].error_status = NVM_REQ_CANCELED;
			}
		}
	} else {
		const bool has_internal_error = (nvm_current_block_info.internal_flags & NVM_SET_ERROR_STATUS_FLAG) != 0u;
		job_result = has_internal_error ? NVM_REQ_NOT_OK : NVM_REQ_OK;
	}

	nvm_update_admin_block_zero_status(job_result, NVM_CLEAR_WRITE_ALL_FLAG);

	nvm_multi_block_callback(NVM_WRITE_ALL, job_result);
}

static FUNC(void, NVM_CODE) nvm_act_start_read_all(void)
{
	nvm_current_job.job_service_id = NVM_INT_FID_READ_ALL;
	nvm_current_block_info.internal_flags &= NVM_CLEAR_ERROR_STATUS_FLAG;
	uint16 index = 0u;

	for (index = 1u; index < NVM_NUM_OF_NVRAM_BLOCKS; index++) {
		if ((nvm_block_descriptor_list[index].ram_block_data_address == NULL) &&
		    (nvm_block_descriptor_list[index].cbk_set_mirror_func_pt == NULL)) {
			nvm_admin_block_table[index].error_status = NVM_REQ_BLOCK_SKIPPED;
			continue;
		}
		if ((nvm_block_descriptor_list[index].flags & NVM_SELECT_BLOCK_FOR_READALL_ON) != 0U) {
			nvm_admin_block_table[index].error_status = NVM_REQ_PENDING;
			nvm_block_notification(nvm_block_descriptor_list[index].nvram_block_identifier, NVM_READ_ALL_SID,
					       NVM_REQ_PENDING);
		} else {
			nvm_admin_block_table[index].error_status = NVM_REQ_BLOCK_SKIPPED;
		}
	}

	nvm_current_job.job_block_id = 0u;
	nvm_subtask_state = NVM_FSM_FSM_FINISHED;
}

static FUNC(void, NVM_CODE) nvm_act_read_next_block(void)
{
	nvm_current_job.job_block_id++;
	nvm_block_id_t current_id = nvm_current_job.job_block_id;

	while ((current_id < NVM_NUM_OF_NVRAM_BLOCKS) &&
	       (nvm_admin_block_table[current_id].error_status == NVM_REQ_BLOCK_SKIPPED)) {
		current_id++;
	}

	if (current_id >= NVM_NUM_OF_NVRAM_BLOCKS) {
		return;
	}

	nvm_current_block_info.Descriptor_pt = &(nvm_block_descriptor_list[current_id]);
	nvm_current_block_info.nv_block_id = nvm_current_block_info.Descriptor_pt->nv_block_base_number;
	nvm_current_block_info.ram_addr = nvm_current_block_info.Descriptor_pt->ram_block_data_address;
	nvm_current_block_info.mngmt_ptr = &(nvm_admin_block_table[current_id]);
	nvm_current_job.job_block_id = current_id;
#if (NVM_ENABLE_DYNAMIC_CONFIGURATION == STD_ON)
	if ((TRUE == nvm_configid_dismatch) &&
	    (!(nvm_is_bitfield_true(nvm_current_block_info.Descriptor_pt->resistant_to_changes_sw)))) {
		nvm_subtask_state = NVM_FSM_RESTORE_INIT;
	} else {
		nvm_subtask_state = NVM_FSM_READ_INIT;
	}
#else
	nvm_subtask_state = NVM_FSM_READ_INIT;
#endif
}

static FUNC(void, NVM_CODE) nvm_act_finish_read_all(void)
{
	nvm_request_result_type job_result;

	const bool has_internal_error = (nvm_current_block_info.internal_flags & NVM_SET_ERROR_STATUS_FLAG) != 0u;
	job_result = has_internal_error ? NVM_REQ_NOT_OK : NVM_REQ_OK;

	nvm_update_admin_block_zero_status(job_result, NVM_CLEAR_READ_ALL_FLAG);
	nvm_multi_block_callback(NVM_READ_ALL_SID, job_result);
}

static FUNC(void, NVM_CODE) nvm_act_wait(void)
{
	nvm_current_block_info.internal_flags |= NVM_SET_WAIT_CONDITION_FLAG;
}

static FUNC(void, NVM_CODE) nvm_act_nop(void)
{
}

static FUNC(void, NVM_CODE) nvm_act_get_high_prio_job(void)
{
#if (NVM_JOB_PRIORIZATION == STD_ON)
	Std_ReturnType result = E_NOT_OK;

	if (nvm_task_state == NVM_FSM_NORMAL_PRIO_JOB_1) {
		if (memif_get_status(nvm_current_block_info.Descriptor_pt->nvram_device_id) == MEMIF_BUSY) {
			memif_cancel(nvm_current_block_info.Descriptor_pt->nvram_device_id);
		}
		(void)nvm_queue_push(&nvm_queue_entry);
	}
	result = nvm_queue_pop(&nvm_high_job_queue, &nvm_queue_entry);

	if (result == E_OK) {
		nvm_current_block_info.Descriptor_pt = &nvm_block_descriptor_list[nvm_queue_entry.block_id];
		nvm_current_block_info.nv_block_id = nvm_current_block_info.Descriptor_pt->nv_block_base_number;
		nvm_current_block_info.mngmt_ptr = &nvm_admin_block_table[nvm_queue_entry.block_id];
		nvm_current_block_info.ram_addr = nvm_queue_entry.data_ptr;
		nvm_current_job.job_service_id = nvm_queue_entry.service_id;
		nvm_current_job.job_block_id = nvm_queue_entry.block_id;

		if (nvm_current_job.job_service_id == NVM_INT_FID_WRITE_BLOCK) {
			nvm_subtask_state = NVM_FSM_WRITE_INIT;
		} else {
			nvm_current_block_info.last_result = NVM_REQ_CANCELED;
		}

		nvm_block_notification(nvm_current_job.job_block_id, nvm_service_table[nvm_current_job.job_service_id],
				       nvm_current_block_info.mngmt_ptr->error_status);
	}
#endif
}

static FUNC(void, NVM_CODE) nvm_act_get_normal_prio_job(void)
{
	Std_ReturnType result = E_NOT_OK;
#if (NVM_JOB_PRIORIZATION == STD_ON)
	result = nvm_get_high_prio_queue_job(&nvm_job_queue, &nvm_queue_entry);
#else
	result = nvm_queue_pop(&nvm_job_queue, &nvm_queue_entry);
#endif
	if (result == E_OK) {
		nvm_current_block_info.Descriptor_pt = &nvm_block_descriptor_list[nvm_queue_entry.block_id];
		nvm_current_block_info.nv_block_id = nvm_current_block_info.Descriptor_pt->nv_block_base_number;
		nvm_current_block_info.mngmt_ptr = &nvm_admin_block_table[nvm_queue_entry.block_id];
		nvm_current_block_info.ram_addr = nvm_queue_entry.data_ptr;
		nvm_current_job.job_service_id = nvm_queue_entry.service_id;
		nvm_current_job.job_block_id = nvm_queue_entry.block_id;

		if (nvm_queue_entry.service_id != NVM_INT_FID_INVALID_JOB) {
			switch (nvm_current_job.job_service_id) {
			case NVM_INT_FID_WRITE_BLOCK:
				nvm_subtask_state = NVM_FSM_WRITE_INIT;
				break;
			case NVM_INT_FID_READ_BLOCK:
				nvm_subtask_state = NVM_FSM_READ_INIT;
				break;
			case NVM_INT_FID_RESTORE_DEFAULTS:
				nvm_subtask_state = NVM_FSM_RESTORE_INIT;
				break;
			case NVM_INT_FID_INVALIDATE_NV_BLOCK:
				nvm_subtask_state = NVM_FSM_INVALIDATE_INIT;
				break;
			case NVM_INT_FID_ERASE_BLOCK:
				nvm_subtask_state = NVM_FSM_ERASE_INIT;
				break;
			default:
				/*do nothing*/
				break;
			}
		} else {
			nvm_current_block_info.last_result = NVM_REQ_CANCELED;
		}
		nvm_block_notification(nvm_current_job.job_block_id, nvm_service_table[nvm_current_job.job_service_id],
				       nvm_current_block_info.mngmt_ptr->error_status);
	}
}

static FUNC(void, NVM_CODE) nvm_act_finish_block(void)
{
	nvm_current_block_info.mngmt_ptr->error_status = nvm_current_block_info.last_result;
	nvm_current_job.job_service_id = NVM_INT_FID_NO_JOB_PENDING;
	nvm_current_job.ram_addr = NULL;
}

static FUNC(void, NVM_CODE) nvm_act_write_all_cancel(void)
{
	uint16 index = 0;

	nvm_admin_block_table[0].error_status = NVM_REQ_CANCELED;

	/* block[blockIndex] has been Writen successfully */
	for (index = nvm_current_job.job_block_id; index > 0u; index--) {
		if (nvm_admin_block_table[index].error_status == NVM_REQ_PENDING) {
			nvm_admin_block_table[index].error_status = NVM_REQ_CANCELED;
		}
	}
}

static FUNC(void, NVM_CODE) nvm_act_add_tried_times(void)
{
	++nvm_current_block_info.retry_times;
}

static FUNC(void, NVM_CODE) nvm_update_compare_crc_data(nvm_request_result_type job_result)
{
	const nvm_block_descriptor_type *desc = nvm_current_block_info.Descriptor_pt;
	nvm_admin_block_type *adm = nvm_current_block_info.mngmt_ptr;

	if ((desc->flags & NVM_CRC_COMP_MECHANISM_ON) != 0u) {
		if (job_result == NVM_REQ_OK) {
			if (desc->block_crc_type == NVM_CRC16_ON) {
				adm->crc_compare_val.crc16 = adm->ram_crc.crc16;
			} else {
				adm->crc_compare_val.crc32 = adm->ram_crc.crc32;
			}
			adm->crc_compare_valid = TRUE;
		} else {
			if (desc->block_crc_type == NVM_CRC16_ON) {
				adm->crc_compare_val.crc16 = 0xffffU;
			} else {
				adm->crc_compare_val.crc32 = 0xffffffffU;
			}
			adm->crc_compare_valid = FALSE;
		}
	}
}

FUNC(void, NVM_CODE)
nvm_block_notification(nvm_block_id_t block_id, nvm_service_id_t service_id, nvm_request_result_type job_result)
{
	const nvm_block_descriptor_type *desc = &nvm_block_descriptor_list[block_id];

	if (job_result != NVM_REQ_PENDING) {
		if (desc->single_block_callback != NULL) {
			(void)desc->single_block_callback(service_id, job_result);
		}
	}
	if (nvm_is_bitfield_true(desc->notify_bsw)) {
		NvM_INVOKE_CURRENT_MODE(block_id, job_result);
	}
}

#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"

#define NVM_START_SEC_CONST
#include "nvm_memmap.h"
CONST(nvm_act_fct_ptr_t, NVM_CONST)
nvm_action_table[NVM_ACT_Nop + 1] = { &nvm_act_get_high_prio_job,
				      &nvm_act_get_normal_prio_job,
				      &nvm_act_read_nv_block,
				      &nvm_act_read_nv_finished,
				      &nvm_act_finish_read_block,
				      &nvm_act_setup_redundant,
				      &nvm_act_finish_restore,
				      &nvm_act_restore_block_defaults,
				      &nvm_act_init_write_block,
				      &nvm_act_write_nv_block,
				      &nvm_act_write_finished,
				      &nvm_act_init_invalidate_block,
				      &nvm_act_erase_nv_block,
				      &nvm_act_start_write_all,
				      &nvm_act_write_next_block,
				      &nvm_act_finish_write_all,
				      &nvm_act_write_all_cancel,
				      &nvm_act_start_read_all,
				      &nvm_act_read_next_block,
				      &nvm_act_finish_read_all,
				      &nvm_act_finish_block,
				      &nvm_act_add_tried_times,
				      &nvm_act_wait,
				      &nvm_act_nop };

#define NVM_STOP_SEC_CONST
#include "nvm_memmap.h"
