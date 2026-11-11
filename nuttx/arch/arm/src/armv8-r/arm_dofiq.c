/****************************************************************************
 * arch/arm/src/armv8-r/arm_dofiq.c
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
#include <nuttx/board.h>
#include <arch/board/board.h>
#include <sched/sched.h>

#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
#include "arm_mpu.h"
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_dofiq
 *
 * Description:
 *   Receives the decoded GIC interrupt information and dispatches control
 *   to the attached fiq handler. It is not allowed to call OS functions
 *   within a FIQ handler.
 *
 ****************************************************************************/

uint32_t *arm_dofiq(int fiq, uint32_t *regs)
{
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
  arm_switch_to_kernel_mpu_set();
#endif
  struct tcb_s *tcb = this_task();
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
  up_mpu_switch(tcb, NULL);
#endif
  board_autoled_on(LED_INIRQ);

#ifdef CONFIG_SUPPRESS_INTERRUPTS
  PANIC();
#else
  irq_dispatch(fiq, regs);
#endif
  tcb = this_task();
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
  board_autoled_off(LED_INIRQ);
  return regs;
}
