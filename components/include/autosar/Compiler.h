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

#ifndef COMPILER_H
#define COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <nuttx/compiler.h>

#if defined (__TASKING__)
#include <stddef.h>
#endif

#include "stdbool.h"

/**
 * @brief The memory class AUTOMATIC shall be provided as empty definition, used for
	  the declaration of local pointers. [SWS_COMPILER_00046]
 */
#define AUTOMATIC

/**
 * @brief The memory class TYPEDEF shall be provided as empty definition. This memory
 *        class shall be used within type definitions, where no memory qualifier can be
 *        specified. This can be necessary for defining pointer types, with e.g. P2VAR,
 *        where the macros require two parameters. First parameter can be specified in the
 *        type definition (distance to the memory location referenced by the pointer), but
 *        the second one (memory allocation of the pointer itself) cannot be defined at this
 *        time. Hence, memory class TYPEDEF shall be applied. [SWS_COMPILER_00059]
 */
#define TYPEDEF

/**
 * @brief The compiler abstraction shall provide the NULL_PTR define with a void pointer
 *        to zero definition.
 */
#ifndef NULL_PTR
#define NULL_PTR NULL
#endif

#define LIKELY(expr)                                    predict_true(expr)
#define UNLIKELY(expr)                                  predict_false(expr)

/* 8.2.2 Function definitions */
/**
 * @brief FUNC macro for the declaration and definition of functions.
 *
 *   @param   retype      return type of the function
 *   @param   memclass    classification of the function itself
 */
#define FUNC(rettype, memclass) rettype memclass
/**
* @brief FUNC_P2CONST macro for the declaration and definition of functions returning a pointer to a constant
*/
#define FUNC_P2CONST(rettype, ptrclass, memclass) const ptrclass rettype * memclass
/**
* @brief FUNC_P2VAR macro for the declaration and definition of functions returning a pointer to a variable.
*/
#define FUNC_P2VAR(rettype, ptrclass, memclass) ptrclass rettype * memclass

/* 8.2.3 Pointer definitions */
/**
 * @brief Definition of pointers in RAM, pointing to variables. See [SWS_COMPILER_00006]
 */
#define P2VAR(ptrtype, memclass, ptrclass) ptrtype * memclass
/**
 * @brief iDefinition of pointers in RAM pointing to constants.
 */
#define P2CONST(ptrtype, memclass, ptrclass) const ptrtype * memclass
/**
 * @brief Constant pointers accessing variables.
 */
#define CONSTP2VAR(ptrtype, memclass, ptrclass) ptrtype * const memclass
/**
* @brief Constant pointers accessing constants.
*/
#define CONSTP2CONST(ptrtype, memclass, ptrclass) const ptrtype * const memclass
/**
* @brief Pointers to functions.
*/
#define P2FUNC(rettype, ptrclass, fctname) rettype (*fctname)
/**
 * @brief The compiler abstraction for const pointer to function.
 */
#define CONSTP2FUNC(rettype, ptrclass, fctname) rettype (* const fctname)

/* 8.2.4 Constant definitions */
/**
 * @brief CONST macro
 */
#define CONST(type, memclass) const type memclass

/* 8.2.5 Variable definitions */
/**
 * @brief VAR macro for the declaration and definition of variables.
 */
#define VAR(vartype, memclass) vartype memclass

#ifdef __cplusplus
}
#endif

#endif /* !defined(COMPILER_H) */
