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

#ifndef COM_ALG_H
#define COM_ALG_H

#include "com_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COM_START_SEC_CODE
#include "com_memmap.h"
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)

/****************************************************************************
 * name: com_alg_copy_signal_group_data_from_pdu_to_shadow_buffer
 *
 * description: copy signal group data from pdu to shadow buffer.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_copy_signal_group_data_from_pdu_to_shadow_buffer(const com_signal_id_type signal_group_id);

/****************************************************************************
 * name: com_alg_copy_sg_data_from_sb_to_tx_pdu
 *
 * description: copy signal group data from shadow buffer to tx pdu buffer.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_copy_sg_data_from_sb_to_tx_pdu(const com_signal_id_type signal_group_id, boolean *data_changed);

/****************************************************************************
 * name: com_alg_copy_sg_data_from_sb_to_rx_pdu
 *
 * description: copy signal group data from shadow buffer to rx pdu buffer.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_copy_sg_data_from_sb_to_rx_pdu(const com_signal_id_type signal_group_id, boolean deferred_buffer_destination,
					boolean *data_changed);
#endif

/****************************************************************************
 * name: com_alg_processing_rx_signal_data_from_pdu
 *
 * description: process rx singnal from pdu buffer.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_processing_rx_signal_data_from_pdu(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *signal,
					    uint8 core_map_idx);

/****************************************************************************
 * name: com_rx_processing_init_rx_signal
 *
 * description: init rx signal value.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_rx_processing_init_rx_signal(const com_rx_signal_type *signal, uint8 core_map_idx);

/****************************************************************************
 * name: com_alg_read_signal_data_from_pdu
 *
 * description: read rx singnal from pdu buffer.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_read_signal_data_from_pdu(const uint8 *com_ipdu_data_ptr, const com_rx_signal_type *signal,
				   uint8 *signal_data);

/****************************************************************************
 * name: com_alg_write_tx_signal_data_to_pdu
 *
 * description: write signal data to pdu buffer function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_write_tx_signal_data_to_pdu(const uint8 *signal_data_ptr, const com_tx_signal_type *tx_signal,
				     uint8 *com_ipdu_data_ptr, boolean *data_changed);

/****************************************************************************
 * name: com_alg_write_rx_signal_data_to_pdu
 *
 * description: write rx signal data to pdu buffer function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_write_rx_signal_data_to_pdu(const uint8 *signal_data_ptr, const com_rx_signal_type *rx_signal,
				     uint8 *com_ipdu_data_ptr, boolean *data_changed);

#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
/****************************************************************************
 * name: com_alg_update_shadow_tx_signal
 *
 * description: update shadow tx signal.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_update_shadow_tx_signal(com_signal_id_type signal_id, const void *signal_data_ptr);

/****************************************************************************
 * name: com_alg_update_shadow_rx_signal
 *
 * description: update shadow rx signal.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_update_shadow_rx_signal(com_signal_id_type signal_id, const void *signal_data_ptr);
#endif

/****************************************************************************
 * name: com_alg_rx_process
 *
 * description: rx process in rx indication.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_rx_process(PduIdType rx_pdu_id, const PduInfoType *pdu_info_ptr);

/****************************************************************************
 * name: com_receive_sig_read_buffer
 *
 * description: read rx singnal from signal buffer.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_receive_sig_read_buffer(const com_rx_signal_type *com_signal, uint8 *signal_data_ptr, uint8 core_map_idx);

/****************************************************************************
 * name: com_alg_tx_handle_dm
 *
 * description: rx process in rx indication.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_tx_handle_dm(const com_tx_ipdu_type *ipdu, com_self_tx_ipdu_type *self_ipdu);

/****************************************************************************
 * name: com_alg_set_tx_confirmation_status
 *
 * description: set tx confirmation status.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_set_tx_confirmation_status(com_self_tx_ipdu_type *self_ipdu, boolean value);

/****************************************************************************
 * name: com_alg_get_tx_confirmation_status
 *
 * description: get tx confirmation status.
 *
 ****************************************************************************/
FUNC(boolean, COM_CODE) com_alg_get_tx_confirmation_status(com_self_tx_ipdu_type *self_ipdu);

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
/****************************************************************************
 * name: com_alg_route_gw_destn_signals
 *
 * description: rout signal to dest signal.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_alg_route_gw_destn_signals(uint8 gw_mapidx, const uint8 *signal_data_ptr);

/****************************************************************************
 * name: com_alg_extract_gw_src_sig_data
 *
 * description: extract gateway src signal data function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_alg_extract_gw_src_sig_data(const com_rx_signal_type *com_signal_src, uint16 ipdu_handle, uint8 *sig_data_ptr);
#endif

#if (COM_SUPPORT_MUTICORE == STD_ON)
/****************************************************************************
 * name: com_alg_get_rx_ipdu_core_map_idx_by_pdu_id
 *
 * description: get rx ipdu core map idx by pdu id.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(uint16 pdu_id);

/****************************************************************************
 * name: com_alg_get_ipdu_core_map_idx_by_core_id
 *
 * description: get core map idx by core id.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_alg_get_ipdu_core_map_idx_by_core_id(uint32 core_id);
#endif

#define COM_STOP_SEC_CODE
#include "com_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* COM_MISC_H */
