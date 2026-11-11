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
#include "vcos_log.h"
#include "dcm_vcos_log.h"
#if (CONFIG_MODULE_COMM == 1)
#include "comm_dcm.h"
#endif
#if (CONFIG_MODULE_PDUR == 1)
#include "pdur.h"
#include "pdur_dcm.h"
#endif
#if (CONFIG_MODULE_RTE == 1)
#include "Rte_Dcm.h"
#endif
#include "SchM_Dcm.h"

#define DECREMENT(timer)                                                                                               \
	{                                                                                                              \
		if ((timer) > 0u) {                                                                                    \
			(timer)--;                                                                                     \
		}                                                                                                      \
	}

#define DCM_SERVICE_TESTERPRESENT_PDU_LEN 2u
#define DCM_DSL_TP_SDU_DEFAULT_LEN 2u

typedef struct {
	boolean init_run;
	const dcm_dsl_protocol_row_type *active_protocol;
} dcm_dsl_run_time_data_type;

#define DCM_START_SEC_PRIVATE_BSS
#include "dcm_memmap.h"
static VAR(dcm_dsl_run_time_data_type, DCM_PRIVATE_BSS) dsl_run_time_data;
static VAR(boolean, DCM_PRIVATE_BSS) dsl_preemption_no_process_done;
static VAR(boolean, DCM_PRIVATE_BSS) dsl_busy_sent;
extern VAR(boolean, DCM_PRIVATE_BSS) global_compare_key_flag;
extern VAR(dcm_sec_lv_type, DCM_PRIVATE_BSS) global_requested_security_level;
#define DCM_STOP_SEC_PRIVATE_BSS
#include "dcm_memmap.h"

#define DCM_START_SEC_MULTI_APP_BSS
#include "dcm_memmap.h"
#if (CONFIG_MODULE_COMM == 1)
static VAR(dcm_com_mode_type, DCM_MULTI_APP_BSS) dsl_net_com_mode[DCM_NOF_COMM_CHANNELS];
#endif
#define DCM_STOP_SEC_MULTI_APP_BSS
#include "dcm_memmap.h"

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

static FUNC_P2CONST(dcm_dsp_session_type, AUTOMATIC, DCM_CODE)
	dsl_get_active_session(const dcm_ses_ctrl_type session_control)
{
	const dcm_dsp_session_type *session_list = dcm_config_ptr->dsp->dsp_session;
	uint8 session_index;

	for (session_index = 0; session_index < DCM_SESSION_EOL_INDEX; session_index++) {
		if (session_list[session_index].dsp_session_level == session_control) {
			break;
		}
	}
	if (session_index == DCM_SESSION_EOL_INDEX) {
		session_index = 0;
		DCM_DET_REPORTERROR(DCM_DCM_CHANGE_DIAGNOSTIC_SESSION_ID_DCM_E_CONFIG_INVALID);
	}

	return &session_list[session_index];
}

static void dsl_update_session_state(dcm_dsl_run_time_protocol_parameters_type *runtime,
	dcm_ses_ctrl_type new_session)
{
	runtime->session_control = new_session;
	dcm_rst_diag_activity_on_session_change();

#if (CONFIG_MODULE_RTE == 1)
	(void)Rte_Switch_DiagnosticSessionControlModeSwitchInterface_DcmDiagnosticSessionControl(new_session);
#endif

#ifdef DCM_CALLBACK_SESSIONCONTROL_ENABLED
	(void)dcm_callback_dcm_diagnostic_session_control(new_session);
#endif
}

#if (CONFIG_MODULE_COMM == 1)
static void dsl_deactivate_diagnostic_comm(dcm_dsl_run_time_protocol_parameters_type *runtime,
	uint8 comm_channel_index)
{
	if (runtime->diagnostic_active_comm) {
		comm_dcm_inactive_diagnostic(dcm_config_ptr->dcm_comm_channel_cfg[comm_channel_index]
			.network_handle);
		runtime->diagnostic_active_comm = FALSE;
	}
}

static void dsl_activate_diagnostic_comm(dcm_dsl_run_time_protocol_parameters_type *runtime,
	uint8 comm_channel_index)
{
	if (!runtime->diagnostic_active_comm) {
		comm_dcm_active_diagnostic(dcm_config_ptr->dcm_comm_channel_cfg[comm_channel_index]
			.network_handle);
		runtime->diagnostic_active_comm = TRUE;
	}
}
#endif

static FUNC(void, DCM_CODE) dsl_change_diag_session(dcm_dsl_run_time_protocol_parameters_type *runtime,
	dcm_ses_ctrl_type new_session)
{
	if (DCM_DEFAULT_SESSION == runtime->session_control) {
		dsp_init(FALSE);
	} else {
		runtime->security_level = DCM_SEC_LEV_LOCKED;
	}

	if (new_session == runtime->session_control) {
		dsl_update_session_state(runtime, new_session);
		return;
	}

#if (CONFIG_MODULE_COMM == 1)
	uint8 comm_channel_index =
		dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[runtime->diag_reqest_rx_pdu_id]
			.comm_channel_internal_index;

	if (DCM_DEFAULT_SESSION == new_session) {
		dsl_deactivate_diagnostic_comm(runtime, comm_channel_index);
	} else {
		dsl_activate_diagnostic_comm(runtime, comm_channel_index);
	}
#endif

	global_compare_key_flag = FALSE;
	global_requested_security_level = 0u;
	dsl_update_session_state(runtime, new_session);
}

FUNC(void, DCM_CODE) dsl_reset_session_timeout_timer(void)
{
	const dcm_dsl_protocol_row_type *active_protocol;
	dcm_dsl_run_time_protocol_parameters_type *runtime;

	active_protocol = dsl_run_time_data.active_protocol;
	if (NULL == active_protocol) {
		return;
	}

	runtime = active_protocol->dsl_run_time_protocol_parameters;
	runtime->s3server_timeout_count = active_protocol->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
	runtime->s3server_started = TRUE;
}

FUNC(Std_ReturnType, DCM_CODE) dsl_get_active_protocol(dcm_protocol_type *protocol_id)
{
	const dcm_dsl_protocol_row_type *active_protocol;

	active_protocol = dsl_run_time_data.active_protocol;
	if (NULL == active_protocol) {
		return E_NOT_OK;
	}
	*protocol_id = active_protocol->dsl_protocol_id;
	return E_OK;
}

FUNC(void, DCM_CODE) dsl_internal_set_security_level(dcm_sec_lv_type sec_level)
{
	const dcm_dsl_protocol_row_type *active_protocol;
	dcm_dsl_run_time_protocol_parameters_type *runtime;

	active_protocol = dsl_run_time_data.active_protocol;
	runtime = active_protocol->dsl_run_time_protocol_parameters;
	runtime->security_level = sec_level;
#if (CONFIG_MODULE_RTE == 1)
	(void)Rte_Switch_DiagnosticSessionControlModeSwitchInterface_DcmDiagnosticSessionControl(sec_level);
#endif
}

FUNC(Std_ReturnType, DCM_CODE) dsl_get_security_level(dcm_sec_lv_type *sec_level)
{
	const dcm_dsl_protocol_row_type *active_protocol;
	const dcm_dsl_run_time_protocol_parameters_type *runtime = NULL;

	active_protocol = dsl_run_time_data.active_protocol;

	if (NULL == active_protocol) {
		return E_NOT_OK;
	}

	runtime = active_protocol->dsl_run_time_protocol_parameters;
	*sec_level = runtime->security_level;
	return E_OK;
}

FUNC(void, DCM_CODE) dsl_set_ses_ctrl_type(dcm_ses_ctrl_type ses_ctrl)
{
	const dcm_dsl_protocol_row_type *active_protocol;
	dcm_dsl_run_time_protocol_parameters_type *runtime;

	active_protocol = dsl_run_time_data.active_protocol;
	runtime = active_protocol->dsl_run_time_protocol_parameters;

	dsl_change_diag_session(runtime, ses_ctrl);

	runtime->s3server_timeout_count = active_protocol->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
	runtime->s3server_started = TRUE;
}

FUNC(Std_ReturnType, DCM_CODE) dsl_get_ses_ctrl_type(dcm_ses_ctrl_type *ses_ctrl_type)
{
	Std_ReturnType ret = E_NOT_OK;
	const dcm_dsl_protocol_row_type *active_protocol;
	const dcm_dsl_run_time_protocol_parameters_type *runtime;

	active_protocol = dsl_run_time_data.active_protocol;

	runtime = active_protocol->dsl_run_time_protocol_parameters;

	*ses_ctrl_type = runtime->session_control;
	ret = E_OK;

	return ret;
}

static inline FUNC(void, DCM_CODE) dsl_release_ex_rx_tx_buffers(const dcm_dsl_protocol_row_type *protocol_row)
{
	dcm_dsl_run_time_protocol_parameters_type *runtime = protocol_row->dsl_run_time_protocol_parameters;

	if (protocol_row->dsl_protocol_tx_buffer != NULL &&
		protocol_row->dsl_protocol_tx_buffer->external_buffer_runtime_data != NULL) {
		protocol_row->dsl_protocol_tx_buffer->external_buffer_runtime_data->status = BUFFER_AVAILABLE;
		protocol_row->dsl_protocol_tx_buffer->external_buffer_runtime_data->dcm_rx_pdu_id = DCM_INVALID_PDU_ID;
	}
	if (protocol_row->dsl_protocol_rx_buffer != NULL &&
		protocol_row->dsl_protocol_rx_buffer->external_buffer_runtime_data != NULL) {
		protocol_row->dsl_protocol_rx_buffer->external_buffer_runtime_data->status = BUFFER_AVAILABLE;
	}
	runtime->external_tx_buffer_status = NOT_IN_USE;
	runtime->external_rx_buffer_status = NOT_IN_USE;
}

FUNC(void, DCM_CODE) dsl_dsd_processing_done(PduIdType rx_pdu_id, dsd_processing_done_result_type response_result)
{
	if (rx_pdu_id >= DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		return;
	}
	const dcm_dsl_protocol_rx_type *protocol_rx =
		&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[rx_pdu_id];
	const dcm_dsl_protocol_row_type *protocol_row = protocol_rx->dsl_protocol_row;
	dcm_dsl_run_time_protocol_parameters_type *runtime = protocol_row->dsl_run_time_protocol_parameters;

	if (DCM_DSD_TX_RESPONSE_SUPPRESSED == response_result) {
		dsl_release_ex_rx_tx_buffers(protocol_row);
		if ((DCM_DEFAULT_SESSION == runtime->session_control) && (runtime->diagnostic_active_comm)) {
#if (CONFIG_MODULE_COMM == 1)
			comm_dcm_inactive_diagnostic(dcm_config_ptr->dcm_comm_channel_cfg
			[protocol_rx->comm_channel_internal_index].network_handle);
#endif
			runtime->diagnostic_active_comm = FALSE;
		}
	} else if (DCM_DSD_TX_RESPONSE_READY == response_result) {
		runtime->external_tx_buffer_status = DSD_PENDING_RESPONSE_SIGNALED;
	} else {
		/* do nothing */
	}
}

static void dsl_fill_response_buffer(dcm_dsl_local_buffer_type *tx_buffer,
	const dcm_dsl_protocol_row_type *protocol, dcm_nrc_type response_code)
{
	tx_buffer->response_code = response_code;
	tx_buffer->status = PROVIDED_TO_DSD;
	tx_buffer->buffer[0] = DCM_SID_NEGATIVE_RESPONSE;
	tx_buffer->buffer[1] = protocol->dsl_protocol_rx_buffer->pdu_info.SduDataPtr[0];
	tx_buffer->buffer[2] = response_code;
	tx_buffer->pdu_info.SduDataPtr = tx_buffer->buffer;
	tx_buffer->pdu_info.SduLength = 3;
	tx_buffer->status = DCM_TRANSMIT_SIGNALED;

#if (DCM_RESPONSE_INDICATION == STD_ON)
	if (dcm_config_ptr != NULL) {
		(void)dcm_config_ptr->dsl->dsl_diag_response_indication(tx_buffer->pdu_info.SduDataPtr,
			tx_buffer->pdu_info.SduLength);
	}
#endif
}

static void dsl_handle_transmit_result(Std_ReturnType result, dcm_dsl_run_time_protocol_parameters_type *runtime,
	const dcm_dsl_protocol_row_type *protocol, dcm_nrc_type response_code)
{
	if ((result == E_OK) && (DCM_E_GENERALREJECT == response_code)) {
		runtime->s3server_timeout_count =
			protocol->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
		runtime->s3server_started = TRUE;
	}
}

static FUNC(Std_ReturnType, DCM_CODE) dsl_send_resp_with_status(const dcm_dsl_protocol_row_type *protocol,
	dcm_nrc_type response_code)
{
	dcm_dsl_run_time_protocol_parameters_type *runtime = protocol->dsl_run_time_protocol_parameters;
	dcm_dsl_local_buffer_type *tx_buffer = &runtime->local_tx_buffer;

	if (tx_buffer->status != NOT_IN_USE) {
		return E_NOT_OK;
	}

	dsl_fill_response_buffer(tx_buffer, protocol, response_code);

	if (DCM_E_RESPONSEPENDING == response_code) {
		dsd_clear_suppress_pos_rsp_msg();
	}

	Std_ReturnType transmit_result = pdur_dcm_transmit(runtime->diag_response_tx_pdu_id,
		&(tx_buffer->pdu_info));

	dsl_handle_transmit_result(transmit_result, runtime, protocol, response_code);

	return transmit_result;
}

static FUNC(uint16, DCM_CODE) dsl_adjust_p2time_to_count(uint16 max_value, uint16 adjust_value)
{
	uint16 p2time = 0u;

	if (max_value > adjust_value) {
		p2time = max_value - adjust_value;
	}
	uint16 p2count = p2time / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
	return p2count;
}

FUNC(void, DCM_CODE) dsl_dsd_send_resp_pending(PduIdType rx_pdu_id)
{
	const dcm_dsl_protocol_rx_type *protocol_rx =
		&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[rx_pdu_id];
	const dcm_dsl_protocol_row_type *protocol_row = protocol_rx->dsl_protocol_row;
	(void)dsl_send_resp_with_status(protocol_row, DCM_E_RESPONSEPENDING);
	dcm_dsl_run_time_protocol_parameters_type *rx_runtime = protocol_row->dsl_run_time_protocol_parameters;
	const dcm_dsp_session_type *session = dsl_get_active_session(rx_runtime->session_control);

	rx_runtime->state_timeout_count =
		dsl_adjust_p2time_to_count(session->dsp_session_p2star_server_max + DCM_MAIN_FUNCTION_PERIOD_TIME_MS,
			protocol_row->tim_str_p2star_server_adjust);
	DECREMENT(rx_runtime->response_pending_count);
}

static FUNC(Std_ReturnType, DCM_CODE)
	dsl_start_protocol_helper(dcm_protocol_type protocol_id, const dcm_dsl_main_connection_type *main_connection)
{
	Std_ReturnType ret = E_OK;

	for (uint8 i = 0; dcm_config_ptr->dsl->dsl_callback_dcm_request_service[i].start_protocol != NULL; i++) {
		Std_ReturnType res =
			dcm_config_ptr->dsl->dsl_callback_dcm_request_service[i].start_protocol
			(protocol_id, main_connection->dsl_rx_tester_source_address, main_connection->connection_id);
		if ((res == E_NOT_OK) || (res == DCM_E_PROTOCOL_NOT_ALLOWED)) {
			ret = E_NOT_OK;
			break;
		}
	}
	return ret;
}

static FUNC(Std_ReturnType, DCM_CODE)
	dsl_stop_protocol_helper(dcm_protocol_type protocol_id, const dcm_dsl_main_connection_type *main_connection)
{
	Std_ReturnType ret = E_OK;

	for (uint8 i = 0; dcm_config_ptr->dsl->dsl_callback_dcm_request_service[i].stop_protocol != NULL; i++) {
		Std_ReturnType res =
		dcm_config_ptr->dsl->dsl_callback_dcm_request_service[i].stop_protocol
		(protocol_id, main_connection->dsl_rx_tester_source_address, main_connection->connection_id);
		if ((res == E_NOT_OK) || (res == DCM_E_PROTOCOL_NOT_ALLOWED)) {
			ret = E_NOT_OK;
			break;
		}
	}
	return ret;
}

static FUNC(boolean, DCM_CODE) dsl_is_tester_present_cmd(const PduInfoType *rx_pdu, PduLengthType sdu_length)
{
	if ((sdu_length == DCM_SERVICE_TESTERPRESENT_PDU_LEN) &&
		(rx_pdu->SduDataPtr[0] == (uint8)DCM_SID_TESTER_PRESENT) &&
		(rx_pdu->SduDataPtr[1] == SUPPRESS_POS_RESP_BIT)) {
		return TRUE;
	}
	if (rx_pdu->SduDataPtr[0] == (uint8)DCM_SID_TESTER_PRESENT) {
		(void)VCOS_LOG(DCM_MODULE_ID, VCOS_LOG_ERROR, DCM_DLT_TESTER_PRESENT_COMMAND, sdu_length,
			rx_pdu->SduDataPtr[0], rx_pdu->SduDataPtr[1]);
	}
	return FALSE;
}

static void dsl_init_buffer_status(const dcm_dsl_buffer_type *buffer,
	dcm_dsl_buffer_status_type target_status, PduIdType pdu_id)
{
	buffer->external_buffer_runtime_data->status = target_status;
	buffer->external_buffer_runtime_data->dcm_rx_pdu_id = pdu_id;
}

static void dsl_init_runtime_parameters(dcm_dsl_run_time_protocol_parameters_type *runtime) {
	runtime->external_rx_buffer_status = NOT_IN_USE;
	runtime->external_tx_buffer_status = NOT_IN_USE;

	runtime->security_level = DCM_SEC_LEV_LOCKED;
	runtime->session_control = DCM_DEFAULT_SESSION;

	runtime->diagnostic_active_comm = FALSE;
	runtime->protocol_started = FALSE;

	runtime->s3server_started = FALSE;
	runtime->s3server_timeout_count = 0u;

	runtime->local_rx_buffer.status = NOT_IN_USE;
	runtime->local_rx_buffer.dcm_rx_pdu_id = DCM_INVALID_PDU_ID;
	runtime->local_tx_buffer.status = NOT_IN_USE;
}

FUNC(void, DCM_CODE) dsl_init(void)
{
	for (uint8 i = 0; i < DCM_DSL_PROTOCOL_LIST_LENGTH; i++) {
		const dcm_dsl_protocol_row_type *entry =
			&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_row_list[i];

		dsl_init_buffer_status(entry->dsl_protocol_rx_buffer, BUFFER_AVAILABLE, DCM_INVALID_PDU_ID);
		dsl_init_buffer_status(entry->dsl_protocol_tx_buffer, BUFFER_AVAILABLE, DCM_INVALID_PDU_ID);

		if (entry->dsl_run_time_protocol_parameters != NULL) {
			dsl_init_runtime_parameters(entry->dsl_run_time_protocol_parameters);
		}
	}

	dsl_run_time_data.init_run = TRUE;
	dsl_run_time_data.active_protocol = NULL;
}

static FUNC(void, DCM_CODE)
	dsl_ex_tx_buffer_status_provided_to_dsd(dcm_dsl_run_time_protocol_parameters_type *runtime,
		const dcm_dsl_protocol_row_type *protocol_row_entry)
{
	const dcm_dsp_session_type *session = NULL;

	DECREMENT(runtime->state_timeout_count);
	if (runtime->state_timeout_count != 0u) {
		return;
	}

	session = dsl_get_active_session(runtime->session_control);
	runtime->state_timeout_count = dsl_adjust_p2time_to_count(session->dsp_session_p2star_server_max,
		protocol_row_entry->tim_str_p2star_server_adjust);
	if ((runtime->response_pending_count != 0U) ||
		(dcm_config_ptr->dsl->dsl_diag_resp_max_num_resp_pend == 0U)) {
		(void)dsl_send_resp_with_status(protocol_row_entry, DCM_E_RESPONSEPENDING);
		DECREMENT(runtime->response_pending_count);
	} else {
		dcm_cancel_pending_req();
		(void)dsl_send_resp_with_status(protocol_row_entry, DCM_E_GENERALREJECT);
		dsl_release_ex_rx_tx_buffers(protocol_row_entry);
	}
}

static void dsl_cleanup_preemption(const dcm_dsl_protocol_row_type *current_protocol,
	const dcm_dsl_protocol_row_type *active_protocol, boolean release_active_buffers)
{
	dsl_preemption_no_process_done = FALSE;
	active_protocol->dsl_run_time_protocol_parameters->protocol_started = FALSE;

	if (release_active_buffers) {
		dsl_release_ex_rx_tx_buffers(active_protocol);
	}

	dsl_release_ex_rx_tx_buffers(current_protocol);
	dcm_rst_diag_activity(current_protocol);
}

static void dsl_handle_preemption(dcm_dsl_run_time_protocol_parameters_type *runtime,
	const dcm_dsl_protocol_row_type *current_protocol,
	const dcm_dsl_protocol_row_type *active_protocol, boolean is_timeout)
{
	dcm_dsl_run_time_protocol_parameters_type *active_runtime =
		active_protocol->dsl_run_time_protocol_parameters;

	if (active_runtime->external_tx_buffer_status == NOT_IN_USE) {
		dsl_cleanup_preemption(current_protocol, active_protocol, FALSE);
	} else if (is_timeout) {
		dsl_cleanup_preemption(current_protocol, active_protocol, TRUE);
		dsp_init(FALSE);
	} else {
		/* do nothing */
	}
}

static FUNC(void, DCM_CODE)
	dsl_ex_tx_buffer_status_preempt_tx_nrc(dcm_dsl_run_time_protocol_parameters_type *runtime,
		const dcm_dsl_protocol_row_type *protocol_row_entry)
{
	if (!dsl_preemption_no_process_done) {
		return;
	}

	if ((FALSE == dsl_busy_sent) &&
		(E_OK == dsl_send_resp_with_status(protocol_row_entry, DCM_E_BUSYREPEATREQUEST))) {
		dsl_busy_sent = TRUE;
		runtime->external_rx_buffer_status = NOT_IN_USE;
		protocol_row_entry->dsl_protocol_rx_buffer->external_buffer_runtime_data->status = BUFFER_AVAILABLE;
	}

	DECREMENT(runtime->preempt_timeout_count);

	const dcm_dsl_protocol_row_type *active_protocol = dsl_run_time_data.active_protocol;

	dsl_handle_preemption(runtime, protocol_row_entry, active_protocol,
		(runtime->preempt_timeout_count == 0u));
}

static void dsl_handle_s3_server_timeout(const dcm_dsl_protocol_row_type *entry,
	dcm_dsl_run_time_protocol_parameters_type *runtime)
{
	if (runtime->session_control == DCM_DEFAULT_SESSION) {
		return;
	}

	if (runtime->s3server_started) {
		DECREMENT(runtime->s3server_timeout_count);
	}

	if (runtime->s3server_timeout_count == 0u) {
		dsl_change_diag_session(runtime, DCM_DEFAULT_SESSION);

		if (DCM_OBD_ON_CAN == entry->dsl_protocol_id) {
			runtime->protocol_started = FALSE;
			if ((dsl_run_time_data.active_protocol != NULL) &&
				(DCM_OBD_ON_CAN == dsl_run_time_data.active_protocol->dsl_protocol_id)) {
				dsl_run_time_data.active_protocol = NULL;
			}
		}
	}
}

static void dsl_process_pending_response(dcm_dsl_run_time_protocol_parameters_type *runtime,
	const dcm_dsl_protocol_row_type *entry)
{
	runtime->external_tx_buffer_status = DCM_TRANSMIT_SIGNALED;

#if (DCM_RESPONSE_INDICATION == STD_ON)
	if (dcm_config_ptr->dsl->dsl_diag_response_indication != NULL) {
		(void)dcm_config_ptr->dsl->dsl_diag_response_indication
		(runtime->diagnostic_response_from_dsd.SduDataPtr,
			runtime->diagnostic_response_from_dsd.SduLength);
	}
#endif

	if (E_OK != pdur_dcm_transmit(runtime->diag_response_tx_pdu_id,
		&runtime->diagnostic_response_from_dsd)) {
		runtime->s3server_timeout_count =
			entry->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
		runtime->s3server_started = TRUE;
		dsl_release_ex_rx_tx_buffers(entry);
	}
}

static void dsl_handle_tx_buffer_status(dcm_dsl_run_time_protocol_parameters_type *runtime,
	const dcm_dsl_protocol_row_type *entry)
{
	if (PROVIDED_TO_DSD == runtime->external_tx_buffer_status) {
		dsl_ex_tx_buffer_status_provided_to_dsd(runtime, entry);
	} else if (DSD_PENDING_RESPONSE_SIGNALED == runtime->external_tx_buffer_status) {
		dsl_process_pending_response(runtime, entry);
	} else if (PREEMPT_TRANSMIT_NRC == runtime->external_tx_buffer_status) {
		dsl_ex_tx_buffer_status_preempt_tx_nrc(runtime, entry);
	} else {
		/* do nothing */
	}
}

FUNC(void, DCM_CODE) dsl_main_function(void) {
	for (uint8 i = 0; i < DCM_DSL_PROTOCOL_LIST_LENGTH; i++) {
		const dcm_dsl_protocol_row_type *entry =
			&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_row_list[i];
		dcm_dsl_run_time_protocol_parameters_type *runtime =
			entry->dsl_run_time_protocol_parameters;

		dsl_handle_s3_server_timeout(entry, runtime);

		dsl_handle_tx_buffer_status(runtime, entry);
	}
}

static boolean dsl_try_use_external_buffer(dcm_dsl_run_time_protocol_parameters_type *runtime,
	const dcm_dsl_buffer_type *external_rx_buffer, PduIdType dcm_rx_pdu_id,
	PduLengthType tp_sdu_length, PduLengthType *rx_buffer_size_ptr)
{
	if ((runtime->external_rx_buffer_status == NOT_IN_USE) &&
		(external_rx_buffer->external_buffer_runtime_data->status == BUFFER_AVAILABLE))
	{
		external_rx_buffer->external_buffer_runtime_data->status = BUFFER_BUSY;
		runtime->diagnostic_request_from_tester.SduDataPtr = external_rx_buffer->pdu_info.SduDataPtr;
		runtime->diagnostic_request_from_tester.SduLength = tp_sdu_length;
		external_rx_buffer->external_buffer_runtime_data->nof_bytes_handled = 0;
		runtime->external_rx_buffer_status = PROVIDED_TO_PDUR;
		external_rx_buffer->external_buffer_runtime_data->dcm_rx_pdu_id = dcm_rx_pdu_id;
		*rx_buffer_size_ptr = external_rx_buffer->pdu_info.SduLength;
		return TRUE;
	}
	return FALSE;
}

static BufReq_ReturnType dsl_try_use_local_buffer(dcm_dsl_run_time_protocol_parameters_type *runtime,
	PduIdType dcm_rx_pdu_id, PduLengthType tp_sdu_length, PduLengthType *rx_buffer_size_ptr,
	boolean internal_request)
{
	dcm_dsl_local_buffer_type *rx_buffer = &runtime->local_rx_buffer;

	if ((rx_buffer->status != NOT_IN_USE) || (internal_request == TRUE)) {
		return BUFREQ_E_NOT_OK;
	}

	if (tp_sdu_length <= (PduLengthType)DCM_DSL_LOCAL_BUFFER_LENGTH) {
		rx_buffer->status = PROVIDED_TO_PDUR;
		rx_buffer->pdu_info.SduDataPtr = rx_buffer->buffer;
		rx_buffer->pdu_info.SduLength = tp_sdu_length;
		rx_buffer->nof_bytes_handled = 0;
		rx_buffer->dcm_rx_pdu_id = dcm_rx_pdu_id;
		*rx_buffer_size_ptr = DCM_DSL_LOCAL_BUFFER_LENGTH;
		runtime->s3server_started = FALSE;
		return BUFREQ_OK;
	}
	return BUFREQ_E_NOT_OK;
}

static BufReq_ReturnType dsl_process_reception(PduIdType dcm_rx_pdu_id, PduLengthType tp_sdu_length,
	PduLengthType *rx_buffer_size_ptr, boolean internal_request)
{
	const dcm_dsl_protocol_rx_type *protocol_rx =
		&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[dcm_rx_pdu_id];
	const dcm_dsl_protocol_row_type *protocol_row = protocol_rx->dsl_protocol_row;
	dcm_dsl_run_time_protocol_parameters_type *runtime =
		protocol_row->dsl_run_time_protocol_parameters;
	const dcm_dsl_buffer_type *external_rx_buffer = protocol_row->dsl_protocol_rx_buffer;

	if (external_rx_buffer->pdu_info.SduLength < tp_sdu_length) {
		return BUFREQ_E_OVFL;
	}

	if (dsl_try_use_external_buffer(runtime, external_rx_buffer, dcm_rx_pdu_id, tp_sdu_length,
		rx_buffer_size_ptr)) {
		runtime->s3server_started = FALSE;
		return BUFREQ_OK;
	}

	return dsl_try_use_local_buffer(runtime, dcm_rx_pdu_id, tp_sdu_length, rx_buffer_size_ptr,
		internal_request);
}

FUNC(BufReq_ReturnType, DCM_CODE) dsl_start_of_reception(PduIdType dcm_rx_pdu_id,
	PduLengthType tp_sdu_length, PduLengthType *rx_buffer_size_ptr, boolean internal_request)
{
	if (dcm_rx_pdu_id >= DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		return BUFREQ_E_NOT_OK;
	}

	DCM_ENTER_EXCLUSIVE_AREA_0();
	BufReq_ReturnType ret = dsl_process_reception(dcm_rx_pdu_id, tp_sdu_length,
		rx_buffer_size_ptr, internal_request);
	DCM_EXIT_EXCLUSIVE_AREA_0();
	return ret;
}

static boolean dsl_is_external_buffer_available
(const dcm_dsl_run_time_protocol_parameters_type *runtime, const dcm_dsl_buffer_type *ext_buf,
	PduIdType pdu_id)
{
	boolean dsl_ex_buf_avail = TRUE;

	dsl_ex_buf_avail = ((runtime->external_rx_buffer_status == PROVIDED_TO_PDUR) &&
		(ext_buf->external_buffer_runtime_data->dcm_rx_pdu_id == pdu_id));

	return dsl_ex_buf_avail;
}

static boolean dsl_is_local_buffer_available
(const dcm_dsl_local_buffer_type *local_buf, PduIdType pdu_id)
{
	boolean dsl_local_buf_avail = TRUE;

	dsl_local_buf_avail = ((local_buf->status == PROVIDED_TO_PDUR) && (local_buf->dcm_rx_pdu_id == pdu_id));

	return dsl_local_buf_avail;
}

static BufReq_ReturnType dsl_copy_to_external_buffer(uint8 *target_ptr,
	const dcm_dsl_buffer_type *ext_buf, const PduInfoType *pdu_info, PduLengthType *size_ptr)
{
	if (pdu_info->SduLength == 0) {
		*size_ptr =
			ext_buf->pdu_info.SduLength - ext_buf->external_buffer_runtime_data->nof_bytes_handled;
		return BUFREQ_OK;
	}

	if (pdu_info->SduLength
		> (ext_buf->pdu_info.SduLength - ext_buf->external_buffer_runtime_data->nof_bytes_handled)) {
		DCM_DET_REPORTERROR(DCM_DCM_COPY_RX_DATA_ID_DCM_E_INTERFACE_BUFFER_OVERFLOW);
		return BUFREQ_E_NOT_OK;
	}

	uint8 *dest = &target_ptr[ext_buf->external_buffer_runtime_data->nof_bytes_handled];

	memcpy(dest, pdu_info->SduDataPtr, pdu_info->SduLength);
	ext_buf->external_buffer_runtime_data->nof_bytes_handled += pdu_info->SduLength;

	*size_ptr = ext_buf->pdu_info.SduLength - ext_buf->external_buffer_runtime_data->nof_bytes_handled;
	return BUFREQ_OK;
}

static BufReq_ReturnType dsl_copy_to_local_buffer(dcm_dsl_local_buffer_type *local_buf,
	const PduInfoType *pdu_info, PduLengthType *size_ptr)
{
	if (pdu_info->SduLength > (DCM_DSL_LOCAL_BUFFER_LENGTH - local_buf->nof_bytes_handled)) {
		DCM_DET_REPORTERROR(DCM_DCM_COPY_RX_DATA_ID_DCM_E_INTERFACE_BUFFER_OVERFLOW);
		return BUFREQ_E_NOT_OK;
	}

	if (pdu_info->SduLength == 0) {
		*size_ptr = DCM_DSL_LOCAL_BUFFER_LENGTH - local_buf->nof_bytes_handled;
		return BUFREQ_OK;
	}

	uint8 *dest = &local_buf->pdu_info.SduDataPtr[local_buf->nof_bytes_handled];

	memcpy(dest, pdu_info->SduDataPtr, pdu_info->SduLength);
	local_buf->nof_bytes_handled += pdu_info->SduLength;

	*size_ptr = DCM_DSL_LOCAL_BUFFER_LENGTH - local_buf->nof_bytes_handled;
	return BUFREQ_OK;
}

static BufReq_ReturnType dsl_handle_buffer_copy(PduIdType dcm_rx_pdu_id,
	const PduInfoType *pdu_info_ptr, PduLengthType *rx_buffer_size_ptr)
{
	const dcm_dsl_protocol_rx_type *protocol_rx =
		&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[dcm_rx_pdu_id];
	const dcm_dsl_protocol_row_type *protocol_row = protocol_rx->dsl_protocol_row;
	dcm_dsl_run_time_protocol_parameters_type *runtime = protocol_row->dsl_run_time_protocol_parameters;
	const dcm_dsl_buffer_type *external_rx_buffer = protocol_row->dsl_protocol_rx_buffer;
	BufReq_ReturnType dsl_cp_ex_buf = BUFREQ_E_NOT_OK;
	BufReq_ReturnType dsl_cp_local_buf = BUFREQ_E_NOT_OK;

	dsl_cp_ex_buf = dsl_copy_to_external_buffer(runtime->diagnostic_request_from_tester.SduDataPtr,
		protocol_row->dsl_protocol_rx_buffer, pdu_info_ptr, rx_buffer_size_ptr);

	if (dsl_is_external_buffer_available(runtime, external_rx_buffer, dcm_rx_pdu_id)) {
		return dsl_cp_ex_buf;
	}

	dsl_cp_local_buf = dsl_copy_to_local_buffer(&runtime->local_rx_buffer, pdu_info_ptr, rx_buffer_size_ptr);
	if (dsl_is_local_buffer_available(&runtime->local_rx_buffer, dcm_rx_pdu_id)) {
		return dsl_cp_local_buf;
	}

	return BUFREQ_E_NOT_OK;
}

FUNC(BufReq_ReturnType, DCM_CODE)dsl_copy_data_to_rx_buffer(PduIdType dcm_rx_pdu_id, const PduInfoType *pdu_info_ptr,
	PduLengthType *rx_buffer_size_ptr)
{
	if (dcm_rx_pdu_id >= DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		return BUFREQ_E_NOT_OK;
	}

	DCM_ENTER_EXCLUSIVE_AREA_0();
	BufReq_ReturnType ret = dsl_handle_buffer_copy(dcm_rx_pdu_id, pdu_info_ptr, rx_buffer_size_ptr);
	DCM_EXIT_EXCLUSIVE_AREA_0();

	return ret;
}

static FUNC(void, DCM_CODE)
	dsl_do_suppress_pos_rsp_msg_ind_bit(dcm_dsl_run_time_protocol_parameters_type *rx_runtime,
		const dcm_dsl_protocol_rx_type *protocol_rx,
		const dcm_dsl_protocol_row_type *protocol_row,
		const dcm_dsl_buffer_type *external_rx_buffer)
{
	rx_runtime->s3server_timeout_count = protocol_row->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
	rx_runtime->s3server_started = TRUE;
	rx_runtime->external_rx_buffer_status = NOT_IN_USE;
	external_rx_buffer->external_buffer_runtime_data->dcm_rx_pdu_id = DCM_INVALID_PDU_ID;
	protocol_row->dsl_protocol_rx_buffer->external_buffer_runtime_data->status = BUFFER_AVAILABLE;

	if (DCM_DEFAULT_SESSION == rx_runtime->session_control) {
		if (rx_runtime->diagnostic_active_comm) {
#if (CONFIG_MODULE_COMM == 1)
			comm_dcm_inactive_diagnostic
			(dcm_config_ptr->dcm_comm_channel_cfg[protocol_rx->comm_channel_internal_index].network_handle);
			rx_runtime->diagnostic_active_comm = FALSE;
#endif
		}
	}
}

static FUNC(void, DCM_CODE) dsl_do_start_current_protocol(dcm_dsl_run_time_protocol_parameters_type *rx_runtime,
	const dcm_dsl_protocol_row_type *protocol_row,
	const dcm_dsl_main_connection_type *main_connection)
{
	Std_ReturnType higher_layer_resp = dsl_start_protocol_helper(protocol_row->dsl_protocol_id, main_connection);

	if (higher_layer_resp == E_OK) {
		dsl_change_diag_session(rx_runtime, DCM_DEFAULT_SESSION);
		rx_runtime->protocol_started = TRUE;
		dsl_run_time_data.active_protocol = protocol_row;
	}
}

static FUNC(void, DCM_CODE)
	dsl_do_check_protocol_pri_and_preemption(dcm_dsl_run_time_protocol_parameters_type *rx_runtime,
	const dcm_dsl_protocol_row_type *protocol_row,
	const dcm_dsl_main_connection_type *main_connection, PduIdType dcm_rx_pdu_id)
{
	Std_ReturnType higher_layer_resp =
		dsl_stop_protocol_helper(dsl_run_time_data.active_protocol->dsl_protocol_id, main_connection);

	if (higher_layer_resp != E_OK) {
		return;
	}

	if (dsl_run_time_data.active_protocol->dsl_run_time_protocol_parameters->external_tx_buffer_status ==
		NOT_IN_USE) {
		higher_layer_resp = dsl_start_protocol_helper(protocol_row->dsl_protocol_id, main_connection);
		if (higher_layer_resp == E_OK) {
			dsl_run_time_data.active_protocol->dsl_run_time_protocol_parameters
				->protocol_started = FALSE;
			rx_runtime->protocol_started = TRUE;
			dsl_run_time_data.active_protocol = protocol_row;
			dsl_change_diag_session(rx_runtime, DCM_DEFAULT_SESSION);
		}
	} else {
		dsl_preemption_no_process_done = TRUE;
		dsl_busy_sent = FALSE;
		if (PREEMPT_TRANSMIT_NRC != rx_runtime->external_tx_buffer_status) {
			rx_runtime->preempt_timeout_count =
				protocol_row->dsl_protocol_preempt_timeout / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
		}
		rx_runtime->external_tx_buffer_status = PREEMPT_TRANSMIT_NRC;
		rx_runtime->response_pending_count = dcm_config_ptr->dsl->dsl_diag_resp_max_num_resp_pend;
		rx_runtime->diag_reqest_rx_pdu_id = dcm_rx_pdu_id;

		(void)pdur_dcm_cancel_transmit(dsd_dsl_get_current_tx_pdu_id());
	}
}

static FUNC(void, DCM_CODE)
	dsl_do_check_and_start_current_protocol(dcm_dsl_run_time_protocol_parameters_type *rx_runtime,
		const dcm_dsl_protocol_row_type *protocol_row,
		const dcm_dsl_main_connection_type *main_connection, PduIdType dcm_rx_pdu_id)
{
	if (dsl_run_time_data.active_protocol == NULL) {
		dsl_do_start_current_protocol(rx_runtime, protocol_row, main_connection);
	} else {
		if ((protocol_row->dsl_protocol_priority <
			dsl_run_time_data.active_protocol->dsl_protocol_priority) ||
			(dsl_run_time_data.active_protocol->dsl_run_time_protocol_parameters->session_control ==
			DCM_DEFAULT_SESSION)) {
			dsl_do_check_protocol_pri_and_preemption(rx_runtime, protocol_row, main_connection,
				dcm_rx_pdu_id);
		} else {
			rx_runtime->external_rx_buffer_status = NOT_IN_USE;
			protocol_row->dsl_protocol_rx_buffer->external_buffer_runtime_data->status = BUFFER_AVAILABLE;
		}
	}
}

static FUNC(void, DCM_CODE)
	dsl_do_reception_in_parallel_to_diag_requests(dcm_dsl_run_time_protocol_parameters_type *rx_runtime,
	const dcm_dsl_protocol_row_type *protocol_row, NotifResultType result, PduIdType dcm_rx_pdu_id)
{
	if ((rx_runtime->local_rx_buffer.status != PROVIDED_TO_PDUR) ||
		(rx_runtime->local_rx_buffer.dcm_rx_pdu_id != dcm_rx_pdu_id)) {
			return;
	}

	boolean received_len_ok = ((0u == rx_runtime->local_rx_buffer.pdu_info.SduLength) ||
		(rx_runtime->local_rx_buffer.pdu_info.SduLength == rx_runtime->local_rx_buffer.nof_bytes_handled));
	if ((result != (NotifResultType)NTFRSLT_OK) || (TRUE != received_len_ok)) {
		rx_runtime->s3server_timeout_count = protocol_row->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
		rx_runtime->s3server_started = TRUE;
	} else {
		if (TRUE == dsl_is_tester_present_cmd(&(rx_runtime->local_rx_buffer.pdu_info),
			rx_runtime->local_rx_buffer.pdu_info.SduLength)) {
			rx_runtime->s3server_timeout_count = protocol_row->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
			rx_runtime->s3server_started = TRUE;
		} else {
			(void)VCOS_LOG(DCM_MODULE_ID, VCOS_LOG_ERROR, DCM_DLT_RECEPTION_PAR_DIAG_REQ,
				rx_runtime->external_rx_buffer_status,
				protocol_row->dsl_protocol_rx_buffer->external_buffer_runtime_data
				->dcm_rx_pdu_id, dcm_rx_pdu_id);
			DCM_DET_REPORTERROR(DCM_DCM_TP_RX_INDICATION_ID_DCM_E_WRONG_BUFFER);
		}
	}
	rx_runtime->local_rx_buffer.status = NOT_IN_USE;
	rx_runtime->local_rx_buffer.dcm_rx_pdu_id = DCM_INVALID_PDU_ID;
}

#if ((DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON))
static FUNC(Std_ReturnType, DCM_CODE) dsl_service_request_notification_indication(const PduInfoType *dsl_rx_buffer)
{
	Std_ReturnType result = E_OK;
	dcm_nrc_type error_code = DCM_E_POSITIVERESPONSE;
	PduIdType rx_pdu_id = 0u;
	uint16 source_address = 0u;
	uint16 rx_connection_id = 0u;
	dcm_protocol_addr_type_type req_type;
	dcm_protocol_type protocol_id;

	if (E_OK != dsl_get_rx_connection_params(rx_pdu_id, &source_address,
		&req_type, &protocol_id, &rx_connection_id)) {
		result = E_NOT_OK;
		return result;
	}

	const dcm_dsl_service_request_notification_type *service_request_notification =
		dcm_config_ptr->dsl->dsl_service_request_notification;

	if (dsl_rx_buffer->SduLength <= 0u) {
		result = E_NOT_OK;
		return result;
	}
	for (uint8 index = 0; service_request_notification[index].indication != NULL; index++) {
		result = service_request_notification[index].indication(dsl_rx_buffer->SduDataPtr[0],
			&(dsl_rx_buffer->SduDataPtr[1]), (uint16)(dsl_rx_buffer->SduLength - 1u),
			req_type, rx_connection_id, &error_code, protocol_id, source_address);
		if (result != E_OK) {
			break;
		}
	}
	return result;
}
#endif

FUNC(void, DCM_CODE)
dsl_tp_rx_indication_from_pdur(PduIdType dcm_rx_pdu_id, NotifResultType result, boolean internal_request,
	boolean startup_response_request)
{
	if (dcm_rx_pdu_id >= DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		return;
	}

	const dcm_dsl_protocol_rx_type *protocol_rx =
		&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[dcm_rx_pdu_id];
	const dcm_dsl_main_connection_type *main_connection = protocol_rx->dsl_main_connection_parent;
	const dcm_dsl_protocol_row_type *protocol_row = protocol_rx->dsl_protocol_row;
	dcm_dsl_run_time_protocol_parameters_type *runtime = protocol_row->dsl_run_time_protocol_parameters;
	const dcm_dsl_buffer_type *external_rx_buffer = protocol_row->dsl_protocol_rx_buffer;

	DCM_ENTER_EXCLUSIVE_AREA_0();

	if ((runtime->external_rx_buffer_status != PROVIDED_TO_PDUR) ||
	    (external_rx_buffer->external_buffer_runtime_data->dcm_rx_pdu_id != dcm_rx_pdu_id)) {
		dsl_do_reception_in_parallel_to_diag_requests(runtime, protocol_row, result, dcm_rx_pdu_id);

		DCM_EXIT_EXCLUSIVE_AREA_0();
		return;
	}

	boolean received_len_ok =
		(external_rx_buffer->external_buffer_runtime_data->nof_bytes_handled ==
			runtime->diagnostic_request_from_tester.SduLength) ||
		(0u == runtime->diagnostic_request_from_tester.SduLength);
	if ((result == (NotifResultType)NTFRSLT_OK) && ((boolean)TRUE == received_len_ok)) {
		if (dsl_is_tester_present_cmd(&(protocol_row->dsl_protocol_rx_buffer->pdu_info),
			runtime->diagnostic_request_from_tester.SduLength)) {
#if ((DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON))
			(void)dsl_service_request_notification_indication(
				&(protocol_row->dsl_protocol_rx_buffer->pdu_info));
#endif
			dsl_do_suppress_pos_rsp_msg_ind_bit(runtime, protocol_rx, protocol_row,
				external_rx_buffer);
		} else {
			if (FALSE == runtime->protocol_started) {
				dsl_do_check_and_start_current_protocol(runtime, protocol_row,
					main_connection, dcm_rx_pdu_id);
			}
			if (TRUE == runtime->protocol_started) {
				if ((DCM_DEFAULT_SESSION == runtime->session_control) && (FALSE == runtime->diagnostic_active_comm)) {
#if (CONFIG_MODULE_COMM == 1)
					comm_dcm_active_diagnostic(dcm_config_ptr->dcm_comm_channel_cfg
						[protocol_rx->comm_channel_internal_index].network_handle);
#endif
					runtime->diagnostic_active_comm = TRUE;
				}
				if (0u == runtime->diagnostic_request_from_tester.SduLength) {
					runtime->diagnostic_request_from_tester.SduLength =
						external_rx_buffer->external_buffer_runtime_data->nof_bytes_handled;
				}
				const dcm_dsp_session_type *session = dsl_get_active_session(runtime->session_control);

				runtime->state_timeout_count = dsl_adjust_p2time_to_count(session->dsp_session_p2server_max,
					protocol_row->tim_str_p2server_adjust);
				runtime->external_rx_buffer_status = PROVIDED_TO_DSD;
				runtime->external_tx_buffer_status = PROVIDED_TO_DSD;
				runtime->diag_response_tx_pdu_id =
					main_connection->dsl_protocol_tx->dcm_dsl_protocol_tx_pdu_id;
				runtime->diag_reqest_rx_pdu_id = dcm_rx_pdu_id;
				runtime->response_pending_count =
					dcm_config_ptr->dsl->dsl_diag_resp_max_num_resp_pend;
				runtime->diagnostic_response_from_dsd =
					protocol_row->dsl_protocol_tx_buffer->pdu_info;
				dsd_dsl_data_indication(&(runtime->diagnostic_request_from_tester),
					protocol_row, protocol_rx->dsl_protocol_addr_type, runtime->diag_response_tx_pdu_id,
					&(runtime->diagnostic_response_from_dsd), dcm_rx_pdu_id, internal_request,
					startup_response_request);
			}
		}
	} else {
		runtime->external_rx_buffer_status = NOT_IN_USE;
		external_rx_buffer->external_buffer_runtime_data->dcm_rx_pdu_id = DCM_INVALID_PDU_ID;
		protocol_row->dsl_protocol_rx_buffer->external_buffer_runtime_data->status = BUFFER_AVAILABLE;
	}

	DCM_EXIT_EXCLUSIVE_AREA_0();
}

static FUNC(BufReq_ReturnType, DCM_CODE)
	dsl_do_copy_tx_data_check_ex_tx_buffer(dcm_dsl_run_time_protocol_parameters_type *runtime,
		const dcm_dsl_buffer_type *external_tx_buffer)
{
	if (DCM_TRANSMIT_SIGNALED == runtime->external_tx_buffer_status) {
		external_tx_buffer->external_buffer_runtime_data->nof_bytes_handled = 0;
		runtime->external_tx_buffer_status = PROVIDED_TO_PDUR;
		return BUFREQ_OK;
	} else if (PROVIDED_TO_PDUR == runtime->external_tx_buffer_status) {
		return BUFREQ_OK;
	} else {
		return BUFREQ_E_NOT_OK;
	}
}

static FUNC(BufReq_ReturnType, DCM_CODE)
	dsl_do_copy_tx_data_check_local_tx_buffer(dcm_dsl_run_time_protocol_parameters_type *runtime)
{
	dcm_dsl_local_buffer_type *tx_buffer = &runtime->local_tx_buffer;

	if (DCM_TRANSMIT_SIGNALED == tx_buffer->status) {
		tx_buffer->pdu_info.SduDataPtr = tx_buffer->buffer;
		tx_buffer->nof_bytes_handled = 0;
		tx_buffer->status = PROVIDED_TO_PDUR;
		return BUFREQ_OK;
	} else if (PROVIDED_TO_PDUR == tx_buffer->status) {
		return BUFREQ_OK;
	} else {
		return BUFREQ_E_NOT_OK;
	}
}

FUNC(BufReq_ReturnType, DCM_CODE) dsl_copy_tx_data(PduIdType dcm_tx_pdu_id, const PduInfoType *pdu_info_ptr,
	const RetryInfoType *retry, PduLengthType *tx_data_cnt_ptr)
{
	BufReq_ReturnType ret = BUFREQ_E_NOT_OK;
	(void)retry;

	const dcm_dsl_protocol_row_type *protocol_row =
		dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_tx_global_list[dcm_tx_pdu_id].dsl_protocol_row;
	dcm_dsl_run_time_protocol_parameters_type *runtime = protocol_row->dsl_run_time_protocol_parameters;
	dcm_dsl_local_buffer_type *tx_buffer = &runtime->local_tx_buffer;
	const dcm_dsl_buffer_type *external_tx_buffer = protocol_row->dsl_protocol_tx_buffer;

	ret = dsl_do_copy_tx_data_check_ex_tx_buffer(runtime, external_tx_buffer);
	switch (ret) {
		case BUFREQ_OK:
			if (0u == pdu_info_ptr->SduLength) {
			} else if (pdu_info_ptr->SduLength >
					(runtime->diagnostic_response_from_dsd.SduLength -
					external_tx_buffer->external_buffer_runtime_data->nof_bytes_handled)) {
				ret = BUFREQ_E_NOT_OK;
			} else {
				(void)memcpy(pdu_info_ptr->SduDataPtr,
					&runtime->diagnostic_response_from_dsd.SduDataPtr
					[external_tx_buffer->external_buffer_runtime_data->nof_bytes_handled],
					pdu_info_ptr->SduLength);
				external_tx_buffer->external_buffer_runtime_data->nof_bytes_handled += pdu_info_ptr->SduLength;
			}
			*tx_data_cnt_ptr = runtime->diagnostic_response_from_dsd.SduLength -
				external_tx_buffer->external_buffer_runtime_data->nof_bytes_handled;
			break;
		case BUFREQ_E_NOT_OK:
			ret = dsl_do_copy_tx_data_check_local_tx_buffer(runtime);
			if (BUFREQ_OK != ret) {
				break;
			}
			if (0u == pdu_info_ptr->SduLength) {
			} else if (pdu_info_ptr->SduLength > (tx_buffer->pdu_info.SduLength - tx_buffer->nof_bytes_handled)) {
				ret = BUFREQ_E_NOT_OK;
			} else {
				(void)memcpy(pdu_info_ptr->SduDataPtr, &tx_buffer->pdu_info
					.SduDataPtr[tx_buffer->nof_bytes_handled], pdu_info_ptr->SduLength);
				tx_buffer->nof_bytes_handled += pdu_info_ptr->SduLength;
			}
			*tx_data_cnt_ptr = tx_buffer->pdu_info.SduLength - tx_buffer->nof_bytes_handled;
			break;
		default:
			break;
	}

	return ret;
}

static FUNC(void, DCM_CODE) dsl_do_local_tx_buffer(const dcm_dsl_protocol_rx_type *protocol_rx,
	dcm_dsl_run_time_protocol_parameters_type *runtime, PduIdType dcm_tx_pdu_id, Std_ReturnType result)
{
	dcm_dsl_local_buffer_type *tx_buffer = &runtime->local_tx_buffer;

	if ((DCM_TRANSMIT_SIGNALED != tx_buffer->status) && (PROVIDED_TO_PDUR != tx_buffer->status)) {
		return;
	}

	if (DCM_E_RESPONSEPENDING != tx_buffer->response_code) {
		if (DCM_DEFAULT_SESSION == runtime->session_control) {
			if (runtime->diagnostic_active_comm) {
#if (CONFIG_MODULE_COMM == 1)
				comm_dcm_inactive_diagnostic(dcm_config_ptr
					->dcm_comm_channel_cfg[protocol_rx->comm_channel_internal_index].network_handle);
#endif
				runtime->diagnostic_active_comm = FALSE;
			}
		}
		tx_buffer->status = NOT_IN_USE;
		return;
	}
#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
	dsd_response_pending_confirmed(dcm_tx_pdu_id, result);
#endif
	tx_buffer->status = NOT_IN_USE;
}

FUNC(void, DCM_CODE) dsl_tp_tx_confirmation(PduIdType dcm_tx_pdu_id, Std_ReturnType result)
{
	const dcm_dsl_protocol_tx_type *protocol_tx =
		&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_tx_global_list[dcm_tx_pdu_id];
	const dcm_dsl_protocol_row_type *protocol_row = protocol_tx->dsl_protocol_row;
	dcm_dsl_run_time_protocol_parameters_type *runtime = protocol_row->dsl_run_time_protocol_parameters;
	Std_ReturnType ret = result;

	if (runtime->diag_reqest_rx_pdu_id < DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		const dcm_dsl_protocol_rx_type *protocol_rx =
			&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[runtime->diag_reqest_rx_pdu_id];
		boolean external_buffer_released = FALSE;

		switch (runtime->external_tx_buffer_status) { // 释放 EXTERNAL TX BUFFER
			case DCM_TRANSMIT_SIGNALED:
				if (result == E_OK) {
					ret = E_NOT_OK;
				}
				/* 无需break  继续执行 */
			case PROVIDED_TO_PDUR:
				if (DCM_DEFAULT_SESSION == runtime->session_control) {
					if (runtime->diagnostic_active_comm) {
#if (CONFIG_MODULE_COMM == 1)
						comm_dcm_inactive_diagnostic(dcm_config_ptr->dcm_comm_channel_cfg
							[protocol_rx->comm_channel_internal_index].network_handle);
#endif
						runtime->diagnostic_active_comm = FALSE;
					}
				}
				external_buffer_released = TRUE;
				dsl_release_ex_rx_tx_buffers(protocol_row);
				runtime->s3server_timeout_count = protocol_row->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
				runtime->s3server_started = TRUE;
				dsd_data_confirmation(protocol_tx->dcm_dsl_protocol_tx_pdu_id, ret);
				break;
			default:
				break;
		}

		if (!external_buffer_released) {
			dsl_do_local_tx_buffer(protocol_rx, runtime, dcm_tx_pdu_id, ret);
		}
	}
}

#if (DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON) ||                              \
	(defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
FUNC(Std_ReturnType, DCM_CODE)
dsl_get_rx_connection_params(PduIdType rx_pdu_id, uint16 *source_address, dcm_protocol_addr_type_type *req_type,
	dcm_protocol_type *protocol_id, uint16 *connection_id)
{
	Std_ReturnType ret = E_NOT_OK;

	if (rx_pdu_id < DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		const dcm_dsl_protocol_rx_type *protocol_rx =
			&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[rx_pdu_id];
		const dcm_dsl_main_connection_type *main_connection = protocol_rx->dsl_main_connection_parent;
		const dcm_dsl_protocol_row_type *protocol_row = protocol_rx->dsl_protocol_row;
		*source_address = main_connection->dsl_rx_tester_source_address;
		*connection_id = main_connection->connection_id;
		*req_type = protocol_rx->dsl_protocol_addr_type;
		*protocol_id = protocol_row->dsl_protocol_id;
		ret = E_OK;
	}
	return ret;
}

#endif

static FUNC(boolean, DCM_CODE) dsl_find_phys_rx_protocol(uint8 protocol_id, uint16 tester_source_address,
	const dcm_dsl_protocol_row_type **protocol_row,
	const dcm_dsl_protocol_rx_type **protocol_phys_rx)
{
	boolean found = FALSE;
	const dcm_dsl_protocol_row_type *protocol_row_entry = NULL;
	const dcm_dsl_main_connection_type *connection = NULL;

	for (uint8 i = 0; i < DCM_DSL_PROTOCOL_LIST_LENGTH; i++) {
		protocol_row_entry = &dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_row_list[i];
		if (protocol_id != protocol_row_entry->dsl_protocol_id) {
			continue;
		}
		for (uint8 j = 0; j < protocol_row_entry->connection_count; j++) {
			connection = &protocol_row_entry->dsl_main_connections[j];
			if (tester_source_address != connection->dsl_rx_tester_source_address) {
				continue;
			}
			*protocol_row = protocol_row_entry;
			*protocol_phys_rx = connection->dsl_physical_protocol_rx;
			found = TRUE;
			break;
		}
	}
	return found;
}

FUNC(Std_ReturnType, DCM_CODE)
dsl_dsp_silently_start_protocol(uint8 session, uint8 protocol_id, uint16 tester_source_address,
	boolean request_full_comm)
{
	const dcm_dsl_protocol_rx_type *protocol_phys_rx = NULL;
	const dcm_dsl_protocol_row_type *rx_protocol_row = NULL;

	if ((boolean)FALSE == dsl_find_phys_rx_protocol(protocol_id, tester_source_address, &rx_protocol_row,
		&protocol_phys_rx)) {
		return E_NOT_OK;
	}
	if ((boolean)FALSE == dsp_dsl_check_session_supported(session)) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_INTEGRATION_ERROR);
	} else {
		rx_protocol_row->dsl_run_time_protocol_parameters->protocol_started = TRUE;
		rx_protocol_row->dsl_run_time_protocol_parameters->session_control = session;
		rx_protocol_row->dsl_run_time_protocol_parameters->security_level = DCM_SEC_LEV_LOCKED;
		dsl_run_time_data.active_protocol = rx_protocol_row;
		if (DCM_DEFAULT_SESSION != session) {
			rx_protocol_row->dsl_run_time_protocol_parameters->s3server_timeout_count =
				rx_protocol_row->tim_str_s3server / DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
			rx_protocol_row->dsl_run_time_protocol_parameters->s3server_started = TRUE;
		}
	}
	if ((boolean)TRUE == request_full_comm) {
#if (CONFIG_MODULE_COMM == 1)
		comm_dcm_active_diagnostic
		(dcm_config_ptr->dcm_comm_channel_cfg[protocol_phys_rx->comm_channel_internal_index]
				.network_handle);
		rx_protocol_row->dsl_run_time_protocol_parameters->diagnostic_active_comm = TRUE;
#endif
	}
	return E_OK;
}

#if (CONFIG_MODULE_COMM == 1)
FUNC(void, DCM_CODE) dsl_com_mode_entered(uint8 network, dcm_com_mode_type com_mode)
{
	for (uint8 i = 0; i < DCM_NOF_COMM_CHANNELS; i++) {
		if (network == dcm_config_ptr->dcm_comm_channel_cfg[i].network_handle) {
			dsl_net_com_mode[i] = com_mode;
			break;
		}
	}
}

static boolean dsl_check_communication_mode(const dcm_dsl_protocol_rx_type *protocol)
{
	boolean check_mode_result = FALSE;

	check_mode_result = (DCM_FULL_COM == dsl_net_com_mode[protocol->comm_channel_internal_index]);

	return check_mode_result;
}
#endif

static Std_ReturnType dsl_process_startup_request(const dcm_dsl_protocol_rx_type *protocol,
	uint8 sid, uint8 sub_fnc)
{
	PduLengthType rx_buffer_size;

	if (BUFREQ_OK != dsl_start_of_reception(protocol->dcm_dsl_protocol_rx_pdu_id, DCM_DSL_TP_SDU_DEFAULT_LEN,
		&rx_buffer_size, TRUE)) {
		return DCM_E_PENDING;
	}

	uint8 data[DCM_DSL_TP_SDU_DEFAULT_LEN];
	PduInfoType request;

	request.SduLength = DCM_DSL_TP_SDU_DEFAULT_LEN;
	request.SduDataPtr = data;
	request.SduDataPtr[0] = sid;
	request.SduDataPtr[1] = sub_fnc;

	boolean copy_success = (BUFREQ_OK == dsl_copy_data_to_rx_buffer
		(protocol->dcm_dsl_protocol_rx_pdu_id, &request, &rx_buffer_size));

	NotifResultType dsl_rx_ind_result = copy_success ? NTFRSLT_OK : NTFRSLT_E_NOT_OK;

	dsl_tp_rx_indication_from_pdur(protocol->dcm_dsl_protocol_rx_pdu_id, dsl_rx_ind_result, TRUE, TRUE);

	Std_ReturnType dsl_copy_result = copy_success ? E_OK : DCM_E_PENDING;

	return dsl_copy_result;
}

FUNC(Std_ReturnType, DCM_CODE)dsl_dsp_response_on_startup_request(uint8 sid,
	uint8 sub_fnc, uint8 protocol_id, uint16 tester_source_address)
{
	const dcm_dsl_protocol_rx_type *protocol_phys_rx = NULL;
	const dcm_dsl_protocol_row_type *rx_protocol_row = NULL;

	if (FALSE == dsl_find_phys_rx_protocol(protocol_id, tester_source_address, &rx_protocol_row, &protocol_phys_rx)) {
		return E_NOT_OK;
	}

#if (CONFIG_MODULE_COMM == 1)
	if (!dsl_check_communication_mode(protocol_phys_rx)) {
		return DCM_E_PENDING;
	}
#endif
	Std_ReturnType dsl_startup_req = E_NOT_OK;

	dsl_startup_req = dsl_process_startup_request(protocol_phys_rx, sid, sub_fnc);

	return dsl_startup_req;
}

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"
