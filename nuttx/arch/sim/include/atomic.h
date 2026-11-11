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
 * only indirectly through arch/chip/up_atomic.h
 */

#ifndef __CHIPS_SIM_INCLUDE_UP_ATOMIC_H
#define __CHIPS_SIM_INCLUDE_UP_ATOMIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <nuttx/compiler.h>
#include <stdatomic.h>

typedef atomic_int Atomic;

/*  \brief          Atomic addition
 *  \param[in/out]  ptr     The address of the data to perform atomic addition on
 *  \param[in]      addVal  The value to be added
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicAdd(Atomic *ptr, Atomic addVal)
{
	return atomic_fetch_add(ptr, addVal);
}

/*  \brief          Atomic subtraction
 *  \param[in/out]  ptr     The address of the data to perform atomic subtraction on
 *  \param[in]      subVal  The value to be subtracted
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicSub(Atomic *ptr, Atomic subVal)
{
	return atomic_fetch_sub(ptr, subVal);
}

/*  \brief          Atomic increment
 *  \param[in/out]  ptr     The address of the data to perform atomic increment on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicInc(Atomic *ptr)
{
	return atomic_fetch_add(ptr, 1);
}

/*  \brief          Atomic decrement
 *  \param[in/out]  ptr     The address of the data to perform atomic decrement on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicDec(Atomic *ptr)
{
	return atomic_fetch_sub(ptr, 1);
}

/*  \brief          Atomic OR
 *  \param[in/out]  ptr     The address of the data to perform atomic OR on
 *  \param[in]      orVal   The value to OR with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicOr(Atomic *ptr, Atomic orVal)
{
	return atomic_fetch_or(ptr, orVal);
}

/*  \brief          Atomic XOR
 *  \param[in/out]  ptr     The address of the data to perform atomic XOR on
 *  \param[in]      xorVal  The value to XOR with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicXor(Atomic *ptr, Atomic xorVal)
{
	return atomic_fetch_xor(ptr, xorVal);
}

/*  \brief          Atomic AND
 *  \param[in/out]  ptr     The address of the data to perform atomic AND on
 *  \param[in]      andVal  The value to AND with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicAnd(Atomic *ptr, Atomic andVal)
{
	return atomic_fetch_and(ptr, andVal);
}

/*  \brief          Atomic AND-NOT
 *  \param[in/out]  ptr      The address of the data to perform atomic AND-NOT on
 *  \param[in]      xandVal  The value to AND-NOT with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicXand(Atomic *ptr, Atomic xandVal)
{
	return *ptr;
}

/*  \brief          Atomic set value
 *  \param[in/out]  ptr     The address of the data to perform atomic set value on
 *  \param[in]      setVal  The value to set
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicSet(Atomic *ptr, Atomic setVal)
{
	return atomic_exchange(ptr, setVal);
}

/*  \brief          Atomic clear value
 *  \param[in/out]  ptr     The address of the data to perform atomic clear on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicClean(Atomic *ptr)
{
	return atomic_exchange(ptr, 0);
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

static inline Atomic Os_AtomicCmpSwap(Atomic *ptr, Atomic oldVal, Atomic newVal)
{
	/* call strong api to avoid spurious failure. */
	return atomic_compare_exchange_strong(ptr, &oldVal, newVal);
}

#ifdef __cplusplus
}
#endif

#endif /* __CHIPS_RT_FRAMEWORK_INCLUDE_UP_ATOMIC_H */
