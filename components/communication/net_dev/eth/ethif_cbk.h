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
 * @file        ethif_cbk.h
 * @brief       EthIf回调函数头文件
 * @details     主要包含EthIf模块供外部模块使用的回调函数
 * @date        2023-3-17
 * @version     V1.0
 **********************************************************************************************************************/

#ifndef ETHIF_CBK_H
#define ETHIF_CBK_H

#include "ethif_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ETHIF_START_SEC_CODE
#include "ethif_memmap.h"

/*!*********************************************************************************************************************
* @brief          接收回调函数
* @details        处理从驱动接收到的数据,并递交给上层
* @param[in]      ctrl_idx      控制器索引
* @param[in]      frame_type    帧类型
* @param[in]      is_broadcast  是否为广播
* @param[in]      phys_addr_ptr  MAC地址指针
* @param[in]      data_ptr      数据指针
* @param[in]      len_byte      长度
* @reentrant      不可重入
* @synchronous    同步
**********************************************************************************************************************/
#if defined(ETHMCAL_AUTOSAR_VERSION_431)
FUNC(void, ETHIF_CODE) ethif_rxindication(uint8 ctrl_idx, eth_frame_type frame_type,
    boolean is_broadcast, const uint8 *phys_addr_ptr, const eth_data_type *data_ptr, uint16 len_byte);
#else
FUNC(void, ETHIF_CODE) ethif_rxindication(uint8 ctrl_idx, eth_frame_type frame_type,
    boolean is_broadcast, const uint8 *phys_addr_ptr, eth_data_type *data_ptr, uint16 len_byte);
#endif

#if defined(ETHMCAL_AUTOSAR_VERSION_422)
/*!*********************************************************************************************************************
* @brief          发送确认函数
* @details        处理从驱动器发送的确认，并传递至上层
* @param[in]      ctrl_idx      控制器索引
* @param[in]      buf_idx       缓冲区索引
* @reentrant      不可重入
* @synchronous    同步
**********************************************************************************************************************/
FUNC(void, ETHIF_CODE) ethif_txconfirmation(uint8 ctrl_idx, eth_buf_idx_type buf_idx);
#else
FUNC(void, ETHIF_CODE) ethif_txconfirmation(uint8 ctrl_idx, eth_buf_idx_type buf_idx, Std_ReturnType result);
#endif
/*!*********************************************************************************************************************
 * @brief          控制器模式回调函数
 * @details        处理控制器模式的变化
 * @param[in]      ctrl_idx      控制器索引
 * @param[in]      ctrlmode     控制器模式
 * @reentrant      对相同ctrl_idx不可重入，对不同ctrl_idx可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, ETHIF_CODE) ethif_ctrlmode_indication(uint8 ctrl_idx, eth_mode_type ctrlmode);
#define ETHIF_STOP_SEC_CODE
#include "ethif_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* ETHIF_CBK_H */
