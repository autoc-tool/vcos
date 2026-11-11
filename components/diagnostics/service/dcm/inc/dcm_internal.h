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

#ifndef DCM_INTERNAL_H
#define DCM_INTERNAL_H

#include "dcm.h"
#if (DCM_DEV_ERROR_DETECT == STD_ON) && (CONFIG_MODULE_DET == 1)
#include "Det.h"
#endif

#if (DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION == STD_ON) || (DCM_USE_SERVICE_READDTCINFORMATION == STD_ON) ||        \
	(DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON) ||                                                               \
	(DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA == STD_ON) ||                                                \
	(DCM_USE_SERVICE_CLEAREMISSIONRELATEDDIAGNOSTICDATA == STD_ON) ||                                              \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES == STD_ON) ||                                    \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE == STD_ON) ||      \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS == STD_ON)
#if (CONFIG_MODULE_DEM == 1)
#include "dem.h"
#else
#warning "Dcm: UDS services 0x14, 0x18 and/or 0x85 will not work without Dem."
#warning "Dcm: OBD services $02, $03, $04, $07 and/or $0A will not work without Dem."
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && (0 < DCM_NOF_IOCONTROL_DIDS)
#define DCM_USE_CONTROL_DIDS ///< 控制did
#endif

#if (DCM_DEV_ERROR_DETECT == STD_ON) && (CONFIG_MODULE_DET == 1)
#define DCM_DET_REPORTERROR(_error)                                                                                    \
	do {                                                                                                           \
		(void)det_report_error_with_info((uint32)_error, NULL_PTR, 0);                                         \
	} while (0) ///< det错误上报
#else
#define DCM_DET_REPORTERROR(_error) ///< det错误上报
#endif

#if (DCM_USE_SERVICE_REQUESTTRANSFEREXIT == STD_ON) || (DCM_USE_SERVICE_TRANSFERDATA == STD_ON) ||                     \
	(DCM_USE_SERVICE_REQUESTDOWNLOAD == STD_ON) || (DCM_USE_SERVICE_REQUESTUPLOAD == STD_ON) ||                    \
	(DCM_USE_SERVICE_REQUESTFILETRANSFER == STD_ON)
#define DCM_USE_UPLOAD_DOWNLOAD
#endif

#define ZERO_SUB_FUNCTION 0x00
#define DCM_FORMAT_LOW_MASK (uint8)0x0f
#define DCM_FORMAT_HIGH_MASK (uint8)0xf0
#define DCM_DID_HIGH_MASK (uint16)0xff00
#define DCM_DID_LOW_MASK (uint16)0xff

#define SID_LEN 1u
#define SF_INDEX 1u
#define SF_LEN 1u
#define DTC_LEN 3u
#define FF_REC_NUM_LEN 1u

#define OFFSET_ONE_BYTE 8u
#define OFFSET_TWO_BYTES 16u
#define OFFSET_THREE_BYTES 24u

#define ALFID_INDEX 1u
#define ALFID_LEN 1u
#define ADDR_START_INDEX 2u

#define IOI_INDEX 1u
#define IOI_LEN 2u
#define IOCP_INDEX 3u
#define IOCP_LEN 1u
#define COR_INDEX 4u
#define DCM_DSP_BIT_SIG(_x) (uint8)(1u << (7u - ((_x) % 8u)))

#define DCM_DSP_DEFAULT_PID_VALUE 0u
#define DCM_DSP_DEFAULT_DATA_VALUE 0u
#define DCM_DSP_DEFAULT_INFO_VALUE 0u
#define DCM_DSP_PID_SIZE 1u
#define DCM_DSP_DEFAULT_RECORD_NUM 0u
#define DCM_DSP_SUP_PIDS_DATA_SIZE 4u
#define DCM_DSP_OBD_LSB_MASK ((uint32)0x000000ffu)
#define DCM_DSP_OBD_REQ_PSG_SIZE 2u
#define DCM_DSP_OBD_REQ_MSG_MAX_SIZE 7u
#define DCM_DSP_SUP_PID_MIN_OFFSET 0x01u
#define DCM_DSP_SUP_PID_MAX_OFFSET 0x20u
#define DCM_DSP_SUP_MIN_INFO 0x01u
#define DCM_DSP_SUP_MAX_INFO 0x20u
#define DCM_DSP_DTC_SIZE 2u

#define DCM_DSP_CTP_IDX 2u
#define DCM_DSP_IS_SUP_0x28_SUBFUNC(_x) ((_x) < 4u)
#define DCM_DSP_UDS_0x28_NUM_COMM_TP 3u
#define DCM_DSP_UDS_0x28_SUB_FUNC_NUM 4u
#define DCM_DSP_IS_EN_0x28_COMM_TP(_x) (((_x) > 0u) && ((_x) < 4u))
#define DCM_DSP_FF_NUM_SIZE 1u
#define DCM_DSP_DTC_SIZE 2u
#define DCM_DSP_OBD_TWO_SVC ((uint8)0x02u)
#define DCM_DSP_OBD_REQ_MSG_MIN_SIZE_TWO 3u
#define DCM_DSP_DATA_ELMT_PID_NOT_SUP_IDX 0
#define DCM_DSP_OBD_SVC_2_PID_AND_FRM_LEN 2u
#define DCM_DSP_OBD_PID_IDX_SVC_2 0u
#define DCM_DSP_OBD_FRM_IDX_SVC_2 1u

#define DCM_DSP_TX_DATA_BIT_0_POS_RESP 0x59u
#define DCM_DSP_TX_DATA_BIT_1_SUB_ID_03 0x03u
#define DCM_DSP_TX_DATA_BIT_1_SUB_ID_04 0x04u

#define DCM_DSP_TIMING_DATA_LEN 4u
#define DCM_DSP_READ_DTC_INFO_LENGTH 0x16u
#define DCM_DSP_READ_DTC_INFO_SUB_ID_0x01 0x01u
#define DCM_DSP_READ_DTC_INFO_SUB_ID_0x06 0x06u
#define DCM_DSP_READ_DTC_INFO_SUB_ID_0x07 0x07u
#define DCM_DSP_READ_DTC_INFO_SUB_ID_0x10 0x10u
#define DCM_DSP_READ_DTC_INFO_SUB_ID_0x11 0x11u
#define DCM_DSP_READ_DTC_INFO_SUB_ID_0x12 0x12u
#define DCM_DSP_READ_DTC_INFO_SUB_ID_0x15 0x15u

#define DCM_DSP_EMS_BYTE_HIGH_DTCS(dtc) (((uint32)(dtc) >> 16) & 0xffu)
#define DCM_DSP_EMS_BYTE_LOW_DTCS(dtc) (((uint32)(dtc) >> 8) & 0xffu)

#define DCM_DSP_IS_AVAIL_PID(_x) ((0u == ((_x) % 0x20u)) && ((_x) <= 0xe0u))
#define DCM_DSP_IS_AVAIL_INFO_TYPE(_x) DCM_DSP_IS_AVAIL_PID(_x)

#define BYTES_TO_DTC(hb, mb, lb) (((uint32)(hb) << 16) | ((uint32)(mb) << 8) | (uint32)(lb))

#define TIMER_DECREMENT(timer)                                                                                         \
	do {                                                                                                           \
		if ((timer) >= DCM_MAIN_FUNCTION_PERIOD_TIME_MS) {                                                     \
			(timer) = (timer)-DCM_MAIN_FUNCTION_PERIOD_TIME_MS;                                            \
		}                                                                                                      \
	} while (0) ///< 计时器递减宏定义

#define TESTER_PRESENT_SDU_LEN 2u

#define DCM_ENTER_EXCLUSIVE_AREA_0 SchM_Enter_DCM_EXCLUSIVE_AREA_0
#define DCM_EXIT_EXCLUSIVE_AREA_0 SchM_Exit_DCM_EXCLUSIVE_AREA_0

typedef uint8 dsp_memory_service_type;
#define DCM_READ_MEMORY (dsp_memory_service_type)0
#define DCM_WRITE_MEMORY (dsp_memory_service_type)1

typedef uint8 dcm_dsp_reset_state_type;
#define DCM_DSP_RESET_NO_RESET (dcm_dsp_reset_state_type)0
#define DCM_DSP_RESET_PENDING (dcm_dsp_reset_state_type)1
#define DCM_DSP_RESET_WAIT_TX_CONF (dcm_dsp_reset_state_type)2

typedef uint8 dsp_jump_to_boot_state;
#define DCM_JTB_IDLE (dsp_jump_to_boot_state)0
#define DCM_JTB_WAIT_RESPONSE_PENDING_TX_CONFIRM (dsp_jump_to_boot_state)1
#define DCM_JTB_EXECUTE (dsp_jump_to_boot_state)2
#define DCM_JTB_RESAPP_FINAL_RESPONSE_TX_CONFIRM (dsp_jump_to_boot_state)3
#define DCM_JTB_RESAPP_WAIT_RESPONSE_PENDING_TX_CONFIRM (dsp_jump_to_boot_state)4
#define DCM_JTB_RESAPP_ASSEMBLE_FINAL_RESPONSE (dsp_jump_to_boot_state)5

typedef uint8 general_pend_state_type;
#define DCM_GENERAL_IDLE (general_pend_state_type)0u
#define DCM_GENERAL_PENDING (general_pend_state_type)1u
#define DCM_GENERAL_FORCE_RCRRP_AWAITING_SEND (general_pend_state_type)2u
#define DCM_GENERAL_FORCE_RCRRP (general_pend_state_type)3u

typedef uint8 delay_timer_type;
#define DELAY_TIMER_DEACTIVE (delay_timer_type)0u
#define DELAY_TIMER_ON_BOOT_ACTIVE (delay_timer_type)1u
#define DELAY_TIMER_ON_EXCEEDING_LIMIT_ACTIVE (delay_timer_type)2u

typedef uint8 dcm_dsp_memory_state_type;
#define DCM_MEMORY_UNUSED      ((dcm_dsp_memory_state_type)0x00u)
#define DCM_MEMORY_READ        ((dcm_dsp_memory_state_type)0x01u)
#define DCM_MEMORY_WRITE       ((dcm_dsp_memory_state_type)0x02u)
#define DCM_MEMORY_FAILED      ((dcm_dsp_memory_state_type)0x03u)

typedef struct {
	uint8 pend_svc;
	general_pend_state_type state;
	const PduInfoType *pdu_rx_data;
	PduInfoType *pdu_tx_data;
} dsp_uds_general_pending_type;

typedef struct {
	dcm_dsp_reset_state_type reset_pending;
	dcm_ecu_rst_type reset_type;
	PduIdType reset_pdu_id;
	PduInfoType *pdu_tx_data;
} dsp_uds_ecu_reset_data_type;

typedef struct {
	uint8 sec_acss_attempts;
	delay_timer_type start_delay_timer;
	uint16 timer_sec_acss_attempt;
} dsp_uds_security_access_chk_param;

typedef struct {
	dcm_protocol_type protocol_id;
	dcm_ses_ctrl_type session;
	dsp_jump_to_boot_state jump_to_boot_state;
	boolean pending_session_change;
	uint16 P2;
	uint16 p2star;
	PduIdType session_pdu_id;
	const PduInfoType *pdu_rx_data;
	PduInfoType *pdu_tx_data;
} dsp_uds_session_control_data_type;

typedef struct {
	boolean com_ctrl_pending;
	uint8 sub_func;
	uint8 subnet;
	uint8 com_type;
	PduIdType confirm_pdu;
	PduIdType rx_pdu;
} dsp_uds_comm_ctrl_data_type;

#if (DCM_RESPONSE_MULTIPLE_PENDING_ENABLED == STD_ON)
typedef struct {
	general_pend_state_type state;
	const PduInfoType *pdu_rx_data;
	PduInfoType *pdu_tx_data;
	PduIdType pdur_tx_pdu_id;
	boolean resp_pend_on_trans_to_boot;
	boolean internal_startup_request;
} dsp_dsc_programming_pending_type;
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON)
typedef uint8 dcm_dsp_io_ctrl_vector[(DCM_MAX_IOCONTROL_DID_SIGNALS + 7) / 8];
typedef struct {
	boolean ctrl_activated;
	boolean pending_ctrl;
	general_pend_state_type state;
	dcm_dsp_io_ctrl_vector signal_affected;
	uint16 pending_signal_index;
	const PduInfoType *pdu_rx_data;
	PduInfoType *pdu_tx_data;
} dsp_uds_io_ctrl_pending_type;

typedef struct {
	boolean ctrl_active;
	dcm_dsp_io_ctrl_vector act_sig_bitfld;
	uint16 did;
} dcm_dsp_io_ctrl_state_type;
#endif

typedef struct {
	boolean req_in_progress;
	dcm_sec_lv_type req_sec_level;
#if (DCM_SECURITY_EOL_INDEX != 0)
	uint8 curr_sec_lev_idx;
	dsp_uds_security_access_chk_param sec_false_attempt_chk[DCM_SECURITY_EOL_INDEX];
#endif
	const dcm_dsp_security_type *req_sec_level_ref;
} dsp_uds_security_access_data_type;

#if (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
typedef struct {
	boolean setting_disabled;
	dem_dtc_group_type dtc_group;
} dcm_dsp_ctrl_dtc_set_type;
#endif

typedef uint8 dcm_data_transfer_type;
#define DCM_NO_DATA_TRANSFER (dcm_data_transfer_type)0
#define DCM_UPLOAD (dcm_data_transfer_type)1
#define DCM_DOWNLOAD (dcm_data_transfer_type)2
#define DCM_WRITEFILE (dcm_data_transfer_type)3
#define DCM_READFILE (dcm_data_transfer_type)4

typedef uint8 dcm_file_transfer_type;
#define DCM_NO_FILE_TRANSFER (dcm_file_transfer_type)0
#define DCM_ADD_FILE (dcm_file_transfer_type)1
#define DCM_DELETE_FILE (dcm_file_transfer_type)2
#define DCM_REPLACE_FILE (dcm_file_transfer_type)3
#define DCM_READ_FILE (dcm_file_transfer_type)4
#define DCM_READ_DIR (dcm_file_transfer_type)5

typedef struct {
	uint8 block_sequence_counter;
	dcm_data_transfer_type transfer_type;
	dcm_file_transfer_type file_transfer_type;
	boolean first_block_received;
	uint32 next_address;
	uint32 upl_bytes_left;
	uint32 upl_mem_block_size;
} dcm_transfer_status_type;

#define DCM_SID_DIAGNOSTIC_SESSION_CONTROL (0x10u) ///< 诊断会话控制服务id
#define DCM_SID_ECU_RESET (0x11u) ///< ecu重置服务id
#define DCM_SID_CLEAR_DIAGNOSTIC_INFORMATION (0x14u) ///< 清除诊断信息服务id
#define DCM_SID_READ_DTC_INFORMATION (0x19u) ///< 读dtc信息服务id
#define DCM_SID_READ_DATA_BY_IDENTIFIER (0x22u) ///< 通过标识读数据服务id
#define DCM_SID_READ_MEMORY_BY_ADDRESS (0x23u) ///< 通过地址读内存服务id
#define DCM_SID_READ_SCALING_DATA_BY_IDENTIFIER (0x24u) ///< 通过标识对缩放数据服务id
#define DCM_SID_SECURITY_ACCESS (0x27u) ///< 安全接入服务id
#define DCM_SID_COMMUNICATION_CONTROL (0x28u) ///< 通信控制服务id
#define DCM_SID_READ_DATA_BY_PERIODIC_IDENTIFIER (0x2au) ///< 通过周期标识读数据服务id
#define DCM_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER (0x2cu) ///< 动态定义数据标识服务id
#define DCM_SID_WRITE_DATA_BY_IDENTIFIER (0x2eu) ///< 通过标识写数据服务id
#define DCM_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER (0x2fu) ///< 通过标识输入输出控制服务id
#define DCM_SID_ROUTINE_CONTROL (0x31u) ///< 历程控制服务id
#define DCM_SID_REQUEST_DOWNLOAD (0x34u) ///< 请求下载服务id
#define DCM_SID_REQUEST_UPLOAD (0x35u) ///< 请求上传服务id
#define DCM_SID_TRANSFER_DATA (0x36u) ///< 发送数据服务id
#define DCM_SID_REQUEST_TRANSFER_EXIT (0x37u) ///< 请求发送退出服务id
#define DCM_SID_REQUEST_FILE_TRANSFER (0x38u) ///< 请求文件发送服务id
#define DCM_SID_WRITE_MEMORY_BY_ADDRESS (0x3du) ///< 通过地址写内存服务id
#define DCM_SID_TESTER_PRESENT (0x3eu) ///< 测试器不在线服务id
#define DCM_SID_CONTROL_DTC_SETTING (0x85u) ///< 控制dtc设置服务id
#define DCM_SID_RESPONSE_ON_EVENT (0x86u) ///< 事件响应服务id
#define DCM_SID_LINK_CONTROL (0x87u) ///< 链路控制服务id
#define DCM_SID_NEGATIVE_RESPONSE (0x7fu) ///< 负响应7f定义

#define DCM_SID_REQUEST_CURRENT_POWERTRAIN_DIAGNOSTIC_DATA (0x01u) ///< 请求当前诊断数据
#define DCM_SID_REQUEST_POWERTRAIN_FREEZE_FRAME_DATA (0x02u) ///< 请求冻结帧数据
#define DCM_SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES (0x03u) ///< 请求相关错误码
#define DCM_SID_CLEAR_EMISSION_RELATED_DIAGNOSTIC_INFORMATION (0x04u) ///< 清除诊断信息
#define DCM_SID_REQUEST_ON_BOARD_MONITORING_TEST_RESULTS_SPECIFIC_MONITORED_SYSTEMS (0x06u) ///< 请求监控结果
#define DCM_SID_REQ_EMIS_DIAG_TROUBLE_CODES_DUR_CURR_OR_LAST_COMPL_DRIVING_CYCLE (0x07u) ///< 请求诊断错误码
#define DCM_SID_REQUEST_VEHICLE_INFORMATION (0x09u) ///< 请求车辆信息
#define DCM_SID_REQUEST_EMISSION_RELATED_DIAGNOSTIC_TROUBLE_CODES_WITH_PERMANENT_STATUS (0x0au) ///< 请求状态

#define SUPPRESS_POS_RESP_BIT ((uint8)0x80u) ///< 抑制响应位
#define DCM_SID_RESPONSE_BIT ((uint8)0x40u) ///< 响应位服务id
#define DCM_VALUE_IS_NOT_USED ((uint8)0x00u) ///< 未使用值

#define DCM_E_RESPONSEPENDING ((dcm_nrc_type)0x78u) ///< 响应结束错误码，仅在dcm内部使用

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b)) ///< 最小值宏定义
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b)) ///< 最大值宏定义
#endif

/*!*********************************************************************************************************************
 * @brief   定义dsd过程处理完毕结果类型
 **********************************************************************************************************************/
typedef uint8 dsd_processing_done_result_type;
#define DCM_DSD_TX_RESPONSE_READY ((dsd_processing_done_result_type)0u) ///< 发送响应就绪
#define DCM_DSD_TX_RESPONSE_SUPPRESSED ((dsd_processing_done_result_type)1u) ///< 发送响应抑制

/*!*********************************************************************************************************************
 * @brief   定义读did状态类型
 **********************************************************************************************************************/
typedef uint8 read_did_pending_state_type;
#define DCM_READ_DID_IDLE ((read_did_pending_state_type)0u) ///< 读did空闲
#define DCM_READ_DID_PENDING_COND_CHECK ((read_did_pending_state_type)1u) ///< 读did检查
#define DCM_READ_DID_PENDING_READ_DATA ((read_did_pending_state_type)2u) ///< 读did数据

#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
/*!*********************************************************************************************************************
 * @brief   定义读vid状态类型
 **********************************************************************************************************************/
typedef uint8 read_vid_pending_state_type;
#define DCM_READ_VID_IDLE ((read_vid_pending_state_type)0u) ///< 读vid空闲
#define DCM_READ_VID_PENDING_READ_DATA ((read_vid_pending_state_type)1u) ///< 读vid数据
#endif
typedef uint8 dcm_com_mode_type;
#define DCM_NO_COM ((dcm_com_mode_type)0u) ///< 无通信
#define DCM_SILENT_COM ((dcm_com_mode_type)1u) ///< 静默通信
#define DCM_FULL_COM ((dcm_com_mode_type)2u) ///< 全通信

/*!*********************************************************************************************************************
 * @brief   定义协议请求类型
 **********************************************************************************************************************/
typedef struct {
	read_did_pending_state_type state; ///< 状态
	uint16 tx_write_pos; ///< 发送写正响应
	uint16 nof_read_dids; ///< 读did个数
	uint16 req_did_index; ///< 请求did索引
	uint16 pending_did; ///< 预留did
	uint16 pending_data_length; ///< 预留数据长度
	uint16 pending_signal_index; ///< 预留信号索引
	uint16 pending_data_start_pos; ///< 预留数据起始正响应
	const PduInfoType *pdu_rx_data; ///< pdu接收数据
	PduInfoType *pdu_tx_data; ///< pdu发送数据
} dsp_uds_read_did_pending_type;

#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
/*!*********************************************************************************************************************
 * @brief   定义获取信息类型值类型
 **********************************************************************************************************************/
typedef struct {
	read_vid_pending_state_type state; ///< 状态
	boolean is_canceled; ///< 是否取消
	uint8 dataindex; ///< 数据索引
	uint8 data_value_buffer_size; ///< 数据值缓存大小
	uint16 data_pos; ///< 数据正响应
	uint8 *data_value_buffer; ///< 数据值缓存
	const dcm_dsp_veh_info_type *source_veh_info_ptr; ///< 源信息指针
	const PduInfoType *pdu_rx_data; ///< pdu接收数据
	PduInfoType *pdu_tx_data; ///< pdu发送数据
} dsp_obd_get_infotype_value_pending_type;
#endif

#define DCM_START_SEC_CODE
#include "dcm_memmap.h"

/*!*********************************************************************************************************************
 * @brief          dsp初始化
 * @details        dsp初始化
 * @param[in]      first_call 首次调用
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_init(boolean first_call);

/*!*********************************************************************************************************************
 * @brief          dsp主函数
 * @details        dsp主函数
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_main_function(void);

/*!*********************************************************************************************************************
 * @brief          dsp处理完成不给响应
 * @details        dsp处理完成不给响应
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_dsp_processing_done_with_no_response(void);

/*!*********************************************************************************************************************
 * @brief          dsp预dsd主函数
 * @details        dsp预dsd主函数
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_pre_dsd_main_function(void);

/*!*********************************************************************************************************************
 * @brief          dsp计时器主函数
 * @details        dsp计时器主函数
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_timer_main_function(void);

#if (DCM_USE_SERVICE_CLEARDIAGNOSTICINFORMATION == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp清除诊断信息
 * @details        dsp清除诊断信息
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      client_id 客户端id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_uds_clear_diag_info(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id);
#endif

#if (DCM_USE_SERVICE_DIAGNOSTICSESSIONCONTROL == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp诊断会话控制
 * @details        dsp诊断会话控制
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      tx_pdu_id 发送pduid
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      resp_pend_on_trans_to_boot 响应转换到boot
 * @param[in]      internal_startup_request 内部启动请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_uds_diag_ses_ctrl(const PduInfoType *pdu_rx_data, PduIdType tx_pdu_id, PduInfoType *pdu_tx_data,
	boolean resp_pend_on_trans_to_boot, boolean internal_startup_request);
#endif

#if (DCM_USE_SERVICE_TESTERPRESENT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp测试离线
 * @details        dsp测试离线
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_tester_present(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_ECURESET == STD_ON)
/*!*********************************************************************************************************************
 * @brief          DSP ecu重启
 * @details        DSP ecu重启
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      tx_pdu_id 发送pduid
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      startup_response_request 启动请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_uds_ecu_reset(const PduInfoType *pdu_rx_data, PduIdType tx_pdu_id, PduInfoType *pdu_tx_data,
	boolean startup_response_request);
#endif

#if (DCM_USE_SERVICE_SECURITYACCESS == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp安全接入
 * @details        dsp安全接入
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_security_access(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_READDATABYIDENTIFIER == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp通过标识读数据
 * @details        dsp通过标识读数据
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_read_data_by_id(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp控制dtc设置
 * @details        dsp控制dtc设置
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      client_id 客户端id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_uds_ctrl_dtc_set(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id);
#endif

/*!*********************************************************************************************************************
 * @brief          DSP dcm确认
 * @details        DSP dcm确认
 * @param[in]      confirm_pdu_id 确认pdu_id
 * @param[in]      result 结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_dcm_confirmation(PduIdType confirm_pdu_id, Std_ReturnType result);

#if (DCM_USE_SERVICE_READMEMORYBYADDRESS == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp通过地址读内存
 * @details        dsp通过地址读内存
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_read_memory_by_addr(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp通过地址io控制
 * @details        dsp通过地址io控制
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_io_ctrl_by_data_id(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_ROUTINECONTROL == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp历程控制
 * @details        dsp历程控制
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_routine_ctrl(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

/*!*********************************************************************************************************************
 * @brief          dsp重置诊断激活
 * @details        dsp重置诊断激活
 * @param[in]      protocol_row_entry 协议入口
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_dsp_reset_diag_activity(const dcm_dsl_protocol_row_type *protocol_row_entry);

#if (DCM_USE_SERVICE_WRITEMEMORYBYADDRESS == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp通过地址写内存
 * @details        dsp通过地址写内存
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_write_memory_by_addr(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp通过地址io控制
 * @details        dsp通过地址io控制
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      rx_pdu_id 接收pdu_id
 * @param[in]      tx_confirm_id 发送确认id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_communication_control(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, PduIdType rx_pdu_id,
	PduIdType tx_confirm_id);
#endif

/*!*********************************************************************************************************************
 * @brief          根据会话改变重置诊断激活
 * @details        根据会话改变重置诊断激活
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_reset_diag_activity_on_session_change(void);

#if (DCM_USE_SERVICE_WRITEDATABYIDENTIFIER == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp通过标识写数据
 * @details        dsp通过标识写数据
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_write_data_by_id(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDIAGNOSTICTROUBLECODES == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp请求相关诊断故障码
 * @details        DSP obd清除相关诊断数据
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      client_id 客户端id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_obd_req_emission_related_diag_trouble_codes(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data,
	uint8 client_id);
#endif

#if (DCM_USE_SERVICE_REQUESTDOWNLOAD == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp请求下载
 * @details        dsp请求下载
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_request_download(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_REQUESTUPLOAD == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp请求上传
 * @details        dsp请求上传
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_request_upload(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTPOWERTRAINFREEZEFRAMEDATA == STD_ON)
/*!*********************************************************************************************************************
 * @brief          DSP obd请求当前动力系统冻结帧数据
 * @details        DSP obd请求当前动力系统冻结帧数据
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_obd_req_powertrain_freeze_frame_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_TRANSFERDATA == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp转换数据
 * @details        dsp转换数据
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_transfer_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_REQUESTTRANSFEREXIT == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp请求转换退出
 * @details        dsp请求转换退出
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_request_transfer_exit(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_REQUESTFILETRANSFER == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp请求文件转换
 * @details        dsp请求文件转换
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_uds_req_file_transfer(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (DCM_USE_SERVICE_READDTCINFORMATION == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp读dtc信息
 * @details        dsp读dtc信息
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      client_id 客户端id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_uds_read_dtc_info(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id);
#endif

#if (DCM_USE_SERVICE_REQUESTCURRENTPOWERTRAINDIAGNOSTICDATA == STD_ON)
/*!*********************************************************************************************************************
 * @brief          DSP obd请求当前动力系统断数据
 * @details        DSP obd请求当前动力系统诊断数据
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_obd_req_cur_powertrain_diag_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CLEAREMISSIONRELATEDDIAGNOSTICDATA == STD_ON)
/*!*********************************************************************************************************************
 * @brief          DSP obd清除相关诊断数据
 * @details        DSP obd清除相关诊断数据
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      client_id 客户端id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_obd_clear_emission_related_diag_data(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data,
	uint8 client_id);
#endif

#if (CONFIG_MODULE_DEM == 1) &&                                                                                        \
	(DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSDETECTEDDURINGCURRENTORLASTCOMPLETEDDRIVINGCYCLE == STD_ON)
FUNC(void, DCM_CODE)
dsp_obd_req_emis_dtc_serv07(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id);
#endif

#if (DCM_USE_SERVICE_REQUESTVEHICLEINFORMATION == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp请求车辆信息
 * @details        dsp请求车辆信息
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_obd_req_vehicle_info(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data);
#endif

#if (CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_REQUESTEMISSIONRELATEDDTCSWITHPERMANENTSTATUS == STD_ON)
/*!*********************************************************************************************************************
 * @brief          dsp请求dtc
 * @details        dsp请求dtc
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      client_id 客户端id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsp_obd_req_emis_dtc_with_perm_st(const PduInfoType *pdu_rx_data, PduInfoType *pdu_tx_data, uint8 client_id);
#endif

#define DCM_GET_MASK_BIT(var, index) (((var) >> (index)) & 0x01u) ///< 获取掩码比特

/*!*********************************************************************************************************************
 * @brief          dsp检测会话等级
 * @details        dsp检测会话等级
 * @param[in]      session_level_mask 会话等级掩码
 * @retval         TRUE dsp检测会话等级成功
 * @retval         FALSE dsp检测会话等级失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE) dsp_check_session_level(const uint8 session_level_mask);

/*!*********************************************************************************************************************
 * @brief          dsp取消请求
 * @details        dsp取消请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_cancel_pending_req(void);

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
/*!*********************************************************************************************************************
 * @brief          dsp响应确认
 * @details        dsp响应确认
 * @param[in]      confirm_pdu_id 确认pdu_id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_response_pending_confirmed(PduIdType confirm_pdu_id);
#endif

/*!*********************************************************************************************************************
 * @brief          dsp检测安全等级
 * @details        dsp检测安全等级
 * @param[in]      security_level_mask 安全等级掩码
 * @retval         TRUE dsp检测安全等级成功
 * @retval         FALSE dsp检测安全等级失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE) dsp_check_security_level(const uint8 security_level_mask);

/*!*********************************************************************************************************************
 * @brief          dsp检测会话是否支持
 * @details        dsp检测会话是否支持
 * @param[in]      session 会话
 * @retval         TRUE dsp检测会话支持
 * @retval         FALSE dsp检测会话不支持
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE) dsp_dsl_check_session_supported(uint8 session);

/*!*********************************************************************************************************************
 * @brief          dsp检测协议开始请求
 * @details        dsp检测协议开始请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsp_check_protocol_start_requests(void);

/*!*********************************************************************************************************************
 * @brief          dsd初始化
 * @details        dsd初始化
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_init(void);

/*!*********************************************************************************************************************
 * @brief          dsd主函数
 * @details        dsd主函数
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_main_function(void);

/*!*********************************************************************************************************************
 * @brief          dsp处理完成
 * @details        dsp处理完成
 * @param[in]      response_code 响应码
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_dsp_processing_done(dcm_nrc_type response_code);

/*!*********************************************************************************************************************
 * @brief          dsp数据确认
 * @details        dsp数据确认
 * @param[in]      confirm_pdu_id 确认pdu_id
 * @param[in]      result 结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_data_confirmation(PduIdType confirm_pdu_id, Std_ReturnType result);

/*!*********************************************************************************************************************
 * @brief          dsp数据指示
 * @details        dsp数据指示
 * @param[in]      pdu_rx_data pdu接收数据
 * @param[in]      protocol_row 协议
 * @param[in]      addr_type 地址类型
 * @param[in]      tx_pdu_id  发送pdu_id
 * @param[in]      pdu_tx_data pdu发送数据
 * @param[in]      rx_context_pdu_id 接收pdu_id
 * @param[in]      internal_request 内部请求
 * @param[in]      startup_response_request  启动响应请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsd_dsl_data_indication(const PduInfoType *pdu_rx_data, const dcm_dsl_protocol_row_type *protocol_row,
	dcm_protocol_addr_type_type addr_type, PduIdType tx_pdu_id, PduInfoType *pdu_tx_data,
	PduIdType rx_context_pdu_id, boolean internal_request, boolean startup_response_request);

/*!*********************************************************************************************************************
 * @brief          dsp获取当前发送pdu_id
 * @details        dsp获取当前发送pdu_id
 * @retval         PduIdType pdu_id范围
 * @reentrant      不可重入
 * @synchronous
 **********************************************************************************************************************/
FUNC(PduIdType, DCM_CODE) dsd_dsl_get_current_tx_pdu_id(void);
#if (DCM_USE_SERVICE_COMMUNICATIONCONTROL == STD_ON) ||                                                                \
	((CONFIG_MODULE_DEM == 1) && (DCM_USE_SERVICE_CONTROLDTCSETTING == STD_ON)) ||                                 \
	((DCM_USE_SERVICE_INPUTOUTPUTCONTROLBYIDENTIFIER == STD_ON) && defined(DCM_USE_CONTROL_DIDS))

/*!*********************************************************************************************************************
 * @brief          dsp检测服务支持激活会话和安全
 * @details        dsp检测服务支持激活会话和安全
 * @param[in]      security_level_mask 安全等级掩码
 * @retval         TRUE dsp检测服务支持激活会话和安全成功
 * @retval         FALSE dsp检测服务支持激活会话和安全失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE) dsd_dsp_check_service_supported_in_active_session_and_security(uint8 sid);
#endif

/*!*********************************************************************************************************************
 * @brief          dsp强制响应
 * @details        dsp强制响应
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_dsp_force_response_pending(void);

#if (defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
/*!*********************************************************************************************************************
 * @brief          dsp获取响应请求
 * @details        dsp获取响应请求
 * @retval         TRUE dsp获取响应请求成功
 * @retval         FALSE dsp获取响应请求失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE) dsd_dsp_get_response_required(void);

/*!*********************************************************************************************************************
 * @brief          dsp响应确认
 * @details        dsp响应确认
 * @param[in]      confirm_pdu_id 确认pdu_id
 * @param[in]      result 结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_response_pending_confirmed(PduIdType confirm_pdu_id, NotifResultType result);

/*!*********************************************************************************************************************
 * @brief          dsp获取测试源地址
 * @details        dsp获取测试源地址
 * @retval         0-65535 测试仪源地址
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint16, DCM_CODE) dsd_dsp_get_tester_source_address(void);
#endif

#if ((DCM_SERVICE_USER_DEFINED == STD_ON) || (DCM_SUB_SERVICE_USER_DEFINED == STD_ON))
/*!*********************************************************************************************************************
 * @brief          dsd外部设置响应
 * @details        dsd外部设置响应
 * @param[in]      p_msg_context 消息指针
 * @param[in]      error_code 错误码
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsd_external_set_neg_response(const dcm_msg_context_type *p_msg_context, dcm_nrc_type error_code);

/*!*********************************************************************************************************************
 * @brief          dsd外部处理完成
 * @details        dsd外部处理完成
 * @param[in]      p_msg_context 消息指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_external_processing_done(const dcm_msg_context_type *p_msg_context);

/*!*********************************************************************************************************************
 * @brief          dsd取消外部服务
 * @details        dsd取消外部服务
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_cancel_pending_external_services(void);
#endif

/*!*********************************************************************************************************************
 * @brief          dsd清除抑制正响应响应消息
 * @details        dsd清除抑制正响应响应消息
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsd_clear_suppress_pos_rsp_msg(void);

/*!*********************************************************************************************************************
 * @brief          dsl初始化
 * @details        dsl初始化
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_init(void);

/*!*********************************************************************************************************************
 * @brief          dsl主函数
 * @details        dsl主函数
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_main_function(void);

/*!*********************************************************************************************************************
 * @brief          dsd处理完成
 * @details        dsd处理完成
 * @param[in]      rx_pdu_id 接收pdu_id
 * @param[in]      response_result 响应结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_dsd_processing_done(PduIdType rx_pdu_id, dsd_processing_done_result_type response_result);

/*!*********************************************************************************************************************
 * @brief          dsl从pdur接收tp消息
 * @details        dsl从pdur接收tp消息
 * @param[in]      dcm_rx_pdu_id 接收pdu_id
 * @param[in]      result 接收结果
 * @param[in]      internal_request 内部请求
 * @param[in]      startup_response_request 启动响应请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE)
dsl_tp_rx_indication_from_pdur(PduIdType dcm_rx_pdu_id, NotifResultType result, boolean internal_request,
	boolean startup_response_request);

/*!*********************************************************************************************************************
 * @brief          dsl获取激活协议
 * @details        dsl获取激活协议
 * @param[in]      protocol_id 协议id
 * @retval         E_OK dsl获取激活协议成功
 * @retval         E_NOT_OK dsl获取激活协议失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dsl_get_active_protocol(dcm_protocol_type *protocol_id);

/*!*********************************************************************************************************************
 * @brief          dsl设置安全等级
 * @details        dsl设置安全等级
 * @param[in]      sec_level 安全等级
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_internal_set_security_level(dcm_sec_lv_type sec_level);

/*!*********************************************************************************************************************
 * @brief          dsl获取安全等级
 * @details        dsl获取安全等级
 * @param[in]      sec_level 安全等级
 * @retval         E_OK dsl获取安全等级成功
 * @retval         E_NOT_OK dsl获取安全等级失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dsl_get_security_level(dcm_sec_lv_type *sec_level);

/*!*********************************************************************************************************************
 * @brief          dsl设置会话控制类型
 * @details        dsl设置会话控制类型
 * @param[in]      ses_ctrl 会话控制
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_set_ses_ctrl_type(dcm_ses_ctrl_type ses_ctrl);

/*!*********************************************************************************************************************
 * @brief          dsl获取会话控制类型
 * @details        dsl获取会话控制类型
 * @param[in]      ses_ctrl_type 会话控制类型
 * @retval         E_OK dsl获取会话控制类型成功
 * @retval         E_NOT_OK dsl获取会话控制类型失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dsl_get_ses_ctrl_type(dcm_ses_ctrl_type *ses_ctrl_type);

/*!*********************************************************************************************************************
 * @brief          dsl tp发送确认
 * @details        dsl tp发送确认
 * @param[in]      dcm_tx_pdu_id 发送pdu_id
 * @param[in]      result 结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_tp_tx_confirmation(PduIdType dcm_tx_pdu_id, Std_ReturnType result);

/*!*********************************************************************************************************************
 * @brief          dsl静态启动协议
 * @details        dsl静态启动协议
 * @param[in]      session 会话
 * @param[in]      protocol_id 协议id
 * @param[in]      tester_source_address 测试源地址
 * @param[in]      request_full_comm 请求全通信
 * @retval         E_OK dsl静态启动协议成功
 * @retval         E_NOT_OK dsl静态启动协议失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dsl_dsp_silently_start_protocol(uint8 session, uint8 protocol_id, uint16 tester_source_address,
	boolean request_full_comm);

/*!*********************************************************************************************************************
 * @brief          dsl启动请求响应
 * @details        dsl启动请求响应
 * @param[in]      sid 服务id
 * @param[in]      sub_fnc 子功能
 * @param[in]      protocol_id 服务地址
 * @param[in]      tester_source_address 测试源地址
 * @retval         E_OK dsl启动请求响应成功
 * @retval         E_NOT_OK dsl启动请求响应失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dsl_dsp_response_on_startup_request(uint8 sid, uint8 sub_fnc, uint8 protocol_id, uint16 tester_source_address);

/*!*********************************************************************************************************************
 * @brief          dsl通信模式入口
 * @details        dsl通信模式入口
 * @param[in]      network 网络
 * @param[in]      com_mode 通信模式
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_com_mode_entered(uint8 network, dcm_com_mode_type com_mode);

/*!*********************************************************************************************************************
 * @brief          dsl重置会话超时定时器
 * @details        dsl重置会话超时定时器
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_reset_session_timeout_timer(void);

/*!*********************************************************************************************************************
 * @brief          dsl拷贝数据到接收缓存
 * @details        dsl拷贝数据到接收缓存
 * @param[in]      dcm_rx_pdu_id 接收pdu_id
 * @param[in]      pdu_info_ptr 信息指针
 * @param[in]      rx_buffer_size_ptr 接收缓存大小
 * @retval         BUFREQ_OK dsl拷贝数据到接收成功
 * @retval         BUFREQ_E_NOT_OK dsl拷贝数据到接收失败
 * @retval         BUFREQ_E_BUSY dsl拷贝数据到接收缓存繁忙
 * @retval         BUFREQ_E_OVFL dsl拷贝数据到接收缓存溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, DCM_CODE)
dsl_copy_data_to_rx_buffer(PduIdType dcm_rx_pdu_id, const PduInfoType *pdu_info_ptr, PduLengthType *rx_buffer_size_ptr);

/*!*********************************************************************************************************************
 * @brief          dsl开始接收
 * @details        dsl开始接收数据
 * @param[in]      dcm_rx_pdu_id 接收pdu_id
 * @param[in]      tp_sdu_length 消息长度
 * @param[in]      rx_buffer_size_ptr 接收缓存大小
 * @param[in]      internal_request 内部请求
 * @retval         BUFREQ_OK dsl拷贝数据到接收成功
 * @retval         BUFREQ_E_NOT_OK dsl拷贝数据到接收失败
 * @retval         BUFREQ_E_BUSY dsl拷贝数据到接收缓存繁忙
 * @retval         BUFREQ_E_OVFL dsl拷贝数据到接收缓存溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, DCM_CODE)
dsl_start_of_reception(PduIdType dcm_rx_pdu_id, PduLengthType tp_sdu_length, PduLengthType *rx_buffer_size_ptr,
	boolean internal_request);

/*!*********************************************************************************************************************
 * @brief          dsl拷贝发送数据
 * @details        dsl拷贝发送数据
 * @param[in]      dcm_tx_pdu_id 发送pdu_id
 * @param[in]      pdu_info_ptr 消息指针
 * @param[in]      retry 重试次数
 * @param[in]      tx_data_cnt_ptr 发送数据计数指针
 * @retval         BUFREQ_OK dsl拷贝数据到接收成功
 * @retval         BUFREQ_E_NOT_OK dsl拷贝数据到接收失败
 * @retval         BUFREQ_E_BUSY dsl拷贝数据到接收缓存繁忙
 * @retval         BUFREQ_E_OVFL dsl拷贝数据到接收缓存溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, DCM_CODE)
dsl_copy_tx_data(PduIdType dcm_tx_pdu_id, const PduInfoType *pdu_info_ptr, const RetryInfoType *retry,
	PduLengthType *tx_data_cnt_ptr);

#if ((DCM_MANUFACTURER_NOTIFICATION == STD_ON) || (DCM_SUPPLIER_NOTIFICATION == STD_ON)) ||                            \
	(defined(DCM_USE_JUMP_TO_BOOT) && (DCM_USE_JUMP_TO_BOOT == STD_ON))
/*!*********************************************************************************************************************
 * @brief          dsl获取接收链接参数
 * @details        dsl获取接收链接参数
 * @param[in]      rx_pdu_id 接收du_id
 * @param[in]      source_address 源地址
 * @param[in]      req_type 请求类型
 * @param[in]      protocol_id 协议id
 * @param[in]      connection_id 链接id
 * @retval         BUFREQ_OK dsl拷贝数据到接收成功
 * @retval         BUFREQ_E_NOT_OK dsl拷贝数据到接收失败
 * @retval         BUFREQ_E_BUSY dsl拷贝数据到接收缓存繁忙
 * @retval         BUFREQ_E_OVFL dsl拷贝数据到接收缓存溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
dsl_get_rx_connection_params(PduIdType rx_pdu_id, uint16 *source_address, dcm_protocol_addr_type_type *req_type,
	dcm_protocol_type *protocol_id, uint16 *connection_id);
#endif

/*!*********************************************************************************************************************
 * @brief          dsl发响应
 * @details        dsl发响应
 * @param[in]      rx_pdu_id 接收du_id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dsl_dsd_send_resp_pending(PduIdType rx_pdu_id);

/*!*********************************************************************************************************************
 * @brief          查找非动态did
 * @details        查找非动态did
 * @param[in]      did_nr did的nr
 * @param[in]      did_ptr didi指针
 * @retval         TRUE 查找非动态did成功
 * @retval         FALSE 查找非动态did失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE) lookup_non_dynamic_did(uint16 did_nr, const dcm_dsp_did_type **did_ptr);

/*!*********************************************************************************************************************
 * @brief          读did数据
 * @details        读did数据
 * @param[in]      did_start_index 启动索引
 * @param[in]      did_ptr didi指针
 * @param[in]      pdu_tx_data 发送数据
 * @param[in]      read_did_pending 读did
 * @retval         0-255 读did数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(dcm_nrc_type, DCM_CODE)
read_did_data(uint16 did_start_index, const dcm_dsp_did_type *did_ptr, PduInfoType *pdu_tx_data,
	dsp_uds_read_did_pending_type *read_did_pending);

/*!*********************************************************************************************************************
 * @brief          获取did数据
 * @details        获取did数据
 * @param[in]      did_ptr didi指针
 * @param[in]      pdu_tx_data 发送数据
 * @param[in]      read_did_pending 读did
 * @param[in]      include_did 包括did
 * @retval         0-255 获取did数据
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(dcm_nrc_type, DCM_CODE)
get_did_data(const dcm_dsp_did_type *did_ptr, const PduInfoType *pdu_tx_data,
	dsp_uds_read_did_pending_type *read_did_pending, boolean include_did);

/*!*********************************************************************************************************************
 * @brief          通过索引获取模式规则回调
 * @details        通过索引获取模式规则回调
 * @param[in]      idx 索引
 * @param[in]      nrc nrc
 * @retval         E_OK  通过索引获取模式规则回调成功
 * @retval         E_NOT_OK  通过索引获取模式规则回调失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE) dcm_mode_mode_rule_cbk_by_idx(const uint8 idx, dcm_mode_rule_nrc_value *nrc);

/*!*********************************************************************************************************************
 * @brief          重置诊断激活
 * @details        重置诊断激活
 * @param[in]      protocol_row_entry 协议入库
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_rst_diag_activity(const dcm_dsl_protocol_row_type *protocol_row_entry);

/*!*********************************************************************************************************************
 * @brief          会话改变时重置诊断激活
 * @details        会话改变时重置诊断激活
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_rst_diag_activity_on_session_change(void);

/*!*********************************************************************************************************************
 * @brief          取消pending请求
 * @details        取消pending请求
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, DCM_CODE) dcm_cancel_pending_req(void);

/*!*********************************************************************************************************************
 * @brief          读sr信号
 * @details        读sr信号
 * @param[in]      data_ptr 数据指针
 * @param[in]      bit_position 比特正响应
 * @param[in]      data_buffer 数据缓存
 * @retval         E_OK  读sr信号成功
 * @retval         E_NOT_OK  读sr信号失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
read_sr_signal(const dcm_dsp_data_type *data_ptr, uint16 bit_position, uint8 *data_buffer);

/*!*********************************************************************************************************************
 * @brief          写sr信号
 * @details        写sr信号
 * @param[in]      data_ptr 数据指针
 * @param[in]      bit_position 比特正响应
 * @param[in]      data_buffer 数据缓存
 * @retval         E_OK  写sr信号成功
 * @retval         E_NOT_OK  写sr信号失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, DCM_CODE)
write_sr_signal(const dcm_dsp_data_type *data_ptr, uint16 bit_position, const uint8 *data_buffer);

/*!*********************************************************************************************************************
 * @brief          通过字节获取影响个数
 * @details        通过字节获取影响个数
 * @param[in]      endianess 大小端
 * @param[in]      start_bit_pos 启动比特正响应
 * @param[in]      bit_length 比特长度
 * @retval         0-65535  通过字节获取影响个数成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint16, DCM_CODE)
get_nof_affected_bytes(dcm_dsp_data_endianess_type endianess, uint16 start_bit_pos, uint16 bit_length);

/*!*********************************************************************************************************************
 * @brief          检测did读条件
 * @details        检测did读条件
 * @param[in]      did_read 读did
 * @param[in]      response_code 响应码
 * @retval         TRUE  检测did读条件成功
 * @retval         FALSE  检测did读条件失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE)
do_check_did_read_condition(const dcm_dsp_did_read_type *did_read, dcm_nrc_type *response_code);

/*!*********************************************************************************************************************
 * @brief          检测did写条件
 * @details        检测did写条件
 * @param[in]      did_write 写did
 * @param[in]      response_code 响应码
 * @retval         TRUE  检测did写条件成功
 * @retval         FALSE  检测did写条件失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE)
do_check_did_write_condition(const dcm_dsp_did_write_type *did_write, dcm_nrc_type *response_code);

/*!*********************************************************************************************************************
 * @brief          检测did控制条件
 * @details        检测did控制条件
 * @param[in]      did_control 控制did
 * @param[in]      response_code 响应码
 * @retval         TRUE  检测did控制条件成功
 * @retval         FALSE  检测did控制条件失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE)
do_check_did_control_condition(const dcm_dsp_did_control_type *did_control,
	dcm_nrc_type *response_code);

/*!*********************************************************************************************************************
 * @brief          检测历程控制条件
 * @details        检测历程控制条件
 * @param[in]      routine 历程
 * @param[in]      response_code 响应码
 * @retval         TRUE  检测历程控制条件成功
 * @retval         FALSE 检测历程控制条件失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, DCM_CODE)
do_check_routine_control_condition(const dcm_dsp_routine_authorization_type routine,
	dcm_nrc_type *response_code);

#if (DCM_VSG_CHECK_ENABLED == STD_ON)
/*!*********************************************************************************************************************
 * @brief          通过vsg id映射获取vsg索引
 * @details        通过vsg id映射获取vsg索引
 * @param[in]      vsg_id vsg标识
 * @retval         0-255  通过vsg id映射获取vsg索引成功
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(uint8, DCM_CODE) dcm_internal_get_vsg_index_by_vsg_id_map(dcm_vsg_id_type vsg_id);
#endif

#define DCM_STOP_SEC_CODE
#include "dcm_memmap.h"

/*!*********************************************************************************************************************
 * @brief          声明dcm配置宏定义
 **********************************************************************************************************************/
#define DCM_START_SEC_PRIVATE_BSS
#include "dcm_memmap.h"
extern P2CONST(dcm_config_type, DCM_PRIVATE_BSS, DCM_CONST) dcm_config_ptr;
#define DCM_STOP_SEC_PRIVATE_BSS
#include "dcm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* DCM_INTERNAL_H */
