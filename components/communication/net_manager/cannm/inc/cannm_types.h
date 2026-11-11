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

#ifndef CANNM_TYPES_H
#define CANNM_TYPES_H

#include "comstack_types.h"
#include "cannm_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!*********************************************************************************************************************
 * @brief   Defines the type of the Pdu byte position of CanNm
 **********************************************************************************************************************/
typedef uint8 cannm_byte_pos_cate;
#define CANNM_PDU_BYTE_POSITION_0       ((cannm_byte_pos_cate)0x00u)         ///< Byte 0
#define CANNM_PDU_BYTE_POSITION_1       ((cannm_byte_pos_cate)0x01u)         ///< Byte 1
#define CANNM_PDU_BYTE_POSITION_OFF     ((cannm_byte_pos_cate)0xFFu)         ///< No padding

/*!*********************************************************************************************************************
 * @brief  CanNm core_id
 **********************************************************************************************************************/
typedef uint32 cannm_core_id_type;

/*!*********************************************************************************************************************
 * @brief  CanNm channel length type
 **********************************************************************************************************************/
typedef uint8 cannm_channel_num_type;

/*!*********************************************************************************************************************
 * @brief   NM network handle type definition
 **********************************************************************************************************************/
typedef uint8 network_handle_type;

/*!*********************************************************************************************************************
 * @brief   Defines the type of CanNm core channel ID mapping
 **********************************************************************************************************************/
typedef struct {
	const uint8 *net_tbl_addr;               ///< The address of a core's corresponding resource in the cache
	cannm_channel_num_type network_table_len;      ///< The length of the corresponding resource of a core in cache
} cannm_core_channel_id_mapping_type;

/*!*********************************************************************************************************************
 * @brief   Defines the type of the CanNm channel
 **********************************************************************************************************************/
typedef struct {
	const uint8                     imdt_tx; ///< Send immediately
	const uint8                     nd_id;                   ///< Node ID
	const cannm_byte_pos_cate pos_nid;              ///< Node ID position
	const cannm_byte_pos_cate pos_cbv;              ///< Control bit vector position

	const boolean                   channel_en:1;                     ///< Whether to activate
	const boolean                   pn_support:1;          ///< Is the local network enabled
	const boolean                   all_msg_wk:1; ///< Whether all nm messages can be woken up
	const network_handle_type         id_net;          ///< network channel
	const PduIdType                 canif_pid;               ///< CanIf的Pdu ID
#if ((defined(CANNM_COM_USER_DATA_SUPPORT)) && (CANNM_COM_USER_DATA_SUPPORT == STD_ON))
	const PduIdType                 ur_data_tx_pid;     ///< ID of the Pdu that sends user data
#endif
	const PduLengthType             msg_size;                ///< length of pdu
	const uint16                    m_func_pid;       ///< main function period
	const uint16                    tm_out;              ///< timeout
	const uint16                    rpt_msg_tm;        ///< Repeat message time
	const uint16                    wait_bs_tm;         ///< wait bus sleep time
	const uint16                    msg_cyc_tm;         ///< cyclic message cycle
#if ((defined(CANNM_BUS_LOAD_REDUCTION_ENABLED)) && (CANNM_BUS_LOAD_REDUCTION_ENABLED == STD_ON) && \
	(CANNM_BUS_LOAD_REDUCTION_ACTIVE == STD_ON) && (CANNM_PASSIVE_MODE_ENABLED == STD_OFF))
	const uint16                    msg_rdc_tm;           ///< Bus load reduction cycle
#endif
	const uint16                    msg_cys_ofst_tm;   ///< Cyclic message offset time
	const uint16                    msg_tmot_tm;       ///< Message timeout
	const uint16                    imdt_nm_cyc_tm;     ///< Immediate cycle time
} cannm_chnl_cate;

/*!*********************************************************************************************************************
 * @brief   Define the type of PN information of CanNm
 **********************************************************************************************************************/
typedef struct {
	uint8 pn_if_ofst;                                  ///< Local network information deviation
	const uint8 *pn_if_msk;                             ///< Local network mask
	const uint8 *pn_idx_to_tm_mp;                    ///< Mapping local network index to time
	const uint8 *tm_idx_to_pn_mp;                    ///< Mapping time index to local network index
	PduLengthType pn_if_size;                             ///< Local network information length
	PduIdType eira_rx_pid;                             ///< Received Sdu Id with EIRA
} cannm_pn_if_cate;

/*!*********************************************************************************************************************
 * @brief   Define the configuration type of CanNm
 **********************************************************************************************************************/
typedef struct {
	const network_handle_type *chnl_num;              ///< Number of nm_tunnel
	const network_handle_type *tunnel_trans;                   ///< Number of nm_tunnel after mapping
	const network_handle_type chnl_num_len;          ///< channel size
#if (CANNM_PN_FEATURE_ENABLED == STD_ON)
	const cannm_pn_if_cate  *pn_if;                 ///< local network information
#endif
	const cannm_chnl_cate *nm_tunnel;                       ///< Channel number
} cannm_cate_cfg;

#ifdef __cplusplus
}
#endif

#endif /* CANNM_TYPES_H */
