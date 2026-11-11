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

#ifndef CANTP_VCOS_LOG_H
#define CANTP_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define CANTP_DLT_CHECK_ID0 "[-CanTp-] Rx NOT_OK (Send FC Fail)\r\n" ///< 发送流控帧失败
#define CANTP_DLT_CHECK_ID1 "[-CanTp-] Rx NOT_OK (New Rx Frame)\r\n" ///< 新接收帧
#define CANTP_DLT_CHECK_ID2 "[-CanTp-] Rx NOT_OK (FF Copy OVEL)\r\n" ///< 首帧拷贝溢出
#define CANTP_DLT_CHECK_ID3 "[-CanTp-] Rx NOT_OK (FF Copy Not Ok)\r\n" ///< 首帧拷贝失败
#define CANTP_DLT_CHECK_ID4 "[-CanTp-] Rx WRONG_SN\r\n" ///< 连续帧sn号校验有误
#define CANTP_DLT_CHECK_ID5 "[-CanTp-] Rx PARAMETER_NOT_OK (CF)\r\n" ///< 接收参数不ok
#define CANTP_DLT_CHECK_ID6 "[-CanTp-] Rx NOT_OK (CF Copy Not Ok)\r\n" ///< 连续帧拷贝失败
#define CANTP_DLT_CHECK_ID7 "[-CanTp-] Rx NOT_OK (CF Copy Busy)\r\n" ///< 连续帧拷贝忙碌
#define CANTP_DLT_CHECK_ID8 "[-CanTp-] Tx NO_BUFFER (FC)\r\n" ///< 流控帧发送无缓存
#define CANTP_DLT_CHECK_ID9 "[-CanTp-] Tx INVALID_FS (FC)\r\n" ///< 发送无效流控帧
#define CANTP_DLT_CHECK_ID10 "[-CanTp-] Rx NOT_OK (Get SF Buf)\r\n" ///< 接收获取单帧缓存失败
#define CANTP_DLT_CHECK_ID11 "[-CanTp-] Rx NOT_OK (Get CF Buf Busy)\r\n" ///< 获取连续帧失败
#define CANTP_DLT_CHECK_ID12 "[-CanTp-] Rx NOT_OK (Get CF Buf Not Ok)\r\n" ///< 获取连续帧失败
#define CANTP_DLT_CHECK_ID13 "[-CanTp-] Tx (Tx Timeout) Result %d\r\n" ///< 发送超时
#define CANTP_DLT_CHECK_ID14 "[-CanTp-] Tx NOT_OK (PduR_CopyTxData)\r\n" ///< 发送拷贝失败
#define CANTP_DLT_CHECK_ID15 "[-CanTp-] Rx NTFRSLT_E_TIMEOUT_CR\r\n" ///< 拷贝结果超时
#define CANTP_DLT_CHECK_ID16 "[-CanTp-] Handle FC State Error\r\n" ///< 处理流控帧状态错误
#define CANTP_DLT_CHECK_ID17 "[-CanTp-] Rx Padding Error\r\n" ///< 接收填充配置错误
#define CANTP_DLT_CHECK_ID18 "[-CanTp-] Tx NOT_OK (CanIf_Transmit) Result %d\r\n" ///< canif发送失败

#ifdef __cplusplus
}
#endif

#endif /* CANTP_VCOS_LOG_H */
