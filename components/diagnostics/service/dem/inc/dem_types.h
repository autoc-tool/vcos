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

#ifndef DEM_TYPES_H
#define DEM_TYPES_H

#include "Std_Types.h"
#if (CONFIG_MODULE_RTE == 1)
#include "Rte_Type.h"
#endif
#include "Os.h"
#ifdef __cplusplus
extern "C" {
#endif


/*!*********************************************************************************************************************
 * @brief   定义dtc格式类型
 **********************************************************************************************************************/
typedef uint8 dem_dtc_format_type;

/*!*********************************************************************************************************************
 * @brief   定义去抖重置状态类型
 **********************************************************************************************************************/
typedef uint8 dem_debounce_reset_status_type;

/*!*********************************************************************************************************************
 * @brief   定义去抖状态类型
 **********************************************************************************************************************/
typedef uint8 dem_debouncing_state_type;

/*!*********************************************************************************************************************
 * @brief   定义事件状态类型
 **********************************************************************************************************************/
typedef uint8 dem_event_status_type;

/*!*********************************************************************************************************************
 * @brief   定义监控状态类型
 **********************************************************************************************************************/
typedef uint8 dem_monitor_status_type;

/*!*********************************************************************************************************************
 * @brief   定义uds状态字节类型
 **********************************************************************************************************************/
typedef uint8 dem_uds_status_byte_type;

/*!*********************************************************************************************************************
 * @brief   定义最大数据值类型
 **********************************************************************************************************************/
typedef uint8 dem_max_data_value_type[31];

/*!*********************************************************************************************************************
 * @brief   定义dtc来源类型
 **********************************************************************************************************************/
typedef uint16 dem_dtc_origin_type;

/*!*********************************************************************************************************************
 * @brief   定义事件id类型
 **********************************************************************************************************************/
typedef uint16 dem_event_id_type;

/*!*********************************************************************************************************************
 * @brief   定义监控数据类型
 **********************************************************************************************************************/
typedef uint32 dem_monitor_data_type;


/*!*********************************************************************************************************************
 * @brief   定义指示器状态类型
 **********************************************************************************************************************/
typedef uint8 dem_indicator_status_type;

/*!*********************************************************************************************************************
 * @brief   定义初始化监控原因类型
 **********************************************************************************************************************/
typedef uint8 dem_init_monitor_reason_type;

/*!*********************************************************************************************************************
 * @brief   定义操作周期id类型
 **********************************************************************************************************************/
typedef uint8 dem_opcycle_id_type;

/*!*********************************************************************************************************************
 * @brief   定义操作周期状态类型
 **********************************************************************************************************************/
typedef uint8 dem_opcycle_state_type;

/*!*********************************************************************************************************************
 * @brief   定义事件状态扩展类型
 **********************************************************************************************************************/
typedef uint8 dem_event_status_rec_type;

/*!*********************************************************************************************************************
 * @brief   定义nvram块索引类型
 **********************************************************************************************************************/
typedef uint16 nvram_block_id_index;

/*!*********************************************************************************************************************
 * @brief   定义dtc记录更新禁用类型
 **********************************************************************************************************************/
typedef struct {
	uint32 dtc;
	dem_dtc_origin_type origin;
} dem_dtc_record_update_disable_type;

/*!*********************************************************************************************************************
 * @brief   定义std_return_type类型
 **********************************************************************************************************************/
typedef Std_ReturnType std_return_type;

#ifndef DEM_DTC_FORMAT_OBD
#define DEM_DTC_FORMAT_OBD ((dem_dtc_format_type)0U) ///< obd格式
#endif

#ifndef DEM_DTC_FORMAT_UDS
#define DEM_DTC_FORMAT_UDS ((dem_dtc_format_type)1U) ///< uds格式
#endif

#ifndef DEM_DTC_FORMAT_INVALID
#define DEM_DTC_FORMAT_INVALID ((dem_dtc_format_type)2U) ///< 无效格式
#endif

#ifndef DEM_DTC_ORIGIN_NOT_USED
#define DEM_DTC_ORIGIN_NOT_USED ((dem_dtc_origin_type)0U) ///< dtc来源未使用
#endif

#ifndef DEM_DTC_ORIGIN_PRIMARY_MEMORY
#define DEM_DTC_ORIGIN_PRIMARY_MEMORY ((dem_dtc_origin_type)1U) ///< dtc来源为基础内存
#endif

#ifndef DEM_DTC_ORIGIN_MIR_MEMORY
#define DEM_DTC_ORIGIN_MIR_MEMORY ((dem_dtc_origin_type)2U) ///< dtc来源为镜像内存
#endif

#ifndef DEM_DTC_ORIGIN_PERMANENT_MEMORY
#define DEM_DTC_ORIGIN_PERMANENT_MEMORY ((dem_dtc_origin_type)3U) ///< dtc来源为永久内存
#endif

#ifndef DEM_DTC_ORIGIN_OBD_REL_MEMORY
#define DEM_DTC_ORIGIN_OBD_REL_MEMORY ((dem_dtc_origin_type)4U) ///< dtc来源为obd相关内存
#endif

#ifndef DEM_DTC_ORIGIN_SECOND_MEMORY
#define DEM_DTC_ORIGIN_SECOND_MEMORY ((dem_dtc_origin_type)5U) ///< dtc来源为第二内存
#endif

#ifndef DEM_DTC_ORIGIN_INVALID
#define DEM_DTC_ORIGIN_INVALID ((dem_dtc_origin_type)6U) ///< 无效dtc来源
#endif

#ifndef DEM_TEST_FAILED
#define DEM_TEST_FAILED ((dem_event_status_rec_type)1U) ///< 测试失败
#endif

#ifndef DEM_TEST_FAILED_THIS_OPERATION_CYCLE
#define DEM_TEST_FAILED_THIS_OPERATION_CYCLE ((dem_event_status_rec_type)2U) ///< 本次操作周期测试失败
#endif

#ifndef DEM_PENDING_DTC
#define DEM_PENDING_DTC ((dem_event_status_rec_type)4U) ///< dtc挂起
#endif

#ifndef DEM_CONFIRMED_DTC
#define DEM_CONFIRMED_DTC ((dem_event_status_rec_type)8U) ///< dtc确认
#endif

#ifndef DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
#define DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR ((dem_event_status_rec_type)16U) ///< 从上次清除后测试未完成
#endif

#ifndef DEM_TEST_FAILED_SINCE_LAST_CLEAR
#define DEM_TEST_FAILED_SINCE_LAST_CLEAR ((dem_event_status_rec_type)32U) ///< 从上次清除后测试失败
#endif

#ifndef DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE
#define DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE ((dem_event_status_rec_type)64U) ///< 本周期测试未完成
#endif

#ifndef DEM_WARNING_INDICATOR_REQUESTED
#define DEM_WARNING_INDICATOR_REQUESTED (dem_event_status_rec_type)128U ///< 指示器警告请求
#endif

#ifndef DEM_EVENT_STATUS_PASSED
#define DEM_EVENT_STATUS_PASSED ((dem_event_status_type)0U) ///< 状态通过
#endif

#ifndef DEM_EVENT_STATUS_FAILED
#define DEM_EVENT_STATUS_FAILED ((dem_event_status_type)1U) ///< 状态失败
#endif

#ifndef DEM_EVENT_STATUS_PREPASSED
#define DEM_EVENT_STATUS_PREPASSED ((dem_event_status_type)2U) ///< 状态预通过
#endif

#ifndef DEM_EVENT_STATUS_PREFAILED
#define DEM_EVENT_STATUS_PREFAILED ((dem_event_status_type)3U) ///< 状态预失败
#endif

#ifndef DEM_EVENT_STATUS_INVALID
#define DEM_EVENT_STATUS_INVALID ((dem_event_status_type)4U) ///< 状态无效
#endif

#ifndef DEM_CYCLE_STATE_START
#define DEM_CYCLE_STATE_START ((dem_opcycle_state_type)0U) ///< 周期状态开始
#endif

#ifndef DEM_CYCLE_STATE_END
#define DEM_CYCLE_STATE_END ((dem_opcycle_state_type)1U) ///< 周期状态结束
#endif

/*!*********************************************************************************************************************
 * @brief   定义设置操作周期状态类型
 **********************************************************************************************************************/
#define DEM_PENDING_CYCLE_STATE_NOCHANGE 0x00u ///< 挂起周期状态未改变
#define DEM_PENDING_CYCLE_STATE_STARTED 0x01u ///< 挂起周期状态开始
#define DEM_PENDING_CYCLE_STATE_STOPPED 0x02u ///< 挂起周期状态停止
#define DEM_PENDING_CYCLE_STATE_RESTARTED 0x03u ///< 挂起周期状态重启

/*!*********************************************************************************************************************
 * @brief   定义转换格式类型
 **********************************************************************************************************************/
typedef uint8 dem_dtc_translation_format_type;
#define DEM_DTC_TRANSLATION_ISO15031_6 ((dem_dtc_translation_format_type)0x00u) ///< 转换iso15031_6
#define DEM_DTC_TRANSLATION_ISO14229_1 ((dem_dtc_translation_format_type)0x01u) ///< 转换iso14229_1
#define DEM_DTC_TRANSLATION_SAEJ1939_73 ((dem_dtc_translation_format_type)0x02u) ///< 转换saej1939_73
#define DEM_DTC_TRANSLATION_ISO11992_4 ((dem_dtc_translation_format_type)0x03u) ///< 转换iso11992_4
#define DEM_DTC_TRANSLATION_J2012DA_FORMAT_04 ((dem_dtc_translation_format_type)0x04u) ///< 转换j2012da_format_04

/*!*********************************************************************************************************************
 * @brief   定义dtc组类型
 **********************************************************************************************************************/
typedef uint32 dem_dtc_group_type;
#define DEM_EMISSION_GROUP_REL_DTCS ((dem_dtc_group_type)0x0u) ///< 排放组
#define DEM_DTC_GROUP_ALL_DTCS ((dem_dtc_group_type)0xffffffu) ///< 所有dtc组

/*!*********************************************************************************************************************
 * @brief   定义dtc状态掩码
 **********************************************************************************************************************/
#define DEM_DTC_STATUS_MASK_ALL ((uint8)0x00u) ///< 全部状态掩码

/*!*********************************************************************************************************************
 * @brief   定义冻结帧类型
 **********************************************************************************************************************/
typedef uint8 dem_ff_kind_type;
#define DEM_FREEZE_FRAME_NON_OBD ((dem_ff_kind_type)0x01u) ///< 非obd冻结帧
#define DEM_FREEZE_FRAME_OBD ((dem_ff_kind_type)0x02u) ///< obd冻结帧

/*!*********************************************************************************************************************
 * @brief   定义冻结帧类型
 **********************************************************************************************************************/
typedef uint8 dem_event_kind_type;
#define DEM_EVENT_KIND_BSW ((dem_event_kind_type)0x01u) ///< 事件类别bsw
#define DEM_EVENT_KIND_SWC ((dem_event_kind_type)0x02u) ///< 事件类别swc

/*!*********************************************************************************************************************
 * @brief   定义预抖动名称类型
 **********************************************************************************************************************/
typedef uint8 dem_debounce_name_type;
#define DEM_NO_PRE_DEBOUNCE ((dem_debounce_name_type)0x00u) ///< 非预抖动
#define DEM_PRE_DEBOUNCE_COUNTER_BASED ((dem_debounce_name_type)0x01u) ///< 基于计数预抖动
#define DEM_PRE_DEBOUNCE_TIME_BASED ((dem_debounce_name_type)0x02u) ///< 基于时间预抖动

/*!*********************************************************************************************************************
 * @brief   定义dtc服务类型
 **********************************************************************************************************************/
typedef uint8 dem_dtc_severity_type;
#define DEM_SEVERITY_NO_SEVERITY ((dem_dtc_severity_type)0x00u) ///< 不严重
#define DEM_SEVERITY_MAINTENANCE_ONLY ((dem_dtc_severity_type)0x20u) ///< 仅维护
#define DEM_SEVERITY_CHECK_AT_NEXT_HALT ((dem_dtc_severity_type)0x40u) ///< 下次停机时检查
#define DEM_SEVERITY_CHECK_IMMEDIATELY ((dem_dtc_severity_type)0x80u) ///< 立即检查

#define DEM_WRONG_DTCORIGIN (0x02u) ///< 错误dtc来源
#define DEM_PENDING (0x03u) ///< 挂起
#define DEM_NO_SUCH_ELEMENT (0x04u) ///< 没有这样的元素
#define DEM_CLEAR_FAILED (0x05u) ///< 清除失败
#define DEM_CLEAR_MEMORY_ERROR (0x06u) ///< 清除内存错误
#define DEM_WRONG_DTC (0x07u) ///< 错误dtc

/*!*********************************************************************************************************************
 * @brief   定义返回数据类型
 **********************************************************************************************************************/
typedef uint8 dem_return_get_ext_data_record_by_dtc_type;
#define DEM_RECORD_OK ((dem_return_get_ext_data_record_by_dtc_type)0x00u) ///< 记录正常
#define DEM_RECORD_WRONG_DTC ((dem_return_get_ext_data_record_by_dtc_type)0x01u) ///< 记录错误dtc
#define DEM_RECORD_WRONG_DTCORIGIN ((dem_return_get_ext_data_record_by_dtc_type)0x02u) ///< 记录错误dtc来源
#define DEM_RECORD_DTCKIND ((dem_return_get_ext_data_record_by_dtc_type)0x03u) ///< 记录dtc种类
#define DEM_RECORD_NUMBER ((dem_return_get_ext_data_record_by_dtc_type)0x04u) ///< 记录个数
#define DEM_RECORD_BUFFERSIZE ((dem_return_get_ext_data_record_by_dtc_type)0x05u) ///< 记录缓存大小
#define DEM_RECORD_PENDING ((dem_return_get_ext_data_record_by_dtc_type)0x06u) ///< 记录挂起

/*!*********************************************************************************************************************
 * @brief   定义返回冻结帧数据类型
 **********************************************************************************************************************/
typedef uint8 dem_return_get_ff_data_by_dtc_type;
#define DEM_GET_FFDATABYDTC_OK ((dem_return_get_ff_data_by_dtc_type)0x00u) ///< 冻结帧dtc正确
#define DEM_GET_FFDATABYDTC_WRONG_DTC ((dem_return_get_ff_data_by_dtc_type)0x01u) ///< 冻结帧错误dtc
#define DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN ((dem_return_get_ff_data_by_dtc_type)0x02u) ///< 冻结帧错误dtc来源
#define DEM_GET_FFDATABYDTC_WRONG_DTCKIND ((dem_return_get_ff_data_by_dtc_type)0x03u) ///< 冻结帧错误dtc种类
#define DEM_GET_FFDATABYDTC_RECORDNUMBER ((dem_return_get_ff_data_by_dtc_type)0x04u) ///< 冻结帧记录号
#define DEM_GET_FFDATABYDTC_WRONG_DATAID ((dem_return_get_ff_data_by_dtc_type)0x05u) ///< 冻结帧错误数据id
#define DEM_GET_FFDATABYDTC_BUFFERSIZE ((dem_return_get_ff_data_by_dtc_type)0x06u) ///< 冻结帧缓存大小
#define DEM_GET_ID_PENDING ((dem_return_get_ff_data_by_dtc_type)0x07u) ///< 挂起id

/*!*********************************************************************************************************************
 * @brief   定义dtc种类类型
 **********************************************************************************************************************/
typedef uint8 dem_dtc_kind_type;

#ifndef DEM_DTC_KIND_ALL_DTCS
#define DEM_DTC_KIND_ALL_DTCS ((dem_dtc_kind_type)0x01u) ///< 全部dtc
#endif

#ifndef DEM_EMISSION_REL_DTCS_KIND
#define DEM_EMISSION_REL_DTCS_KIND ((dem_dtc_kind_type)0x02u) ///< 排放dtc
#endif

#ifndef DEM_INIT_CLEAR_MONITOR
#define DEM_INIT_CLEAR_MONITOR ((dem_init_monitor_reason_type)1u) ///< 初始化监控清除
#endif

#ifndef DEM_INIT_RESTART_MONITOR
#define DEM_INIT_RESTART_MONITOR ((dem_init_monitor_reason_type)2u) ///< 初始化监控重启
#endif

#ifndef DEM_INDICATOR_OFF
#define DEM_INDICATOR_OFF ((dem_indicator_status_type)0U) ///< 指示器关闭
#endif
#ifndef DEM_INDICATOR_CONTINUOUS
#define DEM_INDICATOR_CONTINUOUS ((dem_indicator_status_type)1U) ///< 指示器继续
#endif
#ifndef DEM_INDICATOR_BLINKING
#define DEM_INDICATOR_BLINKING ((dem_indicator_status_type)2U) ///< 指示器闪烁
#endif
#ifndef DEM_INDICATOR_BLINK_CONT
#define DEM_INDICATOR_BLINK_CONT ((dem_indicator_status_type)3U) ///< 指示器连续闪烁
#endif

#ifndef DEM_ACTIVE
#define DEM_ACTIVE ((dem_opcycle_id_type)0x00u) ///< 激活
#endif

#define DEM_OBD_DCY ((dem_opcycle_id_type)0x04u) ///< obd驱动周期

/*!*********************************************************************************************************************
 * @brief   定义返回不使能dtc记录更新类型
 **********************************************************************************************************************/
typedef uint8 dem_return_disable_dtc_record_update_type;
#define DEM_DISABLE_DTCRECUP_OK ((dem_return_disable_dtc_record_update_type)0x00u) ///< 正常
#define DEM_DISABLE_DTCRECUP_WRONG_DTC ((dem_return_disable_dtc_record_update_type)0x01u) ///< 错误dtc
#define DEM_DISABLE_DTCRECUP_WRONG_DTCORIGIN ((dem_return_disable_dtc_record_update_type)0x02u) ///< 错误dtc来源
#define DEM_DISABLE_DTCRECUP_PENDING ((dem_return_disable_dtc_record_update_type)0x03u) ///< 挂起
#define DEM_GET_FFDATABYDTC_PENDING ((dem_return_get_ff_data_by_dtc_type)0x05u) ///< 挂起
#define DEM_RECORD_WRONG_NUMBER ((dem_return_get_ext_data_record_by_dtc_type)0x03u) ///< 错误号

/*!*********************************************************************************************************************
 * @brief   定义环境数据捕捉类型
 **********************************************************************************************************************/
typedef uint8 dem_environment_data_capture_type;
#define DEM_CAPTURE_ASYNCHRONOUS_TO_REPORTING ((dem_environment_data_capture_type)0x00u) ///< 捕捉异步上报
#define DEM_CAPTURE_SYNCHRONOUS_TO_REPORTING ((dem_environment_data_capture_type)0x01u) ///< 捕捉同步上报

/*!*********************************************************************************************************************
 * @brief   定义事件替换策略类型
 **********************************************************************************************************************/
typedef uint8 dem_event_displacement_strategy_type;
#define DEM_DISPLACEMENT_FULL ((dem_event_displacement_strategy_type)0x00u) ///< 全替换
#define DEM_DISPLACEMENT_NONE ((dem_event_displacement_strategy_type)0x01u) ///< 不替换
#define DEM_DISPLACEMENT_PRIO_OCC ((dem_event_displacement_strategy_type)0x02u) ///< 发生优先级替换

/*!*********************************************************************************************************************
 * @brief   定义发生计数处理类型
 **********************************************************************************************************************/
typedef uint8 dem_occctr_processing_type;
#define DEM_PROCESS_OCCCTR_CDTC ((dem_occctr_processing_type)0x00u) ///< 处理occctr_cdtc
#define DEM_PROCESS_OCCCTR_TF ((dem_occctr_processing_type)0x01u) ///< 处理occctr_tf

/*!*********************************************************************************************************************
 * @brief   定义冻结帧记录号类型
 **********************************************************************************************************************/
typedef uint8 dem_ff_recnum_type;
#define DEM_FF_RECNUM_CALCULATED ((dem_ff_recnum_type)0x00u) ///< 冻结帧记录计算
#define DEM_FF_RECNUM_CONFIGURED ((dem_ff_recnum_type)0x01u) ///< 冻结帧记录配置

/*!*********************************************************************************************************************
 * @brief   定义抑制支持类型
 **********************************************************************************************************************/
typedef uint8 dem_suppression_type;
#define DEM_DTC_SUPPRESSION ((dem_suppression_type)0x00u) ///< 抑制dtc
#define DEM_EVENT_AND_DTC_SUPPRESSION ((dem_suppression_type)0x01u) ///< 抑制事件和dtc
#define DEM_EVENT_SUPPRESSION ((dem_suppression_type)0x02u) ///< 抑制事件
#define DEM_NO_SUPPRESSION ((dem_suppression_type)0x03u) ///< 不抑制

#define DEM_TEMP_DEFECTIVE ((dem_debouncing_state_type)0x01u) ///< 暂时有缺陷
#define DEM_FINALLY_DEFECTIVE ((dem_debouncing_state_type)0x02u) ///< 最终有缺陷
#define DEM_TEMP_HEALED ((dem_debouncing_state_type)0x04u) ///< 暂时自愈
#define DEM_TEST_COMPLETE ((dem_debouncing_state_type)0x08u) ///< 测试完成
#define DEM_DTR_UPDATE ((dem_debouncing_state_type)0x10u) ///< dtr更新

/*!*********************************************************************************************************************
 * @brief   定义状态类型
 **********************************************************************************************************************/
typedef uint8 dem_state_type;
#define DEM_STATE_UNINIT ((dem_state_type)0x00u) ///< 未初始化
#define DEM_INIT_PREINIT ((dem_state_type)0x01u) ///< 预初始化
#define DEM_STATE_INIT ((dem_state_type)0x02u) ///< 已初始化
#define DEM_STATE_SHUTDOWN ((dem_state_type)0x03u) ///< 关闭

/*!*********************************************************************************************************************
 * @brief   定义配置状态类型
 **********************************************************************************************************************/
typedef uint8 dem_config_ptr_state_type;
#define DEM_CONFIGPTR_UNINITIALIZED ((dem_config_ptr_state_type)0x00u) ///< 配置未初始化
#define DEM_CONFIGPTR_PREINITIALIZED ((dem_config_ptr_state_type)0x01u) ///< 配置预初始化
#define DEM_CONFIGPTR_INITIALIZED ((dem_config_ptr_state_type)0x02u) ///< 配置已初始化

/*!*********************************************************************************************************************
 * @brief   定义数据元素使用端口类型
 **********************************************************************************************************************/
typedef uint8 dem_data_elem_use_port_type;
#define DEM_CS_PORT ((dem_data_elem_use_port_type)0x00u) ///< cs端口
#define DEM_CS_PORT_WITH_EVENTID ((dem_data_elem_use_port_type)0x01u) ///< 带事件id的cs端口
#define DEM_FUNCTIONAL_CALL ((dem_data_elem_use_port_type)0x02u) ///< 功能调用
#define DEM_FUNCTIONAL_CALL_WITH_EVENTID ((dem_data_elem_use_port_type)0x03u) ///< 带事件id的功能调用
#define DEM_SR_PORT ((dem_data_elem_use_port_type)0x04u) ///< sr端口
#define DEM_INTERNAL ((dem_data_elem_use_port_type)0x05u) ///< 内部

/*!*********************************************************************************************************************
 * @brief   定义内部数据元素类型
 **********************************************************************************************************************/
typedef uint8 dem_internal_data_elem_type;
#define DEM_AGINGCTR ((dem_internal_data_elem_type)0x00u) ///< 老化计数
#define DEM_CURRENT_FDC ((dem_internal_data_elem_type)0x01u) ///< 当前fdc
#define DEM_MAX_FDC_DURING_CURRENT_CYCLE ((dem_internal_data_elem_type)0x02u) ///< 当前周期中最大fdc
#define DEM_CYCLES_SINCE_FIRST_FAILED ((dem_internal_data_elem_type)0x03u) ///< 从第一次失败经过的周期
#define DEM_CYCLES_SINCE_LAST_FAILED ((dem_internal_data_elem_type)0x04u) ///< 从上次失败经过的周期
#define DEM_DTC_PRIORITY ((dem_internal_data_elem_type)0x05u) ///< dtc优先级
#define DEM_FAILED_CYCLES ((dem_internal_data_elem_type)0x06u) ///< 失败周期
#define DEM_OCCCTR ((dem_internal_data_elem_type)0x07u) ///< 发生计数
#define DEM_OVFLIND ((dem_internal_data_elem_type)0x08u) ///< 溢出索引号
#define DEM_SIGNIFICANCE ((dem_internal_data_elem_type)0x09u) ///< 特性
#define DEM_NO_ELEMENT ((dem_internal_data_elem_type)0x0au) ///< 没有元素
#define DEM_AGED_COUNTER ((dem_internal_data_elem_type)0x0bu) ///< 已老化计数
#define DEM_FAULT_PENDING_COUNTER ((dem_internal_data_elem_type)0x0cu) ///< 故障挂起计数

/*!*********************************************************************************************************************
 * @brief   定义指示器失败源类型
 **********************************************************************************************************************/
typedef uint8 dem_indicator_failure_source_type;
#define DEM_FAILURE_CYCLE_EVENT ((dem_indicator_failure_source_type)0x00u) ///< 失败周期事件
#define DEM_FAILURE_CYCLE_INDICATOR ((dem_indicator_failure_source_type)0x01u) ///< 失败周期指示器

/*!*********************************************************************************************************************
 * @brief   定义更新规则类型
 **********************************************************************************************************************/
typedef uint8 dem_update_rule_type;
#define DEM_UPDATE_RECORD_NO ((dem_update_rule_type)0x00u) ///< 不更新记录
#define DEM_UPDATE_RECORD_YES ((dem_update_rule_type)0x01u) ///< 更新记录

/*!*********************************************************************************************************************
 * @brief   定义数据元素大小端类型
 **********************************************************************************************************************/
typedef uint8 dem_data_elem_endianness;
#define DEM_DATA_BIG_ENDIAN ((dem_data_elem_endianness)0x00u) ///< 大端
#define DEM_DATA_LITTLE_ENDIAN ((dem_data_elem_endianness)0x01u) ///< 小端
#define DEM_DATA_OPAQUE ((dem_data_elem_endianness)0x02u) ///< 不透明

/*!*********************************************************************************************************************
 * @brief   定义扩展数据记录触发类型
 **********************************************************************************************************************/
typedef uint8 dem_ext_data_record_trigger_type;
#define DEM_TRIGGER_ON_CONFIRMED ((dem_ext_data_record_trigger_type)0x00u) ///< 确认时触发
#define DEM_TRIGGER_ON_FDC_THRESHOLD ((dem_ext_data_record_trigger_type)0x01u) ///< fdc阈值时触发
#define DEM_TRIGGER_ON_MIRROR ((dem_ext_data_record_trigger_type)0x02u) ///< 镜像时触发
#define DEM_TRIGGER_ON_PASSED ((dem_ext_data_record_trigger_type)0x03u) ///< 通过时触发
#define DEM_TRIGGER_ON_PENDING ((dem_ext_data_record_trigger_type)0x04u) ///< 挂起时触发
#define DEM_TRIGGER_ON_TEST_FAILED ((dem_ext_data_record_trigger_type)0x05u) ///< 测试失败时触发

/*!*********************************************************************************************************************
 * @brief   定义事件队列状态类型
 **********************************************************************************************************************/
typedef uint8 dem_event_queue_status_type;
#define DEM_EVENTQUEUE_INITIAL_STATE ((dem_event_queue_status_type)0x00u) ///< 事件队列初始化状态

#define DEM_SELECTED_SELECTION_INITED ((dem_dtc_selector_result_type)0u) ///< selector处于init状态
#define DEM_SELECTED_SELECTION_DTC ((dem_dtc_selector_result_type)1u) ///< selector状态为单个dtc
#define DEM_SELECTED_SELECTION_GROUP_DTC ((dem_dtc_selector_result_type)2u) ///< selector状态为dtc_group
#define DEM_SELECTED_ALL_DTCS ((dem_dtc_selector_result_type)3u) ///< selector状态为所有dtc
#define DEM_SELECTED_SELECTION_EMISSION_DTC ((dem_dtc_selector_result_type)4U) ///< selector状态为排放相关dtc
#define DEM_SELECTED_SELECTION_NO_MATCH_DTC ((dem_dtc_selector_result_type)5u) ///< selector状态不匹配
#define  DEM_SELECTED_SELECTION_WRONG_ORIGIN ((dem_dtc_selector_result_type)6u) ///< selector状态为错误来源

#define DEM_SELECTED_GROUP_DTC_INVALID (0u) ///< selector组非法
#define DEM_SELECTED_ALL_GROUP_DTCS (1u) ///< selector组所有dtc

/*!*********************************************************************************************************************
 * @brief   定义dtc选择组id类型
 **********************************************************************************************************************/
typedef uint8 dem_dtc_group_id_type;

/*!*********************************************************************************************************************
 * @brief   定义当前selector的结果类型
 **********************************************************************************************************************/
typedef uint8 dem_dtc_selector_result_type;

#define DEM_PERMANENT_DTC_UNUSED (0x00u) ///< 永久没有
#define DEM_PERMANENT_DTC_ACTIVE (0x01u) ///< 永久激活
#define DEM_PERMANENT_DTC_PASSIVE (0x02u) ///< 永久被动
#define DEM_PERMANENT_DTC_CLEARNOW (0x03u) ///< 立即清除

/*!*********************************************************************************************************************
 * @brief   定义过程词类型
 **********************************************************************************************************************/
typedef uint32 dem_atomic_word_type;

#define DEM_MONITOR_TF_STATUS ((dem_monitor_status_type)0x01u) ///< 监控状态为tf
#define DEM_MONITOR_TNCTOC_STATUS ((dem_monitor_status_type)0x02u) ///< 监控状态为tnctoc

#define DEM_E_NO_DTC_AVAILABLE ((std_return_type)10u) ///< 无可用dtc

#ifdef __cplusplus
}
#endif

#endif /* DEM_TYPES_H */
