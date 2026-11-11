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

#ifndef COMSTACK_CFG_H
#define COMSTACK_CFG_H

/* Version and Check Begin */
/* Version Info Begin */
#define COMSTACK_CFG_H_VENDOR_ID    0x8C
#define COMSTACK_CFG_H_AR_RELEASE_MAJOR_VERSION    4
#define COMSTACK_CFG_H_AR_RELEASE_MINOR_VERSION    3
#define COMSTACK_CFG_H_AR_RELEASE_REVISION_VERSION 1
#define COMSTACK_CFG_H_SW_MAJOR_VERSION    1
#define COMSTACK_CFG_H_SW_MINOR_VERSION    0
#define COMSTACK_CFG_H_SW_PATCH_VERSION    0
/* Version Info End */
/* Version and Check End */

/*****************************************************************************
**                      Include Section                                     **
*****************************************************************************/

/*****************************************************************************
**                      Global Symbols                                      **
*****************************************************************************/

/*****************************************************************************
**                      Global Data Types (ECU dependent)                   **
*****************************************************************************/

/* [cover parentID={4FFAA8E2-213A-4482-A81D-4A4E70AF1A37}]PduIdType [/cover]*/
typedef uint16        PduIdType;     /* Type of PDU ID.
                                    Allowed ranges: uint8 .. uint16 */

/* [cover parentID={2C4B621E-4460-43f2-89DC-12DE51986F1D}]PduLengthType
[/cover]*/
typedef uint16       PduLengthType; /* Type of PDU Length.
                                    Allowed ranges: uint8 .. uint32 */

#endif
