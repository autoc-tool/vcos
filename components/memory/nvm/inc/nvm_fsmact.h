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

#ifndef NVM_FSMACT_H
#define NVM_FSMACT_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!********************************************************************************************************************
 * @brief          NvM 状态操作类型枚举
 * @details        定义了NvM中进行状态管理和转换时可执行的不同操作类型。
 **********************************************************************************************************************/
typedef enum {
	NVM_ACT_GetHighPrioJob, ///< 获取高优先级作业
	NVM_ACT_GetNormalPrioJob, ///< 获取普通优先级作业
	NVM_ACT_ReadNvBlock, ///< 读取非易失性内存块
	NVM_ACT_ReadNvFinshed, ///< 完成读取非易失性内存块
	NVM_ACT_FinishReadBlock, ///< 完成读块操作
	NvM_ACT_SetupRedundant, ///< 设置冗余块
	NVM_ACT_FinishRestoreRom, ///< 完成从ROM恢复
	NVM_ACT_RestoreBlockDefaults, ///< 恢复块默认值
	NVM_ACT_InitWriteBlock, ///< 初始化写块操作
	NVM_ACT_WriteNvBlock, ///< 写非易失性内存块
	NVM_ACT_WriteFinshed, ///< 完成写非易失性内存块
	NVM_ACT_InitInvalidteBlock, ///< 初始化使块无效
	NVM_ACT_EraseNvBlock, ///< 擦除非易失性内存块
	NVM_ACT_StartWriteAll, ///< 开始全部写操作
	NVM_ACT_WriteNextBlock, ///< 写下一个块
	NVM_ACT_FinishWriteAll, ///< 完成全部写操作
	NVM_ACT_CancelWriteAll, ///< 取消全部写操作
	NVM_ACT_StartReadAll, ///< 开始全部读操作
	NVM_ACT_ReadNextBlock, ///< 读下一个块
	NVM_ACT_FinishReadAll, ///< 完成全部读操作
	NVM_ACT_FinishBlock, ///< 完成块操作
	NVM_ACT_AddTriedTimes, ///< 添加尝试次数
	NVM_ACT_Wait, ///< 等待
	NVM_ACT_Nop ///< 无操作
} nvm_state_action_type;

/*!********************************************************************************************************************
 * @brief          NvM 动作函数指针类型
 * @details        指向执行NvM操作的函数的无参函数指针类型，这些函数无返回值。
 **********************************************************************************************************************/
typedef void (*nvm_act_fct_ptr_t)(void);

#define NVM_START_SEC_CONST
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          NvM 行动表
 * @details        包含指向执行特定动作的函数的指针的数组
 * @note           该数组被初始化为包含每种操作对应的函数的指针。
 **********************************************************************************************************************/
extern CONST(nvm_act_fct_ptr_t, NVM_CONST) nvm_action_table[NVM_ACT_Nop + 1];
#define NVM_STOP_SEC_CONST
#include "nvm_memmap.h"

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          NvM 块通知函数
 * @details        指定的NvM块操作完成后的回调函数，通知调用方操作完成的结果。
 * @param[in]      block_id 指定的NvM块ID。
 * @param[in]      service_id 调用的服务ID。
 * @param[in]      job_result 作业结果。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE)
nvm_block_notification(nvm_block_id_t block_id, nvm_service_id_t service_id, nvm_request_result_type job_result);
#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
#ifdef __cplusplus
}
#endif

#endif /* NVM_FSMACT_H */
