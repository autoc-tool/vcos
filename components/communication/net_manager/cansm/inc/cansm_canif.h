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

#ifndef CANSM_CANIF_H
#define CANSM_CANIF_H

#include "cansm_types.h"

#ifdef __cplusplus
extern "C" {
#endif
#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Notify the CanSM module that a CAN controller has a BusOff
 * @details     Notify the CanSM about a bus-off event on a CAN controller
 * @param[in]   controller_id    CAN Controller Id
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_controller_bus_off(uint8 controller_id);

#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
/*!*********************************************************************************************************************
 * @brief       Notify the CanSM module of the CAN controller mode change
 * @details     Notifies the CanSM module about CAN controller mode changes
 * @param[in]   controller_id      CAN Controller Id
 * @param[in]   controller_mode    CAN Controller mode
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_controller_mode_indication(uint8 controller_id,
	canif_controller_mode_type controller_mode);
#else
FUNC(void, CANSM_CODE) cansm_controller_mode_indication(uint8 controller_id, Can_ControllerStateType controller_mode);
#endif

/*!*********************************************************************************************************************
 * @brief       Notify the CanSM module of the CAN transceiver mode change
 * @details     Notify the CanSM module about CAN transceiver mode changes
 * @param[in]   transceiver_id    CAN transceiver id
 * @param[in]   transceiver_mode  CAN transceiver mode
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_transceiver_mode_indication(uint8 transceiver_id, CanTrcv_TrcvModeType transceiver_mode);

/*!*********************************************************************************************************************
 * @brief       Notify the CanSM module that the CanIf_ClearTrcvWufFlag process has ended and indicate
 * @details     Indicates the end of the CanIf_ClearTrcvWufFlag API process for the notified CAN transceiver
 * @param[in]   transceiver_id      CAN transceiver id
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_clear_trcv_wuf_flag_indication(uint8 transceiver_id);

/*!*********************************************************************************************************************
 * @brief       Notify the CanSM module that the CanIf_CheckTrcvWakeFlag process has ended and indicate
 * @details     Indicates the end of the CanIf_CheckTrcvWakeFlag API process for the notified CAN transceiver
 * @param[in]   transceiver_id      CAN transceiver id
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_check_transceiver_wake_flag_indication(uint8 transceiver_id);

/*!*********************************************************************************************************************
 * @brief       Notify the CanSM module that the transceiver is operating in PN communication
 * @details     Instructs the transceiver to operate in PN communication mode
 * @param[in]   transceiver_id      CAN transceiver id
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_confirm_pn_availability(uint8 transceiver_id);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANSM_CANIF_H */

