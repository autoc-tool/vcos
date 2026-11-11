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

#ifndef NVM_TYPES_H
#define NVM_TYPES_H

#include "Std_Types.h"
#include "nvm_cfg.h"
#include "Platform_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   NvM API 配置类别
 **********************************************************************************************************************/
#define NVM_API_CONFIG_CLASS_1 (0) ///< API配置类别1
#define NVM_API_CONFIG_CLASS_2 (1) ///< API配置类别2
#define NVM_API_CONFIG_CLASS_3 (2) ///< API配置类别3

#define NVM_SET_WRITE_ALL_FLAG (0x02u) ///< API标志：设置写入所有
#define NVM_CLEAR_WRITE_ALL_FLAG (0xFDu) ///< API标志：清除写入所有
#define NVM_SET_READ_ALL_FLAG (0x04u) ///< API标志：设置读取所有
#define NVM_CLEAR_READ_ALL_FLAG (0xFBu) ///< API标志：清除读取所有
#define NVM_SET_CANCEL_WRITE_ALL_FLAG (0x40u) ///< API标志：设置取消写入所有
#define NVM_CLEAR_CANCEL_WRITE_ALL_FLAG (0xBFu) ///< API标志：清除取消写入所有

/*!*********************************************************************************************************************
 * @brief  请求结果类型
 **********************************************************************************************************************/
#ifndef Rte_TypeDef_NvM_RequestResultType
typedef uint8 nvm_request_result_type;
#endif

/*!*********************************************************************************************************************
 * @brief  多块请求类型
 **********************************************************************************************************************/
#ifndef Rte_TypeDef_NvM_MultiBlockRequestType
typedef uint8 nvm_multi_block_request_t;
#endif

/*!*********************************************************************************************************************
 * @brief  块ID类型
 **********************************************************************************************************************/
#ifndef Rte_TypeDef_NvM_BlockIdType
typedef uint16 nvm_block_id_t;
#endif

/*!*********************************************************************************************************************
 * @brief 服务ID类型
 **********************************************************************************************************************/
#ifndef Rte_TypeDef_NvM_ServiceIdType
typedef uint8 nvm_service_id_t;
#endif

#ifndef NVM_REQ_OK
#define NVM_REQ_OK (0u) ///< 请求成功完成
#endif
#ifndef NVM_REQ_NOT_OK
#define NVM_REQ_NOT_OK (1u) ///< 请求未成功完成
#endif
#ifndef NVM_REQ_PENDING
#define NVM_REQ_PENDING (2u) ///< 请求正在处理中
#endif
#ifndef NVM_REQ_INTEGRITY_FAILED
#define NVM_REQ_INTEGRITY_FAILED (3u) ///< 请求的完整性失败
#endif
#ifndef NVM_REQ_BLOCK_SKIPPED
#define NVM_REQ_BLOCK_SKIPPED (4u) ///< 请求的块被跳过
#endif
#ifndef NVM_REQ_NV_INVALIDATED
#define NVM_REQ_NV_INVALIDATED (5u) ///< 请求的块被无效化
#endif
#ifndef NVM_REQ_CANCELED
#define NVM_REQ_CANCELED (6u) ///< 请求被取消
#endif
#ifndef NVM_REQ_REDUNDANCY_FAILED
#define NVM_REQ_REDUNDANCY_FAILED (7u) ///< 请求的冗余失败
#endif
#ifndef NVM_REQ_RESTORED_FROM_ROM
#define NVM_REQ_RESTORED_FROM_ROM (8u) ///< 请求的数据从ROM恢复
#endif

#ifndef NVM_INIT_SID
#define NVM_INIT_SID (0u) ///< NvM_Init的服务ID
#endif
#ifndef NVM_SET_DATA_INDEX_SID
#define NVM_SET_DATA_INDEX_SID (1u) ///< NvM_SetDataIndex的服务ID
#endif
#ifndef NVM_GET_DATA_INDEX_SID
#define NVM_GET_DATA_INDEX_SID (2u) ///< NvM_GetDataIndex的服务ID
#endif
#ifndef NVM_SET_BLOCK_PROTECTION_SID
#define NVM_SET_BLOCK_PROTECTION_SID (3u) ///< NvM_SetBlockProtection的服务ID
#endif
#ifndef NVM_GET_ERROR_STATUS_SID
#define NVM_GET_ERROR_STATUS_SID (4u) ///< NvM_GetErrorStatus的服务ID
#endif
#ifndef NVM_SET_RAM_BLOCK_STATUS_SID
#define NVM_SET_RAM_BLOCK_STATUS_SID (5u) ///< NvM_SetRamBlockStatus的服务ID
#endif
#ifndef NVM_READ_BLOCK_SID
#define NVM_READ_BLOCK_SID (6u) ///< NvM_ReadBlock的服务ID
#endif
#ifndef NVM_WRITE_BLOCK
#define NVM_WRITE_BLOCK (7u) ///< NvM_WriteBlock的服务ID
#endif
#ifndef NVM_RESTORE_BLOCK_DEFAULTS_SID
#define NVM_RESTORE_BLOCK_DEFAULTS_SID (8u) ///< NvM_RestoreBlockDefaults的服务ID
#endif
#ifndef NVM_ERASE_BLOCK_SID
#define NVM_ERASE_BLOCK_SID (9u) ///< NvM_EraseBlock的服务ID
#endif

#ifndef NVM_CANCEL_WRITE_ALL_SID
#define NVM_CANCEL_WRITE_ALL_SID (10u) ///< NvM_CancelWriteAll的服务ID
#endif
#ifndef NVM_INVALIDATE_NV_BLOCK_SID
#define NVM_INVALIDATE_NV_BLOCK_SID (11u) ///< NvM_InvalidateNvBlock的服务ID
#endif
#ifndef NVM_READ_ALL_SID
#define NVM_READ_ALL_SID (12u) ///< NvM_ReadAll的服务ID
#endif
#ifndef NVM_WRITE_ALL
#define NVM_WRITE_ALL (13u) ///<  NvM_WriteAll的服务ID
#endif
#ifndef NVM_MAIN_FUNCTION_SID
#define NVM_MAIN_FUNCTION_SID (14u) ///< NvM_MainFunction的服务ID
#endif
#ifndef NVM_GET_VERSION_INFO_SID
#define NVM_GET_VERSION_INFO_SID (15u) ///< NvM_GetVersionInfo的服务ID
#endif

#ifndef NVM_CANCEL_JOBS_SID
#define NVM_CANCEL_JOBS_SID (16u) ///< NvM_CancelJobs的服务ID
#endif
#ifndef NVM_VALIDATE_ALL
#define NVM_VALIDATE_ALL (17u) ///< NvM_ValidateAll的服务ID
#endif
#ifndef NVM_FIRST_INIT_ALL
#define NVM_FIRST_INIT_ALL (18u) ///< NvM_FirstInitAll的服务ID
#endif
#ifndef NVM_SET_BLOCK_LOCK_STATUS_SID
#define NVM_SET_BLOCK_LOCK_STATUS_SID (19u) ///< NvM_SetBlockLockStatus的服务ID
#endif

/*!********************************************************************************************************************
 * @brief          定义NvM位字段类型
 **********************************************************************************************************************/
typedef uint32 nvm_bitfield_type;

/*!********************************************************************************************************************
 * @brief          定义单个NvM块操作完成后的回调函数类型
 * @details        当单个NvM块操作 完成后调用此回调函数 以通知操作的结果
 * @param[in]      service_id 该操作的服务ID
 * @param[in]      job_result 操作完成的结果状态
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*nvm_single_block_callback_func_type)(uint8 service_id, nvm_request_result_type job_result);

/*!********************************************************************************************************************
 * @brief          定义多个NvM块操作完成后的回调函数类型
 * @details        在NvM进行多块操作 完成后调用此回调函数 以通知操作的结果
 * @param[in]      service_id 多块操作的服务ID
 * @param[in]      job_result 多块操作完成的结果状态
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
typedef void (*nvm_multi_block_callback_func_t)(nvm_multi_block_request_t service_id,
						   nvm_request_result_type job_result);

/*!********************************************************************************************************************
 * @brief          定义NvM块初始化操作的回调函数类型
 * @details        在NvM块初始化完成后调用此回调函数
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*nvm_init_block_callback_func_type)(void);

/*!********************************************************************************************************************
 * @brief          定义从NvM到RAM的读操作回调函数类型
 * @details        当读取NvM数据到RAM完成后调用此回调函数
 * @param[in]      nvram_buffer 指向存储读取数据的RAM缓冲区的指针
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*nvm_read_ram_from_nvm_cbk_ptr_type)(const void *nvram_buffer);

/*!********************************************************************************************************************
 * @brief          定义从RAM到NvM的写操作回调函数类型
 * @details        当写入数据从RAM到NvM完成后调用此回调函数
 * @param[in]      nvram_buffer 指向存储需要写入NvM数据的RAM缓冲区的指针
 * @retval         Std_ReturnType 操作的返回状态 E_OK或E_NOT_OK
 **********************************************************************************************************************/
typedef Std_ReturnType (*nvm_write_ram_to_nvm_cbk_ptr_type)(void *nvram_buffer);

/*!*********************************************************************************************************************
 * @brief   定义用于存储CRC校验值的联合体
 **********************************************************************************************************************/
union nvm_crc_u {
	uint16 crc16; ///< 16位CRC校验值
	uint32 crc32; ///< 32位CRC校验值
};

/*!*********************************************************************************************************************
 * @brief   定义NvM管理块结构
 **********************************************************************************************************************/
typedef struct {
	uint8 data_index; ///< 如果是“数据集”类型，选中的数据索引
	nvm_request_result_type error_status; ///< 块的状态
	uint8 attributes; ///< 块属性
	boolean crc_compare_valid; ///< CRC比较结果是否有效
	union nvm_crc_u ram_crc; ///< RAM中的CRC校验值
	union nvm_crc_u crc_compare_val; ///< 用于CRC比较的数据值
} nvm_admin_block_type;

/*!********************************************************************************************************************
 * @brief          NvM块描述符类型 描述了NvM块的配置、重试策略、CRC管理和基本信息。
 **********************************************************************************************************************/
typedef struct {
	uint8 block_job_priority; ///< 块作业优先级
	uint8 nv_block_num; ///< NV块号
	uint8 rom_block_num; ///< ROM块号
	uint8 nvram_device_id; ///< NVRAM设备ID

	uint8 max_num_of_read_retries; ///< 最大读重试次数
	uint8 max_num_of_write_retries; ///< 最大写重试次数

	nvm_bitfield_type block_management_type : 2; ///< 块管理类型
	nvm_bitfield_type block_crc_type : 2; ///< 块CRC类型
	nvm_bitfield_type block_use_crc : 1; ///< 是否使用CRC
	nvm_bitfield_type calc_ram_block_crc : 1; ///< 是否计算RAM块CRC
	nvm_bitfield_type resistant_to_changes_sw : 1; ///< 是否对软件更改有弹性
	nvm_bitfield_type notify_bsw : 1; ///< 是否通知BSW

	uint16 nv_block_length; ///< NV块长度
	uint16 nvram_block_identifier; ///< Nvram块标识符
	uint16 nv_block_base_number; ///< NV块基础号码
	uint16 flags; ///< 标志

	uint8 *ram_block_data_address; ///< RAM块数据地址
	uint8 *rom_block_data_adress; ///< ROM块数据地址
	nvm_write_ram_to_nvm_cbk_ptr_type cbk_get_mirror_func_pt; ///< 获取镜像回调函数指针
	nvm_read_ram_from_nvm_cbk_ptr_type cbk_set_mirror_func_pt; ///< 设置镜像回调函数指针
	nvm_init_block_callback_func_type init_block_callback; ///< 块初始化回调函数
	nvm_single_block_callback_func_type single_block_callback; ///< 单块回调函数

} nvm_block_descriptor_type;

#ifdef __cplusplus
}
#endif

#endif /* NVM_TYPES_H */
