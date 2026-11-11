/****************************************************************************
 * arch/tricore/src/common/tricore_svcall.c
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
#include <sched/sched.h>
#include <nuttx/sched.h>

#include "tricore_internal.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static inline_function void tricore_syscall_enter(struct tcb_s *tcb, uintptr_t *regs)
{

  uint32_t cmd = regs[REG_D8];

  __mtcr(TX3XX_CPU_PSW, __mfcr(TX3XX_CPU_PSW) | CPU_PSW_GW_MASK);
  __isync();

  if((cmd == SYS_restore_context) || (cmd == SYS_switch_context)){
    tcb->xcp.regs = regs;
  }
}

#if defined(CONFIG_RT_FRAMEWORK_HOOK) && (CONFIG_RT_FRAMEWORK_HOOK == 1)
static inline_function uint32_t * tricore_rtfw_hook_handler( uint32_t param0, uint32_t param1,
                                                          uint32_t param2, uint32_t *regs)
{
  uint32_t hook_action = param0;
  struct tcb_s *rtcb = NULL;
  struct tcb_s *tcb = NULL;
  uint32_t * new_regs = NULL;
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION == 1)
  uintptr_t *cur_context, *next_context;
  struct mpu_access_right_s *cur_access_right, *next_access_right;
#endif

  switch (hook_action) {
    case HOOK_CALL_FROM_TASK:
    case HOOK_CALL_FROM_HOOK:
      {
        /* Hook and task both have a tcb structure component
        * so treat them the same way
        */

        rtcb = (struct tcb_s *)param1;
        tcb = (struct tcb_s *)param2;
        rtcb->xcp.regs = (uintptr_t *)regs[REG_UPCXI];
        new_regs = tcb->xcp.regs;
      }
      break;
    case HOOK_CALL_FROM_IRQ:
      {
        /* Irq does not have a tcb structure component
        * so treat it separately
        */

        tcb = (struct tcb_s *)param2;
        *(uint32_t **)param1 = (uintptr_t *)regs[REG_UPCXI];
        new_regs = tcb->xcp.regs;
      }
      break;
    case HOOK_RETURN_TO_HOOK:
    case HOOK_RETURN_TO_TASK:
      {
        /* Hook and task both have a tcb structure component
        * so treat them the same way
        */

        rtcb = (struct tcb_s *)param1;
        rtcb->xcp.regs = (uintptr_t *)regs[REG_UPCXI];
        tcb = (struct tcb_s *)param2;
        new_regs = tcb->xcp.regs;
      }
      break;
    case HOOK_RETURN_TO_IRQ:
      {
        /* Irq does not have a tcb structure component
        * so treat it separately
        */

        rtcb = (struct tcb_s *)param1;
        rtcb->xcp.regs = (uintptr_t *)regs[REG_UPCXI];
        tcb = g_running_tasks[this_cpu()];
        new_regs = (uint32_t *)param2;
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION == 1)
        cur_context = rtcb->contexthdl;
        cur_access_right = (struct mpu_access_right_s *)(*(uintptr_t *)(*cur_context));
        next_context = (uintptr_t *)up_get_contexthdl();
        next_access_right = (struct mpu_access_right_s *)(*(uintptr_t *)(*next_context));
        up_mpu_switch_access_right(cur_access_right, next_access_right);
        up_mpu_enable_stack_region((uint32_t)(tcb->stack_alloc_ptr), (uint32_t)(tcb->stack_base_ptr) + tcb->adj_stack_size);
#endif
      }
      break;
    default:
      break;
  }

  if (hook_action != HOOK_RETURN_TO_IRQ) {
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION == 1)
    up_mpu_switch(rtcb, tcb);
#endif
    up_set_contexthdl(tcb->contexthdl);
#ifdef CONFIG_STACK_OVERFLOW_CHECK
    if (0U != up_check_tcbstack_overflow(tcb)) {
      // TODO 待添加TRAP后处理
    }
#endif
  }

  return new_regs;
}
#endif /* CONFIG_RT_FRAMEWORK_HOOK */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: tricore_svcall
 *
 * Description:
 *   This is SVCall exception handler that performs context switching
 *
 ****************************************************************************/

void tricore_svcall(volatile void *trap)
{
  uintptr_t *regs;
  uint32_t cmd;
  int cpu;
  struct tcb_s *tcb = this_task();
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION== 1)
  struct tcb_s *rtcb = NULL;
#endif

  regs = (uintptr_t *)__mfcr(TC3XX_CPU_PCXI);

  /* DSYNC instruction should be executed immediately prior to the MTCR */

  __dsync();

  regs = tricore_csa2addr((uintptr_t)regs);

  tricore_syscall_enter(tcb, regs);
  cmd = regs[REG_D8];

  /* Handle the SVCall according to the command in R0 */

  switch (cmd)
    {
      /* R0=SYS_restore_context:  This a restore context command:
       *
       *   void tricore_fullcontextrestore(uint32_t *restoreregs)
       *          noreturn_function;
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_restore_context
       *   R1 = restoreregs
       *
       * In this case, we simply need to set g_current_regs to restore
       * register area referenced in the saved R1. context == g_current_regs
       * is the normal exception return.  By setting g_current_regs =
       * context[R1], we force the return to the saved context referenced
       * in R1.
       */

      case SYS_restore_context:
        {
          tricore_reclaim_csa(regs[REG_UPCXI]);
          tcb->xcp.regs = (uintptr_t *)regs[REG_D9];
        }
        break;

      case SYS_switch_context:
        {
          *(uintptr_t **)regs[REG_D9] = (uintptr_t *)regs[REG_UPCXI];
          tcb->xcp.regs = (uintptr_t *)regs[REG_D10];
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && !defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION)
          rtcb = container_of(container_of(regs[REG_D9], struct xcptcontext, regs), struct tcb_s, xcp);
#endif
        }
        break;

#if defined(CONFIG_RT_FRAMEWORK_SYSCALL) && (CONFIG_RT_FRAMEWORK_SYSCALL == 1)
      case SYS_syscall_return:
        {
          rtfw_syscall_handler((void *)regs[REG_D9], regs);
          /* for rt framework syscall, no task switch is permitted, so just return */

          return;
        }
#endif

#if defined(CONFIG_RT_FRAMEWORK_HOOK) && (CONFIG_RT_FRAMEWORK_HOOK == 1)
      case SYS_switch_hook_context:
        {
          DEBUGASSERT(regs[REG_D10] != 0 && regs[REG_D11] != 0);
          regs[REG_UPCXI] = (uintptr_t)tricore_rtfw_hook_handler(regs[REG_D9],  regs[REG_D10], regs[REG_D11], regs);
          return;
        }
#endif

#if defined(CONFIG_RT_FRAMEWORK_ISOLATE) && (CONFIG_RT_FRAMEWORK_ISOLATE == 1)
      case SYS_ppc_switch:
        {
          rtfw_ppc_handler(regs[REG_D9], regs[REG_D10], (void *)regs[REG_D11], regs);
          return;
        }
#endif

      default:
        {
          svcerr("ERROR: Bad SYS call: %d\n", (int)regs[REG_D0]);
        }
        break;
    }

  if (regs != tcb->xcp.regs)
    {
      cpu = this_cpu();

      /* Update scheduler parameters */

      nxsched_suspend_scheduler(g_running_tasks[cpu]);
      nxsched_resume_scheduler(tcb);

      /* Record the new "running" task when context switch occurred.
       * g_running_tasks[] is only used by assertion logic for reporting
       * crashes.
       */

      tcb = this_task();

      g_running_tasks[cpu] = tcb;

      regs[REG_UPCXI] = (uintptr_t)tcb->xcp.regs;
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
      up_mpu_switch(rtcb, tcb);
#endif
#if defined(CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK) && (CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK == 1)
      if (tcb->share_stack_info != NULL)
        {
          up_share_stack_copy(tcb);
        }
#endif
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
      up_set_contexthdl(tcb->contexthdl);
#endif
#ifdef CONFIG_STACK_OVERFLOW_CHECK
      if (0U != up_check_tcbstack_overflow(tcb)) {
        // TODO 待添加TRAP后处理
      }
#endif
      __isync();
    }
}
