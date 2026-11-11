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

#ifndef NVM_INTERNAL_H
#define NVM_INTERNAL_H

#include "Std_Types.h"
#include "nvm_types.h"
#include "nvm_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_BLOCK_NATIVE (0x00u) ///< 原生块类型
#define NVM_BLOCK_REDUNDANT (0x01u) ///< 冗余块类型
#define NVM_BLOCK_DATASET (0x02u) ///< 数据集块类型

#define NVM_CRC_OFF (0x00u) ///< CRC校验关闭
#define NVM_CRC16_ON (0x01u) ///< 16位CRC校验开启
#define NVM_CRC32_ON (0x02u) ///< 32位CRC校验开启

#define NVM_VALID_STATE_SET (0x01u) ///< 设置有效状态
#define NVM_VALID_STATE_CL (0xFEu) ///< 清除有效状态
#define NVM_CHANGED_STATE_SET (0x02u) ///< 设置更改状态
#define NVM_CHANGED_STATE_CL (0xFDu) ///< 清除更改状态

#define NVM_SET_LOCK_STATUS (0x40u) ///< 设置锁定状态
#define NVM_CLEAR_LOCK_STATUS (0xBFu) ///< 清除锁定状态
#define NVM_SET_WRITE_PROTECTION_STATUS (0x80u) ///< 设置写保护状态
#define NVM_CLEAR_WRITE_PROTECTION_STATUS (0x7Fu) ///< 清除写保护状态

#define NVM_CALC_RAM_BLOCK_CRC_ON (1u) ///< 启用RAM块CRC校验
#define NVM_CALC_RAM_BLOCK_CRC_OFF (0u) ///< 禁用RAM块CRC校验
#define NVM_BLOCK_WRITE_PROT_ON (2u) ///< 启用块写保护
#define NVM_BLOCK_WRITE_PROT_OFF (0u) ///< 禁用块写保护
#define NVM_BLOCK_WRITE_BLOCK_ONCE_ON (4u) ///< 启用写入块一次
#define NVM_BLOCK_WRITE_BLOCK_ONCE_OFF (0u) ///< 禁用写入块一次
#define NVM_RESISTANT_TO_CHANGED_SW_ON (8u) ///< 启用变更软件阻力
#define NVM_RESISTANT_TO_CHANGED_SW_OFF (0u) ///< 禁用变更软件阻力
#define NVM_SELECT_BLOCK_FOR_READALL_ON (16u) ///< 选择块用于ReadAll
#define NVM_SELECT_BLOCK_FOR_READALL_OFF (0u) ///< 取消选择块用于ReadAll
#define NVM_SELECT_BLOCK_FOR_WRITEALL_ON (32u) ///< 选择块用于WriteAll
#define NVM_SELECT_BLOCK_FOR_WRITEALL_OFF (0u) ///< 取消选择块用于WriteAll
#define NVM_CBK_DURING_READALL_ON (64u) ///< 在ReadAll期间启用回调函数
#define NVM_CBK_DURING_READALL_OFF (0u) ///< 在ReadAll期间禁用回调函数
#define NVM_CRC_COMP_MECHANISM_ON (128u) ///< 启用CRC比较机制
#define NVM_CRC_COMP_MECHANISM_OFF (0u) ///< 禁用CRC比较机制
#define NVM_BLOCK_USE_SET_RAM_BLOCK_STATUS_ON (256u) ///< 启用SetRamBlockStatus
#define NVM_BLOCK_USE_SET_RAM_BLOCK_STATUS_OFF (0u) ///< 禁用SetRamBlockStatus

extern uint16 nvm_compiled_config_id;
extern void nvm_multi_block_callback(nvm_multi_block_request_t service_id, nvm_request_result_type job_result);
extern const nvm_block_descriptor_type nvm_block_descriptor_list[NVM_NUM_OF_NVRAM_BLOCKS];
extern uint8 nvm_work_buffer[NVM_MAX_BLOCK_LENGTH];
extern nvm_admin_block_type nvm_admin_block_table[NVM_NUM_OF_NVRAM_BLOCKS];

typedef struct {
	boolean is_retry;
	uint8 retry_test_threshold;
	uint8 current_retry;
} test_nvm_retry_t;

extern test_nvm_retry_t test_nvm_retry;
#ifdef __cplusplus
}
#endif

#endif /*NVM_INTERNAL_H*/
