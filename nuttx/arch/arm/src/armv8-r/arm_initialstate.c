/****************************************************************************
 * arch/arm/src/armv8-r/arm_initialstate.c
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

#include <nuttx/arch.h>

#include "arm.h"
#include "arm_internal.h"

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
#ifdef CONFG_SMP
uintptr_t *g_interrupt_context[CONFIG_SMP_NCPUS][CONFIG_ARCH_IRQ_NESTING_LIMIT + 1];
#else
#undef g_interrupt_context
__percpu_bss uintptr_t *g_interrupt_context[CONFIG_ARCH_IRQ_NESTING_LIMIT + 1];
#define g_interrupt_context this_cpu_var(g_interrupt_context)
#endif
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_initial_state
 *
 * Description:
 *   A new thread is being started and a new TCB has been created. This
 *   function is called to initialize the processor specific portions of
 *   the new TCB.
 *
 *   This function must setup the initial architecture registers and/or
 *   stack so that execution will begin at tcb->start on the next context
 *   switch.
 *
 ****************************************************************************/

void up_initial_state(struct tcb_s *tcb)
{
  struct xcptcontext *xcp = &tcb->xcp;
  uint32_t cpsr;

  /* Initialize the initial exception register context structure */

  memset(xcp, 0, sizeof(struct xcptcontext));

  /* Initialize the idle thread stack */

  if (tcb->pid == IDLE_PROCESS_ID)
    {
#ifdef CONFIG_BMP
      if (up_cpu_index() > 0)
        {
          up_cpu_idlestack(up_cpu_index(), tcb,
                           CONFIG_IDLETHREAD_STACKSIZE);
        }
      else
#endif
        {
          tcb->stack_alloc_ptr = (void *)(g_idle_topstack -
                                          CONFIG_IDLETHREAD_STACKSIZE);
          tcb->stack_base_ptr  = tcb->stack_alloc_ptr;
          tcb->adj_stack_size  = CONFIG_IDLETHREAD_STACKSIZE;
        }

#ifdef CONFIG_STACK_COLORATION
      /* If stack debug is enabled, then fill the stack with a
       * recognizable value that we can use later to test for high
       * water marks.
       */

      arm_stack_color(tcb->stack_alloc_ptr, 0);
#endif /* CONFIG_STACK_COLORATION */

      return;
    }

  /* Initialize the context registers to stack top */

  xcp->regs = (void *)((uint32_t)tcb->stack_base_ptr +
                                 tcb->adj_stack_size -
                                 XCPTCONTEXT_SIZE);

  /* Initialize the xcp registers */

  memset(xcp->regs, 0, XCPTCONTEXT_SIZE);

  /* Save the initial stack pointer */

  xcp->regs[REG_SP] = (uint32_t)tcb->stack_base_ptr +
                                tcb->adj_stack_size;

  /* Save the task entry point */

  xcp->regs[REG_PC] = (uint32_t)tcb->start;

  /* If this task is running PIC, then set the PIC base register to the
   * address of the allocated D-Space region.
   */

#ifdef CONFIG_PIC
  if (tcb->dspace != NULL)
    {
      /* Set the PIC base register (probably R10) to the address of the
       * alloacated D-Space region.
       */

      xcp->regs[REG_PIC] = (uint32_t)tcb->dspace->region;
    }
#endif

  /* Set supervisor-mode and disable FIQs, regardless of how NuttX is
   * configured and of what kind of thread is being started.  That is
   * because all threads, even user-mode threads will start in kernel
   * trampoline at nxtask_start() or pthread_start().  The thread's
   * privileges will be dropped before transitioning to user code.
   */

  cpsr = PSR_MODE_SYS;

  /* Enable or disable interrupts, based on user configuration */

#ifdef CONFIG_SUPPRESS_INTERRUPTS
  /* Disable interrupts (both IRQs and FIQs) */

  cpsr |= (PSR_I_BIT | PSR_F_BIT);

#else /* CONFIG_SUPPRESS_INTERRUPTS */
  /* Leave IRQs enabled (Also FIQs if CONFIG_ARMV8R_DECODEFIQ is selected) */

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  /* In RT framework, irq state is configured, so set it here */

  if (tcb->contexthdl != NULL){
    if ((tcb->flags & TCB_FLAG_THREAD_INITIAL_STATE_MASK) == TCB_FLAG_THREAD_DISABLE_IRQ) {
          cpsr |= PSR_I_BIT;
      }
  }
#endif

#ifndef CONFIG_ARMV8R_DECODEFIQ

  cpsr |= PSR_F_BIT;

#endif /* !CONFIG_ARMV8R_DECODEFIQ */

#ifdef CONFIG_ARM_THUMB
  cpsr |= PSR_T_BIT;
#endif

#ifdef CONFIG_ENDIAN_BIG

  cpsr |= PSR_E_BIT;

#endif /* CONFIG_ENDIAN_BIG */
#endif /* CONFIG_SUPPRESS_INTERRUPTS */

  xcp->regs[REG_CPSR] = cpsr;

#if defined(CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK) && (CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK == 1)
  if (tcb->share_stack_info != NULL) {
    DEBUGASSERT(CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ > XCPTCONTEXT_SIZE);
    /* Copy thread local storage to shared stack */
    memcpy(tcb->share_stack_info->temp_stack->temp_stack_buffer,
      (void *)((uint32_t)tcb->stack_base_ptr + tcb->adj_stack_size -
      CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ),
      CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ);
  }
#endif
}
