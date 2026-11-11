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

#ifndef COMM_INTERNAL_H
#define COMM_INTERNAL_H

#include "comm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   comm向nm指示不同模式
 **********************************************************************************************************************/
#define COMM_NM_IND_NONE ((uint8)(0u))
#define COMM_NM_IND_NETWORK_MODE ((uint8)(1u))
#define COMM_NM_IND_PREPARE_BUS_SLEEP ((uint8)(1u << 1))
#define COMM_NM_IND_BUS_SLEEP ((uint8)(1u << 2))
#define COMM_NM_IND_RESTART ((uint8)(1u << 3))

#define COMM_DET_REPORTERROR(...)

#define COMM_VALIDATE(expression, error_id, ret)                                                                       \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			return ret;                                                                                    \
		}                                                                                                      \
	} while (0)

#define COMM_VALIDATE_NO_RV(expression, error_id)                                                                      \
	do {                                                                                                           \
		if (!(expression)) {                                                                                   \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

#define COMM_VALIDATE_INIT(error_id) COMM_VALIDATE((COMM_INIT == g_comm_mgr.init_status), (error_id), E_NOT_OK)

#define COMM_VALIDATE_INIT_NO_RV(error_id) COMM_VALIDATE_NO_RV((COMM_INIT == g_comm_mgr.init_status), (error_id))

#define COMM_VALIDATE_PARAM(expression, error_id) COMM_VALIDATE((expression), (error_id), E_NOT_OK)

#define COMM_VALIDATE_PARAM_NO_RV(expression, error_id) COMM_VALIDATE_NO_RV((expression), (error_id))

#define COMM_VALIDATE_USER(user, error_id) COMM_VALIDATE_PARAM(((user) < COMM_USER_COUNT), (error_id))

#define COMM_VALIDATE_CHANNEL(channel, error_id) COMM_VALIDATE_PARAM(((channel) < COMM_CHANNEL_COUNT), (error_id))

#define COMM_VALIDATE_CHANNEL_NO_RV(channel, error_id)                                                                 \
	COMM_VALIDATE_PARAM_NO_RV(((channel) < COMM_CHANNEL_COUNT), (error_id))

#define COMM_IS_MASK_SET(src, mask) (((src) & (mask)) == (mask))

/*!*********************************************************************************************************************
 * @brief   comm用户类型
 **********************************************************************************************************************/
typedef struct {
	comm_mode_type req_mode;
} comm_user_mode_type;

/*!*********************************************************************************************************************
 * @brief   comm通道类型
 **********************************************************************************************************************/
typedef struct {
	boolean is_allow_com;
	boolean is_req_bus_sm;
	boolean is_user_req_pending;
	boolean is_nw_start_ind;
	boolean is_ecum_wkup_ind;
	boolean is_dcm_req;
	boolean is_inter_req;
	boolean is_fcmd_timer_stopped;
	uint8 nm_ind_mask;
	comm_mode_type mode;
	comm_state_type state;
	comm_mode_type user_req_mode;
	comm_mode_type last_req_mode;
	uint32 fcmd_time_left;
	uint64 user_req_mask;
} comm_channel_type;

/*!*********************************************************************************************************************
 * @brief   comm管理数据类型
 **********************************************************************************************************************/
typedef struct {
	comm_init_status_type init_status;
	comm_user_mode_type users[COMM_USER_COUNT];
	comm_channel_type channels[COMM_CHANNEL_COUNT];
} comm_mgr_type;

#define COMM_START_SEC_CODE
#include "comm_memmap.h"

/****************************************************************************
 * name: comm_reset_state_requests
 *
 * description: comm_reset_state_requests.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_reset_state_requests(comm_channel_type *channel_info);

/****************************************************************************
 * name: comm_is_bus_off_ind
 *
 * description: comm_is_bus_off_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_bus_off_ind(const comm_channel_config_type *channel_cfg,
						const comm_channel_type *channel_info, comm_mode_type mode);

/****************************************************************************
 * name: comm_is_bus_off_recovery_ind
 *
 * description: comm_is_bus_off_recovery_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_bus_off_recovery_ind(const comm_channel_config_type *channel_cfg,
							 const comm_channel_type *channel_info,
							 comm_mode_type mode);

/****************************************************************************
 * name: comm_is_cansm_repeat_max_ind
 *
 * description: comm_is_cansm_repeat_max_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_cansm_repeat_max_ind(const comm_channel_config_type *channel_cfg,
						  const comm_channel_type *channel_info, comm_mode_type mode);

/****************************************************************************
 * name: comm_is_valid_ind
 *
 * description: comm_is_valid_ind.
 *
 ****************************************************************************/
FUNC(boolean, COMM_CODE) comm_is_valid_ind(const comm_channel_type *channel_info, comm_mode_type mode);

/****************************************************************************
 * name: comm_handle_channel_state
 *
 * description: comm_handle_channel_state.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_handle_channel_state(const comm_channel_config_type *channel_cfg,
							  boolean is_request);

/****************************************************************************
 * name: comm_handle_timers
 *
 * description: comm_handle_timers.
 *
 ****************************************************************************/
void comm_handle_timers(const comm_channel_config_type *channel_cfg, comm_channel_type *channel_info);

/****************************************************************************
 * name: comm_request_user_com_mode
 *
 * description: comm_request_user_com_mode.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_request_user_com_mode(comm_user_handle_type user, comm_mode_type mode);

/****************************************************************************
 * name: comm_handle_bus_sm_channel_mode_ind
 *
 * description: comm_handle_bus_sm_channel_mode_ind.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_handle_bus_sm_channel_mode_ind(NetworkHandleType channel, comm_mode_type mode);

/****************************************************************************
 * name: comm_handle_nm_ind
 *
 * description: comm_handle_nm_ind.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_handle_nm_ind(NetworkHandleType channel, uint8 service_id, uint8 nm_indication);

/****************************************************************************
 * name: comm_handle_dcm_active_diag
 *
 * description: comm_handle_dcm_active_diag.
 *
 ****************************************************************************/
FUNC(void, COMM_CODE) comm_handle_dcm_active_diag(NetworkHandleType channel, boolean is_active);

#define COMM_STOP_SEC_CODE
#include "comm_memmap.h"

#define COMM_START_SEC_MULTI_APP_BSS
#include "comm_memmap.h"

extern VAR(comm_mgr_type, COMM_MULTI_APP_BSS) g_comm_mgr;
extern P2CONST(comm_config_type, COMM_MULTI_APP_BSS, COMM_APPL_CONST) g_comm_cfg;

#define COMM_STOP_SEC_MULTI_APP_BSS
#include "comm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* COMM_INTERNAL_H */
