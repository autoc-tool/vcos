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

#ifndef CANNM_INTERNAL_H
#define CANNM_INTERNAL_H

#if (CANNM_DEV_ERROR_DETECT == STD_ON) && defined(VCOS_MODULE_CONFIG_DET) && (VCOS_MODULE_CONFIG_DET == 1)
#include "Det.h"
#endif
#if (CONFIG_MODULE_NM == 1)
#include "nmstack_types.h"
#endif
#include "rtfw_atomic.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define CANNM_DET_REPORT(errid)

#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
#define CANNM_SPONTANEOUS_TRANSMISSION                      ((uint8)(3u))     ///< Automatically send three times
#define CANNM_ONGOING_TRANSMISSION                          ((uint8)(1u))     ///< Sending 1 time
#define CANNM_NO_TRANSMISSION                               ((uint8)(0u))     ///< Not sent 0 times
#endif

#define CANNM_CBV_REPEAT_MESSAGE_REQUEST                    ((uint8)(0x01u))  ///< CBV Repeat Message Request Bit
#define CANNM_CBV_NM_COORDINATOR_SLEEP                      ((uint8)(0x08u))  ///< CBV nm coordinator sleep bit
#define CANNM_CBV_ACTIVE_WAKEUP                             ((uint8)(0x10u))  ///< CBV channel_en wake-up bit
#define CANNM_CBV_PNI                                       ((uint8)(0x40u))  ///< CBV local nm information bits
#define CANNM_LSBIT_MASK                                    ((uint8)(0x1u))   ///< Minimum bit mask
#define CANNM_PDU_LENGTH                                    ((uint8)(8u))     ///< CanNm message length
#define CANNM_MAX_BIT_IN_ONE_BYTE                           ((uint8)(8u))     ///< CanNm bit counter in one byte
#define CANNM_INVALID_VALUE                                 ((uint8)(0xFFu))  ///< CanNm user data default value
/*!*********************************************************************************************************************
 * @brief   Defines the type of the initialization state of CanNm
 **********************************************************************************************************************/
typedef uint8 cannm_init_status_cate;
#define CANNM_STATUS_UNINIT                                 ((cannm_init_status_cate)0x00u)  ///< uninitialized state
#define CANNM_STATUS_INIT                                   ((cannm_init_status_cate)0x01u)  ///< initialization state

/*!*********************************************************************************************************************
 * @brief   Defines the type of the internal channel of CanNm
 **********************************************************************************************************************/
typedef struct {
#if (CANNM_COM_USER_DATA_SUPPORT == STD_ON)
	uint8                       tx_sts;                          ///< transmission status
	uint8                       spts_tx_msg[CANNM_PDU_LENGTH]; ///< spontaneous tx message sdu
#elif (CANNM_USER_DATA_ENABLED == STD_ON)
	boolean                     is_pdu_set:1;                            ///< Whether to set user data
#endif
	uint8                       imdt_tx_sd;               ///< Send immediately
	uint16                      left_tmot_tm;                             ///< Timeout remaining time
	uint16                      left_rpt_msg_tm;                      ///< Repeat message remaining time
	uint16                      left_wait_bs_tm;                      ///< Wait for bus sleep time
	uint16                      left_msg_cyc_tm;                       ///< Remaining time of loop message
	uint16                      left_msg_cyc_ofst_tm;                ///< Cyclic message offset time
	uint16                      left_msg_tmot_tm;                     ///< Message timeout remaining time
	nm_mode_type                 mode;                                         ///< network mode
	nm_state_type                state;                                        ///< network status

	boolean                     support_comm:1;                       ///< Whether communication is enabled
	boolean                     filter_pdu:1;                   ///< Whether message filter enabled
	boolean                     requested:1;                                   ///< Whether network request
	boolean                     imdt_en_mode:1;                       ///< Send now mode activated
	uint8                       pdu_tx[CANNM_PDU_LENGTH];              ///< Send Message Sdu
	uint8                       pdu_rx[CANNM_PDU_LENGTH];              ///< Receive message Sdu
} cannm_op_chnl_cate;

/*!*********************************************************************************************************************
 * @brief   Defines the internal type of CanNm
 **********************************************************************************************************************/
typedef struct {
	cannm_init_status_cate        init_status;                    ///< initialization state
	cannm_op_chnl_cate  nm_tunnel[CANNM_CHANNEL_COUNT];   ///< Channel number
} cannm_internal_type;

/*!*********************************************************************************************************************
 * @brief   Defines the statistics type of CanNm
 **********************************************************************************************************************/
typedef struct {
	Atomic     tx_ok_msg_cnt;                     ///< Statistics of successfully sent messages
	Atomic     tx_not_ok_msg_cnt;                 ///< Statistics of failed message sending
	Atomic     tt_tx_ok_msg_cnt;                  ///< Triggering statistics of successfully sent messages
	Atomic     tt_tx_not_ok_msg_cnt;              ///< Trigger sending failure message statistics
	Atomic     txconf_ok_msg_cnt;         ///< Statistics of successful sending confirmed messages
	Atomic     rx_ok_msg_cnt;                     ///< Statistics of successfully received messages
} cannm_msg_cnt_cate;

/*!*********************************************************************************************************************
 * @brief   Declare CanNm statistics
 **********************************************************************************************************************/
extern cannm_msg_cnt_cate cannm_msg_cnt;

#define CANNM_START_SEC_CODE
#include "cannm_memmap.h"
/*!*********************************************************************************************************************
 * @brief          CanNm Timeout Timer
 * @details        CanNm Timeout Timer
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant       Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_tmot_time_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm Repeat Message Timer
 * @details        CanNm Repeat Message Timer
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rpt_msg_time_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm waits for bus sleep mode processing flow
 * @details        In the main function, CanNm waits for the bus sleep mode processing flow
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_mode_pbs_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm working mode processing flow
 * @details        In the main function, CanNm working mode processing flow
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function perio
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_mode_net_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm message loop timer
 * @details        CanNm message loop timer
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_msg_cyc_time_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm send timeout timer
 * @details        CanNm send timeout timer
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_tx_tmot_troc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm main function
 * @details        CanNm main function
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_m_func_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm main function
 * @details        CanNm main function
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_mtpl_m_func_proc(void);

#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          CanNm clear flag
 * @details        CanNm clears the repeat message request flag in Pdu
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_clr_rpt_msg_req_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);
#endif

/*!*********************************************************************************************************************
 * @brief          CanNm send message interface
 * @details        CanNm send message interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_tx_msg(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          CanNm gets user data offset
 * @details        CanNm gets user data offset
 * @param[out]     channel_conf Configure channel data
 * @ret_val         0-0xFFFFFFFF PduLengthType
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(PduLengthType, CANNM_CODE) cannm_ur_data_ofst_get(const cannm_chnl_cate *channel_conf);

/*!*********************************************************************************************************************
 * @brief          CanNm gets user data
 * @details        CanNm gets user data
 * @param[in]      channel_conf Configure channel data
 * @param[out]     MessageSduPtr message length pointer
 * @ret_val         0-255 Message Sdu data first address
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC_P2VAR(uint8, AUTOMATIC, CANNM_CODE) cannm_ur_data_ptr_get(const cannm_chnl_cate *channel_conf,
	uint8 *MessageSduPtr);

#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          CanNm receives data normally
 * @details        CanNm receives data normally
 * @param[in]      channel_conf Configure channel data
 * @param[in]      pni PN flag
 * @ret_val         E_OK CanNm receives data successfully
 * @ret_val         E_NOT_OK CanNm failed to receive data normally
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_nor_rx_proc(const cannm_chnl_cate *channel_conf,
	boolean pni);
/*!*********************************************************************************************************************
 * @brief          CanNm discards received data
 * @details        CanNm discards received data
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      pni PN flag
 * @ret_val         E_OK CanNm receives data successfully
 * @ret_val         E_NOT_OK CanNm failed to receive data normally
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_dscd_rx_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean pni);
#endif
/*!*********************************************************************************************************************
 * @brief          CanNm gets user data
 * @details        CanNm gets user data
 * @param[in]      channel_conf Configure channel data
 * @ret_val         0-0xFFFFFFFF PduLengthType
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(PduLengthType, CANNM_CODE) cannm_ur_data_len_get(const cannm_chnl_cate *channel_conf);

/*!*********************************************************************************************************************
 * @brief          Prepare bus sleep to repeat message state interface
 * @details        Prepare bus sleep to repeat message state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      is_nw_req Whether network request
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_pbs_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean is_nw_req);

/*!*********************************************************************************************************************
 * @brief          Prepare bus sleep to bus sleep interface
 * @details        Prepare bus sleep to bus sleep interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_pbs_to_bs_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Bus sleep to repeated message state interface
 * @details        Bus sleep to repeated message state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      is_nw_req Whether network request
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_bs_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean is_nw_req);

/*!*********************************************************************************************************************
 * @brief          Bus-sleep to bus-sleep interface
 * @details        Bus-sleep to bus-sleep interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_bs_to_bs_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Duplicate message status to duplicate message status interface
 * @details        Duplicate message status to duplicate message status interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Repeat message state to ready sleep state interface
 * @details        Repeat message state to ready sleep state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_rdy_slp_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Repeat message status to normal operation status interface
 * @details        Repeat message status to normal operation status interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rpt_msg_to_nor_opr_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

#if ((CANNM_NODE_DETECTION_ENABLED == STD_ON) || (STD_OFF != CANNM_PN_HANDLE_MULTIPLE_NETWORK_REQUESTS))

/*!*********************************************************************************************************************
 * @brief          Normal operation state to repeated message state interface
 * @details        Normal operation state to repeated message state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_nor_opr_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);
#endif

/*!*********************************************************************************************************************
 * @brief          Normal operation state to ready sleep state interface
 * @details        Normal operation state to ready sleep state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_nor_opr_to_rdy_slp_proc( const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Normal operation state to normal operation state interface
 * @details        Normal operation state to normal operation state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_nor_opr_to_nor_opr_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Ready sleep state to normal operation state interface
 * @details        Ready sleep state to normal operation state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_nor_opr_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Ready sleep state to prepare bus sleep interface
 * @details        Ready sleep state to prepare bus sleep interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_pbs_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

#if ((CANNM_NODE_DETECTION_ENABLED == STD_ON) || (STD_OFF != CANNM_PN_HANDLE_MULTIPLE_NETWORK_REQUESTS))

/*!*********************************************************************************************************************
 * @brief          Ready sleep state to repeat message state interface
 * @details        Ready sleep state to repeat message state interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rdy_slp_to_rpt_msg_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);
#endif

/*!*********************************************************************************************************************
 * @brief          Netmode to Netmode Interface
 * @details        Netmode to Netmode Interface
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_net_mode_to_net_mode_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);
#if (CANNM_ACTIVE_WUP_BIT_FEATURE_ENABLED == STD_ON)

/*!*********************************************************************************************************************
 * @brief          Set the channel_en wake-up bit in the control bit vector
 * @details        Set the channel_en wake-up bit in the control bit vector
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_set_active_wk_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Clear the channel_en wakeup bit in the control bit vector
 * @details        Clear the channel_en wakeup bit in the control bit vector
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_clr_active_wk_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);
#endif

#if (CANNM_NODE_DETECTION_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          Receive network management messages in CanNm network mode
 * @details        Receive network management messages in CanNm network mode
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      repeat_message_bit_indication Repeat message bit indication
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_net_mode_rx_ind_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean repeat_message_bit_indication);
#endif

/*!*********************************************************************************************************************
 * @brief          CanNm processing flow in duplicate message state
 * @details        CanNm processing flow in duplicate message state
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_state_rpt_msg_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Processing flow under normal CanNm operation
 * @details        Processing flow under normal CanNm operation
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_state_nor_opr_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Processing flow in CanNm ready sleep state
 * @details        Processing flow in CanNm ready sleep state
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      CanNmMainFunctionPeriod main function period
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_state_rdy_slp_handling(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          Set the local network flag in the control bit vector
 * @details        Set the local network flag in the control bit vector
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_set_pni_cbv(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal);

/*!*********************************************************************************************************************
 * @brief          Package receiving process
 * @details        Package receiving process
 * @param[in]      channel_conf Configure channel data
 * @param[in]      channel_internal Channel number
 * @param[in]      pni local network identifier
 * @reentrant      Non reentrant
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANNM_CODE) cannm_rx_proc(const cannm_chnl_cate *channel_conf,
	cannm_op_chnl_cate *channel_internal, boolean pni);
#endif

#define   CANNM_STOP_SEC_CODE
#include "cannm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANNM_INTERNAL_H */
