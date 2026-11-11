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

#ifndef __MEMIF_H
#define __MEMIF_H

#include "memif_types.h"
#include "memif_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEMIF_E_NO_ERROR (0x00u) ///< 未发生错误
#define MEMIF_E_PARAM_DEVICE (0x01u) ///< 无效的设备参数
#define MEMIF_E_PARAM_POINTER (0x02u) ///< 无效的指针参数

#define MEMIF_SET_MODE_SID (0x01u) ///< 设置模式服务ID
#define MEMIF_READ_SID (0x02u) ///< 读取数据服务ID
#define MEMIF_WRITE_SID (0x03u) ///< 写入数据服务ID
#define MEMIF_CANCEL_SID (0x04u) ///< 取消操作服务ID
#define MEMIF_GET_STATUS_SID (0x05u) ///< 获取状态服务ID
#define MEMIF_GET_JOB_RESULT_SID (0x06u) ///< 获取任务结果服务ID
#define MEMIF_INVALIDATE_BLOCK_SID (0x07u) ///< 标记块无效服务ID
#define MEMIF_GET_VERSION_INFO_SID (0x08u) ///< 获取版本信息服务ID
#define MEMIF_ERASE_SID (0x09u) ///<块服务ID

#define MEMIF_BROADCAST_ID (0xFFU) /// API 定义ID

/*!********************************************************************************************************************
 * @brief          从EA/FEE中读取数据
 * @details        从指定 device_index 的设备中的 Block 读取数据到 data_buffer_ptr 里
 * @param[in]      device_index 设备索引
 * @param[in]      block_number 块编号
 * @param[in]      block_offset 块偏移量
 * @param[in]      data_buffer_ptr 指向目标RAM缓冲区的指针
 * @param[in]      length 数据长度
 * @retval         E_OK 请求被接受
 * @retval         E_NOT_OK 请求未被接受
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
Std_ReturnType memif_read(uint8 device_index, uint16 block_number, uint16 block_offset, uint8 *data_buffer_ptr,
			  uint16 length);

/*!********************************************************************************************************************
 * @brief          向EA/FEE写入数据
 * @details        将 data_buffer_ptr 中的内容写入到指定 device_index 设备的 Block 中
 * @param[in]      device_index 设备索引
 * @param[in]      block_number 块编号
 * @param[in]      data_buffer_ptr 指向源RAM缓冲区的指针
 * @retval         E_OK 请求被接受
 * @retval         E_NOT_OK 请求未被接受
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
Std_ReturnType memif_write(uint8 device_index, uint16 block_number, uint8 *data_buffer_ptr);

/*!********************************************************************************************************************
 * @brief          取消异步操作
 * @details        取消指定 device_index 设备上的异步操作
 * @param[in]      device_index 设备索引
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
void memif_cancel(uint8 device_index);

/*!********************************************************************************************************************
 * @brief          获取设备状态
 * @details        获取指定 device_index 设备状态，也可获取所有设备状态（使用 Broadcast ID）
 * @param[in]      device_index 设备索引
 * @retval         MemIf_StatusType 底层设备状态
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
MemIf_StatusType memif_get_status(uint8 device_index);

/*!********************************************************************************************************************
 * @brief          获取任务执行结果
 * @details        获取指定 device_index 设备上的任务执行结果
 * @param[in]      device_index 设备索引
 * @retval         MemIf_JobResultType 任务执行结果
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
MemIf_JobResultType memif_get_job_result(uint8 device_index);

/*!********************************************************************************************************************
 * @brief          将 Block 标记为无效
 * @details        将指定 device_index 设备的 Block 标记为无效
 * @param[in]      device_index 设备索引
 * @param[in]      block_number 块编号
 * @retval         E_OK 请求被接受
 * @retval         E_NOT_OK 请求未被接受
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
Std_ReturnType memif_invalidate_block(uint8 device_index, uint16 block_number);

/*!********************************************************************************************************************
 * @brief          擦除 Block
 * @details        将指定 device_index 设备的 Block 擦除，仅能擦除优先级为 Immediate 的 Block
 * @param[in]      device_index 设备索引
 * @param[in]      block_number 块编号
 * @retval         E_OK 请求被接受
 * @retval         E_NOT_OK 请求未被接受
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
Std_ReturnType memif_erase_immediate_block(uint8 device_index, uint16 block_number);

/*!********************************************************************************************************************
 * @brief          设置设备运行模式
 * @details        设置所有设备驱动的运行模式
 * @param[in]      mode 操作模式
 * @reentrant      非可重入
 * @synchronous    异步
 **********************************************************************************************************************/
void memif_set_mode(MemIf_ModeType mode);

#ifdef __cplusplus
}
#endif

#endif /* __MEMIF_H */