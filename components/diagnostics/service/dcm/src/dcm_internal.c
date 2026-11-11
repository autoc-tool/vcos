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
#include "dcm.h"
#include "dcm_internal.h"
#if (CONFIG_MODULE_RTE == 1)
#include "Rte_Dcm.h"
#endif
#if (CONFIG_MODULE_NVM == 1)
#include "nvm.h"
#endif

#define IS_SIGNED_DATATYPE(_x) (((_x) == DCM_SINT8) || ((_x) == DCM_SINT16) || ((_x) == DCM_SINT32))

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

#if (DCM_VSG_CHECK_ENABLED == STD_ON)
FUNC(uint8, DCM_CODE) dcm_internal_get_vsg_index_by_vsg_id_map(dcm_vsg_id_type vsg_id)
{
	uint8 vsg_index = 0;
	const dcm_vsg_id_type *vsg_id_map = dcm_config_ptr->dsp->dsp_vehicle_system_group->vsg_id_map;

	for (vsg_index = 0; vsg_index < DCM_NOF_VEHICLE_SYSTEM_GROUPS; vsg_index++) {
		if (vsg_id == vsg_id_map[vsg_index]) {
			break;
		}
	}
	return vsg_index;
}

static FUNC(boolean, DCM_CODE) dcm_internal_check_vsg(const uint8 vsg_ref_size, const dcm_vsg_id_type *vsg_ref)
{
	boolean is_pass = FALSE;
	const dcm_vsg_state_type *vsg_enable_state = dcm_config_ptr->dsp->dsp_vehicle_system_group->vsg_enable_state;
	uint8 vsg_state_index = 0;

	for (uint8 i = 0; i < vsg_ref_size; i++) {
		vsg_state_index = dcm_internal_get_vsg_index_by_vsg_id_map(vsg_ref[i]);
		if (vsg_state_index < DCM_NOF_VEHICLE_SYSTEM_GROUPS) {
			if (DCM_VSG_ENABLED == vsg_enable_state[vsg_state_index]) {
				is_pass = TRUE;
				break;
			}
		}
	}

	return is_pass;
}
#endif

FUNC(boolean, DCM_CODE)
do_check_did_read_condition(const dcm_dsp_did_read_type *did_read, dcm_nrc_type *response_code)
{
#if (DCM_VSG_CHECK_ENABLED == STD_ON)
	if (did_read->dsp_did_read_vsg_ref_size > 0u) {
		if (FALSE ==
			dcm_internal_check_vsg(did_read->dsp_did_read_vsg_ref_size, did_read->dsp_did_read_vsg_ref)) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
			return FALSE;
		}
	}
#endif

	if (FALSE == dsp_check_session_level(did_read->dsp_did_read_session_mask)) {
		*response_code = DCM_E_REQUESTOUTOFRANGE;
		return FALSE;
	}

	if (FALSE == dsp_check_security_level(did_read->dsp_did_read_security_mask)) {
		*response_code = DCM_E_SECURITYACCESSDENIED;
		return FALSE;
	}

	return TRUE;
}

FUNC(boolean, DCM_CODE)
do_check_did_write_condition(const dcm_dsp_did_write_type *did_write, dcm_nrc_type *response_code)
{
#if (DCM_VSG_CHECK_ENABLED == STD_ON)
	if (did_write->dsp_did_write_vsg_ref_size > 0u) {
		if (FALSE ==
			dcm_internal_check_vsg(did_write->dsp_did_write_vsg_ref_size, did_write->dsp_did_write_vsg_ref)) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
			return FALSE;
		}
	}
#endif

	if (FALSE == dsp_check_session_level(did_write->dsp_did_write_session_mask)) {
		*response_code = DCM_E_REQUESTOUTOFRANGE;
		return FALSE;
	}

	if (FALSE == dsp_check_security_level(did_write->dsp_did_write_security_mask)) {
		*response_code = DCM_E_SECURITYACCESSDENIED;
		return FALSE;
	}

	return TRUE;
}

FUNC(boolean, DCM_CODE)
do_check_did_control_condition(const dcm_dsp_did_control_type *did_control, dcm_nrc_type *response_code)
{
#if (DCM_VSG_CHECK_ENABLED == STD_ON)
	if (did_control->dsp_did_control_vsg_ref_size > 0u) {
		if (FALSE == dcm_internal_check_vsg(did_control->dsp_did_control_vsg_ref_size,
				did_control->dsp_did_control_vsg_ref)) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
			return FALSE;
		}
	}
#endif
	if (FALSE == dsp_check_session_level(did_control->dsp_did_control_session_mask)) {
		*response_code = DCM_E_REQUESTOUTOFRANGE;
		return FALSE;
	}

	if (FALSE == dsp_check_security_level(did_control->dsp_did_control_security_mask)) {
		*response_code = DCM_E_SECURITYACCESSDENIED;
		return FALSE;
	}

	return TRUE;
}

FUNC(boolean, DCM_CODE)
do_check_routine_control_condition(const dcm_dsp_routine_authorization_type routine,
		dcm_nrc_type *response_code)
{
#if (DCM_VSG_CHECK_ENABLED == STD_ON)
	if (routine.dsp_routine_vsg_ref_size > 0u) {
		if (FALSE == dcm_internal_check_vsg(routine.dsp_routine_vsg_ref_size, routine.dsp_routine_vsg_ref)) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
			return FALSE;
		}
	}
#endif

	if (FALSE == dsp_check_session_level(routine.dsp_routine_session_mask)) {
		*response_code = DCM_E_REQUESTOUTOFRANGE;
		return FALSE;
	}

	if (FALSE == dsp_check_security_level(routine.dsp_routine_security_mask)) {
		*response_code = DCM_E_SECURITYACCESSDENIED;
		return FALSE;
	}

	return TRUE;
}

FUNC(boolean, DCM_CODE) lookup_non_dynamic_did(uint16 did_nr, const dcm_dsp_did_type **did_ptr)
{
	const dcm_dsp_did_type *dsp_did = dcm_config_ptr->dsp->dsp_did;
	boolean did_found = FALSE;

	for (uint16 index = 0; index < DCM_DID_LIST_EOL_INDEX; index++) {
		if (dsp_did[index].dsp_did_identifier == did_nr) {
			*did_ptr = &dsp_did[index];
			did_found = TRUE;
			break;
		}
	}
	return did_found;
}

#if (defined(DCM_DSP_DID_RANGE_ENABLED) && (DCM_DSP_DID_RANGE_ENABLED == STD_ON))
static FUNC(boolean, DCM_CODE) lookup_did_range(uint16 did_nr, const dcm_dsp_did_range_type **did_range_ptr)
{
	const dcm_dsp_did_range_type *dsp_did_range = dcm_config_ptr->dsp->dsp_did_range;
	boolean did_found = FALSE;
	boolean is_did_available = FALSE;
	dcm_did_supt_type supported = FALSE;
	Std_ReturnType ret = E_NOT_OK;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	while (FALSE == dsp_did_range->dcm_eol) {
		if (dsp_did_range->did_range_is_did_available != NULL) {
			ret = dsp_did_range->did_range_is_did_available(did_nr, op_status, &supported);
			if (ret == E_OK) {
				if (supported == DCM_DID_SUPPORTED) {
					is_did_available = TRUE;
				}
			}
		} else {
			is_did_available = TRUE;
		}
		if ((dsp_did_range->dsp_did_range_upper_limit >= did_nr) &&
		    (dsp_did_range->dsp_did_range_lower_limit <= did_nr) && (is_did_available == TRUE)) {
			break;
		}
		dsp_did_range++;
	}

	if (FALSE == dsp_did_range->dcm_eol) {
		did_found = TRUE;
		*did_range_ptr = dsp_did_range;
	}

	return did_found;
}

static FUNC(void, DCM_CODE) do_check_in_pend_state_and_include_did_range(uint16 did, uint16 datalength,
	const PduInfoType *pdu_tx_data, uint16 *tx_pos, uint16 *did_data_start_pos, dcm_op_status_type *op_status,
	boolean in_pending_state, boolean include_did)
{
	if ((boolean)TRUE == in_pending_state) {
		*op_status = DCM_PENDING;
		return;
	}
	if ((boolean)TRUE == include_did) {
		pdu_tx_data->SduDataPtr[*tx_pos] = (uint8)((did >> 8u) & 0xffu);
		(*tx_pos)++;
		pdu_tx_data->SduDataPtr[*tx_pos] = (uint8)(did & 0xffu);
		(*tx_pos)++;
	}
	*did_data_start_pos = *tx_pos;
	(void)memset(&pdu_tx_data->SduDataPtr[*tx_pos], 0, datalength);
}

static FUNC(dcm_nrc_type, DCM_CODE)
	get_did_range_data(uint16 did, const dcm_dsp_did_range_type *did_range_ptr, const PduInfoType *pdu_tx_data,
		dsp_uds_read_did_pending_type *read_did_pending, boolean include_did)
{
	dcm_nrc_type error_code = DCM_E_POSITIVERESPONSE;
	dcm_op_status_type op_status = DCM_INITIAL;
	Std_ReturnType result = E_OK;
	uint16 required_buf_size;
	uint16 datalength;
	boolean in_pending_state = (DCM_READ_DID_PENDING_COND_CHECK == read_did_pending->state) ||
		(DCM_READ_DID_PENDING_READ_DATA == read_did_pending->state);

	if (did_range_ptr->read_did_range_data_length_fnc == NULL) {
		error_code = DCM_E_CONDITIONSNOTCORRECT;
		return error_code;
	}

	op_status = ((boolean)TRUE == in_pending_state) ? DCM_PENDING : DCM_INITIAL;
	if (DCM_READ_DID_IDLE == read_did_pending->state) {
		if (E_OK != did_range_ptr->read_did_range_data_length_fnc(did, op_status, &datalength)) {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
			result = E_NOT_OK;
		}
	} else {
		datalength = read_did_pending->pending_data_length;
	}

	if (E_OK != result) {
		error_code = DCM_E_CONDITIONSNOTCORRECT;
		return error_code;
	}
	required_buf_size = read_did_pending->tx_write_pos + datalength;

	if ((TRUE == include_did) && (FALSE == in_pending_state)) {
		required_buf_size += 2u;
	}
	if ((TRUE != in_pending_state) && (required_buf_size > pdu_tx_data->SduLength)) {
		error_code = DCM_E_RESPONSETOOLONG;
		return error_code;
	}

	do_check_in_pend_state_and_include_did_range(did, datalength, pdu_tx_data,
		&read_did_pending->tx_write_pos, &read_did_pending->pending_data_start_pos, &op_status,
		in_pending_state, include_did);

	result = (NULL_PTR != did_range_ptr->did_range_read_data_fnc) ? did_range_ptr->did_range_read_data_fnc(did,
		&pdu_tx_data->SduDataPtr[(read_did_pending->pending_data_start_pos)], op_status,
		&datalength, &error_code) : E_NOT_OK;

	switch (result) {
		case DCM_E_PENDING:
			read_did_pending->state = DCM_READ_DID_PENDING_READ_DATA;
			read_did_pending->pending_data_length = datalength;
			error_code = DCM_E_RESPONSEPENDING;
			break;
		case E_OK:
			read_did_pending->state = DCM_READ_DID_IDLE;
			if (read_did_pending->tx_write_pos <
				((read_did_pending->pending_data_start_pos) + datalength)) {
				read_did_pending->tx_write_pos = ((read_did_pending->pending_data_start_pos) + datalength);
			}
			break;
		default:
			error_code = DCM_E_CONDITIONSNOTCORRECT;
			break;
	}

	return error_code;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	read_did_range_data(uint16 did, uint16 did_start_index, const dcm_dsp_did_range_type *did_range_ptr,
		PduInfoType *pdu_tx_data, dsp_uds_read_did_pending_type *read_did_pending)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (did_range_ptr->dsp_did_range_info_ref->dsp_did_access.dsp_did_read != NULL_PTR) {
		if (do_check_did_read_condition(did_range_ptr->dsp_did_range_info_ref->dsp_did_access.dsp_did_read,
						&response_code)) {
			if (read_did_pending->nof_read_dids >= did_start_index) {
				response_code =
					get_did_range_data(did, did_range_ptr, pdu_tx_data,
								&read_did_pending->tx_write_pos, &read_did_pending->state,
								&read_did_pending->pending_data_length,
								&read_did_pending->pending_data_start_pos, TRUE);
				switch (response_code) {
					case DCM_E_RESPONSEPENDING:
						read_did_pending->pending_did = did;
						break;
					case DCM_E_POSITIVERESPONSE:
						read_did_pending->nof_read_dids++;
						break;
					default:
						break;
				}
			}
		}
	} else {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}
	return response_code;
}
#endif

FUNC(dcm_nrc_type, DCM_CODE) read_did_data(uint16 did_start_index, const dcm_dsp_did_type *did_ptr,
	PduInfoType *pdu_tx_data, dsp_uds_read_did_pending_type *read_did_pending)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (did_ptr->dsp_did_info_ref->dsp_did_access.dsp_did_read == NULL_PTR) {
		return DCM_E_REQUESTOUTOFRANGE;
	}
	if (!do_check_did_read_condition(did_ptr->dsp_did_info_ref->dsp_did_access.dsp_did_read,
		&response_code)) {
			return DCM_E_POSITIVERESPONSE;
		}
	if ((did_ptr->dsp_signal_ref == NULL_PTR) || (read_did_pending->nof_read_dids < did_start_index)) {
			return DCM_E_POSITIVERESPONSE;
	}

	response_code = get_did_data(did_ptr, pdu_tx_data, read_did_pending, TRUE);
	switch (response_code) {
		case DCM_E_RESPONSEPENDING:
			read_did_pending->pending_did = did_ptr->dsp_did_identifier;
			break;
		case DCM_E_POSITIVERESPONSE:
			read_did_pending->nof_read_dids++;
			read_did_pending->pending_signal_index = 0;
			break;
		default:
			break;
	}

	return response_code;
}

#ifdef DCM_SUPPORT_DATA_PORT_SR
static FUNC(Std_ReturnType, DCM_CODE) write_sr_data_to_port(const dcm_dsp_data_type *data_ptr, uint32 data_val)
{
	Std_ReturnType ret = E_NOT_OK;
#if (DCM_DSP_DATA_WRITE_DATA_FNC_ENABLED == STD_ON)
	const dcm_data_type dsp_data_type = data_ptr->dsp_data_type;
	dcm_callback_write_data_fnc_type *dsp_data_write_data_fnc = &(data_ptr->dsp_data_write_data_fnc);

	if (DCM_SINT8 == dsp_data_type) {
		ret = dsp_data_write_data_fnc->sr_data_write_fnc_sint8((sint8)data_val);
	} else if (DCM_SINT16 == dsp_data_type) {
		ret = dsp_data_write_data_fnc->sr_data_write_fnc_sint16((sint16)data_val);
	} else if (DCM_SINT32 == dsp_data_type) {
		ret = dsp_data_write_data_fnc->sr_data_write_fnc_sint32((sint32)data_val);
	} else if (DCM_BOOLEAN == dsp_data_type) {
		ret = dsp_data_write_data_fnc->sr_data_write_fnc_boolean((boolean)data_val);
	} else if (DCM_UINT32 == dsp_data_type) {
		ret = dsp_data_write_data_fnc->sr_data_write_fnc_uint32(data_val);
	} else if (DCM_UINT16 == dsp_data_type) {
		ret = dsp_data_write_data_fnc->sr_data_write_fnc_uint16((uint16)data_val);
	} else {
		ret = (DCM_UINT8 == dsp_data_type)
			? dsp_data_write_data_fnc->sr_data_write_fnc_uint8((uint8)data_val) : E_NOT_OK;
	}
#else
	(void)data_ptr;
	(void)data_val;
#endif
	return ret;
}

FUNC(Std_ReturnType, DCM_CODE)
write_sr_signal(const dcm_dsp_data_type *data_ptr, uint16 bit_position, const uint8 *data_buffer)
{
	Std_ReturnType ret = E_OK;
	uint32 pdu_data = 0UL;
	const uint16 bit_size = data_ptr->dsp_data_bit_size;

	switch (data_ptr->dsp_data_endianess) {
		case DCM_BIG_ENDIAN:
			uint16 lsb_index =
				(((bit_position ^ (uint16)0x7) + bit_size - (uint16)1) ^ (uint16)7);
			const uint8 *pdu_data_ptr = ((&data_buffer[lsb_index / 8u]) - 3);
			uint8 bit_shift = (uint8)(lsb_index % 8u);
			uint32 i = 0u;

			while (i < 4u) {
				pdu_data = (pdu_data << 8u) | pdu_data_ptr[i];
				i++;
			}
			pdu_data >>= bit_shift;
			if (((uint16)32u - (uint16)bit_shift) < bit_size) {
				pdu_data |= (uint32)pdu_data_ptr[-1] << (32u - bit_shift);
			}
			break;
		case DCM_LITTLE_ENDIAN:
			uint16 lsb_index = (uint16)bit_position;
			const uint8 *pdu_data_ptr = &data_buffer[(bit_position / 8u)];
			uint8 bit_shift = (uint8)(lsb_index % 8u);
			sint32 i = 3;

			while (i >= 0) {
				pdu_data = (uint32)((pdu_data << 8u) | pdu_data_ptr[i]);
				i--;
			}
			pdu_data >>= bit_shift;
			if ((uint16)(32u - bit_shift) < bit_size) {
				pdu_data |= (uint32)pdu_data_ptr[4] << (32u - bit_shift);
			}
			break;
		default:
			ret = E_NOT_OK;
			break;
	}

	if (E_OK != ret) {
		return ret;
	}

	uint32 mask_value = 0xfffffffful >> (32u - bit_size);

	pdu_data &= mask_value;
	uint32 signmask = ~(mask_value >> 1u);

	if (IS_SIGNED_DATATYPE(data_ptr->dsp_data_type)) {
		if (0u < (pdu_data & signmask)) {
			pdu_data |= signmask;
		}
	}

	ret = write_sr_data_to_port(data_ptr, pdu_data);

	return ret;
}

static FUNC(uint32, DCM_CODE) read_sr_signal_data_value(dcm_data_type data_type, uint8 *temp_data)
{
	uint32 data_val = 0ul;

	if ((DCM_BOOLEAN == data_type) || (DCM_UINT8 == data_type) || (DCM_SINT8 == data_type)) {
		data_val = *(temp_data);
	} else if ((DCM_UINT16 == data_type) || (DCM_SINT16 == data_type)) {
		data_val = *((const uint16 *)temp_data);
	} else if ((DCM_UINT32 == data_type) || (DCM_SINT32 == data_type)) {
		data_val = *((const uint32 *)temp_data);
	} else {
		/* do nothing */
	}

	return data_val;
}

FUNC(Std_ReturnType, DCM_CODE)
read_sr_signal(const dcm_dsp_data_type *data_ptr, uint16 bit_position, uint8 *data_buffer)
{
	Std_ReturnType ret = E_NOT_OK;

#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON)
	uint8 temp_data[4];
	uint32 data_val = 0u;

	ret = data_ptr->dsp_data_read_data_fnc.sr_data_read_fnc((void *)temp_data);
	uint32 mask_value = 0xfffffffful >> (32u - data_ptr->dsp_data_bit_size);

	data_val = read_sr_signal_data_value(data_ptr->dsp_data_type, temp_data);
	data_val &= mask_value;

	switch (data_ptr->dsp_data_endianess) {
		case DCM_BIG_ENDIAN:
			uint16 lsb_index =
				(((bit_position ^ (uint16)0x7) + data_ptr->dsp_data_bit_size - (uint16)1) ^ (uint16)7);
			uint8 *pdu_data_ptr = ((&data_buffer[lsb_index / 8u]) - 3u);
			uint32 pdu_data = 0u;
			uint32 i = 0u;

			while (i < 4u) {
				pdu_data = ((pdu_data << 8) | pdu_data_ptr[i]);
				i++;
			}
			uint8 bit_shift = (uint8)(lsb_index % 8u);
			uint32 mask_lo = ~(mask_value << bit_shift);

			uint32 new_pdu_data = (pdu_data & mask_lo) | (data_val << bit_shift);
			sint16 j = 3;

			while (j >= 0) {
				pdu_data_ptr[j] = (uint8)new_pdu_data;
				new_pdu_data >>= 8;
				j--;
			}
			uint8 max_bits_written = 32u - bit_shift;

			if (max_bits_written < data_ptr->dsp_data_bit_size) {
				pdu_data_ptr--;
				pdu_data = *pdu_data_ptr;
				uint32 mask_hi = ~(mask_value >> max_bits_written);
				uint32 sig_hi = data_val >> max_bits_written;

				new_pdu_data = (pdu_data & mask_hi) | sig_hi;
				*pdu_data_ptr = (uint8)new_pdu_data;
			}
			break;
		case DCM_LITTLE_ENDIAN:
			uint16 lsb_index = bit_position;
			uint8 *pdu_data_ptr = (&data_buffer[lsb_index / 8u]);
			uint32 pdu_data = 0u;
			sint32 i = 3;

			while (i >= 0) {
				pdu_data = (pdu_data << 8u) | pdu_data_ptr[i];
				i--;
			}
			uint8 bit_shift = (uint8)(lsb_index % 8u);
			uint32 mask_lo = ~(mask_value << bit_shift);

			uint32 new_pdu_data = (pdu_data & mask_lo) | (data_val << bit_shift);
			uint32 j = 0u;

			while (j < 4u) {
				pdu_data_ptr[j] = (uint8)new_pdu_data;
				new_pdu_data >>= 8;
				j++;
			}
			uint8 max_bits_written = 32u - bit_shift;

			if (max_bits_written < data_ptr->dsp_data_bit_size) {
				pdu_data_ptr = &pdu_data_ptr[4];
				pdu_data = *pdu_data_ptr;
				uint32 mask_hi = ~(mask_value >> max_bits_written);

				*pdu_data_ptr = (uint8)((pdu_data & mask_hi) | (data_val >> max_bits_written));
			}
			break;
		default:
			ret = E_NOT_OK;
			break;
	}
#else
	(void)data_ptr;
	(void)bit_position;
	(void)data_buffer;
#endif
	return ret;
}
#endif

FUNC(uint16, DCM_CODE)
get_nof_affected_bytes(dcm_dsp_data_endianess_type endianess, uint16 start_bit_pos, uint16 bit_length)
{
	uint16 affected_bytes = 1u;
	uint8 bits_in_first_byte;

	bits_in_first_byte = (DCM_BIG_ENDIAN == endianess)
		? (uint8)((start_bit_pos % 8u) + 1u) : (uint8)(8u - (start_bit_pos % 8u));

	if (bits_in_first_byte <= bit_length) {
		affected_bytes += (uint16)(((bit_length - bits_in_first_byte) + 7u) / 8u);
	}
	return affected_bytes;
}

static FUNC(Std_ReturnType, DCM_CODE) do_check_dsp_data_condition_check_read(const dcm_dsp_data_type *data_ptr,
	read_did_pending_state_type *pending_state, dcm_op_status_type *op_status, dcm_nrc_type *error_code)
{
	Std_ReturnType result = E_OK;
#if (DCM_DSP_DATA_CONDITION_CHECK_READ_FNC_ENABLED == STD_ON)
	const dcm_callback_condition_check_read_fnc_type *dsp_data_condition_check_read_fnc =
		&(data_ptr->dsp_data_condition_check_read_fnc);
#endif

	if (DATA_PORT_SYNCH == data_ptr->dsp_data_use_port) {
#if (DCM_DSP_DATA_CONDITION_CHECK_READ_FNC_ENABLED == STD_ON)
		result = (NULL_PTR == dsp_data_condition_check_read_fnc->synch_condition_check_read_fnc)
			? E_OK : dsp_data_condition_check_read_fnc->synch_condition_check_read_fnc(error_code);
#else
		result = E_OK;
#endif
		if (DCM_E_PENDING == result) {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
			result = E_NOT_OK;
		}
	} else if (DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port) {
#if (DCM_DSP_DATA_CONDITION_CHECK_READ_FNC_ENABLED == STD_ON)
		result = (NULL_PTR == dsp_data_condition_check_read_fnc->asynch_condition_check_read_fnc) ? E_OK
		: dsp_data_condition_check_read_fnc->asynch_condition_check_read_fnc(*op_status, error_code);
#else
		result = E_OK;
#endif
		*pending_state = (DCM_E_PENDING == result) ? DCM_READ_DID_PENDING_COND_CHECK : DCM_READ_DID_IDLE;
		if (DCM_E_PENDING != result) {
			*op_status = DCM_INITIAL;
		}
	}
#if (CONFIG_MODULE_NVM == 1) && defined(DCM_USE_NVM_DID)
	else if (DATA_PORT_BLOCK_ID == data_ptr->dsp_data_use_port) {
		result = E_OK;
		*error_code = DCM_E_POSITIVERESPONSE;
	}
#endif
	else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_CONFIG_INVALID);
		result = E_NOT_OK;
	}

	return result;
}

static FUNC(void, DCM_CODE) do_check_in_pend_state_and_include_did(const dcm_dsp_did_type *did_ptr,
	const PduInfoType *pdu_tx_data, uint16 *tx_pos, uint16 *did_data_start_pos, dcm_op_status_type *op_status,
	boolean in_pending_state, boolean include_did)
{
	if ((boolean)TRUE == in_pending_state) {
		*op_status = DCM_PENDING;
		return;
	}
	if ((boolean)TRUE == include_did) {
		pdu_tx_data->SduDataPtr[*tx_pos] = (uint8)((did_ptr->dsp_did_identifier >> 8u) & 0xffu);
		(*tx_pos)++;
		pdu_tx_data->SduDataPtr[*tx_pos] = (uint8)(did_ptr->dsp_did_identifier & 0xffu);
		(*tx_pos)++;
	}
	*did_data_start_pos = *tx_pos;
	(void)memset(&pdu_tx_data->SduDataPtr[*tx_pos], 0, did_ptr->dsp_did_data_byte_size);
}

static FUNC(Std_ReturnType, DCM_CODE) do_check_dsp_data_read_data_length
	(const dcm_dsp_signal_type *signal_ptr, uint16 *data_len)
{
	Std_ReturnType result = E_OK;
	const dcm_dsp_data_type *data_ptr = signal_ptr->dsp_signal_data_ref;

	if ((TRUE != data_ptr->dsp_did_fixed_length) && (DATA_PORT_SR != data_ptr->dsp_data_use_port)) {
		return result;
	}
	*data_len = get_nof_affected_bytes(data_ptr->dsp_data_endianess, signal_ptr->dsp_signal_bit_position,
		data_ptr->dsp_data_bit_size);

	return result;
}

static FUNC(Std_ReturnType, DCM_CODE) do_check_dsp_data_read_data_fnc(const PduInfoType *pdu_tx_data,
	const dcm_dsp_signal_type *signal_ptr, const uint16 *did_data_start_pos,
	read_did_pending_state_type *pending_state, dcm_op_status_type op_status)
{
	Std_ReturnType result = E_OK;
	const dcm_dsp_data_type *data_ptr = signal_ptr->dsp_signal_data_ref;
#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON)
	const dcm_callback_read_data_fnc_type *dsp_data_read_data_fnc = &(data_ptr->dsp_data_read_data_fnc);
#endif

#if ((DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON) || ((CONFIG_MODULE_NVM == 1) && defined(DCM_USE_NVM_DID)))
	uint16 signal_first_byte_pos = signal_ptr->dsp_signal_bit_position / 8u;
#endif

	if ((DATA_PORT_SYNCH == data_ptr->dsp_data_use_port) || (DATA_PORT_ECU_SIGNAL == data_ptr->dsp_data_use_port)) {
#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON)
		if (NULL_PTR == dsp_data_read_data_fnc->synch_data_read_fnc) {
			result = E_NOT_OK;
		} else if (E_OK != dsp_data_read_data_fnc->synch_data_read_fnc
			(&pdu_tx_data->SduDataPtr[(*did_data_start_pos) + signal_first_byte_pos])) {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
			result = E_NOT_OK;
		} else {
			/* do nothing */
		}
#else
		result = E_NOT_OK;
#endif
	} else if (DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port) {
#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON)
		result = (NULL_PTR == dsp_data_read_data_fnc->asynch_data_read_fnc) ? E_NOT_OK
			: dsp_data_read_data_fnc->asynch_data_read_fnc(op_status,
				&pdu_tx_data->SduDataPtr[(*did_data_start_pos) + signal_first_byte_pos]);
#else
		result = E_NOT_OK;
#endif
#if (CONFIG_MODULE_NVM == 1) && defined(DCM_USE_NVM_DID)
	} else if (DATA_PORT_BLOCK_ID == data_ptr->dsp_data_use_port) {
		if (*pending_state == DCM_READ_DID_PENDING_READ_DATA) {
			nvm_request_result_type error_status = NVM_REQ_NOT_OK;

			if (E_OK != nvm_get_error_status(data_ptr->dsp_nvm_use_block_id, &error_status)) {
				DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
				result = E_NOT_OK;
			} else {
				switch (error_status) {
					case NVM_REQ_PENDING:
						result = DCM_E_PENDING;
						break;
					case NVM_REQ_OK:
						*pending_state = DCM_READ_DID_IDLE;
						result = E_OK;
						break;
					default:
						DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
						result = E_NOT_OK;
						break;
				}
			}
		} else {
			nvm_request_result_type error_status;

			if (E_OK != nvm_get_error_status(data_ptr->dsp_nvm_use_block_id, &error_status)) {
				DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
				result = E_NOT_OK;
			}
			switch (error_status) {
				case NVM_REQ_PENDING:
					result = DCM_E_PENDING;
					break;
				case NVM_REQ_OK:
				case NVM_REQ_RESTORED_FROM_ROM:
					if (E_OK == nvm_read_block(data_ptr->dsp_nvm_use_block_id,
							&pdu_tx_data->SduDataPtr[(*did_data_start_pos) + signal_first_byte_pos])) {
						*pending_state = DCM_READ_DID_PENDING_READ_DATA;
						result = DCM_E_PENDING;
					} else {
						DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
						result = E_NOT_OK;
					}
					break;
				default:
					DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
					result = E_NOT_OK;
					break;
			}
		}
#endif
#ifdef DCM_SUPPORT_DATA_PORT_SR
	} else if (DATA_PORT_SR == data_ptr->dsp_data_use_port) {
		result = read_sr_signal(data_ptr, signal_ptr->dsp_signal_bit_position,
			&pdu_tx_data->SduDataPtr[(*did_data_start_pos)]);
#endif
	} else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_CONFIG_INVALID);
		result = E_NOT_OK;
	}

	return result;
}

static FUNC(dcm_nrc_type, DCM_CODE) do_check_dsp_data_read_data(uint16 signal_index, dcm_op_status_type op_status,
	const PduInfoType *pdu_tx_data, const dcm_dsp_signal_type *signal_ptr,
	dsp_uds_read_did_pending_type *read_did_pending)
{
	uint16 data_len = 0u;
	dcm_nrc_type error_code = DCM_E_POSITIVERESPONSE;

	Std_ReturnType result = do_check_dsp_data_read_data_length(signal_ptr, &data_len);

	if (E_OK != result) {
		error_code = DCM_E_CONDITIONSNOTCORRECT;
		return error_code;
	}

	uint16 signal_first_byte_pos = signal_ptr->dsp_signal_bit_position / 8u;

	if (((PduLengthType)read_did_pending->pending_data_start_pos + (PduLengthType)signal_first_byte_pos +
	(PduLengthType)data_len) > pdu_tx_data->SduLength) {
		error_code = DCM_E_REQUESTOUTOFRANGE;
		return error_code;
	}

	result = do_check_dsp_data_read_data_fnc(pdu_tx_data, signal_ptr,
		&read_did_pending->pending_data_start_pos,
		&read_did_pending->state, op_status);
	if (DCM_E_PENDING == result) {
		read_did_pending->state = DCM_READ_DID_PENDING_READ_DATA;
		read_did_pending->pending_data_length = data_len;
		read_did_pending->pending_signal_index = signal_index;
		error_code = DCM_E_RESPONSEPENDING;
		return error_code;
	}
	if (result != E_OK) {
		error_code = DCM_E_CONDITIONSNOTCORRECT;
		return error_code;
	}
	read_did_pending->state = DCM_READ_DID_IDLE;
	if (read_did_pending->tx_write_pos <
		((read_did_pending->pending_data_start_pos) + signal_first_byte_pos + data_len)) {
		read_did_pending->tx_write_pos = ((read_did_pending->pending_data_start_pos) +
		signal_first_byte_pos + data_len);
	}

	return error_code;
}

FUNC(dcm_nrc_type, DCM_CODE) get_did_data(const dcm_dsp_did_type *did_ptr, const PduInfoType *pdu_tx_data,
	dsp_uds_read_did_pending_type *read_did_pending, boolean include_did)
{
	const dcm_dsp_data_type *data_ptr;
	const dcm_dsp_signal_type *signal_ptr;
	dcm_nrc_type error_code = DCM_E_POSITIVERESPONSE;
	uint16 required_buf_size;
	dcm_op_status_type op_status = DCM_INITIAL;
	Std_ReturnType result = E_OK;

	boolean in_pending_state = (DCM_READ_DID_PENDING_COND_CHECK == read_did_pending->state) ||
		(DCM_READ_DID_PENDING_READ_DATA == read_did_pending->state);
	required_buf_size = read_did_pending->tx_write_pos + did_ptr->dsp_did_data_byte_size;
	if ((TRUE == include_did) && (FALSE == in_pending_state)) {
		required_buf_size = required_buf_size + 2u;
	}
	if ((TRUE != in_pending_state) && (required_buf_size > pdu_tx_data->SduLength)) {
		error_code = DCM_E_RESPONSETOOLONG;
		return error_code;
	}

	do_check_in_pend_state_and_include_did(did_ptr, pdu_tx_data, &read_did_pending->tx_write_pos,
		&read_did_pending->pending_data_start_pos, &op_status,
		in_pending_state, include_did);
	uint16 signal_index = read_did_pending->pending_signal_index;

	while ((signal_index < did_ptr->dsp_nof_signals) && (DCM_E_POSITIVERESPONSE == error_code)) {
		signal_ptr = &did_ptr->dsp_signal_ref[signal_index];
		data_ptr = signal_ptr->dsp_signal_data_ref;

		if (((DCM_READ_DID_PENDING_COND_CHECK == read_did_pending->state) ||
			(DCM_READ_DID_IDLE == read_did_pending->state)) &&
			(DATA_PORT_ECU_SIGNAL != data_ptr->dsp_data_use_port) &&
			(DATA_PORT_SR != data_ptr->dsp_data_use_port)) {
			result = do_check_dsp_data_condition_check_read(data_ptr, &read_did_pending->state,
				&op_status, &error_code);
		} else {
			result = E_OK;
			error_code = DCM_E_POSITIVERESPONSE;
		}

		if ((result == E_OK) && (error_code == DCM_E_POSITIVERESPONSE)) {
			error_code = do_check_dsp_data_read_data(signal_index, op_status, pdu_tx_data,
				signal_ptr, read_did_pending);
		} else if (DCM_E_PENDING == result) {
			read_did_pending->pending_signal_index = signal_index;
			error_code = DCM_E_RESPONSEPENDING;
		} else {
			error_code = DCM_E_CONDITIONSNOTCORRECT;
		}
		signal_index++;
	}

	return error_code;
}

static FUNC(Std_ReturnType, DCM_CODE)
	dcm_mode_rule_check_condition(dcm_logical_operator_type operator, const dcm_mode_rule_type *mode_rule)
{
	Std_ReturnType result = (DCM_AND == operator) ? E_OK : E_NOT_OK;

	for (uint8 i = 0; i < mode_rule->nof_condition_in_rule; i++) {
		dcm_mode_condition_type mode_condition =
			dcm_config_ptr->dcm_processing_conditions->mode_condition[mode_rule->condition_index_list[i]];
		uint8 target = mode_condition.dcm_processing_condition_mode;
		dcm_condition_type condition = mode_condition.condition;

		if (DCM_AND == operator) {
			if ((DCM_EQUALS == condition) && (mode_condition.swc_mode_ref() != target)) {
				result = E_NOT_OK;
				break;
			}
			if ((DCM_EQUALS_NOT == condition) && (mode_condition.swc_mode_ref() == target)) {
				result = E_NOT_OK;
				break;
			}
		}
		if (DCM_OR == operator) {
			if ((DCM_EQUALS == condition) && (mode_condition.swc_mode_ref() == target)) {
				result = E_OK;
				break;
			}
			if ((DCM_EQUALS_NOT == condition) && (mode_condition.swc_mode_ref() != target)) {
				result = E_OK;
				break;
			}
		}
	}

	return result;
}

FUNC(Std_ReturnType, DCM_CODE) dcm_mode_mode_rule_cbk_by_idx(const uint8 idx, dcm_mode_rule_nrc_value *nrc)
{
#if (DCM_MODEMGR_RULES_SIZE == 0)
	(void)idx;
	(void)nrc;
	return E_OK;
#else
	const dcm_mode_rule_type *dcm_mode_rules_cfg = dcm_config_ptr->dcm_processing_conditions->mode_rule;
	const dcm_mode_condition_type *dcm_mode_condition_cfg =
		dcm_config_ptr->dcm_processing_conditions->mode_condition;
	if ((idx >= (uint8)DCM_MODEMGR_RULES_SIZE) || (dcm_mode_rules_cfg == NULL) ||
		(dcm_mode_condition_cfg == NULL)) {
		return E_OK;
	}

	if ((0u == dcm_mode_rules_cfg[idx].nof_rule_in_rule) && (0u == dcm_mode_rules_cfg[idx].nof_condition_in_rule)) {
		return E_OK;
	}

	dcm_logical_operator_type operator= dcm_mode_rules_cfg[idx].logical_operator;
	Std_ReturnType result = (DCM_AND == operator) ? E_NOT_OK : E_OK;

	if ((DCM_AND == operator) && (E_OK == dcm_mode_rule_check_condition(operator, &dcm_mode_rules_cfg[idx]))) {
		result = E_OK;
		for (uint8 i = 0; i < dcm_mode_rules_cfg[idx].nof_rule_in_rule; i++) {
			uint8 rule_id = dcm_mode_rules_cfg[idx].rule_index_list[i];

			if (E_NOT_OK == dcm_mode_mode_rule_cbk_by_idx(rule_id, nrc)) {
				result = E_NOT_OK;
				break;
			}
		}
	}

	if ((DCM_OR == operator) && (E_NOT_OK == dcm_mode_rule_check_condition(operator, &dcm_mode_rules_cfg[idx]))) {
		result = E_NOT_OK;
		for (uint8 i = 0; i < dcm_mode_rules_cfg[idx].nof_rule_in_rule; i++) {
			uint8 rule_id = dcm_mode_rules_cfg[idx].rule_index_list[i];

			if (E_OK == dcm_mode_mode_rule_cbk_by_idx(rule_id, nrc)) {
				result = E_OK;
				break;
			}
		}
	}

	*nrc = dcm_mode_rules_cfg[idx].mode_rule_nrc_value;
	return result;
#endif
}

FUNC(void, DCM_CODE) dcm_rst_diag_activity_on_session_change(void)
{
	dsp_reset_diag_activity_on_session_change();
#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
	dsd_cancel_pending_external_services();
#endif
}

FUNC(void, DCM_CODE) dcm_rst_diag_activity(const dcm_dsl_protocol_row_type *protocol_row_entry)
{
	dcm_dsp_reset_diag_activity(protocol_row_entry);
#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
	dsd_cancel_pending_external_services();
#endif
}

FUNC(void, DCM_CODE) dcm_cancel_pending_req(void)
{
	dsp_cancel_pending_req();
#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
	dsd_cancel_pending_external_services();
#endif
}

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"
