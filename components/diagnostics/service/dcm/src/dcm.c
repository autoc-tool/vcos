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
#if (CONFIG_MODULE_DEM == 1)
#if (DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION == STD_ON) || (DCM_USE_SERVICE_READDTCINFORMATION == STD_ON) ||        \
	(DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
#include "dem.h"
#endif
#endif
#if (CONFIG_MODULE_PDUR == 1)
#include "pdur_dcm.h"
#endif

#define DCM_UNINITIALIZED ((uint8)0u)
#define DCM_INITIALIZED ((uint8)1u)

#define DCM_START_SEC_MULTI_APP_BSS
#include "dcm_memmap.h"
static VAR(uint8, DCM_MULTI_APP_BSS) dcm_state;
P2CONST(dcm_config_type, DCM_MULTI_APP_BSS, dcm_var_class) dcm_config_ptr;
#define DCM_STOP_SEC_MULTI_APP_BSS
#include "dcm_memmap.h"

#define DCM_START_SEC_PRIVATE_BSS
#include "dcm_memmap.h"
static VAR(boolean, DCM_PRIVATE_BSS) dcm_first_main_after_init;
#define DCM_STOP_SEC_PRIVATE_BSS
#include "dcm_memmap.h"

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

FUNC(void, DCM_CODE) dcm_init(const dcm_config_type *config_ptr)
{
	if (NULL == config_ptr) {
		DCM_DET_REPORTERROR(DCM_DCM_INIT_ID_DCM_E_CONFIG_INVALID);
		return;
	}

	dcm_config_ptr = config_ptr;
	dsl_init();
	dsp_init(TRUE);
	dcm_first_main_after_init = TRUE;
	dcm_state = DCM_INITIALIZED;
	return;
}

FUNC(void, DCM_CODE) Dcm_MainFunction(void)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_MAIN_ID_DCM_E_UNINIT);
		return;
	}

	if (dcm_first_main_after_init) {
		dsp_check_protocol_start_requests();
		dcm_first_main_after_init = FALSE;
	}

	dsp_pre_dsd_main_function();
	dsd_main_function();
	dsp_main_function();
	dsp_timer_main_function();
	dsl_main_function();
}

FUNC(BufReq_ReturnType, DCM_CODE)
dcm_start_of_reception(PduIdType dcm_rx_pdu_id, const PduInfoType *info, PduLengthType tp_sdu_length,
	PduLengthType *rx_buffer_size_ptr)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_START_OF_RECEPTION_ID_DCM_E_UNINIT);
		return BUFREQ_E_NOT_OK;
	}

	if (dcm_rx_pdu_id >= DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		DCM_DET_REPORTERROR(DCM_DCM_START_OF_RECEPTION_ID_DCM_E_PARAM);
		return BUFREQ_E_NOT_OK;
	}
	BufReq_ReturnType return_code;
	(void)info;

	return_code = dsl_start_of_reception(dcm_rx_pdu_id, tp_sdu_length, rx_buffer_size_ptr, FALSE);

	return return_code;
}

FUNC(BufReq_ReturnType, DCM_CODE)
dcm_copy_rx_data(PduIdType dcm_rx_pdu_id, const PduInfoType *pdu_info_ptr, PduLengthType *rx_buffer_size_ptr)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_COPY_RX_DATA_ID_DCM_E_UNINIT);
		return BUFREQ_E_NOT_OK;
	}

	if (dcm_rx_pdu_id >= DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		DCM_DET_REPORTERROR(DCM_DCM_COPY_RX_DATA_ID_DCM_E_PARAM);
		return BUFREQ_E_NOT_OK;
	}

	return dsl_copy_data_to_rx_buffer(dcm_rx_pdu_id, pdu_info_ptr, rx_buffer_size_ptr);
}

FUNC(void, DCM_CODE) dcm_tp_rx_indication(PduIdType dcm_rx_pdu_id, NotifResultType result)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_TP_RX_INDICATION_ID_DCM_E_UNINIT);
		return;
	}

	if (dcm_rx_pdu_id >= DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		DCM_DET_REPORTERROR(DCM_DCM_TP_RX_INDICATION_ID_DCM_E_PARAM);
		return;
	}

	dsl_tp_rx_indication_from_pdur(dcm_rx_pdu_id, result, FALSE, FALSE);
}

FUNC(void, DCM_CODE) dcm_tp_tx_confirmation(PduIdType dcm_tx_pdu_id, NotifResultType result)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_TP_TX_CONFIRMATION_ID_DCM_E_UNINIT);
		return;
	}

	if (dcm_tx_pdu_id >= DCM_DSL_TX_PDU_ID_LIST_LENGTH) {
		DCM_DET_REPORTERROR(DCM_DCM_TP_TX_CONFIRMATION_ID_DCM_E_PARAM);
		return;
	}

	dsl_tp_tx_confirmation(dcm_tx_pdu_id, (Std_ReturnType)result);
}

FUNC(BufReq_ReturnType, DCM_CODE)
dcm_copy_tx_data(PduIdType dcm_tx_pdu_id, const PduInfoType *pdu_info_ptr, const RetryInfoType *retry,
		 PduLengthType *tx_data_cnt_ptr)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_COPY_TX_DATA_ID_DCM_E_UNINIT);
		return BUFREQ_E_NOT_OK;
	}

	if (dcm_tx_pdu_id >= DCM_DSL_TX_PDU_ID_LIST_LENGTH) {
		DCM_DET_REPORTERROR(DCM_DCM_COPY_TX_DATA_ID_DCM_E_PARAM);
		return BUFREQ_E_NOT_OK;
	}

	return dsl_copy_tx_data(dcm_tx_pdu_id, pdu_info_ptr, retry, tx_data_cnt_ptr);
}

FUNC(void, DCM_CODE) dcm_comm_full_com_mode_entered(uint8 network_id)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_COMM_FULL_COM_MODE_ENTERED_ID_DCM_E_UNINIT);
		return;
	}

#if (CONFIG_MODULE_COMM != 1)
	(void)network_id;
#else
	dsl_com_mode_entered(network_id, DCM_FULL_COM);
#endif
}

FUNC(void, DCM_CODE) dcm_comm_silent_com_mode_entered(uint8 network_id)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_COMM_SILENT_COM_MODE_ENTERED_ID_DCM_E_UNINIT);
		return;
	}

#if (CONFIG_MODULE_COMM != 1)
	(void)network_id;
#else
	dsl_com_mode_entered(network_id, DCM_SILENT_COM);
#endif
}

FUNC(void, DCM_CODE) dcm_comm_no_com_mode_entered(uint8 network_id)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_COMM_NO_COM_MODE_ENTERED_ID_DCM_E_UNINIT);
		return;
	}

#if (CONFIG_MODULE_COMM != 1)
	(void)network_id;
#else
	dsl_com_mode_entered(network_id, DCM_NO_COM);
#endif
}

#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
FUNC(void, DCM_CODE)
dcm_external_set_neg_response(const dcm_msg_context_type *p_msg_context, dcm_nrc_type error_code)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_EXTERNALSETNEGRESPONSE_ID_DCM_E_UNINIT);
		return;
	}

	if (NULL == p_msg_context) {
		DCM_DET_REPORTERROR(DCM_DCM_EXTERNALSETNEGRESPONSE_ID_DCM_E_PARAM);
		return;
	}
	dsd_external_set_neg_response(p_msg_context, error_code);
}

FUNC(void, DCM_CODE) dcm_external_processing_done(const dcm_msg_context_type *p_msg_context)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_EXTERNALPROCESSINGDONE_ID_DCM_E_UNINIT);
		return;
	}

	if (NULL == p_msg_context) {
		DCM_DET_REPORTERROR(DCM_DCM_EXTERNALPROCESSINGDONE_ID_DCM_E_PARAM);
		return;
	}
	dsd_external_processing_done(p_msg_context);
}
#endif

FUNC(Std_ReturnType, DCM_CODE) dcm_get_sec_lev(dcm_sec_lv_type *sec_level)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_GET_SECURITY_LEVEL_ID_DCM_E_UNINIT);
		return E_NOT_OK;
	}

	if (E_OK == dsl_get_security_level(sec_level)) {
		return E_OK;
	}
	*sec_level = DCM_SEC_LEV_LOCKED;
	return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) dcm_get_ses_ctrl_type(dcm_ses_ctrl_type *ses_ctrl_type)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_GET_SES_CTRL_TYPE_ID_DCM_E_UNINIT);
		return E_NOT_OK;
	}

	if (E_OK == dsl_get_ses_ctrl_type(ses_ctrl_type)) {
		return E_OK;
	}
	*ses_ctrl_type = DCM_DEFAULT_SESSION;
	return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) dcm_reset_to_default_session(void)
{
	if (DCM_INITIALIZED != dcm_state) {
		DCM_DET_REPORTERROR(DCM_DCM_RESETTODEFAULTSESSION_ID_DCM_E_UNINIT);
		return E_NOT_OK;
	}

	dsl_set_ses_ctrl_type(DCM_DEFAULT_SESSION);
	return E_OK;
}

#if (DCM_VSG_CHECK_ENABLED == STD_ON)
FUNC(Std_ReturnType, DCM_CODE) dcm_vsg_set_single(dcm_vsg_id_type vsg_id, dcm_vsg_state_type state)
{
	if ((DCM_INITIALIZED != dcm_state) || (vsg_id == 0u)) {
		return DCM_E_NOT_OK;
	}

	if ((state != DCM_VSG_ENABLED) && (state != DCM_VSG_DISABLED)) {
		return DCM_E_NOT_OK;
	}

	uint8 vsg_index = dcm_internal_get_vsg_index_by_vsg_id_map(vsg_id);

	if (vsg_index >= DCM_NOF_VEHICLE_SYSTEM_GROUPS) {
		return DCM_E_NOT_OK;
	}

	dcm_vsg_state_type *vsg_enable_state = dcm_config_ptr->dsp->dsp_vehicle_system_group->vsg_enable_state;

	vsg_enable_state[vsg_index] = state;

	return DCM_E_OK;
}
#endif

FUNC(void, DCM_CODE) dcm_de_init(uint8 value)
{
	dcm_state = value;
}

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"
