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

/* including file */
#include <string.h>
#if (CONFIG_MODULE_PDUR == 1)
#include "pdur.h"
#endif
#include "com.h"
#include "com_internal.h"
#include "com_alg.h"
#include "SchM_Com.h"
#include "com_vcos_log.h"
#include "vcos_log.h"
#include "Compiler.h"

#define COM_START_SEC_CODE
#include "com_memmap.h"

/***********************************************************************************************************************
 *  com_update_shadow_signal()
 **********************************************************************************************************************/
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
static FUNC(void, COM_CODE) com_update_shadow_signal(com_signal_id_type signal_id, const void *signal_data_ptr);
#endif

/****************************************************************************
 * name: com_tx_ipdu_init
 *
 * description: tx IPDU init.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_tx_ipdu_init(com_core_id_type core_id);

/****************************************************************************
 * name: com_rx_ipdu_init
 *
 * description: rx IPDU init.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_rx_ipdu_init(com_core_id_type core_id);

/****************************************************************************
 * name: com_tx_ipdu_init
 *
 * description: tx IPDU init.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_tx_ipdu_init(com_core_id_type core_id)
{
	boolean data_changed = FALSE;
	const com_tx_signal_type *signal;
	uint16 buffer_index = 0u;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
	(void)core_id;
	uint16 start_tx_ipdu_idx = 0;
	uint16 end_tx_ipdu_idx = COM_MAX_N_TXIPDUS;
#else
	uint8 core_map_idx = 0;
	uint32 com_core_id = (uint32)core_id;

	core_map_idx = com_alg_get_ipdu_core_map_idx_by_core_id(com_core_id);
	uint16 start_tx_ipdu_idx = com_core_map_info_struct[core_map_idx].start_tx_ipdu_idx;
	uint16 end_tx_ipdu_idx = com_core_map_info_struct[core_map_idx].end_tx_ipdu_idx;
#endif
	for (uint16 i = start_tx_ipdu_idx; i < end_tx_ipdu_idx; i++) {
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
		boolean pdu_has_group_signal = FALSE;
#endif
		const com_tx_ipdu_type *ipdu = COM_GET_TXIPDU(i);
		com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(i);

		self_ipdu->com_tx_dm_timer = 0u;

#if (COM_SUPPORT_MUTICORE == STD_OFF)
		self_ipdu->com_ipdu_data_ptr = &com_self_tx_buffer[buffer_index];
#else
		self_ipdu->com_ipdu_data_ptr = &(com_core_map_info_struct[core_map_idx].tx_buffer_ref[buffer_index]);
#endif
		buffer_index += ipdu->com_ipdu_size;
		self_ipdu->com_txipdu_tx_mode = TRUE;
		com_tx_mode_mode_type com_tx_mode_mode_ram = ipdu->com_tx_ipdu->com_tx_mode_true->com_tx_mode_mode;

		if ((COM_PERIODIC == com_tx_mode_mode_ram) || (COM_MIXED == com_tx_mode_mode_ram)) {
			self_ipdu->com_txmode_period_timer =
				ipdu->com_tx_ipdu->com_tx_mode_true->com_tx_mode_time_offset_factor + 1u;
		}

		uint8 com_tx_ipdu_unused_areas_default_ram = ipdu->com_tx_ipdu->com_tx_ipdu_unused_areas_default;

		if (com_tx_ipdu_unused_areas_default_ram != 0u) {
			(void)memset((void *)self_ipdu->com_ipdu_data_ptr, com_tx_ipdu_unused_areas_default_ram,
				     ipdu->com_ipdu_size);
		}

		self_ipdu->com_txipdu_min_delay_timer = 0u;
		uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
		uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;

#if ((COM_TXTOU_ENABLE == STD_ON) && (COM_TXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
		uint16 tx_tout_info_idx_ram = ipdu->tx_tout_info_idx;

		if (tx_tout_info_idx_ram != COM_NO_REF_INDEX) {
			self_ipdu->com_tx_deadline_cnt =
				com_tx_pdu_tout_struct[tx_tout_info_idx_ram].com_timeout_factor;
		}
#endif
		for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
			signal = COM_GET_TXSIGNAL(ipdu_signal_start_idx);
#if (COM_TXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
			if (0u < signal->com_timeout_factor) {
				self_ipdu->com_tx_deadline_cnt = signal->com_timeout_factor;
			}
#endif
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
			if (TRUE == signal->com_signal_self_use_update_bit) {
				COM_CLEARBIT(self_ipdu->com_ipdu_data_ptr, signal->com_update_bit_pos);
			}
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
			if (TRUE == signal->com_self_is_signal_group) {
				pdu_has_group_signal = TRUE;
				com_self_tx_signal_type *self_signal = com_get_self_tx_signal(ipdu_signal_start_idx);
#if (COM_SUPPORT_MUTICORE == STD_OFF)
				self_signal->com_self_shadow_buffer = (void *)&com_self_tx_buffer[buffer_index];
#else
				self_signal->com_self_shadow_buffer =
					(void *)&(com_core_map_info_struct[core_map_idx].tx_buffer_ref[buffer_index]);
#endif
				for (uint16 h = 0u; signal->com_group_signal[h] != COM_GROUPSIGNALREFS_END; h++) {
					uint16 group_com_handle_id = signal->com_group_signal[h];
					const com_tx_signal_type *group_signal = COM_GET_TXSIGNAL(group_com_handle_id);
					com_self_tx_signal_type *self_group_signal =
						com_get_self_tx_signal(group_com_handle_id);

					self_group_signal->com_self_shadow_buffer =
						((void *)self_signal->com_self_shadow_buffer);

					com_alg_update_shadow_tx_signal(group_com_handle_id,
									group_signal->com_signal_init_value);
				}
				com_alg_copy_sg_data_from_sb_to_tx_pdu((com_signal_id_type)ipdu_signal_start_idx,
								       &data_changed);
			} else
#endif
			{
				if ((signal->com_sig_init_value_is_zero == FALSE) ||
				    (com_tx_ipdu_unused_areas_default_ram != 0u)) {
					com_alg_write_tx_signal_data_to_pdu(signal->com_signal_init_value, signal,
									    self_ipdu->com_ipdu_data_ptr,
									    &data_changed);
				}
			}
		}
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
		if (TRUE == pdu_has_group_signal) {
			buffer_index += ipdu->com_ipdu_size;
		}
#endif
	}
}

/****************************************************************************
 * name: com_rx_ipdu_init
 *
 * description: rx IPDU init.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_rx_ipdu_init(com_core_id_type core_id)
{
	boolean data_changed = FALSE;
	const com_rx_signal_type *signal;
	uint16 buffer_index = 0u;
	uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
	(void)core_id;
	uint16 start_rx_ipdu_idx = 0;
	uint16 end_rx_ipdu_idx = COM_MAX_N_RXIPDUS;
#else
	uint32 com_core_id = (uint32)core_id;

	core_map_idx = com_alg_get_ipdu_core_map_idx_by_core_id(com_core_id);
	uint16 start_rx_ipdu_idx = com_core_map_info_struct[core_map_idx].start_rx_ipdu_idx;
	uint16 end_rx_ipdu_idx = com_core_map_info_struct[core_map_idx].end_rx_ipdu_idx;
#endif
	for (uint16 i = start_rx_ipdu_idx; i < end_rx_ipdu_idx; i++) {
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
		boolean pdu_has_group_signal = FALSE;
#endif
		const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(i);
		com_self_rx_ipdu_type *self_ipdu = COM_GET_SELF_RXIPDU(i);

		self_ipdu->com_rxipdu_dm_ctrl = TRUE;

		if ((COM_DEFERRED == ipdu->com_ipdu_signal_processing) || (TRUE == ipdu->com_ipdu_has_signal_group)) {
#if (COM_SUPPORT_MUTICORE == STD_OFF)
			self_ipdu->com_ipdu_data_ptr = &com_self_rx_buffer[buffer_index];
#else
			self_ipdu->com_ipdu_data_ptr =
				&(com_core_map_info_struct[core_map_idx].rx_buffer_ref[buffer_index]);
#endif
			self_ipdu->com_self_ipdu_has_ipdu_buff = TRUE;
			buffer_index += ipdu->com_ipdu_size;
		}

		uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
		uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;
#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
		uint16 rx_tout_info_idx_ram = ipdu->rx_tout_info_idx;

		if (rx_tout_info_idx_ram != COM_NO_REF_INDEX) {
			self_ipdu->com_rx_deadline_cnt =
				com_rx_pdu_tout_struct[rx_tout_info_idx_ram].com_first_timeout_factor;
		}
#endif
		for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
			signal = COM_GET_RXSIGNAL(ipdu_signal_start_idx);
#if ((COM_SIGNAL_GROUP_ENABLE == STD_ON) || (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF))
			com_self_rx_signal_type *self_signal = com_get_self_rx_signal(ipdu_signal_start_idx);
#endif
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
			if (0u < signal->com_timeout_factor) {
				self_signal->com_rx_deadline_cnt = signal->com_first_timeout_factor;
			}
#endif
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
			if (TRUE == signal->com_self_is_signal_group) {
				pdu_has_group_signal = TRUE;
#if (COM_SUPPORT_MUTICORE == STD_OFF)
				self_signal->com_self_shadow_buffer = (void *)&com_self_rx_buffer[buffer_index];
#else
				self_signal->com_self_shadow_buffer =
					(void *)&(com_core_map_info_struct[core_map_idx].rx_buffer_ref[buffer_index]);
#endif
				for (uint16 h = 0u; signal->com_group_signal[h] != COM_GROUPSIGNALREFS_END; h++) {
					uint16 group_com_handle_id = signal->com_group_signal[h];
					const com_rx_signal_type *group_signal =
						COM_GET_RXSIGNAL(signal->com_group_signal[h]);
					com_self_rx_signal_type *self_group_signal =
						com_get_self_rx_signal(group_com_handle_id);
					self_group_signal->com_self_shadow_buffer =
						((void *)self_signal->com_self_shadow_buffer);
					com_alg_update_shadow_rx_signal(group_com_handle_id,
									group_signal->com_signal_init_value);
				}
				com_alg_copy_sg_data_from_sb_to_rx_pdu((com_signal_id_type)ipdu_signal_start_idx, FALSE,
								       &data_changed);
			} else
#endif
			{
				if (signal->com_sig_init_value_is_zero == FALSE) {
					com_rx_processing_init_rx_signal(signal, core_map_idx);
					if ((COM_DEFERRED == ipdu->com_ipdu_signal_processing) ||
					    (TRUE == ipdu->com_ipdu_has_signal_group)) {
						com_alg_write_rx_signal_data_to_pdu(signal->com_signal_init_value,
										    signal,
										    self_ipdu->com_ipdu_data_ptr,
										    &data_changed);
					}
				}
			}
		}
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
		if (TRUE == pdu_has_group_signal) {
			buffer_index += ipdu->com_ipdu_size;
		}
#endif
	}
}

/****************************************************************************
 * name: com_init
 *
 * description: com init.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_init(const com_config_type *config)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
#else
	com_core_id_type core_id = 0;
#endif
	if (NULL == config) {
		COM_DET_REPORT(COM_COM_INIT_ID_COM_E_PARAM_POINTER);
		return;
	}
	com_config = config;

	com_tx_ipdu_init(core_id);
	com_rx_ipdu_init(core_id);

	com_open_txipdu();
	com_open_rxipdu();

#if (COM_SUPPORTED_TXSIGNALPROX == STD_ON)
	com_signal_fifo_init();
#endif
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_init_status[core_id] = COM_INIT;
#else
	com_init_status = COM_INIT;
#endif
}

/****************************************************************************
 * name: com_de_init
 *
 * description: com deinit.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_de_init(void)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
	uint32 com_core_id = (uint32)core_id;
	uint8 core_map_idx = com_alg_get_ipdu_core_map_idx_by_core_id(com_core_id);
	uint16 start_tx_ipdu_idx = com_core_map_info_struct[core_map_idx].start_tx_ipdu_idx;
	uint16 end_tx_ipdu_idx = com_core_map_info_struct[core_map_idx].end_tx_ipdu_idx;
	uint16 start_rx_ipdu_idx = com_core_map_info_struct[core_map_idx].start_rx_ipdu_idx;
	uint16 end_rx_ipdu_idx = com_core_map_info_struct[core_map_idx].end_rx_ipdu_idx;
#else
	uint8 com_init_status_temp = com_init_status;
	uint16 start_tx_ipdu_idx = 0;
	uint16 end_tx_ipdu_idx = COM_MAX_N_TXIPDUS;
	uint16 start_rx_ipdu_idx = 0;
	uint16 end_rx_ipdu_idx = COM_MAX_N_RXIPDUS;
#endif
	if (com_init_status_temp != COM_INIT) {
		COM_DET_REPORT(COM_COM_DEINIT_ID_COM_E_UNINIT);
		return;
	}
	for (uint16 i = start_tx_ipdu_idx; i < end_tx_ipdu_idx; i++) {
		com_self_tx_ipdu[i].com_ipdu_started = FALSE;
	}
	for (uint16 i = start_rx_ipdu_idx; i < end_rx_ipdu_idx; i++) {
		com_self_rx_ipdu[i].com_ipdu_started = FALSE;
	}
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_init_status[core_id] = COM_UNINIT;
#else
	com_init_status = COM_UNINIT;
#endif
}

/****************************************************************************
 * name: com_tx_ipdu_group_control
 *
 * description: com tx ipdu group control.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_tx_ipdu_group_control(const com_ipg_vec ipg_vec, boolean initialize)
{
	const com_tx_ipdu_type *ipdu;

	for (uint16 i = 0u; i < COM_MAX_N_TXIPDUS; i++) {
		const com_ipg_type *const ipg_refs = com_tx_ipdu_const[i].com_ipg_refs;
		boolean started = FALSE;

		for (uint32 idx = 0u; 0u == ipg_refs[idx].com_self_eol; idx++) {
			uint16 byte_index = ipg_refs[idx].com_ipg_handle_id / 8u;
			uint8 bit_index = (uint8)(ipg_refs[idx].com_ipg_handle_id % 8u);

			if (((ipg_vec[byte_index] >> bit_index) & 1u) != 0u) {
				started = TRUE;
				break;
			}
		}

		if ((com_self_tx_ipdu[i].com_ipdu_started != started) || (TRUE == initialize)) {
			com_self_tx_ipdu[i].com_ipdu_started = started;
			ipdu = COM_GET_TXIPDU(i);
			const com_tx_mode_type *tx_mode_ptr = (TRUE == com_self_tx_ipdu[i].com_txipdu_tx_mode) ?
								      ipdu->com_tx_ipdu->com_tx_mode_true :
								      ipdu->com_tx_ipdu->com_tx_mode_false;

			com_self_tx_ipdu[i].com_tx_dm_timer = 0u;
			com_self_tx_ipdu[i].com_txipdu_tx_confirmations_num = 0u;
			com_self_tx_ipdu[i].com_txmode_repet_period_timer = 0u;
			com_self_tx_ipdu[i].com_txipdu_repetitions_left_num = 0u;

			if ((COM_PERIODIC == tx_mode_ptr->com_tx_mode_mode) ||
			    (COM_MIXED == tx_mode_ptr->com_tx_mode_mode)) {
				com_self_tx_ipdu[i].com_txmode_period_timer =
					tx_mode_ptr->com_tx_mode_time_offset_factor + 1u;
			}

			if ((TRUE == started) && (0u < com_self_tx_ipdu[i].com_tx_deadline_cnt)) {
				if (COM_NONE == tx_mode_ptr->com_tx_mode_mode) {
					com_self_tx_ipdu[i].com_tx_dm_timer = com_self_tx_ipdu[i].com_tx_deadline_cnt;
				}
			}
		}
	}
}

static FUNC(void, COM_CODE) com_rx_openipdu_group_control(uint16 ipd_idx)
{
#if (COM_RXTOU_ENABLE == STD_ON)
	const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(ipd_idx);
#endif
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
	const com_rx_signal_type *com_signal;
	com_self_rx_signal_type *self_signal;
#endif
	if (TRUE == com_self_rx_ipdu[ipd_idx].com_rxipdu_dm_ctrl) {
#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
		uint16 rx_tout_info_idx_ram = ipdu->rx_tout_info_idx;

		if (rx_tout_info_idx_ram != COM_NO_REF_INDEX) {
			com_self_rx_ipdu[ipd_idx].com_rx_deadline_cnt =
				com_rx_pdu_tout_struct[rx_tout_info_idx_ram].com_first_timeout_factor;
		}
#endif
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
		uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
		uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;

		while (ipdu_signal_start_idx < ipdu_signal_end_idx) {
			com_signal = COM_GET_RXSIGNAL(ipdu_signal_start_idx);
			self_signal = com_get_self_rx_signal(ipdu_signal_start_idx);

			if (0u < com_signal->com_timeout_factor) {
				if (0u < com_signal->com_first_timeout_factor) {
					self_signal->com_rx_deadline_cnt = com_signal->com_first_timeout_factor;
				} else {
					self_signal->com_rx_deadline_cnt = 0u;
				}
			}
			ipdu_signal_start_idx++;
		}
#endif
	}
}

/****************************************************************************
 * name: com_rx_ipdu_group_control
 *
 * description: com rx ipdu group control.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_rx_ipdu_group_control(const com_ipg_vec ipg_vec, boolean initialize)
{
	for (uint16 i = 0u; i < COM_MAX_N_RXIPDUS; i++) {
		const com_ipg_type *const ipg_refs = com_rx_ipdu_const[i].com_ipg_refs;
		boolean started = FALSE;

		for (uint32 idx = 0u; (0u == ipg_refs[idx].com_self_eol); idx++) {
			if (FALSE == started) {
				uint16 byte_index;
				uint8 bit_index;

				byte_index = ipg_refs[idx].com_ipg_handle_id / 8u;
				bit_index = (uint8)(ipg_refs[idx].com_ipg_handle_id % 8u);
				started = (((ipg_vec[byte_index] >> bit_index) & 1u) != 0u);
			}
		}
		if ((com_self_rx_ipdu[i].com_ipdu_started != started) || (TRUE == initialize)) {
			com_self_rx_ipdu[i].com_ipdu_started = started;
			if (TRUE == started) {
				com_rx_openipdu_group_control(i);
				com_self_rx_ipdu[i].com_rxipdu_dm_ctrl = TRUE;
			} else {
				com_self_rx_ipdu[i].com_rxipdu_dm_ctrl = FALSE;
			}
		}
	}
}

/****************************************************************************
 * name: com_ipdu_group_control
 *
 * description: com ipdu group control.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_ipdu_group_control(const com_ipg_vec ipg_vec, boolean initialize)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	uint8 com_init_status_temp = com_init_status[0];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	if (COM_INIT != com_init_status_temp) {
		COM_DET_REPORT(COM_COM_IPDUGROUPCONTROL_ID_COM_E_UNINIT);
		return;
	}
	com_tx_ipdu_group_control(ipg_vec, initialize);
	com_rx_ipdu_group_control(ipg_vec, initialize);
}

/****************************************************************************
 * name: com_clear_ipdu_group_vector
 *
 * description: com clear ipdu group vector.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_clear_ipdu_group_vector(com_ipg_vec ipg_vec)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	uint8 com_init_status_temp = com_init_status[0];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	if (COM_INIT != com_init_status_temp) {
		COM_DET_REPORT(COM_COM_CLEARIPDUGROUPVECTOR_ID_COM_E_UNINIT);
		return;
	}
	(void)memset(ipg_vec, 0, sizeof(com_ipg_vec));
}

/****************************************************************************
 * name: com_set_ipdu_group
 *
 * description: com set ipdu group vector.
 *
 ****************************************************************************/
FUNC(void, COM_CODE)
com_set_ipdu_group(com_ipg_vec ipg_vec, com_ipdu_group_id_type ipdu_group_id, boolean bitval)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	uint8 com_init_status_temp = com_init_status[0];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	if (COM_INIT != com_init_status_temp) {
		COM_DET_REPORT(COM_COM_SETIPDUGROUP_ID_COM_E_UNINIT);
		return;
	}
	uint16 byte_index;
	uint8 bit_index;

	byte_index = ipdu_group_id / 8u;
	bit_index = (uint8)(ipdu_group_id % 8u);
#if (COM_DEBUG_SUPPORT == STD_ON)
	(void)VCOS_LOG(COM_MODULE_ID, VCOS_LOG_INFO, COM_DLT_IPDUGROUP_ID, (uint32)ipdu_group_id);
#endif
	if (TRUE == bitval) {
		ipg_vec[byte_index] |= (1u << bit_index);
	} else {
		ipg_vec[byte_index] &= ~(1u << bit_index);
	}
}

/****************************************************************************
 * name: com_open_reception_dm
 *
 * description: com_open_reception_dm.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_open_reception_dm(uint16 ipdu_idx, boolean enabled)
{
#if (COM_RXTOU_ENABLE == STD_ON)
	const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(ipdu_idx);
#endif
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
	const com_rx_signal_type *com_signal;
	com_self_rx_signal_type *self_signal;
#endif
	com_self_rx_ipdu[ipdu_idx].com_rxipdu_dm_ctrl = enabled;
	if (TRUE == enabled) {
#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
		uint16 rx_tout_info_idx_ram = ipdu->rx_tout_info_idx;

		if (rx_tout_info_idx_ram != COM_NO_REF_INDEX) {
			com_self_rx_ipdu[ipdu_idx].com_rx_deadline_cnt =
				com_rx_pdu_tout_struct[rx_tout_info_idx_ram].com_first_timeout_factor;
		}
#endif
#if (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
		uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
		uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;

		for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
			com_signal = COM_GET_RXSIGNAL(ipdu_signal_start_idx);
			self_signal = com_get_self_rx_signal(ipdu_signal_start_idx);

			if (com_signal->com_timeout_factor > 0u) {
				self_signal->com_rx_deadline_cnt = (com_signal->com_first_timeout_factor > 0u) ?
									   com_signal->com_first_timeout_factor :
									   0u;
			}
		}
#endif
	}
}

/****************************************************************************
 * name: com_reception_dm_control
 *
 * description: com rx ipdu deadline monitor control.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_reception_dm_control(const com_ipg_vec ipg_vec)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	uint8 com_init_status_temp = com_init_status[0];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	if (COM_INIT != com_init_status_temp) {
		COM_DET_REPORT(COM_COM_RECEPTIONDMCONTROL_ID_COM_E_UNINIT);
		return;
	}
	for (uint16 i = 0u; i < COM_MAX_N_RXIPDUS; i++) {
		const com_ipg_type *const ipg_refs = com_rx_ipdu_const[i].com_ipg_refs;
		boolean enabled = FALSE;

		for (uint16 idx = 0u; 0u == ipg_refs[idx].com_self_eol; idx++) {
			if (enabled == FALSE) {
				uint16 byte_index;
				uint8 bit_index;

				byte_index = ipg_refs[idx].com_ipg_handle_id / 8u;
				bit_index = (uint8)(ipg_refs[idx].com_ipg_handle_id % 8u);
				enabled = (((ipg_vec[byte_index] >> bit_index) & 1u) != 0u);
			}
		}
		if (com_self_rx_ipdu[i].com_rxipdu_dm_ctrl != enabled) {
			com_open_reception_dm(i, enabled);
		}
	}
}

/****************************************************************************
 * name: com_get_ipdu_states
 *
 * description: get ipdu states.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE) com_get_ipdu_states(PduIdType pdu_id, boolean *ipdu_mode, boolean is_tx_ipdu)
{
	Std_ReturnType ret = E_OK;

	if (ipdu_mode == NULL) {
		ret = E_NOT_OK;
		return ret;
	}
	if (is_tx_ipdu == TRUE) {
		if (pdu_id >= COM_MAX_N_TXIPDUS) {
			ret = E_NOT_OK;
			return ret;
		}
		*ipdu_mode = com_self_tx_ipdu[pdu_id].com_ipdu_started;
	} else {
		if (pdu_id >= COM_MAX_N_RXIPDUS) {
			ret = E_NOT_OK;
			return ret;
		}
		*ipdu_mode = com_self_rx_ipdu[pdu_id].com_ipdu_started;
	}
	return ret;
}

/****************************************************************************
 * name: com_set_ipdu_states
 *
 * description: set ipdu states.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE) com_set_ipdu_states(PduIdType pdu_id, boolean ipdu_mode, boolean is_tx_ipdu)
{
	Std_ReturnType ret = E_OK;

	if (is_tx_ipdu == TRUE) {
		if (pdu_id >= COM_MAX_N_TXIPDUS) {
			ret = E_NOT_OK;
			return ret;
		}
		com_self_tx_ipdu[pdu_id].com_ipdu_started = ipdu_mode;
	} else {
		if (pdu_id >= COM_MAX_N_RXIPDUS) {
			ret = E_NOT_OK;
			return ret;
		}
		com_self_rx_ipdu[pdu_id].com_ipdu_started = ipdu_mode;
	}
	return ret;
}

/****************************************************************************
 * name: com_open_txipdu
 *
 * description: open txipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_open_txipdu(void)
{
	for (uint8 i = 0; i < COM_MAX_N_TXIPDUS; i++) {
		com_self_tx_ipdu[i].com_ipdu_started = TRUE;
	}
	return;
}

/****************************************************************************
 * name: com_open_rxipdu
 *
 * description: open rxipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_open_rxipdu(void)
{
	for (uint8 i = 0; i < COM_MAX_N_RXIPDUS; i++) {
		com_self_rx_ipdu[i].com_ipdu_started = TRUE;
	}
	return;
}

/****************************************************************************
 * name: com_close_txipdu
 *
 * description: close txipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_close_txipdu(void)
{
	for (uint8 i = 0; i < COM_MAX_N_TXIPDUS; i++) {
		com_self_tx_ipdu[i].com_ipdu_started = FALSE;
	}
	return;
}

/****************************************************************************
 * name: com_close_rxipdu
 *
 * description: close rxipdu.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_close_rxipdu(void)
{
	for (uint8 i = 0; i < COM_MAX_N_RXIPDUS; i++) {
		com_self_rx_ipdu[i].com_ipdu_started = FALSE;
	}
	return;
}

/****************************************************************************
 * name: com_validation_common
 *
 * description: com_validation_common.
 *
 ****************************************************************************/
static FUNC(uint8, COM_CODE) com_validation_common(uint16 id, uint8 init_state, uint16 max_id)
{
	uint8 ret = E_OK;

	if (COM_INIT != init_state) {
		return COM_SERVICE_NOT_AVAILABLE;
	}

	if (id >= max_id) {
		return COM_SERVICE_NOT_AVAILABLE;
	}
	return ret;
}

/****************************************************************************
 * name: com_send_signal
 *
 * description: send signal value to pdu buffer.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_send_signal(com_signal_id_type signal_id, const void *signal_data_ptr)
{
	uint8 ret = E_OK;
	boolean data_changed = FALSE;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	ret = com_validation_common(signal_id, com_init_status_temp, COM_MAX_N_TXSIGNALS);
	if (ret != E_OK) {
		return ret;
	}
	const com_tx_signal_type *signal = COM_GET_TXSIGNAL(signal_id);
	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;

	if (NO_PDU_REFERENCE == com_ipdu_handle_id_ram) {
		ret = COM_SERVICE_NOT_AVAILABLE;
	}
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	else if (TRUE == signal->com_self_is_group_signal) {
		com_update_shadow_signal(signal_id, signal_data_ptr);
	}
#endif
	else {
		const com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(com_ipdu_handle_id_ram);
		uint8 *com_ipdu_data_ptr = (uint8 *)self_ipdu->com_ipdu_data_ptr;

		com_alg_write_tx_signal_data_to_pdu((const uint8 *)signal_data_ptr, signal, com_ipdu_data_ptr,
						    &data_changed);
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
		if (TRUE == signal->com_signal_self_use_update_bit) {
			COM_SETBIT(com_ipdu_data_ptr, signal->com_update_bit_pos);
		}
#endif
		com_transfer_property_type transfer_property = signal->com_transfer_property;

		if (transfer_property != COM_PENDING) {
			if (FALSE ==
			    com_alg_trigger_tx_on_conditions(com_ipdu_handle_id_ram, data_changed, transfer_property)) {
				ret = COM_SERVICE_NOT_AVAILABLE;
			}
		}
	}
	return ret;
}

/****************************************************************************
 * name: com_receive_signal
 *
 * description: read signal value from signal buffer.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_receive_signal(com_signal_id_type signal_id, void *signal_data_ptr)
{
	uint8 ret;

	ret = E_OK;

#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	ret = com_validation_common(signal_id, com_init_status_temp, COM_MAX_N_RXSIGNALS);
	if (ret != E_OK) {
		return ret;
	}
	if (signal_data_ptr == NULL) {
		COM_DET_REPORT(COM_COM_RECEIVESIGNAL_ID_COM_E_PARAM);
		return COM_SERVICE_NOT_AVAILABLE;
	}
	const com_rx_signal_type *signal = COM_GET_RXSIGNAL(signal_id);
	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;
	const com_self_rx_ipdu_type *self_ipdu = COM_GET_SELF_RXIPDU(com_ipdu_handle_id_ram);
	uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	core_map_idx = com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(com_ipdu_handle_id_ram);
#endif
	com_receive_sig_read_buffer(signal, signal_data_ptr, core_map_idx);

	if ((FALSE == self_ipdu->com_ipdu_started) && (E_OK == ret)) {
		ret = COM_SERVICE_NOT_AVAILABLE;
	}
	return ret;
}

/****************************************************************************
 * name: com_invalidate_signal
 *
 * description: send signal invalidate value to pdu buffer.
 *
 ****************************************************************************/
#if (COM_SIGNAL_DATAINVALIDVALUE_ENABLE == STD_ON)
FUNC(uint8, COM_CODE) com_invalidate_signal(com_signal_id_type signal_id)
{
	uint8 ret = E_OK;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	ret = com_validation_common(signal_id, com_init_status_temp, COM_MAX_N_TXSIGNALS);
	if (ret != E_OK) {
		return ret;
	}

	const com_tx_signal_type *signal = COM_GET_TXSIGNAL(signal_id);
	const uint8 *signal_data_ptr = signal->com_signal_data_invalid_value;

	if (NULL == signal_data_ptr) {
		return COM_SERVICE_NOT_AVAILABLE;
	}

	ret = com_send_signal(signal_id, signal_data_ptr);

	return ret;
}
#endif

/****************************************************************************
 * name: com_trigger_transmit
 *
 * description: trigger ipdu transmit; copy pdu buffer to low level.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE) com_trigger_transmit(PduIdType tx_pdu_id, PduInfoType *pdu_info_ptr)
{
	Std_ReturnType status = E_OK;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	uint8 ret = com_validation_common(tx_pdu_id, com_init_status_temp, COM_MAX_N_TXIPDUS);

	if (ret != E_OK) {
		return E_NOT_OK;
	}

	const com_tx_ipdu_type *ipdu = COM_GET_TXIPDU(tx_pdu_id);
	const com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(tx_pdu_id);

	uint32 com_trigger_transmit_ipdu_callout_ram = ipdu->com_trigger_transmit_ipdu_callout;
	uint16 com_ipdu_size_ram = ipdu->com_ipdu_size;

	PduInfoType pdu_info_package;

	pdu_info_package.SduDataPtr = (uint8 *)(self_ipdu->com_ipdu_data_ptr);
	pdu_info_package.SduLength = com_ipdu_size_ram;

	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	if ((COM_NO_FUNCTION_CALLOUT != com_trigger_transmit_ipdu_callout_ram) &&
	    (NULL != com_trigger_transmit_ipdu_callouts[com_trigger_transmit_ipdu_callout_ram])) {
		(void)com_trigger_transmit_ipdu_callouts[com_trigger_transmit_ipdu_callout_ram](tx_pdu_id,
												&pdu_info_package);
	}

	(void)memcpy((void *)pdu_info_ptr->SduDataPtr, (const void *)self_ipdu->com_ipdu_data_ptr, com_ipdu_size_ram);
	SchM_Exit_COM_EXCLUSIVE_AREA_0();

	pdu_info_ptr->SduLength = com_ipdu_size_ram;
	if (FALSE == self_ipdu->com_ipdu_started) {
		status = E_NOT_OK;
	}
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	else {
		if (COM_TRIGGERTRANSMIT == ipdu->com_tx_ipdu->com_tx_ipdu_clear_update_bit) {
			uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
			uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;

			for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
				const com_tx_signal_type *com_signal = COM_GET_TXSIGNAL(ipdu_signal_start_idx);

				if (TRUE == com_signal->com_signal_self_use_update_bit) {
					COM_CLEARBIT(self_ipdu->com_ipdu_data_ptr, com_signal->com_update_bit_pos);
				}
			}
		}
		status = E_OK;
	}
#endif
	return status;
}

/****************************************************************************
 * name: com_trigger_ipdu_send
 *
 * description: com send ipdu to driver.
 *
 ****************************************************************************/
FUNC(Std_ReturnType, COM_CODE)
com_trigger_ipdu_send(PduIdType pdu_id, com_self_tx_ipdu_type *self_ipdu, const com_tx_ipdu_type *ipdu)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	uint8 ret = com_validation_common(pdu_id, com_init_status_temp, COM_MAX_N_TXIPDUS);

	if (ret != E_OK) {
		return E_NOT_OK;
	}

	if (COM_TX_TRIGGERED == com_alg_trigger_ipdu_send(pdu_id, self_ipdu, ipdu)) {
		return E_OK;
	}
	return E_NOT_OK;
}

/****************************************************************************
 * name: com_rx_indication
 *
 * description: com rx indication function in interrupt context.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_rx_indication(PduIdType rx_pdu_id, const PduInfoType *pdu_info_ptr)
{
	boolean status = TRUE;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	uint8 ret = com_validation_common(rx_pdu_id, com_init_status_temp, COM_MAX_N_RXIPDUS);

	if (ret != E_OK) {
		return;
	}

	const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(rx_pdu_id);
	const com_self_rx_ipdu_type *self_ipdu = COM_GET_SELF_RXIPDU(rx_pdu_id);
	uint32 com_rx_ipdu_callout_ram = ipdu->com_rx_ipdu_callout;

	if (TRUE == self_ipdu->com_ipdu_started) {
		if (COM_NO_FUNCTION_CALLOUT != com_rx_ipdu_callout_ram) {
			if (FALSE == com_rx_ipdu_callouts[com_rx_ipdu_callout_ram](rx_pdu_id, pdu_info_ptr)) {
				status = FALSE;
			}
		}
		if (TRUE == status) {
			com_alg_rx_process(rx_pdu_id, pdu_info_ptr);
		}
	}
	return;
}

/****************************************************************************
 * name: com_tx_confirmation
 *
 * description: ipdu tx confirmation function in interrupt context.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_tx_confirmation(PduIdType tx_pdu_id, Std_ReturnType result)
{
#if ((COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON) || ((COM_IPDU_ALL_DEFERRED == STD_OFF) && (COM_TXNOTIFICATION == STD_ON)))
	uint32 ipdu_signal_start_idx;
	uint32 ipdu_signal_end_idx;
	const com_tx_signal_type *com_signal = NULL;
#endif
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	uint8 ret = com_validation_common(tx_pdu_id, com_init_status_temp, COM_MAX_N_TXIPDUS);

	if (ret != E_OK) {
		return;
	}
	if (E_OK != result) {
		return;
	}
	const com_tx_ipdu_type *ipdu = COM_GET_TXIPDU(tx_pdu_id);
	com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(tx_pdu_id);
	uint32 com_tx_ipdu_minimum_delay_factor_ram = ipdu->com_tx_ipdu->com_tx_ipdu_minimum_delay_factor;

	if (0u < self_ipdu->com_tx_deadline_cnt) {
		com_alg_tx_handle_dm(ipdu, self_ipdu);
	}

	if ((0u < com_tx_ipdu_minimum_delay_factor_ram) && (0u < self_ipdu->com_txipdu_min_delay_timer)) {
		self_ipdu->com_txipdu_min_delay_timer = com_tx_ipdu_minimum_delay_factor_ram;
	}

#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
	if (COM_CONFIRMATION == ipdu->com_tx_ipdu->com_tx_ipdu_clear_update_bit) {
		ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
		ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;
		for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
			com_signal = COM_GET_TXSIGNAL(ipdu_signal_start_idx);
			if (TRUE == com_signal->com_signal_self_use_update_bit) {
				COM_CLEARBIT(self_ipdu->com_ipdu_data_ptr, com_signal->com_update_bit_pos);
			}
		}
	}
#endif

#if (COM_IPDU_ALL_DEFERRED == STD_OFF) && (COM_TXNOTIFICATION == STD_ON)
	if (COM_IMMEDIATE == ipdu->com_ipdu_signal_processing) {
		ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
		ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;
		for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
			com_signal = COM_GET_TXSIGNAL(ipdu_signal_start_idx);
			uint32 com_notification_ram = com_signal->com_notification;

			if (COM_NO_FUNCTION_CALLOUT != com_notification_ram) {
				com_tx_signal_notification[com_notification_ram]();
			}
		}
	} else
#endif
	{
		com_alg_set_tx_confirmation_status(self_ipdu, TRUE);
	}
}

/****************************************************************************
 * name: com_send_signal_group
 *
 * description: copy signal group shadow buffer to pdu buffer.
 *
 ****************************************************************************/
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
FUNC(uint8, COM_CODE) com_send_signal_group(com_signal_group_id_type signal_group_id)
{
	uint8 ret = E_OK;
	boolean data_changed = FALSE;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	ret = com_validation_common(signal_group_id, com_init_status_temp, COM_MAX_N_TXSIGNALS);
	if (E_OK != ret) {
		return ret;
	}
	const com_tx_signal_type *signal = COM_GET_TXSIGNAL(signal_group_id);

	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;

	if (NO_PDU_REFERENCE == com_ipdu_handle_id_ram) {
		ret = COM_SERVICE_NOT_AVAILABLE;
	} else if (TRUE == signal->com_self_is_signal_group) {
		com_alg_copy_sg_data_from_sb_to_tx_pdu(signal_group_id, &data_changed);
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
		const com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(com_ipdu_handle_id_ram);

		if (TRUE == signal->com_signal_self_use_update_bit) {
			COM_SETBIT(self_ipdu->com_ipdu_data_ptr, signal->com_update_bit_pos);
		}
#endif
		com_transfer_property_type transfer_property = signal->com_transfer_property;

		if (transfer_property != COM_PENDING) {
			if (FALSE ==
			    com_alg_trigger_tx_on_conditions(com_ipdu_handle_id_ram, data_changed, transfer_property)) {
				ret = COM_SERVICE_NOT_AVAILABLE;
			}
		}
	} else {
		ret = E_NOT_OK;
	}
	return ret;
}

/****************************************************************************
 * name: com_receive_signal_group
 *
 * description: copy signal group from pdu buffer to shadow buffer.
 *
 ****************************************************************************/
FUNC(uint8, COM_CODE) com_receive_signal_group(com_signal_group_id_type signal_group_id)
{
	uint8 status;

	status = E_NOT_OK;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	status = com_validation_common(signal_group_id, com_init_status_temp, COM_MAX_N_RXSIGNALS);
	if (E_OK != status) {
		return status;
	}

	const com_rx_signal_type *signal = COM_GET_RXSIGNAL(signal_group_id);

	uint16 com_ipdu_handle_id_ram = signal->com_ipdu_id;

	if (NO_PDU_REFERENCE == com_ipdu_handle_id_ram) {
		status = COM_SERVICE_NOT_AVAILABLE;
	} else if (TRUE == signal->com_self_is_signal_group) {
		com_alg_copy_signal_group_data_from_pdu_to_shadow_buffer(signal_group_id);
		if (FALSE == COM_GET_SELF_RXIPDU(com_ipdu_handle_id_ram)->com_ipdu_started) {
			status = COM_SERVICE_NOT_AVAILABLE;
		} else {
			status = E_OK;
		}
	} else {
		/* do nothing */
	}
	return status;
}

/****************************************************************************
 * name: com_update_shadow_signal
 *
 * description: update signal shadow buffer.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_update_shadow_signal(com_signal_id_type signal_id, const void *signal_data_ptr)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	uint8 ret = com_validation_common(signal_id, com_init_status_temp, COM_MAX_N_TXSIGNALS);

	if (E_OK != ret) {
		return;
	}
	const com_self_tx_signal_type *self_group_signal = com_get_self_tx_signal(signal_id);

	if (NULL != self_group_signal->com_self_shadow_buffer) {
		com_alg_update_shadow_tx_signal(signal_id, signal_data_ptr);
	}
}
#endif

#define COM_STOP_SEC_CODE
#include "com_memmap.h"
