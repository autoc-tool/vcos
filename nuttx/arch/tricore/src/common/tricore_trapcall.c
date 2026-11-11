/****************************************************************************
 * arch/tricore/src/common/tricore_trapcall.c
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

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <debug.h>
#include <syscall.h>

#include <arch/irq.h>
#if (defined(CONFIG_ARCH_HAVE_TRAP_HANDLER) && (CONFIG_ARCH_HAVE_TRAP_HANDLER == 1))
#include <arch/trap.h>
#endif /* CONFIG_ARCH_HAVE_TRAP_HANDLER */
#include <sched/sched.h>
#include <nuttx/sched.h>

#if (defined(CONFIG_ARCH_HAVE_TRAP_HANDLER) && (CONFIG_ARCH_HAVE_TRAP_HANDLER == 1))
#include "tricore_internal.h"
#endif /* CONFIG_ARCH_HAVE_TRAP_HANDLER */

#include "IfxCpu_Trap.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: tricore_trapcall
 *
 * Description:
 *   This is Trap exception handler
 *
 ****************************************************************************/

void tricore_trapcall(volatile void *trap)
{
  uintptr_t *regs;
  uintptr_t isp = __mfcr(CPU_ISP);

#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
  __asm volatile("mov.a  a10, %0" ::"d"(isp) : "a10");
#else
  __setareg(a10, isp);
#endif

  regs = tricore_csa2addr(__mfcr(TC3XX_CPU_PCXI));

  __mtcr(TX3XX_CPU_PSW, __mfcr(TX3XX_CPU_PSW) | CPU_PSW_GW_MASK);
  __isync();

  up_set_current_regs(regs);

  up_irq_save();
#if (defined(CONFIG_ARCH_HAVE_TRAP_HANDLER) && (CONFIG_ARCH_HAVE_TRAP_HANDLER == 1))
  tricore_exception_handler((const tricore_cpu_trap_t*)trap);
#endif /* CONFIG_ARCH_HAVE_TRAP_HANDLER */
  PANIC_WITH_REGS("Trap", CURRENT_REGS);
}
