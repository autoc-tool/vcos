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

#ifndef STD_TYPES_H
#define STD_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <Platform_Types.h>
#include "Compiler.h"

/**
 * @brief Physical state 5V or 3.3V or 1.8v
 */
#define STD_HIGH    0x01u

/**
 * @brief Physical state 0V.
 */
#define STD_LOW     0x00u

/**
 * @brief Logical state active.
 */
#define STD_ACTIVE  0x01u

/**
 * @brief Logical state idle.
 */
#define STD_IDLE    0x00u

/**
 * @brief ON State.
 */
#define STD_ON      0x01u

/**
 * @brief OFF state.
 */
#define STD_OFF     0x00u

/**
 * @brief Return code for failure/error.
 */
#define E_NOT_OK 0x01u

/**
 * Because E_OK is already defined within OSEK, the symbol E_OK has to be
 * shared. To avoid name clashes and redefinition problems, the symbols have to be
 * defined in the following way (approved within implementation):
 */
/**
 * @brief Success return code
 */
#define E_OK 0x00u

/**
 * @brief To compatile to OSEK definition.
 */
typedef unsigned char StatusType;

typedef uint8 Std_ReturnType;

/**
 * @brief This type shall be used to request the version of a BSW module using the <Module
 *         name>_GetVersionInfo() function.
 */
typedef struct {
    uint16  vendorID;               /**< @brief vendor ID */
    uint16  moduleID;               /**< @brief module ID */
    uint8   sw_major_version;       /**< @brief software major version */
    uint8   sw_minor_version;       /**< @brief software minor version */
    uint8   sw_patch_version;       /**< @brief software patch version */
} Std_VersionInfoType;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef STD_TYPES_H */
