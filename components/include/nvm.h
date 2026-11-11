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

#ifndef __NVM_H
#define __NVM_H

#include "nvm_cfg.h"
#include "nvm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_MODULE_ID (20u) ///< NVM模块的标识符
#define NVM_INSTANCE_ID (0u) ///< 实例标识符，如果存在多个NVM实例时使用

#define NVM_SW_MAJOR_VERSION (0u) ///< NVM软件组件的主要版本号
#define NVM_SW_MINOR_VERSION (0u) ///< NVM软件组件的次要版本号
#define NVM_SW_PATCH_VERSION (1u) ///< NVM软件组件的补丁级别

#define NVM_E_NO_ERROR (0x00u) ///< 未发生错误
#define NVM_E_PARAM_BLOCK_ID (0x0Au) ///< 无效的块ID参数
#define NVM_E_PARAM_BLOCK_TYPE (0x0Bu) ///< 无效的块类型参数
#define NVM_E_PARAM_BLOCK_DATA_IDX (0x0Cu) ///< 无效的块数据索引参数
#define NVM_E_PARAM_ADDRESS (0x0Du) ///< 无效的地址参数
#define NVM_E_PARAM_DATA (0x0Eu) ///< 无效的数据参数
#define NVM_E_PARAM_POINTER (0x0Fu) ///< 无效的指针参数
#define NVM_E_BLOCK_WITHOUT_DEFAULTS (0x11u) ///< 块没有默认值

#define NVM_E_NOT_INITIALIZED (0x14u) ///< NVM未初始化
#define NVM_E_BLOCK_PENDING (0x15u) ///< 块待处理操作
#define NVM_E_BLOCK_CONFIG (0x18u) ///< 块的配置错误
#define NVM_E_BLOCK_LOCKED (0x19u) ///< 块已锁定

#define NVM_E_LIST_OVERFLOW (0x50u) ///< 块列表溢出
#define NVM_E_NV_WRITE_PROTECTED (0x51u) ///< 非易失性内存写保护

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          NvM 初始化
 * @details        初始化 NvM 内部变量和设置为其默认状态，配置模块所需的资源，为后续操作提供一致的起始状态。
 * @reentrant      非可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_init(void);

/*!********************************************************************************************************************
 * @brief          NvM 反初始化
 * @details        反初始化 NvM 内部变量和设置为未初始化状态，使其回到未初始化的状态，为关闭或重启系统做准备。
 * @reentrant      非可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_deinit(void);

/*!********************************************************************************************************************
 * @brief          创建ReadAll Job
 * @details        请求读取所有配置块（Block），将数据从非易失性存储读取到 RAM，通常在系统启动时调用。
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_read_all(void);

/*!********************************************************************************************************************
 * @brief          创建WriteAll Job
 * @details        请求将所有RAM块（Block）写入到相应的非易失性存储（NV Block）中，通常用于系统休眠时保存当前运行时的数据。
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_write_all(void);

/*!********************************************************************************************************************
 * @brief          取消WriteAll job
 * @details        取消当前进行的WriteAll操作，如果可能的话，以避免不必要的数据写入操作。
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_cancel_write_all(void);

/*!********************************************************************************************************************
 * @brief          设置 NvM block dataindex
 * @details        该请求将设置 dataset 类型的 NVRAM block 的 dataindex，用于选择性地访问或修改相关数据集。
 * @param[in]      block_id 存储块的标识符。
 * @param[in]      data_index 要设置的数据索引值。
 * @retval         E_OK dataindex 成功设置。
 * @retval         E_NOT_OK 设置失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_set_data_index(nvm_block_id_t block_id, uint8 data_index);

/*!********************************************************************************************************************
 * @brief          获取 NvM block dataindex
 * @details        该请求将获取 dataset 类型的 NVRAM block 的 dataindex，帮助上层应用程序确定被操作的数据集。
 * @param[in]      block_id 存储块的标识符。
 * @param[in,out]  data_index_ptr 指向变量的指针，该变量将更新为获取到的数据索引值。
 * @retval         E_OK dataindex 成功获取。
 * @retval         E_NOT_OK 获取失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_get_data_index(nvm_block_id_t block_id, uint8 *data_index_ptr);

/*!********************************************************************************************************************
 * @brief          设置 NvM block 锁定状态
 * @details        更改 NV 块的锁定状态，可以防止对关键数据的意外修改，确保数据保密性和安全性。
 * @param[in]      block_id 存储块的标识符。
 * @param[in]      block_locked 锁定状态，true为锁定，false为解锁。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_set_block_lock_status(nvm_block_id_t block_id, boolean block_locked);

/*!********************************************************************************************************************
 * @brief          控制 NvM 主功能的执行
 * @details        提供独立于全局状态的信息，返回指定块的错误或成功状态，以帮助诊断和处理块级操作问题。
 * @param[in]      enable_execute 启用或禁用执行的标志。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE) nvm_control_main_function(boolean enable_execute);

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_1)

/*!********************************************************************************************************************
 * @brief          获取 NvM block 结果状态
 * @details        该请求将获取每个 block 独立的 error/status 信息
 * @param[in]      block_id 存储块的标识符。
 * @param[in,out]  request_result 指向请求结果的指针。
 * @retval         E_OK 状态成功获取。
 * @retval         E_NOT_OK 获取失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_get_error_status(nvm_block_id_t block_id, nvm_request_result_type *request_result);

#if (NVM_ENABLE_SET_RAM_BLOCK_STATUS_API == STD_ON)

/*!********************************************************************************************************************
 * @brief          设置 NvM block 中 RAM Block 状态
 * @details        该请求将设置 permanent RAM block 状态，包括 valid/invalid。
 * @param[in]      block_id 存储块的标识符。
 * @param[in]      block_changed RAM块状态改变标志。
 * @retval         E_OK 成功设置 RAM block 状态。
 * @retval         E_NOT_OK 未能成功设置 RAM block 状态。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
Std_ReturnType nvm_set_ram_block_status(nvm_block_id_t block_id, boolean block_changed);
#endif

/*!********************************************************************************************************************
 * @brief          读取指定的 NvM block
 * @details        请求将 NV block 中的数据读取到所对应的 RAM block 中。
 * @param[in]      block_id 存储块的标识符。
 * @param[in]      dst_ptr 指向目标RAM块的指针。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_read_block(nvm_block_id_t block_id, void *dst_ptr);

/*!********************************************************************************************************************
 * @brief          将数据写入到 NvM block
 * @details        请求将 RAM block 中的数据写入到所对应的 NV block 中。
 * @param[in]      block_id 存储块的标识符。
 * @param[in]      src_ptr 指向源RAM块的指针。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_write_block(nvm_block_id_t block_id, const void *src_ptr);

/*!********************************************************************************************************************
 * @brief          将 NvM block 中的数据恢复为默认值
 * @details        使用 ROM block 中的数据拷贝到相对应的 RAM block 中。
 * @param[in]      block_id 存储块的标识符。
 * @param[in]      dst_ptr 指向目标RAM块的指针。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_restore_block_defaults(nvm_block_id_t block_id, void *dst_ptr);

/*!********************************************************************************************************************
 * @brief          取消处于等待状态下的任务
 * @details        将指定 Block 相关的所有任务都取消。
 * @param[in]      block_id 指定的非易失性内存（NvM）块的标识符。
 * @retval         E_OK 成功取消。
 * @retval         E_NOT_OK 操作失败。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_cancel_jobs(nvm_block_id_t block_id);

/*!********************************************************************************************************************
 * @brief          读取指定的 NvM block
 * @details        请求将 NV block 中的数据读取到所配置的 RAM block 中。
 * @param[in]      block_id 存储块的唯一标识符。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_read_pram_block(nvm_block_id_t block_id);

/*!********************************************************************************************************************
 * @brief          将数据写入到 NvM block
 * @details        请求将配置的 RAM block 中的数据写入到所对应的 NV block 中。
 * @param[in]      block_id 存储块的唯一标识符。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_write_pram_block(nvm_block_id_t block_id);

/*!********************************************************************************************************************
 * @brief          将 NvM block 中的数据恢复为默认值
 * @details        使用配置的 ROM block 中的数据拷贝到相对应的 RAM block 中。
 * @param[in]      block_id 存储块的唯一标识符。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_restore_pram_block_defaults(nvm_block_id_t block_id);

#endif

#if (NVM_API_CONFIG_CLASS > NVM_API_CONFIG_CLASS_2)
/*!********************************************************************************************************************
 * @brief          设置 NvM Block 写保护
 * @details        设置 NvM Block 为写保护后将禁止该 Block 被写入和擦除。
 * @param[in]      block_id 存储块的唯一标识符。
 * @param[in]      protection_enabled 是否启用写保护。
 * @retval         E_OK 写保护设置成功。
 * @retval         E_NOT_OK 写保护设置失败。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_set_block_protection(nvm_block_id_t block_id, boolean protection_enabled);

/*!********************************************************************************************************************
 * @brief          创建擦除指定 NV block 的任务
 * @details        发起请求以清除指定的 NV 存储块内容，但不影响其对应的 RAM 块数据。该步骤通常为Immediate Block写入的前置调用。
 * @param[in]      block_id 存储块的唯一标识符。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_erase_nv_block(nvm_block_id_t block_id);

/*!********************************************************************************************************************
 * @brief          创建将指定 NV block 标记为无效的任务
 * @details        此函数发起请求以将指定的存储块标记为无效，确保不再被访问或使用，以节省存储资源。
 * @param[in]      block_id 存储块的唯一标识符。
 * @retval         E_OK 成功创建请求。
 * @retval         E_NOT_OK 未能成功创建请求。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, NVM_CODE) nvm_invalidate_nv_block(nvm_block_id_t block_id);

#endif

#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* __NVM_H */
