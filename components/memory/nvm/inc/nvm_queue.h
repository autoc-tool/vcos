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

#ifndef NVM_QUEUE_H
#define NVM_QUEUE_H

#include "Std_Types.h"
#include "nvm_cfg.h"
#include "nvm_types.h"
#include "nvm_fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   NvM 队列数据项结构体定义
 **********************************************************************************************************************/
typedef struct {
#if (NVM_JOB_PRIORIZATION == STD_ON)
	uint8 job_prio; ///< 任务优先级 (如果启用了任务优先化)
#endif
	nvm_internal_service_id_t service_id; ///< 服务ID
	nvm_block_id_t block_id; ///< 块ID
	uint8 *data_ptr; ///< 数据指针（源或目的）
} nvm_queue_data_t; ///< 定义NvM队列数据类型

/*!*********************************************************************************************************************
 * @brief   NvM 队列结构体定义
 **********************************************************************************************************************/
typedef struct {
	uint16 max_cnt; ///< 列表中的最大元素数量
	uint16 curr_cnt; ///< 当前元素数量

	uint16 data_size; ///< 元素的数据大小
	nvm_queue_data_t *head; ///< 列表头指针
	nvm_queue_data_t *tail; ///< 列表尾指针

	nvm_queue_data_t *buf_start; ///< 缓冲区开始指针
	nvm_queue_data_t *buf_end; ///< 缓冲区结束指针
} nvm_queue_t; ///< 定义NvM队列类型

#if (NVM_JOB_PRIORIZATION == STD_ON)
/*!*********************************************************************************************************************
 * @brief   NvM 优先任务队列
 **********************************************************************************************************************/
extern nvm_queue_t nvm_high_job_queue;
#endif
/*!*********************************************************************************************************************
 * @brief   NvM 作业队列
 **********************************************************************************************************************/
extern nvm_queue_t nvm_job_queue;

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          初始化NvM队列
 * @details        为NvM作业处理准备队列，设置队列的初识状态。
 * @reentrant      非可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_queue_init(void);

/*!********************************************************************************************************************
 * @brief          反初始化NvM队列
 * @details        为NvM作业处理准备队列设置队列的结束初识状态。
 * @reentrant      非可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_queue_deinit(void);

/*!********************************************************************************************************************
 * @brief          将数据推入NvM队列
 * @details        将作业数据添加到NvM队列尾部。
 * @param[in]      data_ptr 指向要插入队列的作业数据的指针。
 * @retval         E_OK 成功将数据推入队列。
 * @retval         E_NOT_OK 无法将数据添加到队列。
 * @synchronous    同步
 * @reentrant      可重入
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_queue_push(const nvm_queue_data_t *data_ptr);

/*!********************************************************************************************************************
 * @brief          从NvM队列中弹出数据
 * @details        将队列头部的作业数据移除，并复制到指定的内存位置。
 * @param[in,out]  queue_ptr 指向作业队列的指针。
 * @param[out]     data_ptr 指向用于接收弹出数据的内存位置的指针。
 * @retval         E_OK 成功从队列中弹出数据。
 * @retval         E_NOT_OK 队列为空或弹出操作失败。
 * @synchronous    同步
 * @reentrant      可重入
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_queue_pop(nvm_queue_t *queue_ptr, nvm_queue_data_t *data_ptr);
/*!********************************************************************************************************************
 * @brief          使队列中指定Block的作业无效
 * @details        在队列中查找特定Block ID的所有作业，并将其标记为无效。
 * @param[in]      block_id 指定的非易失性内存（NvM）块的标识符。
 * @retval         E_OK 成功标记为无效。
 * @retval         E_NOT_OK 操作失败或没有找到作业。
 * @synchronous    同步
 * @reentrant      可重入
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_invalidate_queue_job(nvm_block_id_t block_id);
/*!********************************************************************************************************************
 * @brief          处理高优先级的NvM作业事件
 * @details        检查高优先级队列中是否有待处理的作业，并进行处理。
 * @retval         TRUE 有待处理的高优先级作业。
 * @retval         FALSE 没有高优先级作业或操作失败。
 * @synchronous    同步
 * @reentrant      可重入
 **********************************************************************************************************************/
FUNC(boolean, NVM_CODE) nvm_event_high_prio_job(void);
/*!********************************************************************************************************************
 * @brief          处理普通优先级的NvM作业事件
 * @details        检查普通优先级队列中是否有待处理的作业，并进行处理。
 * @retval         TRUE 有待处理的普通优先级作业。
 * @retval         FALSE 没有普通优先级作业或操作失败。
 * @synchronous    同步
 * @reentrant      可重入
 **********************************************************************************************************************/
FUNC(boolean, NVM_CODE) nvm_event_normal_prio_job(void);

#if (NVM_JOB_PRIORIZATION == STD_ON)
/*!********************************************************************************************************************
 * @brief          获取高优先级队列中的作业
 * @details        从高优先级队列中获取一个作业，并复制到指定的内存位置。
 * @param[in,out]  queue_ptr 指向作业队列的指针。
 * @param[out]     data_ptr 指向用于接收作业数据的内存位置的指针。
 * @retval         E_OK 成功获取作业数据。
 * @retval         E_NOT_OK 队列为空或操作失败。
 * @synchronous    同步
 * @reentrant      可重入
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_get_high_prio_queue_job(nvm_queue_t *queue_ptr, nvm_queue_data_t *data_ptr);
#endif
#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
#ifdef __cplusplus
}
#endif

#endif /* NVM_QUEUE_H */
