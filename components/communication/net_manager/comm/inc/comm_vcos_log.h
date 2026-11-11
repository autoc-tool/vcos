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

#ifndef COMM_VCOS_LOG_H
#define COMM_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_DLT_CHANNEL_ID "ComM Channel ID: %d\r\n" ///< 日志记录通道id
#define COMM_DLT_COMMODE "ComM ComMode: %d\r\n" ///< 日志记录通信模式
#define COMM_DLT_SERVICE_ID "ComM Service ID: %d\r\n" ///< 日志记录服务id
#define COMM_DLT_NM_MASK "ComM NmIndicationMask: %d\r\n" ///< 日志记录nm指示掩码
#define COMM_DLT_DEINIT "ComM deinit completed\r\n" ///< 日志记录去初始化完成
#define COMM_DLT_DEBUG_NM "ComM Channel ID: %d, Service ID:%d,NmIndicationMask:%d\r\n" ///< 日志记录通道id/服务id/nm掩码
#define COMM_DLT_DEBUG_BUSSM "ComM Channel ID: %d, ComMode:%d\r\n" ///< 日志记录通道id/通信模式

#ifdef __cplusplus
}
#endif

#endif /* COMM_VCOS_LOG_H */
