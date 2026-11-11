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

#ifndef COM_H
#define COM_H

#include "Std_Types.h"
#include "comstack_types.h"
#include "com_pbcfg.h"
#include "com_lcfg.h"
#include "com_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COM_BUSY (0x81u)
#define COM_SERVICE_NOT_AVAILABLE (0x80u)
#define COM_START_SEC_CODE
#include "com_memmap.h"
/****************************************************************************
 * name: com_send_signal
 *
 * description: send signal value to pdu buffer.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_send_signal(com_signal_id_type signal_id, const void *signal_data_ptr);

/****************************************************************************
 * name: com_receive_signal
 *
 * description: read signal value from signal buffer.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_receive_signal(com_signal_id_type signal_id, void *signal_data_ptr);

/****************************************************************************
 * name: com_invalidate_signal
 *
 * description: send signal invalidate value to pdu buffer.
 *
 ****************************************************************************/
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
FUNC(uint8, COM_CODE) com_invalidate_signal(com_signal_id_type signal_id);
#endif

/****************************************************************************
 * name: com_trigger_transmit
 *
 * description: trigger ipdu transmit; copy pdu buffer to low level.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE) com_trigger_transmit(PduIdType tx_pdu_id, PduInfoType *pdu_info_ptr);

/****************************************************************************
 * name: com_trigger_ipdu_send
 *
 * description: com send ipdu to driver.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE)
com_trigger_ipdu_send(PduIdType pdu_id, com_self_tx_ipdu_type *self_ipdu, const com_tx_ipdu_type *ipdu);

/****************************************************************************
 * name: com_rx_indication
 *
 * description: com rx indication function in interrupt context.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_rx_indication(PduIdType rx_pdu_id, const PduInfoType *pdu_info_ptr);

/****************************************************************************
 * name: com_tx_confirmation
 *
 * description: ipdu tx confirmation function in interrupt context.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_tx_confirmation(PduIdType tx_pdu_id, Std_ReturnType result);

#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
/****************************************************************************
 * name: com_send_signal_group
 *
 * description: copy signal group shadow buffer to pdu buffer.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_send_signal_group(com_signal_group_id_type signal_group_id);

/****************************************************************************
 * name: com_receive_signal_group
 *
 * description: copy signal group from pdu buffer to shadow buffer.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_receive_signal_group(com_signal_group_id_type signal_group_id);
#endif

/****************************************************************************
 * name: com_init
 *
 * description: com init.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_init(const com_config_type *config);

/****************************************************************************
 * name: com_de_init
 *
 * description: com deinit.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_de_init(void);

/****************************************************************************
 * name: com_ipdu_group_control
 *
 * description: com ipdu group control.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_ipdu_group_control(const com_ipg_vec ipg_vec, boolean initialize);

/****************************************************************************
 * name: com_clear_ipdu_group_vector
 *
 * description: com clear ipdu group vector.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_clear_ipdu_group_vector(com_ipg_vec ipg_vec);

/****************************************************************************
 * name: com_set_ipdu_group
 *
 * description: com set ipdu group vector.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_set_ipdu_group(com_ipg_vec ipg_vec, com_ipdu_group_id_type ipdu_group_id, boolean bitval);

/****************************************************************************
 * name: com_reception_dm_control
 *
 * description: com rx ipdu deadline monitor control.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_reception_dm_control(const com_ipg_vec ipg_vec);

/****************************************************************************
 * name: com_get_ipdu_states
 *
 * description: get ipdu states.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE) com_get_ipdu_states(PduIdType pdu_id, boolean *ipdu_mode, boolean is_tx_ipdu);

/****************************************************************************
 * name: com_set_ipdu_states
 *
 * description: set ipdu states.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE) com_set_ipdu_states(PduIdType pdu_id, boolean ipdu_mode, boolean is_tx_ipdu);

/****************************************************************************
 * name: com_open_txipdu
 *
 * description: open txipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_open_txipdu(void);

/****************************************************************************
 * name: com_open_rxipdu
 *
 * description: open rxipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_open_rxipdu(void);

/****************************************************************************
 * name: com_close_txipdu
 *
 * description: close txipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_close_txipdu(void);

/****************************************************************************
 * name: com_close_rxipdu
 *
 * description: close rxipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_close_rxipdu(void);

#define COM_STOP_SEC_CODE
#include "com_memmap.h"
#ifdef __cplusplus
}
#endif
#endif /* COM_H */
