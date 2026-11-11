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

#ifndef NMSTACK_TYPES_H
#define NMSTACK_TYPES_H

#include "comstack_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!*********************************************************************************************************************
 * @brief   NM function return type definition
 **********************************************************************************************************************/
typedef uint8 nm_return_type;
#define NM_E_OK                                 ((nm_return_type)0x00u)  ///< Executed successfully
#define NM_E_NOT_OK                             ((nm_return_type)0x01u)  ///< Execution failed
#define NM_E_NOT_EXECUTED                       ((nm_return_type)0x02u)  ///< Not executed

/*!*********************************************************************************************************************
 * @brief   NM network handle type definition
 **********************************************************************************************************************/
typedef uint8 network_handle_type;
/*!*********************************************************************************************************************
 * @brief   network Management Operation mode Definition
 **********************************************************************************************************************/
typedef uint8 nm_mode_type;
#define NM_MODE_BUS_SLEEP                       ((nm_mode_type)0x00u)  ///< bus sleep mode
#define NM_MODE_PREPARE_BUS_SLEEP               ((nm_mode_type)0x01u)  ///< Prepare to enter bus sleep mode
#define NM_MODE_SYNCHRONIZE                     ((nm_mode_type)0x02u)  ///< synchronous mode
#define NM_MODE_NETWORK                         ((nm_mode_type)0x03u)  ///< network mode

/*!*********************************************************************************************************************
 * @brief   network Management state Machine state Definition
 **********************************************************************************************************************/
typedef uint8 nm_state_type;
#define NM_STATE_BUS_SLEEP                      ((nm_state_type)0x00u)  ///< bus sleep state
#define NM_STATE_PREPARE_BUS_SLEEP              ((nm_state_type)0x01u)  ///< prepare for bus sleep state
#define NM_STATE_READY_SLEEP                    ((nm_state_type)0x02u)  ///< ready sleep
#define NM_STATE_NORMAL_OPERATION               ((nm_state_type)0x03u)  ///< normal operation
#define NM_STATE_REPEAT_MESSAGE                 ((nm_state_type)0x04u)  ///< repeat message
#define NM_STATE_SYNCHRONIZE                    ((nm_state_type)0x05u)  ///< sync status

/*!*********************************************************************************************************************
 * @brief   network Management Type Definition
 **********************************************************************************************************************/
typedef uint8 nm_line_cate;
#define NM_BUSNM_CANNM                         ((nm_line_cate)0x00u)  ///< CAN network management
#define NM_BUSNM_FRNM                          ((nm_line_cate)0x01u)  ///< FlexRay network management
#define NM_BUSNM_LINNM                         ((nm_line_cate)0x02u)  ///< LIN network management
#define NM_BUSNM_UDPNM                         ((nm_line_cate)0x03u)  ///< UDP network management
#define NM_BUSNM_OSEKNM                        ((nm_line_cate)0x04u)  ///< OSEK network management
#define NM_BUSNM_UNDEF                         ((nm_line_cate)0xFFu)  ///< Undefined network management

/*!*********************************************************************************************************************
 * @brief   NM channel configuration structure
 **********************************************************************************************************************/
typedef struct {
	const nm_line_cate                  line_cate; ///< The type of network management bus used by channel
} nm_line_channel_cate;

/*!*********************************************************************************************************************
 * @brief   NM module configuration structure
 **********************************************************************************************************************/
typedef struct {
	const nm_line_channel_cate    *nm_tunnel;         ///< A reference to the nm_line_channel_cate array
	const network_handle_type     *tunnel_trans;      ///< Array of mappings from network to nm_line_channel_cate
	const network_handle_type     tunnel_trans_len;   ///< The number of elements in the tunnel_trans
} nm_cate_config;

#ifdef __cplusplus
}
#endif

#endif /* NMSTACK_TYPES_H */

