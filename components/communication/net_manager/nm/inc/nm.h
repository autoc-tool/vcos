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

#ifndef NM_H
#define NM_H

#include "Std_Types.h"
#include "comstack_types.h"
#include "nmstack_types.h"
#include "nm_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* for stub */
/*=============== NM_SERVICEID_PASSIVESTARTUP ===============*/
#define NM_NM_SERVICEID_PASSIVESTARTUP_NM_E_UNINIT 0x001d0100UL
#define NM_NM_SERVICEID_PASSIVESTARTUP_NM_E_HANDLE_UNDEF 0x001d0101UL

/*=============== NM_SERVICEID_NETWORKREQUEST ===============*/
#define NM_NM_SERVICEID_NETWORKREQUEST_NM_E_UNINIT 0x001d0200UL
#define NM_NM_SERVICEID_NETWORKREQUEST_NM_E_HANDLE_UNDEF 0x001d0201UL

/*=============== NM_SERVICEID_NETWORKRELEASE ===============*/
#define NM_NM_SERVICEID_NETWORKRELEASE_NM_E_UNINIT 0x001d0300UL
#define NM_NM_SERVICEID_NETWORKRELEASE_NM_E_HANDLE_UNDEF 0x001d0301UL

/*=============== NM_SERVICEID_DISABLECOMMUNICATION ===============*/
#define NM_NM_SERVICEID_DISABLECOMMUNICATION_NM_E_UNINIT 0x001d0400UL
#define NM_NM_SERVICEID_DISABLECOMMUNICATION_NM_E_HANDLE_UNDEF 0x001d0401UL

/*=============== NM_SERVICEID_ENABLECOMMUNICATION ===============*/
#define NM_NM_SERVICEID_ENABLECOMMUNICATION_NM_E_UNINIT 0x001d0500UL
#define NM_NM_SERVICEID_ENABLECOMMUNICATION_NM_E_HANDLE_UNDEF 0x001d0501UL

/*=============== NM_SERVICEID_SETUSERDATA ===============*/
#define NM_NM_SERVICEID_SETUSERDATA_NM_E_UNINIT 0x001d0600UL
#define NM_NM_SERVICEID_SETUSERDATA_NM_E_HANDLE_UNDEF 0x001d0601UL
#define NM_NM_SERVICEID_SETUSERDATA_NM_E_PARAM_POINTER 0x001d0602UL

/*=============== NM_SERVICEID_GETUSERDATA ===============*/
#define NM_NM_SERVICEID_GETUSERDATA_NM_E_UNINIT 0x001d0700UL
#define NM_NM_SERVICEID_GETUSERDATA_NM_E_HANDLE_UNDEF 0x001d0701UL
#define NM_NM_SERVICEID_GETUSERDATA_NM_E_PARAM_POINTER 0x001d0702UL

/*=============== NM_SERVICEID_GETPDUDATA ===============*/
#define NM_NM_SERVICEID_GETPDUDATA_NM_E_UNINIT 0x001d0800UL
#define NM_NM_SERVICEID_GETPDUDATA_NM_E_HANDLE_UNDEF 0x001d0801UL
#define NM_NM_SERVICEID_GETPDUDATA_NM_E_PARAM_POINTER 0x001d0802UL

/*=============== NM_SERVICEID_REPEATMESSAGEREQUEST ===============*/
#define NM_NM_SERVICEID_REPEATMESSAGEREQUEST_NM_E_UNINIT 0x001d0900UL
#define NM_NM_SERVICEID_REPEATMESSAGEREQUEST_NM_E_HANDLE_UNDEF 0x001d0901UL

/*=============== NM_SERVICEID_GETNODEIDENTIFIER ===============*/
#define NM_NM_SERVICEID_GETNODEIDENTIFIER_NM_E_UNINIT 0x001d0a00UL
#define NM_NM_SERVICEID_GETNODEIDENTIFIER_NM_E_HANDLE_UNDEF 0x001d0a01UL
#define NM_NM_SERVICEID_GETNODEIDENTIFIER_NM_E_PARAM_POINTER 0x001d0a02UL

/*=============== NM_SERVICEID_GETLOCALNODEIDENTIFIER ===============*/
#define NM_NM_SERVICEID_GETLOCALNODEIDENTIFIER_NM_E_UNINIT 0x001d0b00UL
#define NM_NM_SERVICEID_GETLOCALNODEIDENTIFIER_NM_E_HANDLE_UNDEF 0x001d0b01UL
#define NM_NM_SERVICEID_GETLOCALNODEIDENTIFIER_NM_E_PARAM_POINTER 0x001d0b02UL

/*=============== NM_SERVICEID_CHECKREMOTESLEEPINDICATION ===============*/
#define NM_NM_SERVICEID_CHECKREMOTESLEEPINDICATION_NM_E_UNINIT 0x001d0d00UL
#define NM_NM_SERVICEID_CHECKREMOTESLEEPINDICATION_NM_E_HANDLE_UNDEF 0x001d0d01UL
#define NM_NM_SERVICEID_CHECKREMOTESLEEPINDICATION_NM_E_PARAM_POINTER 0x001d0d02UL

/*=============== NM_SERVICEID_GETSTATE ===============*/
#define NM_NM_SERVICEID_GETSTATE_NM_E_UNINIT 0x001d0e00UL
#define NM_NM_SERVICEID_GETSTATE_NM_E_HANDLE_UNDEF 0x001d0e01UL
#define NM_NM_SERVICEID_GETSTATE_NM_E_PARAM_POINTER 0x001d0e02UL

/*=============== NM_SERVICEID_MAINFUNCTION ===============*/
#define NM_NM_SERVICEID_MAINFUNCTION_NM_E_UNINIT 0x001d1000UL

/*=============== NM_SERVICEID_NETWORKSTARTINDICATION ===============*/
#define NM_NM_SERVICEID_NETWORKSTARTINDICATION_NM_E_UNINIT 0x001d1100UL
#define NM_NM_SERVICEID_NETWORKSTARTINDICATION_NM_E_HANDLE_UNDEF 0x001d1101UL

/*=============== NM_SERVICEID_NETWORKMODE ===============*/
#define NM_NM_SERVICEID_NETWORKMODE_NM_E_UNINIT 0x001d1200UL
#define NM_NM_SERVICEID_NETWORKMODE_NM_E_HANDLE_UNDEF 0x001d1201UL

/*=============== NM_SERVICEID_PREPAREBUSSLEEPMODE ===============*/
#define NM_NM_SERVICEID_PREPAREBUSSLEEPMODE_NM_E_UNINIT 0x001d1300UL
#define NM_NM_SERVICEID_PREPAREBUSSLEEPMODE_NM_E_HANDLE_UNDEF 0x001d1301UL

/*=============== NM_SERVICEID_BUSSLEEPMODE ===============*/
#define NM_NM_SERVICEID_BUSSLEEPMODE_NM_E_UNINIT 0x001d1400UL
#define NM_NM_SERVICEID_BUSSLEEPMODE_NM_E_HANDLE_UNDEF 0x001d1401UL

/*=============== NM_SERVICEID_PDURXINDICATION ===============*/
#define NM_NM_SERVICEID_PDURXINDICATION_NM_E_UNINIT 0x001d1500UL
#define NM_NM_SERVICEID_PDURXINDICATION_NM_E_HANDLE_UNDEF 0x001d1501UL

/*=============== NM_SERVICEID_STATECHANGENOTIFICATION ===============*/
#define NM_NM_SERVICEID_STATECHANGENOTIFICATION_NM_E_UNINIT 0x001d1600UL
#define NM_NM_SERVICEID_STATECHANGENOTIFICATION_NM_E_HANDLE_UNDEF 0x001d1601UL

/*=============== NM_SERVICEID_REMOTESLEEPINDICATION ===============*/
#define NM_NM_SERVICEID_REMOTESLEEPINDICATION_NM_E_UNINIT 0x001d1700UL
#define NM_NM_SERVICEID_REMOTESLEEPINDICATION_NM_E_HANDLE_UNDEF 0x001d1701UL

/*=============== NM_SERVICEID_REMOTESLEEPCANCELLATION ===============*/
#define NM_NM_SERVICEID_REMOTESLEEPCANCELLATION_NM_E_UNINIT 0x001d1800UL
#define NM_NM_SERVICEID_REMOTESLEEPCANCELLATION_NM_E_HANDLE_UNDEF 0x001d1801UL

/*=============== NM_SERVICEID_REMOTESYNCHRONISATIONPOINT ===============*/
#define NM_NM_SERVICEID_REMOTESYNCHRONISATIONPOINT_NM_E_UNINIT 0x001d1900UL
#define NM_NM_SERVICEID_REMOTESYNCHRONISATIONPOINT_NM_E_HANDLE_UNDEF 0x001d1901UL

/*=============== NM_SERVICEID_REPEATMESSAGEINDICATION ===============*/
#define NM_NM_SERVICEID_REPEATMESSAGEINDICATION_NM_E_UNINIT 0x001d1a00UL
#define NM_NM_SERVICEID_REPEATMESSAGEINDICATION_NM_E_HANDLE_UNDEF 0x001d1a01UL

/*=============== NM_SERVICEID_TXTIMEOUTEXCEPTION ===============*/
#define NM_NM_SERVICEID_TXTIMEOUTEXCEPTION_NM_E_UNINIT 0x001d1b00UL
#define NM_NM_SERVICEID_TXTIMEOUTEXCEPTION_NM_E_HANDLE_UNDEF 0x001d1b01UL

/*=============== NM_SERVICEID_CARWAKEUPINDICATION ===============*/
#define NM_NM_SERVICEID_CARWAKEUPINDICATION_NM_E_UNINIT 0x001d1d00UL
#define NM_NM_SERVICEID_CARWAKEUPINDICATION_NM_E_HANDLE_UNDEF 0x001d1d01UL

/*=============== NM_SERVICEID_COORDREADYTOSLEEPINDICATION ===============*/
#define NM_NM_SERVICEID_COORDREADYTOSLEEPINDICATION_NM_E_UNINIT 0x001d1e00UL
#define NM_NM_SERVICEID_COORDREADYTOSLEEPINDICATION_NM_E_HANDLE_UNDEF 0x001d1e01UL
/* for stub */

#define NM_DET_REPORT(errid)

#define  NM_START_SEC_CODE
#include "nm_memmap.h"

/*!*********************************************************************************************************************
 * @brief          Nm Init
 * @details        Initialize Nm
 * @reentrant      Non Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_init(void);

/*!*********************************************************************************************************************
 * @brief          Nm deinit
 * @details        Nm deinit
 * @reentrant      Non Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_de_init(void);

/*!*********************************************************************************************************************
 * @brief          Nm passive start up
 * @details        Nm passive start up
 * @param[in]      id_net Communication channel handle
 * @ret_val        E_OK           Passive start successful
 * @ret_val        E_NOT_OK       Passive start failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_pas_stup(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Nm request comm
 * @details        Nm request comm
 * @param[in]      id_net Communication channel handle
 * @ret_val        E_OK           Request bus communication successful
 * @ret_val        E_NOT_OK       Bus communication request failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_req_net(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Nm release comm
 * @details        Nm release comm
 * @param[in]      id_net Communication channel handle
 * @ret_val        E_OK           Successfully released network
 * @ret_val        E_NOT_OK       Failed to release network
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_rel_net(const network_handle_type id_net);

#if (NM_COM_CONTROL_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          Nm disable comm
 * @details        Nm disable comm
 * @param[in]      id_net Communication channel handle
 * @ret_val        E_OK           Close successfully
 * @ret_val        E_NOT_OK       Closing failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_disable_comm(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Nm enable comm
 * @details        Nm enable comm
 * @param[in]      id_net Communication channel handle
 * @ret_val        E_OK           Successfully enabled
 * @ret_val        E_NOT_OK       Enabling failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_enable_comm(const network_handle_type id_net);
#endif

#if defined(NM_COM_USER_DATA_ENABLED) && (NM_COM_USER_DATA_ENABLED == STD_OFF) && (NM_USER_DATA_ENABLED == STD_ON)
&& defined(NM_PASSIVE_MODE) && (NM_PASSIVE_MODE == STD_OFF)
/*!*********************************************************************************************************************
 * @brief          Nm set user data
 * @details        Nm set user data
 * @param[in]      id_net Communication channel handle
 * @param[in]      data_ur_ptr Pointer to user-defined data
 * @ret_val        E_OK          Setting successful
 * @ret_val        E_NOT_OK      Setting failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_set_ur_data(const network_handle_type id_net,
	const uint8 *const data_ur_ptr);
#endif

/*!*********************************************************************************************************************
 * @brief          Nm get user data
 * @details        Nm get user data
 * @param[in]      id_net Communication channel handle
 * @param[out]     data_ur_ptr Pointer to store user-defined data obtained
 * @ret_val         E_OK          Successfully obtained
 * @ret_val         E_NOT_OK      Acquisition failed
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_get_ur_data(const network_handle_type id_net,
	uint8 *const data_ur_ptr);

/*!*********************************************************************************************************************
 * @brief          Nm get pdu data
 * @details        Nm get pdu data
 * @param[in]      id_net Communication channel handle
 * @param[out]     nm_pdu_data Pointer to store the obtained PDU data
 * @ret_val         E_OK       Successfully obtained
 * @ret_val         E_NOT_OK   Acquisition failed
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_get_msg_data(const network_handle_type id_net, uint8 *const nm_pdu_data);

/*!*********************************************************************************************************************
 * @brief          Nm repeat msg request
 * @details        Nm repeat msg request
 * @param[in]      id_net Communication channel handle
 * @ret_val        E_OK          Setting successful
 * @ret_val        E_NOT_OK      Setting failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_req_rpt_msg(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Nm get node id
 * @details        Nm get node id
 * @param[in]      id_net Communication channel handle
 * @param[out]     nm_node_id_ptr Store the pointer of the obtained node ID
 * @ret_val        E_OK        Successfully obtained
 * @ret_val        E_NOT_OK    Acquisition failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_get_nd_id(const network_handle_type id_net,
	uint8 *const nm_node_id_ptr);

/*!*********************************************************************************************************************
 * @brief          Nm get local node id
 * @details        Nm get local node id
 * @param[in]      id_net Communication channel handle
 * @param[out]     nm_node_id_ptr Store the pointer of the obtained local node ID
 * @ret_val         E_OK        Successfully obtained
 * @ret_val         E_NOT_OK    Acquisition failed
 * @reentrant      Different network handles can be reentrant, but the same network handle cannot be reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_get_local_nd_id(const network_handle_type id_net,
	uint8 *const nm_node_id_ptr);

/*!*********************************************************************************************************************
 * @brief          Nm get state
 * @details        Nm get state
 * @param[in]      id_net Communication channel handle
 * @param[out]     nm_st Pointer for storing status
 * @param[out]     nm_md Pointer for storage mode
 * @ret_val         E_OK       Successfully obtained
 * @ret_val         E_NOT_OK   Acquisition failed
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NM_CODE) nm_get_state(const network_handle_type id_net,
	nm_state_type *const nm_st, nm_mode_type *const nm_md);

/*!*********************************************************************************************************************
 * @brief          Nm start indication
 * @details        Nm start indication
 * @param[in]      id_net Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_start_net_ind(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Nm network mode
 * @details        Nm network mode
 * @param[in]      id_net Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_mode_net_proc(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Nm prepare bus sleep mode
 * @details        Nm prepare bus sleep mode
 * @param[in]      id_net Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_mode_pbs_proc(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Nm bus sleep mode
 * @details        Nm bus sleep mode
 * @param[in]      id_net Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_mode_bs_proc(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          Notify synchronization status
 * @details        Notify synchronization status
 * @param[in]      id_net Communication channel handle
 * @param[in]      pdu_info_ptr Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_rx_pdu_ind(network_handle_type id_net , const PduInfoType *pdu_info_ptr);

#if (NM_STATE_CHANGE_IND_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          state change notification
 * @details        Used to notify network management of changes in state machine status
 * @param[in]      id_net Communication channel handle
 * @param[in]      nm_pre_state Previous state
 * @param[in]      nm_cur_state Current status
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_ntfy_state_chg(const network_handle_type id_net,
	const nm_state_type nm_pre_state, const nm_state_type nm_cur_state);

/*!*********************************************************************************************************************
 * @brief          state change notification
 * @details        Used to notify network management of changes in state machine status
 * @param[in]      id_net Communication channel handle
 * @param[in]      nm_pre_state Previous state
 * @param[in]      nm_cur_state Current status
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
void nm_ntfy_state_chg_bswm(const network_handle_type id_net,
	const nm_state_type nm_pre_state, const nm_state_type nm_cur_state);
#endif
/*!*********************************************************************************************************************
 * @brief          Repeated message request notification
 * @details        Notification can be configured to enable
 * @param[in]      id_net Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_rpt_msg_ind(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          NM message sending timeout exception notification
 * @details        This function is called by a specific bus NM module, and it can be configured to be enabled.
 * @param[in]      id_net Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Asynchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_tx_tmot_excpt_pro(const network_handle_type id_net);

/*!*********************************************************************************************************************
 * @brief          User defined state change callback function
 * @details        It is called by a specific bus NM module, and the notification is configurable.
 * @param[in]      id_net Communication channel handle
 * @param[in]      nm_pre_state Previous state
 * @param[in]      nm_cur_state Current status
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_state_chg_cbk(const network_handle_type id_net,
	const nm_state_type nm_pre_state, const nm_state_type nm_cur_state);

/*!*********************************************************************************************************************
 * @brief          User defined PduRx_indication callback function
 * @details        The function is called by a specific bus NM module, and the notification is configurable.
 * @param[in]      id_net Communication channel handle
 * @param[in]      pdu_info_ptr 指向接收到的PDU信息的指针
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_pdu_rx_cbk(network_handle_type id_net, const PduInfoType *pdu_info_ptr);

/*!*********************************************************************************************************************
 * @brief          User defined PduRx_TotalIndication callback function
 * @details        The specific bus NM module calls the user-defined function, and the notification is configurable.
 * @param[in]      id_net Communication channel handle
 * @reentrant      Reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, NM_CODE) nm_pdu_rx_total_ind(network_handle_type id_net);

#define  NM_STOP_SEC_CODE
#include "nm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* NM_H */

