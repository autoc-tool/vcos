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

#ifndef __ARCH_TRICORE_INCLUDE_TC3XX_RTFW_ATOMIC_H
#define __ARCH_TRICORE_INCLUDE_TC3XX_RTFW_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <nuttx/compiler.h>

#if defined(__GNUC__) && defined(__TRICORE__) && !defined(__HIGHTEC__)
#include "IfxCpu_IntrinsicsGcc.h"
#define atomic_cmp_swap __cmpAndSwap
#elif defined(__HIGHTEC__)
#include "IfxCpu_IntrinsicsGnuc.h"
#define atomic_cmp_swap __cmpAndSwap
#elif defined(__TASKING__)
#define atomic_cmp_swap __cmpswapw
#endif /*__TASKING__*/

typedef volatile aligned_data(4) int32_t Atomic;

/*  \brief          Atomic addition
 *  \param[in/out]  ptr     The address of the data to perform atomic addition on
 *  \param[in]      addVal  The value to be added
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicAdd(Atomic *ptr, int32_t addVal)
{
	register int32_t cmpVal;
	volatile int32_t preVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				    ((unsigned int)(preVal + addVal)),
				    ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic subtraction
 *  \param[in/out]  ptr     The address of the data to perform atomic subtraction on
 *  \param[in]      subVal  The value to be subtracted
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicSub(Atomic *ptr, int32_t subVal)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				    ((unsigned int)(preVal - subVal)),
				    ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic increment
 *  \param[in/out]  ptr     The address of the data to perform atomic increment on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicInc(Atomic *ptr)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				    ((unsigned int)(preVal + 1U)),
				    ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic decrement
 *  \param[in/out]  ptr     The address of the data to perform atomic decrement on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicDec(Atomic *ptr)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				    ((unsigned int)(preVal - 1U)),
				    ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic OR
 *  \param[in/out]  ptr     The address of the data to perform atomic OR on
 *  \param[in]      orVal   The value to OR with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicOr(Atomic *ptr, int32_t orVal)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				      ((unsigned int)(preVal | orVal)),
				      ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic XOR
 *  \param[in/out]  ptr     The address of the data to perform atomic XOR on
 *  \param[in]      xorVal  The value to XOR with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicXor(Atomic *ptr, int32_t xorVal)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				    ((unsigned int)(preVal ^ xorVal)),
				    ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic AND
 *  \param[in/out]  ptr     The address of the data to perform atomic AND on
 *  \param[in]      andVal  The value to AND with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicAnd(Atomic *ptr, int32_t andVal)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				      ((unsigned int)(preVal & andVal)),
				      ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic AND-NOT
 *  \param[in/out]  ptr      The address of the data to perform atomic AND-NOT on
 *  \param[in]      xandVal  The value to AND-NOT with
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicXand(Atomic *ptr, int32_t xandVal)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				    ((unsigned int)(~(preVal & xandVal))),
				    ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic set value
 *  \param[in/out]  ptr     The address of the data to perform atomic set value on
 *  \param[in]      setVal  The value to set
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicSet(Atomic *ptr, int32_t setVal)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				      ((unsigned int)(setVal)),
				      ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
	return preVal;
}

/*  \brief          Atomic clear value
 *  \param[in/out]  ptr     The address of the data to perform atomic clear on
 *  \return         The value before the atomic operation
 */

static inline Atomic Os_AtomicClean(Atomic *ptr)
{
	volatile int32_t preVal;
	register int32_t cmpVal;

	do {
		preVal = *ptr;
		cmpVal = atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				      ((unsigned int)(0x00U)),
				      ((unsigned int)(preVal)));
	} while (cmpVal != preVal);
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
	if (oldVal == atomic_cmp_swap(((unsigned int *)(void *)(ptr)),
				 ((unsigned int)(newVal)),
				 ((unsigned int)(oldVal)))) {
		return 1;
	} else {
		return 0;
	}
}

#ifdef __cplusplus
}
#endif

#endif /* __ARCH_TRICORE_INCLUDE_TC3XX_RTFW_ATOMIC_H */
