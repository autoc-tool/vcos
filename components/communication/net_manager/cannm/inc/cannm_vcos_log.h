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

#ifndef CANNM_VCOS_LOG_H
#define CANNM_VCOS_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#define CANNM_DLT_PASSIVE_START_UP_ID_00                              "CanNm passive startup, channel_id %u\r\n"
#define CANNM_DLT_NETWORK_REQUEST_ID_01                               "CanNm request network, channel_id %u\r\n"
#define CANNM_DLT_NETWORK_RELEASE_ID_02                               "CanNm release network, channel_id %u\r\n"
#define CANNM_DLT_RX_INDICATION_ID_03                                 "CanNm receive pdu, cannm_rx_pdu_id %u\r\n"
#define CANNM_DLT_RX_INDICATION_ID_04                                 "CanNm receive sdu, pni %u\r\n"
#define CANNM_DLT_SET_PNI_CBV_ID_05                                   "CanNm set pni in CBV\r\n"
#define CANNM_DLT_PREPAREBUSSLEEP_TO_REPEATMESSAGE_ID_06              "Prepare bus sleep to repeat message\r\n"
#define CANNM_DLT_PREPAREBUSSLEEP_TO_REPEATMESSAGE_ID_07              "Notify Nm repeat message state\r\n"
#define CANNM_DLT_PREPAREBUSSLEEP_TO_BUSSLEEP_ID_08                   "Prepare bus sleep to bus sleep state\r\n"
#define CANNM_DLT_PREPAREBUSSLEEP_TO_BUSSLEEP_ID_09                   "Notify Nm bus sleep state\r\n"
#define CANNM_DLT_BUSSLEEP_TO_REPEATMESSAGE_ID_10                     "Bus sleep to repeat message\r\n"
#define CANNM_DLT_BUSSLEEP_TO_REPEATMESSAGE_ID_11                     "Notify Nm repeat message state\r\n"
#define CANNM_DLT_BUSSLEEP_TO_BUSSLEEP_ID_12                          "Bus sleep to bus sleep\r\n"
#define CANNM_DLT_REPEATMESSAGE_TO_REPEATMESSAGE_ID_13                "Repeat message to repeat message\r\n"
#define CANNM_DLT_REPEATMESSAGE_TO_READYSLEEP_ID_14                   "Repeat message state to ready sleep\r\n"
#define CANNM_DLT_REPEATMESSAGE_TO_READYSLEEP_ID_15                   "Notify Nm ready sleep state\r\n"
#define CANNM_DLT_REPEATMESSAGE_TO_NORMALOPERATION_ID_16              "Repeat message to normal operation\r\n"
#define CANNM_DLT_REPEATMESSAGE_TO_NORMALOPERATION_ID_17              "Notify Nm normal operation state\r\n"
#define CANNM_DLT_NORMALOPERATION_TO_REPEATMESSAGE_ID_18              "Normal operation to repeat message\r\n"
#define CANNM_DLT_NORMALOPERATION_TO_REPEATMESSAGE_ID_19              "Notify Nm repeat message state\r\n"
#define CANNM_DLT_NORMALOPERATION_TO_READYSLEEP_ID_20                 "Normal operation to ready sleep\r\n"
#define CANNM_DLT_NORMALOPERATION_TO_READYSLEEP_ID_21                 "Notify Nm ready sleep state\r\n"
#define CANNM_DLT_READYSLEEP_TO_NORMALOPERATION_ID_22                 "Ready sleep to normal operation\r\n"
#define CANNM_DLT_READYSLEEP_TO_NORMALOPERATION_ID_23                 "Notify Nm normal operation state\r\n"
#define CANNM_DLT_READYSLEEP_TO_PREPAREBUSSLEEP_ID_24                 "Ready sleep to prepare bus sleep\r\n"
#define CANNM_DLT_READYSLEEP_TO_PREPAREBUSSLEEP_ID_25                 "Notify Nm prepare bus sleep state\r\n"
#define CANNM_DLT_READYSLEEP_TO_REPEATMESSAGE_ID_26                   "Notify Nm repeat message state\r\n"
#define CANNM_DLT_RX_NORMAL_RECEPTION_ID_27                           "PN disabled, normal reception\r\n"
#define CANNM_DLT_RX_NORMAL_RECEPTION_ID_28                           "No pni, normal reception\r\n"
#define CANNM_DLT_RX_DISCARD_RECEPTION_ID_29                          "No pni, discard reception\r\n"
#define CANNM_DLT_RX_DISCARD_RECEPTION_ID_30                          "Filter disabled, discard reception\r\n"
#define CANNM_DLT_RX_PROCESS_ID_31                                    "EIRA changed, PduId %u\r\n"
#define CANNM_DLT_RX_PROCESS_ID_32                                    "ERA changed, PduId %u\r\n"
#define CANNM_DLT_TX_PDU_PROCESS_ID_33                                "EIRA changed, PduId %u\r\n"
#define CANNM_DLT_READYSLEEP_TO_REPEATMESSAGE_ID_34                   "Ready sleep to repeat message\r\n"
#define CANNM_DLT_DISABLE_COMMUNICATION_ID_35                         "Set support_comm to FALSE\r\n"
#define CANNM_DLT_ENABLE_COMMUNICATION_ID_36                          "Set support_comm to TRUE\r\n"
#define CANNM_DLT_REQUEST_BUS_SYNCHRONIZATION_ID_37                   "Request bus synchronization failed\r\n"
#define CANNM_DLT_SET_SLEEP_READY_BIT_ID_38                           "Set sleep ready bit failed\r\n"
#define CANNM_DLT_RX_BUSSLEEP_ID_39                                   "CanNm receives a NM PDU in bus sleep mode\r\n"

#ifdef __cplusplus
}
#endif

#endif
