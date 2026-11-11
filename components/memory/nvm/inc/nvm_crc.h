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
#ifndef NVM_CRC_H
#define NVM_CRC_H

#include "nvm_cfg.h"
#include "nvm_types.h"
#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_START_SEC_CODE
#include "nvm_memmap.h"
/*!********************************************************************************************************************
 * @brief          获取块CRC长度
 * @details        返回指定NvM块的CRC长度。
 * @param[in]      block_ptr 指向块描述符的指针。
 * @retval         CRC长度的字节数。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint8, NVM_CODE) nvm_get_block_crc_len(const nvm_block_descriptor_type *block_ptr);

/*!********************************************************************************************************************
 * @brief          将CRC写入缓冲区
 * @details        基于管理区块和块描述信息将CRC写入指定的缓冲区。
 * @param[out]     buf_ptr 指向目标缓冲区的指针。
 * @param[in]      block_ptr 指向块描述符的指针。
 * @param[in]      adm_ptr 指向管理区块的指针。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE)
nvm_write_crc_to_buffer(uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, const nvm_admin_block_type *adm_ptr);

/*!********************************************************************************************************************
 * @brief          从缓冲区读取CRC
 * @details        从指定缓冲区读取并更新管理区块中的CRC信息。
 * @param[in]      buf_ptr 指向源缓冲区的指针。
 * @param[in]      block_ptr 指向块描述符的指针。
 * @param[out]     adm_ptr 指向管理区块的指针，用于更新CRC信息。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE)
nvm_read_crc_from_buffer(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, nvm_admin_block_type *adm_ptr);

/*!********************************************************************************************************************
 * @brief          计算读取操作的CRC
 * @details        计算给定缓冲区内容的CRC，并与管理区块中存储的CRC进行比较。
 * @param[in]      buf_ptr 指向源缓冲区的指针。
 * @param[in]      block_ptr 指向块描述符的指针。
 * @param[in]      adm_ptr 指向管理区块的指针，包含要比较的CRC信息。
 * @retval         TRUE 如果计算的CRC与管理区块中的CRC匹配。
 * @retval         FALSE 如果CRC不匹配。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, NVM_CODE)
nvm_verify_read_crc(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, const nvm_admin_block_type *adm_ptr);

/*!********************************************************************************************************************
 * @brief          计算写入操作的CRC
 * @details        计算给定缓冲区内容的CRC，并更新管理区块中的CRC信息。
 * @param[in]      buf_ptr 指向源缓冲区的指针。
 * @param[in]      block_ptr 指向块描述符的指针。
 * @param[out]     adm_ptr 指向管理区块的指针，用于更新CRC信息。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, NVM_CODE)
nvm_update_write_crc(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, nvm_admin_block_type *adm_ptr);

/*!********************************************************************************************************************
 * @brief          CRC比较
 * @details        比较给定缓冲区内容的CRC与管理区块中存储的CRC是否一致。
 * @param[in]      buf_ptr 指向源缓冲区的指针。
 * @param[in]      block_ptr 指向块描述符的指针。
 * @param[in]      adm_ptr 指向管理区块的指针，包含要比较的CRC信息。
 * @retval         TRUE 如果CRC匹配。
 * @retval         FALSE 如果CRC不匹配。
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, NVM_CODE)
nvm_check_crc(const uint8 *buf_ptr, const nvm_block_descriptor_type *block_ptr, const nvm_admin_block_type *adm_ptr);
#define NVM_STOP_SEC_CODE
#include "nvm_memmap.h"
#ifdef __cplusplus
}
#endif

#endif /* NVM_CRC_H */
