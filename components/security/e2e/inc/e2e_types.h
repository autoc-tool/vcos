/*
 * copyright (c) 2025 li auto inc. and its affiliates
 * licensed under the apache license, version 2.0(the "License");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#ifndef E2E_TYPES_H
#define E2E_TYPES_H
#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define E2E_E_INPUTERR_NULL (0x13u)
#define E2E_E_INPUTERR_WRONG (0x17u)
#define E2E_E_INTERR (0x19u)
#define E2E_E_OK (0x00u)
#define E2E_E_WRONGSTATE (0x1au)

/*!*********************************************************************************************************************
 * @brief   定义pc校验状态类型
 * 0x80 0x40 0x20 0x10 0x08 0x04 0x02 0x01
 * 0    0    0    0    1    0    0    0            0x08 OK
 * 0    0    0    1    0    0    0    0            0x10 REPERATED
 * 0    0    1    0    0    0    0    0            0x20 WRONGSEQUENCE
 * 0    1    0    0    0    0    0    0            0x40 ERROR
 * 1    0    0    0    0    0    0    0            0x80 NONEWDATA
 * 1    1    1    1    1    0    0    0            0xF8 MAP_MASK
 * 0    0    1    0    1    0    0    0            0x28 XOR_MASK
 **********************************************************************************************************************/
#define E2E_P_OK                                0x08u
#define E2E_P_REPEATED                          0x10u
#define E2E_P_WRONGSEQUENCE                     0x20u
#define E2E_P_ERROR                             0x40u
#define E2E_P_NONEWDATA                         0x80u
#define E2E_P_NOTAVAILABLE                      0xFFu
#define E2E_P_MAP_MASK                          0xF8u
#define E2E_P_XOR_MASK                          0x28u
#ifdef __cplusplus
}
#endif

#endif /* E2E_TYPES_H */
