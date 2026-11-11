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

/* This file should never be included directly but, rather,
 * only indirectly through arch/chip/irq.h
 */

#ifndef __ARCH_ARM_INCLUDE_ARMV8_RTFW_ATOMIC_H
#define __ARCH_ARM_INCLUDE_ARMV8_RTFW_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <nuttx/compiler.h>

typedef volatile aligned_data(4) int32_t Atomic;

/*  \brief          Atomic addition
 *  \param[in/out]  ptr     The address of the data to perform atomic addition on
 *  \param[in]      addVal  The value to be added
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicAdd(Atomic *ptr, int32_t addVal)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex    %0, [%3]\n"
        "add    %1, %0, %4\n"
        "strex    %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(addVal)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic subtraction
 *  \param[in/out]  ptr     The address of the data to perform atomic subtraction on
 *  \param[in]      subVal  The value to be subtracted
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicSub(Atomic *ptr, int32_t subVal)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex    %0, [%3]\n"
        "sub    %1, %0, %4\n"
        "strex    %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(subVal)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic increment
 *  \param[in/out]  ptr     The address of the data to perform atomic increment on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicInc(Atomic *ptr)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex   %0, [%3]\n"
        "add   %1, %0, #1\n"
        "strex   %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic decrement
 *  \param[in/out]  ptr     The address of the data to perform atomic decrement on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicDec(Atomic *ptr)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex   %0, [%3]\n"
        "sub   %1, %0, #1\n"
        "strex   %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic OR
 *  \param[in/out]  ptr     The address of the data to perform atomic OR on
 *  \param[in]      orVal   The value to OR with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicOr(Atomic *ptr, int32_t orVal)
{
    register int32_t preVal , val, status;

    __asm __volatile__(
        "1:ldrex    %0, [%3]\n"
        "orr    %1, %0, %4\n"
        "strex    %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(orVal)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic XOR
 *  \param[in/out]  ptr     The address of the data to perform atomic XOR on
 *  \param[in]      xorVal  The value to XOR with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicXor(Atomic *ptr, int32_t xorVal)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex    %0, [%3]\n"
        "eor    %1, %0, %4\n"
        "strex    %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(xorVal)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic AND
 *  \param[in/out]  ptr     The address of the data to perform atomic AND on
 *  \param[in]      andVal  The value to AND with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicAnd(Atomic *ptr, int32_t andVal)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex    %0, [%3]\n"
        "and    %1, %0, %4\n"
        "strex    %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(andVal));

    return preVal;
}

/*  \brief          Atomic AND-NOT
 *  \param[in/out]  ptr      The address of the data to perform atomic AND-NOT on
 *  \param[in]      xandVal  The value to AND-NOT with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicXand(Atomic *ptr, int32_t xandVal)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex    %0, [%3]\n"
        "and    %1, %0, %4\n"
        "mvn   %1, %1\n"
        "strex    %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(xandVal)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic set value
 *  \param[in/out]  ptr     The address of the data to perform atomic set value on
 *  \param[in]      setVal  The value to set
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicSet(Atomic *ptr, int32_t setVal)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex   %0, [%3]\n"
        "strex   %2, %4, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(setVal)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic clear value
 *  \param[in/out]  ptr     The address of the data to perform atomic clear on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicClean(Atomic *ptr)
{
    register int32_t  preVal , val, status;

    __asm __volatile__(
        "1:ldrex    %0, [%3]\n"
        "mov    %1, #0\n"
        "strex    %2, %1, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "DMB"
        : "=&r"(preVal), "=&r"(val), "=&r"(status)
        : "r"(ptr)
        : "cc", "memory");

    return preVal;
}

/*  \brief          Atomic compare and swap
 *  \details        Compare *ptr with oldVal; if they are equal, update *ptr to newVal and return true,
 *                  otherwise return false
 *  \param[in/out]  ptr     The address of the data to perform atomic compare and swap on
 *  \param[in]      oldVal  The old value to compare with
 *  \param[in]      newVal  The new value to update if comparison succeeds
 *  \return
 *                  true (1)    Compare and swap succeeded
 *                  false (0)   Compare and swap failed
 */

static inline int32_t Os_AtomicCmpSwap(Atomic *ptr, int32_t oldVal, int32_t newVal)
{
    register int32_t status, val, ret;

    __asm __volatile__(
        "1:ldrex    %1, [%3]\n"
        "teq    %1, %4\n"
        "bne    2f\n"
        "strex    %2, %5, [%3]\n"
        "teq    %2, #0\n"
        "bne    1b\n"
        "mov    %0, #1\n"
        "b    3f\n"
        "2:    clrex\n"
        "mov    %0, #0\n"
        "3:    \n"
        "DMB"
        : "=&r"(ret), "=&r"(val), "=&r"(status)
        : "r"(ptr), "r"(oldVal), "r"(newVal)
        : "cc", "memory");

    return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* __ARCH_ARM_INCLUDE_ARMV8_RTFW_ATOMIC_H */
