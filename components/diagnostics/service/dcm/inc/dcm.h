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

#ifndef DCM_H
#define DCM_H

#include "dcm_types.h"
#include "dcm_cbk.h"
#include "dcm_lcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

/*!*********************************************************************************************************************
 * @brief          dcm初始化
 * @details        dcm初始化
 * @param[in]      config_ptr 配置指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_init(const dcm_config_type *config_ptr);

/*!*********************************************************************************************************************
 * @brief          dcm去初始化
 * @details        dcm去初始化
 * @param[in]      value 去初始化值
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_de_init(uint8 value);

/*!*********************************************************************************************************************
 * @brief          dcm写内存
 * @details        dcm写内存
 * @param[in]      op_status 状态
 * @param[in]      memory_identifier 内存标识
 * @param[in]      memory_address 内存地址
 * @param[in]      memory_size 内存大小
 * @param[in]      memory_data 内存数据
 * @retval         0-255 拷贝接收数据成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(dcm_return_write_memory_type, DCM_CODE)
dcm_write_memory(dcm_op_status_type op_status, uint8 memory_identifier, uint32 memory_address, uint32 memory_size,
	const uint8 *memory_data);

/*!*********************************************************************************************************************
 * @brief          dcm读内存
 * @details        dcm读内存
 * @param[in]      op_status 状态
 * @param[in]      memory_identifier 内存标识
 * @param[in]      memory_address 内存地址
 * @param[in]      memory_size 内存大小
 * @param[in]      memory_data 内存数据
 * @retval         0-255 拷贝接收数据成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(dcm_return_read_memory_type, DCM_CODE)
dcm_read_memory(dcm_op_status_type op_status, uint8 memory_identifier, uint32 memory_address, uint32 memory_size,
	uint8 *memory_data);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求下载
 * @details        dcm处理请求下载
 * @param[in]      op_status 状态
 * @param[in]      data_format_identifier 数据格式标识
 * @param[in]      memory_address 内存地址
 * @param[in]      memory_size 内存大小
 * @param[in]      block_length 块长度
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求下载成功
 * @retval         E_NOT_OK dcm处理请求下载不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_download(dcm_op_status_type op_status, uint8 data_format_identifier, uint32 memory_address,
	uint32 memory_size, uint32 *block_length, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求上传
 * @details        dcm处理请求上传
 * @param[in]      op_status 状态
 * @param[in]      data_format_identifier 数据格式标识
 * @param[in]      memory_address 内存地址
 * @param[in]      memory_size 内存大小
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求上传成功
 * @retval         E_NOT_OK dcm处理请求上传不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_upload(dcm_op_status_type op_status, uint8 data_format_identifier, uint32 memory_address,
	uint32 memory_size, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求转换退出
 * @details        dcm处理请求转换退出
 * @param[in]      op_status 状态
 * @param[in]      parameter_record 参数记录
 * @param[in]      parameter_record_size 参数记录大小
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求转换退出成功
 * @retval         E_NOT_OK dcm处理请求转换退出不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_transfer_exit(dcm_op_status_type op_status, const uint8 *parameter_record,
	uint32 parameter_record_size, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求地址文件
 * @details        dcm处理请求地址文件
 * @param[in]      op_status 状态
 * @param[in]      file_path_and_name_length 文件路径和名称长度
 * @param[in]      file_path_and_name 文件路径和名称
 * @param[in]      data_format_identifier 数据格式标识
 * @param[in]      file_size_uncompressed 文件未压缩大小
 * @param[in]      file_size_compressed 文件压缩大小
 * @param[in]      max_number_of_block_length 块长度最大个数
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求地址文件成功
 * @retval         E_NOT_OK dcm处理请求地址文件不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_add_file(dcm_op_status_type op_status, uint16 file_path_and_name_length,
	const uint8 *file_path_and_name, uint8 data_format_identifier,
	uint32 file_size_uncompressed, uint32 file_size_compressed,
	uint32 *max_number_of_block_length, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求删除文件
 * @details        dcm处理请求删除文件
 * @param[in]      op_status 状态
 * @param[in]      file_path_and_name_length 文件路径和名称长度
 * @param[in]      file_path_and_name 文件路径和名称
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求地址文件成功
 * @retval         E_NOT_OK dcm处理请求地址文件不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_delete_file(dcm_op_status_type op_status, uint16 file_path_and_name_length,
	const uint8 *file_path_and_name, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求替换文件
 * @details        dcm处理请求替换文件
 * @param[in]      op_status 状态
 * @param[in]      file_path_and_name_length 文件路径和名称长度
 * @param[in]      file_path_and_name 文件路径和名称
 * @param[in]      data_format_identifier 数据格式标识
 * @param[in]      file_size_uncompressed 文件未压缩大小
 * @param[in]      file_size_compressed 文件压缩大小
 * @param[in]      max_number_of_block_length 块长度最大个数
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求替换文件成功
 * @retval         E_NOT_OK dcm处理请求替换文件不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_replace_file(dcm_op_status_type op_status, uint16 file_path_and_name_length,
	const uint8 *file_path_and_name, uint8 data_format_identifier,
	uint32 file_size_uncompressed, uint32 file_size_compressed,
	uint32 *max_number_of_block_length, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求读文件
 * @details        dcm处理请求读文件
 * @param[in]      op_status 状态
 * @param[in]      file_path_and_name_length 文件路径和名称长度
 * @param[in]      file_path_and_name 文件路径和名称
 * @param[in]      data_format_identifier 数据格式标识
 * @param[in]      file_size_uncompressed 文件未压缩大小
 * @param[in]      file_size_compressed 文件压缩大小
 * @param[in]      max_number_of_block_length 块长度最大个数
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求替换文件成功
 * @retval         E_NOT_OK dcm处理请求替换文件不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_read_file(dcm_op_status_type op_status, uint16 file_path_and_name_length,
	const uint8 *file_path_and_name, uint8 data_format_identifier,
	uint32 *file_size_uncompressed, uint32 *file_size_compressed,
	uint32 *max_number_of_block_length, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求读路径
 * @details        dcm处理请求读路径
 * @param[in]      op_status 状态
 * @param[in]      file_path_and_name_length 文件路径和名称长度
 * @param[in]      file_path_and_name 文件路径和名称
 * @param[in]      dir_info_length 路径信息长度
 * @param[in]      max_number_of_block_length 块长度最大个数
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求读路径成功
 * @retval         E_NOT_OK dcm处理请求读路径不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_process_request_read_dir(dcm_op_status_type op_status, uint16 file_path_and_name_length,
	const uint8 *file_path_and_name, uint32 *dir_info_length,
	uint32 *max_number_of_block_length, dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm处理请求写文件
 * @details        dcm处理请求写文件
 * @param[in]      op_status 状态
 * @param[in]      data_length 数据长度
 * @param[in]      data 数据
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm处理请求读路径成功
 * @retval         E_NOT_OK dcm处理请求读路径不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_write_file(dcm_op_status_type op_status, uint32 data_length, uint8 *data,
	dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm读文件或者路径
 * @details        dcm读文件或者路径
 * @param[in]      op_status 状态
 * @param[in]      data_length 数据长度
 * @param[in]      data 数据
 * @param[in]      error_code 错误码
 * @retval         E_OK dcm读文件或者路径成功
 * @retval         E_NOT_OK dcm读文件或者路径不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dcm_read_file_or_dir(dcm_op_status_type op_status, uint32 *data_length, uint8 *data,
	dcm_nrc_type *error_code);

/*!*********************************************************************************************************************
 * @brief          dcm获取下载响应参数记录
 * @details        dcm获取下载响应参数记录
 * @param[in]      parameter_record_length 参数记录长度
 * @param[in]      data 数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dcm_get_download_response_parameter_record(const uint8 *data, PduLengthType *parameter_record_length);

/*!*********************************************************************************************************************
 * @brief          dcm获取转换退出响应参数记录
 * @details        dcm获取转换退出响应参数记录
 * @param[in]      parameter_record_length 参数记录长度
 * @param[in]      data 数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dcm_get_transfer_exit_response_parameter_record(const uint8 *data, PduLengthType *parameter_record_length);

/*!*********************************************************************************************************************
 * @brief          dcm设置程序条件
 * @details        dcm设置程序条件
 * @param[in]      prog_conditions 条件
 * @retval         E_OK dcm读文件或者路径成功
 * @retval         E_NOT_OK dcm读文件或者路径不成功
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_set_prog_conditions(const dcm_prog_conditions_type *prog_conditions);

/*!*********************************************************************************************************************
 * @brief          dcm获取程序条件
 * @details        dcm获取程序条件
 * @param[in]      prog_conditions 条件
 * @retval         E_OK dcm获取程序条件成功
 * @retval         E_NOT_OK dcm获取程序条件不成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(dcm_ecu_start_mode_type, DCM_CODE) dcm_get_prog_conditions(dcm_prog_conditions_type *prog_conditions);

#ifdef DCM_CALLBACK_SESSIONCONTROL_ENABLED
/*!*********************************************************************************************************************
 * @brief          dcm回调诊断会话控制
 * @details        dcm回调诊断会话控制
 * @param[in]      session 会话
 * @retval         E_OK dcm回调诊断会话控制成功
 * @retval         E_NOT_OK dcm回调诊断会话控制不成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_callback_dcm_diagnostic_session_control(dcm_ses_ctrl_type session);
#endif

#ifdef DCM_CALLBACK_ECURESET_ENABLED
/*!*********************************************************************************************************************
 * @brief          dcm回调ecu函数
 * @details        dcm回调诊断会话控制
 * @param[in]      session 会话
 * @retval         E_OK dcm回调诊断会话控制成功
 * @retval         E_NOT_OK dcm回调诊断会话控制不成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_callback_dcm_ecu_reset(uint8 reset_mode);
#endif

#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
/*!*********************************************************************************************************************
 * @brief          dcm设置负相应
 * @details        dcm设置负相应
 * @param[in]      p_msg_context 消息上下文
 * @param[in]      error_code 错误码
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dcm_external_set_neg_response(const dcm_msg_context_type *p_msg_context, dcm_nrc_type error_code);

/*!*********************************************************************************************************************
 * @brief          dcm外部处理完成
 * @details        dcm外部处理完成
 * @param[in]      p_msg_context 消息上下文
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_external_processing_done(const dcm_msg_context_type *p_msg_context);
#endif

#ifndef RTE_CORE
/*!*********************************************************************************************************************
 * @brief          dcm获取安全等级
 * @details        dcm获取安全等级
 * @param[in]      sec_level 安全等级
 * @retval         E_OK dcm获取安全等级成功
 * @retval         E_NOT_OK dcm获取安全等级不成功
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_get_sec_lev(dcm_sec_lv_type *sec_level);

/*!*********************************************************************************************************************
 * @brief          dcm获取会话控制类型
 * @details        dcm获取会话控制类型
 * @param[in]      ses_ctrl_type 会话控制类型
 * @retval         E_OK dcm获取会话控制类型成功
 * @retval         E_NOT_OK dcm获取会话控制类型不成功
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_get_ses_ctrl_type(dcm_ses_ctrl_type *ses_ctrl_type);

/*!*********************************************************************************************************************
 * @brief          dcm恢复默认会话
 * @details        dcm恢复默认会话
 * @retval         E_OK dcm恢复默认会话成功
 * @retval         E_NOT_OK dcm恢复默认会话不成功
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_reset_to_default_session(void);
#endif

#if (DCM_VSG_CHECK_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          DCM vsg设置单个
 * @details        DCM vsg设置单个
 * @retval         E_OK dcmvsg设置单个成功
 * @retval         E_NOT_OK dcmvsg设置单个不成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_vsg_set_single(dcm_vsg_id_type vsg_id, dcm_vsg_state_type state);
#endif

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DCM_H */
