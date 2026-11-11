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

#ifndef DEM_H
#define DEM_H

#include "dem_types.h"
#include "dem_cfg.h"
#include "dem_lcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEM_START_SEC_CODE
#include "dem_memmap.h"

/*!*********************************************************************************************************************
 * @brief          dem master预初始化
 * @details        dem master预初始化
 * @param[in]      dem配置指针config_ptr
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_master_pre_init(const dem_config_type *config_ptr);

/*!*********************************************************************************************************************
 * @brief          dem satellite预初始化
 * @details        dem satellite预初始化
 * @param[in]      satellite id
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_satellite_pre_init(dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 * @brief          dem预初始化
 * @details        dem预初始化,单核场景使用
 * @param[in]      dem配置指针config_ptr
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_pre_init(const dem_config_type *config_ptr);

/*!*********************************************************************************************************************
 * @brief          dem master初始化
 * @details        dem master初始化
 * @param[in]      dem配置指针config_ptr
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_master_init(const dem_config_type *config_ptr);

/*!*********************************************************************************************************************
 * @brief          dem satellite初始化
 * @details        dem satellite初始化
 * @param[in]      satellite id
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_satellite_init(dem_satellite_id_type satellite_id);

/*!*********************************************************************************************************************
 * @brief          dem初始化
 * @details        dem初始化,单核场景使用
 * @param[in]      dem配置指针config_ptr
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_init(const dem_config_type *config_ptr);

/*!*********************************************************************************************************************
 * @brief          dem关闭
 * @details        dem关闭
 * @param[in]      void
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_shutdown(void);

/*!*********************************************************************************************************************
 * @brief          dem初始化内存
 * @details        dem初始化内存
 * @param[in]      void
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_init_memory(void);

/*!*********************************************************************************************************************
 * @brief          dem master主循环函数
 * @details        dem master主循环函数
 * @param[in]      void
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) Dem_MasterMainFunction(void);

/*!*********************************************************************************************************************
 * @brief          dem satellite主循环函数
 * @details        dem satellite主循环函数
 * @param[in]      void
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) Dem_SatelliteMainFunction(void);

/*!*********************************************************************************************************************
 * @brief          dem主循环函数
 * @details        dem主循环函数,单核场景使用
 * @param[in]      void
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) Dem_MainFunction(void);

/*!*********************************************************************************************************************
 * @brief          dem获取指示器状态
 * @details        dem获取指示器状态
 * @param[in]      indicator_id 指示器id
 * @param[out]     indicator_status 指示器状态
 * @retval         E_OK 获取指示器状态成功
 * @retval         E_NOT_OK 获取指示器状态失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_indicator_status(uint8 indicator_id, dem_indicator_status_type *indicator_status);

/*!*********************************************************************************************************************
 * @brief          dem设置事件状态
 * @details        dem设置事件状态
 * @param[in]      event_id 事件id
 * @param[in]      event_status 事件状态
 * @retval         E_OK 设置事件状态成功
 * @retval         E_NOT_OK 设置事件状态失败
 * @reentrant      TRUE
 * @synchronous    FALSE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_set_event_status(dem_event_id_type event_id, dem_event_status_type event_status);

/*!*********************************************************************************************************************
 * @brief          dem重置事件状态
 * @details        dem重置事件状态
 * @param[in]      event_id 事件id
 * @retval         E_OK 重置事件状态成功
 * @retval         E_NOT_OK 重置事件状态失败
 * @reentrant      TRUE
 * @synchronous    FALSE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_reset_event_status(dem_event_id_type event_id);

/*!*********************************************************************************************************************
 * @brief          dem获取事件UDS状态
 * @details        dem获取事件UDS状态
 * @param[in]      event_id 事件id
 * @param[out]     uds_status_byte UDS状态
 * @retval         E_OK 获取事件UDS状态成功
 * @retval         E_NOT_OK 获取事件UDS状态失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_event_uds_status(dem_event_id_type event_id, dem_uds_status_byte_type *uds_status_byte);

/*!*********************************************************************************************************************
 * @brief          dem获取事件故障检测计数器
 * @details        dem获取事件故障检测计数器
 * @param[in]      event_id 事件id
 * @param[out]     fault_detection_counter 故障检测计数器
 * @retval         E_OK 获取事件故障检测计数器成功
 * @retval         E_NOT_OK 获取事件故障检测计数器失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_fdc(dem_event_id_type event_id, sint8 *fault_detection_counter);

/*!*********************************************************************************************************************
 * @brief          dem设置操作周期状态
 * @details        dem设置操作周期状态
 * @param[in]      opcycle_id 操作周期id
 * @param[in]      cycle_state 操作周期状态
 * @retval         E_OK 设置操作周期状态成功
 * @retval         E_NOT_OK 设置操作周期状态失败
 * @reentrant      FALSE
 * @synchronous    FALSE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_set_opcycle_state(dem_opcycle_id_type opcycle_id, dem_opcycle_state_type cycle_state);

/*!*********************************************************************************************************************
 * @brief          dem重启操作周期
 * @details        dem重启操作周期
 * @param[in]      opcycle_id 操作周期id
 * @retval         E_OK 重启操作周期成功
 * @retval         E_NOT_OK 重启操作周期失败
 * @reentrant      FALSE
 * @synchronous    FALSE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_restart_opcycle(uint8 opcycle_id);

/*!*********************************************************************************************************************
 * @brief          dem获取事件的dtc
 * @details        dem获取事件的dtc
 * @param[in]      event_id 事件id
 * @param[in]      dtc_format dtc格式
 * @param[out]     dtc_of_event 事件的dtc
 * @retval         E_OK 获取事件的dtc成功
 * @retval         E_NOT_OK 获取事件的dtc失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_dtc_of_event(dem_event_id_type event_id, dem_dtc_format_type dtc_format, uint32 *dtc_of_event);

/*!*********************************************************************************************************************
 * @brief          dem获取dtc状态可用掩码
 * @details        dem获取dtc状态可用掩码
 * @param[in]      client_id 客户端id
 * @param[out]     dtc_status_mask dtc状态可用掩码
 * @retval         E_OK 获取dtc状态可用掩码成功
 * @retval         E_NOT_OK 获取dtc状态可用掩码失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_dtc_status_availability_mask(uint8 client_id, uint8 *dtc_status_mask);

/*!*********************************************************************************************************************
 * @brief          dem设置dtc过滤器
 * @details        dem设置dtc过滤器
 * @param[in]      client_id 客户端id
 * @param[in]      dtc_status_mask dtc状态掩码
 * @param[in]      dtc_format dtc格式
 * @param[in]      dtc_origin dtc来源
 * @param[in]      filter_with_severity 是否使用严重性过滤
 * @param[in]      dtc_severity_mask 严重性掩码
 * @param[in]      filter_for_fault_detection_counter 是否过滤故障检测计数器
 * @retval         E_OK 设置dtc过滤器成功
 * @retval         E_NOT_OK 设置dtc过滤器失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_set_dtc_filter(uint8 client_id, uint8 dtc_status_mask, dem_dtc_format_type dtc_format,
		   dem_dtc_origin_type dtc_origin, boolean filter_with_severity,
		   dem_dtc_severity_type dtc_severity_mask, boolean filter_for_fault_detection_counter);

/*!*********************************************************************************************************************
 * @brief          dem获取dtc状态
 * @details        dem获取dtc状态
 * @param[in]      client_id 客户端id
 * @param[out]     dtc_status dtc状态
 * @retval         E_OK 获取dtc状态成功
 * @retval         E_NOT_OK 获取dtc状态失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_get_status_of_dtc(uint8 client_id, dem_event_status_rec_type *dtc_status);

/*!*********************************************************************************************************************
 * @brief          dem获取符合过滤器的dtc数量
 * @details        dem获取符合过滤器的dtc数量
 * @param[in]      client_id 客户端id
 * @param[out]     number_of_filtered_dtc dtc数量
 * @retval         E_OK 获取符合过滤器的dtc数量成功
 * @retval         E_NOT_OK 获取符合过滤器的dtc数量失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_get_number_of_filtered_dtc(uint8 client_id, uint16 *number_of_filtered_dtc);

/*!*********************************************************************************************************************
 * @brief          dem获取下一个符合过滤器的dtc
 * @details        dem获取下一个符合过滤器的dtc
 * @param[in]      client_id 客户端id
 * @param[out]     dtc dtc
 * @param[out]     dtc_status dtc状态
 * @retval         E_OK 获取下一个符合过滤器的dtc成功
 * @retval         E_NOT_OK 获取下一个符合过滤器的dtc失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_next_filtered_dtc(uint8 client_id, uint32 *dtc, dem_event_status_rec_type *dtc_status);

/*!*********************************************************************************************************************
 * @brief          dem获取dtc转换格式类型
 * @details        dem获取dtc转换格式类型
 * @param[in]      client_id 客户端id
 * @retval         dtc转换格式类型
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_dtc_translation_format_type, DEM_CODE) dem_get_translation_type(uint8 client_id);

/*!*********************************************************************************************************************
 * @brief          dem清除dtc
 * @details        dem清除dtc
 * @param[in]      client_id 客户端id
 * @retval         清除dtc成功与否结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_clear_dtc(uint8 client_id);

/*!*********************************************************************************************************************
 * @brief          dem关闭dtc设置
 * @details        dem关闭dtc设置
 * @param[in]      client_id 客户端id
 * @retval         关闭dtc设置结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_disable_dtc_setting(uint8 client_id);

/*!*********************************************************************************************************************
 * @brief          dem开启dtc设置
 * @details        dem开启dtc设置
 * @param[in]      client_id 客户端id
 * @retval         开启dtc设置结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_enable_dtc_setting(uint8 client_id);

/*!*********************************************************************************************************************
 * @brief          dem获取扩展dtc数据
 * @details        dem获取扩展dtc数据
 * @param[in]      dtc dtc
 * @param[in]      dtc_origin dtc来源
 * @param[in]      ext_data_number 扩展数据号
 * @param[in]      dest_buffer 返回buffer地址
 * @param[in]      buf_size buffer长度
 * @retval         获取扩展dtc数据结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_return_get_ext_data_record_by_dtc_type, DEM_CODE)
dem_get_ext_data_record_by_dtc(uint32 dtc, dem_dtc_origin_type dtc_origin, uint8 ext_data_number, uint8 *dest_buffer,
			       uint16 *buf_size);

/*!*********************************************************************************************************************
 * @brief          dem获取dtc冻结帧
 * @details        dem获取dtc冻结帧
 * @param[in]      dtc dtc
 * @param[in]      dtc_origin dtc来源
 * @param[in]      recnum 记录号
 * @param[in]      dest_buffer 返回buffer地址
 * @param[in]      buf_size buffer长度
 * @retval         获取dtc冻结帧结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_return_get_ff_data_by_dtc_type, DEM_CODE)
dem_get_ff_data_by_dtc(uint32 dtc, dem_dtc_origin_type dtc_origin, uint8 recnum, uint8 *dest_buffer, uint16 *buf_size);

/*!*********************************************************************************************************************
 * @brief          dem设置冻结帧记录过滤器
 * @details        dem设置冻结帧记录过滤器
 * @param[in]      client_id 客户端id
 * @param[in]      dtc_format dtc格式
 * @retval         设置冻结帧记录过滤器结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_set_ff_record_filter(uint8 client_id, dem_dtc_format_type dtc_format);

/*!*********************************************************************************************************************
 * @brief          dem获取冻结帧记录数量
 * @details        dem获取冻结帧记录数量
 * @param[in]      client_id 客户端id
 * @param[out]     number_of_filtered_records 冻结帧记录数量
 * @retval         获取冻结帧记录数量结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_number_of_ff_records(uint8 client_id, uint16 *number_of_filtered_records);

/*!*********************************************************************************************************************
 * @brief          dem获取下一个符合过滤器的记录
 * @details        dem获取下一个符合过滤器的记录
 * @param[in]      client_id 客户端id
 * @param[out]     dtc dtc
 * @param[out]     recnum 记录数
 * @retval         获取下一个符合过滤器的记录结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_get_next_filtered_record(uint8 client_id, uint32 *dtc, uint8 *recnum);

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dem设置使能条件
 * @details        dem设置使能条件
 * @param[in]      condition_id 条件id
 * @param[in]      condition_fulfilled 使能条件满足与否
 * @retval         设置使能条件结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_set_enable_condition(uint8 condition_id, boolean condition_fulfilled);
#endif

#if (DEM_STORAGE_CONDITION_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dem设置存储条件
 * @details        dem设置存储条件
 * @param[in]      condition_id 条件id
 * @param[in]      condition_fulfilled 存储条件满足与否
 * @retval         设置存储条件结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_set_storage_condition(uint8 condition_id, boolean condition_fulfilled);
#endif

#if (DEM_CFG_DATA_OVER_FLOW_INDICATION == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dem获取事件存储溢出状态
 * @details        dem获取事件存储溢出状态
 * @param[in]      client_id 客户端id
 * @param[in]      dtc_origin dtc来源
 * @param[out]     overflow_indication 溢出状态
 * @retval         获取事件存储溢出状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_event_memory_overflow(uint8 client_id, dem_dtc_origin_type dtc_origin, boolean *overflow_indication);
#endif

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dem设置dtc抑制状态
 * @details        dem设置dtc抑制状态
 * @param[in]      client_id 客户端id
 * @param[in]      suppression_status 抑制状态
 * @retval         设置dtc抑制状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_set_dtc_suppression(uint8 client_id, boolean suppression_status);

/*!*********************************************************************************************************************
 * @brief          dem获取dtc抑制状态
 * @details        dem获取dtc抑制状态
 * @param[in]      client_id 客户端id
 * @param[out]     suppression_status 抑制状态
 * @retval         获取dtc抑制状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_get_dtc_suppression(uint8 client_id, boolean *suppression_status);
#endif

#if (DEM_EVENT_AVAILABLE_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dem设置事件可用状态
 * @details        dem设置事件可用状态
 * @param[in]      event_id 事件id
 * @param[in]      available_status 可用状态
 * @retval         设置事件可用状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_set_event_available(dem_event_id_type event_id, boolean available_status);
#endif

/*!*********************************************************************************************************************
 * @brief          dem获取事件监控状态
 * @details        dem获取事件监控状态
 * @param[in]      event_id 事件id
 * @param[out]     monitor_status 监控状态
 * @retval         获取事件监控状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_monitor_status(dem_event_id_type event_id, dem_monitor_status_type *monitor_status);

/*!*********************************************************************************************************************
 * @brief          dem获取操作周期状态
 * @details        dem获取操作周期状态
 * @param[in]      opcycle_id 操作周期id
 * @retval         操作周期状态
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(dem_opcycle_state_type, DEM_CODE) dem_get_opcycle_state(uint8 opcycle_id);

/*!*********************************************************************************************************************
 * @brief          dem重置事件去抖状态
 * @details        dem重置事件去抖状态
 * @param[in]      event_id 事件id
 * @param[in]      debounce_reset_status 去抖状态
 * @retval         重置事件去抖状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_reset_event_debounce_status(dem_event_id_type event_id, dem_debounce_reset_status_type debounce_reset_status);

/*!*********************************************************************************************************************
 * @brief          dem选择dtc
 * @details        dem选择dtc
 * @param[in]      client_id 客户端id
 * @param[in]      dtc dtc
 * @param[in]      dtc_format dtc格式
 * @param[in]      dtc_origin dtc来源
 * @retval         选择dtc结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_select_dtc(uint8 client_id, uint32 dtc, dem_dtc_format_type dtc_format, dem_dtc_origin_type dtc_origin);

#if defined(DEM_USE_INDICATORS)
/*!*********************************************************************************************************************
 * @brief          dem设置警报器状态
 * @details        dem设置警报器状态
 * @param[in]      event_id 事件id
 * @param[in]      wir_status 警报器状态
 * @retval         设置警报器状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_set_wir_status(dem_event_id_type event_id, boolean wir_status);
#endif

/*!*********************************************************************************************************************
 * @brief          dem获取事件去抖状态
 * @details        dem获取事件去抖状态
 * @param[in]      event_id 事件id
 * @param[out]     debouncing_state 去抖状态
 * @retval         获取事件去抖状态结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_debouncing_of_event(dem_event_id_type event_id, dem_debouncing_state_type *debouncing_state);

#if (DEM_ENABLE_PRESTORED_FF == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dem预存储冻结帧
 * @details        dem预存储冻结帧
 * @param[in]      event_id 事件id
 * @retval         预存储冻结帧结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_prestore_ff(dem_event_id_type event_id);

/*!*********************************************************************************************************************
 * @brief          dem清除预存储冻结帧
 * @details        dem清除预存储冻结帧
 * @param[in]      event_id 事件id
 * @retval         清除预存储冻结帧结果
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE) dem_clear_prestored_ff(dem_event_id_type event_id);
#endif

/*!*********************************************************************************************************************
 * @brief          获取obd冻结帧的dtc
 * @details        获取obd冻结帧的dtc
 * @param[in]      frame_number 冻结帧数字
 * @param[in]      dtc dtc指针
 * @param[in]      dtc_format dtc格式
 * @retval         E_OK 获取obd冻结帧的dtc成功
 * @retval         E_NOT_OK 获取obd冻结帧的dtc失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_get_dtc_of_obd_ff(uint8 frame_number, uint32 *dtc, dem_dtc_format_type dtc_format);

/*!*********************************************************************************************************************
 * @brief          读取obd冻结帧的数据
 * @details        读取obd冻结帧的数据
 * @param[in]      pid pid
 * @param[in]      data_elem_index_of_pid pid数据元素索引
 * @param[in]      dest_buffer 目的缓存
 * @param[in]      buf_size 缓存大小
 * @retval         E_OK  读取obd冻结帧的数据成功
 * @retval         E_NOT_OK 读取obd冻结帧的数据失败
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(std_return_type, DEM_CODE)
dem_read_data_of_obd_ff(uint8 pid, uint8 data_elem_index_of_pid, uint8 *dest_buffer, uint8 *buf_size);

#if (DEM_OBD_SUPPORT == STD_ON) && (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          将永久内存中的状态位置为passiv
 * @details        修改永久故障码的清除逻辑，从关联mil灯状态切换至warm up cycle后清除
 * @param[in]      event_id 事件id
 * @retval         void
 * @reentrant      FALSE
 * @synchronous    TRUE
 **********************************************************************************************************************/
FUNC(void, DEM_CODE) dem_set_all_perm_status_passive(void);
#endif

#define DEM_STOP_SEC_CODE
#include "dem_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DEM_H */
