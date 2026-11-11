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

#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <nuttx/semaphore.h>
#include <nuttx/circbuf.h>
#include <nuttx/mutex.h>
#include <nuttx/signal.h>
#include <nuttx/irq.h>
#include <semaphore.h>

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "Os.h"

#ifndef portTICK_PERIOD_MS
#define portTICK_PERIOD_MS 1
#endif

#ifndef errQUEUE_EMPTY
#define errQUEUE_EMPTY ERROR
#endif

#define xTaskGetTickCountFromISR()            clock_systime_ticks()
#define xTaskGetTickCount()                   clock_systime_ticks()
#define vTaskDelay(t)                         nxsig_usleep(TICK2USEC(t))


#define taskCHECK_IF_IN_ISR()                 up_interrupt_context()
typedef long                                  BaseType_t;

typedef struct circ_queue_s
{
    FAR struct circbuf_s circ;
    sem_t txsem;
    sem_t rxsem;
    int size;
    bool alloc;
} circ_queue_t;

typedef circ_queue_t                          Queue_t;
typedef circ_queue_t*                         QueueHandle_t;
typedef pid_t                                 TaskHandle_t;
typedef clock_t                               TickType_t;
typedef sem_t                                 Semaphore_t;
typedef sem_t *                               SemaphoreHandle_t;
typedef SemaphoreHandle_t                     sys_sem_t;


#define pdFALSE                               ( false )
#define pdTRUE                                ( true )
#define pdPASS                                ( OK )
#define pdFAIL                                ( ERROR )
#define portMAX_DELAY                         (-1)

#if SYS_LIGHTWEIGHT_PROT
    typedef unsigned char sys_prot_t;
#endif

#define sys_sem_valid(sem)                    (((sem) != NULL) && (*(sem) != NULL))
#define sys_sem_set_invalid(sem)              (*(sem) = NULL)

typedef struct rmutex_s sys_mutex_t;
#define sys_mutex_valid(sem)                  (((mutex) != NULL) && (*(mutex) != NULL))
#define sys_mutex_set_invalid(mutex)          (*(mutex) = NULL)

#define MAX_QUEUES        		10
#define MAX_QUEUE_ENTRIES 		30

typedef struct {
      QueueHandle_t xQueue;
}MboxHandle_t;

typedef MboxHandle_t 	                        sys_mbox_t;

#define sys_mbox_valid(mbox)                  (((mbox) != NULL) && ((mbox)->xQueue != NULL))
#define sys_mbox_set_invalid(mbox)            ((mbox)->xQueue = NULL)

typedef TaskHandle_t                          sys_thread_t;

static inline int cq_sem_post(FAR sem_t *sem)
{
    int semcount;
    nxsem_get_value(sem, &semcount);
    if (semcount < 1) {
        return nxsem_post(sem);
    }
    return 0;
}

static inline circ_queue_t *xqueue_create_static(int length, int size, void *buffer, circ_queue_t *cq)
{
    circbuf_init(&cq->circ, buffer, length * size);
    nxsem_init(&cq->txsem, 0, 0);
    nxsem_init(&cq->rxsem, 0, 0);
    cq->size = size;
    return cq;
}

static inline int xqueue_send_from_isr(circ_queue_t *cq,
    const void *const pvItemToQueue, BaseType_t *prio)
{
    irqstate_t flags;

    flags = enter_critical_section();
    if (circbuf_space(&cq->circ) < cq->size) {
        leave_critical_section(flags);
        return ERROR;
    }

    circbuf_write(&cq->circ, pvItemToQueue, cq->size);
    cq_sem_post(&cq->rxsem);

    leave_critical_section(flags);
    return 0;
}

static inline int xqueue_send(circ_queue_t *cq,
    const void * const pvItemToQueue, BaseType_t *prio)
{
  irqstate_t flags;

  flags = enter_critical_section();

  while (circbuf_space(&cq->circ) < cq->size)
    {
      nxsem_wait(&cq->txsem);
    }

  circbuf_write(&cq->circ, pvItemToQueue, cq->size);

  cq_sem_post(&cq->rxsem);

  leave_critical_section(flags);

  return OK;
}

static inline int cq_remain_data_size(circ_queue_t *cq)
{
    int used_size = 0;

    irqstate_t flags;

    flags = enter_critical_section();
    used_size = circbuf_used(&cq->circ);
    leave_critical_section(flags);

    return used_size;
}

static inline int cq_receive(circ_queue_t *cq, void *const pvBuffer, clock_t ticks, bool peek)
{
    irqstate_t flags;
    int ret;

again:
    flags = enter_critical_section();

    if (circbuf_used(&cq->circ) >= cq->size) {
        if (!peek) {
            circbuf_read(&cq->circ, pvBuffer, cq->size);
        }
        else {
            circbuf_peek(&cq->circ, pvBuffer, cq->size);
        }
        cq_sem_post(&cq->txsem);

        leave_critical_section(flags);
        return OK;
    }

    if (ticks == 0) {
        leave_critical_section(flags);
        return ERROR;
    }
    else if (ticks == portMAX_DELAY) {
        ret = nxsem_wait(&cq->rxsem);
    }
    else {
        ret = nxsem_tickwait(&cq->rxsem, ticks);
    }

    leave_critical_section(flags);

    if (ret < 0) {
        return ERROR;
    }
    else {
        goto again;
    }

    return OK;
}

static inline int xqueue_receive(circ_queue_t *cq, void *const pvBuffer, clock_t ticks)
{
    return cq_receive(cq, pvBuffer, ticks, false);
}

static inline void vqueue_delete(circ_queue_t *cq)
{
  irqstate_t flags;

  flags = enter_critical_section();

  nxsem_reset(&cq->txsem, 0);
  nxsem_reset(&cq->rxsem, 0);

  if (cq->alloc) {
      kmm_free(cq);
  }

  leave_critical_section(flags);
}

 #if LWIP_NETCONN_SEM_PER_THREAD
 #define TCPIP_START_SEC_CODE
 #include "tcpip_memmap.h"
 FUNC_P2VAR(sys_sem_t, AUTOMATIC, TCPIP_CODE) sys_arch_thread_sem_get(void);
 FUNC(void, TCPIP_CODE) sys_arch_thread_sem_alloc(void);
 FUNC(void, TCPIP_CODE) sys_arch_thread_sem_free(void);
 #define TCPIP_STOP_SEC_CODE
 #include "tcpip_memmap.h"

 #define LWIP_NETCONN_THREAD_SEM_GET()   sys_arch_thread_sem_get()
 #define LWIP_NETCONN_THREAD_SEM_ALLOC() sys_arch_thread_sem_alloc()
 #define LWIP_NETCONN_THREAD_SEM_FREE()  sys_arch_thread_sem_free()
 #endif /* LWIP_NETCONN_SEM_PER_THREAD */

 #define TCPIP_START_SEC_CODE
 #include "tcpip_memmap.h"
 FUNC(void, TCPIP_CODE) sys_arch_msleep(u32_t delay_ms);
 #define TCPIP_STOP_SEC_CODE
 #include "tcpip_memmap.h"
 #define sys_msleep(ms) sys_arch_msleep(ms)

 #endif /* __ARCH_SYS_ARCH_H__ */

































