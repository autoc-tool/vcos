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
 * @file        ethif_internal.h
 * @brief       EthIf内部头文件
 * @details     主要包含EthIf模块内部使用的函数、类型定义
 * @date        2023-3-17
 * @version     V1.0
 **********************************************************************************************************************/

#ifndef ETHIF_INTERNAL_H
#define ETHIF_INTERNAL_H

#include "ethif_types.h"
#include "ethif_cfg.h"
#include "Eth_GeneralTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ETH_FRAME_TYPE_VLAN (0x8100u) ///< VLAN帧类型
#define ETH_FRAME_VLAN_HI (0x81u) ///< VLAN帧类型高八位
#define ETH_FRAME_VLAN_LOW (0x00u) ///< VLAN帧类型低八位
#define PCP_SHIFT_BITS_5 (5u) ///< PCP位移量 5
#define PCP_SHIFT_BITS_4 (4u) ///< PCP位移量 4
#define VLAN_MASK_NIBBLE (0x0F00u) ///< VLAN掩码 4 位
#define VLAN_MASK_BYTE (0x00FFu) ///< VLAN掩码 1 字节
#define SHIFT_EIGHT_BITS (8u) ///< 右移8位
#define FRAME_MASK_HI (0xFF00u) ///< 帧类型掩码 2 字节
#define FRAME_MASK_LOW (0x00FFu) ///< 帧类型掩码 1 字节
#define VLAN_TAG_SIZE (4u) ///< VLAN标签大小
#define PRIORITY_SIZE (8u) ///< 优先级大小
#define ETHIF_VID_MASK (0x0FFFu) ///< VID掩码
#define ETHIF_SHIFT_BYTE1 (8u) ///< 右移8位
#define ETHIF_BYTE_MASK (0xFFu) ///< 字节掩码
#define INVALID_BUFFER_INDEX (0xFFFFFFFFUL) ///< 无效缓冲区索引
#define INVALID_FRAME_TYPE (0xFFFFu) ///< 无效帧类型
#define ETHIF_INVALID_CTRLIDX (0xFFu) ///< 无效控制器索引
#define TRCV_LINKSTATECHECK_TIMES (2u)
#define ETH_FRAME_TYPE_IPV4 (0x0800u) ///< IPV4帧类型
#define ETH_FRAME_TYPE_ARP (0x0806u) ///< ARP帧类型
#define ETH_FRAME_TYPE_PTP (0x88F7u) ///< 以太时间同步帧类型
#define ETHIF_GET_STATS_INFO (STD_ON) ///< EthIf获取统计信息宏
#define ETHIF_ETH_MAX_DROP_COUNTER_NUM (14U) ///< EthIf最大丢包计数器种类

/*!*********************************************************************************************************************
 * @brief   定义EthIf内部状态类型
 **********************************************************************************************************************/
typedef struct {
	ethif_statetype init_status; ///< 初始化状态

	eth_frame_type ethif_runtime[ETHIf_MAX_TXBUFSTOTAL]; ///< 运行时每个缓存区对应的帧类型

	eth_mode_type ethif_ctrlmode[ETHIF_CTRLS_CNT]; ///< 控制器的控制器模式
	eth_trcv_linkstate_type ethif_linkstate[ETHIF_CTRLS_CNT]; ///< 控制器的链路状态
	uint8 trcv_linkstate_check_count; ///< 上层实际可获取真实链路状态的次数
} ethif_internal_type;

#if (ETHIF_GET_STATS_INFO == STD_ON)
/*!*********************************************************************************************************************
 * @brief   定义EthIf统计信息类型
 **********************************************************************************************************************/
typedef struct {
	uint16 xmit[ETHIF_CTRLS_CNT]; ///< 发送被调用次数
	uint16 xmit_ok[ETHIF_CTRLS_CNT]; ///< 发送成功次数
	uint16 recv; ///< 接收回调被调用次数
	uint16 recv_ok[ETHIF_CTRLS_CNT]; ///< 接收并成功通知上层次数
	uint16 txcfm; ///< 发送确认次数
	uint16 txcfm_ok[3]; ///< 发送确认成功送至上层的次数 0---IPV4  1---ARP  2---ETHTSYN
	ethif_statetype init_status; ///< EthIf初始化状态  0---未初始化  1---已初始化
	eth_mode_type ethif_ctrlmode[ETHIF_CTRLS_CNT]; ///< 控制器的控制器模式
	eth_trcv_linkstate_type ethif_linkstate[ETHIF_CTRLS_CNT]; ///< 控制器的链路状态
	uint16 ethif_vlan_id[ETHIF_CTRLS_CNT]; ///< 控制器的VLAN ID
	uint32 crcerr; ///< eth收包crc错误统计
} ethif_stats_type;
#endif

#define eth_get_controller_mode(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_get_controller_mode)
#define eth_set_controller_mode(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_set_controller_mode)
#define eth_get_phys_addr(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_get_phys_addr)
#define eth_set_phys_addr(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_set_phys_addr)
#define eth_provide_txbuffer(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_provide_txbuffer)
#define eth_transmit(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_transmit)
#define eth_txconfirmation(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_txconfirmation)
#define eth_update_phys_addr_filter(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_update_phys_addr_filter)
#define eth_receive(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_receive)
#define eth_get_controller_config(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_get_controller_config)
#define eth_get_currenttime(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_get_currenttime)
#define eth_enable_egress_timestamp(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_enable_egress_timestamp)
#define eth_get_egress_timestamp(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_get_egress_timestamp)
#define eth_get_ingress_timestamp(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_get_ingress_timestamp)
#define eth_set_correctiontime(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_set_correctiontime)
#define eth_set_globaltime(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].eth_set_globaltime)
#define trcv_get_linkstate(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].trcv_get_linkstate)
#define swt_get_linkstate(index) (ethif_config_pointer->ethdrv_api_of_pcconfig_ptr[index].swt_get_linkstate)

#define ETHIF_START_SEC_CODE
#include "ethif_memmap.h"

/*!*********************************************************************************************************************
 * @brief          异步获取链路状态
 * @details        异步获取链路状态
 * @param[in]      ctrl_idx      控制器索引
 * @param[out]     link_state    链路状态
 * @retval         E_OK         获取链路状态成功
 * @retval         E_NOT_OK     获取链路状态失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ETHIF_CODE) ethif_get_linkstate(uint8 ctrl_idx, eth_trcv_linkstate_type *link_state);

/*!*********************************************************************************************************************
 * @brief          检查链路状态变化
 * @details        检查链路状态变化，如有变化，通知上层模块
 * @retval         E_OK     检查链路状态变化成功
 * @retval         E_NOT_OK 检查链路状态变化失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ETHIF_CODE) ethif_check_linkstate_chg(void);

/*!*********************************************************************************************************************
 * @brief          获取vlanId对应的控制器index
 * @details        获取vlanId对应的控制器index
 * @param[in]      eth_ctrl_idx   控制器索引
 * @param[in]      vlan_id       vlan_id
 * @retval                      vlanId对应的控制器ID
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint8, ETHIF_CODE) ethif_match_vid(uint8 eth_ctrl_idx, uint16 vlan_id);

/*!*********************************************************************************************************************
 * @brief          获取ETH驱动控制器配置
 * @details        获取索引为CtrlIdx的ETH驱动控制器配置
 * @param[in]      ctrl_idx          控制器索引
 * @param[out]     controller_cfg    控制器配置指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ETHIF_CODE) ethif_get_controller_config(uint8 ctrl_idx, ethif_controller_cfg *controller_cfg);

/*!*********************************************************************************************************************
 * @brief          检查ethCtrlIdx和vlanId是否相匹配
 * @details        检查ethCtrlIdx和vlanId是否相匹配
 * @param[in]      eth_ctrl_idx   控制器索引
 * @param[in]      vlan_id       VLAN ID
 * @retval         TRUE         匹配
 * @retval         FALSE        不匹配
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, ETHIF_CODE) ethif_is_vid_corrected(uint8 eth_ctrl_idx, uint16 vlan_id);

/*!*********************************************************************************************************************
 * @brief          设置ETHTRCV的链路状态
 * @details        设置ETHTRCV的链路状态
 * @param[in]      trcv_idx      TRCV索引
 * @param[in]      link_state    链路状态
 * @retval         E_OK 成功
 * @retval         E_NOT_OK 失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
extern FUNC(Std_ReturnType, ETHIF_CODE) ethif_ethtrcv_set_linkstate(uint8 trcv_idx, eth_trcv_linkstate_type link_state);

/*!*********************************************************************************************************************
 * @brief          更新ETHTRCV的链路状态
 * @details        更新ETHTRCV的链路状态
 * @param[in]      linkstate    链路状态
 * @retval         E_OK 成功
 * @retval         E_NOT_OK 失败

 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
extern FUNC(Std_ReturnType, ETHIF_CODE) ethtrcv_update_linkstate(void);

#define ETHIF_STOP_SEC_CODE
#include "ethif_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* ETHIF_INTERNAL_H */