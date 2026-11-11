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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nvm_queue.h"
#include "nvm_fsm.h"
#include "nvm_internal.h"
#include "SchM_NvM.h"
#include "Std_Types.h"

#define NVM_START_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

#if (NVM_JOB_PRIORIZATION == STD_ON)
VAR(nvm_queue_t, NVM_PRIVATE_BSS) nvm_high_job_queue;
VAR(static nvm_queue_data_t, NVM_PRIVATE_BSS) nvm_high_job_queue_data[NVM_SIZE_IMMEDIATE_JOB_QUEUE];
#endif

VAR(nvm_queue_t, NVM_PRIVATE_BSS) nvm_job_queue;
VAR(static nvm_queue_data_t, NVM_PRIVATE_BSS) nvm_job_queue_data[NVM_SIZE_STANDARD_JOB_QUEUE];

#define NVM_STOP_SEC_PRIVATE_BSS
#include "nvm_memmap.h"

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"

static FUNC(void, NVM_CODE) NvM_QueueElementForward(nvm_queue_t *queue_ptr, nvm_queue_data_t **pelem)
{
	(*pelem) = (nvm_queue_data_t *)((char *)(*pelem) + queue_ptr->data_size);
	if ((*pelem) == queue_ptr->buf_end) {
		(*pelem) = queue_ptr->buf_start;
	}
}

FUNC(void, NVM_CODE) nvm_queue_deinit(void)
{
#if (NVM_JOB_PRIORIZATION == STD_ON)
	(void)memset(&nvm_high_job_queue, 0, sizeof(nvm_high_job_queue));
#endif
	(void)memset(&nvm_job_queue, 0, sizeof(nvm_job_queue));
}

FUNC(void, NVM_CODE) nvm_queue_init(void)
{
	nvm_queue_t *queue_buffer = NULL;
	uint16 max_cnt = 0;
	uint16 data_size = (uint16)(sizeof(nvm_queue_data_t));

#if (NVM_JOB_PRIORIZATION == STD_ON)
	queue_buffer = &nvm_high_job_queue;
	max_cnt = sizeof(nvm_high_job_queue_data) / sizeof(nvm_queue_data_t);

	queue_buffer->buf_start = nvm_high_job_queue_data;
	queue_buffer->max_cnt = max_cnt;
	queue_buffer->buf_end = (nvm_queue_data_t *)((char *)(queue_buffer->buf_start) + (data_size * max_cnt));
	queue_buffer->head = queue_buffer->buf_start;
	queue_buffer->tail = queue_buffer->buf_start;
	queue_buffer->data_size = data_size;
	queue_buffer->curr_cnt = 0;
#endif
	queue_buffer = &nvm_job_queue;
	max_cnt = sizeof(nvm_job_queue_data) / sizeof(nvm_queue_data_t);

	queue_buffer->buf_start = nvm_job_queue_data;
	queue_buffer->max_cnt = max_cnt;
	queue_buffer->buf_end = (nvm_queue_data_t *)((char *)(queue_buffer->buf_start) + (data_size * max_cnt));
	queue_buffer->head = queue_buffer->buf_start;
	queue_buffer->tail = queue_buffer->buf_start;
	queue_buffer->data_size = data_size;
	queue_buffer->curr_cnt = 0;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_queue_push(const nvm_queue_data_t *data_ptr)
{
	nvm_queue_t *queue_ptr = NULL;

#if (NVM_JOB_PRIORIZATION == STD_ON)
	if ((data_ptr->job_prio == 0u) && (data_ptr->service_id == NVM_INT_FID_WRITE_BLOCK)) {
		queue_ptr = &nvm_high_job_queue;
	} else {
		queue_ptr = &nvm_job_queue;
	}
#else
	queue_ptr = &nvm_job_queue;
#endif

	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	if (queue_ptr->curr_cnt == queue_ptr->max_cnt) {
		SchM_Exit_NVM_EXCLUSIVE_AREA_0();
		return E_NOT_OK; /* No more room */
	}

	(void)memcpy(queue_ptr->head, data_ptr, queue_ptr->data_size);

	NvM_QueueElementForward(queue_ptr, &(queue_ptr->head));

	++queue_ptr->curr_cnt;
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();
	return E_OK;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_queue_pop(nvm_queue_t *queue_ptr, nvm_queue_data_t *data_ptr)
{
	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	if ((queue_ptr == NULL) || (queue_ptr->curr_cnt == 0U)) {
		SchM_Exit_NVM_EXCLUSIVE_AREA_0();
		return E_NOT_OK;
	}

	(void)memcpy(data_ptr, queue_ptr->tail, queue_ptr->data_size);
	queue_ptr->tail = (nvm_queue_data_t *)((char *)queue_ptr->tail + queue_ptr->data_size);
	if (queue_ptr->tail == queue_ptr->buf_end) {
		queue_ptr->tail = queue_ptr->buf_start;
	}
	--queue_ptr->curr_cnt;
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();
	return E_OK;
}

FUNC(Std_ReturnType, NVM_CODE) nvm_invalidate_queue_job(nvm_block_id_t block_id)
{
	Std_ReturnType return_value = E_NOT_OK;
	nvm_queue_t *queue_ptr = &nvm_job_queue;
	uint32 counts = queue_ptr->curr_cnt;
	nvm_queue_data_t *elem = queue_ptr->tail;

	SchM_Enter_NVM_EXCLUSIVE_AREA_0();
	while (counts > 0U) {
		if (elem->block_id == block_id) {
			elem->service_id = NVM_INT_FID_INVALID_JOB;
			return_value = E_OK;
		}
		elem = (nvm_queue_data_t *)((char *)elem + queue_ptr->data_size);
		if (elem == queue_ptr->buf_end) {
			elem = queue_ptr->buf_start;
		}
		counts--;
	}
	SchM_Exit_NVM_EXCLUSIVE_AREA_0();
#if (NVM_JOB_PRIORIZATION == STD_ON)
	if (nvm_block_descriptor_list[block_id].block_job_priority == 0u) {
		queue_ptr = &nvm_high_job_queue;
		counts = queue_ptr->curr_cnt;
		elem = queue_ptr->tail;
		SchM_Enter_NVM_EXCLUSIVE_AREA_0();
		while (counts > 0U) {
			if (elem->block_id == block_id) {
				elem->service_id = NVM_INT_FID_INVALID_JOB;
				return_value = E_OK;
			}
			elem = (nvm_queue_data_t *)((char *)elem + queue_ptr->data_size);
			if (elem == queue_ptr->buf_end) {
				elem = queue_ptr->buf_start;
			}
			counts--;
		}
		SchM_Exit_NVM_EXCLUSIVE_AREA_0();
	}
#endif
	return return_value;
}

#if (NVM_JOB_PRIORIZATION == STD_ON)

FUNC(Std_ReturnType, NVM_CODE) nvm_get_high_prio_queue_job(nvm_queue_t *queue_ptr, nvm_queue_data_t *data_ptr)
{
	if ((queue_ptr == NULL) || (queue_ptr->curr_cnt == 0U)) {
		return E_NOT_OK;
	}

	uint8 high_prio = 255;
	uint16 high_index = 0;
	uint16 counts = queue_ptr->curr_cnt;
	nvm_queue_data_t *elem = queue_ptr->tail;

	while (counts > 0U) {
		if (elem->job_prio < high_prio) {
			high_prio = elem->job_prio;
			high_index = counts;
		}
		NvM_QueueElementForward(queue_ptr, &elem);
		counts--;
	}

	elem = queue_ptr->tail;
	counts = queue_ptr->curr_cnt;
	while (counts > 0U) {
		if (elem->job_prio == high_prio) {
			(void)memcpy(data_ptr, elem, queue_ptr->data_size);
			--queue_ptr->curr_cnt;
			nvm_queue_data_t *temp = elem;

			for (uint16 index = 0; index < high_index; index++) {
				elem = temp;
				NvM_QueueElementForward(queue_ptr, &temp);
				(void)memcpy(elem, temp, queue_ptr->data_size);
			}
			queue_ptr->head = elem;
			break;
		}
		NvM_QueueElementForward(queue_ptr, &elem);
		counts--;
	}

	return E_OK;
}

#endif

FUNC(boolean, NVM_CODE) nvm_event_high_prio_job(void)
{
#if (NVM_JOB_PRIORIZATION == STD_ON)
	if (nvm_high_job_queue.curr_cnt > 0) {
		return TRUE;
	}
#endif
	return FALSE;
}

FUNC(boolean, NVM_CODE) nvm_event_normal_prio_job(void)
{
	if (nvm_job_queue.curr_cnt == 0u) {
		return FALSE;
	}

	return TRUE;
}

#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
