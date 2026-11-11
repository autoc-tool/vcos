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

#ifndef CANTP_CBK_H
#define CANTP_CBK_H

#include "comstack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CANTP_START_SEC_CODE
#include "cantp_memmap.h"
/*!*********************************************************************************************************************
 * @brief          cantp接收回调函数
 * @details        cantp接收回调函数
 * @param[in]      cantp_rx_pdu_id      接收pdu_id
 * @param[in]      cantp_rx_pdu_ptr   接收pdu头指针
 * @reentrant      相同pdu_id不可重入，不同pdu_id可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, CANTP_CODE) cantp_rx_indication(PduIdType cantp_rx_pdu_id, const PduInfoType *cantp_rx_pdu_ptr);

/*!*********************************************************************************************************************
 * @brief          cantp发送确认接口
 * @details        cantp发送确认接口
 * @param[in]      cantp_tx_pdu_id cantp的发送pdu_id
 * @param[in]      result cantp的发送确认结果
 * @reentrant      相同pdu_id不可重入，不同pdu_id可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, CANTP_CODE) cantp_tx_confirmation(PduIdType cantp_tx_pdu_id, Std_ReturnType result);
#define CANTP_STOP_SEC_CODE
#include "cantp_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANTP_CBK_H */
