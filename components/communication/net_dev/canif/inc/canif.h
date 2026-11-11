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

#ifndef CANIF_H
#define CANIF_H

#include "canif_cbk.h"
#include "canif_lcfg.h"
#include "canif_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CANIF_MODULE_ID (60u)

#define CANIF_START_SEC_CODE
#include "canif_memmap.h"

/*!*********************************************************************************************************************
 *  @brief          初始化canif模块
 *  @details        该服务初始化can接口的内部和外部接口以供进一步处理。
 *  @param[in]      config_ptr   指向配置参数集的指针，例如使用为了后期构建参数
 *  @reentrant      支持重入
 *  @synchronous    同步
 *  @swsid          SWS_CANIF_00001
 **********************************************************************************************************************/
FUNC(void, CANIF_CODE) canif_init(const canif_config_type *config_ptr);

#if !(defined(AUTOSAR_CANIF_VERSION_421)) || (defined(AUTOSAR_CANIF_VERSION_403))

/*!*********************************************************************************************************************
 *  @brief          更改控制器模式
 *  @details        该服务调用相应的can driver服务来更改can控制器模式。
 *  @param[in]      controller_id     分配给can控制器的抽象canif
 *controller_id,请求用于模式转换。
 *  @param[in]      controller_mode   请求的模式转换
 *  @retval         E_OK            控制器模式请求已被接受
 *  @retval         E_NOT_OK        控制器模式请求未被接受
 *  @reentrant      不支持重入
 *  @synchronous    同步
 *  @swsid          SWS_CANIF_00003
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_controller_mode(uint8 controller_id, Can_ControllerStateType controller_mode);
#else

/*!*********************************************************************************************************************
 *  @brief          更改控制器模式
 *  @details        该服务调用相应的can driver服务来更改can控制器模式。
 *  @param[in]      controller_id     分配给can控制器的抽象canif
 *controller_id,请求用于模式转换。
 *  @param[in]      controller_mode   请求的模式转换
 *  @retval         E_OK            控制器模式请求已被接受
 *  @retval         E_NOT_OK        控制器模式请求未被接受
 *  @reentrant      不支持重入
 *  @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_controller_mode(uint8 controller_id, canif_controller_mode_type controller_mode);
#endif

/*!*********************************************************************************************************************
 *  @brief          获取控制器模式
 *  @details        该服务调用对应的can driver服务来获取can控制器的当前状态
 *  @param[in]      controller_id        抽象的canif
 *controller_id，分配给can控制器，当前操作模式需要此控制器。
 *  @param[out]     controller_mode_ptr   指向存储位置的指针，CAN
 *控制器的当前模式将存储在该位置。
 *  @retval         E_OK            控制器模式请求已被接受
 *  @retval         E_NOT_OK        控制器模式请求未被接受
 *  @reentrant      相同控制器不可重入，不同控制器可重入
 *  @synchronous    同步
 *  @swsid          SWS_CANIF_00229
 **********************************************************************************************************************/

FUNC(Std_ReturnType, CANIF_CODE)
canif_get_controller_mode(uint8 controller_id, canif_controller_mode_type *controller_mode_ptr);

/*!*********************************************************************************************************************
 *  @brief          发送pdu
 *  @details        请求pdu发送
 *  @param[in]      can_tx_pdu_id 要传输的pdu的id
 *  @param[in]      pdu_info_ptr                                    PDU
 *数据的长度和指针以及指向元数据的指针。
 *  @retval         CANIF_TRANSMIT_OK 发送请求已被can驱动接受 or
 *发送请求未被can驱动接受，但成功入队canif tx_buffer，报文未丢弃
 *  @retval         CANIF_TRANSMIT_NOT_OK 发送请求未被can驱动接受，报文丢弃
 *  @retval         CANIF_TRANSMIT_BUSY_TXBUFFER_NOT_INSERTED
 *发送请求未被can驱动接受，并且canif tx_buffer入队没成功，报文丢弃
 *  @retval         CANIF_TRANSMIT_BUSY_NO_TXBUFFER_CONFIG
 *发送请求未被can驱动接受，并且没有配置canif tx_buffer，报文丢弃
 *  @reentrant      支持重入不同的pdu_ids。不支持相同的pdu_id重入。
 *  @synchronous     同步
 *  @swsid           SWS_CANIF_00005
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_transmit(PduIdType can_tx_pdu_id, const PduInfoType *pdu_info_ptr);

/*!*********************************************************************************************************************
 *  @brief          设置pdu模式
 *  @details        该服务在预定义的逻辑pdu通道的l-pdu处设置请求的模式。
 *  @param[in]      channel_id     连接到对应的canif
 *controller_id的自己的ecu的所有pdu被分配给物理can控制器。
 *  @param[in]      pdu_mode_request   请求pdu模式转换
 *  @retval         E_OK            模式转换请求已被接受
 *  @retval         E_NOT_OK        模式转换请求未被接受
 *  @reentrant      相同控制器不可重入，不同控制器可重入
 *  @synchronous    同步
 *  @swsid           SWS_CANIF_00008
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_pdu_mode(uint8 channel_id, canif_pdu_mode_type pdu_mode_request);

/*!*********************************************************************************************************************
 *  @brief          获取pdu模式
 *  @details        该服务在预定义的逻辑pdu通道的l-pdu处设置请求的模式。
 *  @param[in]      channel_id     连接到对应的canif
 *controller_id的自己的ecu的所有pdu被分配给物理can控制器。
 *  @param[out]     pdu_mode_ptr       指向内存位置的指针，将存储逻辑 PDU
 *通道的当前模式。
 *  @retval         E_OK            pdu模式转换请求已被接受
 *  @retval         E_NOT_OK        pdu模式转换请求未被接受
 *  @reentrant      相同控制器不可重入，不同控制器可重入
 *  @synchronous    同步
 *  @swsid           SWS_CANIF_00009
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_get_pdu_mode(uint8 channel_id, canif_pdu_mode_type *pdu_mode_ptr);

#if (CANIF_TRANSCEIVER_API == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          设置收发器模式
 *  @details        该服务通过调用相应的can 收发器驱动服务来改变tansceiver
 *transceiver_id的操作模式。
 *  @param[in]      transceiver_id     抽象的 canif transceiver_id，分配给一个
 *can收发器，请求用于模式转换
 *  @param[in]      transceiver_mode   请求模式转换
 *  @retval         E_OK            收发器模式请求已被接受
 *  @retval         E_NOT_OK        收发器模式请求未被接受
 *  @reentrant      不可重入
 *  @synchronous    异步
 *  @swsid           SWS_CANIF_00287
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_trcv_mode(uint8 transceiver_id, CanTrcv_TrcvModeType transceiver_mode);

/*!*********************************************************************************************************************
 *  @brief      获取收发器模式
 *  @details
 *此函数调用cantrcv_get_op_mode并使用cantrcv提供的值op_mode更新参数transceiver_mode_ptr。
 *  @param[in]      transceiver_id            抽象的 canif
 *transceiver_id，分配给 CAN 收发器，为当前操作模式请求。。
 *  @param[out]     transceiver_mode_ptr 收发器连接到的请求网络的请求模式。
 *  @retval         E_OK            收发器模式请求已被接受
 *  @retval         E_NOT_OK        收发器模式请求未被接受
 *  @reentrant      不可重入
 *  @synchronous    同步
 *  @swsid           SWS_CANIF_00288
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_get_trcv_mode(CanTrcv_TrcvModeType *transceiver_mode_ptr, uint8 transceiver_id);

/*!*********************************************************************************************************************
 *  @brief      获取收发器唤醒原因
 *  @details
 *该服务通过调用相应的can收发器驱动服务返回唤醒收发器transceiver_id的原因。
 *  @param[in]      transceiver_id            抽象的 canif
 *transceiver_id，分配给 CAN 收发器，出于唤醒原因请求。
 *  @param[out]     trcv_wu_reason_ptr
 *提供指向所请求的收发器唤醒原因应返回的位置的指针
 *  @retval         E_OK                     收发器唤醒原因请求已被接受。
 *  @retval         E_NOT_OK                 收发器唤醒原因请求未被接受。
 *  @reentrant      不支持重入
 *  @synchronous    同步
 *  @swsid           SWS_CANIF_00289
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_get_trcv_wakeup_reason(uint8 transceiver_id, CanTrcv_TrcvWakeupReasonType *trcv_wu_reason_ptr);

/*!*********************************************************************************************************************
 *  @brief          设置收发唤醒模式
 *  @details        该函数应调用cantrcv_set_trcv_wakeup_mode。
 *  @param[in]      transceiver_id     抽象的 canif transceiver_id，分配给一个
 *CAN 收发器，请求用于模式转换
 *  @param[in]      trcv_wakeup_mode    请求模式转换
 *  @retval         E_OK 如果唤醒通知状态已更改为请求模式，将返回e_ok。
 *  @retval         E_NOT_OK
 *如果唤醒通知状态更改失败或参数超出允许范围，将返回e_not_ok。
 *  @reentrant      不支持重入
 *  @synchronous    同步
 *  @swsid          SWS_CANIF_290
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_set_trcv_wakeup_mode(uint8 transceiver_id, CanTrcv_TrcvWakeupModeType trcv_wakeup_mode);
#endif

#if ((CANIF_CTRL_WAKEUP_SUPPORT == STD_ON) || (CANIF_TRCV_WAKEUP_SUPPORT == STD_ON)) && (CANIF_WAKEUP_FUN_ENABLE == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          检测唤醒源
 *  @details
 *此服务检查底层can驱动程序或can收发器驱动程序是否已发出唤醒事件信号。
 *  @param[in]      wakeup_source    发起唤醒事件的源设备：CAN 控制器或
 *can收发器
 *  @retval         E_OK           如果检测唤醒请求已被接收，将返回e_ok
 *  @retval         E_NOT_OK       如果检测唤醒请求未被接收，将返回e_not_ok
 *  @reentrant      不支持重入
 *  @synchronous    异步
 *  @swsid          SWS_CANIF_219
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_check_wakeup(ecum_wakeup_source_t wakeup_source);

#if (CANIF_PUBLIC_WAKEUP_CHECK_VALIDATION_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          验证被动唤醒
 *  @details        执行此服务以验证先前的唤醒事件。
 *  @param[in]      wakeup_source
 *启动唤醒事件且必须验证的源设备：can控制器或can收发器
 *  @retval         E_OK           如果检测验证请求已被接收，将返回e_ok
 *  @retval         E_NOT_OK       如果检测验证请求未被接收，将返回e_not_ok
 *  @reentrant      不支持重入
 *  @synchronous    同步
 *  @swsid          SWS_CANIF_178
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_check_validation(ecum_wakeup_source_t wakeup_source);
#endif
#endif

#if (CANIF_PUBLIC_TXCONFIRM_POLLING_SUPPORT == STD_ON)
/*!*********************************************************************************************************************
 *  @brief          获取发送状态
 *  @details
 *此服务报告自上次can控制器启动以来是否已为整个can控制器完成任何tx确认。
 *  @param[in]      controller_id   分配给can控制器的抽象canif controller_id
 *  @retval         canif_notif_status_type can控制器的所有tx
 *pdu的组合tx确认状态
 *  @reentrant      相同控制器不可重入，不同控制器可重入
 *  @synchronous    同步
 *  @swsid          SWS_CANIF_734
 **********************************************************************************************************************/
FUNC(canif_notif_status_type, CANIF_CODE)
canif_get_tx_confirmation_state(uint8 controller_id);
#endif

#if ((CANIF_PUBLIC_PN_SUPPORT == STD_ON) && (CANIF_TRCV_PN_ENABLE == STD_ON))
/*!*********************************************************************************************************************
 *  @brief          清除收发唤醒标志
 *  @details        请求canif模块清除指定can收发器的wuf标志。
 *  @param[in]      transceiver_id   抽象canif
 *transceiver_id，分配给指定的can收发器。
 *  @retval         E_OK           请求已被接受
 *  @retval         E_NOT_OK       请求未被接受
 *  @reentrant      不可重入
 *  @synchronous    异步
 *  @swsid          SWS_CANIF_760
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_clear_trcv_wuf_flag(uint8 transceiver_id);

/*!*********************************************************************************************************************
 *  @brief         清除校验唤醒源
 *  @details        请求canif模块检查指定can收发器的唤醒标志。
 *  @param[in]      transceiver_id   抽象canif
 *transceiver_id，分配给指定的can收发器。
 *  @retval         E_OK           请求已被接受
 *  @retval         E_NOT_OK       请求未被接受
 *  @reentrant      不可重入
 *  @synchronous    异步
 *  @swsid          SWS_CANIF_761
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANIF_CODE)
canif_check_trcv_wake_flag(uint8 transceiver_id);
#endif

#define CANIF_STOP_SEC_CODE
#include "canif_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANIF_H */
