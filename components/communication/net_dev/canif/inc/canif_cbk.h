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

#ifndef CANIF_CBK_H
#define CANIF_CBK_H

#include "canif_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CANIF_START_SEC_CODE
#include "canif_memmap.h"

/*!*********************************************************************************************************************
 * @brief          canif发送确认接口
 * @details        该服务确认先前成功处理的can tx_pdu传输
 * @param[in]      can_tx_pdu_id 发送pdu的id，此id指定相应的can L-PDU
 *ID，并指定can驱动程序实例以及相应的can控制器设备。
 * @reentrant      相同pdu_id不可重入，不同pdu_id可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_00007
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE) canif_tx_confirmation(PduIdType can_tx_pdu_id);

#if defined(AUTOSAR_CANIF_VERSION_403)
/*!*********************************************************************************************************************
 * @brief          接收回调函数
 * @details        canif接收回调函数
 * @param[in]      hrh      接收表
 * @param[in]      can_id    标识
 * @param[in]      can_dlc   长度
 * @param[in]      can_sdu_ptr  sdu指针
 * @reentrant      相同pdu_id不可重入，不同pdu_id可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_00006
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE)
canif_rx_indication(Can_HwHandleType hrh, Can_IdType can_id, uint8 can_dlc, const uint8 *can_sdu_ptr);
#else
/*!*********************************************************************************************************************
 * @brief          接收回调函数
 * @details 此服务指示在通过所有过滤器和验证检查后成功接收到canif的接收到的can
 *rx LPDU
 * @param[in]      mailbox      识别hrh及其对应的can控制器
 * @param[in]      pdu_info_ptr    指向接收到的 L-PDU 的指针
 * @reentrant      相同pdu_id不可重入，不同pdu_id可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_00006
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE)
canif_rx_indication(const Can_HwType *mailbox, const PduInfoType *pdu_info_ptr);
#endif

/*!*********************************************************************************************************************
 * @brief          控制busoff
 * @details        此服务指示一个控制器bus_off事件，该事件引用具有抽象canif
 *controller_id的相应can控制器
 * @param[in]      controller_id
 *分配给can控制器的抽象canif控制器，其中发生总线关闭
 * @reentrant      相同控制器不可重入，不同控制器可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_00218
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE) canif_controller_bus_off(uint8 controller_id);

#if ((CANIF_PUBLIC_PN_SUPPORT == STD_ON) && (CANIF_TRCV_PN_ENABLE== STD_ON))
/*!*********************************************************************************************************************
 * @brief          pn确认可用
 * @details        该服务指示收发器在pn通信模式下运行，引用具有抽象id的相应can收发器。
 * @reentrant      对于不同的收发器可重入，相同收发器不可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_00815
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE) canif_confirm_pn_availability(uint8 transceiver_id);
/*!*********************************************************************************************************************
 * @brief          清除收发唤醒标志位
 * @details
 *该服务表明收发器已经清除了引用相应can收发器的wuf_flag，该can收发器具有抽象的canif
 *transceiver_id。
 * @reentrant      对于不同的收发器可重入，相同收发器不可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_00762
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE)
canif_clear_trcv_wuf_flag_indication(uint8 transceiver_id);

/*!*********************************************************************************************************************
 * @brief          清除收发唤醒标志位
 * @details
 *该服务表明收发器唤醒标志的检查已由相应的can收发器完成，带有抽象的id。
 *该指示用于处理异步收发器通信。
 * @param[in]      transceiver_id   抽象canif
 *transceiver_id，分配给指定的can收发器，检查pn可用性。
 * @reentrant      对于不同的收发器可重入，相同收发器不可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_00763
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE)
canif_check_trcv_wake_flag_indication(uint8 transceiver_id);
#endif

#if (CANIF_TRANSCEIVER_API == STD_ON)
/*!*********************************************************************************************************************
 * @brief          收发器模式指示
 * @details        该服务指示一个收发器状态转换，它引用具有抽象canif
 *transceiver_id的相应can收发器。
 * @param[in]      transceiver_id   抽象canif
 *transceiver_id，分配给指定的can收发器，检查pn可用性。
 * @param[in]      transceiver_mode 收发器模式
 * @reentrant      对于不同的收发器可重入，相同收发器不可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_764
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE)
canif_trcv_mode_indication(uint8 transceiver_id, CanTrcv_TrcvModeType transceiver_mode);
#endif

#if (defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))
/*!*********************************************************************************************************************
 * @brief          控制器模式指示
 * @details        该服务指示一个控制器状态转换，它引用具有抽象canif
 *controller_id的相应can控制器。
 * @param[in]      controller_id   抽象 canif
 *controller_id，分配给一个can控制器，其状态已转换。
 * @param[in]      controller_mode can控制器转换到的模式
 * @reentrant      对于不同的控制器可重入，相同控制器不可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_699
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE)
canif_controller_mode_indication(uint8 controller_id, canif_controller_mode_type controller_mode);
#else
/*!*********************************************************************************************************************
 * @brief          控制器模式指示
 * @details        该服务指示一个控制器状态转换，它引用具有抽象canif
 *controller_id的相应can控制器。
 * @param[in]      controller_id   抽象 canif
 *controller_id，分配给一个can控制器，其状态已转换。
 * @param[in]      controller_mode can控制器转换到的模式
 * @reentrant      对于不同的控制器可重入，相同控制器不可重入
 * @synchronous    同步
 * @swsid          SWS_CANIF_699
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE)
canif_controller_mode_indication(uint8 controller_id, Can_ControllerStateType controller_mode);
#endif

#define CANIF_STOP_SEC_CODE
#include "canif_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANIF_CBK_H */
