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

#ifndef NVM_FSMEVENT_H
#define NVM_FSMEVENT_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   NvM 事件类型枚举
 **********************************************************************************************************************/
typedef enum {
	NVM_EVENT_HAVE_JOB, ///< 有作业待处理
	NVM_EVENT_HAVE_HIGHPRIO_JOB, ///< 有高优先级作业待处理
	NVM_EVENT_NORMAL_PRIO_JOB, ///< 有普通优先级作业待处理
	NVM_EVENT_HAVEWRITEALL_JOB, ///< 有“写入所有”作业待处理
	NVM_EVENT_NV_BUSY, ///< 非易失性内存忙
	NVM_EVENT_CRC_MATCH, ///< CRC匹配
	NVM_EVENT_CrcCompare, ///< CRC对比中
	NVM_EVENT_LAST_RESULT_OK, ///< 上次结果成功
	NVM_EVENT_CANCEL_WRITE_ALL, ///< 取消“写入所有”命令
	NVM_EVENT_SUBTASK_RUNNING, ///< 子任务正在运行
	NVM_EVENT_REDUNDANT_BLOCK, ///< 冗余块检测
	NVM_EVENT_READ_RETRY, ///< 读取重试
	NVM_EVENT_WRITE_RETRY, ///< 写入重试
	NVM_EVENT_WRITEALL_END, ///< “写入所有”结束
	NVM_EVENT_LAST_BLOCK_DONE_READALL, ///< “读取所有”最后一块完成
	NVM_EVENT_TRUE ///< 真（事件成立）
} nvm_event_type;

/*!********************************************************************************************************************
 * @brief          NvM 事件处理函数的指针类型
 * @details        指向返回布尔值的无参NvM事件处理函数的函数指针类型。
 * @retval         TRUE 事件处理成功或条件成立。
 * @retval         FALSE 事件处理失败或条件不成立。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
typedef boolean (*nvm_event_ptr_t)(void);

#define NVM_START_SEC_CONST
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          NvM 事件处理函数表
 * @details        包含NvM事件处理函数的指针的数组
 * @note           该数组被初始化为包含每种事件对应的处理函数的指针。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
extern CONST(nvm_event_ptr_t, NVM_CONST) nvm_event_table[NVM_EVENT_TRUE + 1];
#define NVM_STOP_SEC_CONST
#include "nvm_memmap.h"
#ifdef __cplusplus
}
#endif

#endif /* NVM_FSMEVENT_H */
