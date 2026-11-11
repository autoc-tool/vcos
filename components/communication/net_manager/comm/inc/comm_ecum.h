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

#ifndef COMM_ECUM_H
#define COMM_ECUM_H

#include "comstack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_START_SEC_CODE
#include "comm_memmap.h"
/*!*********************************************************************************************************************
 * @brief          执行特定通道的唤醒通知
 * @details        当ecu通过指定的通道检测到唤醒事件时，调用此函数进行唤醒通知
 * @param[in]      channel 通道标识符，表示唤醒通知应该在哪个通信通道上进行
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_ecum_wake_up_indication(NetworkHandleType channel);

#define COMM_STOP_SEC_CODE
#include "comm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* COMM_ECUM_H */
