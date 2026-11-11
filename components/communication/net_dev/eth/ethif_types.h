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
/*!*********************************************************************************************************************
 * @file        ethif_types.h
 * @brief       EthIf类型头文件
 * @details     主要包含EthIf模块相关类型的定义
 * @date        2023-3-17
 * @version     V1.0
 **********************************************************************************************************************/

#ifndef ETHIF_TYPES_H
#define ETHIF_TYPES_H

#include "ethif_cfg.h"
#include "Platform_Types.h"
#include "stdbool.h"
#include "EthTrcv_GeneralTypes.h"
#include "Compiler.h"
#include "Eth_GeneralTypes.h"
#include "comstack_types.h"
#include "lwipopts.h"
#include "Eth_GEthMac.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
* @brief   定义EthIf初始化状态类型
**********************************************************************************************************************/
typedef enum {
	ETHIF_STATE_UNINIT, ///< 未初始化
	ETHIF_STATE_INIT, ///< 已初始化
} ethif_statetype;

/*!*********************************************************************************************************************
* @brief   定义Eth控制器配置类型
**********************************************************************************************************************/
typedef struct {
	uint8 eth_mii_interface; ///< MII接口类型
	uint8 eth_trans_baudrate; ///< 传输波特率
	uint8 eth_phy_role; ///< PHY角色
} ethif_controller_cfg;

/*!*********************************************************************************************************************
* @brief   定义EthIf控制器配置类型
**********************************************************************************************************************/
typedef struct {
	uint8 ethif_ctrl_id; ///< 控制器索引
	uint8 ethif_max_tx_bufs_total; ///< 最大传输缓冲区数量
	uint16 ethif_ctrl_mtu; ///< 控制器MTU
	uint8 ethif_eth_ctrl_id; ///< 关联的Eth控制器索引
	uint8 ethif_eth_trcv_id; ///< 关联的EthTransceiver索引
	uint8 ethif_switch_port_group_id; ///< 关联的SwitchPortGroup索引
	uint16 ethif_vlan_id; ///< VLAN ID
	uint8 ethif_link_aggr_threshold; //< 链路状态阈值
	uint8 eth_drv_api_idx; ///< 驱动API索引
} ethif_controller_type;

/*!*********************************************************************************************************************
* @brief   定义EthIf帧类型及对应的接收函数、发送确认函数回调索引
**********************************************************************************************************************/
typedef struct {
	/* Frame Type */
	uint32 ethif_frame_type; ///< 帧类型
	/* RxIndication handle */
	uint8 ethif_rxindication_handle; ///< 接收函数回调索引
	/* TxConfirmation handle */
	uint8 ethif_txconfirmation_handle; ///< 发送确认函数回调索引
} ethif_frame_owner_type;

#if (ETHIF_SWITCH_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
* @brief   定义EthIf交换机类型
**********************************************************************************************************************/
typedef struct {
	uint8 ethif_eth_switch_id; ///< Eth Switch 索引
	uint8 ethif_switch_idx; ///< EthIf Switch 索引
} ethif_switch_type;
#endif

/*!*********************************************************************************************************************
* @brief    定义链路状态改变时上层回调函数指针类型
**********************************************************************************************************************/
typedef void (*ethif_ul_linkstate_chg_type)(uint8 ctrl_idx, eth_trcv_linkstate_type transceiver_linkstate);

/*!*********************************************************************************************************************
* @brief    定义接收数据时上层回调函数指针类型
**********************************************************************************************************************/
typedef void (*ethif_ul_rxindication_type)(uint8 ctrl_idx, eth_frame_type frame_type, boolean is_broadcast,
					const uint8 *phys_addr_ptr, eth_data_type *data_ptr, uint16 len_byte);

/*!*********************************************************************************************************************
* @brief    定义获取物理地址函数指针类型
**********************************************************************************************************************/
typedef void (*eth_get_phys_addr_ptr)(uint8 ctrl_idx, uint8 *phys_addr_ptr);

/*!*********************************************************************************************************************
* @brief    定义设置物理地址函数指针类型
**********************************************************************************************************************/
typedef void (*eth_set_phys_addr_ptr)(uint8 ctrl_idx, const uint8 *phys_addr_ptr);

/*!*********************************************************************************************************************
* @brief    定义发送函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*eth_transmit_ptr)(uint8 ctrl_idx, eth_buf_idx_type buf_idx, eth_frame_type frame_type,
					boolean tx_confirmation, uint16 len_byte, const uint8 *phys_addr_ptr);

/*!*********************************************************************************************************************
* @brief    定义发送确认函数指针类型
**********************************************************************************************************************/
typedef void (*eth_txconfirmation_ptr)(uint8 ctrl_idx);

/*!*********************************************************************************************************************
* @brief    定义更新物理地址过滤器函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*eth_update_phys_addr_filter_ptr)(uint8 ctrl_idx, const uint8 *phys_addr_ptr,
							eth_filter_action_type action);

/*!*********************************************************************************************************************
* @brief    定义获取驱动配置函数指针类型
**********************************************************************************************************************/
typedef void (*eth_get_controller_config_ptr)(uint8 ctrl_idx, ethif_controller_cfg *ctrl_cfg);

/*!*********************************************************************************************************************
* @brief    定义获取当前时间函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*eth_get_currenttime_ptr)(const uint8 ctrl_idx, eth_timestamp_qual_type *const time_qual_ptr,
						eth_timestamp_type *const timestamp_ptr);

/*!*********************************************************************************************************************
* @brief    定义设置全局时间函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*eth_set_globaltime_ptr)(const uint8 ctrl_idx, const eth_timestamp_type *const timestamp_ptr);

/*!*********************************************************************************************************************
* @brief    定义设置校正时间函数指针类型
**********************************************************************************************************************/
typedef void (*eth_set_correctiontime_ptr)(const uint8 ctrl_idx, const eth_time_int_diff_type *const timeoffset_ptr,
					const eth_rateratio_type *const rateratio_ptr);

/*!*********************************************************************************************************************
* @brief    定义获取发送缓冲区函数指针类型
**********************************************************************************************************************/
typedef BufReq_ReturnType (*eth_provide_txbuffer_ptr)(uint8 ctrl_idx, uint8 priority, eth_buf_idx_type *bufidx_ptr,
	eth_data_type **buf_ptr, uint16 *lenbyte_ptr);

/*!*********************************************************************************************************************
* @brief    定义接收函数指针类型
**********************************************************************************************************************/
typedef void (*eth_receive_ptr)(uint8 ctrl_idx, uint8 fifo_idx, eth_rx_status_type *rx_status_ptr);

#if defined(ETHMCAL_AUTOSAR_VERSION_440) || defined(ETHMCAL_AUTOSAR_VERSION_431)
/*!*********************************************************************************************************************
* @brief    定义启用Engress时间戳函数指针类型
**********************************************************************************************************************/
typedef void (*eth_enable_egress_timestamp_ptr)(uint8 ctrl_idx, eth_buf_idx_type buf_idx);
#else
/*!*********************************************************************************************************************
* @brief    定义启用Engress时间戳函数指针类型
**********************************************************************************************************************/
typedef void (*eth_enable_egress_timestamp_ptr)(const uint8 ctrl_idx, const uint8 buf_idx);
#endif

#if defined(ETHMCAL_AUTOSAR_VERSION_440)

/*!*********************************************************************************************************************
* @brief    定义获取Engress时间戳函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*eth_get_egress_timestamp_ptr)(uint8 ctrl_idx, eth_buf_idx_type buf_idx,
							eth_timestamp_qual_type *timequal_ptr,
							eth_timestamp_type *timestamp_ptr);

/*!*********************************************************************************************************************
* @brief    定义获取Ingress时间戳函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*eth_get_ingress_timestamp_ptr)(uint8 ctrl_idx, const eth_data_type *data_ptr,
							eth_timestamp_qual_type *timequal_ptr,
							eth_timestamp_type *timestamp_ptr);

#elif defined(ETHMCAL_AUTOSAR_VERSION_431)

/*!*********************************************************************************************************************
* @brief    定义获取Engress时间戳函数指针类型
**********************************************************************************************************************/
typedef void (*eth_get_egress_timestamp_ptr)(uint8 ctrl_idx, eth_buf_idx_type buf_idx, eth_timestamp_qual_type *timequal_ptr,
					eth_timestamp_type *timestamp_ptr);

/*!*********************************************************************************************************************
* @brief    定义获取Ingress时间戳函数指针类型
**********************************************************************************************************************/
typedef void (*eth_get_ingress_timestamp_ptr)(uint8 ctrl_idx, const eth_data_type *data_ptr,
						eth_timestamp_qual_type *timequal_ptr, eth_timestamp_type *timestamp_ptr);

#else
/*!*********************************************************************************************************************
* @brief    定义获取Engress时间戳函数指针类型
**********************************************************************************************************************/
typedef void (*eth_get_egress_timestamp_ptr)(const uint8 ctrl_idx, const uint8 buf_idx,
					eth_timestamp_qual_type *const time_qual_ptr,
					eth_timestamp_type *const timestamp_ptr);

/*!*********************************************************************************************************************
* @brief    定义获取Ingress时间戳函数指针类型
**********************************************************************************************************************/
typedef void (*eth_get_ingress_timestamp_ptr)(const uint8 ctrl_idx, const eth_data_type *const data_ptr,
						eth_timestamp_qual_type *const time_qual_ptr,
						eth_timestamp_type *const timestamp_ptr);

#endif

/*!*********************************************************************************************************************
* @brief    定义获取交换机port链路状态函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*swt_get_linkstate_ptr)(uint8 switch_idx, uint8 swt_port_id,
						eth_trcv_linkstate_type *swt_port_linkstate);

/*!*********************************************************************************************************************
* @brief    定义获取收发器链路状态函数指针类型
**********************************************************************************************************************/
typedef Std_ReturnType (*trcv_get_linkstate_ptr)(uint8 trcv_id, eth_trcv_linkstate_type *trcv_linkstate);

/*!*********************************************************************************************************************
* @brief    定义发送确认时上层回调函数指针类型
**********************************************************************************************************************/
typedef void (*ethif_ul_txconfirmation_type)(uint8 ctrl_idx, eth_buf_idx_type buf_idx);

/*!*********************************************************************************************************************
* @brief   定义EthIf对接的ETH驱动API类型
**********************************************************************************************************************/
typedef struct {
	eth_get_phys_addr_ptr eth_get_phys_addr; ///< 获取物理地址API
	eth_set_phys_addr_ptr eth_set_phys_addr; ///< 设置物理地址API
	eth_provide_txbuffer_ptr eth_provide_txbuffer; ///< 提供发送缓冲区API
	eth_transmit_ptr eth_transmit; ///< 发送数据API
#if (STD_OFF == ETHIF_ENABLE_TX_INTERRUPT)
	eth_txconfirmation_ptr eth_txconfirmation; ///< 发送确认API
#endif
	eth_update_phys_addr_filter_ptr eth_update_phys_addr_filter; ///< 更新物理地址过滤API
#if (STD_OFF == ETHIF_ENABLE_RX_INTERRUPT)
	eth_receive_ptr eth_receive; ///< 接收数据API
#endif
	eth_get_controller_config_ptr eth_get_controller_config; ///< 获取控制器配置API
#if (STD_ON == ETHIF_GLOBAL_TIME_SUPPORT)
	eth_get_currenttime_ptr eth_get_currenttime; ///< 获取当前时间API
	eth_enable_egress_timestamp_ptr eth_enable_egress_timestamp; ///< 使能Egress时间戳API
	eth_get_egress_timestamp_ptr eth_get_egress_timestamp; ///< 获取Egress时间戳API
	eth_get_ingress_timestamp_ptr eth_get_ingress_timestamp; ///< 获取Ingress时间戳API
	eth_set_correctiontime_ptr eth_set_correctiontime; ///< 设置时间校正API
	eth_set_globaltime_ptr eth_set_globaltime; ///< 设置全局时间API
#endif
	trcv_get_linkstate_ptr trcv_get_linkstate; ///< 收发器获取链路状态API
	swt_get_linkstate_ptr swt_get_linkstate; ///< 交换机获取链路状态API
} ethif_ethdrv_api_type;

/*!*********************************************************************************************************************
* @brief   定义EthIf交换机端口类型
**********************************************************************************************************************/
typedef struct {
	uint8 ethif_swt_port_id; ///< 交换机端口ID
	uint8 ethif_swt_port_role; ///< 交换机端口角色
	uint8 ethif_swt_id; ///< 交换机ID
} ethif_switch_port_type;

/*!*********************************************************************************************************************
* @brief   定义EthIf交换机端口组类型
**********************************************************************************************************************/
typedef struct {
	uint8 port_count; ///< 端口数量
	ethif_switch_port_type *ethif_switch_portlist; ///< 端口列表
} ethif_switch_port_group_type;

/*!*********************************************************************************************************************
* @brief   定义EthIf配置信息类型
**********************************************************************************************************************/
typedef struct {
	const ethif_controller_type *ethif_ctrl_cfg; ///< EthIf控制器配置数据指针
	const ethif_frame_owner_type *ethif_owner_cfg; ///< EthIf帧所有者配置数据指针
#if (ETHIF_SWITCH_SUPPORT == STD_ON)
	const ethif_switch_type *ethif_switch_cfg; ///< EthIf交换机配置数据指针
#endif
	const ethif_ul_txconfirmation_type *ethif_ul_txconfirmation; ///< EthIf上层发送确认回调函数列表指针
	const ethif_ul_rxindication_type *ethif_ul_rxIndication; ///< EthIf上层接收回调函数列表指针
	const ethif_ul_linkstate_chg_type *ethif_ul_linkstate_chg; ///< EthIf上层链路状态变化回调函数列表指针
	/*const EthIfTrcvLinkStateChgType*  EthIfTrcvLink;       pointer to hold Trcv Link state functions list */
	uint8 ethif_ctrl_count; ///< EthIf控制器数量
	uint8 ethif_trcv_count; ///< EthIf收发器数量
	uint8 ethif_switch_count; ///< EthIf交换机数量
	uint8 ethif_owners_count; ///< EthIf帧所有者数量
	const ethif_ethdrv_api_type *ethdrv_api_of_pcconfig_ptr; ///< EthIf驱动接口指针
	const ethif_switch_port_group_type *ethif_swt_port_group_cfg; ///< EthIf交换机端口组配置数据指针
} ethif_config_type;

#ifdef __cplusplus
}
#endif

#endif /* ETHIF_TYPES_H */
 