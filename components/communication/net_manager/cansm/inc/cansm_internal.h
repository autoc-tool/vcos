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

#ifndef CANSM_INTERNAL_H
#define CANSM_INTERNAL_H

#include "cansm.h"
#if (CONFIG_MODULE_CANIF == 1)
#include "canif.h"
#endif

#if defined(CANSM_DEV_ERROR_DETECT) && (CANSM_DEV_ERROR_DETECT == STD_ON) && defined(VCOS_MODULE_CONFIG_DET)
&& (VCOS_MODULE_CONFIG_DET == 1)
#include "Det.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CANSM_DET_REPORTERROR(_error)

/*!*********************************************************************************************************************
 * @brief   cansm_internal_init_status_type
 **********************************************************************************************************************/
#define  CANSM_STATUS_UNINIT                                ((uint8)0u)  ///< CanSM is not currently initialized
#define  CANSM_STATUS_INIT                                  ((uint8)1u)  ///< CanSM has now completed initialization

/*!*********************************************************************************************************************
 * @brief   cansm_internal_bsm_state_type
 **********************************************************************************************************************/
#define  CANSM_BSM_S_NOT_INITIALIZED                        ((uint8)0u)  ///< uninitialized state
#define  CANSM_BSM_S_PRE_NOCOM                              ((uint8)1u)  ///< Pre NoCom status
#define  CANSM_BSM_S_NOCOM                                  ((uint8)2u)  ///< NoCom  status
#define  CANSM_BSM_S_PRE_FULLCOM                            ((uint8)3u)  ///< Pre FullComm  status
#define  CANSM_BSM_S_FULLCOM                                ((uint8)4u)  ///< FullComm status
#define  CANSM_BSM_S_SILENTCOM                              ((uint8)5u)  ///< SilentCom  status
#define  CANSM_BSM_S_SILENTCOM_BOR                          ((uint8)6u)  ///< SilentCom Bor  status
#define  CANSM_BSM_S_WUVALIDATION                           ((uint8)7u)  ///< WakeUp Validation  status
#define  CANSM_BSM_S_CHANGE_BAUDRATE                        ((uint8)8u)  ///< Baud rate change status

/*!*********************************************************************************************************************
 * @brief   cansm_internal_pre_no_comm_state_type
 **********************************************************************************************************************/
#define  CANSM_PRENOCOMM_S_CC_STOPPED                       ((uint8)0u)  ///< S_CC_STOPPED status
#define  CANSM_PRENOCOMM_S_CC_SLEEP                         ((uint8)1u)  ///< S_CC_SLEEP status
#define  CANSM_PRENOCOMM_S_TRCV_NORMAL                      ((uint8)2u)  ///< S_TRCV_NORMAL status
#define  CANSM_PRENOCOMM_S_TRCV_STANDBY                     ((uint8)3u)  ///< S_TRCV_STANDBY status
#define  CANSM_PRENOCOMM_S_PN_CLEAR_WUF                     ((uint8)4u)  ///< S_PN_CLEAR_WUF status
#define  CANSM_PRENOCOMM_S_CHECK_WUF_IN_CC_SLEEP            ((uint8)5u)  ///< S_CHECK_WUF_IN_CC_SLEEP status
#define  CANSM_PRENOCOMM_S_CHECK_WUF_IN_NOT_CC_SLEEP        ((uint8)6u)  ///< S_CHECK_WUF_IN_NOT_CC_SLEEP status

/*!*********************************************************************************************************************
 * @brief   cansm_internal_pre_full_comm_state_type
 **********************************************************************************************************************/
#define  CANSM_PREFULLCOMM_S_TRCV_NORMAL                    ((uint8)0u)  ///< S_TRCV_NORMAL status
#define  CANSM_PREFULLCOMM_S_CC_STOPPED                     ((uint8)1u)  ///< S_CC_STOPPED status
#define  CANSM_PREFULLCOMM_S_CC_STARTED                     ((uint8)2u)  ///< S_CC_STARTED status

/*!*********************************************************************************************************************
 * @brief   cansm_internal_full_comm_state_type
 **********************************************************************************************************************/
#define  CANSM_FULLCOMM_S_BUSOFF_CHECK                      ((uint8)0u)  ///< S_BUSOFF_CHECK status
#define  CANSM_FULLCOMM_S_NO_BUSOFF                         ((uint8)1u)  ///< S_NO_BUSOFF status
#define  CANSM_FULLCOMM_S_RESTART_CC                        ((uint8)2u)  ///< S_RESTART_CC status
#define  CANSM_FULLCOMM_S_TX_OFF                            ((uint8)3u)  ///< S_TX_OFF status
#define  CANSM_FULLCOMM_S_TX_TIMEOUT_EXCEPTION              ((uint8)4u)  ///< S_TX_TIMEOUT_EXCEPTION status

/*!*********************************************************************************************************************
 * @brief   cansm_internal_tx_timeout_exception_state_type
 **********************************************************************************************************************/
#define  CANSM_FullComm_TX_TIMEOUT_EXCEPTION_S_CC_STOPPED   ((uint8)0u)  ///< S_CC_STOPPED status
#define  CANSM_FullComm_TX_TIMEOUT_EXCEPTION_S_CC_STARTED   ((uint8)1u)  ///< S_CC_STARTED status

/*!*********************************************************************************************************************
 * @brief   cansm_internal_wu_validation_state_type
 **********************************************************************************************************************/
#define  CANSM_WUVALIDATION_S_TRCV_NORMAL                   ((uint8)0u) ///< S_TRCV_NORMAL status
#define  CANSM_WUVALIDATION_S_CC_STOPPED                    ((uint8)1u) ///< S_CC_STOPPED status
#define  CANSM_WUVALIDATION_S_CC_STARTED                    ((uint8)2u) ///< S_CC_STARTED status
#define  CANSM_WUVALIDATION_S_WAIT_LEAVE                    ((uint8)3u) ///< S_WAIT_LEAVE status

/*!*********************************************************************************************************************
 * @brief   cansm_internal_pre_no_comm_exit_point_type
 **********************************************************************************************************************/
#define  CANSM_PRENOCOMM_EXIT_NONE                          ((uint8)0u)  ///< Initial value for  Pre NoCom substate
#define  CANSM_PRENOCOMM_EXIT_NOCOM                         ((uint8)1u)  ///< Exiting Pre NoCom substate due to E_NOCOM
#define  CANSM_PRENOCOMM_EXIT_REP_MAX                       ((uint8)2u)  ///< Exiting Pre NoCom  due to Repeat_Max
#define  CANSM_PRENOCOMM_EXIT_START_WUF                     ((uint8)3u)  ///< Exiting Pre NoCom due to flag

/*!*********************************************************************************************************************
 * @brief   cansm_internal_pre_full_comm_exit_point_type
 **********************************************************************************************************************/
#define  CANSM_PREFULLCOMM_EXIT_NONE                        ((uint8)0u)  ///< Initial value for  Pre FullComm substate
#define  CANSM_PREFULLCOMM_EXIT_FULLCOMM                    ((uint8)1u)  ///< Pre FullComm due to FullComm request
#define  CANSM_PREFULLCOMM_EXIT_REP_MAX                     ((uint8)2u)  ///< Exit the Pre FullComm due to Repeat_Max

/*!*********************************************************************************************************************
 * @brief   cansm_internal_full_comm_exit_point_type
 **********************************************************************************************************************/
#define  CANSM_FULLCOMM_EXIT_NONE                           ((uint8)0u)  ///< Initial value for exiting FullComm
#define  CANSM_FULLCOMM_EXIT_PRENOCOMM                      ((uint8)1u)  ///< Exiting FullComm due to a NoComm request
#define  CANSM_FULLCOMM_EXIT_SILENTCOMM                     ((uint8)2u)  ///< Exit FullComm due to SilentComm request
#define  CANSM_FULLCOMM_EXIT_REP_MAX                        ((uint8)3u)  ///< Exit FullComm substate due to Repeat_Max

/*!*********************************************************************************************************************
 * @brief   cansm_internal_silent_comm_bor_exit_point_type
 **********************************************************************************************************************/
#define  CANSM_SILENTCOMMBOR_EXIT_NONE                      ((uint8)0u)  ///< Initial value for SilentCommBor substate
#define  CANSM_SILENTCOMMBOR_EXIT_PRENOCOMM                 ((uint8)1u)  ///< SilentCommBor due to NoComm request
#define  CANSM_SILENTCOMMBOR_EXIT_SILENTCOMM                ((uint8)2u)  ///< SilentCommBor due to completion of Bor
#define  CANSM_SILENTCOMMBOR_EXIT_REP_MAX                   ((uint8)3u)  ///< Exit SilentCommBor due to Repeat_Max

/*!*********************************************************************************************************************
 * @brief   cansm_internal_wu_validation_exit_point_type
 **********************************************************************************************************************/
#define  CANSM_WUVALIDATION_EXIT_NONE                       ((uint8)0u)  ///< Initial value for WuValidation substate
#define  CANSM_WUVALIDATION_EXIT_PRENOCOMM                  ((uint8)1u)  ///< Exit WuValidation due to SoptWakeUpFlag
#define  CANSM_WUVALIDATION_EXIT_PREFULLCOMM                ((uint8)2u)  ///< Exit WuValidation due to FullComm request
#define  CANSM_WUVALIDATION_EXIT_REP_MAX                    ((uint8)3u)  ///< Exit WuValidation due to Repeat_Max

/*!*********************************************************************************************************************
 * @brief   cansm_internal_transition_type
 **********************************************************************************************************************/
#define  CANSM_T_CC_STOPPED_CC_SLEEP                       ((uint8)0u)  ///< S_CC_STOPPED to S_CC_SLEEP
#define  CANSM_T_CC_SLEEP_TRCV_NORMAL                      ((uint8)1u)  ///< S_CC_SLEEP to S_TRCV_STANDBY
#define  CANSM_T_TRCV_NORMAL_TRCV_STANDBY                  ((uint8)2u)  ///< S_TRCV_NORMAL to S_TRCV_STANDBY
#define  CANSM_T_TRCV_STANDBY_NONE                         ((uint8)3u)  ///< S_TRCV_STANDBY is last status
#define  CANSM_T_TRCV_NORMAL_CC_STOPPED                    ((uint8)4u)  ///< S_TRCV_NORMAL to S_CC_STOPPED
#define  CANSM_T_PN_CLEAR_WUF_CC_STOPPED                   ((uint8)5u)  ///< S_CC_STOPPEDto S_CC_STOPPED
#define  CANSM_T_CC_STOPPED_TRCV_NORMAL                    ((uint8)6u)  ///< S_CC_STOPPED to S_TRCV_NORMAL
#define  CANSM_T_TRCV_STANDBY_CC_SLEEP                     ((uint8)7u)  ///< S_TRCV_STANDBY to S_CC_SLEEP
#define  CANSM_T_CC_SLEEP_CHECK_WUF_IN_CC_SLEEP            ((uint8)8u)  ///< S_CC_SLEEP to S_CHECK_WUF_IN_CC_SLEEP
#define  CANSM_T_CHECK_WUF_IN_CC_SLEEP_NONE                ((uint8)9u)  ///< S_CHECK_WUF_IN_CC_SLEEP  is last status
#define  CANSM_T_CHECK_WUF_IN_NOT_CC_SLEEP_PN_CLEAR_WUF    ((uint8)10u)  ///< S_CHECK_WUF_IN_NOT_CC_SLEEP S_PN_CLEAR_WUF
#define  CANSM_T_CC_SLEEP_NONE                             ((uint8)11u)  ///< S_CC_SLEEP  is last status
#define  CANSM_T_CC_STOPPED_CC_STARTED                     ((uint8)12u)  ///< S_CC_STOPPED to S_CC_STARTED
#define  CANSM_T_CC_STARTED_NONE                           ((uint8)13u)  ///< S_CC_STATRED  is last status
#define  CANSM_T_NOF_TRANSITIONS                           ((uint8)14u)  ///< The number of sub-state transition types

/*!*********************************************************************************************************************
 * @brief   cansm_internal_transition_return_type
 **********************************************************************************************************************/
#define  CANSM_T_REQ_ACCEPTED                                     ((uint8)0u)  ///< Request accepted
#define  CANSM_T_WAIT_INDICATION                                  ((uint8)1u)  ///< Wait until Indaication Instructions
#define  CANSM_T_FAIL                                             ((uint8)2u)  ///< Request failed
#define  CANSM_T_DONE                                             ((uint8)3u)  ///< Request not accepted
#define  CANSM_T_REPEAT_MAX                                       ((uint8)4u)  ///< Repeat_Max appears
#define  CANSM_T_TIMEOUT                                          ((uint8)5u)  ///< Sleep mode waits until timeout

#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
#define CANSM_CANIF_CTRLMODE_START CANIF_CS_STARTED         ///< CANIF_CS_STARTED status
#define CANSM_CANIF_CTRLMODE_STOP  CANIF_CS_STOPPED         ///< CANIF_CS_STOPPED status
#define CANSM_CANIF_CTRLMODE_SLEEP CANIF_CS_SLEEP           ///< CANIF_CS_SLEEP status
#else
#define CANSM_CANIF_CTRLMODE_START CAN_CS_STARTED           ///< CANIF_CS_STARTED status
#define CANSM_CANIF_CTRLMODE_STOP  CAN_CS_STOPPED           ///< CANIF_CS_STOPPED status
#define CANSM_CANIF_CTRLMODE_SLEEP CAN_CS_SLEEP             ///< CANIF_CS_SLEEP status
#endif

/*!*********************************************************************************************************************
 * @brief   cansm_network_state_machine_execute
 **********************************************************************************************************************/
#define CANSM_NETWORK_EXECUTE_INIT      0x00u    // Executed in the Init phase, cansm_network_state_machine_execute
#define CANSM_NETWORK_EXECUTE_MAINFUNC  0x01u    // Executed in the MainFunc stage, cansm_network_state_machine_execute

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief        CanSM sub-state switching process
 * @details      The function completes the sub-state switching process of CanSM
 * @param[inout] network_sm_ptr         CanSM network status
 * @param[in]    transition           Substate transition type
 * @ret_val      CANSM_T_REQ_ACCEPTED       Request Request accepted
 * @ret_val      CANSM_T_WAIT_INDICATION    Wait until Indaication Instructions
 * @ret_val      CANSM_T_FAIL               Request request not accepted
 * @ret_val      CANSM_T_DONE               The current switching process ends
 * @ret_val      CANSM_T_REPEAT_MAX         Repeat_Max appears
 * @ret_val      CANSM_T_TIMEOUT            Sleep mode waits until timeout
 * @reentrant    FALSE
 * @synchronous  TRUE
 **********************************************************************************************************************/
FUNC(cansm_internal_transition_return_type, CANSM_CODE) cansm_internal_execute(cansm_internal_network_type
	*network_sm_ptr, cansm_internal_transition_type transition);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Get CanSM network index
 * @details     Get CanSM network index
 * @param[in]   network_handle   network Handle
 * @reentrant   Non Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC_P2VAR(cansm_internal_network_type, AUTOMATIC, CANSM_CODE) cansm_get_cansm_network_index(network_handle_type
	network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Get CanSM global network index
 * @details     Get CanSM global network index
 * @param[in]   network_handle   network Handle
 * @reentrant   Non Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC_P2VAR(cansm_external_network_type, AUTOMATIC, CANSM_CODE) cansm_get_cansm_global_network_index(network_handle_type
	network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Get the spin lock of the CanSM operation controller
 * @details     Get the spin lock of the CanSM operation controller
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANSM_CODE) cansm_get_ctrl_spin_lock(void);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Release the spin lock of CanSM operation controller
 * @details     Release the spin lock of CanSM operation controller
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANSM_CODE) cansm_release_ctr_spin_lock(void);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANSM_INTERNAL_H */

