/****************************************************************************
 * arch/arm/src/armv8-r/arm_syscall.c
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

#include <nuttx/arch.h>
#include <nuttx/sched.h>
#include <nuttx/nuttx.h>

#include "arm.h"
#include "arm_internal.h"
#include "sched/sched.h"
#include "signal/signal.h"

#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
#include "arm_mpu.h"
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: dump_syscall
 *
 * Description:
 *   Dump the syscall registers
 *
 ****************************************************************************/

static void dump_syscall(const char *tag, uint32_t cmd, const uint32_t *regs)
{
  /* The SVCall software interrupt is called with R0 = system call command
   * and R1..R7 =  variable number of arguments depending on the system call.
   */

#ifdef CONFIG_LIB_SYSCALL
  if (cmd >= CONFIG_SYS_RESERVED)
    {
      svcinfo("SYSCALL %s: regs: %p cmd: %" PRId32 " name: %s\n", tag,
              regs, cmd, g_funcnames[cmd - CONFIG_SYS_RESERVED]);
    }
  else
#endif
    {
      svcinfo("SYSCALL %s: regs: %p cmd: %" PRId32 "\n", tag, regs, cmd);
    }

  svcinfo("  R0: %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32
          " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "\n",
          regs[REG_R0],  regs[REG_R1],  regs[REG_R2],  regs[REG_R3],
          regs[REG_R4],  regs[REG_R5],  regs[REG_R6],  regs[REG_R7]);
  svcinfo("  R8: %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32
          " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "\n",
          regs[REG_R8],  regs[REG_R9],  regs[REG_R10], regs[REG_R11],
          regs[REG_R12], regs[REG_R13], regs[REG_R14], regs[REG_R15]);
  svcinfo("CPSR: %08" PRIx32 "\n", regs[REG_CPSR]);
}

/****************************************************************************
 * Name: dispatch_syscall
 *
 * Description:
 *   Call the stub function corresponding to the system call.  NOTE the non-
 *   standard parameter passing:
 *
 *     R0 = SYS_ call number
 *     R1 = parm0
 *     R2 = parm1
 *     R3 = parm2
 *     R4 = parm3
 *     R5 = parm4
 *     R6 = parm5
 *
 *   The values of R4-R5 may be preserved in the proxy called by the user
 *   code if they are used (but otherwise will not be).
 *
 *   WARNING: There are hard-coded values in this logic!
 *
 *   Register usage:
 *
 *     R0 - Need not be preserved.
 *     R1-R3 - Need to be preserved until the stub is called.  The values of
 *       R0 and R1 returned by the stub must be preserved.
 *     R4-R11 must be preserved to support the expectations of the user-space
 *       callee.  R4-R6 may have been preserved by the proxy, but don't know
 *       for sure.
 *     R12 - Need not be preserved
 *     R13 - (stack pointer)
 *     R14 - Need not be preserved
 *     R15 - (PC)
 *
 ****************************************************************************/

#ifdef CONFIG_LIB_SYSCALL
static void dispatch_syscall(void) naked_function;
static void dispatch_syscall(void)
{
  __asm__ __volatile__
  (
    " sub sp, sp, #16\n"           /* Create a stack frame to hold 3 parms + lr */
    " str r4, [sp, #0]\n"          /* Move parameter 4 (if any) into position */
    " str r5, [sp, #4]\n"          /* Move parameter 5 (if any) into position */
    " str r6, [sp, #8]\n"          /* Move parameter 6 (if any) into position */
    " str lr, [sp, #12]\n"         /* Save lr in the stack frame */
    " ldr ip, =g_stublookup\n"     /* R12=The base of the stub lookup table */
    " ldr ip, [ip, r0, lsl #2]\n"  /* R12=The address of the stub for this SYSCALL */
    " blx ip\n"                    /* Call the stub (modifies lr) */
    " ldr lr, [sp, #12]\n"         /* Restore lr */
    " add sp, sp, #16\n"           /* Destroy the stack frame */
    " mov r2, r0\n"                /* R2=Save return value in R2 */
    " mov r0, %0\n"                /* R0=SYS_syscall_return */
    " svc %1\n"::"i"(SYS_syscall_return),
                 "i"(SYS_syscall)  /* Return from the SYSCALL */
  );
}
#endif
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
static inline_function void arm_syscall_enter(struct tcb_s *tcb, uint32_t *regs)
{
  uint32_t cmd = regs[REG_R0];

  if (up_interrupt_context()) {
    /* Called by interrupt, no task context switch is permitted */

    DEBUGASSERT((cmd != SYS_restore_context) && (cmd != SYS_switch_context));
  } else {
    /* Called by task, this tcp is equal to running task
     * if not SYS_restore_context and not SYS_switch_context */

    if((cmd == SYS_restore_context) || (cmd == SYS_switch_context)){
      tcb->xcp.regs = regs;
    }
  }
}

#if defined(CONFIG_RT_FRAMEWORK_HOOK) && (CONFIG_RT_FRAMEWORK_HOOK == 1)
noinline_function uint32_t * arm_rtfw_hook_handler(uint32_t param0, uint32_t param1, uint32_t param2, uint32_t * regs)
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
        rtcb->xcp.regs = regs;
        new_regs = tcb->xcp.regs;
      }
      break;
    case HOOK_CALL_FROM_IRQ:
      {
        /* Irq does not have a tcb structure component
        * so treat it separately
        */

        tcb = (struct tcb_s *)param2;
        *(uint32_t **)param1 = regs;
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
        rtcb->xcp.regs = regs;
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
        rtcb->xcp.regs = regs;
        new_regs = (uint32_t *)param2;
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION == 1)
        cur_context = rtcb->contexthdl;
        cur_access_right = (struct mpu_access_right_s *)(*(uintptr_t *)(*cur_context));
        next_context = (uintptr_t *)up_get_contexthdl();
        next_access_right = (struct mpu_access_right_s *)(*(uintptr_t *)(*next_context));
        up_mpu_switch_access_right(cur_access_right, next_access_right);
        up_mpu_enable_set(next_access_right->set_num);
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
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
    arm_switch_to_task_mpu_set(tcb);
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
  }
  return new_regs;
}
#endif
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: arm_syscall
 *
 * Description:
 *   SVC interrupts will vector here with insn=the SVC instruction and
 *   xcp=the interrupt context
 *
 *   The handler may get the SVC number be de-referencing the return
 *   address saved in the xcp and decoding the SVC instruction
 *
 ****************************************************************************/

uint32_t *arm_syscall(uint32_t *regs)
{
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
  arm_switch_to_kernel_mpu_set();
#endif
  struct tcb_s *tcb = this_task();
  uint32_t cmd;
  int cpu;
#ifdef CONFIG_BUILD_PROTECTED
  uint32_t cpsr;
#endif
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION== 1)
  struct tcb_s *rtcb = NULL;
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
  uintptr_t *cur_context = NULL;
  struct mpu_access_right_s *thread_access_right;
#endif
#endif

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  arm_syscall_enter(tcb, regs);
#else
  /* Nested interrupts are not supported */

  DEBUGASSERT(up_current_regs() == NULL);

  tcb->xcp.regs = regs;
#endif
  /* Current regs non-zero indicates that we are processing an interrupt;
   * current_regs is also used to manage interrupt level context switches.
   */

  up_set_current_regs(regs);

  /* The SYSCALL command is in R0 on entry.  Parameters follow in R1..R7 */

  cmd = regs[REG_R0];

  /* The SVCall software interrupt is called with R0 = system call command
   * and R1..R7 =  variable number of arguments depending on the system call.
   */

  dump_syscall("Entry", cmd, regs);

  /* Handle the SVCall according to the command in R0 */

  switch (cmd)
    {
      /* R0=SYS_syscall_return:  This a SYSCALL return command:
       *
       *   void arm_syscall_return(void);
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_syscall_return
       *
       * We need to restore the saved return address and return in
       * unprivileged thread mode.
       */

#ifdef CONFIG_LIB_SYSCALL
      case SYS_syscall_return:
        {
          struct tcb_s *rtcb = this_task();
          int index = (int)rtcb->xcp.nsyscalls - 1;

          /* Make sure that there is a saved SYSCALL return address. */

          DEBUGASSERT(index >= 0);

          /* Setup to return to the saved SYSCALL return address in
           * the original mode.
           */

          regs[REG_PC]        = rtcb->xcp.syscall[index].sysreturn;
#ifdef CONFIG_BUILD_PROTECTED
          regs[REG_CPSR]      = rtcb->xcp.syscall[index].cpsr;
#endif
          /* The return value must be in R0-R1.  dispatch_syscall()
           * temporarily moved the value for R0 into R2.
           */

          regs[REG_R0]         = regs[REG_R2];

#ifdef CONFIG_ARCH_KERNEL_STACK
          /* If this is the outermost SYSCALL and if there is a saved user
           * stack pointer, then restore the user stack pointer on this
           * final return to user code.
           */

          if (index == 0 && rtcb->xcp.ustkptr != NULL)
            {
              regs[REG_SP]      = (uint32_t)rtcb->xcp.ustkptr;
              rtcb->xcp.ustkptr = NULL;
            }
#endif

          /* Save the new SYSCALL nesting level */

          rtcb->xcp.nsyscalls   = index;

          /* Handle any signal actions that were deferred while processing
           * the system call.
           */

          rtcb->flags          &= ~TCB_FLAG_SYSCALL;
          nxsig_unmask_pendingsignal();
        }
        break;
#endif

      /* R0=SYS_restore_context:  Restore task context
       *
       * void arm_fullcontextrestore(uint32_t *restoreregs)
       *   noreturn_function;
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_restore_context
       *   R1 = restoreregs
       */

      case SYS_restore_context:
        {
          /* Replace 'regs' with the pointer to the register set in
           * regs[REG_R1].  On return from the system call, that register
           * set will determine the restored context.
           */

          tcb->xcp.regs = (uint32_t *)regs[REG_R1];
          DEBUGASSERT(up_current_regs());
        }
        break;

      /* R0=SYS_switch_context:  This a switch context command:
       *
       *   void arm_switchcontext(uint32_t **saveregs,
       *                          uint32_t *restoreregs);
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_switch_context
       *   R1 = saveregs
       *   R2 = restoreregs
       *
       * In this case, we do both: We save the context registers to the save
       * register area reference by the saved contents of R1 and then set
       * regs to the save register area referenced by the saved
       * contents of R2.
       */

      case SYS_switch_context:
        {
          DEBUGASSERT(regs[REG_R1] != 0 && regs[REG_R2] != 0);
          *(uint32_t **)regs[REG_R1] = regs;
          tcb->xcp.regs = (uint32_t *)regs[REG_R2];
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && !defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION)
          rtcb = container_of(container_of(regs[REG_R1], struct xcptcontext, regs), struct tcb_s, xcp);
#endif
        }
        break;

#if defined(CONFIG_RT_FRAMEWORK_SYSCALL) && (CONFIG_RT_FRAMEWORK_SYSCALL == 1)
      case SYS_syscall_return:
        {
          tcb->flags |= TCB_FLAG_THREAD_IN_KERNEL_MPU_SET;
          rtfw_syscall_handler((void *)regs[REG_R1], regs);
          tcb->flags &= ~TCB_FLAG_THREAD_IN_KERNEL_MPU_SET;
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
          cur_context = (uintptr_t *)up_get_contexthdl();
          thread_access_right = (struct mpu_access_right_s *)(*(uintptr_t *)(*cur_context));
          up_mpu_enable_set(thread_access_right->set_num);
#endif
        }

        /* no task switch is permitted, just return regs */

        return regs;
#endif

#if defined(CONFIG_RT_FRAMEWORK_HOOK) && (CONFIG_RT_FRAMEWORK_HOOK == 1)
      case SYS_switch_hook_context:
        {
          DEBUGASSERT(regs[REG_R2] != 0 && regs[REG_R3] != 0);
          return arm_rtfw_hook_handler(regs[REG_R1],  regs[REG_R2], regs[REG_R3], regs);
        }
#endif

#if defined(CONFIG_RT_FRAMEWORK_ISOLATE) && (CONFIG_RT_FRAMEWORK_ISOLATE == 1)
      case SYS_ppc_switch:
        {
          rtfw_ppc_handler(regs[REG_R1], regs[REG_R2], (void *)regs[REG_R3], regs);
          return regs;
        }
#endif

      /* R0=SYS_task_start:  This a user task start
       *
       *   void up_task_start(main_t taskentry, int argc, char *argv[])
       *     noreturn_function;
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_task_start
       *   R1 = taskentry
       *   R2 = argc
       *   R3 = argv
       */

#ifdef CONFIG_BUILD_PROTECTED
      case SYS_task_start:
        {
          /* Set up to return to the user-space _start function in
           * unprivileged mode.  We need:
           *
           *   R0   = argc
           *   R1   = argv
           *   PC   = taskentry
           *   CSPR = user mode
           */

          regs[REG_PC]   = regs[REG_R1];
          regs[REG_R0]   = regs[REG_R2];
          regs[REG_R1]   = regs[REG_R3];

          cpsr           = regs[REG_CPSR] & ~PSR_MODE_MASK;
          regs[REG_CPSR] = cpsr | PSR_MODE_USR;
        }
        break;
#endif

      /* R0=SYS_pthread_start:  This a user pthread start
       *
       *   void up_pthread_start(pthread_startroutine_t entrypt,
       *                         pthread_addr_t arg) noreturn_function;
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_pthread_start
       *   R1 = entrypt
       *   R2 = arg
       */

#if !defined(CONFIG_BUILD_FLAT) && !defined(CONFIG_DISABLE_PTHREAD)
      case SYS_pthread_start:
        {
          /* Set up to enter the user-space pthread start-up function in
           * unprivileged mode. We need:
           *
           *   R0   = startup
           *   R1   = arg
           *   PC   = entrypt
           *   CSPR = user mode
           */

          regs[REG_PC]   = regs[REG_R1];
          regs[REG_R0]   = regs[REG_R2];
          regs[REG_R1]   = regs[REG_R3];

          cpsr           = regs[REG_CPSR] & ~PSR_MODE_MASK;
          regs[REG_CPSR] = cpsr | PSR_MODE_USR;
        }
        break;
#endif

#ifdef CONFIG_BUILD_PROTECTED
      /* R0=SYS_signal_handler:  This a user signal handler callback
       *
       * void signal_handler(_sa_sigaction_t sighand, int signo,
       *                     siginfo_t *info, void *ucontext);
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_signal_handler
       *   R1 = sighand
       *   R2 = signo
       *   R3 = info
       *   R4 = ucontext
       */

      case SYS_signal_handler:
        {
          struct tcb_s *rtcb = this_task();

          /* Remember the caller's return address */

          DEBUGASSERT(rtcb->xcp.sigreturn == 0);
          rtcb->xcp.sigreturn  = regs[REG_PC];

          /* Set up to return to the user-space trampoline function in
           * unprivileged mode.
           */

          regs[REG_PC]   = (uint32_t)ARCH_DATA_RESERVE->ar_sigtramp;
          cpsr           = regs[REG_CPSR] & ~PSR_MODE_MASK;
          regs[REG_CPSR] = cpsr | PSR_MODE_USR;

          /* Change the parameter ordering to match the expectation of struct
           * userpace_s signal_handler.
           */

          regs[REG_R0]   = regs[REG_R1]; /* sighand */
          regs[REG_R1]   = regs[REG_R2]; /* signal */
          regs[REG_R2]   = regs[REG_R3]; /* info */
          regs[REG_R3]   = regs[REG_R4]; /* ucontext */

#ifdef CONFIG_ARCH_KERNEL_STACK
          /* If we are signalling a user process, then we must be operating
           * on the kernel stack now.  We need to switch back to the user
           * stack before dispatching the signal handler to the user code.
           * The existence of an allocated kernel stack is sufficient
           * information to make this decision.
           */

          if (rtcb->xcp.kstack != NULL)
            {
              uint32_t usp;

              DEBUGASSERT(rtcb->xcp.kstkptr == NULL);

              /* Copy "info" into user stack */

              if (rtcb->sigdeliver)
                {
                  usp = rtcb->xcp.saved_regs[REG_SP];
                }
              else
                {
                  usp = rtcb->xcp.regs[REG_SP];
                }

              /* Create a frame for info and copy the kernel info */

              usp = usp - sizeof(siginfo_t);
              memcpy((void *)usp, (void *)regs[REG_R2], sizeof(siginfo_t));

              /* Now set the updated SP and user copy of "info" to R2 */

              rtcb->xcp.kstkptr = (uint32_t *)regs[REG_SP];
              regs[REG_SP]      = usp;
              regs[REG_R2]      = usp;
            }
#endif
        }
        break;
#endif

#ifdef CONFIG_BUILD_PROTECTED
      /* R0=SYS_signal_handler_return:  This a user signal handler callback
       *
       *   void signal_handler_return(void);
       *
       * At this point, the following values are saved in context:
       *
       *   R0 = SYS_signal_handler_return
       */

      case SYS_signal_handler_return:
        {
          struct tcb_s *rtcb = this_task();

          /* Set up to return to the kernel-mode signal dispatching logic. */

          DEBUGASSERT(rtcb->xcp.sigreturn != 0);

          regs[REG_PC]         = rtcb->xcp.sigreturn;
          cpsr                 = regs[REG_CPSR] & ~PSR_MODE_MASK;
          regs[REG_CPSR]       = cpsr | PSR_MODE_SYS;
          rtcb->xcp.sigreturn  = 0;

#ifdef CONFIG_ARCH_KERNEL_STACK
          /* We must enter here be using the user stack.  We need to switch
           * to back to the kernel user stack before returning to the kernel
           * mode signal trampoline.
           */

          if (rtcb->xcp.kstack != NULL)
            {
              DEBUGASSERT(rtcb->xcp.kstkptr != NULL);

              regs[REG_SP]      = (uint32_t)rtcb->xcp.kstkptr;
              rtcb->xcp.kstkptr = NULL;
            }
#endif
        }
        break;
#endif

      /* This is not an architecture-specific system call.  If NuttX is built
       * as a standalone kernel with a system call interface, then all of the
       * additional system calls must be handled as in the default case.
       */

      default:
        {
#ifdef CONFIG_LIB_SYSCALL
          struct tcb_s *rtcb = this_task();
          int index = rtcb->xcp.nsyscalls;

          /* Verify that the SYS call number is within range */

          DEBUGASSERT(cmd >= CONFIG_SYS_RESERVED && cmd < SYS_maxsyscall);

          /* Make sure that there is a no saved SYSCALL return address.  We
           * cannot yet handle nested system calls.
           */

          DEBUGASSERT(index < CONFIG_SYS_NNEST);

          /* Setup to return to dispatch_syscall in privileged mode. */

          rtcb->xcp.syscall[index].sysreturn = regs[REG_PC];
#ifdef CONFIG_BUILD_PROTECTED
          rtcb->xcp.syscall[index].cpsr      = regs[REG_CPSR];
#endif

          regs[REG_PC]   = (uint32_t)dispatch_syscall;
#ifdef CONFIG_BUILD_PROTECTED
          cpsr           = regs[REG_CPSR] & ~PSR_MODE_MASK;
          regs[REG_CPSR] = cpsr | PSR_MODE_SYS;
#endif
          /* Offset R0 to account for the reserved values */

          regs[REG_R0]  -= CONFIG_SYS_RESERVED;

          /* Indicate that we are in a syscall handler. */

          rtcb->flags   |= TCB_FLAG_SYSCALL;

#ifdef CONFIG_ARCH_KERNEL_STACK
          /* If this is the first SYSCALL and if there is an allocated
           * kernel stack, then switch to the kernel stack.
           */

          if (index == 0 && rtcb->xcp.kstack != NULL)
            {
              rtcb->xcp.ustkptr = (uint32_t *)regs[REG_SP];
              if (rtcb->xcp.kstkptr != NULL)
                {
                  regs[REG_SP]  = (uint32_t)rtcb->xcp.kstkptr;
                }
              else
                {
                  regs[REG_SP]  = (uint32_t)rtcb->xcp.kstack +
                                  ARCH_KERNEL_STACKSIZE;
                }
            }
#endif

          /* Save the new SYSCALL nesting level */

          rtcb->xcp.nsyscalls   = index + 1;
#else
          svcerr("ERROR: Bad SYS call: 0x%" PRIx32 "\n", regs[REG_R0]);
#endif
        }
        break;
    }

  if (regs != tcb->xcp.regs)
    {
      cpu = this_cpu();

      /* Update scheduler parameters */

      nxsched_suspend_scheduler(g_running_tasks[cpu]);
      nxsched_resume_scheduler(tcb);

      /* Record the new "running" task.  g_running_tasks[] is only used by
       * assertion logic for reporting crashes.
       */

      g_running_tasks[cpu] = tcb;

      /* Restore the cpu lock */

      restore_critical_section(tcb, cpu);
      regs = tcb->xcp.regs;
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
      up_mpu_switch(rtcb, tcb);
#endif
#if defined(CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK) && (CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK == 1)
      if (tcb->share_stack_info != NULL)
        {
          up_share_stack_copy(tcb);
        }
#endif
#if defined(CONFIG_RT_FRAMEWORK ) && (CONFIG_RT_FRAMEWORK == 1)
      up_set_contexthdl(tcb->contexthdl);
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
      arm_switch_to_task_mpu_set(tcb);
#endif
#endif
#ifdef CONFIG_STACK_OVERFLOW_CHECK
      if (0U != up_check_tcbstack_overflow(tcb)) {
        // TODO 待添加TRAP后处理
      }
#endif
    }

  /* Report what happened */

  dump_syscall("Exit", cmd, regs);

#if !defined(CONFIG_RT_FRAMEWORK) || (CONFIG_RT_FRAMEWORK == 0)
  /* Set current_regs to NULL to indicate that we are no longer in an
  * interrupt handler.
  */

  up_set_current_regs(NULL);
#endif


  /* Return the last value of curent_regs.  This supports context switches
   * on return from the exception.  That capability is only used with the
   * SYS_context_switch system call.
   */

  return regs;
}
