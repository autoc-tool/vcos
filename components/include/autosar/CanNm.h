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

#ifndef AUTOSAROS_INC_CANNM_H
#define AUTOSAROS_INC_CANNM_H

#include "cannm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NetworkHandleType
typedef network_handle_type NetworkHandleType;
#endif

#ifndef CanNm_ConfigType
typedef cannm_cate_cfg CanNm_ConfigType;
#endif

#ifndef CanNm_ChannelType
typedef cannm_chnl_cate CanNm_ChannelType;
#endif

#ifndef CanNm_Internal_ChannelType
typedef cannm_op_chnl_cate CanNm_Internal_ChannelType;
#endif

#ifndef CanNm_GetChannelRunTimeData
#define CanNm_GetChannelRunTimeData cannm_get_chnl_run_time_data
#endif

#ifndef CanNm_Init
#define CanNm_Init cannm_init
#endif

#ifndef CanNm_DeInit
#define CanNm_DeInit cannm_de_init
#endif

#ifndef CanNm_PassiveStartUp
#define CanNm_PassiveStartUp cannm_pas_stup
#endif

#ifndef CanNm_NetworkRequest
#define CanNm_NetworkRequest cannm_req_net
#endif

#ifndef CanNm_NetworkRelease
#define CanNm_NetworkRelease cannm_rel_net
#endif

#ifndef CanNm_DisableCommunication
#define CanNm_DisableCommunication cannm_disable_comm
#endif

#ifndef CanNm_EnableCommunication
#define CanNm_EnableCommunication cannm_enable_comm
#endif

#ifndef CanNm_SetUserData
#define CanNm_SetUserData cannm_set_ur_data
#endif

#ifndef CanNm_GetUserData
#define CanNm_GetUserData cannm_get_ur_data
#endif

#ifndef CanNm_GetNodeIdentifier
#define CanNm_GetNodeIdentifier cannm_get_nd_id
#endif

#ifndef CanNm_GetLocalNodeIdentifier
#define CanNm_GetLocalNodeIdentifier cannm_get_local_nd_id
#endif

#ifndef CanNm_RepeatMessageRequest
#define CanNm_RepeatMessageRequest cannm_req_rpt_msg
#endif

#ifndef CanNm_GetPduData
#define CanNm_GetPduData cannm_get_msg_data
#endif

#ifndef CanNm_GetState
#define CanNm_GetState cannm_get_state
#endif

#ifndef CanNm_TxConfirmation
#define CanNm_TxConfirmation cannm_tx_confirmation
#endif

#ifndef CanNm_NetworkMode_RxIndication
#define CanNm_NetworkMode_RxIndication cannm_net_mode_rx_ind_proc
#endif

#ifndef CanNm_RxIndication
#define CanNm_RxIndication cannm_rx_indication
#endif

#ifndef CanNm_Transmit
#define CanNm_Transmit cannm_transmit
#endif

#ifndef CanNm_NmStateRepeatMessageHandling
#define CanNm_NmStateRepeatMessageHandling cannm_state_rpt_msg_handling
#endif

#ifndef CanNm_NmStateNormalOperationHandling
#define CanNm_NmStateNormalOperationHandling cannm_state_nor_opr_handling
#endif

#ifndef CanNm_NmStateReadySleepHandling
#define CanNm_NmStateReadySleepHandling cannm_state_rdy_slp_handling
#endif

#ifndef CanNm_NmModeNetworkHandling
#define CanNm_NmModeNetworkHandling cannm_mode_net_handling
#endif

#ifndef CanNm_MainFunctionInternal
#define CanNm_MainFunctionInternal cannm_m_func_proc
#endif

#ifndef CanNm_MultipleMainFunction
#define CanNm_MultipleMainFunction cannm_mtpl_m_func_proc
#endif

#ifndef CanNm_ConfirmPnAvailability
#define CanNm_ConfirmPnAvailability cannm_confirm_pn_availability
#endif

#ifndef CanNm_Internal_TickTimeoutTime
#define CanNm_Internal_TickTimeoutTime cannm_tmot_time_proc
#endif

#ifndef CanNm_Internal_TickRepeatMessageTime
#define CanNm_Internal_TickRepeatMessageTime cannm_rpt_msg_time_proc
#endif

#ifndef CanNm_NmModePrepareBusSleepHandling
#define CanNm_NmModePrepareBusSleepHandling cannm_mode_pbs_handling
#endif

#ifndef CanNm_Internal_TickTxTimeout
#define CanNm_Internal_TickTxTimeout cannm_tx_tmot_troc
#endif

#ifndef CanNm_Internal_TickMessageCycleTime
#define CanNm_Internal_TickMessageCycleTime cannm_msg_cyc_time_proc
#endif

#ifndef CanNm_Internal_TransmitMessage
#define CanNm_Internal_TransmitMessage cannm_tx_msg
#endif

#ifndef CanNm_TriggerTransmit
#define CanNm_TriggerTransmit cannm_trigger_transmit
#endif

#ifndef CanNm_Internal_GetUserDataOffset
#define CanNm_Internal_GetUserDataOffset cannm_ur_data_ofst_get
#endif

#ifndef CanNm_Internal_GetUserDataPtr
#define CanNm_Internal_GetUserDataPtr cannm_ur_data_ptr_get
#endif

#ifndef CanNm_Internal_GetUserDataLength
#define CanNm_Internal_GetUserDataLength cannm_ur_data_len_get
#endif

#ifndef CanNm_Internal_ClearRptMsgRqstCbv
#define CanNm_Internal_ClearRptMsgRqstCbv cannm_clr_rpt_msg_req_cbv
#endif

#ifndef CanNm_Internal_SetPNICbv
#define CanNm_Internal_SetPNICbv cannm_set_pni_cbv
#endif

#ifndef CanNm_Internal_SetActiveWakeupCbv
#define CanNm_Internal_SetActiveWakeupCbv cannm_set_active_wk_cbv
#endif

#ifndef CanNm_Internal_ClearActiveWakeupCbv
#define CanNm_Internal_ClearActiveWakeupCbv cannm_clr_active_wk_cbv
#endif

#ifndef CanNm_Internal_PrepareBusSleep_to_RepeatMessage
#define CanNm_Internal_PrepareBusSleep_to_RepeatMessage cannm_pbs_to_rpt_msg_proc
#endif

#ifndef CanNm_Internal_PrepareBusSleep_to_BusSleep
#define CanNm_Internal_PrepareBusSleep_to_BusSleep cannm_pbs_to_bs_proc
#endif

#ifndef CanNm_Internal_BusSleep_to_RepeatMessage
#define CanNm_Internal_BusSleep_to_RepeatMessage cannm_bs_to_rpt_msg_proc
#endif

#ifndef CanNm_Internal_BusSleep_to_BusSleep
#define CanNm_Internal_BusSleep_to_BusSleep cannm_bs_to_bs_proc
#endif

#ifndef CanNm_Internal_RepeatMessage_to_RepeatMessage
#define CanNm_Internal_RepeatMessage_to_RepeatMessage cannm_rpt_msg_to_rpt_msg_proc
#endif

#ifndef CanNm_Internal_RepeatMessage_to_ReadySleep
#define CanNm_Internal_RepeatMessage_to_ReadySleep cannm_rpt_msg_to_rdy_slp_proc
#endif

#ifndef CanNm_Internal_RepeatMessage_to_NormalOperation
#define CanNm_Internal_RepeatMessage_to_NormalOperation cannm_rpt_msg_to_nor_opr_proc
#endif

#ifndef CanNm_Internal_NormalOperation_to_RepeatMessage
#define CanNm_Internal_NormalOperation_to_RepeatMessage cannm_nor_opr_to_rpt_msg_proc
#endif

#ifndef CanNm_Internal_NormalOperation_to_ReadySleep
#define CanNm_Internal_NormalOperation_to_ReadySleep cannm_nor_opr_to_rdy_slp_proc
#endif

#ifndef CanNm_Internal_NormalOperation_to_NormalOperation
#define CanNm_Internal_NormalOperation_to_NormalOperation cannm_nor_opr_to_nor_opr_proc
#endif

#ifndef CanNm_Internal_ReadySleep_to_NormalOperation
#define CanNm_Internal_ReadySleep_to_NormalOperation cannm_rdy_slp_to_nor_opr_proc
#endif

#ifndef CanNm_Internal_ReadySleep_to_PrepareBusSleep
#define CanNm_Internal_ReadySleep_to_PrepareBusSleep cannm_rdy_slp_to_pbs_proc
#endif

#ifndef CanNm_Internal_ReadySleep_to_RepeatMessage
#define CanNm_Internal_ReadySleep_to_RepeatMessage cannm_rdy_slp_to_rpt_msg_proc
#endif

#ifndef CanNm_Internal_NetworkMode_to_NetworkMode
#define CanNm_Internal_NetworkMode_to_NetworkMode cannm_net_mode_to_net_mode_proc
#endif

#ifndef CanNm_Internal_RxProcess_Normal_Reception
#define CanNm_Internal_RxProcess_Normal_Reception cannm_nor_rx_proc
#endif

#ifndef CanNm_Internal_RxProcess_Discard_Reception
#define CanNm_Internal_RxProcess_Discard_Reception cannm_dscd_rx_proc
#endif

#ifndef CanNm_Internal_RxProcess
#define CanNm_Internal_RxProcess cannm_rx_proc
#endif

#ifndef CanNm_Monitor_GetCurrentRequested
#define CanNm_Monitor_GetCurrentRequested cannm_monitor_get_current_requested
#endif

#ifndef CanNm_GetCanIfFirstNmMsgId
#define CanNm_GetCanIfFirstNmMsgId cannm_get_canif_first_nm_msg_id
#endif

#ifndef CanNm_NormalOperation_to_RepeatMessage_Immediate_Transmission
#define CanNm_NormalOperation_to_RepeatMessage_Immediate_Transmission cannm_nor_opr_to_rpt_msg_imdt_tx
#endif

#ifndef CanNm_RepeatMessage_to_RepeatMessage_Immediate_Transmission
#define CanNm_RepeatMessage_to_RepeatMessage_Immediate_Transmission cannm_rpt_msg_to_rpt_msg_imdt_tx
#endif

#ifndef CanNm_ReadySleep_to_RepeatMessage_Immediate_Transmission
#define CanNm_ReadySleep_to_RepeatMessage_Immediate_Transmission cannm_rdy_slp_to_rpt_msg_imdt_tx
#endif

#ifdef __cplusplus
}
#endif

#endif /* !defined(AUTOSAROS_INC_CANNM_H) */
