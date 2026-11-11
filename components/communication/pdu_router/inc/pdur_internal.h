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

#ifndef PDUR_INTERNAL_H
#define PDUR_INTERNAL_H

#include "pdur.h"
#include "pdur_cfg.h"
#include "pdur_buffermanager.h"
#include "pdur_types.h"
#if (PDUR_DEV_ERROR_DETECT == STD_ON) && (VCOS_MODULE_CONFIG_DET == 1)
#include "Det.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PDUR_DET_REPORT(errid)

#define PDU_MAX_ROUTING_PATH (4096u)
#define PDUR_PDU_ID_MASK (4095u)
/*!*********************************************************************************************************************
 * @brief    定义pdur路由状态类型
 **********************************************************************************************************************/
typedef uint8 pdur_route_status_type;
#define PDUR_E_OK ((pdur_route_status_type)0x00u)
#define PDUR_E_DISABLED ((pdur_route_status_type)0x01u)
#define PDUR_E_REJECTED ((pdur_route_status_type)0x02u)

#define PDUR_GATEWAY_ONTHEFLY 0x00u
#define PDUR_GATEWAY_DIRECT 0x01u

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"

/*!*********************************************************************************************************************
 * @brief          检查接收缓存是否足够
 * @details        在on-the-fly路机制中检查接收缓存是否足够
 * @param[in]      buffer_manager_ptr          缓存管理指针
 * @param[in]      info                      信息指针
 * @param[in]      buffer_size_ptr             缓存大小指针
 * @retval         BUFREQ_OK                 pdur拷贝数据成功
 * @retval         BUFREQ_E_NOT_OK           pdur拷贝数据失败
 * @retval         BUFREQ_E_BUSY             pdur拷贝数据忙碌
 * @retval         BUFREQ_E_OVFL             pdur拷贝数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_buffer_and_transfer_copy_data_into_ring_buffer(pdur_buffer_manager_type *buffer_manager_ptr,
						    const PduInfoType *info, PduLengthType *buffer_size_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          检查待发送数据是否足够本次复制长度
 * @details        在on-the-fly路机制中检查待发送数据是否足够本次复制长度
 * @param[in]      buffer_manager_ptr          缓存管理指针
 * @param[in]      send_len                   报文长度
 * @param[in]      dest_pdu_idx                当前目的在路由路径中的位置编号
 * @retval         TRUE                      检查待发送数据足够本次复制长度
 * @retval         FALSE                     检查待发送数据不足够本次复制长度
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE)
pdur_buffer_and_transfer_check_send_data_enough(const pdur_buffer_manager_type *buffer_manager_ptr,
						PduLengthType send_len, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          从环形队列存储空间中复制出发送的数据
 * @details        on-the-fly机制中从环形队列存储空间中复制出发送的数据
 * @param[in]      buffer_manager_ptr          缓存管理指针
 * @param[in]      info                      pdu信息
 * @param[in]      dest_pdu_idx                当前目的在路由路径中的位置编号
 * @param[in]      available_data_ptr          复制后仍存在等待复制的数据长度
 * @param[in]      retry                     下层是否要求retry
 * @retval         BUFREQ_OK                 复制出发送的数据成功
 * @retval         BUFREQ_E_NOT_OK           复制出发送的数据失败
 * @retval         BUFREQ_E_BUSY             复制出发送的数据忙碌
 * @retval         BUFREQ_E_OVFL             复制出发送的数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_buffer_and_transfer_copy_data_from_ring_buffer(pdur_buffer_manager_type *buffer_manager_ptr,
						    const PduInfoType *info, uint8 dest_pdu_idx,
						    PduLengthType *available_data_ptr, const RetryInfoType *retry);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur发送失败处理流程
 * @details        on-the-pdur发送失败处理流程
 * @param[in]      src_pdu_id                  src_pdu_id
 * @param[in]      dest_pdu_idx                当前正在处理的dest_pdu在routing_path中的位置
 * @retval         BUFREQ_OK                 pdur发送失败处理流程成功
 * @retval         BUFREQ_E_NOT_OK           pdur发送失败处理流程失败
 * @retval         BUFREQ_E_BUSY             pdur发送失败处理流程忙碌
 * @retval         BUFREQ_E_OVFL             pdur发送失败处理流程溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE) pdur_lo_tp_transmit_failed_process(PduIdType src_pdu_id, uint8 dest_pdu_idx);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur释放buffer，如果queue中有多个元素，则根据参数选择释放最前面的或者最后面的
 * @details        pdur释放buffer函数
 * @param[in]      pdu_id                     报文id
 * @param[in]      is_queue_first              当前
 * @retval         BUFREQ_OK                 pdur释放buffer成功
 * @retval         BUFREQ_E_NOT_OK           pdur释放buffer失败
 * @retval         BUFREQ_E_BUSY             pdur释放buffer忙碌
 * @retval         BUFREQ_E_OVFL             pdur释放buffer溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE) pdur_release_buffer(PduIdType pdu_id, boolean is_queue_first);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur释放buffer后，检查是否需要立刻触发下一个缓存发送
 * @details        pdur释放buffer函数
 * @param[in]      pdu_id                     报文id
 * @retval         E_OK                      pdur触发下一个buffer成功
 * @retval         E_NOT_OK                  pdur触发下一个buffer失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_release_buffer_check_transmit(PduIdType src_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由路径使能接口
 * @details        pdur路由路径使能接口
 * @param[in]      destination            目的指针
 * @retval         TRUE                   使能
 * @retval         FALSE                  不使能
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_check_routing_path_enabled(const pdur_dest_pdu_type *destination);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          检查路由路径的所有目的所属的路由路径组情况
 * @details        遍历路由路径的所有dest的routing_path_group状态
 * @param[in]      routing_path_ptr         路由路径
 * @retval         TRUE                   全部禁用
 * @retval         FALSE                  部分使能
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_check_all_dest_disabled(const pdur_routing_path_type *routing_path_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#if (PDUR_SUPPORT_ROUTING_PATH_GROUP == STD_ON)
#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由路径使能接口
 * @details        pdur路由路径使能接口
 * @param[in]      id                        路由路径组id
 * @param[in]      routing_group_status        路由路径组状态
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE)
pdur_set_routing_path_group_status(pdur_routing_path_group_id_type id, boolean routing_group_status);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由路径状态接口
 * @details        pdur路由路径状态接口
 * @param[in]      id                        路由路径组id
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(boolean, PDUR_CODE) pdur_get_routing_path_group_status(pdur_routing_path_group_id_type id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur模块获取模块状态的接口
 * @details        pdur模块获取模块状态的接口
 * @retval         PDUR_UNINIT    未初始化状态
 * @retval         PDUR_ONLINE    在线状态
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(pdur_state_type, PDUR_CODE) pdur_get_pdur_state(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur模块设定模块状态的接口
 * @details        pdur模块设定模块状态的接口
 * @param[in]      state            设定的状态
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_set_pdur_state(pdur_state_type state);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur模块获取pdur配置的接口
 * @details        pdur模块获取pdur配置的接口
 * @retval         pdur_pb_config_type 公共配置类型指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC_P2CONST(pdur_pb_config_type, AUTOMATIC, PDUR_CODE) pdur_get_pdur_config(void);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur模块设置pdur配置的接口
 * @details        pdur模块设置pdur配置的接口
 * @param[in]      config            配置项变量指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_set_pdur_config(const pdur_pb_config_type *config);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由发送
 * @details        pdur路由发送
 * @param[in]      destination               目的指针
 * @param[in]      pdu_info                   报文信息指针
 * @retval         E_OK                      没有错误
 * @retval         E_NOT_OK                  错误
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_route_transmit(const pdur_dest_pdu_type *destination, const PduInfoType *pdu_info);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由接收接口
 * @details        pdur路由接收接口
 * @param[in]      destination               目的指针
 * @param[in]      pdu_info                   报文信息指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_route_rx_indication(const pdur_dest_pdu_type *destination, const PduInfoType *pdu_info);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由发送确认接口
 * @details        pdur路由发送确认接口
 * @param[in]      route               路由指针
 * @param[in]      result              结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_route_tx_confirmation(const pdur_routing_path_type *route, NotifResultType result);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由触发发送接口
 * @details        pdur路由触发发送接口
 * @param[in]      route               路由指针
 * @param[in]      pdu_info             报文信息指针
 * @retval         E_OK                pdur路由触发发送接口成功
 * @retval         E_NOT_OK            pdur路由触发发送接口失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, PDUR_CODE) pdur_route_trigger_transmit(const pdur_routing_path_type *route, PduInfoType *pdu_info);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由拷贝rx数据接口
 * @details        pdur路由拷贝rx数据接口
 * @param[in]      destination         目的指针
 * @param[in]      info                信息指针
 * @param[in]      buffer_size_ptr       缓存大小指针
 * @retval         BUFREQ_OK           pdur下层tp模块拷贝接收数据成功
 * @retval         BUFREQ_E_NOT_OK     pdur下层tp模块拷贝接收数据失败
 * @retval         BUFREQ_E_BUSY       pdur下层tp模块拷贝接收数据忙碌
 * @retval         BUFREQ_E_OVFL       pdur下层tp模块拷贝接收数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_route_copy_rx_data(const pdur_dest_pdu_type *destination, const PduInfoType *info, PduLengthType *buffer_size_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由拷贝tx数据接口
 * @details        pdur路由拷贝tx数据接口
 * @param[in]      route               路由指针
 * @param[in]      info                信息指针
 * @param[in]      retry               重试指针
 * @param[in]      available_data_ptr    可用数据指针
 * @retval         BUFREQ_OK           pdur下层tp模块拷贝接收数据成功
 * @retval         BUFREQ_E_NOT_OK     pdur下层tp模块拷贝接收数据失败
 * @retval         BUFREQ_E_BUSY       pdur下层tp模块拷贝接收数据忙碌
 * @retval         BUFREQ_E_OVFL       pdur下层tp模块拷贝接收数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_route_copy_tx_data(const pdur_routing_path_type *route, const PduInfoType *info, const RetryInfoType *retry,
			PduLengthType *available_data_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由开始接收接口
 * @details        pdur路由开始接收接口
 * @param[in]      destination         目的指针
 * @param[in]      tp_sdu_length         tp的sdu长度
 * @param[in]      buffer_size_ptr       缓存区大小指针
 * @param[in]      info                pdu信息结构体指针
 * @retval         BUFREQ_OK           pdur下层tp模块拷贝接收数据成功
 * @retval         BUFREQ_E_NOT_OK     pdur下层tp模块拷贝接收数据失败
 * @retval         BUFREQ_E_BUSY       pdur下层tp模块拷贝接收数据忙碌
 * @retval         BUFREQ_E_OVFL       pdur下层tp模块拷贝接收数据溢出
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_route_start_of_reception(const pdur_dest_pdu_type *destination, const PduInfoType *info,
			      PduLengthType tp_sdu_length, PduLengthType *buffer_size_ptr);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由tp模块发送确认接口
 * @details        pdur路由tp模块发送确认接口
 * @param[in]      route               路由指针
 * @param[in]      result              结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_route_tp_tx_confirmation(const pdur_routing_path_type *route, NotifResultType result);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由tp接收接口
 * @details        pdur路由tp接收接口
 * @param[in]      destination               目的指针
 * @param[in]      result                    结果
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_route_tp_rx_indication(const pdur_dest_pdu_type *destination, NotifResultType result);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          pdur路由if消息长度检测
 * @details        根据长度检测策略分为忽略、截断、忽略和不启用等四种情况
 * @param[in]      destination               目的指针
 * @param[in]      pdu_info                   pdu信息
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(pdur_dest_pdu_len_strategy_type, PDUR_CODE)
pdur_check_and_handle_dest_pdu_len_strategy(const pdur_dest_pdu_type *destination, const PduInfoType *pdu_info);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          根据入参的dest_pdu_id获取其src_pdu_id
 * @details        根据入参的dest_pdu_id获取其src_pdu_id
 * @param[in]      dest_pdu_id               目的pdu的编号
 * @retval         对应的src_pdu_id
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(PduIdType, PDUR_CODE) pdur_get_src_pdu_id_by_dest_pdu_id(PduIdType dest_pdu_id);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
/*!*********************************************************************************************************************
 * @brief          在pdur_buffer_manager中的pending_dst_cnt变化时检查当前pending_dst_cnt是否已经和所有的下层模块dst_number相同
 * @details        在所有的dest都会到idle状态之后才能允许释放缓存
 * @param[in]      source_pdu_id               源pdu的编号
 * @param[in]      is_queue_first              当前检查队列首个还是队列末个元素
 * @retval         void
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, PDUR_CODE) pdur_check_pending_dst_cnt_reached(PduIdType source_pdu_id, boolean is_queue_first);
#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* PDUR_INTERNAL_H */
