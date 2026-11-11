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

#ifndef DCM_CBK_H
#define DCM_CBK_H

#include "comstack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

/*!*********************************************************************************************************************
 * @brief          DCM 拷贝接收数据
 * @details        DCM 拷贝接收数据
 * @param[in]      dcm_rx_pdu_id 配置接收的pduid
 * @param[in]      pdu_info_ptr PDU 数据源
 * @param[out]     rx_buffer_size_ptr 接收数据区buffer长度指针
 * @retval         BUFREQ_OK 拷贝接收数据成功
 * @retval         BUFREQ_E_NOT_OK 拷贝接收数据失败
 * @retval         BUFREQ_E_BUSY 拷贝接收数据 忙碌
 * @retval         BUFREQ_E_OVFL 拷贝接收数据溢出
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, DCM_CODE)
dcm_copy_rx_data(PduIdType dcm_rx_pdu_id, const PduInfoType *pdu_info_ptr, PduLengthType *rx_buffer_size_ptr);

/*!*********************************************************************************************************************
 * @brief          DCM 开始接收数据
 * @details        DCM 开始接收数据
 * @param[in]      dcm_rx_pdu_id 配置接收的pduid
 * @param[in]      info       pdu信息结构体指针
 * @param[in]      tp_sdu_length 数据长度
 * @param[in]      rx_buffer_size_ptr 接收数据区buffer长度指针
 * @retval         BUFREQ_OK 开始接收数据成功
 * @retval         BUFREQ_E_NOT_OK 开始接收数据失败
 * @retval         BUFREQ_E_BUSY 开始接收数据忙碌
 * @retval         BUFREQ_E_OVFL 开始接收数据溢出
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, DCM_CODE)
dcm_start_of_reception(PduIdType dcm_rx_pdu_id, const PduInfoType *info, PduLengthType tp_sdu_length,
	PduLengthType *rx_buffer_size_ptr);

/*!*********************************************************************************************************************
 * @brief          DCM tp 数据接收回调
 * @details        DCM tp 数据接收回调
 * @param[in]      dcm_rx_pdu_id 配置接收的pduid
 * @param[in]      result 接收结果
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_tp_rx_indication(PduIdType dcm_rx_pdu_id, NotifResultType result);

/*!*********************************************************************************************************************
 * @brief          DCM 拷贝发送数据
 * @details        DCM 拷贝发送数据
 * @param[in]      dcm_tx_pdu_id 配置发送的pduid
 * @param[in]      pdu_info_ptr PDU 数据源
 * @param[in]      retry 重试次数
 * @param[in]      tx_data_cnt_ptr 发送数据计数
 * @retval         BUFREQ_OK 拷贝发送数据成功
 * @retval         BUFREQ_E_NOT_OK 拷贝发送数据失败
 * @retval         BUFREQ_E_BUSY 拷贝发送数据 忙碌
 * @retval         BUFREQ_E_OVFL 拷贝发送数据溢出
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, DCM_CODE)
dcm_copy_tx_data(PduIdType dcm_tx_pdu_id, const PduInfoType *pdu_info_ptr, const RetryInfoType *retry,
	PduLengthType *tx_data_cnt_ptr);

/*!*********************************************************************************************************************
 * @brief          DCM tp 发送确认
 * @details        DCM tp 发送确认
 * @param[in]      dcm_tx_pdu_id 配置发送的pduid
 * @param[in]      result 结果
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_tp_tx_confirmation(PduIdType dcm_tx_pdu_id, NotifResultType result);

/*!*********************************************************************************************************************
 * @brief          comm 通知dcm 切换到 no_com_mode 模式
 * @details        comm 通知dcm 切换到 no_com_mode 模式
 * @param[in]      network_id 通信网络id
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_comm_no_com_mode_entered(uint8 network_id);

/*!*********************************************************************************************************************
 * @brief          comm 通知dcm 切换到 silent_com 模式
 * @details        comm 通知dcm 切换到 silent_com 模式
 * @param[in]      network_id 通信网络id
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_comm_silent_com_mode_entered(uint8 network_id);

/*!*********************************************************************************************************************
 * @brief          comm 通知dcm 切换到 full_com_mode 模式
 * @details        comm 通知dcm 切换到 full_com_mode 模式
 * @param[in]      network_id 通信网络id
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_comm_full_com_mode_entered(uint8 network_id);

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DCM_CBK_H */
