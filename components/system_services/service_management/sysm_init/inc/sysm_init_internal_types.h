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

#ifndef ECUM_INTERNAL_TYPES_H
#define ECUM_INTERNAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
/*!*********************************************************************************************************************
 * @brief          Including File
 **********************************************************************************************************************/
#include "sysm_init_types.h"
#if (ECUM_DEV_ERROR_DETECT == STD_ON)
#include "det.h"
#endif

#if (CONFIG_MODULE_WDGM == 1)
#include "wdgm.h"
#endif

#if (CONFIG_MODULE_COMM == 1)
#include "comm.h"
#endif

/*!*********************************************************************************************************************
 * @brief  唤醒源配置的定义
 **********************************************************************************************************************/
typedef struct {
	ecum_wakeup_source_t ecum_wakeup_source_id; ///<  唤醒源
	uint32 ecum_validation_timeout; ///<  唤醒源验证超时
#if (CONFIG_MODULE_COMM == 1)
	uint8 ecum_comm_channel; ///<  EcuM 和 ComM 交互的通道
#endif
} ecum_wakeup_source_config_t;

/*!*********************************************************************************************************************
 * @brief  休眠模式的定义
 **********************************************************************************************************************/
typedef struct {
	uint8 ecum_sleep_mode_id; ///<  休眠模式
	ecum_wakeup_source_t ecum_wakeup_source_mask; ///<  休眠模式对应的唤醒源
	uint32_t ecum_sleep_mode_mcu_mode; ///<  休眠模式对应的 MCU 模式
} ecum_sleep_mode_t;

#if (CONFIG_MODULE_COMM == 1)
/*!*********************************************************************************************************************
 * @brief  EcuM 和 ComM 交互配置
 **********************************************************************************************************************/
typedef struct {
	uint8 ecum_comm_network_handle; ///<  网络句柄
	comm_bus_type ecum_com_bus_t; ///<  总线类型
} ecum_comm_config_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* ECUM_INTERNAL_TYPES_H */