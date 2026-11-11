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
 * @file        ethif.h
 * @brief       EthIf头文件
 * @details     主要包含EthIf功能相关的API、宏定义等
 * @date        2023-3-17
 * @version     V1.0
 **********************************************************************************************************************/

#ifndef ETHIF_H
#define ETHIF_H

#include "ethif_types.h"
#include "ethif_cbk.h"
#include "ethif_lcfg.h"
#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ETHIF_MODULE_ID (65u) ///< 模块ID

/* Error IDs  */
#define ETHIF_E_INV_CTRL_IDX (1u) ///< (DET) 控制器索引无效
#define ETHIF_E_INV_TRCV_IDX (2u) ///< (DET) 收发器索引无效
#define ETHIF_E_NOT_INITIALIZED (3u) ///< (DET) 未初始化
#define ETHIF_E_PARAM_POINTER (4u) ///< (DET) 参数指针无效
#define ETHIF_E_INV_PARAM (5u) ///< (DET) 参数无效
#define ETHIF_E_INIT_FAILED (6u) ///< (DET) 初始化失败

/* Service IDs */
#define ETHIF_SERVICE_ID_INIT (0x01u) ///< (Service ID) ethif_init
#define ETHIF_SERVICE_ID_SET_CTRL_MODE (0x03u) ///< (Service ID) EthIf_SetControllerMode
#define ETHIF_SERVICE_ID_GET_CTRL_MODE (0x04u) ///< (Service ID) ethif_get_controller_mode
#define ETHIF_SERVICE_ID_SET_TRCV_MODE (0x06u) ///< (Service ID) EthIf_SetTransceiverMode
#define ETHIF_SERVICE_ID_GET_TRCV_MODE (0x07u) ///< (Service ID) EthIf_GetTransceiverMode
#define ETHIF_SERVICE_ID_SET_TRCV_WAKEUP_MODE (0x2Eu) ///< (Service ID) EthIf_SetTransceiverWakeupMode
#define ETHIF_SERVICE_ID_GET_TRCV_WAKEUP_MODE (0x2Fu) ///< (Service ID) EthIf_GetTransceiverWakeupMode
#define ETHIF_SERVICE_ID_CHECK_WAKE_UP (0x30u) ///< (Service ID) EthIf_CheckWakeup
#define ETHIF_SERVICE_ID_GET_PHY_ADDR (0x08u) ///< (Service ID) EthIf_GetPhyAddress
#define ETHIF_SERVICE_ID_SET_PHY_ADDR (0x0Du) ///< (Service ID) EthIf_SetPhyAddress
#define ETHIF_SERVICE_ID_UPDATE_PHY_ADDR_FILTER (0x0Cu) ///< (Service ID) EthIf_UpdatePhyAddressFilter
#define ETHIF_SERVICE_ID_GET_PORT_MAC_ADDR (0x28u) ///< (Service ID) EthIf_GetPortMacAddress
#define ETHIF_SERVICE_ID_GET_ARL_TABEL (0x29u) ///< (Service ID) EthIf_GetArlTable
#define ETHIF_SERVICE_ID_GET_BUF_LEVEL (0x2Au) ///< (Service ID) EthIf_GetBufferLevel
#define ETHIF_SERVICE_ID_GET_DROP_COUNT (0x2Bu) ///< (Service ID) EthIf_GetDropCount
#define ETHIF_SERVICE_ID_STORE_CONFIG (0x2Cu) ///< (Service ID) EthIf_StoreConfig
#define ETHIF_SERVICE_ID_RESET_CONFIG (0x2Du) ///< (Service ID) EthIf_ResetConfig
#define ETHIF_SERVICE_ID_GET_CURRENT_TIME (0x22u) ///< (Service ID) EthIf_GetCurrentTime
#define ETHIF_SERVICE_ID_ENABLE_EGRESS_TIME_STAMP (0x23u) ///< (Service ID) EthIf_EnableEgressTimeStamp
#define ETHIF_SERVICE_ID_GET_EGRESS_TIME_STAMP (0x24u) ///< (Service ID) EthIf_GetEgressTimeStamp
#define ETHIF_SERVICE_ID_GET_INGRESS_TIME_STAMP (0x25u) ///< (Service ID) EthIf_GetIngressTimeStamp
#define ETHIF_SERVICE_ID_SET_CORRECTION_TIME (0x26u) ///< (Service ID) EthIf_SetCorrectionTime
#define ETHIF_SERVICE_ID_SET_GLOBAL_TIME (0x27u) ///< (Service ID) EthIf_SetGlobalTime
#define ETHIF_SERVICE_ID_PROVIDE_TX_BUFFER (0x09u) ///< (Service ID) ethif_provide_tx_buffer
#define ETHIF_SERVICE_ID_TRANSMIT (0x0Au) ///< (Service ID) ethif_transmit
#define ETHIF_SERVICE_ID_GET_VERSION_INFO (0x0Bu) ///< (Service ID) EthIf_GetVersionInfo
#define ETHIF_SERVICE_ID_GET_PHY_INFO (0x15u) ///< (Service ID) EthIf_GetPhyInfo
#define ETHIF_SERVICE_ID_GET_PHY_LINKSTATE (0x16u) ///< (Service ID) EthIf_GetPhyLinkState

/* Callback notifications function service Ids */
#define ETHIF_SERVICE_ID_RX_INDICATION (0x10u) ///< (Service ID) ethif_rxindication
#define ETHIF_SERVICE_ID_TX_CONFIRMATION (0x11u) ///< (Service ID) ethif_txconfirmation
#define ETHIF_SERVICE_ID_CTRL_MODE_INDICATION (0x0Eu) ///< (Service ID) ethif_ctrlmode_indication
#define ETHIF_SERVICE_ID_TRCV_MODE_INDICATION (0x0Fu) ///< (Service ID) EthIf_TrcvModeIndication

/* Scheduled functions service IDs */
#define ETHIF_SERVICE_ID_MAIN_FUNCTION_RX (0x20u) ///< (Service ID) ethif_mainfunction_rx
#define ETHIF_SERVICE_ID_MAIN_FUNCTION_TX (0x21u) ///< (Service ID) ethif_mainfunction_tx

#define ETHIF_START_SEC_CODE
#include "ethif_memmap.h"

/*!*********************************************************************************************************************
 * @brief          初始化EthIf
 * @details        获取EthIf的配置信息，将初始化状态转换为ETHIF_STATE_INIT
 * @param[in]      cfg_ptr   EthIf配置信息结构体指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ETHIF_CODE) ethif_init(const ethif_config_type *cfg_ptr);

/*!*********************************************************************************************************************
 * @brief          获取MAC地址
 * @details        设置索引为ctrl_idx的控制器的MAC地址，放在phys_addr_ptr指针中
 * @param[in]      ctrl_idx      控制器索引
 * @param[out]     phys_addr_ptr  物理地址指针
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_EthIf_00061
 **********************************************************************************************************************/
FUNC(void, ETHIF_CODE) ethif_get_phys_addr( uint8 ctrl_idx, uint8* phys_addr_ptr);

/*!*********************************************************************************************************************
 * @brief          设置MAC地址
 * @details        设置索引为ctrl_idx的控制器的MAC地址为phys_addr_ptr指针中的值
 * @param[in]      ctrl_idx      控制器索引
 * @param[in]      phys_addr_ptr  物理地址指针
 * @reentrant      对相同ctrl_idx不可重入，对不同ctrl_idx可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ETHIF_CODE) ethif_set_phys_addr(uint8 ctrl_idx, const uint8 *phys_addr_ptr);

/*!*********************************************************************************************************************
 * @brief          更新物理地址过滤器
 * @details        更新索引为ctrl_idx的控制器的物理地址过滤器
 * @param[in]      ctrl_idx      控制器索引
 * @param[in]      phys_addr_ptr  物理地址指针
 * @param[in]      action       过滤器操作
 * @retval         E_OK         更新物理地址过滤器成功
 * @retval         E_NOT_OK     更新物理地址过滤器失败
 * @reentrant      对相同ctrl_idx不可重入，对不同ctrl_idx可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ETHIF_CODE) ethif_update_phys_addr_filter(uint8 ctrl_idx,
	const uint8 *phys_addr_ptr, eth_filter_action_type action);

/*!*********************************************************************************************************************
 * @brief          获取发送缓冲区
 * @details        为发送数据提供缓冲区
 * @param[in]      ctrl_idx 控制器索引
 * @param[in]      frame_type 帧类型
 * @param[in]      priority 优先级
 * @param[in]      bufidx_ptr 缓冲区索引指针
 * @param[in]      buf_ptr 缓冲区指针
 * @param[in]      lenbyte_ptr 长度字节指针
 * @retval         BUFREQ_OK 获取发送缓冲区成功
 * @retval         BUFREQ_NOT_OK 获取发送缓冲区失败
 * @retval         BUFREQ_E_BUSY 获取发送缓冲区忙
 * @retval         BUFREQ_E_OVFL 获取发送缓冲区溢出
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, ETHIF_CODE) ethif_provide_tx_buffer(uint8 ctrl_idx, eth_frame_type frame_type,
	uint8 priority, eth_buf_idx_type *bufidx_ptr, eth_data_type **buf_ptr, uint16 *lenbyte_ptr);

/*!*********************************************************************************************************************
 * @brief          发送数据
 * @details        基于申请到的buffer,将buffer中的数据发送出去
 * @param[in]      ctrl_idx 控制器索引
 * @param[in]      buf_idx 缓冲区索引
 * @param[in]      frame_type 帧类型
 * @param[in]      tx_confirmation 传输确认
 * @param[in]      len_byte 长度字节
 * @param[in]      phys_addr_ptr 物理地址指针
 * @retval         E_OK 发送数据成功
 * @retval         E_NOT_OK 发送数据失败
 * @reentrant      对不同的ctrl_idx和buf_idx，是可重入的
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, ETHIF_CODE) ethif_transmit(uint8 ctrl_idx, eth_buf_idx_type buf_idx,
	eth_frame_type frame_type, boolean tx_confirmation, uint16 len_byte, const uint8 *phys_addr_ptr);

/*!*********************************************************************************************************************
 * @brief          控制器模式回调函数
 * @details        处理控制器模式的变化
 * @param[in]      ctrl_idx      控制器索引
 * @param[in]      ctrlmode     控制器模式
 * @reentrant      对相同ctrl_idx不可重入，对不同ctrl_idx可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ETHIF_CODE) ethif_ctrlmode_indication(uint8 ctrl_idx, eth_mode_type ctrlmode);

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
	eth_timestamp_qual_type* time_qual_ptr, eth_timestamp_type* timestamp_ptr);

#endif

#define ETHIF_STOP_SEC_CODE
#include "ethif_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* ETHIF_H */
