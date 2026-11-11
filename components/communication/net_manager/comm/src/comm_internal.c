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
#include "rtfw_atomic.h"

#if defined(CONFIG_MODULE_CANSM) && (CONFIG_MODULE_CANSM == 1)
#include "cansm.h"
#endif

#if defined(CONFIG_MODULE_NM) && (CONFIG_MODULE_NM == 1)
#include "nm.h"
#endif

#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
#include "bswm_bsw.h"
#endif

#if defined(CONFIG_MODULE_DCM) && (CONFIG_MODULE_DCM == 1)
#include "dcm_cbk.h"
#endif

#include "comm_vcos_log.h"
#include "vcos_log.h"
#include "SchM_ComM.h"
#include "Compiler.h"

#define COMM_START_SEC_MULTI_APP_BSS
#include "comm_memmap.h"

VAR(comm_mgr_type, COMM_MULTI_APP_BSS) g_comm_mgr;
P2CONST(comm_config_type, COMM_MULTI_APP_BSS, COMM_APPL_CONST) g_comm_cfg;

#define COMM_STOP_SEC_MULTI_APP_BSS
#include "comm_memmap.h"

#define COMM_START_SEC_CODE
#include "comm_memmap.h"

/****************************************************************************
 * name: comm_start_channel_fcmd_timer
 *
 * description: comm_start_channel_fcmd_timer.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_start_channel_fcmd_timer(comm_channel_type *channel_info);

/****************************************************************************
 * name: comm_stop_channel_fcmd_timer
 *
 * description: comm_stop_channel_fcmd_timer.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_stop_channel_fcmd_timer(comm_channel_type *channel_info);

/****************************************************************************
 * name: comm_is_allow_to_exit_full_com
 *
 * description: comm_is_allow_to_exit_full_com.
 *
 ****************************************************************************/
static FUNC(boolean, COMM_CODE) comm_is_allow_to_exit_full_com(const comm_channel_config_type *channel_cfg,
							       const comm_channel_type *channel_info);

/****************************************************************************
 * name: comm_process_com_mode
 *
 * description: comm_process_com_mode.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_process_com_mode(const comm_channel_config_type *channel_cfg,
							     comm_mode_type com_mode);

/****************************************************************************
 * name: comm_reset_ind
 *
 * description: comm_reset_ind.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_reset_ind(comm_channel_type *channel_info);

/****************************************************************************
 * name: comm_process_nm_notify_on_variant
 *
 * description: comm_process_nm_notify_on_variant.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_process_nm_notify_on_variant(const comm_channel_config_type *channel_cfg,
									 boolean bus_off_recovery);

/****************************************************************************
 * name: comm_process_nm_notify_on_variant_none
 *
 * description: comm_process_nm_notify_on_variant_none.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE)
comm_process_nm_notify_on_variant_none(const comm_channel_config_type *channel_cfg);

/****************************************************************************
 * name: comm_process_notify_nm
 *
 * description: comm_process_notify_nm.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_process_notify_nm(const comm_channel_config_type *channel_cfg,
						      boolean bus_off_recovery);

/****************************************************************************
 * name: comm_enter_network_requested
 *
 * description: comm_enter_network_requested.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_network_requested(const comm_channel_config_type *channel_cfg,
								    comm_channel_type *channel_info,
								    boolean is_request);

/****************************************************************************
 * name: comm_enter_ready_sleep
 *
 * description: comm_enter_ready_sleep.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_ready_sleep(const comm_channel_config_type *channel_cfg,
							      comm_channel_type *channel_info, boolean is_request);

/****************************************************************************
 * name: comm_enter_no_com
 *
 * description: comm_enter_no_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_no_com(const comm_channel_config_type *channel_cfg,
							 comm_channel_type *channel_info, boolean is_request);

/****************************************************************************
 * name: comm_enter_silent_com
 *
 * description: comm_enter_silent_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_silent_com(const comm_channel_config_type *channel_cfg,
							     comm_channel_type *channel_info, boolean is_request);

/****************************************************************************
 * name: comm_handle_from_no_com
 *
 * description: comm_handle_from_no_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_handle_from_no_com(const comm_channel_config_type *channel_cfg,
							       comm_channel_type *channel_info, boolean is_request);

/****************************************************************************
 * name: comm_handle_from_full_com
 *
 * description: comm_handle_from_full_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_handle_from_full_com(const comm_channel_config_type *channel_cfg,
								 comm_channel_type *channel_info, boolean is_request);

/****************************************************************************
 * name: comm_handle_from_silent_com
 *
 * description: comm_handle_from_silent_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_handle_from_silent_com(const comm_channel_config_type *channel_cfg,
								   comm_channel_type *channel_info, boolean is_request);

/****************************************************************************
 * name: comm_handle_full_com_min_time
 *
 * description: comm_handle_full_com_min_time.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_handle_full_com_min_time(const comm_channel_config_type *channel_cfg,
							   comm_channel_type *channel_info);

/****************************************************************************
 * name: get_comm_channel_config
 *
 * description: get_comm_channel_config.
 *
 ****************************************************************************/
static const comm_channel_config_type *get_comm_channel_config(uint8 index, const comm_user_config_type *user_cfg);

/****************************************************************************
 * name: comm_update_channel_req_mode
 *
 * description: comm_update_channel_req_mode.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_update_channel_req_mode(comm_channel_type *channel_info,
						  const comm_channel_config_type *channel_cfg,
						  comm_mode_type mode);

/****************************************************************************
 * name: comm_get_channel_current_com_mode
 *
 * description: comm_get_channel_current_com_mode.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_get_channel_current_com_mode(const comm_channel_config_type *channel_cfg,
									 comm_mode_type *mode);

/****************************************************************************
 * name: comm_check_user_channels_mode
 *
 * description: comm_check_user_channels_mode.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_check_user_channels_mode(const comm_user_config_type *user_cfg,
								     comm_mode_type mode);

/****************************************************************************
 * name: comm_update_user_channels_info
 *
 * description: comm_update_user_channels_info.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_update_user_channels_info(comm_user_handle_type user,
							    const comm_user_config_type *user_cfg, comm_mode_type mode);

/****************************************************************************
 * name: comm_indication_bsw_modules
 *
 * description: comm_indication_bsw_modules.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_notify_bsw_modules(NetworkHandleType channel, comm_mode_type mode);

/****************************************************************************
 * name: comm_start_channel_fcmd_timer
 *
 * description: comm_start_channel_fcmd_timer.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_start_channel_fcmd_timer(comm_channel_type *channel_info)
{
	channel_info->fcmd_time_left = COMM_FULL_COM_MIN_DURATION_TIME;
	channel_info->is_fcmd_timer_stopped = FALSE;
}

/****************************************************************************
 * name: comm_stop_channel_fcmd_timer
 *
 * description: comm_stop_channel_fcmd_timer.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_stop_channel_fcmd_timer(comm_channel_type *channel_info)
{
	channel_info->is_fcmd_timer_stopped = TRUE;
	channel_info->fcmd_time_left = 0u;
}

/****************************************************************************
 * name: comm_is_allow_to_exit_full_com
 *
 * description: comm_is_allow_to_exit_full_com.
 *
 ****************************************************************************/
static FUNC(boolean, COMM_CODE) comm_is_allow_to_exit_full_com(const comm_channel_config_type *channel_cfg,
							       const comm_channel_type *channel_info)
{
	if ((COMM_NM_VARIANT_NONE == channel_cfg->nm_variant) && (0u != channel_info->fcmd_time_left)) {
		return FALSE;
	}

	return TRUE;
}

/****************************************************************************
 * name: comm_process_com_mode
 *
 * description: comm_process_com_mode.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_process_com_mode(const comm_channel_config_type *channel_cfg,
							     comm_mode_type com_mode)
{
	Std_ReturnType status = E_NOT_OK;
	NetworkHandleType channel = channel_cfg->comm_channel;

#if defined(CONFIG_MODULE_CANSM) && (CONFIG_MODULE_CANSM == 1)
	if (COMM_BUS_TYPE_CAN == channel_cfg->bus_type) {
		status = cansm_request_com_mode(channel, com_mode);
	}
#endif
	if (E_OK == status) {
		g_comm_mgr.channels[channel].is_req_bus_sm = TRUE;
	}

	return status;
}

/****************************************************************************
 * name: comm_reset_ind
 *
 * description: comm_reset_ind.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_reset_ind(comm_channel_type *channel_info) {
	channel_info->is_ecum_wkup_ind = FALSE;
	channel_info->is_nw_start_ind = FALSE;
}

/****************************************************************************
 * name: comm_process_nm_notify_on_variant
 *
 * description: comm_process_nm_notify_on_variant.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_process_nm_notify_on_variant(const comm_channel_config_type *channel_cfg,
									 boolean bus_off_recovery) {
	Std_ReturnType status = E_NOT_OK;

#if defined(CONFIG_MODULE_NM) && (CONFIG_MODULE_NM == 1)
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel_cfg->comm_channel];

	switch (channel_info->state) {
		case COMM_STATE_FULL_COM_NET_REQ:
			if ((channel_info->user_req_mask != 0u) || (TRUE == channel_info->is_dcm_req)) {
				status = (COMM_NM_VARIANT_FULL == channel_cfg->nm_variant) ?
					 nm_req_net(channel_cfg->nm_channel) :
					 nm_pas_stup(channel_cfg->nm_channel);

				comm_reset_ind(channel_info);

				return status;
			}

			if ((TRUE == channel_info->is_ecum_wkup_ind) || (TRUE == channel_info->is_nw_start_ind)) {
				status = nm_pas_stup(channel_cfg->nm_channel);

				comm_reset_ind(channel_info);

				return status;
			}

			if (TRUE == bus_off_recovery) {
				return nm_pas_stup(channel_cfg->nm_channel);
			}
			break;
		case COMM_STATE_FULL_COM_READY_SLEEP:
			if (COMM_NM_VARIANT_FULL == channel_cfg->nm_variant) {
				return nm_rel_net(channel_cfg->nm_channel);
			}
			break;
		default:
			break;
	}
#endif

	return status;
}

/****************************************************************************
 * name: comm_process_nm_notify_on_variant_none
 *
 * description: comm_process_nm_notify_on_variant_none.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE)
comm_process_nm_notify_on_variant_none(const comm_channel_config_type *channel_cfg) {
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel_cfg->comm_channel];

	if (COMM_STATE_FULL_COM_NET_REQ == channel_info->state) {
		comm_start_channel_fcmd_timer(channel_info);
	}

	comm_reset_ind(channel_info);

	return E_OK;
}

/****************************************************************************
 * name: comm_process_notify_nm
 *
 * description: comm_process_notify_nm.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_process_notify_nm(const comm_channel_config_type *channel_cfg,
						      boolean bus_off_recovery)
{
	Std_ReturnType status = E_OK;

	switch (channel_cfg->nm_variant) {
		case COMM_NM_VARIANT_FULL: // Fall through
		case COMM_NM_VARIANT_PASSIVE:
			status = comm_process_nm_notify_on_variant(channel_cfg, bus_off_recovery);
			break;
		case COMM_NM_VARIANT_NONE:
			status = comm_process_nm_notify_on_variant_none(channel_cfg);
			break;
		default:
			break;
	}

	return status;
}

/****************************************************************************
 * name: comm_enter_network_requested
 *
 * description: comm_enter_network_requested.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_network_requested(const comm_channel_config_type *channel_cfg,
								    comm_channel_type *channel_info, boolean is_request)
{
	Std_ReturnType status = E_OK;
	Std_ReturnType ret = E_OK;
	comm_state_type state = channel_info->state;

	switch (state) {
		case COMM_STATE_NO_COM_NO_REQ_PENDING:
			channel_info->state = COMM_STATE_NO_COM_REQ_PENDING;
			break;
		case COMM_STATE_FULL_COM_READY_SLEEP:
			channel_info->state = COMM_STATE_FULL_COM_NET_REQ;
			ret = comm_process_notify_nm(channel_cfg, FALSE);
			break;
		default:
			break;
	}

	if (FALSE == channel_info->is_allow_com) {
		return (status & ret);
	}

	status = comm_process_com_mode(channel_cfg, COMM_FULL_COMMUNICATION);
	if (E_OK != status) {
		return (status & ret);
	}

	channel_info->state = COMM_STATE_FULL_COM_NET_REQ;
	channel_info->last_req_mode = COMM_FULL_COMMUNICATION;

	if (FALSE == is_request) {
		if ((FALSE == channel_info->is_ecum_wkup_ind) && (FALSE == channel_info->is_dcm_req)) {
			channel_info->is_nw_start_ind = TRUE;
		}
		comm_reset_state_requests(channel_info);
	} else {
		channel_info->is_user_req_pending = FALSE;
	}

	ret = comm_process_notify_nm(channel_cfg, FALSE);

	return (status & ret);
}

/****************************************************************************
 * name: comm_enter_ready_sleep
 *
 * description: comm_enter_ready_sleep.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_ready_sleep(const comm_channel_config_type *channel_cfg,
							      comm_channel_type *channel_info, boolean is_request)
{
	Std_ReturnType status = E_OK;
	Std_ReturnType ret = E_OK;

	channel_info->mode = COMM_FULL_COMMUNICATION;
	channel_info->state = COMM_STATE_FULL_COM_READY_SLEEP;

	ret = comm_process_notify_nm(channel_cfg, FALSE);
	status = comm_process_com_mode(channel_cfg, COMM_FULL_COMMUNICATION);

	if (E_OK != status) {
		return status;
	}

	channel_info->last_req_mode = COMM_FULL_COMMUNICATION;

	if (FALSE == is_request) {
		comm_reset_state_requests(channel_info);
	} else {
		channel_info->is_user_req_pending = FALSE;
	}

	return ret;
}

/****************************************************************************
 * name: comm_enter_no_com
 *
 * description: comm_enter_no_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_no_com(const comm_channel_config_type *channel_cfg,
							 comm_channel_type *channel_info, boolean is_request)
{
	Std_ReturnType status = comm_process_com_mode(channel_cfg, COMM_NO_COMMUNICATION);

	if (E_OK != status) {
		return status;
	}

	channel_info->last_req_mode = COMM_NO_COMMUNICATION;

	if (FALSE == is_request) {
		comm_reset_state_requests(channel_info);
	} else {
		channel_info->is_user_req_pending = FALSE;
	}

	return status;
}

/****************************************************************************
 * name: comm_enter_silent_com
 *
 * description: comm_enter_silent_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_enter_silent_com(const comm_channel_config_type *channel_cfg,
							     comm_channel_type *channel_info, boolean is_request)
{
	Std_ReturnType status = comm_process_com_mode(channel_cfg, COMM_SILENT_COMMUNICATION);

	if (E_OK != status) {
		return status;
	}

	channel_info->last_req_mode = COMM_SILENT_COMMUNICATION;

	if (FALSE == is_request) {
		comm_reset_state_requests(channel_info);
	} else {
		channel_info->is_user_req_pending = FALSE;
	}

	return status;
}

/****************************************************************************
 * name: comm_handle_from_no_com
 *
 * description: comm_handle_from_no_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_handle_from_no_com(const comm_channel_config_type *channel_cfg,
							       comm_channel_type *channel_info, boolean is_request)
{
	Std_ReturnType status = E_OK;

	if (COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_BUS_SLEEP)) {
		channel_info->state = COMM_STATE_NO_COM_NO_REQ_PENDING;
		comm_reset_state_requests(channel_info);

		return E_OK;
	}

	if ((TRUE == channel_info->is_ecum_wkup_ind) || (TRUE == channel_info->is_dcm_req) ||
	    (channel_info->user_req_mask != 0u) || (COMM_FULL_COMMUNICATION == channel_info->user_req_mode) ||
	    COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_RESTART)) {
		return comm_enter_network_requested(channel_cfg, channel_info, is_request);
	}

	if (COMM_STATE_FULL_COM_NET_REQ != channel_info->state) {
		status = comm_enter_no_com(channel_cfg, channel_info, is_request);
		channel_info->state = COMM_STATE_NO_COM_NO_REQ_PENDING;
		channel_info->is_req_bus_sm = FALSE;
	}

	return status;
}

/****************************************************************************
 * name: comm_handle_from_full_com
 *
 * description: comm_handle_from_full_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_handle_from_full_com(const comm_channel_config_type *channel_cfg,
								 comm_channel_type *channel_info, boolean is_request)
{
	Std_ReturnType status = E_OK;

	if (COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_BUS_SLEEP)) {
		return comm_enter_no_com(channel_cfg, channel_info, is_request);
	}

	if (COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_PREPARE_BUS_SLEEP)) {
		if (COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_NETWORK_MODE) &&
			((COMM_NM_VARIANT_FULL == channel_cfg->nm_variant) ||
			 (COMM_NM_VARIANT_PASSIVE == channel_cfg->nm_variant))) {
			comm_reset_state_requests(channel_info);
			return status;
		}

		if ((COMM_NM_VARIANT_FULL == channel_cfg->nm_variant) ||
			(COMM_NM_VARIANT_PASSIVE == channel_cfg->nm_variant)) {
			return comm_enter_silent_com(channel_cfg, channel_info, is_request);
		}
	}

	if ((channel_info->user_req_mask == 0u) && (FALSE == channel_info->is_dcm_req) &&
	    (COMM_NO_COMMUNICATION == channel_info->user_req_mode)) {
		if (TRUE == comm_is_allow_to_exit_full_com(channel_cfg, channel_info)) {
			if (COMM_STATE_FULL_COM_READY_SLEEP != channel_info->state) {
				return comm_enter_ready_sleep(channel_cfg, channel_info, is_request);
			}
		}
	} else if (COMM_STATE_FULL_COM_NET_REQ != channel_info->state) {
		return comm_enter_network_requested(channel_cfg, channel_info, is_request);
	} else if (COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_NETWORK_MODE)) {
		channel_info->nm_ind_mask &= ~(COMM_NM_IND_NETWORK_MODE);
		channel_info->is_inter_req = FALSE;
	} else {
		channel_info->is_inter_req = FALSE;
	}

	return status;
}

/****************************************************************************
 * name: comm_handle_from_silent_com
 *
 * description: comm_handle_from_silent_com.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_handle_from_silent_com(const comm_channel_config_type *channel_cfg,
								   comm_channel_type *channel_info, boolean is_request)
{
	Std_ReturnType status = E_OK;

	if (COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_BUS_SLEEP)) {
		return comm_enter_no_com(channel_cfg, channel_info, is_request);
	}

	if (COMM_IS_MASK_SET(channel_info->nm_ind_mask, COMM_NM_IND_NETWORK_MODE)) {
		return comm_enter_ready_sleep(channel_cfg, channel_info, is_request);
	}

	if ((channel_info->user_req_mask == 0u) && (FALSE == channel_info->is_dcm_req) &&
	    ((COMM_NO_COMMUNICATION == channel_info->user_req_mode))) {
		return E_OK;
	}

	if ((channel_info->user_req_mask != 0u) || (COMM_FULL_COMMUNICATION == channel_info->user_req_mode) ||
		(TRUE == channel_info->is_dcm_req)) {
		status = comm_enter_network_requested(channel_cfg, channel_info, is_request);
	} else {
		status = E_OK;
	}

	return status;
}

/****************************************************************************
 * name: comm_handle_full_com_min_time
 *
 * description: comm_handle_full_com_min_time.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_handle_full_com_min_time(const comm_channel_config_type *channel_cfg,
							   comm_channel_type *channel_info)
{
	if (TRUE == channel_info->is_fcmd_timer_stopped) {
		return;
	}

	if ((COMM_FULL_COMMUNICATION != channel_info->mode) && (COMM_SILENT_COMMUNICATION != channel_info->mode)) {
		return;
	}

	if (channel_cfg->main_func_period >= channel_info->fcmd_time_left) {
		comm_stop_channel_fcmd_timer(channel_info);
		channel_info->is_inter_req = TRUE;
	} else {
		channel_info->fcmd_time_left -= channel_cfg->main_func_period;
	}
}

/****************************************************************************
 * name: get_comm_channel_config
 *
 * description: get_comm_channel_config.
 *
 ****************************************************************************/
static const comm_channel_config_type *get_comm_channel_config(uint8 index, const comm_user_config_type *user_cfg)
{
	return user_cfg->channels[index];
}

/****************************************************************************
 * name: comm_update_channel_req_mode
 *
 * description: comm_update_channel_req_mode.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_update_channel_req_mode(comm_channel_type *channel_info,
						  const comm_channel_config_type *channel_cfg,
						  comm_mode_type mode)
{
	(void)channel_cfg;

	if ((COMM_FULL_COMMUNICATION == mode) || (0u == channel_info->user_req_mask)) {
		channel_info->user_req_mode = mode;
		channel_info->is_user_req_pending = TRUE;
	}
}

/****************************************************************************
 * name: comm_get_channel_current_com_mode
 *
 * description: comm_get_channel_current_com_mode.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_get_channel_current_com_mode(const comm_channel_config_type *channel_cfg,
									 comm_mode_type *mode)
{
	Std_ReturnType status = E_NOT_OK;

#if defined(CONFIG_MODULE_CANSM) && (CONFIG_MODULE_CANSM == 1)
	if (COMM_BUS_TYPE_CAN == channel_cfg->bus_type) {
		status = cansm_get_cur_com_mode(channel_cfg->comm_channel, mode);
	}
#endif

	return status;
}

/****************************************************************************
 * name: comm_check_user_channels_mode
 *
 * description: comm_check_user_channels_mode.
 *
 ****************************************************************************/
static FUNC(Std_ReturnType, COMM_CODE) comm_check_user_channels_mode(const comm_user_config_type *user_cfg,
								     comm_mode_type mode)
{
	uint8 channel_num = user_cfg->channel_num;

	for (uint8 i = 0; i < channel_num; i++) {
		const comm_channel_config_type *channel_cfg = get_comm_channel_config(i, user_cfg);
		comm_channel_type *channel_info = &g_comm_mgr.channels[channel_cfg->comm_channel];
		comm_mode_type com_mode = COMM_NO_COMMUNICATION;
		Std_ReturnType status = E_OK;

		if (TRUE == channel_info->is_req_bus_sm) {
			continue;
		}

		status = comm_get_channel_current_com_mode(channel_cfg, &com_mode);
		if ((E_OK == status) && (com_mode != channel_info->mode)) {
			return E_NOT_OK;
		}
	}

	return E_OK;
}

/****************************************************************************
 * name: comm_update_user_channels_info
 *
 * description: comm_update_user_channels_info.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_update_user_channels_info(comm_user_handle_type user,
							    const comm_user_config_type *user_cfg, comm_mode_type mode)
{
	uint8 channel_num = user_cfg->channel_num;
	uint64 user_mask = (1ULL << user);

	g_comm_mgr.users[user].req_mode = mode;

	for (uint8 i = 0; i < channel_num; i++) {
		const comm_channel_config_type *channel_cfg = get_comm_channel_config(i, user_cfg);
		comm_channel_type *channel_info = &g_comm_mgr.channels[channel_cfg->comm_channel];

		if (COMM_NO_COMMUNICATION == mode) {
			channel_info->user_req_mask &= ~(user_mask);
		} else if (COMM_FULL_COMMUNICATION == mode) {
			channel_info->user_req_mask |= user_mask;
		}

		comm_update_channel_req_mode(channel_info, channel_cfg, mode);
	}
}

/****************************************************************************
 * name: comm_indication_bsw_modules
 *
 * description: comm_indication_bsw_modules.
 *
 ****************************************************************************/
static FUNC(void, COMM_CODE) comm_notify_bsw_modules(NetworkHandleType channel, comm_mode_type mode)
{
#if (defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)) || \
	(defined(CONFIG_MODULE_DCM) && (CONFIG_MODULE_DCM == 1))
	(void)VCOS_LOG(COMM_MODULE_ID, VCOS_LOG_DEBUG, COMM_DLT_DEBUG_BUSSM, channel, mode);

#if defined(CONFIG_SERVM_BSWM) && (CONFIG_SERVM_BSWM == 1)
	bswm_comm_current_mode(channel, mode);
#endif

#if defined(CONFIG_MODULE_DCM) && (CONFIG_MODULE_DCM == 1)
	if (COMM_NO_COMMUNICATION == mode) {
		dcm_comm_no_com_mode_entered(channel);
	} else if (COMM_FULL_COMMUNICATION == mode) {
		dcm_comm_full_com_mode_entered(channel);
	} else if (COMM_SILENT_COMMUNICATION == mode) {
		dcm_comm_silent_com_mode_entered(channel);
	} else {
		/* do nothing */
	}
#endif
#endif
}

/****************************************************************************
 * name: comm_reset_state_requests
 *
 * description: comm_reset_state_requests.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_reset_state_requests(comm_channel_type *channel_info)
{
	channel_info->nm_ind_mask = COMM_NM_IND_NONE;
	channel_info->is_inter_req = FALSE;
}

/****************************************************************************
 * name: comm_is_bus_off_ind
 *
 * description: comm_is_bus_off_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_bus_off_ind(const comm_channel_config_type *channel_cfg,
						const comm_channel_type *channel_info, comm_mode_type mode)
{
	if (COMM_BUS_TYPE_CAN == channel_cfg->bus_type) {
		return ((FALSE == channel_info->is_req_bus_sm) && (COMM_SILENT_COMMUNICATION == mode));
	}

	return FALSE;
}

/****************************************************************************
 * name: comm_is_bus_off_recovery_ind
 *
 * description: comm_is_bus_off_recovery_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_bus_off_recovery_ind(const comm_channel_config_type *channel_cfg,
							 const comm_channel_type *channel_info,
							 comm_mode_type mode)
{
	if (COMM_BUS_TYPE_CAN == channel_cfg->bus_type) {
		return ((FALSE == channel_info->is_req_bus_sm) && (COMM_FULL_COMMUNICATION == mode));
	}

	return FALSE;
}

/****************************************************************************
 * name: comm_is_cansm_repeat_max_ind
 *
 * description: comm_is_cansm_repeat_max_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_cansm_repeat_max_ind(const comm_channel_config_type *channel_cfg,
						  const comm_channel_type *channel_info, comm_mode_type mode)
{
	return ((COMM_BUS_TYPE_CAN == channel_cfg->bus_type) && (FALSE == channel_info->is_req_bus_sm) &&
		(COMM_NO_COMMUNICATION == mode));
}

/****************************************************************************
 * name: comm_is_valid_ind
 *
 * description: comm_is_valid_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_valid_ind(const comm_channel_type *channel_info, comm_mode_type mode)
{
	return ((TRUE == channel_info->is_req_bus_sm) && (channel_info->last_req_mode == mode));
}

/****************************************************************************
 * name: comm_handle_channel_state
 *
 * description: comm_handle_channel_state.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_handle_channel_state(const comm_channel_config_type *channel_cfg,
							  boolean is_request)
{
	Std_ReturnType status = E_NOT_OK;
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel_cfg->comm_channel];

	if (COMM_NO_COMMUNICATION == channel_info->mode) {
		status = comm_handle_from_no_com(channel_cfg, channel_info, is_request);
	} else if (COMM_FULL_COMMUNICATION == channel_info->mode) {
		status = comm_handle_from_full_com(channel_cfg, channel_info, is_request);
	} else if (COMM_SILENT_COMMUNICATION == channel_info->mode) {
		status = comm_handle_from_silent_com(channel_cfg, channel_info, is_request);
	} else {
		/* do nothing */
	}

	return status;
}

/****************************************************************************
 * name: comm_handle_timers
 *
 * description: comm_handle_timers.
 *
 ****************************************************************************/
void comm_handle_timers(const comm_channel_config_type *channel_cfg, comm_channel_type *channel_info)
{
	switch (channel_cfg->nm_variant) {
		case COMM_NM_VARIANT_NONE:
			comm_handle_full_com_min_time(channel_cfg, channel_info);
			break;
		default:
			/* do nothing */
			break;
	}
}

/****************************************************************************
 * name: comm_request_user_com_mode
 *
 * description: comm_request_user_com_mode.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_request_user_com_mode(comm_user_handle_type user, comm_mode_type mode)
{
	Std_ReturnType total_status = E_OK;
	const comm_user_config_type *user_cfg = &g_comm_cfg->users[user];

	SchM_Enter_COMM_EXCLUSIVE_AREA_0();

	total_status = comm_check_user_channels_mode(user_cfg, mode);
	if (E_NOT_OK != total_status) {
		comm_update_user_channels_info(user, user_cfg, mode);
	}

	SchM_Exit_COMM_EXCLUSIVE_AREA_0();

	return total_status;
}

/****************************************************************************
 * name: comm_handle_bus_sm_channel_mode_ind
 *
 * description: comm_handle_bus_sm_channel_mode_ind.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_handle_bus_sm_channel_mode_ind(NetworkHandleType channel, comm_mode_type mode)
{
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel];
	comm_mode_type curr_mode = channel_info->mode;

	channel_info->mode = mode;

	if (curr_mode != mode) {
		comm_notify_bsw_modules(channel, mode);
	}

	channel_info->is_req_bus_sm = FALSE;

	if (COMM_NO_COMMUNICATION == mode) {
		channel_info->state = COMM_STATE_NO_COM_NO_REQ_PENDING;
	} else if (COMM_FULL_COMMUNICATION == mode) {
		if (COMM_STATE_FULL_COM_READY_SLEEP != channel_info->state) {
			channel_info->state = COMM_STATE_FULL_COM_NET_REQ;
		}
	} else if (COMM_SILENT_COMMUNICATION == mode) {
		channel_info->state = COMM_STATE_SILENT_COM;
	} else {
		/* do nothing */
	}
}

/****************************************************************************
 * name: comm_handle_nm_ind
 *
 * description: comm_handle_nm_ind.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_handle_nm_ind(NetworkHandleType channel, uint8 service_id, uint8 nm_indication)
{
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel];

	SchM_Enter_COMM_EXCLUSIVE_AREA_0();

	channel_info->is_inter_req = TRUE;
	channel_info->nm_ind_mask |= nm_indication;

	SchM_Exit_COMM_EXCLUSIVE_AREA_0();

	(void)VCOS_LOG(COMM_MODULE_ID, VCOS_LOG_DEBUG, COMM_DLT_DEBUG_NM, channel, service_id,
		       channel_info->nm_ind_mask);
}

/****************************************************************************
 * name: comm_handle_dcm_active_diag
 *
 * description: comm_handle_dcm_active_diag.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_handle_dcm_active_diag(NetworkHandleType channel, boolean is_active)
{
	comm_channel_type *channel_info = &g_comm_mgr.channels[channel];

	SchM_Enter_COMM_EXCLUSIVE_AREA_0();

	channel_info->is_inter_req = TRUE;
	channel_info->is_dcm_req = is_active;

	SchM_Exit_COMM_EXCLUSIVE_AREA_0();
}

#define COMM_STOP_SEC_CODE
#include "comm_memmap.h"
