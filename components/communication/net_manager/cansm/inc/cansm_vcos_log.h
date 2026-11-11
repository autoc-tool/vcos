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

#ifndef CANSM_VCOS_LOG_H
#define CANSM_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define CANSM_DLT_CHECK_ID_00 "[-CanSM-] Net[%d] Bsm is %d\r\n"                           ///< Bsm state
#define CANSM_DLT_CHECK_ID_01 "[-CanSM-] Net[%d] Report BusOff !\r\n"                     ///< Report BusOff
#define CANSM_DLT_CHECK_ID_02 "[-CanSM-] Net[%d] Finish L1 Bor\r\n"                       ///< Finish L1 Bor
#define CANSM_DLT_CHECK_ID_03 "[-CanSM-] Net[%d] Finish L2 Bor\r\n"                       ///< Finish L2 Bor
#define CANSM_DLT_CHECK_ID_04 "[-CanSM-] Net[%d] CommMode to %d\r\n"                      ///< ComM mode
#define CANSM_DLT_CHECK_ID_05 "[-CanSM-] Net[%d] Set Ctrl[%d] mode to %d\r\n"             ///< Set mode
#define CANSM_DLT_CHECK_ID_06 "[-CanSM-] Net[%d] Ctrl[%d] Indication %ld\r\n"              ///< Control indication
#define CANSM_DLT_CHECK_ID_07 "[-CanSM-] Net[%d] Set Trcv[%d] mode to %d\r\n"             ///< Set Trcv mode
#define CANSM_DLT_CHECK_ID_08 "[-CanSM-] Net[%d] Trcv[%d] Indication %d\r\n"              ///< Indication
#define CANSM_DLT_CHECK_ID_09 "[-CanSM-] Net[%d] Trcv[%d] ClearWufFlag \r\n"              ///< Clear wuf flag
#define CANSM_DLT_CHECK_ID_10 "[-CanSM-] Net[%d] Trcv[%d] Indication Clear\r\n"           ///< Indication clear
#define CANSM_DLT_CHECK_ID_11 "[-CanSM-] Net[%d] Trcv[%d] CheckWufFlag \r\n"              ///< Check wuf flag
#define CANSM_DLT_CHECK_ID_12 "[-CanSM-] Net[%d] Trcv[%d] Indication Check\r\n"           ///< Trcv indication
#define CANSM_DLT_CHECK_ID_13 "[-CanSM-] Net[%d] Set Ctrl[%d] pdu_mode to %ld\r\n"          ///< ctrl_id and pdu_mode
#define CANSM_DLT_CHECK_ID_14 "[-CanSM-] Net[%d] Trcv[%d] Pn Availability\r\n"            ///< Pn availability

#ifdef __cplusplus
}
#endif

#endif /* CANSM_VCOS_LOG_H */
