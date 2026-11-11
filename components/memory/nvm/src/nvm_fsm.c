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
#include "nvm_fsm.h"

#define NVM_START_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

VAR(nvm_block_info_type, NVM_PRIVATE_BSS) nvm_current_block_info;
VAR(nvm_job_t, NVM_PRIVATE_BSS) nvm_current_job;
VAR(nvm_fsm_state_type, NVM_PRIVATE_BSS) nvm_task_state;
VAR(nvm_fsm_state_type, NVM_PRIVATE_BSS) nvm_subtask_state;

#define NVM_STOP_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

#define NVM_START_SEC_CONST
#include "nvm_memmap.h"
CONST(nvm_service_id_t, NVM_CONST)
nvm_service_table[NVM_INT_FID_INVALID_JOB + 1u] = {
	NVM_WRITE_BLOCK, NVM_READ_BLOCK_SID, NVM_RESTORE_BLOCK_DEFAULTS_SID, NVM_INVALIDATE_NV_BLOCK_SID, NVM_ERASE_BLOCK_SID,
	NVM_WRITE_ALL,	 NVM_READ_ALL_SID,	 NVM_MAIN_FUNCTION_SID,	     NVM_CANCEL_JOBS_SID
};

#define NVM_STATE(_ev_true, _act_true, _next_true, _act_false, _next_false)                                            \
	{                                                                                                              \
		{ (_ev_true), (_act_true), (_next_true) },                                                             \
		{                                                                                                      \
			(_act_false), (_next_false)                                                                    \
		}                                                                                                      \
	}

#define NVM_STATE_WAIT_NV(_exec_action_on_idle, _next_on_idle, _self_state)                                            \
	NVM_STATE(NVM_EVENT_NV_BUSY, NVM_ACT_Wait, (_self_state), (_exec_action_on_idle), (_next_on_idle))

static CONST(nvm_state_descr_type, NVM_CONST) nvm_state_descr_table_at[] = {
	/* NVM_FSM_UNINIT */
	NVM_STATE(NVM_EVENT_TRUE, NVM_ACT_Wait, NVM_FSM_UNINIT, NVM_ACT_Wait, NVM_FSM_UNINIT),

	/* NVM_FSM_IDLE */
	NVM_STATE(NVM_EVENT_HAVE_JOB, NVM_ACT_Nop, NVM_FSM_HAVE_JOB, NVM_ACT_Wait, NVM_FSM_IDLE),

	/* NVM_FSM_HAVE_JOB */
	NVM_STATE(NVM_EVENT_HAVE_HIGHPRIO_JOB, NVM_ACT_GetHighPrioJob, NVM_FSM_HIGH_PRIO_JOB, NVM_ACT_Nop,
		  NVM_FSM_NOTHIGHPRIO_JOB),

	/* NVM_FSM_HIGH_PRIO_JOB */
	NVM_STATE(NVM_EVENT_SUBTASK_RUNNING, NVM_ACT_Nop, NVM_FSM_HIGH_PRIO_JOB, NVM_ACT_FinishBlock, NVM_FSM_IDLE),

	/* NVM_FSM_NOTHIGHPRIO_JOB */
	NVM_STATE(NVM_EVENT_NORMAL_PRIO_JOB, NVM_ACT_GetNormalPrioJob, NVM_FSM_NORMAL_PRIO_JOB_1, NVM_ACT_Nop,
		  NVM_FSM_MULTI_BLOCK_JOB),

	/* NVM_FSM_NORMAL_PRIO_JOB_1 */
	NVM_STATE(NVM_EVENT_HAVE_HIGHPRIO_JOB, NVM_ACT_GetHighPrioJob, NVM_FSM_HIGH_PRIO_JOB, NVM_ACT_Nop,
		  NVM_FSM_NORMAL_PRIO_JOB_2),

	/* NVM_FSM_NORMAL_PRIO_JOB_2 */
	NVM_STATE(NVM_EVENT_SUBTASK_RUNNING, NVM_ACT_Nop, NVM_FSM_NORMAL_PRIO_JOB_1, NVM_ACT_FinishBlock, NVM_FSM_IDLE),

	/* NVM_FSM_MULTI_BLOCK_JOB */
	NVM_STATE(NVM_EVENT_HAVEWRITEALL_JOB, NVM_ACT_StartWriteAll, NVM_FSM_WRITEALL_PROC_1, NVM_ACT_StartReadAll,
		  NVM_FSM_READALL_PROC_1),

	/* NVM_FSM_READ_INIT */
	NVM_STATE_WAIT_NV(NVM_ACT_ReadNvBlock, NVM_FSM_READ_READ_DATA, NVM_FSM_READ_INIT),

	/* NVM_FSM_READ_READ_DATA */
	NVM_STATE_WAIT_NV(NVM_ACT_Nop, NVM_FSM_READ_NV_FINISH, NVM_FSM_READ_READ_DATA),

	/* NVM_FSM_READ_NV_FINISH */
	NVM_STATE(NVM_EVENT_LAST_RESULT_OK, NVM_ACT_Nop, NVM_FSM_READ_CMP_CRC, NVM_ACT_Nop, NVM_FSM_READ_RETRY),

	/* NVM_FSM_READ_CMP_CRC */
	NVM_STATE(NVM_EVENT_CRC_MATCH, NVM_ACT_FinishReadBlock, NVM_FSM_FSM_FINISHED, NVM_ACT_Nop, NVM_FSM_READ_RETRY),

	/* NVM_FSM_READ_RETRY */
	NVM_STATE(NVM_EVENT_READ_RETRY, NVM_ACT_AddTriedTimes, NVM_FSM_READ_INIT, NVM_ACT_Nop, NVM_FSM_READ_RECOV),

	/* NVM_FSM_READ_RECOV */
	NVM_STATE(NVM_EVENT_REDUNDANT_BLOCK, NvM_ACT_SetupRedundant, NVM_FSM_READ_INIT, NVM_ACT_FinishRestoreRom,
		  NVM_FSM_FSM_FINISHED),

	/* NVM_FSM_WRITE_INIT */
	NVM_STATE(NVM_EVENT_TRUE, NVM_ACT_InitWriteBlock, NVM_FSM_CRCCOMPARE, NVM_ACT_Nop, NVM_FSM_FSM_FINISHED),

	/* NVM_FSM_CRCCOMPARE */
	NVM_STATE(NVM_EVENT_CrcCompare, NVM_ACT_WriteFinshed, NVM_FSM_FSM_FINISHED, NVM_ACT_Nop, NVM_FSM_WRITE_DATA_1),

	/* NVM_FSM_WRITE_DATA_1 */
	NVM_STATE_WAIT_NV(NVM_ACT_WriteNvBlock, NVM_FSM_WRITE_DATA_2, NVM_FSM_WRITE_DATA_1),

	/* NVM_FSM_WRITE_DATA_2 */
	NVM_STATE_WAIT_NV(NVM_ACT_Nop, NVM_FSM_WRITE_RETRY, NVM_FSM_WRITE_DATA_2),

	/* NVM_FSM_WRITE_RETRY */
	NVM_STATE(NVM_EVENT_WRITE_RETRY, NVM_ACT_AddTriedTimes, NVM_FSM_WRITE_DATA_1, NVM_ACT_Nop,
		  NVM_FSM_WRITE_REDUNDANT),

	/* NVM_FSM_WRITE_REDUNDANT */
	NVM_STATE(NVM_EVENT_REDUNDANT_BLOCK, NvM_ACT_SetupRedundant, NVM_FSM_WRITE_DATA_1, NVM_ACT_WriteFinshed,
		  NVM_FSM_FSM_FINISHED),

	/* NVM_FSM_RESTORE_INIT */
	NVM_STATE(NVM_EVENT_TRUE, NVM_ACT_RestoreBlockDefaults, NVM_FSM_FSM_FINISHED, NVM_ACT_Nop,
		  NVM_FSM_FSM_FINISHED),

	/* NVM_FSM_WRITEALL_PROC_1 */
	NVM_STATE(NVM_EVENT_SUBTASK_RUNNING, NVM_ACT_Nop, NVM_FSM_WRITEALL_PROC_1, NVM_ACT_WriteNextBlock,
		  NVM_FSM_WRITEALL_PROC_2),

	/* NVM_FSM_WRITEALL_PROC_2 */
	NVM_STATE(NVM_EVENT_WRITEALL_END, NVM_ACT_FinishWriteAll, NVM_FSM_IDLE, NVM_ACT_Nop, NVM_FSM_WRITEALL_PROC_1),

	/* NVM_FSM_READALL_PROC_1 */
	NVM_STATE(NVM_EVENT_SUBTASK_RUNNING, NVM_ACT_Nop, NVM_FSM_READALL_PROC_1, NVM_ACT_Nop, NVM_FSM_READALL_PROC_2),

	/* NVM_FSM_READALL_PROC_2 */
	NVM_STATE(NVM_EVENT_LAST_BLOCK_DONE_READALL, NVM_ACT_FinishReadAll, NVM_FSM_IDLE, NVM_ACT_ReadNextBlock,
		  NVM_FSM_READALL_PROC_1),

	/* NVM_FSM_INVALIDATE_INIT */
	NVM_STATE_WAIT_NV(NVM_ACT_InitInvalidteBlock, NVM_FSM_INVALIDATE_ING, NVM_FSM_INVALIDATE_INIT),

	/* NVM_FSM_INVALIDATE_ING */
	NVM_STATE_WAIT_NV(NVM_ACT_Nop, NVM_FSM_INVALIDATE_REDUNDANT, NVM_FSM_INVALIDATE_ING),

	/* NVM_FSM_INVALIDATE_REDUNDANT */
	NVM_STATE(NVM_EVENT_REDUNDANT_BLOCK, NvM_ACT_SetupRedundant, NVM_FSM_INVALIDATE_INIT, NVM_ACT_FinishBlock,
		  NVM_FSM_FSM_FINISHED),

	/* NVM_FSM_ERASE_INIT */
	NVM_STATE_WAIT_NV(NVM_ACT_EraseNvBlock, NVM_FSM_ERASE_ING, NVM_FSM_ERASE_INIT),

	/* NVM_FSM_ERASE_ING */
	NVM_STATE_WAIT_NV(NVM_ACT_Nop, NVM_FSM_ERASE_REDUNDANT, NVM_FSM_ERASE_ING),

	/* NVM_FSM_ERASE_REDUNDANT */
	NVM_STATE(NVM_EVENT_REDUNDANT_BLOCK, NvM_ACT_SetupRedundant, NVM_FSM_ERASE_INIT, NVM_ACT_FinishBlock,
		  NVM_FSM_FSM_FINISHED),

	/* NVM_FSM_FSM_FINISHED */
	NVM_STATE(NVM_EVENT_TRUE, NVM_ACT_Nop, NVM_FSM_FSM_FINISHED, NVM_ACT_Nop, NVM_FSM_FSM_FINISHED)
};

#define NVM_STOP_SEC_CONST
#include "nvm_memmap.h"

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
static FUNC(boolean, NVM_CODE) nvm_fsm_event(nvm_event_type nvm_queries_at)
{
	return nvm_event_table[nvm_queries_at]();
}

static FUNC(void, NVM_CODE) nvm_fsm_action(nvm_state_action_type nvm_actions_at)
{
	nvm_action_table[nvm_actions_at]();
}

FUNC(void, NVM_CODE) nvm_fsm_init(void)
{
	nvm_current_block_info.internal_flags = 0u;

	nvm_current_job.job_service_id = NVM_INT_FID_NO_JOB_PENDING;
	nvm_current_job.job_block_id = 0u;
	nvm_current_job.ram_addr = NULL;
	nvm_current_block_info.retry_times = 0u;

	nvm_task_state = NVM_FSM_IDLE;
	nvm_subtask_state = NVM_FSM_FSM_FINISHED;
}

FUNC(void, NVM_CODE) nvm_fsm_deinit(void)
{
	(void)memset(&nvm_current_job, 0, sizeof(nvm_current_job));
	(void)memset(&nvm_current_block_info, 0, sizeof(nvm_current_block_info));
	nvm_task_state = NVM_FSM_UNINIT;
	nvm_subtask_state = NVM_FSM_UNINIT;
}

FUNC(nvm_fsm_state_type, NVM_CODE) nvm_fsm_excute(nvm_fsm_state_type nvm_current_state)
{
	nvm_fsm_state_type nvm_ret_state_tloc;
	nvm_state_action_type change_actions_ptloc = NVM_ACT_Nop;
	const nvm_state_descr_type *current_state_ptloc = &nvm_state_descr_table_at[nvm_current_state];

	if (nvm_fsm_event(current_state_ptloc->change_conditions.queries)) {
		nvm_ret_state_tloc = current_state_ptloc->change_conditions.next_state;
		change_actions_ptloc = current_state_ptloc->change_conditions.actions;
	} else {
		nvm_ret_state_tloc = current_state_ptloc->final_condition.next_state;
		change_actions_ptloc = current_state_ptloc->final_condition.actions;
	}

	nvm_fsm_action(change_actions_ptloc);
	return nvm_ret_state_tloc;
}

#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
