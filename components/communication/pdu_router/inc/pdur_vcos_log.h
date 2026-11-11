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

#ifndef PDUR_VCOS_LOG_H
#define PDUR_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define PDUR_DLT_CHECK_ID0 "Alloc buffer Size %u, SduLength %u, status %u\r\n"
#define PDUR_DLT_CHECK_ID1 "PduId %u, Len %u, Data %u %u %u %u %u %u %u %u\r\n"
#define PDUR_DLT_CHECK_ID2 "PduR_ARC_RouteTpRxIndication, UpModule.\r\n"
#define PDUR_DLT_CHECK_ID3 "PduR_ARC_RxIndication lasted %llu ms.\r\n"
#define PDUR_DLT_CHECK_ID4 "Release buffer status[%u], PduId[%u]\r\n"
#define PDUR_DLT_CHECK_ID5 "PduId %u, Data %u %u %u %u %u %u %u %u\r\n"
#define PDUR_DLT_CHECK_ID6 "PduId %u, gateway on the fly.\r\n"
#define PDUR_DLT_CHECK_ID7 "BUFREQ_E_BUSY, PduId %u\r\n"
#define PDUR_DLT_CHECK_ID8 "PduId %u, Data %u %u %u %u %u %u %u %u\r\n"
#define PDUR_DLT_CHECK_ID9 "PduId %u, Data %u %u %u %u %u %u %u %u\r\n"
#define PDUR_DLT_CHECK_ID10 "Data %u %u %u %u %u %u %u %u\r\n"
#define PDUR_DLT_CHECK_ID11 "Enable PduR routing path group, Id[%u]\r\n"
#define PDUR_DLT_CHECK_ID12 "Disable PduR routing path group, Id[%u]\r\n"
#define PDUR_DLT_CHECK_ID13 "CId %u, Num %u, NofDst %u, Init %u\r\n"
#define PDUR_DLT_CHECK_ROUTING_STATUS_NOT_IDLE "routingState not idle\r\n"
#define PDUR_DLT_CHECK_BUFFER_POOL_ALLOC_FAIL "Buffer alloc failed\r\n"
#define PDUR_DLT_CHECK_ROUTING_STATE_NOT_ACTIVE "Routing state not active\r\n"
#define PDUR_DLT_CHECK_REMAIN_BUFFER_ZERO "Remain buffer zero\r\n"
#define PDUR_DLT_CHECK_REMAIN_BUFFER_LESS_THAN_SDU_LENGTH "Remain buffer less than SduLength\r\n"
#define PDUR_DLT_CHECK_ROUTING_STATE_ERROR "Routing state error\r\n"
#define PDUR_DLT_CHECK_RESULT_E_NOT_OK "Result E_NOT_OK\r\n"
#define PDUR_DLT_CHECK_EVC_FULL "EvC is full\r\n"
#define PDUR_DLT_CHECK_BUFFER_CAN_READ_ZERO "Buffer can read length is 0\r\n"
#define PDUR_DLT_CHECK_SDULENGTH_ZERO "Info SduLength is 0\r\n"
#define PDUR_DLT_CHECK_READREMAINLENGTH_SDULENGTH_ZERO "Read remain length and SduLength both are 0\r\n"
#define PDUR_DLT_CHECK_READREMAINLENGTH_LESS_THAN_SDULENGTH "Read remain length less than SduLength\r\n"
#define PDUR_DLT_CHECK_PENDING_DEST_CNT_NOT_ZERO "Pending Destination count not zero\r\n"
#define PDUR_DLT_CHECK_EVCDATA_LESS_THAN_SDULENGTH "EvC Data less than SduLength\r\n"
#define PDUR_DLT_CHECK_EVCDATA_NULL "EvC Data is NULL\r\n"
#define PDUR_DLT_CHECK_EVCDATA_LENGTH_ZERO "EvC Data length is zero\r\n"
#define PDUR_DLT_CHECK_TYPE_LARGER_THAN_PDUR_BUFFER_POOL_MAX "Type is larger than PDUR_BUFFER_POOL_MAX\r\n"
#define PDUR_DLT_CHECK_COREID_LARGER_THAN_CORE_MAX_NUM "CoreID is larger than CORE_MAX_NUM\r\n"
#define PDUR_DLT_CHECK_EVC_FIFO_DESC_REF_TABLE_NULL "PduREvcFifoDescRefTable is NULL\r\n"
#define PDUR_DLT_CHECK_EVC_FIFO_EMPTY "PduR EvC fifo is empty\r\n"
#define PDUR_DLT_CHECK_EVC_FIFO_POP_NOT_OK "PduR EvC fifo pop failed\r\n"
#define PDUR_DLT_CHECK_PDUID_NOT_EVC_PDUID "PduId is not EvC PduId\r\n"
#define PDUR_DLT_CHECK_EVC_FIFO_PUSH_NOT_OK "PduR EvC Fifo push not ok\r\n"
#define PDUR_DLT_CHECK_EVC_FIFO_DATA_NULL "PduR EvC Fifo data is NULL\r\n"

#define PDUR_DLT_CHECK_TRANSMIT_FAILED "[-PduR-] Dest: %u, PduId: %u \r\n"
#define PDUR_DLT_CHECK_IF_ROUTING_FAILED "[-PduR-] PduId %u Transmit Failed \r\n"
#ifdef __cplusplus
}
#endif

#endif
