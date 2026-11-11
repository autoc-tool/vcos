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

#ifndef PDUR_H
#define PDUR_H

#include <stddef.h>
#include "pdur_types.h"
#include "pdur_cfg.h"
#include "pdur_pbcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PDUR_MODULE_ID (51u)

#define PDUR_INVALID_CONFIGID (0xFFu)

#if (PDUR_ZERO_COST_OPERATION == STD_ON)
#define PduR_Init(...)
#define PduR_GetVersionInfo(...)
#define PduR_GetConfigurationId(...) 0
#define PduR_EnableRouting(...)
#define PduR_DisableRouting(...)
#else

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur初始化
 * @details        pdur初始化
 * @param[in]      config_ptr          配置指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_init(const pdur_pb_config_type *config_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur获取配置信息
 * @details        pdur获取配置信息
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(pdur_pb_config_id_type, PDUR_CODE) pdur_get_configuration_id(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur使能路由
 * @details        pdur使能路由
 * @param[in]      id 路由路径组id
 * @return         void
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_enable_routing(pdur_routing_path_group_id_type id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur去使能路由
 * @details        pdur去使能路由
 * @param[in]      id         路由路径组id
 * @param[in]      initialize 初始值
 * @return         void
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_disable_routing(pdur_routing_path_group_id_type id, boolean initialize);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur上层模块取消发送
 * @details        pdur上层模块取消发送
 * @param[in]      pdu_id          报文id
 * @param[in]      service_id      服务id
 * @retval         E_OK           pdur上层模块取消发送成功
 * @retval         E_NOT_OK       pdur上层模块取消发送失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_up_cancel_transmit(PduIdType pdu_id, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur上层模块改变参数
 * @details        pdur上层模块改变参数
 * @param[in]      pdu_id          报文id
 * @param[in]      parameter      要改变的参数
 * @param[in]      value          要改变的值
 * @param[in]      service_id      服务id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE)
pdur_up_change_parameter(PduIdType pdu_id, TPParameterType parameter, uint16 value, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur上层模块取消接收
 * @details        pdur上层模块取消接收
 * @param[in]      pdu_id          报文id
 * @param[in]      service_id      服务id
 * @retval         E_OK           pdur上层模块取消接收成功
 * @retval         E_NOT_OK       pdur上层模块取消接收失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_up_cancel_receive(PduIdType pdu_id, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur下层tp模块拷贝接收数据
 * @details        pdur下层tp模块拷贝接收数据
 * @param[in]      pdu_id               报文id
 * @param[in]      info                信息指针
 * @param[in]      buffer_size_ptr       缓存大小指针
 * @param[in]      service_id           服务id
 * @retval         BUFREQ_OK           pdur下层tp模块拷贝接收数据成功
 * @retval         BUFREQ_E_NOT_OK     pdur下层tp模块拷贝接收数据失败
 * @retval         BUFREQ_E_BUSY       pdur下层tp模块拷贝接收数据忙碌
 * @retval         BUFREQ_E_OVFL       pdur下层tp模块拷贝接收数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_lo_tp_copy_rx_data(PduIdType pdu_id, const PduInfoType *info, PduLengthType *buffer_size_ptr, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur下层tp模块接收指示
 * @details        pdur下层tp模块接收指示
 * @param[in]      pdu_id          报文id
 * @param[in]      result         通知结果
 * @param[in]      service_id      服务id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_lo_tp_rx_indication(PduIdType pdu_id, NotifResultType result, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur下层tp模块开始接收
 * @details        pdur下层tp模块开始接收
 * @param[in]      pdu_id               报文id
 * @param[in]      tp_sdu_length         tp报文长度
 * @param[in]      buffer_size_ptr       缓存大小指针
 * @param[in]      service_id           服务id
 * @retval         BUFREQ_OK           pdur下层tp模块拷贝接收数据成功
 * @retval         BUFREQ_E_NOT_OK     pdur下层tp模块拷贝接收数据失败
 * @retval         BUFREQ_E_BUSY       pdur下层tp模块拷贝接收数据忙碌
 * @retval         BUFREQ_E_OVFL       pdur下层tp模块拷贝接收数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_lo_tp_start_of_reception(PduIdType pdu_id, PduLengthType tp_sdu_length, PduLengthType *buffer_size_ptr,
			      uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur下层tp模块拷贝发送数据
 * @details        pdur下层tp模块拷贝发送数据
 * @param[in]      dest_pdu_id           报文id
 * @param[in]      info                信息指针
 * @param[in]      retry               重试指针
 * @param[in]      available_data_ptr    可用数据指针
 * @param[in]      service_id           服务id
 * @retval         BUFREQ_OK           pdur下层tp模块拷贝接收数据成功
 * @retval         BUFREQ_E_NOT_OK     pdur下层tp模块拷贝接收数据失败
 * @retval         BUFREQ_E_BUSY       pdur下层tp模块拷贝接收数据忙碌
 * @retval         BUFREQ_E_OVFL       pdur下层tp模块拷贝接收数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_lo_tp_copy_tx_data(PduIdType dest_pdu_id, const PduInfoType *info, RetryInfoType *retry,
			PduLengthType *available_data_ptr, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由tp发送确认接口
 * @details        pdur路由tp发送确认接口
 * @param[in]      pdu_id                     报文id
 * @param[in]      result                    结果
 * @param[in]      service_id                 服务id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_tp_tx_confirmation(PduIdType dest_pdu_id, NotifResultType result, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur发送接口
 * @details        pdur发送接口
 * @param[in]      pdu_id                     报文id
 * @param[in]      pdu_info                   报文信息
 * @param[in]      service_id                 服务id
 * @retval         E_OK                      pdur发送成功
 * @retval         E_NOT_OK                  pdur发送失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_transmit(PduIdType pdu_id, const PduInfoType *pdu_info, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur接收接口
 * @details        pdur接收接口
 * @param[in]      pdu_id                     报文id
 * @param[in]      pdu_info                   报文信息
 * @param[in]      service_id                 服务id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_rx_indication(PduIdType pdu_id, const PduInfoType *pdu_info, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur发送确认接口
 * @details        pdur发送确认接口
 * @param[in]      pdu_id                    报文id
 * @param[in]      result                    结果
 * @param[in]      service_id                服务id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_tx_confirmation(PduIdType pdu_id, uint8 result, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur触发发送接口
 * @details        pdur触发发送接口
 * @param[in]      pdu_id                     报文id
 * @param[in]      pdu_info                   pdu_info的头指针
 * @param[in]      service_id                 服务id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_trigger_transmit(PduIdType pdu_id, PduInfoType *pdu_info, uint8 service_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur主函数
 * @details        pdur主函数
 * @param[in]      void
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_internal_main_function(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#if (PDUR_MULTICORE_SUPPORT == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur中断处理函数
 * @details        pdur中断处理函数
 * @param[in]      void
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_internal_interrupt_service_routine(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
