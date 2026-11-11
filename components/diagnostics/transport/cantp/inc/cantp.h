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

#ifndef CANTP_H
#define CANTP_H

#include "cantp_types.h"
#include "cantp_cbk.h"
#include "cantp_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CANTP_MODULE_ID (35u) ///<  模块id

#define CANTP_START_SEC_CODE
#include "cantp_memmap.h"
/*!*********************************************************************************************************************
 * @brief          cantp模块初始化
 * @details        cantp模块初始化
 * @param[in]      cfg_ptr 配置数据指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, CANTP_CODE) cantp_init(const cantp_config_type *cfg_ptr);

/*!*********************************************************************************************************************
 * @brief          cantp模块关闭
 * @details        cantp模块关闭
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, CANTP_CODE) cantp_shutdown(void);

/*!*********************************************************************************************************************
 * @brief          请求传输 PDU
 * @details        请求传输 PDU
 * @param[in]      cantp_tx_pdu_id 要传输的pdu的标识符
 * @param[in]      cantp_tx_info_ptr  pdu数据的长度和指针
 * @retval         E_OK 传输成功
 * @retval         E_NOT_OK 传输失败
 * @reentrant      相同pdu_id不可重入，不同pdu_id可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANTP_CODE) cantp_transmit(PduIdType cantp_tx_pdu_id, const PduInfoType *cantp_tx_info_ptr);

#if (CANTP_TRANSMIT_CANCEL_API == STD_ON)
/*!*********************************************************************************************************************
 * @brief          取消发送请求
 * @details        取消发送请求
 * @param[in]      tx_pdu_id 取消传输的pdu的标识符
 * @retval         E_OK 取消发送请求成功
 * @retval         E_NOT_OK 取消发送请求失败
 * @reentrant      相同pdu_id不可重入，不同pdu_id可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANTP_CODE) cantp_cancel_transmit(PduIdType tx_pdu_id);
#endif

#if (CANTP_RECEIVE_CANCEL_API == STD_ON)
/*!*********************************************************************************************************************
 * @brief          取消接收请求
 * @details        取消接收请求
 * @param[in]      rx_pdu_id 要接收的pdu的标识符
 * @retval         E_OK 取消接收请求成功
 * @retval         E_NOT_OK 取消接收请求失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANTP_CODE) cantp_cancel_receive(PduIdType rx_pdu_id);
#endif

#if (CANTP_CHANGE_PARAMETER_API == STD_ON)

/*!*********************************************************************************************************************
 * @brief          修改传输层参数
 * @details        修改传输层参数
 * @param[in]      id 要修改的pdu的标识符
 * @param[in]      parameter 要修改的参数
 * @param[in]      value 要修改的值
 * @retval         E_OK 修改传输层参数成功
 * @retval         E_NOT_OK 修改传输层参数失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANTP_CODE) cantp_change_parameter(PduIdType id, tp_parameter_type parameter, uint16 value);
#endif

#if (CANTP_READ_PARAMETER_API == STD_ON)
/*!*********************************************************************************************************************
 * @brief          读取传输层参数
 * @details        读取传输层参数
 * @param[in]      id    要读取的pdu的标识符
 * @param[in]      parameter    要读取的参数
 * @param[in]      value    读取的值
 * @retval         E_OK 读取传输层参数成功
 * @retval         E_NOT_OK 读取传输层参数失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANTP_CODE) cantp_read_parameter(PduIdType id, tp_parameter_type parameter, uint16 *value);
#endif
#define CANTP_STOP_SEC_CODE
#include "cantp_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANTP_H */
