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

#include "cantp_internal.h"
#include "SchM_CanTp.h"
#if (CONFIG_MODULE_CANIF == 1)
#include "canif.h"
#endif
#if (CONFIG_MODULE_PDUR == 1)
#include "pdur_cantp.h"
#endif
#include "vcos_log.h"
#include "cantp_vcos_log.h"
#include "Os.h"

#define CANTP_START_SEC_CODE
#include "cantp_memmap.h"

static FUNC(void, CANTP_CODE) reset_channel_runtime(cantp_channel_type *runtime, cantp_direction_type dir) {
	runtime->mode = (CANTP_ISO15765_TRANSMIT == dir) ? CANTP_TX_WAIT : CANTP_RX_WAIT;
	runtime->iso15765.state = CANTP_IDLE;
	runtime->sdu_id = CANTP_INVALID_PDU_ID;
}

static FUNC(PduLengthType, CANTP_CODE) get_min_dl(uint8 byte_count, cantp_state_type has_padding)
{
	PduLengthType min_dl = CANTP_MIN_DL_0;

	if ((byte_count <= CANTP_MIN_DL_0) || (byte_count > CANTP_MIN_DL_64)) {
		min_dl = CANTP_MIN_DL_0;
	} else if (byte_count <= CANTP_MIN_DL_8) {
		min_dl = (CANTP_ON == has_padding) ? CANTP_MIN_DL_8 : byte_count;
	} else if (byte_count <= CANTP_MIN_DL_12) {
		min_dl = CANTP_MIN_DL_12;
	} else if (byte_count <= CANTP_MIN_DL_16) {
		min_dl = CANTP_MIN_DL_16;
	} else if (byte_count <= CANTP_MIN_DL_20) {
		min_dl = CANTP_MIN_DL_20;
	} else if (byte_count <= CANTP_MIN_DL_24) {
		min_dl = CANTP_MIN_DL_24;
	} else if (byte_count <= CANTP_MIN_DL_32) {
		min_dl = CANTP_MIN_DL_32;
	} else if (byte_count <= CANTP_MIN_DL_48) {
		min_dl = CANTP_MIN_DL_48;
	} else {
		min_dl = CANTP_MIN_DL_64;
	}

	return min_dl;
}

static FUNC(uint8, CANTP_CODE) get_addr_offset(cantp_addr_format_type format)
{
	uint8 addr_offset = 0;

#if ((CANTP_EXTENDED_ADDR_TX_CFG == STD_ON) || (CANTP_EXTENDED_ADDR_RX_CFG == STD_ON))
	if (CANTP_EXTENDED_ADDR == format) {
		addr_offset = 1;
	}
#else
	(void)format;
#endif
	return addr_offset;
}

static FUNC(BufReq_ReturnType, CANTP_CODE)
copy_segment_to_upper_rx_buffer(const cantp_rx_nsdu_type *rx_config,
				cantp_channel_type *rx_runtime, uint8 *segment,
				PduLengthType segment_size)
{
	BufReq_ReturnType ret = BUFREQ_E_NOT_OK;
	PduInfoType pdu;

	(void)memset(&pdu, 0, sizeof(PduInfoType));

#if (CONFIG_MODULE_PDUR == 1)
	/* 申请pdur buffer */
	if (CANTP_RX_WAIT_CF == rx_runtime->iso15765.state) {
		/* 等待cf状态不传输数据,获取当前pdur可用buffer大小 */
		ret = pdur_cantp_copy_rx_data(rx_config->pdur_pdu_id, &pdu, &rx_runtime->size_buffer);
	} else {
		ret = pdur_cantp_start_of_reception(rx_config->pdur_pdu_id, NULL, rx_runtime->transfer_total,
						    &rx_runtime->size_buffer);
		/* 返回ok的话说明buffer locked */
		/* 等到接收到pdur_cantp_tx_confirmation 或 pdur_cantp_rx_indication后解锁 */
	}
#endif

	if (BUFREQ_OK != ret) {
		return ret;
	}

	/* 待传数据小于可用buffer则进行数据拷贝 */
	pdu.SduLength = segment_size;
	pdu.SduDataPtr = segment;
#if (CONFIG_MODULE_PDUR == 1)
	ret = pdur_cantp_copy_rx_data(rx_config->pdur_pdu_id, &pdu, &rx_runtime->size_buffer);
#endif
	if (BUFREQ_OK == ret) {
		rx_runtime->transfer_count += pdu.SduLength;
	}

	return ret;
}

static FUNC(Std_ReturnType, CANTP_CODE)
can_transmit_with_rx_padding_cfg(const cantp_rx_nsdu_type *rx_config, cantp_channel_type *rx_runtime,
				 PduInfoType *pdu_info)
{
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	boolean padding_activation = (CANTP_ON == rx_config->cantp_rx_padding_activation) &&
				     (CANTP_ON == cantp_cfg->cantp_general->padding_activation);
	if (padding_activation) {
		for (uint32 i = pdu_info->SduLength; i < CANTP_CAN20_FL; i++) {
			pdu_info->SduDataPtr[i] = cantp_cfg->cantp_general->padding_pattern;
		}
	}
	pdu_info->SduLength = CANTP_CAN20_FL; /* 流控帧长度固定8byte */
	rx_runtime->iso15765.nas_nar_pending = TRUE;

	return canif_transmit(rx_config->canif_fc_pdu_id, pdu_info);
}

static FUNC(Std_ReturnType, CANTP_CODE)
can_transmit_with_tx_padding_cfg(const cantp_tx_nsdu_type *tx_config, cantp_channel_type *tx_runtime,
				 PduInfoType *pdu_info)
{
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	boolean padding_activation = (CANTP_ON == tx_config->cantp_tx_padding_activation) &&
				     (CANTP_ON == cantp_cfg->cantp_general->padding_activation);
	PduLengthType pad_len = cantp_get_padding_length((uint8)(pdu_info->SduLength),
							 padding_activation, tx_runtime->canfd_support);
	pad_len = MIN(pad_len, tx_config->cantp_npdu_len);
	if (padding_activation) {
		for (uint32 i = pdu_info->SduLength; i < pad_len; i++) {
			pdu_info->SduDataPtr[i] = cantp_cfg->cantp_general->padding_pattern;
		}
		pdu_info->SduLength = pad_len;
	}
	tx_runtime->iso15765.nas_nar_pending = TRUE;

	return canif_transmit(tx_config->canif_pdu_id, pdu_info);
}

static FUNC(BufReq_ReturnType, CANTP_CODE)
send_next_tx_frame(const cantp_tx_nsdu_type *tx_config, cantp_channel_type *tx_runtime,
			PduInfoType *pdu_info_ptr, RetryInfoType *retry_info)
{
	BufReq_ReturnType ret = BUFREQ_E_NOT_OK;
	PduInfoType pdu_info;
	PduLengthType offset = pdu_info_ptr->SduLength;

	pdu_info.SduDataPtr = &pdu_info_ptr->SduDataPtr[offset];
	pdu_info.SduLength =
		MIN((tx_runtime->transfer_total - tx_runtime->transfer_count), (tx_config->cantp_npdu_len - offset));

#if (CONFIG_MODULE_PDUR == 1)
	ret = pdur_cantp_copy_tx_data(tx_config->pdur_pdu_id, &pdu_info, retry_info, &tx_runtime->size_buffer);
#endif
	if ((BUFREQ_E_NOT_OK == ret) || (BUFREQ_E_OVFL == ret)) {
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_WARN, CANTP_DLT_CHECK_ID14);
	} else if (BUFREQ_OK == ret) { /* 数据复制成功 */
		Std_ReturnType canif_ret = E_OK;

		pdu_info_ptr->SduLength += pdu_info.SduLength;
		tx_runtime->transfer_count += pdu_info.SduLength;

		pdu_info.SduDataPtr = &pdu_info_ptr->SduDataPtr[0];
		pdu_info.SduLength += offset;

		tx_runtime->iso15765.state_timeout_count = tx_config->cantp_nas + (uint16)1;
		tx_runtime->iso15765.state = CANTP_TX_WAIT_CONFIRMATION;

		canif_ret = can_transmit_with_tx_padding_cfg(tx_config, tx_runtime, &pdu_info);
		if (E_OK != canif_ret) {
			// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_WARN, CANTP_DLT_CHECK_ID18, canif_ret);
			ret = BUFREQ_E_NOT_OK; /* 发送失败 */
		}
	}
	return ret;
}

static FUNC(void, CANTP_CODE)
send_flow_control_frame(const cantp_rx_nsdu_type *rx_config, cantp_channel_type *rx_runtime,
			BufReq_ReturnType flow_status)
{
	uint8 index = 0;
	uint8 sdu_data[8];
	uint16 max_payload_cf = 0;
	uint16 computed_bs = 0;
	PduInfoType pdu_info;
	Std_ReturnType ret = E_OK;

	(void)memset(&pdu_info, 0, sizeof(pdu_info));
	pdu_info.SduDataPtr = &sdu_data[0];
#if (CANTP_EXTENDED_ADDR_RX_CFG == STD_ON)
	if (CANTP_EXTENDED_ADDR == rx_config->cantp_addressing_format) {
		sdu_data[index++] = rx_runtime->iso15765.extended_address;
	}
#endif

	if (BUFREQ_OK == flow_status) {
		sdu_data[index++] = CANTP_ISO15765_TPCI_FC | CANTP_ISO15765_FLOW_CONTROL_STATUS_CTS;
		/* 最长的cf的payload长度 减去pci与地址字节 */
		max_payload_cf = (uint16)(rx_runtime->initial_dlc - index - CANTP_PCI_LENGTH_CF);
		computed_bs = (uint16)(rx_runtime->size_buffer / max_payload_cf);
		computed_bs = MIN(computed_bs, rx_runtime->iso15765.config_bs);
		rx_runtime->iso15765.bs = rx_runtime->iso15765.config_bs;
		sdu_data[index++] = (uint8)computed_bs;
		sdu_data[index++] = rx_runtime->iso15765.config_stmin;
		rx_runtime->iso15765.next_flow_control_count = computed_bs;
		pdu_info.SduLength = index;
		rx_runtime->cantp_wft_max_counter = 0;
	} else if (BUFREQ_E_BUSY == flow_status) {
		sdu_data[index++] = CANTP_ISO15765_TPCI_FC | CANTP_ISO15765_FLOW_CONTROL_STATUS_WAIT;
		index += 2U;
		pdu_info.SduLength = index;
		rx_runtime->cantp_wft_max_counter++;
	} else if (BUFREQ_E_OVFL == flow_status) {
		sdu_data[index++] = CANTP_ISO15765_TPCI_FC | CANTP_ISO15765_FLOW_CONTROL_STATUS_OVFLW;
		index += 2U;
		pdu_info.SduLength = index;
	} else {
		/* do nothing */
	}

	ret = can_transmit_with_rx_padding_cfg(rx_config, rx_runtime, &pdu_info);
	if (E_OK == ret) {
		return;
	}

#if (CONFIG_MODULE_PDUR == 1)
	pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_E_NOT_OK);
#endif
	// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID0);
	reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);
}

static FUNC(void, CANTP_CODE)
construct_frame_header(cantp_frame_type frame_type, const cantp_tx_nsdu_type *tx_config,
		       cantp_channel_type *tx_runtime, PduInfoType *pdu_info)
{
	uint8 *sdu_data = pdu_info->SduDataPtr; // 从pdu_info获取数据缓冲区指针
	uint16 byte_count = 0;
	uint8 max_sfdl = CANTP_MAX_SF_DL_STDADDR;

#if (CANTP_EXTENDED_ADDR_TX_CFG == STD_ON)
	if (CANTP_EXTENDED_ADDR == tx_config->cantp_addressing_mode) {
		sdu_data[byte_count++] = tx_config->cantp_nta;
		max_sfdl = CANTP_MAX_SF_DL_EXTENDADDR;
	}
#endif
	if (CANTP_SINGLE_FRAME == frame_type) {
		if (tx_runtime->transfer_total > max_sfdl) {
			/* 首字节按canfd的规则填零,第二个字节填sf_dl */
			sdu_data[byte_count++] = CANTP_CANFD_FIRST_BYTE_ZERO;
			sdu_data[byte_count++] = (uint8)(CANTP_ISO15765_TPCI_DL_FD & (uint8)(tx_runtime->transfer_total));
		} else {
			/* 标准can的填写方法,首字节前4bit恒为0,后4bit_sf_dl */
			sdu_data[byte_count++] = (uint8)(CANTP_ISO15765_TPCI_DL & (uint8)(tx_runtime->transfer_total));
		}
	} else { // 第1个字节高4位，值为1，低4位+第2个字节为dlc长度
		if (tx_runtime->transfer_total <= CANTP_FF_MAXDL) {
			sdu_data[byte_count++] =
				(uint8)(CANTP_ISO15765_TPCI_FF | (uint8)((tx_runtime->transfer_total & 0xf00u) >> 8U));
			sdu_data[byte_count++] = (uint8)(tx_runtime->transfer_total & 0xffu);
		} else { // 第1个字节高4位，值为1，低4位和第2个字节恒为0，第3-6个字节位dlc
			sdu_data[byte_count++] = CANTP_ISO15765_TPCI_FF; // 第1个字节为10
			sdu_data[byte_count++] = 0; // 第2个字节为00
			// 按照大端格式填充dlc的第一个字节
			sdu_data[byte_count++] =
				(uint8)(((uint32)tx_runtime->transfer_total) >> 24);
			// 按照大端格式填充dlc的第二个字节
			sdu_data[byte_count++] =
				(uint8)(((uint32)tx_runtime->transfer_total) >> 16);
			// 按照大端格式填充dlc的第三个字节
			sdu_data[byte_count++] =
				(uint8)(((uint32)tx_runtime->transfer_total) >> 8);
			// 按照大端格式填充dlc的第四个字节
			sdu_data[byte_count++] =
				(uint8)((uint32)tx_runtime->transfer_total);
		}
		tx_runtime->iso15765.next_flow_control_count = 1;
		tx_runtime->iso15765.bs = 1;
	}
	pdu_info->SduLength = byte_count;
}

static FUNC(void, CANTP_CODE)
on_copy_cf_to_upper_ok(const cantp_rx_nsdu_type *rx_config, cantp_channel_type *rx_runtime)
{
	if (rx_runtime->transfer_total <= rx_runtime->transfer_count) {
		reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_OK);
#endif
	} else {
		rx_runtime->iso15765.frames_handled_count++;
		COUNT_DECREMENT(rx_runtime->iso15765.next_flow_control_count);
		if ((0U == rx_runtime->iso15765.next_flow_control_count) && (rx_runtime->iso15765.bs > 0U)) {
			send_flow_control_frame(rx_config, rx_runtime, BUFREQ_OK);
		} else {
			rx_runtime->iso15765.state_timeout_count = ((rx_config->cantp_ncr) + 1U);
		}
	}
}

static FUNC(void, CANTP_CODE) abort_ongoing_rx_pdu(cantp_channel_type *rx_runtime, uint8 core_id)
{
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();

	if (CANTP_RX_PROCESSING == rx_runtime->mode) {
		CANTP_VALIDATE_NO_RV(CANTP_INVALID_PDU_ID != rx_runtime->sdu_id,
			       CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_INVALID_RX_SDUID);
		PduIdType pdur_pdu_ongoing = cantp_cfg->cantp_nsdu_channel_info[core_id]
						.nsdu_cfg_addr[rx_runtime->sdu_id]
						.config_data.cantp_rx_nsdu.pdur_pdu_id;
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_rx_indication(pdur_pdu_ongoing, NTFRSLT_E_NOT_OK);
#endif
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID1);
	}
	cantp_init_runtime_data(rx_runtime, CANTP_ISO15765_RECEIVE);
}

static FUNC(void, CANTP_CODE) copy_single_frame_data(const cantp_rx_nsdu_type *rx_config,
							    cantp_channel_type *rx_runtime, uint8 *data,
							    PduLengthType pdu_length)
{
	BufReq_ReturnType ret = copy_segment_to_upper_rx_buffer(rx_config, rx_runtime, data, pdu_length);

#if (CONFIG_MODULE_PDUR == 1)
	if (BUFREQ_OK == ret) {
		pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_OK);
	} else {
		pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_E_NOT_OK);
	}
#endif
	reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);
}

static FUNC(void, CANTP_CODE)
handle_single_frame(const cantp_rx_nsdu_type *rx_config, cantp_channel_type *rx_runtime,
		    const PduInfoType *rx_pdu_data, PduIdType cantp_rx_nsdu_id, uint8 core_id)
{
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	uint8 addr_offset = get_addr_offset(rx_config->cantp_addressing_format);
	PduLengthType pdu_length = rx_pdu_data->SduDataPtr[addr_offset] & CANTP_ISO15765_TPCI_DL;
	uint8 payload_offset = addr_offset + CANTP_PCI_LENGTH_SF;
	uint8 min_payload_length = 1;
	uint8 max_payload_length = CANTP_CAN20_FL - payload_offset;
	cantp_state_type padding_activation = CANTP_OFF;

	if (rx_pdu_data->SduLength > CANTP_CAN20_FL) {
		CANTP_VALIDATE_NO_RV(0UL == pdu_length, CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_INVALID_RX_LENGTH);
		pdu_length = (rx_pdu_data->SduDataPtr[addr_offset + 1U] & CANTP_ISO15765_TPCI_DL_FD);
		payload_offset = addr_offset + CANTP_PCI_LENGTH_LSF;
		min_payload_length = CANTP_CAN20_FL - addr_offset;
		max_payload_length = (uint8)(rx_config->cantp_npdu_len - payload_offset);
	}

	if ((CANTP_ON == cantp_cfg->cantp_general->padding_activation) &&
	    (CANTP_ON == rx_config->cantp_rx_padding_activation)) {
		padding_activation = CANTP_ON;
	}
	CANTP_VALIDATE_NO_RV(rx_pdu_data->SduLength ==
		get_min_dl((uint8)(pdu_length + payload_offset), padding_activation),
		CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_INVALID_RX_LENGTH);
	CANTP_VALIDATE_NO_RV(pdu_length >= min_payload_length,
			     CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_INVALID_RX_LENGTH);
	CANTP_VALIDATE_NO_RV(pdu_length <= max_payload_length,
			     CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_INVALID_RX_LENGTH);

	abort_ongoing_rx_pdu(rx_runtime, core_id);
	rx_runtime->transfer_total = pdu_length;
	rx_runtime->iso15765.state = CANTP_RX_WAIT_SF_FF_PDUR_BUFFER;
	rx_runtime->mode = CANTP_RX_PROCESSING;
	rx_runtime->iso15765.state_timeout_count = rx_config->cantp_nbr + (uint16)1;
	rx_runtime->sdu_id = cantp_rx_nsdu_id;
	uint8 *data = &rx_pdu_data->SduDataPtr[payload_offset];

	copy_single_frame_data(rx_config, rx_runtime, data, pdu_length);
	return;
}

static FUNC(void, CANTP_CODE)
copy_first_frame_data(const cantp_rx_nsdu_type *rx_config, cantp_channel_type *rx_runtime,
		      const PduInfoType *rx_pdu_data, uint8 payload_offset)
{
	PduLengthType payload_length = rx_pdu_data->SduLength - payload_offset;
	BufReq_ReturnType ret = copy_segment_to_upper_rx_buffer(rx_config, rx_runtime,
					&rx_pdu_data->SduDataPtr[payload_offset], payload_length);

	if (BUFREQ_OK == ret) {
		rx_runtime->iso15765.state_timeout_count = rx_config->cantp_ncr + (uint16)1;
		rx_runtime->iso15765.state = CANTP_RX_WAIT_CF;
		rx_runtime->mode = CANTP_RX_PROCESSING;
		send_flow_control_frame(rx_config, rx_runtime, BUFREQ_OK);

		return;
	}

	if ((BUFREQ_E_OVFL == ret) || (BUFREQ_E_BUSY == ret) || (BUFREQ_E_NOT_OK == ret)) {
		if (BUFREQ_E_NOT_OK == ret) {
			// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID3);
		} else {
			// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID2);
			send_flow_control_frame(rx_config, rx_runtime, BUFREQ_E_OVFL);
		}
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_rx_indication(rx_runtime->sdu_id, NTFRSLT_E_NOT_OK);
#endif
		reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);
	}
}

static FUNC(void, CANTP_CODE)
handle_first_frame(const cantp_rx_nsdu_type *rx_config, cantp_channel_type *rx_runtime,
		   const PduInfoType *rx_pdu_data, PduIdType cantp_rx_nsdu_id, uint8 core_id)
{
	if (CANTP_FUNCTIONAL == rx_config->cantp_rx_ta_type) {
		return;
	}
	uint8 addr_offset = get_addr_offset(rx_config->cantp_addressing_format);

	PduLengthType pdu_length = rx_pdu_data->SduDataPtr[addr_offset + 1U] +
				   ((PduLengthType)((rx_pdu_data->SduDataPtr[addr_offset]) & (uint8)0xf) << 8U);
	uint8 payload_offset = addr_offset + CANTP_PCI_LENGTH_FF;
	PduLengthType length_lower_limit =
		(rx_pdu_data->SduLength > CANTP_CAN20_FL) ?
			(rx_pdu_data->SduLength - (PduLengthType)addr_offset - CANTP_PCI_LENGTH_LSF) :
			(CANTP_CAN20_FL - (PduLengthType)addr_offset - CANTP_PCI_LENGTH_SF);

#if (CANTP_LONG_FF_SUPPORT == STD_ON)
	if (0UL == pdu_length) {
		pdu_length = (PduLengthType)(((uint32)(rx_pdu_data->SduDataPtr[addr_offset + 2U]) << 24) |
					     ((uint32)(rx_pdu_data->SduDataPtr[addr_offset + 3U]) << 16) |
					     ((uint32)(rx_pdu_data->SduDataPtr[addr_offset + 4U]) << 8) |
					     ((uint32)(rx_pdu_data->SduDataPtr[addr_offset + 5U])));
		payload_offset = addr_offset + CANTP_PCI_LENGTH_LFF;
		length_lower_limit = CANTP_FF_MAXDL;
	}
#endif

	if ((pdu_length > length_lower_limit) && (rx_pdu_data->SduLength <= rx_config->cantp_npdu_len) &&
	    (rx_pdu_data->SduLength >= CANTP_CAN20_FL)) {
		abort_ongoing_rx_pdu(rx_runtime, core_id);
		rx_runtime->initial_dlc = (uint8)rx_pdu_data->SduLength;
		rx_runtime->transfer_total = pdu_length;
		rx_runtime->sdu_id = cantp_rx_nsdu_id;
		rx_runtime->iso15765.frames_handled_count = CANTP_CF_FSN;
		rx_runtime->iso15765.state = CANTP_RX_WAIT_SF_FF_PDUR_BUFFER;
		rx_runtime->mode = CANTP_RX_PROCESSING;
		rx_runtime->iso15765.state_timeout_count = rx_config->cantp_nbr;
		rx_runtime->iso15765.extended_address = rx_config->cantp_nta;

		copy_first_frame_data(rx_config, rx_runtime, rx_pdu_data, payload_offset);
	}

	return;
}

static FUNC(void, CANTP_CODE)
handle_consecutive_frame(const cantp_rx_nsdu_type *rx_config, cantp_channel_type *rx_runtime,
			 const PduInfoType *rx_pdu_data)
{
	if ((CANTP_FUNCTIONAL == rx_config->cantp_rx_ta_type) || (CANTP_RX_WAIT_CF != rx_runtime->iso15765.state)) {
		return;
	}

	uint8 addr_offset = get_addr_offset(rx_config->cantp_addressing_format);
	uint8 segment_number = (uint8)(rx_pdu_data->SduDataPtr[addr_offset] & CANTP_SEGMENT_NUMBER_MASK);

	if (segment_number != (rx_runtime->iso15765.frames_handled_count & CANTP_SEGMENT_NUMBER_MASK)) {
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_E_WRONG_SN);
#endif
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID4);
		reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);

		return;
	}

	uint8 payload_offset = addr_offset + CANTP_PCI_LENGTH_CF;
	/* sdu的数据长度(抛去nta与sn位后的数据位) 本连续帧的有效数据长度*/
	PduLengthType current_segment_size =
		(PduLengthType)rx_runtime->initial_dlc -
		(PduLengthType)payload_offset;
	PduLengthType bytes_left_to_copy =
		rx_runtime->transfer_total - rx_runtime->transfer_count; /* 尚未传完的pdu长度 */
	boolean is_date_length_valid = FALSE;

	if (current_segment_size <= bytes_left_to_copy) {
		if (rx_pdu_data->SduLength == rx_runtime->initial_dlc) {
			is_date_length_valid = TRUE;
		}
	} else {
		if (rx_pdu_data->SduLength >= (bytes_left_to_copy + payload_offset)) {
			current_segment_size = bytes_left_to_copy;
			is_date_length_valid = TRUE;
		}
	}

	if (FALSE == is_date_length_valid) {
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_E_PARAMETER_NOT_OK);
#endif
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID5);

		reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);

		return;
	}

	/* 将接收到的数据复制到pdur提供的缓冲区中 */
	BufReq_ReturnType ret = copy_segment_to_upper_rx_buffer(rx_config, rx_runtime,
					&rx_pdu_data->SduDataPtr[payload_offset], current_segment_size);

	if (BUFREQ_E_NOT_OK == ret) {
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_rx_indication(rx_config->pdur_pdu_id, NTFRSLT_E_NOT_OK);
#endif
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID6);
		reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);
	} else if (BUFREQ_OK == ret) {
		on_copy_cf_to_upper_ok(rx_config, rx_runtime);
	} else {
		/* do nothing */
	}
}

static FUNC(void, CANTP_CODE)
handle_flow_control_frame(const cantp_tx_nsdu_type *tx_config, cantp_channel_type *tx_runtime,
			  const PduInfoType *tx_pdu_data)
{
	uint8 index = get_addr_offset(tx_config->cantp_addressing_mode);
	uint8 fc_status = 0u;

	if (CANTP_TX_WAIT_FC != tx_runtime->iso15765.state) {
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID16);
		return;
	}

	fc_status = tx_pdu_data->SduDataPtr[index++] & CANTP_ISO15765_TPCI_FS_MASK;

	if (CANTP_ISO15765_FLOW_CONTROL_STATUS_CTS == fc_status) {
		tx_runtime->iso15765.bs = tx_pdu_data->SduDataPtr[index++];
		tx_runtime->iso15765.next_flow_control_count = tx_runtime->iso15765.bs;
		tx_runtime->iso15765.stmin = tx_pdu_data->SduDataPtr[index];
		tx_runtime->iso15765.state_timeout_count =
			tx_config->cantp_ncs + (uint16)1; /* 改变状态并启动定时器 */
		tx_runtime->iso15765.state = CANTP_TX_WAIT_TRANSMIT;

		return;
	}

	if (CANTP_ISO15765_FLOW_CONTROL_STATUS_WAIT == fc_status) {
		tx_runtime->iso15765.state_timeout_count = tx_config->cantp_nbs + (uint16)1;
		tx_runtime->iso15765.state = CANTP_TX_WAIT_FC;

		return;
	}

#if (CONFIG_MODULE_PDUR == 1)
	if (CANTP_ISO15765_FLOW_CONTROL_STATUS_OVFLW == fc_status) {
		pdur_cantp_tx_confirmation(tx_config->pdur_pdu_id, NTFRSLT_E_NO_BUFFER);
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID8);
	} else {
		pdur_cantp_tx_confirmation(tx_config->pdur_pdu_id, NTFRSLT_E_INVALID_FS);
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID9);
	}
#endif
	reset_channel_runtime(tx_runtime, CANTP_ISO15765_TRANSMIT);
}

static FUNC(uint16, CANTP_CODE) calc_stmin_timer(uint8 stmin_value, uint8 core_id)
{
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();

	if ((stmin_value >= CANTP_STMIN_MICROSEC_MIN) && (stmin_value <= CANTP_STMIN_MICROSEC_MAX)) {
		return 1;
	}

	uint8 stmin_temp = MIN(stmin_value, CANTP_STMIN_MILLISEC_MAX);
	uint16 stmin_timer =
		(uint16)((uint32)stmin_temp / cantp_cfg->cantp_mainfunction_period_list[core_id]) + 1u;

	return stmin_timer;
}

static FUNC(void, CANTP_CODE)
tx_sdu_state_machine_timeout_check(cantp_channel_type *tx_runtime, PduIdType pdu_id,
				   NotifResultType result)
{
	COUNT_DECREMENT(tx_runtime->iso15765.state_timeout_count);
	if (0UL == tx_runtime->iso15765.state_timeout_count) { /* 检查否超时 */
		reset_channel_runtime(tx_runtime, CANTP_ISO15765_TRANSMIT);
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_tx_confirmation(pdu_id, result);
#endif
		// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID13, result);
	}
}

static FUNC(void, CANTP_CODE)
tx_sdu_state_machine_transmit_tx_buffer(const cantp_tx_nsdu_type *tx_cfg,
					cantp_channel_type *tx_runtime,
					RetryInfoType *retry_info)
{
	PduInfoType pdu_info;

	// 单帧或首帧若是busy则transfer_count一定为0，不为0则是连续帧busy
	if (0 == tx_runtime->transfer_count) {
		uint8 sdu_data[CANTP_SDU_DATA_LENGTH]; // 扩展数据

		pdu_info.SduDataPtr = &sdu_data[0];
		cantp_frame_type iso15765frame =
			cantp_calc_tx_frame_type(tx_cfg, tx_runtime->transfer_total);
		construct_frame_header(iso15765frame, tx_cfg, tx_runtime, &pdu_info);

	} else {
		tx_runtime->iso15765.frames_handled_count++;
		uint8 sdu_data[CANTP_SDU_DATA_LENGTH]; // 扩展数据
		uint16 byte_count = 1;

		pdu_info.SduDataPtr = &sdu_data[0];
		/* 准备下一帧的tx buffer */
#if (CANTP_EXTENDED_ADDR_TX_CFG == STD_ON)
		if (CANTP_EXTENDED_ADDR == tx_cfg->cantp_addressing_mode) {
			byte_count++;
		}
#endif
		pdu_info.SduLength = byte_count;
		sdu_data[byte_count - 1U] =
			(uint8)((tx_runtime->iso15765.frames_handled_count & CANTP_SEGMENT_NUMBER_MASK) +
				CANTP_ISO15765_TPCI_CF); /* 构造pci */
	}

	BufReq_ReturnType ret = send_next_tx_frame(tx_cfg, tx_runtime, &pdu_info, retry_info);

	;
	if (BUFREQ_E_BUSY == ret) {
		tx_runtime->iso15765.state = CANTP_TX_WAIT_COPY_TX_BUSY;
		tx_sdu_state_machine_timeout_check(tx_runtime, tx_cfg->pdur_pdu_id,
						   NTFRSLT_E_NOT_OK); /* 检查n_cs是否超时 */
	} else if (BUFREQ_OK != ret) {
		reset_channel_runtime(tx_runtime, CANTP_ISO15765_TRANSMIT);
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_tx_confirmation(tx_cfg->pdur_pdu_id, NTFRSLT_E_NOT_OK);
#endif
	} else {
		/* do nothing */
	}
}

FUNC(uint8, CANTP_CODE) cantp_get_core_id(void)
{
	uint8 core_id = 0u;

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	core_id = (uint8)get_core_id();
#endif

	return core_id;
}

FUNC_P2CONST(cantp_runtime_data_type, AUTOMATIC, CANTP_CODE) cantp_get_core_runtime_data(uint8 core_id)
{
	const cantp_runtime_data_type *channel_core_tab = cantp_get_channel_core_table();

	return &channel_core_tab[core_id];
}

FUNC(void, CANTP_CODE)
cantp_handle_send_next_tx_frame(const cantp_tx_nsdu_type *tx_config, cantp_channel_type *tx_runtime, uint8 core_id)
{
	COUNT_DECREMENT(tx_runtime->iso15765.next_flow_control_count);
	if (tx_runtime->transfer_total <= tx_runtime->transfer_count) { /* 传输完成 */
		reset_channel_runtime(tx_runtime, CANTP_ISO15765_TRANSMIT);
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_tx_confirmation(tx_config->pdur_pdu_id, NTFRSLT_OK);
#endif
		return;
	}

	if ((0U == tx_runtime->iso15765.next_flow_control_count) &&
		   (tx_runtime->iso15765.bs > 0U)) { /* cf已发完,期望收到fc */
		tx_runtime->iso15765.state_timeout_count = tx_config->cantp_nbs + (uint16)1;
		tx_runtime->iso15765.state = CANTP_TX_WAIT_FC;

		return;
	}

	/* stmin之后发送下一帧cf 根据stmin判断状态*/
	if (0U == tx_runtime->iso15765.stmin) {
		tx_runtime->iso15765.frames_handled_count++;
		PduInfoType pdu_info;
		RetryInfoType retry_info;
		uint8 sdu_data[CANTP_SDU_DATA_LENGTH] = {0}; // 扩展数据
		uint16 byte_count = 1;

		pdu_info.SduDataPtr = &sdu_data[0];
		/* 准备下一帧的tx buffer */
#if (CANTP_EXTENDED_ADDR_TX_CFG == STD_ON)
		if (CANTP_EXTENDED_ADDR == tx_config->cantp_addressing_mode) {
			byte_count++;
		}
#endif
		retry_info.TpDataState = TP_DATACONF;
		retry_info.TxTpDataCnt = tx_runtime->transfer_count;

		pdu_info.SduLength = byte_count;
		sdu_data[byte_count - 1U] =
			(uint8)((tx_runtime->iso15765.frames_handled_count & CANTP_SEGMENT_NUMBER_MASK) +
				CANTP_ISO15765_TPCI_CF); /* 构造pci */
		BufReq_ReturnType ret = send_next_tx_frame(tx_config, tx_runtime, &pdu_info, &retry_info);

		if (BUFREQ_E_BUSY == ret) {
			tx_runtime->iso15765.state = CANTP_TX_WAIT_COPY_TX_BUSY;
		} else if (BUFREQ_OK != ret) {
			reset_channel_runtime(tx_runtime, CANTP_ISO15765_TRANSMIT);
#if (CONFIG_MODULE_PDUR == 1)
			pdur_cantp_tx_confirmation(tx_config->pdur_pdu_id, NTFRSLT_E_NOT_OK);
#endif
		} else {
			/* do nothing */
		}
	} else {
		tx_runtime->iso15765.state_timeout_count =
			calc_stmin_timer(tx_runtime->iso15765.stmin, core_id);
		tx_runtime->iso15765.state = CANTP_TX_WAIT_STMIN;
	}
}

FUNC(void, CANTP_CODE)
cantp_handle_rx_frame(cantp_frame_type frame_type, PduIdType cantp_rx_nsdu_id, const PduInfoType *cantp_rx_pdu_ptr,
		const cantp_rx_nsdu_type *rx_config, const cantp_tx_nsdu_type *tx_config,
		cantp_channel_type *runtime_params, uint8 core_id)
{
	switch (frame_type) {
		case CANTP_SINGLE_FRAME:
			handle_single_frame(rx_config, runtime_params, cantp_rx_pdu_ptr, cantp_rx_nsdu_id, core_id);
			break;
		case CANTP_FIRST_FRAME:
			handle_first_frame(rx_config, runtime_params, cantp_rx_pdu_ptr, cantp_rx_nsdu_id, core_id);
			break;
		case CANTP_CONSECUTIVE_FRAME:
			handle_consecutive_frame(rx_config, runtime_params, cantp_rx_pdu_ptr);
			break;
		case CANTP_FLOW_CONTROL_FRAME:
			handle_flow_control_frame(tx_config, runtime_params, cantp_rx_pdu_ptr);
			break;
		default:
			break;
	}
}

FUNC(boolean, CANTP_CODE)
cantp_check_rx_ind_addr(cantp_frame_type frame_type, const PduInfoType *pdu_ptr,
			const cantp_rx_nsdu_type *rx_config, const cantp_tx_nsdu_type *tx_config)
{
	if ((CANTP_FLOW_CONTROL_FRAME == frame_type) && (CANTP_EXTENDED_ADDR == tx_config->cantp_addressing_mode) &&
	    (pdu_ptr->SduDataPtr[0] != tx_config->cantp_nsa)) {
		return FALSE;
	}

	if ((CANTP_FLOW_CONTROL_FRAME != frame_type) && (CANTP_EXTENDED_ADDR == rx_config->cantp_addressing_format) &&
	    (pdu_ptr->SduDataPtr[0] != rx_config->cantp_nta)) {
		return FALSE;
	}

	return TRUE;
}

FUNC(PduLengthType, CANTP_CODE)
cantp_get_padding_length(uint8 current_length, boolean padding_activation, boolean cantp_fd_support)
{
	uint8 pad_len = 0;

	if (current_length <= CANTP_CAN20_FL) {
		if (FALSE == padding_activation) {
			pad_len = current_length;
		} else {
			pad_len = CANTP_CAN20_FL;
		}
	}

#if (CANTP_CANFD_SUPPORT == STD_ON)
	if ((TRUE == cantp_fd_support) && (current_length > CANTP_CAN20_FL)) {
		if (current_length <= CANTP_MIN_DL_24) {
			pad_len = (uint8)(((uint8)(current_length - 1u) & 0xfcu) + 0x04u);
		} else if (current_length <= CANTP_MIN_DL_64) {
			pad_len = (uint8)(((uint8)(current_length - 1u) & 0xf0u) + 0x10u);
		} else {
			/* do nothing */
		}
	}
#endif

	return pad_len;
}

FUNC(cantp_frame_type, CANTP_CODE)
cantp_get_frame_type(const cantp_addr_format_type *format_type, const PduInfoType *cantp_rx_pdu_ptr)
{
	uint8 tpci_offset = 0u;
	uint8 tpci = 0u;
	cantp_frame_type type = CANTP_INVALID_FRAME;

#if (CANTP_EXTENDED_ADDR_RX_CFG == STD_ON)
	if ((CANTP_STANDARD_ADDR != *format_type) && (CANTP_EXTENDED_ADDR != *format_type)) {
#else
	if (CANTP_STANDARD_ADDR != *format_type) {
#endif
		return CANTP_INVALID_FRAME;
	}

#if (CANTP_EXTENDED_ADDR_RX_CFG == STD_ON)
	if (CANTP_EXTENDED_ADDR == *format_type) {
		tpci_offset = 1;
	}
#endif
	tpci = (cantp_rx_pdu_ptr->SduDataPtr[tpci_offset]) & CANTP_ISO15765_TPCI_MASK;

	if (CANTP_ISO15765_TPCI_SF == tpci) {
		type = CANTP_SINGLE_FRAME;
	} else if (CANTP_ISO15765_TPCI_FF == tpci) {
		type = CANTP_FIRST_FRAME;
	} else if (CANTP_ISO15765_TPCI_CF == tpci) {
		type = CANTP_CONSECUTIVE_FRAME;
	} else if (CANTP_ISO15765_TPCI_FC == tpci) {
		type = CANTP_FLOW_CONTROL_FRAME;
	} else {
		type = CANTP_INVALID_FRAME;
	}

	return type;
}

/* 通过入参rx_pdu_id和静态配置中的pdu_list确定tp_index（pdu_list的index） */
FUNC(PduIdType, CANTP_CODE)
cantp_get_pdu_index_from_ipdu_list(PduIdType cantp_rx_pdu_id, const PduInfoType *cantp_rx_pdu_ptr, uint8 core_id)
{
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const cantp_rx_id_type *cantp_ipdu_list =
		cantp_cfg->cantp_ipdu_channel_info[core_id].ipdu_cfg_addr;
	PduIdType cantp_rx_pdu_index = cantp_get_pdu_index(core_id, cantp_rx_pdu_id);
	PduIdType pdu_index = CANTP_INVALID_PDU_ID;
#if (CANTP_EXTENDED_ADDR_RX_CFG != STD_ON)
	(void)cantp_rx_pdu_ptr;
#endif
	do {
		if (cantp_rx_pdu_id != cantp_ipdu_list[cantp_rx_pdu_index].cantp_pdu_id) {
			break; /* 当能在静态配置表中cantp_rx_pdu_id位置对应的rx_pdu_id不正确 */
		}
#if (CANTP_EXTENDED_ADDR_RX_CFG == STD_ON)
		const cantp_nsdu_type *cantp_nsdu_list =
			cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;
		if (CANTP_EXTENDED_ADDR ==
		    cantp_ipdu_list[cantp_rx_pdu_index].cantp_addressing_mode) { /* 如果是扩展寻址 */
			const cantp_rx_nsdu_type *rx_config;
			const cantp_tx_nsdu_type *tx_config;

			if (CANTP_FLOW_CONTROL_FRAME ==
			    cantp_get_frame_type(&cantp_ipdu_list[cantp_rx_pdu_index].cantp_addressing_mode,
						 cantp_rx_pdu_ptr)) {
				if (CANTP_INVALID_PDU_ID ==
				    cantp_ipdu_list[cantp_rx_pdu_index].cantp_referring_tx_index) {
					break;
				}
				tx_config =
					&cantp_nsdu_list[cantp_ipdu_list[cantp_rx_pdu_index].cantp_referring_tx_index]
						 .config_data.cantp_tx_nsdu;
				/* nsa参数必须符合 */
				if (cantp_rx_pdu_ptr->SduDataPtr[0] == tx_config->cantp_nsa) {
					pdu_index = (PduIdType)cantp_rx_pdu_index;
				}
			} else {
				if (CANTP_INVALID_PDU_ID == cantp_rx_pdu_index) {
					break;
				}
				if (CANTP_ISO15765_RECEIVE != cantp_nsdu_list[cantp_rx_pdu_index].direction) {
					break;
				}
				rx_config = &cantp_nsdu_list[cantp_rx_pdu_index].config_data.cantp_rx_nsdu;
				if (cantp_rx_pdu_ptr->SduDataPtr[0] == rx_config->cantp_nta) {
					pdu_index = (PduIdType)cantp_rx_pdu_index;
				}
			}
		} else { // 非扩展寻址 - 正常寻址
			pdu_index = (PduIdType)cantp_rx_pdu_index;
		}
#else
		{ // 非扩展寻址 - 正常寻址
			pdu_index = (PduIdType)cantp_rx_pdu_index;
		}
#endif /* CANTP_EXTENDED_ADDR_RX_CFG */
	} while (FALSE);
	return pdu_index;
}

FUNC(Std_ReturnType, CANTP_CODE)
cantp_check_rx_ind_valid(PduIdType cantp_rx_pdu_id, const PduInfoType *cantp_rx_pdu_ptr, uint8 core_id)
{
	const cantp_runtime_data_type *core_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;

	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return E_NOT_OK;
	}
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();

	CANTP_VALIDATE(*(core_runtime->state) == CANTP_ON,
		 CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_UNINIT);
	CANTP_VALIDATE(cantp_rx_pdu_id < cantp_cfg->cantp_ipdu_channel_info[core_id].number_of_pdus,
		 CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_INVALID_RX_ID);
	CANTP_VALIDATE(NULL_PTR != cantp_rx_pdu_ptr, CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_PARAM_POINTER);
	CANTP_VALIDATE(NULL_PTR != cantp_rx_pdu_ptr->SduDataPtr,
		       CANTP_SERVICE_ID_CANTP_RX_INDICATION_CANTP_E_PARAM_POINTER);

	return E_OK;
}

FUNC(Std_ReturnType, CANTP_CODE)
cantp_tx_frame(cantp_frame_type frame_type, const cantp_tx_nsdu_type *tx_config, cantp_channel_type *tx_runtime)
{
	PduInfoType pdu_info;
	RetryInfoType retry_info;
	Std_ReturnType ret = E_OK;
	uint8 sdu_data[CANTP_SDU_DATA_LENGTH] = { 0 };

	pdu_info.SduDataPtr = &sdu_data[0];

	construct_frame_header(frame_type, tx_config, tx_runtime, &pdu_info);
	retry_info.TpDataState = TP_DATACONF;
	retry_info.TxTpDataCnt = tx_runtime->transfer_count;
	tx_runtime->iso15765.state = CANTP_TX_WAIT_TRANSMIT; /* 通道占用 */

#if (CANTP_SYNC_TRANSMIT == STD_ON)
	BufReq_ReturnType buf_ret = send_next_tx_frame(tx_config, tx_runtime, &pdu_info, &retry_info);

	if ((BUFREQ_OK != buf_ret) && (BUFREQ_E_BUSY != buf_ret)) {
		ret = E_NOT_OK;
		reset_channel_runtime(tx_runtime, CANTP_ISO15765_TRANSMIT);
#if (CONFIG_MODULE_PDUR == 1)
		pdur_cantp_tx_confirmation(tx_config->pdur_pdu_id, NTFRSLT_E_NOT_OK);
#endif
	}
#endif

	return ret;
}

FUNC(cantp_frame_type, CANTP_CODE)
cantp_calc_tx_frame_type(const cantp_tx_nsdu_type *tx_config, PduLengthType SduLength)
{
	/* 通过寻址方式确定最长载荷长度 */
	/* 如果报文内容长于载荷，则为首帧，否则为单帧max_payload > transfer_total */
	uint8 max_payload = 0;
	cantp_frame_type ret = CANTP_INVALID_FRAME;
	uint8 addr_offset = get_addr_offset(tx_config->cantp_addressing_mode);

	if (tx_config->cantp_npdu_len <= CANTP_CAN20_FL) {
		max_payload = CANTP_CAN20_FL - addr_offset - CANTP_PCI_LENGTH_SF;
	} else {
		max_payload = (uint8)(tx_config->cantp_npdu_len - addr_offset - CANTP_PCI_LENGTH_LSF);
	}

	if (SduLength <= max_payload) {
		ret = CANTP_SINGLE_FRAME;
	} else if (CANTP_PHYSICAL == tx_config->cantp_tx_ta_type) {
		ret = CANTP_FIRST_FRAME;
	} else {
		CANTP_DET_REPORTERROR(CANTP_SERVICE_ID_CANTP_TRANSMIT_CANTP_E_INVALID_TATYPE);
		/* 功能寻址不支持多帧 */
	}

	return ret;
}

FUNC(void, CANTP_CODE) cantp_rx_sdu_state_machine(const cantp_rx_nsdu_type *rx_cfg, cantp_channel_type *rx_runtime)
{
	switch (rx_runtime->iso15765.state) {
		case CANTP_RX_WAIT_CF:
			if (CANTP_PHYSICAL == rx_cfg->cantp_rx_ta_type) {
				COUNT_DECREMENT(rx_runtime->iso15765.state_timeout_count);
				if (0UL == rx_runtime->iso15765.state_timeout_count) {
#if (CONFIG_MODULE_PDUR == 1)
					pdur_cantp_rx_indication(rx_cfg->pdur_pdu_id,
								 NTFRSLT_E_TIMEOUT_CR);
#endif
					// (void)VCOS_LOG(CANTP_MODULE_ID, VCOS_LOG_INFO, CANTP_DLT_CHECK_ID15);
					reset_channel_runtime(rx_runtime, CANTP_ISO15765_RECEIVE);
				}
			}
			break;
		default:
			break;
	}
}

FUNC(void, CANTP_CODE) cantp_tx_sdu_state_machine(const cantp_tx_nsdu_type *tx_cfg, cantp_channel_type *tx_runtime)
{
	RetryInfoType retry_info;

	switch (tx_runtime->iso15765.state) {
		case CANTP_TX_WAIT_STMIN:
			COUNT_DECREMENT(tx_runtime->iso15765.state_timeout_count);
			if (0UL != tx_runtime->iso15765.state_timeout_count) {
				break;
			}
			tx_runtime->iso15765.state = CANTP_TX_WAIT_TRANSMIT;
			tx_runtime->iso15765.state_timeout_count = (tx_cfg->cantp_ncs) + 1U;
			/* stmin已满足,无break,继续处理 */
		case CANTP_TX_WAIT_TRANSMIT:
			retry_info.TpDataState = TP_DATACONF;
			retry_info.TxTpDataCnt = tx_runtime->transfer_count;
			tx_sdu_state_machine_transmit_tx_buffer(tx_cfg, tx_runtime, &retry_info);
			break;
		case CANTP_TX_WAIT_COPY_TX_BUSY:
			tx_sdu_state_machine_timeout_check(tx_runtime, tx_cfg->pdur_pdu_id,
							   NTFRSLT_E_NOT_OK); /* 检查n_cs是否超时 */
			retry_info.TpDataState = TP_DATARETRY;
			retry_info.TxTpDataCnt = tx_runtime->transfer_count;
			tx_sdu_state_machine_transmit_tx_buffer(tx_cfg, tx_runtime, &retry_info);
			break;
		case CANTP_TX_WAIT_FC:
			tx_sdu_state_machine_timeout_check(tx_runtime, tx_cfg->pdur_pdu_id,
							   NTFRSLT_E_TIMEOUT_BS); /* 检查n_bs是否超时 */
			break;
		case CANTP_TX_WAIT_CONFIRMATION:
			tx_sdu_state_machine_timeout_check(tx_runtime, tx_cfg->pdur_pdu_id,
							   NTFRSLT_E_TIMEOUT_A); /* 检查n_as是否超时 */
			break;
		default:
			/* do nothing */
			break;
	}
}

FUNC(void, CANTP_CODE) cantp_init_params(uint8 core_id)
{
	const cantp_rx_nsdu_type *rx_config_params = NULL;
	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();
	const uint32 sdu_num = cantp_cfg->cantp_nsdu_channel_info[core_id].number_of_sdus;
	const cantp_nsdu_type *cantp_nsdu_config = cantp_cfg->cantp_nsdu_channel_info[core_id].nsdu_cfg_addr;
	const cantp_runtime_data_type *core_runtime = cantp_get_core_runtime_data(core_id);
	cantp_channel_table_type *channel_table = core_runtime->channel_table_addr;

	if (NULL == channel_table) {
		return;
	}

	for (uint8 i = 0; i < sdu_num; i++) {
		if (CANTP_ISO15765_RECEIVE == cantp_nsdu_config[i].direction) {
			rx_config_params = &cantp_nsdu_config[i].config_data.cantp_rx_nsdu;
			channel_table[rx_config_params->cantp_rx_channel]
				.channel_list[CANTP_RX_CHANNEL]
				.iso15765.config_stmin = (uint8)rx_config_params->cantp_st_min;
			channel_table[rx_config_params->cantp_rx_channel]
				.channel_list[CANTP_RX_CHANNEL]
				.iso15765.config_bs = rx_config_params->cantp_bs;
		}
	}
}

FUNC(void, CANTP_CODE) cantp_init_runtime_data(cantp_channel_type *runtime_params, cantp_direction_type dir)
{
	runtime_params->transfer_total = 0;
	runtime_params->transfer_count = 0;
	runtime_params->size_buffer = 0;
	runtime_params->iso15765.nas_nar_pending = FALSE;
	runtime_params->iso15765.frames_handled_count = 0;
	runtime_params->iso15765.next_flow_control_count = 0;

	reset_channel_runtime(runtime_params, dir);

	if (CANTP_ISO15765_RECEIVE == dir) {
		runtime_params->cantp_wft_max_counter = 0;
	}
}

FUNC(void, CANTP_CODE) cantp_internal_main_function(void)
{
	PduIdType sdu_rx_id = 0u;
	PduIdType sdu_tx_id = 0u;
	cantp_channel_type *rx_runtime = NULL;
	cantp_channel_type *tx_runtime = NULL;
	const cantp_rx_nsdu_type *rx_cfg = NULL;
	const cantp_tx_nsdu_type *tx_cfg = NULL;
	const cantp_runtime_data_type *core_runtime = NULL;
	cantp_channel_table_type *channel_table = NULL;
	uint8 core_id = cantp_get_core_id();

#if (CANTP_MULTICORE_CORE_NUMBER > 1u)
	CANTP_VALIDATE_NO_RV(core_id < CANTP_MULTICORE_CORE_NUMBER,
			     CANTP_SERVICE_ID_CANTP_MAIN_FUNCTION_CANTP_E_GET_COREID);
#endif
	core_runtime = cantp_get_core_runtime_data(core_id);
	channel_table = core_runtime->channel_table_addr;
	if (NULL == channel_table) {
		return;
	}

	CANTP_VALIDATE_NO_RV(*(core_runtime->state) == CANTP_ON, CANTP_SERVICE_ID_CANTP_MAIN_FUNCTION_CANTP_E_UNINIT);

	const uint32 cantp_cur_core_max_channel = core_runtime->channel_table_len;

	const cantp_config_type *cantp_cfg = cantp_get_config_ptr();

	for (uint32 i = 0; i < (uint32)cantp_cur_core_max_channel; i++) {
		sdu_tx_id = channel_table[i].channel_list[CANTP_TX_CHANNEL].sdu_id;
		sdu_rx_id = channel_table[i].channel_list[CANTP_RX_CHANNEL].sdu_id;

		if (CANTP_INVALID_PDU_ID != sdu_tx_id) {
			tx_cfg = (const cantp_tx_nsdu_type *)&cantp_cfg->cantp_nsdu_channel_info[core_id]
					.nsdu_cfg_addr[sdu_tx_id]
					.config_data.cantp_tx_nsdu;
			tx_runtime = &channel_table[i].channel_list[CANTP_TX_CHANNEL];
			cantp_tx_sdu_state_machine(tx_cfg, tx_runtime);
		}

		if (CANTP_INVALID_PDU_ID != sdu_rx_id) {
			rx_cfg = (const cantp_rx_nsdu_type *)&cantp_cfg->cantp_nsdu_channel_info[core_id]
					.nsdu_cfg_addr[sdu_rx_id]
					.config_data.cantp_rx_nsdu;
			rx_runtime = &channel_table[i].channel_list[CANTP_RX_CHANNEL];
			cantp_rx_sdu_state_machine(rx_cfg, rx_runtime);
		}
	}
	return;
}

#define CANTP_STOP_SEC_CODE
#include "cantp_memmap.h"
