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

#ifndef CANSM_H
#define CANSM_H

#include "comstack_types.h"
#include "cansm_types.h"
#include "comm_types.h"
#include "cansm_lcfg.h"
#if ((CONFIG_MODULE_COMM == 1) && defined(VCOS_MODULE_CONFIG_RTE) && (VCOS_MODULE_CONFIG_RTE == 1))
#include "Rte_ComM_Type.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CANSM_VENDOR_ID                             (60U)       ///< vendor Id
#define CANSM_MODULE_ID                             (140U)      ///< module Id

#define INVALID_NETWORK_ID                          (0xFFU)     ///< Invalid network id
#define INVALID_CONTROLLER_ID                       (0xFFU)     ///< Invalid controller id
#define INVALID_TRANSCEIVER_ID                      (0xFFU)     ///< Invalid transceiver id

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       CanSM module initialization
 * @details     This service initializes the CanSM module
 * @param[in]   config_ptr  Pointer to the initialization structure of CanSM built parameters
 * @reentrant   Non Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_init(const cansm_config_type *config_ptr);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       CanSM version information
 * @details     This service provides version information for the module
 * @param[out]  versioninfo  Pointer to the location where the version information of the module is stored
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(void, CANSM_CODE) cansm_get_version_info(Std_VersionInfoType* versioninfo);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Request CanSM module to switch communication mode
 * @details     Change the communication mode of the CAN network to the requested mode
 * @param[in]   network_handle   network handle
 * @param[in]   comm_mode       Request to switch network mode
 * @ret_val      E_OK            Request acceptance
 * @ret_val      E_NOT_OK        Request Rejection
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANSM_CODE) cansm_request_com_mode(network_handle_type network_handle, comm_mode_type comm_mode);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Output the current communication mode of CanSM module
 * @details     Output the communication mode of the current CAN network
 * @param[in]   network_handle   network handle
 * @param[in]   comm_mode_ptr    Current network mode
 * @ret_val      E_OK            Request acceptance
 * @ret_val      E_NOT_OK        Request Rejection
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANSM_CODE) cansm_get_cur_com_mode(network_handle_type network_handle,
	comm_mode_type *comm_mode_ptr);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Obtain the number of CanSM networks
 * @details     Obtain the number of CanSM networks
 * @reentrant   Non Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(uint8, CANSM_CODE) cansm_get_network_count(void);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       EcuM notifies CanSM that there is a startup wake-up resource available
 * @details     When the wake-up source starts, EcuM will call this function
 * @param[in]   network_handle   network handle
 * @ret_val      E_OK            Request acceptance
 * @ret_val      E_NOT_OK        Request Rejection
 * @reentrant   Non Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANSM_CODE) cansm_start_wake_up_source(network_handle_type network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       EcuM notifies CanSM that there is a stop wake-up resource
 * @details     When the wake-up source stops, EcuM will call this function
 * @param[in]   network_handle   network handle
 * @ret_val      E_OK            Request acceptance
 * @ret_val      E_NOT_OK        Request Rejection
 * @reentrant   Non Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANSM_CODE) cansm_stop_wake_up_source(network_handle_type network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       CanNm has detected a tx timeout exception in some affected CAN networks
 * @details     Notify the CanSM module that CanNm has detected a tx timeout exception in the affected CAN network
 * @param[in]   network_handle   network handle
 * @ret_val      E_OK            Request acceptance
 * @ret_val      E_NOT_OK        Request Rejection
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(Std_ReturnType, CANSM_CODE) cansm_tx_timeout_exception(network_handle_type network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       CanSM obtains the bus_off_status of the network corresponding to this core based on the NetworkId
 * @details     When NetworkId is valid, return the bus_off_status of the corresponding network; otherwise, return False
 * @param[in]   network_handle   network handle
 * @ret_val      bus_off_status
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(boolean, CANSM_CODE) cansm_get_bus_off_status(network_handle_type network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       CanSM obtains the bus_off_counter corresponding to the network of this core based on the NetworkId
 * @details     When NetworkId is valid, return bus_off_status of corresponding network; otherwise, return 0xFFFF
 * @param[in]   network_handle   network handle
 * @ret_val      bus_off_counter
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC(uint16, CANSM_CODE) cansm_get_bus_off_counter(network_handle_type network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#define CANSM_START_SEC_CODE
#include "cansm_memmap.h"
/*!*********************************************************************************************************************
 * @brief       Obtains state machine structure pointer of corresponding network of local core based on the NetworkId
 * @details     When NetworkId is valid, return a pointer to the corresponding network; otherwise, it is a null pointer
 * @param[in]   network_handle   network handle
 * @ret_val      Pointer to network
 * @reentrant   Reentrant
 * @synchronous Synchronous
 **********************************************************************************************************************/
FUNC_P2VAR(cansm_internal_network_type, const, CANSM_CODE) cansm_get_network_ptr(network_handle_type network_handle);
#define CANSM_STOP_SEC_CODE
#include "cansm_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* CANSM_H */

