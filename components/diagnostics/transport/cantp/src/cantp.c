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

#include "cantp.h"
#include "cantp_internal.h"
#include "SchM_CanTp.h"
#if (CONFIG_MODULE_CANIF == 1)
#include "canif.h"
#endif
#if (CONFIG_MODULE_PDUR == 1)
#include "pdur_cantp.h"
#endif
#include "vcos_log.h"
#include "cantp_vcos_log.h"
#include "Os.h"

#define CANTP_START_SEC_CODE
#include "cantp_memmap.h"

FUNC(void, CANTP_CODE) cantp_init(const cantp_config_type *cfg_ptr)
{
	cantp_channel_table_type *channel_table = NULL;
	const cantp_runtime_data_type *core_runtime = NULL;
	uint8 core_id = cantp_get_core_id();

	CANTP_VALIDATE_NO_RV(NULL != cfg_ptr, CANTP_SERVICE_ID_CANTP_INIT_CANTP_E_PARAM_POINTER);
#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE_NO_RV(core_id < CANTP_MULTICORE_CORE_NUMBER, CANTP_SERVICE_ID_CANTP_INIT_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return;
	}

	CANTP_VALIDATE_NO_RV(*(core_runtime->state) == CANTP_OFF, CANTP_SERVICE_ID_CANTP_INIT_CANTP_E_PARAM_ID);

	cantp_set_config_ptr(cfg_ptr);

	for (uint8 i = 0; i < (uint8)core_runtime->channel_table_len; i++) {
		cantp_init_runtime_data(&channel_table[i].channel_list[CANTP_TX_CHANNEL], CANTP_ISO15765_TRANSMIT);
		cantp_init_runtime_data(&channel_table[i].channel_list[CANTP_RX_CHANNEL], CANTP_ISO15765_RECEIVE);
	}

	cantp_init_params(core_id);

	*(core_runtime->state) = CANTP_ON;
}

FUNC(void, CANTP_CODE) cantp_shutdown(void)
{
	cantp_channel_table_type *channel_table = NULL;
	const cantp_runtime_data_type *core_runtime = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE_NO_RV(core_id < CANTP_MULTICORE_CORE_NUMBER, CANTP_SERVICE_ID_CANTP_SHUTDOWN_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return;
	}

	CANTP_VALIDATE_NO_RV(*(core_runtime->state) == CANTP_ON, CANTP_SERVICE_ID_CANTP_SHUTDOWN_CANTP_E_UNINIT);

	*(core_runtime->state) = CANTP_OFF;
}

FUNC(Std_ReturnType, CANTP_CODE) cantp_transmit(PduIdType cantp_tx_pdu_id, const PduInfoType *cantp_tx_info_ptr)
{
	cantp_channel_type *tx_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;
	const cantp_runtime_data_type *core_runtime = NULL;
	const cantp_tx_nsdu_type *tx_config = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE(core_id < CANTP_MULTICORE_CORE_NUMBER, CANTP_SERVICE_ID_CANTP_TRANSMIT_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return E_NOT_OK;
	}

	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const cantp_nsdu_type *cantp_nsdu_list = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;
	PduIdType cantp_tx_pdu_index = cantp_get_pdu_index(core_id, cantp_tx_pdu_id);

	CANTP_VALIDATE(NULL_PTR != cantp_tx_info_ptr, CANTP_SERVICE_ID_CANTP_TRANSMIT_CANTP_E_PARAM_POINTER);
	CANTP_VALIDATE(*(core_runtime->state) == CANTP_ON, CANTP_SERVICE_ID_CANTP_TRANSMIT_CANTP_E_UNINIT);
	CANTP_VALIDATE(cantp_tx_pdu_index < cantp_cfg->cantp_ipdu_channel_info[core_id].number_of_pdus,
		       CANTP_SERVICE_ID_CANTP_TRANSMIT_CANTP_E_INVALID_TX_ID);
#if (CANTP_LONG_FF_SUPPORT != STD_ON)
	CANTP_VALIDATE(cantp_tx_info_ptr->SduLength <= CANTP_FF_MAXDL,
		       CANTP_SERVICE_ID_CANTP_TRANSSduLengthMIT_CANTP_E_INVALID_TX_LENGTH);
#endif
	CANTP_VALIDATE(CANTP_INVALID_PDU_ID != cantp_tx_pdu_index,
		       CANTP_SERVICE_ID_CANTP_TRANSMIT_CANTP_E_INVALID_TX_ID);

	tx_config = &cantp_nsdu_list[cantp_tx_pdu_index].config_data.cantp_tx_nsdu;
	tx_runtime = &channel_table[tx_config->cantp_tx_channel].channel_list[CANTP_TX_CHANNEL];
	if (CANTP_IDLE != tx_runtime->iso15765.state) {
		return E_NOT_OK;
	}

	cantp_frame_type iso15765frame = cantp_calc_tx_frame_type(tx_config, cantp_tx_info_ptr->SduLength);

	if ((CANTP_SINGLE_FRAME != iso15765frame) && (CANTP_FIRST_FRAME != iso15765frame)) {
		return E_NOT_OK;
	}

	tx_runtime->transfer_count = 0;
	tx_runtime->iso15765.frames_handled_count = 0;
	tx_runtime->transfer_total = cantp_tx_info_ptr->SduLength;
	tx_runtime->iso15765.state_timeout_count = tx_config->cantp_ncs + (uint16)1;
	tx_runtime->mode = CANTP_TX_PROCESSING;
	tx_runtime->sdu_id = cantp_tx_pdu_index;
	tx_runtime->canfd_support = cantp_nsdu_list[cantp_tx_pdu_index].cantp_fd_support;

	return cantp_tx_frame(iso15765frame, tx_config, tx_runtime);
}

FUNC(void, CANTP_CODE) cantp_rx_indication(PduIdType cantp_rx_pdu_id, const PduInfoType *cantp_rx_pdu_ptr)
{
	cantp_channel_table_type *channel_table = NULL;
	const cantp_runtime_data_type *core_runtime = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE_NO_RV(core_id < CANTP_MULTICORE_CORE_NUMBER,
			     CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return;
	}

	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();

	const cantp_rx_id_type *cantp_ipdu_list = cantp_cfg->cantp_ipdu_channel_info[core_id].ipdu_cfg_addr;
	const cantp_nsdu_type *cantp_nsdu_list = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;

	PduIdType cantp_rx_pdu_id_cur = cantp_get_pdu_index(core_id, cantp_rx_pdu_id);

	if (E_OK != cantp_check_rx_ind_valid(cantp_rx_pdu_id_cur, cantp_rx_pdu_ptr, core_id)) {
		return;
	}
	PduIdType pdu_index = cantp_get_pdu_index_from_ipdu_list(cantp_rx_pdu_id, cantp_rx_pdu_ptr, core_id);

	CANTP_VALIDATE_NO_RV(CANTP_INVALID_PDU_ID != pdu_index,
			     CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_INVALID_RX_ID);

	cantp_frame_type frame_type =
		cantp_get_frame_type(&cantp_ipdu_list[pdu_index].cantp_addressing_mode, cantp_rx_pdu_ptr);

	if (CANTP_INVALID_FRAME == frame_type) {
		return;
	}

	const cantp_rx_nsdu_type *rx_config_params = NULL;
	const cantp_tx_nsdu_type *tx_config_params = NULL;
	cantp_channel_type *runtime_params = NULL;
	PduIdType cantp_rx_nsdu_id = CANTP_INVALID_PDU_ID;
	PduLengthType pad_len = 0;
	boolean padding_activation = FALSE;

	if (CANTP_FLOW_CONTROL_FRAME == frame_type) { /* 流控帧 - 流控帧是一个接收通道负责发送的帧 */
		PduIdType cantp_tx_nsdu_id = cantp_ipdu_list[pdu_index].cantp_referring_tx_index;

		CANTP_VALIDATE_NO_RV(CANTP_NO_REFERRING_TX_INDEX != cantp_tx_nsdu_id,
			       CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_RX_INVALID_INDEX);
		tx_config_params = &cantp_nsdu_list[cantp_tx_nsdu_id].config_data.cantp_tx_nsdu;
		runtime_params =
			&channel_table[tx_config_params->cantp_tx_channel].channel_list[CANTP_TX_CHANNEL];
		padding_activation = (CANTP_ON == cantp_cfg->cantp_general->padding_activation) &&
				     (CANTP_ON == tx_config_params->cantp_tx_padding_activation);
		pad_len = cantp_get_padding_length((uint8)cantp_rx_pdu_ptr->SduLength, padding_activation,
								 runtime_params->canfd_support);
	} else { /* 非流控帧 */
		cantp_rx_nsdu_id = pdu_index;
		rx_config_params = &cantp_nsdu_list[cantp_rx_nsdu_id].config_data.cantp_rx_nsdu;
		runtime_params =
			&channel_table[rx_config_params->cantp_rx_channel].channel_list[CANTP_RX_CHANNEL];
		runtime_params->canfd_support = cantp_nsdu_list[cantp_rx_nsdu_id].cantp_fd_support;
		padding_activation = (CANTP_ON == cantp_cfg->cantp_general->padding_activation) &&
				     (CANTP_ON == rx_config_params->cantp_rx_padding_activation);
		pad_len = cantp_get_padding_length((uint8)(cantp_rx_pdu_ptr->SduLength),
								 padding_activation, runtime_params->canfd_support);
	}

	CANTP_VALIDATE_NO_RV(cantp_check_rx_ind_addr(frame_type, cantp_rx_pdu_ptr, rx_config_params, tx_config_params),
			     CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_PARAM_ADDRESS);
	if (pad_len != cantp_rx_pdu_ptr->SduLength) {
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_WARN, CANTP_DLT_CHECK_ID17);
	}

	cantp_handle_rx_frame(frame_type, cantp_rx_nsdu_id, cantp_rx_pdu_ptr, rx_config_params, tx_config_params,
			runtime_params, core_id);
}

FUNC(void, CANTP_CODE) cantp_tx_confirmation(PduIdType cantp_tx_pdu_id, Std_ReturnType result)
{
	(void)result;
	const cantp_rx_nsdu_type *rx_config_params = NULL;
	const cantp_tx_nsdu_type *tx_config_params = NULL;
	const cantp_runtime_data_type *core_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE_NO_RV(core_id < CANTP_MULTICORE_CORE_NUMBER,
			     CANTP_SERVICE_ID_CANTP_TX_CONFIRMATION_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return;
	}
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const cantp_nsdu_type *cantp_nsdu_list = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;

	PduIdType cantp_tx_pdu_index = cantp_get_pdu_index(core_id, cantp_tx_pdu_id);

	CANTP_VALIDATE_NO_RV(*(core_runtime->state) == CANTP_ON, CANTP_SERVICE_ID_CANTP_TX_CONFIRMATION_CANTP_E_UNINIT);
	CANTP_VALIDATE_NO_RV(cantp_tx_pdu_index < cantp_cfg->cantp_ipdu_channel_info[core_id].number_of_pdus,
			     CANTP_SERVICE_ID_CANTP_TX_CONFIRMATION_CANTP_E_INVALID_TX_ID);

	if (CANTP_INVALID_PDU_ID == cantp_tx_pdu_index) {
		return;
	}

	if (CANTP_ISO15765_TRANSMIT == cantp_nsdu_list[cantp_tx_pdu_index].direction) {
		tx_config_params = (const cantp_tx_nsdu_type *)&cantp_nsdu_list[cantp_tx_pdu_index]
						.config_data.cantp_tx_nsdu;
		cantp_channel_type *tx_runtime =
			&channel_table[tx_config_params->cantp_tx_channel]
					.channel_list[CANTP_TX_CHANNEL];
		if (CANTP_TX_WAIT_CONFIRMATION == tx_runtime->iso15765.state) {
			cantp_handle_send_next_tx_frame(tx_config_params, tx_runtime, core_id);
		}
	} else {
		rx_config_params = (const cantp_rx_nsdu_type *)&cantp_nsdu_list[cantp_tx_pdu_index]
					.config_data.cantp_rx_nsdu;
		channel_table[rx_config_params->cantp_rx_channel].channel_list[CANTP_RX_CHANNEL]
			.iso15765.nas_nar_pending = FALSE;
	}

	return;
}

#if (CANTP_TRANSMIT_CANCEL_API == STD_ON)
FUNC(Std_ReturnType, CANTP_CODE) cantp_cancel_transmit(PduIdType tx_pdu_id)
{
	const cantp_runtime_data_type *core_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE(core_id < CANTP_MULTICORE_CORE_NUMBER, CANTP_SERVICE_ID_CANTP_CANCEL_TRANSMIT_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return E_NOT_OK;
	}
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const cantp_nsdu_type *cantp_nsdu_list = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;

	PduIdType tx_pdu_index = cantp_get_pdu_index(core_id, tx_pdu_id);

	CANTP_VALIDATE(tx_pdu_index < cantp_cfg->cantp_ipdu_channel_info[core_id].number_of_pdus,
		       CANTP_SERVICE_ID_CANTP_CANCEL_TRANSMIT_CANTP_E_PARAM_ID);

	CANTP_VALIDATE(CANTP_INVALID_PDU_ID != tx_pdu_index,
		       CANTP_SERVICE_ID_CANTP_CANCEL_TRANSMIT_CANTP_E_INVALID_TX_ID);

	const cantp_tx_nsdu_type *tx_config = &cantp_nsdu_list[tx_pdu_index].config_data.cantp_tx_nsdu;
	cantp_channel_type *tx_runtime =
		&channel_table[tx_config->cantp_tx_channel].channel_list[CANTP_TX_CHANNEL];

	CANTP_VALIDATE(CANTP_TX_PROCESSING == tx_runtime->mode,
		       CANTP_SERVICE_ID_CANTP_CANCEL_TRANSMIT_CANTP_E_OPER_NOT_SUPPORTED);

#if (CONFIG_MODULE_PDUR == 1)
	pdur_cantp_tx_confirmation(tx_config->pdur_pdu_id, NTFRSLT_E_NOT_OK);
#endif
	cantp_init_runtime_data(tx_runtime, CANTP_ISO15765_TRANSMIT);

	return E_OK;
}
#endif

#if (CANTP_RECEIVE_CANCEL_API == STD_ON)
FUNC(Std_ReturnType, CANTP_CODE) cantp_cancel_receive(PduIdType rx_pdu_id)
{
	const cantp_runtime_data_type *core_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE(core_id < CANTP_MULTICORE_CORE_NUMBER, CANTP_SERVICE_ID_CANTP_CANCEL_RECEIVE_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return E_NOT_OK;
	}
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const cantp_nsdu_type *cantp_nsdu_list = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;
	PduIdType rx_pdu_index = cantp_get_pdu_index(core_id, rx_pdu_id);

	CANTP_VALIDATE(rx_pdu_index < cantp_cfg->cantp_ipdu_channel_info[core_id].number_of_pdus,
		 CANTP_SERVICE_ID_CANTP_CANCEL_RECEIVE_CANTP_E_PARAM_ID);
	CANTP_VALIDATE(CANTP_INVALID_PDU_ID != rx_pdu_index,
		       CANTP_SERVICE_ID_CANTP_CANCEL_RECEIVE_CANTP_E_INVALID_RX_ID);

	if (CANTP_ISO15765_TRANSMIT == cantp_nsdu_list[rx_pdu_index].direction) {
		return E_NOT_OK;
	}

	const cantp_rx_nsdu_type *rx_config = &cantp_nsdu_list[rx_pdu_index].config_data.cantp_rx_nsdu;

	if (CANTP_FUNCTIONAL == rx_config->cantp_rx_ta_type) {
		return E_NOT_OK;
	}

	cantp_channel_type *rx_runtime = &channel_table[rx_config->cantp_rx_channel].channel_list[CANTP_RX_CHANNEL];

	CANTP_VALIDATE(CANTP_RX_PROCESSING == rx_runtime->mode,
		       CANTP_SERVICE_ID_CANTP_CANCEL_RECEIVE_CANTP_E_OPER_NOT_SUPPORTED);
#if (CONFIG_MODULE_PDUR == 1)
	pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_E_NOT_OK);
#endif
	cantp_init_runtime_data(rx_runtime, CANTP_ISO15765_RECEIVE);

	return E_OK;
}
#endif

#if (CANTP_CHANGE_PARAMETER_API == STD_ON)
FUNC(Std_ReturnType, CANTP_CODE) cantp_change_parameter(PduIdType id, tp_parameter_type parameter, uint16 value)
{
	const cantp_runtime_data_type *core_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE(core_id < CANTP_MULTICORE_CORE_NUMBER,
		       CANTP_SERVICE_ID_CANTP_CHANGE_PARAMETER_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return E_NOT_OK;
	}
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const cantp_nsdu_type *cantp_nsdu_list = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;
	PduIdType index = cantp_get_pdu_index(core_id, id);

	CANTP_VALIDATE(index < cantp_cfg->cantp_nsdu_channel_info[core_id].number_of_sdus,
		       CANTP_SERVICE_ID_CANTP_CHANGE_PARAMETER_CANTP_E_PARAM_ID);
	CANTP_VALIDATE(CANTP_ISO15765_RECEIVE == cantp_nsdu_list[index].direction,
		       CANTP_SERVICE_ID_CANTP_CHANGE_PARAMETER_CANTP_E_PARAM_ID);

	uint8 channel_id = cantp_nsdu_list[index].config_data.cantp_rx_nsdu.cantp_rx_channel;
	cantp_channel_type *rx_runtime =
		&channel_table[channel_id].channel_list[CANTP_RX_CHANNEL];

	if ((TP_STMIN == parameter) && (value < CANTP_STMIN_MILLISEC_MAX)) {
		rx_runtime->iso15765.config_stmin = (uint8)value;
	} else if (TP_BS == parameter) {
		rx_runtime->iso15765.config_bs = (uint8)value;
	} else {
		CANTP_VALIDATE(FALSE, CANTP_SERVICE_ID_CANTP_CHANGE_PARAMETER_CANTP_E_PARAM_ID);
	}

	return E_OK;
}
#endif

#if (CANTP_READ_PARAMETER_API == STD_ON)
FUNC(Std_ReturnType, CANTP_CODE) cantp_read_parameter(PduIdType id, tp_parameter_type parameter, uint16 *value)
{
	const cantp_runtime_data_type *core_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE(core_id < CANTP_MULTICORE_CORE_NUMBER, CANTP_SERVICE_ID_CANTP_READ_PARAMETER_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return E_NOT_OK;
	}
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const cantp_nsdu_type *cantp_nsdu_list = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;
	PduIdType index = cantp_get_pdu_index(core_id, id);

	CANTP_VALIDATE(index < cantp_cfg->cantp_ipdu_channel_info[core_id].number_of_pdus,
		       CANTP_SERVICE_ID_CANTP_READ_PARAMETER_CANTP_E_PARAM_ID);
	CANTP_VALIDATE(CANTP_ISO15765_RECEIVE == cantp_nsdu_list[index].direction,
		       CANTP_SERVICE_ID_CANTP_READ_PARAMETER_CANTP_E_PARAM_ID);

	uint8 channel_id = cantp_nsdu_list[index].config_data.cantp_rx_nsdu.cantp_rx_channel;
	const cantp_channel_type *rx_runtime = &channel_table[channel_id].channel_list[CANTP_RX_CHANNEL];

	if (TP_STMIN == parameter) {
		*value = (uint16)rx_runtime->iso15765.config_stmin;
	} else if (TP_BS == parameter) {
		*value = (uint16)rx_runtime->iso15765.config_bs;
	} else {
		CANTP_VALIDATE(FALSE, CANTP_SERVICE_ID_CANTP_READ_PARAMETER_CANTP_E_PARAM_ID);
	}

	return E_OK;
}
#endif
#define CANTP_STOP_SEC_CODE
#include "cantp_memmap.h"
