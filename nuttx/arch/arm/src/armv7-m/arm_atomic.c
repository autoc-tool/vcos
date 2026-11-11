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

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>

#include <stdint.h>
#include <nuttx/compiler.h>
#include <arch/armv7-m/atomic.h>

/*  \brief          Atomic addition
 *  \param[in/out]  ptr     The address of the data to perform atomic addition on
 *  \param[in]      addVal  The value to be added
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicAdd(Atomic *ptr, int32_t addVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr += addVal;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic subtraction
 *  \param[in/out]  ptr     The address of the data to perform atomic subtraction on
 *  \param[in]      subVal  The value to be subtracted
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicSub(Atomic *ptr, int32_t subVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr -= subVal;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic increment
 *  \param[in/out]  ptr     The address of the data to perform atomic increment on
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicInc(Atomic *ptr)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr += 1;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic decrement
 *  \param[in/out]  ptr     The address of the data to perform atomic decrement on
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicDec(Atomic *ptr)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr -= 1;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic OR
 *  \param[in/out]  ptr     The address of the data to perform atomic OR on
 *  \param[in]      orVal   The value to OR with
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicOr(Atomic *ptr, int32_t orVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr |= orVal;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic XOR
 *  \param[in/out]  ptr     The address of the data to perform atomic XOR on
 *  \param[in]      xorVal  The value to XOR with
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicXor(Atomic *ptr, int32_t xorVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr ^= xorVal;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic AND
 *  \param[in/out]  ptr     The address of the data to perform atomic AND on
 *  \param[in]      andVal  The value to AND with
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicAnd(Atomic *ptr, int32_t andVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr &= andVal;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic AND-NOT
 *  \param[in/out]  ptr      The address of the data to perform atomic AND-NOT on
 *  \param[in]      xandVal  The value to AND-NOT with
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicXand(Atomic *ptr, int32_t xandVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr &= (~xandVal); // Perform the bitwise AND-NOT operation
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic set value
 *  \param[in/out]  ptr     The address of the data to perform atomic set value on
 *  \param[in]      setVal  The value to set
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicSet(Atomic *ptr, int32_t setVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr = setVal;
    __asm __volatile__("cpsie if");
    return ret;
}

/*  \brief          Atomic clear value
 *  \param[in/out]  ptr     The address of the data to perform atomic clear on
 *  \return         The value before the atomic operation
 */

weak_function Atomic Os_AtomicClean(Atomic *ptr)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    ret = *ptr;
    *ptr = 0;
    __asm __volatile__("cpsie if");
    return ret;
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

weak_function int32_t Os_AtomicCmpSwap(Atomic *ptr, int32_t oldVal, int32_t newVal)
{
    Atomic ret;
    __asm __volatile__("cpsid if");
    if (*ptr == oldVal)
    {
        *ptr = newVal;
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    __asm __volatile__("cpsie if");
    return ret;
}
