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

#ifndef AUTOSAROS_INC_NM_H
#define AUTOSAROS_INC_NM_H

#include "nm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NetworkHandleType
typedef network_handle_type NetworkHandleType;
#endif

#ifndef Nm_StateType
typedef nm_state_type Nm_StateType;
#endif

#ifndef Nm_ModeType
typedef nm_mode_type Nm_ModeType;
#endif

#ifndef Nm_Init
#define Nm_Init nm_init
#endif

#ifndef Nm_DeInit
#define Nm_DeInit nm_de_init
#endif

#ifndef Nm_PassiveStartUp
#define Nm_PassiveStartUp nm_pas_stup
#endif

#ifndef Nm_NetworkRequest
#define Nm_NetworkRequest nm_req_net
#endif

#ifndef Nm_NetworkRelease
#define Nm_NetworkRelease nm_rel_net
#endif

#ifndef Nm_DisableCommunication
#define Nm_DisableCommunication nm_disable_comm
#endif

#ifndef Nm_EnableCommunication
#define Nm_EnableCommunication nm_enable_comm
#endif

#ifndef Nm_SetUserData
#define Nm_SetUserData nm_set_ur_data
#endif

#ifndef Nm_GetUserData
#define Nm_GetUserData nm_get_ur_data
#endif

#ifndef Nm_GetPduData
#define Nm_GetPduData nm_get_msg_data
#endif

#ifndef Nm_RepeatMessageRequest
#define Nm_RepeatMessageRequest nm_req_rpt_msg
#endif

#ifndef Nm_GetNodeIdentifier
#define Nm_GetNodeIdentifier nm_get_nd_id
#endif

#ifndef Nm_GetLocalNodeIdentifier
#define Nm_GetLocalNodeIdentifier nm_get_local_nd_id
#endif

#ifndef Nm_GetState
#define Nm_GetState nm_get_state
#endif

#ifndef Nm_NetworkStartIndication
#define Nm_NetworkStartIndication nm_start_net_ind
#endif

#ifndef Nm_NetworkMode
#define Nm_NetworkMode nm_mode_net_proc
#endif

#ifndef Nm_PrepareBusSleepMode
#define Nm_PrepareBusSleepMode nm_mode_pbs_proc
#endif

#ifndef Nm_BusSleepMode
#define Nm_BusSleepMode nm_mode_bs_proc
#endif

#ifndef Nm_PduRxIndication
#define Nm_PduRxIndication nm_rx_pdu_ind
#endif

#ifndef Nm_StateChangeBswMNotification
#define Nm_StateChangeBswMNotification nm_ntfy_state_chg_bswm
#endif

#ifndef Nm_StateChangeNotification
#define Nm_StateChangeNotification nm_ntfy_state_chg
#endif

#ifndef Nm_RepeatMessageIndication
#define Nm_RepeatMessageIndication nm_rpt_msg_ind
#endif

#ifndef Nm_TxTimeoutException
#define Nm_TxTimeoutException nm_tx_tmot_excpt_pro
#endif

#ifndef Nm_MainFunction
#define Nm_MainFunction nm_main_function
#endif

#ifdef __cplusplus
}
#endif

#endif /* !defined(AUTOSAROS_INC_NM_H) */
