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

#include "ethif.h"
#include "ethif_internal.h"
#include "ethif_cbk.h"
#include "ethif_cfg.h"
#include "Eth_GeneralTypes.h"
#include "stdio.h"

#include "vcos_log.h"
#include "ethif_vcos_log.h"

#define ETHIF_DET_REPORTERROR(_cond, errid, ...)                                                                       \
	do {                                                                                                           \
		if (!(_cond)) {                                                                                        \
			return __VA_ARGS__;                                                                            \
		}                                                                                                      \
	} while (0)

#define ETHIF_DET_REPORTERROR_NO_RV(_cond, errid)                                                                      \
	do {                                                                                                           \
		if (!(_cond)) {                                                                                        \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

/* EthIf module configuration. */
extern const ethif_config_type ethif_config_data;
extern ethif_ethdrv_api_type ethif_ethdrv_api[1];

extern ethif_internal_type ethif_internal;
extern const ethif_config_type *ethif_config_pointer;
extern ethif_stats_type ethif_stats;
extern uint32 ethif_eth_dropcount[ETHIF_ETH_MAX_DROP_COUNTER_NUM];

#define ETHIF_START_SEC_CODE
#include "ethif_memmap.h"
/**
 * Init function for EthIf
 * @param cfg_ptr
 */
FUNC(void, ETHIF_CODE) ethif_init(const ethif_config_type *cfg_ptr)
{
	ETHIF_DET_REPORTERROR_NO_RV((NULL != cfg_ptr), ETHIF_ETHIF_SERVICE_ID_INIT_ETHIF_E_INIT_FAILED);

	uint8 i;

	ethif_config_pointer = cfg_ptr;
	for (i = 0; i < ETHIf_MAX_TXBUFSTOTAL; i++) {
		ethif_internal.ethif_runtime[i] = INVALID_FRAME_TYPE;
	}

	for (i = 0; i < ethif_config_pointer->ethif_ctrl_count; i++) {
		ethif_internal.ethif_ctrlmode[i] = ETH_MODE_ACTIVE;
		ethif_internal.ethif_linkstate[i] = ETHTRCV_LINK_STATE_ACTIVE;
	}

	ethif_internal.init_status = ETHIF_STATE_INIT;
}

/**
 *
 * @param ctrl_idx
 * @param ctrlmode_ptr
 * @return
 */
FUNC(Std_ReturnType, ETHIF_CODE) ethif_get_controller_mode(uint8 ctrl_idx, eth_mode_type *ctrlmode_ptr)
{
	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
			      ETHIF_ETHIF_SERVICE_ID_GET_CTRL_MODE_ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

	ETHIF_DET_REPORTERROR((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
			      ETHIF_ETHIF_SERVICE_ID_GET_CTRL_MODE_ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

	ETHIF_DET_REPORTERROR((NULL != ctrlmode_ptr), ETHIF_ETHIF_SERVICE_ID_GET_CTRL_MODE_ETHIF_E_PARAM_POINTER,
			      E_NOT_OK);

	Std_ReturnType ret;

	if (ctrl_idx < ETHIF_CTRLS_CNT) {
		*ctrlmode_ptr = ethif_internal.ethif_ctrlmode[ctrl_idx];
		ret = E_OK;
	} else {
		ret = E_NOT_OK;
	}

	return ret;
}

/**
 *
 * @param ctrl_idx
 * @param phys_addr_ptr
 */
FUNC(void, ETHIF_CODE) ethif_set_phys_addr(uint8 ctrl_idx, const uint8 *phys_addr_ptr)
{
	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_SET_PHY_ADDR_ETHIF_E_NOT_INITIALIZED);

	ETHIF_DET_REPORTERROR_NO_RV((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
				    ETHIF_ETHIF_SERVICE_ID_SET_PHY_ADDR_ETHIF_E_INV_CTRL_IDX);

	ETHIF_DET_REPORTERROR_NO_RV((NULL != phys_addr_ptr), ETHIF_ETHIF_SERVICE_ID_SET_PHY_ADDR_ETHIF_E_PARAM_POINTER);

	uint8 eth_drv_api_idx = 0;

	eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
	eth_set_phys_addr(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id, phys_addr_ptr);
}

/**
 *
 * @param ctrl_idx
 * @param phys_addr_ptr
 */
FUNC(void, ETHIF_CODE) ethif_get_phys_addr(uint8 ctrl_idx, uint8 *phys_addr_ptr)
{
	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_GET_PHY_ADDR_ETHIF_E_NOT_INITIALIZED);

	ETHIF_DET_REPORTERROR_NO_RV((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
				    ETHIF_ETHIF_SERVICE_ID_GET_PHY_ADDR_ETHIF_E_INV_CTRL_IDX);

	ETHIF_DET_REPORTERROR_NO_RV((NULL != phys_addr_ptr), ETHIF_ETHIF_SERVICE_ID_GET_PHY_ADDR_ETHIF_E_PARAM_POINTER);

	uint8 eth_drv_api_idx = 0;

	eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
	eth_get_phys_addr(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id, phys_addr_ptr);
}

FUNC(Std_ReturnType, ETHIF_CODE) ethif_update_phys_addr_filter(uint8 ctrl_idx,
	const uint8 *phys_addr_ptr, eth_filter_action_type action)
{
	Std_ReturnType ret;

	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
			      ETHIF_ETHIF_SERVICE_ID_GET_PHY_ADDR_ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

	ETHIF_DET_REPORTERROR((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
			      ETHIF_ETHIF_SERVICE_ID_GET_PHY_ADDR_ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

	ETHIF_DET_REPORTERROR((NULL != phys_addr_ptr), ETHIF_ETHIF_SERVICE_ID_GET_PHY_ADDR_ETHIF_E_PARAM_POINTER,
			      E_NOT_OK);

#if (ETHIF_PHYS_ADRS_FILTER_API == STD_ON)
	uint8 eth_drv_api_idx = 0;

	eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
	ret = eth_update_phys_addr_filter(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id,
						     phys_addr_ptr, action);
#else
	(void)action;
	ret = E_NOT_OK;
#endif
	return ret;
}

#if defined(ETHMCAL_AUTOSAR_VERSION_422)
FUNC(void, ETHIF_CODE) ethif_txconfirmation(uint8 ctrl_idx, eth_buf_idx_type buf_idx)
#else
FUNC(void, ETHIF_CODE) ethif_txconfirmation(uint8 ctrl_idx, eth_buf_idx_type buf_idx, Std_ReturnType Result)
#endif
{
	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_TX_CONFIRMATION_ETHIF_E_NOT_INITIALIZED);

	ETHIF_DET_REPORTERROR_NO_RV((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
				    ETHIF_ETHIF_SERVICE_ID_TX_CONFIRMATION_ETHIF_E_INV_CTRL_IDX);

#if (ETHIF_GET_STATS_INFO == STD_ON)
	ethif_stats.txcfm++;
#endif
	uint8 i = 0;
	uint8 txhandle;

	if ((NULL != ethif_config_pointer->ethif_ul_txconfirmation) &&
	    (INVALID_FRAME_TYPE != ethif_internal.ethif_runtime[buf_idx])) {
		while (i < ethif_config_pointer->ethif_owners_count) {
			if (ethif_internal.ethif_runtime[buf_idx] ==
			    ethif_config_pointer->ethif_owner_cfg[i].ethif_frame_type) {
				txhandle = ethif_config_pointer->ethif_owner_cfg[i].ethif_txconfirmation_handle;

				if (INVALID_ETHIF_HANDLE != txhandle) {
					ethif_config_pointer->ethif_ul_txconfirmation[txhandle](ctrl_idx, buf_idx);

					ethif_internal.ethif_runtime[buf_idx] = INVALID_FRAME_TYPE;

#if (ETHIF_GET_STATS_INFO == STD_ON)
					if (ETH_FRAME_TYPE_IPV4 ==
					    ethif_config_pointer->ethif_owner_cfg[i].ethif_frame_type) {
						ethif_stats.txcfm_ok[0]++;
					} else if (ETH_FRAME_TYPE_ARP ==
						   ethif_config_pointer->ethif_owner_cfg[i].ethif_frame_type) {
						ethif_stats.txcfm_ok[1]++;
					} else if (ETH_FRAME_TYPE_PTP ==
						   ethif_config_pointer->ethif_owner_cfg[i].ethif_frame_type) {
						ethif_stats.txcfm_ok[2]++;
					} else {
						;
					}
#endif
				}
				break;
			}
			i++;
		}
	}
}

/**
 *
 * @param ctrl_idx
 * @param ctrlmode
 */
FUNC(void, ETHIF_CODE) ethif_ctrlmode_indication(uint8 ctrl_idx, eth_mode_type ctrlmode)
{
	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_CTRL_MODE_INDICATION_ETHIF_E_NOT_INITIALIZED);

	ETHIF_DET_REPORTERROR_NO_RV((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
				    ETHIF_ETHIF_SERVICE_ID_CTRL_MODE_INDICATION_ETHIF_E_INV_CTRL_IDX);
}

#if (ETHIF_ENABLE_RX_INTERRUPT == STD_OFF)
FUNC(void, ETHIF_CODE) ethif_mainfunction_rx(void)
{
	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_MAIN_FUNCTION_RX_ETHIF_E_NOT_INITIALIZED);

	eth_rx_status_type rx_status_ptr;

	rx_status_ptr = ETH_NOT_RECEIVED;
	uint8 i = 0;
	uint8 ctrl_idx = 0;
	uint8 eth_drv_api_idx = 0;
	uint8 fifo_idx = 0;

	while (ctrl_idx < ETHIF_CTRLS_CNT) {
		ctrl_idx++;
		eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
		/*Attention： this function uses 3 parameters */
		eth_receive(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id, fifo_idx,
					  &rx_status_ptr);
		i = 0;
		while (i < ETHIF_RX_INDICATION_ITERATIONS) {
			if (rx_status_ptr == ETH_RECEIVED_MORE_DATA_AVAILABLE) {
				/*Attention： this function uses 3 parameters */
				eth_receive(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id,
							  fifo_idx, &rx_status_ptr);
			} else {
				break;
			}
			i++;
		}
	}
}
#endif

#if (ETHIF_ENABLE_TX_INTERRUPT == STD_OFF)
/**
 *
 */
FUNC(void, ETHIF_CODE) ethif_mainfunction_tx(void)
{
	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_MAIN_FUNCTION_TX_ETHIF_E_NOT_INITIALIZED);

	uint8 ctrl_idx;

	for (ctrl_idx = 0; ctrl_idx < ETHIF_CTRLS_CNT; ctrl_idx++) {
		uint8 eth_drv_api_idx = 0;

		eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
		eth_txconfirmation(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id);
	}
}
#endif

#if (ETHIF_ENABLE_MAIN_FUNCTION_STATE == STD_ON)
FUNC(void, ETHIF_CODE) ethif_mainfunction_state(void)
{
#ifdef ETHIF_TRCVLINK_SUPPORT
	if (ETHIF_STATE_INIT != ethif_internal.init_status) {
		(void)VCOS_LOG(ETHIF_MODULE_ID, VCOS_LOG_INFO, ETHIF_DLT_CHECK_ID5);
		return;
	}

#if (ETHIF_TRCV_SUPPORT == STD_ON)
	(void)ethif_check_linkstate_chg();
#endif
#if (ETHIF_GET_COUNTER_VALUES_ENABLE == STD_ON)
	/*check Crc error start*/
	/*Only one ethmcal controller*/
	uint8 eth_ctrl_idx = 0;
	Std_ReturnType ret = E_NOT_OK;
	const uint8 crc_error_pos = 1;
	(void)memset(ethif_eth_dropcount, 0, sizeof(ethif_eth_dropcount));
#if defined(ETHMCAL_AUTOSAR_VERSION_422)
	uint8 count_values = ETHIF_ETH_MAX_DROP_COUNTER_NUM;

	ret = ETHIF_GET_COUNTER_VALUES_API(eth_ctrl_idx, count_values, ethif_eth_dropcount);
#else
	eth_counter_type counter_ptr;

	ret = ETHIF_GET_COUNTER_VALUES_API(eth_ctrl_idx, &counter_ptr);
	if (E_OK == ret) {
		ethif_eth_dropcount[0] = counter_ptr.drop_pkt_buf_over_run;
		ethif_eth_dropcount[1] = counter_ptr.drop_pkt_crc;
		ethif_eth_dropcount[2] = counter_ptr.undersize_pkt;
		ethif_eth_dropcount[3] = counter_ptr.oversize_pkt;
		ethif_eth_dropcount[4] = counter_ptr.algnmt_err;
		ethif_eth_dropcount[5] = counter_ptr.sqe_test_err;
		ethif_eth_dropcount[6] = counter_ptr.disc_inbd_pkt;
		ethif_eth_dropcount[7] = counter_ptr.err_inbd_pkt;
		ethif_eth_dropcount[8] = counter_ptr.disc_otbd_pkt;
		ethif_eth_dropcount[9] = counter_ptr.err_otbd_pkt;
		ethif_eth_dropcount[10] = counter_ptr.sngl_coll_pkt;
		ethif_eth_dropcount[11] = counter_ptr.mult_coll_pkt;
		ethif_eth_dropcount[12] = counter_ptr.dfrd_pkt;
		ethif_eth_dropcount[13] = counter_ptr.lat_coll_pkt;
	}
#endif
	if (E_OK == ret) {
		/* Dropped packets due to CRC errors */
		if (ethif_eth_dropcount[crc_error_pos] > ethif_stats.crcerr) {
			ethif_stats.crcerr = ethif_eth_dropcount[crc_error_pos];
			(void)VCOS_LOG(ETHIF_MODULE_ID, VCOS_LOG_ERROR, ETHIF_DLT_CHECK_ID3, ethif_stats.crcerr);
		}
	}
	/*check Crc error end*/
#endif
	return;
#endif
}

FUNC(void, ETHIF_CODE) EthIf_MainFunctionState(void)
{
	ethif_mainfunction_state();
}
#endif

FUNC(BufReq_ReturnType, ETHIF_CODE) ethif_provide_tx_buffer(uint8 ctrl_idx,
	eth_frame_type frame_type, uint8 priority, eth_buf_idx_type *bufidx_ptr,
	eth_data_type **buf_ptr, uint16 *lenbyte_ptr)
{
	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
			      ETHIF_ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER_ETHIF_E_NOT_INITIALIZED, BUFREQ_E_NOT_OK);

	ETHIF_DET_REPORTERROR((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
			      ETHIF_ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER_ETHIF_E_INV_CTRL_IDX, BUFREQ_E_NOT_OK);

	ETHIF_DET_REPORTERROR((NULL != bufidx_ptr), ETHIF_ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER_ETHIF_E_PARAM_POINTER,
			      BUFREQ_E_NOT_OK);

	ETHIF_DET_REPORTERROR((NULL != buf_ptr), ETHIF_ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER_ETHIF_E_PARAM_POINTER,
			      BUFREQ_E_NOT_OK);

	ETHIF_DET_REPORTERROR((NULL != lenbyte_ptr), ETHIF_ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER_ETHIF_E_PARAM_POINTER,
			      BUFREQ_E_NOT_OK);

	BufReq_ReturnType ret = BUFREQ_E_NOT_OK;
	eth_data_type *intrnl_buf;
	(void)frame_type;
	uint8 eth_drv_api_idx;

	if ((ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_ctrl_mtu >= *lenbyte_ptr) &&
	    (ethif_internal.ethif_ctrlmode[ctrl_idx] == ETH_MODE_ACTIVE)) {
		eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
		if (INVALID_VLAN_ID != ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_vlan_id) {
			if (priority < PRIORITY_SIZE) {
				*lenbyte_ptr = (*lenbyte_ptr + VLAN_TAG_SIZE);
		/* Attention: this function uses five parameters */
				ret = eth_provide_txbuffer(eth_drv_api_idx)
				(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id,
						      priority, bufidx_ptr, buf_ptr, lenbyte_ptr);
				intrnl_buf = *buf_ptr;
				if (BUFREQ_OK == ret) {
					intrnl_buf[0] = (uint8)(priority << PCP_SHIFT_BITS_5);
					intrnl_buf[0] &= (uint8)((~((uint8)1) << (PCP_SHIFT_BITS_4)));
					intrnl_buf[0] |=
						(uint8)((ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_vlan_id &
							 VLAN_MASK_NIBBLE) >>
							SHIFT_EIGHT_BITS);
					intrnl_buf[1] = (uint8)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_vlan_id &
							       VLAN_MASK_BYTE);
					intrnl_buf[2] = (uint8)((frame_type >> ETHIF_SHIFT_BYTE1) & ETHIF_BYTE_MASK);
					intrnl_buf[3] = (uint8)(frame_type & ETHIF_BYTE_MASK);
				}

				*buf_ptr = &intrnl_buf[VLAN_TAG_SIZE];
				if ((*lenbyte_ptr) > VLAN_TAG_SIZE) {
					*lenbyte_ptr = (*lenbyte_ptr - VLAN_TAG_SIZE);
				}

			} else {
				ret = BUFREQ_E_NOT_OK;
			}
		} else {
	    /* Attention: this function uses five parameters */
			ret = eth_provide_txbuffer(eth_drv_api_idx)
			(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id, priority,
					      bufidx_ptr, buf_ptr, lenbyte_ptr);
		}
	} else {
		ret = BUFREQ_E_NOT_OK;
	}
	if (BUFREQ_OK != ret) {
		(void)VCOS_LOG(ETHIF_MODULE_ID, VCOS_LOG_INFO, ETHIF_DLT_CHECK_ID0, ret);
	}
	return ret;
}

FUNC(Std_ReturnType, ETHIF_CODE) ethif_transmit(uint8 ctrl_idx, eth_buf_idx_type buf_idx,
	eth_frame_type frame_type, boolean tx_confirmation, uint16 len_byte, const uint8 *phys_addr_ptr)
{
	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
			      ETHIF_ETHIF_SERVICE_ID_TRANSMIT_ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

	ETHIF_DET_REPORTERROR((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
			      ETHIF_ETHIF_SERVICE_ID_TRANSMIT_ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

	ETHIF_DET_REPORTERROR((NULL != phys_addr_ptr), ETHIF_ETHIF_SERVICE_ID_TRANSMIT_ETHIF_E_PARAM_POINTER, E_NOT_OK);

	Std_ReturnType ret;

#if (ETHIF_GET_STATS_INFO == STD_ON)
	ethif_stats.xmit[ctrl_idx]++;
#endif

	if ((ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_ctrl_mtu >= len_byte) &&
	    (ethif_internal.ethif_ctrlmode[ctrl_idx] == ETH_MODE_ACTIVE)) {
		if (NULL != ethif_config_pointer->ethif_ul_txconfirmation) {
			if (TRUE == tx_confirmation) {
				ethif_internal.ethif_runtime[buf_idx] =
					frame_type;

			} else {
				ethif_internal.ethif_runtime[buf_idx] = INVALID_FRAME_TYPE;
			}
		}
		if (INVALID_VLAN_ID != ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_vlan_id) {
			uint8 eth_drv_api_idx = 0;

			eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
			ret = eth_transmit(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id,
							 buf_idx, ETH_FRAME_TYPE_VLAN, tx_confirmation,
							 (len_byte + VLAN_TAG_SIZE), phys_addr_ptr);
		} else {
			uint8 eth_drv_api_idx = 0;

			eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
			ret = eth_transmit(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id,
							 buf_idx, frame_type, tx_confirmation, len_byte, phys_addr_ptr);
		}
		if (E_OK != ret) {
			ethif_internal.ethif_runtime[buf_idx] = INVALID_FRAME_TYPE;
		}

	} else {
		ret = E_NOT_OK;
	}
	if (E_OK != ret) {
		(void)VCOS_LOG(ETHIF_MODULE_ID, VCOS_LOG_ERROR, ETHIF_DLT_CHECK_ID1);
	} else {
#if (ETHIF_GET_STATS_INFO == STD_ON)
		ethif_stats.xmit_ok[ctrl_idx]++;
#endif
	}

	return ret;
}

/**
 * @param eth_ctrl_idx  Ethernet controller index
 * @param vlan_id       VLAN ID that shall be used in case no VLAN is used ETHIF_INV_VLAN_ID
 */
FUNC(boolean, ETHIF_CODE) ethif_is_vid_corrected(uint8 eth_ctrl_idx, uint16 vlan_id)
{
	(void)vlan_id;
	(void)eth_ctrl_idx;
	uint8 ethif_ctrlidx = 0;
	boolean ret = (boolean)E_NOT_OK;

	for (ethif_ctrlidx = 0; ethif_ctrlidx < ethif_config_pointer->ethif_ctrl_count; ethif_ctrlidx++) {
		if (ethif_config_pointer->ethif_ctrl_cfg[ethif_ctrlidx].ethif_vlan_id == vlan_id) {
			ret = (boolean)E_OK;
			return ret;
		}
	}
	return ret;
}

FUNC(uint8, ETHIF_CODE) ethif_match_vid(uint8 eth_ctrl_idx, uint16 vlan_id)
{
	(void)vlan_id;
	(void)eth_ctrl_idx;
	uint8 ethif_ctrlidx = 0;

	for (ethif_ctrlidx = 0; ethif_ctrlidx < ethif_config_pointer->ethif_ctrl_count; ethif_ctrlidx++) {
		if (ethif_config_pointer->ethif_ctrl_cfg[ethif_ctrlidx].ethif_vlan_id == vlan_id) {
			return ethif_config_pointer->ethif_ctrl_cfg[ethif_ctrlidx].ethif_ctrl_id;
		}
	}
	return ETHIF_INVALID_CTRLIDX;
}

/**
 * @param ctrl_idx
 * @param frame_type
 * @param is_broadcast
 * @param phys_addr_ptr
 * @param data_ptr
 * @param len_byte
 */
FUNC(void, ETHIF_CODE) ethif_rxindication(uint8 ctrl_idx, eth_frame_type frame_type,
	boolean is_broadcast, const uint8 *phys_addr_ptr, eth_data_type *data_ptr, uint16 len_byte)
{

	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_RX_INDICATION_ETHIF_E_NOT_INITIALIZED);

	ETHIF_DET_REPORTERROR_NO_RV((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
				    ETHIF_ETHIF_SERVICE_ID_RX_INDICATION_ETHIF_E_INV_CTRL_IDX);

	ETHIF_DET_REPORTERROR_NO_RV((NULL != data_ptr), ETHIF_ETHIF_SERVICE_ID_RX_INDICATION_ETHIF_E_PARAM_POINTER);

#if (ETHIF_GET_STATS_INFO == STD_ON)
	ethif_stats.recv++;
#endif
	uint8 i;
	uint8 handle;
	eth_frame_type adapt_frametype;

	adapt_frametype = 0u;
	eth_data_type *adapt_dataptr;

	adapt_dataptr = NULL;
	uint16 adapt_lenbyte;

	adapt_lenbyte = 0u;
	boolean status;

	status = TRUE;
	uint8 ethif_ctrlidx = 0;
	uint16 vlan_id = INVALID_VLAN_ID;

#if (ETHIF_GET_STATS_INFO == STD_ON)
	eth_data_type *ul_data_ptr = NULL;
	const uint8 ip_type_pos = 9u;
	const uint8 ip_headlength_pos = 0u;
	const uint8 ip_icmp_type = 0x01u;
	uint8 ip_headlength_min = 5u;
	uint8 ip_headlength = 0;

	if (frame_type == ETH_FRAME_TYPE_VLAN) {
		ul_data_ptr = (eth_data_type *)&data_ptr[VLAN_TAG_SIZE];
	} else {
		ul_data_ptr = (eth_data_type *)data_ptr;
	}
	ip_headlength = ul_data_ptr[ip_headlength_pos] & 0X0Fu;
	/* check ip headerlength */
	if ((ip_headlength >= ip_headlength_min) && (ul_data_ptr[ip_type_pos] == ip_icmp_type)) {
	}
#endif

	if (frame_type == ETH_FRAME_TYPE_VLAN) {
		uint16 tCI = 0u;

		tCI |= (uint16)data_ptr[0] << ETHIF_SHIFT_BYTE1; /*lint !e9033 no harm in shifting */
		tCI |= (uint16)data_ptr[1]; /*lint !e9033 no harm in taking the byte pointer*/
		vlan_id = tCI & ETHIF_VID_MASK;

		ethif_ctrlidx = ethif_match_vid(ctrl_idx, vlan_id);
		if (ETHIF_INVALID_CTRLIDX != ethif_ctrlidx) {
			adapt_frametype = 0u;
			adapt_dataptr = (eth_data_type *)&data_ptr[VLAN_TAG_SIZE];
			adapt_lenbyte = len_byte - VLAN_TAG_SIZE;
			adapt_frametype |= (eth_frame_type)data_ptr[2]
					  << ETHIF_SHIFT_BYTE1; /*lint !e9033 no harm in shifting */
			adapt_frametype |= (eth_frame_type)data_ptr[3]; /*lint !e9033 no harm in taking the byte pointer*/
		} else {
			status = FALSE;
		}
	} else {
		ethif_ctrlidx = ethif_match_vid(ctrl_idx, vlan_id);
		if (ETHIF_INVALID_CTRLIDX != ethif_ctrlidx) {
			adapt_frametype = frame_type;
			adapt_dataptr = (eth_data_type *)data_ptr;
			adapt_lenbyte = len_byte;
		} else {
			status = FALSE;
		}
	}

	if ((status == TRUE) && (ethif_internal.ethif_ctrlmode[ethif_ctrlidx] == ETH_MODE_ACTIVE)) {
		for (i = 0; i < ethif_config_pointer->ethif_owners_count; i++) {
			if (adapt_frametype == ethif_config_pointer->ethif_owner_cfg[i].ethif_frame_type) {
				handle = ethif_config_pointer->ethif_owner_cfg[i].ethif_rxindication_handle;
				if (handle != INVALID_ETHIF_HANDLE) {
					ethif_config_pointer->ethif_ul_rxIndication[handle](ethif_ctrlidx, adapt_frametype,
											 is_broadcast, phys_addr_ptr,
											 adapt_dataptr, adapt_lenbyte);
#if (ETHIF_GET_STATS_INFO == STD_ON)
					ethif_stats.recv_ok[ethif_ctrlidx]++;
#endif
				}
				break;
			}
		}
	}
}

FUNC(void, ETHIF_CODE) ethif_get_controller_config(uint8 ctrl_idx, ethif_controller_cfg *controller_cfg)
{
	ETHIF_DET_REPORTERROR_NO_RV((ETHIF_STATE_INIT == ethif_internal.init_status),
				    ETHIF_ETHIF_SERVICE_ID_GET_PHY_INFO_ETHIF_E_NOT_INITIALIZED);

	ETHIF_DET_REPORTERROR_NO_RV((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
				    ETHIF_ETHIF_SERVICE_ID_GET_PHY_INFO_ETHIF_E_INV_CTRL_IDX);

	ETHIF_DET_REPORTERROR_NO_RV((NULL != controller_cfg), ETHIF_ETHIF_SERVICE_ID_GET_PHY_INFO_ETHIF_E_PARAM_POINTER);

	uint8 eth_drv_api_idx = 0;

	eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
	eth_get_controller_config(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id, controller_cfg);
}

FUNC(Std_ReturnType, ETHIF_CODE) ethif_get_linkstate(uint8 ctrl_idx, eth_trcv_linkstate_type *link_state)
{
	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
			      ETHIF_ETHIF_SERVICE_ID_GET_PHY_LINKSTATE_ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

	ETHIF_DET_REPORTERROR((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
			      ETHIF_ETHIF_SERVICE_ID_GET_PHY_LINKSTATE_ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

	ETHIF_DET_REPORTERROR((NULL != link_state), ETHIF_ETHIF_SERVICE_ID_GET_PHY_LINKSTATE_ETHIF_E_PARAM_POINTER,
			      E_NOT_OK);

	Std_ReturnType ret = E_NOT_OK;
	uint8 link_count = 0;
	uint8 eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
	uint8 ethif_eth_trcv_id = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_trcv_id;
	uint8 ethif_swt_portgroup_id = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_switch_port_group_id;
	uint8 ethif_link_aggr_threshold = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_link_aggr_threshold;

	if (INVALID_TRCV_ID != ethif_eth_trcv_id) {
		eth_trcv_linkstate_type eth_trcv_linkstate = ETHTRCV_LINK_STATE_DOWN;

		ret = trcv_get_linkstate(eth_drv_api_idx)(ethif_eth_trcv_id, &eth_trcv_linkstate);
		if ((E_OK == ret) && (ETHTRCV_LINK_STATE_ACTIVE == eth_trcv_linkstate)) {
			link_count++;
		}
	}
	if (INVALID_SWTGROUP_ID != ethif_swt_portgroup_id) {
		uint8 swt_linkcount = 0;
		ethif_switch_port_group_type group = ethif_config_pointer->ethif_swt_port_group_cfg[ethif_swt_portgroup_id];

		for (uint8 i = 0; i < group.port_count; i++) {
			uint8 swt_port_id = group.ethif_switch_portlist[i].ethif_swt_port_id;
			uint8 swt_portrole = group.ethif_switch_portlist[i].ethif_swt_port_role;
			uint8 swt_id = group.ethif_switch_portlist[i].ethif_swt_id;
			eth_trcv_linkstate_type swt_port_linkstate = ETHTRCV_LINK_STATE_DOWN;

			ret = swt_get_linkstate(eth_drv_api_idx)(swt_id, swt_port_id, &swt_port_linkstate);
			if (E_OK == ret) {
				if ((ETHSWT_HOST_PORT == swt_portrole) || (ETHSWT_UP_LINK_PORT == swt_portrole)) {
					if (ETHTRCV_LINK_STATE_DOWN == swt_port_linkstate) {
						swt_linkcount = 0;
						break;
					}
					swt_linkcount++;
				} else {
					if (ETHTRCV_LINK_STATE_ACTIVE == swt_port_linkstate) {
						swt_linkcount++;
					}
				}
			}
		}
		link_count = link_count + swt_linkcount;
	}
	if (link_count >= ethif_link_aggr_threshold) {
		*link_state = ETHTRCV_LINK_STATE_ACTIVE;
	} else {
		*link_state = ETHTRCV_LINK_STATE_DOWN;
	}
	return E_OK;
}

FUNC(Std_ReturnType, ETHIF_CODE) ethif_ethsm_get_linkstate(uint8 ctrl_idx)
{
	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
			      ETHIF_ETHIF_SERVICE_ID_ETHSM_GET_PHY_LINKSTATE_ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

	ETHIF_DET_REPORTERROR((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
			      ETHIF_ETHIF_SERVICE_ID_ETHSM_GET_PHY_LINKSTATE_ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

	(void)ctrl_idx;
	Std_ReturnType ret = E_NOT_OK;

	if (ethif_internal.trcv_linkstate_check_count > 0u) {
		ethif_internal.trcv_linkstate_check_count -= 1u;
		ret = ethtrcv_update_linkstate();
	} else {
		ret = E_OK;
	}
	return ret;
}

FUNC(Std_ReturnType, ETHIF_CODE) ethif_check_linkstate_chg(void)
{
	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
			      ETHIF_ETHIF_SERVICE_ID_GET_PHY_LINKSTATE_ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

	uint8 ctrl_idx = 0;
	Std_ReturnType ret = E_NOT_OK;
	eth_trcv_linkstate_type linkstate_old;
	uint8 count = 0;

	for (ctrl_idx = 0; ctrl_idx < ethif_config_pointer->ethif_ctrl_count; ctrl_idx++) {
		linkstate_old = ethif_internal.ethif_linkstate[ctrl_idx];
		ret = ethif_get_linkstate(ctrl_idx, &(ethif_internal.ethif_linkstate[ctrl_idx]));
		if (linkstate_old != ethif_internal.ethif_linkstate[ctrl_idx]) {
			for (count = 0; count < ETHIF_ULLINKSTATECHG_CNT; count++) {
				ethif_config_pointer->ethif_ul_linkstate_chg[count](ctrl_idx,
										ethif_internal.ethif_linkstate[ctrl_idx]);
			}
		}
	}
	return ret;
}

#if (ETHIF_GLOBAL_TIME_SUPPORT == STD_ON)

/*!*********************************************************************************************************************
 * @brief          获取当前Mac时钟时间
 * @details        获取索引为ctrl_idx的控制器的当前时间
 * @param[in]      ctrl_idx 控制器索引
 * @param[out]     time_qual_ptr 时间质量指针
 * @param[out]     timestamp_ptr 时间戳指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ETHIF_CODE) ethif_get_current_time(uint8 ctrl_idx,
	eth_timestamp_qual_type* time_qual_ptr, eth_timestamp_type* timestamp_ptr)
{
	ETHIF_DET_REPORTERROR((ETHIF_STATE_INIT == ethif_internal.init_status),
		ETHIF_ETHIF_SERVICE_ID_GET_CURRENT_TIME_ETHIF_E_NOT_INITIALIZED, E_NOT_OK);

	ETHIF_DET_REPORTERROR((ctrl_idx < ethif_config_pointer->ethif_ctrl_count),
		ETHIF_ETHIF_SERVICE_ID_GET_CURRENT_TIME_ETHIF_E_INV_CTRL_IDX, E_NOT_OK);

	ETHIF_DET_REPORTERROR(((NULL != time_qual_ptr) && (NULL != timestamp_ptr)),
		ETHIF_ETHIF_SERVICE_ID_GET_CURRENT_TIME_ETHIF_E_PARAM_POINTER, E_NOT_OK);

	uint8 eth_drv_api_idx = 0;

	Std_ReturnType ret;

	eth_drv_api_idx = ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].eth_drv_api_idx;
	ret = eth_get_currenttime(eth_drv_api_idx)(ethif_config_pointer->ethif_ctrl_cfg[ctrl_idx].ethif_eth_ctrl_id,
		time_qual_ptr, timestamp_ptr);
	return ret;
}

#endif

#define ETHIF_STOP_SEC_CODE
#include "ethif_memmap.h"
