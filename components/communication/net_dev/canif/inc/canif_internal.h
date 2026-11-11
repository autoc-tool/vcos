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

#ifndef CANIF_INTERNAL_H
#define CANIF_INTERNAL_H

#include "canif_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXTENDED_CANID_MAX 0x1FFFFFFFU
#define STANDARD_CANID_MAX 0x7FFU
#define EXT_ID_BIT_POS 31U
#define CAN_FD_BIT_POS 30U
#define EXT_ID_STD_ID_START_BIT 18U
#define CANIF_INVALID_CANID 0xFFFFFFFFU

#define CANIF_ECUM_WKSOURCE_NONE ((ecum_wakeup_source_t)0U)

#define CANIF_IS_EXTENDED_CAN_ID(_x) (0UL != ((_x) & (1UL << EXT_ID_BIT_POS)))
#define CANIF_IS_CANFD(_x) (0 != ((_x) & (1UL << CAN_FD_BIT_POS)))

#define CANIF_DET_REPORT(errid)

#define CANIF_DET_REPORTERR(_cond, errid)                                                                              \
	do {                                                                                                           \
		if (!(_cond)) {                                                                                        \
			CANIF_DET_REPORT(errid);                                                                       \
			return;                                                                                        \
		}                                                                                                      \
	} while (0)

#define CANIF_DET_REPORTERR_NO_RV(_cond, errid, ...)                                                                   \
	do {                                                                                                           \
		if (!(_cond)) {                                                                                        \
			CANIF_DET_REPORT(errid);                                                                       \
			return __VA_ARGS__;                                                                            \
		}                                                                                                      \
	} while (0)

#define CANIF_DET_REPORTERR_NO_RV_INITSTATUS_WITH_COREID(_coreid, _errid)                                              \
	CANIF_DET_REPORTERR((CANIF_INIT == canif_init_status[_coreid]), _errid)

#define CANIF_DET_REPORTERR_INITSTATUS_WITH_COREID(_coreid, _errid, _ret)                                              \
	CANIF_DET_REPORTERR_NO_RV((CANIF_INIT == canif_init_status[_coreid]), _errid, _ret)

#define CANIF_CHECK_IS_VALID_CONTROLLER_MODE(_x)                                                                       \
	((CANIF_CS_STARTED == (_x)) || (CANIF_CS_SLEEP == (_x)) || (CANIF_CS_STOPPED == (_x)))

#define CANIF_CHECK_IS_VALID_CONTROLLER_STATE(_x)                                                                      \
	((CAN_CS_STARTED == (_x)) || (CAN_CS_SLEEP == (_x)) || (CAN_CS_STOPPED == (_x)))

#define CANIF_GET_CHANNEL_CONTROLLER(_channel) canif_config_ptr->canif_ctrl_config[_channel].canif_ctrl_can_ctrl_id

#define CANIF_GET_CAN_TRANSCEIVERID(_id) canif_config_ptr->canif_trcv_config[_id].canif_cantrcv_id

#define CANIF_GET_CAN_TRANSCEIVERFCTGRPID(_id) canif_config_ptr->canif_trcv_config[_id].canif_cantrcv_fct_grp_id

#define CANIF_GET_CAN_TRANSCEIVERCHNLREFID(_id) canif_config_ptr->canif_trcv_config[_id].canif_cantrcv_id_ref

#define cantrcv_check_wake_flag(index) canif_config_ptr->cantrcv_drv_api[index].check_wake_flag_fct

#define cantrcv_clear_trcv_wuf_flag(index) canif_config_ptr->cantrcv_drv_api[index].clear_trcv_wuf_flag_fct

#define cantrcv_check_wakeup(index) canif_config_ptr->cantrcv_drv_api[index].check_wakeup_fct

#define cantrcv_set_op_mode(index) canif_config_ptr->cantrcv_drv_api[index].set_op_mode_fct

#define cantrcv_get_op_mode(index) canif_config_ptr->cantrcv_drv_api[index].get_op_mode_fct

#define cantrcv_get_bus_wu_reason(index) canif_config_ptr->cantrcv_drv_api[index].get_bus_wu_reason_fct

#define cantrcv_set_wakeup_mode(index) canif_config_ptr->cantrcv_drv_api[index].set_wakeup_mode_fct

typedef struct {
	uint8 dlc;
	PduIdType pdu_id;
	Can_IdType can_id;
#if (CANIF_CANFD_SUPPORT == STD_ON)
	uint8 data[64];
#else
	uint8 data[8];
#endif
} canif_l_pdu_type;

#if (CANIF_PUBLIC_TX_BUFFERING == STD_ON)
typedef struct {
	boolean in_use;
	canif_l_pdu_type l_pdu;
} canif_buffer_entry_type;

#if (CANIF_TX_BUF_FIFO_ENABLE == STD_ON)
typedef struct {
	uint16 u16_write_index;
	uint16 u16_read_index;
	uint16 u16_queue_size;
} canif_fifo_buffer_info_type;
#endif
#endif

typedef uint32 canif_frame_receive_status_type;
#define CANIF_FRAME_RECEIVED_NONE ((canif_frame_receive_status_type)0U)
#define CANIF_FRAME_RECEIVED_FIRST ((canif_frame_receive_status_type)1U)

typedef uint32 canif_nm_msg_cnt_type;

typedef struct {
#if (CANIF_PUBLIC_PN_SUPPORT == STD_ON)
	boolean pn_tx_filter_enabled;
#endif
#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
	canif_frame_receive_status_type first_rx_indication;
#endif
	canif_controller_mode_type ctrl_mode;
	canif_pdu_mode_type pdu_mode;
	canif_nm_msg_cnt_type nm_cnt;
} canif_ctrl_runtime_state_type;

#if (CANIF_TRANSCEIVER_API == STD_ON)
typedef struct {
	CanTrcv_TrcvModeType trcv_mode;
} canif_trcv_runtime_state_type;
#endif

typedef struct {
	canif_ctrl_runtime_state_type ctrl_runtime_state[CANIF_CONTROLLER_CHANNEL_CNT];
#if (CANIF_TRANSCEIVER_API == STD_ON)
	canif_trcv_runtime_state_type trcv_runtime_state[CANIF_TRANSCEIVER_CHANNEL_CNT];
#endif
} canif_runtime_state_type;

#define CANIF_START_SEC_CODE
#include "canif_memmap.h"

#if (CANIF_PUBLIC_APPL_CANIF_TX_CALLOUT_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          由用户实现的 tx callout 函数原型声明
 *  @details        由用户实现的 tx callout 函数原型声明
 *  @param[in]      controller   can通道id
 *  @param[in]      data_ptr      pdu信息
 *  @retval         E_OK:   成功
 *  @retval         E_NOT_OK: 失败
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
appl_canif_tx_callout(const uint8 controller, const Can_PduType *data_ptr);
#endif

#if (CANIF_PUBLIC_APPL_CANIF_TXCONFIRMATION_CALLOUT_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          由用户实现的 tx_confirmation callout 函数原型声明
 *  @details        由用户实现的 tx_confirmation callout 函数原型声明
 *  @param[in]      controller   can通道id
 *  @param[in]      data_ptr      pdu信息
 *  @retval         CAN_OK:   成功
 *  @retval         CAN_NOT_OK: 失败
 **********************************************************************************************************************/
FUNC(Can_ReturnType, CANIF_CODE)
appl_canif_tx_confirmation_callout(const uint8 controller, const Can_PduType *data_ptr);
#endif

#if (CANIF_PUBLIC_APPL_CANIF_RXINDICATION_CALLOUT_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          由用户实现的 rx_indication callout 函数原型声明
 *  @details        由用户实现的 rx_indication callout 函数原型声明
 *  @param[in]      controller   can通道id
 *  @param[in]      ID           can_id
 *  @param[in]      data_length   报文数据长度
 *  @param[in]      data_ptr      报文payload
 *  @retval         CAN_OK:   成功
 *  @retval         CAN_NOT_OK: 失败
 **********************************************************************************************************************/
FUNC(Can_ReturnType, CANIF_CODE)
appl_canif_rx_indication_callout(const uint8 controller, const Can_IdType ID, const uint8 data_length,
				 const uint8 *data_ptr);
#endif

#define CANIF_STOP_SEC_CODE
#include "canif_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANIF_INTERNAL_H */
