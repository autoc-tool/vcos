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

#ifndef CRC_TYPES_H
#define CRC_TYPES_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

 /* CRC8 配置 */
 #define CRC_8_INVALID      (0x00)  ///< CRC8（8位循环冗余校验）无效配置
 #define CRC_8_RUNTIME      (0x02)  ///< CRC8 运行时计算配置
 #define CRC_8_TABLE        (0x04)  ///< CRC8 查表法配置

 /* CRC 8H2F 配置 */
 #define CRC_8H2F_INVALID   (0x00)  ///< CRC 8H2F 无效配置
 #define CRC_8H2F_RUNTIME   (0x02)  ///< CRC 8H2F 运行时计算配置
 #define CRC_8H2F_TABLE     (0x04)  ///< CRC 8H2F 查表

 /* CRC 16 配置 */
 #define CRC_16_INVALID     (0x00)  ///< CRC16（16位循环冗余校验）无效配置
 #define CRC_16_RUNTIME     (0x02)  ///< CRC16 运行时计算配置
 #define CRC_16_TABLE       (0x04)  ///< CRC16 查表法配置

 /* CRC 16 ARC 配置 */
 #define CRC_16_ARC_INVALID  (0x00)  ///< CRC16 ARC（16位循环冗余校验，ARC变种）无效配置
 #define CRC_16_ARC_RUNTIME  (0x02)  ///< CRC16 ARC 运行时计算配置
 #define CRC_16_ARC_TABLE    (0x04)  ///< CRC16 ARC 查表法配置

 /* CRC 32 配置 */
 #define CRC_32_INVALID     (0x00)  ///< CRC32（32位循环冗余校验）无效配置
 #define CRC_32_RUNTIME     (0x02)  ///< CRC32 运行时计算配置
 #define CRC_32_TABLE       (0x04)  ///< CRC32 查表法配置

 /* CRC 32P4 配置 */
 #define CRC_32P4_INVALID   (0x00)  ///< CRC 32P4 无效配置
 #define CRC_32P4_RUNTIME   (0x02)  ///< CRC 32P4 运行时计算配置
 #define CRC_32P4_TABLE     (0x04)  ///< CRC 32P4 查表法配置

 /* CRC 64 配置 */
 #define CRC_64_INVALID     (0x00)  ///< CRC64（64位循环冗余校验）无效配置
 #define CRC_64_RUNTIME     (0x02)  ///< CRC64 运行时计算配置
 #define CRC_64_TABLE       (0x04)  ///< CRC64 查表法配置

#ifdef __cplusplus
}
#endif

#endif  /* CRC_TYPES_H */
