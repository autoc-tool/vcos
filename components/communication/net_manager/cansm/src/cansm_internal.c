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

#if defined(CONFIG_MODULE_CANIF) && (CONFIG_MODULE_CANIF == 1)
#include "canif.h"
#endif
#include "cansm_internal.h"
#include "cansm_cfg.h"
#include "cansm_vcos_log.h"
#include "vcos_log.h"
#include "cansm_types.h"
#if defined(VCOS_MODULE_CONFIG_AUTOSAROS) && (VCOS_MODULE_CONFIG_AUTOSAROS == 1)
#include "Os_Sys_Cfg.h"
#endif
#if ((CANSM_SUPPORT_MULTICORE == STD_ON) && defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1))
#include "Os.h"
#include "Spinlock_Cfg.h"
#endif

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
static FUNC(Std_ReturnType, CANSM_CODE) cansm_internal_get_ctrl_mode_ind(const cansm_internal_network_type
	*network_sm_ptr)
{
	Std_ReturnType total_status = E_OK;

	if (network_sm_ptr->controller_mode_buf.ind_ctrl_mode !=
		network_sm_ptr->controller_mode_buf.pending_ctrl_mode) {
		total_status = E_NOT_OK;
	}
	if (network_sm_ptr->controller_mode_buf.has_pending_ctrl_indication == (boolean)TRUE) {
		total_status = E_NOT_OK;
	}

	return total_status;
}

static FUNC(Std_ReturnType, CANSM_CODE) cansm_internal_set_network_ctrl_mode(cansm_internal_network_type
	*network_sm_ptr, cansm_canif_ctrl_mode controller_mode)
{
	Std_ReturnType total_status = E_OK;
	uint8 controller_id = network_sm_ptr->controller_mode_buf.controller_id;

	if (network_sm_ptr->controller_mode_buf.ind_ctrl_mode != controller_mode) {
		network_sm_ptr->controller_mode_buf.has_pending_ctrl_indication = (boolean)TRUE;
		network_sm_ptr->controller_mode_buf.pending_ctrl_mode = controller_mode;
		(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_05,
			network_sm_ptr->cfg_info_ptr->id_net, controller_id, controller_mode);
		Std_ReturnType set_ret = E_NOT_OK;
		Std_ReturnType spin_lock_ret = cansm_get_ctrl_spin_lock();

		if (spin_lock_ret == E_OK) {
#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
			set_ret = canif_set_controller_mode(controller_id, (canif_controller_mode_type)controller_mode);
#else
			set_ret = canif_set_controller_mode(controller_id, (Can_ControllerStateType)controller_mode);
#endif
			(void)cansm_release_ctr_spin_lock();
		}
		if (set_ret != E_OK) {
			network_sm_ptr->controller_mode_buf.has_pending_ctrl_indication = (boolean)FALSE;
			total_status = E_NOT_OK;
		}
	}
	return total_status;
}

#if defined(CANSM_TRCV_SUPPORT) && (CANSM_TRCV_SUPPORT == STD_ON)
static FUNC(Std_ReturnType, CANSM_CODE) cansm_internal_set_network_trcv_mode(cansm_internal_network_type
	*network_sm_ptr, CanTrcv_TrcvModeType trcv_mode)
{
	network_sm_ptr->transceiver_mode_buf.has_pend_trcv_ind = (boolean)TRUE;
	network_sm_ptr->transceiver_mode_buf.pend_trcv_mode = trcv_mode;
	uint8 trcv_id = network_sm_ptr->transceiver_mode_buf.trcv_id;

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_07,
	network_sm_ptr->cfg_info_ptr->id_net, trcv_id, trcv_mode);
	if (E_OK != canif_set_trcv_mode(trcv_id, trcv_mode)) {
		network_sm_ptr->transceiver_mode_buf.has_pend_trcv_ind = (boolean)FALSE;
		return E_NOT_OK;
	}
	return E_OK;
}

static FUNC(Std_ReturnType, CANSM_CODE) cansm_get_trcv_mode_ind(const cansm_internal_network_type
	*network_sm_ptr)
{
	Std_ReturnType total_status = E_OK;

	if (network_sm_ptr->transceiver_mode_buf.ind_trcv_mode !=
	network_sm_ptr->transceiver_mode_buf.pend_trcv_mode) {
		total_status = E_NOT_OK;
	}
	if (network_sm_ptr->transceiver_mode_buf.has_pend_trcv_ind == (boolean)TRUE) {
		total_status = E_NOT_OK;
	}
	return total_status;
}

#if defined(CANSM_PNC_SUPPORT) && (CANSM_PNC_SUPPORT == STD_ON)
static FUNC(Std_ReturnType, CANSM_CODE) cansm_internal_check_wuf(cansm_internal_network_type *network_sm_ptr)
{
	Std_ReturnType total_status = E_OK;
	uint8 trcv_id = network_sm_ptr->transceiver_mode_buf.trcv_id;

	network_sm_ptr->transceiver_mode_buf.check_wuf_pend_ind = (boolean)TRUE;
	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_11,
	network_sm_ptr->cfg_info_ptr->id_net, trcv_id);
	if (E_OK != canif_check_trcv_wake_flag(trcv_id)) {
		network_sm_ptr->transceiver_mode_buf.check_wuf_pend_ind = (boolean)FALSE;
		total_status = E_NOT_OK;
	}
	return total_status;
}

static FUNC(Std_ReturnType, CANSM_CODE) cansm_internal_clr_wuf(cansm_internal_network_type *network_sm_ptr)
{
	Std_ReturnType total_status = E_OK;
	uint8 trcv_id = network_sm_ptr->transceiver_mode_buf.trcv_id;

	network_sm_ptr->transceiver_mode_buf.clear_wuf_pend_ind = (boolean)TRUE;

	(void)VCOS_LOG(CANSM_MODULE_ID, VCOS_LOG_DEBUG, CANSM_DLT_CHECK_ID_09,
	network_sm_ptr->cfg_info_ptr->id_net, trcv_id);

	if (E_OK != canif_clear_trcv_wuf_flag(trcv_id)) {
		network_sm_ptr->transceiver_mode_buf.clear_wuf_pend_ind = (boolean)FALSE;
		total_status = E_NOT_OK;
	}

	return total_status;
}

static FUNC(Std_ReturnType, CANSM_CODE) cansm_check_wuf_ind(const cansm_internal_network_type *network_sm_ptr)
{
	Std_ReturnType total_status = E_OK;

	if (network_sm_ptr->transceiver_mode_buf.check_wuf_pend_ind == (boolean)TRUE) {
		total_status = E_NOT_OK;
	}
	return total_status;
}

static FUNC(Std_ReturnType, CANSM_CODE) cansm_internal_clr_wuf_ind(const cansm_internal_network_type *network_sm_ptr)
{
	Std_ReturnType total_status = E_OK;

	if (network_sm_ptr->transceiver_mode_buf.clear_wuf_pend_ind == (boolean)TRUE) {
		total_status = E_NOT_OK;
	}
	return total_status;
}
#endif
#endif

static FUNC(Std_ReturnType, CANSM_CODE) mode_req_action(cansm_internal_network_type *network_sm_ptr,
const cansm_action_type *action, const cansm_action_mode *mode)
{
	Std_ReturnType action_ret = E_NOT_OK;

	switch (*action) {
		case CANSM_A_CC_MODE:
			action_ret = cansm_internal_set_network_ctrl_mode(network_sm_ptr, mode->ccMode);
			break;

#if defined(CANSM_TRCV_SUPPORT) && (CANSM_TRCV_SUPPORT == STD_ON)
		case CANSM_A_TRCV_MODE:
			action_ret = cansm_internal_set_network_trcv_mode(network_sm_ptr, mode->trcv_mode);
			break;
#endif

#if defined(CANSM_PNC_SUPPORT) && (CANSM_PNC_SUPPORT == STD_ON)
		case CANSM_A_WUF:
			switch(mode->wufAction) {
				case CANSM_CHECK_WUF:
					action_ret = cansm_internal_check_wuf(network_sm_ptr);
					break;
				case CANSM_CLEAR_WUF:
					action_ret = cansm_internal_clr_wuf(network_sm_ptr);
					break;
				default:
					break;
			}
			break;
#endif
		default:
			break;
	}

	return action_ret;
}

static FUNC(Std_ReturnType, CANSM_CODE) mode_read_indicated(const cansm_internal_network_type *network_sm_ptr,
	const cansm_action_type *action, const cansm_action_mode *mode)
{
	Std_ReturnType indication_ret = E_NOT_OK;

	switch (*action) {
		case CANSM_A_CC_MODE:
			indication_ret = cansm_internal_get_ctrl_mode_ind(network_sm_ptr);
			break;
	
#if defined(CANSM_TRCV_SUPPORT) && (CANSM_TRCV_SUPPORT == STD_ON)
		case CANSM_A_TRCV_MODE:
			indication_ret = cansm_get_trcv_mode_ind(network_sm_ptr);
			break;
#endif

#if defined(CANSM_PNC_SUPPORT) && (CANSM_PNC_SUPPORT == STD_ON)
		case CANSM_A_WUF:
			switch (mode->wufAction) {
				case CANSM_CHECK_WUF:
					indication_ret = cansm_check_wuf_ind(network_sm_ptr);
					break;
				case CANSM_CLEAR_WUF:
					indication_ret = cansm_internal_clr_wuf_ind(network_sm_ptr);
					break;
				default:
					break;
			}
			break;
#endif
		default:
			CANSM_DET_REPORTERROR(CANSM_CANSM_SERVICEID_MAINFUNCTION_CANSM_E_INVALID_ACTION);
			break;
	}

	return indication_ret;
}

static FUNC(cansm_internal_transition_return_type, CANSM_CODE) check_mode_req_not_acpt
(cansm_internal_network_type *network_sm_ptr, cansm_internal_transition_type transition)
{
	cansm_internal_transition_return_type ret = CANSM_T_WAIT_INDICATION;
	const cansm_transition_type *transition_config = cansm_get_transition_config();
	const cansm_transition_type *transition_ptr = &transition_config[transition];

	network_sm_ptr->repeat_counter = (network_sm_ptr->repeat_counter >= 0xFFu)
	? 0u : (network_sm_ptr->repeat_counter + 1u);
	Std_ReturnType action_ret = mode_req_action(network_sm_ptr, &(transition_ptr->FirstAction),
	&(transition_ptr->FirstMode));
	if (E_OK == action_ret) {
		network_sm_ptr->request_accepted = (boolean)TRUE;
		ret = CANSM_T_REQ_ACCEPTED;
	} else {
		network_sm_ptr->is_stop_loop = (boolean)TRUE;
		ret = CANSM_T_FAIL;

		const cansm_config_type *config_ptr = cansm_get_config_ptr();

		if (network_sm_ptr->repeat_counter > config_ptr->cansm_mode_request_repetition_max) {
			network_sm_ptr->repeat_counter = 0u;
			ret = CANSM_T_REPEAT_MAX;
		}
	}

	return ret;
}

static FUNC(cansm_internal_transition_return_type, CANSM_CODE) check_mode_indicated_timeout(cansm_internal_network_type
	*network_sm_ptr, cansm_internal_transition_type transition)
{
	cansm_internal_transition_return_type ret = CANSM_T_WAIT_INDICATION;
	const cansm_transition_type *transition_config = cansm_get_transition_config();
	const cansm_transition_type *transition_ptr = &transition_config[transition];

	network_sm_ptr->request_accepted = (boolean)FALSE;

#if (defined(CANSM_PNC_SUPPORT)) && (CANSM_PNC_SUPPORT == STD_ON)
	if ((network_sm_ptr->bsm_state == CANSM_BSM_S_PRE_NOCOM) &&
	(network_sm_ptr->cfg_info_ptr->cantrcv_en_pn) && (CANSM_A_CC_MODE == transition_ptr->FirstAction) &&
	((uint8)CANIF_CS_SLEEP == transition_ptr->FirstMode.ccMode)) {
		return CANSM_T_TIMEOUT;
	}
#endif
	const cansm_config_type *config_ptr = cansm_get_config_ptr();

	if (network_sm_ptr->repeat_counter <= config_ptr->cansm_mode_request_repetition_max) {
		network_sm_ptr->repeat_counter++;
		const Std_ReturnType action_ret = mode_req_action(network_sm_ptr, &transition_ptr->FirstAction,
			&transition_ptr->FirstMode);
		network_sm_ptr->request_accepted = (action_ret == E_OK);
		ret = (action_ret == E_OK) ? CANSM_T_REQ_ACCEPTED : CANSM_T_FAIL;
		network_sm_ptr->is_stop_loop = (action_ret != E_OK);
	} else {
		network_sm_ptr->repeat_counter = 0;
		ret = CANSM_T_REPEAT_MAX;
	}

	return ret;
}

static FUNC(cansm_internal_transition_return_type, CANSM_CODE) check_mode_req_has_acpt
(cansm_internal_network_type *network_sm_ptr, cansm_internal_transition_type transition)
{
	cansm_internal_transition_return_type ret = CANSM_T_WAIT_INDICATION;
	const cansm_transition_type *transition_config = cansm_get_transition_config();
	const cansm_transition_type *transition_ptr = &transition_config[transition];

	Std_ReturnType indication_ret = mode_read_indicated(network_sm_ptr, &(transition_ptr->FirstAction),
	&(transition_ptr->FirstMode));
	if (E_OK == indication_ret) {
		network_sm_ptr->repeat_counter = 0u;
		network_sm_ptr->request_accepted = (boolean)FALSE;

		if (CANSM_A_NO_MODE != transition_ptr->SecondAction) {
			if (E_OK == mode_req_action(network_sm_ptr,
				&(transition_ptr->SecondAction), &(transition_ptr->SecondMode))) {
				network_sm_ptr->request_accepted = (boolean)TRUE;
			} else {
				network_sm_ptr->is_stop_loop = (boolean)TRUE;
			}
			network_sm_ptr->repeat_counter = network_sm_ptr->repeat_counter + 1u;
		}
		ret = CANSM_T_DONE;
	} else if (network_sm_ptr->sub_state_timer >= network_sm_ptr->cfg_info_ptr->cansm_req_md_rept_tm) {
		ret = check_mode_indicated_timeout(network_sm_ptr, transition);
	} else {
	/* Do Nothing */
	}
	return ret;
}

FUNC(cansm_internal_transition_return_type, CANSM_CODE) cansm_internal_execute(cansm_internal_network_type
	*network_sm_ptr, cansm_internal_transition_type transition)
{
	cansm_internal_transition_return_type ret = network_sm_ptr->request_accepted
	? check_mode_req_has_acpt(network_sm_ptr, transition)
	: check_mode_req_not_acpt(network_sm_ptr, transition);
	return ret;
}

FUNC(Std_ReturnType, CANSM_CODE) cansm_get_ctrl_spin_lock(void)
{
#if ((CANSM_SUPPORT_MULTICORE == STD_ON) && defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1))

	uint16 cansm_try_spin_lock_time = 0u;
	TryToGetSpinlockType lock_result = TRYTOGETSPINLOCK_NOSUCCESS;

	do {
		(void)TryToGetSpinlock((SpinlockIdType)CANSM_MULTICORE_TRCV_SET_SPINLOCK_ID, &lock_result);
		if (lock_result == TRYTOGETSPINLOCK_SUCCESS) {
			return E_OK;
		} else {
			cansm_try_spin_lock_time++;
		}
	} while (cansm_try_spin_lock_time < CANSM_SPINLOCK_RETRY_COUNTER);
	CANSM_DET_REPORTERROR(CanSM_CANSM_GET_SPINLOCK_CANSM_GET_SPINLOCK_FAILED);
	return E_NOT_OK;
#else
	return E_OK;
#endif
}

FUNC(Std_ReturnType, CANSM_CODE) cansm_release_ctr_spin_lock(void)
{
#if ((CANSM_SUPPORT_MULTICORE == STD_ON) && defined(VCOS_OS_CONFIG_SPINLOCK) && (VCOS_OS_CONFIG_SPINLOCK == 1))
	uint16 cansm_try_spin_lock_time = 0u;
	StatusType result;

	do {
		result = ReleaseSpinlock((SpinlockIdType)CANSM_MULTICORE_TRCV_SET_SPINLOCK_ID);
		if (result == E_OK) {
			return E_OK;
		} else {
			cansm_try_spin_lock_time++;
		}
	} while (cansm_try_spin_lock_time < CANSM_SPINLOCK_RETRY_COUNTER);
	CANSM_DET_REPORTERROR(CanSM_CANSM_REL_SPINLOCK_CANSM_REL_SPINLOCK_FAILED);
	return E_NOT_OK;
#else
	return E_OK;
#endif
}

#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

