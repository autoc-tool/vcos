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

#ifndef COMM_DCM_H
#define COMM_DCM_H

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_START_SEC_CODE
#include "comm_memmap.h"
/*!*********************************************************************************************************************
 * @brief          dcm指示活动诊断
 * @details        当dcm检测到活动诊断会话时，调用此函数来通知comm进行相应的处理
 * @param[in]      channel 通道标识符，表示活动诊断应该在哪个通信通道上进行
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_dcm_active_diagnostic(NetworkHandleType channel);

/*!*********************************************************************************************************************
 * @brief          dcm指示非活动诊断
 * @details        当dcm检测到非活动诊断会话时，调用此函数来通知comm结束相关处理
 * @param[in]      channel 通道标识符，表示非活动诊断相关处理应该在哪个通信通道上结束
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_dcm_inactive_diagnostic(NetworkHandleType channel);
#define COMM_STOP_SEC_CODE
#include "comm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* COMM_DCM_H */
