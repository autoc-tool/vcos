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
#include "comm.h"
#include "comm_internal.h"
#include "comm_vcos_log.h"
#include "vcos_log.h"
#include "SchM_ComM.h"
#include "Compiler.h"

#define COMM_START_SEC_CODE
#include "comm_memmap.h"

/****************************************************************************
 * name: comm_init
 *
 * description: comm_init.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_init(const comm_config_type *config)
{
	COMM_VALIDATE_PARAM_NO_RV((NULL != config), COMM_COMM_SERVICEID_INIT_COMM_E_WRONG_PARAMETERS);
	COMM_VALIDATE_PARAM_NO_RV((NULL != config->channels), COMM_COMM_SERVICEID_INIT_COMM_E_WRONG_PARAMETERS);
	COMM_VALIDATE_PARAM_NO_RV((NULL != config->users), COMM_COMM_SERVICEID_INIT_COMM_E_WRONG_PARAMETERS);

	uint8 i = 0;

	g_comm_cfg = config;

	for (i = 0; i < COMM_CHANNEL_COUNT; i++) {
		g_comm_mgr.channels[i].is_fcmd_timer_stopped = TRUE;
	}

	g_comm_mgr.init_status = COMM_INIT;
}

/****************************************************************************
 * name: comm_deinit
 *
 * description: comm_deinit.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_deinit(void)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_DEINIT_COMM_E_UNINIT);

	g_comm_mgr.init_status = COMM_UNINIT;

	(void)VCOS_LOG(COMM_MODULE_ID, VCOS_LOG_INFO, COMM_DLT_DEINIT);
}

/****************************************************************************
 * name: comm_get_state
 *
 * description: comm_get_state.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_get_state(NetworkHandleType channel, comm_state_type *state)
{
	COMM_VALIDATE_INIT(COMM_COMM_SERVICEID_GETSTATE_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL(channel, COMM_COMM_SERVICEID_GETSTATE_COMM_E_WRONG_PARAMETERS);
	COMM_VALIDATE_PARAM((NULL != state), COMM_COMM_SERVICEID_GETSTATE_COMM_E_WRONG_PARAMETERS);

	*state = g_comm_mgr.channels[channel].state;

	return E_OK;
}

/****************************************************************************
 * name: comm_get_status
 *
 * description: comm_get_status.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_get_status(comm_init_status_type *status)
{
	COMM_VALIDATE_PARAM((NULL != status), COMM_COMM_SERVICEID_GETSTATUS_COMM_E_WRONG_PARAMETERS);

	*status = g_comm_mgr.init_status;

	return E_OK;
}

/****************************************************************************
 * name: comm_request_com_mode
 *
 * description: comm_request_com_mode.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_request_com_mode(comm_user_handle_type user, comm_mode_type com_mode)
{
	COMM_VALIDATE_INIT(COMM_COMM_SERVICEID_REQUESTCOMMODE_COMM_E_UNINIT);
	COMM_VALIDATE_USER(user, COMM_COMM_SERVICEID_REQUESTCOMMODE_COMM_E_WRONG_PARAMETERS);
	COMM_VALIDATE_PARAM((COMM_SILENT_COMMUNICATION != com_mode),
				COMM_COMM_SERVICEID_REQUESTCOMMODE_COMM_E_WRONG_PARAMETERS);

	return comm_request_user_com_mode(user, com_mode);
}

/****************************************************************************
 * name: comm_nm_net_start_indication
 *
 * description: comm_nm_net_start_indication.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_nm_net_start_indication(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_NM_NETWORKSTARTINDICATION_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_NM_NETWORKSTARTINDICATION_COMM_E_WRONG_PARAMETERS);

	comm_handle_nm_ind(channel, COMM_SERVICEID_NM_NETWORKSTARTINDICATION, COMM_NM_IND_RESTART);
}

/****************************************************************************
 * name: comm_nm_net_mode
 *
 * description: comm_nm_net_mode.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_nm_net_mode(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_NM_NETWORKMODE_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_NM_NETWORKMODE_COMM_E_WRONG_PARAMETERS);

	comm_handle_nm_ind(channel, COMM_SERVICEID_NM_NETWORKMODE, COMM_NM_IND_NETWORK_MODE);
}

/****************************************************************************
 * name: comm_nm_pre_bus_sleep_mode
 *
 * description: comm_nm_pre_bus_sleep_mode.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_nm_pre_bus_sleep_mode(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE_COMM_E_WRONG_PARAMETERS);

	comm_handle_nm_ind(channel, COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE, COMM_NM_IND_PREPARE_BUS_SLEEP);
}

/****************************************************************************
 * name: comm_nm_bus_sleep_mode
 *
 * description: comm_nm_bus_sleep_mode.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_nm_bus_sleep_mode(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_NM_BUSSLEEPMODE_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_NM_BUSSLEEPMODE_COMM_E_WRONG_PARAMETERS);

	comm_handle_nm_ind(channel, COMM_SERVICEID_NM_BUSSLEEPMODE, COMM_NM_IND_BUS_SLEEP);
}

/****************************************************************************
 * name: comm_nm_restart_indication
 *
 * description: comm_nm_restart_indication.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_nm_restart_indication(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_NM_RESTARTINDICATION_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_NM_RESTARTINDICATION_COMM_E_WRONG_PARAMETERS);

	comm_handle_nm_ind(channel, COMM_SERVICEID_NM_RESTARTINDICATION, COMM_NM_IND_RESTART);
}

/****************************************************************************
 * name: comm_dcm_active_diagnostic
 *
 * description: comm_dcm_active_diagnostic.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_dcm_active_diagnostic(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC_COMM_E_WRONG_PARAMETERS);

	comm_handle_dcm_active_diag(channel, TRUE);
}

/****************************************************************************
 * name: comm_dcm_inactive_diagnostic
 *
 * description: comm_dcm_inactive_diagnostic.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_dcm_inactive_diagnostic(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC_COMM_E_WRONG_PARAMETERS);

	comm_handle_dcm_active_diag(channel, FALSE);
}

/****************************************************************************
 * name: comm_bus_sm_mode_indication
 *
 * description: comm_bus_sm_mode_indication.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_bus_sm_mode_indication(NetworkHandleType channel, const comm_mode_type *com_mode)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_BUSSM_MODEINDICATION_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_BUSSM_MODEINDICATION_COMM_E_WRONG_PARAMETERS);
	COMM_VALIDATE_PARAM_NO_RV((NULL != com_mode),
					 COMM_COMM_SERVICEID_BUSSM_MODEINDICATION_COMM_E_WRONG_PARAMETERS);

	comm_mode_type mode = *com_mode;
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel];
	const comm_channel_config_type *channel_cfg = &g_comm_cfg->channels[channel];

	SchM_Enter_COMM_EXCLUSIVE_AREA_0();

	if (comm_is_valid_ind(channel_info, mode) ||
	    comm_is_bus_off_ind(channel_cfg, channel_info, mode) ||
	    comm_is_bus_off_recovery_ind(channel_cfg, channel_info, mode) ||
	    comm_is_cansm_repeat_max_ind(channel_cfg, channel_info, mode)) {
		comm_handle_bus_sm_channel_mode_ind(channel, mode);
	}

	SchM_Exit_COMM_EXCLUSIVE_AREA_0();
}

/****************************************************************************
 * name: comm_main_function
 *
 * description: comm_main_function.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_main_function(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_MAINFUNCTION_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_MAINFUNCTION_COMM_E_WRONG_PARAMETERS);

	const comm_channel_config_type *channel_cfg = &g_comm_cfg->channels[channel];
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel];

	SchM_Enter_COMM_EXCLUSIVE_AREA_0();

	comm_handle_timers(channel_cfg, channel_info);

	if (TRUE == channel_info->is_inter_req) {
		(void)comm_handle_channel_state(channel_cfg, FALSE);
	}

	if (TRUE == channel_info->is_user_req_pending) {
		if (TRUE == channel_info->is_inter_req) {
			if (!COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_PREPARE_BUS_SLEEP)) {
				comm_reset_state_requests(channel_info);
			}
		}
		(void)comm_handle_channel_state(channel_cfg, TRUE);
	}

	SchM_Exit_COMM_EXCLUSIVE_AREA_0();
}

/****************************************************************************
 * name: comm_communication_allowed
 *
 * description: comm_communication_allowed.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_communication_allowed(NetworkHandleType channel, boolean allowed)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_COMMUNICATIONALLOWED_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_COMMUNICATIONALLOWED_COMM_E_WRONG_PARAMETERS);

	g_comm_mgr.channels[channel].is_allow_com = allowed;
}

/****************************************************************************
 * name: comm_ecum_wake_up_indication
 *
 * description: comm_ecum_wake_up_indication.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_ecum_wake_up_indication(NetworkHandleType channel)
{
	COMM_VALIDATE_INIT_NO_RV(COMM_COMM_SERVICEID_ECUM_WAKEUPINDICATION_COMM_E_UNINIT);
	COMM_VALIDATE_CHANNEL_NO_RV(channel, COMM_COMM_SERVICEID_ECUM_WAKEUPINDICATION_COMM_E_WRONG_PARAMETERS);

	comm_channel_type *channel_info = &g_comm_mgr.channels[channel];

	SchM_Enter_COMM_EXCLUSIVE_AREA_0();

	if ((COMM_NO_COMMUNICATION == channel_info->mode) ||
	    ((COMM_STATE_SILENT_COM == channel_info->state) &&
	     (COMM_NO_COMMUNICATION == channel_info->last_req_mode) &&
	     (TRUE == channel_info->is_req_bus_sm))) {
		channel_info->is_ecum_wkup_ind = TRUE;
		channel_info->is_inter_req = TRUE;
	}

	SchM_Exit_COMM_EXCLUSIVE_AREA_0();
}

#define COMM_STOP_SEC_CODE
#include "comm_memmap.h"
