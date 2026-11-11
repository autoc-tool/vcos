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

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"
#include "comstack_cfg.h"

typedef uint8 IcomConfigIdType;

typedef uint8 PNCHandleType;

typedef uint8 BusTrcvErrorType;
#define BUSTRCV_OK ((BusTrcvErrorType)0x00u)
#define BUSTRCV_E_ERROR ((BusTrcvErrorType)0x01u)

typedef enum {
	ICOM_SWITCH_E_OK,
	ICOM_SWITCH_E_FAILED
} IcomSwitch_ErrorType;

typedef struct {
	uint8 *SduDataPtr;
	uint8 *MetaDataPtr;
	PduLengthType SduLength;
} PduInfoType;

typedef enum {
	TP_STMIN = 0,
	TP_BS,
	TP_BC,
} TPParameterType;

typedef enum {
	TP_DATACONF,
	TP_DATARETRY,
	TP_CONFPENDING,
	TP_NORETRY,
} TpDataStateType;

typedef struct {
	TpDataStateType TpDataState;
	PduLengthType TxTpDataCnt;
} RetryInfoType;

typedef enum {
	BUFREQ_OK,
	BUFREQ_E_NOT_OK,
	BUFREQ_E_BUSY,
	BUFREQ_E_OVFL
} BufReq_ReturnType;

#define BUSTRCV_NO_ERROR 0x00
#define BUSBUSTRCV_E_ERROR 0x01

/*****************************************************************************
**                      Global Data                                         **
*****************************************************************************/
typedef uint8 NotifResultType;
#define NTFRSLT_OK 0x00
#define NTFRSLT_E_NOT_OK 0x01
#define NTFRSLT_E_TIMEOUT_A 0x02
#define NTFRSLT_E_TIMEOUT_BS 0x03
#define NTFRSLT_E_TIMEOUT_CR 0x04
#define NTFRSLT_E_WRONG_SN 0x05
#define NTFRSLT_E_INVALID_FS 0x06
#define NTFRSLT_E_UNEXP_PDU 0x07
#define NTFRSLT_E_WFT_OVRN 0x08
#define NTFRSLT_E_ABORT 0x09
#define NTFRSLT_E_NO_BUFFER 0x0A
#define NTFRSLT_E_CANCELATION_OK 0x0B
#define NTFRSLT_E_CANCELATION_NOT_OK 0x0C
#define NTFRSLT_PARAMETER_OK 0x0D
#define NTFRSLT_E_PARAMETER_NOT_OK 0x0E
#define NTFRSLT_E_RX_ON 0x0F
#define NTFRSLT_E_VALUE_NOT_OK 0x10

#define NTFRSLT_E_FR_ML_MISMATCH 0x5B
#define NTFRSLT_E_FR_WRONG_BP 0x5C
#define NTFRSLT_E_FR_TX_ON 0x5D

typedef uint8 NetworkHandleType;

#endif
