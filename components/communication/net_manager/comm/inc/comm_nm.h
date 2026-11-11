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

#ifndef COMM_NM_H
#define COMM_NM_H

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_START_SEC_CODE
#include "comm_memmap.h"
/*!********************************************************************************************************************
 * @brief          接收到网络启动的nm消息指示
 * @details        在总线睡眠模式下接收到nm消息。
 * @param[in]      channel 网络通道的句柄。
 * @reentrant     可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_nm_net_start_indication(NetworkHandleType channel);

/*!********************************************************************************************************************
 * @brief          网络模式通知
 * @details        当nm模块成功进入网络模式时调用，用于通知comm网络已被激活。
 * @param[in]      channel 网络通道的句柄。
 * @reentrant     可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_nm_net_mode(NetworkHandleType channel);

/*!********************************************************************************************************************
 * @brief          进入prepare bus-sleep模式通知
 * @details        当nm模块将要进入prepare bus-sleep模式时调用，通知comm网络准备进入节能状态。
 * @param[in]      channel 网络通道的句柄
 * @reentrant     可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_nm_pre_bus_sleep_mode(NetworkHandleType channel);

/*!********************************************************************************************************************
 * @brief          进入bus-sleep模式通知
 * @details        当nm模块进入bus-sleep模式时调用，通知comm网络已进入最低功耗状态。
 * @param[in]      channel 网络通道的句柄
 * @reentrant     可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_nm_bus_sleep_mode(NetworkHandleType channel);

/*!********************************************************************************************************************
 * @brief          nm指示位复位通知
 * @details        当需要复位nm指示位以重新开始通信时调用。
 * @param[in]      channel 网络通道的句柄
 * @reentrant     可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_nm_restart_indication(NetworkHandleType channel);
#define COMM_STOP_SEC_CODE
#include "comm_memmap.h"
#ifdef __cplusplus
}
#endif

#endif /* COMM_NM_H */
