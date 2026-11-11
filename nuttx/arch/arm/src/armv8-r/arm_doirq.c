/****************************************************************************
 * arch/arm/src/armv8-r/arm_doirq.c
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
#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <assert.h>

#include <nuttx/board.h>
#include <arch/board/board.h>
#include <sched/sched.h>

#include "arm_internal.h"
#include "arm_gic.h"

#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
#include "arm_mpu.h"
#endif


/****************************************************************************
 * Public Functions
 ****************************************************************************/

uint32_t *arm_doirq(int irq, uint32_t *regs)
{
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  uint8_t irq_nesting;
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
  arm_switch_to_kernel_mpu_set();
#endif
#endif
  struct tcb_s *tcb = this_task();

  board_autoled_on(LED_INIRQ);

#ifdef CONFIG_SUPPRESS_INTERRUPTS
  PANIC();
#else

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  irq_nesting = up_increase_irq_nesting();
  DEBUGASSERT(irq_nesting <= CONFIG_ARCH_IRQ_NESTING_LIMIT);
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
  if (irq_nesting == 1) {
    up_mpu_switch(tcb, NULL);
  }
#endif
#else
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
  up_mpu_switch(tcb, NULL);
#endif
  DEBUGASSERT(up_current_regs() == NULL);
#endif
  /* if irq == GIC_SMP_CPUSTART
   * We are initiating the multi-core jumping state to up_idle,
   * and we will use this_task(). Therefore, it cannot be overridden.
   */

#ifdef CONFIG_SMP
  if (irq != GIC_SMP_CPUSTART)
#endif
    {
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
      if (irq_nesting == 1) {
#endif
        tcb->xcp.regs = regs;
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
      }
#endif
    }

  /* Current regs non-zero indicates that we are processing an interrupt;
   * current_regs is also used to manage interrupt level context switches.
   */

  up_set_current_regs(regs);

  /* Deliver the IRQ */

  irq_dispatch(irq, regs);

  tcb = this_task();

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  if (irq_nesting == 1)
  {
#endif
    if (regs != tcb->xcp.regs)
      {
        /* Update scheduler parameters */

        nxsched_suspend_scheduler(g_running_tasks[this_cpu()]);
        nxsched_resume_scheduler(tcb);

        /* Record the new "running" task when context switch occurred.
        * g_running_tasks[] is only used by assertion logic for reporting
        * crashes.
        */

        g_running_tasks[this_cpu()] = tcb;
        regs = tcb->xcp.regs;
      }

#if !defined(CONFIG_RT_FRAMEWORK) || (CONFIG_RT_FRAMEWORK == 0)
    /* Set current_regs to NULL to indicate that we are no longer in an
    * interrupt handler.
    */

    up_set_current_regs(NULL);
#endif

    board_autoled_off(LED_INIRQ);

#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
    up_mpu_switch(NULL, tcb);
#endif
#if defined(CONFIG_RT_FRAMEWORK ) && (CONFIG_RT_FRAMEWORK == 1)
    up_set_contexthdl(tcb->contexthdl);
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
    arm_switch_to_task_mpu_set(tcb);
#endif
#endif
#if defined(CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK) && (CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK == 1)
    if (tcb->share_stack_info != NULL)
      {
        up_share_stack_copy(tcb);
      }
#endif
#ifdef CONFIG_STACK_OVERFLOW_CHECK
    if (0U != up_check_tcbstack_overflow(tcb)) {
      // TODO 待添加TRAP后处理
    }
#endif
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  }
  (void)up_decrease_irq_nesting();
#endif
#endif
  return regs;
}

/****************************************************************************
 * Name: up_irqinitialize
 *
 * Description:
 *   This function is called by up_initialize() during the bring-up of the
 *   system.  It is the responsibility of this function to but the interrupt
 *   subsystem into the working and ready state.
 *
 ****************************************************************************/

void up_irqinitialize(void)
{
  /* The following operations need to be atomic, but since this function is
   * called early in the initialization sequence, we expect to have exclusive
   * access to the GIC.
   */

  /* Initialize the Generic Interrupt Controller (GIC) for CPU0 */

  arm_gic_initialize();   /* Initialization common to all CPUs */

#ifndef CONFIG_SUPPRESS_INTERRUPTS

  /* And finally, enable interrupts */

  up_irq_enable();
#endif
}
