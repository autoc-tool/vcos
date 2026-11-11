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

typedef struct {
	boolean inner_req;
	boolean stup_resp_req;
	dcm_protocol_addr_type_type address_type;
	PduIdType pdur_tx_pdu_id;
	PduIdType rx_pdu_id;
	const PduInfoType *pdu_rx_data;
	PduInfoType *pdu_tx_data;
	const dcm_dsl_protocol_row_type *protocol_row;
} msg_data_type;

#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
typedef uint8 service_pending_state_type;
#define DCM_SERVICE_IDLE (service_pending_state_type)0
#define DCM_SERVICE_PENDING (service_pending_state_type)1
#define DCM_SERVICE_WAIT_DONE (service_pending_state_type)2
#define DCM_SERVICE_DONE (service_pending_state_type)3
#define DCM_SERVICE_WAIT_TX_CONFIRM (service_pending_state_type)4

#define DCM_DSD_CURRENT_MESSAGE_CONTEXT_OFFSET_1   1u
#define DCM_DSD_CURRENT_MESSAGE_CONTEXT_OFFSET_2   2u

typedef struct {
	boolean sub_function_en;
	uint8 sub_fnc_id;
	dcm_nrc_type neg_resp_code;
	service_pending_state_type state;
} external_service_state_type;
#endif

#define DCM_START_SEC_PRIVATE_BSS
#include "dcm_memmap.h"
static VAR(msg_data_type, DCM_PRIVATE_BSS) dsd_pdu;
static VAR(uint8, DCM_PRIVATE_BSS) dsd_cur_sid;
static VAR(boolean, DCM_PRIVATE_BSS) dsd_supp_pos_rsp_msg;
#if ((DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON))
static VAR(uint16, DCM_PRIVATE_BSS) dsd_src_addr;
static VAR(dcm_protocol_addr_type_type, DCM_PRIVATE_BSS) dsd_req_type;
static VAR(uint16, DCM_PRIVATE_BSS) dsd_con_id;
static VAR(dcm_protocol_type, DCM_PRIVATE_BSS) dsd_protocol_id;
#endif

static VAR(boolean, DCM_PRIVATE_BSS) dsd_data_indication;
#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
static VAR(dcm_msg_context_type, DCM_PRIVATE_BSS) dsd_msg_context;
static VAR(external_service_state_type, DCM_PRIVATE_BSS) dsd_ex_service;
#endif
#define DCM_STOP_SEC_PRIVATE_BSS
#include "dcm_memmap.h"

#define IS_SUPRESSED_NRC_ON_FUNC_ADDR(_x)                                                                              \
	((DCM_E_SERVICENOTSUPPORTED == (_x)) || (DCM_E_SUBFUNCTIONNOTSUPPORTED == (_x)) ||                             \
	(DCM_E_REQUESTOUTOFRANGE == (_x)) || (DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION == (_x)) ||                \
	(DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION == (_x)))

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

static FUNC(void, DCM_CODE) dsd_create_and_send_ncr(dcm_nrc_type response_code)
{
	if ((dsd_pdu.address_type == DCM_PROTOCOL_FUNCTIONAL_ADDR_TYPE) && IS_SUPRESSED_NRC_ON_FUNC_ADDR(response_code)) {
		dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_SUPPRESSED);
	} else {
		if (dsd_pdu.pdu_tx_data != NULL_PTR) {
			dsd_pdu.pdu_tx_data->SduDataPtr[0] = DCM_SID_NEGATIVE_RESPONSE;
			dsd_pdu.pdu_tx_data->SduDataPtr[1] = dsd_cur_sid;
			dsd_pdu.pdu_tx_data->SduDataPtr[2] = response_code;
			dsd_pdu.pdu_tx_data->SduLength = 3;
		}
		dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_READY);
	}
}

#if ((DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON))
static FUNC(Std_ReturnType, DCM_CODE) dsd_service_req_notification_ind(void)
{
	Std_ReturnType result = E_OK;
	dcm_nrc_type error_code = DCM_E_POSITIVERESPONSE;

	if (E_OK == dsl_get_rx_connection_params(dsd_pdu.rx_pdu_id, &dsd_src_addr, &dsd_req_type,
		&dsd_protocol_id, &dsd_con_id)) {
		const dcm_dsl_service_request_notification_type *service_request_notification =
			dcm_config_ptr->dsl->dsl_service_request_notification;

		for (uint8 index = 0; service_request_notification[index].indication != NULL; index++) {
			result = service_request_notification[index].indication(dsd_cur_sid,
				&dsd_pdu.pdu_rx_data->SduDataPtr[1], dsd_pdu.pdu_rx_data->SduLength - 1u,
				dsd_req_type, dsd_con_id, &error_code, dsd_protocol_id,
				dsd_src_addr);
			if (result != E_OK) {
				if (DCM_E_REQUEST_NOT_ACCEPTED == result) {
					dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_SUPPRESSED);
				} else {
					dsd_create_and_send_ncr(error_code);
				}
				break;
			}
		}
	}
	return result;
}

static FUNC(void, DCM_CODE) dsd_service_req_notification_conf(NotifResultType result)
{
	dcm_conf_status_type status;

	if (dsd_pdu.pdu_tx_data->SduDataPtr[0] == DCM_SID_NEGATIVE_RESPONSE) {
		status = (result == NTFRSLT_OK) ? DCM_RES_NEG_OK : DCM_RES_NEG_NOT_OK;
	} else {
		status = (result == NTFRSLT_OK) ? DCM_RES_POS_OK : DCM_RES_POS_NOT_OK;
	}

	const dcm_dsl_service_request_notification_type *service_request_notification =
		dcm_config_ptr->dsl->dsl_service_request_notification;

	for (uint8 index = 0; service_request_notification[index].confirmation != NULL; index++) {
		(void)service_request_notification[index].confirmation(dsd_cur_sid, dsd_req_type, dsd_con_id,
			status, dsd_protocol_id, dsd_src_addr);
	}
}

#endif

#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
static FUNC(void, DCM_CODE) dsd_set_cur_msg_context(boolean is_sub_function)
{
	uint8 offset = 0u;

	if (TRUE == is_sub_function) {
		offset = DCM_DSD_CURRENT_MESSAGE_CONTEXT_OFFSET_2;
	} else {
		offset = DCM_DSD_CURRENT_MESSAGE_CONTEXT_OFFSET_1;
	}
	dsd_msg_context.dcm_rx_pdu_id = dsd_pdu.rx_pdu_id;
	dsd_msg_context.id_context = (uint8)dsd_pdu.rx_pdu_id;
	if (DCM_PROTOCOL_FUNCTIONAL_ADDR_TYPE == dsd_pdu.address_type) {
		dsd_msg_context.msg_add_info.req_type = TRUE;
	} else {
		dsd_msg_context.msg_add_info.req_type = FALSE;
	}
	dsd_msg_context.msg_add_info.suppress_pos_response = dsd_supp_pos_rsp_msg;
	dsd_msg_context.req_data = &dsd_pdu.pdu_rx_data->SduDataPtr[offset];
	dsd_msg_context.req_data_len = (dcm_msg_len_type)(dsd_pdu.pdu_rx_data->SduLength - offset);
	dsd_msg_context.res_data = &dsd_pdu.pdu_tx_data->SduDataPtr[offset];
	dsd_msg_context.res_data_len = 0;
	dsd_msg_context.res_max_data_len = (dcm_msg_len_type)(dsd_pdu.pdu_tx_data->SduLength - offset);
}
#endif

static FUNC(void, DCM_CODE) dsd_run_internal_service(uint8 SID)
{
	if (DCM_SID_DIAGNOSTIC_SESSION_CONTROL == SID) {
#if (DCM_USE_SERVICE_DIAGNOSTICSESSIONCONTROL == STD_ON)
		dsp_uds_diag_ses_ctrl(dsd_pdu.pdu_rx_data, dsd_pdu.pdur_tx_pdu_id,
			dsd_pdu.pdu_tx_data,
			dsd_pdu.protocol_row->dsl_send_resp_pend_on_trans_to_boot,
			dsd_pdu.inner_req);
#endif
	} else if (DCM_SID_ECU_RESET == SID) {
#if (DCM_USE_SERVICE_ECURESET == STD_ON)
		dsp_uds_ecu_reset(dsd_pdu.pdu_rx_data, dsd_pdu.pdur_tx_pdu_id, dsd_pdu.pdu_tx_data,
			dsd_pdu.stup_resp_req);
#endif
	} else if (DCM_SID_CLEAR_DIAGNOSTIC_INFORMATION == SID) {
#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION == STD_ON)
		dsp_uds_clear_diag_info(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data,
			dsd_pdu.protocol_row->dcm_dem_client_id);
#endif
	} else if (DCM_SID_READ_DTC_INFORMATION == SID) {
#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_READDTCINFORMATION == STD_ON)
		dsp_uds_read_dtc_info(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data,
			dsd_pdu.protocol_row->dcm_dem_client_id);
#endif
	} else if (DCM_SID_READ_DATA_BY_IDENTIFIER == SID) {
#if (DCM_USE_SERVICE_READDATABYIDENTIFIER == STD_ON)
		dsp_uds_read_data_by_id(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_READ_MEMORY_BY_ADDRESS == SID) {
#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON)
		dsp_uds_read_memory_by_addr(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_WRITE_MEMORY_BY_ADDRESS == SID) {
#if (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
		dsp_uds_write_memory_by_addr(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_SECURITY_ACCESS == SID) {
#if (DCM_USE_SERVICE_SECURITYACCESS == STD_ON)
		dsp_uds_security_access(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_WRITE_DATA_BY_IDENTIFIER == SID) {
#if (DCM_USE_SERVICE_WRITEDATABYIDENTIFIER == STD_ON)
		dsp_uds_write_data_by_id(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_ROUTINE_CONTROL == SID) {
#if (DCM_USE_SERVICE_ROUTINECONTROL == STD_ON)
		dsp_uds_routine_ctrl(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_TESTER_PRESENT == SID) {
#if (DCM_USE_SERVICE_TESTERPRESENT == STD_ON)
		dsp_uds_tester_present(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_CONTROL_DTC_SETTING == SID) {
#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
		dsp_uds_ctrl_dtc_set(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data,
			dsd_pdu.protocol_row->dcm_dem_client_id);
#endif
	} else if (DCM_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER == SID) {
#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON)
		dsp_io_ctrl_by_data_id(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_COMMUNICATION_CONTROL == SID) {
#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
		dsp_communication_control(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data, dsd_pdu.rx_pdu_id,
			dsd_pdu.pdur_tx_pdu_id);
#endif
	} else if (DCM_SID_REQUEST_DOWNLOAD == SID) {
#if (DCM_USE_SERVICE_REQUESTDOWNLOAD == STD_ON)
		dsp_uds_request_download(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_UPLOAD == SID) {
#if (DCM_USE_SERVICE_REQUESTUPLOAD == STD_ON)
	dsp_uds_request_upload(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_TRANSFER_DATA == SID) {
#if (DCM_USE_SERVICE_TRANSFERDATA == STD_ON)
		dsp_uds_transfer_data(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_TRANSFER_EXIT == SID) {
#if (DCM_USE_SERVICE_REQUESTTRANSFEREXIT == STD_ON)
		dsp_uds_request_transfer_exit(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_FILE_TRANSFER == SID) {
#if (DCM_USE_SERVICE_REQUESTFILETRANSFER == STD_ON)
		dsp_uds_req_file_transfer(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_CURRENT_POWERTRAIN_DIAGNOSTIC_DATA == SID) {
#if (DCM_USE_SERVICE_REQUESTCURRENTPOWERTRAINDIAGNOSTICDATA == STD_ON)
		dsp_obd_req_cur_powertrain_diag_data(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_POWERTRAIN_FREEZE_FRAME_DATA == SID) {
#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA == STD_ON)
		dsp_obd_req_powertrain_freeze_frame_data(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_CLEAR_EMISSION_RELATED_DIAGNOSTIC_INFORMATION == SID) {
#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CLEAREMISSIONRELATEDDIAGNOSTICDATA == STD_ON)
		dsp_obd_clear_emission_related_diag_data(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data,
			dsd_pdu.protocol_row->dcm_dem_client_id);
#endif
	} else if (DCM_SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES == SID) {
#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES == STD_ON)
		dsp_obd_req_emission_related_diag_trouble_codes(dsd_pdu.pdu_rx_data,
			dsd_pdu.pdu_tx_data, dsd_pdu.protocol_row->dcm_dem_client_id);
#endif
	} else if (DCM_SID_REQ_EMIS_DIAG_TROUBLE_CODES_DUR_CURR_OR_LAST_COMPL_DRIVING_CYCLE == SID) {
#if (CONFIG_MODULE_DEM == 1) && \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE == STD_ON)
		dsp_obd_req_emis_dtc_serv07(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data,
			dsd_pdu.protocol_row->dcm_dem_client_id);
#endif
	} else if (DCM_SID_REQUEST_VEHICLE_INFORMATION == SID) {
#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
		dsp_obd_req_vehicle_info(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data);
#endif
	} else if (DCM_SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES_WITH_PERMANENT_STATUS == SID) {
#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS == STD_ON)
		dsp_obd_req_emis_dtc_with_perm_st(dsd_pdu.pdu_rx_data, dsd_pdu.pdu_tx_data,
			dsd_pdu.protocol_row->dcm_dem_client_id);
#endif
	} else {
		dsd_create_and_send_ncr(DCM_E_SERVICENOTSUPPORTED);
	}
}

static FUNC(boolean, DCM_CODE) dsd_lookup_sub_service(const dcm_dsd_service_type *sid_conf_ptr, uint8 sub_service,
	const dcm_dsd_sub_service_type **sub_service_ptr, dcm_nrc_type *resp_code)
{
	boolean found = FALSE;

	*resp_code = DCM_E_POSITIVERESPONSE;
	if ((sid_conf_ptr->dsd_sid_sub_service_count <= 0u) || (NULL_PTR == sid_conf_ptr->dsd_sub_service_list)) {
		return FALSE;
	}

	for (uint8 i = 0; i < sid_conf_ptr->dsd_sid_sub_service_count; i++) {
		const dcm_dsd_sub_service_type *const dsd_sub_service_list = &sid_conf_ptr->dsd_sub_service_list[i];

		if (sub_service != dsd_sub_service_list->dsd_sub_service_id) {
			continue;
		}
		*sub_service_ptr = dsd_sub_service_list;

		if (!dsp_check_session_level(dsd_sub_service_list->dsd_sub_service_session_mask)) {
				*resp_code = DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION;
			} else {
				if (FALSE == dsp_check_security_level(dsd_sub_service_list->dsd_sub_service_security_mask)) {
						*resp_code = DCM_E_SECURITYACCESSDENIED;
				}
			}
		found = TRUE;
		break;
	}

	return found;
}

#if (DCM_SUB_SERVICE_USER_DEFINED == STD_ON)
static boolean dsd_handle_user_defined_service(const dcm_dsd_sub_service_type *sub_service_ptr) {
	dsd_set_cur_msg_context(TRUE);
	dsd_ex_service.neg_resp_code = DCM_E_POSITIVERESPONSE;
	dsd_ex_service.state = DCM_SERVICE_WAIT_DONE;
	dsd_ex_service.sub_function_en = TRUE;
	dsd_ex_service.sub_fnc_id = sub_service_ptr->dsd_sub_service_id;

	if (DCM_E_PENDING == sub_service_ptr->dsd_sub_service_fnc(DCM_INITIAL, &dsd_msg_context)) {
		dsd_ex_service.state = DCM_SERVICE_PENDING;
	}
	return FALSE;
}
#endif

static FUNC(void, DCM_CODE) dsd_select_service_function(const dcm_dsd_service_type *sid_conf_ptr) {
	if ((0u == sid_conf_ptr->dsd_sid_sub_service_count) || (NULL_PTR == sid_conf_ptr->dsd_sub_service_list)) {
		dsd_run_internal_service(sid_conf_ptr->dsd_sid_tab_service_id);
		return;
	}

	if (dsd_pdu.pdu_rx_data->SduLength <= 1u) {
		dsd_create_and_send_ncr(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
		return;
	}

	const dcm_dsd_sub_service_type *sub_service_ptr;
	dcm_nrc_type resp_code;
	boolean process_service = TRUE;

	if (!dsd_lookup_sub_service(sid_conf_ptr, dsd_pdu.pdu_rx_data->SduDataPtr[1], &sub_service_ptr, &resp_code)) {
		dsd_create_and_send_ncr(DCM_E_SUBFUNCTIONNOTSUPPORTED);
		process_service = FALSE;
		return;
	}
	if (DCM_E_POSITIVERESPONSE != resp_code) {
		dsd_create_and_send_ncr(resp_code);
		process_service = FALSE;
		return;
	}
	if (dcm_mode_mode_rule_cbk_by_idx(sub_service_ptr->sub_service_mode_rule_index, &resp_code) != E_OK) {
		dsd_create_and_send_ncr(resp_code);
		process_service = FALSE;
		return;
	}
#if (DCM_SUB_SERVICE_USER_DEFINED == STD_ON)
	if (NULL_PTR != sub_service_ptr->dsd_sub_service_fnc) {
		process_service = dsd_handle_user_defined_service(sub_service_ptr);
	}
#endif
	if (TRUE == process_service) {
		dsd_run_internal_service(sid_conf_ptr->dsd_sid_tab_service_id);
	}
}

static FUNC(boolean, DCM_CODE) dsd_lookup_sid(uint8 sid, const dcm_dsd_service_type **sid_ptr)
{
	boolean return_status = FALSE;

	if ((dsd_pdu.protocol_row == NULL_PTR) || (dsd_pdu.protocol_row->dsl_protocol_sid_table == NULL_PTR)) {
		return return_status;
	}

	const dcm_dsd_service_type *service = dsd_pdu.protocol_row->dsl_protocol_sid_table->dsd_service;

	for (uint8 i = 0; i < dsd_pdu.protocol_row->dsl_protocol_sid_table->nof_services; i++) {
		if (sid == service[i].dsd_sid_tab_service_id) {
			*sid_ptr = &service[i];
			return_status = TRUE;
			break;
		}
	}

	return return_status;
}

#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
static FUNC(Std_ReturnType, DCM_CODE) dsd_dsl_run_ex_services(dcm_op_status_type op_status)
{
	Std_ReturnType ret = E_NOT_OK;
	const dcm_dsd_service_type *sid_cfg_ptr = NULL_PTR;

	if (FALSE == dsd_lookup_sid(dsd_cur_sid, &sid_cfg_ptr)) {
		return E_NOT_OK;
	}

	if (FALSE == dsd_ex_service.sub_function_en) {
		if (NULL_PTR != sid_cfg_ptr->dsp_sid_tab_fnc) {
			ret = sid_cfg_ptr->dsp_sid_tab_fnc(op_status, NULL_PTR);
		}
	} else {
		const dcm_dsd_sub_service_type *sub_service_ptr;
		dcm_nrc_type resp;

		if ((TRUE == dsd_lookup_sub_service(sid_cfg_ptr, dsd_ex_service.sub_fnc_id, &sub_service_ptr,
			&resp)) && (NULL_PTR != sub_service_ptr->dsd_sub_service_fnc)) {
			ret = sub_service_ptr->dsd_sub_service_fnc(op_status, NULL_PTR);
		}
	}

	return ret;
}

static FUNC(void, DCM_CODE) dsd_ex_service_main_function(void)
{
	switch (dsd_ex_service.state) {
		case DCM_SERVICE_IDLE:
			return;
		case DCM_SERVICE_PENDING:
			if (DCM_E_PENDING != dsd_dsl_run_ex_services(DCM_PENDING)) {
				dsd_ex_service.state = DCM_SERVICE_WAIT_DONE;
			}
			break;
		case DCM_SERVICE_DONE:
			if (DCM_E_POSITIVERESPONSE != dsd_ex_service.neg_resp_code) {
				dsd_dsp_processing_done(dsd_ex_service.neg_resp_code);
			} else {
				dsd_pdu.pdu_tx_data->SduLength++;
				dsd_dsp_processing_done(DCM_E_POSITIVERESPONSE);
			}
			dsd_ex_service.state = DCM_SERVICE_WAIT_TX_CONFIRM;
			break;
		default:
			break;
	}
}

FUNC(void, DCM_CODE) dsd_cancel_pending_external_services(void)
{
	if (DCM_SERVICE_PENDING == dsd_ex_service.state) {
		(void)dsd_dsl_run_ex_services(DCM_CANCEL);
	}
	if (DCM_SERVICE_WAIT_DONE == dsd_ex_service.state) {
		(void)dsd_dsl_run_ex_services(DCM_CANCEL);
	}
	dsd_ex_service.state = DCM_SERVICE_IDLE;
}

FUNC(void, DCM_CODE)
dsd_external_set_neg_response(const dcm_msg_context_type *p_msg_context, dcm_nrc_type error_code)
{
	if (p_msg_context->dcm_rx_pdu_id != dsd_msg_context.dcm_rx_pdu_id) {
		return;
	}

	dsd_ex_service.neg_resp_code = error_code;
}

FUNC(void, DCM_CODE) dsd_external_processing_done(const dcm_msg_context_type *p_msg_context)
{
	if (p_msg_context->dcm_rx_pdu_id != dsd_msg_context.dcm_rx_pdu_id) {
		return;
	}

	dsd_ex_service.state = DCM_SERVICE_DONE;
	if (DCM_E_POSITIVERESPONSE != dsd_ex_service.neg_resp_code) {
		return;
	}

	if (TRUE == dsd_ex_service.sub_function_en) {
		(void)memcpy(&dsd_pdu.pdu_tx_data->SduDataPtr[2], p_msg_context->res_data,
			p_msg_context->res_data_len);
		dsd_pdu.pdu_tx_data->SduLength = (PduLengthType)p_msg_context->res_data_len;
		dsd_pdu.pdu_tx_data->SduDataPtr[1] = dsd_ex_service.sub_fnc_id;
		dsd_pdu.pdu_tx_data->SduLength++;
	} else {
		(void)memcpy(&dsd_pdu.pdu_tx_data->SduDataPtr[1], p_msg_context->res_data,
			p_msg_context->res_data_len);
		dsd_pdu.pdu_tx_data->SduLength = (PduLengthType)p_msg_context->res_data_len;
	}
}

static FUNC(void, DCM_CODE) dsd_confirmation(NotifResultType result) {
	if (DCM_SERVICE_WAIT_TX_CONFIRM != dsd_ex_service.state) {
		return;
	}

	dcm_confirmation_status_type dcm_status;

	if ((NotifResultType)NTFRSLT_OK == result) {
		dcm_status = (DCM_E_POSITIVERESPONSE == dsd_ex_service.neg_resp_code)
			? DCM_RES_POS_OK : DCM_RES_NEG_OK;
	} else {
		dcm_status = (DCM_E_POSITIVERESPONSE == dsd_ex_service.neg_resp_code)
			? DCM_RES_POS_NOT_OK : DCM_RES_NEG_NOT_OK;
	}
	dsd_ex_service.state = DCM_SERVICE_IDLE;
}
#endif

#if (DCM_SERVICE_USER_DEFINED == STD_ON)
static void dsd_handle_custom_service(const dcm_dsd_service_type *conf) {
	dsd_set_cur_msg_context(FALSE);
	dsd_ex_service.neg_resp_code = DCM_E_POSITIVERESPONSE;
	dsd_ex_service.sub_function_en = FALSE;
	dsd_ex_service.state = (DCM_E_PENDING == conf->dsp_sid_tab_fnc(DCM_INITIAL, &dsd_msg_context))
		? DCM_SERVICE_PENDING : DCM_SERVICE_WAIT_DONE;
}
#endif

static FUNC(void, DCM_CODE) dsd_handle_request(void) {
	if (0u == dsd_pdu.pdu_rx_data->SduLength) {
		dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_SUPPRESSED);
		return;
	}

	dsd_cur_sid = dsd_pdu.pdu_rx_data->SduDataPtr[0];
#if ((DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON))
	if (dsd_service_req_notification_ind() != E_OK) {
		return;
	}
#endif

	if (((dsd_cur_sid & 0x7fu) >= 0x40u) && (DCM_RESPOND_ALL_REQUEST != STD_ON)) {
		dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_SUPPRESSED);
		return;
	}

	const dcm_dsd_service_type *sid_conf_ptr = NULL_PTR;

	if (!dsd_lookup_sid(dsd_cur_sid, &sid_conf_ptr) || (NULL == sid_conf_ptr)) {
		dsd_create_and_send_ncr(DCM_E_SERVICENOTSUPPORTED);
		return;
	}

	boolean skip_checks =
#if (DCM_SERVICE_USER_DEFINED == STD_ON)
		dsd_pdu.stup_resp_req && (NULL_PTR == sid_conf_ptr->dsp_sid_tab_fnc);
#else
		dsd_pdu.stup_resp_req;
#endif

	if (!skip_checks) {
		if (!dsp_check_session_level(sid_conf_ptr->dsd_sid_tab_session_mask)) {
			dsd_create_and_send_ncr(DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION);
			return;
		}
		if (!dsp_check_security_level(sid_conf_ptr->dsd_sid_tab_security_mask)) {
			dsd_create_and_send_ncr(DCM_E_SECURITYACCESSDENIED);
			return;
		}
	}

	dsd_supp_pos_rsp_msg =
		((sid_conf_ptr->dsd_sid_sub_service_count > 0u) &&
		(SUPPRESS_POS_RESP_BIT == (dsd_pdu.pdu_rx_data->SduDataPtr[1] & SUPPRESS_POS_RESP_BIT)));
	if (dsd_supp_pos_rsp_msg) {
		dsd_pdu.pdu_rx_data->SduDataPtr[1] &= ~SUPPRESS_POS_RESP_BIT;
	}

	dcm_nrc_type error_code = DCM_E_POSITIVERESPONSE;

	if (dcm_mode_mode_rule_cbk_by_idx(sid_conf_ptr->dsd_sid_tab_mode_rule_index, &error_code) != E_OK) {
		dsd_create_and_send_ncr(error_code);
		return;
	}

#if (DCM_SERVICE_USER_DEFINED == STD_ON)
	if (sid_conf_ptr->dsp_sid_tab_fnc != NULL_PTR) {
		dsd_handle_custom_service(sid_conf_ptr);
		return;
	}
#endif

	dsd_select_service_function(sid_conf_ptr);
}

FUNC(void, DCM_CODE) dsd_main_function(void)
{
	if (TRUE == dsd_data_indication) {
		dsd_data_indication = FALSE;
		dsd_handle_request();
	}
#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
	dsd_ex_service_main_function();
#endif
}

FUNC(void, DCM_CODE) dsd_dsp_processing_done_with_no_response(void)
{
	dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_SUPPRESSED);
}

FUNC(void, DCM_CODE) dsd_dsp_processing_done(dcm_nrc_type response_code)
{
	if (response_code != DCM_E_POSITIVERESPONSE) {
		dsd_create_and_send_ncr(response_code);
		return;
	}

	if (dsd_supp_pos_rsp_msg == FALSE) {
		if (dsd_pdu.pdu_tx_data != NULL_PTR) {
			dsd_pdu.pdu_tx_data->SduDataPtr[0] = dsd_cur_sid | DCM_SID_RESPONSE_BIT;
		}
		dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_READY);
		return;
	}

	dsp_dcm_confirmation(dsd_pdu.pdur_tx_pdu_id, E_OK);
	dsl_dsd_processing_done(dsd_pdu.rx_pdu_id, DCM_DSD_TX_RESPONSE_SUPPRESSED);
#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
	dsd_confirmation(NTFRSLT_OK);
#endif
}

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))

FUNC(uint16, DCM_CODE) dsd_dsp_get_tester_source_address(void)
{
	uint16 tester_src_addr;
	dcm_protocol_addr_type_type req_type;
	dcm_protocol_type dsd_protocol_id;
	uint16 dsd_con_id;

	if (E_OK == dsl_get_rx_connection_params(dsd_pdu.rx_pdu_id, &tester_src_addr, &req_type, &dsd_protocol_id,
		&dsd_con_id)) {
			return tester_src_addr;
		}
	tester_src_addr = 0xffff;
	return tester_src_addr;
}

FUNC(boolean, DCM_CODE) dsd_dsp_get_response_required(void)
{
	if (FALSE == dsd_supp_pos_rsp_msg) {
		return TRUE;
	} else {
		return FALSE;
	}
}
#endif

FUNC(void, DCM_CODE) dsd_dsp_force_response_pending(void)
{
	dsl_dsd_send_resp_pending(dsd_pdu.rx_pdu_id);
}

FUNC(void, DCM_CODE) dsd_data_confirmation(PduIdType confirm_pdu_id, Std_ReturnType result)
{
	dsp_dcm_confirmation(confirm_pdu_id, result);

#if ((DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON))
	dsd_service_req_notification_conf(result);
#endif
#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
	dsd_confirmation(result);
#endif
}

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))

FUNC(void, DCM_CODE) dsd_response_pending_confirmed(PduIdType confirm_pdu_id, NotifResultType result)
{
	if (NTFRSLT_OK != result) {
		return;
	}

	dsp_response_pending_confirmed(confirm_pdu_id);
}
#endif

FUNC(void, DCM_CODE)
dsd_dsl_data_indication(const PduInfoType *pdu_rx_data, const dcm_dsl_protocol_row_type *protocol_row,
	dcm_protocol_addr_type_type address_type, PduIdType tx_pdu_id, PduInfoType *pdu_tx_data,
	PduIdType rx_context_pdu_id, boolean inner_req, boolean stup_resp_req)
{
	dsd_pdu.protocol_row = protocol_row;
	dsd_pdu.pdur_tx_pdu_id = tx_pdu_id;
	dsd_pdu.rx_pdu_id = rx_context_pdu_id;
	dsd_pdu.pdu_tx_data = pdu_tx_data;
	dsd_pdu.pdu_rx_data = pdu_rx_data;
	dsd_pdu.inner_req = inner_req;
	dsd_pdu.stup_resp_req = stup_resp_req;
	dsd_pdu.address_type = address_type;
	dsd_data_indication = TRUE;
}

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON) || defined(DCM_USE_CONTROL_DIDS) ||                               \
	((CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON))
FUNC(boolean, DCM_CODE) dsd_dsp_check_service_supported_in_active_session_and_security(uint8 sid)
{
	boolean ret = FALSE;
	const dcm_dsd_service_type *sid_conf_ptr = NULL;

	if (dsd_lookup_sid(sid, &sid_conf_ptr) && dsp_check_session_level(sid_conf_ptr->dsd_sid_tab_session_mask) &&
		dsp_check_security_level(sid_conf_ptr->dsd_sid_tab_security_mask)) {
		ret = TRUE;
	}

	return ret;
}
#endif

FUNC(void, DCM_CODE) dsd_clear_suppress_pos_rsp_msg(void)
{
	dsd_supp_pos_rsp_msg = FALSE;
}

FUNC(PduIdType, DCM_CODE) dsd_dsl_get_current_tx_pdu_id(void)
{
	return dsd_pdu.pdur_tx_pdu_id;
}

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"
