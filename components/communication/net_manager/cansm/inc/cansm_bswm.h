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

#ifndef CANSM_BSWM_H
#define CANSM_BSWM_H

#ifdef __cplusplus
extern "C" {
#endif

/*!*********************************************************************************************************************
 * @brief   Definition of communication mode type for CanSM to notify BswM
 **********************************************************************************************************************/
typedef uint8 cansm_bswm_current_state_type;
#define CANSM_BSWM_NO_COMMUNICATION       ((cansm_bswm_current_state_type)0x00u)          ///< no communication
#define CANSM_BSWM_SILENT_COMMUNICATION   ((cansm_bswm_current_state_type)0x01u)          ///< silent communication
#define CANSM_BSWM_FULL_COMMUNICATION     ((cansm_bswm_current_state_type)0x02u)          ///< full communication
#define CANSM_BSWM_BUS_OFF                ((cansm_bswm_current_state_type)0x03u)          ///< BusOff
#define CANSM_BSWM_CHANGE_BAUDRATE        ((cansm_bswm_current_state_type)0x04u)          ///< change baudrate

#ifdef __cplusplus
}
#endif

#endif /* CANSM_BSWM_H */
