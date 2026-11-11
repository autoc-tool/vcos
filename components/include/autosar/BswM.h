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

#ifndef AUTOSAROS_INC_BSWM_H
#define AUTOSAROS_INC_BSWM_H

#include "bswm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BswM_ConfigType
typedef bswm_config_type BswM_ConfigType;
#endif

#ifndef BswM_Init
#define BswM_Init                   bswm_init
#endif

#ifndef BswM_Deinit
#define BswM_Deinit                 bswm_deinit
#endif

#ifndef BswM_GetInitStatus
#define BswM_GetInitStatus          bswm_get_init_status
#endif

#if (BSWM_GENERICREQUEST_ENABLED == STD_ON)
#ifndef BswM_RequestMode
#define BswM_RequestMode            bswm_request_mode
#endif
#endif

#if (BSWM_VERSION_INFO_API == STD_ON)
#ifndef BswM_GetVersionInfo
#define BswM_GetVersionInfo         bswm_get_version_info
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif