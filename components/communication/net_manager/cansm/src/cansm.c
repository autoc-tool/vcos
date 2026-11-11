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

#include "comstack_types.h"
#include "cansm.h"
#include "cansm_internal.h"
#include "cansm_bswm.h"
#include "cansm_canif.h"
#include "cansm_cfg.h"
#include "cansm_types.h"
#include "SchM_CanSM.h"
#if (CONFIG_MODULE_CANIF == 1)
#include "canif.h"
#endif
#include "Compiler.h"
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
#include "bswm_bsw.h"
#include "bswm.h"
#endif
#if (CONFIG_MODULE_COMM == 1)
#include "comm.h"
#endif
#if (CONFIG_MODULE_CANNM == 1)
#include "cannm.h"
#endif
#if (CONFIG_MODULE_DEM == 1)
#include "dem.h"
#endif
#include "cansm_vcos_log.h"
#include "vcos_log.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
static FUNC_P2VAR(cansm_internal_network_type, AUTOMATIC, CANSM_CODE) cansm_canIf_trcv_chnl_to_cansn_nw_hdl(uint8
	transceiver_id);
static FUNC_P2VAR(cansm_internal_network_type, AUTOMATIC, CANSM_CODE) cansm_canif_ctr_chnl_to_cansm_nw_hdl(uint8
	controller_id);
static FUNC(void, CANSM_CODE) cansm_internal_execute_pre_no_com(cansm_internal_network_type *network_sm_ptr);
static FUNC(void, CANSM_CODE) cansm_internal_execute_pre_full_com(cansm_internal_network_type *network_sm_ptr);
static FUNC(void, CANSM_CODE) cansm_network_state_machine_execute(cansm_internal_network_type *network_sm_ptr,
	uint8 exe_type);
static FUNC(void, CANSM_CODE) cansm_internal_enter_state_pre_no_com(cansm_internal_network_type *network_sm_ptr);
static FUNC(void, CANSM_CODE) cansm_internal_enter_state(cansm_internal_network_type *network_sm_ptr,
	cansm_internal_bsm_state_type state);
static FUNC(void, CANSM_CODE) cansm_internal_set_network_pdu_mode(const cansm_internal_network_type *network_sm_ptr,
	canif_pdu_mode_type pdu_mode);
static FUNC(void, CANSM_CODE) cansm_internal_enter_state_pre_full_com(cansm_internal_network_type *network_sm_ptr);
static FUNC(void, CANSM_CODE) cansm_internal_enter_state_pre_no_com(cansm_internal_network_type *network_sm_ptr)
{
	if (network_sm_ptr->cfg_info_ptr->cantrcv_en_pn == (boolean)TRUE) {
		network_sm_ptr->pre_no_comm_state = CANSM_PRENOCOMM_S_PN_CLEAR_WUF;
	} else {
		network_sm_ptr->pre_no_comm_state = CANSM_PRENOCOMM_S_CC_STOPPED;
	}
	network_sm_ptr->transceiver_mode_buf.ind_trcv_mode = CANTRCV_TRCVMODE_NORMAL;
	network_sm_ptr->controller_mode_buf.ind_ctrl_mode = (cansm_canif_ctrl_mode)CANIF_CS_UNINIT;
	cansm_internal_execute_pre_no_com(network_sm_ptr);
}

static FUNC(void, CANSM_CODE) cansm_internal_enter_state_pre_full_com(cansm_internal_network_type *network_sm_ptr)
{
	if (network_sm_ptr->cfg_info_ptr->can_en_trcv == (boolean)TRUE) {
		network_sm_ptr->pre_full_comm_state = CANSM_PREFULLCOMM_S_TRCV_NORMAL;
	} else {
		network_sm_ptr->pre_full_comm_state = CANSM_PREFULLCOMM_S_CC_STOPPED;
	}
	network_sm_ptr->transceiver_mode_buf.ind_trcv_mode = CANTRCV_TRCVMODE_NORMAL;
	network_sm_ptr->controller_mode_buf.ind_ctrl_mode = (cansm_canif_ctrl_mode)CANIF_CS_UNINIT;
	cansm_internal_execute_pre_full_com(network_sm_ptr);
}

static FUNC(void, CANSM_CODE) cansm_internal_enter_state_wu_validation(cansm_internal_network_type *network_sm_ptr)
{
	if (network_sm_ptr->cfg_info_ptr->can_en_trcv == (boolean)TRUE) {
		network_sm_ptr->wu_validation_state = CANSM_WUVALIDATION_S_TRCV_NORMAL;
	} else {
		network_sm_ptr->wu_validation_state = CANSM_WUVALIDATION_S_CC_STOPPED;
	}
	network_sm_ptr->transceiver_mode_buf.ind_trcv_mode = CANTRCV_TRCVMODE_NORMAL;
	network_sm_ptr->controller_mode_buf.ind_ctrl_mode = (cansm_canif_ctrl_mode)CANIF_CS_UNINIT;
}

static FUNC(void, CANSM_CODE) cansm_internal_enter_state(cansm_internal_network_type *network_sm_ptr,
	cansm_internal_bsm_state_type state)
{
	network_sm_ptr->repeat_counter = 0;
	network_sm_ptr->sub_state_timer = 0;
	network_sm_ptr->bsm_state = state;

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_INFO, CANSM_DLT_CHECK_ID_00,
	network_sm_ptr->cfg_info_ptr->id_net, state);
	switch (state) {
	case CANSM_BSM_S_PRE_NOCOM:
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
		bswm_cansm_current_state(network_sm_ptr->cfg_info_ptr->id_net, CANSM_BSWM_NO_COMMUNICATION);
#endif
		cansm_internal_enter_state_pre_no_com(network_sm_ptr);
		break;
	case CANSM_BSM_S_NOCOM:
		break;
	case CANSM_BSM_S_PRE_FULLCOM:
		cansm_internal_enter_state_pre_full_com(network_sm_ptr);
		break;
	case CANSM_BSM_S_WUVALIDATION:
		cansm_internal_enter_state_wu_validation(network_sm_ptr);
		break;
	case CANSM_BSM_S_FULLCOM:
		network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_BUSOFF_CHECK;
		break;
	case CANSM_BSM_S_NOT_INITIALIZED:
	case CANSM_BSM_S_SILENTCOM:
	case CANSM_BSM_S_SILENTCOM_BOR:
		break;
	default:
		/* nothing to do */
		break;
	}
}

FUNC(void, CANSM_CODE) cansm_init(const cansm_config_type *config_ptr)
{
	if (NULL == config_ptr) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_INIT_CANSM_E_PARAM_POINTER);
		return;
	}
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		return;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();
	const cansm_global_network_core_table_type *global_network_core_table = cansm_get_global_network_core_table();

	if (network_core_table[core_id].network_table_addr == NULL) {
		return;
	}
	cansm_set_config_ptr(config_ptr);
	cansm_set_module_status(CANSM_STATUS_INIT);
	const cansm_network_core_table_type *network_state_machine_this_core = &network_core_table[core_id];
	const cansm_global_network_core_table_type *global_network_state_machine_this_core =
	&global_network_core_table[core_id];
	const cansm_config_type *cansm_config_ptr = cansm_get_config_ptr();

	for (network_handle_type i = 0; i < network_core_table[core_id].network_table_len; ++i) {
		cansm_internal_network_type *network_sm_ptr = &network_state_machine_this_core->network_table_addr[i];
		cansm_external_network_type *global_network_sm_ptr =
		&global_network_state_machine_this_core->network_table_addr[i];
		(void)memset(global_network_sm_ptr, 0x00u, sizeof(cansm_external_network_type));
		(void)memset(network_sm_ptr, 0x00u, sizeof(cansm_internal_network_type));
		network_sm_ptr->cfg_info_ptr = &cansm_config_ptr->networks[core_id].net_cfg_tbl_add[i];
		network_sm_ptr->bsm_state = CANSM_BSM_S_PRE_NOCOM;
		network_sm_ptr->controller_mode_buf.controller_id = network_sm_ptr->cfg_info_ptr->canif_id_ctrl;
		network_sm_ptr->transceiver_mode_buf.trcv_id = network_sm_ptr->cfg_info_ptr->canif_id_trcv;
		network_sm_ptr->pre_full_comm_state = CANSM_PREFULLCOMM_S_CC_STOPPED;
		network_sm_ptr->wu_validation_state = CANSM_WUVALIDATION_S_CC_STOPPED;
		if (network_sm_ptr->cfg_info_ptr->cantrcv_en_pn == (boolean)TRUE) {
			network_sm_ptr->pre_no_comm_state = CANSM_PRENOCOMM_S_PN_CLEAR_WUF;
		}

#if ((CONFIG_MODULE_COMM == 1) && defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1))
		comm_init_status_type comm_status = COMM_UNINIT;
		bswm_init_status_t bswm_status = BSWM_UNINIT;
		(void)comm_get_status(&comm_status);
		(void)bswm_get_status(&bswm_status);
		if ((COMM_INIT == comm_status) && (BSWM_INIT == bswm_status)) {
			cansm_network_state_machine_execute(network_sm_ptr, CANSM_NETWORK_EXECUTE_INIT);
		}
#endif
	}
	return;
}

FUNC(void, CANSM_CODE) cansm_controller_bus_off(uint8 controller_id)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CONTROLLERBUSOFF_CANSM_E_UNINIT);
		return;
	}

	Std_ReturnType result = E_NOT_OK;
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		return;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();
	cansm_internal_network_type *network_table_addr = network_core_table[core_id].network_table_addr;

	if (network_table_addr == NULL) {
		return;
	}
	for (network_handle_type network_idx = 0u; network_idx < network_core_table[core_id].network_table_len;
	network_idx++) {
		if (controller_id == network_table_addr[network_idx].controller_mode_buf.controller_id) {
			if ((CANSM_BSM_S_FULLCOM == network_table_addr[network_idx].bsm_state)
#if (CANSM_SILENT_BOR_SUPPORT == STD_ON)
			|| (CANSM_BSM_S_SILENTCOM == network_table_addr[network_idx].bsm_state)
#endif
			) {
				network_table_addr[network_idx].bus_off_event = (boolean)TRUE;
			} else {
				CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CONTROLLERBUSOFF_CANSM_E_INVALID_BUSOFF);
			}
		}
		result = E_OK;
	}
	if (E_OK != result) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CONTROLLERBUSOFF_CANSM_E_PARAM_CONTROLLER);
	}
}

FUNC(Std_ReturnType, CANSM_CODE) cansm_request_com_mode(network_handle_type network_handle, comm_mode_type comm_mode)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_REQUESTCOMMODE_CANSM_E_UNINIT);
		return E_NOT_OK;
	}

	if ((COMM_NO_COMMUNICATION != comm_mode) && (COMM_SILENT_COMMUNICATION != comm_mode) &&
	(COMM_FULL_COMMUNICATION != comm_mode)) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_REQUESTCOMMODE_CANSM_E_INVALID_NETWORK_MODE);
		return E_NOT_OK;
	}

	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	if (network_sm_global_ptr == NULL) {
		return E_NOT_OK;
	}
	network_sm_global_ptr->req_mode = comm_mode;
	return E_OK;
}

FUNC(Std_ReturnType, CANSM_CODE) cansm_get_cur_com_mode(network_handle_type network_handle,
	comm_mode_type *comm_mode_ptr)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_GETCURRENTCOMMODE_CANSM_E_UNINIT);
		return E_NOT_OK;
	}
	if (NULL == comm_mode_ptr) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_GETCURRENTCOMMODE_CANSM_E_PARAM_POINTER);
		return E_NOT_OK;
	}
	const cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	if (network_sm_global_ptr == NULL) {
		return E_NOT_OK;
	}
	*comm_mode_ptr = network_sm_global_ptr->cur_com_mode;
	return E_OK;
}

FUNC(uint8, CANSM_CODE) cansm_get_network_count(void)
{
	uint8 network_count = 0;

	if (cansm_get_module_status() != CANSM_STATUS_UNINIT) {
		network_count = CANSM_NETWORK_COUNT;
	}
	return network_count;
}

FUNC_P2VAR(cansm_internal_network_type, AUTOMATIC, CANSM_CODE) cansm_get_cansm_network_index(network_handle_type
	network_handle)
{
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();
	const cansm_config_type *config_ptr = cansm_get_config_ptr();

	for (uint8 core_id = 0u; core_id < CANSM_MULTICORE_CORE_NUMBER; core_id++) {
		if (config_ptr->networks[core_id].net_cfg_tbl_add == NULL) {
			continue;
		}
		for (uint8 networkIndex = 0u;
			networkIndex < config_ptr->networks[core_id].network_cfg_tbl_len; networkIndex++) {
			if (config_ptr->networks[core_id].net_cfg_tbl_add[networkIndex].id_net == network_handle) {
				return &network_core_table[core_id].network_table_addr[networkIndex];
			}
		}
	}
	return NULL;
}

FUNC_P2VAR(cansm_external_network_type, AUTOMATIC, CANSM_CODE) cansm_get_cansm_global_network_index(network_handle_type
	network_handle)
{
	const cansm_global_network_core_table_type *network_core_table = cansm_get_global_network_core_table();
	const cansm_config_type *config_ptr = cansm_get_config_ptr();

	for (uint8 core_id = 0u; core_id < CANSM_MULTICORE_CORE_NUMBER; core_id++) {
		if (config_ptr->networks[core_id].net_cfg_tbl_add == NULL) {
			continue;
		}
		for (uint8 networkIndex = 0u;
			networkIndex < config_ptr->networks[core_id].network_cfg_tbl_len; networkIndex++) {
			if (config_ptr->networks[core_id].net_cfg_tbl_add[networkIndex].id_net == network_handle) {
				return &network_core_table[core_id].network_table_addr[networkIndex];
			}
		}
	}
	return NULL;
}

FUNC(Std_ReturnType, CANSM_CODE) cansm_tx_timeout_exception(network_handle_type network_handle)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_TXTIMEOUTEXCEPTION_CANSM_E_UNINIT);
		return E_NOT_OK;
	}

	cansm_internal_network_type *network_sm_ptr = cansm_get_cansm_network_index(network_handle);

	if (network_sm_ptr == NULL) {
		return E_NOT_OK;
	}
	if ((network_sm_ptr->bsm_state == CANSM_BSM_S_FULLCOM) &&
	(network_sm_ptr->full_comm_state == CANSM_FULLCOMM_S_NO_BUSOFF)) {
		network_sm_ptr->tx_timeout_flag = (boolean)TRUE;
	}
	return E_OK;
}

static FUNC(void, CANSM_CODE) cansm_internal_set_network_pdu_mode(const cansm_internal_network_type *network_sm_ptr,
	canif_pdu_mode_type pdu_mode)
{
	uint8 controller_id = network_sm_ptr->controller_mode_buf.controller_id;

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_INFO, CANSM_DLT_CHECK_ID_13,
	network_sm_ptr->cfg_info_ptr->id_net, controller_id, pdu_mode);
	if (E_OK != canif_set_pdu_mode(controller_id, pdu_mode)) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_UNEXPECTED_EXECUTION_SET_PDU_MODE);
	}
}

static FUNC(void, CANSM_CODE) cansm_reset_pend_ind(cansm_internal_network_type *network_sm_ptr)
{
	network_sm_ptr->controller_mode_buf.has_pending_ctrl_indication = (boolean)FALSE;
	network_sm_ptr->transceiver_mode_buf.has_pend_trcv_ind = (boolean)FALSE;
}

static FUNC(void, CANSM_CODE) cansm_internal_comm_bus_sm_mode_indication(const cansm_internal_network_type
	*network_sm_ptr, comm_mode_type comm_mode)
{
	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	network_sm_global_ptr->cur_com_mode = comm_mode;

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_04, network_handle, comm_mode);

#if (CONFIG_MODULE_COMM == 1)
	comm_bus_sm_mode_indication(network_handle, &comm_mode);
#endif
}

#if (CANSM_PNC_SUPPORT == STD_ON)
static FUNC(cansm_internal_transition_return_type, CANSM_CODE) cansm_internal_de_init_with_pn_support
(cansm_internal_network_type *network_sm_ptr, cansm_internal_pre_no_comm_exit_point_type *exit_point,
	cansm_internal_pre_no_comm_state_type *next_sub_state)
{
	cansm_internal_transition_return_type t_ret = CANSM_T_DONE;

	switch (network_sm_ptr->pre_no_comm_state) {
	case CANSM_PRENOCOMM_S_PN_CLEAR_WUF:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_PN_CLEAR_WUF_CC_STOPPED);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_CC_STOPPED;
		} else {
			break;
		}
	case CANSM_PRENOCOMM_S_CC_STOPPED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STOPPED_TRCV_NORMAL);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_TRCV_NORMAL;
		} else {
			break;
		}
	case CANSM_PRENOCOMM_S_TRCV_NORMAL:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_TRCV_NORMAL_TRCV_STANDBY);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_TRCV_STANDBY;
		} else {
		break;
		}
	case CANSM_PRENOCOMM_S_TRCV_STANDBY:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_TRCV_STANDBY_CC_SLEEP);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_CC_SLEEP;
		} else {
			break;
		}
	case CANSM_PRENOCOMM_S_CC_SLEEP:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_SLEEP_CHECK_WUF_IN_CC_SLEEP);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_CHECK_WUF_IN_CC_SLEEP;
		} else if (CANSM_T_TIMEOUT == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_CHECK_WUF_IN_NOT_CC_SLEEP;
			break;
		} else {
			break;
		}
	case CANSM_PRENOCOMM_S_CHECK_WUF_IN_CC_SLEEP:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CHECK_WUF_IN_CC_SLEEP_NONE);
		if (CANSM_T_DONE == t_ret) {
			*exit_point = CANSM_PRENOCOMM_EXIT_NOCOM;
		}
		break;
	case CANSM_PRENOCOMM_S_CHECK_WUF_IN_NOT_CC_SLEEP:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CHECK_WUF_IN_NOT_CC_SLEEP_PN_CLEAR_WUF);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_PN_CLEAR_WUF;
		}
		break;
	default:
		CANSM_DET_REPORTERROR(CANSM_CANSM_UNEXPECTED_EXECUTION_DEINIT_WITHOUT_PN);
		break;
	}
	return t_ret;
}
#endif

static FUNC(cansm_internal_transition_return_type, CANSM_CODE) cansm_internal_de_init_without_pn_support
(cansm_internal_network_type *network_sm_ptr, cansm_internal_pre_no_comm_exit_point_type *exit_point,
	cansm_internal_pre_no_comm_state_type *next_sub_state)
{
	cansm_internal_transition_return_type t_ret = CANSM_T_DONE;

	switch (network_sm_ptr->pre_no_comm_state) {
	case CANSM_PRENOCOMM_S_CC_STOPPED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STOPPED_CC_SLEEP);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_CC_SLEEP;
		} else {
			break;
		}
	case CANSM_PRENOCOMM_S_CC_SLEEP:
		if (network_sm_ptr->cfg_info_ptr->can_en_trcv == (boolean)TRUE) {
			t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_SLEEP_TRCV_NORMAL);
			if (CANSM_T_DONE == t_ret) {
				*next_sub_state = CANSM_PRENOCOMM_S_TRCV_NORMAL;
			} else {
				break;
			}
		} else {
			t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_SLEEP_NONE);
			if (CANSM_T_DONE == t_ret) {
				*exit_point = CANSM_PRENOCOMM_EXIT_NOCOM;
			}
		break;
		}
	case CANSM_PRENOCOMM_S_TRCV_NORMAL:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_TRCV_NORMAL_TRCV_STANDBY);
		if (CANSM_T_DONE == t_ret) {
			*next_sub_state = CANSM_PRENOCOMM_S_TRCV_STANDBY;
		} else {
			break;
		}
	case CANSM_PRENOCOMM_S_TRCV_STANDBY:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_TRCV_STANDBY_NONE);
		if (CANSM_T_DONE == t_ret) {
			*exit_point = CANSM_PRENOCOMM_EXIT_NOCOM;
		}
		break;
	default:
		CANSM_DET_REPORTERROR(CANSM_CANSM_UNEXPECTED_EXECUTION_DEINIT_WITH_PN);
		break;
	}
	return t_ret;
}

static FUNC(void, CANSM_CODE) cansm_internal_full_comm_e_bus_off(cansm_internal_network_type *network_sm_ptr)
{
	network_sm_ptr->bus_off_time = network_sm_ptr->cfg_info_ptr->cansm_val_comp;
	network_sm_ptr->bus_off_event = (boolean)FALSE;
	network_sm_ptr->bus_off_status = (boolean)TRUE;
	network_sm_ptr->controller_mode_buf.ind_ctrl_mode = (cansm_canif_ctrl_mode)CANIF_CS_UNINIT;

	network_sm_ptr->bus_off_counter = (network_sm_ptr->bus_off_counter == 0xFFFFu) ? 0u
	: (network_sm_ptr->bus_off_counter + 1u);
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
	bswm_cansm_current_state(network_sm_ptr->cfg_info_ptr->id_net, CANSM_BSWM_BUS_OFF);
#endif

	cansm_internal_comm_bus_sm_mode_indication(network_sm_ptr, COMM_SILENT_COMMUNICATION);

#if defined(CONFIG_MODULE_DEM) && (CONFIG_MODULE_DEM == 1)
	if (DEM_EVENT_ID_NULL != network_sm_ptr->cfg_info_ptr->cansm_bsof_dem_evt_id) {
		(void)dem_set_event_status(network_sm_ptr->cfg_info_ptr->cansm_bsof_dem_evt_id,
			DEM_EVENT_STATUS_PREFAILED);
	}
#endif
#if (CANSM_BUSOFF_BEGIN == STD_ON)
	const cansm_config_type *config_ptr = cansm_get_config_ptr();

	config_ptr->bus_off_begin_fnc(network_sm_ptr->cfg_info_ptr->id_net);
#endif
}

static FUNC(cansm_internal_full_comm_exit_point_type, CANSM_CODE) cansm_internal_full_comm_bus_off_check
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_full_comm_exit_point_type exit_point = CANSM_FULLCOMM_EXIT_NONE;

	network_sm_ptr->sub_state_timer = (network_sm_ptr->sub_state_timer == 0xFFFFu) ? 0u
	: (network_sm_ptr->sub_state_timer + 1u);

	if (network_sm_ptr->bus_off_event == (boolean)TRUE) {
		(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_INFO, CANSM_DLT_CHECK_ID_01,
					   network_sm_ptr->cfg_info_ptr->id_net);
		cansm_internal_full_comm_e_bus_off(network_sm_ptr);

		cansm_internal_transition_return_type t_ret =
		cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STARTED_NONE);

		switch (t_ret) {
			case CANSM_T_REPEAT_MAX:
				cansm_reset_pend_ind(network_sm_ptr);
				CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_MAINFUNCTION_CANSM_E_MODE_REQUEST_TIMEOUT);
				exit_point = CANSM_FULLCOMM_EXIT_REP_MAX;
				break;
			default:
				network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_RESTART_CC;
				break;
		}
		network_sm_ptr->sub_state_timer = 0;
	} else if (network_sm_ptr->sub_state_timer >= network_sm_ptr->cfg_info_ptr->cansm_en_bor_time_tx) {
#if defined(CONFIG_MODULE_DEM) && (CONFIG_MODULE_DEM == 1)
		if (DEM_EVENT_ID_NULL != network_sm_ptr->cfg_info_ptr->cansm_bsof_dem_evt_id) {
			(void)dem_set_event_status(network_sm_ptr->cfg_info_ptr->cansm_bsof_dem_evt_id,
				DEM_EVENT_STATUS_PASSED);
		}
#endif

		if (network_sm_ptr->bus_off_status == (boolean)TRUE) {
			network_sm_ptr->bus_off_status = (boolean)FALSE;
#if (CANSM_BUSOFF_END == STD_ON)
			const cansm_config_type *config_ptr = cansm_get_config_ptr();

			config_ptr->bus_off_end_fnc(network_sm_ptr->cfg_info_ptr->id_net);
#endif
		}

		network_sm_ptr->bus_off_counter = 0;
		network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_NO_BUSOFF;
	} else {
		/* 保持当前状态 */
	}

	return exit_point;
}

static FUNC(cansm_internal_full_comm_exit_point_type, CANSM_CODE) cansm_internal_full_comm_no_bus_off
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_full_comm_exit_point_type exit_point = CANSM_FULLCOMM_EXIT_NONE;

	if (network_sm_ptr->bus_off_event == (boolean)TRUE) {
		(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_INFO, CANSM_DLT_CHECK_ID_01,
			network_sm_ptr->cfg_info_ptr->id_net);
		cansm_internal_full_comm_e_bus_off(network_sm_ptr);
		network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_RESTART_CC;
		network_sm_ptr->sub_state_timer = 0;
	} else if (network_sm_ptr->tx_timeout_flag == (boolean)TRUE) {
		network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_TX_TIMEOUT_EXCEPTION;
		network_sm_ptr->tx_timeout_exception_state = CANSM_FullComm_TX_TIMEOUT_EXCEPTION_S_CC_STOPPED;
		network_sm_ptr->tx_timeout_flag = (boolean)FALSE;
		network_sm_ptr->sub_state_timer = 0;
	} else {
		/* Stay in this state. */
	}

	return exit_point;
}

static FUNC(cansm_internal_full_comm_exit_point_type, CANSM_CODE) cansm_internal_full_comm_restart_cc
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_full_comm_exit_point_type exit_point = CANSM_FULLCOMM_EXIT_NONE;

	network_sm_ptr->sub_state_timer = (network_sm_ptr->sub_state_timer == 0xFFFFu) ? 0u
									: (network_sm_ptr->sub_state_timer + 1u);
	cansm_internal_transition_return_type t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STARTED_NONE);

	switch (t_ret) {
		case CANSM_T_DONE:
			network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_TX_OFF;
			network_sm_ptr->sub_state_timer = 0;
			break;
		case CANSM_T_FAIL:
		case CANSM_T_REQ_ACCEPTED:
			network_sm_ptr->sub_state_timer = 0;
			break;
		case CANSM_T_REPEAT_MAX:
			cansm_reset_pend_ind(network_sm_ptr);
			CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_MAINFUNCTION_CANSM_E_MODE_REQUEST_TIMEOUT);
			exit_point = CANSM_FULLCOMM_EXIT_PRENOCOMM;
			network_sm_ptr->sub_state_timer = 0;
			network_sm_ptr->bus_off_event = (boolean)FALSE;
			break;
		default:
			break;
	}

	return exit_point;
}

static FUNC(void, CANSM_CODE) cansm_internal_full_comm_tx_off(cansm_internal_network_type *network_sm_ptr)
{
	network_sm_ptr->sub_state_timer = (network_sm_ptr->sub_state_timer == 0xFFFFu) ? 0u
	: (network_sm_ptr->sub_state_timer + 1u);

	if (((network_sm_ptr->bus_off_time >= network_sm_ptr->cfg_info_ptr->cansm_l1_bor_tm) &&
	(network_sm_ptr->bus_off_counter <= network_sm_ptr->cfg_info_ptr->cansm_l1_to_l2_bor_cnt)) ||
	((network_sm_ptr->bus_off_time >= network_sm_ptr->cfg_info_ptr->cansm_l2_bor_tm) &&
	(network_sm_ptr->bus_off_counter > network_sm_ptr->cfg_info_ptr->cansm_l1_to_l2_bor_cnt))) {
		cansm_internal_set_network_pdu_mode(network_sm_ptr, CANIF_ONLINE);
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
		bswm_cansm_current_state(network_sm_ptr->cfg_info_ptr->id_net, CANSM_BSWM_FULL_COMMUNICATION);
#endif
		cansm_internal_comm_bus_sm_mode_indication(network_sm_ptr, COMM_FULL_COMMUNICATION);
		network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_BUSOFF_CHECK;
		network_sm_ptr->sub_state_timer = 0;
	
		if (network_sm_ptr->bus_off_counter <= network_sm_ptr->cfg_info_ptr->cansm_l1_to_l2_bor_cnt) {
			(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_INFO, CANSM_DLT_CHECK_ID_02,
			network_sm_ptr->cfg_info_ptr->id_net);
		} else {
			(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_INFO, CANSM_DLT_CHECK_ID_03,
			network_sm_ptr->cfg_info_ptr->id_net);
		}
	}
}

static FUNC(void, CANSM_CODE) cansm_internal_full_comm_tx_timeout_exception(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_transition_return_type t_ret = CANSM_T_DONE;
	cansm_internal_tx_timeout_exception_state_type next_sub_state = network_sm_ptr->tx_timeout_exception_state;

	switch (network_sm_ptr->tx_timeout_exception_state) {
	case CANSM_FullComm_TX_TIMEOUT_EXCEPTION_S_CC_STOPPED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STOPPED_CC_STARTED);
		if (CANSM_T_DONE == t_ret) {
			next_sub_state = CANSM_FullComm_TX_TIMEOUT_EXCEPTION_S_CC_STARTED;
		} else {
			break;
		}
	case CANSM_FullComm_TX_TIMEOUT_EXCEPTION_S_CC_STARTED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STARTED_NONE);
		if (CANSM_T_DONE == t_ret) {
			network_sm_ptr->full_comm_state = CANSM_FULLCOMM_S_NO_BUSOFF;
			cansm_internal_set_network_pdu_mode(network_sm_ptr, CANIF_ONLINE);
		}
		break;
	default:
		CANSM_DET_REPORTERROR(CANSM_CANSM_UNEXPECTED_EXECUTION_TX_TIMEOUT);
		break;
	}

	if (next_sub_state != network_sm_ptr->tx_timeout_exception_state) {
		network_sm_ptr->tx_timeout_exception_state = next_sub_state;
	}

	network_sm_ptr->sub_state_timer = (network_sm_ptr->sub_state_timer == 0xFFFFu) ? 0u
	: (network_sm_ptr->sub_state_timer + 1u);
}

static FUNC(cansm_internal_transition_return_type, CANSM_CODE) cansm_internal_wu_validation_state_handle
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_transition_return_type t_ret = CANSM_T_DONE;
	cansm_internal_wu_validation_state_type next_sub_state = network_sm_ptr->wu_validation_state;

	switch (network_sm_ptr->wu_validation_state) {
	case CANSM_WUVALIDATION_S_TRCV_NORMAL:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_TRCV_NORMAL_CC_STOPPED);
		if (CANSM_T_DONE == t_ret) {
			next_sub_state = CANSM_WUVALIDATION_S_CC_STOPPED;
		} else {
			break;
		}
	case CANSM_WUVALIDATION_S_CC_STOPPED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STOPPED_CC_STARTED);
		if (CANSM_T_DONE == t_ret) {
			next_sub_state = CANSM_WUVALIDATION_S_CC_STARTED;
		} else {
			break;
		}
	case CANSM_WUVALIDATION_S_CC_STARTED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STARTED_NONE);
		if (CANSM_T_DONE == t_ret) {
			next_sub_state = CANSM_WUVALIDATION_S_WAIT_LEAVE;
		} else {
			break;
		}
	case CANSM_WUVALIDATION_S_WAIT_LEAVE:
		break;
	default:
		CANSM_DET_REPORTERROR(CANSM_CANSM_UNEXPECTED_EXECUTION_WUVALIDATION);
		break;
	}

	if (next_sub_state != network_sm_ptr->wu_validation_state) {
		network_sm_ptr->wu_validation_state = next_sub_state;
	}

	return t_ret;
}

static FUNC(cansm_internal_pre_no_comm_exit_point_type, CANSM_CODE) cansm_internal_bsm_s_pre_no_com
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_pre_no_comm_exit_point_type exit_point = CANSM_PRENOCOMM_EXIT_NONE;
	cansm_internal_transition_return_type t_ret = CANSM_T_DONE;
	cansm_internal_pre_no_comm_state_type next_sub_state = network_sm_ptr->pre_no_comm_state;
	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	if (network_sm_global_ptr->start_wake_up_flag == (boolean)TRUE) {
		network_sm_global_ptr->start_wake_up_flag = (boolean)FALSE;
		exit_point = CANSM_PRENOCOMM_EXIT_START_WUF;
		return exit_point;
	}

#if (CANSM_PNC_SUPPORT == STD_ON)
	switch (network_sm_ptr->cfg_info_ptr->cantrcv_en_pn) {
		case TRUE:
			t_ret = cansm_internal_de_init_with_pn_support(network_sm_ptr, &exit_point, &next_sub_state);
			break;
		case FALSE:
			t_ret = cansm_internal_de_init_without_pn_support(network_sm_ptr, &exit_point, &next_sub_state);
			break;
		default:
			break;
	}
#else
	t_ret = cansm_internal_de_init_without_pn_support(network_sm_ptr, &exit_point, &next_sub_state);
#endif

	if (CANSM_T_WAIT_INDICATION != t_ret) {
		network_sm_ptr->sub_state_timer = 0;
	}

	if (CANSM_T_REPEAT_MAX == t_ret) {
		cansm_reset_pend_ind(network_sm_ptr);
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_MAINFUNCTION_CANSM_E_MODE_REQUEST_TIMEOUT);
		exit_point = CANSM_PRENOCOMM_EXIT_REP_MAX;
	}

	if (next_sub_state != network_sm_ptr->pre_no_comm_state) {
		network_sm_ptr->pre_no_comm_state = next_sub_state;
	}

	network_sm_ptr->sub_state_timer = (network_sm_ptr->sub_state_timer == 0xFFFFu) ? 0u
	: (network_sm_ptr->sub_state_timer + 1u);
	return exit_point;
}

static FUNC(cansm_internal_pre_full_comm_exit_point_type, CANSM_CODE) cansm_internal_bsm_s_pre_full_com
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_pre_full_comm_exit_point_type exit_point = CANSM_PREFULLCOMM_EXIT_NONE;
	cansm_internal_transition_return_type t_ret = CANSM_T_DONE;
	cansm_internal_pre_full_comm_state_type next_sub_state = network_sm_ptr->pre_full_comm_state;

	switch (network_sm_ptr->pre_full_comm_state) {
	case CANSM_PREFULLCOMM_S_TRCV_NORMAL:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_TRCV_NORMAL_CC_STOPPED);
		if (CANSM_T_DONE == t_ret) {
			next_sub_state = CANSM_PREFULLCOMM_S_CC_STOPPED;
		} else {
			break;
		}
	case CANSM_PREFULLCOMM_S_CC_STOPPED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STOPPED_CC_STARTED);
		if (CANSM_T_DONE == t_ret) {
			next_sub_state = CANSM_PREFULLCOMM_S_CC_STARTED;
		} else {
			break;
		}
	case CANSM_PREFULLCOMM_S_CC_STARTED:
		t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STARTED_NONE);
		if (CANSM_T_DONE == t_ret) {
			exit_point = CANSM_PREFULLCOMM_EXIT_FULLCOMM;
		}
		break;
	default:
		CANSM_DET_REPORTERROR(CANSM_CANSM_UNEXPECTED_EXECUTION_PREFULLCOM);
		break;
	}

	if (CANSM_T_WAIT_INDICATION != t_ret) {
		network_sm_ptr->sub_state_timer = 0;
	}

	if (CANSM_T_REPEAT_MAX == t_ret) {
		cansm_reset_pend_ind(network_sm_ptr);
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_MAINFUNCTION_CANSM_E_MODE_REQUEST_TIMEOUT);
		exit_point = CANSM_PREFULLCOMM_EXIT_REP_MAX;
	}

	if (next_sub_state != network_sm_ptr->pre_full_comm_state) {
		network_sm_ptr->pre_full_comm_state = next_sub_state;
	}

	network_sm_ptr->sub_state_timer++;
	return exit_point;
}

static FUNC(cansm_internal_full_comm_exit_point_type, CANSM_CODE) cansm_internal_bsm_s_full_com
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_full_comm_exit_point_type exit_point = CANSM_FULLCOMM_EXIT_NONE;

	switch (network_sm_ptr->full_comm_state) {
	case CANSM_FULLCOMM_S_BUSOFF_CHECK:
		exit_point = cansm_internal_full_comm_bus_off_check(network_sm_ptr);
		break;
	case CANSM_FULLCOMM_S_NO_BUSOFF:
		exit_point = cansm_internal_full_comm_no_bus_off(network_sm_ptr);
		break;
	case CANSM_FULLCOMM_S_RESTART_CC:
		exit_point = cansm_internal_full_comm_restart_cc(network_sm_ptr);
		break;
	case CANSM_FULLCOMM_S_TX_OFF:
		cansm_internal_full_comm_tx_off(network_sm_ptr);
		break;
	case CANSM_FULLCOMM_S_TX_TIMEOUT_EXCEPTION:
		cansm_internal_full_comm_tx_timeout_exception(network_sm_ptr);
		break;
	default:
		CANSM_DET_REPORTERROR(CANSM_CANSM_UNEXPECTED_EXECUTION_FULLCOM);
		break;
	}
	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	const cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	switch (network_sm_global_ptr->req_mode) {
		case COMM_NO_COMMUNICATION:
		case COMM_SILENT_COMMUNICATION:
			network_sm_ptr->bus_off_status = (boolean)FALSE;
			exit_point = (network_sm_global_ptr->req_mode == COMM_NO_COMMUNICATION)
						? CANSM_FULLCOMM_EXIT_PRENOCOMM
						: CANSM_FULLCOMM_EXIT_SILENTCOMM;
			break;
		default:
			break;
	}
	network_sm_ptr->bus_off_time++;
	return exit_point;
}

static FUNC(cansm_internal_wu_validation_exit_point_type, CANSM_CODE) cansm_internal_bsm_s_wu_validation
(cansm_internal_network_type *network_sm_ptr)
{
	network_sm_ptr->sub_state_timer = (network_sm_ptr->sub_state_timer == 0xFFFFu) ? 0u
	: (network_sm_ptr->sub_state_timer + 1u);

	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	if (network_sm_global_ptr->stop_wake_up_flag == (boolean)TRUE) {
		network_sm_global_ptr->stop_wake_up_flag = (boolean)FALSE;
		return CANSM_WUVALIDATION_EXIT_PRENOCOMM;
	}
	if (COMM_FULL_COMMUNICATION == network_sm_global_ptr->req_mode) {
		return CANSM_WUVALIDATION_EXIT_PREFULLCOMM;
	}
	cansm_internal_transition_return_type t_ret = cansm_internal_wu_validation_state_handle(network_sm_ptr);

	if (CANSM_T_WAIT_INDICATION != t_ret) {
		network_sm_ptr->sub_state_timer = 0u;
	}
	if (CANSM_T_REPEAT_MAX == t_ret) {
		cansm_reset_pend_ind(network_sm_ptr);
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_MAINFUNCTION_CANSM_E_MODE_REQUEST_TIMEOUT);
		return CANSM_WUVALIDATION_EXIT_REP_MAX;
	}
	return CANSM_WUVALIDATION_EXIT_NONE;
}

static FUNC(void, CANSM_CODE) cansm_internal_execute_pre_no_com(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_pre_no_comm_exit_point_type preNoComExit = cansm_internal_bsm_s_pre_no_com(network_sm_ptr);

	switch (preNoComExit) {
		case CANSM_PRENOCOMM_EXIT_NOCOM:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_NOCOM);
			cansm_internal_comm_bus_sm_mode_indication(network_sm_ptr, COMM_NO_COMMUNICATION);
			break;
		case CANSM_PRENOCOMM_EXIT_REP_MAX:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_NOCOM);
			break;
		case CANSM_PRENOCOMM_EXIT_START_WUF:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_WUVALIDATION);
			break;
		default:
			break;
	}
}

static FUNC(void, CANSM_CODE) cansm_internal_execute_no_com(cansm_internal_network_type *network_sm_ptr)
{
	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	if (network_sm_global_ptr->start_wake_up_flag == (boolean)TRUE) {
		network_sm_global_ptr->start_wake_up_flag = (boolean)FALSE;
		network_sm_global_ptr->stop_wake_up_flag = (boolean)FALSE;
		cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_WUVALIDATION);
	}
	if (COMM_FULL_COMMUNICATION == network_sm_global_ptr->req_mode) {
		cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_FULLCOM);
	}
}

static FUNC(void, CANSM_CODE) cansm_internal_execute_pre_full_com(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_pre_full_comm_exit_point_type pre_full_comm_exit =
	cansm_internal_bsm_s_pre_full_com(network_sm_ptr);

	switch (pre_full_comm_exit) {
		case CANSM_PREFULLCOMM_EXIT_FULLCOMM:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_FULLCOM);
			cansm_internal_set_network_pdu_mode(network_sm_ptr, CANIF_ONLINE);
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
			bswm_cansm_current_state(network_sm_ptr->cfg_info_ptr->id_net, CANSM_BSWM_FULL_COMMUNICATION);
#endif
			cansm_internal_comm_bus_sm_mode_indication(network_sm_ptr, COMM_FULL_COMMUNICATION);
			break;
		case CANSM_PREFULLCOMM_EXIT_REP_MAX:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_NOCOM);
			break;
		default:
			break;
	}
}

static FUNC(void, CANSM_CODE) cansm_internal_execute_full_com(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_full_comm_exit_point_type full_comm_exit = cansm_internal_bsm_s_full_com(network_sm_ptr);

	switch (full_comm_exit) {
		case CANSM_FULLCOMM_EXIT_PRENOCOMM:
		case CANSM_FULLCOMM_EXIT_REP_MAX:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_NOCOM);
			break;
		case CANSM_FULLCOMM_EXIT_SILENTCOMM:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_SILENTCOM);
			#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
			bswm_cansm_current_state(network_sm_ptr->cfg_info_ptr->id_net, CANSM_BSWM_SILENT_COMMUNICATION);
			#endif
			cansm_internal_set_network_pdu_mode(network_sm_ptr, CANIF_TX_OFFLINE);
			cansm_internal_comm_bus_sm_mode_indication(network_sm_ptr, COMM_SILENT_COMMUNICATION);
			break;
		default:
			break;
	}
}

static FUNC(void, CANSM_CODE) cansm_internal_execute_silent_com(cansm_internal_network_type *network_sm_ptr)
{
	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	const cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

#if (CANSM_SILENT_BOR_SUPPORT == STD_ON)
	if (network_sm_ptr->bus_off_event == (boolean)TRUE) {
		network_sm_ptr->bus_off_event = (boolean)FALSE;
		network_sm_ptr->bus_off_status = (boolean)TRUE;
#if defined(CONFIG_MODULE_DEM) && (CONFIG_MODULE_DEM == 1)
		if (DEM_EVENT_ID_NULL != network_sm_ptr->cfg_info_ptr->cansm_bsof_dem_evt_id) {
			(void)dem_set_event_status(network_sm_ptr->cfg_info_ptr->cansm_bsof_dem_evt_id,
				DEM_EVENT_STATUS_PREFAILED);
		}
#endif
#if (CANSM_BUSOFF_BEGIN == STD_ON)
		const cansm_config_type *config_ptr = cansm_get_config_ptr();
	
		config_ptr->bus_off_begin_fnc(network_sm_ptr->cfg_info_ptr->id_net);
#endif
		cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_SILENTCOM_BOR);
	} else
#endif
	switch (network_sm_global_ptr->req_mode) {
		case COMM_NO_COMMUNICATION:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_NOCOM);
			break;
		case COMM_FULL_COMMUNICATION:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_FULLCOM);
			cansm_internal_set_network_pdu_mode(network_sm_ptr, CANIF_ONLINE);
			cansm_internal_comm_bus_sm_mode_indication(network_sm_ptr, COMM_FULL_COMMUNICATION);
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
			bswm_cansm_current_state(network_sm_ptr->cfg_info_ptr->id_net, CANSM_BSWM_FULL_COMMUNICATION);
#endif
			break;
		default:
			break;
	}
}

#if (CANSM_SILENT_BOR_SUPPORT == STD_ON)
static FUNC(cansm_internal_silent_comm_bor_exit_point_type, CANSM_CODE) cansm_internal_bsm_s_silent_com_bor
(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_silent_comm_bor_exit_point_type exit_point = CANSM_SILENTCOMMBOR_EXIT_NONE;
	cansm_internal_transition_return_type t_ret = CANSM_T_DONE;
	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	network_sm_ptr->sub_state_timer = (network_sm_ptr->sub_state_timer == 0xFFFFu) ? 0u
	: (network_sm_ptr->sub_state_timer + 1u);
	t_ret = cansm_internal_execute(network_sm_ptr, CANSM_T_CC_STARTED_NONE);

	switch (t_ret) {
		case CANSM_T_DONE:
			exit_point = CANSM_SILENTCOMMBOR_EXIT_SILENTCOMM;
			break;
		case CANSM_T_FAIL:
		case CANSM_T_REQ_ACCEPTED:
			network_sm_ptr->sub_state_timer = 0;
			break;
		case CANSM_T_REPEAT_MAX:
			cansm_reset_pend_ind(network_sm_ptr);
			CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_MAINFUNCTION_CANSM_E_MODE_REQUEST_TIMEOUT);
			exit_point = CANSM_SILENTCOMMBOR_EXIT_REP_MAX;
			network_sm_ptr->sub_state_timer = 0;
			network_sm_ptr->bus_off_event = (boolean)FALSE;
			break;
		default:
			break;
	}

	if ((exit_point == CANSM_SILENTCOMMBOR_EXIT_NONE) &&
		(COMM_NO_COMMUNICATION == network_sm_global_ptr->req_mode)) {
		exit_point = CANSM_SILENTCOMMBOR_EXIT_PRENOCOMM;
	}

	if (exit_point != CANSM_SILENTCOMMBOR_EXIT_NONE) {
		network_sm_ptr->bus_off_status = (boolean)FALSE;
#if (CANSM_BUSOFF_END == STD_ON)
		const cansm_config_type *config_ptr = cansm_get_config_ptr();

		config_ptr->bus_off_end_fnc(network_sm_ptr->cfg_info_ptr->id_net);
#endif
	}

	return exit_point;
}

static FUNC(void, CANSM_CODE) cansm_internal_execute_silent_com_bor(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_silent_comm_bor_exit_point_type silent_comm_bor_exit =
	cansm_internal_bsm_s_silent_com_bor(network_sm_ptr);
	switch (silent_comm_bor_exit) {
		case CANSM_SILENTCOMMBOR_EXIT_SILENTCOMM:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_SILENTCOM);
			break;
		case CANSM_SILENTCOMMBOR_EXIT_PRENOCOMM:
		case CANSM_SILENTCOMMBOR_EXIT_REP_MAX:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_NOCOM);
			break;
		default:
			break;
	}
}
#endif

static FUNC(void, CANSM_CODE) cansm_internal_execute_wu_validation(cansm_internal_network_type *network_sm_ptr)
{
	cansm_internal_wu_validation_exit_point_type wu_validation_exit =
	cansm_internal_bsm_s_wu_validation(network_sm_ptr);

	switch (wu_validation_exit) {
		case CANSM_WUVALIDATION_EXIT_PREFULLCOMM:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_FULLCOM);
			cansm_internal_set_network_pdu_mode(network_sm_ptr, CANIF_ONLINE);
#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
			bswm_cansm_current_state(network_sm_ptr->cfg_info_ptr->id_net, CANSM_BSWM_FULL_COMMUNICATION);
#endif
			cansm_internal_comm_bus_sm_mode_indication(network_sm_ptr, COMM_FULL_COMMUNICATION);
			break;
		case CANSM_WUVALIDATION_EXIT_PRENOCOMM:
		case CANSM_WUVALIDATION_EXIT_REP_MAX:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_NOCOM);
			break;
		default:
			break;
	}
}

static FUNC(boolean, CANSM_CODE) cansm_check_fast_execute(const cansm_internal_network_type *network_sm_ptr,
	uint8 loop_count, uint8 exe_type)
{
	boolean is_fast = (boolean)TRUE;
	network_handle_type network_handle = network_sm_ptr->cfg_info_ptr->id_net;
	const cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);
	uint8 max_execute_time = 1u;

	switch (exe_type) {
		case CANSM_NETWORK_EXECUTE_INIT:
			max_execute_time = CANSM_MAIN_MAX_EXECUTE_TIME_INIT;
			break;
		case CANSM_NETWORK_EXECUTE_MAINFUNC:
			max_execute_time = CANSM_MAIN_MAX_EXECUTE_TIME_MAINFUNC;
			break;
		default:
			break;
	}
	if ((loop_count >= max_execute_time) || network_sm_ptr->is_stop_loop ||
	((network_sm_ptr->bsm_state == CANSM_BSM_S_NOCOM) && (network_sm_global_ptr->req_mode ==
		COMM_NO_COMMUNICATION)) || ((network_sm_ptr->bsm_state ==
		CANSM_BSM_S_SILENTCOM) && (network_sm_global_ptr->req_mode == COMM_SILENT_COMMUNICATION)) ||
		((network_sm_ptr->bsm_state == CANSM_BSM_S_FULLCOM) && ((network_sm_global_ptr->req_mode ==
		COMM_FULL_COMMUNICATION))) || ((network_sm_ptr->bsm_state == CANSM_BSM_S_WUVALIDATION) &&
		(network_sm_ptr->wu_validation_state == CANSM_WUVALIDATION_S_WAIT_LEAVE))) {
		is_fast = (boolean)FALSE;
	}
	return is_fast;
}

static FUNC(void, CANSM_CODE) cansm_network_state_machine_execute(cansm_internal_network_type *network_sm_ptr,
	uint8 exe_type)
{
	network_sm_ptr->is_stop_loop = (boolean)FALSE;
	uint8 loop_count = 0;

	do {
		loop_count++;
		switch (network_sm_ptr->bsm_state) {
			case CANSM_BSM_S_NOT_INITIALIZED:
			cansm_internal_enter_state(network_sm_ptr, CANSM_BSM_S_PRE_NOCOM);
			break;
			case CANSM_BSM_S_PRE_NOCOM:
			cansm_internal_execute_pre_no_com(network_sm_ptr);
			break;
			case CANSM_BSM_S_NOCOM:
			cansm_internal_execute_no_com(network_sm_ptr);
			break;
			case CANSM_BSM_S_PRE_FULLCOM:
			cansm_internal_execute_pre_full_com(network_sm_ptr);
			break;
			case CANSM_BSM_S_FULLCOM:
			cansm_internal_execute_full_com(network_sm_ptr);
			break;
			case CANSM_BSM_S_SILENTCOM:
			cansm_internal_execute_silent_com(network_sm_ptr);
			break;
	#if (CANSM_SILENT_BOR_SUPPORT == STD_ON)
			case CANSM_BSM_S_SILENTCOM_BOR:
			cansm_internal_execute_silent_com_bor(network_sm_ptr);
			break;
	#endif
			case CANSM_BSM_S_WUVALIDATION:
			cansm_internal_execute_wu_validation(network_sm_ptr);
			break;
			default:
			break;  /* nothing to do,other enumeration values do not need to be processed */
		}
	} while (cansm_check_fast_execute(network_sm_ptr, loop_count, exe_type));
}

FUNC(void, CANSM_CODE) cansm_internal_main_function(void)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		return;
	}
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		return;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();

	if (network_core_table[core_id].network_table_addr == NULL) {
		return;
	}
	for (network_handle_type network_idx = 0u; network_idx < network_core_table[core_id].network_table_len;
	network_idx++) {
		cansm_network_state_machine_execute(&network_core_table[core_id].network_table_addr[network_idx],
		CANSM_NETWORK_EXECUTE_MAINFUNC);
	}
	return;
}

static FUNC_P2VAR(cansm_internal_network_type, AUTOMATIC, CANSM_CODE) cansm_canif_ctr_chnl_to_cansm_nw_hdl(uint8
	controller_id)
{
	cansm_internal_network_type *network_sm_ptr = NULL;
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		return network_sm_ptr;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();

	if (network_core_table[core_id].network_table_addr == NULL) {
		return network_sm_ptr;
	}
	for (network_handle_type network_idx = 0u; network_idx < network_core_table[core_id].network_table_len;
	network_idx++) {
		if (controller_id ==
			network_core_table[core_id].network_table_addr[network_idx].controller_mode_buf.controller_id) {
				network_sm_ptr = &network_core_table[core_id].network_table_addr[network_idx];
				break;
		}
	}
	return network_sm_ptr;
}

static FUNC_P2VAR(cansm_internal_network_type, AUTOMATIC, CANSM_CODE) cansm_canIf_trcv_chnl_to_cansn_nw_hdl
(uint8 transceiver_id)
{
	cansm_internal_network_type *network_sm_ptr = NULL;
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		return network_sm_ptr;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();

	if (network_core_table[core_id].network_table_addr == NULL) {
		return network_sm_ptr;
	}
	for (network_handle_type network_idx = 0u; network_idx < network_core_table[core_id].network_table_len;
	network_idx++) {
	if (transceiver_id ==
		network_core_table[core_id].network_table_addr[network_idx].transceiver_mode_buf.trcv_id) {
			network_sm_ptr = &network_core_table[core_id].network_table_addr[network_idx];
			break;
		}
	}
	return network_sm_ptr;
}

FUNC(void, CANSM_CODE) cansm_clear_trcv_wuf_flag_indication(uint8 transceiver_id)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CLEARTRCVWUFINDICATION_CANSM_E_UNINIT);
		return;
	}
	cansm_internal_network_type *network_sm_ptr = cansm_canIf_trcv_chnl_to_cansn_nw_hdl(transceiver_id);

	if (network_sm_ptr == NULL) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CLEARTRCVWUFINDICATION_CANSM_E_PARAM_TRANSCEIVER);
		return;
	}
	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_10,
	network_sm_ptr->cfg_info_ptr->id_net, transceiver_id);

	network_sm_ptr->transceiver_mode_buf.clear_wuf_pend_ind = (boolean)FALSE;
	return;
}

FUNC(void, CANSM_CODE) cansm_check_transceiver_wake_flag_indication(uint8 transceiver_id)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CHECKTRCVWUFINDICATION_CANSM_E_UNINIT);
		return;
	}
	cansm_internal_network_type *network_sm_ptr = cansm_canIf_trcv_chnl_to_cansn_nw_hdl(transceiver_id);

	if (network_sm_ptr == NULL) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CHECKTRCVWUFINDICATION_CANSM_E_PARAM_TRANSCEIVER);
		return;
	}
	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_12,
	network_sm_ptr->cfg_info_ptr->id_net, transceiver_id);

	network_sm_ptr->transceiver_mode_buf.check_wuf_pend_ind = (boolean)FALSE;
	return;
}

FUNC(void, CANSM_CODE) cansm_confirm_pn_availability(uint8 transceiver_id)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CONFIRMPNAVAILABILITY_CANSM_E_UNINIT);
		return;
	}
	const cansm_internal_network_type *network_sm_ptr = cansm_canIf_trcv_chnl_to_cansn_nw_hdl(transceiver_id);

	if (network_sm_ptr == NULL) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CONFIRMPNAVAILABILITY_CANSM_E_PARAM_TRANSCEIVER);
		return;
	}

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_14,
	network_sm_ptr->cfg_info_ptr->id_net, transceiver_id);

#if (CONFIG_MODULE_CANNM == 1)
	cannm_confirm_pn_availability(network_sm_ptr->cfg_info_ptr->id_net);
#endif
	return;
}

FUNC(void, CANSM_CODE) cansm_transceiver_mode_indication(uint8 transceiver_id, CanTrcv_TrcvModeType transceiver_mode)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_TRANSCEIVERMODEINDICATION_CANSM_E_UNINIT);
		return;
	}
	cansm_internal_network_type *network_sm_ptr = cansm_canIf_trcv_chnl_to_cansn_nw_hdl(transceiver_id);

	if (network_sm_ptr == NULL) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_TRANSCEIVERMODEINDICATION_CANSM_E_PARAM_TRANSCEIVER);
		return;
	}

	if ((network_sm_ptr->transceiver_mode_buf.has_pend_trcv_ind == (boolean)TRUE) &&
	(transceiver_mode == network_sm_ptr->transceiver_mode_buf.pend_trcv_mode)) {
		network_sm_ptr->transceiver_mode_buf.has_pend_trcv_ind = (boolean)FALSE;
	}

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_08,
	network_sm_ptr->cfg_info_ptr->id_net, transceiver_id, transceiver_mode);

	network_sm_ptr->transceiver_mode_buf.ind_trcv_mode = transceiver_mode;
	return;
}

#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
FUNC(void, CANSM_CODE) cansm_controller_mode_indication(uint8 controller_id, canif_controller_mode_type controller_mode)
#else
FUNC(void, CANSM_CODE) cansm_controller_mode_indication(uint8 controller_id, Can_ControllerStateType controller_mode)
#endif
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CONTROLLERMODEINDICATION_CANSM_E_UNINIT);
		return;
	}

	cansm_internal_network_type *network_sm_ptr = cansm_canif_ctr_chnl_to_cansm_nw_hdl(controller_id);

	if (network_sm_ptr == NULL) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_CONTROLLERMODEINDICATION_CANSM_E_PARAM_CONTROLLER);
		return;
	}

	if ((network_sm_ptr->controller_mode_buf.has_pending_ctrl_indication == (boolean)TRUE) &&
	((cansm_canif_ctrl_mode)controller_mode == network_sm_ptr->controller_mode_buf.pending_ctrl_mode)) {
		network_sm_ptr->controller_mode_buf.has_pending_ctrl_indication = (boolean)FALSE;
	}

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_06,
	network_sm_ptr->cfg_info_ptr->id_net, controller_id, (uint32)controller_mode);

	network_sm_ptr->controller_mode_buf.ind_ctrl_mode = (cansm_canif_ctrl_mode)controller_mode;

	return;
}

FUNC(Std_ReturnType, CANSM_CODE) cansm_start_wake_up_source(network_handle_type network_handle)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_STARTWAKEUPSOURCE_CANSM_E_UNINIT);
		return E_NOT_OK;
	}
	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	if (network_sm_global_ptr == NULL) {
		return E_NOT_OK;
	}
	network_sm_global_ptr->start_wake_up_flag = (boolean)TRUE;
	return E_OK;
}

FUNC(Std_ReturnType, CANSM_CODE) cansm_stop_wake_up_source(network_handle_type network_handle)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_STOTWAKEUPSOURCE_CANSM_E_UNINIT);
		return E_NOT_OK;
	}
	cansm_external_network_type *network_sm_global_ptr = cansm_get_cansm_global_network_index(network_handle);

	if (network_sm_global_ptr == NULL) {
		return E_NOT_OK;
	}
	network_sm_global_ptr->stop_wake_up_flag = (boolean)TRUE;
	return E_OK;
}

FUNC(uint16, CANSM_CODE) cansm_get_bus_off_counter(network_handle_type network_handle)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_COUNTER_UINIT);
		return 0xFFFFu;
	}
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_COUNTER_CORE_ERROR);
		return 0xFFFFu;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();
	network_handle_type network_table_len = network_core_table[core_id].network_table_len;

	if (network_handle >= network_table_len) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_COUNTER_NETWORK_HANDLE_ERROR);
		return 0xFFFFu;
	}
	const cansm_internal_network_type *const network = network_core_table[core_id].network_table_addr;

	if (network == NULL) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_COUNTER_CORE_ERROR);
		return 0xFFFFu;
	}
	return network[network_handle].bus_off_counter;
}

FUNC(boolean, CANSM_CODE) cansm_get_bus_off_status(network_handle_type network_handle)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_STATUS_UINIT);
		return (boolean)FALSE;
	}
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_STATUS_CORE_ERROR);
		return (boolean)FALSE;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();
	network_handle_type network_table_len = network_core_table[core_id].network_table_len;

	if (network_handle >= network_table_len) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_STATUS_NETWORK_HANDLE_ERROR);
		return (boolean)FALSE;
	}
	const cansm_internal_network_type *const network = network_core_table[core_id].network_table_addr;

	if (network == NULL) {
		CANSM_DET_REPORTERROR(CANSM_CANSM_GET_BUSOFF_STATUS_CORE_ERROR);
		return (boolean)FALSE;
	}
	return network[network_handle].bus_off_status;
}

FUNC_P2VAR(cansm_internal_network_type, const, CANSM_CODE) cansm_get_network_ptr(network_handle_type network_handle)
{
	if (cansm_get_module_status() == CANSM_STATUS_UNINIT) {
		CANSM_DET_REPORTERROR(CanSM_CANSM_GET_BSMSTATE_UINIT);
		return NULL;
	}
	uint8 core_id = (uint8)GetCoreID();

	if (core_id >= CANSM_MULTICORE_CORE_NUMBER) {
		CANSM_DET_REPORTERROR(CanSM_CANSM_GET_BSMSTATE_CORE_ERROR);
		return NULL;
	}
	const cansm_network_core_table_type *network_core_table = cansm_get_network_core_table();
	network_handle_type network_table_len = network_core_table[core_id].network_table_len;

	if (network_handle >= network_table_len) {
		CANSM_DET_REPORTERROR(CanSM_CANSM_GET_BSMSTATE_NETWORK_HANDLE_ERROR);
		return NULL;
	}
	const cansm_internal_network_type *const network = network_core_table[core_id].network_table_addr;

	if (network == NULL) {
		CANSM_DET_REPORTERROR(CanSM_CANSM_GET_BSMSTATE_CORE_ERROR);
		return NULL;
	}
	return &network[network_handle];
}

#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

