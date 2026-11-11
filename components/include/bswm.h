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
#ifndef BSWM_H
#define BSWM_H
#include "bswm_types.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BSWM_VENDOR_ID (60u) ///< 制造商ID
#define BSWM_MODULE_ID (42u) ///< 模块ID

#define BSWM_AR_RELEASE_MAJOR_VERSION (4u) ///< Autosar主版本号
#define BSWM_AR_RELEASE_MINOR_VERSION (0u) ///< Autosar次版本号
#define BSWM_AR_RELEASE_PATCH_VERSION (3u) ///< Autosar修订版本号

#define BSWM_AR_MAJOR_VERSION (BSWM_AR_RELEASE_MAJOR_VERSION) ///< Autosar主版本号
#define BSWM_AR_MINOR_VERSION (BSWM_AR_RELEASE_MINOR_VERSION) ///< Autosar次版本号
#define BSWM_AR_PATCH_VERSION (BSWM_AR_RELEASE_PATCH_VERSION) ///< Autosar修订版本号

#define BSWM_SW_MAJOR_VERSION (4u) ///< 软件主版本号
#define BSWM_SW_MINOR_VERSION (0u) ///< 软件次版本号
#define BSWM_SW_PATCH_VERSION (1u) ///< 软件修订版本号

#define BSWM_E_NO_INIT (0x01u) ///<  DET模块未初始化
#define BSWM_E_NULL_POINTER (0x02u) ///<  空指针
#define BSWM_E_PARAM_INVALID (0x03u) ///<  参数非法
#define BSWM_E_REQ_USER_OUT_OF_RANGE (0x04u) ///<  请求用户越界
#define BSWM_E_REQ_MODE_OUT_OF_RANGE (0x05u) ///<  请求模式越界
#define BSWM_E_PARAM_CONFIG (0x06u) ///<  参数配置
#define BSWM_E_PARAM_POINTER (0x07u) ///<  参数指针

#define BSWM_SERVICEID_INIT (0x00u) ///< (Service ID)  初始化
#define BSWM_SERVICEID_GETVERSIONINFO (0x01u) ///< (Service ID)  获取版本信息
#define BSWM_SERVICEID_REQUESTMODE (0x02u) ///< (Service ID)  请求模式
#define BSWM_SERVICEID_MAINFUNCTION (0x03u) ///< (Service ID)  主函数
#define BSWM_SERVICEID_DEINIT (0x04u) ///< (Service ID)  反初始化
#define BSWM_SERVICEID_CANSMCURRENTSTATE (0x05u) ///< (Service ID)  CanSM 当前状态
#define BSWM_SERVICEID_DCMCOMMUNICATIONMODE (0x06u) ///< (Service ID)  DCM 通信模式
#define BSWM_SERVICEID_NMCURRENTMODE (0x07u) ///< (Service ID)  NM 当前模式
#define BSWM_SERVICEID_LINSMCURRENTSTATE (0x09u) ///< (Service ID)  LinSM 当前状态
#define BSWM_SERVICEID_LINSMCURRENTSCHEDULE (0x0Au) ///< (Service ID)  LinSM 当前调度表
#define BSWM_SERVICEID_LINTPREQUESTMODE (0x0Bu) ///< (Service ID)  LinTP 请求模式
#define BSWM_SERVICEID_FRSMCURRENTSTATE (0x0Cu) ///< (Service ID)  FrSM 当前状态
#define BSWM_SERVICEID_ETHSMCURRENTSTATE (0x0Du) ///< (Service ID)  EthSM 当前状态
#define BSWM_SERVICEID_COMMCURRENTMODE (0x0Eu) ///< (Service ID)  ComM 当前模式
#define BSWM_SERVICEID_ECUMCURRENTSTATE (0x0Fu) ///< (Service ID)  EcuM 当前状态
#define BSWM_SERVICEID_ECUMCURRENTWAKEUP (0x10u) ///< (Service ID)  EcuM 当前唤醒
#define BSWM_SERVICEID_ECUMREQUEST (0x11u) ///< (Service ID)  EcuM 请求
#define BSWM_SERVICEID_COMMCURRENTPNCMODE (0x15u) ///< (Service ID)  ComM 当前PNC模式
#define BSWM_SERVICEID_SDCLIENTSERVICECURRENTSTATE (0x1Fu) ///< (Service ID)  SD Client服务当前状态
#define BSWM_SERVICEID_SDEVENTHANDLERCURRENTSTATE (0x20u) ///< (Service ID)  SD 事件处理程序当前状态
#define BSWM_SERVICEID_SDCONSUMEDEVENTGROUPCURRENTSTATE (0x21u) ///< (Service ID)  SD 消费的事件组当前状态
#define BSWM_SERVICEID_IMMEDIATEREQUEST (0x30u) ///< (Service ID)  立即请求

#define BSWM_UNDEFINED_REQUEST_VALUE (0xFF) ///< 未定义请求值

#define BSWM_START_SEC_CODE
#include "bswm_memmap.h"
/*!*********************************************************************************************************************
 * @brief          初始化 BswM
 * @details        将BswM 模块初始化为默认值
 * @param[in]      config_ptr 配置指针
 * @reentrant      是否可重入需要根据是否多核区分
 * @synchronous    同步
 * @swsid          SWS_BswM_00002
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_init(const bswm_config_type *config_ptr);

/*!*********************************************************************************************************************
 * @brief          反初始化 BswM
 * @details        将BswM 模块反初始化为默认值
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00119
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_deinit(void);

/*!*********************************************************************************************************************
 * @brief          获取BswM的初始化状态
 * @details        获取BswM的初始化状态
 * @param[in]      bswm_status               BswM的状态
 * @reentrant      重入
 * @synchronous    同步
 * @swsid          AUTOSAR文档中没有
 **********************************************************************************************************************/
FUNC(Std_ReturnType, BSWM_CODE) bswm_get_status(bswm_init_status_t *bswm_status);

/*!*********************************************************************************************************************
 * @brief          BswM 请求模式
 * @details        请求模式的通用函数调用。该函数只能由没有特定模式请求接口的其他 BSW 模块使用。
 * @param[in]      requesting_user 请求用户
 * @param[in]      requested_mode 请求的模式
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00046
 **********************************************************************************************************************/
#if (BSWM_GENERICREQUEST_ENABLED == STD_ON)
FUNC(void, BSWM_CODE) bswm_request_mode(bswm_user_t requesting_user, bswm_mode_t requested_mode);
#endif

/*!*********************************************************************************************************************
 * @brief          BswM版本信息
 * @details        获取BswM版本信息
 * @param[in]      version_info   版本信息
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00003
 * ********************************************************************************************************************/
#if (BSWM_VERSION_INFO_API == STD_ON)
FUNC(void, BSWM_CODE) bswm_get_version_info(Std_VersionInfoType *version_info);
#endif

#define BSWM_STOP_SEC_CODE
#include "bswm_memmap.h"
/*!*********************************************************************************************************************
 * @brief          BswM配置项
 **********************************************************************************************************************/
extern const bswm_config_type bswm_config;

#ifdef __cplusplus
}
#endif

#endif /* BSWM_H */