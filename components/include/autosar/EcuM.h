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

#ifndef AUTOSAROS_INC_ECUM_H
#define AUTOSAROS_INC_ECUM_H

#include "sysm_init.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EcuM_GetVersionInfo
#define EcuM_GetVersionInfo         ecum_get_version_info
#endif
#ifndef EcuM_Init
#define EcuM_Init                   platform_init
#endif
#ifndef EcuM_StartupTwo
#define EcuM_StartupTwo             system_init
#endif
#ifndef EcuM_Shutdown
#define EcuM_Shutdown               ecum_shutdown
#endif
#ifndef EcuM_SetState
#define EcuM_SetState               ecum_set_state
#endif
#ifndef EcuM_GetState
#define EcuM_GetState               ecum_get_state
#endif
#ifndef EcuM_RequestRUN
#define EcuM_RequestRUN             ecum_request_run
#endif
#ifndef EcuM_ReleaseRUN
#define EcuM_ReleaseRUN             ecum_release_run
#endif
#ifndef EcuM_RequestPOST_RUN
#define EcuM_RequestPOST_RUN        ecum_request_post_run
#endif
#ifndef EcuM_ReleasePOST_RUN
#define EcuM_ReleasePOST_RUN        ecum_release_post_run
#endif
#ifndef EcuM_SelectShutdownTarget
#define EcuM_SelectShutdownTarget   ecum_select_shutdown_target
#endif
#ifndef EcuM_GetShutdownTarget
#define EcuM_GetShutdownTarget      ecum_get_shutdown_target
#endif
#ifndef EcuM_GetLastShutdownTarget
#define EcuM_GetLastShutdownTarget  ecum_get_last_shutdown_target
#endif
#ifndef EcuM_SelectShutdownCause
#define EcuM_SelectShutdownCause    ecum_select_shutdown_cause
#endif
#ifndef EcuM_GetShutdownCause
#define EcuM_GetShutdownCause       ecum_get_shutdown_cause
#endif
#ifndef EcuM_SelectBootTarget
#define EcuM_SelectBootTarget       ecum_select_boot_target
#endif
#ifndef EcuM_GetBootTarget
#define EcuM_GetBootTarget          ecum_get_boot_target
#endif
#ifndef EcuM_GetPendingWakeupEvents
#define EcuM_GetPendingWakeupEvents ecum_get_pending_wakeup_events
#endif
#ifndef EcuM_ClearWakeupEvent
#define EcuM_ClearWakeupEvent       ecum_clear_wakeup_event
#endif
#ifndef EcuM_GetValidatedWakeupEvents
#define EcuM_GetValidatedWakeupEvents ecum_get_validated_wakeup_events
#endif
#ifndef EcuM_GetStatusOfWakeupSource
#define EcuM_GetStatusOfWakeupSource ecum_get_status_of_wakeup_source
#endif
#ifndef EcuM_SetWakeupEvent
#define EcuM_SetWakeupEvent         ecum_set_wakeup_event
#endif
#ifndef EcuM_ValidateWakeupEvent
#define EcuM_ValidateWakeupEvent    ecum_validate_wakeup_event
#endif
#ifndef EcuM_GoDown
#define EcuM_GoDown                 ecum_go_down
#endif
#ifndef EcuM_GoHalt
#define EcuM_GoHalt                 ecum_go_halt
#endif
#ifndef EcuM_GoPoll
#define EcuM_GoPoll                 ecum_go_poll
#endif

#ifdef __cplusplus
}
#endif

#endif // ECUM_H
