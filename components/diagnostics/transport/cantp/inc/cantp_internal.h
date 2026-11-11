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

#ifndef CANTP_INTERNAL_H
#define CANTP_INTERNAL_H

#include "cantp_types.h"
#include "cantp_cfg.h"
#include "cantp_pbcfg.h"
#if (CANTP_DEV_ERROR_DETECT == STD_ON) && (CONFIG_MODULE_DET == 1)
#include "Det.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (CANTP_DEV_ERROR_DETECT == STD_ON) && (CONFIG_MODULE_DET == 1)
#define CANTP_DET_REPORTERROR(_error)                                                                                  \
	do {                                                                                                           \
		(void)det_report_error_with_info((uint32)_error, NULL_PTR, 0);                                         \
	} while (0)

#else
#define CANTP_DET_REPORTERROR(_error)
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define CANTP_VALIDATE(_exp, _err)                                                                                     \
	do {                                                                                                           \
		if (!(_exp)) {                                                                                         \
			CANTP_DET_REPORTERROR(_err);                                                                   \
			return E_NOT_OK;                                                                               \
		}                                                                                                      \
	} while (0)

#define CANTP_VALIDATE_NO_RV(_exp, _err)                                                                               \
	do {                                                                                                           \
		if (!(_exp)) {                                                                                         \
			CANTP_DET_REPORTERROR(_err);                                                                   \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

#define COUNT_DECREMENT(timer)                                                                                         \
	do {                                                                                                           \
		if ((timer) > (uint16)0) {                                                                             \
			(timer) = (timer) - (uint16)1;                                                                 \
		}                                                                                                      \
	} while (0)

#define CANTP_ISO15765_FLOW_CONTROL_STATUS_CTS (0U) ///< 流控帧cts状态
#define CANTP_ISO15765_FLOW_CONTROL_STATUS_WAIT (1U) ///< 流控帧等待状态
#define CANTP_ISO15765_FLOW_CONTROL_STATUS_OVFLW (2U) ///< 流控帧溢出状态

#define CANTP_ISO15765_TPCI_MASK (0xf0u) ///< pci掩码
#define CANTP_ISO15765_TPCI_SF (0x00u) ///< 单帧(single frame) bit7-4 恒为 0
#define CANTP_ISO15765_TPCI_FF (0x10u) ///< 首帧(first frame) bit7-4 恒为 1
#define CANTP_ISO15765_TPCI_CF (0x20u) ///< 连续帧(consecutive frame) bit7-4 恒为2
#define CANTP_ISO15765_TPCI_FC (0x30u) ///< 流控帧(flow control) bit7-4 恒为3
#define CANTP_ISO15765_TPCI_DL (0xfu) ///< 单帧 SF_DL mask
#define CANTP_ISO15765_TPCI_DL_FD (0xffu) ///< 单帧(CANFD) SF_DL mask
#define CANTP_ISO15765_TPCI_FS_MASK (0x0fu) ///< 流控帧 FS mask

#define CANTP_CANFD_FIRST_BYTE_ZERO (0U) ///< 首字节为0

#define CANTP_SEGMENT_NUMBER_MASK (0x0ful) ///< 分段个数掩码

#define CANTP_PCI_LENGTH_SF (1u) ///< 单帧长度
#define CANTP_PCI_LENGTH_FF (2u) ///< 首帧长度
#define CANTP_PCI_LENGTH_CF (1u) ///< 连续帧长度
#define CANTP_PCI_LENGTH_FC (3u) ///< 流控帧长度
#define CANTP_PCI_LENGTH_LSF (2u) ///< 最小单帧长度
#define CANTP_PCI_LENGTH_LFF (6u) ///< 最小首帧长度

#define CANTP_CAN20_FL (8u) ///< CAN 2.0 帧长度
#define CANTP_CANFD_FL (64u) ///<  can_fd 帧长度
#define CANTP_FF_MAXDL (4095u) ///< 首帧最大消息长度
#define CANTP_FF_MAXDL_EXT (65535u) ///< 地址拓展首帧最大消息长度

#define CANTP_CF_FSN (0x01u) ///< 首个连续帧序列号

#define CANTP_STMIN_MILLISEC_MAX (0x7fu) ///< 百万微秒最大
#define CANTP_STMIN_MICROSEC_MIN (0xf1u) ///< 微秒最小
#define CANTP_STMIN_MICROSEC_MAX (0xf9u) ///< 微秒最大

#define CANTP_TX_CHANNEL (0u) ///< 发送通道
#define CANTP_RX_CHANNEL (1u) ///< 接收通道
#define CANTP_NOF_CHANNELS (2u) ///< 非单独通道

#define CANTP_INVALID_PDU_ID ((PduIdType)0xffffu) ///< 无效pdu_id

#define CANTP_MAX_SF_DL_STDADDR (7u) ///< 普通寻址 CAN2.0 SF 最大sf_dl
#define CANTP_MAX_SF_DL_EXTENDADDR (6u) ///< 扩展寻址 CAN2.0 SF 最大sf_dl

#define CANTP_SDU_DATA_LENGTH (64u) /// cantp数据长度

typedef uint8 cantp_min_dl_type;
#define CANTP_MIN_DL_0 ((cantp_min_dl_type)0u)
#define CANTP_MIN_DL_8 ((cantp_min_dl_type)8u)
#define CANTP_MIN_DL_12 ((cantp_min_dl_type)12u)
#define CANTP_MIN_DL_16 ((cantp_min_dl_type)16u)
#define CANTP_MIN_DL_20 ((cantp_min_dl_type)20u)
#define CANTP_MIN_DL_24 ((cantp_min_dl_type)24u)
#define CANTP_MIN_DL_32 ((cantp_min_dl_type)32u)
#define CANTP_MIN_DL_48 ((cantp_min_dl_type)48u)
#define CANTP_MIN_DL_64 ((cantp_min_dl_type)64u)

/*!*********************************************************************************************************************
 * @brief   定义cantp的发送状态类型
 **********************************************************************************************************************/
typedef uint8 cantp_transfer_state_types;
#define CANTP_UNINITIALIZED ((uint8)0u) ///<  未初始化
#define CANTP_IDLE ((uint8)1u) ///<  空闲
#define CANTP_RX_WAIT_SF_FF_PDUR_BUFFER ((uint8)2u) ///<  等待单帧/首帧的接收缓冲区申请成功
#define CANTP_RX_WAIT_CF ((uint8)3u) ///<  等待连续帧
#define CANTP_TX_WAIT_STMIN ((uint8)4u) ///<  最小发送间隔等待期
#define CANTP_TX_WAIT_TRANSMIT ((uint8)5u) ///<  等待向下层模块发送
#define CANTP_TX_WAIT_FC ((uint8)6u) ///<  等待接收流控帧
#define CANTP_TX_WAIT_CONFIRMATION ((uint8)7u) ///<  等待下层模块发送确认
#define CANTP_TX_WAIT_COPY_TX_BUSY ((uint8)8u) ///<  tx复制上层数据busy时在主周期函数重试

/*!*********************************************************************************************************************
 * @brief   定义cantp中规定的帧类型
 **********************************************************************************************************************/
typedef enum {
	CANTP_INVALID_FRAME, ///< 无效帧
	CANTP_SINGLE_FRAME, ///< 单帧
	CANTP_FIRST_FRAME, ///< 首帧
	CANTP_CONSECUTIVE_FRAME, ///< 连续帧
	CANTP_FLOW_CONTROL_FRAME, ///< 流控帧
} cantp_frame_type;

/*!*********************************************************************************************************************
 * @brief   定义iso-15765-2 中规定的网络层中参数组
 **********************************************************************************************************************/
typedef struct {
	boolean nas_nar_pending; ///< 是否处于nas, nar计时状态
	uint8 extended_address; ///< 扩展地址
	uint8 stmin; ///< 多帧传输时连续帧的最小间隔时间 (only valid for TX)
	uint8 bs; ///< blocksize
	uint8 config_stmin; ///< ui配置的stmin
	uint8 config_bs; ///< ui配置的bs
	cantp_transfer_state_types state; ///< 通道状态机
	uint16 next_flow_control_count; ///< count down to next flow control
	uint16 frames_handled_count; ///< counter keeping track total frames handled
	uint16 state_timeout_count; ///< 超时计数器 counter for timeout
} cantp_transfer_control_type;

/*!*********************************************************************************************************************
 * @brief   定义连接通道中包含的子通道的管理结构体
 **********************************************************************************************************************/
typedef struct {
	boolean canfd_support; ///< 是否支持can_fd
	cantp_transfer_instance_mode mode; ///< 通道状态
	uint8 cantp_wft_max_counter; ///< FC.wait的上限
	uint8 initial_dlc; ///< ff的dlc长度 用于后续cf长度判断
	PduIdType sdu_id; ///< 报文id
	PduLengthType transfer_total; ///< PDU 总长度
	PduLengthType transfer_count; ///< 正在传输中的pdu长度(counter ongoing transfer)
	PduLengthType size_buffer; ///< rx中pdur剩余可用的buffer大小
	cantp_transfer_control_type iso15765; ///< 15765-2中规定的每个子通道所有计数器
} cantp_channel_type;

/*!*********************************************************************************************************************
 * @brief  通道接收方向 + 通道发送方向 + 功能帧
 **********************************************************************************************************************/
typedef struct {
	cantp_channel_type channel_list[CANTP_NOF_CHANNELS]; ///< 单通道列表
} cantp_channel_table_type;

/*!*********************************************************************************************************************
 * @brief 运行时参数 - 主要是不同连接通道的状态和计时器
 **********************************************************************************************************************/
typedef struct {
	cantp_state_type *state; ///< 当前cantp模块所处的状态 cantp_on或cantp_off
	cantp_channel_table_type *channel_table_addr; ///< cantp模块当前所有通道的数量
	uint32 channel_table_len;
} cantp_runtime_data_type;

/*!*********************************************************************************************************************
 * @brief 运行时数据
 **********************************************************************************************************************/
#define CANTP_START_SEC_CODE
#include "cantp_memmap.h"

FUNC_P2CONST(cantp_runtime_data_type, AUTOMATIC, CANTP_CODE) cantp_get_channel_core_table(void);
FUNC(uint8, CANTP_CODE) cantp_get_core_id(void);
FUNC_P2CONST(cantp_runtime_data_type, AUTOMATIC, CANTP_CODE) cantp_get_core_runtime_data(uint8 core_id);
FUNC(void, CANTP_CODE)
cantp_handle_send_next_tx_frame(const cantp_tx_nsdu_type *tx_config, cantp_channel_type *tx_runtime, uint8 core_id);
FUNC(void, CANTP_CODE)
cantp_handle_rx_frame(cantp_frame_type frame_type, PduIdType cantp_rx_nsdu_id, const PduInfoType *cantp_rx_pdu_ptr,
		      const cantp_rx_nsdu_type *rx_config, const cantp_tx_nsdu_type *tx_config,
		      cantp_channel_type *runtime_params, uint8 core_id);
FUNC(boolean, CANTP_CODE)
cantp_check_rx_ind_addr(cantp_frame_type frame_type, const PduInfoType *pdu_ptr,
			const cantp_rx_nsdu_type *rx_config, const cantp_tx_nsdu_type *tx_config);
FUNC(PduLengthType, CANTP_CODE)
cantp_get_padding_length(uint8 current_length, boolean padding_activation, boolean cantp_fd_support);
FUNC(cantp_frame_type, CANTP_CODE)
cantp_get_frame_type(const cantp_addr_format_type *format_type, const PduInfoType *cantp_rx_pdu_ptr);
FUNC(PduIdType, CANTP_CODE)
cantp_get_pdu_index_from_ipdu_list(PduIdType cantp_rx_pdu_id, const PduInfoType *cantp_rx_pdu_ptr, uint8 core_id);
FUNC(Std_ReturnType, CANTP_CODE)
cantp_check_rx_ind_valid(PduIdType cantp_rx_pdu_id, const PduInfoType *cantp_rx_pdu_ptr, uint8 core_id);
FUNC(Std_ReturnType, CANTP_CODE)
cantp_tx_frame(cantp_frame_type frame_type, const cantp_tx_nsdu_type *tx_config, cantp_channel_type *tx_runtime);
FUNC(cantp_frame_type, CANTP_CODE)
cantp_calc_tx_frame_type(const cantp_tx_nsdu_type *tx_config, PduLengthType SduLength);
FUNC(void, CANTP_CODE) cantp_tx_sdu_state_machine(const cantp_tx_nsdu_type *tx_cfg, cantp_channel_type *tx_runtime);
FUNC(void, CANTP_CODE) cantp_rx_sdu_state_machine(const cantp_rx_nsdu_type *rx_cfg, cantp_channel_type *rx_runtime);
FUNC(void, CANTP_CODE) cantp_init_params(uint8 core_id);
FUNC(void, CANTP_CODE) cantp_init_runtime_data(cantp_channel_type *rx_runtime_params, cantp_direction_type dir);
FUNC(void, CANTP_CODE) cantp_internal_main_function(void);

#define CANTP_STOP_SEC_CODE
#include "cantp_memmap.h"
#ifdef __cplusplus
}
#endif

#endif /* CANTP_INTERNAL_H */
