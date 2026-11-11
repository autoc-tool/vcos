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
#ifndef BSWM_BSW_H
#define BSWM_BSW_H

#if (CONFIG_MODULE_CANSM == 1)
#include "cansm.h"
#include "cansm_bswm.h"
#endif
#if (CONFIG_MODULE_COMM == 1)
#include "comm.h"
#endif
#if (CONFIG_MODULE_DCM == 1)
#include "dcm.h"
#endif
#if (CONFIG_SERVM_ECUM == 1)
#include "sysm_init.h"
#endif
#if (CONFIG_MODULE_ETHSM == 1)
#include "ethsm.h"
#endif
#if (CONFIG_MODULE_LINSM == 1)
#include "linsm.h"
#include "linif.h"
#endif
#if (CONFIG_MODULE_LINTP == 1)
#include "linif.h"
#endif
#if (CONFIG_MODULE_NM == 1)
#include "nm.h"
#endif
#if (CONFIG_MODULE_NVM == 1)
#include "nvm.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BSWM_START_SEC_CODE
#include "bswm_memmap.h"

#if (CONFIG_MODULE_CANSM == 1)
/*!*********************************************************************************************************************
 * @brief          CanSM向BswM指示当前模式
 * @details        该函数由CanSM调用，向BswM报告指定CAN网络的当前状态，从而使BswM能根据CAN网络的状态来调整策略。
 * @param[in]      network                 标识的CAN网络。
 * @param[in]      current_state            CanSM当前的状态。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00049
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_cansm_current_state(network_handle_type network, cansm_bswm_current_state_type current_state);
#endif

#if (CONFIG_MODULE_COMM == 1)
/*!*********************************************************************************************************************
 * @brief          ComM向BswM指示一个通道当前模式
 * @details        该函数由ComM调用，向BswM指示特定网络通道的当前通信模式，以支持BswM进行系统级的通信控制。
 * @param[in]      network         标识的网络通道。
 * @param[in]      requested_mode   网络通道请求的或者当前通信模式。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00047
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_comm_current_mode(network_handle_type network, comm_mode_type requested_mode);

/*!*********************************************************************************************************************
 * @brief          ComM向BswM指示PNC当前模式
 * @details        此函数由ComM调用来向BswM报告特定的部分网络通信集群(PNC)当前的模式，以便协调网络的休眠和唤醒策略。
 * @param[in]      PNC              部分网络通信集群(PNC)的标识。
 * @param[in]      current_pnc_mode   指示PNC的当前模式。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00148
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_comm_current_pnc_mode(PNCHandleType pnc, comm_pnc_mode_type current_pnc_mode);
#endif

#if (CONFIG_MODULE_DCM == 1)
/*!*********************************************************************************************************************
 * @brief          DCM调用此函数以报告更新的应用程序
 * @details        通过该函数，DCM报告应用程序更新成功，通知BswM进行相应的处理策略调整。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00158
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_dcm_application_updated(void);

/*!*********************************************************************************************************************
 * @brief          Dcm向BswM指示通信模式的当前状态
 * @details        该函数用于通知BswM Dcm当前的通信模式状态，使BswM能据此调整相关通信策略。
 * @param[in]      network        表示被诊断的网络通道。
 * @param[in]      requested_mode  表明要请求或当前设置的Dcm通信模式。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00048
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE)
bswm_dcm_communication_mode_current_state(network_handle_type network, dcm_comm_mode_type requested_mode);
#endif

#if (CONFIG_MODULE_ETHSM == 1)
/*!*********************************************************************************************************************
 * @brief          EthSM向BswM指示它当前的状态
 * @details        此函数用于通知BswM以太网状态管理(EthSM)的当前状态，使BswM可以据此执行相应的网络管理策略或行为。
 * @param[in]      network                 标识要通报状态的以太网通道。
 * @param[in]      current_state            EthSM的当前网络模式状态。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00050
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_ethsm_current_state(network_handle_type network, ethsm_network_mode_state_type current_state);
#endif

#if (CONFIG_MODULE_LINSM == 1)
/*!*********************************************************************************************************************
 * @brief          指示特定LIN频道的当前活动调度表
 * @details        该函数用于指示当前被激活的LIN调度表，允许BswM基于当前网络调度策略来进行管理。
 * @param[in]      network                 标识要通报的LIN网络。
 * @param[in]      current_schedule         被激活的调度表的引用。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00058
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_linsm_current_schedule(network_handle_type network, linif_sch_handle_type current_schedule);

/*!*********************************************************************************************************************
 * @brief          LinSM向BswM指示它的当前状态
 * @details        此函数用于通知BswM LIN状态管理(LinSM)的当前状态，以便BswM可以据此执行特定的网络管理策略或行为。
 * @param[in]      network                 标识要通报的LIN网络。
 * @param[in]      current_state            LIN状态管理器(LinSM)的当前状态。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00052
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_linsm_current_state(network_handle_type network, linsm_mode_type current_state);
#endif

#if (CONFIG_MODULE_LINTP == 1)
/*!*********************************************************************************************************************
 * @brief          LinTp向BswM请求相应LIN通道的模式
 * @details        该函数用于请求切换LinTp模式，以适应不同的LIN通信场景需求。
 *  @param[in]     network                  LIN网络的句柄，用于指定哪个LIN通道。
 *  @param[in]     lintp_requested_mode       请求切换到的LinTp模式。
 *  @reentrant     可重入
 *  @synchronous   同步
 *  @swsid         SWS_BswM_00156
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_lintp_request_mode(network_handle_type network, lintp_mode lintp_requested_mode);
#endif

#if (CONFIG_MODULE_NM == 1)
/*!*********************************************************************************************************************
 * @brief          Nm向BswM通知当前状态
 * @details        该函数用于将网络管理(Nm)的当前状态变化通知给基础软件模块管理器(BswM)。
 * @param[in]      network                 网络的句柄，用于指定哪个网络的Nm状态发生了变化。
 * @param[in]      current_state            表示Nm的当前状态。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00194
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_nm_state_change_notification(network_handle_type network, nm_state_type current_state);
#endif

#if (CONFIG_MODULE_NVM == 1)
/*!*********************************************************************************************************************
 * @brief          NvM 向 BswM 通知Block当前模式
 * @details        NvM在NvM块的请求处理完成后调用，用以通知BswM模块当前块的模式。BswM根据NvM块的当前状态执行相关动作。
 * @param[in]      Block                   NvM块的唯一标识符，指定哪个NvM块的状态发生了变化。
 * @param[in]      current_block_mode        指明NvM块当前的请求结果状态，如NVM_REQ_OK表示请求成功完成。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00104
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE) bswm_nvm_current_block_mode(nvm_block_id_t block, nvm_request_result_type current_block_mode);

/*!*********************************************************************************************************************
 * @brief          NVM 向 BswM 通知多块业务当前状态
 * @details        本函数由NvM在完成其多块请求作业后调用，用以通知BswM模块这些块的当前作业模式。
 * @param[in]      multi_block_request       表示NvM多块请求的类型，如多块读取或多块写入请求。
 * @param[in]      current_job_mode          表示多块请求作业的当前结果状态。
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_BswM_00152
 **********************************************************************************************************************/
FUNC(void, BSWM_CODE)
bswm_nvm_current_job_mode(nvm_multi_block_request_t multi_block_request, nvm_request_result_type current_job_mode);
#endif
#define BSWM_STOP_SEC_CODE
#include "bswm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* BSWM_BSW_H */