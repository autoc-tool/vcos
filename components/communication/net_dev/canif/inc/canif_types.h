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

#ifndef CANIF_TYPES_H
#define CANIF_TYPES_H

#include "can_generaltypes.h"
#include "canif_cfg.h"
#include "comstack_types.h"

#include "Os.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_SERVM_ECUM) && (CONFIG_SERVM_ECUM == 1)
#include "sysm_init_cfg.h"
#include "sysm_init_types.h"
#else
#ifndef ecum_wakeup_source_t
typedef uint32 ecum_wakeup_source_t;
#endif
#ifndef ECUM_WKSOURCE_USER_CNT
#define ECUM_WKSOURCE_USER_CNT 10U
#endif
#ifndef ECUM_WKSOURCE_SYSTEM_CNT
#define ECUM_WKSOURCE_SYSTEM_CNT (6U)
#endif
#endif

#define CANIF_CAN_WRITE_BUSY (2U) ///< 写忙碌错误

/*!*********************************************************************************************************************
 * @brief   分核init标志位状态类型
 **********************************************************************************************************************/
typedef uint32 canif_init_status_type;
#define CANIF_UNINIT ((canif_init_status_type)0u)
#define CANIF_INIT ((canif_init_status_type)1u)

/*!*********************************************************************************************************************
 * @brief   核id类型
 **********************************************************************************************************************/
typedef uint8 canif_core_id_type;

/*!*********************************************************************************************************************
 * @brief   canif_transmit返回值类型
 **********************************************************************************************************************/
#define CANIF_TRANSMIT_OK ((Std_ReturnType)0u)
#define CANIF_TRANSMIT_NOT_OK ((Std_ReturnType)1u)
#define CANIF_TRANSMIT_BUSY_TXBUFFER_NOT_INSERTED ((Std_ReturnType)2u)
#define CANIF_TRANSMIT_BUSY_NO_TXBUFFER_CONFIG ((Std_ReturnType)3u)

/*!*********************************************************************************************************************
 * @brief   控制器模式类型
 **********************************************************************************************************************/
typedef uint32 canif_controller_mode_type;
#define CANIF_CS_UNINIT ((canif_controller_mode_type)0U)
#define CANIF_CS_STOPPED ((canif_controller_mode_type)1U)
#define CANIF_CS_STARTED ((canif_controller_mode_type)2U)
#define CANIF_CS_SLEEP ((canif_controller_mode_type)3U)

/*!*********************************************************************************************************************
 * @brief   定义PDU通道组状态类型。
 **********************************************************************************************************************/
typedef uint32 canif_pdu_mode_type;
#define CANIF_OFFLINE ((canif_pdu_mode_type)0U)
#define CANIF_TX_OFFLINE ((canif_pdu_mode_type)1U)
#define CANIF_TX_OFFLINE_ACTIVE ((canif_pdu_mode_type)2U)
#define CANIF_ONLINE ((canif_pdu_mode_type)3U)

/*!*********************************************************************************************************************
 * @brief   定义通知状态类型。
 **********************************************************************************************************************/
typedef uint32 canif_notif_status_type;
#define CANIF_NO_NOTIFICATION ((canif_notif_status_type)0U)
#define CANIF_TX_RX_NOTIFICATION ((canif_notif_status_type)1U)

/*!*********************************************************************************************************************
 * @brief   定义是否可以在运行时更改 PDU can id。
 **********************************************************************************************************************/
typedef uint8 canif_pdu_type;
#define CANIF_PDU_TYPE_STATIC ((canif_pdu_type)0u)
#define CANIF_PDU_TYPE_DYNAMIC ((canif_pdu_type)1u)

/*!*********************************************************************************************************************
 * @brief   定义pdu can id 类型。
 **********************************************************************************************************************/
typedef uint8 canif_can_id_type;
#define CANIF_CAN_ID_TYPE_29 ((canif_can_id_type)0u)
#define CANIF_CAN_FD_ID_TYPE_29 ((canif_can_id_type)1u)
#define CANIF_CAN_ID_TYPE_11 ((canif_can_id_type)2u)
#define CANIF_CAN_FD_ID_TYPE_11 ((canif_can_id_type)3u)

/*!*********************************************************************************************************************
 * @brief   定义 canif PDU 是指基本的还是完整的 CAN 硬件对象
 **********************************************************************************************************************/
typedef uint8 canif_hoh_type;
#define CANIF_HANDLE_TYPE_BASIC ((canif_hoh_type)0u)
#define CANIF_HANDLE_TYPE_FULL ((canif_hoh_type)1u)

/*!*********************************************************************************************************************
 * @brief   定义用于配置 canid 范围的参数
 **********************************************************************************************************************/
typedef struct {
	uint32 rx_pdu_lower_can_id;
	uint32 rx_pdu_upper_can_id;
} canif_hrh_range_config_type;

/*!*********************************************************************************************************************
 * @brief   接收回调函数类型
 **********************************************************************************************************************/
typedef uint8 canif_rx_indication_type;
#define CANIF_RX_INDICATION_TYPE_ADVANCED_TYPE ((canif_rx_indication_type)0u)
#define CANIF_RX_INDICATION_TYPE_CDD_TYPE ((canif_rx_indication_type)1u)

/*!*********************************************************************************************************************
 * @brief   定义指示函数类型
 **********************************************************************************************************************/
typedef void (*canif_advanced_rx_indication_fct_type)(PduIdType can_rx_pdu_id, const PduInfoType *pdu_info_ptr);

/*!*********************************************************************************************************************
 * @brief   定义cdd 类型
 **********************************************************************************************************************/
typedef void (*canif_cdd_rx_indication_fct_type)(PduIdType can_rx_pdu_id, const PduInfoType *pdu_info_ptr);

/*!*********************************************************************************************************************
 * @brief   定义canif接收类型
 **********************************************************************************************************************/
typedef union {
	canif_advanced_rx_indication_fct_type advanced_rx_indication_type;
	canif_cdd_rx_indication_fct_type cdd_rx_indication_type;
} canif_rx_indication_fct_type;

/*!*********************************************************************************************************************
 * @brief   定义canif_rx_indication_fct_list中使用的类型
 **********************************************************************************************************************/
typedef struct {
	canif_rx_indication_fct_type rx_indication_fct;
	canif_rx_indication_type rx_indication_type;
} canif_user_rx_indication_fct_list_type;

/*!*********************************************************************************************************************
 * @brief   定义接收回调函数类型
 **********************************************************************************************************************/
typedef Std_ReturnType (*canif_rx_callout_fct_type)(PduIdType can_rx_pdu_id, const PduInfoType *pdu_info_ptr);

typedef uint8 canif_pdu_dlc_type;

/*!*********************************************************************************************************************
 * @brief   定义接收pdu类型
 **********************************************************************************************************************/
typedef struct {
	boolean rx_pdu_canid_extended;
	uint8 rx_pdu_ul_rx_indication;
	canif_pdu_dlc_type rx_pdu_dlc;
	PduIdType rx_pdu_pdu_id;
	uint16 rx_pdu_index;
	canif_rx_callout_fct_type rx_pdu_rx_callout_fct_ptr;
	uint32 rx_pdu_lower_can_id;
	uint32 rx_pdu_upper_can_id;
	uint32 rx_pdu_can_id;
} canif_rx_pdu_config_type;

/*!*********************************************************************************************************************
 * @brief   此容器包含每个硬件接收对象 (HRH) 的配置参数
 **********************************************************************************************************************/
typedef struct {
	canif_hoh_type hrh_type;
	boolean hrh_sw_filter;
	canif_channel_id_type hrh_can_ctrl_id_ref;
	uint16 hrh_ctrl_rx_pdu_num;
	const canif_hrh_range_config_type *hrh_range_config;
	const canif_rx_pdu_config_type *hrh_rx_pdu_list;
} canif_hrh_config_type;

/*!*********************************************************************************************************************
 * @brief   此容器包含与每个hth相关的参数
 **********************************************************************************************************************/
typedef struct {
	canif_hoh_type hth_type;
	canif_channel_id_type hth_can_ctrl_id_ref;
	uint8 hth_can_drv_api_idx;
	Can_HwHandleType hth_id_sym_ref;
} canif_hth_config_type;

/*!*********************************************************************************************************************
 * @brief   此容器包含对每个底层 CAN 驱动程序的配置设置的引用
 **********************************************************************************************************************/
typedef struct {
	const canif_hrh_config_type *canif_hrh_config;
	const canif_hth_config_type *canif_hth_config;
	const uint16 *can_hoh_to_canif_hrh_map;
	uint32 hrh_list_size;
} canif_hoh_config_type;

/*!*********************************************************************************************************************
 * @brief   tx 缓冲区的定义
 **********************************************************************************************************************/
typedef struct {
	const canif_hth_config_type *tx_buffer_hth_ref;
	uint16 tx_buffer_size;
	uint16 tx_buffer_id;
} canif_tx_buffer_config_type;

typedef void (*canif_user_tx_confirmation_type)(PduIdType tx_pdu_id, Std_ReturnType result);

typedef Std_ReturnType (*canif_txmessage_callout)(PduIdType tx_pduid, Can_PduType *ptrcan_pdu);

#define CANIF_NO_FUNCTION_CALLOUT 0xFFU

typedef struct {
	boolean tx_pdu_pn_filter_en;
	canif_pdu_dlc_type tx_pdu_dlc;
	uint8 tx_pdu_ul_tx_confirmation;
	canif_can_id_type tx_pdu_can_id_type;
	canif_pdu_type tx_pdu_type;
	PduIdType tx_pdu_id;
	const canif_tx_buffer_config_type *tx_pdu_buffer_ref;
	uint32 tx_pdu_can_id;
	canif_txmessage_callout tx_pdu_txmsg_callout_fct_ptr;
} canif_tx_pdu_config_type;

typedef void (*canif_bus_off_notification_func_type)(uint8 controller_id);

#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
typedef void (*canif_wakeup_valid_notification_func_type)(ecum_wakeup_source_t wakeup_source);
#endif

#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
typedef void (*canif_controller_mode_indication_func_type)(uint8 controller_id,
							   canif_controller_mode_type controller_mode);
#else
typedef void (*canif_controller_mode_indication_func_type)(uint8 controller_id,
							   Can_ControllerStateType controller_mode);
#endif

typedef void (*canif_trcv_check_wake_flag_indication_func_type)(uint8 transceiver_id);

typedef void (*canif_trcv_clear_wake_flag_indication_func_type)(uint8 transceiver_id);

typedef void (*canif_trcv_confirm_pn_availability_func_type)(uint8 transceiver_id);

typedef void (*canif_trcv_mode_indication_func_type)(uint8 transceiver_id, CanTrcv_TrcvModeType transceiver_mode);

typedef struct {
	canif_bus_off_notification_func_type bus_off_notification_fct;
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
	canif_wakeup_valid_notification_func_type wakeup_valid_notification_fct;
#endif
	canif_controller_mode_indication_func_type controller_mode_indication_fct;
	canif_trcv_check_wake_flag_indication_func_type trcv_check_wake_flag_indication_fct;
	canif_trcv_clear_wake_flag_indication_func_type trcv_clear_wake_flag_indication_fct;
	canif_trcv_confirm_pn_availability_func_type trcv_confirm_pn_availability_fct;
	canif_trcv_mode_indication_func_type trcv_mode_indication_fct;
} canif_dispatch_config_type;

typedef struct {
	uint32 canif_all_tx_pdu_num;
	const canif_tx_buffer_config_type *canif_all_tx_buf_cfg_ptr;
	const canif_hoh_config_type *canif_all_hoh_cfg_ptr;
	const canif_rx_pdu_config_type *canif_all_rx_pdu_cfg_ptr;
	const canif_tx_pdu_config_type *canif_all_tx_pdu_cfg_ptr;
} canif_init_config_type;

typedef struct {
	uint8 canif_ctrl_can_ctrl_id;
	boolean canif_ctrl_pn_filter_set;
	uint16 canif_ctrl_tx_buf_num;
	const canif_tx_buffer_config_type *const *canif_ctrl_tx_buf_ref_list;
} canif_controller_config_type;

#if (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON)
typedef struct {
	boolean canif_cantrcv_wu_sup;
	canif_trcv_id_type canif_cantrcv_id_ref;
	canif_trcv_id_type canif_cantrcv_id;
	canif_channel_id_type canif_cantrcv_to_canif_ctrl_chnl_ref;
	uint8 canif_cantrcv_fct_grp_id;
	canif_core_id_type canif_cantrcv_core_id;
	ecum_wakeup_source_t canif_cantrcv_wu_src;
} canif_transceiver_config_type;
#endif

typedef struct {
	uint16 canif_tx_buffer_start_index_of_cur_core;
	uint16 canif_tx_buffer_end_index_of_cur_core;
} canif_tx_buffer_index_range_type;

typedef Std_ReturnType (*cantrcv_clear_trcv_wuf_flag_func_type)(uint8 transceiver);

typedef Std_ReturnType (*cantrcv_check_wake_flag_func_type)(uint8 transceiver);

typedef Std_ReturnType (*cantrcv_check_wakeup_func_type)(uint8 transceiver);

typedef Std_ReturnType (*cantrcv_set_op_mode_func_type)(uint8 transceiver, CanTrcv_TrcvModeType op_mode);

typedef Std_ReturnType (*cantrcv_set_wakeup_mode_func_type)(uint8 transceiver,
							    CanTrcv_TrcvWakeupModeType trcv_wakeup_mode);

typedef Std_ReturnType (*cantrcv_get_bus_wu_reason_func_type)(uint8 transceiver, CanTrcv_TrcvWakeupReasonType *reason);

typedef Std_ReturnType (*cantrcv_get_op_mode_func_type)(uint8 transceiver, CanTrcv_TrcvModeType *op_mode);

typedef struct {
	cantrcv_check_wake_flag_func_type check_wake_flag_fct;
	cantrcv_clear_trcv_wuf_flag_func_type clear_trcv_wuf_flag_fct;
	cantrcv_check_wakeup_func_type check_wakeup_fct;
	cantrcv_get_bus_wu_reason_func_type get_bus_wu_reason_fct;
	cantrcv_get_op_mode_func_type get_op_mode_fct;
	cantrcv_set_op_mode_func_type set_op_mode_fct;
	cantrcv_set_wakeup_mode_func_type set_wakeup_mode_fct;
} canif_cantrcv_fct_tbl_type;

typedef struct {
	const canif_init_config_type *canif_init_config;
#if (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON)
	const canif_transceiver_config_type *canif_trcv_config;
	const canif_cantrcv_fct_tbl_type *cantrcv_drv_api;
#endif
	const canif_controller_config_type *canif_ctrl_config;
} canif_config_type;

#ifdef __cplusplus
}
#endif

#endif /* CANIF_TYPES_H */
