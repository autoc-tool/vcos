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

#ifndef CANNM_H
#define CANNM_H

#include "comstack_types.h"
#if (CONFIG_MODULE_NM == 1)
#include "nmstack_types.h"
#endif
#include "cannm_types.h"
#include "cannm_cbk.h"
#include "cannm_lcfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CANNM_START_SEC_CODE
#include "cannm_memmap.h"

/*!*********************************************************************************************************************
 * @brief          Initialize the CanNm module.
 * @details        Initialize the CanNm module.
 * @param[in]      cannm_config config ptr
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_init(const cannm_cate_cfg *const cannm_config);

/*!*********************************************************************************************************************
 * @brief          CanNm passive start up.
 * @details        Go to network mode.
 * @param[in]      nm_id channel id
 * @ret_val        E_OK Passive startup successfully
 * @ret_val        E_NOT_OK Passive startup failed
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_pas_stup(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          CanNm request network.
 * @details        Go to network mode.
 * @param[in]      nm_id channel id
 * @ret_val        E_OK network Request successfully
 * @ret_val        E_NOT_OK network Request failed
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_req_net(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          CanNm release network.
 * @details        Go to sleep mode.
 * @param[in]      nm_id channel id
 * @ret_val        E_OK network release successfully
 * @ret_val        E_NOT_OK network release failed
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_rel_net(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          trigger tx api
 * @details        trigger tx message.
 * @param[in]      tx_pdu_id tx PduId
 * @param[in]      pdu_info_ptr pdu_info_ptr
 * @ret_val         E_OK trigger tx ok
 * @ret_val         E_NOT_OK trigger tx not ok
 * @reentrant      Reentrant for different PduIds. Non reentrant for the same PduId.
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_trigger_transmit(PduIdType tx_pdu_id, PduInfoType *pdu_info_ptr);

#if (CANNM_COM_CONTROL_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          CanNm disable comm
 * @details        Disable the NM PDU tx
 * @param[in]      nm_id channel id
 * @ret_val        E_OK disable communication OK
 * @ret_val        E_NOT_OK disable communication not OK
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_disable_comm(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          CanNm enable comm
 * @details        CanNm enable the NM PDU tx
 * @param[in]      nm_id channel id
 * @ret_val        E_OK enable communication OK
 * @ret_val        E_NOT_OK enable communication not OK
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_enable_comm(const network_handle_type nm_id);

#endif
/*!*********************************************************************************************************************
 * @brief          CanNm set user data
 * @details        CanNm set user data.
 * @param[in]      nm_id channel id
 * @param[in]      nm_user_data_ptr Set user data
 * @ret_val        E_OK Set user data OK
 * @ret_val        E_NOT_OK Set user data not OK
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_set_ur_data(const network_handle_type nm_id,
	const uint8 *const nm_user_data_ptr);

/*!*********************************************************************************************************************
 * @brief          CanNm get user data
 * @details        CanNm get user data
 * @param[in]      nm_id channel id
 * @param[in]      nm_user_data_ptr user data
 * @ret_val        E_OK Get user data OK
 * @ret_val        E_NOT_OK Get user data not OK
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_ur_data(const network_handle_type nm_id,
	uint8 *const nm_user_data_ptr);

/*!*********************************************************************************************************************
 * @brief          CanNm get node identifier
 * @details        CanNm get node identifier
 * @param[in]      nm_id channel id
 * @param[in]      nm_node_id_ptr node id
 * @ret_val        E_OK Get node identifier OK
 * @ret_val        E_NOT_OK Get node identifier not OK
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_nd_id(const network_handle_type nm_id,
	uint8 *const nm_node_id_ptr);

/*!*********************************************************************************************************************
 * @brief          CanNm Get local node id
 * @details        CanNm Get local node id
 * @param[in]      nm_id channel id
 * @param[in]      nm_node_id_ptr node id
 * @ret_val        E_OK Get local node identifier OK
 * @ret_val        E_NOT_OK Get local node identifier not OK
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_local_nd_id(const network_handle_type nm_id,
	uint8 *const nm_node_id_ptr);

/*!*********************************************************************************************************************
 * @brief          CanNm repeat msg req
 * @details        CanNm repeat message req
 * @param[in]      nm_id channel id
 * @ret_val        E_OK Repeat message request OK
 * @ret_val        E_NOT_OK Repeat message request not OK
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_req_rpt_msg(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          CanNm get msg data
 * @details        CanNm get msg data
 * @param[in]      nm_id channel id
 * @param[in]      nm_pdu_data_ptr pdu data
 * @ret_val        E_OK Get pdu data OK
 * @ret_val        E_NOT_OK Get pdu data not OK
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_msg_data(const network_handle_type nm_id, uint8 *const nm_pdu_data_ptr);

#if (CONFIG_MODULE_NM == 1)
/*!*********************************************************************************************************************
 * @brief          CanNm get state
 * @details        CanNm get the state and the mode.
 * @param[in]      nm_id channel id
 * @ret_val        E_OK Get state OK
 * @ret_val        E_NOT_OK Get state not OK
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_get_state(const network_handle_type nm_id, nm_state_type *const nm_st,
	nm_mode_type *const nm_md);
#endif

/*!*********************************************************************************************************************
 * @brief          CanNm transmit api
 * @details        CanNm transmit api
 * @param[in]      cannm_user_data_tx_pdu_id User data tx PduId
 * @param[in]      pdu_info_ptr PduInfo
 * @ret_val        E_OK transmit api OK
 * @ret_val        E_NOT_OK transmit api not OK
 * @reentrant      same cannm_user_data_tx_pdu_id non reentrant，different cannm_user_data_tx_pdu_id reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_transmit(PduIdType cannm_user_data_tx_pdu_id, const PduInfoType *pdu_info_ptr);

/*!*********************************************************************************************************************
 * @brief          CanNm get run time
 * @details        CanNm get run time
 * @param[in]      channel_id channel id
 * @param[in]      left_msg_cyc_tm Message cycle time left
 * @param[in]      left_tmot_tm Timeout time left
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_get_chnl_run_time_data(const network_handle_type channel_id,
	uint16 *left_msg_cyc_tm, uint16 *left_tmot_tm);

/*!*********************************************************************************************************************
 * @brief          CanNm confirm Pn
 * @details        CanNm confirm Pn
 * @param[in]      nm_id channel id
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_confirm_pn_availability(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          CanNm get current requested
 * @details        CanNm get current requested
 * @param[in]      channel channel id
 * @ret_val        TRUE Get current requested OK
 * @ret_val        FALSE Get current requested not OK
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(boolean, CANNM_CODE) cannm_monitor_get_current_requested(uint8 channel);

/*!*********************************************************************************************************************
 * @brief          Get CanIf first nm message id
 * @details        CanNm Get CanIf first nm message id
 * @ret_val         0-65535 PduId
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(uint16, CANNM_CODE) cannm_get_canif_first_nm_msg_id(void);

/*!*********************************************************************************************************************
 * @brief          CanNm deinit
 * @details        CanNm deinit api
 * @reentrant      Non Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_de_init(void);

/*!*********************************************************************************************************************
 * @brief          CanNm switches from normal operation to repeat message state to quickly send nm messages
 * @details        CanNm switches from normal operation to repeat message state to quickly send nm messages
 * @param[in]      nm_id channel id
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_nor_opr_to_rpt_msg_imdt_tx(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          CanNm switches from the repeat message state to the repeat message state to quickly send nm messages
 * @details        CanNm switches from the repeat message state to the repeatmessage state to quickly send nm messages
 * @param[in]      nm_id channel id
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_rpt_msg_imdt_tx(const network_handle_type nm_id);

/*!*********************************************************************************************************************
 * @brief          CanNm switches from the ready-to-sleep state to the repeat message state to quickly send nm messages
 * @details        CanNm switches from the ready-to-sleep state to the repeat message state to quickly send nm messages
 * @param[in]      nm_id channel id
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_rpt_msg_imdt_tx(const network_handle_type nm_id);
/*!*********************************************************************************************************************
 * @brief          CanNm switches from one state to another state
 * @details        CanNm switches from one state to another state
 * @param[in]      channel_internal channel internal
 * @param[in]      mode channel internal mode
 * @param[in]      state channel internal state
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_switch_mode_state_proc(cannm_op_chnl_cate *channel_internal, nm_mode_type mode,
	nm_state_type state);
/*!*********************************************************************************************************************
 * @brief          CanNm immediately transmission process
 * @details        CanNm immediately transmission process
 * @param[in]      channel_internal channel internal
 * @reentrant      Reentrant (but not for the same NM-Channel)
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_imdt_tx_proc(cannm_op_chnl_cate *channel_internal);
#define   CANNM_STOP_SEC_CODE
#include "cannm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANNM_H */
