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

/* including file */
#include "pdur.h"
#include "pdur_types.h"
#include "pdur_routingcfg.h"
#include "pdur_pbcfg.h"
#include "pdur_internal.h"
#if (PDUR_ZERO_COST_OPERATION == STD_OFF)

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_is_up_module(uint8 input)
{
	if ((input > PDUR_UP_MODULES) && (input < PDUR_LOIF_MODULES)) {
		return (boolean)TRUE;
	}
	return FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_is_if_module(uint8 input)
{
	if ((input > PDUR_LOIF_MODULES) && (input < PDUR_LOTP_MODULES)) {
		return (boolean)TRUE;
	}
	return (boolean)FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_is_tp_module(uint8 input)
{
	if ((input > PDUR_LOTP_MODULES) && (input < PDUR_END_OF_MODULES)) {
		return (boolean)TRUE;
	}
	return (boolean)FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(boolean, PDUR_CODE) pdur_is_lo_module(uint8 input)
{
	if ((pdur_is_if_module(input) == (boolean)TRUE) || (pdur_is_tp_module(input) == (boolean)TRUE)) {
		return (boolean)TRUE;
	}
	return (boolean)FALSE;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_route_transmit(const pdur_dest_pdu_type *destination, const PduInfoType *pdu_info)
{
	Std_ReturnType ret_val = E_NOT_OK;

	if (destination == NULL) {
		return ret_val;
	}
	uint8 route_table_len = pdur_get_len_of_route_transmit_func_table();
	const pdur_route_transmit_func_table_type *route_func_table = pdur_get_route_transmit_func_table();

	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == destination->dest_module) {
			ret_val = route_func_table[itr].func_ptr(destination->dest_pdu_id, pdu_info);
			break;
		}
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_route_rx_indication(const pdur_dest_pdu_type *destination, const PduInfoType *pdu_info)
{
	if (destination == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTETPRXINDICATION_PDUR_E_NULL_POINTER);
		return;
	}
	uint8 route_table_len = pdur_get_len_of_route_rx_indication_func_table();
	const pdur_route_rx_indication_func_table_type *route_func_table = pdur_get_route_rx_indication_func_table();

	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == destination->dest_module) {
			route_func_table[itr].func_ptr(destination->dest_pdu_id, pdu_info);
			break;
		}
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_up_cancel_transmit(PduIdType pdu_id, uint8 service_id)
{
	(void)service_id;
#if (PDUR_SUPPORT_CANCELTRANSMIT == STD_ON)
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ALLOCATEBUFFER_PDUR_E_PDU_ID_INVALID);
		return E_NOT_OK;
	}
	uint8 route_table_len = pdur_get_len_of_route_cancel_transmit_func_table();
	const pdur_route_cancel_transmit_func_table_type *route_func_table =
		pdur_get_route_cancel_transmit_func_table();
	Std_ReturnType ret_val = E_OK;
	const pdur_routing_path_type *route = &pdur_config_ptr->routing_paths[pdu_id];

	for (uint8 itr = 0u; itr < route->pdur_destinatn_num; itr++) {
		const pdur_dest_pdu_type *destination = route->pdur_dest_pdus[itr];

		for (uint8 itr2 = 0u; itr2 < route_table_len; itr2++) {
			Std_ReturnType ret_val_temp = E_NOT_OK;

			if (route_func_table[itr2].module_id != destination->dest_module) {
				continue;
			}
			ret_val_temp = route_func_table[itr2].func_ptr(destination->dest_pdu_id);
			if (ret_val_temp != E_OK) {
				ret_val = ret_val_temp;
			}
			break;
		}
	}
	return ret_val;
#else
	(void)pdu_id;
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_up_cancel_receive(PduIdType pdu_id, uint8 service_id)
{
	(void)service_id;
#if (PDUR_SUPPORT_CANCELRECEIVE == STD_ON)
	const pdur_pb_config_type *pdur_config_ptr = pdur_get_pdur_config();

	if (pdu_id >= pdur_config_ptr->n_routing_paths) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ALLOCATEBUFFER_PDUR_E_PDU_ID_INVALID);
		return E_NOT_OK;
	}
	uint8 route_table_len = pdur_get_len_of_route_cancel_receive_func_table();
	const pdur_route_cancel_receive_func_table_type *route_func_table = pdur_get_route_cancel_receive_func_table();
	Std_ReturnType ret_val = E_OK;
	const pdur_routing_path_type *route = &pdur_config_ptr->routing_paths[pdu_id];

	for (uint8 itr = 0u; itr < route->pdur_destinatn_num; itr++) {
		const pdur_dest_pdu_type *destination = route->pdur_dest_pdus[itr];

		for (uint8 itr2 = 0u; itr2 < route_table_len; itr2++) {
			Std_ReturnType ret_val_temp = E_NOT_OK;

			if (route_func_table[itr2].module_id != destination->dest_module) {
				continue;
			}
			ret_val_temp = route_func_table[itr2].func_ptr(destination->dest_pdu_id);
			if (ret_val_temp != E_OK) {
				ret_val = ret_val_temp;
			}
			break;
		}
	}
	return ret_val;
#else
	(void)pdu_id;
	return E_OK;
#endif
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_route_tp_rx_indication(const pdur_dest_pdu_type *destination, NotifResultType result)
{
	if (destination == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTETPRXINDICATION_PDUR_E_NULL_POINTER);
		return;
	}
	uint8 route_table_len = pdur_get_len_of_route_tp_rx_indication_func_table();
	const pdur_route_tp_rx_indication_func_table_type *route_func_table =
		pdur_get_route_tp_rx_indication_func_table();
	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == destination->dest_module) {
			route_func_table[itr].func_ptr(destination->dest_pdu_id, result);
			break;
		}
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_route_tp_tx_confirmation(const pdur_routing_path_type *route, NotifResultType result)
{
	if (route == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTETPTXCONFIRMATION_PDUR_E_NULL_POINTER);
		return;
	}
	uint8 route_table_len = pdur_get_len_of_route_tp_tx_confirmation_func_table();
	const pdur_route_tp_tx_confirmation_func_table_type *route_func_table =
		pdur_get_route_tp_tx_confirmation_func_table();
	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == route->src_module) {
			route_func_table[itr].func_ptr(route->src_pdu_id, result);
			break;
		}
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(void, PDUR_CODE) pdur_route_tx_confirmation(const pdur_routing_path_type *route, NotifResultType result)
{
	if (route == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTETPTXCONFIRMATION_PDUR_E_NULL_POINTER);
		return;
	}
	uint8 route_table_len = pdur_get_len_of_route_tx_confirmation_func_table();
	const pdur_route_tx_confirmation_func_table_type *route_func_table =
		pdur_get_route_tx_confirmation_func_table();
	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == route->src_module) {
			route_func_table[itr].func_ptr(route->src_pdu_id, result);
			break;
		}
	}
	return;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(Std_ReturnType, PDUR_CODE) pdur_route_trigger_transmit(const pdur_routing_path_type *route, PduInfoType *pdu_info)
{
	Std_ReturnType ret_val = E_NOT_OK;

	if (route == NULL) {
		return ret_val;
	}
	uint8 route_table_len = pdur_get_len_of_route_trigger_transmit_func_table();
	const pdur_route_trigger_transmit_func_table_type *route_func_table =
		pdur_get_route_trigger_transmit_func_table();
	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == route->src_module) {
			ret_val = route_func_table[itr].func_ptr(route->src_pdu_id, pdu_info);
			break;
		}
	}
	if (ret_val != E_OK) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTETRIGGERTRANSMIT_FAILED);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_route_copy_rx_data(const pdur_dest_pdu_type *destination, const PduInfoType *info, PduLengthType *buffer_size_ptr)
{
	BufReq_ReturnType ret_val = BUFREQ_E_NOT_OK;

	if (destination == NULL) {
		return ret_val;
	}
	uint8 route_table_len = pdur_get_len_of_route_copy_rx_data_func_table();
	const pdur_route_copy_rx_data_func_table_type *route_func_table = pdur_get_route_copy_rx_data_func_table();

	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == destination->dest_module) {
			ret_val = route_func_table[itr].func_ptr(destination->dest_pdu_id, info, buffer_size_ptr);
			break;
		}
	}
	if (ret_val != BUFREQ_OK) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTECOPYRXDATA_FAILED);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_route_copy_tx_data(const pdur_routing_path_type *route, const PduInfoType *info, const RetryInfoType *retry,
			PduLengthType *available_data_ptr)
{
	BufReq_ReturnType ret_val = BUFREQ_E_NOT_OK;

	if (route == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTECOPYTXDATA_PDUR_E_NULL_POINTER);
		return ret_val;
	}
	uint8 route_table_len = pdur_get_len_of_route_copy_tx_data_func_table();
	const pdur_route_copy_tx_data_func_table_type *route_func_table = pdur_get_route_copy_tx_data_func_table();

	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == route->src_module) {
			ret_val = route_func_table[itr].func_ptr(route->src_pdu_id, info, retry, available_data_ptr);
			break;
		}
	}
	if (ret_val != BUFREQ_OK) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTECOPYTXDATA_FAILED);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(BufReq_ReturnType, PDUR_CODE)
pdur_route_start_of_reception(const pdur_dest_pdu_type *destination, const PduInfoType *info,
			      PduLengthType tp_sdu_length, PduLengthType *buffer_size_ptr)
{
	BufReq_ReturnType ret_val = BUFREQ_E_NOT_OK;

	if (destination == NULL) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTECOPYRXDATA_PDUR_E_NULL_POINTER);
		return ret_val;
	}
	uint8 route_table_len = pdur_get_len_of_route_start_of_reception_func_table();
	const pdur_route_start_of_reception_func_table_type *route_func_table =
		pdur_get_route_start_of_reception_func_table();
	for (uint8 itr = 0u; itr < route_table_len; itr++) {
		if (route_func_table[itr].module_id == destination->dest_module) {
			ret_val = route_func_table[itr].func_ptr(destination->dest_pdu_id, info, tp_sdu_length,
								 buffer_size_ptr);
			break;
		}
	}
	if (ret_val != BUFREQ_OK) {
		PDUR_DET_REPORT(PDUR_PDUR_SERVICEID_ROUTESTARTOFRECEPTION_FAILED);
	}
	return ret_val;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
#endif

#define PDUR_START_SEC_CODE
#include "pdur_memmap.h"
FUNC(pdur_dest_pdu_len_strategy_type, PDUR_CODE)
pdur_check_and_handle_dest_pdu_len_strategy(const pdur_dest_pdu_type *destination, const PduInfoType *pdu_info)
{
	if (pdur_is_if_module(destination->dest_module) == (boolean)FALSE) {
		return PDUR_STRATEGY_UNUSED;
	}
	if (destination->pdu_len_strategy == PDUR_STRATEGY_UNUSED) {
		return PDUR_STRATEGY_UNUSED;
	}
	if (pdu_info->SduLength <= destination->pdu_len_in_ecuc) {
		return PDUR_STRATEGY_UNUSED;
	}
	return destination->pdu_len_strategy;
}

#define PDUR_STOP_SEC_CODE
#include "pdur_memmap.h"
