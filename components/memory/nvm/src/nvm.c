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

#include <stdlib.h>
#include <string.h>

#include "memif.h"
#include "nvm.h"
#include "nvm_queue.h"
#include "nvm_cbk.h"
#include "nvm_internal.h"
#include "nvm_crc.h"
#include "nvm_fsm.h"
#include "SchM_NvM.h"
#include "vcos_log.h"
#include "nvm_vcos_log.h"

#define DET_VALIDATE_NO_RV(_exp, _err)                                                                                 \
	do {                                                                                                           \
		if (!(_exp)) {                                                                                         \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

#define DET_VALIDATE_RV(_exp, _err, _rv)                                                                               \
	do {                                                                                                           \
		if (!(_exp)) {                                                                                         \
			return _rv;                                                                                    \
		}                                                                                                      \
	} while (0)
#define NVM_DET_REPORTERROR(_err)

#define NVM_START_SEC_PRIVATE_BSS
#include "nvm_memmap.h"
VAR(static uint32, NVM_PRIVATE_BSS) write_block_call_cnt;
VAR(uint8, NVM_PRIVATE_BSS) nvm_api_flags;
VAR(static boolean, NVM_PRIVATE_BSS) nvm_enable_execute;
#define NVM_STOP_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
FUNC(void, NVM_CODE) nvm_mainfunction(void)
{
	if (nvm_enable_execute == TRUE) {
		DET_VALIDATE_NO_RV((nvm_task_state != NVM_FSM_UNINIT), NVM_NVM_MAINFUNCTION_NVM_E_NOT_INITIALIZED);
		nvm_current_block_info.internal_flags &= NVM_CLEAR_WAIT_CONDITION_FLAG;
		do {
			nvm_task_state = nvm_fsm_excute(nvm_task_state);
			nvm_subtask_state = nvm_fsm_excute(nvm_subtask_state);
		} while ((nvm_current_block_info.internal_flags & NVM_SET_WAIT_CONDITION_FLAG) == 0u);
	}
}

FUNC(void, NVM_CODE) nvm_init(void)
{
	uint16 index = 0u;

	nvm_enable_execute = TRUE;

	nvm_queue_init();

	/* set the ram attribute of block 0 to ok */
	nvm_admin_block_table[0].error_status = NVM_REQ_OK;

	/* Initiate the administration blocks */
	for (index = 1u; index < NVM_NUM_OF_NVRAM_BLOCKS; index++) {
		nvm_admin_block_table[index].data_index = 0u;
		nvm_admin_block_table[index].attributes = 0u;
		if ((nvm_block_descriptor_list[index].flags & NVM_BLOCK_WRITE_PROT_ON) != 0u) {
			nvm_admin_block_table[index].attributes |= NVM_SET_WRITE_PROTECTION_STATUS;
		}
		nvm_admin_block_table[index].crc_compare_valid = FALSE;
	}

	/* Init FSM State */
	nvm_fsm_init();
}

FUNC(void, NVM_CODE) nvm_deinit(void)
{
	nvm_enable_execute = FALSE;

	// 清零NvM工作缓冲区
	(void)memset(nvm_work_buffer, 0, sizeof(nvm_work_buffer));

	nvm_queue_deinit();
	nvm_fsm_deinit();
}

FUNC(void, NVM_CODE) nvm_read_all(void)
{
	(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_INFO, NVM_DLT_SYS_READALL);
	DET_VALIDATE_NO_RV((nvm_task_state != NVM_FSM_UNINIT), NVM_NVM_READ_ALL_NVM_E_NOT_INITIALIZED);
	DET_VALIDATE_NO_RV((nvm_admin_block_table[0].error_status != NVM_REQ_PENDING),
			   NVM_NVM_READ_ALL_NVM_E_BLOCK_PENDING);

	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	nvm_admin_block_table[0].error_status = NVM_REQ_PENDING;
	nvm_api_flags |= NVM_SET_READ_ALL_FLAG;
	nvm_current_job.job_service_id = NVM_INT_FID_READ_ALL;
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();

	nvm_multi_block_callback(NVM_READ_ALL_SID, NVM_REQ_PENDING);
}

FUNC(void, NVM_CODE) nvm_write_all(void)
{
	(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_INFO, NVM_DLT_SYS_WRITEALL);
	DET_VALIDATE_NO_RV((nvm_task_state != NVM_FSM_UNINIT), NVM_NVM_WRITE_ALL_NVM_E_NOT_INITIALIZED);
	DET_VALIDATE_NO_RV((nvm_admin_block_table[0].error_status != NVM_REQ_PENDING),
			   NVM_NVM_WRITE_ALL_NVM_E_BLOCK_PENDING);

	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	nvm_admin_block_table[0].error_status = NVM_REQ_PENDING;
	nvm_api_flags &= NVM_CLEAR_CANCEL_WRITE_ALL_FLAG;
	nvm_api_flags |= NVM_SET_WRITE_ALL_FLAG;
	nvm_current_job.job_service_id = NVM_INT_FID_WRITE_ALL;
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();

	nvm_multi_block_callback(NVM_WRITE_ALL, NVM_REQ_PENDING);
}

FUNC(void, NVM_CODE) nvm_set_block_lock_status(nvm_block_id_t block_id, boolean block_locked)
{
	DET_VALIDATE_NO_RV((nvm_task_state != NVM_FSM_UNINIT), NVM_NVM_SET_BLOCK_LOCK_STATUS_NVM_E_NOT_INITIALIZED);
	DET_VALIDATE_NO_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_SET_BLOCK_LOCK_STATUS_NVM_E_PARAM_BLOCK_ID);
	DET_VALIDATE_NO_RV((nvm_admin_block_table[block_id].error_status != NVM_REQ_PENDING),
			   NVM_NVM_SET_BLOCK_LOCK_STATUS_NVM_E_BLOCK_PENDING);

	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	if (block_locked) {
		nvm_admin_block_table[block_id].attributes |= NVM_SET_LOCK_STATUS;
	} else {
		nvm_admin_block_table[block_id].attributes &= NVM_CLEAR_LOCK_STATUS;
	}
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();
}

FUNC(Std_ReturnType, NVM_CODE) nvm_read_block(nvm_block_id_t block_id, void *dst_ptr)
{
	nvm_queue_data_t queue_entry;
	Std_ReturnType result = E_NOT_OK;

	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_READ_BLOCK_NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_READ_BLOCK_NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
	DET_VALIDATE_RV(!((dst_ptr == NULL) && (nvm_block_descriptor_list[block_id].ram_block_data_address == NULL) &&
			  (nvm_block_descriptor_list[block_id].cbk_set_mirror_func_pt == NULL)),
			NVM_NVM_READ_BLOCK_NVM_E_PARAM_ADDRESS, E_NOT_OK);
	DET_VALIDATE_RV((nvm_admin_block_table[block_id].error_status != NVM_REQ_PENDING),
			NVM_NVM_READ_BLOCK_NVM_E_BLOCK_PENDING, E_NOT_OK);

	queue_entry.service_id = NVM_INT_FID_READ_BLOCK;
	queue_entry.block_id = block_id;
	queue_entry.data_ptr = (uint8 *)dst_ptr;

#if (NVM_JOB_PRIORIZATION == STD_ON)
	queue_entry.job_prio = nvm_block_descriptor_list[block_id].block_job_priority;
#endif
	result = nvm_queue_push(&queue_entry);
	if (result == E_OK) {
		SchM_Enter_NVM_EXCLUSIVE_AREA_0();
		nvm_admin_block_table[block_id].error_status = NVM_REQ_PENDING;
		nvm_admin_block_table[block_id].attributes &= (NVM_CHANGED_STATE_CL & NVM_VALID_STATE_CL);
		SchM_Exit_NVM_EXCLUSIVE_AREA_0();
	} else {
		NVM_DET_REPORTERROR(NVM_NVM_READ_BLOCK_NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}
	return E_OK;
}

/**
 * Service to copy a RAM block to its correspnding NVRAM block
 *
 * @param block_id
 * @param src_ptr
 * @return
 */
FUNC(Std_ReturnType, NVM_CODE) nvm_write_block(nvm_block_id_t block_id, const void *src_ptr)
{
	nvm_queue_data_t queue_entry;
	Std_ReturnType result = E_NOT_OK;

	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_WRITE_BLOCK_NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_WRITE_BLOCK_NVM_E_PARAM_BLOCK_ID, E_NOT_OK);

	nvm_admin_block_type *adm_ptr = &nvm_admin_block_table[block_id];

	DET_VALIDATE_RV(adm_ptr->data_index < nvm_block_descriptor_list[block_id].nv_block_num,
			NVM_NVM_WRITE_BLOCK_NVM_E_PARAM_BLOCK_DATA_IDX, E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->attributes & NVM_SET_WRITE_PROTECTION_STATUS) == 0u, NVM_NVM_WRITE_BLOCK_NVM_E_NV_WRITE_PROTECTED,
			E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->attributes & NVM_SET_LOCK_STATUS) == 0u, NVM_NVM_WRITE_BLOCK_NVM_E_BLOCK_LOCKED,
			E_NOT_OK);
	DET_VALIDATE_RV(!((src_ptr == NULL) && (nvm_block_descriptor_list[block_id].ram_block_data_address == NULL) &&
			  (nvm_block_descriptor_list[block_id].cbk_get_mirror_func_pt == NULL)),
			NVM_NVM_WRITE_BLOCK_NVM_E_PARAM_ADDRESS, E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->error_status != NVM_REQ_PENDING), NVM_NVM_WRITE_BLOCK_NVM_E_BLOCK_PENDING, E_NOT_OK);

	queue_entry.service_id = NVM_INT_FID_WRITE_BLOCK;
	queue_entry.block_id = block_id;
	queue_entry.data_ptr = (uint8 *)src_ptr;
#if (NVM_JOB_PRIORIZATION == STD_ON)
	queue_entry.job_prio = nvm_block_descriptor_list[block_id].block_job_priority;
#endif
	result = nvm_queue_push(&queue_entry);
	if (result == E_OK) {
		SchM_Enter_NVM_EXCLUSIVE_AREA_0();
		adm_ptr->error_status = NVM_REQ_PENDING;
		adm_ptr->attributes |= (NVM_CHANGED_STATE_SET | NVM_VALID_STATE_SET);
		SchM_Exit_NVM_EXCLUSIVE_AREA_0();
	} else {
		NVM_DET_REPORTERROR(NVM_NVM_WRITE_BLOCK_NVM_E_LIST_OVERFLOW);

		write_block_call_cnt++;
		(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_ERROR, NVM_DLT_SYS_WRITEBLOCK, block_id, E_NOT_OK,
			       write_block_call_cnt);

		return E_NOT_OK;
	}

	write_block_call_cnt++;
	(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_INFO, NVM_DLT_SYS_WRITEBLOCK, block_id, E_OK, write_block_call_cnt);

	return E_OK;
}

FUNC(void, NVM_CODE) nvm_cancel_write_all(void)
{
	(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_INFO, NVM_DLT_SYS_CANCEL_WRITEALL);

	DET_VALIDATE_NO_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_CANCEL_WRITE_ALL_NVM_E_NOT_INITIALIZED);

	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	nvm_api_flags |= NVM_SET_CANCEL_WRITE_ALL_FLAG;
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();
}

FUNC(Std_ReturnType, NVM_CODE) nvm_set_data_index(nvm_block_id_t block_id, uint8 data_index)
{
	nvm_admin_block_table[block_id].data_index = data_index;
	nvm_admin_block_table[block_id].error_status = NVM_REQ_OK;

	return E_OK;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_get_data_index(nvm_block_id_t block_id, uint8 *data_index_ptr)
{
	*data_index_ptr = nvm_admin_block_table[block_id].data_index;
	return E_OK;
}

FUNC(void, NVM_CODE) nvm_control_main_function(boolean enable_execute)
{
	nvm_enable_execute = enable_execute;
}

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_1)

FUNC(Std_ReturnType, NVM_CODE) nvm_get_error_status(nvm_block_id_t block_id, nvm_request_result_type *request_result)
{
	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_GET_ERROR_STATUS_NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_GET_ERROR_STATUS_NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
	DET_VALIDATE_RV((request_result != NULL), NVM_NVM_GET_ERROR_STATUS_NVM_E_PARAM_DATA, E_NOT_OK);

	*request_result = nvm_admin_block_table[block_id].error_status;
	return E_OK;
}

#if (NVM_ENABLE_SET_RAM_BLOCK_STATUS_API == STD_ON)

FUNC(Std_ReturnType, NVM_CODE) nvm_set_ram_block_status(nvm_block_id_t block_id, boolean block_changed)
{
	nvm_admin_block_type *adm_ptr = &nvm_admin_block_table[block_id];

	DET_VALIDATE_RV((block_id < NVM_NUM_OF_NVRAM_BLOCKS), NVM_NVM_SET_RAM_BLOCK_STATUS_NVM_E_PARAM_BLOCK_ID,
			E_NOT_OK);
	DET_VALIDATE_RV((nvm_task_state != NVM_FSM_UNINIT), NVM_NVM_SET_RAM_BLOCK_STATUS_NVM_E_NOT_INITIALIZED,
			E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->error_status != NVM_REQ_PENDING), NVM_NVM_SET_RAM_BLOCK_STATUS_NVM_E_BLOCK_PENDING,
			E_NOT_OK);

	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	if (TRUE == block_changed) {
		adm_ptr->attributes |= (NVM_CHANGED_STATE_SET | NVM_VALID_STATE_SET);
	} else {
		adm_ptr->attributes &= (NVM_CHANGED_STATE_CL & NVM_VALID_STATE_CL);
	}
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();

	return E_OK;
}
#endif

FUNC(Std_ReturnType, NVM_CODE) nvm_restore_block_defaults(nvm_block_id_t block_id, void *dst_ptr)
{
	const nvm_block_descriptor_type *block_ptr = &nvm_block_descriptor_list[block_id];
	nvm_admin_block_type *adm_ptr = &nvm_admin_block_table[block_id];
	Std_ReturnType result = E_NOT_OK;
	nvm_queue_data_t queue_entry;

	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_RESTORE_BLOCK_DEFAULTS_NVM_E_NOT_INITIALIZED,
			E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_RESTORE_BLOCK_DEFAULTS_NVM_E_PARAM_BLOCK_ID,
			E_NOT_OK);
	/* It must be a permanent RAM block but no ram_block_data_address -> error */
	DET_VALIDATE_RV(!((dst_ptr == NULL) && (block_ptr->ram_block_data_address == NULL)),
			NVM_NVM_RESTORE_BLOCK_DEFAULTS_NVM_E_PARAM_ADDRESS, E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->error_status != NVM_REQ_PENDING), NVM_NVM_RESTORE_BLOCK_DEFAULTS_NVM_E_BLOCK_PENDING,
			E_NOT_OK);
	DET_VALIDATE_RV(!((block_ptr->rom_block_data_adress == NULL) && (block_ptr->init_block_callback == NULL)),
			NVM_NVM_RESTORE_BLOCK_DEFAULTS_NVM_E_BLOCK_CONFIG, E_NOT_OK);

	queue_entry.block_id = block_id;
	queue_entry.service_id = NVM_INT_FID_RESTORE_DEFAULTS;
	queue_entry.data_ptr = (uint8 *)dst_ptr;
#if (NVM_JOB_PRIORIZATION == STD_ON)
	queue_entry.job_prio = nvm_block_descriptor_list[block_id].block_job_priority;
#endif
	result = nvm_queue_push(&queue_entry);
	if (result == E_OK) {
		SchM_Enter_NVM_EXCLUSIVE_AREA_0();
		adm_ptr->error_status = NVM_REQ_PENDING;
		adm_ptr->attributes &= (NVM_CHANGED_STATE_CL & NVM_VALID_STATE_CL);
		SchM_Exit_NVM_EXCLUSIVE_AREA_0();
	} else {
		NVM_DET_REPORTERROR(NVM_NVM_RESTORE_BLOCK_DEFAULTS_NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	return result;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_cancel_jobs(nvm_block_id_t block_id)
{
	Std_ReturnType return_value = E_NOT_OK;

	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_CANCEL_JOBS_NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_CANCEL_JOBS_NVM_E_PARAM_BLOCK_ID, E_NOT_OK);

	return_value = nvm_invalidate_queue_job(block_id);
	if (return_value == E_OK) {
		nvm_admin_block_table[block_id].error_status = NVM_REQ_CANCELED;
	} else {
		(void)VCOS_LOG(NVM_MODULE_ID, VCOS_LOG_ERROR, NVM_DLT_JOB_CANCEL_FAIL, block_id);
	}

	return return_value;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_read_pram_block(nvm_block_id_t block_id)
{
	return nvm_read_block(block_id, NULL);
}

FUNC(Std_ReturnType, NVM_CODE) nvm_write_pram_block(nvm_block_id_t block_id)
{
	return nvm_write_block(block_id, NULL);
}

FUNC(Std_ReturnType, NVM_CODE) nvm_restore_pram_block_defaults(nvm_block_id_t block_id)
{
	return nvm_restore_block_defaults(block_id, NULL);
}

#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_2)

FUNC(Std_ReturnType, NVM_CODE) nvm_set_block_protection(nvm_block_id_t block_id, boolean protection_enabled)
{
	const nvm_block_descriptor_type *block_ptr = &nvm_block_descriptor_list[block_id];
	nvm_admin_block_type *adm_ptr = &nvm_admin_block_table[block_id];
	Std_ReturnType result = E_NOT_OK;

	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_SET_BLOCK_PROTECTION_NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_SET_BLOCK_PROTECTION_NVM_E_PARAM_BLOCK_ID,
			E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->error_status != NVM_REQ_PENDING), NVM_NVM_SET_BLOCK_PROTECTION_NVM_E_BLOCK_PENDING,
			E_NOT_OK);

	if ((block_ptr->flags & NVM_BLOCK_WRITE_BLOCK_ONCE_ON) == 0u) {
		const uint8 operation = protection_enabled ? (adm_ptr->attributes | NVM_SET_WRITE_PROTECTION_STATUS) :
							     (adm_ptr->attributes & NVM_CLEAR_WRITE_PROTECTION_STATUS);
		SchM_Enter_NVM_EXCLUSIVE_AREA_0();
		adm_ptr->attributes = operation;
		SchM_Exit_NVM_EXCLUSIVE_AREA_0();
		result = E_OK;
	}
	return result;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_invalidate_nv_block(nvm_block_id_t block_id)
{
	nvm_admin_block_type *adm_ptr = &nvm_admin_block_table[block_id];
	Std_ReturnType result = E_NOT_OK;
	nvm_queue_data_t queue_entry;

	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_INVALIDATE_NV_BLOCK_NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_INVALIDATE_NV_BLOCK_NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->error_status != NVM_REQ_PENDING), NVM_NVM_INVALIDATE_NV_BLOCK_NVM_E_BLOCK_PENDING,
			E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->attributes & NVM_SET_WRITE_PROTECTION_STATUS) == 0u,
			NVM_NVM_INVALIDATE_NV_BLOCK_NVM_E_NV_WRITE_PROTECTED, E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->attributes & NVM_SET_LOCK_STATUS) == 0u, NVM_NVM_INVALIDATE_NV_BLOCK_NVM_E_BLOCK_LOCKED,
			E_NOT_OK);

	queue_entry.block_id = block_id;
	queue_entry.service_id = NVM_INT_FID_INVALIDATE_NV_BLOCK;
#if (NVM_JOB_PRIORIZATION == STD_ON)
	queue_entry.job_prio = nvm_block_descriptor_list[block_id].block_job_priority;
#endif
	result = nvm_queue_push(&queue_entry);
	if (result == E_OK) {
		adm_ptr->error_status = NVM_REQ_PENDING;
	} else {
		NVM_DET_REPORTERROR(NVM_NVM_INVALIDATE_NV_BLOCK_NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	return result;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_erase_nv_block(nvm_block_id_t block_id)
{
	nvm_admin_block_type *adm_ptr = &nvm_admin_block_table[block_id];
	Std_ReturnType result = E_NOT_OK;
	nvm_queue_data_t queue_entry;

	DET_VALIDATE_RV(nvm_task_state != NVM_FSM_UNINIT, NVM_NVM_ERASE_BLOCK_NVM_E_NOT_INITIALIZED, E_NOT_OK);
	DET_VALIDATE_RV(block_id < NVM_NUM_OF_NVRAM_BLOCKS, NVM_NVM_ERASE_BLOCK_NVM_E_PARAM_BLOCK_ID, E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->error_status != NVM_REQ_PENDING), NVM_NVM_ERASE_BLOCK_NVM_E_BLOCK_PENDING, E_NOT_OK);
	DET_VALIDATE_RV(adm_ptr->data_index < nvm_block_descriptor_list[block_id].nv_block_num,
			NVM_NVM_ERASE_BLOCK_NVM_E_PARAM_BLOCK_DATA_IDX, E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->attributes & NVM_SET_WRITE_PROTECTION_STATUS) == 0u, NVM_NVM_ERASE_BLOCK_NVM_E_NV_WRITE_PROTECTED,
			E_NOT_OK);
	DET_VALIDATE_RV((adm_ptr->attributes & NVM_SET_LOCK_STATUS) == 0u, NVM_NVM_ERASE_BLOCK_NVM_E_BLOCK_LOCKED,
			E_NOT_OK);
	DET_VALIDATE_RV(nvm_block_descriptor_list[block_id].block_job_priority == 0u,
			NVM_NVM_ERASE_BLOCK_NVM_E_BLOCK_CONFIG, E_NOT_OK);

	queue_entry.block_id = block_id;
	queue_entry.service_id = NVM_INT_FID_ERASE_BLOCK;
#if (NVM_JOB_PRIORIZATION == STD_ON)
	queue_entry.job_prio = nvm_block_descriptor_list[block_id].block_job_priority;
#endif
	result = nvm_queue_push(&queue_entry);
	if (result == E_OK) {
		adm_ptr->error_status = NVM_REQ_PENDING;
	} else {
		NVM_DET_REPORTERROR(NVM_NVM_ERASE_BLOCK_NVM_E_LIST_OVERFLOW);
		return E_NOT_OK;
	}

	return result;
}

#endif

#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
