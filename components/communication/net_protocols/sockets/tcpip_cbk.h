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

#ifndef TCPIP_CBK_H
#define TCPIP_CBK_H

#include "Eth_GeneralTypes.h"
#include "EthTrcv_GeneralTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
/*!*********************************************************************************************************************
 * @brief          接收回调函数
 * @details        处理从EthIf接收到的数据,并递交给上层
 * @param[in]      ctrl_idx      控制器索引
 * @param[in]      frame_type    帧类型
 * @param[in]      is_broadcast  是否为广播
 * @param[in]      phys_addr_ptr  MAC地址指针
 * @param[in]      data_ptr      数据指针
 * @param[in]      len_byte      长度
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00029
 **********************************************************************************************************************/
FUNC(void, TCPIP_CODE) tcpip_rx_indication(uint8 ctrl_idx, eth_frame_type frame_type,
	boolean is_broadcast, const uint8 *phys_addr_ptr, const uint8 *data_ptr, uint16 len_byte);

/*!*********************************************************************************************************************
 * @brief          发送确认函数
 * @details        处理从EthIf发送的确认，并传递至上层
 * @param[in]      ethif_ctrl_idx 控制器索引
 * @param[in]      eth_buf_idx    缓冲区索引
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, TCPIP_CODE) tcpip_tx_confirmation(uint8 ethif_ctrl_idx, eth_buf_idx_type eth_buf_idx);

#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* TCPIP_CBK_H */
