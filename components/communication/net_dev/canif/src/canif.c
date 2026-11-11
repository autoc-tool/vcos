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

/* Including File */
#include <string.h>

#include "canif.h"
#include "canif_can.h"
#include "canif_internal.h"
#include "canif_pbcfg.h"
#include "canif_vcos_log.h"
#include "rtfw_atomic.h"
#if (CONFIG_MODULE_CANNM == 1)
#include "cannm.h"
#endif

#include "SchM_CanIf.h"
#include "vcos_log.h"

#define CANIF_START_SEC_CODE
#include "canif_memmap.h"

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
static FUNC(Std_ReturnType, CANIF_CODE)
	canif_queue_item_replace_or_add(const canif_tx_buffer_config_type *buffer_ptr, const Can_PduType *can_pdu_ptr,
					boolean replace_allowed)
{
	Std_ReturnType ret = E_NOT_OK;
	canif_buffer_entry_type *buff_ptr = NULL;
	uint16 start_index = CANIF_INVALID_TXBUFFER_START_INDEX;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return ret;
	}

	SchM_Enter_CANIF_EXCLUSIVE_AREA_0();

	const uint16 *buffer_start_index_ptr = buffer_start_index[core_id];

	if (NULL != buffer_start_index_ptr) {
		start_index = buffer_start_index_ptr[buffer_ptr->tx_buffer_id];
	}

#if (CANIF_TX_BUF_FIFO_ENABLE == STD_ON)
	canif_fifo_buffer_info_type *fifo_buffer_info_ptr = fifo_buffer_info[core_id];
	canif_buffer_entry_type *tx_pdu_buffer_ptr = tx_pdu_buffer[core_id];

	if ((NULL != fifo_buffer_info_ptr) && (NULL != tx_pdu_buffer_ptr) &&
	    (CANIF_INVALID_TXBUFFER_START_INDEX != start_index)) {
		if (fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_queue_size < buffer_ptr->tx_buffer_size) {
			uint16 u16_write_index = fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_write_index;

			buff_ptr = &tx_pdu_buffer_ptr[u16_write_index + start_index];
			fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_write_index++;
			fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_queue_size++;
			if (fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_write_index >=
			    (buffer_ptr->tx_buffer_size)) {
				fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_write_index = 0;
			}
#if (CANIF_PUBLIC_TXBUFFER_MAX_DEPTH_FETCH_SUPPORT == STD_ON)
			uint16 *canif_tx_buffer_max_depth_ptr = canif_tx_buffer_max_depth[core_id];

			if ((NULL != canif_tx_buffer_max_depth_ptr) &&
			    (fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_queue_size >
			     canif_tx_buffer_max_depth_ptr[buffer_ptr->tx_buffer_id])) {
				canif_tx_buffer_max_depth_ptr[buffer_ptr->tx_buffer_id] =
					fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_queue_size;
			}
#endif
		} else {
			ret = E_NOT_OK;
		}
	}
#endif

	if (NULL != buff_ptr) {
		buff_ptr->in_use = TRUE;
		buff_ptr->l_pdu.dlc = can_pdu_ptr->length;
		buff_ptr->l_pdu.can_id = can_pdu_ptr->id;
		buff_ptr->l_pdu.pdu_id = can_pdu_ptr->swPduHandle;
		(void)memcpy(buff_ptr->l_pdu.data, can_pdu_ptr->sdu, can_pdu_ptr->length);
		ret = E_OK;
	}

	SchM_Exit_CANIF_EXCLUSIVE_AREA_0();
	return ret;
}

static FUNC(Std_ReturnType, CANIF_CODE)
	canif_get_pdu_from_queue(const canif_tx_buffer_config_type *buffer_ptr, Can_PduType *can_pdu_ptr)
{
	Std_ReturnType ret = E_NOT_OK;
	boolean found = FALSE;
	uint16 start_index = CANIF_INVALID_TXBUFFER_START_INDEX;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return ret;
	}

	SchM_Enter_CANIF_EXCLUSIVE_AREA_0();

	const uint16 *buffer_start_index_ptr = buffer_start_index[core_id];
	canif_buffer_entry_type *tx_pdu_buffer_ptr = tx_pdu_buffer[core_id];

	if (NULL != buffer_start_index_ptr) {
		start_index = buffer_start_index_ptr[buffer_ptr->tx_buffer_id];
	}

#if (CANIF_TX_BUF_FIFO_ENABLE == STD_ON)
	canif_fifo_buffer_info_type *fifo_buffer_info_ptr = fifo_buffer_info[core_id];

	if ((NULL != fifo_buffer_info_ptr) && (NULL != tx_pdu_buffer_ptr) &&
	    (CANIF_INVALID_TXBUFFER_START_INDEX != start_index)) {
		if (fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_queue_size != 0UL) {
			uint16 u16_read_index = fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_read_index;

			can_pdu_ptr->id = tx_pdu_buffer_ptr[u16_read_index + start_index].l_pdu.can_id;
			can_pdu_ptr->length = tx_pdu_buffer_ptr[u16_read_index + start_index].l_pdu.dlc;
			can_pdu_ptr->swPduHandle = tx_pdu_buffer_ptr[u16_read_index + start_index].l_pdu.pdu_id;
			can_pdu_ptr->sdu = tx_pdu_buffer_ptr[u16_read_index + start_index].l_pdu.data;

			tx_pdu_buffer_ptr[u16_read_index + start_index].in_use = FALSE;
			found = TRUE;

			fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_read_index++;
			fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_queue_size--;

			if (fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_read_index >=
			    buffer_ptr->tx_buffer_size) {
				fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_read_index = 0UL;
			}
		} else {
			found = FALSE;
		}
	}
#endif

	SchM_Exit_CANIF_EXCLUSIVE_AREA_0();
	if (found)
		ret = E_OK;
	else
		ret = E_NOT_OK;

	return ret;
}

static FUNC(void, CANIF_CODE) canif_clear_queue(const canif_tx_buffer_config_type *buffer_ptr)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}

	uint16 start_index = CANIF_INVALID_TXBUFFER_START_INDEX;

	SchM_Enter_CANIF_EXCLUSIVE_AREA_0();

	canif_buffer_entry_type *tx_pdu_buffer_ptr = tx_pdu_buffer[core_id];
	const uint16 *buffer_start_index_ptr = buffer_start_index[core_id];

	if (NULL != buffer_start_index_ptr) {
		start_index = buffer_start_index_ptr[buffer_ptr->tx_buffer_id];
		if ((CANIF_INVALID_TXBUFFER_START_INDEX != start_index) && (NULL != tx_pdu_buffer_ptr)) {
			for (uint16 i = start_index; i < (start_index + buffer_ptr->tx_buffer_size); i++) {
				tx_pdu_buffer_ptr[i].in_use = FALSE;
			}
		}
	}

#if (CANIF_TX_BUF_FIFO_ENABLE == STD_ON)
	canif_fifo_buffer_info_type *fifo_buffer_info_ptr = fifo_buffer_info[core_id];

	if (NULL != fifo_buffer_info_ptr) {
		fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_write_index = 0u;
		fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_read_index = 0u;
		fifo_buffer_info_ptr[buffer_ptr->tx_buffer_id].u16_queue_size = 0u;
	}
#endif

	SchM_Exit_CANIF_EXCLUSIVE_AREA_0();
}
#endif

#if (CANIF_TRANSCEIVER_API == STD_ON)
static FUNC(Std_ReturnType, CANIF_CODE) trcv_to_chnl_id(uint8 transceiver, uint8 *channel)
{
	Std_ReturnType ret = E_NOT_OK;

	if (NULL != channel) {
		const canif_transceiver_config_type *canif_transceiver_config_ptr = canif_config_ptr->canif_trcv_config;
		uint8 i = 0;

		while (i < CANIF_TRANSCEIVER_CHANNEL_CNT) {
			if (canif_transceiver_config_ptr[i].canif_cantrcv_id == transceiver) {
				*channel = i;
				ret = E_OK;
				break;
			}
			i++;
		}
	}
	return ret;
}
#endif

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
static FUNC(void, CANIF_CODE) canif_clear_tx_buffers(canif_channel_id_type chnl_id)
{
	const canif_tx_buffer_config_type *const *tx_buffer_ref_list =
		canif_config_ptr->canif_ctrl_config[chnl_id].canif_ctrl_tx_buf_ref_list;
	uint16 cur_chnl_tx_buf_cnt = canif_config_ptr->canif_ctrl_config[chnl_id].canif_ctrl_tx_buf_num;

	for (uint16 i = 0; i < cur_chnl_tx_buf_cnt; i++) {
		if (NULL != tx_buffer_ref_list[i]) {
			canif_clear_queue(tx_buffer_ref_list[i]);
		}
	}
}

static FUNC(void, CANIF_CODE)
	canif_enter_pdu_mode_with_tx_clear(canif_channel_id_type chnl_id, canif_pdu_mode_type pdu_mode)
{
	if ((CANIF_OFFLINE == pdu_mode) || (CANIF_TX_OFFLINE == pdu_mode)) {
		canif_clear_tx_buffers(chnl_id);
	}
}

static FUNC(void, CANIF_CODE)
	canif_enter_controller_mode_with_tx_clear(canif_channel_id_type chnl_id, canif_controller_mode_type ctrl_mode)
{
	if (CANIF_CS_STOPPED == ctrl_mode) {
		canif_clear_tx_buffers((uint8)chnl_id);
	}
}
#endif

#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON) &&                                                        \
	((CANIF_CTRL_WAKEUP_SUPPORT == STD_ON) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALID_BY_NM == STD_ON)
static FUNC(void, CANIF_CODE) canif_set_wakeup_frame_received_flag(uint8 channel_id, uint8 rx_pdu_ul)
#else
static FUNC(void, CANIF_CODE) canif_set_wakeup_frame_received_flag(uint8 channel_id)
#endif
{
	if (CANIF_CS_STARTED == canif_runtime_state.ctrl_runtime_state[channel_id].ctrl_mode) {
		if (CANIF_FRAME_RECEIVED_NONE ==
		    canif_runtime_state.ctrl_runtime_state[channel_id].first_rx_indication) {
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALID_BY_NM == STD_ON)
			if (rx_pdu_ul == CANIF_RXPDU_USER_RXINDICATION_UL_CANNM)
#endif
			{
				canif_runtime_state.ctrl_runtime_state[channel_id].first_rx_indication =
					CANIF_FRAME_RECEIVED_FIRST;
			}
		}
	}
}
#endif

static inline FUNC(Std_ReturnType, CANIF_CODE)
	canif_internal_dlc_check(uint8 can_dlc, const canif_rx_pdu_config_type *rx_pdu_cfg_ptr)
{
	Std_ReturnType ret = E_NOT_OK;
#if (CANIF_PRIVATE_DLC_CHECK == STD_ON)
	if (can_dlc < rx_pdu_cfg_ptr->rx_pdu_dlc) {
		CANIF_DET_REPORT(CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_DLC);
		return ret;
	}
#endif
	ret = E_OK;
	return ret;
}

static inline FUNC(boolean, CANIF_CODE) canif_is_extend_can_id(Can_IdType can_id)
{
	if (CANIF_IS_EXTENDED_CAN_ID(can_id)) {
		if ((can_id & ~(3UL << CAN_FD_BIT_POS)) > EXTENDED_CANID_MAX) {
			return TRUE;
		}
	}
	return FALSE;
}

static inline FUNC(boolean, CANIF_CODE) canif_is_standard_can_id(Can_IdType can_id)
{
	if (!CANIF_IS_EXTENDED_CAN_ID(can_id)) {
		if ((can_id & ~(3UL << CAN_FD_BIT_POS)) > STANDARD_CANID_MAX) {
			return TRUE;
		}
	}
	return FALSE;
}

static inline FUNC(Std_ReturnType, CANIF_CODE) canif_can_id_validation_check(Can_IdType can_id)
{
	boolean is_canid_invalid = TRUE;
	Std_ReturnType ret = E_NOT_OK;

	is_canid_invalid = (canif_is_extend_can_id(can_id) || canif_is_standard_can_id(can_id));
#if (CANIF_CANFD_SUPPORT == STD_OFF)
	if ((is_canid_invalid == TRUE) || CANIF_IS_CANFD(can_id)) {
#else
	if (is_canid_invalid == TRUE) {
#endif
		CANIF_DET_REPORT(CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_CANID);
		return ret;
	}
	ret = E_OK;
	return ret;
}

static inline FUNC(Std_ReturnType, CANIF_CODE) canif_data_length_validation_check(uint8 can_dlc)
{
	Std_ReturnType ret = E_NOT_OK;
#if (CANIF_CANFD_SUPPORT == STD_ON)
	if (can_dlc <= 64UL)
		ret = E_OK;
	else
		ret = E_NOT_OK;
#else
	if (can_dlc <= 8u)
		ret = E_OK;
	else
		ret = E_NOT_OK;
#endif
	CANIF_DET_REPORTERR_NO_RV((ret == E_OK), CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_DLC, ret);
	return ret;
}

static FUNC(boolean, CANIF_CODE) canif_linear_search(const canif_rx_pdu_config_type *rx_pdu_cfg_ptr, uint16 max_number,
						     uint32 can_id, const canif_rx_pdu_config_type **rx_pdu)
{
	boolean found = FALSE;
	boolean is_extended_id = CANIF_IS_EXTENDED_CAN_ID(can_id);
	uint32 actual_can_id;
#if (CANIF_CANFD_SUPPORT == STD_ON)
	actual_can_id = can_id & ~(3UL << CAN_FD_BIT_POS);
#else
	actual_can_id = can_id & ~(1UL << EXT_ID_BIT_POS);
#endif
	uint16 rx_pdu_index = 0;

	while ((rx_pdu_index < max_number) && (!found)) {
		if ((((actual_can_id <= rx_pdu_cfg_ptr[rx_pdu_index].rx_pdu_upper_can_id) &&
		      (actual_can_id >= rx_pdu_cfg_ptr[rx_pdu_index].rx_pdu_lower_can_id)) ||
		     (actual_can_id == rx_pdu_cfg_ptr[rx_pdu_index].rx_pdu_can_id)) &&
		    (is_extended_id == rx_pdu_cfg_ptr[rx_pdu_index].rx_pdu_canid_extended)) {
			found = TRUE;
			*rx_pdu = &rx_pdu_cfg_ptr[rx_pdu_index];
			break;
		}
		rx_pdu_index++;
	}

	return found;
}

static FUNC(boolean, CANIF_CODE) canif_is_rxpdu_match(Can_IdType can_id, const canif_hrh_config_type *canif_hrh_config,
						      const canif_rx_pdu_config_type **rx_pdu_ptr_final)
{
	boolean ispdu_match = FALSE;
	const canif_rx_pdu_config_type *rx_pdu_cfg_ptr = canif_hrh_config->hrh_rx_pdu_list;

	if (0u != canif_hrh_config->hrh_ctrl_rx_pdu_num) {
		if ((CANIF_HANDLE_TYPE_FULL == canif_hrh_config->hrh_type) || !canif_hrh_config->hrh_sw_filter) {
			ispdu_match = TRUE;
		} else {
			ispdu_match =
				canif_linear_search(canif_hrh_config->hrh_rx_pdu_list,
						    canif_hrh_config->hrh_ctrl_rx_pdu_num, can_id, &rx_pdu_cfg_ptr);
		}
	}

	*rx_pdu_ptr_final = rx_pdu_cfg_ptr;
	return ispdu_match;
}

static inline FUNC(Std_ReturnType, CANIF_CODE)
	canif_rx_callout(const canif_rx_pdu_config_type *rx_pdu_cfg_ptr, const PduInfoType *pdu_info_ptr)
{
	Std_ReturnType ret = E_NOT_OK;

	if (rx_pdu_cfg_ptr->rx_pdu_rx_callout_fct_ptr != NULL_PTR)
		ret = rx_pdu_cfg_ptr->rx_pdu_rx_callout_fct_ptr(rx_pdu_cfg_ptr->rx_pdu_pdu_id, pdu_info_ptr);
	else
		ret = E_OK;

	return ret;
}

static inline FUNC(Std_ReturnType, CANIF_CODE)
	canif_tx_callout(const canif_tx_pdu_config_type *txpdu_cfg_ptr, Can_PduType *ptrcan_pdu)
{
	Std_ReturnType ret = E_NOT_OK;

	if (txpdu_cfg_ptr->tx_pdu_txmsg_callout_fct_ptr != NULL)
		ret = txpdu_cfg_ptr->tx_pdu_txmsg_callout_fct_ptr(txpdu_cfg_ptr->tx_pdu_id, ptrcan_pdu);
	else
		ret = E_OK;

	return ret;
}

static FUNC(void, CANIF_CODE) canif_transfer_rxdata_to_upper(const canif_rx_pdu_config_type *rx_pdu_cfg_ptr,
							     uint8 can_dlc, uint8 *can_sdu_ptr)
{
	Std_ReturnType ret_rx_callout = E_NOT_OK;
	PduInfoType pdu_info = { 0 };
	uint8 upper_rxindication_fct_index = rx_pdu_cfg_ptr->rx_pdu_ul_rx_indication;

	if (upper_rxindication_fct_index != CANIF_NO_FUNCTION_CALLOUT) {
		pdu_info.SduLength = (PduLengthType)can_dlc;
		pdu_info.SduDataPtr = (uint8 *)can_sdu_ptr;
		canif_user_rx_indication_fct_list_type upper_rxindication_fct_cfg =
			canif_user_rx_indications[upper_rxindication_fct_index];
		switch (upper_rxindication_fct_cfg.rx_indication_type) {
		case CANIF_RX_INDICATION_TYPE_ADVANCED_TYPE: {
			canif_advanced_rx_indication_fct_type rx_indication_fct;

			rx_indication_fct = upper_rxindication_fct_cfg.rx_indication_fct.advanced_rx_indication_type;
			if (rx_indication_fct != NULL_PTR) {
#if (CANIF_PUBLIC_DEBUG_SUPPORT == STD_ON)
				(void)Os_AtomicInc((Atomic *)&rx_messagestatistic[rx_pdu_cfg_ptr->rx_pdu_index]);
#endif
				ret_rx_callout = canif_rx_callout(rx_pdu_cfg_ptr, &pdu_info);
				if (E_OK == ret_rx_callout) {
					rx_indication_fct(rx_pdu_cfg_ptr->rx_pdu_pdu_id, &pdu_info);
				}
			}
		} break;
		case CANIF_RX_INDICATION_TYPE_CDD_TYPE: {
			canif_cdd_rx_indication_fct_type rx_indication_fct;

			rx_indication_fct = upper_rxindication_fct_cfg.rx_indication_fct.cdd_rx_indication_type;
			if (rx_indication_fct != NULL_PTR) {
#if (CANIF_PUBLIC_DEBUG_SUPPORT == STD_ON)
				(void)Os_AtomicInc((Atomic *)&rx_messagestatistic[rx_pdu_cfg_ptr->rx_pdu_index]);
#endif
				rx_indication_fct(rx_pdu_cfg_ptr->rx_pdu_pdu_id, &pdu_info);
			}
		} break;
		default:
			break;
		}
	}
	return;
}

static FUNC(Std_ReturnType, CANIF_CODE)
	canif_can_write_post_process(uint8 write_ret, const canif_tx_pdu_config_type *tx_pdu_ptr, Can_PduType can_pdu)
{
	Std_ReturnType ret_status = CANIF_TRANSMIT_NOT_OK;

	if (CANIF_CAN_WRITE_BUSY == write_ret) {
#if (CANIF_PUBLIC_DEBUG_SUPPORT == STD_ON)
		(void)Os_AtomicInc((Atomic *)&tx_message_busy_count[can_pdu.swPduHandle]);
#endif
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
		if (0UL != tx_pdu_ptr->tx_pdu_buffer_ref->tx_buffer_size) {
			Std_ReturnType ret =
				canif_queue_item_replace_or_add(tx_pdu_ptr->tx_pdu_buffer_ref, &can_pdu, TRUE);

			if (ret == E_OK)
				ret_status = CANIF_TRANSMIT_OK;
			else
				ret_status = CANIF_TRANSMIT_BUSY_TXBUFFER_NOT_INSERTED;
		} else {
			ret_status = CANIF_TRANSMIT_BUSY_NO_TXBUFFER_CONFIG;
		}
#else
		ret_status = CANIF_TRANSMIT_BUSY_NO_TXBUFFER_CONFIG;
#endif
	} else if (E_NOT_OK == write_ret) {
		ret_status = CANIF_TRANSMIT_NOT_OK;
	} else if (E_OK == write_ret) {
		ret_status = CANIF_TRANSMIT_OK;
	} else {
		ret_status = CANIF_TRANSMIT_NOT_OK;
	}

#if (CANIF_PUBLIC_DEBUG_SUPPORT == STD_ON)
	if (ret_status == CANIF_TRANSMIT_OK)
		(void)Os_AtomicInc((Atomic *)&tx_message_ok_count[can_pdu.swPduHandle]);
	else
		(void)Os_AtomicInc((Atomic *)&tx_message_drop_count[can_pdu.swPduHandle]);
#endif

	return ret_status;
}

static FUNC(Std_ReturnType, CANIF_CODE)
	canif_transmit_to_can_driver(const uint8 controller_id, PduIdType can_tx_pdu_id,
				     const canif_tx_pdu_config_type *tx_pdu_ptr, const PduInfoType *pdu_info_ptr)
{
	Can_PduType can_pdu;
	Std_ReturnType write_ret = E_NOT_OK;
	Std_ReturnType ret = E_NOT_OK;

	can_pdu.id = tx_pdu_ptr->tx_pdu_can_id;

	switch (tx_pdu_ptr->tx_pdu_can_id_type) {
	case CANIF_CAN_ID_TYPE_29: {
		can_pdu.id |= (1UL << EXT_ID_BIT_POS);
	} break;
	case CANIF_CAN_FD_ID_TYPE_11: {
		can_pdu.id |= (1UL << CAN_FD_BIT_POS);
	} break;
	case CANIF_CAN_FD_ID_TYPE_29: {
		can_pdu.id |= (3UL << CAN_FD_BIT_POS);
	} break;
	default:
		break;
	}

	if (pdu_info_ptr->SduLength < tx_pdu_ptr->tx_pdu_dlc)
		can_pdu.length = (uint8)pdu_info_ptr->SduLength;
	else
		can_pdu.length = tx_pdu_ptr->tx_pdu_dlc;

	can_pdu.sdu = pdu_info_ptr->SduDataPtr;
	can_pdu.swPduHandle = can_tx_pdu_id;

#if (CANIF_PUBLIC_APPL_CANIF_TX_CALLOUT_SUPPORT == STD_ON)
	Std_ReturnType ret_appl = appl_canif_tx_callout(controller_id, &can_pdu);

	if (E_OK == ret_appl)
#else
	(void)controller_id;
#endif
	{
		(void)can_pdu;
		Std_ReturnType ret_tx_callout = canif_tx_callout(tx_pdu_ptr, &can_pdu);

		if (E_OK == ret_tx_callout) {
			Can_HwHandleType hth_id_sym_ref =
				tx_pdu_ptr->tx_pdu_buffer_ref->tx_buffer_hth_ref->hth_id_sym_ref;
#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
			Can_ReturnType canwrite_ret = CAN_NOT_OK;

			canwrite_ret = _canif_can_write(hth_id_sym_ref, &can_pdu);
			switch (canwrite_ret) {
			case CAN_OK:
				write_ret = E_OK;
				break;
			case CAN_NOT_OK:
				write_ret = E_NOT_OK;
				break;
			case CAN_BUSY:
				write_ret = CANIF_CAN_WRITE_BUSY;
				break;
			default:
				write_ret = E_NOT_OK;
				break;
			}
#else
			write_ret = _canif_can_write(hth_id_sym_ref, &can_pdu);
#endif
			ret = canif_can_write_post_process(write_ret, tx_pdu_ptr, can_pdu);
		}
	}

	return ret;
}

static inline FUNC(Std_ReturnType, CANIF_CODE) canif_channel_mode_check_on_transmit(canif_channel_id_type chnl_id)
{
	Std_ReturnType ret = E_NOT_OK;
	canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr = &canif_runtime_state.ctrl_runtime_state[chnl_id];

	if ((CANIF_CS_STARTED != canif_ctrl_runtime_state_ptr->ctrl_mode) ||
	    (CANIF_TX_OFFLINE == canif_ctrl_runtime_state_ptr->pdu_mode) ||
	    (CANIF_OFFLINE == canif_ctrl_runtime_state_ptr->pdu_mode)) {
		CANIF_DET_REPORT(CANIF_CANIF_TRANSMIT_ID_CANIF_E_PARAM_CTRLMODE);
		CANIF_DET_REPORT(CANIF_CANIF_TRANSMIT_ID_CANIF_E_PARAM_PDUMODE);
		return ret;
	}
	ret = E_OK;
	return ret;
}

static FUNC(Std_ReturnType, CANIF_CODE)
	canif_pn_fliter_process(const canif_tx_pdu_config_type *tx_pdu_ptr, canif_channel_id_type chnl_id)
{
	Std_ReturnType ret = E_OK;
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
	if ((canif_config_ptr->canif_ctrl_config[chnl_id].canif_ctrl_pn_filter_set) &&
	    (canif_runtime_state.ctrl_runtime_state[chnl_id].pn_tx_filter_enabled) &&
	    (!tx_pdu_ptr->tx_pdu_pn_filter_en)) {
		ret = E_NOT_OK;
	}
#endif
	return ret;
}

#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
static FUNC(Std_ReturnType, CANIF_CODE)
	canif_change_controller_mode(uint8 can_ctrl_id, Can_StateTransitionType can_transition)
#else
static FUNC(Std_ReturnType, CANIF_CODE)
	canif_change_controller_mode(uint8 can_ctrl_id, Can_ControllerStateType can_transition)
#endif
{
	Std_ReturnType ret = E_NOT_OK;
#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
	Can_ReturnType canret = CAN_NOT_OK;

	canret = _canif_can_set_controller_mode(can_ctrl_id, can_transition);
	switch (canret) {
	case CAN_OK:
		ret = E_OK;
		break;
	case CAN_NOT_OK:
		ret = E_NOT_OK;
		break;
	default:
		ret = E_NOT_OK;
		break;
	}
#else
	ret = _canif_can_set_controller_mode(can_ctrl_id, can_transition);
#endif
	(void)VCOS_LOG(CANIF_MODULE_ID, VCOS_LOG_INFO, CANIF_DLT_CHECK_ID0, can_ctrl_id, can_transition, ret);

	return ret;
}

#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
static inline FUNC(void, CANIF_CODE) set_pn_filter_enable(uint8 controller_id)
{
	canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr =
		&canif_runtime_state.ctrl_runtime_state[controller_id];
	if (((canif_ctrl_runtime_state_ptr->pn_tx_filter_enabled == (boolean)FALSE) &&
	     (canif_ctrl_runtime_state_ptr->nm_cnt != 0u)) &&
	    canif_config_ptr->canif_ctrl_config[controller_id].canif_ctrl_pn_filter_set) {
		canif_ctrl_runtime_state_ptr->pn_tx_filter_enabled = (boolean)TRUE;
	}
	return;
}

static inline FUNC(void, CANIF_CODE) set_pn_filter_disable(uint8 controller_id)
{
	canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr =
		&canif_runtime_state.ctrl_runtime_state[controller_id];
	if (((canif_ctrl_runtime_state_ptr->pn_tx_filter_enabled == (boolean)TRUE) &&
	     (canif_ctrl_runtime_state_ptr->nm_cnt != 0u)) &&
	    canif_config_ptr->canif_ctrl_config[controller_id].canif_ctrl_pn_filter_set) {
		canif_ctrl_runtime_state_ptr->pn_tx_filter_enabled = (boolean)FALSE;
	}
	return;
}
#endif

static FUNC(Std_ReturnType, CANIF_CODE) canif_change_pdu_mode(canif_pdu_mode_type pdu_mode_request,
							      canif_pdu_mode_type *new_mode_ptr, uint8 controller_id)
{
	Std_ReturnType ret = E_OK;

	switch (pdu_mode_request) {
	case CANIF_OFFLINE:
		*new_mode_ptr = CANIF_OFFLINE;
		break;
	case CANIF_TX_OFFLINE:
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
		set_pn_filter_enable(controller_id);
#endif
		*new_mode_ptr = CANIF_TX_OFFLINE;
		break;
	case CANIF_TX_OFFLINE_ACTIVE:
		*new_mode_ptr = CANIF_TX_OFFLINE_ACTIVE;
		break;
	case CANIF_ONLINE:
		*new_mode_ptr = CANIF_ONLINE;
		break;
	default:
		CANIF_DET_REPORT(CANIF_CANIF_SETPDUMODE_ID_CANIF_E_PARAM_PDUMODE);
		ret = E_NOT_OK;
		break;
	}
	return ret;
}

FUNC(void, CANIF_CODE) canif_init(const canif_config_type *config_ptr)
{
	CANIF_DET_REPORTERR((NULL != config_ptr), CANIF_CANIF_INIT_ID_CANIF_E_PARAM_POINTER);

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}

	canif_config_ptr = config_ptr;

	canif_init_status[core_id] = CANIF_UNINIT;

	if (core_id == CANIF_MULTICORE_MASTER_COREID) {
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
		PduIdType can_tx_pdu_id = 0;

		while (can_tx_pdu_id < CANIF_MAX_NOF_TX_PDU) {
			const canif_tx_pdu_config_type *tx_pdu_ptr =
				&canif_config_ptr->canif_init_config->canif_all_tx_pdu_cfg_ptr[can_tx_pdu_id];
			if (tx_pdu_ptr->tx_pdu_pn_filter_en) {
				uint8 ctrlid =
					(uint8)tx_pdu_ptr->tx_pdu_buffer_ref->tx_buffer_hth_ref->hth_can_ctrl_id_ref;

				CANIF_DET_REPORTERR((ctrlid < CANIF_CONTROLLER_CHANNEL_CNT),
						    CANIF_CANIF_CONTROLLER_MODE_INDICATION_ID_CANIF_E_PARAM_CONTROLLER);

				uint8 chnlid = ctrlid;
				(void)Os_AtomicInc((Atomic *)&canif_runtime_state.ctrl_runtime_state[chnlid].nm_cnt);
			}
			can_tx_pdu_id++;
		}
#endif
		uint8 ctrl_chnl = 0;

		while (ctrl_chnl < CANIF_CONTROLLER_CHANNEL_CNT) {
			canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr =
				&canif_runtime_state.ctrl_runtime_state[ctrl_chnl];
			canif_ctrl_runtime_state_ptr->ctrl_mode = CANIF_CS_STOPPED;
			canif_ctrl_runtime_state_ptr->pdu_mode = CANIF_OFFLINE;
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
			canif_ctrl_runtime_state_ptr->pn_tx_filter_enabled = (boolean)FALSE;
			if (canif_config_ptr->canif_ctrl_config[ctrl_chnl].canif_ctrl_pn_filter_set &&
			    (canif_ctrl_runtime_state_ptr->nm_cnt != 0u)) {
				canif_ctrl_runtime_state_ptr->pn_tx_filter_enabled = (boolean)TRUE;
			}
#endif

#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
			canif_ctrl_runtime_state_ptr->first_rx_indication = CANIF_FRAME_RECEIVED_NONE;
#endif
			ctrl_chnl++;
		}

#if (CANIF_TRANSCEIVER_API == STD_ON)
		canif_trcv_runtime_state_type *canif_trcv_runtime_state_ptr =
			&canif_runtime_state.trcv_runtime_state[0];
		uint8 trcv_chnl = 0;

		while (trcv_chnl < CANIF_TRANSCEIVER_CHANNEL_CNT) {
			canif_trcv_runtime_state_ptr[trcv_chnl].trcv_mode = CANTRCV_TRCVMODE_NORMAL;
			trcv_chnl++;
		}
#endif
	}

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
	canif_buffer_entry_type *tx_pdu_buffer_ptr = tx_pdu_buffer[core_id];
	uint16 start_index_tx_pdu_buffer =
		canif_tx_buffer_lpdu_index_range[core_id].canif_tx_buffer_start_index_of_cur_core;
	uint16 end_index_tx_pdu_buffer =
		canif_tx_buffer_lpdu_index_range[core_id].canif_tx_buffer_end_index_of_cur_core;
	if ((NULL != tx_pdu_buffer_ptr) && (CANIF_INVALID_TXBUFFER_START_INDEX != start_index_tx_pdu_buffer) &&
	    (CANIF_INVALID_TXBUFFER_END_INDEX != end_index_tx_pdu_buffer)) {
		uint16 buf_index = 0;

		while (buf_index < (end_index_tx_pdu_buffer - start_index_tx_pdu_buffer + 1U)) {
			tx_pdu_buffer_ptr[buf_index].in_use = FALSE;
			buf_index++;
		}
	}

	uint16 *buffer_start_index_ptr = buffer_start_index[core_id];
	uint16 start_index_txbuffer = canif_tx_buffer_index_range[core_id].canif_tx_buffer_start_index_of_cur_core;
	uint16 end_index_txbuffer = canif_tx_buffer_index_range[core_id].canif_tx_buffer_end_index_of_cur_core;

	const canif_tx_buffer_config_type *canif_all_tx_buf_cfg_ptr =
		canif_config_ptr->canif_init_config->canif_all_tx_buf_cfg_ptr;

	if ((NULL != buffer_start_index_ptr) && (CANIF_INVALID_TXBUFFER_START_INDEX != start_index_txbuffer) &&
	    (CANIF_INVALID_TXBUFFER_END_INDEX != end_index_txbuffer)) {
		uint16 indx = 0;
		uint16 buf_index = start_index_txbuffer;

		while (buf_index <= end_index_txbuffer) {
			buffer_start_index_ptr[canif_all_tx_buf_cfg_ptr[buf_index].tx_buffer_id] = indx;
			indx += canif_all_tx_buf_cfg_ptr[buf_index].tx_buffer_size;
			buf_index++;
		}
	}

#if (CANIF_TX_BUF_FIFO_ENABLE == STD_ON)
	canif_fifo_buffer_info_type *fifo_buffer_info_ptr = fifo_buffer_info[core_id];

	if ((NULL != fifo_buffer_info_ptr) && (CANIF_INVALID_TXBUFFER_START_INDEX != start_index_txbuffer) &&
	    (CANIF_INVALID_TXBUFFER_END_INDEX != end_index_txbuffer)) {
		uint16 buffer_index = 0;

		while (buffer_index < (end_index_txbuffer - start_index_txbuffer + 1U)) {
			fifo_buffer_info_ptr[buffer_index].u16_write_index = 0;
			fifo_buffer_info_ptr[buffer_index].u16_read_index = 0;
			fifo_buffer_info_ptr[buffer_index].u16_queue_size = 0;
			buffer_index++;
		}
	}
#endif
#endif

	canif_init_status[core_id] = CANIF_INIT;
}

#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
FUNC(void, CANIF_CODE)
canif_controller_mode_indication(uint8 controller_id, canif_controller_mode_type controller_mode)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}
	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id,
							 CANIF_CANIF_CONTROLLER_MODE_INDICATION_ID_CANIF_E_UNINIT);

	CANIF_DET_REPORTERR(CANIF_CHECK_IS_VALID_CONTROLLER_MODE(controller_mode),
			    CANIF_CANIF_CONTROLLER_MODE_INDICATION_ID_CANIF_E_PARAM_CTRLMODE);

	CANIF_DET_REPORTERR((controller_id < CANIF_CONTROLLER_CHANNEL_CNT),
			    CANIF_CANIF_CONTROLLER_MODE_INDICATION_ID_CANIF_E_PARAM_CONTROLLER);

	canif_channel_id_type channel = (canif_channel_id_type)controller_id;

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
	canif_enter_controller_mode_with_tx_clear(channel, controller_mode);
#endif
	if (NULL != canif_dispatch_config.controller_mode_indication_fct) {
		canif_dispatch_config.controller_mode_indication_fct(channel, controller_mode);
	}

	canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr = &canif_runtime_state.ctrl_runtime_state[channel];

	canif_ctrl_runtime_state_ptr->ctrl_mode = controller_mode;
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
	if (CANIF_CS_SLEEP == controller_mode) {
		canif_ctrl_runtime_state_ptr->first_rx_indication = CANIF_FRAME_RECEIVED_NONE;
	}

#endif
}
#else
FUNC(void, CANIF_CODE)
canif_controller_mode_indication(uint8 controller_id, Can_ControllerStateType controller_mode)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}
	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id,
							 CANIF_CANIF_CONTROLLER_MODE_INDICATION_ID_CANIF_E_UNINIT);

	CANIF_DET_REPORTERR(CANIF_CHECK_IS_VALID_CONTROLLER_STATE(controller_mode),
			    CANIF_CANIF_CONTROLLER_MODE_INDICATION_ID_CANIF_E_PARAM_CTRLMODE);

	canif_channel_id_type channel = 0;

	CANIF_DET_REPORTERR((controller_id < CANIF_CONTROLLER_CHANNEL_CNT),
			    CANIF_CANIF_CONTROLLER_MODE_INDICATION_ID_CANIF_E_PARAM_CONTROLLER);
	channel = (canif_channel_id_type)controller_id;
	canif_controller_mode_type canif_controller_mode = CANIF_CS_UNINIT;

	switch (controller_mode) {
	case CAN_CS_STARTED:
		canif_controller_mode = CANIF_CS_STARTED;
		break;
	case CAN_CS_STOPPED:
		canif_controller_mode = CANIF_CS_STOPPED;
		break;
	case CAN_CS_SLEEP:
		canif_controller_mode = CANIF_CS_SLEEP;
		break;
	default:
		break;
	}
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
	canif_enter_controller_mode_with_tx_clear(channel, canif_controller_mode);
#endif
	if (NULL != canif_dispatch_config.controller_mode_indication_fct) {
		canif_dispatch_config.controller_mode_indication_fct(channel, controller_mode);
	}

	canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr = &canif_runtime_state.ctrl_runtime_state[channel];

	canif_ctrl_runtime_state_ptr->ctrl_mode = canif_controller_mode;
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
	if (CANIF_CS_SLEEP == canif_controller_mode) {
		canif_ctrl_runtime_state_ptr->first_rx_indication = CANIF_FRAME_RECEIVED_NONE;
	}

#endif
}
#endif
#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_controller_mode(uint8 controller_id, canif_controller_mode_type controller_mode)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}

	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_SET_CONTROLLER_MODE_ID_CANIF_E_UNINIT,
						   E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((controller_id < CANIF_CONTROLLER_CHANNEL_CNT),
				  CANIF_CANIF_SET_CONTROLLER_MODE_ID_CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV(CANIF_CHECK_IS_VALID_CONTROLLER_MODE(controller_mode),
				  CANIF_CANIF_SET_CONTROLLER_MODE_ID_CANIF_E_PARAM_CTRLMODE, E_NOT_OK);

	uint8 can_ctrl_id = CANIF_GET_CHANNEL_CONTROLLER(controller_id);

	canif_controller_mode_type curr_mode = canif_runtime_state.ctrl_runtime_state[controller_id].ctrl_mode;

	Can_StateTransitionType can_transition = CAN_T_START;
	Std_ReturnType ret = E_OK;

	switch (controller_mode) {
	case CANIF_CS_STOPPED:
		if (CANIF_CS_SLEEP == curr_mode)
			can_transition = CAN_T_WAKEUP;
		else
			can_transition = CAN_T_STOP;

		(void)canif_set_pdu_mode(controller_id, CANIF_TX_OFFLINE);
		break;
	case CANIF_CS_STARTED:
		can_transition = CAN_T_START;
		break;
	case CANIF_CS_SLEEP:
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
		set_pn_filter_enable(controller_id);
#endif
		can_transition = CAN_T_SLEEP;
		(void)canif_set_pdu_mode(controller_id, CANIF_OFFLINE);
		break;
	default:
		ret = E_NOT_OK;
		break;
	}

	if (E_OK == ret) {
		ret = canif_change_controller_mode(can_ctrl_id, can_transition);
	}

	return ret;
}
#else
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_controller_mode(uint8 controller_id, Can_ControllerStateType controller_mode)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_SET_CONTROLLER_MODE_ID_CANIF_E_UNINIT,
						   E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((controller_id < CANIF_CONTROLLER_CHANNEL_CNT),
				  CANIF_CANIF_SET_CONTROLLER_MODE_ID_CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);

	uint8 can_ctrl_id = CANIF_GET_CHANNEL_CONTROLLER(controller_id);

	Can_ControllerStateType can_transition = CAN_CS_UNINIT;
	Std_ReturnType ret = E_OK;

	switch (controller_mode) {
	case CAN_CS_STOPPED:
		can_transition = CAN_CS_STOPPED;
		(void)canif_set_pdu_mode(controller_id, CANIF_TX_OFFLINE);
		break;
	case CAN_CS_STARTED:
		can_transition = CAN_CS_STARTED;
		break;
	case CAN_CS_SLEEP:
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
		set_pn_filter_enable(controller_id);
#endif
		can_transition = CAN_CS_SLEEP;
		(void)canif_set_pdu_mode(controller_id, CANIF_OFFLINE);
		break;
	default:
		ret = E_NOT_OK;
		CANIF_DET_REPORT(CANIF_CANIF_SET_CONTROLLER_MODE_ID_CANIF_E_PARAM_CTRLMODE);
		break;
	}
	if (E_OK == ret) {
		ret = canif_change_controller_mode(can_ctrl_id, can_transition);
	}

	return ret;
}
#endif

FUNC(Std_ReturnType, CANIF_CODE)
canif_get_controller_mode(uint8 controller_id, canif_controller_mode_type *controller_mode_ptr)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_GET_CONTROLLER_MODE_ID_CANIF_E_UNINIT,
						   E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((controller_id < CANIF_CONTROLLER_CHANNEL_CNT),
				  CANIF_CANIF_GET_CONTROLLER_MODE_ID_CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);
	CANIF_DET_REPORTERR_NO_RV((NULL != controller_mode_ptr),
				  CANIF_CANIF_GET_CONTROLLER_MODE_ID_CANIF_E_PARAM_POINTER, E_NOT_OK);

	*controller_mode_ptr = canif_runtime_state.ctrl_runtime_state[controller_id].ctrl_mode;

	return E_OK;
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_transmit(PduIdType can_tx_pdu_id, const PduInfoType *pdu_info_ptr)
{
	Std_ReturnType ret = E_NOT_OK;
	Std_ReturnType transmit_status = E_NOT_OK;
	canif_channel_id_type channel = 0;
	canif_user_tx_confirmation_type canif_usr_tx_confirmation_ptr = NULL;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_TRANSMIT_ID_CANIF_E_UNINIT, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((NULL != pdu_info_ptr), CANIF_CANIF_TRANSMIT_ID_CANIF_E_PARAM_POINTER, E_NOT_OK);
	CANIF_DET_REPORTERR_NO_RV((can_tx_pdu_id < canif_config_ptr->canif_init_config->canif_all_tx_pdu_num),
				  CANIF_CANIF_TRANSMIT_ID_CANIF_E_INVALID_TXPDUID, E_NOT_OK);

	const canif_tx_pdu_config_type *tx_pdu_ptr =
		&canif_config_ptr->canif_init_config->canif_all_tx_pdu_cfg_ptr[can_tx_pdu_id];
	uint8 controller = (uint8)tx_pdu_ptr->tx_pdu_buffer_ref->tx_buffer_hth_ref->hth_can_ctrl_id_ref;

	CANIF_DET_REPORTERR_NO_RV((controller < CANIF_CONTROLLER_CHANNEL_CNT),
				  CANIF_CANIF_TRANSMIT_ID_CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);
	channel = (canif_channel_id_type)controller;

	ret = canif_channel_mode_check_on_transmit(channel);
	if (E_OK == ret) {
		ret = canif_pn_fliter_process(tx_pdu_ptr, channel);
		if (E_OK == ret) {
			if (CANIF_TX_OFFLINE_ACTIVE == canif_runtime_state.ctrl_runtime_state[channel].pdu_mode) {
				canif_usr_tx_confirmation_ptr =
					canif_user_tx_confirmations[tx_pdu_ptr->tx_pdu_ul_tx_confirmation];
				if ((CANIF_NO_FUNCTION_CALLOUT != tx_pdu_ptr->tx_pdu_ul_tx_confirmation) &&
				    (canif_usr_tx_confirmation_ptr != NULL)) {
					canif_usr_tx_confirmation_ptr(tx_pdu_ptr->tx_pdu_id, E_OK);
				}
				transmit_status = E_OK;
			}
			if (transmit_status == E_NOT_OK)
				ret = canif_transmit_to_can_driver(channel, can_tx_pdu_id, tx_pdu_ptr, pdu_info_ptr);
			else
				ret = transmit_status;
		}
	}
	return ret;
}

FUNC(void, CANIF_CODE) canif_tx_confirmation(PduIdType can_tx_pdu_id)
{
	const canif_tx_pdu_config_type *tx_pdu_ptr;

	canif_user_tx_confirmation_type canif_usr_tx_confirmation_ptr = NULL;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}
	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_TXCONFIRMATION_ID_CANIF_E_UNINIT);

	CANIF_DET_REPORTERR((can_tx_pdu_id < canif_config_ptr->canif_init_config->canif_all_tx_pdu_num),
			    CANIF_CANIF_TXCONFIRMATION_ID_CANIF_E_PARAM_LPDU);
	tx_pdu_ptr = &canif_config_ptr->canif_init_config->canif_all_tx_pdu_cfg_ptr[can_tx_pdu_id];
	canif_pdu_mode_type mode;
	uint8 channel = 0;
	const canif_hth_config_type *canif_hth_config_ptr = tx_pdu_ptr->tx_pdu_buffer_ref->tx_buffer_hth_ref;
	uint8 canif_can_controller_id = canif_hth_config_ptr->hth_can_ctrl_id_ref;
#if (CANIF_PUBLIC_DEBUG_SUPPORT == STD_ON)

	(void)Os_AtomicInc((Atomic *)&tx_messagestatistic[can_tx_pdu_id]);
#endif

	if (!(canif_can_controller_id < CANIF_CONTROLLER_CHANNEL_CNT)) {
		return;
	}
	channel = canif_can_controller_id;
#if (CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON)
	canif_tx_conf_state[channel] = CANIF_TX_RX_NOTIFICATION;
#endif
	if (E_OK == canif_get_pdu_mode(channel, &mode)) {
		if ((mode == CANIF_TX_OFFLINE_ACTIVE) || (mode == CANIF_ONLINE)) {
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
			if (mode == CANIF_ONLINE) {
				Can_PduType can_pdu;
				Std_ReturnType ret_get =
					canif_get_pdu_from_queue(tx_pdu_ptr->tx_pdu_buffer_ref, &can_pdu);

				if (E_OK == ret_get) {
#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
					if (CAN_OK ==
					    _canif_can_write(canif_hth_config_ptr->hth_id_sym_ref, &can_pdu)) {
#else
					if (E_OK == _canif_can_write(canif_hth_config_ptr->hth_id_sym_ref, &can_pdu)) {
#endif
					}
				}
			}
#endif

#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
			set_pn_filter_disable(channel);
#endif

#if (CANIF_PUBLIC_APPL_CANIF_TXCONFIRMATION_CALLOUT_SUPPORT == STD_ON)
			Can_PduType can_pdu_callout;

			can_pdu_callout.swPduHandle = can_tx_pdu_id;
			Can_ReturnType appl_ret = appl_canif_tx_confirmation_callout(channel, &can_pdu_callout);
			(void)appl_ret;
#endif
			canif_usr_tx_confirmation_ptr =
				canif_user_tx_confirmations[tx_pdu_ptr->tx_pdu_ul_tx_confirmation];
			if ((CANIF_NO_FUNCTION_CALLOUT != tx_pdu_ptr->tx_pdu_ul_tx_confirmation) &&
			    (canif_usr_tx_confirmation_ptr != NULL)) {
				canif_usr_tx_confirmation_ptr(tx_pdu_ptr->tx_pdu_id, E_OK);
			}
		}
	}
}

#if defined(AUTOSAR_CANIF_VERSION_403)
FUNC(void, CANIF_CODE)
canif_rx_indication(Can_HwHandleType hrh, Can_IdType can_id, uint8 can_dlc, const uint8 *can_sdu_ptr)
{
#else

FUNC(void, CANIF_CODE)
canif_rx_indication(const Can_HwType *mailbox, const PduInfoType *pdu_info_ptr)
{
	uint8 *can_sdu_ptr = NULL;
	Can_IdType can_id;
	Can_HwHandleType hrh;
	uint8 can_dlc;
	canif_core_id_type core_id;

	core_id = (canif_core_id_type)GetCoreID();
	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}

	CANIF_DET_REPORTERR((NULL != pdu_info_ptr), CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_POINTER);
	CANIF_DET_REPORTERR((NULL != mailbox), CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_POINTER);

	hrh = mailbox->Hoh;
	can_id = mailbox->CanId;
	can_sdu_ptr = pdu_info_ptr->SduDataPtr;
	can_dlc = (uint8)pdu_info_ptr->SduLength;

#endif
	canif_pdu_mode_type mode;
	boolean pdu_match = FALSE;
	uint16 hrh_cfg_index = CANIF_INVALID_CANIF_HRH;

	const canif_hoh_config_type *canif_all_hoh_cfg_ptr = canif_config_ptr->canif_init_config->canif_all_hoh_cfg_ptr;

	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_RXINDICATION_ID_CANIF_E_UNINIT);

#if (CANIF_CAN_HOH_ORDERED_IN_CAN_HRH_FIRST == CANIF_CAN_HOH_ORDERED_TYPE)
	CANIF_DET_REPORTERR((hrh < canif_all_hoh_cfg_ptr->hrh_list_size),
			    CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_HRH);
	hrh_cfg_index = canif_all_hoh_cfg_ptr->can_hoh_to_canif_hrh_map[hrh];
#else
	const uint16 *can_hoh_to_canif_hrh_map_ptr = canif_all_hoh_cfg_ptr->can_hoh_to_canif_hrh_map;

	for (uint16 hrhidx = 0; hrhidx < CANIF_MAX_NOF_HRH_COUNT; hrhidx++) {
		if (can_hoh_to_canif_hrh_map_ptr[hrhidx] == hrh) {
			hrh_cfg_index = hrhidx;
			break;
		}
	}
#endif

	CANIF_DET_REPORTERR((CANIF_INVALID_CANIF_HRH != hrh_cfg_index), CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_HRH);

	const canif_hrh_config_type *canif_hrh_config = &canif_all_hoh_cfg_ptr->canif_hrh_config[hrh_cfg_index];

	if (E_NOT_OK == canif_can_id_validation_check(can_id)) {
		return;
	}
	if (E_NOT_OK == canif_data_length_validation_check(can_dlc)) {
		return;
	}

	CANIF_DET_REPORTERR((NULL != can_sdu_ptr), CANIF_CANIF_RXINDICATION_ID_CANIF_E_PARAM_POINTER);

	uint8 channel = 0;
	uint8 can_ctrl_id = canif_hrh_config->hrh_can_ctrl_id_ref;

	if (!(can_ctrl_id < CANIF_CONTROLLER_CHANNEL_CNT)) {
		return;
	}
	channel = can_ctrl_id;
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
	set_pn_filter_disable(channel);
#endif
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON) &&                                                        \
	((CANIF_CTRL_WAKEUP_SUPPORT == STD_ON) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALID_BY_NM == STD_OFF)
	canif_set_wakeup_frame_received_flag(channel);
#endif
#endif
	const canif_rx_pdu_config_type *rx_pdu_matched_ptr = NULL;

	pdu_match = canif_is_rxpdu_match(can_id, canif_hrh_config, &rx_pdu_matched_ptr);
	if (pdu_match) {
		if (E_NOT_OK == canif_internal_dlc_check(can_dlc, rx_pdu_matched_ptr)) {
			return;
		} else {
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON) &&                                                        \
	((CANIF_CTRL_WAKEUP_SUPPORT == STD_ON) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALID_BY_NM == STD_ON)
			canif_set_wakeup_frame_received_flag(channel, rx_pdu_matched_ptr->rx_pdu_ul_rx_indication);
#endif
#endif
			if ((E_OK == canif_get_pdu_mode(channel, &mode)) &&
			    ((CANIF_ONLINE == mode) || (CANIF_TX_OFFLINE == mode))) {
#if (CANIF_PUBLIC_APPL_CANIF_RXINDICATION_CALLOUT_SUPPORT == STD_ON)
				Can_ReturnType appl_ret =
					appl_canif_rx_indication_callout(channel, can_id, can_dlc, can_sdu_ptr);
				(void)appl_ret;
#endif
				canif_transfer_rxdata_to_upper(rx_pdu_matched_ptr, can_dlc, can_sdu_ptr);
			}
		}
	}
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_set_pdu_mode(uint8 channel_id, canif_pdu_mode_type pdu_mode_request)
{
	Std_ReturnType ret = E_OK;
	canif_ctrl_runtime_state_type *ctrl_rt_state_ptr = &canif_runtime_state.ctrl_runtime_state[channel_id];
	canif_pdu_mode_type curr_mode = ctrl_rt_state_ptr->pdu_mode;
	canif_pdu_mode_type new_mode = curr_mode;
	canif_controller_mode_type ctrl_mode = ctrl_rt_state_ptr->ctrl_mode;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}

	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_SETPDUMODE_ID_CANIF_E_UNINIT, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((channel_id < CANIF_CONTROLLER_CHANNEL_CNT),
				  CANIF_CANIF_SETPDUMODE_ID_CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);

	if (ctrl_mode != CANIF_CS_STARTED) {
		ret = E_NOT_OK;
		return ret;
	}

	ret = canif_change_pdu_mode(pdu_mode_request, &new_mode, channel_id);
	if (ret == E_OK) {
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
		canif_enter_pdu_mode_with_tx_clear(channel_id, pdu_mode_request);
#endif
		ctrl_rt_state_ptr->pdu_mode = new_mode;
		(void)VCOS_LOG(CANIF_MODULE_ID, VCOS_LOG_INFO, CANIF_DLT_CHECK_ID1, channel_id, new_mode);
	}
	return ret;
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_get_pdu_mode(uint8 channel_id, canif_pdu_mode_type *pdu_mode_ptr)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_GETPDUMODE_ID_CANIF_E_UNINIT, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((channel_id < CANIF_CONTROLLER_CHANNEL_CNT),
				  CANIF_CANIF_GETPDUMODE_ID_CANIF_E_PARAM_CONTROLLERID, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((NULL != pdu_mode_ptr), CANIF_CANIF_GETPDUMODE_ID_CANIF_E_PARAM_POINTER, E_NOT_OK);
	*pdu_mode_ptr = canif_runtime_state.ctrl_runtime_state[channel_id].pdu_mode;

	return E_OK;
}

FUNC(void, CANIF_CODE) canif_controller_bus_off(uint8 controller_id)
{
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}

	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_CONTROLLER_BUSOFF_ID_CANIF_E_UNINIT);

	CANIF_DET_REPORTERR((controller_id < CANIF_CONTROLLER_CHANNEL_CNT),
			    CANIF_CANIF_CONTROLLER_BUSOFF_ID_CANIF_E_PARAM_CONTROLLER);

	canif_channel_id_type channel = (canif_channel_id_type)controller_id;

	canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr = &canif_runtime_state.ctrl_runtime_state[channel];

	if (CANIF_CS_UNINIT != canif_ctrl_runtime_state_ptr->ctrl_mode) {
#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
		canif_enter_controller_mode_with_tx_clear(channel, CANIF_CS_STOPPED);
#endif
		(void)canif_set_pdu_mode(channel, CANIF_TX_OFFLINE);
		canif_ctrl_runtime_state_ptr->ctrl_mode = CANIF_CS_STOPPED;
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
		set_pn_filter_disable(channel);
#endif
	}
#if (CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON)
	canif_tx_conf_state[channel] = CANIF_NO_NOTIFICATION;
#endif

	if (NULL != canif_dispatch_config.bus_off_notification_fct) {
		canif_dispatch_config.bus_off_notification_fct(channel);
	}
}

#if (CANIF_TRANSCEIVER_API == STD_ON)
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_trcv_mode(uint8 transceiver_id, CanTrcv_TrcvModeType transceiver_mode)
{
	uint8 trcv_hw_cnl_id;
	uint8 trcv_fct_grp_id;
	uint8 trcv_chnl_ref_id;
	boolean valid_sts = FALSE;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_SET_TRANSCEIVERMODE_ID_CANIF_E_UNINIT,
						   E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((transceiver_id < CANIF_TRANSCEIVER_CHANNEL_CNT),
				  CANIF_CANIF_SET_TRANSCEIVERMODE_ID_CANIF_E_PARAM_TRCV, E_NOT_OK);

	valid_sts = ((CANTRCV_TRCVMODE_STANDBY == transceiver_mode) || (CANTRCV_TRCVMODE_SLEEP == transceiver_mode) ||
		     (CANTRCV_TRCVMODE_NORMAL == transceiver_mode));
	CANIF_DET_REPORTERR_NO_RV((valid_sts), CANIF_CANIF_SET_TRANSCEIVERMODE_ID_CANIF_E_PARAM_TRCVMODE, E_NOT_OK);

	trcv_hw_cnl_id = CANIF_GET_CAN_TRANSCEIVERID(transceiver_id);
	trcv_fct_grp_id = CANIF_GET_CAN_TRANSCEIVERFCTGRPID(trcv_hw_cnl_id);
	trcv_chnl_ref_id = CANIF_GET_CAN_TRANSCEIVERCHNLREFID(trcv_hw_cnl_id);
	return cantrcv_set_op_mode(trcv_fct_grp_id)(trcv_chnl_ref_id, transceiver_mode);
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_get_trcv_mode(CanTrcv_TrcvModeType *transceiver_mode_ptr, uint8 transceiver_id)
{
	uint8 trcv_hw_cnl_id;
	uint8 trcv_fct_grp_id;
	uint8 trcv_chnl_ref_id;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_GET_TRANSCEIVERMODE_ID_CANIF_E_UNINIT,
						   E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((transceiver_id < CANIF_TRANSCEIVER_CHANNEL_CNT),
				  CANIF_CANIF_GET_TRANSCEIVERMODE_ID_CANIF_E_PARAM_TRCV, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((NULL != transceiver_mode_ptr),
				  CANIF_CANIF_GET_TRANSCEIVERMODE_ID_CANIF_E_PARAM_POINTER, E_NOT_OK);

	trcv_hw_cnl_id = CANIF_GET_CAN_TRANSCEIVERID(transceiver_id);
	trcv_fct_grp_id = CANIF_GET_CAN_TRANSCEIVERFCTGRPID(trcv_hw_cnl_id);
	trcv_chnl_ref_id = CANIF_GET_CAN_TRANSCEIVERCHNLREFID(trcv_hw_cnl_id);

	return cantrcv_get_op_mode(trcv_fct_grp_id)(trcv_chnl_ref_id, transceiver_mode_ptr);
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_get_trcv_wakeup_reason(uint8 transceiver_id, CanTrcv_TrcvWakeupReasonType *trcv_wu_reason_ptr)
{
	uint8 trcv_hw_cnl_id;
	uint8 trcv_fct_grp_id;
	uint8 trcv_chnl_ref_id;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_GET_TRCVWAKEUPREASON_ID_CANIF_E_UNINIT,
						   E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((transceiver_id < CANIF_TRANSCEIVER_CHANNEL_CNT),
				  CANIF_CANIF_GET_TRCVWAKEUPREASON_ID_CANIF_E_PARAM_TRCV, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((NULL != trcv_wu_reason_ptr),
				  CANIF_CANIF_GET_TRCVWAKEUPREASON_ID_CANIF_E_PARAM_POINTER, E_NOT_OK);

	trcv_hw_cnl_id = CANIF_GET_CAN_TRANSCEIVERID(transceiver_id);
	trcv_fct_grp_id = CANIF_GET_CAN_TRANSCEIVERFCTGRPID(trcv_hw_cnl_id);
	trcv_chnl_ref_id = CANIF_GET_CAN_TRANSCEIVERCHNLREFID(trcv_hw_cnl_id);

	return cantrcv_get_bus_wu_reason(trcv_fct_grp_id)(trcv_chnl_ref_id, trcv_wu_reason_ptr);
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_set_trcv_wakeup_mode(uint8 transceiver_id, CanTrcv_TrcvWakeupModeType trcv_wakeup_mode)
{
	uint8 trcv_hw_cnl_id;
	uint8 trcv_fct_grp_id;
	uint8 trcv_chnl_ref_id;
	boolean valid_sts = FALSE;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_SET_TRANSCEIVERWAKEMODE_ID_CANIF_E_UNINIT,
						   E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((transceiver_id < CANIF_TRANSCEIVER_CHANNEL_CNT),
				  CANIF_CANIF_SET_TRANSCEIVERWAKEMODE_ID_CANIF_E_PARAM_TRCV, E_NOT_OK);

	valid_sts = ((trcv_wakeup_mode == CANTRCV_WUMODE_ENABLE) || (trcv_wakeup_mode == CANTRCV_WUMODE_DISABLE) ||
		     (trcv_wakeup_mode == CANTRCV_WUMODE_CLEAR));
	CANIF_DET_REPORTERR_NO_RV((valid_sts), CANIF_CANIF_SET_TRANSCEIVERWAKEMODE_ID_CANIF_E_PARAM_TRCVWAKEUPMODE,
				  E_NOT_OK);

	trcv_hw_cnl_id = CANIF_GET_CAN_TRANSCEIVERID(transceiver_id);
	trcv_fct_grp_id = CANIF_GET_CAN_TRANSCEIVERFCTGRPID(trcv_hw_cnl_id);
	trcv_chnl_ref_id = CANIF_GET_CAN_TRANSCEIVERCHNLREFID(trcv_hw_cnl_id);

	return cantrcv_set_wakeup_mode(trcv_fct_grp_id)(trcv_chnl_ref_id, trcv_wakeup_mode);
}

FUNC(void, CANIF_CODE)
canif_trcv_mode_indication(uint8 transceiver_id, CanTrcv_TrcvModeType transceiver_mode)
{
	uint8 channel;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}
	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id,
							 CANIF_CANIF_TRANSCEIVER_MODE_INDICATION_ID_CANIF_E_UNINIT);

	CANIF_DET_REPORTERR((E_OK == trcv_to_chnl_id(transceiver_id, &channel)),
			    CANIF_CANIF_TRANSCEIVER_MODE_INDICATION_ID_CANIF_E_PARAM_TRCV);

	canif_runtime_state.trcv_runtime_state[channel].trcv_mode = transceiver_mode;

	if (NULL != canif_dispatch_config.trcv_mode_indication_fct) {
		canif_dispatch_config.trcv_mode_indication_fct(channel, transceiver_mode);
	}

	return;
}
#endif

static FUNC(boolean, CANIF_CODE) canif_check_is_wu_src_valid(ecum_wakeup_source_t wakeup_source)
{
	ecum_wakeup_source_t last_wak_up_src_val =
		(ecum_wakeup_source_t)(1UL << (ECUM_WKSOURCE_USER_CNT + ECUM_WKSOURCE_SYSTEM_CNT - 1UL));
	boolean found = FALSE;

	uint64 wak_up_src = 1;

	while (wak_up_src <= last_wak_up_src_val) {
		if ((ecum_wakeup_source_t)wak_up_src == wakeup_source) {
			found = TRUE;
			break;
		}
		wak_up_src <<= 1u;
	}
	return found;
}

#if (CANIF_WAKEUP_FUN_ENABLE == STD_ON)
#if (CANIF_CTRL_WAKEUP_SUPPORT == STD_ON)
static FUNC(Std_ReturnType, CANIF_CODE) canif_candriver_check_wakeup(ecum_wakeup_source_t wakeup_source)
{
	(void)wakeup_source;
	return E_NOT_OK;
}
#endif

#if (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON)
static FUNC(Std_ReturnType, CANIF_CODE) canif_cantrcv_check_wakeup(ecum_wakeup_source_t wakeup_source)
{
	Std_ReturnType ret = E_NOT_OK;

	uint8 index = 0;

	while (index < CANIF_TRANSCEIVER_CHANNEL_CNT) {
		const canif_transceiver_config_type *canif_transceiver_config_ptr =
			&canif_config_ptr->canif_trcv_config[index];
		if ((canif_transceiver_config_ptr->canif_cantrcv_wu_sup) &&
		    (canif_transceiver_config_ptr->canif_cantrcv_wu_src == wakeup_source)) {
#if (STD_ON == CANIF_MULTICORE_SUPPORT)
			CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(canif_transceiver_config_ptr->canif_cantrcv_core_id,
								   CANIF_CANIF_CHECKWAKEUP_ID_CANIF_E_UNINIT, E_NOT_OK);
#endif
			uint8 trcv_hw_cnl_id = CANIF_GET_CAN_TRANSCEIVERID(index);
			uint8 trcv_fct_grp_id = CANIF_GET_CAN_TRANSCEIVERFCTGRPID(trcv_hw_cnl_id);
			uint8 trcv_chnl_ref_id = CANIF_GET_CAN_TRANSCEIVERCHNLREFID(trcv_hw_cnl_id);

			if (E_OK == (uint32)cantrcv_check_wakeup(trcv_fct_grp_id)(trcv_chnl_ref_id)) {
				(void)VCOS_LOG(CANIF_MODULE_ID, VCOS_LOG_INFO, CANIF_DLT_CHECK_ID2);
				ret = E_OK;
			}
		}
		index++;
	}

	return ret;
}
#endif

#if ((CANIF_CTRL_WAKEUP_SUPPORT == STD_ON) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON))
FUNC(Std_ReturnType, CANIF_CODE)
canif_check_wakeup(ecum_wakeup_source_t wakeup_source)
{
	Std_ReturnType ret = E_NOT_OK;
	canif_core_id_type core_id;

	core_id = (canif_core_id_type)GetCoreID();
	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}

#if (STD_OFF == CANIF_MULTICORE_SUPPORT)
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_CHECKWAKEUP_ID_CANIF_E_UNINIT, E_NOT_OK);
#endif

	boolean wu_src_found = canif_check_is_wu_src_valid(wakeup_source);

	CANIF_DET_REPORTERR_NO_RV((wu_src_found), CANIF_CANIF_CHECKWAKEUP_ID_CANIF_E_PARAM_WAKEUPSOURCE, E_NOT_OK);

#if (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON)
	ret = canif_cantrcv_check_wakeup(wakeup_source);
#endif

#if (CANIF_CTRL_WAKEUP_SUPPORT == STD_ON)
	ret = canif_candriver_check_wakeup(wakeup_source);
#endif
	return ret;
}
#endif
#endif

#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
static FUNC(Std_ReturnType, CANIF_CODE) canif_candriver_check_validation(ecum_wakeup_source_t wakeup_source)
{
	(void)wakeup_source;
	return E_NOT_OK;
}

static FUNC(Std_ReturnType, CANIF_CODE) canif_cantrcv_check_validation(ecum_wakeup_source_t wakeup_source)
{
	Std_ReturnType ret = E_NOT_OK;

	canif_trcv_id_type canif_trcv_chnl_id = 0xFFU;
	canif_channel_id_type canif_ctrl_chnl_id = 0xFFu;
	ecum_wakeup_source_t wu_src_to_upper = CANIF_ECUM_WKSOURCE_NONE;
	boolean wu_src_found = FALSE;

	uint8 index = 0;

	while (index < CANIF_TRANSCEIVER_CHANNEL_CNT) {
		const canif_transceiver_config_type *canif_transceiver_config_ptr =
			&canif_config_ptr->canif_trcv_config[index];
		if (canif_transceiver_config_ptr->canif_cantrcv_wu_src == wakeup_source) {
#if (STD_ON == CANIF_MULTICORE_SUPPORT)
			CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(canif_transceiver_config_ptr->canif_cantrcv_core_id,
								   CANIF_CANIF_CHECKVALIDATION_ID_CANIF_E_UNINIT,
								   E_NOT_OK);
#endif
			wu_src_found = TRUE;
			canif_trcv_chnl_id = index;
			canif_ctrl_chnl_id = canif_transceiver_config_ptr->canif_cantrcv_to_canif_ctrl_chnl_ref;
			break;
		}
		index++;
	}

	if (wu_src_found) {
		canif_trcv_runtime_state_type *canif_trcv_runtime_state_ptr =
			&canif_runtime_state.trcv_runtime_state[canif_trcv_chnl_id];
		canif_ctrl_runtime_state_type *canif_ctrl_runtime_state_ptr =
			&canif_runtime_state.ctrl_runtime_state[canif_ctrl_chnl_id];
#if (CANIF_TRANSCEIVER_API == STD_ON)
		if ((CANTRCV_TRCVMODE_NORMAL == canif_trcv_runtime_state_ptr->trcv_mode) &&
		    (CANIF_CS_STARTED == canif_ctrl_runtime_state_ptr->ctrl_mode)) {
#else
		if ((CANIF_CS_STARTED == canif_ctrl_runtime_state_ptr->ctrl_mode)) {
#endif
			if (CANIF_FRAME_RECEIVED_FIRST == canif_ctrl_runtime_state_ptr->first_rx_indication) {
				wu_src_to_upper = wakeup_source;
			} else {
				return E_NOT_OK;
			}
			if (canif_dispatch_config.wakeup_valid_notification_fct != NULL) {
				(void)VCOS_LOG(CANIF_MODULE_ID, VCOS_LOG_INFO, CANIF_DLT_CHECK_ID4, wu_src_to_upper);
				canif_dispatch_config.wakeup_valid_notification_fct(wu_src_to_upper);
			}
			ret = E_OK;
		}
	} else {
		ret = E_OK;
#if (STD_ON == CANIF_MULTICORE_SUPPORT)
		CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(CANIF_MULTICORE_MASTER_COREID,
							   CANIF_CANIF_CHECKVALIDATION_ID_CANIF_E_UNINIT, E_NOT_OK);
#endif
	}
	return ret;
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_check_validation(ecum_wakeup_source_t wakeup_source)
{
	Std_ReturnType ret = E_OK;
	canif_core_id_type core_id;

	core_id = (canif_core_id_type)GetCoreID();
	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}

	(void)VCOS_LOG(CANIF_MODULE_ID, VCOS_LOG_INFO, CANIF_DLT_CHECK_ID3, wakeup_source);
#if (STD_OFF == CANIF_MULTICORE_SUPPORT)
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_CHECKVALIDATION_ID_CANIF_E_UNINIT, E_NOT_OK);
#endif

	boolean wu_src_found = canif_check_is_wu_src_valid(wakeup_source);

	CANIF_DET_REPORTERR_NO_RV((wu_src_found), CANIF_CANIF_CHECKVALIDATION_ID_CANIF_E_PARAM_WAKEUPSOURCE, E_NOT_OK);

#if (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON)
	ret = canif_cantrcv_check_validation(wakeup_source);
#endif

#if (CANIF_CTRL_WAKEUP_SUPPORT == STD_ON)
	ret = canif_candriver_check_validation(wakeup_source);
#endif

	return ret;
}
#endif

#if (CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON)
FUNC(canif_notif_status_type, CANIF_CODE)
canif_get_tx_confirmation_state(uint8 controller_id)
{
	canif_notif_status_type ret = CANIF_NO_NOTIFICATION;
	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return ret;
	}

	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_GETTXCONFIRMATIONSTATE_ID_CANIF_E_UNINIT,
						   CANIF_NO_NOTIFICATION);
	CANIF_DET_REPORTERR_NO_RV((controller_id < CANIF_CONTROLLER_CHANNEL_CNT),
				  CANIF_CANIF_GETTXCONFIRMATIONSTATE_ID_CANIF_E_PARAM_CONTROLLERID,
				  CANIF_NO_NOTIFICATION);

	canif_channel_id_type channel_id = (canif_channel_id_type)controller_id;

	ret = canif_tx_conf_state[channel_id];

	return ret;
}
#endif

#if ((CANIF_PUBLIC_PN_SUPPORT == STD_ON) && (CANIF_TRCV_PN_ENABLE == STD_ON))
FUNC(void, CANIF_CODE) canif_confirm_pn_availability(uint8 transceiver_id)
{
	uint8 channel;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}
	VALIDATE_NO_RV_CANIF_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_CONFIRM_PNAVAILABILITY_ID_CANIF_E_UNINIT);

	VALIDATE_NO_RV((E_OK == trcv_to_chnl_id(transceiver_id, &channel)),
		       CANIF_CANIF_CONFIRM_PNAVAILABILITY_ID_CANIF_E_PARAM_TRCV);

	if (canif_dispatch_config.canif_trcv_confirm_pn_availability_notification != NULL) {
		canif_dispatch_config.canif_trcv_confirm_pn_availability_notification(channel);
	}

	return;
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_clear_trcv_wuf_flag(uint8 transceiver_id)
{
	uint8 trcv_hw_cnl_id;
	uint8 trcv_fct_grp_id;
	uint8 trcv_chnl_ref_id;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_CLEARTRCVWUFFLAG_ID_CANIF_E_UNINIT, E_NOT_OK);

	CANIF_DET_REPORTERR_NO_RV((transceiver_id < CANIF_TRANSCEIVER_CHANNEL_CNT),
				  CANIF_CANIF_CLEARTRCVWUFFLAG_ID_CANIF_E_PARAM_TRCV, E_NOT_OK);

	trcv_hw_cnl_id = CANIF_GET_CAN_TRANSCEIVERID(transceiver_id);
	trcv_fct_grp_id = CANIF_GET_CAN_TRANSCEIVERFCTGRPID(trcv_hw_cnl_id);
	trcv_chnl_ref_id = CANIF_GET_CAN_TRANSCEIVERCHNLREFID(trcv_hw_cnl_id);

	return cantrcv_clear_trcv_wuf_flag(trcv_fct_grp_id)(trcv_chnl_ref_id);
}

FUNC(Std_ReturnType, CANIF_CODE)
canif_check_trcv_wake_flag(uint8 transceiver_id)
{
	uint8 trcv_hw_cnl_id;
	uint8 trcv_fct_grp_id;
	uint8 trcv_chnl_ref_id;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return E_NOT_OK;
	}
	CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(core_id, CANIF_CANIF_CHECKTRCVWAKEFLAG_ID_CANIF_E_UNINIT, E_NOT_OK);
	CANIF_DET_REPORTERR_NO_RV((transceiver_id < CANIF_TRANSCEIVER_CHANNEL_CNT),
				  CANIF_CANIF_CHECKTRCVWAKEFLAG_ID_CANIF_E_PARAM_TRCV, E_NOT_OK);

	trcv_hw_cnl_id = CANIF_GET_CAN_TRANSCEIVERID(transceiver_id);
	trcv_fct_grp_id = CANIF_GET_CAN_TRANSCEIVERFCTGRPID(trcv_hw_cnl_id);
	trcv_chnl_ref_id = CANIF_GET_CAN_TRANSCEIVERCHNLREFID(trcv_hw_cnl_id);

	return cantrcv_check_wake_flag(trcv_fct_grp_id)(trcv_chnl_ref_id);
}

FUNC(void, CANIF_CODE)
canif_clear_trcv_wuf_flag_indication(uint8 transceiver_id)
{
	uint8 channel;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}
	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id,
							 CANIF_CANIF_CLEARTRCV_WUFFLAG_INDICATION_CANIF_E_UNINIT);

	CANIF_DET_REPORTERR((E_OK == trcv_to_chnl_id(transceiver_id, &channel)),
			    CANIF_CANIF_CLEARTRCV_WUFFLAG_INDICATION_CANIF_E_PARAM_TRCV);

	if (canif_dispatch_config.trcv_clear_wake_flag_indication_fct != NULL) {
		canif_dispatch_config.trcv_clear_wake_flag_indication_fct(channel);
	}

	return;
}

FUNC(void, CANIF_CODE)
canif_check_trcv_wake_flag_indication(uint8 transceiver_id)
{
	uint8 channel;

	canif_core_id_type core_id = (canif_core_id_type)GetCoreID();

	if (core_id >= CANIF_MAX_ACTIVATED_CORE_CNT) {
		return;
	}
	CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(core_id,
							 CANIF_CANIF_CHECKTRCV_WAKEFLAG_INDICATION_CANIF_E_UNINIT);

	CANIF_DET_REPORTERR((E_OK == trcv_to_chnl_id(transceiver_id, &channel)),
			    CANIF_CANIF_CHECKTRCV_WAKEFLAG_INDICATION_CANIF_E_PARAM_TRCV);

	if (canif_dispatch_config.trcv_check_wake_flag_indication_fct != NULL) {
		canif_dispatch_config.trcv_check_wake_flag_indication_fct(channel);
	}
	return;
}
#endif

#define CANIF_STOP_SEC_CODE
#include "canif_memmap.h"
