/****************************************************************************
 * arch/tricore/src/common/tricore_doirq.c
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
#include <assert.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <nuttx/addrenv.h>
#include <nuttx/board.h>

#include <arch/board/board.h>

#include <sched/sched.h>

#include "tricore_internal.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static inline_function  void tricore_doirq(void)
{
  tricore_set_isp();
#ifdef CONFIG_SUPPRESS_INTERRUPTS
  PANIC();
#else
  uint8_t irq_nesting;
  Ifx_CPU_ICR icr;
  uintptr_t *regs;
  struct tcb_s *tcb = this_task();

  __mtcr(TX3XX_CPU_PSW, __mfcr(TX3XX_CPU_PSW) | CPU_PSW_GW_MASK);
  __isync();

  icr.U = __mfcr(TC3XX_CPU_ICR);
  regs = (uintptr_t *)__mfcr(TC3XX_CPU_PCXI);

  board_autoled_on(LED_INIRQ);

  /* Interrupt nesting level non-zero indicates that we are processing an interrupt;
   * irq_nesting is also used to manage interrupt level context switches.
   */

  irq_nesting = up_increase_irq_nesting();

  DEBUGASSERT(irq_nesting <= CONFIG_ARCH_IRQ_NESTING_LIMIT);

  up_set_current_regs(regs);

  /* Deliver the IRQ */

  if (irq_nesting == 1) {
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
    up_mpu_switch(tcb, NULL);
#endif
    tcb->xcp.regs = regs;
  }

  irq_dispatch(icr.B.CCPN, regs);

  tcb = this_task();

  /* Check for a context switch.  If a context switch occurred, then
   * g_current_regs will have a different value than it did on entry.  If an
   * interrupt level context switch has occurred, then restore the floating
   * point state and the establish the correct address environment before
   * returning from the interrupt.
   */

  if (irq_nesting == 1)
    {
      if (regs != tcb->xcp.regs)
        {
          /* Update scheduler parameters */

          nxsched_suspend_scheduler(g_running_tasks[this_cpu()]);
          nxsched_resume_scheduler(tcb);

  #ifdef CONFIG_ARCH_ADDRENV
          /* Make sure that the address environment for the previously
          * running task is closed down gracefully (data caches dump,
          * MMU flushed) and set up the address environment for the new
          * thread at the head of the ready-to-run list.
          */

          addrenv_switch(NULL);
  #endif

          /* Record the new "running" task when context switch occurred.
          * g_running_tasks[] is only used by assertion logic for reporting
          * crashes.
          */

          g_running_tasks[this_cpu()] = this_task();

          __mtcr(TC3XX_CPU_PCXI, (uintptr_t)tcb->xcp.regs);
          __isync();
        }
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
        up_mpu_switch(NULL, tcb);
#endif
#if defined(CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK) && (CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK == 1)
        if (tcb->share_stack_info != NULL)
          {
            up_share_stack_copy(tcb);
          }
#endif
        up_set_contexthdl(tcb->contexthdl);
#ifdef CONFIG_STACK_OVERFLOW_CHECK
        if (0U != up_check_tcbstack_overflow(tcb)) {
          // TODO 待添加TRAP后处理
        }
#endif
    }

  (void)up_decrease_irq_nesting();

  board_autoled_off(LED_INIRQ);
#endif
}

#define DEFINE_IFX_INTERRUPT_INTERNAL(core) \
IFX_INTERRUPT_INTERNAL(tricore_doirq##core, core, 255) { tricore_doirq(); }

DEFINE_IFX_INTERRUPT_INTERNAL(0)
DEFINE_IFX_INTERRUPT_INTERNAL(1)
DEFINE_IFX_INTERRUPT_INTERNAL(2)
DEFINE_IFX_INTERRUPT_INTERNAL(3)
DEFINE_IFX_INTERRUPT_INTERNAL(4)
DEFINE_IFX_INTERRUPT_INTERNAL(5)
