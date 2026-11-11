/****************************************************************************
 * sched/irq/irq_timeout.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/clock.h>

#include <assert.h>
#include <debug.h>

#include <nuttx/irq.h>

#include "irq/irq.h"
#include "syslog.h"

#ifdef CONFIG_SCHED_IRQMONITOR

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

 #ifdef CONFIG_ARCH_MINIMAL_VECTORTABLE
 #  define NUSER_IRQS CONFIG_ARCH_NUSER_INTERRUPTS
 #else
 #  define NUSER_IRQS NR_IRQS
 #endif

/****************************************************************************
* Public Functions
****************************************************************************/

int irq_attach_timeout_hook(int irq, isr_timeout_hook_t handler, FAR struct timespec ts, FAR void *arg)
{
  unsigned int ndx = IRQ_TO_NDX(irq);
  if (ndx < 0 || ndx >= NUSER_IRQS)
  {
    return -EINVAL;
  }
  irqstate_t flags;
  flags = spin_lock_irqsave(NULL);

  g_irqvector[ndx].timeout_handler = handler;
  g_irqvector[ndx].timeout_hook_arg = arg;
  g_irqvector[ndx].timeout_ts = ts;

  spin_unlock_irqrestore(NULL, flags);
  return OK;
}

#endif /* CONFIG_SCHED_IRQMONITOR */
