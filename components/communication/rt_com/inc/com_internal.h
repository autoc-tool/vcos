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

#ifndef COM_INTERNAL_H
#define COM_INTERNAL_H

#include "com_types.h"

#if (COM_DEV_ERROR_DETECT == STD_ON) && (VCOS_MODULE_CONFIG_DET == 1)
#include "Det.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define COM_DET_REPORT(errid)

#define SignalTypeToSize(type, length)                                                                                 \
	(((type) == COM_UINT8) ?                                                                                       \
		 sizeof(uint8) :                                                                                       \
		 ((type) == COM_UINT16) ?                                                                              \
		 sizeof(uint16) :                                                                                      \
		 ((type) == COM_UINT32) ?                                                                              \
		 sizeof(uint32) :                                                                                      \
		 ((type) == COM_UINT64) ?                                                                              \
		 sizeof(uint64) :                                                                                      \
		 ((type) == COM_UINT8_N) ?                                                                             \
		 (sizeof(uint8) * (length)) :                                                                          \
		 ((type) == COM_SINT8) ?                                                                               \
		 sizeof(sint8) :                                                                                       \
		 ((type) == COM_SINT16) ?                                                                              \
		 sizeof(sint16) :                                                                                      \
		 ((type) == COM_SINT32) ? sizeof(sint32) : ((type) == COM_SINT64) ? sizeof(sint64) : sizeof(boolean))

/*!*********************************************************************************************************************
 *  @brief  com_tx_trigger_status_type
 **********************************************************************************************************************/
typedef enum {
	COM_TX_TRIGGERED = 0,
	COM_TX_MIN_DELAY_NOT_FULFILLED,
	COM_TX_NOT_TRIGGERED
} com_tx_trigger_status_type;

/*!*********************************************************************************************************************
 *  @brief  com_invalid_signal_action_type
 **********************************************************************************************************************/
typedef enum {
	COM_NONE_INVALID_VALUE = 0,
	COM_INVALID_ACTION_NOTIFY,
	COM_INVALID_ACTION_REPLACE
} com_invalid_signal_action_type;

#define COM_TESTBIT(source, bit) (*((uint8 *)(source) + ((bit) / 8u)) & (uint8)(1u << ((bit) % 8u)))
#define COM_SETBIT(dest, bit) (*((uint8 *)(dest) + ((bit) / 8u)) |= (uint8)(1u << ((bit) % 8u)))
#define COM_CLEARBIT(dest, bit) (*((uint8 *)(dest) + ((bit) / 8u)) &= ~(uint8)(1u << ((bit) % 8u)))

#define COM_GET_TXSIGNAL(signal_id) (&com_tx_signal_const[signal_id])
#define COM_GET_RXSIGNAL(signal_id) (&com_rx_signal_const[signal_id])

#if ((COM_SIG_GATEWAY_ENABLE == STD_ON) || (COM_SIGNAL_GROUP_ENABLE == STD_ON))
#define com_get_self_tx_signal(signal_id) (&com_self_tx_signal[signal_id])
#endif

#if ((COM_SIG_GATEWAY_ENABLE == STD_ON) || (COM_SIGNAL_GROUP_ENABLE == STD_ON) ||                                      \
	(COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF))
#define com_get_self_rx_signal(signal_id) (&com_self_rx_signal[signal_id])
#endif

#define COM_GET_TXIPDU(ipdu_id) (&com_tx_ipdu_const[ipdu_id])
#define COM_GET_RXIPDU(ipdu_id) (&com_rx_ipdu_const[ipdu_id])
#define COM_GET_SELF_TXIPDU(ipdu_id) (&com_self_tx_ipdu[ipdu_id])
#define COM_GET_SELF_RXIPDU(ipdu_id) (&com_self_rx_ipdu[ipdu_id])

#define COM_START_SEC_CODE
#include "com_memmap.h"

/****************************************************************************
 * name: com_alg_trigger_ipdu_send
 *
 * description: ipdu send function.
 *
 ****************************************************************************/
FUNC(com_tx_trigger_status_type, COM_CODE)
com_alg_trigger_ipdu_send(PduIdType pdu_id, com_self_tx_ipdu_type *self_ipdu, const com_tx_ipdu_type *ipdu);

/****************************************************************************
 * name: com_alg_trigger_tx_on_conditions
 *
 * description: trigger signal send.
 *
 ****************************************************************************/
FUNC(boolean, COM_CODE)
com_alg_trigger_tx_on_conditions(uint16 pdu_handle_id, boolean data_changed,
				  com_transfer_property_type transfer_property);

#define COM_STOP_SEC_CODE
#include "com_memmap.h"
#ifdef __cplusplus
}
#endif
#endif /* COM_INTERNAL_H */
