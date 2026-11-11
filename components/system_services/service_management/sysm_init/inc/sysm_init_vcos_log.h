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

#ifndef ECUM_VCOS_LOG_H
#define ECUM_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define ECUM_DLT_ENTER_RUN "[EcuM] EcuM enter to RUN \r\n" ///< EcuM进入RUN状态
#define ECUM_DLT_CORE_ID "[EcuM] Core ID is %d \r\n" ///< 核心ID
#define ECUM_DLT_WAKEUP_REACTION "[EcuM] EcuM in state WAKEUP_REACTION \r\n" ///< EcuM处于WAKEUP_REACTION状态
#define ECUM_DLT_GO_DOWN "[EcuM] System in state Godown \r\n" ///< 系统进入Godown状态
#define ECUM_DLT_SLEEP "[EcuM] EcuM in state Sleep \r\n" ///< EcuM处于Sleep状态
#define ECUM_DLT_CHECK_WAKEUP "[EcuM] EcuM in state CheckWakeup \r\n" ///< EcuM处于CheckWakeup状态
#define ECUM_DLT_SWITCH_OFF "[EcuM] EcuM_AL_SwitchOff \r\n" ///< EcuM_AL_SwitchOff
#define ECUM_DLT_DRIVER_RESTART "[EcuM] EcuM_AL_DriverRestart \r\n" ///< EcuM_AL_DriverRestart
#define ECUM_DLT_WAKEUPSOURCE_ID "[EcuM] wakeupsource is %lu \r\n" ///< 唤醒源ID
#define ECUM_DLT_VCOSMODULE_TIME "[EcuM] VCOS Module Init Time is %f ms \r\n" ///< VCOS模块初始时间
#define ECUM_DLT_STARTUP2_TIME "[EcuM] Startup Two %f ms \r\n" ///< 启动阶段2时间
#define ECUM_DLT_INITZERO_TIME "[EcuM] INITZERO %f ms \r\n" ///< INITZERO时间
#define ECUM_DLT_INITONE_TIME "[EcuM] INITONE %f ms \r\n" ///< INITONE时间
#define ECUM_DLT_INITTWO_TIME "[EcuM] INITTWO %f ms \r\n" ///< INITTWO时间
#define ECUM_DLT_INITTHREE_TIME "[EcuM] INITTHREE %f ms \r\n" ///< INITTHREE时间
#define ECUM_DLT_RTESYNC_TIME "[EcuM] RteSync %f ms \r\n" ///< RTE同步时间
#define ECUM_DLT_STARTUP_TIME_DEFAULT "[EcuM] startup: %lu, %f ms \r\n" ///< 启动时间戳
#define ECUM_DLT_INITTWO_TIME_DEFAULT "[EcuM] InitTwo: %lu, %f ms\r\n" ///< INITTWO时间戳
#define ECUM_DLT_INITTHREE_TIME_DEFAULT "[EcuM] InitThree: %lu, %f ms\r\n" ///< INITTHREE时间戳
#define ECUM_DLT_RTESYNC_TIME_DEFAULT "[EcuM] RteSync: %lu, %f ms\r\n" ///< RTE同步时间戳
#ifdef __cplusplus
}
#endif

#endif