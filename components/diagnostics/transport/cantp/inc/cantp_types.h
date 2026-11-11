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

#ifndef CANTP_TYPES_H
#define CANTP_TYPES_H

#include "comstack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   定义nsdu方向
 **********************************************************************************************************************/
typedef uint8 cantp_direction_type;
#define CANTP_ISO15765_TRANSMIT ((uint8)0u) ///< tx方向
#define CANTP_ISO15765_RECEIVE ((uint8)1u) ///< rx方向

/*!*********************************************************************************************************************
 * @brief   定义寻址方式
 **********************************************************************************************************************/
typedef uint8 cantp_addr_format_type;
#define CANTP_EXTENDED_ADDR ((uint8)0u) ///< 扩展寻址
#define CANTP_STANDARD_ADDR ((uint8)1u) ///< 普通寻址

/*!*********************************************************************************************************************
 * @brief   定义cantp状态
 **********************************************************************************************************************/
typedef uint8 cantp_state_type;
#define CANTP_OFF ((uint8)0u) ///< 关闭
#define CANTP_ON ((uint8)1u) ///< 开启

/*!*********************************************************************************************************************
 * @brief   定义通道状态
 **********************************************************************************************************************/
typedef uint8 cantp_transfer_instance_mode;
#define CANTP_RX_WAIT ((uint8)0u) ///< rx等待中
#define CANTP_RX_PROCESSING ((uint8)1u) ///< rx处理中
#define CANTP_TX_WAIT ((uint8)2u) ///< tx等待中
#define CANTP_TX_PROCESSING ((uint8)3u) ///< tx处理中

/*!*********************************************************************************************************************
 * @brief   定义目的地址个数类型
 **********************************************************************************************************************/
typedef uint8 cantp_nta_type;

/*!*********************************************************************************************************************
 * @brief   定义源地址个数类型
 **********************************************************************************************************************/
typedef uint8 cantp_nsa_type;

/*!*********************************************************************************************************************
 * @brief   定义通信方式
 **********************************************************************************************************************/
typedef uint8 cantp_ta_type;
#define CANTP_FUNCTIONAL ((uint8)0u) ///< 功能寻址
#define CANTP_PHYSICAL ((uint8)1u) ///<  物理寻址

/*!*********************************************************************************************************************
 * @brief   定义tx_nsdu结构体
 **********************************************************************************************************************/
typedef struct {
	const cantp_nsa_type cantp_nsa; ///< 源地址
	const cantp_nta_type cantp_nta; ///< 目的地址
	const cantp_addr_format_type cantp_addressing_mode; ///< 寻址方式
	const cantp_state_type cantp_tx_padding_activation; ///< padding使能
	const cantp_ta_type cantp_tx_ta_type; ///< 通信方式(功能寻址或物理寻址)
	const uint8 cantp_tx_channel; ///< 发送通道
	const uint16 cantp_nas; ///< n_as (要求时长(ms) / main_function 周期)
	const uint16 cantp_nbs; ///< n_bs (要求时长(ms) / main_function 周期)
	const uint16 cantp_ncs; ///< n_cs (要求时长(ms) / main_function 周期)
	const PduIdType canif_pdu_id; ///< canif 报文id
	const PduIdType pdur_pdu_id; ///< pdur 报文id
	const PduLengthType cantp_npdu_len; ///< 发送npdu长度
} cantp_tx_nsdu_type;

/*!*********************************************************************************************************************
 * @brief   定义rx_nsdu结构体类型
 **********************************************************************************************************************/
typedef struct {
	const cantp_nsa_type cantp_nsa; ///< 源地址
	const cantp_nta_type cantp_nta; ///< 目的地址
	const cantp_addr_format_type cantp_addressing_format; ///< 寻址方式
	const cantp_state_type cantp_rx_padding_activation; ///< padding使能
	const cantp_ta_type cantp_rx_ta_type; ///< 通信方式(功能寻址或物理寻址)
	const uint8 cantp_bs; ///< 允许发送的连续帧数量上限。如果bs=0x0，则代表无限制
	const uint8 cantp_rx_channel; ///< 接收通道
	const uint8 cantp_wft_max; ///< 允许接收方连续传递的等待流控帧的数量上限
	const uint16 cantp_nar; ///< n_ar (要求时长(ms) / main_function 周期)
	const uint16 cantp_nbr; ///< n_br (要求时长(ms) / main_function 周期)
	const uint16 cantp_ncr; ///< n_cr (要求时长(ms) / main_function 周期)
	const uint16 cantp_st_min; ///< 多帧传输时连续帧的最小间隔时间
	const PduIdType canif_fc_pdu_id; ///< canif PDU index(发送流控帧)
	const PduIdType pdur_pdu_id; ///< 报文id
	const PduLengthType cantp_npdu_len; ///< rx_npdu长度
} cantp_rx_nsdu_type;

/*!*********************************************************************************************************************
 * @brief   定义每个连接通道的结构体
 **********************************************************************************************************************/
typedef struct {
	const boolean cantp_fd_support; ///< 是否支持can_fd
	const cantp_direction_type direction; ///< 通道方向
	union {
		const cantp_rx_nsdu_type cantp_rx_nsdu; ///< tx nsdu配置
		const cantp_tx_nsdu_type cantp_tx_nsdu; ///< rx nsdu配置
	} config_data;
} cantp_nsdu_type;

/*!*********************************************************************************************************************
 * @brief   定义每个核的nsdu配置类型
 **********************************************************************************************************************/
typedef struct {
	const cantp_nsdu_type *nsdu_cfg_addr; ///< nsdu配置列表指针
	const uint32 channel_cfg_table_len; ///< 通道个数
	const uint32 number_of_sdus;
} cantp_nsdu_config_type;

/*!*********************************************************************************************************************
 * @brief   定义i-pdu列表的结构体
 **********************************************************************************************************************/
typedef struct {
	const cantp_addr_format_type cantp_addressing_mode; ///< 寻址格式
	const PduIdType cantp_nsdu_index; ///< rx通道关联的nsdu index
	const PduIdType cantp_referring_tx_index; ///< rx通道关联的tx的nsdu index
	const PduIdType cantp_pdu_id; ///< cantp的pdu id
} cantp_rx_id_type;

/*!*********************************************************************************************************************
 * @brief   定义每个核的ipdu配置类型
 **********************************************************************************************************************/
typedef struct {
	const cantp_rx_id_type *ipdu_cfg_addr; ///< ipdu配置列表指针
	const uint32 channel_cfg_table_len; ///< 通道个数
	const uint32 number_of_pdus;
} cantp_ipdu_config_type;

/*!*********************************************************************************************************************
 * @brief   定义cantp general结构体
 **********************************************************************************************************************/
typedef struct {
	const uint8 padding_pattern; ///< padding值
	const cantp_state_type padding_activation; ///< padding使能
} cantp_general_type;

/*!*********************************************************************************************************************
 * @brief   定义cantp模块配置总结构体
 **********************************************************************************************************************/
typedef struct {
	const cantp_general_type *cantp_general; ///< general配置
	const cantp_nsdu_config_type *cantp_nsdu_channel_info; ///< 相当于连接通道
	const cantp_ipdu_config_type *cantp_ipdu_channel_info; ///< ipdu列表，使用接收的pdu_id索引
	const uint32 *cantp_mainfunction_period_list; ///< cantp主函数周期列表
} cantp_config_type;

#ifdef __cplusplus
}
#endif

#endif /* CANTP_TYPES_H */
