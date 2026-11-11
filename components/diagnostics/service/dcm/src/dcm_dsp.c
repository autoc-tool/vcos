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
#if (CONFIG_SERVM_BSWM == 1)
#include "bswm_bsw.h"
#endif
#if (CONFIG_MODULE_NVM == 1)
#include "nvm.h"
#endif

#define DCM_START_SEC_PRIVATE_BSS
#include "dcm_memmap.h"
static VAR(dsp_uds_ecu_reset_data_type, DCM_PRIVATE_BSS) dsp_ecu_reset_data;
static VAR(dsp_uds_session_control_data_type, DCM_PRIVATE_BSS) dsp_session_ctrl_data;
static VAR(dsp_uds_read_did_pending_type, DCM_PRIVATE_BSS) dsp_read_did_pending;

#if (DCM_USE_SERVICE_WRITEDATABYIDENTIFIER == STD_ON)
static VAR(dsp_uds_general_pending_type, DCM_PRIVATE_BSS) dsp_write_did_pending;
#endif

static VAR(dsp_uds_general_pending_type, DCM_PRIVATE_BSS) dsp_routine_control_pending;
#if (DCM_RESPONSE_MULTIPLE_PENDING_ENABLED == STD_ON)
static VAR(dsp_dsc_programming_pending_type, DCM_PRIVATE_BSS) dsp_dsc_programming_pending;
#endif
#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
static VAR(dsp_obd_get_infotype_value_pending_type, DCM_PRIVATE_BSS) dsp_uds_infotype_services_pending;
#endif

static VAR(dsp_uds_general_pending_type, DCM_PRIVATE_BSS) dsp_security_access_pending;
#ifdef DCM_USE_UPLOAD_DOWNLOAD
static VAR(dsp_uds_general_pending_type, DCM_PRIVATE_BSS) dsp_upload_download_pending;
static VAR(dcm_transfer_status_type, DCM_PRIVATE_BSS) dsp_transfer_status;
#endif

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
static VAR(dsp_uds_comm_ctrl_data_type, DCM_PRIVATE_BSS) dsp_comm_ctrl_data;
static VAR(dcm_comm_mode_type, DCM_PRIVATE_BSS) dsp_channel_com_mode[DCM_NOF_COMM_CHANNELS];
#endif

static VAR(dsp_uds_security_access_data_type, DCM_PRIVATE_BSS) dsp_security_acces_data;

#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON) || (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
static VAR(dcm_dsp_memory_state_type, DCM_PRIVATE_BSS) dsp_mem_state;
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON)
static VAR(dcm_dsp_io_ctrl_state_type, DCM_PRIVATE_BSS) dsp_io_ctrl_state_list[DCM_NOF_IOCONTROL_DIDS];
static VAR(dsp_uds_io_ctrl_pending_type, DCM_PRIVATE_BSS) dsp_io_ctrl_data;
#endif

#if (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
static VAR(dcm_dsp_ctrl_dtc_set_type, DCM_PRIVATE_BSS) dsp_dtc_setting;
#endif

static VAR(dcm_prog_conditions_type, DCM_PRIVATE_BSS) dsp_prog_conditions;
static VAR(general_pend_state_type, DCM_PRIVATE_BSS) dsp_prog_condition_stup_resp_state;

VAR(boolean, DCM_PRIVATE_BSS) global_compare_key_flag;
VAR(dcm_sec_lv_type, DCM_PRIVATE_BSS) global_requested_security_level;
#define DCM_STOP_SEC_PRIVATE_BSS
#include "dcm_memmap.h"

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

static FUNC(void, DCM_CODE) dsp_cancel_pending_did(uint16 did_nr, uint16 signal_index,
	read_did_pending_state_type pending_state, const PduInfoType *pdu_tx_data);
static FUNC(dcm_nrc_type, DCM_CODE) dsp_uds_security_access_compare_key_sub_fnc(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data, dcm_sec_lv_type requested_security_level, boolean is_cancel);
static FUNC(void, DCM_CODE) dsp_cancel_pending_routine(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
static FUNC(dcm_nrc_type, DCM_CODE) dsp_uds_security_access_get_seed_sub_fnc(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data, dcm_sec_lv_type requested_security_level, boolean is_cancel);
static FUNC(void, DCM_CODE) dsp_cancel_pending_security_access(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data);
static FUNC(void, DCM_CODE) dsp_init_io_ctrl_data(dcm_op_status_type *op_status);
#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && defined(DCM_USE_CONTROL_DIDS)
static FUNC(void, DCM_CODE) dsp_stop_io_ctrl(boolean check_session_and_sec_level);
static FUNC(void, DCM_CODE)
	dsp_io_ctrl_stop_activated(uint16 did_nr, const dcm_dsp_io_ctrl_vector signal_activated);
static FUNC(Std_ReturnType, DCM_CODE)
	dsp_function_input_output_ctrl(const dcm_dsp_data_type *data_ptr, dcm_io_ctrl_para_type action,
		dcm_op_status_type op_status, uint8 *control_option_record, dcm_nrc_type *response_code);
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
static FUNC(void, DCM_CODE) dsp_enable_dtc_setting(uint8 client_id);
#endif

static FUNC(void, DCM_CODE) dsp_set_op_status_depend_on_state(const dsp_uds_general_pending_type *p_general_pending,
	dcm_op_status_type *op_status, boolean is_cancel);

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_internal_comm_ctrl(uint8 sub_func, uint8 sub_net, uint8 com_type, PduIdType rx_pdu_id,
		boolean execute_mode_switch);
#endif

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
static FUNC(void, DCM_CODE) dsp_stop_communication_control(boolean check_session_and_sec_level)
{
	if ((FALSE == check_session_and_sec_level) ||
		(FALSE == dsd_dsp_check_service_supported_in_active_session_and_security(DCM_SID_COMMUNICATION_CONTROL))) {
		for (uint8 i = 0; i < DCM_NOF_COMM_CHANNELS; i++) {
			if (DCM_ENABLE_RX_TX_NORM_NM != dsp_channel_com_mode[i]) {
				if (dcm_config_ptr->dcm_comm_channel_cfg[i].mode_switch_fnc != NULL) {
					(void)dcm_config_ptr->dcm_comm_channel_cfg[i].mode_switch_fnc
					(RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NORM_NM);
				}
#if (CONFIG_SERVM_BSWM == 1)
				bswm_dcm_communication_mode_current_state
				(dcm_config_ptr->dcm_comm_channel_cfg[i].network_handle, DCM_ENABLE_RX_TX_NORM_NM);
#endif
				dsp_channel_com_mode[i] = DCM_ENABLE_RX_TX_NORM_NM;
			}
		}
	}
}
#endif

FUNC(void, DCM_CODE) dsp_reset_diag_activity_on_session_change(void)
{
#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && defined(DCM_USE_CONTROL_DIDS)
	dsp_stop_io_ctrl(TRUE);
	dsp_io_ctrl_data.state = DCM_GENERAL_IDLE;
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
	dcm_protocol_type protocol_id = dsp_session_ctrl_data.protocol_id;
	uint8 client_id = 0;

	for (uint8 i = 0; i < DCM_DSL_PROTOCOL_LIST_LENGTH; i++) {
		if (protocol_id == dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_row_list[i].dsl_protocol_id) {
			client_id = dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_row_list[i].dcm_dem_client_id;
			break;
		}
	}
	dsp_enable_dtc_setting(client_id);
#endif
#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
	dsp_stop_communication_control(TRUE);
#endif
	dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_IDLE;
	dsp_session_ctrl_data.pending_session_change = FALSE;
	dsp_prog_condition_stup_resp_state = DCM_GENERAL_IDLE;
}

FUNC(void, DCM_CODE) dcm_dsp_reset_diag_activity(const dcm_dsl_protocol_row_type *protocol_row_entry)
{
#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && defined(DCM_USE_CONTROL_DIDS)
	dsp_stop_io_ctrl(FALSE);
	dsp_io_ctrl_data.state = DCM_GENERAL_IDLE;
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
	dsp_enable_dtc_setting(protocol_row_entry->dcm_dem_client_id);
#endif

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
	dsp_stop_communication_control(FALSE);
#endif
	dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_IDLE;
	dsp_session_ctrl_data.pending_session_change = FALSE;
	dsp_prog_condition_stup_resp_state = DCM_GENERAL_IDLE;
}

static void dsp_init_common_data(void)
{
	dsp_security_acces_data.req_in_progress = FALSE;
	dsp_security_acces_data.curr_sec_lev_idx = 0;
	dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_NO_RESET;
	dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_IDLE;
	dsp_session_ctrl_data.pending_session_change = FALSE;
	dsp_prog_condition_stup_resp_state = DCM_GENERAL_IDLE;
	global_requested_security_level = 0u;
	global_compare_key_flag = FALSE;

#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON) || (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
	dsp_mem_state = DCM_MEMORY_UNUSED;
#endif

#ifdef DCM_USE_UPLOAD_DOWNLOAD
	dsp_transfer_status.file_transfer_type = DCM_NO_FILE_TRANSFER;
	dsp_transfer_status.transfer_type = DCM_NO_DATA_TRANSFER;
	dsp_transfer_status.first_block_received = FALSE;
	dsp_transfer_status.block_sequence_counter = 0;
	dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
#endif

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
	dsp_comm_ctrl_data.com_ctrl_pending = FALSE;
#endif
}

static void dsp_set_delay_timer(uint8 index, uint8 attempts)
{
	const dcm_dsp_security_type *security = &dcm_config_ptr->dsp->dsp_security[index];

	if ((attempts > 0u) && (attempts == security->dsp_security_num_att_delay)) {
		dsp_security_acces_data.sec_false_attempt_chk[index].timer_sec_acss_attempt =
			security->dsp_security_delay_time;
		dsp_security_acces_data.sec_false_attempt_chk[index].start_delay_timer =
			DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE;
	} else if (security->dsp_security_delay_time_on_boot >= DCM_MAIN_FUNCTION_PERIOD_TIME_MS) {
		dsp_security_acces_data.sec_false_attempt_chk[index].timer_sec_acss_attempt =
			security->dsp_security_delay_time_on_boot;
		dsp_security_acces_data.sec_false_attempt_chk[index].start_delay_timer =
			DELAY_TIMER_ON_BOOT_ACTIVE;
	} else {
		dsp_security_acces_data.sec_false_attempt_chk[index].start_delay_timer =
			DELAY_TIMER_DEACTIVE;
	}
}

static void dsp_init_first_call_data(void)
{
#if (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
	dsp_dtc_setting.setting_disabled = FALSE;
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON)
	(void)memset(dsp_io_ctrl_state_list, 0, sizeof(dsp_io_ctrl_state_list));
	dsp_io_ctrl_data.state = DCM_GENERAL_IDLE;
#endif

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
	(void)memset(dsp_channel_com_mode, (uint8)(DCM_ENABLE_RX_TX_NORM_NM),
		sizeof(dsp_channel_com_mode));
#endif
}

static void dsp_init_security_parameters(void)
{
	uint8 index = 0;

	while (index < DCM_SECURITY_EOL_INDEX) {
		uint8 attempts = 0;
#if (DCM_SEC_ATT_CNTR_ENABLED == STD_ON)
		if (dcm_config_ptr->dsp->dsp_security[index].get_attempt_counter != NULL) {
			(void)dcm_config_ptr->dsp->dsp_security[index].get_attempt_counter(DCM_INITIAL, &attempts);
		}
#endif

		dsp_security_acces_data.sec_false_attempt_chk[index].sec_acss_attempts = attempts;

		dsp_set_delay_timer(index, attempts);

		index++;
	}
}

FUNC(void, DCM_CODE) dsp_init(boolean first_call)
{
	dsp_init_common_data();

	if (first_call) {
		dsp_init_first_call_data();
	}

	dsp_init_security_parameters();
}

FUNC(void, DCM_CODE) dsp_check_protocol_start_requests(void)
{
	if (DCM_WARM_START != dcm_get_prog_conditions(&dsp_prog_conditions)) {
		return;
	}

#if (CONFIG_SERVM_BSWM == 1)
	if (dsp_prog_conditions.appl_updated) {
		bswm_dcm_application_updated();
	}
#endif
	dsp_prog_conditions.appl_updated = FALSE;
	if (((uint8)DCM_SID_DIAGNOSTIC_SESSION_CONTROL == dsp_prog_conditions.sid) ||
		((uint8)DCM_SID_ECU_RESET == dsp_prog_conditions.sid)) {
		uint8 session = ((uint8)DCM_SID_DIAGNOSTIC_SESSION_CONTROL == dsp_prog_conditions.sid) ?
			dsp_prog_conditions.sub_fnc_id : DCM_DEFAULT_SESSION;
		Std_ReturnType ret =
			dsl_dsp_silently_start_protocol(session, dsp_prog_conditions.protocol_id,
				dsp_prog_conditions.tester_source_add,
				dsp_prog_conditions.response_required);
		if ((E_OK == ret) && (dsp_prog_conditions.response_required)) {
			dsp_prog_condition_stup_resp_state = DCM_GENERAL_PENDING;
		} else {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		}
	}
}

static FUNC(Std_ReturnType, DCM_CODE) dsp_get_reset_rte_mode(dcm_ecu_rst_type reset_type, uint8 *rte_mode)
{
	Std_ReturnType ret = E_OK;

	if (DCM_HARD_RESET == reset_type) {
		*rte_mode = RTE_MODE_DcmEcuReset_HARD;
	} else if (DCM_SOFT_RESET == reset_type) {
		*rte_mode = RTE_MODE_DcmEcuReset_SOFT;
	} else if (DCM_KEY_OFF_ON_RESET == reset_type) {
		*rte_mode = RTE_MODE_DcmEcuReset_KEYONOFF;
	} else {
		ret = E_NOT_OK;
	}

	return ret;
}

static boolean dsp_should_process_reset(void)
{
	return (dsp_ecu_reset_data.reset_pending == DCM_DSP_RESET_PENDING);
}

static Std_ReturnType dsp_execute_ecu_reset(uint8 rte_mode)
{
#ifdef DCM_CALLBACK_ECURESET_ENABLED
	return dcm_callback_dcm_ecu_reset(rte_mode);
#else
#if (CONFIG_MODULE_RTE == 1)
	return Rte_Switch_EcuResetModeSwitchInterface_DcmEcuReset(rte_mode);
#else
	return E_NOT_OK;
#endif
#endif
}

static void dsp_handle_reset_result(Std_ReturnType result)
{
	if (E_OK == result) {
		dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_WAIT_TX_CONF;
		dsp_ecu_reset_data.pdu_tx_data->SduDataPtr[1] = dsp_ecu_reset_data.reset_type;
		dsp_ecu_reset_data.pdu_tx_data->SduLength = 2;
		dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
	} else if (DCM_E_PENDING == result) {
		dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_PENDING;
	} else {
		dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_NO_RESET;
		dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
	}
}

static FUNC(void, DCM_CODE) dsp_reset_main_function(void)
{
	if (!dsp_should_process_reset()) {
		return;
	}

	uint8 rte_mode;

	if (E_OK != dsp_get_reset_rte_mode(dsp_ecu_reset_data.reset_type, &rte_mode)) {
		return;
	}

	const Std_ReturnType result = dsp_execute_ecu_reset(rte_mode);

	dsp_handle_reset_result(result);
}

#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON) || (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
static FUNC(void, DCM_CODE) dsp_memory_main_function(void)
{
	dcm_return_write_memory_type write_ret;
	dcm_return_read_memory_type read_ret;

	switch (dsp_mem_state) {
		case DCM_MEMORY_UNUSED:
			break;
		case DCM_MEMORY_READ:
			read_ret = dcm_read_memory(DCM_PENDING, 0, 0, 0, NULL);
			switch (read_ret) {
				case DCM_READ_OK:
					dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
					dsp_mem_state = DCM_MEMORY_UNUSED;
					break;
				case DCM_READ_FAILED:
					dsd_dsp_processing_done(DCM_E_GENERALPROGRAMMINGFAILURE);
					dsp_mem_state = DCM_MEMORY_UNUSED;
					break;
				default:
					break;
			}
			break;
		case DCM_MEMORY_WRITE:
			write_ret = dcm_write_memory(DCM_PENDING, 0, 0, 0, NULL);
			switch (write_ret) {
				case DCM_WRITE_OK:
					dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
					dsp_mem_state = DCM_MEMORY_UNUSED;
					break;
				case DCM_WRITE_FAILED:
					dsd_dsp_processing_done(DCM_E_GENERALPROGRAMMINGFAILURE);
					dsp_mem_state = DCM_MEMORY_UNUSED;
					break;
				default:
					break;
			}
			break;

		default:
			/* do nothing */
			break;
	}
}
#endif

static FUNC(void, DCM_CODE) dsp_read_did_main_function(void)
{
#if (DCM_USE_SERVICE_READDATABYIDENTIFIER == STD_ON)
	if (DCM_READ_DID_IDLE != dsp_read_did_pending.state) {
		dsp_uds_read_data_by_id(dsp_read_did_pending.pdu_rx_data,
			dsp_read_did_pending.pdu_tx_data);
	}
#endif
#if (DCM_USE_SERVICE_WRITEDATABYIDENTIFIER == STD_ON)
	if (DCM_GENERAL_PENDING == dsp_write_did_pending.state) {
		dsp_uds_write_data_by_id(dsp_write_did_pending.pdu_rx_data,
			dsp_write_did_pending.pdu_tx_data);
	}
#endif
}

static FUNC(void, DCM_CODE) dsp_routine_control_main_function(void)
{
	switch (dsp_routine_control_pending.state) {
		case DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND:
			dsp_routine_control_pending.state = DCM_GENERAL_FORCE_RCRRP;
			break;
		case DCM_GENERAL_PENDING:
		case DCM_GENERAL_FORCE_RCRRP:
			#if (DCM_USE_SERVICE_ROUTINECONTROL == STD_ON)
			dsp_uds_routine_ctrl(dsp_routine_control_pending.pdu_rx_data,
				dsp_routine_control_pending.pdu_tx_data);
			#endif
			break;
		default:
			break;
	}
}

#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
static FUNC(void, DCM_CODE) dsp_infotype_services_main_function(void)
{
	if (DCM_READ_VID_PENDING_READ_DATA == dsp_uds_infotype_services_pending.state) {
		dsp_obd_req_vehicle_info(dsp_uds_infotype_services_pending.pdu_rx_data,
			dsp_uds_infotype_services_pending.pdu_tx_data);
	} else {
		/* do nothing */
	}
}
#endif

static FUNC(void, DCM_CODE) dsp_security_access_main_function(void)
{
#if (DCM_USE_SERVICE_SECURITYACCESS == STD_ON)
	if (DCM_GENERAL_PENDING == dsp_security_access_pending.state) {
		dsp_uds_security_access(dsp_security_access_pending.pdu_rx_data,
			dsp_security_access_pending.pdu_tx_data);
	}
#endif
}

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && defined(DCM_USE_CONTROL_DIDS)
static FUNC(void, DCM_CODE) dsp_io_control_main_function(void)
{
	if (DCM_GENERAL_PENDING != dsp_io_ctrl_data.state) {
		return;
	}
	dsp_io_ctrl_by_data_id(dsp_io_ctrl_data.pdu_rx_data, dsp_io_ctrl_data.pdu_tx_data);
}

static FUNC(void, DCM_CODE) dsp_cancel_pending_io_control_by_data_identifier(const PduInfoType *pdu_rx_data)
{
	dcm_nrc_type response_code;
	const dcm_dsp_did_type *did_ptr = NULL_PTR;
	uint16 did_nr = (uint16)(((uint16)(pdu_rx_data->SduDataPtr[IOI_INDEX]) << 8u) & DCM_DID_HIGH_MASK) +
		(uint16)(pdu_rx_data->SduDataPtr[IOI_INDEX + 1u] & DCM_DID_LOW_MASK);

	if (FALSE == lookup_non_dynamic_did(did_nr, &did_ptr)) {
		return;
	}

	dsp_io_ctrl_stop_activated(did_nr, dsp_io_ctrl_data.signal_affected);

	if (dsp_io_ctrl_data.pending_signal_index >= did_ptr->dsp_nof_signals) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
		return;
	}
	if (DCM_GENERAL_PENDING != dsp_io_ctrl_data.state) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
		return;
	}

	const dcm_dsp_data_type *data_ptr =
		did_ptr->dsp_signal_ref[dsp_io_ctrl_data.pending_signal_index].dsp_signal_data_ref;

	if ((boolean)TRUE != dsp_io_ctrl_data.pending_ctrl) {
#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON)
		if ((data_ptr->dsp_data_read_data_fnc.asynch_data_read_fnc != NULL_PTR) &&
			(DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port)) {
			(void)data_ptr->dsp_data_read_data_fnc.asynch_data_read_fnc(DCM_CANCEL, NULL_PTR);
		}
#endif
	} else {
		if (data_ptr->dsp_data_use_port == DATA_PORT_ASYNCH) {
			(void)dsp_function_input_output_ctrl(data_ptr, pdu_rx_data->SduDataPtr[IOCP_INDEX], DCM_CANCEL,
				NULL_PTR, &response_code);
		}
	}
}
#endif

#ifdef DCM_USE_UPLOAD_DOWNLOAD
static FUNC(void, DCM_CODE) dsp_cancel_pending_upload_download(uint8 SID)
{
	dcm_nrc_type resp_code;
	uint32 dsp_block_size;
	uint32 file_size1;
	uint32 file_size2;

	if (DCM_SID_REQUEST_DOWNLOAD == SID) {
		(void)dcm_process_request_download(DCM_CANCEL, 0, 0, 0, &dsp_block_size, &resp_code);
	} else if (DCM_SID_REQUEST_UPLOAD == SID) {
		(void)dcm_process_request_upload(DCM_CANCEL, 0, 0, 0, &resp_code);
	} else if (DCM_SID_TRANSFER_DATA == SID) {
		switch (dsp_transfer_status.transfer_type) {
			case DCM_DOWNLOAD:
				(void)dcm_write_memory(DCM_CANCEL, 0, 0, 0, NULL);
				break;
			case DCM_UPLOAD:
				(void)dcm_read_memory(DCM_CANCEL, 0, 0, 0, NULL);
				break;
			case DCM_WRITEFILE:
				(void)dcm_write_file(DCM_CANCEL, 0, NULL, &resp_code);
				break;
			case DCM_READFILE:
				(void)dcm_read_file_or_dir(DCM_CANCEL, NULL, NULL, &resp_code);
				break;
			default:
				break;
		}
	} else if (DCM_SID_REQUEST_TRANSFER_EXIT == SID) {
		(void)dcm_process_request_transfer_exit(DCM_CANCEL, NULL, 0, &resp_code);
	} else if (DCM_SID_REQUEST_FILE_TRANSFER == SID) {
		switch (dsp_transfer_status.file_transfer_type) {
			case DCM_ADD_FILE:
				(void)dcm_process_request_add_file(DCM_CANCEL, 0, NULL, 0, 0, 0, &dsp_block_size, &resp_code);
				break;
			case DCM_DELETE_FILE:
				(void)dcm_process_request_delete_file(DCM_CANCEL, 0, NULL, &resp_code);
				break;
			case DCM_REPLACE_FILE:
				(void)dcm_process_request_replace_file(DCM_CANCEL, 0, NULL, 0, 0, 0, &dsp_block_size, &resp_code);
				break;
			case DCM_READ_FILE:
				(void)dcm_process_request_read_file(DCM_CANCEL, 0, NULL, 0, &file_size1, &file_size2,
					&dsp_block_size, &resp_code);
				break;
			case DCM_READ_DIR:
				(void)dcm_process_request_read_dir(DCM_CANCEL, 0, NULL, &file_size1, &dsp_block_size, &resp_code);
				break;
			default:
				break;
		}
	} else {
		/* do nothing */
	}
}

static FUNC(void, DCM_CODE) dsp_upload_download(uint8 SID, const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	if (DCM_SID_REQUEST_DOWNLOAD == SID) {
#if (DCM_USE_SERVICE_REQUESTDOWNLOAD == STD_ON)
		dsp_uds_request_download(pdu_rx_data, pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_UPLOAD == SID) {
#if (DCM_USE_SERVICE_REQUESTUPLOAD == STD_ON)
		dsp_uds_request_upload(pdu_rx_data, pdu_tx_data);
#endif
	} else if (DCM_SID_TRANSFER_DATA == SID) {
#if (DCM_USE_SERVICE_TRANSFERDATA == STD_ON)
		dsp_uds_transfer_data(pdu_rx_data, pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_TRANSFER_EXIT == SID) {
#if (DCM_USE_SERVICE_REQUESTTRANSFEREXIT == STD_ON)
		dsp_uds_request_transfer_exit(pdu_rx_data, pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_FILE_TRANSFER == SID) {
#if (DCM_USE_SERVICE_REQUESTFILETRANSFER == STD_ON)
		dsp_uds_req_file_transfer(pdu_rx_data, pdu_tx_data);
#endif
	} else {
		/* do nothing */
	}
}

static FUNC(void, DCM_CODE) dsp_upload_download_main_function(void)
{
	switch (dsp_upload_download_pending.state) {
		case DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND:
			dsp_upload_download_pending.state = DCM_GENERAL_FORCE_RCRRP;
			break;
		case DCM_GENERAL_PENDING:
		case DCM_GENERAL_FORCE_RCRRP:
			dsp_upload_download(dsp_upload_download_pending.pend_svc,
				dsp_upload_download_pending.pdu_rx_data,
				dsp_upload_download_pending.pdu_tx_data);
			break;
		default:
			break;
	}
}
#endif

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
FUNC(void, DCM_CODE) dsp_jump_to_boot_main_function(void)
{
#if (DCM_USE_SERVICE_DIAGNOSTICSESSIONCONTROL == STD_ON)
	if ((DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE == dsp_session_ctrl_data.jump_to_boot_state) ||
	    (DCM_JTB_EXECUTE == dsp_session_ctrl_data.jump_to_boot_state)) {
		dsp_uds_diag_ses_ctrl(dsp_session_ctrl_data.pdu_rx_data,
			dsp_session_ctrl_data.session_pdu_id,
			dsp_session_ctrl_data.pdu_tx_data, FALSE, FALSE);
	}
#endif
}
#endif

static FUNC(void, DCM_CODE) dsp_startup_service_response_main_function(void)
{
	if (DCM_GENERAL_PENDING != dsp_prog_condition_stup_resp_state) {
		return;
	}

	Std_ReturnType req_ret = dsl_dsp_response_on_startup_request(dsp_prog_conditions.sid,
		dsp_prog_conditions.sub_fnc_id, dsp_prog_conditions.protocol_id,
		(uint16)dsp_prog_conditions.tester_source_add);
	if (DCM_E_PENDING != req_ret) {
		if (E_OK != req_ret) {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		}
		dsp_prog_condition_stup_resp_state = DCM_GENERAL_IDLE;
	}
}

FUNC(void, DCM_CODE) dsp_pre_dsd_main_function(void)
{
#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
	dsp_jump_to_boot_main_function();
#endif

	dsp_startup_service_response_main_function();
}

#if (DCM_RESPONSE_MULTIPLE_PENDING_ENABLED == STD_ON)
static FUNC(void, DCM_CODE) dsp_diagnostic_session_control_programming_main_function(void)
{
	switch (dsp_dsc_programming_pending.state) {
		case DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND:
			dsp_dsc_programming_pending.state = DCM_GENERAL_FORCE_RCRRP;
			break;
		case DCM_GENERAL_PENDING:
		case DCM_GENERAL_FORCE_RCRRP:
			#if (DCM_USE_SERVICE_DIAGNOSTICSESSIONCONTROL == STD_ON)
			dsp_uds_diag_ses_ctrl(dsp_dsc_programming_pending.pdu_rx_data,
				dsp_dsc_programming_pending.pdur_tx_pdu_id,
				dsp_dsc_programming_pending.pdu_tx_data,
				dsp_dsc_programming_pending.resp_pend_on_trans_to_boot,
				dsp_dsc_programming_pending.internal_startup_request);
			#endif
			break;
		default:
			break;
	}
}
#endif

static void dsp_execute_core_functions(void)
{
	dsp_reset_main_function();
	dsp_read_did_main_function();
	dsp_routine_control_main_function();
	dsp_security_access_main_function();
}

static void dsp_execute_conditional_functions(void)
{
#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON) || \
	(DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
	dsp_memory_main_function();
#endif

#if (DCM_RESPONSE_MULTIPLE_PENDING_ENABLED == STD_ON)
	dsp_diagnostic_session_control_programming_main_function();
#endif

#ifdef DCM_USE_UPLOAD_DOWNLOAD
	dsp_upload_download_main_function();
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && \
	defined(DCM_USE_CONTROL_DIDS)
	dsp_io_control_main_function();
#endif

#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
	dsp_infotype_services_main_function();
#endif
}

FUNC(void, DCM_CODE) dsp_main_function(void)
{
	dsp_execute_core_functions();
	dsp_execute_conditional_functions();
}

FUNC(void, DCM_CODE) dsp_timer_main_function(void)
{
	for (uint8 i = 0; i < DCM_SECURITY_EOL_INDEX; i++) {
		if ((DELAY_TIMER_ON_BOOT_ACTIVE != dsp_security_acces_data.sec_false_attempt_chk[i].start_delay_timer) &&
			DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE !=
			dsp_security_acces_data.sec_false_attempt_chk[i].start_delay_timer) {
				continue;
			}

		TIMER_DECREMENT
		(dsp_security_acces_data.sec_false_attempt_chk[i].timer_sec_acss_attempt);

		if (dsp_security_acces_data.sec_false_attempt_chk[i].timer_sec_acss_attempt >=
			DCM_MAIN_FUNCTION_PERIOD_TIME_MS) {
			break;
		}

		dsp_security_acces_data.sec_false_attempt_chk[i].start_delay_timer =
			DELAY_TIMER_DEACTIVE;

		if (dsp_security_acces_data
			.sec_false_attempt_chk[dsp_security_acces_data.curr_sec_lev_idx]
			.sec_acss_attempts <= 0UL) {
			break;
		}

		dsp_security_acces_data
			.sec_false_attempt_chk[dsp_security_acces_data.curr_sec_lev_idx]
			.sec_acss_attempts--;
#if (DCM_SEC_ATT_CNTR_ENABLED == STD_ON)
		dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

		if (dsp_write_did_pending.state == DCM_GENERAL_PENDING) {
			op_status = (dcm_op_status_type)DCM_PENDING;
		}
		uint8 attempt_count = dsp_security_acces_data
			.sec_false_attempt_chk[dsp_security_acces_data.curr_sec_lev_idx].sec_acss_attempts;
		if ((dsp_security_acces_data.req_sec_level_ref != NULL) &&
			(dsp_security_acces_data.req_sec_level_ref->set_attempt_counter != NULL)) {
			(void)dsp_security_acces_data.req_sec_level_ref
				->set_attempt_counter(op_status, attempt_count);
		}
#endif
	}
}

FUNC(void, DCM_CODE) dsp_cancel_pending_req(void)
{
	if (dsp_read_did_pending.state != DCM_READ_DID_IDLE) {
		dsp_cancel_pending_did(dsp_read_did_pending.pending_did,
			dsp_read_did_pending.pending_signal_index, dsp_read_did_pending.state,
			dsp_read_did_pending.pdu_tx_data);
		dsp_read_did_pending.state = DCM_READ_DID_IDLE;
	}

#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON) || (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
	dsp_mem_state = DCM_MEMORY_UNUSED;
#endif

	dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_NO_RESET;

#if (DCM_USE_SERVICE_WRITEDATABYIDENTIFIER == STD_ON)
	dsp_write_did_pending.state = DCM_GENERAL_IDLE;
#endif

	if (dsp_routine_control_pending.state != DCM_GENERAL_IDLE) {
		dsp_cancel_pending_routine(dsp_routine_control_pending.pdu_rx_data,
			dsp_routine_control_pending.pdu_tx_data);
		dsp_routine_control_pending.state = DCM_GENERAL_IDLE;
	}

#if (DCM_RESPONSE_MULTIPLE_PENDING_ENABLED == STD_ON)
	if (dsp_dsc_programming_pending.state != DCM_GENERAL_IDLE) {
		(void)memset(&dsp_dsc_programming_pending, 0,
			sizeof(dsp_dsc_programming_pending_type));
	}
#endif

#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
	if (dsp_uds_infotype_services_pending.state != DCM_READ_VID_IDLE) {
		dsp_uds_infotype_services_pending.is_canceled = TRUE;
		dsp_obd_req_vehicle_info(dsp_uds_infotype_services_pending.pdu_rx_data,
			dsp_uds_infotype_services_pending.pdu_tx_data);
		dsp_uds_infotype_services_pending.state = DCM_READ_VID_IDLE;
	}
#endif

	if (dsp_security_access_pending.state != DCM_GENERAL_IDLE) {
		dsp_cancel_pending_security_access(dsp_security_access_pending.pdu_rx_data,
			dsp_security_access_pending.pdu_tx_data);
		dsp_security_access_pending.state = DCM_GENERAL_IDLE;
	}

#if defined(DCM_USE_UPLOAD_DOWNLOAD)
	if (dsp_upload_download_pending.state != DCM_GENERAL_IDLE) {
		dsp_cancel_pending_upload_download(dsp_upload_download_pending.pend_svc);
		dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
	}
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && defined(DCM_USE_CONTROL_DIDS)
	if (dsp_io_ctrl_data.state == DCM_GENERAL_PENDING) {
		dsp_cancel_pending_io_control_by_data_identifier(dsp_io_ctrl_data.pdu_rx_data);
		dsp_io_ctrl_data.state = DCM_GENERAL_IDLE;
	}
#endif

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
	dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_IDLE;
#endif
}

FUNC(boolean, DCM_CODE) dsp_check_session_level(const uint8 session_level_mask)
{
	boolean level_found = FALSE;
	dcm_ses_ctrl_type current_session;
	Std_ReturnType return_status = dsl_get_ses_ctrl_type(&current_session);

	if (return_status != E_OK) {
		return FALSE;
	}

	if (session_level_mask == 0u) {
		level_found = TRUE;
	} else {
		const dcm_dsp_session_type *session_list = dcm_config_ptr->dsp->dsp_session;

		for (uint8 i = 0; i < DCM_SESSION_EOL_INDEX; i++) {
			if ((DCM_GET_MASK_BIT(session_level_mask, i) != 0u) &&
				(session_list[i].dsp_session_level == current_session)) {
				level_found = TRUE;
				break;
			}
		}
	}

	return level_found;
}

FUNC(boolean, DCM_CODE) dsp_check_security_level(const uint8 security_level_mask)
{
	boolean level_found = FALSE;
	dcm_sec_lv_type current_security_level;
	Std_ReturnType return_status = dsl_get_security_level(&current_security_level);

	if (return_status != E_OK) {
		return FALSE;
	}

	if (security_level_mask == 0u) {
		level_found = TRUE;
	} else {
		const dcm_dsp_security_type *security_list = dcm_config_ptr->dsp->dsp_security;

		for (uint8 i = 0; i < DCM_SECURITY_EOL_INDEX; i++) {
			if ((DCM_GET_MASK_BIT(security_level_mask, i) != 0u) &&
				(security_list[i].dsp_security_level == current_security_level)) {
				level_found = TRUE;
				break;
			}
		}
	}
	return level_found;
}

FUNC(boolean, DCM_CODE) dsp_dsl_check_session_supported(uint8 session)
{
	boolean rst = FALSE;
	const dcm_dsp_session_type *session_list = dcm_config_ptr->dsp->dsp_session;

	for (uint8 i = 0; i < DCM_SESSION_EOL_INDEX; i++) {
		if (session_list[i].dsp_session_level == session) {
			rst = TRUE;
			break;
		}
	}
	return rst;
}

static FUNC(uint8, DCM_CODE)
dsp_set_session_control_timing(const dsp_uds_session_control_data_type *session_data, uint8 *timing_data)
{
	uint16 p2server_star_max10ms = session_data->p2star / 10u;

	timing_data[0u] = (uint8)((uint8)(session_data->P2 >> 8u) & (uint8)0xffu);
	timing_data[1u] = (uint8)(session_data->P2 & 0xffu);
	timing_data[2u] = (uint8)((uint8)(p2server_star_max10ms >> 8u) & (uint8)0xffu);
	timing_data[3u] = (uint8)(p2server_star_max10ms & 0xffu);

	return DCM_DSP_TIMING_DATA_LEN;
}

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
static Std_ReturnType dsp_trigger_ecu_reset(uint8 reset_mode)
{
#ifdef DCM_CALLBACK_ECURESET_ENABLED
	return dcm_callback_dcm_ecu_reset(reset_mode);
#elif (CONFIG_MODULE_RTE == 1)
	return Rte_Switch_EcuResetModeSwitchInterface_DcmEcuReset(reset_mode);
#else
	return E_NOT_OK;
#endif
}

static Std_ReturnType dsp_handle_boot_switch(dcm_boot_type boot_mode)
{
	switch (boot_mode) {
		case DCM_OEM_BOOT:
		case DCM_OEM_BOOT_RESPAPP:
			return dsp_trigger_ecu_reset(RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER);
		case DCM_SYS_BOOT:
		case DCM_SYS_BOOT_RESPAPP:
			return dsp_trigger_ecu_reset(RTE_MODE_DcmEcuReset_JUMPTOSYSSUPPLIERBOOTLOADER);
		case DCM_NO_BOOT:
			return E_OK;
		default:
			return E_NOT_OK;
	}
}
#endif

static void dsp_update_session_control_data(const dcm_dsp_session_type *session_list,
	uint8 session_index, dcm_ses_ctrl_type req_session_type, PduIdType tx_pdu_id,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dsp_session_ctrl_data.session = req_session_type;
	dsp_session_ctrl_data.session_pdu_id = tx_pdu_id;
	dsp_session_ctrl_data.pdu_rx_data = pdu_rx_data;
	dsp_session_ctrl_data.pdu_tx_data = pdu_tx_data;
	dsp_session_ctrl_data.P2 = session_list[session_index].dsp_session_p2server_max;
	dsp_session_ctrl_data.p2star = session_list[session_index].dsp_session_p2star_server_max;

	dcm_protocol_type active_protocol_id;
	Std_ReturnType status = dsl_get_active_protocol(&active_protocol_id);

	dsp_session_ctrl_data.protocol_id = (status == E_OK) ? active_protocol_id : 0xff;
}

static void dsp_handle_normal_session_change(const dcm_dsp_session_type *session_list,
	uint8 session_index, dcm_ses_ctrl_type req_session_type, PduInfoType *pdu_tx_data)
{
#if (DCM_RESPONSE_MULTIPLE_PENDING_ENABLED == STD_ON)
	if ((DCM_PROGRAMMING_SESSION == req_session_type) &&
		(DCM_NO_BOOT == session_list[session_index].dsp_session_for_boot)) {
		process_pending_response(session_index, pdu_tx_data);
		return;
	}
#endif

	pdu_tx_data->SduDataPtr[1] = req_session_type;
	uint8 timing_len = dsp_set_session_control_timing(&dsp_session_ctrl_data,
		&pdu_tx_data->SduDataPtr[2]);
	pdu_tx_data->SduLength = 2u + timing_len;
	dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
}

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
static void dsp_handle_immediate_boot(const dcm_dsp_session_type *session_list,
	uint8 session_index, dcm_ses_ctrl_type req_session_type, PduInfoType *pdu_tx_data)
{
	dcm_boot_type boot_mode = session_list[session_index].dsp_session_for_boot;

	switch (boot_mode) {
		case DCM_OEM_BOOT:
		case DCM_SYS_BOOT:
			if (E_OK == dcm_set_prog_conditions(&dsp_prog_conditions)) {
				dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_EXECUTE;
			} else {
				dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
			}
			break;
		case DCM_OEM_BOOT_RESPAPP:
		case DCM_SYS_BOOT_RESPAPP:
			dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM;
			pdu_tx_data->SduDataPtr[1] = req_session_type;
			uint8 timing_len = dsp_set_session_control_timing(&dsp_session_ctrl_data,
				&pdu_tx_data->SduDataPtr[2]);
			pdu_tx_data->SduLength = 2u + timing_len;
			dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
			break;
		default:
			break;

	}
}

static void dsp_handle_pending_boot_response(const dcm_dsp_session_type *session_list,
	uint8 session_index)
{
	dcm_boot_type boot_mode = session_list[session_index].dsp_session_for_boot;

	if ((boot_mode == DCM_OEM_BOOT_RESPAPP) || (boot_mode == DCM_SYS_BOOT_RESPAPP)) {
		dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM;
	} else {
		dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM;
	}

	dsd_dsp_force_response_pending();
}

static void dsp_handle_boot_transition(const dcm_dsp_session_type *session_list,
	uint8 session_index, dcm_ses_ctrl_type req_session_type,
	PduInfoType *pdu_tx_data, boolean resp_pend_on_trans_to_boot)
{
	boolean prog_req = FALSE;

	if (DCM_PROGRAMMING_SESSION == req_session_type) {
		prog_req = TRUE;
	}

	dsp_prog_conditions.sid = DCM_SID_DIAGNOSTIC_SESSION_CONTROL;
	dsp_prog_conditions.reprograming_request = prog_req;
	dsp_prog_conditions.sub_fnc_id = req_session_type;
	dsp_prog_conditions.response_required = dsd_dsp_get_response_required();

	dcm_protocol_type protocol_id;

	if (E_OK == dsl_get_active_protocol(&protocol_id)) {
		dsp_prog_conditions.protocol_id = protocol_id;
	}
	uint16 src_addr = dsd_dsp_get_tester_source_address();

	dsp_prog_conditions.tester_source_add = (uint8)(src_addr & 0xff);

	if (FALSE == resp_pend_on_trans_to_boot) {
		dsp_handle_immediate_boot(session_list, session_index, req_session_type, pdu_tx_data);
	} else {
		dsp_handle_pending_boot_response(session_list, session_index);
	}

	if ((0 != (src_addr & (uint16)~0xff)) || (E_OK != dsl_get_active_protocol(&protocol_id))) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
	}
}
#endif

static FUNC(void, DCM_CODE)
dsp_handle_session_ctrl_on_jtb_idle(const PduInfoType *pdu_rx_data,
	PduIdType tx_pdu_id, PduInfoType *pdu_tx_data,
	boolean resp_pend_on_trans_to_boot, boolean internal_startup_request)
{
	if (pdu_rx_data->SduLength != 2u) {
		dsd_dsp_processing_done(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
		return;
	}

	const dcm_dsp_session_type *session_list = dcm_config_ptr->dsp->dsp_session;
	dcm_ses_ctrl_type req_session_type = pdu_rx_data->SduDataPtr[1];

	uint8 session_index;

	for (session_index = 0; session_index < DCM_SESSION_EOL_INDEX; session_index++) {
		if (session_list[session_index].dsp_session_level == req_session_type) {
			break;
		}
	}
	if (session_index >= DCM_SESSION_EOL_INDEX) {
		dsd_dsp_processing_done(DCM_E_SUBFUNCTIONNOTSUPPORTED);
		return;
	}

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
	if (!internal_startup_request) {
		Std_ReturnType result = dsp_handle_boot_switch(session_list[session_index].dsp_session_for_boot);

		if (result != E_OK) {
			dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
			return;
		}
	}
#endif

	dsp_update_session_control_data(session_list, session_index, req_session_type,
		tx_pdu_id, pdu_rx_data, pdu_tx_data);

	if ((DCM_NO_BOOT == session_list[session_index].dsp_session_for_boot) || internal_startup_request) {
		dsp_handle_normal_session_change(session_list, session_index,
			req_session_type, pdu_tx_data);
	} else {
#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
		dsp_handle_boot_transition(session_list, session_index,
			req_session_type, pdu_tx_data, resp_pend_on_trans_to_boot);
#else
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
		dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
#endif
	}
}

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
static FUNC(void, DCM_CODE) dsp_handle_session_ctrl_on_jtb_execute(void) {
	Std_ReturnType ret = E_OK;

#ifdef DCM_CALLBACK_ECURESET_ENABLED
	ret = dcm_callback_dcm_ecu_reset(RTE_MODE_DcmEcuReset_EXECUTE);
#else
#if (CONFIG_MODULE_RTE == 1)
	ret = Rte_Switch_EcuResetModeSwitchInterface_DcmEcuReset(RTE_MODE_DcmEcuReset_EXECUTE);
#endif
#endif
	if (E_OK != ret) {
		dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
	}
	dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_IDLE;
}

static FUNC(void, DCM_CODE)
dsp_handle_session_ctrl_on_jtb_resapp_assemble_final_resp(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data) {
	uint8 timing_param_len = 0u;

	dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM;
	pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];

	timing_param_len = dsp_set_session_control_timing(&dsp_session_ctrl_data, &pdu_tx_data->SduDataPtr[2]);

	pdu_tx_data->SduLength = 2u + timing_param_len;
	dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
}
#endif

FUNC(void, DCM_CODE)
dsp_uds_diag_ses_ctrl(const PduInfoType *pdu_rx_data, PduIdType tx_pdu_id, PduInfoType *pdu_tx_data,
	boolean resp_pend_on_trans_to_boot, boolean internal_startup_request)
{
	switch (dsp_session_ctrl_data.jump_to_boot_state) {
		case DCM_JTB_IDLE:
			dsp_handle_session_ctrl_on_jtb_idle(pdu_rx_data, tx_pdu_id, pdu_tx_data,
				resp_pend_on_trans_to_boot, internal_startup_request);
			break;
#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
		case DCM_JTB_EXECUTE:
			dsp_handle_session_ctrl_on_jtb_execute();
			break;
		case DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE:
			dsp_handle_session_ctrl_on_jtb_resapp_assemble_final_resp(pdu_rx_data, pdu_tx_data);
			break;
#endif
	}
}

FUNC(void, DCM_CODE)
dsp_uds_ecu_reset(const PduInfoType *pdu_rx_data, PduIdType tx_pdu_id, PduInfoType *pdu_tx_data,
	boolean startup_response_request)
{
	dcm_ecu_rst_type req_reset_type;
	uint8 rte_mode;

	if (pdu_rx_data->SduLength != 2u) {
		dsd_dsp_processing_done(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
		return;
	}

	req_reset_type = (dcm_ecu_rst_type)pdu_rx_data->SduDataPtr[1];
	Std_ReturnType result = E_NOT_OK;

	if (TRUE == startup_response_request) {
		pdu_tx_data->SduDataPtr[1] = req_reset_type;
		pdu_tx_data->SduLength = 2;
		dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
		return;
	}
	if ((DCM_DISABLE_RAPID_POWER_SHUTDOWN == req_reset_type) ||
		(DCM_ENABLE_RAPID_POWER_SHUTDOWN == req_reset_type)) {
			dsd_dsp_processing_done(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			return;
		}

	result = dsp_get_reset_rte_mode(req_reset_type, &rte_mode);
	if (E_OK != result) {
		dsd_dsp_processing_done(DCM_E_SUBFUNCTIONNOTSUPPORTED);
		return;
	}

#ifdef DCM_CALLBACK_ECURESET_ENABLED
	result = dcm_callback_dcm_ecu_reset(rte_mode);
#else
#if (CONFIG_MODULE_RTE == 1)
	result = Rte_Switch_EcuResetModeSwitchInterface_DcmEcuReset(rte_mode);
#endif
#endif
	dsp_ecu_reset_data.reset_pdu_id = tx_pdu_id;
	dsp_ecu_reset_data.pdu_tx_data = pdu_tx_data;
	dsp_ecu_reset_data.reset_type = req_reset_type;

	if (E_OK == result) {
		dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_WAIT_TX_CONF;
		pdu_tx_data->SduDataPtr[1] = req_reset_type;
		pdu_tx_data->SduLength = 2;
		dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
	} else if (DCM_E_PENDING == result) {
		dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_PENDING;
	} else {
		dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_NO_RESET;
		dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
	}
}

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION == STD_ON)
FUNC(void, DCM_CODE)
dsp_uds_clear_diag_info(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id)
{
	uint32 dtc;
	Std_ReturnType result;
	Dem_DTCFormatType dtc_format = DEM_DTC_FORMAT_UDS;

	if (pdu_rx_data->SduLength != 4u) {
		dsd_dsp_processing_done(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
		return;
	}
	dtc = BYTES_TO_DTC(pdu_rx_data->SduDataPtr[1], pdu_rx_data->SduDataPtr[2], pdu_rx_data->SduDataPtr[3]);

	if (DEM_EMISSION_GROUP_REL_DTCS == dtc) {
		dtc_format = DEM_DTC_FORMAT_OBD;
	}
	(void)dem_select_dtc(client_id, dtc, dtc_format, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
	result = dem_clear_dtc(client_id);
	if (DEM_CLEAR_FAILED == result) {
		dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
	} else if (E_OK == result) {
		pdu_tx_data->SduLength = 1;
		dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
	} else {
		dsd_dsp_processing_done(DCM_E_REQUESTOUTOFRANGE);
	}
}
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_READDTCINFORMATION == STD_ON)
static FUNC(uint8, DCM_CODE) dsp_get_byte_from_dword_by_index(uint32 data, uint8 idx)
{
	uint8 ret;

	ret = (idx < 4u) ? ((uint8)(data >> (idx * (uint8)8)) & (uint8)0xff) : 0u;

	return ret;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_read_dtc_info_sub_0x01_0x07_0x11_0x12_dtc_filter_result(uint8 client_id, const PduInfoType *pdu_rx_data,
	uint16 *number_of_filtered_dtc)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType set_dtc_filter_result;
	uint8 dsp_dtc_sts_mask =
		(pdu_rx_data->SduDataPtr[1] == 0x07u) ? pdu_rx_data->SduDataPtr[3] : pdu_rx_data->SduDataPtr[2];
	if (dsp_dtc_sts_mask == 0x00u) {
		return DCM_E_POSITIVERESPONSE;
	}

	if (DCM_DSP_READ_DTC_INFO_SUB_ID_0x01 == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, dsp_dtc_sts_mask, DEM_DTC_FORMAT_UDS,
			DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else if (DCM_DSP_READ_DTC_INFO_SUB_ID_0x07 == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, dsp_dtc_sts_mask, DEM_DTC_FORMAT_UDS,
			DEM_DTC_ORIGIN_PRIMARY_MEMORY, TRUE, pdu_rx_data->SduDataPtr[2], FALSE);
	} else if (DCM_DSP_READ_DTC_INFO_SUB_ID_0x12 == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, dsp_dtc_sts_mask, DEM_DTC_FORMAT_UDS,
			DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else if (DCM_DSP_READ_DTC_INFO_SUB_ID_0x11 == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, dsp_dtc_sts_mask, DEM_DTC_FORMAT_UDS,
			DEM_DTC_ORIGIN_MIR_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else {
		set_dtc_filter_result = E_NOT_OK;
	}

	if (set_dtc_filter_result != E_OK) {
		return DCM_E_REQUESTOUTOFRANGE;
	}

	Std_ReturnType get_numer_result = dem_get_number_of_filtered_dtc(client_id, number_of_filtered_dtc);

	if (get_numer_result != E_OK) {
		response_code = DCM_E_GENERALREJECT;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_read_dtc_info_sub_0x01_0x07_0x11_0x12(uint8 client_id,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	uint16 number_of_filtered_dtc = 0;

	response_code = dsp_read_dtc_info_sub_0x01_0x07_0x11_0x12_dtc_filter_result(client_id, pdu_rx_data,
		&number_of_filtered_dtc);
	if (response_code != DCM_E_POSITIVERESPONSE) {
		return response_code;
	}

	uint8 dtc_availablity_mask;
	Std_ReturnType result;

	result = dem_get_dtc_status_availability_mask(client_id, &dtc_availablity_mask);
	if (result != E_OK) {
		dtc_availablity_mask = 0;
	}

	pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
	pdu_tx_data->SduDataPtr[2] = dtc_availablity_mask;
	pdu_tx_data->SduDataPtr[3] = dem_get_translation_type(client_id);
	pdu_tx_data->SduDataPtr[4] = (uint8)((number_of_filtered_dtc >> (uint8)8u) & (uint16)0x00ffu);
	pdu_tx_data->SduDataPtr[5] = (uint8)(number_of_filtered_dtc & 0xffu);
	pdu_tx_data->SduLength = 6;

	return response_code;
}

static FUNC(Std_ReturnType, DCM_CODE)
	dsp_do_read_dtc_info_sub_set_dtc_filter_sub_0x02_0x0a_0x0f_0x13_0x15(uint8 client_id,
		const PduInfoType *pdu_rx_data)
{
	Std_ReturnType set_dtc_filter_result = E_OK;

	if (0x02 == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result =
		dem_set_dtc_filter(client_id, pdu_rx_data->SduDataPtr[2], DEM_DTC_FORMAT_UDS,
			DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else if (0x0a == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, DEM_DTC_STATUS_MASK_ALL,
			DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else if (0x0f == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, pdu_rx_data->SduDataPtr[2], DEM_DTC_FORMAT_UDS,
			DEM_DTC_ORIGIN_MIR_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else if (0x013 == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, pdu_rx_data->SduDataPtr[2], DEM_DTC_FORMAT_UDS,
			DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else if (0x015 == pdu_rx_data->SduDataPtr[1]) {
		set_dtc_filter_result = dem_set_dtc_filter(client_id, DEM_DTC_STATUS_MASK_ALL,
			DEM_DTC_FORMAT_UDS, DEM_DTC_ORIGIN_PERMANENT_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
	} else {
		set_dtc_filter_result = E_NOT_OK;
	}

	return set_dtc_filter_result;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_read_dtc_info_sub_after_filter(uint8 client_id,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	uint8 dsp_dtc_sts_mask = 0;
	uint16 dtc_idx = 1;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (dem_get_dtc_status_availability_mask(client_id, &dsp_dtc_sts_mask) != E_OK) {
		dsp_dtc_sts_mask = 0;
	}

	pdu_tx_data->SduDataPtr[dtc_idx++] = pdu_rx_data->SduDataPtr[1];
	pdu_tx_data->SduDataPtr[dtc_idx++] = dsp_dtc_sts_mask;

	if (dsp_dtc_sts_mask == 0x00u) {
		pdu_tx_data->SduLength = dtc_idx;
		return response_code;
	}

	uint16 nof_filtered_dt_cs = 0;

	if (dem_get_number_of_filtered_dtc(client_id, &nof_filtered_dt_cs) != E_OK) {
		pdu_tx_data->SduLength = dtc_idx;
		return DCM_E_REQUESTOUTOFRANGE;
	}

	if (((nof_filtered_dt_cs * 4u) + dtc_idx) > pdu_tx_data->SduLength) {
		pdu_tx_data->SduLength = dtc_idx;
		return DCM_E_RESPONSETOOLONG;
	}

	uint32 dtc;
	dem_event_status_rec_type dtc_status;
	Std_ReturnType result = dem_get_next_filtered_dtc(client_id, &dtc, &dtc_status);

	while (result == E_OK) {
		pdu_tx_data->SduDataPtr[dtc_idx++] = dsp_get_byte_from_dword_by_index(dtc, 2u);
		pdu_tx_data->SduDataPtr[dtc_idx++] = dsp_get_byte_from_dword_by_index(dtc, 1u);
		pdu_tx_data->SduDataPtr[dtc_idx++] = dsp_get_byte_from_dword_by_index(dtc, 0u);
		pdu_tx_data->SduDataPtr[dtc_idx++] = dtc_status;
		result = dem_get_next_filtered_dtc(client_id, &dtc, &dtc_status);
	}

	if (result != DEM_NO_SUCH_ELEMENT) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	pdu_tx_data->SduLength = dtc_idx;
	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_read_dtc_info_sub_0x02_0x0a_0x0f_0x13_0x15
(uint8 client_id, const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	if ((pdu_rx_data->SduDataPtr[1u] != 0x0au) && (pdu_rx_data->SduDataPtr[1] != 0x15u) &&
		(pdu_rx_data->SduDataPtr[2u] == 0x00u)) {
		uint8 dsp_dtc_sts_mask = 0u;

		dem_get_dtc_status_availability_mask(client_id, &dsp_dtc_sts_mask);

		pdu_tx_data->SduDataPtr[1u] = pdu_rx_data->SduDataPtr[1];
		pdu_tx_data->SduDataPtr[2u] = dsp_dtc_sts_mask;
		pdu_tx_data->SduLength = 3u;
		return DCM_E_POSITIVERESPONSE;
	}

	Std_ReturnType set_result =
		dsp_do_read_dtc_info_sub_set_dtc_filter_sub_0x02_0x0a_0x0f_0x13_0x15(client_id, pdu_rx_data);

	dcm_nrc_type read_dtc_result = DCM_E_POSITIVERESPONSE;

	read_dtc_result = (set_result == E_OK)
		? dsp_do_read_dtc_info_sub_after_filter(client_id, pdu_rx_data, pdu_tx_data)
		: DCM_E_REQUESTOUTOFRANGE;

	return read_dtc_result;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_read_dtc_info_sub_0x06_0x10_para(const PduInfoType *pdu_rx_data, dem_dtc_origin_type *dtc_origin,
	uint8 *start_rec_num, uint8 *end_rec_num)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	uint8 sdu_data_1 = pdu_rx_data->SduDataPtr[1];
	uint8 sdu_data_5 = pdu_rx_data->SduDataPtr[5];

	if (DCM_DSP_READ_DTC_INFO_SUB_ID_0x06 == sdu_data_1) {
		*dtc_origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
	} else if (DCM_DSP_READ_DTC_INFO_SUB_ID_0x10 == sdu_data_1) {
		*dtc_origin = DEM_DTC_ORIGIN_MIR_MEMORY;
	} else {
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		*dtc_origin = 0;
	}

	if (0xff == sdu_data_5) {
		*start_rec_num = 0x00;
		*end_rec_num = 0xef;
	} else if (0xfe == sdu_data_5) {
		*start_rec_num = 0x90;
		*end_rec_num = 0xef;
	} else {
		*start_rec_num = sdu_data_5;
		*end_rec_num = *start_rec_num;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_read_dtc_info_sub_0x06_0x10(uint8 client_id, const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	uint8 start_rec_num;
	uint8 end_rec_num;
	dem_dtc_origin_type dtc_origin;

	dcm_nrc_type response_code =
		dsp_read_dtc_info_sub_0x06_0x10_para(pdu_rx_data, &dtc_origin, &start_rec_num, &end_rec_num);

	if (response_code != DCM_E_POSITIVERESPONSE) {
		return response_code;
	}

	dem_event_status_rec_type status_of_dtc;

	uint32 dtc = BYTES_TO_DTC(pdu_rx_data->SduDataPtr[2], pdu_rx_data->SduDataPtr[3],
		pdu_rx_data->SduDataPtr[4]);

	(void)dem_select_dtc(client_id, dtc, DEM_DTC_FORMAT_UDS, dtc_origin);

	if (E_OK != dem_get_status_of_dtc(client_id, &status_of_dtc)) {
		return DCM_E_REQUESTOUTOFRANGE;
	}

	dem_return_get_ext_data_record_by_dtc_type get_extended_data_record_by_dtc_result;
	PduLengthType rec_length;
	PduLengthType tx_index = 6;
	boolean found_valid_record_number = FALSE;

	pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
	pdu_tx_data->SduDataPtr[2] = dsp_get_byte_from_dword_by_index(dtc, 2u);
	pdu_tx_data->SduDataPtr[3] = dsp_get_byte_from_dword_by_index(dtc, 1u);
	pdu_tx_data->SduDataPtr[4] = dsp_get_byte_from_dword_by_index(dtc, 0u);
	pdu_tx_data->SduDataPtr[5] = status_of_dtc;
	uint8 rec_num = start_rec_num;

	while ((rec_num <= end_rec_num) && (DCM_E_POSITIVERESPONSE == response_code)) {
		if (pdu_tx_data->SduLength > (tx_index + 1u)) {
			rec_length = (pdu_tx_data->SduLength - tx_index - 1u);
		} else {
			rec_length = 0u;
		}
		get_extended_data_record_by_dtc_result = dem_get_ext_data_record_by_dtc(dtc, dtc_origin,
			rec_num, &pdu_tx_data->SduDataPtr[tx_index + 1u], &rec_length);
		if (DEM_RECORD_OK == get_extended_data_record_by_dtc_result) {
			found_valid_record_number = TRUE;
			if (rec_length > 0u) {
				pdu_tx_data->SduDataPtr[tx_index] = rec_num;
				tx_index++;
				tx_index += rec_length;
			}
		} else if (DEM_RECORD_BUFFERSIZE == get_extended_data_record_by_dtc_result) {
			response_code = DCM_E_RESPONSETOOLONG;
		} else {
			/* do nothing */
		}
		rec_num++;
	}

	pdu_tx_data->SduLength = tx_index;

	if (!found_valid_record_number) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_read_dtc_info_sub_0x04(uint8 client_id, const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	PduLengthType size_of_tx_buf = pdu_tx_data->SduLength;
	dem_return_get_ff_data_by_dtc_type get_f_fby_dtc_return_code = DEM_GET_FFDATABYDTC_OK;
	dem_event_status_rec_type dtc_status = 0;

	uint32 dtc_number = (((uint32)pdu_rx_data->SduDataPtr[2]) << 16) + (((uint32)pdu_rx_data->SduDataPtr[3]) << 8) +
		((uint32)pdu_rx_data->SduDataPtr[4]);

	(void)dem_select_dtc(client_id, dtc_number, DEM_DTC_FORMAT_UDS,
		DEM_DTC_ORIGIN_PRIMARY_MEMORY);
	Std_ReturnType get_status_of_dtc = dem_get_status_of_dtc(client_id, &dtc_status);

	if (E_OK != get_status_of_dtc) {
		return DCM_E_REQUESTOUTOFRANGE;
	}

	uint8 record_number = pdu_rx_data->SduDataPtr[5];

	if (0xffu != record_number) {
		get_f_fby_dtc_return_code =
		dem_get_ff_data_by_dtc(dtc_number, DEM_DTC_ORIGIN_PRIMARY_MEMORY, record_number,
			&pdu_tx_data->SduDataPtr[6], &size_of_tx_buf);
	} else {
		get_f_fby_dtc_return_code = DEM_GET_FFDATABYDTC_WRONG_DTC;
		PduLengthType nof_bytes_copied = 0;
		PduLengthType buf_size_left = 0;
		dem_return_get_ff_data_by_dtc_type ret = DEM_GET_FFDATABYDTC_OK;

		for (uint8 record_index = 0;
			(record_index < record_number) && (DEM_GET_FFDATABYDTC_BUFFERSIZE != get_f_fby_dtc_return_code);
			record_index++) {
			buf_size_left = pdu_tx_data->SduLength - nof_bytes_copied - 6u;
			ret = dem_get_ff_data_by_dtc(dtc_number, DEM_DTC_ORIGIN_PRIMARY_MEMORY, record_index,
				&pdu_tx_data->SduDataPtr[6u + nof_bytes_copied], &buf_size_left);

			if (DEM_GET_FFDATABYDTC_OK == ret) {
				nof_bytes_copied += buf_size_left;
				get_f_fby_dtc_return_code = DEM_GET_FFDATABYDTC_OK;
			} else if (DEM_GET_FFDATABYDTC_BUFFERSIZE == ret) {
				get_f_fby_dtc_return_code = DEM_GET_FFDATABYDTC_BUFFERSIZE;
			} else {
				/* do nothing */
			}
		}
		size_of_tx_buf = nof_bytes_copied;
	}
	if (DEM_GET_FFDATABYDTC_OK == get_f_fby_dtc_return_code) {
		pdu_tx_data->SduDataPtr[0] = DCM_DSP_TX_DATA_BIT_0_POS_RESP;
		pdu_tx_data->SduDataPtr[1] = DCM_DSP_TX_DATA_BIT_1_SUB_ID_04;
		pdu_tx_data->SduDataPtr[2] = pdu_rx_data->SduDataPtr[2];
		pdu_tx_data->SduDataPtr[3] = pdu_rx_data->SduDataPtr[3];
		pdu_tx_data->SduDataPtr[4] = pdu_rx_data->SduDataPtr[4];
		pdu_tx_data->SduDataPtr[5] = dtc_status;
		pdu_tx_data->SduLength = size_of_tx_buf + 6u;
		response_code = DCM_E_POSITIVERESPONSE;
	} else if (DEM_GET_FFDATABYDTC_BUFFERSIZE == get_f_fby_dtc_return_code) {
		response_code = DCM_E_RESPONSETOOLONG;
	} else {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_read_dtc_info_sub_0x03(uint8 client_id, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	uint16 num_filtered_records = 0;
	uint32 dtc = 0;
	uint8 record_number = 0;
	uint16 nof_bytes_copied = 0;

	if ((E_OK != dem_set_ff_record_filter(client_id, DEM_DTC_FORMAT_UDS)) ||
		(E_OK != dem_get_number_of_ff_records(client_id, &num_filtered_records))) {
			response_code = DCM_E_REQUESTOUTOFRANGE;
			pdu_tx_data->SduDataPtr[0] = DCM_DSP_TX_DATA_BIT_0_POS_RESP;
			pdu_tx_data->SduDataPtr[1] = DCM_DSP_TX_DATA_BIT_1_SUB_ID_03;
			pdu_tx_data->SduLength = (PduLengthType)(SID_LEN + (uint16)SF_LEN + nof_bytes_copied);
			return response_code;
		}
	if ((((DTC_LEN + FF_REC_NUM_LEN) * num_filtered_records) + SID_LEN + SF_LEN) >
		pdu_tx_data->SduLength) {
			response_code = DCM_E_RESPONSETOOLONG;
			pdu_tx_data->SduDataPtr[0] = DCM_DSP_TX_DATA_BIT_0_POS_RESP;
			pdu_tx_data->SduDataPtr[1] = DCM_DSP_TX_DATA_BIT_1_SUB_ID_03;
			pdu_tx_data->SduLength = (PduLengthType)(SID_LEN + (uint16)SF_LEN + nof_bytes_copied);
			return response_code;
		}

	uint16 i = 0;

	while ((i < num_filtered_records) && (DCM_E_POSITIVERESPONSE == response_code)) {
		if (E_OK != dem_get_next_filtered_record(client_id, &dtc, &record_number)) {
			response_code = DCM_E_REQUESTOUTOFRANGE;
		} else {
			pdu_tx_data->SduDataPtr[SID_LEN + (uint16)SF_LEN + nof_bytes_copied] =
				dsp_get_byte_from_dword_by_index(dtc, 2u);
			nof_bytes_copied++;
			pdu_tx_data->SduDataPtr[SID_LEN + (uint16)SF_LEN + nof_bytes_copied] =
				dsp_get_byte_from_dword_by_index(dtc, 1u);
			nof_bytes_copied++;
			pdu_tx_data->SduDataPtr[SID_LEN + (uint16)SF_LEN + nof_bytes_copied] =
				dsp_get_byte_from_dword_by_index(dtc, 0u);
			nof_bytes_copied++;
			pdu_tx_data->SduDataPtr[SID_LEN + (uint16)SF_LEN + nof_bytes_copied] =
				record_number;
			nof_bytes_copied++;
		}
		i++;
	}

	pdu_tx_data->SduDataPtr[0] = DCM_DSP_TX_DATA_BIT_0_POS_RESP;
	pdu_tx_data->SduDataPtr[1] = DCM_DSP_TX_DATA_BIT_1_SUB_ID_03;
	pdu_tx_data->SduLength = (PduLengthType)(SID_LEN + (uint16)SF_LEN + nof_bytes_copied);

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_switch_read_dtc_info_sub(uint8 client_id,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 sub_function_number)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if ((0x01 == sub_function_number) || (0x07 == sub_function_number) || (0x11 == sub_function_number) ||
	(0x12 == sub_function_number)) {
		response_code = dsp_read_dtc_info_sub_0x01_0x07_0x11_0x12(client_id, pdu_rx_data, pdu_tx_data);
	} else if ((0x02 == sub_function_number) || (0x0a == sub_function_number) || (0x0f == sub_function_number) ||
	(0x13 == sub_function_number) || (0x15 == sub_function_number)) {
		response_code = dsp_read_dtc_info_sub_0x02_0x0a_0x0f_0x13_0x15(client_id, pdu_rx_data, pdu_tx_data);
	} else if (0x03 == sub_function_number) {
		response_code = dsp_read_dtc_info_sub_0x03(client_id, pdu_tx_data);
	} else if (0x04 == sub_function_number) {
		response_code = dsp_read_dtc_info_sub_0x04(client_id, pdu_rx_data, pdu_tx_data);
	} else if ((0x06 == sub_function_number) || (0x10 == sub_function_number)) {
		response_code = dsp_read_dtc_info_sub_0x06_0x10(client_id, pdu_rx_data, pdu_tx_data);
	} else {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	return response_code;
}

FUNC(void, DCM_CODE)
dsp_uds_read_dtc_info(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id)
{
	const uint8 data_size[DCM_DSP_READ_DTC_INFO_LENGTH] = {
		0, 3, 3, 2, 6, 3, 6, 4, 4, 5, 2, 2, 2, 2, 2, 3, 6, 3, 3, 3, 2, 2};
	const boolean enable_sub_func[DCM_DSP_READ_DTC_INFO_LENGTH] = {
		FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
		FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE
	};

	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	uint8 sub_function_number = pdu_rx_data->SduDataPtr[1];
	if (sub_function_number > DCM_DSP_READ_DTC_INFO_SUB_ID_0x15) {
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		dsd_dsp_processing_done(response_code);
		return;
	}
	if (!enable_sub_func[sub_function_number]) {
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		dsd_dsp_processing_done(response_code);
		return;
	}
	response_code = (pdu_rx_data->SduLength == data_size[sub_function_number])
		? dsp_do_switch_read_dtc_info_sub(client_id, pdu_rx_data, pdu_tx_data, sub_function_number)
		: DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	dsd_dsp_processing_done(response_code);
}
#endif

static FUNC(void, DCM_CODE) dsp_cancel_pending_did(uint16 did_nr, uint16 signal_index,
	read_did_pending_state_type pending_state, const PduInfoType *pdu_tx_data)
{
#if (DCM_DSP_DATA_CONDITION_CHECK_READ_FNC_ENABLED == STD_OFF)
	(void)pdu_tx_data;
#endif
	const dcm_dsp_did_type *did_ptr = NULL_PTR;

	if (lookup_non_dynamic_did(did_nr, &did_ptr)) {
		if (signal_index < did_ptr->dsp_nof_signals) {
			if (DCM_READ_DID_PENDING_COND_CHECK == pending_state) {
#if (DCM_DSP_DATA_CONDITION_CHECK_READ_FNC_ENABLED == STD_ON)
				const dcm_dsp_data_type *data_ptr =
					did_ptr->dsp_signal_ref[signal_index].dsp_signal_data_ref;

				if ((data_ptr->dsp_data_condition_check_read_fnc.asynch_condition_check_read_fnc !=
					NULL_PTR) && (DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port)) {
					(void)data_ptr->dsp_data_condition_check_read_fnc
						.asynch_condition_check_read_fnc(DCM_CANCEL, pdu_tx_data->SduDataPtr);
				}
#endif
			} else if (DCM_READ_DID_PENDING_READ_DATA == pending_state) {
#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON)
				const dcm_dsp_data_type *data_ptr =
					did_ptr->dsp_signal_ref[signal_index].dsp_signal_data_ref;

				if ((data_ptr->dsp_data_read_data_fnc.asynch_data_read_fnc != NULL_PTR) &&
					(DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port)) {
					(void)data_ptr->dsp_data_read_data_fnc.asynch_data_read_fnc(DCM_CANCEL,
						pdu_tx_data->SduDataPtr);
				}
#endif
			} else {
				DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
			}
		} else {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
		}
	}
}

FUNC(void, DCM_CODE) dsp_uds_read_data_by_id(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	uint16 did_nr;
	const dcm_dsp_did_type *did_ptr = NULL_PTR;
#if (defined(DCM_DSP_DID_RANGE_ENABLED) && (DCM_DSP_DID_RANGE_ENABLED == STD_ON))
	const dcm_dsp_did_range_type *did_range_ptr = NULL_PTR;
#endif

	boolean no_requested_did_supported = TRUE;
	uint16 req_did_start_index = 0u;
	uint16 nof_dids_read_in_pending_req = 0u;

	dsp_uds_read_did_pending_type temp_read_did_pending;
	(void)memset(&temp_read_did_pending, 0, sizeof(dsp_uds_read_did_pending_type));
	temp_read_did_pending.tx_write_pos = 1u;

	if ((((pdu_rx_data->SduLength - 1u) % 2u) == 0u) && (pdu_rx_data->SduLength >= 3u) &&
		(pdu_rx_data->SduLength <= ((DCM_MAX_NOF_DID_TO_READ * 2u) + 1u))) {
		uint8 local_rx_sdu_data[(DCM_MAX_NOF_DID_TO_READ * 2u) + 1u];
		(void)memcpy(local_rx_sdu_data, pdu_rx_data->SduDataPtr, pdu_rx_data->SduLength);
		if (DCM_READ_DID_IDLE != dsp_read_did_pending.state) {
			nof_dids_read_in_pending_req = dsp_read_did_pending.nof_read_dids;
			req_did_start_index = dsp_read_did_pending.req_did_index;
			temp_read_did_pending.state = dsp_read_did_pending.state;
			temp_read_did_pending.tx_write_pos = dsp_read_did_pending.tx_write_pos;
			temp_read_did_pending.pending_data_length = dsp_read_did_pending.pending_data_length;
			temp_read_did_pending.pending_signal_index = dsp_read_did_pending.pending_signal_index;
			temp_read_did_pending.pending_data_start_pos = dsp_read_did_pending.pending_data_start_pos;
		}
		uint16 i = req_did_start_index;

		while ((i < (uint16)((pdu_rx_data->SduLength - 1u) / 2u)) && (response_code == DCM_E_POSITIVERESPONSE)) {
			did_nr = (uint16)((uint16)local_rx_sdu_data[1u + (i * 2u)] << 8u) +
				local_rx_sdu_data[2u + (i * 2u)];
			if (!lookup_non_dynamic_did(did_nr, &did_ptr)) {
#if (defined(DCM_DSP_DID_RANGE_ENABLED) && (DCM_DSP_DID_RANGE_ENABLED == STD_ON))
				if (lookup_did_range(did_nr, &did_range_ptr)) {
					no_requested_did_supported = FALSE;
					response_code = read_did_range_data(did_nr, nof_dids_read_in_pending_req,
						did_range_ptr, pdu_tx_data);
					if (DCM_E_RESPONSEPENDING == response_code) {
						(void)memcpy(pdu_rx_data->SduDataPtr, local_rx_sdu_data,
							pdu_rx_data->SduLength);
						dsp_read_did_pending.req_did_index = i;
					} else {
						nof_dids_read_in_pending_req = 0u;
						temp_read_did_pending.nof_read_dids = 0u;
					}
				}
#endif
			} else {
				no_requested_did_supported = FALSE;
				response_code = read_did_data(nof_dids_read_in_pending_req, did_ptr, pdu_tx_data,
					&temp_read_did_pending);
				if (DCM_E_RESPONSEPENDING == response_code) {
					(void)memcpy(pdu_rx_data->SduDataPtr, local_rx_sdu_data, pdu_rx_data->SduLength);
					dsp_read_did_pending.req_did_index = i;
				} else {
					nof_dids_read_in_pending_req = 0u;
					temp_read_did_pending.nof_read_dids = 0u;
				}
			}
			i++;
		}
	} else {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if ((response_code != DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT) && no_requested_did_supported) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}
	switch (response_code) {
		case DCM_E_POSITIVERESPONSE:
			pdu_tx_data->SduLength = temp_read_did_pending.tx_write_pos;
			break;
		case DCM_E_RESPONSEPENDING:
			dsp_read_did_pending.state = temp_read_did_pending.state;
			dsp_read_did_pending.pdu_rx_data = (const PduInfoType *)pdu_rx_data;
			dsp_read_did_pending.pdu_tx_data = pdu_tx_data;
			dsp_read_did_pending.nof_read_dids = temp_read_did_pending.nof_read_dids;
			dsp_read_did_pending.tx_write_pos = temp_read_did_pending.tx_write_pos;
			dsp_read_did_pending.pending_did = temp_read_did_pending.pending_did;
			dsp_read_did_pending.pending_data_length = temp_read_did_pending.pending_data_length;
			dsp_read_did_pending.pending_signal_index = temp_read_did_pending.pending_signal_index;
			dsp_read_did_pending.pending_data_start_pos = temp_read_did_pending.pending_data_start_pos;
			break;
		default:
			dsp_read_did_pending.state = DCM_READ_DID_IDLE;
			dsp_read_did_pending.nof_read_dids = 0;
			dsd_dsp_processing_done(response_code);
			break;
	}
}

#if (DCM_USE_SERVICE_WRITEDATABYIDENTIFIER == STD_ON)
#if (CONFIG_MODULE_NVM == 1) && defined(DCM_USE_NVM_DID)
static FUNC(dcm_nrc_type, DCM_CODE)	dsp_write_data_to_nvm_block(nvm_block_id_type block_id,
	uint8 *data, boolean is_pending)
{
	if (FALSE == is_pending) {
		nvm_set_block_lock_status(block_id, FALSE);
		if (E_OK != nvm_write_block(block_id, data)) {
			nvm_set_block_lock_status(block_id, TRUE);
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
			return DCM_E_GENERALREJECT;
		}
		return DCM_E_RESPONSEPENDING;
	}

	nvm_request_result_type error_status;

	if (E_OK != nvm_get_error_status(block_id, &error_status)) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		return DCM_E_GENERALREJECT;
	}

	dcm_nrc_type response_code;

	switch (error_status) {
		case NVM_REQ_PENDING:
			response_code = DCM_E_RESPONSEPENDING;
			break;
		case NVM_REQ_OK:
			response_code = DCM_E_POSITIVERESPONSE;
			nvm_set_block_lock_status(block_id, TRUE);
			break;
		default:
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
			response_code = DCM_E_GENERALREJECT;
			nvm_set_block_lock_status(block_id, TRUE);
			break;
	}
	return response_code;
}
#endif

static FUNC(dcm_nrc_type, DCM_CODE) dsp_write_data_for_non_block_id_port(const dcm_dsp_signal_type *signal_ptr,
	const PduInfoType *pdu_rx_data)
{
#if (DCM_DSP_DATA_WRITE_DATA_FNC_ENABLED != STD_ON)
	(void)signal_ptr;
	(void)pdu_rx_data;
	return DCM_E_REQUESTOUTOFRANGE;
#endif
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType result = E_OK;

	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	if (dsp_write_did_pending.state == DCM_GENERAL_PENDING) {
		op_status = (dcm_op_status_type)DCM_PENDING;
	}

	if (NULL_PTR == signal_ptr->dsp_signal_data_ref->dsp_data_write_data_fnc.fix_len_synch_data_write_fnc) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
		return response_code;
	}

#ifdef DCM_SUPPORT_DATA_PORT_SR
		if (DATA_PORT_SR == signal_ptr->dsp_signal_data_ref->dsp_data_use_port) {
			result = write_sr_signal(signal_ptr->dsp_signal_data_ref, signal_ptr->dsp_signal_bit_position,
				&pdu_rx_data->SduDataPtr[3]);
		} else
#endif
			if (DATA_PORT_SYNCH == signal_ptr->dsp_signal_data_ref->dsp_data_use_port) {
			if (signal_ptr->dsp_signal_data_ref->dsp_did_fixed_length) {
				result = signal_ptr->dsp_signal_data_ref->dsp_data_write_data_fnc
					.fix_len_synch_data_write_fnc(&pdu_rx_data->SduDataPtr
					[3u + (signal_ptr->dsp_signal_bit_position / 8u)], &response_code);
			}
		} else if (DATA_PORT_ASYNCH == signal_ptr->dsp_signal_data_ref->dsp_data_use_port) {
			if (signal_ptr->dsp_signal_data_ref->dsp_did_fixed_length) {
				result = signal_ptr->dsp_signal_data_ref->dsp_data_write_data_fnc
					.fix_len_asynch_data_write_fnc(&pdu_rx_data->SduDataPtr
					[3u + (signal_ptr->dsp_signal_bit_position / 8u)],
					op_status, &response_code);
			}
		} else {
			/* do nothing */
		}
		if ((DCM_E_RESPONSEPENDING == response_code) || (DCM_E_PENDING == result)) {
			response_code = DCM_E_RESPONSEPENDING;
		} else if ((result != E_OK) && (response_code == DCM_E_POSITIVERESPONSE)) {
			response_code = DCM_E_CONDITIONSNOTCORRECT;
		} else {
			/* do nothing */
		}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_write_did_data(const dcm_dsp_did_type *did_ptr,
	const PduInfoType *pdu_rx_data, uint16 write_did_len)
{
	if (did_ptr->dsp_did_info_ref->dsp_did_access.dsp_did_write == NULL_PTR) {
		return DCM_E_REQUESTOUTOFRANGE;
	}

	dcm_nrc_type response_code;
	if (!do_check_did_write_condition(did_ptr->dsp_did_info_ref->dsp_did_access.dsp_did_write,
		&response_code)) {
		return response_code;
	}

	if (did_ptr->dsp_signal_ref == NULL_PTR) {
		return DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if (did_ptr->dsp_signal_ref[0].dsp_signal_data_ref->dsp_did_fixed_length &&
		(write_did_len != did_ptr->dsp_did_data_byte_size)) {
		return DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	uint16 i = 0;

	while (i < did_ptr->dsp_nof_signals) {
		const dcm_dsp_signal_type *signal_ptr = &did_ptr->dsp_signal_ref[i];

		if (signal_ptr->dsp_signal_data_ref->dsp_data_use_port != DATA_PORT_BLOCK_ID) {
			response_code = dsp_write_data_for_non_block_id_port(signal_ptr, pdu_rx_data);
		} else {
#if (CONFIG_MODULE_NVM == 1) && defined(DCM_USE_NVM_DID)
			response_code = dsp_write_data_to_nvm_block
				(signal_ptr->dsp_signal_data_ref->dsp_nvm_use_block_id,
				&pdu_rx_data->SduDataPtr[3 + (signal_ptr->dsp_signal_bit_position / 8)],
				(dsp_write_did_pending.state == DCM_GENERAL_PENDING));
#endif
		}

		if (response_code != DCM_E_POSITIVERESPONSE) {
			break;
		}

		i++;
	}

	return response_code;
}

#if (defined(DCM_DSP_DID_RANGE_ENABLED) && (DCM_DSP_DID_RANGE_ENABLED == STD_ON))
static FUNC(dcm_nrc_type, DCM_CODE) dsp_write_did_range_data(uint16 did, const dcm_dsp_did_range_type *did_range_ptr,
	const PduInfoType *pdu_rx_data, uint16 write_did_len)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType result = E_NOT_OK;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	if (dsp_write_did_pending.state == DCM_GENERAL_PENDING) {
		op_status = (dcm_op_status_type)DCM_PENDING;
	}
	if (did_range_ptr->dsp_did_range_info_ref->dsp_did_access.dsp_did_write == NULL_PTR) {
		return DCM_E_REQUESTOUTOFRANGE;
	}
	if (!do_check_did_write_condition(did_range_ptr->dsp_did_range_info_ref->dsp_did_access.dsp_did_write,
		&response_code)) {
		return response_code;
	}
	if (did_range_ptr->read_did_range_data_length_fnc == NULL) {
		response_code = DCM_E_CONDITIONSNOTCORRECT;
		return response_code;
	}
	uint16 data_len = 0;
	uint16 diduserlength = 0;

	result = did_range_ptr->read_did_range_data_length_fnc(did, op_status, &diduserlength);
	if (E_OK == result) {
		data_len = diduserlength;
	} else {
		response_code = DCM_E_CONDITIONSNOTCORRECT;
		return response_code;
	}
	if (write_did_len != data_len) {
		return response_code;
	}
	if (NULL_PTR == did_range_ptr->did_range_write_data_fnc) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
		return response_code;
	}
	result = did_range_ptr->did_range_write_data_fnc(did, &pdu_rx_data->SduDataPtr[3],
		op_status, data_len, &response_code);
	if ((DCM_E_RESPONSEPENDING == response_code) || (DCM_E_PENDING == result)) {
		response_code = DCM_E_RESPONSEPENDING;
	} else if ((result != E_OK) && (response_code == DCM_E_POSITIVERESPONSE)) {
		response_code = DCM_E_CONDITIONSNOTCORRECT;
	} else {
		/* do nothing */
	}

	return response_code;
}
#endif

FUNC(void, DCM_CODE) dsp_uds_write_data_by_id(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
#if (defined(DCM_DSP_DID_RANGE_ENABLED) && (DCM_DSP_DID_RANGE_ENABLED == STD_ON))
	const dcm_dsp_did_range_type *did_range_ptr = NULL_PTR;
#endif

	if (pdu_rx_data->SduLength < 3u) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsp_write_did_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}

	uint16 did_data_length = (uint16)(pdu_rx_data->SduLength - 3U);
	uint16 did_nr = (uint16)((uint16)pdu_rx_data->SduDataPtr[1] << 8) + pdu_rx_data->SduDataPtr[2];
	const dcm_dsp_did_type *did_ptr = NULL_PTR;

	if (lookup_non_dynamic_did(did_nr, &did_ptr)) {
		response_code = dsp_write_did_data(did_ptr, pdu_rx_data, did_data_length);
		if (DCM_E_RESPONSEPENDING == response_code) {
			dsp_write_did_pending.state = DCM_GENERAL_PENDING;
			dsp_write_did_pending.pdu_rx_data = pdu_rx_data;
			dsp_write_did_pending.pdu_tx_data = pdu_tx_data;
			return;
		}

		if (DCM_E_POSITIVERESPONSE == response_code) {
			pdu_tx_data->SduLength = 3;
			pdu_tx_data->SduDataPtr[1] = (uint8)((did_nr >> 8) & 0xffu);
			pdu_tx_data->SduDataPtr[2] = (uint8)(did_nr & 0xffu);
		}

		dsp_write_did_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}
#if (defined(DCM_DSP_DID_RANGE_ENABLED) && (DCM_DSP_DID_RANGE_ENABLED == STD_ON))
	if (lookup_did_range(did_nr, &did_range_ptr)) {
		response_code = dsp_write_did_range_data(did_nr, did_range_ptr, pdu_rx_data, did_data_length);
		if (DCM_E_RESPONSEPENDING == response_code) {
			dsp_write_did_pending.state = DCM_GENERAL_PENDING;
			dsp_write_did_pending.pdu_rx_data = pdu_rx_data;
			dsp_write_did_pending.pdu_tx_data = pdu_tx_data;
			return;
		}

		if (DCM_E_POSITIVERESPONSE == response_code) {
			pdu_tx_data->SduLength = 3;
			pdu_tx_data->SduDataPtr[1] = (uint8)((did_nr >> 8) & 0xffu);
			pdu_tx_data->SduDataPtr[2] = (uint8)(did_nr & 0xffu);
		}

		dsp_write_did_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}
#endif
	response_code = DCM_E_REQUESTOUTOFRANGE;
	dsp_write_did_pending.state = DCM_GENERAL_IDLE;
	dsd_dsp_processing_done(response_code);
}
#endif

static FUNC(void, DCM_CODE) dsp_cancel_pending_security_access(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	if ((pdu_rx_data->SduDataPtr[1] < 0x01u) || (pdu_rx_data->SduDataPtr[1] > 0x42u)) {
		return;
	}
	boolean is_request_seed = FALSE;

	if ((pdu_rx_data->SduDataPtr[1] & 0x01u) == 0x01u) {
		is_request_seed = TRUE;
	}
	dcm_sec_lv_type requested_security_level = (pdu_rx_data->SduDataPtr[1] + 1u) / 2u;

	if (FALSE == is_request_seed) {
		(void)dsp_uds_security_access_compare_key_sub_fnc(pdu_rx_data, pdu_tx_data,
			requested_security_level, TRUE);
	} else {
		(void)dsp_uds_security_access_get_seed_sub_fnc(pdu_rx_data, pdu_tx_data,
			requested_security_level, TRUE);
	}
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_do_new_security_level_ask_for_seed(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data,
		const dcm_dsp_security_type *security_type,
		dcm_sec_lv_type requested_security_level, dcm_op_status_type op_status)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	dcm_nrc_type get_seed_error_code;

	if ((security_type->get_seed.get_seed_without_record != NULL_PTR) ||
		(security_type->get_seed.get_seed_with_record != NULL_PTR)) {
		Std_ReturnType get_seed_result;

		get_seed_result = (security_type->dsp_security_adr_size > 0u)
			? security_type->get_seed.get_seed_with_record(&pdu_rx_data->SduDataPtr[2], op_status,
				&pdu_tx_data->SduDataPtr[2], &get_seed_error_code)
			: security_type->get_seed.get_seed_without_record(op_status,
				&pdu_tx_data->SduDataPtr[2], &get_seed_error_code);

		switch (get_seed_result) {
			case DCM_E_PENDING:
				response_code = DCM_E_RESPONSEPENDING;
				break;
			case E_OK:
				pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
				pdu_tx_data->SduLength =
					(PduLengthType)((PduLengthType)2 + security_type->dsp_security_seed_size);
				dsp_security_acces_data.req_sec_level = requested_security_level;
				dsp_security_acces_data.req_sec_level_ref = security_type;
				dsp_security_acces_data.req_in_progress = TRUE;
				break;
			default:
				response_code = get_seed_error_code;
				break;
		}
	} else {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_uds_security_access_get_seed_sub_fnc(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data, dcm_sec_lv_type requested_security_level, boolean is_cancel)
{
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (dsp_security_access_pending.state == DCM_GENERAL_PENDING) {
		op_status = (is_cancel) ? (dcm_op_status_type)DCM_CANCEL : (dcm_op_status_type)DCM_PENDING;
	}

	uint8 security_index = 0;
	const dcm_dsp_security_type *security_type = NULL;

	for (security_index = 0; security_index < DCM_SECURITY_EOL_INDEX; security_index++) {
		security_type = &dcm_config_ptr->dsp->dsp_security[security_index];
		if (security_type->dsp_security_level == requested_security_level) {
			break;
		}
	}

	if (security_index >= DCM_SECURITY_EOL_INDEX) {
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		return response_code;
	}

#if (DCM_SECURITY_EOL_INDEX != 0)
	dsp_security_acces_data.curr_sec_lev_idx = security_index;
	if (dsp_security_acces_data.sec_false_attempt_chk[security_index].start_delay_timer != DELAY_TIMER_DEACTIVE) {
		return DCM_E_REQUIREDTIMEDELAYNOTEXPIRED;
	}
#endif

	if (pdu_rx_data->SduLength != ((PduLengthType)security_type->dsp_security_adr_size + 2u)) {
		return DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	dcm_sec_lv_type active_sec_level;

	if (dsl_get_security_level(&active_sec_level) != E_OK) {
		active_sec_level = DCM_SEC_LEV_LOCKED;
	}

	if ((PduLengthType)security_type->dsp_security_seed_size + 2u > pdu_tx_data->SduLength) {
		return DCM_E_RESPONSETOOLONG;
	}

	if (requested_security_level != active_sec_level) {
		return dsp_do_new_security_level_ask_for_seed(pdu_rx_data, pdu_tx_data, security_type,
			requested_security_level, op_status);
	}

	dsp_security_acces_data.req_in_progress = TRUE;
	pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
	(void)memset(&pdu_tx_data->SduDataPtr[2], 0, security_type->dsp_security_seed_size);
	pdu_tx_data->SduLength = (PduLengthType)security_type->dsp_security_seed_size + 2u;

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_check_and_run_compare_key_fnc(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data, dcm_op_status_type op_status, boolean is_cancel)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	const dcm_dsp_security_type *req_sec_level_ref = dsp_security_acces_data.req_sec_level_ref;

	if (pdu_rx_data->SduLength !=
		(PduLengthType)((PduLengthType)2 + req_sec_level_ref->dsp_security_key_size)) {
			response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			return response_code;
		}
	if (NULL_PTR == req_sec_level_ref->compare_key) {
		response_code = DCM_E_CONDITIONSNOTCORRECT;
		return response_code;
	}

	Std_ReturnType compare_key_result;

	compare_key_result = req_sec_level_ref->compare_key(&pdu_rx_data->SduDataPtr[2], op_status, &response_code);
	if (is_cancel) {
		response_code = DCM_E_POSITIVERESPONSE;
	} else if (compare_key_result == E_OK) {
		dsp_security_acces_data.req_in_progress = FALSE;
		dsl_internal_set_security_level(req_sec_level_ref->dsp_security_level);
		pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
		pdu_tx_data->SduLength = 2;
		global_compare_key_flag = TRUE;
		global_requested_security_level = req_sec_level_ref->dsp_security_level;
	} else if (compare_key_result == DCM_E_PENDING) {
		response_code = DCM_E_RESPONSEPENDING;
	} else {
		dsp_security_acces_data.req_in_progress = FALSE;
		response_code = DCM_E_INVALIDKEY;
		if ((TRUE == global_compare_key_flag) && (global_requested_security_level ==
			req_sec_level_ref->dsp_security_level)) {
			response_code = DCM_E_REQUESTSEQUENCEERROR;
		}
	}

	return response_code;
}

static FUNC(void, DCM_CODE) dsp_do_check_security_attempt_chk(dcm_nrc_type *response_code, dcm_op_status_type op_status)
{
#if (DCM_SECURITY_EOL_INDEX == 0)
	return;
#endif
#if (DCM_SEC_ATT_CNTR_ENABLED == STD_ON)
	uint8 attempt_count = 0;
#endif
	dsp_uds_security_access_chk_param *current_attempt =
		&dsp_security_acces_data.sec_false_attempt_chk[dsp_security_acces_data.curr_sec_lev_idx];
	const dcm_dsp_security_type *security_config =
		&dcm_config_ptr->dsp->dsp_security[dsp_security_acces_data.curr_sec_lev_idx];
	const dcm_dsp_security_type *req_sec_level_ref = dsp_security_acces_data.req_sec_level_ref;

	if (*response_code != DCM_E_INVALIDKEY) {
		if (*response_code == DCM_E_POSITIVERESPONSE) {
			uint8 attempt_count_before_reset = current_attempt->sec_acss_attempts;

			current_attempt->sec_acss_attempts = 0u;

#if (DCM_SEC_ATT_CNTR_ENABLED == STD_ON)
			if (attempt_count_before_reset != 0u) {
				attempt_count = current_attempt->sec_acss_attempts;
				if (req_sec_level_ref->set_attempt_counter != NULL) {
					(void)req_sec_level_ref->set_attempt_counter(op_status, attempt_count);
				}
			}
#else
			(void)attempt_count_before_reset;
#endif
		}
		return;
	}
	if (current_attempt->sec_acss_attempts < req_sec_level_ref->dsp_security_num_att_delay) {
		current_attempt->sec_acss_attempts += (uint8)1;
#if (DCM_SEC_ATT_CNTR_ENABLED == STD_ON)
		attempt_count = current_attempt->sec_acss_attempts;
		if (req_sec_level_ref->set_attempt_counter != NULL) {
			(void)req_sec_level_ref->set_attempt_counter(op_status, attempt_count);
		}
#endif
	}
	if (current_attempt->sec_acss_attempts < req_sec_level_ref->dsp_security_num_att_delay) {
			return;
		}

	current_attempt->start_delay_timer = DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE;
	if (security_config->dsp_security_delay_time >= DCM_MAIN_FUNCTION_PERIOD_TIME_MS) {
		current_attempt->timer_sec_acss_attempt = security_config->dsp_security_delay_time;
	} else {
		current_attempt->timer_sec_acss_attempt = DCM_MAIN_FUNCTION_PERIOD_TIME_MS;
	}
	if (security_config->dsp_security_delay_time > 0u) {
		*response_code = DCM_E_EXCEEDNUMBEROFATTEMPTS;
	}

	return;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_uds_security_access_compare_key_sub_fnc(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data, dcm_sec_lv_type requested_security_level, boolean is_cancel)
{
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (dsp_security_access_pending.state == DCM_GENERAL_PENDING) {
		op_status = (is_cancel) ? (dcm_op_status_type)DCM_CANCEL: (dcm_op_status_type)DCM_PENDING;
	}
	if (requested_security_level != dsp_security_acces_data.req_sec_level) {
#if defined(CFG_DCM_SECURITY_ACCESS_NRC_FIX)
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
#else
		response_code = DCM_E_REQUESTSEQUENCEERROR;
#endif
		return response_code;
	}
	if (!dsp_security_acces_data.req_in_progress) {
		response_code = DCM_E_REQUESTSEQUENCEERROR;
		return response_code;
	}

#if (DCM_SECURITY_EOL_INDEX != 0u)
	if (dsp_security_acces_data
			.sec_false_attempt_chk[dsp_security_acces_data.curr_sec_lev_idx]
			.start_delay_timer != DELAY_TIMER_DEACTIVE) {
		response_code = DCM_E_REQUIREDTIMEDELAYNOTEXPIRED;
		return response_code;
	}
#endif
	response_code = dsp_do_check_and_run_compare_key_fnc(pdu_rx_data, pdu_tx_data, op_status, is_cancel);
	dsp_do_check_security_attempt_chk(&response_code, op_status);

	return response_code;
}

static FUNC(boolean, DCM_CODE) dsp_lookup_routine(uint16 routine_id, const dcm_dsp_routine_type **routine_ptr)
{
	const dcm_dsp_routine_type *dsp_routine = dcm_config_ptr->dsp->dsp_routine;
	boolean routine_found = FALSE;

	for (uint16 index = 0; index < DCM_RID_LIST_EOL_INDEX; index++) {
		if (dsp_routine[index].dsp_routine_identifier == routine_id) {
			*routine_ptr = &dsp_routine[index];
			routine_found = TRUE;
			break;
		}
	}

	return routine_found;
}

FUNC(void, DCM_CODE) dsp_uds_security_access(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (pdu_rx_data->SduLength < 2u) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsp_security_access_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}
	if ((pdu_rx_data->SduDataPtr[1] < 0x01u) || (pdu_rx_data->SduDataPtr[1] > 0x42u)) {
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		dsp_security_access_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}

	boolean is_request_seed = (boolean)(pdu_rx_data->SduDataPtr[1] & 0x01u);
	dcm_sec_lv_type requested_security_level = (pdu_rx_data->SduDataPtr[1] + 1u) / 2u;

	response_code = (is_request_seed) ? dsp_uds_security_access_get_seed_sub_fnc(pdu_rx_data,
		pdu_tx_data, requested_security_level, FALSE)
		: dsp_uds_security_access_compare_key_sub_fnc(pdu_rx_data, pdu_tx_data, requested_security_level, FALSE);

	if (DCM_E_RESPONSEPENDING == response_code) {
		dsp_security_access_pending.state = DCM_GENERAL_PENDING;
		dsp_security_access_pending.pdu_rx_data = pdu_rx_data;
		dsp_security_access_pending.pdu_tx_data = pdu_tx_data;
	} else {
		dsp_security_access_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
	}
}

static FUNC(void, DCM_CODE) dsp_set_op_status_depend_on_state(const dsp_uds_general_pending_type *p_general_pending,
		dcm_op_status_type *op_status, boolean is_cancel)
{
	if ((p_general_pending->state == DCM_GENERAL_PENDING) ||
		(p_general_pending->state == DCM_GENERAL_FORCE_RCRRP)) {
		if (is_cancel) {
			*op_status = (dcm_op_status_type)DCM_CANCEL;
		} else {
			*op_status = (p_general_pending->state == DCM_GENERAL_FORCE_RCRRP)
				? (dcm_op_status_type)DCM_FORCE_RCRRP_OK : (dcm_op_status_type)DCM_PENDING;
		}
	}
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_start_routine(const dcm_dsp_routine_type *routine_ptr,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, boolean is_cancel)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routine_result;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	dsp_set_op_status_depend_on_state(&dsp_routine_control_pending, &op_status, is_cancel);

	if (NULL_PTR == routine_ptr->dsp_start_routine_fnc) {
		return DCM_E_REQUESTOUTOFRANGE;
	}
	if ((routine_ptr->dsp_start_routine_ctrl_opt_rec_size + 4u) != pdu_rx_data->SduLength) {
		return DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if ((routine_ptr->dsp_start_routine_sts_opt_rec_size + 4u) > pdu_tx_data->SduLength) {
		return DCM_E_RESPONSETOOLONG;
	}

	PduLengthType current_data_length = pdu_rx_data->SduLength - 4u;

	routine_result = routine_ptr->dsp_start_routine_fnc(&pdu_rx_data->SduDataPtr[4], op_status,
		&pdu_tx_data->SduDataPtr[4], &current_data_length, &response_code);
	if (routine_result == DCM_E_PENDING) {
		response_code = DCM_E_RESPONSEPENDING;
		return response_code;
	}
	if (routine_result == DCM_E_FORCE_RCRRP) {
		response_code = DCM_E_FORCE_RCRRP;
		return response_code;
	}
	if (routine_result == E_OK) {
		response_code = DCM_E_POSITIVERESPONSE;
		pdu_tx_data->SduLength = current_data_length + 4u;
	} else if (DCM_E_POSITIVERESPONSE == response_code) {
		response_code = DCM_E_CONDITIONSNOTCORRECT;
	} else {
		/* do nothing */
	}

	if ((response_code == DCM_E_POSITIVERESPONSE) && (pdu_tx_data->SduLength >
		((PduLengthType)routine_ptr->dsp_start_routine_sts_opt_rec_size + 4u))) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_PARAM);
		pdu_tx_data->SduLength = routine_ptr->dsp_start_routine_sts_opt_rec_size + 4u;
	}
	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_stop_routine(const dcm_dsp_routine_type *routine_ptr,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, boolean is_cancel)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routine_result;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;
	const PduLengthType sts_opt_rec_size = routine_ptr->dsp_stop_routine_sts_opt_rec_size;
	const PduLengthType ctrl_opt_rec_size = routine_ptr->dsp_stop_routine_ctrl_opt_rec_size;

	dsp_set_op_status_depend_on_state(&dsp_routine_control_pending, &op_status, is_cancel);
	if (routine_ptr->dsp_stop_routine_fnc == NULL_PTR) {
		return DCM_E_SUBFUNCTIONNOTSUPPORTED;
	}
	if ((ctrl_opt_rec_size + 4U) != pdu_rx_data->SduLength) {
		return DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if ((sts_opt_rec_size + 4U) > pdu_tx_data->SduLength) {
		return DCM_E_RESPONSETOOLONG;
	}

	PduLengthType current_data_length = pdu_rx_data->SduLength - 4u;

	routine_result = routine_ptr->dsp_stop_routine_fnc(&pdu_rx_data->SduDataPtr[4], op_status,
		&pdu_tx_data->SduDataPtr[4], &current_data_length, &response_code);

	switch(routine_result) {
		case DCM_E_FORCE_RCRRP:
			response_code = DCM_E_FORCE_RCRRP;
			break;
		case DCM_E_PENDING:
			response_code = DCM_E_RESPONSEPENDING;
			break;
		case E_OK:
			response_code = DCM_E_POSITIVERESPONSE;
			pdu_tx_data->SduLength = current_data_length + 4u;
			break;
		default:
			if (DCM_E_POSITIVERESPONSE == response_code) {
				response_code = DCM_E_CONDITIONSNOTCORRECT;
			}
			break;

	}

	if ((response_code == DCM_E_POSITIVERESPONSE) && (pdu_tx_data->SduLength > (sts_opt_rec_size + 4u))) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_PARAM);
		pdu_tx_data->SduLength = sts_opt_rec_size + 4u;
	}
	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_req_routine_results(const dcm_dsp_routine_type *routine_ptr,
	PduInfoType *pdu_tx_data, boolean is_cancel)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;
	const PduLengthType rtn_ctrl_opt_rec_size = routine_ptr->dsp_req_res_rtn_ctrl_opt_rec_size;

	dsp_set_op_status_depend_on_state(&dsp_routine_control_pending, &op_status, is_cancel);
	if (routine_ptr->dsp_request_result_routine_fnc == NULL_PTR) {
		return DCM_E_SUBFUNCTIONNOTSUPPORTED;
	}
	if ((rtn_ctrl_opt_rec_size + 4u) > pdu_tx_data->SduLength) {
		return DCM_E_RESPONSETOOLONG;
	}

	PduLengthType current_data_length = 0u;

	Std_ReturnType routine_result = routine_ptr->dsp_request_result_routine_fnc(op_status,
		&pdu_tx_data->SduDataPtr[4], &current_data_length, &response_code);

	switch(routine_result) {
		case DCM_E_FORCE_RCRRP:
			response_code = DCM_E_FORCE_RCRRP;
			break;
		case DCM_E_PENDING:
			response_code = DCM_E_RESPONSEPENDING;
			break;
		case E_OK:
			response_code = DCM_E_POSITIVERESPONSE;
			pdu_tx_data->SduLength = current_data_length + 4u;
			break;
		default:
			if (DCM_E_POSITIVERESPONSE == response_code) {
				response_code = DCM_E_CONDITIONSNOTCORRECT;
			}
			break;

	}

	if ((response_code == DCM_E_POSITIVERESPONSE) && (pdu_tx_data->SduLength > (rtn_ctrl_opt_rec_size + 4u))) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_PARAM);
		pdu_tx_data->SduLength = rtn_ctrl_opt_rec_size + 4u;
	}
	return response_code;
}

static FUNC(void, DCM_CODE) dsp_cancel_pending_routine(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	if ((NULL == pdu_rx_data) || (NULL == pdu_tx_data)) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_PARAM);
		return;
	}
	uint8 sub_function_number = pdu_rx_data->SduDataPtr[1];
	uint16 routine_id = 0;
	const dcm_dsp_routine_type *routine_ptr = NULL_PTR;

	routine_id = (uint16)((uint16)pdu_rx_data->SduDataPtr[2] << 8) + pdu_rx_data->SduDataPtr[3];

	if (!dsp_lookup_routine(routine_id, &routine_ptr)) {
		return;
	}
	if (0x01 == sub_function_number) {
		(void)dsp_start_routine(routine_ptr, pdu_rx_data, pdu_tx_data, TRUE);
	} else if (0x02 == sub_function_number) {
		(void)dsp_stop_routine(routine_ptr, pdu_rx_data, pdu_tx_data, TRUE);
	} else if (0x03 == sub_function_number) {
		(void)dsp_req_routine_results(routine_ptr, pdu_tx_data, TRUE);
	} else {
		/* do nothing */
	}

}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_excute_routine_ctrl(uint8 sub_function_number,
	const dcm_dsp_routine_type *routine_ptr, const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType result = E_NOT_OK;

	result = dcm_mode_mode_rule_cbk_by_idx
	(routine_ptr->dsp_routine_authorization.dsp_routine_mode_rule_index, &response_code);
	if (result != E_OK) {
		return response_code;
	}

	if (0x01 == sub_function_number) {
		response_code = dsp_start_routine(routine_ptr, pdu_rx_data, pdu_tx_data, FALSE);
	} else if (0x02 == sub_function_number) {
		response_code = dsp_stop_routine(routine_ptr, pdu_rx_data, pdu_tx_data, FALSE);
	} else {
		response_code = (0x03 == sub_function_number)
			? dsp_req_routine_results(routine_ptr, pdu_tx_data, FALSE)
			: DCM_E_SUBFUNCTIONNOTSUPPORTED;
	}

	return response_code;
}

FUNC(void, DCM_CODE) dsp_uds_routine_ctrl(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (pdu_rx_data->SduLength < 4u) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsp_routine_control_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}

	uint8 sub_function_number = pdu_rx_data->SduDataPtr[1];
	if ((sub_function_number <= 0u) || (sub_function_number >= 4u)) {
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		dsp_routine_control_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}
	uint16 routine_id = (uint16)((uint16)pdu_rx_data->SduDataPtr[2] << 8) + pdu_rx_data->SduDataPtr[3];
	const dcm_dsp_routine_type *routine_ptr = NULL_PTR;

	if (!dsp_lookup_routine(routine_id, &routine_ptr)) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
		dsp_routine_control_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}

	if (do_check_routine_control_condition(routine_ptr->dsp_routine_authorization, &response_code)) {
		response_code = dsp_do_excute_routine_ctrl(sub_function_number, routine_ptr, pdu_rx_data, pdu_tx_data);
	}

	switch (response_code) {
		case DCM_E_RESPONSEPENDING:
			dsp_routine_control_pending.state = DCM_GENERAL_PENDING;
			dsp_routine_control_pending.pdu_rx_data = pdu_rx_data;
			dsp_routine_control_pending.pdu_tx_data = pdu_tx_data;
			break;
		case DCM_E_FORCE_RCRRP:
			dsp_routine_control_pending.state = DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND;
			dsp_routine_control_pending.pdu_rx_data = pdu_rx_data;
			dsp_routine_control_pending.pdu_tx_data = pdu_tx_data;
			dsd_dsp_force_response_pending();
			break;
		default:
			if (response_code == DCM_E_POSITIVERESPONSE) {
				pdu_tx_data->SduDataPtr[1] = sub_function_number;
				pdu_tx_data->SduDataPtr[2] = (uint8)((routine_id >> (uint8)8) & (uint16)0x00ffu);
				pdu_tx_data->SduDataPtr[3] = (uint8)(routine_id & 0xffu);
			}
			dsp_routine_control_pending.state = DCM_GENERAL_IDLE;
			dsd_dsp_processing_done(response_code);
			break;
	}
}

FUNC(void, DCM_CODE) dsp_uds_tester_present(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	if (pdu_rx_data->SduLength != (PduLengthType)TESTER_PRESENT_SDU_LEN) {
		dsd_dsp_processing_done(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
		return;
	}

	switch (pdu_rx_data->SduDataPtr[1] & DCM_FORMAT_LOW_MASK) {
		case ZERO_SUB_FUNCTION:
			dsl_reset_session_timeout_timer();

			pdu_tx_data->SduDataPtr[1] = ZERO_SUB_FUNCTION;
			pdu_tx_data->SduLength = 2;
			dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
			break;

		default:
			dsd_dsp_processing_done(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			break;
	}
}

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
static FUNC(void, DCM_CODE) dsp_enable_dtc_setting(uint8 client_id)
{
	if (!dsp_dtc_setting.setting_disabled) {
		return;
	}

	dcm_ses_ctrl_type current_session = DCM_DEFAULT_SESSION;

	if (E_OK != dsl_get_ses_ctrl_type(&current_session)) {
		current_session = DCM_DEFAULT_SESSION;
	}
	if (((DCM_DEFAULT_SESSION != current_session) &&
		dsd_dsp_check_service_supported_in_active_session_and_security(DCM_SID_CONTROL_DTC_SETTING))) {
		return;
	}
	if (E_OK != dem_enable_dtc_setting(client_id)) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
	}
#if (CONFIG_MODULE_RTE == 1)
			(void)Rte_Switch_ControlDTCSettingModeSwitchInterface_DcmControlDTCSetting
			(RTE_MODE_DcmControlDTCSetting_ENABLEDTCSETTING);
#endif
	dsp_dtc_setting.setting_disabled = FALSE;
}

FUNC(void, DCM_CODE)
dsp_uds_ctrl_dtc_set(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id)
{
	Std_ReturnType result_code;

	if (pdu_rx_data->SduLength != 2u) {
		dsd_dsp_processing_done(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
		return;
	}
	if (0x01 == pdu_rx_data->SduDataPtr[1]) {
		result_code = dem_enable_dtc_setting(client_id);
		if (result_code == E_OK) {
			pdu_tx_data->SduDataPtr[1] = 0x01;
			pdu_tx_data->SduLength = 2;
			dsp_dtc_setting.setting_disabled = FALSE;
#if (CONFIG_MODULE_RTE == 1)
			(void)Rte_Switch_ControlDTCSettingModeSwitchInterface_DcmControlDTCSetting
			(DCM_ENABLEDTCSETTING);
#endif
			dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
		} else {
			dsd_dsp_processing_done(DCM_E_REQUESTOUTOFRANGE);
		}
	} else if (0x02 == pdu_rx_data->SduDataPtr[1]) {
		result_code = dem_disable_dtc_setting(client_id);
		if (result_code == E_OK) {
			pdu_tx_data->SduDataPtr[1] = 0x02;
			pdu_tx_data->SduLength = 2;
			dsp_dtc_setting.setting_disabled = TRUE;
			dsp_dtc_setting.dtc_group = DEM_DTC_GROUP_ALL_DTCS;

#if (CONFIG_MODULE_RTE == 1)
			(void)Rte_Switch_ControlDTCSettingModeSwitchInterface_DcmControlDTCSetting
			(DCM_DISABLEDTCSETTING);
#endif
			dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
		} else {
			dsd_dsp_processing_done(DCM_E_REQUESTOUTOFRANGE);
		}
	} else {
		dsd_dsp_processing_done(DCM_E_SUBFUNCTIONNOTSUPPORTED);
	}
}
#endif

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
FUNC(void, DCM_CODE) dsp_response_pending_confirmed(PduIdType confirm_pdu_id)
{
	if ((DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM != dsp_session_ctrl_data.jump_to_boot_state) &&
		(DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM != dsp_session_ctrl_data.jump_to_boot_state)) {
			return;
	}
	if (confirm_pdu_id != dsp_session_ctrl_data.session_pdu_id) {
		return;
	}

	switch (dsp_session_ctrl_data.jump_to_boot_state) {
		case DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM:
			Std_ReturnType status = E_NOT_OK;

			if (E_OK == dcm_set_prog_conditions(&dsp_prog_conditions)) {
#ifdef DCM_CALLBACK_ECURESET_ENABLED
				if (E_OK == dcm_callback_dcm_ecu_reset(RTE_MODE_DcmEcuReset_EXECUTE)) {
					status = E_OK;
				}
#else
#if (CONFIG_MODULE_RTE == 1)
				if (E_OK == Rte_Switch_EcuResetModeSwitchInterface_DcmEcuReset
					(RTE_MODE_DcmEcuReset_EXECUTE)) {
					status = E_OK;
				}
#else
			status = E_OK;
#endif
#endif
			} else {
				DCM_DET_REPORTERROR(DCM_E_SET_PROG_CONDITIONS_FAIL);
			}
			if (E_OK != status) {
				dsd_dsp_processing_done(DCM_E_CONDITIONSNOTCORRECT);
			}
			dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_IDLE;
			break;
		case DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM:
			dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE;
			break;
		default:
			break;
	}
}
#endif

FUNC(void, DCM_CODE) dsp_dcm_confirmation(PduIdType confirm_pdu_id, Std_ReturnType result)
{
	dsl_reset_session_timeout_timer();
#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
	if (confirm_pdu_id == dsp_session_ctrl_data.session_pdu_id) {
		if ((DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM == dsp_session_ctrl_data.jump_to_boot_state) &&
			((NotifResultType)NTFRSLT_OK == result)) {
			if (E_OK != dcm_set_prog_conditions(&dsp_prog_conditions)) {
				DCM_DET_REPORTERROR(DCM_E_SET_PROG_CONDITIONS_FAIL);
			} else {
#ifdef DCM_CALLBACK_ECURESET_ENABLED
				(void)dcm_callback_dcm_ecu_reset(RTE_MODE_DcmEcuReset_EXECUTE);
#else
#if (CONFIG_MODULE_RTE == 1)
					(void)Rte_Switch_EcuResetModeSwitchInterface_DcmEcuReset
					(RTE_MODE_DcmEcuReset_EXECUTE);
#endif
#endif
			}
		}
		dsp_session_ctrl_data.jump_to_boot_state = DCM_JTB_IDLE;
	}
#endif

	if ((confirm_pdu_id == dsp_ecu_reset_data.reset_pdu_id) && (DCM_DSP_RESET_WAIT_TX_CONF ==
		dsp_ecu_reset_data.reset_pending)) {
			if ((NotifResultType)NTFRSLT_OK == result) {
#if (CONFIG_MODULE_RTE == 1)
				(void)Rte_Switch_EcuResetModeSwitchInterface_DcmEcuReset
				(RTE_MODE_DcmEcuReset_EXECUTE);
#endif
				dsl_set_ses_ctrl_type(DCM_DEFAULT_SESSION);
			}
			dsp_ecu_reset_data.reset_pending = DCM_DSP_RESET_NO_RESET;
	}

	if ((TRUE == dsp_session_ctrl_data.pending_session_change) && (confirm_pdu_id ==
		dsp_session_ctrl_data.session_pdu_id)) {
		if (E_OK == result) {
			dsl_set_ses_ctrl_type(dsp_session_ctrl_data.session);
		}
		dsp_session_ctrl_data.pending_session_change = FALSE;
	}
#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
	if ((dsp_comm_ctrl_data.com_ctrl_pending) && (confirm_pdu_id == dsp_comm_ctrl_data.confirm_pdu)) {
		if (E_OK == result) {
			(void)dsp_internal_comm_ctrl(dsp_comm_ctrl_data.sub_func,
				dsp_comm_ctrl_data.subnet, dsp_comm_ctrl_data.com_type,
				dsp_comm_ctrl_data.rx_pdu, TRUE);
		}
		dsp_comm_ctrl_data.com_ctrl_pending = FALSE;
	}
#endif
}

#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON) || (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
static FUNC_P2CONST(dcm_dsp_memory_range_info, AUTOMATIC, DCM_CODE)
	dsp_find_memory_range(dsp_memory_service_type service_type, const dcm_dsp_memory_type *memory_info_ptr,
		uint32 memory_address, uint32 memory_size)
{
	const dcm_dsp_memory_range_info *memory_range = NULL_PTR;

	if (DCM_READ_MEMORY != service_type) {
		return memory_range;
	}

	if (memory_info_ptr->read_memory_range_info_length > 0u) {
		uint8 read_mem_idx = 0;

		while (read_mem_idx < memory_info_ptr->read_memory_range_info_length) {
			if ((memory_address >= memory_info_ptr->read_memory_info[read_mem_idx].memory_address_low) &&
				(memory_address <= memory_info_ptr->read_memory_info[read_mem_idx].memory_address_high) &&
				((memory_address + memory_size - 1u) <=
				memory_info_ptr->read_memory_info[read_mem_idx].memory_address_high)) {
				memory_range = &memory_info_ptr->read_memory_info[read_mem_idx];
			}
			read_mem_idx++;
		}
	}
	if (memory_info_ptr->write_memory_range_info_length > 0u) {
		uint8 write_mem_idx = 0;

		while (write_mem_idx < memory_info_ptr->write_memory_range_info_length) {
			if ((memory_address >= memory_info_ptr->write_memory_info[write_mem_idx].memory_address_low) &&
				(memory_address <= memory_info_ptr->write_memory_info[write_mem_idx].memory_address_high) &&
				((memory_address + memory_size - 1u) <=
				memory_info_ptr->write_memory_info[write_mem_idx].memory_address_high)) {
				memory_range = &memory_info_ptr->write_memory_info[write_mem_idx];
			}
			write_mem_idx++;
		}
	}
	return memory_range;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_check_address_range(dsp_memory_service_type service_type, uint32 memory_address, uint32 memory_size)
{
	uint8 i = 0;
	const dcm_dsp_memory_range_info *memory_range_info = NULL_PTR;
	const dcm_dsp_memory_type *dsp_memory_info = dcm_config_ptr->dsp->dsp_memory;
	dcm_nrc_type diag_response_code = DCM_E_REQUESTOUTOFRANGE;

	while (i < DCM_DSP_MEMORY_LIST_LENGTH) {
		memory_range_info = dsp_find_memory_range(service_type, &dsp_memory_info[i], memory_address, memory_size);

		if (NULL != memory_range_info) {
			if (dsp_check_session_level(memory_range_info->memory_range_session_mask)) {
				diag_response_code = dsp_check_security_level(memory_range_info->memory_range_security_mask)
				? DCM_E_POSITIVERESPONSE : DCM_E_SECURITYACCESSDENIED;
			}
			break;
		}
		i++;
	}
	return diag_response_code;
}
#endif

#if (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
static FUNC(dcm_nrc_type, DCM_CODE) dsp_write_memory_data(dcm_op_status_type *op_status, uint32 memory_address,
	uint32 memory_size, const uint8 *source_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	dcm_return_write_memory_type write_ret;

	*op_status = DCM_INITIAL;
	write_ret = dcm_write_memory(*op_status, 0, memory_address, memory_size, source_data);

	switch (write_ret) {
		case DCM_WRITE_FAILED:
			response_code = DCM_E_GENERALPROGRAMMINGFAILURE;
			break;
		case DCM_WRITE_PENDING:
			*op_status = DCM_PENDING;
			break;
		default:
			response_code = DCM_E_POSITIVERESPONSE;
			break;
	}

	return response_code;
}

FUNC(void, DCM_CODE) dsp_uds_write_memory_by_addr(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type diag_response_code;
	uint8 size_format = 0u;
	uint8 address_format = 0u;
	uint32 memory_address = 0u;
	uint32 memory_size = 0u;
	dcm_op_status_type op_status = DCM_INITIAL;

	if (pdu_rx_data->SduLength < 5u) {
		diag_response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsd_dsp_processing_done(diag_response_code);
		return;
	}

	size_format = (pdu_rx_data->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_HIGH_MASK) >> 4u;
	address_format = pdu_rx_data->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_LOW_MASK;
	if ((address_format == 0u) || (size_format == 0u)) {
		diag_response_code = DCM_E_REQUESTOUTOFRANGE;
		dsd_dsp_processing_done(diag_response_code);
		return;
	}

	uint8 addr_idx = 0u;

	while (addr_idx < address_format) {
		memory_address <<= 8;
		memory_address += (uint32)(pdu_rx_data->SduDataPtr[(uint8)ADDR_START_INDEX + addr_idx]);
		addr_idx++;
	}

	uint8 size_idx = 0u;

	while (size_idx < size_format) {
		memory_size <<= 8;
		memory_size +=
			(uint32)(pdu_rx_data->SduDataPtr[(uint8)ADDR_START_INDEX + address_format + size_idx]);
		size_idx++;
	}
	if ((memory_size + SID_LEN + ALFID_LEN + (PduLengthType)address_format +
		(PduLengthType)size_format) != pdu_rx_data->SduLength) {
			diag_response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			dsd_dsp_processing_done(diag_response_code);
			return;
		}

	diag_response_code = dsp_check_address_range(DCM_WRITE_MEMORY, memory_address, memory_size);
	if (DCM_E_POSITIVERESPONSE == diag_response_code) {
		diag_response_code = ((memory_size <= 0u) || ((SID_LEN + ALFID_LEN + (uint32)address_format +
			(uint32)size_format) > pdu_tx_data->SduLength)) ? DCM_E_REQUESTOUTOFRANGE
			: dsp_write_memory_data(&op_status, memory_address, memory_size,
				&pdu_rx_data->SduDataPtr[SID_LEN + (uint8)ALFID_LEN + address_format + size_format]);
	}

	if (DCM_E_POSITIVERESPONSE != diag_response_code) {
		dsd_dsp_processing_done(diag_response_code);
	} else {
		pdu_tx_data->SduLength = (SID_LEN + ALFID_LEN + (uint32)address_format + (uint32)size_format);
		pdu_tx_data->SduDataPtr[ALFID_INDEX] = pdu_rx_data->SduDataPtr[ALFID_INDEX];
		uint32 i = 0;

		while (i < (address_format + size_format)) {
			pdu_tx_data->SduDataPtr[(uint8)ADDR_START_INDEX + i] =
				pdu_rx_data->SduDataPtr[(uint8)ADDR_START_INDEX + i];
			if (DCM_WRITE_PENDING == op_status) {
				dsp_mem_state = DCM_MEMORY_WRITE;
			} else {
				dsd_dsp_processing_done(diag_response_code);
			}
			i++;
		}
	}
}
#endif

#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON)
static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_read_memory_data(dcm_op_status_type *op_status, uint32 memory_address, uint32 memory_size,
		const PduInfoType *pdu_tx_data)
{
	dcm_return_read_memory_type read_ret;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	*op_status = DCM_INITIAL;
	read_ret = dcm_read_memory(*op_status, 0, memory_address, memory_size, &pdu_tx_data->SduDataPtr[1]);
	switch (read_ret) {
		case DCM_READ_FAILED:
			response_code = DCM_E_GENERALPROGRAMMINGFAILURE;
			break;
		case DCM_READ_PENDING:
			*op_status = DCM_READ_PENDING;
			break;
		default:
			break;
	}
	return response_code;
}

FUNC(void, DCM_CODE) dsp_uds_read_memory_by_addr(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type diag_response_code;
	uint8 size_format;
	uint8 address_format;
	uint32 memory_address = 0;
	uint32 memory_size = 0;
	dcm_op_status_type op_status = DCM_INITIAL;

	if (pdu_rx_data->SduLength < 4u) {
		diag_response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsd_dsp_processing_done(diag_response_code);
		return;
	}

	size_format = (pdu_rx_data->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_HIGH_MASK) >> 4u;
	address_format = pdu_rx_data->SduDataPtr[ALFID_INDEX] & DCM_FORMAT_LOW_MASK;
	if ((address_format == 0u) || (size_format == 0u)) {
		diag_response_code = DCM_E_REQUESTOUTOFRANGE;
		dsd_dsp_processing_done(diag_response_code);
		return;
	}
	if ((SID_LEN + ALFID_LEN + (PduLengthType)address_format + (PduLengthType)size_format) !=
		pdu_rx_data->SduLength) {
			diag_response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			dsd_dsp_processing_done(diag_response_code);
			return;
		}
	uint8 idx = 0;

	while (idx < address_format) {
		memory_address <<= 8;
		memory_address += (uint32)(pdu_rx_data->SduDataPtr[(uint8)ADDR_START_INDEX + idx]);
		idx++;
	}
	idx = 0;

	while (idx < size_format) {
		memory_size <<= 8;
		memory_size += (uint32)(pdu_rx_data->SduDataPtr[(uint8)ADDR_START_INDEX + address_format + idx]);
		idx++;
	}
	if ((memory_size <= 0u) || (memory_size > (uint32)(pdu_tx_data->SduLength - SID_LEN))) {
		diag_response_code = DCM_E_REQUESTOUTOFRANGE;
		dsd_dsp_processing_done(diag_response_code);
		return;
	}

	diag_response_code =
		dsp_check_address_range(DCM_READ_MEMORY, memory_address, memory_size);
	if (DCM_E_POSITIVERESPONSE == diag_response_code) {
		diag_response_code = dsp_read_memory_data(&op_status, memory_address,
				memory_size, pdu_tx_data);
	}

	if (DCM_E_POSITIVERESPONSE != diag_response_code) {
		dsd_dsp_processing_done(diag_response_code);
		return;
	}

	pdu_tx_data->SduLength = (uint16)(SID_LEN + memory_size);
	if (DCM_READ_PENDING != op_status) {
		dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
	} else {
		dsp_mem_state = DCM_MEMORY_READ;
	}
}
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON)
#ifdef DCM_USE_CONTROL_DIDS
static FUNC(void, DCM_CODE) dsp_stop_io_ctrl(boolean check_session_and_sec_level)
{
	dcm_nrc_type response_code;
	const dcm_dsp_signal_type *signal_ptr;
	const dcm_dsp_did_type *did_ptr = NULL_PTR;
	const dcm_dsp_did_control_type *dsp_did_ctrl = NULL_PTR;

	boolean service_supported = dsd_dsp_check_service_supported_in_active_session_and_security
		(DCM_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER);

	for (uint16 i = 0u; i < (uint16)DCM_NOF_IOCONTROL_DIDS; i++) {
		if (FALSE == dsp_io_ctrl_state_list[i].ctrl_active) {
			continue;
		}

		if (FALSE == lookup_non_dynamic_did(dsp_io_ctrl_state_list[i].did, &did_ptr)) {
			dsp_io_ctrl_state_list[i].ctrl_active = FALSE;
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
			continue;
		}

		dsp_did_ctrl = did_ptr->dsp_did_info_ref->dsp_did_access.dsp_did_control;

		if (NULL_PTR == dsp_did_ctrl) {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
			dsp_io_ctrl_state_list[i].ctrl_active = FALSE;
			continue;
		}

		boolean return_to_ecu = TRUE;

		if (service_supported && check_session_and_sec_level) {
			if (do_check_did_control_condition(dsp_did_ctrl, &response_code)) {
				return_to_ecu = FALSE;
			}
		}

		if (FALSE == return_to_ecu) {
			continue;
		}

		for (uint16 sig_index = 0; sig_index < did_ptr->dsp_nof_signals; sig_index++) {

			if (0u == (dsp_io_ctrl_state_list[i].act_sig_bitfld[sig_index / 8u] &
					DCM_DSP_BIT_SIG(sig_index))) {
				continue;
			}

			signal_ptr = &did_ptr->dsp_signal_ref[sig_index];
#if (DCM_DSP_DATA_RETURN_CONTROL_TO_ECU_FNC_ENABLED == STD_ON)
			if (signal_ptr->dsp_signal_data_ref->dsp_data_return_control_to_ecu_fnc
				.func_return_control_to_ecu_fnc != NULL_PTR) {
				if (signal_ptr->dsp_signal_data_ref->dsp_data_use_port == DATA_PORT_ECU_SIGNAL) {
					(void)signal_ptr->dsp_signal_data_ref->dsp_data_return_control_to_ecu_fnc
						.ecu_signal_return_control_to_ecu_fnc(DCM_RETURN_CONTROL_TO_ECU,
								NULL_PTR);
				} else {
					(void)signal_ptr->dsp_signal_data_ref->dsp_data_return_control_to_ecu_fnc
						.func_return_control_to_ecu_fnc(&response_code);
				}
			}
#endif
			dsp_io_ctrl_state_list[i].act_sig_bitfld[sig_index / 8u] &= ~(DCM_DSP_BIT_SIG(sig_index));
		}
		dsp_io_ctrl_state_list[i].ctrl_active = FALSE;
	}
}

static boolean dsp_find_valid_index(uint16 did_nr, boolean active, uint16 *indx)
{
	uint16 did_index = DCM_NOF_IOCONTROL_DIDS;
	uint16 unused_index = DCM_NOF_IOCONTROL_DIDS;
	uint16 i = 0u;

	while (i < (uint16)DCM_NOF_IOCONTROL_DIDS) {
		if (did_nr == dsp_io_ctrl_state_list[i].did) {
			did_index = i;
			break;
		} else if (unused_index == DCM_NOF_IOCONTROL_DIDS && !dsp_io_ctrl_state_list[i].ctrl_active) {
			unused_index = i;
		}
		i++;
	}

	if (did_index < DCM_NOF_IOCONTROL_DIDS) {
		*indx = did_index;
		return TRUE;
	} else if (active && (unused_index < DCM_NOF_IOCONTROL_DIDS)) {
		*indx = unused_index;
		return TRUE;
	}
	return FALSE;
}

static void dsp_update_io_ctrl_state(uint16 indx, uint16 did_nr,
	const dcm_dsp_io_ctrl_vector signal_affected, boolean active)
{
	dcm_dsp_io_ctrl_state_type *io_ctrl_state = &dsp_io_ctrl_state_list[indx];

	io_ctrl_state->ctrl_active = FALSE;
	io_ctrl_state->did = did_nr;
	uint8 byte_index = 0;

	while (byte_index < sizeof(dcm_dsp_io_ctrl_vector)) {
		if (FALSE == active) {
			io_ctrl_state->act_sig_bitfld[byte_index] &= ~(signal_affected[byte_index]);
		} else {
			io_ctrl_state->act_sig_bitfld[byte_index] |= signal_affected[byte_index];
		}
		if (io_ctrl_state->act_sig_bitfld[byte_index] != 0U) {
			io_ctrl_state->ctrl_active = TRUE;
		}
		byte_index++;
	}
}

static FUNC(void, DCM_CODE)
dsp_io_ctrl_set_active(uint16 did_nr, const dcm_dsp_io_ctrl_vector signal_affected, boolean active)
{
	uint16 indx = DCM_NOF_IOCONTROL_DIDS;
	boolean index_ok = dsp_find_valid_index(did_nr, active, &indx);

	if (index_ok) {
		dsp_update_io_ctrl_state(indx, did_nr, signal_affected, active);
	} else if (active) {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_EXECUTION);
	}
}

static FUNC(void, DCM_CODE)
	dsp_io_ctrl_stop_activated(uint16 did_nr, const dcm_dsp_io_ctrl_vector signal_activated)
{
	const dcm_dsp_did_type *did_ptr = NULL_PTR;
	const dcm_dsp_signal_type *signal_ptr;
	dcm_nrc_type response_code;
	boolean done = FALSE;
	dcm_dsp_io_ctrl_vector currently_active;

	(void)memset(currently_active, 0, sizeof(dcm_dsp_io_ctrl_vector));
	if (lookup_non_dynamic_did(did_nr, &did_ptr)) {
		for (uint16 i = 0; (i < (uint16)DCM_NOF_IOCONTROL_DIDS) && !done; i++) {
			if (did_nr == dsp_io_ctrl_state_list[i].did) {
				(void)memcpy(currently_active, dsp_io_ctrl_state_list[i].act_sig_bitfld,
						sizeof(dcm_dsp_io_ctrl_vector));
				done = TRUE;
			}
		}
	}

	if (NULL_PTR == did_ptr) {
		return;
	}

	for (uint16 sig_index = 0; sig_index < did_ptr->dsp_nof_signals; sig_index++) {
		if ((0u == (currently_active[sig_index / 8u] & DCM_DSP_BIT_SIG(sig_index))) ||
			(0u == (signal_activated[sig_index / 8u] & DCM_DSP_BIT_SIG(sig_index)))) {
				continue;
		}
		signal_ptr = &did_ptr->dsp_signal_ref[sig_index];
#if (DCM_DSP_DATA_RETURN_CONTROL_TO_ECU_FNC_ENABLED == STD_ON)
		if ((signal_ptr->dsp_signal_data_ref->dsp_data_use_port == DATA_PORT_ECU_SIGNAL) &&
			(signal_ptr->dsp_signal_data_ref->dsp_data_return_control_to_ecu_fnc
				.ecu_signal_return_control_to_ecu_fnc != NULL_PTR)) {
			(void)signal_ptr->dsp_signal_data_ref->dsp_data_return_control_to_ecu_fnc
				.ecu_signal_return_control_to_ecu_fnc(DCM_RETURN_CONTROL_TO_ECU, NULL_PTR);
		} else if (signal_ptr->dsp_signal_data_ref->dsp_data_return_control_to_ecu_fnc
			.func_return_control_to_ecu_fnc != NULL_PTR) {
				(void)signal_ptr->dsp_signal_data_ref->dsp_data_return_control_to_ecu_fnc
				.func_return_control_to_ecu_fnc(&response_code);
		} else {
			/* do nothing */
		}
#endif
	}

}

static FUNC(Std_ReturnType, DCM_CODE)
	dsp_function_input_output_ctrl_reset_to_default(const dcm_dsp_data_type *data_ptr, dcm_op_status_type op_status,
		dcm_nrc_type *response_code)
{
	Std_ReturnType ret_val = E_NOT_OK;
#if (DCM_DSP_DATA_RESET_TO_DEFAULT_FNC_ENABLED == STD_ON)
	if (data_ptr->dsp_data_reset_to_default_fnc.func_synch_reset_to_default_fnc != NULL_PTR) {
		if (DATA_PORT_SYNCH == data_ptr->dsp_data_use_port) {
			ret_val =
				data_ptr->dsp_data_reset_to_default_fnc.func_synch_reset_to_default_fnc(response_code);
		} else if (DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port) {
			ret_val = data_ptr->dsp_data_reset_to_default_fnc.func_asynch_reset_to_default_fnc(op_status,
				response_code);
		} else {
			/* do nothing */
		}
	}
#else
	(void)op_status;
	(void)response_code;
	(void)data_ptr;
#endif
	return ret_val;
}

static FUNC(Std_ReturnType, DCM_CODE)
	dsp_function_input_output_ctrl_freeze_current_state(const dcm_dsp_data_type *data_ptr,
		dcm_op_status_type op_status, dcm_nrc_type *response_code)
{
	Std_ReturnType ret_val = E_NOT_OK;
#if (DCM_DSP_DATA_FREEZE_CURRENT_STATE_FNC_ENABLED == STD_ON)
	if (data_ptr->dsp_data_freeze_current_state_fnc.func_synch_freeze_current_state_fnc != NULL_PTR) {
		if (DATA_PORT_SYNCH == data_ptr->dsp_data_use_port) {
			ret_val = data_ptr->dsp_data_freeze_current_state_fnc.func_synch_freeze_current_state_fnc
			(response_code);
		} else if (DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port) {
			ret_val = data_ptr->dsp_data_freeze_current_state_fnc.func_asynch_freeze_current_state_fnc
			(op_status, response_code);
		} else {
			/* do nothing */
		}
	}
#else
	(void)data_ptr;
	(void)op_status;
	(void)response_code;
#endif
	return ret_val;
}

static FUNC(Std_ReturnType, DCM_CODE)
	dsp_function_input_output_ctrl_short_term_adjustmemt(const dcm_dsp_data_type *data_ptr,
		dcm_op_status_type op_status, uint8 *control_option_record, dcm_nrc_type *response_code)
{
	Std_ReturnType ret_val = E_NOT_OK;

#if (DCM_DSP_DATA_SHORT_TERM_ADJUSTMENT_FNC_ENABLED == STD_ON)
	if (data_ptr->dsp_data_short_term_adjustment_fnc.func_synch_short_term_adjustment_fnc != NULL_PTR) {
		if (DATA_PORT_SYNCH == data_ptr->dsp_data_use_port) {
			ret_val = data_ptr->dsp_data_short_term_adjustment_fnc.func_synch_short_term_adjustment_fnc
			(control_option_record, response_code);
		} else if (DATA_PORT_ASYNCH == data_ptr->dsp_data_use_port) {
			ret_val = data_ptr->dsp_data_short_term_adjustment_fnc.func_asynch_short_term_adjustment_fnc
			(control_option_record, op_status, response_code);
		} else {
			/* do nothing */
		}
	}
#else
	(void)data_ptr;
	(void)op_status;
	(void)control_option_record;
	(void)response_code;
#endif
	return ret_val;
}

static FUNC(Std_ReturnType, DCM_CODE) dsp_function_input_output_ctrl(const dcm_dsp_data_type *data_ptr,
	dcm_io_ctrl_para_type action, dcm_op_status_type op_status, uint8 *control_option_record,
		dcm_nrc_type *response_code)
{
	*response_code = DCM_E_REQUESTOUTOFRANGE;
	Std_ReturnType ret_val = E_NOT_OK;

	if (DCM_RETURN_CONTROL_TO_ECU == action) {
#if (DCM_DSP_DATA_RETURN_CONTROL_TO_ECU_FNC_ENABLED == STD_ON)
		if (data_ptr->dsp_data_return_control_to_ecu_fnc.func_return_control_to_ecu_fnc != NULL_PTR) {
			ret_val = data_ptr->dsp_data_return_control_to_ecu_fnc.func_return_control_to_ecu_fnc
			(response_code);
		}
#endif
	} else if (DCM_RESET_TO_DEFAULT == action) {
		ret_val = dsp_function_input_output_ctrl_reset_to_default(data_ptr, op_status, response_code);
	} else if (DCM_SHORT_TERM_ADJUSTMENT == action) {
		ret_val = dsp_function_input_output_ctrl_short_term_adjustmemt(data_ptr, op_status,
			control_option_record, response_code);
	} else if (DCM_FREEZE_CURRENT_STATE == action) {
		ret_val = dsp_function_input_output_ctrl_freeze_current_state(data_ptr, op_status, response_code);
	} else {
		/* do nothing */
	}

	return ret_val;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_io_ctrl_execute(const dcm_dsp_did_type *did_ptr,
	dcm_io_ctrl_para_type action, uint8 *data, dsp_uds_io_ctrl_pending_type *pending_ptr,
	dcm_op_status_type init_op_status, const uint8 *ctrl_enable_mask_ptr)
{
	Std_ReturnType ret_val = E_OK;
	dcm_op_status_type op_status = init_op_status;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	const dcm_dsp_signal_type *signal_ptr;
	uint16 sig_index = pending_ptr->pending_signal_index;

	while ((sig_index < did_ptr->dsp_nof_signals) && (E_OK == ret_val) &&
	(DCM_E_POSITIVERESPONSE == response_code)) {
		signal_ptr = &did_ptr->dsp_signal_ref[sig_index];

		if ((NULL_PTR != ctrl_enable_mask_ptr) &&
			(!(boolean)(ctrl_enable_mask_ptr[sig_index / 8u] & DCM_DSP_BIT_SIG(sig_index)))) {
				sig_index++;
				continue;
		}

		ret_val = dsp_function_input_output_ctrl(signal_ptr->dsp_signal_data_ref, action, op_status,
			&data[(signal_ptr->dsp_signal_bit_position / 8u)], &response_code);
		pending_ptr->state = DCM_GENERAL_IDLE;
		pending_ptr->pending_signal_index = 0;
		if (E_OK == ret_val) {
			pending_ptr->signal_affected[sig_index / 8u] |= DCM_DSP_BIT_SIG(sig_index);
			pending_ptr->ctrl_activated = TRUE;
			op_status = DCM_INITIAL;
			response_code = DCM_E_POSITIVERESPONSE;
		} else if (E_NOT_OK == ret_val) {
			if (DCM_E_POSITIVERESPONSE == response_code) {
				response_code = DCM_E_CONDITIONSNOTCORRECT;
			}
		} else if (DCM_E_PENDING == ret_val) {
			pending_ptr->pending_ctrl = TRUE;
			pending_ptr->pending_signal_index = sig_index;
			pending_ptr->state = DCM_GENERAL_PENDING;
			response_code = DCM_E_RESPONSEPENDING;
		} else {
			response_code = DCM_E_CONDITIONSNOTCORRECT;
		}
		sig_index++;
	}
	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_io_ctrl_read_data(const dcm_dsp_did_type *did_ptr, uint8 *data, dsp_uds_io_ctrl_pending_type *pending_ptr,
		dcm_op_status_type init_op_status)
{
	const dcm_dsp_signal_type *signal_ptr;
	dcm_op_status_type op_status = init_op_status;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType ret_val = E_OK;

#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_OFF)
	(void)data;
#endif

	for (uint16 sig_index = pending_ptr->pending_signal_index;
		(sig_index < did_ptr->dsp_nof_signals) && (E_OK == ret_val); sig_index++) {
		signal_ptr = &did_ptr->dsp_signal_ref[sig_index];
#if (DCM_DSP_DATA_READ_DATA_FNC_ENABLED == STD_ON)
		if (signal_ptr->dsp_signal_data_ref->dsp_data_read_data_fnc.synch_data_read_fnc == NULL_PTR) {
			continue;
		}
		switch (signal_ptr->dsp_signal_data_ref->dsp_data_use_port) {
			case DATA_PORT_SYNCH:
			case DATA_PORT_ECU_SIGNAL:
				if (E_OK != signal_ptr->dsp_signal_data_ref->dsp_data_read_data_fnc.synch_data_read_fnc
					(&data[(signal_ptr->dsp_signal_bit_position / 8u)])) {
					DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
					ret_val = E_NOT_OK;
				}
				break;
			case DATA_PORT_ASYNCH:
				ret_val = signal_ptr->dsp_signal_data_ref->dsp_data_read_data_fnc.asynch_data_read_fnc
				(op_status, &data[(signal_ptr->dsp_signal_bit_position / 8u)]);
				break;
			default:
				DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_CONFIG_INVALID);
				ret_val = E_NOT_OK;
				break;
		}

		pending_ptr->state = DCM_GENERAL_IDLE;
		op_status = DCM_INITIAL;
		if (DCM_E_PENDING == ret_val) {
			pending_ptr->pending_ctrl = FALSE;
			pending_ptr->pending_signal_index = sig_index;
			pending_ptr->state = DCM_GENERAL_PENDING;
			response_code = DCM_E_RESPONSEPENDING;
		} else {
			response_code = (E_NOT_OK == ret_val) ? DCM_E_CONDITIONSNOTCORRECT : DCM_E_REQUESTOUTOFRANGE;
		}
#endif
	}
	return response_code;
}

static FUNC(void, DCM_CODE) dsp_init_io_ctrl_data(dcm_op_status_type *op_status)
{
	if (DCM_GENERAL_IDLE == dsp_io_ctrl_data.state) {
		dsp_io_ctrl_data.pending_signal_index = 0u;
		dsp_io_ctrl_data.pending_ctrl = TRUE;
		dsp_io_ctrl_data.ctrl_activated = FALSE;
		(void)memset(dsp_io_ctrl_data.signal_affected, 0, sizeof(dsp_io_ctrl_data.signal_affected));
		*op_status = DCM_INITIAL;
	}
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_check_io_ctrl_execute(const PduInfoType *pdu_rx_data,
	PduLengthType tx_sdulength, const dcm_dsp_did_type *did_ptr, dcm_op_status_type *op_status)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	uint16 did_size = did_ptr->dsp_did_data_byte_size;
	uint16 control_option_size = (DCM_SHORT_TERM_ADJUSTMENT == pdu_rx_data->SduDataPtr[IOCP_INDEX]) ?
		did_ptr->dsp_did_data_byte_size : 0u;
	boolean request_ok = ((uint16)pdu_rx_data->SduLength ==
		((uint16)SID_LEN + (uint16)IOI_LEN + (uint16)IOCP_LEN + control_option_size));

	const uint8 *mask_ptr = NULL_PTR;

	if (did_ptr->dsp_nof_signals > 1u) {
		mask_ptr = &pdu_rx_data->SduDataPtr[(uint16)COR_INDEX + control_option_size];
	}

	if (!request_ok) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		return response_code;
	}

	if (tx_sdulength < (SID_LEN + IOI_LEN + IOCP_LEN + (PduLengthType)did_size)) {
		response_code = DCM_E_RESPONSETOOLONG;
		return response_code;
	}

	response_code = dsp_io_ctrl_execute(did_ptr, pdu_rx_data->SduDataPtr[IOCP_INDEX],
		&pdu_rx_data->SduDataPtr[COR_INDEX], &dsp_io_ctrl_data, *op_status, mask_ptr);
	if (DCM_E_POSITIVERESPONSE == response_code) {
		*op_status = DCM_INITIAL;
	}

	return response_code;
}

static FUNC(void, DCM_CODE) dsp_do_check_io_ctrl_read_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data,
	const dcm_dsp_did_type *did_ptr, dcm_nrc_type *response_code, dcm_op_status_type op_status)
{
	uint16 did_size = 0u;

	if (NULL_PTR != did_ptr) {
		did_size = did_ptr->dsp_did_data_byte_size;
	}

	if (DCM_E_POSITIVERESPONSE != *response_code) {
		return;
	}

	pdu_tx_data->SduLength = (PduLengthType)SID_LEN + (PduLengthType)IOI_LEN + (PduLengthType)IOCP_LEN +
		(PduLengthType)did_size;
	pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
	pdu_tx_data->SduDataPtr[2] = pdu_rx_data->SduDataPtr[2];
	pdu_tx_data->SduDataPtr[3] = pdu_rx_data->SduDataPtr[IOCP_INDEX];

	if (NULL_PTR == did_ptr) {
		return;
	}
	*response_code = dsp_io_ctrl_read_data(did_ptr, &pdu_tx_data->SduDataPtr[4], &dsp_io_ctrl_data, op_status);
}

static FUNC(void, DCM_CODE) dsp_do_check_io_ctrl_active(uint16 did_nr, const PduInfoType *pdu_rx_data,
	dcm_nrc_type response_code)
{
	if (DCM_RETURN_CONTROL_TO_ECU == pdu_rx_data->SduDataPtr[IOCP_INDEX]) {
		dsp_io_ctrl_set_active(did_nr, dsp_io_ctrl_data.signal_affected, FALSE);
	} else if ((DCM_RESET_TO_DEFAULT == pdu_rx_data->SduDataPtr[IOCP_INDEX]) ||
		(DCM_FREEZE_CURRENT_STATE == pdu_rx_data->SduDataPtr[IOCP_INDEX]) ||
		(DCM_SHORT_TERM_ADJUSTMENT == pdu_rx_data->SduDataPtr[IOCP_INDEX])) {
		dsp_io_ctrl_set_active(did_nr, dsp_io_ctrl_data.signal_affected, TRUE);
	} else {
		/* do nothing */
	}

	if (dsp_io_ctrl_data.ctrl_activated && (DCM_RETURN_CONTROL_TO_ECU != pdu_rx_data->SduDataPtr[IOCP_INDEX]) &&
		(DCM_E_RESPONSEPENDING != response_code) && (DCM_E_POSITIVERESPONSE != response_code)) {
		dsp_io_ctrl_stop_activated(did_nr, dsp_io_ctrl_data.signal_affected);
	}
}
#endif /* DCM_USE_CONTROL_DIDS */

FUNC(void, DCM_CODE) dsp_io_ctrl_by_data_id(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
#ifndef DCM_USE_CONTROL_DIDS
	(void)pdu_tx_data;

	if (pdu_rx_data->SduLength >= (SID_LEN + IOI_LEN + IOCP_LEN)) {
		dsd_dsp_processing_done(DCM_E_REQUESTOUTOFRANGE);
	} else {
		dsd_dsp_processing_done(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}

	return;
#else
	uint16 did_nr = 0u;
	boolean did_found;
	const dcm_dsp_did_type *did_ptr = NULL_PTR;
	dcm_op_status_type op_status = DCM_PENDING;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	const dcm_dsp_did_control_type *dsp_did_ctrl = NULL_PTR;

	dsp_init_io_ctrl_data(&op_status);
	dsp_io_ctrl_data.pdu_tx_data = pdu_tx_data;
	dsp_io_ctrl_data.pdu_rx_data = pdu_rx_data;
	did_nr = (((uint16)pdu_rx_data->SduDataPtr[IOI_INDEX] << 8u) & DCM_DID_HIGH_MASK) +
		(pdu_rx_data->SduDataPtr[IOI_INDEX + 1u] & DCM_DID_LOW_MASK);
	did_found = lookup_non_dynamic_did(did_nr, &did_ptr);

	if (!dsp_io_ctrl_data.pending_ctrl) {
		dsp_do_check_io_ctrl_read_data(pdu_rx_data, pdu_tx_data, did_ptr, &response_code, op_status);
		dsp_do_check_io_ctrl_active(did_nr, pdu_rx_data, response_code);

		if (DCM_E_RESPONSEPENDING != response_code) {
			dsd_dsp_processing_done(response_code);
		}
		return;
	}

	if (pdu_rx_data->SduLength < (SID_LEN + IOI_LEN + IOCP_LEN)) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsp_do_check_io_ctrl_read_data(pdu_rx_data, pdu_tx_data, did_ptr, &response_code, op_status);
		dsp_do_check_io_ctrl_active(did_nr, pdu_rx_data, response_code);
		dsd_dsp_processing_done(response_code);
		return;
	}

	if (FALSE == did_found) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
		dsp_do_check_io_ctrl_read_data(pdu_rx_data, pdu_tx_data, did_ptr, &response_code, op_status);
		dsp_do_check_io_ctrl_active(did_nr, pdu_rx_data, response_code);
		dsd_dsp_processing_done(response_code);
		return;
	}
	dsp_did_ctrl = did_ptr->dsp_did_info_ref->dsp_did_access.dsp_did_control;
	if ((NULL_PTR != dsp_did_ctrl) && (did_ptr->dsp_nof_signals != 0u) &&
		((pdu_rx_data->SduDataPtr[IOCP_INDEX]) <= DCM_SHORT_TERM_ADJUSTMENT)) {
		if (do_check_did_control_condition(dsp_did_ctrl, &response_code)) {
			response_code = dsp_do_check_io_ctrl_execute(pdu_rx_data,
				pdu_tx_data->SduLength, did_ptr, &op_status);
		}
		dsp_do_check_io_ctrl_read_data(pdu_rx_data, pdu_tx_data, did_ptr, &response_code, op_status);
		dsp_do_check_io_ctrl_active(did_nr, pdu_rx_data, response_code);

		if (DCM_E_RESPONSEPENDING != response_code) {
			dsd_dsp_processing_done(response_code);
		}
		return;
	}
	response_code = DCM_E_REQUESTOUTOFRANGE;
	dsp_do_check_io_ctrl_read_data(pdu_rx_data, pdu_tx_data, did_ptr, &response_code, op_status);
	dsp_do_check_io_ctrl_active(did_nr, pdu_rx_data, response_code);
	dsd_dsp_processing_done(response_code);
#endif
}
#endif

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
static FUNC(Std_ReturnType, DCM_CODE) dsp_get_comm_ctrl_modes(uint8 sub_func, uint8 com_type,
	uint8 *rte_mode, dcm_comm_mode_type *dcm_mode)
{
	Std_ReturnType ret = E_NOT_OK;
	const uint8 dcm_mode_map[DCM_DSP_UDS_0x28_NUM_COMM_TP][DCM_DSP_UDS_0x28_SUB_FUNC_NUM] = {
		{ DCM_ENABLE_RX_TX_NORM, DCM_ENABLE_RX_DISABLE_TX_NORM, DCM_DISABLE_RX_ENABLE_TX_NORM,
		  DCM_DISABLE_RX_TX_NORMAL },
		{ DCM_ENABLE_RX_TX_NM, DCM_ENABLE_RX_DISABLE_TX_NM, DCM_DISABLE_RX_ENABLE_TX_NM, DCM_DISABLE_RX_TX_NM },
		{ DCM_ENABLE_RX_TX_NORM_NM, DCM_ENABLE_RX_DISABLE_TX_NORM_NM, DCM_DISABLE_RX_ENABLE_TX_NORM_NM,
		  DCM_DISABLE_RX_TX_NORM_NM }
	};
	const uint8 rte_mode_map[DCM_DSP_UDS_0x28_NUM_COMM_TP][DCM_DSP_UDS_0x28_SUB_FUNC_NUM] = {
		{ RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NORM,
		  RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NORM,
		  RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NORM,
		  RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NORMAL },
		{ RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NM,
		  RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NM,
		  RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NM,
		  RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NM },
		{ RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NORM_NM,
		  RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NORM_NM,
		  RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NORM_NM,
		  RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NORM_NM }
	};

	if ((!DCM_DSP_IS_SUP_0x28_SUBFUNC(sub_func)) || (!DCM_DSP_IS_EN_0x28_COMM_TP(com_type))) {
		return ret;
	}

	*rte_mode = rte_mode_map[com_type - 1u][sub_func];
	*dcm_mode = dcm_mode_map[com_type - 1u][sub_func];
	ret = E_OK;

	return ret;
}

static FUNC(void, DCM_CODE) dsp_com_ctrl_execute_mode(uint8 rte_mode, dcm_comm_mode_type dcm_mode,
	uint8 comm_channel_internal_index, boolean execute_mode_switch)
{
	if (execute_mode_switch) {
		if (dcm_config_ptr->dcm_comm_channel_cfg[comm_channel_internal_index].mode_switch_fnc != NULL) {
			(void)dcm_config_ptr->dcm_comm_channel_cfg[comm_channel_internal_index].mode_switch_fnc(rte_mode);
		}
		dsp_channel_com_mode[comm_channel_internal_index] = dcm_mode;
#if (CONFIG_SERVM_BSWM == 1)
		bswm_dcm_communication_mode_current_state
		(dcm_config_ptr->dcm_comm_channel_cfg[comm_channel_internal_index].network_handle, dcm_mode);
#endif
	}
}

FUNC(void, DCM_CODE)
dsp_communication_control(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, PduIdType rx_pdu_id,
	PduIdType tx_confirm_id)
{
	dcm_nrc_type response_code = DCM_E_REQUESTOUTOFRANGE;
	uint8 sub_func = pdu_rx_data->SduDataPtr[SF_INDEX];
	uint8 sub_net;
	uint8 com_type;

	if (pdu_rx_data->SduLength != 3u) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsd_dsp_processing_done(response_code);
		return;
	}
	if (!DCM_DSP_IS_SUP_0x28_SUBFUNC(sub_func)) {
		response_code = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		dsd_dsp_processing_done(response_code);
		return;
	}

	sub_net = pdu_rx_data->SduDataPtr[DCM_DSP_CTP_IDX] >> 4u;
	com_type = pdu_rx_data->SduDataPtr[DCM_DSP_CTP_IDX] & 3u;
	response_code =
		dsp_internal_comm_ctrl(sub_func, sub_net, com_type, rx_pdu_id, FALSE);

	if (DCM_E_POSITIVERESPONSE != response_code) {
		dsp_comm_ctrl_data.com_ctrl_pending = FALSE;
	} else {
		dsp_comm_ctrl_data.com_ctrl_pending = TRUE;
		dsp_comm_ctrl_data.sub_func = sub_func;
		dsp_comm_ctrl_data.subnet = sub_net;
		dsp_comm_ctrl_data.com_type = com_type;
		dsp_comm_ctrl_data.confirm_pdu = tx_confirm_id;
		dsp_comm_ctrl_data.rx_pdu = rx_pdu_id;
	}

	if (response_code != DCM_E_POSITIVERESPONSE) {
		dsd_dsp_processing_done(response_code);
		return;
	}

	pdu_tx_data->SduLength = SID_LEN + (uint8)SF_LEN;
	pdu_tx_data->SduDataPtr[SF_INDEX] = pdu_rx_data->SduDataPtr[SF_INDEX];

	dsd_dsp_processing_done(response_code);
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_internal_comm_ctrl(uint8 sub_func, uint8 sub_net, uint8 com_type, PduIdType rx_pdu_id,
	boolean execute_mode_switch)
{
	uint8 rte_mode;
	dcm_comm_mode_type dcm_mode;
	dcm_nrc_type response_code = DCM_E_REQUESTOUTOFRANGE;

	if (E_OK != dsp_get_comm_ctrl_modes(sub_func, (com_type), &rte_mode, &dcm_mode)) {
		return response_code;
	}

	if (0xfu == sub_net) {
		if (rx_pdu_id < DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
			const dcm_dsl_protocol_rx_type *protocol_rx =
				&dcm_config_ptr->dsl->dsl_protocol->dsl_protocol_rx_global_list[rx_pdu_id];
			dsp_com_ctrl_execute_mode(rte_mode, dcm_mode,
				protocol_rx->comm_channel_internal_index, execute_mode_switch);
			response_code = DCM_E_POSITIVERESPONSE;
		}
	} else {
		if (NULL_PTR == dcm_config_ptr->dsp->dsp_com_control) {
			return response_code;
		}

		if (0u == sub_net) {
			const uint8 *all_channels =
				dcm_config_ptr->dsp->dsp_com_control->dsp_control_all_channel;
			for (uint8 i = 0; i < DCM_NOF_COMCONTROL_ALL_CHANNELS; i++) {
				dsp_com_ctrl_execute_mode(rte_mode, dcm_mode, all_channels[i],
					execute_mode_switch);
				response_code = DCM_E_POSITIVERESPONSE;
			}
			return response_code;
		}
#if (DCM_NOF_COMCONTROL_SPECIFIC_CHANNELS > 0)
		const dcm_dsp_com_control_specific_channel_type *specific_channels =
			dcm_config_ptr->dsp->dsp_com_control->dsp_control_specific_channel;

		for (uint8 i = 0; i < DCM_NOF_COMCONTROL_SPECIFIC_CHANNELS; i++) {
			if (sub_net == specific_channels[i].subnet_number) {
				dsp_com_ctrl_execute_mode(rte_mode, dcm_mode,
					specific_channels[i].comm_channel_index, execute_mode_switch);
				response_code = DCM_E_POSITIVERESPONSE;
			}
		}
#endif
	}

	return response_code;
}
#endif

#if (DCM_USE_SERVICE_REQUESTCURRENTPOWERTRAINDIAGNOSTICDATA == STD_ON) ||                                              \
	(DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA == STD_ON)
static FUNC(boolean, DCM_CODE) dsp_lookup_pid(uint8 pid_id, dcm_pid_svc_type service, const dcm_dsp_pid_type **pid_ptr)
{
	boolean pid_found = FALSE;

#if (DCM_PID_LIST_EOL_INDEX != 0)
	const dcm_dsp_pid_type *dsp_pid = dcm_config_ptr->dsp->dsp_pid;
	uint16 index = 0;

	while (index < DCM_PID_LIST_EOL_INDEX) {
		if ((dsp_pid[index].dsp_pid_identifier == pid_id) && (dsp_pid[index].dcm_pid_enabled == TRUE) &&
			((DCM_SERVICE_01_02 == dsp_pid[index].dsp_pid_service) ||
				(service == dsp_pid[index].dsp_pid_service))) {
			*pid_ptr = &dsp_pid[index];
			pid_found = TRUE;
		}
		index++;
	}
#endif
	return pid_found;
}
#endif

#if (DCM_USE_SERVICE_REQUESTCURRENTPOWERTRAINDIAGNOSTICDATA == STD_ON)
static FUNC(boolean, DCM_CODE)
	dsp_set_availability_pid_value(uint8 pid, dcm_pid_svc_type pid_service, uint32 *pid_value)
{
	uint32 pid_data = 0u;
	boolean set_ok = TRUE;

#if (DCM_PID_LIST_EOL_INDEX != 0)
	uint8 shift = 0u;
	uint32 temp = 0u;
	const dcm_dsp_pid_type *dsp_pid = dcm_config_ptr->dsp->dsp_pid;

	for (uint16 index = 0; index < DCM_PID_LIST_EOL_INDEX; index++) {
		uint8 dsp_pid_identifier = dsp_pid[index].dsp_pid_identifier;

		if ((DCM_SERVICE_01_02 != dsp_pid[index].dsp_pid_service) &&
			(pid_service != dsp_pid[index].dsp_pid_service)) {
				continue;
		}
		if ((dsp_pid_identifier >= (pid + DCM_DSP_SUP_PID_MIN_OFFSET)) &&
			(dsp_pid_identifier < (pid + DCM_DSP_SUP_PID_MAX_OFFSET))) {
			shift = dsp_pid_identifier - pid;
			temp = (uint32)1 << (uint32)((uint32)DCM_DSP_SUP_PID_MAX_OFFSET - (uint32)shift);
			pid_data |= temp;
		} else if (dsp_pid_identifier > (pid + DCM_DSP_SUP_PID_MAX_OFFSET)) {
			pid_data |= 1u;
		} else {
			/* do nothing */
		}
	}
#endif

	if (0u == pid_data) {
		set_ok = FALSE;
	}
	(*pid_value) = pid_data;

	return set_ok;
}

FUNC(void, DCM_CODE)
dsp_obd_req_cur_powertrain_diag_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	uint32 obd_data = 0u;
	uint16 dsp_avail_pids_num = 0u;
	uint16 dsp_find_pid_num = 0u;
	uint16 tx_pos = SID_LEN;
	uint16 dsp_tx_len = SID_LEN;
	uint16 pid_num = (uint16)(pdu_rx_data->SduLength - SID_LEN);
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	const dcm_dsp_pid_type *dsp_src_pid_ptr = NULL_PTR;

	if ((pdu_rx_data->SduLength < (PduLengthType)DCM_DSP_OBD_REQ_PSG_SIZE) ||
		(pdu_rx_data->SduLength > (PduLengthType)DCM_DSP_OBD_REQ_MSG_MAX_SIZE)) {
			response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			dsd_dsp_processing_done_with_no_response();
			return;
		}

	uint8 local_rx_sdu_data[DCM_DSP_OBD_REQ_MSG_MAX_SIZE];
	(void)memcpy(local_rx_sdu_data, pdu_rx_data->SduDataPtr, pdu_rx_data->SduLength);
	uint16 i = 0u;

	while (i < pid_num) {
		if (DCM_DSP_IS_AVAIL_PID(local_rx_sdu_data[i + 1u])) {
			dsp_avail_pids_num++;
			dsp_tx_len += (uint16)((uint16)DCM_DSP_PID_SIZE + (uint16)DCM_DSP_SUP_PIDS_DATA_SIZE);
		} else if (TRUE == dsp_lookup_pid(local_rx_sdu_data[i + 1u], DCM_SERVICE_01, &dsp_src_pid_ptr)) {
			dsp_tx_len += (uint16)((uint16)DCM_DSP_PID_SIZE + dsp_src_pid_ptr->dsp_pid_size);
		} else {
			/* do nothing */
		}
		i++;
	}

	if (dsp_tx_len > pdu_tx_data->SduLength) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
		dsd_dsp_processing_done_with_no_response();
		return;
	}

	if (pid_num == dsp_avail_pids_num) {
		i = 0;

		while (i < pid_num) {
			if (TRUE == dsp_set_availability_pid_value(local_rx_sdu_data[i + 1u],
					DCM_SERVICE_01, &obd_data)) {
				pdu_tx_data->SduDataPtr[tx_pos++] = local_rx_sdu_data[i + 1u];
				pdu_tx_data->SduDataPtr[tx_pos++] =
					(uint8)(((obd_data) & (DCM_DSP_OBD_LSB_MASK << OFFSET_THREE_BYTES)) >>
						OFFSET_THREE_BYTES);
				pdu_tx_data->SduDataPtr[tx_pos++] =
					(uint8)(((obd_data) & (DCM_DSP_OBD_LSB_MASK << OFFSET_TWO_BYTES)) >>
						OFFSET_TWO_BYTES);
				pdu_tx_data->SduDataPtr[tx_pos++] =
					(uint8)(((obd_data) & (DCM_DSP_OBD_LSB_MASK << OFFSET_ONE_BYTE)) >>
						OFFSET_ONE_BYTE);
				pdu_tx_data->SduDataPtr[tx_pos++] = (uint8)((obd_data)&DCM_DSP_OBD_LSB_MASK);
			} else if ((uint8)DCM_DSP_DEFAULT_PID_VALUE == local_rx_sdu_data[i + 1u]) {
				pdu_tx_data->SduDataPtr[tx_pos++] = local_rx_sdu_data[i + 1u];
				pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
				pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
				pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
				pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
			} else {
				dsp_find_pid_num++;
			}
			i++;
		}
	} else if ((uint16)0 == dsp_avail_pids_num) {
		i = 0;

		while (i < pid_num) {
			if (FALSE == dsp_lookup_pid(local_rx_sdu_data[i + 1u], DCM_SERVICE_01, &dsp_src_pid_ptr)) {
				dsp_find_pid_num++;
				i++;
				continue;
			}

			(void)memset(&pdu_tx_data->SduDataPtr[tx_pos + 1u], 0, dsp_src_pid_ptr->dsp_pid_size);
			if ((dsp_src_pid_ptr->dsp_get_pid_val_fnc != NULL) &&
				(E_OK == dsp_src_pid_ptr->dsp_get_pid_val_fnc(&pdu_tx_data->SduDataPtr[tx_pos + 1u]))) {
				pdu_tx_data->SduDataPtr[tx_pos] = local_rx_sdu_data[i + 1u];
				tx_pos += (uint16)dsp_src_pid_ptr->dsp_pid_size + 1u;
			} else {
				response_code = DCM_E_CONDITIONSNOTCORRECT;
				break;
			}
			i++;
		}
	} else {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}
	if (pid_num == dsp_find_pid_num) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	} else {
		/* do nothing */
	}

	if (DCM_E_POSITIVERESPONSE == response_code) {
		pdu_tx_data->SduLength = tx_pos;
	} else {
		/* do nothing */
	}

	if ((response_code == DCM_E_POSITIVERESPONSE) || (response_code == DCM_E_BUSYREPEATREQUEST)) {
		dsd_dsp_processing_done(response_code);
	} else {
		dsd_dsp_processing_done_with_no_response();
	}

	return;
}
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA == STD_ON)
FUNC(void, DCM_CODE)
dsp_obd_req_powertrain_freeze_frame_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	uint16 dsp_avail_pids_num = 0u;
	uint16 dsp_find_pid_num = 0u;
	uint16 tx_pos = SID_LEN;
	uint16 dsp_tx_len = SID_LEN;
	uint32 dtc = 0;
	uint32 dsp_sup_bit_fld = 0u;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	const dcm_dsp_pid_type *dsp_src_pid_ptr = NULL_PTR;

	if ((pdu_rx_data->SduLength < DCM_DSP_OBD_REQ_MSG_MIN_SIZE_TWO) ||
	    (pdu_rx_data->SduLength > DCM_DSP_OBD_REQ_MSG_MAX_SIZE) || (((pdu_rx_data->SduLength - 1u) % 2u) != 0u)) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsd_dsp_processing_done_with_no_response();
		return;
	}

	uint16 pid_num = ((uint16)(pdu_rx_data->SduLength - 1u) >> 1u);
	uint8 local_rx_sdu_data[DCM_DSP_OBD_REQ_MSG_MAX_SIZE];
	(void)memcpy(local_rx_sdu_data, pdu_rx_data->SduDataPtr, pdu_rx_data->SduLength);
	const uint8 *pid_and_frame_ptr = &local_rx_sdu_data[1];
	uint16 i = 0u;

	while (i < pid_num) {
		if (pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2] == DCM_DSP_OBD_TWO_SVC) {
			dsp_tx_len += (uint16)DCM_DSP_PID_SIZE + (uint16)DCM_DSP_FF_NUM_SIZE + (uint16)DCM_DSP_DTC_SIZE;
		} else if (DCM_DSP_IS_AVAIL_PID(pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2])) {
			dsp_avail_pids_num++;
			dsp_tx_len += (uint16)((uint16)DCM_DSP_PID_SIZE + (uint16)DCM_DSP_SUP_PIDS_DATA_SIZE);
		} else if (TRUE == dsp_lookup_pid(pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2], DCM_SERVICE_02,
			&dsp_src_pid_ptr)) {
			dsp_tx_len +=
				(uint16)((uint16)DCM_DSP_PID_SIZE + (uint16)DCM_DSP_FF_NUM_SIZE + dsp_src_pid_ptr->dsp_pid_size);
		} else {
			/* do nothing */
		}
		pid_and_frame_ptr += DCM_DSP_OBD_SVC_2_PID_AND_FRM_LEN;
		i++;
	}

	if (dsp_tx_len > (pdu_tx_data->SduLength)) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
		dsd_dsp_processing_done_with_no_response();

		return;
	}

	if (pid_num == dsp_avail_pids_num) {
		pid_and_frame_ptr = &local_rx_sdu_data[1];
		i = 0;

		while (i < pid_num) {
			if (pid_and_frame_ptr[DCM_DSP_OBD_FRM_IDX_SVC_2] != (uint8)DCM_DSP_DEFAULT_RECORD_NUM) {
				response_code = DCM_E_REQUESTOUTOFRANGE;
				break;
			} else {
				if (TRUE == dsp_set_availability_pid_value(pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2],
					DCM_SERVICE_02, &dsp_sup_bit_fld)) {
					pdu_tx_data->SduDataPtr[tx_pos++] = pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2];
					pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_RECORD_NUM;
					pdu_tx_data->SduDataPtr[tx_pos++] = (uint8)(((dsp_sup_bit_fld) &
						(DCM_DSP_OBD_LSB_MASK << OFFSET_THREE_BYTES)) >> OFFSET_THREE_BYTES);
					pdu_tx_data->SduDataPtr[tx_pos++] = (uint8)(((dsp_sup_bit_fld) &
						(DCM_DSP_OBD_LSB_MASK << OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
					pdu_tx_data->SduDataPtr[tx_pos++] = (uint8)(((dsp_sup_bit_fld) &
							(DCM_DSP_OBD_LSB_MASK << OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
					pdu_tx_data->SduDataPtr[tx_pos++] = (uint8)((dsp_sup_bit_fld)&DCM_DSP_OBD_LSB_MASK);
				} else if ((uint8)DCM_DSP_DEFAULT_PID_VALUE == pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2]) {
					pdu_tx_data->SduDataPtr[tx_pos++] = pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2];
					pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_RECORD_NUM;
					pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
					pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
					pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
					pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_DATA_VALUE;
				} else {
					dsp_find_pid_num++;
				}
			}
			pid_and_frame_ptr += DCM_DSP_OBD_SVC_2_PID_AND_FRM_LEN;
			i++;
		}
	} else if ((uint16)0 == dsp_avail_pids_num) {
		pid_and_frame_ptr = &local_rx_sdu_data[1];
		i = 0;

		while (i < pid_num) {
			if (pid_and_frame_ptr[DCM_DSP_OBD_FRM_IDX_SVC_2] != (uint8)DCM_DSP_DEFAULT_RECORD_NUM) {
				response_code = DCM_E_REQUESTOUTOFRANGE;
				break;
			} else {
				uint8 buf_size = 0;
				if (pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2] != DCM_DSP_OBD_TWO_SVC) {
					pdu_tx_data->SduDataPtr[tx_pos++] =
						pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2];
					pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_RECORD_NUM;
					buf_size = (uint8)(pdu_tx_data->SduLength - tx_pos);

					if (E_OK == dem_read_data_of_obd_ff
						(pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2], DCM_DSP_DATA_ELMT_PID_NOT_SUP_IDX,
							&(pdu_tx_data->SduDataPtr[tx_pos]), &buf_size)) {
						tx_pos = tx_pos + buf_size;
					} else {
						tx_pos -= 2u;
						dsp_find_pid_num++;
					}
				} else {
					if (E_OK != dem_get_dtc_of_obd_ff(DCM_DSP_DEFAULT_RECORD_NUM, &dtc, DEM_DTC_FORMAT_OBD)) {
						pdu_tx_data->SduDataPtr[tx_pos++] =
							pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2];
						pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_RECORD_NUM;
						pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_RECORD_NUM;
						pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_RECORD_NUM;
					} else {
						pdu_tx_data->SduDataPtr[tx_pos++] =
							pid_and_frame_ptr[DCM_DSP_OBD_PID_IDX_SVC_2];
						pdu_tx_data->SduDataPtr[tx_pos++] = DCM_DSP_DEFAULT_RECORD_NUM;
						pdu_tx_data->SduDataPtr[tx_pos++] =
							(uint8)(((dtc) & (DCM_DSP_OBD_LSB_MASK
								<< OFFSET_TWO_BYTES)) >> OFFSET_TWO_BYTES);
						pdu_tx_data->SduDataPtr[tx_pos++] =
							(uint8)(((dtc) & (DCM_DSP_OBD_LSB_MASK
								<< OFFSET_ONE_BYTE)) >> OFFSET_ONE_BYTE);
					}
				}
			}
			pid_and_frame_ptr += DCM_DSP_OBD_SVC_2_PID_AND_FRM_LEN;
			i++;
		}

	} else {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}
	if (pid_num == dsp_find_pid_num) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	if (DCM_E_POSITIVERESPONSE == response_code) {
		pdu_tx_data->SduLength = tx_pos;
	}

	if (response_code == DCM_E_POSITIVERESPONSE) {
		dsd_dsp_processing_done(response_code);
	} else {
		dsd_dsp_processing_done_with_no_response();
	}

	return;
}
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CLEAREMISSIONRELATEDDIAGNOSTICDATA == STD_ON)
FUNC(void, DCM_CODE) dsp_obd_clear_emission_related_diag_data(const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data, uint8 client_id)
{
	uint16 message_len = (uint16)(pdu_rx_data->SduLength);
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType result;

	if (message_len != SID_LEN) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	} else {
		(void)dem_select_dtc(client_id, DEM_EMISSION_GROUP_REL_DTCS, DEM_DTC_FORMAT_OBD,
			DEM_DTC_ORIGIN_OBD_REL_MEMORY);
		result = dem_clear_dtc(client_id);
		if (result != E_OK) {
			response_code = DCM_E_CONDITIONSNOTCORRECT;
		} else {
#if ((DEM_OBD_SUPPORT == STD_ON) && (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON))
			dem_set_all_perm_status_passive();
#endif
		}
	}

	if (DCM_E_POSITIVERESPONSE == response_code) {
		pdu_tx_data->SduLength = SID_LEN;
	}

	if ((response_code == DCM_E_POSITIVERESPONSE) || (response_code == DCM_E_CONDITIONSNOTCORRECT)) {
		dsd_dsp_processing_done(response_code);
	} else {
		dsd_dsp_processing_done_with_no_response();
	}

	return;
}
#endif

#if (CONFIG_MODULE_DEM == 1)
#if (DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES == STD_ON) ||                                        \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE == STD_ON) ||      \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS == STD_ON)
static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_obd_sevice_03_07_0a(uint8 client_id, PduInfoType *pdu_tx_data, Std_ReturnType set_dtc_filter_result)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (set_dtc_filter_result != E_OK) {
		return DCM_E_CONDITIONSNOTCORRECT;
	}

	uint32 dtc;
	dem_event_status_rec_type dtc_status;
	uint8 dsp_dtc_nr = 0;
	uint16 indx = 2;

	while ((dem_get_next_filtered_dtc(client_id, &dtc, &dtc_status)) == E_OK) {
		PduLengthType next_indx = indx + (PduLengthType)DCM_DSP_DTC_SIZE;

		if (next_indx >= pdu_tx_data->SduLength) {
			response_code = DCM_E_REQUESTOUTOFRANGE;
			break;
		}

		pdu_tx_data->SduDataPtr[indx] = (uint8)DCM_DSP_EMS_BYTE_HIGH_DTCS(dtc);
		pdu_tx_data->SduDataPtr[(uint16)1 + indx] = (uint8)DCM_DSP_EMS_BYTE_LOW_DTCS(dtc);
		indx = next_indx;
		dsp_dtc_nr++;
	}

	if (response_code != DCM_E_POSITIVERESPONSE) {
		return response_code;
	}
	pdu_tx_data->SduLength = indx;
	pdu_tx_data->SduDataPtr[1] = dsp_dtc_nr;

	return response_code;
}
#endif
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES == STD_ON)
FUNC(void, DCM_CODE)
dsp_obd_req_emission_related_diag_trouble_codes(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data,
	uint8 client_id)
{
	Std_ReturnType set_dtc_filter_result;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (pdu_rx_data->SduLength != SID_LEN) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsd_dsp_processing_done_with_no_response();
		return;
	}

	set_dtc_filter_result = dem_set_dtc_filter(client_id, DEM_CONFIRMED_DTC, DEM_DTC_FORMAT_OBD,
		DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);

	response_code = dsp_obd_sevice_03_07_0a(client_id, pdu_tx_data, set_dtc_filter_result);

	if (response_code == DCM_E_POSITIVERESPONSE) {
		dsd_dsp_processing_done(response_code);
	} else {
		dsd_dsp_processing_done_with_no_response();
	}
}
#endif

#if (CONFIG_MODULE_DEM == 1) &&                                                                                        \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE == STD_ON)
FUNC(void, DCM_CODE) dsp_obd_req_emis_dtc_serv07(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data,
	uint8 client_id)
{
	Std_ReturnType set_dtc_filter_result;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (pdu_rx_data->SduLength != SID_LEN) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsd_dsp_processing_done_with_no_response();
		return;
	}

	set_dtc_filter_result =
		dem_set_dtc_filter(client_id, DEM_PENDING_DTC, DEM_DTC_FORMAT_OBD,
			DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);

	response_code = dsp_obd_sevice_03_07_0a(client_id, pdu_tx_data, set_dtc_filter_result);

	if (response_code == DCM_E_POSITIVERESPONSE) {
		dsd_dsp_processing_done(response_code);
	} else {
		dsd_dsp_processing_done_with_no_response();
	}

	return;
}
#endif

#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
static FUNC(boolean, DCM_CODE) dsp_lookup_info_type(uint8 info_type, const dcm_dsp_veh_info_type **info_type_ptr)
{
	const dcm_dsp_veh_info_type *dsp_veh_info = dcm_config_ptr->dsp->dsp_veh_info;
	boolean info_type_found = FALSE;

#if (DCM_PID_LIST_EOL_INDEX != 0)
	for (uint16 index = 0; index < DCM_VID_LIST_EOL_INDEX; index++) {
		if (dsp_veh_info[index].dsp_veh_info_type == info_type) {
			*info_type_ptr = &dsp_veh_info[index];
			info_type_found = TRUE;
			break;
		}
	}
#else
	(void)dsp_veh_info;
#endif

	return info_type_found;
}

static FUNC(boolean, DCM_CODE) dsp_set_availability_info_type_value(uint8 info_type, uint32 *dsp_data_buf)
{
	uint32 databuf = 0u;

#if (DCM_PID_LIST_EOL_INDEX != 0)
	uint8 shift = 0u;
	uint32 temp = 0u;
	const dcm_dsp_veh_info_type *dsp_veh_info = dcm_config_ptr->dsp->dsp_veh_info;
	uint16 index = 0;

	while (index < DCM_VID_LIST_EOL_INDEX) {
		uint8 dsp_veh_info_type = dsp_veh_info[index].dsp_veh_info_type;

		if (dsp_veh_info_type == 0u) {
			index++;
			continue;
		}
		if ((dsp_veh_info_type >= (info_type + DCM_DSP_SUP_MIN_INFO)) &&
		    (dsp_veh_info_type < (info_type + DCM_DSP_SUP_MAX_INFO))) {
			shift = dsp_veh_info_type - info_type;
			temp = (uint32)1 << (uint32)((uint32)DCM_DSP_SUP_MAX_INFO - (uint32)shift);
			databuf |= temp;
		} else if ((dsp_veh_info_type > (info_type + DCM_DSP_SUP_MAX_INFO)
				)) {
			databuf |= 1u;
		} else {
			/* do nothing */
		}
		index++;
	}
#endif
	boolean set_info_type_ok = (databuf == 0u) ? FALSE : TRUE;

	(*dsp_data_buf) = databuf;
	return set_info_type_ok;
}

static FUNC(Std_ReturnType, DCM_CODE)
	dsp_read_vit_data(uint8 *data_buffer, uint16 *data_pos, const dcm_dsp_veh_info_type *source_veh_info_ptr)
{
	Std_ReturnType ret = E_OK;
	dcm_op_status_type op_status = DCM_INITIAL;
	const dcm_dsp_veh_info_data_type *data_items = source_veh_info_ptr->dsp_veh_info_data_items;

	for (uint8 data_idx = 0u; (data_idx < source_veh_info_ptr->dsp_veh_info_number_of_data_items) && (E_OK == ret);
		data_idx++) {
		uint8 data_value_buffer_size = data_items[data_idx].dsp_veh_info_size;

		ret = (data_items[data_idx].dsp_get_veh_info_type_fnc == NULL) ? E_NOT_OK
		: data_items[data_idx].dsp_get_veh_info_type_fnc(op_status, &data_buffer[*data_pos],
			&data_value_buffer_size);

		switch (ret) {
			case E_OK:
				*data_pos += data_items[data_idx].dsp_veh_info_size;
				break;
			case DCM_E_PENDING:
				dsp_uds_infotype_services_pending.dataindex = data_idx;
				dsp_uds_infotype_services_pending.data_pos = *data_pos;
				dsp_uds_infotype_services_pending.source_veh_info_ptr = source_veh_info_ptr;
				dsp_uds_infotype_services_pending.state = DCM_READ_VID_PENDING_READ_DATA;
				break;
			default:
				ret = E_NOT_OK;
				break;
		}
	}
	return ret;
}

static FUNC(Std_ReturnType, DCM_CODE) dsp_read_pending_veh_info_data(uint8 *data_buffer,
	const dcm_dsp_veh_info_type *source_veh_info_ptr, uint16 *out_data_pos)
{
	Std_ReturnType ret = E_OK;
	dcm_op_status_type op_status = DCM_PENDING;
	uint16 data_pos = dsp_uds_infotype_services_pending.data_pos;
	const dcm_dsp_veh_info_data_type *data_items = source_veh_info_ptr->dsp_veh_info_data_items;

	uint8 dataindex_start = dsp_uds_infotype_services_pending.dataindex;

	if (dsp_uds_infotype_services_pending.is_canceled == TRUE) {
		op_status = DCM_CANCEL;
	}

	for (uint8 data_idx = dataindex_start;
		(data_idx < source_veh_info_ptr->dsp_veh_info_number_of_data_items) && (E_OK == ret); data_idx++) {
		uint8 data_value_buffer_size = data_items[data_idx].dsp_veh_info_size;

		ret = (data_items[data_idx].dsp_get_veh_info_type_fnc == NULL) ? E_NOT_OK
			: data_items[data_idx].dsp_get_veh_info_type_fnc(op_status, &data_buffer[data_pos],
				&data_value_buffer_size);

		switch (ret) {
			case E_OK:
				data_pos += data_items[data_idx].dsp_veh_info_size;
				break;
			case DCM_E_PENDING:
				dsp_uds_infotype_services_pending.dataindex = data_idx;
				dsp_uds_infotype_services_pending.data_pos = data_pos;
				dsp_uds_infotype_services_pending.source_veh_info_ptr = source_veh_info_ptr;
				dsp_uds_infotype_services_pending.state = DCM_READ_VID_PENDING_READ_DATA;
				break;
			default:
				ret = E_NOT_OK;
		}
	}

	*out_data_pos = data_pos;

	return ret;
}

static FUNC(void, DCM_CODE) dsp_handle_obd_req_veh_info_resp_code(PduInfoType *pdu_tx_data, uint16 tx_pos,
	dcm_nrc_type response_code) {
	switch (response_code) {
		case DCM_E_RESPONSEPENDING:
			/* do nothing */
			break;
		case DCM_E_POSITIVERESPONSE:
			pdu_tx_data->SduLength = tx_pos;
			dsp_uds_infotype_services_pending.state = DCM_READ_VID_IDLE;
			dsd_dsp_processing_done(response_code);
			break;
		default:
			dsp_uds_infotype_services_pending.state = DCM_READ_VID_IDLE;
			dsd_dsp_processing_done_with_no_response();
			break;
	}
}

FUNC(void, DCM_CODE) dsp_obd_req_vehicle_info(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	uint8 dsp_avail_info_types_num = 0u;
	uint16 tx_pos = SID_LEN;
	uint32 dsp_data_buf;
	uint8 find_num = 0u;
	Std_ReturnType ret = E_NOT_OK;
	PduLengthType info_type_num = pdu_rx_data->SduLength - 1u;
	const dcm_dsp_veh_info_type *source_veh_info_ptr = NULL_PTR;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (dsp_uds_infotype_services_pending.state == DCM_READ_VID_PENDING_READ_DATA) {
		ret = dsp_read_pending_veh_info_data(&pdu_tx_data->SduDataPtr[0],
			dsp_uds_infotype_services_pending.source_veh_info_ptr, &tx_pos);
		switch (ret) {
			case DCM_E_PENDING:
				response_code = DCM_E_RESPONSEPENDING;
				break;
			case E_OK:
				/* do nothing */
				break;
			default:
				response_code = DCM_E_CONDITIONSNOTCORRECT;
				break;
		}

		dsp_handle_obd_req_veh_info_resp_code(pdu_tx_data, tx_pos, response_code);
		return;
	}

	if (dsp_uds_infotype_services_pending.state != DCM_READ_VID_IDLE) {
		dsp_handle_obd_req_veh_info_resp_code(pdu_tx_data, tx_pos, response_code);
		return;
	}

	if ((pdu_rx_data->SduLength < (PduLengthType)DCM_DSP_OBD_REQ_PSG_SIZE) ||
		(pdu_rx_data->SduLength > (PduLengthType)DCM_DSP_OBD_REQ_MSG_MAX_SIZE)) {
		dsp_handle_obd_req_veh_info_resp_code(pdu_tx_data, tx_pos, DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
		return;
	}

	uint8 local_rx_sdu_data[DCM_DSP_OBD_REQ_MSG_MAX_SIZE];
	(void)memcpy(local_rx_sdu_data, pdu_rx_data->SduDataPtr, pdu_rx_data->SduLength);
	uint16 i = 0u;

	while (i < info_type_num) {
		if (DCM_DSP_IS_AVAIL_INFO_TYPE(local_rx_sdu_data[i + 1u])) {
			dsp_avail_info_types_num++;
		}
		i++;
	}

	if (info_type_num == dsp_avail_info_types_num) {
		i = 0u;

		while (i < info_type_num) {
			if (TRUE ==
				dsp_set_availability_info_type_value(local_rx_sdu_data[i + 1u], &dsp_data_buf)) {
				pdu_tx_data->SduDataPtr[tx_pos] = local_rx_sdu_data[i + 1u];
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)1] =
					(uint8)((dsp_data_buf & (DCM_DSP_OBD_LSB_MASK << OFFSET_THREE_BYTES)) >>
						OFFSET_THREE_BYTES);
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)2] =
					(uint8)((dsp_data_buf & (DCM_DSP_OBD_LSB_MASK << OFFSET_TWO_BYTES)) >>
						OFFSET_TWO_BYTES);
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)3] =
					(uint8)((dsp_data_buf & (DCM_DSP_OBD_LSB_MASK << OFFSET_ONE_BYTE)) >>
						OFFSET_ONE_BYTE);
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)4] =
					(uint8)(dsp_data_buf & DCM_DSP_OBD_LSB_MASK);
				tx_pos = tx_pos + (uint16)5;
			} else if ((uint8)DCM_DSP_DEFAULT_INFO_VALUE == local_rx_sdu_data[i + 1u]) {
				pdu_tx_data->SduDataPtr[tx_pos] = local_rx_sdu_data[i + 1u];
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)1] = DCM_DSP_DEFAULT_DATA_VALUE;
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)2] = DCM_DSP_DEFAULT_DATA_VALUE;
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)3] = DCM_DSP_DEFAULT_DATA_VALUE;
				pdu_tx_data->SduDataPtr[tx_pos + (uint16)4] = DCM_DSP_DEFAULT_DATA_VALUE;
				tx_pos = tx_pos + (uint16)5;
			} else {
				find_num++;
			}
			i++;
		}
	} else if ((dsp_avail_info_types_num == (uint8)0) &&
		   (pdu_rx_data->SduLength == (PduLengthType)DCM_DSP_OBD_REQ_PSG_SIZE)) {
			if (TRUE == dsp_lookup_info_type(local_rx_sdu_data[1], &source_veh_info_ptr)) {
				if (pdu_tx_data->SduLength >= (PduLengthType)((PduLengthType)3 +
						source_veh_info_ptr->dsp_veh_info_total_size)) {
					pdu_tx_data->SduDataPtr[tx_pos] = local_rx_sdu_data[1];
					tx_pos++;
					pdu_tx_data->SduDataPtr[tx_pos] =
						source_veh_info_ptr->dsp_veh_info_number_of_data_items;
					tx_pos++;
					ret = dsp_read_vit_data(&pdu_tx_data->SduDataPtr[0], &tx_pos, source_veh_info_ptr);
					switch (ret) {
						case E_NOT_OK:
							response_code = DCM_E_CONDITIONSNOTCORRECT;
							break;
						case DCM_E_PENDING:
							response_code = DCM_E_RESPONSEPENDING;
							(void)memcpy(pdu_rx_data->SduDataPtr, local_rx_sdu_data,
								pdu_rx_data->SduLength);
							dsp_uds_infotype_services_pending.pdu_rx_data = pdu_rx_data;
							dsp_uds_infotype_services_pending.pdu_tx_data = pdu_tx_data;
							break;
						default:
							break;
					}
				} else {
					response_code = DCM_E_REQUESTOUTOFRANGE;
				}
			} else {
				find_num++;
			}
	} else {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	if (find_num == info_type_num) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	dsp_handle_obd_req_veh_info_resp_code(pdu_tx_data, tx_pos, response_code);
}
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS == STD_ON)

FUNC(void, DCM_CODE)
dsp_obd_req_emis_dtc_with_perm_st(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id)
{
	Std_ReturnType set_dtc_filter_result;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (pdu_rx_data->SduLength != SID_LEN) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	} else {
		set_dtc_filter_result =
			dem_set_dtc_filter(client_id, DEM_DTC_STATUS_MASK_ALL, DEM_DTC_FORMAT_OBD,
				DEM_DTC_ORIGIN_PERMANENT_MEMORY, FALSE, DCM_VALUE_IS_NOT_USED, FALSE);
		response_code = dsp_obd_sevice_03_07_0a(client_id, pdu_tx_data, set_dtc_filter_result);
	}

	if (response_code == DCM_E_POSITIVERESPONSE) {
		dsd_dsp_processing_done(response_code);
	} else {
		dsd_dsp_processing_done_with_no_response();
	}
}
#endif

#ifdef DCM_USE_UPLOAD_DOWNLOAD
FUNC(void, DCM_CODE) dsp_uds_upload_download_pending_set(general_pend_state_type state,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 pend_svc)
{
	dsp_upload_download_pending.state = state;
	dsp_upload_download_pending.pdu_rx_data = pdu_rx_data;
	dsp_upload_download_pending.pdu_tx_data = pdu_tx_data;
	dsp_upload_download_pending.pend_svc = pend_svc;
}
#endif

#if (DCM_USE_SERVICE_REQUESTDOWNLOAD == STD_ON)
FUNC(void, DCM_CODE) dsp_uds_request_download(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	uint32 dsp_block_size = 0u;
	uint32 memory_size = 0u;
	Std_ReturnType ret = E_NOT_OK;
	uint32 memory_address = 0u;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	dsp_set_op_status_depend_on_state(&dsp_upload_download_pending, &op_status, FALSE);
	if (pdu_rx_data->SduLength < 5u) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}

	uint8 data_format_identifier = pdu_rx_data->SduDataPtr[1];

	if (pdu_rx_data->SduDataPtr[2] == 0u) {
		response_code = DCM_E_REQUESTOUTOFRANGE;
		dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}

	uint8 nof_size_bytes = (pdu_rx_data->SduDataPtr[2] & DCM_FORMAT_HIGH_MASK) >> 4u;
	uint8 nof_addr_bytes = pdu_rx_data->SduDataPtr[2] & DCM_FORMAT_LOW_MASK;

	if (!((pdu_rx_data->SduLength == (PduLengthType)(3u + nof_size_bytes + nof_addr_bytes)) &&
		(0u < nof_size_bytes) && (4u >= nof_size_bytes) && (0u < nof_addr_bytes) &&
		(4u >= nof_addr_bytes))) {
			response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	} else {
		uint8 idx = 0;

		while (idx < nof_addr_bytes) {
			memory_address <<= 8;
			memory_address += (uint32)pdu_rx_data->SduDataPtr[3u + idx];
			idx++;
		}
		idx = 0;

		while (idx < nof_size_bytes) {
			memory_size <<= 8;
			memory_size += (uint32)pdu_rx_data->SduDataPtr[3u + nof_addr_bytes + idx];
			idx++;
		}
		ret = (DCM_INITIAL == op_status) ? dcm_process_request_download(op_status, data_format_identifier,
			memory_address, memory_size, &dsp_block_size, &response_code)
			: dcm_process_request_download(op_status, 0, 0, 0, &dsp_block_size, &response_code);

		if (E_OK == ret) {
			pdu_tx_data->SduDataPtr[1] = 0x40;
			pdu_tx_data->SduDataPtr[2] = (uint8)((dsp_block_size >> 24) & 0xffu);
			pdu_tx_data->SduDataPtr[3] = (uint8)((dsp_block_size >> 16) & 0xffu);
			pdu_tx_data->SduDataPtr[4] = (uint8)((dsp_block_size >> 8) & 0xffu);
			pdu_tx_data->SduDataPtr[5] = (uint8)(dsp_block_size & 0xffu);
			pdu_tx_data->SduLength = 6;
			response_code = DCM_E_POSITIVERESPONSE;
			dsp_transfer_status.transfer_type = DCM_DOWNLOAD;
			dsp_transfer_status.block_sequence_counter = 1;
			dsp_transfer_status.first_block_received = FALSE;
			dsp_transfer_status.next_address = memory_address;
		} else if (DCM_E_PENDING == ret) {
			response_code = DCM_E_RESPONSEPENDING;
		} else if (DCM_E_FORCE_RCRRP == ret) {
			response_code = DCM_E_FORCE_RCRRP;
		} else if (E_NOT_OK == ret) {
			/* do nothing */
		} else {
			DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
			response_code = DCM_E_REQUESTOUTOFRANGE;
		}
	}

	switch (response_code) {
		case DCM_E_FORCE_RCRRP:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_DOWNLOAD);
			dsd_dsp_force_response_pending();
			break;
		case DCM_E_RESPONSEPENDING:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_PENDING,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_DOWNLOAD);
			break;
		default:
			dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
			dsd_dsp_processing_done(response_code);
			break;
	}
}
#endif

#if (DCM_USE_SERVICE_REQUESTUPLOAD == STD_ON)
FUNC(void, DCM_CODE) dsp_uds_request_upload(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	uint32 memory_address = 0u;
	uint32 memory_size = 0u;
	Std_ReturnType ret = E_NOT_OK;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	uint32 max_number_of_block_length = 0u;

	dsp_set_op_status_depend_on_state(&dsp_upload_download_pending, &op_status, FALSE);

	if (pdu_rx_data->SduLength < 5u) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	} else {
		uint8 data_format_identifier = pdu_rx_data->SduDataPtr[1];
		uint8 nof_size_bytes = (pdu_rx_data->SduDataPtr[2] & DCM_FORMAT_HIGH_MASK) >> 4u;
		uint8 nof_addr_bytes = pdu_rx_data->SduDataPtr[2] & DCM_FORMAT_LOW_MASK;

		if (!((pdu_rx_data->SduLength == (3u + (PduLengthType)nof_size_bytes + (PduLengthType)nof_addr_bytes)) &&
			(0u < nof_size_bytes) && (4u >= nof_size_bytes) && (0u < nof_addr_bytes) &&
			(4u >= nof_addr_bytes))) {
			response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		} else {
			uint8 idx = 0;

			while (idx < nof_addr_bytes) {
				memory_address <<= 8;
				memory_address += (pdu_rx_data->SduDataPtr[3u + idx]);
				idx++;
			}
			idx = 0;

			while (idx < nof_size_bytes) {
				memory_size <<= 8;
				memory_size += (pdu_rx_data->SduDataPtr[3u + nof_addr_bytes + idx]);
				dsp_transfer_status.upl_bytes_left = memory_size;
				idx++;
			}
			Std_ReturnType ret = (DCM_INITIAL == op_status) ? dcm_process_request_upload(op_status,
				data_format_identifier, memory_address, memory_size, &response_code)
			: dcm_process_request_upload(op_status, 0, 0, 0, &response_code);

			if (E_OK == ret) {
				max_number_of_block_length = (uint32)pdu_tx_data->SduLength;
				pdu_tx_data->SduDataPtr[1] = 0x40;
				pdu_tx_data->SduDataPtr[2] =
					(uint8)((max_number_of_block_length >> 24) & 0xffu);
				pdu_tx_data->SduDataPtr[3] =
					(uint8)((max_number_of_block_length >> 16) & 0xffu);
				pdu_tx_data->SduDataPtr[4] = (uint8)((max_number_of_block_length >> 8) & 0xffu);
				pdu_tx_data->SduDataPtr[5] = (uint8)(max_number_of_block_length & 0xffu);
				pdu_tx_data->SduLength = 6;
				response_code = DCM_E_POSITIVERESPONSE;
				dsp_transfer_status.transfer_type = DCM_UPLOAD;
				dsp_transfer_status.block_sequence_counter = 1;
				dsp_transfer_status.first_block_received = FALSE;
				dsp_transfer_status.next_address = memory_address;
				dsp_transfer_status.upl_mem_block_size = max_number_of_block_length - 2u;
			} else if (DCM_E_PENDING == ret) {
				response_code = DCM_E_RESPONSEPENDING;
			} else if (DCM_E_FORCE_RCRRP == ret) {
				response_code = DCM_E_FORCE_RCRRP;
			} else if (E_NOT_OK == ret) {
				/* do nothing */
			} else {
				DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
				response_code = DCM_E_REQUESTOUTOFRANGE;
			}
		}
	}

	switch (response_code) {
		case DCM_E_FORCE_RCRRP:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_UPLOAD);
			dsd_dsp_force_response_pending();
			break;
		case DCM_E_RESPONSEPENDING:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_PENDING,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_UPLOAD);
			break;
		default:
			dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
			dsd_dsp_processing_done(response_code);
			break;
	}
}
#endif

#if (DCM_USE_SERVICE_TRANSFERDATA == STD_ON)
static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_dcm_dwonload_write_memory(dcm_op_status_type op_status,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, PduLengthType *resp_param_rec_len)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	dcm_return_write_memory_type write_ret;

	write_ret = (DCM_INITIAL == op_status) ? dcm_write_memory(op_status, 0, dsp_transfer_status.next_address,
		(uint32)(pdu_rx_data->SduLength - 2u), &pdu_rx_data->SduDataPtr[2])
		: dcm_write_memory(op_status, 0, 0, 0, NULL);

	if (DCM_WRITE_OK == write_ret) {
		dsp_transfer_status.block_sequence_counter++;
		dsp_transfer_status.first_block_received = TRUE;
		dsp_transfer_status.next_address += (uint32)(pdu_rx_data->SduLength - 2u);
		pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
		dcm_get_download_response_parameter_record(&pdu_tx_data->SduDataPtr[2], resp_param_rec_len);
		pdu_tx_data->SduLength = 2u + *resp_param_rec_len;
	} else if (DCM_WRITE_PENDING == write_ret) {
		response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_WRITE_FORCE_RCRRP == write_ret) {
		response_code = DCM_E_FORCE_RCRRP;
	} else {
		response_code = DCM_E_GENERALREJECT;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_dcm_upload_write_memory(dcm_op_status_type op_status,
	PduInfoType *pdu_tx_data, PduLengthType *resp_param_rec_len)
{
	dcm_return_read_memory_type read_ret = DCM_READ_OK;
	uint32 read_mem_size = 0;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	if (DCM_INITIAL == op_status) {
		read_mem_size = MIN((dsp_transfer_status.upl_mem_block_size), (dsp_transfer_status.upl_bytes_left));
		read_ret = dcm_read_memory(op_status, 0, dsp_transfer_status.next_address, read_mem_size,
			&pdu_tx_data->SduDataPtr[2]);
	} else {
		read_ret = dcm_read_memory(op_status, 0, 0, 0, NULL);
	}
	if (DCM_READ_OK == read_ret) {
		dsp_transfer_status.first_block_received = TRUE;
		dsp_transfer_status.next_address += read_mem_size;
		dsp_transfer_status.upl_bytes_left -= read_mem_size;
		*resp_param_rec_len = (PduLengthType)read_mem_size;
		pdu_tx_data->SduDataPtr[1] = dsp_transfer_status.block_sequence_counter;
		dsp_transfer_status.block_sequence_counter++;
		pdu_tx_data->SduLength = 2u + *resp_param_rec_len;
	} else if (DCM_READ_PENDING == read_ret) {
		response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_READ_FORCE_RCRRP == read_ret) {
		response_code = DCM_E_FORCE_RCRRP;
	} else {
		response_code = DCM_E_GENERALREJECT;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_dcm_dwonload_write_file(dcm_op_status_type op_status,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, PduLengthType *resp_param_rec_len)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;

	Std_ReturnType write_ret =
		(DCM_INITIAL == op_status) ? dcm_write_file(op_status, ((uint32)pdu_rx_data->SduLength - 2u),
			&pdu_rx_data->SduDataPtr[2], &response_code) : dcm_write_file(op_status, 0, NULL, &response_code);

	if (E_OK == write_ret) {
		dsp_transfer_status.block_sequence_counter++;
		dsp_transfer_status.first_block_received = TRUE;
		pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
		dcm_get_download_response_parameter_record(&pdu_tx_data->SduDataPtr[2], resp_param_rec_len);
		pdu_tx_data->SduLength = 2u + *resp_param_rec_len;
	} else if (DCM_E_FORCE_RCRRP == write_ret) {
		response_code = DCM_E_FORCE_RCRRP;
	} else if (DCM_E_PENDING == write_ret) {
		response_code = DCM_E_RESPONSEPENDING;
	} else {
		response_code = DCM_E_GENERALREJECT;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE)
	dsp_do_dcm_upload_read_file_or_dir(dcm_op_status_type op_status, PduInfoType *pdu_tx_data,
		PduLengthType *resp_param_rec_len)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType read_ret;
	uint32 data_length = (uint32)pdu_tx_data->SduLength - 2u;

	if (DCM_INITIAL == op_status) {
		read_ret = dcm_read_file_or_dir(op_status, &data_length, &pdu_tx_data->SduDataPtr[2], &response_code);
	} else {
		read_ret = dcm_read_file_or_dir(op_status, NULL, NULL, &response_code);
	}
	if (E_OK == read_ret) {
		dsp_transfer_status.first_block_received = TRUE;
		*resp_param_rec_len = (PduLengthType)data_length;
		pdu_tx_data->SduDataPtr[1] = dsp_transfer_status.block_sequence_counter;
		dsp_transfer_status.block_sequence_counter++;
		pdu_tx_data->SduLength = 2u + *resp_param_rec_len;
	} else if (DCM_E_PENDING == read_ret) {
		response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_E_FORCE_RCRRP == read_ret) {
		response_code = DCM_E_FORCE_RCRRP;
	} else {
		response_code = DCM_E_GENERALREJECT;
	}

	return response_code;
}

static FUNC(dcm_nrc_type, DCM_CODE) dsp_do_check_block_sequence_counter(dcm_op_status_type op_status,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, PduLengthType *resp_param_rec_len)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	uint8 ok_counter;
	Std_ReturnType read_ret;
	uint32 data_length;

	ok_counter = (0U == dsp_transfer_status.block_sequence_counter) ? 0xFFU
	: (dsp_transfer_status.block_sequence_counter - 1U);
	if ((FALSE == dsp_transfer_status.first_block_received) || (pdu_rx_data->SduDataPtr[1] != ok_counter)) {
		response_code = DCM_E_WRONGBLOCKSEQUENCECOUNTER;
	} else {
		pdu_tx_data->SduDataPtr[1] = pdu_rx_data->SduDataPtr[1];
		switch (dsp_transfer_status.transfer_type) {
			case DCM_DOWNLOAD:
			case DCM_WRITEFILE:
				dcm_get_download_response_parameter_record(&pdu_tx_data->SduDataPtr[2], resp_param_rec_len);
				break;
			case DCM_UPLOAD:
				read_ret = dcm_read_memory(op_status, 0,
					dsp_transfer_status.next_address, MIN((dsp_transfer_status.upl_mem_block_size),
					(dsp_transfer_status.upl_bytes_left)), &pdu_tx_data->SduDataPtr[2]);
				if (DCM_READ_OK == read_ret) {
					/* do nothing */
				} else {
					response_code = (DCM_READ_PENDING == read_ret) ? DCM_E_RESPONSEPENDING : DCM_E_GENERALREJECT;
				}
				break;
			case DCM_READFILE:
				data_length = (uint32)pdu_tx_data->SduLength - 2u;
				read_ret = dcm_read_file_or_dir(op_status, &data_length,
					&pdu_tx_data->SduDataPtr[2], &response_code);
				if (E_OK == read_ret) {
					/* do nothing */
				} else if (DCM_E_FORCE_RCRRP == read_ret) {
					response_code = DCM_E_FORCE_RCRRP;
				} else if (DCM_E_PENDING == read_ret) {
					response_code = DCM_E_RESPONSEPENDING;
				} else {
					response_code = DCM_E_GENERALREJECT;
				}
				break;
			default:
				break;
		}
		pdu_tx_data->SduLength = 2u + *resp_param_rec_len;
	}

	return response_code;
}

static FUNC(void, DCM_CODE) dsp_do_dsp_uds_upload_download_pending(dcm_nrc_type response_code,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	switch (response_code) {
		case DCM_E_RESPONSEPENDING:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_PENDING,
			pdu_rx_data, pdu_tx_data, DCM_SID_TRANSFER_DATA);
			break;
		case DCM_E_FORCE_RCRRP:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND,
			pdu_rx_data, pdu_tx_data, DCM_SID_TRANSFER_DATA);
			dsd_dsp_force_response_pending();
			break;
		default:
			dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
			dsd_dsp_processing_done(response_code);
			break;
	}
}

FUNC(void, DCM_CODE) dsp_uds_transfer_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	PduLengthType resp_param_rec_len;
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	dsp_set_op_status_depend_on_state(&dsp_upload_download_pending, &op_status, FALSE);

	if (DCM_NO_DATA_TRANSFER == dsp_transfer_status.transfer_type) {
		response_code = DCM_E_REQUESTSEQUENCEERROR;
		dsp_do_dsp_uds_upload_download_pending(response_code, pdu_rx_data, pdu_tx_data);
		return;
	}
	if (!(((pdu_rx_data->SduLength >= 2u) && ((DCM_UPLOAD == dsp_transfer_status.transfer_type) ||
			(DCM_READFILE == dsp_transfer_status.transfer_type))) ||
		((pdu_rx_data->SduLength >= 3u) && ((DCM_DOWNLOAD == dsp_transfer_status.transfer_type) ||
			(DCM_WRITEFILE == dsp_transfer_status.transfer_type))))) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		dsp_do_dsp_uds_upload_download_pending(response_code, pdu_rx_data, pdu_tx_data);
		return;
	}

	resp_param_rec_len = pdu_tx_data->SduLength - 2u;

	if ((pdu_rx_data->SduDataPtr[1] != dsp_transfer_status.block_sequence_counter)) {
		response_code = dsp_do_check_block_sequence_counter(op_status, pdu_rx_data, pdu_tx_data,
			&resp_param_rec_len);
		dsp_do_dsp_uds_upload_download_pending(response_code, pdu_rx_data, pdu_tx_data);
		return;
	}

	if (DCM_DOWNLOAD == dsp_transfer_status.transfer_type) {
		response_code = dsp_do_dcm_dwonload_write_memory(op_status, pdu_rx_data,
			pdu_tx_data, &resp_param_rec_len);
	} else if (DCM_UPLOAD == dsp_transfer_status.transfer_type) {
		response_code = dsp_do_dcm_upload_write_memory(op_status, pdu_tx_data, &resp_param_rec_len);
	} else if (DCM_WRITEFILE == dsp_transfer_status.transfer_type) {
		response_code = dsp_do_dcm_dwonload_write_file(op_status, pdu_rx_data, pdu_tx_data,
			&resp_param_rec_len);
	} else {
		response_code = (DCM_READFILE == dsp_transfer_status.transfer_type)
			? dsp_do_dcm_upload_read_file_or_dir(op_status, pdu_tx_data, &resp_param_rec_len)
			: DCM_E_REQUESTOUTOFRANGE;
	}

	dsp_do_dsp_uds_upload_download_pending(response_code, pdu_rx_data, pdu_tx_data);
}
#endif

#if (DCM_USE_SERVICE_REQUESTTRANSFEREXIT == STD_ON)
FUNC(void, DCM_CODE) dsp_uds_request_transfer_exit(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	Std_ReturnType ret;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	dsp_set_op_status_depend_on_state(&dsp_upload_download_pending, &op_status, FALSE);

	if (DCM_NO_DATA_TRANSFER == dsp_transfer_status.transfer_type) {
		response_code = DCM_E_REQUESTSEQUENCEERROR;
		dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
		dsd_dsp_processing_done(response_code);
		return;
	}
	ret = (DCM_INITIAL == op_status) ? dcm_process_request_transfer_exit(DCM_INITIAL, &pdu_rx_data->SduDataPtr[1],
		(uint32)pdu_rx_data->SduLength - 1u, &response_code)
		: dcm_process_request_transfer_exit(op_status, NULL, 0, &response_code);

	if (E_OK == ret) {
		dsp_transfer_status.transfer_type = DCM_NO_DATA_TRANSFER;
		PduLengthType resp_param_rec_len = pdu_tx_data->SduLength;

		dcm_get_transfer_exit_response_parameter_record(&pdu_tx_data->SduDataPtr[1],
			&resp_param_rec_len);
		pdu_tx_data->SduLength = resp_param_rec_len + 1u;
		response_code = DCM_E_POSITIVERESPONSE;
	} else if (DCM_E_PENDING == ret) {
		response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_E_FORCE_RCRRP == ret) {
		response_code = DCM_E_FORCE_RCRRP;
	} else if (E_NOT_OK == ret) {
		/* do nothing */
	} else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	switch (response_code) {
		case DCM_E_FORCE_RCRRP:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_TRANSFER_EXIT);
			dsd_dsp_force_response_pending();
			break;
		case DCM_E_RESPONSEPENDING:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_PENDING,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_TRANSFER_EXIT);
			break;
		default:
			dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
			dsd_dsp_processing_done(response_code);
			break;
	}
}
#endif

#if (DCM_USE_SERVICE_REQUESTFILETRANSFER == STD_ON)
static FUNC(void, DCM_CODE)
	dsp_do_dcm_uds_request_file_add_file(dcm_op_status_type op_status, const PduInfoType *pdu_rx_data,
		PduInfoType *pdu_tx_data, dcm_nrc_type *response_code)
{
	Std_ReturnType ret;
	uint16 file_path_and_name_length = ((uint16)pdu_rx_data->SduDataPtr[2] << 8) + pdu_rx_data->SduDataPtr[3];
	uint8 data_format_identifier = pdu_rx_data->SduDataPtr[4u + file_path_and_name_length];
	uint8 file_size_parameter_length = pdu_rx_data->SduDataPtr[5u + file_path_and_name_length];
	uint32 file_size_uncompressed = 0u;
	uint32 file_size_compressed = 0u;
	uint32 max_number_of_block_length = 0u;

	if ((file_size_parameter_length < 1u) || (file_size_parameter_length > 8u)) {
		*response_code = DCM_E_REQUESTOUTOFRANGE;
		return;
	}

	if (file_size_parameter_length > 4u) {
		*response_code = DCM_E_CONDITIONSNOTCORRECT;
		return;
	}

	if (pdu_rx_data->SduLength !=
		(6u + (2u * (uint16)file_size_parameter_length) + file_path_and_name_length)) {
		*response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		return;
	}

	for (uint8 idx = 0; idx < file_size_parameter_length; idx++) {
		file_size_uncompressed <<= 8;
		file_size_compressed <<= 8;
		file_size_uncompressed +=
			(pdu_rx_data->SduDataPtr[6u + file_path_and_name_length + idx]);
		file_size_compressed +=
			(pdu_rx_data->SduDataPtr[6u + file_path_and_name_length +
			file_size_parameter_length + idx]);
	}

	if (DCM_INITIAL == op_status) {
		ret = dcm_process_request_add_file(op_status, file_path_and_name_length,
			&pdu_rx_data->SduDataPtr[4],
			data_format_identifier,
			file_size_uncompressed, file_size_compressed,
			&max_number_of_block_length, response_code);
	} else {
		ret = dcm_process_request_add_file(op_status, 0, NULL, 0, 0, 0,
			&max_number_of_block_length, response_code);
	}

	if (E_OK == ret) {
		pdu_tx_data->SduDataPtr[1] = DCM_ADD_FILE;
		pdu_tx_data->SduDataPtr[2] = 0x04;
		pdu_tx_data->SduDataPtr[3] =
			(uint8)((max_number_of_block_length >> 24) & 0xffu);
		pdu_tx_data->SduDataPtr[4] =
			(uint8)((max_number_of_block_length >> 16) & 0xffu);
		pdu_tx_data->SduDataPtr[5] =
			(uint8)((max_number_of_block_length >> 8) & 0xffu);
		pdu_tx_data->SduDataPtr[6] =
			(uint8)(max_number_of_block_length & 0xffu);
		pdu_tx_data->SduDataPtr[7] = data_format_identifier;
		pdu_tx_data->SduLength = 8;
		*response_code = DCM_E_POSITIVERESPONSE;
		dsp_transfer_status.transfer_type = DCM_WRITEFILE;
		dsp_transfer_status.file_transfer_type = DCM_ADD_FILE;
		dsp_transfer_status.block_sequence_counter = 1;
		dsp_transfer_status.first_block_received = FALSE;
	} else if (DCM_E_PENDING == ret) {
		*response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_E_FORCE_RCRRP == ret) {
		*response_code = DCM_E_FORCE_RCRRP;
	} else if (E_NOT_OK == ret) {
		if (DCM_E_POSITIVERESPONSE == *response_code) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
		}
	} else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		*response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	return;
}

static FUNC(void, DCM_CODE) dsp_do_dcm_uds_req_file_delete_file(dcm_op_status_type op_status,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, dcm_nrc_type *response_code)
{
	Std_ReturnType ret;
	uint16 file_path_and_name_length = ((uint16)pdu_rx_data->SduDataPtr[2] << 8) + pdu_rx_data->SduDataPtr[3];

	if (pdu_rx_data->SduLength != (4u + file_path_and_name_length)) {
		*response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		return;
	}
	ret = (DCM_INITIAL == op_status) ? dcm_process_request_delete_file(op_status, file_path_and_name_length,
		&pdu_rx_data->SduDataPtr[4], response_code)
		: dcm_process_request_delete_file(op_status, 0, NULL, response_code);

	if (E_OK == ret) {
		dsp_transfer_status.file_transfer_type = DCM_DELETE_FILE;
		pdu_tx_data->SduDataPtr[1] = DCM_DELETE_FILE;
		pdu_tx_data->SduLength = 2;
		*response_code = DCM_E_POSITIVERESPONSE;
	} else if (DCM_E_PENDING == ret) {
		*response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_E_FORCE_RCRRP == ret) {
		*response_code = DCM_E_FORCE_RCRRP;
	} else if (E_NOT_OK == ret) {
		if (DCM_E_POSITIVERESPONSE == *response_code) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
		}
	} else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		*response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	return;
}

static FUNC(void, DCM_CODE) dsp_do_dcm_uds_req_file_replace_file(dcm_op_status_type op_status,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, dcm_nrc_type *response_code)
{
	Std_ReturnType ret;
	uint16 file_path_and_name_length = ((uint16)pdu_rx_data->SduDataPtr[2] << 8) + pdu_rx_data->SduDataPtr[3];
	uint8 data_format_identifier = pdu_rx_data->SduDataPtr[4u + file_path_and_name_length];
	uint8 file_size_parameter_length = pdu_rx_data->SduDataPtr[5u + file_path_and_name_length];
	uint32 file_size_uncompressed = 0u;
	uint32 file_size_compressed = 0u;
	uint32 max_number_of_block_length = 0u;

	if ((file_size_parameter_length < 1u) || (file_size_parameter_length > 8u)) {
		*response_code = DCM_E_REQUESTOUTOFRANGE;
		return;
	}

	if (file_size_parameter_length > 4u) {
		*response_code = DCM_E_CONDITIONSNOTCORRECT;
		return;
	}

	if (pdu_rx_data->SduLength !=
		(6u + (2u * (uint16)file_size_parameter_length) + file_path_and_name_length)) {
		*response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		return;
	}

	for (uint8 idx = 0; idx < file_size_parameter_length; idx++) {
		file_size_uncompressed <<= 8;
		file_size_compressed <<= 8;
		file_size_uncompressed +=
			(pdu_rx_data->SduDataPtr[6u + file_path_and_name_length + idx]);
		file_size_compressed +=
			(pdu_rx_data->SduDataPtr[6u + file_path_and_name_length +
			file_size_parameter_length + idx]);
	}

	ret = (DCM_INITIAL == op_status) ? dcm_process_request_replace_file(op_status, file_path_and_name_length,
			&pdu_rx_data->SduDataPtr[4], data_format_identifier, file_size_uncompressed,
			file_size_compressed, &max_number_of_block_length, response_code)
			: dcm_process_request_replace_file(op_status, 0, NULL, 0, 0, 0,
			&max_number_of_block_length, response_code);

	if (E_OK == ret) {
		pdu_tx_data->SduDataPtr[1] = DCM_REPLACE_FILE;
		pdu_tx_data->SduDataPtr[2] = 0x04;
		pdu_tx_data->SduDataPtr[3] =
			(uint8)((max_number_of_block_length >> 24) & 0xffu);
		pdu_tx_data->SduDataPtr[4] =
			(uint8)((max_number_of_block_length >> 16) & 0xffu);
		pdu_tx_data->SduDataPtr[5] =
			(uint8)((max_number_of_block_length >> 8) & 0xffu);
		pdu_tx_data->SduDataPtr[6] =
			(uint8)(max_number_of_block_length & 0xffu);
		pdu_tx_data->SduDataPtr[7] = data_format_identifier;
		pdu_tx_data->SduLength = 8;
		*response_code = DCM_E_POSITIVERESPONSE;
		dsp_transfer_status.transfer_type = DCM_WRITEFILE;
		dsp_transfer_status.file_transfer_type = DCM_REPLACE_FILE;
		dsp_transfer_status.block_sequence_counter = 1;
		dsp_transfer_status.first_block_received = FALSE;
	} else if (DCM_E_PENDING == ret) {
		*response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_E_FORCE_RCRRP == ret) {
		*response_code = DCM_E_FORCE_RCRRP;
	} else if (E_NOT_OK == ret) {
		if (*response_code == DCM_E_POSITIVERESPONSE) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
		}
	} else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		*response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	return;
}

static FUNC(void, DCM_CODE)
	dsp_do_dcm_uds_req_file_read_file(dcm_op_status_type op_status, const PduInfoType *pdu_rx_data,
	PduInfoType *pdu_tx_data, dcm_nrc_type *response_code)
{
	Std_ReturnType ret;
	uint16 file_path_and_name_length = ((uint16)pdu_rx_data->SduDataPtr[2] << 8) + pdu_rx_data->SduDataPtr[3];
	uint8 data_format_identifier = pdu_rx_data->SduDataPtr[4u + file_path_and_name_length];

	uint32 file_size_uncompressed = 0u;
	uint32 file_size_compressed = 0u;
	uint32 max_number_of_block_length = 0u;

	if (pdu_rx_data->SduLength != (5u + file_path_and_name_length)) {
		*response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		return;
	}

	ret = (DCM_INITIAL == op_status)
	? dcm_process_request_read_file(op_status, file_path_and_name_length,
		&pdu_rx_data->SduDataPtr[4], data_format_identifier,
		&file_size_uncompressed, &file_size_compressed,
		&max_number_of_block_length, response_code)
	: dcm_process_request_read_file(op_status, 0, NULL, 0, &file_size_uncompressed,
		&file_size_compressed, &max_number_of_block_length, response_code);

	if (E_OK == ret) {
		max_number_of_block_length = (uint32)MIN((max_number_of_block_length),
			((uint32)(pdu_tx_data->SduLength) - 2u));
		pdu_tx_data->SduDataPtr[1] = DCM_READ_FILE;
		pdu_tx_data->SduDataPtr[2] = 0x04;
		pdu_tx_data->SduDataPtr[3] = (uint8)((max_number_of_block_length >> 24) & 0xffu);
		pdu_tx_data->SduDataPtr[4] = (uint8)((max_number_of_block_length >> 16) & 0xffu);
		pdu_tx_data->SduDataPtr[5] = (uint8)((max_number_of_block_length >> 8) & 0xffu);
		pdu_tx_data->SduDataPtr[6] = (uint8)(max_number_of_block_length & 0xffu);
		pdu_tx_data->SduDataPtr[7] = data_format_identifier;
		pdu_tx_data->SduDataPtr[8] = 0x00;
		pdu_tx_data->SduDataPtr[9] = 0x04;
		pdu_tx_data->SduDataPtr[10] = (uint8)((file_size_uncompressed >> 24) & 0xffu);
		pdu_tx_data->SduDataPtr[11] = (uint8)((file_size_uncompressed >> 16) & 0xffu);
		pdu_tx_data->SduDataPtr[12] = (uint8)((file_size_uncompressed >> 8) & 0xffu);
		pdu_tx_data->SduDataPtr[13] = (uint8)(file_size_uncompressed & 0xffu);
		pdu_tx_data->SduDataPtr[14] = (uint8)((file_size_compressed >> 24) & 0xffu);
		pdu_tx_data->SduDataPtr[15] = (uint8)((file_size_compressed >> 16) & 0xffu);
		pdu_tx_data->SduDataPtr[16] = (uint8)((file_size_compressed >> 8) & 0xffu);
		pdu_tx_data->SduDataPtr[17] = (uint8)(file_size_compressed & 0xffu);
		pdu_tx_data->SduLength = 18;

		response_code = DCM_E_POSITIVERESPONSE;
		dsp_transfer_status.transfer_type = DCM_READFILE;
		dsp_transfer_status.file_transfer_type = DCM_READ_FILE;
		dsp_transfer_status.block_sequence_counter = 1;
		dsp_transfer_status.first_block_received = FALSE;
	} else if (DCM_E_PENDING == ret) {
		*response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_E_FORCE_RCRRP == ret) {
		*response_code = DCM_E_FORCE_RCRRP;
	} else if (E_NOT_OK == ret) {
		if (DCM_E_POSITIVERESPONSE == *response_code) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
		}
	} else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		*response_code = DCM_E_REQUESTOUTOFRANGE;
	}
	return;
}

static FUNC(void, DCM_CODE) dsp_do_dcm_uds_req_file_read_dir(dcm_op_status_type op_status,
	const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, dcm_nrc_type *response_code)
{
	uint32 dir_info_length = 0u;
	uint32 max_number_of_block_length = 0u;
	uint16 file_path_and_name_length = ((uint16)pdu_rx_data->SduDataPtr[2] << 8) + pdu_rx_data->SduDataPtr[3];

	if (pdu_rx_data->SduLength != (4u + file_path_and_name_length)) {
		*response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		return;
	}

	Std_ReturnType ret = (DCM_INITIAL == op_status) ? dcm_process_request_read_dir(op_status, file_path_and_name_length,
		&pdu_rx_data->SduDataPtr[4], &dir_info_length, &max_number_of_block_length, response_code)
		: dcm_process_request_read_dir(op_status, 0, NULL, &dir_info_length, &max_number_of_block_length,
			response_code);

	if (E_OK == ret) {
		max_number_of_block_length = (uint32)MIN((max_number_of_block_length),
		((uint32)(pdu_tx_data->SduLength) - 2u));
		pdu_tx_data->SduDataPtr[1] = DCM_READ_DIR;
		pdu_tx_data->SduDataPtr[2] = 0x04;
		pdu_tx_data->SduDataPtr[3] = (uint8)((max_number_of_block_length >> 24) & 0xffu);
		pdu_tx_data->SduDataPtr[4] = (uint8)((max_number_of_block_length >> 16) & 0xffu);
		pdu_tx_data->SduDataPtr[5] = (uint8)((max_number_of_block_length >> 8) & 0xffu);
		pdu_tx_data->SduDataPtr[6] = (uint8)(max_number_of_block_length & 0xffu);
		pdu_tx_data->SduDataPtr[7] = 0x00;
		pdu_tx_data->SduDataPtr[8] = 0x00;
		pdu_tx_data->SduDataPtr[9] = 0x04;
		pdu_tx_data->SduDataPtr[10] = (uint8)((dir_info_length >> 24) & 0xffu);
		pdu_tx_data->SduDataPtr[11] = (uint8)((dir_info_length >> 16) & 0xffu);
		pdu_tx_data->SduDataPtr[12] = (uint8)((dir_info_length >> 8) & 0xffu);
		pdu_tx_data->SduDataPtr[13] = (uint8)(dir_info_length & 0xffu);
		pdu_tx_data->SduLength = 14;
		response_code = DCM_E_POSITIVERESPONSE;
		dsp_transfer_status.transfer_type = DCM_READFILE;
		dsp_transfer_status.file_transfer_type = DCM_READ_FILE;
		dsp_transfer_status.block_sequence_counter = 1;
		dsp_transfer_status.first_block_received = FALSE;
	} else if (DCM_E_PENDING == ret) {
		*response_code = DCM_E_RESPONSEPENDING;
	} else if (DCM_E_FORCE_RCRRP == ret) {
		*response_code = DCM_E_FORCE_RCRRP;
	} else if (E_NOT_OK == ret) {
		if (DCM_E_POSITIVERESPONSE == *response_code) {
			*response_code = DCM_E_REQUESTOUTOFRANGE;
		}
	} else {
		DCM_DET_REPORTERROR(DCM_DCM_GLOBAL_ID_DCM_E_UNEXPECTED_RESPONSE);
		*response_code = DCM_E_REQUESTOUTOFRANGE;
	}

	return;
}

FUNC(void, DCM_CODE) dsp_uds_req_file_transfer(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data)
{
	dcm_nrc_type response_code = DCM_E_POSITIVERESPONSE;
	dcm_op_status_type op_status = (dcm_op_status_type)DCM_INITIAL;

	dsp_set_op_status_depend_on_state(&dsp_upload_download_pending, &op_status, FALSE);
	if (pdu_rx_data->SduLength < 5u) {
		response_code = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	} else {
		uint8 mode_of_operation = pdu_rx_data->SduDataPtr[1];

		if (DCM_ADD_FILE == mode_of_operation) {
			dsp_do_dcm_uds_request_file_add_file(op_status, pdu_rx_data, pdu_tx_data, &response_code);
		} else if (DCM_DELETE_FILE == mode_of_operation) {
			dsp_do_dcm_uds_req_file_delete_file(op_status, pdu_rx_data, pdu_tx_data,
				&response_code);
		} else if (DCM_REPLACE_FILE == mode_of_operation) {
			dsp_do_dcm_uds_req_file_replace_file(op_status, pdu_rx_data, pdu_tx_data,
				&response_code);
		} else if (DCM_READ_DIR == mode_of_operation) {
			dsp_do_dcm_uds_req_file_read_dir(op_status, pdu_rx_data, pdu_tx_data, &response_code);
		} else if (DCM_READ_FILE == mode_of_operation) {
			dsp_do_dcm_uds_req_file_read_file(op_status, pdu_rx_data, pdu_tx_data, &response_code);
		} else {
			response_code = DCM_E_REQUESTOUTOFRANGE;
		}
	}

	switch (response_code) {
		case DCM_E_RESPONSEPENDING:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_PENDING,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_FILE_TRANSFER);
			break;
		case DCM_E_FORCE_RCRRP:
			dsp_uds_upload_download_pending_set(DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND,
			pdu_rx_data, pdu_tx_data, DCM_SID_REQUEST_FILE_TRANSFER);
			dsd_dsp_force_response_pending();
			break;
		default:
			dsp_upload_download_pending.state = DCM_GENERAL_IDLE;
			dsd_dsp_processing_done(response_code);
			break;
	}
}
#endif

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"
