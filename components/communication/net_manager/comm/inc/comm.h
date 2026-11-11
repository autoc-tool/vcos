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
#ifndef COMM_H
#define COMM_H

#include "comstack_types.h"
#include "comm_types.h"
#include "comm_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_MODULE_ID (12u)

#define COMM_E_UNINIT (0x01u)
#define COMM_E_WRONG_PARAMETERS (0x02u)
#define COMM_E_PARAM_POINTER (0x03u)
#define COMM_E_INIT_FAILED (0x04u)

#define COMM_SERVICEID_INIT (0x01u)
#define COMM_SERVICEID_DEINIT (0x02u)
#define COMM_SERVICEID_GETSTATUS (0x03u)
#define COMM_SERVICEID_GETINHIBITIONSTATUS (0x04u)
#define COMM_SERVICEID_REQUESTCOMMODE (0x05u)
#define COMM_SERVICEID_GETMAXCOMMODE (0x06u)
#define COMM_SERVICEID_GETREQUESTEDCOMMODE (0x07u)
#define COMM_SERVICEID_GETCURRENTCOMMODE (0x08u)
#define COMM_SERVICEID_PREVENTWAKEUP (0x09u)
#define COMM_SERVICEID_LIMITCHANNELTONOCOMMODE (0x0bu)
#define COMM_SERVICEID_LIMITECUTONOCOMMODE (0x0cu)
#define COMM_SERVICEID_READINHIBITCOUNTER (0x0du)
#define COMM_SERVICEID_RESETINHIBITCOUNTER (0x0eu)
#define COMM_SERVICEID_SETECUGROUPCLASSIFICATION (0x0fu)
#define COMM_SERVICEID_GETVERSIONINFO (0x10u)
#define COMM_SERVICEID_NM_NETWORKSTARTINDICATION (0x15u)
#define COMM_SERVICEID_NM_NETWORKMODE (0x18u)
#define COMM_SERVICEID_NM_PREPAREBUSSLEEPMODE (0x19u)
#define COMM_SERVICEID_NM_BUSSLEEPMODE (0x1au)
#define COMM_SERVICEID_NM_RESTARTINDICATION (0x1bu)
#define COMM_SERVICEID_DCM_ACTIVEDIAGNOSTIC (0x1fu)
#define COMM_SERVICEID_DCM_INACTIVEDIAGNOSTIC (0x20u)
#define COMM_SERVICEID_ECUM_RUNMODEINDICATION (0x29u)
#define COMM_SERVICEID_ECUM_WAKEUPINDICATION (0x2au)
#define COMM_SERVICEID_ECUM_PNCWAKEUPINDICATION (0x25u)
#define COMM_SERVICEID_BUSSM_MODEINDICATION (0x33u)
#define COMM_SERVICEID_MAINFUNCTION (0x60u)
#define COMM_SERVICEID_GETSTATE (0x34u)
#define COMM_SERVICEID_COMMUNICATIONALLOWED (0x35u)

#define COMM_E_MODE_LIMITATION (0x02u)

#define COMM_START_SEC_CODE
#include "comm_memmap.h"
/*!********************************************************************************************************************
 * @brief          comm 初始化
 * @details        根据传入的配置参数初始化comm模块
 * @param[in]      config 配置项指针
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_init(const comm_config_type *config);

/*!********************************************************************************************************************
 * @brief          comm 反初始化
 * @details        释放comm模块所使用的资源，并将模块状态回复到初始状态
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_deinit(void);

/*!********************************************************************************************************************
 * @brief          通信方式指示
 * @details        通知通信管理器（comm）有关网络频道的当前通信模式
 * @param[in]      channel 用于标识网络频道的网络句柄。
 * @param[in]      com_mode 指向当前通信模式的指针。
 * @reentrant      可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_bus_sm_mode_indication(NetworkHandleType channel, const comm_mode_type *com_mode);

/*!********************************************************************************************************************
 * @brief          获取comm子状态
 * @details        查询指定网络频道的当前通信管理器（comm）状态
 * @param[in]      channel 用于标识网络频道的网络句柄
 * @param[out]     state 指向用于接收当前comm状态的变量的指针
 * @retval         E_OK 表示成功获取状态
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_get_state(NetworkHandleType channel, comm_state_type *state);

/*!*********************************************************************************************************************
 * @brief          获取comm初始化状态
 * @details        返回通信管理器（comm）的初始化状态
 * @param[out]     status 指向用于接收当前comm初始化状态的变量的指针。
 * @retval         e_ok表示成功获取状态
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_get_status(comm_init_status_type *status);

/*!******************************************************************************************************************
 * @brief          用户请求通信模式
 * @details        由用户请求一个特定的通信模式
 * @param[in]      user 用户句柄，用于标识发出通信模式请求的用户
 * @param[in]      com_mode 请求的通信模式，如comm_mode_type所定义的通信模式
 * @retval         E_OK                         成功切换到新模式
 * @retval         E_NOT_OK                     切换到新模式失败
 * @retval         COMM_E_MODE_LIMITATION       由于模式禁止，无法授予模式
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, COMM_CODE) comm_request_com_mode(comm_user_handle_type user, comm_mode_type com_mode);

/*!******************************************************************************************************************
 * @brief          设置通信允许状态
 * @details        当允许通信时，ecum 或 bswm 应向 comm 指示
 * @param[in]      channel 网络通道的句柄，指定将受到控制的通信通道。
 * @param[in]      allowed 布尔值，表示通信是否被允许。true表示允许通道上的通信，false表示禁止通道上的通信。
 * @reentrant      不可重入
 * @synchronous    异步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_communication_allowed(NetworkHandleType channel, boolean allowed);

/*!******************************************************************************************************************
 * @brief          comm 主函数
 * @details        comm 主函数
 * @param[in]      channel 网络通道的句柄，指定将受到控制的通信通道。
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(void, COMM_CODE) comm_main_function(NetworkHandleType channel);

#define COMM_STOP_SEC_CODE
#include "comm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /*COMM_H*/
