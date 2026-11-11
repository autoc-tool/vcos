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
#ifndef BSWM_VCOS_LOG_H
#define BSWM_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define BSWM_DLT_ENABLED_PDUGROUP "[BswM] EnabledPduGroup is %d \r\n" ///< 已启动的PDU组
#define BSWM_DLT_DISABLED_PDUGROUP "[BswM] DisabledPduGroup is %d \r\n" ///< 已禁用的PDU组
#define BSWM_DLT_LIN_SCHEDULE "[BswM] Lin channel is %d and Schedule is %d \r\n" ///< LIN通道调度表
#define BSWM_DLT_TIMER_ID "[BswM] timer_id is %d \r\n" ///< 定时器ID
#define BSWM_DLT_TIMER_VALUE "[BswM] timer_value is %lu \r\n" ///< 定时器的读数值
#define BSWM_DLT_TIMER_STATE "[BswM] timer_state is %d \r\n" ///< 定时器状态值
#define BSWM_DLT_NMCONTROL_STATUS "[BswM] Nm control status is %d \r\n" ///< 网络管理状态
#define BSWM_DLT_DEINIT "[BswM] BswM deinit finished \r\n" ///< BswM模块始化

#ifdef __cplusplus
}
#endif

#endif /* BSWM_VCOS_LOG_H */