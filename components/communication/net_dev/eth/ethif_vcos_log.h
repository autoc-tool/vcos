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
 * @file        ethif_vcos_log.h
 * @brief       EthIf的日志定义宏头文件
 * @details     定义了EthIf的日志宏
 * @date        2024-05-24
 * @version     V1.0
 * @copyright   Copyright (C) 2015-2024 Lixiang Group Holding Limited
 **********************************************************************************************************************/

 #ifndef ETHIF_VCOS_LOG_H
 #define ETHIF_VCOS_LOG_H
 
 #ifdef __cplusplus
 extern "C"
 {
 #endif
 
 #define ETHIF_DLT_CHECK_ID0 "[-ETHIF-] EthIf_ProvideTxBuffer ERR: %d\r\n"   ///< 定义EthIf申请buffer失败日志宏
 #define ETHIF_DLT_CHECK_ID1 "[-ETHIF-] EthIf_Transmit ERR\r\n"              ///< 定义EthIf发送数据失败日志宏
 #define ETHIF_DLT_CHECK_ID2 "[-ETHIF-] EthIf_SetControllerMode ERR\r\n"     ///< 定义EthIf设置控制器模式失败日志宏
 #define ETHIF_DLT_CHECK_ID3 "[-ETHIF-] Eth RX Packets CRC ERR:%d\r\n"     ///< 定义Ethcrc失败日志宏
 #define ETHIF_DLT_CHECK_ID4_1 "ETHIFRxIpHd1:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n" ///< 定义EthIfICMP收包IP头部日志宏
 #define ETHIF_DLT_CHECK_ID4_2 "ETHIFRxIpHd2:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n" ///< 定义EthIfICMP收包IP头部日志宏
 #define ETHIF_DLT_CHECK_ID4_3 "ETHIFRxIcmpHd:%02x%02x%02x%02x%02x%02x%02x%02x\r\n"       ///< 定义EthIfICMP收包ICMP头部日志宏
 #define ETHIF_DLT_CHECK_ID5 "[-ETHIF-] EthIf_MainFunctionState Uninit Report\r\n"   ///< 定义调用EthIf_MainFunctionState函数时EthIf模块未初始化的日志宏
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif  /* ETHIF_VCOS_LOG_H */
 