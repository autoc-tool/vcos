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

#ifndef CANSM_TYPES_H
#define CANSM_TYPES_H

#include "comstack_types.h"
#include "comm.h"
#if defined(CONFIG_MODULE_DEM) && (CONFIG_MODULE_DEM == 1)
#include "dem.h"
#endif
#if (CONFIG_MODULE_CANIF == 1)
#include "canif.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   Define Busoff start notification
 **********************************************************************************************************************/
typedef void(*cansm_bus_off_begin_notify)(uint8 network_handle);

/*!*********************************************************************************************************************
 * @brief   Define Busoff End Notification
 **********************************************************************************************************************/
typedef void(*cansm_bus_off_end_notify)(uint8 network_handle);

/*!*********************************************************************************************************************
 * @brief   Internal initialization status of CanSM
 **********************************************************************************************************************/
typedef uint8 cansm_internal_init_status_type;

/*!*********************************************************************************************************************
 * @brief   Basic state Machine (BSM) type definition for CanSM
 **********************************************************************************************************************/
typedef uint8 cansm_internal_bsm_state_type;

/*!*********************************************************************************************************************
 * @brief   Definition of sub state types for Pre NoComm state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_pre_no_comm_state_type;

/*!*********************************************************************************************************************
 * @brief   Definition of sub state types for Pre FullComm state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_pre_full_comm_state_type;

/*!*********************************************************************************************************************
 * @brief   Definition of sub state types for FullComm state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_full_comm_state_type;

/*!*********************************************************************************************************************
 * @brief   Definition of Substate Types for FullComm_TX_TIMEOUT_deCEPTION Status
 **********************************************************************************************************************/
typedef uint8 cansm_internal_tx_timeout_exception_state_type;

/*!*********************************************************************************************************************
 * @brief   Definition of sub state types for WuValidation state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_wu_validation_state_type;

/*!*********************************************************************************************************************
 * @brief   Exit type of Pre NoCom sub state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_pre_no_comm_exit_point_type;

/*!*********************************************************************************************************************
 * @brief   Exit type of Pre FullComm sub state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_pre_full_comm_exit_point_type;

/*!*********************************************************************************************************************
 * @brief   Exit type of FullComm sub state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_full_comm_exit_point_type;

/*!*********************************************************************************************************************
 * @brief   Exit type of SilentCommBor sub state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_silent_comm_bor_exit_point_type;

/*!*********************************************************************************************************************
 * @brief   Exit type of WuValidation sub state
 **********************************************************************************************************************/
typedef uint8 cansm_internal_wu_validation_exit_point_type;

/*!*********************************************************************************************************************
 * @brief   Definition of sub state switching type
 **********************************************************************************************************************/
typedef uint8 cansm_internal_transition_type;

/*!*********************************************************************************************************************
 * @brief   Definition of return value for sub state switching process
 **********************************************************************************************************************/
typedef uint8 cansm_internal_transition_return_type;

/*!*********************************************************************************************************************
 * @brief   NM network handle type definition
 **********************************************************************************************************************/
typedef uint8 network_handle_type;

/*!*********************************************************************************************************************
 * @brief   Define network Type
 **********************************************************************************************************************/
typedef struct {
	const boolean can_en_trcv;              ///< Is the CAN transceiver available
	const boolean cantrcv_en_pn;              ///< Does the CAN transceiver support PN
	const network_handle_type id_net; ///< Handle of ComM
	const uint8 cansm_l1_to_l2_bor_cnt;        ///< Number of times Bor switches from fast to slow recovery
	const uint8 canif_id_ctrl;               ///< CAN Controller Id
	const uint8 canif_id_trcv;              ///< CAN transceiver ID
	const uint8 cansm_val_comp;          ///< Time compensation during BUSOFF setting
	const uint16 cansm_req_md_rept_tm; ///< Maximum RequestRepetition Duration
	const uint16 cansm_en_bor_time_tx;          ///< Bor's TimeTxEnsured duration
	const uint16 cansm_l1_bor_tm;                 ///< Bor's L1 duration
	const uint16 cansm_l2_bor_tm;                 ///< Bor's L2 duration
#if defined(CONFIG_MODULE_DEM) && (CONFIG_MODULE_DEM == 1)
	dem_event_id_type cansm_bsof_dem_evt_id;       ///< BusOff的EventId
#endif
} cansm_network_type;

/*!*********************************************************************************************************************
 * @brief   Define the network type for each core
 **********************************************************************************************************************/
typedef struct {
	const cansm_network_type *net_cfg_tbl_add; ///< The address of the corresponding resource of certain core
	uint8 network_cfg_tbl_len;                      ///< The length of the corresponding resource of certain core
} cansm_network_config_type;

/*!*********************************************************************************************************************
 * @brief   Controller mode definition
 **********************************************************************************************************************/
typedef uint8 cansm_canif_ctrl_mode;

/*!*********************************************************************************************************************
 * @brief   Definition of state Structure Related to CanSM Internal Controller
 **********************************************************************************************************************/
typedef struct {
	uint8 controller_id;                                     ///< CAN Controller Id
	boolean has_pending_ctrl_indication;                     ///< Identify whether there are pending controller mode
	cansm_canif_ctrl_mode pending_ctrl_mode;                 ///< Controller mode for hanging up
	cansm_canif_ctrl_mode ind_ctrl_mode;                     ///< Received controller mode indication
} cansm_internal_ctrl_status_type;

/*!*********************************************************************************************************************
 * @brief   Definition of state Structures Related to CanSM Internal Transceivers
 **********************************************************************************************************************/
typedef struct {
	uint8 trcv_id;                                           ///< Transceiver ID
	boolean has_pend_trcv_ind;                       ///< Identify there are pending transceiver mode
	boolean check_wuf_pend_ind;                      ///< Check whether there are wkf pending indication
	boolean clear_wuf_pend_ind;                      ///< Clean whether there are wkf pending indication
	CanTrcv_TrcvModeType pend_trcv_mode;                   ///< On hook transceiver mode
	CanTrcv_TrcvModeType ind_trcv_mode;                       ///< Received transceiver mode indication
} cansm_internal_trcv_status_type;

/*!*********************************************************************************************************************
 * @brief   Global network state Definition of CanSM
 **********************************************************************************************************************/
typedef struct {
	boolean start_wake_up_flag;                                ///< Activate wake-up resource identifier
	boolean stop_wake_up_flag;                                 ///< Stop awakening resource identifier
	comm_mode_type req_mode;                            ///< ComM's communication mode request
	comm_mode_type cur_com_mode;                           ///< Current communication mode of CanSM
} cansm_external_network_type;

/*!*********************************************************************************************************************
 * @brief   CanSM Multi core Channel Information Table
 **********************************************************************************************************************/
typedef struct {
	cansm_external_network_type* network_table_addr;        ///< Address of corresponding resource of certain core
	network_handle_type network_table_len;                  ///< Length of corresponding resource of certain core
} cansm_global_network_core_table_type;

/*!*********************************************************************************************************************
 * @brief   Internal network state Definition of CanSM
 **********************************************************************************************************************/
typedef struct {
	boolean is_stop_loop;                                     ///< Do you want to stop the Do While loop
	boolean bus_off_event;                                    ///< Is BusOff appearing
	boolean bus_off_status;                                   ///< BusOff status
	boolean request_accepted;                                ///< Identify whether the request has been accepted
	boolean tx_timeout_flag;                                  ///< Tx timeout identifier
	uint8 repeat_counter;                                    ///< Repeat count
	uint16 bus_off_counter;                                   ///< Counting of BusOff
	uint16 sub_state_timer;                                   ///< Count of current sub states
	uint16 bus_off_time;                                      ///< The current duration of BusOff
	cansm_internal_bsm_state_type bsm_state;                   ///< The Bsm state of CanSM
	cansm_internal_pre_no_comm_state_type pre_no_comm_state;       ///< The sub states of PreNoComm
	cansm_internal_pre_full_comm_state_type pre_full_comm_state;   ///< The sub states of PreFullComm
	cansm_internal_full_comm_state_type full_comm_state;         ///< The sub states of FullComm
	cansm_internal_wu_validation_state_type wu_validation_state; ///< Sub states of WuValidation
	cansm_internal_tx_timeout_exception_state_type tx_timeout_exception_state; ///< FullComm_TX_TIMEOUT_deCEPTION
	cansm_internal_ctrl_status_type controller_mode_buf;        ///< controller status
	cansm_internal_trcv_status_type transceiver_mode_buf;       ///< Transceiver status
	const cansm_network_type *cfg_info_ptr;                 ///< Configuration information content
} cansm_internal_network_type;

/*!*********************************************************************************************************************
 * @brief   CanSM Multi core Channel Information Table
 **********************************************************************************************************************/
typedef struct {
	cansm_internal_network_type *network_table_addr;     ///< The address of the corresponding resource of a core
	network_handle_type network_table_len;                ///< The length of the corresponding resource of a core
} cansm_network_core_table_type;

/*!*********************************************************************************************************************
 * @brief   Define the data structure of parameters after CanSM construction
 **********************************************************************************************************************/
typedef struct {
	const uint8 cansm_mode_request_repetition_max;   ///< Maximum RequestRepetition Times
	const network_handle_type nm_chnl_trans_size;      ///< Number of channel mappings between CanSM and ComM
	const network_handle_type *nm_chnl_trans;         ///< Channel Mapping between CanSM and ComM
	const cansm_network_config_type *networks;     ///< network configuration of CanSM
	cansm_bus_off_begin_notify bus_off_begin_fnc;      ///< Busoff start notification
	cansm_bus_off_end_notify bus_off_end_fnc;          ///< Busoff End Notification
} cansm_config_type;

#ifdef __cplusplus
}
#endif

#endif /* CANSM_TYPES_H */

