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

#ifndef NVM_FSM_H
#define NVM_FSM_H

#include "nvm_types.h"
#include "nvm_cfg.h"
#include "nvm_fsmact.h"
#include "nvm_fsmevent.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_SET_WAIT_CONDITION_FLAG (0x01u) ///< 设置等待条件的内部标志
#define NVM_CLEAR_WAIT_CONDITION_FLAG (0xFEu) ///< 清除等待条件的内部标志
#define NVM_SET_ERROR_STATUS_FLAG (0x10u) ///< 设置错误状态的内部标志
#define NVM_CLEAR_ERROR_STATUS_FLAG (0xEFu) ///< 清除错误状态的内部标志
#define NVM_SET_DYN_CONFIG_MISMATCH_FLAG (0x20u) ///< 设置动态配置不匹配状态的内部标志
#define NVM_CLEAR_DYN_CONFIG_MISMATCH_FLAG (0xDFu) ///< 清除动态配置不匹配状态的内部标志

/**********************************************************************************************************************
 * @brief API 类型定义
 *********************************************************************************************************************/
typedef enum {
	NVM_STATE_UNINITIALIZED = 0, ///< 未初始化
	NVM_STATE_IDLE, ///< 空闲
	NVM_STATE_READALL, ///< 读取所有
	NVM_STATE_WRITEALL, ///< 写入所有
	NVM_STATE_READ_BLOCK, ///< 读取块
	NVM_STATE_WRITE_BLOCK, ///< 写入块
	NVM_STATE_RESTORE_BLOCK, ///< 恢复块
	NVM_STATE_INVALIDATE_NVBLOCK, ///< 使NV块无效
	NVM_STATE_ERASE_NVBLOCK, ///< 擦除NV块
} nvm_state_type;

/*!********************************************************************************************************************
 * @brief          NvM任务状态机的状态枚举
 *********************************************************************************************************************/
typedef enum {
	/* 任务状态机可能的状态 */
	NVM_FSM_UNINIT, ///< 未初始化状态。
	NVM_FSM_IDLE, ///< 空闲状态。
	NVM_FSM_HAVE_JOB, ///< 有作业可处理状态。
	NVM_FSM_HIGH_PRIO_JOB, ///< 高优先级作业状态。
	NVM_FSM_NOTHIGHPRIO_JOB, ///< 非高优先级作业状态。
	NVM_FSM_NORMAL_PRIO_JOB_1, ///< 普通优先级作业第一种状态。
	NVM_FSM_NORMAL_PRIO_JOB_2, ///< 普通优先级作业第二种状态。
	NVM_FSM_MULTI_BLOCK_JOB, ///< 多块作业状态。
	NVM_FSM_READ_INIT, ///< 读取操作初始化状态。
	NVM_FSM_READ_READ_DATA, ///< 读取数据的读取操作状态。
	NVM_FSM_READ_NV_FINISH, ///< NV读取完成状态。
	NVM_FSM_READ_CMP_CRC, ///< 读取操作中的CRC比较状态。
	NVM_FSM_READ_RETRY, ///< 重新读取操作状态。
	NVM_FSM_READ_RECOV, ///< 失败的读取操作恢复状态。
	NVM_FSM_WRITE_INIT, ///< 写入操作初始化状态。
	NVM_FSM_CRCCOMPARE, ///< 写入操作中的CRC比较状态。
	NVM_FSM_WRITE_DATA_1, ///< 写入数据第一部分状态。
	NVM_FSM_WRITE_DATA_2, ///< 写入数据第二部分状态。
	NVM_FSM_WRITE_RETRY, ///< 重新写入操作状态。
	NVM_FSM_WRITE_REDUNDANT, ///< 写入冗余块状态。
	NVM_FSM_RESTORE_INIT, ///< 恢复默认设置状态。
	NVM_FSM_WRITEALL_PROC_1, ///< 写入所有操作处理第一部分状态。
	NVM_FSM_WRITEALL_PROC_2, ///< 写入所有操作处理第二部分状态。
	NVM_FSM_READALL_PROC_1, ///< 读取所有操作处理第一部分状态。
	NVM_FSM_READALL_PROC_2, ///< 读取所有操作处理第二部分状态。
	NVM_FSM_INVALIDATE_INIT, ///< 开始使NV块无效操作状态。
	NVM_FSM_INVALIDATE_ING, ///< NV块无效操作进行中状态。
	NVM_FSM_INVALIDATE_REDUNDANT, ///< 使冗余块无效状态。
	NVM_FSM_ERASE_INIT, ///< 开始擦除操作状态。
	NVM_FSM_ERASE_ING, ///< 擦除操作进行中状态。
	NVM_FSM_ERASE_REDUNDANT, ///< 擦除冗余块状态。
	NVM_FSM_FSM_FINISHED ///< 状态机完成其序列的状态。
} nvm_fsm_state_type;

/*!********************************************************************************************************************
 * @brief          用于保存指向RAM数据的指针的类
 *********************************************************************************************************************/
typedef uint8 *nvm_ram_address_type;

/*!********************************************************************************************************************
 * @brief          NvM块信息结构
 *********************************************************************************************************************/
typedef struct {
	const nvm_block_descriptor_type *Descriptor_pt; ///< 指向块描述符的指针。
	nvm_admin_block_type *mngmt_ptr; ///< 指向RAM管理信息的指针。
	nvm_ram_address_type ram_addr; ///< 指向块的RAM地址的指针。
	uint16 nv_block_id; ///< 传递给下层模块的块ID。
	uint16 byte_count; ///< 用于块级复制的字节计数。
	nvm_request_result_type last_result; ///< 最后的作业结果。
	uint8 internal_flags; ///< 内部作业标志。
	nvm_state_type *nv_state; ///< 块的当前状态。
	uint8 retry_times; ///< 操作重试次数。
} nvm_block_info_type;

/*!********************************************************************************************************************
 * @brief           用于描述基于事件和动作的状态转换的结构
 *********************************************************************************************************************/
typedef struct {
	nvm_event_type queries; ///< 决定执行动作的查询。
	nvm_state_action_type actions; ///< 要执行的动作ID。
	nvm_fsm_state_type next_state; ///< 转换到的下一个状态。
} nvm_state_change_if_descr_type;

/*!********************************************************************************************************************
 * @brief           用于描述没有匹配条件时的最终状态转换的结构
 *********************************************************************************************************************/
typedef struct {
	nvm_state_action_type actions; ///< 要执行的动作ID。
	nvm_fsm_state_type next_state; ///< 转换到的下一个状态。
} nvm_state_change_else_descr_type;

/*!********************************************************************************************************************
 * @brief           用于描述可能的变化条件的状态的结构
 *********************************************************************************************************************/
typedef struct {
	nvm_state_change_if_descr_type change_conditions; ///< 所有带查询的条件。
	nvm_state_change_else_descr_type final_condition; ///< 没有查询的回退条件。
} nvm_state_descr_type;

/*!********************************************************************************************************************
 * @brief           NvM内部服务ID
 *********************************************************************************************************************/
#define NVM_INT_FID_WRITE_BLOCK (0x00u) ///< 写块操作的服务ID。
#define NVM_INT_FID_READ_BLOCK (0x01u) ///< 读块操作的服务ID。
#define NVM_INT_FID_RESTORE_DEFAULTS (0x02u) ///< 恢复默认操作的服务ID。
#define NVM_INT_FID_INVALIDATE_NV_BLOCK (0x03u) ///< 使NV块无效操作的服务ID。
#define NVM_INT_FID_ERASE_BLOCK (0x04u) ///< 擦除块操作的服务ID。
#define NVM_INT_FID_WRITE_ALL (0x05u) ///< 写入所有操作的服务ID。
#define NVM_INT_FID_READ_ALL (0x06u) ///< 读取所有操作的服务ID。
#define NVM_INT_FID_NO_JOB_PENDING (0x07u) ///< 表示没有待处理的作业。
#define NVM_INT_FID_INVALID_JOB (0x08u) ///< 取消的任务置为无效ID。

typedef uint8 nvm_internal_service_id_t;

/*!********************************************************************************************************************
 * @brief           要排队的NvM作业结构
 *********************************************************************************************************************/
typedef struct {
	nvm_block_id_t job_block_id; ///< NvM块ID。
	nvm_internal_service_id_t job_service_id; ///< 请求的作业。
	nvm_ram_address_type ram_addr; ///< 数据指针。
} nvm_job_t; ///< 要排队的作业的类型。

/*!********************************************************************************************************************
 * @brief           当前NvM块的信息结构
 *********************************************************************************************************************/
extern nvm_block_info_type nvm_current_block_info;
/*!********************************************************************************************************************
 * @brief           当前的NvM作业信息
 *********************************************************************************************************************/
extern nvm_job_t nvm_current_job;
/*!********************************************************************************************************************
 * @brief           NvM模块的API标志位
 *********************************************************************************************************************/
extern uint8 nvm_api_flags;

/*!********************************************************************************************************************
 * @brief           NvM服务ID表 包含所有内部服务ID的数组
 *********************************************************************************************************************/
extern const nvm_service_id_t nvm_service_table[NVM_INT_FID_INVALID_JOB + 1u];
/*!********************************************************************************************************************
 * @brief           NvM任务的当前状态
 *********************************************************************************************************************/
extern nvm_fsm_state_type nvm_task_state;
/*!********************************************************************************************************************
 * @brief           NvM子任务的当前状态
 *********************************************************************************************************************/
extern nvm_fsm_state_type nvm_subtask_state;

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          初始化NvM状态机
 * @details        设置NvM状态机到初始状态，为后续的NvM操作做准备。
 * @reentrant      非可重入
 * @synchronous    同步
 *********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_fsm_init(void);

/*!********************************************************************************************************************
 * @brief          反初始化NvM状态机
 * @details        设置NvM状态机到位初始状态，为NvM的结束做准备。
 * @reentrant      非可重入
 * @synchronous    同步
 *********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_fsm_deinit(void);

/*!********************************************************************************************************************
 * @brief          执行NvM状态机
 * @details        根据当前状态执行NvM状态机并返回下一个状态。
 * @param[in]      nvm_current_state 当前状态。
 * @retval         下一个要执行的NvM状态。
 * @reentrant      非可重入
 * @synchronous    同步
 *********************************************************************************************************************/
FUNC(nvm_fsm_state_type, NVM_CODE) nvm_fsm_excute(nvm_fsm_state_type nvm_current_state);
#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
#ifdef __cplusplus
}
#endif

#endif /* NVM_FSM_H */
