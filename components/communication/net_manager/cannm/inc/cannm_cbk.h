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

#ifndef CANNM_CBK_H
#define CANNM_CBK_H

#include "cannm_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif
#define CANNM_START_SEC_CODE
#include "cannm_memmap.h"
/*!*********************************************************************************************************************
 * @brief          Indication of a received PDU from a lower layer communication interface module.
 * @details        The callback function cannm_rx_indication called by the CAN Interface and
 *                 implemented by the CanNm module. It is called in case of a receive indication event
 *                 of the CAN Driver.
 * @param[in]      cannm_rx_pdu_id ID of the received PDU.
 * @param[in]      pdu_info_ptr   Contains the length (SduLength) of the received PDU, a pointer to a
 *                 buffer (SduDataPtr) containing the PDU, and the MetaData related to this PDU.
 * @reentrant      Reentrant for different PduIds. Non reentrant for the same PduId.
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_rx_indication(PduIdType cannm_rx_pdu_id, const PduInfoType *pdu_info_ptr);

#if ((CANNM_PASSIVE_MODE_ENABLED == STD_OFF) && (CANNM_IMMEDIATE_TXCONF_ENABLED == STD_OFF))
/*!*********************************************************************************************************************
 * @brief          Indication of confirmation the transmission of a PDU
 * @details        The lower layer communication interface module confirms the transmission of a
 *                 PDU, or the failure to transmit a PDU.
 * @param[in]      cannm_tx_pdu_id ID of the PDU that has been transmitted.
 * @param[in]      result result of confirmation the transmission of a PDU
 * @reentrant      Reentrant for different PduIds. Non reentrant for the same PduId.
 * @synchronous    Synchronous
 **********************************************************************************************************************/
FUNC(void, CANNM_CODE) cannm_tx_confirmation(PduIdType cannm_tx_pdu_id, Std_ReturnType result);

#endif

#define   CANNM_STOP_SEC_CODE
#include "cannm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif  /* CANNM_CBK_H */
