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

#ifndef __NVM_CBK_H /* SWS_NvM_00551  */
#define __NVM_CBK_H

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          NvM 作业结束通知
 * @details        当NvM管理的作业成功完成时调用此通知函数
 * @reentrant      可重入
 * @synchronous    异步
 *********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_job_end_notification(void);

/*!********************************************************************************************************************
 * @brief          NvM 作业错误通知
 * @details        当NvM管理的作业因发生错误而未能成功完成时调用此通知函数
 * @reentrant      可重入
 * @synchronous    异步
 *********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_job_error_notification(void);
#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* __NVM_CBK_H */
