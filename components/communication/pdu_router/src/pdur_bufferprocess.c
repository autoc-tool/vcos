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

/* including file */
#include "pdur.h"
#include "pdur_internal.h"
#include "pdur_cfg.h"
#include "string.h"
#include "pdur_buffermanager.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_buffer_and_transfer_copy_data_into_ring_buffer(pdur_buffer_manager_type *buffer_manager_ptr,
						    const PduInfoType *info, PduLengthType *buffer_size_ptr)
{
	(void)memcpy(&buffer_manager_ptr->buf_addr[buffer_manager_ptr->buffer_write_offset], info->SduDataPtr,
		     info->SduLength);
	buffer_manager_ptr->buffer_write_offset += info->SduLength;
	if (buffer_size_ptr != NULL) {
		*buffer_size_ptr = buffer_manager_ptr->buf_length - buffer_manager_ptr->buffer_write_offset;
	}
	return BUFREQ_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE)
pdur_buffer_and_transfer_check_send_data_enough(const pdur_buffer_manager_type *buffer_manager_ptr,
						PduLengthType send_len, uint8 dest_pdu_idx)
{
	boolean ret_val = (boolean)TRUE;

	if (buffer_manager_ptr->buf_addr == NULL) {
		return (boolean)FALSE;
	}
	PduLengthType write_offset = buffer_manager_ptr->buffer_write_offset;
	PduLengthType read_offset = buffer_manager_ptr->buffer_read_offset[dest_pdu_idx];

	if (write_offset < read_offset) {
		return (boolean)FALSE;
	} else if ((write_offset - read_offset) < send_len) {
		return (boolean)FALSE;
	} else if (read_offset > buffer_manager_ptr->buf_length) {
		return (boolean)FALSE;
	} else if (send_len > buffer_manager_ptr->buf_length) {
		return (boolean)FALSE;
	} else {
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_buffer_and_transfer_copy_data_from_ring_buffer(pdur_buffer_manager_type *buffer_manager_ptr,
						    const PduInfoType *info, uint8 dest_pdu_idx,
						    PduLengthType *available_data_ptr, const RetryInfoType *retry)
{
	PduLengthType read_offset = buffer_manager_ptr->buffer_read_offset[dest_pdu_idx];

	if ((retry != NULL) && (retry->TpDataState == TP_DATARETRY)) {
		read_offset = retry->TxTpDataCnt;
	}
	const uint8 *src_ptr = &buffer_manager_ptr->buf_addr[read_offset];
	(void)memcpy(info->SduDataPtr, src_ptr, info->SduLength);
	buffer_manager_ptr->buffer_read_offset[dest_pdu_idx] = read_offset + info->SduLength;
	if (available_data_ptr != NULL) {
		*available_data_ptr = (buffer_manager_ptr->buffer_write_offset -
				       buffer_manager_ptr->buffer_read_offset[dest_pdu_idx]);
	}
	return BUFREQ_OK;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
