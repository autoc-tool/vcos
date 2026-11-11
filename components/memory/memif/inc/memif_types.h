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

#ifndef __MEMIF_TYPES_H
#define __MEMIF_TYPES_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEMIF_NUMBER_OF_DEVICES (1u) ///< 底层存储抽象模块的数量

/*!*********************************************************************************************************************
 * @brief   存储抽象模块和驱动当前状态类型枚举
 **********************************************************************************************************************/
typedef enum {
	MEMIF_UNINIT, ///< 存储抽象模块或驱动还未初始化
	MEMIF_IDLE, ///< 存储抽象模块或驱动处于空闲状态
	MEMIF_BUSY, ///< 存储抽象模块或驱动处于忙碌状态
	MEMIF_BUSY_INTERNAL ///< 存储抽象模块因内部管理操作处于忙碌状态
} MemIf_StatusType;

/*!*********************************************************************************************************************
 * @brief   上一个任务的执行结果类型枚举
 **********************************************************************************************************************/
typedef enum {
	MEMIF_JOB_OK, ///< 任务成功结束
	MEMIF_JOB_FAILED, ///< 任务失败
	MEMIF_JOB_PENDING, ///< 任务还未结束
	MEMIF_JOB_CANCELED, ///< 任务被取消
	MEMIF_BLOCK_INCONSISTENT, ///< 所请求的块可能包含错误数据
	MEMIF_BLOCK_INVALID ///< 所请求的块已被标记为无效
} MemIf_JobResultType;

/*!*********************************************************************************************************************
 * @brief   存储抽象模块和驱动的操作模式类型枚举
 **********************************************************************************************************************/
typedef enum {
	MEMIF_MODE_SLOW, ///< 慢速模式
	MEMIF_MODE_FAST ///< 快速模式
} MemIf_ModeType;

/*!*********************************************************************************************************************
 * @brief          读取函数指针类型
 * @details        定义从底层存储模块读取操作的函数指针类型
 * @param[in]      block_number 块编号
 * @param[in]      block_offset 块偏移量
 * @param[in]      data_buffer_ptr 指向目标RAM缓冲区的指针
 * @param[in]      length 数据长度
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*memif_read_type )(uint16 block_number, uint16 block_offset, uint8 *data_buffer_ptr, uint16 length);

/*!*********************************************************************************************************************
 * @brief          写入函数指针类型
 * @details        定义向底层存储模块写入操作的函数指针类型
 * @param[in]      block_number 块编号
 * @param[in]      data_buffer_ptr 指向源RAM缓冲区的指针
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*memif_write_type)(uint16 block_number, const uint8 *data_buffer_ptr);

/*!*********************************************************************************************************************
 * @brief          立即擦除块函数指针类型
 * @details        定义立即擦除底层存储模块块的函数指针类型
 * @param[in]      block_number 块编号
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*memif_erase_immediate_block_type)(uint16 block_number);

/*!*********************************************************************************************************************
 * @brief          无效化块函数指针类型
 * @details        定义将底层存储模块块标记为无效的函数指针类型
 * @param[in]      block_number 块编号
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*memif_invalidate_block_type)(uint16 block_number);

/*!*********************************************************************************************************************
 * @brief          取消函数指针类型
 * @details        定义取消当前操作的函数指针类型
 **********************************************************************************************************************/
typedef void (*memif_cancel_type)(void);

/*!*********************************************************************************************************************
 * @brief          获取状态函数指针类型
 * @details        定义获取当前存储模块状态的函数指针类型
 * @retval         MemIf_StatusType 当前存储模块状态
 **********************************************************************************************************************/
typedef MemIf_StatusType (*memif_get_status_type)(void);

/*!*********************************************************************************************************************
 * @brief          获取任务执行结果函数指针类型
 * @details        定义获取当前任务执行结果的函数指针类型
 * @retval         MemIf_JobResultType 当前任务执行结果
 **********************************************************************************************************************/
typedef MemIf_JobResultType (*memif_get_job_result_type)(void);

/*!*********************************************************************************************************************
 * @brief          设置模式函数指针类型
 * @details        定义设置当前操作模式的函数指针类型
 * @param[in]      mode 要设置的操作模式
 **********************************************************************************************************************/
typedef void (*memif_set_mode_type)(MemIf_ModeType mode);

/*!*********************************************************************************************************************
 * @brief       存储抽象层硬件抽象API结构体类型定义
 **********************************************************************************************************************/
typedef struct {
	memif_read_type  read; ///< 读取操作的函数指针
	memif_write_type write; ///< 写入操作的函数指针
	memif_erase_immediate_block_type erase_immediate_block; ///< 立即擦除块操作的函数指针
	memif_invalidate_block_type invalidate_block; ///< 无效化块操作的函数指针
	memif_cancel_type cancel; ///< 取消操作的函数指针
	memif_get_status_type get_status; ///< 获取状态操作的函数指针
	memif_get_job_result_type get_job_result; ///< 获取任务执行结果操作的函数指针
	memif_set_mode_type set_mode; ///< 设置模式操作的函数指针
} memif_abs_api_type;

#ifdef __cplusplus
}
#endif

#endif /* __MEMIF_TYPES_H */