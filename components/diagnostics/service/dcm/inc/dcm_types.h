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

#ifndef DCM_TYPES_H
#define DCM_TYPES_H

#include "Std_Types.h"
#if (CONFIG_MODULE_RTE == 1)
#include "Rte_Dcm_Type.h"
#endif
#include "comstack_types.h"
#include "dcm_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   dcm负相应码类型
 **********************************************************************************************************************/
typedef uint8 dcm_nrc_type;

#define DCM_E_POSITIVERESPONSE ((dcm_nrc_type)0x00u) ///< 正响应
#define DCM_E_GENERALREJECT ((dcm_nrc_type)0x10u) ///< 通用拒绝
#define DCM_E_SERVICENOTSUPPORTED ((dcm_nrc_type)0x11u) ///< 服务不支持
#define DCM_E_SUBFUNCTIONNOTSUPPORTED ((dcm_nrc_type)0x12u) ///< 子函数不支持
#define DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT ((dcm_nrc_type)0x13u) ///< 不正确消息长度
#define DCM_E_RESPONSETOOLONG ((dcm_nrc_type)0x14u) ///< 响应太长
#define DCM_E_BUSYREPEATREQUEST ((dcm_nrc_type)0x21u) ///< 忙碌重复请求
#define DCM_E_CONDITIONSNOTCORRECT ((dcm_nrc_type)0x22u) ///< 条件不对
#define DCM_E_REQUESTSEQUENCEERROR ((dcm_nrc_type)0x24u) ///< 请求序列号错误
#define DCM_E_NORESPONSEFROMSUBNETCOMPONENT ((dcm_nrc_type)0x25u) ///< 没有响应
#define DCM_E_FAILUREPREVENTSEXECUTIONOFREQUESTEDACTION ((dcm_nrc_type)0x26u) ///< 失败执行
#define DCM_E_REQUESTOUTOFRANGE ((dcm_nrc_type)0x31u) ///< 请求越界
#define DCM_E_SECURITYACCESSDENIED ((dcm_nrc_type)0x33u) ///< 安全接入
#define DCM_E_INVALIDKEY ((dcm_nrc_type)0x35u) ///< 无效id KEY
#define DCM_E_EXCEEDNUMBEROFATTEMPTS ((dcm_nrc_type)0x36u) ///< 超过数目
#define DCM_E_REQUIREDTIMEDELAYNOTEXPIRED ((dcm_nrc_type)0x37u) ///< 请求时间延迟
#define DCM_E_UPLOADDOWNLOADNOTACCEPTED ((dcm_nrc_type)0x70u) ///< 上传下载不接受
#define DCM_E_TRANSFERDATASUSPENDED ((dcm_nrc_type)0x71u) ///< 发送数据抑制
#define DCM_E_GENERALPROGRAMMINGFAILURE ((dcm_nrc_type)0x72u) ///< 通用语法错误
#define DCM_E_WRONGBLOCKSEQUENCECOUNTER ((dcm_nrc_type)0x73u) ///< 错误序列号
#define DCM_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION ((dcm_nrc_type)0x7eu) ///< 子功能不支持
#define DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION ((dcm_nrc_type)0x7fu) ///< 服务不支持
#define DCM_E_RPMTOOHIGH ((dcm_nrc_type)0x81u) ///< rem太高
#define DCM_E_RPMTOOLOW ((dcm_nrc_type)0x82u) ///< rem太低
#define DCM_E_ENGINEISRUNNING ((dcm_nrc_type)0x83u) ///< 正在运行
#define DCM_E_ENGINEISNOTRUNNING ((dcm_nrc_type)0x84u) ///< 未运行
#define DCM_E_ENGINERUNTIMETOOLOW ((dcm_nrc_type)0x85u) ///< 运行时间太小
#define DCM_E_TEMPERATURETOOHIGH ((dcm_nrc_type)0x86u) ///< 温度太高
#define DCM_E_TEMPERATURETOOLOW ((dcm_nrc_type)0x87u) ///< 温度太低
#define DCM_E_VEHICLESPEEDTOOHIGH ((dcm_nrc_type)0x88u) ///< 车速太高
#define DCM_E_VEHICLESPEEDTOOLOW ((dcm_nrc_type)0x89u) ///< 车速太低
#define DCM_E_THROTTLE_PEDALTOOHIGH ((dcm_nrc_type)0x8au) ///< 车速太高
#define DCM_E_THROTTLE_PEDALTOOLOW ((dcm_nrc_type)0x8bu) ///< 车速太低
#define DCM_E_TRANSMISSIONRANGENOTINNEUTRAL ((dcm_nrc_type)0x8cu) ///< 发送未中断
#define DCM_E_TRANSMISSIONRANGENOTINGEAR ((dcm_nrc_type)0x8du) ///< 发送范围不是整数
#define DCM_E_BRAKESWITCH_NOTCLOSED ((dcm_nrc_type)0x8fu) ///< 刹车开关未关闭
#define DCM_E_SHIFTERLEVERNOTINPARK ((dcm_nrc_type)0x90u) ///< 切换未停
#define DCM_E_TORQUECONVERTERCLUTCHLOCKED ((dcm_nrc_type)0x91u) ///< 转换锁定
#define DCM_E_VOLTAGETOOHIGH ((dcm_nrc_type)0x92u) ///< 电压太高
#define DCM_E_VOLTAGETOOLOW ((dcm_nrc_type)0x93u) ///< 电压太低
#define DCM_E_OK ((dcm_nrc_type)0x00u) ///< 正常错误码
#define DCM_E_NOT_OK ((dcm_nrc_type)0x01u) ///< 不正常错误码
#define DCM_E_PROTOCOL_NOT_ALLOWED ((dcm_nrc_type)0x05u) ///< 协议不允许错误码
#define DCM_E_REQUEST_NOT_ACCEPTED ((dcm_nrc_type)0x08u) ///< 请求未接受错误码
#define DCM_E_PENDING ((dcm_nrc_type)0x0au) ///< 未完成错误码
#define DCM_E_FORCE_RCRRP ((dcm_nrc_type)0x0cu) ///< 正在进行错误码

/*!*********************************************************************************************************************
 * @brief   dcm确认状态类型
 **********************************************************************************************************************/
typedef uint8 dcm_conf_status_type;

#define DCM_RES_POS_OK ((dcm_conf_status_type)0u) ///< 正响应ok
#define DCM_RES_POS_NOT_OK ((dcm_conf_status_type)1u) ///< 正响应不ok
#define DCM_RES_NEG_OK ((dcm_conf_status_type)2u) ///< 负相应ok
#define DCM_RES_NEG_NOT_OK ((dcm_conf_status_type)3u) ///< 负相应不ok

/*!*********************************************************************************************************************
 * @brief   DCM 运行状态类型
 **********************************************************************************************************************/
typedef uint8 dcm_op_status_type;

#define DCM_INITIAL ((dcm_op_status_type)0u) ///< 初始化
#define DCM_PENDING ((dcm_op_status_type)1u) ///< 预留
#define DCM_CANCEL ((dcm_op_status_type)2u) ///< 取消
#define DCM_FORCE_RCRRP_OK ((dcm_op_status_type)3u) ///< 强制ok

/*!*********************************************************************************************************************
 * @brief   DCM 运行状态类型
 **********************************************************************************************************************/
typedef uint8 dcm_protocol_type;

#define DCM_OBD_ON_CAN ((dcm_protocol_type)0u) ///< obd基于can
#define DCM_OBD_ON_IP ((dcm_protocol_type)2u) ///< obd基于ip
#define DCM_UDS_ON_CAN ((dcm_protocol_type)3u) ///< uds基于can
#define DCM_UDS_ON_IP ((dcm_protocol_type)5u) ///< uds基于ip
#define DCM_NO_ACTIVE_PROTOCOL ((dcm_protocol_type)12u) ///< 未激活协议

/*!*********************************************************************************************************************
 * @brief   安全等级类型
 **********************************************************************************************************************/
typedef uint8 dcm_sec_lv_type;

#define DCM_SEC_LEV_LOCKED ((dcm_sec_lv_type)0x00u) ///< 安全等级锁定
#define DCM_SEC_LEV_L1 ((dcm_sec_lv_type)0x01u) ///< 安全等级l1
#define DCM_SEC_LEV_ALL ((dcm_sec_lv_type)0xffu) ///< 安全等级全部

/*!*********************************************************************************************************************
 * @brief   ecu重置类型
 **********************************************************************************************************************/
typedef uint8 dcm_ecu_rst_type;

#define DCM_HARD_RESET ((dcm_ecu_rst_type)0x01u) ///< 硬线重启
#define DCM_KEY_OFF_ON_RESET ((dcm_ecu_rst_type)0x02u) ///< 基于重启钥匙不启用
#define DCM_SOFT_RESET ((dcm_ecu_rst_type)0x03u) ///< 软件重启
#define DCM_ENABLE_RAPID_POWER_SHUTDOWN ((dcm_ecu_rst_type)0x04u) ///< 使能快速电源关闭
#define DCM_DISABLE_RAPID_POWER_SHUTDOWN ((dcm_ecu_rst_type)0x05u) ///< 不使能快速电源关闭

#ifndef RTE_MODE_DcmEcuReset_NONE
#define RTE_MODE_DcmEcuReset_NONE (0U) ///< 不重启
#endif
#ifndef RTE_MODE_DcmEcuReset_HARD
#define RTE_MODE_DcmEcuReset_HARD (1U) ///< 硬线重启
#endif
#ifndef RTE_MODE_DcmEcuReset_KEYONOFF
#define RTE_MODE_DcmEcuReset_KEYONOFF (2U) ///< 无钥匙重启
#endif
#ifndef RTE_MODE_DcmEcuReset_SOFT
#define RTE_MODE_DcmEcuReset_SOFT (3U) ///< 软件重启
#endif
#ifndef RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER
#define RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER (4U) ///< 跳到bootloader
#endif
#ifndef RTE_MODE_DcmEcuReset_JUMPTOSYSSUPPLIERBOOTLOADER
#define RTE_MODE_DcmEcuReset_JUMPTOSYSSUPPLIERBOOTLOADER (5U) ///< 跳到系统供电bootloader
#endif
#ifndef RTE_MODE_DcmEcuReset_EXECUTE
#define RTE_MODE_DcmEcuReset_EXECUTE (6U) ///< 执行重启
#endif

/*!*********************************************************************************************************************
 * @brief   通信模式类型
 **********************************************************************************************************************/
typedef uint8 dcm_comm_mode_type;

#define DCM_ENABLE_RX_TX_NORM ((dcm_comm_mode_type)0x00u) ///< 使能接收发送模式
#define DCM_ENABLE_RX_DISABLE_TX_NORM ((dcm_comm_mode_type)0x01u) ///< 使能接收不使能发送模式
#define DCM_DISABLE_RX_ENABLE_TX_NORM ((dcm_comm_mode_type)0x02u) ///< 不使能接收使能发送模式
#define DCM_DISABLE_RX_TX_NORMAL ((dcm_comm_mode_type)0x03u) ///< 不使能接收不使能发送模式
#define DCM_ENABLE_RX_TX_NM ((dcm_comm_mode_type)0x04u) ///< 使能发送接收nm
#define DCM_ENABLE_RX_DISABLE_TX_NM ((dcm_comm_mode_type)0x05u) ///< 使能接收不使能发送nm
#define DCM_DISABLE_RX_ENABLE_TX_NM ((dcm_comm_mode_type)0x06u) ///< 不使能接收使能发送nm
#define DCM_DISABLE_RX_TX_NM ((dcm_comm_mode_type)0x07u) ///< 不使能接收发送nm
#define DCM_ENABLE_RX_TX_NORM_NM ((dcm_comm_mode_type)0x08u) ///< 使能发送接收模式nm
#define DCM_ENABLE_RX_DISABLE_TX_NORM_NM ((dcm_comm_mode_type)0x09u) ///< 使能接收不使能发送模式nm
#define DCM_DISABLE_RX_ENABLE_TX_NORM_NM ((dcm_comm_mode_type)0x0au) ///< 不使能接收使能发送模式nm
#define DCM_DISABLE_RX_TX_NORM_NM ((dcm_comm_mode_type)0x0bu) ///< 不使能接收发送模式nm

#ifndef RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NORM
#define RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NORM (0U) ///< 使能接收发送模式
#endif
#ifndef RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NORM
#define RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NORM (1U) ///< 使能接收不使能发送模式
#endif
#ifndef RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NORM
#define RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NORM (2U) ///< 不使能接收使能发送模式
#endif
#ifndef RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NORMAL
#define RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NORMAL (3U) ///< 不使能发送接收模式
#endif
#ifndef RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NM
#define RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NM (4U) ///< 使能发送接收nm
#endif
#ifndef RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NM
#define RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NM (5U) ///< 使能接收不使能发送nm
#endif
#ifndef RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NM
#define RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NM (6U) ///< 不使能接收使能发送nm
#endif
#ifndef RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NM
#define RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NM (7U) ///< 不使能接收发送nm
#endif
#ifndef RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NORM_NM
#define RTE_MODE_DcmComControl_DCM_ENABLE_RX_TX_NORM_NM (8U) ///< 使能接收使能发送模式nm
#endif
#ifndef RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NORM_NM
#define RTE_MODE_DcmComControl_DCM_ENABLE_RX_DISABLE_TX_NORM_NM (9U) ///< 使能接收不使能发送模式nm
#endif
#ifndef RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NORM_NM
#define RTE_MODE_DcmComControl_DCM_DISABLE_RX_ENABLE_TX_NORM_NM (10U) ///< 不使能接收使能发送模式nm
#endif
#ifndef RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NORM_NM
#define RTE_MODE_DcmComControl_DCM_DISABLE_RX_TX_NORM_NM (11U) ///< 不使能接收发送模式nm
#endif

/*!*********************************************************************************************************************
 * @brief   会话控制类型
 **********************************************************************************************************************/
typedef uint8 dcm_ses_ctrl_type;

#define DCM_DEFAULT_SESSION ((dcm_ses_ctrl_type)0x01u) ///< 默认会话
#define DCM_PROGRAMMING_SESSION ((dcm_ses_ctrl_type)0x02u) ///< 程序会话
#define DCM_EXTENDED_DIAGNOSTIC_SESSION ((dcm_ses_ctrl_type)0x03u) ///< 超出诊断会话
#define DCM_SAFTEY_SYSTEM_DIAGNOSTIC_SESSION ((dcm_ses_ctrl_type)0x04u) ///< 安全系统诊断会话
#define DCM_OBD_SESSION ((dcm_ses_ctrl_type)0x05u) ///< obd会话
#define DCM_ALL_SESSION_LEVEL ((dcm_ses_ctrl_type)0xffu) ///< 全部会话等级

/*!*********************************************************************************************************************
 * @brief   pid服务类型
 **********************************************************************************************************************/
typedef uint8 dcm_pid_svc_type;

#define DCM_SERVICE_01 ((dcm_pid_svc_type)0x01u) ///< 服务01
#define DCM_SERVICE_02 ((dcm_pid_svc_type)0x02u) ///< 服务02
#define DCM_SERVICE_01_02 ((dcm_pid_svc_type)0x03u) ///< 服务01_02

/*!*********************************************************************************************************************
 * @brief   返回读内存类型
 **********************************************************************************************************************/
typedef uint8 dcm_return_read_memory_type;

#define DCM_READ_OK ((dcm_return_read_memory_type)0x00u) ///< 读ok
#define DCM_READ_PENDING ((dcm_return_read_memory_type)0x01u) ///< 读未完成
#define DCM_READ_FAILED ((dcm_return_read_memory_type)0x02u) ///< 读失败
#define DCM_READ_FORCE_RCRRP ((dcm_return_read_memory_type)0x03u) ///< 正在读取

/*!*********************************************************************************************************************
 * @brief   返回写内存类型
 **********************************************************************************************************************/
typedef uint8 dcm_return_write_memory_type;

#define DCM_WRITE_OK ((dcm_return_write_memory_type)0x00u) ///< 写ok
#define DCM_WRITE_PENDING ((dcm_return_write_memory_type)0x01u) ///< 写未完成
#define DCM_WRITE_FAILED ((dcm_return_write_memory_type)0x02u) ///< 写失败
#define DCM_WRITE_FORCE_RCRRP ((dcm_return_write_memory_type)0x03u) ///< 正在写

/*!*********************************************************************************************************************
 * @brief   io控制参数类型
 **********************************************************************************************************************/
typedef uint8 dcm_io_ctrl_para_type;
#define DCM_RETURN_CONTROL_TO_ECU ((dcm_io_ctrl_para_type)0x0u) ///< 返回控制到ecu
#define DCM_RESET_TO_DEFAULT ((dcm_io_ctrl_para_type)0x01u) ///< 恢复默认
#define DCM_FREEZE_CURRENT_STATE ((dcm_io_ctrl_para_type)0x02u) ///< 冻结当前状态
#define DCM_SHORT_TERM_ADJUSTMENT ((dcm_io_ctrl_para_type)0x03u) ///< 短暂调整

/*!*********************************************************************************************************************
 * @brief   did支持类型
 **********************************************************************************************************************/
typedef uint8 dcm_did_supt_type;

#define DCM_DID_SUPPORTED ((dcm_did_supt_type)0u) ///< did支持
#define DCM_DID_NOT_SUPPORTED ((dcm_did_supt_type)1u) ///< did不支持

/*!*********************************************************************************************************************
 * @brief   控制dtc设置类型
 **********************************************************************************************************************/
typedef uint8 dcm_ctrl_dtc_set_type;

#define DCM_ENABLEDTCSETTING ((dcm_ctrl_dtc_set_type)0u) ///< 使能dtc设置
#define DCM_DISABLEDTCSETTING ((dcm_ctrl_dtc_set_type)1u) ///< 不使能dtc设置

/*!*********************************************************************************************************************
 * @brief   ecu开启模式类型
 **********************************************************************************************************************/
typedef uint8 dcm_ecu_start_mode_type;
#define DCM_COLD_START ((dcm_ecu_start_mode_type)0x00u) ///< 冷状态
#define DCM_WARM_START ((dcm_ecu_start_mode_type)0x01u) ///< 热状态

/*!*********************************************************************************************************************
 * @brief   协议地址类型
 **********************************************************************************************************************/
typedef uint8 dcm_protocol_addr_type_type;
#define DCM_PROTOCOL_FUNCTIONAL_ADDR_TYPE ((dcm_protocol_addr_type_type)1u) ///< 协议功能地址类型
#define DCM_PROTOCOL_PHYSICAL_ADDR_TYPE ((dcm_protocol_addr_type_type)2u) ///< 协议物理地址类型

/*!*********************************************************************************************************************
 * @brief   vsg标识类型
 **********************************************************************************************************************/
typedef uint8 dcm_vsg_id_type;

/*!*********************************************************************************************************************
 * @brief   vsg状态类型
 **********************************************************************************************************************/
typedef uint8 dcm_vsg_state_type;
#define DCM_VSG_ENABLED ((dcm_vsg_state_type)0u) ///< vsg使能
#define DCM_VSG_DISABLED ((dcm_vsg_state_type)1u) ///< vsg不使能

/*!*********************************************************************************************************************
 * @brief   程序条件类型
 **********************************************************************************************************************/
typedef struct {
	uint8 protocol_id; ///< 协议id
	uint8 tester_source_add; ///< 测试源地址
	uint8 sid; ///< 标识
	uint8 sub_fnc_id; ///< 子功能id
	boolean reprograming_request; ///< 程序请求
	boolean appl_updated; ///< 应用更新
	boolean response_required; ///< 响应需求
} dcm_prog_conditions_type;

/*!*********************************************************************************************************************
 * @brief   消息类型
 **********************************************************************************************************************/
typedef uint8 *dcm_msg_type;
typedef struct {
	boolean req_type; ///< 请求类型
	boolean suppress_pos_response; ///< 抑制正响应
} dcm_msg_add_info_type;

/*!*********************************************************************************************************************
 * @brief   id背景类型
 **********************************************************************************************************************/
typedef uint8 dcm_id_context_type;
/*!*********************************************************************************************************************
 * @brief   消息长度类型
 **********************************************************************************************************************/
typedef uint32 dcm_msg_len_type;

/*!*********************************************************************************************************************
 * @brief  消息背景类型
 **********************************************************************************************************************/
typedef struct {
	dcm_id_context_type id_context; ///< 标识背景
	PduIdType dcm_rx_pdu_id; ///< 接收pduid
	dcm_msg_type req_data; ///< 请求数据
	dcm_msg_type res_data; ///< 响应数据
	dcm_msg_add_info_type msg_add_info; ///< 消息地址信息
	dcm_msg_len_type req_data_len; ///< 请求数据长度
	dcm_msg_len_type res_data_len; ///< 响应数据长度
	dcm_msg_len_type res_max_data_len; ///< 响应最数据长度
} dcm_msg_context_type;

#ifdef __cplusplus
}
#endif

#endif /*DCM_TYPES_H*/
