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

/* lwIP includes. */
#include <arch/cc.h>
#include <arch/sys_arch.h>

#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/sio.h"

#if SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
#define ETH_START_SEC_PRIVATE_BSS
#include "eth_memmap.h"
static VAR(SemaphoreHandle_t, ETH_PRIVATE_BSS) sys_arch_protect_mutex;
#if (VCOS_OS_CONFIG_DYNAMIC_ALLOCATION == 0)
static VAR(Semaphore_t, ETH_PRIVATE_BSS) sys_arch_mutex;
#endif
#define ETH_STOP_SEC_PRIVATE_BSS
#include "eth_memmap.h"
#endif
#if SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_SANITY_CHECK
#define ETH_START_SEC_PRIVATE_BSS
#include "eth_memmap.h"
static VAR(sys_prot_t, ETH_PRIVATE_BSS) sys_arch_protect_nesting;
#define ETH_STOP_SEC_PRIVATE_BSS
#include "eth_memmap.h"
#endif

#define ETH_START_SEC_PRIVATE_BSS
#include "eth_memmap.h"
uint8_t QueueStore0[TCPIP_MBOX_SIZE * sizeof(void *)];
circ_queue_t cq;
Semaphore_t LwIpSem;
Semaphore_t pxMutexBuffer;
#define ETH_STOP_SEC_PRIVATE_BSS
#include "eth_memmap.h"
#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
/* Initialize this module (see description in sys.h) */
FUNC(void, TCPIP_CODE) lwip_sys_init(void)
{
#if SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX
	/* initialize sys_arch_protect global mutex */
#if (VCOS_OS_CONFIG_DYNAMIC_ALLOCATION == 1)
	sys_arch_protect_mutex = xSemaphoreCreateRecursiveMutex();
#else
	sys_arch_protect_mutex = xSemaphoreCreateRecursiveMutexStatic(&sys_arch_mutex);
#endif
	LWIP_ASSERT("failed to create sys_arch_protect mutex", sys_arch_protect_mutex != NULL);
#endif /* SYS_LIGHTWEIGHT_PROT && LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX */
}

/* Returns the current time in milliseconds */
FUNC(u32_t, TCPIP_CODE) sys_now(void)
{
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

FUNC(void, TCPIP_CODE) sys_arch_msleep(u32_t delay_ms)
{
	TickType_t delay_ticks = delay_ms / portTICK_PERIOD_MS;

	// vTaskDelay(delay_ticks);
}

FUNC(err_t, TCPIP_CODE) sys_mbox_new(sys_mbox_t *mbox, int size)
{
	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("parameter invalid (size > 0)", size > 0);

	mbox->xQueue = xqueue_create_static(((size > MAX_QUEUE_ENTRIES) ? MAX_QUEUE_ENTRIES : size), sizeof(void *),
					  QueueStore0, &cq);
	if (mbox->xQueue == NULL) {
		LWIP_ASSERT("OSQCreate", mbox->xQueue != NULL);
		return ERR_MEM;
	}

	return ERR_OK;
}

FUNC(void, TCPIP_CODE) sys_mbox_free(sys_mbox_t *mbox)
{
	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->xQueue != NULL", mbox->xQueue != NULL);

	vqueue_delete(mbox->xQueue);
	mbox->xQueue = NULL;
}

FUNC(void, TCPIP_CODE) sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
#ifndef LWIP_NOASSERT
	long ret;
#endif
	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->xQueue != NULL", mbox->xQueue != NULL);

	if (taskCHECK_IF_IN_ISR() != 0) {
#ifndef LWIP_NOASSERT
		ret = xqueue_send_from_isr(mbox->xQueue, &msg, NULL);
#else
		(void)xqueue_send_from_isr(mbox->xQueue, &msg, NULL);
#endif
	} else {
#ifndef LWIP_NOASSERT
		ret = xqueue_send(mbox->xQueue, &msg, NULL);
#else
		(void)xqueue_send(mbox->xQueue, &msg, NULL);
#endif
	}
#ifndef LWIP_NOASSERT
	LWIP_ASSERT("OSQPost", ret == pdTRUE);
#endif
}

uint32_t last_isr_post_time_ms = 0;
uint32_t last_pre_post_time_ms = 0;
uint32_t last_post_time_ms = 0;
FUNC(err_t, TCPIP_CODE) sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	BaseType_t ret;

	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->xQueue != NULL", mbox->xQueue != NULL);

	if (taskCHECK_IF_IN_ISR() != 0) {
		last_pre_post_time_ms = sys_now();
		ret = xqueue_send_from_isr(mbox->xQueue, &msg, NULL);
		last_post_time_ms = sys_now();
	} else {
		ret = xqueue_send(mbox->xQueue, &msg, 0);
	}
	if (ret == 0) {
		return ERR_OK;
	} else {
		LWIP_ASSERT("OSQTryPost", ret == errQUEUE_FULL);
		return ERR_MEM;
	}
	return 0;
}

FUNC(err_t, TCPIP_CODE) sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
	return sys_mbox_trypost(mbox, msg);
}

volatile int mbox_fecth_count = 0;
FUNC(u32_t, TCPIP_CODE) sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	BaseType_t ret = 0;
	void *msg_dummy;

	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->xQueue != NULL", mbox->xQueue != NULL);

	if (!msg) {
		msg = &msg_dummy;
	}

	if (!timeout) {
		/* wait infinite */
		if (taskCHECK_IF_IN_ISR() != 0) {
			LWIP_ASSERT("should not be called from ISR", taskCHECK_IF_IN_ISR() != 0);
		} else {
			/* printf("before recieve %u\n max delay tick per ms %u", sys_now(), portTICK_PERIOD_MS); */
			ret = xqueue_receive(mbox->xQueue, &(*msg), portMAX_DELAY);
			/* printf("after recieve %u max delay\n", sys_now()); */
		}
		LWIP_ASSERT("OSQFetch", ret == pdTRUE);
	} else {
		TickType_t timeout_ticks = timeout / portTICK_PERIOD_MS;

		if (taskCHECK_IF_IN_ISR() != 0) {
			LWIP_ASSERT("should not be called from ISR", taskCHECK_IF_IN_ISR() != 0);
		} else {
			ret = xqueue_receive(mbox->xQueue, &(*msg), timeout_ticks);
			/* printf("after recieve %u timeout_ticks %u\n", sys_now(), timeout_ticks); */
		}
		if (ret == errQUEUE_EMPTY) {
			/* timed out */
			*msg = NULL;
			return SYS_ARCH_TIMEOUT;
		}
		LWIP_ASSERT("OSQFetch", ret == pdTRUE);
	}
	mbox_fecth_count++;
	return 1;
}

FUNC(u32_t, TCPIP_CODE) sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	BaseType_t ret;
	void *msg_dummy;

	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("mbox->xQueue != NULL", mbox->xQueue != NULL);

	if (!msg) {
		msg = &msg_dummy;
	}
	if (taskCHECK_IF_IN_ISR() != 0) {
		/* TODO */
		/* ret = xQueueReceiveFromISR(mbox->xQueue, &(*msg), NULL); */
		ret = errQUEUE_EMPTY;
	} else {
		ret = xqueue_receive(mbox->xQueue, &(*msg), 0);
	}
	if (ret == errQUEUE_EMPTY) {
		*msg = NULL;
		return SYS_MBOX_EMPTY;
	}

	LWIP_ASSERT("OSQTryFetch", ret == pdTRUE);
	return 1;
}

FUNC(sys_thread_t, TCPIP_CODE) sys_thread_new(const char *name,
	lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
	return 0;
}

#if !LWIP_COMPAT_MUTEX
FUNC(err_t, TCPIP_CODE) sys_mutex_new(sys_mutex_t *mutex)
{
	LWIP_ASSERT("mutex != NULL", mutex != NULL);

	if (nxrmutex_init(mutex) != 0) {
		LWIP_ASSERT("OSMutexCreate ", mutex != NULL);
		/* printf("sys_mutex_new failed\n"); */
		return ERR_MEM;
	}

	return ERR_OK;
}

FUNC(void, TCPIP_CODE) sys_mutex_lock(sys_mutex_t *mutex)
{
#ifndef LWIP_NOASSERT
	long ret;
#endif
	LWIP_ASSERT("mutex != NULL", mutex != NULL);
	LWIP_ASSERT("*mutex != NULL", *mutex != NULL);
#if !LWIP_DISABLE_SYS_MUTEX
	if (taskCHECK_IF_IN_ISR() != 0) {
		return;
	} else {
#ifndef LWIP_NOASSERT
		ret = nxrmutex_lock(mutex);
#else
		(void)nxrmutex_lock(mutex);
#endif
	}
#ifndef LWIP_NOASSERT
	LWIP_ASSERT("OSMutexlock", ret == pdTRUE);
#endif
#endif
}

FUNC(void, TCPIP_CODE) sys_mutex_unlock(sys_mutex_t *mutex)
{
#ifndef LWIP_NOASSERT
	long ret;
#endif
	LWIP_ASSERT("mutex != NULL", mutex != NULL);
	LWIP_ASSERT("*mutex != NULL", *mutex != NULL);
#if !LWIP_DISABLE_SYS_MUTEX
	if (taskCHECK_IF_IN_ISR() != 0) {
		return;
	} else {
#ifndef LWIP_NOASSERT
		ret = nxrmutex_unlock(mutex);
#else
		(void)nxrmutex_unlock(mutex);
#endif
	}
#ifndef LWIP_NOASSERT
	LWIP_ASSERT("OSMutexUnlock", ret == pdTRUE);
#endif
#endif
}

FUNC(void, TCPIP_CODE) sys_mutex_free(sys_mutex_t *mutex)
{
	LWIP_ASSERT("mutex != NULL", mutex != NULL);
	LWIP_ASSERT("*mutex != NULL", *mutex != NULL);

	(void)nxrmutex_destroy(mutex);
	mutex = NULL;
}

#endif /* LWIP_COMPAT_MUTEX */

FUNC(sys_prot_t, TCPIP_CODE) sys_arch_protect(void)
{
	SuspendAllInterrupts();
	return 0;
}

FUNC(void, TCPIP_CODE) sys_arch_unprotect(sys_prot_t pval)
{
	LWIP_UNUSED_ARG(pval);
	ResumeAllInterrupts();
}

#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"
