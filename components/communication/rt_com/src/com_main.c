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
#include "com.h"
#include "com_internal.h"
#include "com_alg.h"
#include "SchM_Com.h"

#define TIMERCNTDEC(timer)                                                                                             \
	do {                                                                                                           \
		if (0u < (timer)) {                                                                                    \
			(timer) = (timer)-1U;                                                                          \
		}                                                                                                      \
	} while (0)

#define TIMERCNTDECDIRECT(timer)                                                                                       \
	do {                                                                                                           \
		(timer) = (timer)-1U;                                                                                  \
	} while (0)

#define COM_START_SEC_CODE
#include "com_memmap.h"

#include "Compiler.h"

/****************************************************************************
 * name: com_process_mixed_or_periodic_tx_mode
 *
 * description: process mixed or periodic tx ipdu.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE)
	com_process_mixed_or_periodic_tx_mode(uint16 ipdu_id, boolean dm_time_out, const com_tx_ipdu_type *ipdu,
					      const com_tx_mode_type *tx_mode_ptr,
					      com_tx_mode_mode_type com_tx_mode_mode)
{
	com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(ipdu_id);
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
	Std_ReturnType ipdu_send_ret = E_NOT_OK;
	const com_ipdu_tx_type *com_ipdu_tx_ram = ipdu->com_tx_ipdu;
#endif
	TIMERCNTDEC(self_ipdu->com_txmode_period_timer);
	TIMERCNTDEC(self_ipdu->com_txmode_repet_period_timer);
	if ((COM_MIXED == com_tx_mode_mode) && (0u < self_ipdu->com_txipdu_repetitions_left_num)) {
		if (FALSE == dm_time_out) {
			if ((0u == self_ipdu->com_txmode_repet_period_timer) &&
			    (0u == self_ipdu->com_txipdu_min_delay_timer)) {
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
				ipdu_send_ret = com_trigger_ipdu_send((PduIdType)ipdu_id, self_ipdu, ipdu);
#else
				(void)com_trigger_ipdu_send((PduIdType)ipdu_id, self_ipdu, ipdu);
#endif
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
				if ((ipdu_send_ret == E_NOT_OK) &&
				    (TRUE == com_ipdu_tx_ram->enable_retry_failed_transmit_requests)) {
				} else
#endif
				{
					self_ipdu->com_txmode_repet_period_timer =
						tx_mode_ptr->com_tx_mode_repetition_period_factor;
					self_ipdu->com_txipdu_repetitions_left_num--;
					if (self_ipdu->com_txipdu_repetitions_left_num == 0u) {
						self_ipdu->com_txmode_repet_period_timer = 0u;
					}
				}
			}
		} else {
			self_ipdu->com_txipdu_repetitions_left_num = 0u;
			self_ipdu->com_txipdu_tx_confirmations_num = 0u;
		}
	}

	if ((0u == self_ipdu->com_txmode_period_timer) && (0u == self_ipdu->com_txipdu_min_delay_timer)) {
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
		ipdu_send_ret = com_trigger_ipdu_send((PduIdType)ipdu_id, self_ipdu, ipdu);
#else
		(void)com_trigger_ipdu_send((PduIdType)ipdu_id, self_ipdu, ipdu);
#endif
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
		if ((ipdu_send_ret == E_NOT_OK) && (TRUE == com_ipdu_tx_ram->enable_retry_failed_transmit_requests)) {
		} else
#endif
		{
			self_ipdu->com_txmode_period_timer = tx_mode_ptr->com_tx_mode_time_period_factor;
		}

		if ((0u < self_ipdu->com_tx_deadline_cnt) && (0u == self_ipdu->com_tx_dm_timer)) {
			self_ipdu->com_tx_dm_timer = self_ipdu->com_tx_deadline_cnt;
		}
	}
	return;
}

/****************************************************************************
 * name: com_process_direct_tx_mode
 *
 * description: process direct tx ipdu.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE)
	com_process_direct_tx_mode(uint16 ipdu_id, boolean dm_time_out, const com_tx_ipdu_type *ipdu,
				   const com_tx_mode_type *tx_mode_ptr)
{
	com_self_tx_ipdu_type *self_ipdu = COM_GET_SELF_TXIPDU(ipdu_id);

	TIMERCNTDEC(self_ipdu->com_txmode_repet_period_timer);
	if (0u < self_ipdu->com_txipdu_repetitions_left_num) {
		if (FALSE == dm_time_out) {
			if ((0u == self_ipdu->com_txmode_repet_period_timer) &&
			    (0u == self_ipdu->com_txipdu_min_delay_timer)) {
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
				Std_ReturnType ipdu_send_ret = E_NOT_OK;

				ipdu_send_ret = com_trigger_ipdu_send((PduIdType)ipdu_id, self_ipdu, ipdu);
#else
				(void)com_trigger_ipdu_send((PduIdType)ipdu_id, self_ipdu, ipdu);
#endif
#if (COM_IPDU_RETRYFAILEDTRANS_ENABLE == STD_ON)
				if ((ipdu_send_ret == E_NOT_OK) &&
				    (TRUE == ipdu->com_tx_ipdu->enable_retry_failed_transmit_requests)) {
				} else
#endif
				{
					self_ipdu->com_txmode_repet_period_timer =
						tx_mode_ptr->com_tx_mode_repetition_period_factor;
					self_ipdu->com_txipdu_repetitions_left_num--;
					if (self_ipdu->com_txipdu_repetitions_left_num == 0u) {
						self_ipdu->com_txmode_repet_period_timer = 0u;
					}
				}
			}
		} else {
			self_ipdu->com_txipdu_repetitions_left_num = 0u;
			self_ipdu->com_txipdu_tx_confirmations_num = 0u;
		}
	}
	return;
}

/****************************************************************************
 * name: com_rxipu_tout_siggroup
 *
 * description: com rxipu tout siggroup.
 *
 ****************************************************************************/
#if (COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE == STD_ON) && (COM_SIGNAL_GROUP_ENABLE == STD_ON)
static FUNC(void, COM_CODE) com_rxipu_tout_siggroup(const com_rx_signal_type *com_signal,
						    com_self_rx_ipdu_type *self_ipdu, const com_rx_ipdu_type *ipdu)
{
	boolean signal_changed = FALSE;

	if (COM_TIMEOUT_DATA_ACTION_REPLACE == com_signal->com_rx_data_timeout_action) {
		const com_rx_signal_type *com_groupsignal = NULL;
		uint16 group_signal_id = 0;

		for (uint32 i = 0u; COM_GROUPSIGNALREFS_END != com_signal->com_group_signal[i]; i++) {
			group_signal_id = com_signal->com_group_signal[i];
			com_groupsignal = COM_GET_RXSIGNAL(group_signal_id);
			com_alg_write_rx_signal_data_to_pdu(com_groupsignal->com_signal_init_value, com_groupsignal,
							    self_ipdu->com_ipdu_data_ptr, &signal_changed);
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
			if ((TRUE == ipdu->com_ipdu_gw_routing_req) &&
			    (TRUE == com_groupsignal->com_sig_gw_routing_req)) {
				com_get_self_rx_signal(group_signal_id)->com_signal_updated_gw_routing = TRUE;
				if (ipdu->com_ipdu_signal_processing == COM_IMMEDIATE) {
					com_get_self_rx_signal(group_signal_id)->com_signal_routing_req =
						com_get_self_rx_signal(group_signal_id)->com_signal_updated_gw_routing;
					com_get_self_rx_signal(group_signal_id)->com_signal_updated_gw_routing = FALSE;
				}
			}
#endif
		}
		self_ipdu->com_ipdu_updated = TRUE;
	}
}
#endif

/****************************************************************************
 * name: com_rxipu_tout_sig
 *
 * description: com rxipu tout sig.
 *
 ****************************************************************************/
#if (COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE == STD_ON)
static FUNC(void, COM_CODE) com_rxipu_tout_sig(const com_rx_signal_type *com_signal, com_self_rx_ipdu_type *self_ipdu,
					       uint16 pduid, com_self_rx_signal_type *self_signal, const com_rx_ipdu_type *ipdu)
{
	boolean signal_changed = FALSE;

	if (COM_TIMEOUT_DATA_ACTION_REPLACE == com_signal->com_rx_data_timeout_action) {
		self_ipdu->com_ipdu_updated = TRUE;
		if (ipdu->com_ipdu_signal_processing == COM_IMMEDIATE) {
			uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_ON)
			core_map_idx = com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(pduid);
#endif
			com_rx_processing_init_rx_signal(com_signal, core_map_idx);
		} else {
			com_alg_write_rx_signal_data_to_pdu(com_signal->com_signal_init_value, com_signal,
							    self_ipdu->com_ipdu_data_ptr, &signal_changed);
		}
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
		if ((TRUE == ipdu->com_ipdu_gw_routing_req) && (TRUE == com_signal->com_sig_gw_routing_req)) {
			self_signal->com_signal_updated_gw_routing = TRUE;
			if (ipdu->com_ipdu_signal_processing == COM_IMMEDIATE) {
				self_signal->com_signal_routing_req = self_signal->com_signal_updated_gw_routing;
				self_signal->com_signal_updated_gw_routing = FALSE;
			}
		}
#endif
	}
	(void)self_signal;
}
#endif

/****************************************************************************
 * name: com_rxipu_tout_sig_replace
 *
 * description: com rxipu tout sig replace value.
 *
 ****************************************************************************/
#if (COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE == STD_ON)
static FUNC(void, COM_CODE)
	com_rxipu_tout_sig_replace(const com_rx_signal_type *com_signal, com_self_rx_ipdu_type *self_ipdu,
				   const com_rx_ipdu_type *ipdu, uint16 pduid, com_self_rx_signal_type *self_signal)
{
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
	if (FALSE != com_signal->com_self_is_signal_group) {
		com_rxipu_tout_siggroup(com_signal, self_ipdu, ipdu);
	} else {
		com_rxipu_tout_sig(com_signal, self_ipdu, pduid, self_signal, ipdu);
	}
#else
	com_rxipu_tout_sig(com_signal, self_ipdu, pduid, self_signal, ipdu);
	(void)ipdu;
#endif
}
#endif

/****************************************************************************
 * name: com_tout_mon_rx
 *
 * description: rx ipdu deadline monitor.
 *
 ****************************************************************************/
#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF))
static FUNC(void, COM_CODE) com_tout_mon_rx(uint8 main_function_id)
{
	uint32 idx;
	uint16 signalid;
	uint16 pduid;
	const com_rx_signal_type *com_signal = NULL;
	com_self_rx_signal_type *self_signal = NULL;
	uint32 com_timeout_notification_ram;
	const com_rx_tout_info_type *signal_tout_info = NULL;
	com_self_rx_ipdu_type *self_ipdu = NULL;
	const com_rx_ipdu_type *ipdu = NULL;
	uint16 start_idx = com_rx_main_function_des[main_function_id].start_idx;

	if (start_idx == NO_PDU_REFERENCE) {
		return;
	}
	uint16 rx_tout_signal_num = com_rx_main_function_des[main_function_id].rx_tout_signal_num;
	uint16 end_idx = start_idx + rx_tout_signal_num;

	for (idx = start_idx; idx < end_idx; idx++) {
		signal_tout_info = &com_rx_signal_tout_info[idx];
		pduid = signal_tout_info->com_ipdu_id_ref;
		self_ipdu = COM_GET_SELF_RXIPDU(pduid);
		ipdu = COM_GET_RXIPDU(pduid);
		if ((TRUE == self_ipdu->com_rxipdu_dm_ctrl) && (TRUE == self_ipdu->com_ipdu_started)) {
			signalid = signal_tout_info->com_signal_id_ref;
			self_signal = com_get_self_rx_signal(signalid);
			if (0u != self_signal->com_rx_deadline_cnt) {
				TIMERCNTDECDIRECT(self_signal->com_rx_deadline_cnt);
				if (0u == self_signal->com_rx_deadline_cnt) {
					com_signal = COM_GET_RXSIGNAL(signalid);
#if (COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE == STD_ON)
					com_rxipu_tout_sig_replace(com_signal, self_ipdu, ipdu, pduid, self_signal);
#endif
					com_timeout_notification_ram = com_signal->com_timeout_notification;
					if (COM_NO_FUNCTION_CALLOUT != com_timeout_notification_ram) {
						com_rx_signal_timeout_notification[com_timeout_notification_ram]();
					}
					self_signal->com_rx_deadline_cnt = com_signal->com_timeout_factor;
				}
			}
		}
	}
	(void)ipdu;
	return;
}
#endif

/****************************************************************************
 * name: com rx handle signal tout
 *
 * description: com rx handle signal tout.
 *
 ****************************************************************************/
#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
#if (COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE == STD_ON)
static FUNC(void, COM_CODE) com_rx_handle_signal_tout(com_self_rx_ipdu_type *self_ipdu, uint16 ipdu_id)
{
	const com_rx_ipdu_type *ipdu = COM_GET_RXIPDU(ipdu_id);
	uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
	uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;
	const com_rx_signal_type *com_signal = NULL;
	com_self_rx_signal_type *self_signal = NULL;

	for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
		com_signal = COM_GET_RXSIGNAL(ipdu_signal_start_idx);
		self_signal = com_get_self_rx_signal(ipdu_signal_start_idx);
		com_rxipu_tout_sig_replace(com_signal, self_ipdu, ipdu, ipdu_id, self_signal);
	}
	return;
}
#endif

/****************************************************************************
 * name: com time out mon rx notifications
 *
 * description: com_tout_mon_rx_notifications.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE)
	com_tout_mon_rx_notifications(com_self_rx_ipdu_type *self_ipdu, uint32 idx)
{
	uint16 start_timeout_noti_idx = com_rx_pdu_tout_struct[idx].start_timeout_noti_idx;
	uint16 timeout_noti_num = com_rx_pdu_tout_struct[idx].timeout_noti_num;

	if (start_timeout_noti_idx != COM_NO_REF_INDEX) {
		uint16 end_timeout_noti_idx = start_timeout_noti_idx + timeout_noti_num;

		for (uint16 i = start_timeout_noti_idx; i < end_timeout_noti_idx; i++) {
			com_rx_signal_timeout_notification[i]();
		}
	}
	self_ipdu->com_rx_deadline_cnt = com_rx_pdu_tout_struct[idx].com_timeout_factor;
	return;
}

/****************************************************************************
 * name: com_tout_mon_rx_signal
 *
 * description: com_tout_mon_rx_signal.
 *
 ****************************************************************************/
static inline FUNC(void, COM_CODE) com_tout_mon_rx_signal(uint8 main_function_id)
{
	uint32 idx;
	uint16 ipdu_id = 0;
	uint16 rx_toutmessag_start_idx = com_rx_main_function_des[main_function_id].rx_toutmessag_start_idx;

	if (rx_toutmessag_start_idx != NO_PDU_REFERENCE) {
		com_self_rx_ipdu_type *self_ipdu = NULL;
		uint16 rx_toutmessag_num = com_rx_main_function_des[main_function_id].rx_toutmessag_num;
		uint16 rx_toutmessag_end_idx = rx_toutmessag_start_idx + rx_toutmessag_num;

		for (idx = rx_toutmessag_start_idx; idx < rx_toutmessag_end_idx; idx++) {
			ipdu_id = com_rx_pdu_tout_struct[idx].ipdu_id_ref;
			self_ipdu = COM_GET_SELF_RXIPDU(ipdu_id);
			if ((TRUE == self_ipdu->com_rxipdu_dm_ctrl) && (TRUE == self_ipdu->com_ipdu_started)) {
				if (0u != self_ipdu->com_rx_deadline_cnt) {
					TIMERCNTDECDIRECT(self_ipdu->com_rx_deadline_cnt);
					if (0u == self_ipdu->com_rx_deadline_cnt) {
						com_tout_mon_rx_notifications(self_ipdu, idx);
#if (COM_TIMEOUT_DATA_ACTION_REPLACE_ENABLE == STD_ON)
						com_rx_handle_signal_tout(self_ipdu, ipdu_id);
#endif
					}
				}
			}
		}
	}
	return;
}
#endif

/****************************************************************************
 * name: com_process_rx_signal_ub
 *
 * description: rx signal process to  check signal update bit.
 *
 ****************************************************************************/
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
static FUNC(boolean, COM_CODE)
	com_process_rx_signal_ub(const com_rx_signal_type *com_signal, const com_self_rx_ipdu_type *self_ipdu)
{
	boolean com_signal_self_use_update_bit_ram = com_signal->com_signal_self_use_update_bit;
	boolean sig_ub_states = FALSE;

	if ((FALSE == com_signal_self_use_update_bit_ram) ||
	    ((TRUE == com_signal_self_use_update_bit_ram) &&
	     (COM_TESTBIT(self_ipdu->com_ipdu_data_ptr, com_signal->com_update_bit_pos) > 0u))) {
		sig_ub_states = TRUE;
	}
	return sig_ub_states;
}
#endif

/****************************************************************************
 * name: com_process_rx_signal
 *
 * description: process rx ipdu to rx signal.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE)
	com_process_rx_signal(const com_rx_ipdu_type *ipdu, com_self_rx_ipdu_type *self_ipdu,
					   uint8 core_map_idx, uint8 main_function_id)
{
	uint16 ipdu_signal_start_idx = ipdu->signal_start_idx;
	uint16 ipdu_signal_end_idx = ipdu->signal_end_idx;
	const uint8 *pdu_buffer = self_ipdu->com_ipdu_data_ptr;
	const com_rx_signal_type *com_signal = NULL;
	uint16 signal_idx = 0;
	uint32 com_notification_ram = 0;
	uint16 *rx_notification_record = com_rx_main_function_des[main_function_id].rx_notification_record_ref;
	uint16 ipdu_max_sig_num = com_rx_main_function_des[main_function_id].ipdu_max_sig_num;

	SchM_Enter_COM_EXCLUSIVE_AREA_0();
	for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
		com_signal = COM_GET_RXSIGNAL(ipdu_signal_start_idx);
#if (COM_SIGNAL_UPDATEBIT_ENABLE == STD_ON)
		boolean ub_states = com_process_rx_signal_ub(com_signal, self_ipdu);

		if (ub_states == TRUE)
#endif
		{
#if (COM_SIGNAL_GROUP_ENABLE == STD_ON)
			if ((FALSE == com_signal->com_self_is_signal_group) &&
			    (FALSE == com_signal->com_self_is_group_signal)) {
				com_alg_processing_rx_signal_data_from_pdu(pdu_buffer, com_signal, core_map_idx);
			}
#else
			com_alg_processing_rx_signal_data_from_pdu(pdu_buffer, com_signal, core_map_idx);
#endif
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
			com_self_rx_signal_type *self_signal = com_get_self_rx_signal(ipdu_signal_start_idx);

			self_signal->com_signal_routing_req = self_signal->com_signal_updated_gw_routing;
			self_signal->com_signal_updated_gw_routing = FALSE;
#endif
			com_notification_ram = com_signal->com_notification;
			if (COM_NO_FUNCTION_CALLOUT != com_notification_ram) {
				if (signal_idx < ipdu_max_sig_num) {
					rx_notification_record[signal_idx] = (uint16)com_notification_ram;
					signal_idx++;
				}
			}
		}
	}
	SchM_Exit_COM_EXCLUSIVE_AREA_0();
	for (uint16 i = 0; i < signal_idx; i++) {
		com_rx_signal_notification[rx_notification_record[i]]();
	}
	return;
}

/****************************************************************************
 * name: com_process_deffered_rxpdu_sub
 *
 * description: process deffered pdu sub function.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_process_deffered_rxpdu_sub(uint16 pdu_id, const com_rx_ipdu_type *ipdu,
							       com_self_rx_ipdu_type *self_ipdu, uint8 main_function_id)
{
	self_ipdu->com_ipdu_updated = FALSE;
	uint8 core_map_idx = 0;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	core_map_idx = com_alg_get_rx_ipdu_core_map_idx_by_pdu_id(pdu_id);
#endif
	com_process_rx_signal(ipdu, self_ipdu, core_map_idx, main_function_id);
	return;
}

/****************************************************************************
 * name: com_process_deffered_rxpdu
 *
 * description: process deffered pdu function.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_process_deffered_rxpdu(uint8 main_function_id)
{
	com_self_rx_ipdu_type *self_ipdu = NULL;
	const com_rx_ipdu_type *ipdu = NULL;
	uint16 start_ipdu_idx_of_main_function_rx =
		com_rx_main_function_des[main_function_id].start_ipdu_idx_of_main_function_rx;
	uint16 ipdu_num_of_main_function_rx =
		com_rx_main_function_des[main_function_id].ipdu_num_of_main_function_rx;
	uint16 end_ipdu_idx_of_main_function_rx = start_ipdu_idx_of_main_function_rx + ipdu_num_of_main_function_rx;

	for (uint16 pdu_id = start_ipdu_idx_of_main_function_rx; pdu_id < end_ipdu_idx_of_main_function_rx; pdu_id++) {
		self_ipdu = COM_GET_SELF_RXIPDU(pdu_id);
		if ((TRUE == self_ipdu->com_ipdu_started) && (TRUE == self_ipdu->com_ipdu_updated)) {
			ipdu = COM_GET_RXIPDU(pdu_id);
#if (COM_IPDU_ALL_DEFERRED == STD_ON)
			com_process_deffered_rxpdu_sub(pdu_id, ipdu, self_ipdu, main_function_id);
#else
			if (COM_DEFERRED == ipdu->com_ipdu_signal_processing) {
				com_process_deffered_rxpdu_sub(pdu_id, ipdu, self_ipdu, main_function_id);
			}
#endif
		}
	}
	return;
}

/****************************************************************************
 * name: com_main_function_rx_run
 *
 * description: com rx internal main function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_main_function_rx_run(uint8 main_function_id)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	if (COM_INIT != com_init_status_temp) {
		COM_DET_REPORT(COM_COM_MAINFUNCTIONRX_ID_COM_E_UNINIT);
		return;
	}
	if (main_function_id >= COM_MAINFUNCTION_RX_NUM) {
		COM_DET_REPORT(COM_COM_MAINFUNCTIONRX_ID_COM_E_PARAM);
		return;
	}
#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF))
	if (com_rx_main_function_des[main_function_id].rx_timeout_info == TRUE) {
		com_tout_mon_rx(main_function_id);
	}
#endif

#if ((COM_RXTOU_ENABLE == STD_ON) && (COM_RXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
	if (com_rx_main_function_des[main_function_id].rx_timeout_info == TRUE) {
		com_tout_mon_rx_signal(main_function_id);
	}
#endif

	com_process_deffered_rxpdu(main_function_id);
	return;
}

/****************************************************************************
 * name: com_tx_mode_hdlr_main_function_tx
 *
 * description: tx mode handle main function.
 *
 ****************************************************************************/
static FUNC(void, COM_CODE) com_tx_mode_hdlr_main_function_tx(uint16 pdu_id, const com_tx_ipdu_type *ipdu,
							      com_self_tx_ipdu_type *self_ipdu, boolean dm_time_out)
{
	const com_tx_mode_type *tx_mode_ptr = NULL;
	const com_ipdu_tx_type *com_ipdu_tx_ram = ipdu->com_tx_ipdu;

	if (TRUE == self_ipdu->com_txipdu_tx_mode) {
		tx_mode_ptr = com_ipdu_tx_ram->com_tx_mode_true;
	} else {
		tx_mode_ptr = com_ipdu_tx_ram->com_tx_mode_false;
	}
	TIMERCNTDEC(self_ipdu->com_txipdu_min_delay_timer);

	com_tx_mode_mode_type com_tx_mode_mode_ram = tx_mode_ptr->com_tx_mode_mode;

	if ((COM_PERIODIC == com_tx_mode_mode_ram) || (COM_MIXED == com_tx_mode_mode_ram)) {
		com_process_mixed_or_periodic_tx_mode(pdu_id, dm_time_out, ipdu, tx_mode_ptr, com_tx_mode_mode_ram);
	} else if (COM_DIRECT == com_tx_mode_mode_ram) {
		com_process_direct_tx_mode(pdu_id, dm_time_out, ipdu, tx_mode_ptr);
	} else {
		/* do nothing */
	}
	return;
}

/****************************************************************************
 * name: com_tx_time_out_main_function_tx
 *
 * description: tx timeout main function.
 *
 ****************************************************************************/
#if (COM_TXTOU_ENABLE == STD_ON)
static FUNC(boolean, COM_CODE)
	com_tx_time_out_main_function_tx(com_self_tx_ipdu_type *self_ipdu, const com_tx_ipdu_type *ipdu)
{
	boolean dm_time_out = FALSE;

	if (self_ipdu->com_tx_deadline_cnt > 0u) {
		if (self_ipdu->com_tx_dm_timer > 0u) {
			TIMERCNTDECDIRECT(self_ipdu->com_tx_dm_timer);
			if (self_ipdu->com_tx_dm_timer == 0u) {
				dm_time_out = TRUE;
			}
		}
	}
	if (TRUE == dm_time_out) {
#if (COM_TXIPDU_SAME_TIMEOUT_ENABLE == STD_OFF)
		uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
		uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;
		const com_tx_signal_type *com_signal;
		uint32 com_timeout_notification_raw;

		;
		for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
			com_signal = COM_GET_TXSIGNAL(ipdu_signal_start_idx);
			com_timeout_notification_raw = com_signal->com_timeout_notification;
			if (COM_NO_FUNCTION_CALLOUT != com_timeout_notification_raw) {
				com_tx_signal_timeout_notification[com_timeout_notification_raw]();
			}
		}
#endif
#if ((COM_TXTOU_ENABLE == STD_ON) && (COM_TXIPDU_SAME_TIMEOUT_ENABLE == STD_ON))
		uint16 tx_tout_info_idx = ipdu->tx_tout_info_idx;

		if (tx_tout_info_idx != COM_NO_REF_INDEX) {
			uint16 start_timeout_noti_idx =
				com_tx_pdu_tout_struct[tx_tout_info_idx].start_timeout_noti_idx;
			uint16 timeout_noti_num = com_tx_pdu_tout_struct[tx_tout_info_idx].timeout_noti_num;

			if (start_timeout_noti_idx != COM_NO_REF_INDEX) {
				uint16 end_timeout_noti_idx = start_timeout_noti_idx + timeout_noti_num;

				for (uint16 tx_tout_noti_idx = start_timeout_noti_idx;
				     tx_tout_noti_idx < end_timeout_noti_idx; tx_tout_noti_idx++) {
					com_tx_signal_timeout_notification[tx_tout_noti_idx]();
				}
			}
		}
#endif
	}
	return dm_time_out;
}
#endif

/****************************************************************************
 * name: com_tx_confirmation_main_function_tx
 *
 * description: tx confirmation main function.
 *
 ****************************************************************************/
#if (COM_TXNOTIFICATION == STD_ON)
static FUNC(void, COM_CODE) com_tx_confirmation_main_function_tx(uint8 main_function_id, const com_tx_ipdu_type *ipdu,
								 com_self_tx_ipdu_type *self_ipdu)
{
	if (com_tx_main_function_des[main_function_id].tx_comfirmation_info == TRUE) {
		boolean confirmation_status = com_alg_get_tx_confirmation_status(self_ipdu);

		com_alg_set_tx_confirmation_status(self_ipdu, FALSE);
		if (confirmation_status == TRUE) {
			uint32 ipdu_signal_start_idx = (uint32)ipdu->signal_start_idx;
			uint32 ipdu_signal_end_idx = (uint32)ipdu->signal_end_idx;
			const com_tx_signal_type *com_signal;
			uint32 com_notification_raw;

			for (; ipdu_signal_start_idx < ipdu_signal_end_idx; ipdu_signal_start_idx++) {
				com_signal = COM_GET_TXSIGNAL(ipdu_signal_start_idx);
				com_notification_raw = com_signal->com_notification;
				if (COM_NO_FUNCTION_CALLOUT != com_notification_raw) {
					com_tx_signal_notification[com_notification_raw]();
				}
			}
		}
	}
	return;
}
#endif

/****************************************************************************
 * name: com_main_function_tx_run
 *
 * description: com tx internal main function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_main_function_tx_run(uint8 main_function_id)
{
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	if (COM_INIT != com_init_status_temp) {
		COM_DET_REPORT(COM_COM_MAINFUNCTIONTX_ID_COM_E_UNINIT);
		return;
	}
	if (main_function_id >= COM_MAINFUNCTION_TX_NUM) {
		COM_DET_REPORT(COM_COM_MAINFUNCTIONTX_ID_COM_E_PARAM);
		return;
	}
	uint16 start_ipdu_idx_of_main_function_tx =
		com_tx_main_function_des[main_function_id].start_ipdu_idx_of_main_function_tx;
	uint16 ipdu_num_of_main_function_tx =
		com_tx_main_function_des[main_function_id].ipdu_num_of_main_function_tx;
	if (start_ipdu_idx_of_main_function_tx != NO_PDU_REFERENCE) {
		uint16 end_ipdu_idx_of_main_function_tx =
			start_ipdu_idx_of_main_function_tx + ipdu_num_of_main_function_tx;
		const com_tx_ipdu_type *ipdu;
		com_self_tx_ipdu_type *self_ipdu = NULL;
		boolean dm_time_out;

		for (uint16 ipdu_idx = start_ipdu_idx_of_main_function_tx; ipdu_idx < end_ipdu_idx_of_main_function_tx;
		     ipdu_idx++) {
			self_ipdu = COM_GET_SELF_TXIPDU(ipdu_idx);
			ipdu = &com_tx_ipdu_const[ipdu_idx];
			dm_time_out = FALSE;
			if (TRUE == self_ipdu->com_ipdu_started) {
#if (COM_TXTOU_ENABLE == STD_ON)
				dm_time_out = com_tx_time_out_main_function_tx(self_ipdu, ipdu);
#endif
				com_tx_mode_hdlr_main_function_tx(ipdu_idx, ipdu, self_ipdu, dm_time_out);
#if (COM_TXNOTIFICATION == STD_ON)
				com_tx_confirmation_main_function_tx(main_function_id, ipdu, self_ipdu);
#endif
			}
		}
	}
	return;
}

/****************************************************************************
 * name: com_gw_signal
 *
 * description: com signal gateway function.
 *
 ****************************************************************************/
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
static FUNC(void, COM_CODE) com_gw_signal(uint8 gw_mapidx, uint16 sig_handle)
{
	uint8 data[64] = { 0u };
	boolean is_updated = FALSE;
	const com_rx_signal_type *com_signal_src = COM_GET_RXSIGNAL(sig_handle);

	is_updated = com_get_self_rx_signal(sig_handle)->com_signal_routing_req;
	uint16 ipdu_handle = com_signal_src->com_ipdu_id;

	if (TRUE == is_updated) {
		com_alg_extract_gw_src_sig_data(com_signal_src, ipdu_handle, data);
		com_alg_route_gw_destn_signals(gw_mapidx, data);
		com_get_self_rx_signal(sig_handle)->com_signal_routing_req = FALSE;
	}
	return;
}
#endif

/****************************************************************************
 * name: com_main_function_gw_run
 *
 * description: com signal gateway main function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_main_function_gw_run(uint8 main_function_id)
{
	(void)main_function_id;
#if (COM_SUPPORT_MUTICORE == STD_ON)
	com_core_id_type core_id = (com_core_id_type)GetCoreID();
	uint8 com_init_status_temp = com_init_status[core_id];
#else
	uint8 com_init_status_temp = com_init_status;
#endif
	if (COM_INIT != com_init_status_temp) {
		COM_DET_REPORT(COM_COM_MAINFUNCTIONROUTESIGNALS_ID_COM_E_UNINIT);
		return;
	}

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)

	uint16 sig_handle;

	for (uint8 i = 0u; i < COM_MAX_N_GWMAPPINGS; i++) {
		sig_handle = com_config->com_gw_mapping_ref[i].com_gw_src_sig_id;
		com_gw_signal(i, sig_handle);
	}
#endif
	return;
}

#define COM_STOP_SEC_CODE
#include "com_memmap.h"
