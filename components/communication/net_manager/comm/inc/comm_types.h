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

#ifndef COMM_TYPES_H
#define COMM_TYPES_H

#if defined(VCOS_MODULE_CONFIG_RTE) && (VCOS_MODULE_CONFIG_RTE == 1)
#include "Rte_Type.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef COMM_NO_COMMUNICATION
#define COMM_NO_COMMUNICATION (0U)
#endif

#ifndef COMM_SILENT_COMMUNICATION
#define COMM_SILENT_COMMUNICATION (1U)
#endif

#ifndef COMM_FULL_COMMUNICATION
#define COMM_FULL_COMMUNICATION (2U)
#endif

#define COMM_INH_STATUS_NONE (0u)
#define COMM_INH_STATUS_WAKE_UP (1u)
#define COMM_INH_STATUS_NO_COMMUNICATION (uint8)(1u << 1)

typedef uint8 comm_init_status_type;
#define COMM_UNINIT ((comm_init_status_type)0u)
#define COMM_INIT ((comm_init_status_type)1u)

typedef uint8 comm_state_type;
#define COMM_STATE_NO_COM_NO_REQ_PENDING ((comm_state_type)0u)
#define COMM_STATE_NO_COM_REQ_PENDING ((comm_state_type)1u)
#define COMM_STATE_FULL_COM_NET_REQ ((comm_state_type)2u)
#define COMM_STATE_FULL_COM_READY_SLEEP ((comm_state_type)3u)
#define COMM_STATE_SILENT_COM ((comm_state_type)4u)

typedef uint8 comm_bus_type;
#define COMM_BUS_TYPE_CAN ((comm_bus_type)0u)
#define COMM_BUS_TYPE_LIN ((comm_bus_type)1u)
#define COMM_BUS_TYPE_ETH ((comm_bus_type)2u)

typedef uint8 comm_nm_variant_type;
#define COMM_NM_VARIANT_NONE ((comm_nm_variant_type)0u)
#define COMM_NM_VARIANT_PASSIVE ((comm_nm_variant_type)1u)
#define COMM_NM_VARIANT_FULL ((comm_nm_variant_type)2u)

typedef uint8 comm_pnc_mode_type;
#define COMM_PNC_REQUESTED ((comm_pnc_mode_type)0u)
#define COMM_PNC_READY_SLEEP ((comm_pnc_mode_type)1u)
#define COMM_PNC_PREPARE_SLEEP ((comm_pnc_mode_type)2u)
#define COMM_PNC_NO_COMMUNICATION ((comm_pnc_mode_type)3u)

typedef struct {
	const comm_bus_type bus_type;
	const comm_nm_variant_type nm_variant;
	const NetworkHandleType comm_channel;
	const NetworkHandleType nm_channel;
	const uint32 main_func_period;
} comm_channel_config_type;

typedef struct {
	const uint8 channel_num;

	const comm_channel_config_type *const *channels;
} comm_user_config_type;

typedef struct {
	const comm_channel_config_type *channels;
	const comm_user_config_type *users;
} comm_config_type;

typedef uint8 comm_user_handle_type;

typedef uint8 comm_mode_type;

#ifdef __cplusplus
}
#endif

#endif
