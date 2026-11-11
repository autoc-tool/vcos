/****************************************************************************
 * arch/arm/include/armv8-r/irq.h
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

/* This file should never be included directly but, rather, only indirectly
 * through nuttx/irq.h
 */

#ifndef __ARCH_ARM_INCLUDE_ARMV8_R_IRQ_H
#define __ARCH_ARM_INCLUDE_ARMV8_R_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/irq.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#endif

#include <arch/armv8-r/cp15.h>

#ifndef __ASSEMBLY__
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
#include <arch/armv8-r/atomic.h>
#endif /* CONFIG_RT_FRAMEWORK */
#include <arch/armv8-r/mpu.h>
#endif

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

/* IRQ Stack Frame Format:
 *
 * Context is always saved/restored in the same way:
 *
 *   (1) stmia rx, {r0-r14}
 *   (2) then the PC and CPSR
 *
 * This results in the following set of indices that can be used to access
 * individual registers in the xcp.regs array:
 */

/* If the MCU supports a floating point unit, then it will be necessary
 * to save the state of the FPU status register and data registers on
 * each context switch.  These registers are not saved during interrupt
 * level processing, however. So, as a consequence, floating point
 * operations may NOT be performed in interrupt handlers.
 *
 * The FPU provides an extension register file containing 32 single-
 * precision registers. These can be viewed as:
 *
 * - Sixteen 64-bit double word registers, D0-D15
 * - Thirty-two 32-bit single-word registers, S0-S31
 *   S<2n> maps to the least significant half of D<n>
 *   S<2n+1> maps to the most significant half of D<n>.
 */

#ifdef CONFIG_ARCH_FPU
#  define REG_D0            (0)  /* D0 */
#  define REG_S0            (0)  /* S0 */
#  define REG_S1            (1)  /* S1 */
#  define REG_D1            (2)  /* D1 */
#  define REG_S2            (2)  /* S2 */
#  define REG_S3            (3)  /* S3 */
#  define REG_D2            (4)  /* D2 */
#  define REG_S4            (4)  /* S4 */
#  define REG_S5            (5)  /* S5 */
#  define REG_D3            (6)  /* D3 */
#  define REG_S6            (6)  /* S6 */
#  define REG_S7            (7)  /* S7 */
#  define REG_D4            (8)  /* D4 */
#  define REG_S8            (8)  /* S8 */
#  define REG_S9            (9)  /* S9 */
#  define REG_D5            (10) /* D5 */
#  define REG_S10           (10) /* S10 */
#  define REG_S11           (11) /* S11 */
#  define REG_D6            (12) /* D6 */
#  define REG_S12           (12) /* S12 */
#  define REG_S13           (13) /* S13 */
#  define REG_D7            (14) /* D7 */
#  define REG_S14           (14) /* S14 */
#  define REG_S15           (15) /* S15 */
#  define REG_D8            (16) /* D8 */
#  define REG_S16           (16) /* S16 */
#  define REG_S17           (17) /* S17 */
#  define REG_D9            (18) /* D9 */
#  define REG_S18           (18) /* S18 */
#  define REG_S19           (19) /* S19 */
#  define REG_D10           (20) /* D10 */
#  define REG_S20           (20) /* S20 */
#  define REG_S21           (21) /* S21 */
#  define REG_D11           (22) /* D11 */
#  define REG_S22           (22) /* S22 */
#  define REG_S23           (23) /* S23 */
#  define REG_D12           (24) /* D12 */
#  define REG_S24           (24) /* S24 */
#  define REG_S25           (25) /* S25 */
#  define REG_D13           (26) /* D13 */
#  define REG_S26           (26) /* S26 */
#  define REG_S27           (27) /* S27 */
#  define REG_D14           (28) /* D14 */
#  define REG_S28           (28) /* S28 */
#  define REG_S29           (29) /* S29 */
#  define REG_D15           (30) /* D15 */
#  define REG_S30           (30) /* S30 */
#  define REG_S31           (31) /* S31 */
#  ifdef CONFIG_ARM_HAVE_DPFPU32
#    define REG_D16         (32) /* D16 */
#    define REG_D17         (34) /* D17 */
#    define REG_D18         (36) /* D18 */
#    define REG_D19         (38) /* D19 */
#    define REG_D20         (40) /* D20 */
#    define REG_D21         (42) /* D21 */
#    define REG_D22         (44) /* D22 */
#    define REG_D23         (46) /* D23 */
#    define REG_D24         (48) /* D24 */
#    define REG_D25         (50) /* D25 */
#    define REG_D26         (52) /* D26 */
#    define REG_D27         (54) /* D27 */
#    define REG_D28         (56) /* D28 */
#    define REG_D29         (58) /* D29 */
#    define REG_D30         (60) /* D30 */
#    define REG_D31         (62) /* D31 */
#    define REG_FPSCR       (64) /* Floating point status and control */
#    define FPU_CONTEXT_REGS  (65)
#  else
#    define REG_FPSCR       (32) /* Floating point status and control */
#    define FPU_CONTEXT_REGS  (33)
#  endif
#else
#  define FPU_CONTEXT_REGS  (0)
#endif

#define REG_R13             (FPU_CONTEXT_REGS+0)
#define REG_R14             (FPU_CONTEXT_REGS+1)
#define REG_R0              (FPU_CONTEXT_REGS+2)
#define REG_R1              (FPU_CONTEXT_REGS+3)
#define REG_R2              (FPU_CONTEXT_REGS+4)
#define REG_R3              (FPU_CONTEXT_REGS+5)
#define REG_R4              (FPU_CONTEXT_REGS+6)
#define REG_R5              (FPU_CONTEXT_REGS+7)
#define REG_R6              (FPU_CONTEXT_REGS+8)
#define REG_R7              (FPU_CONTEXT_REGS+9)
#define REG_R8              (FPU_CONTEXT_REGS+10)
#define REG_R9              (FPU_CONTEXT_REGS+11)
#define REG_R10             (FPU_CONTEXT_REGS+12)
#define REG_R11             (FPU_CONTEXT_REGS+13)
#define REG_R12             (FPU_CONTEXT_REGS+14)
#define REG_R15             (FPU_CONTEXT_REGS+15)
#define REG_CPSR            (FPU_CONTEXT_REGS+16)

#define ARM_CONTEXT_REGS    (17)

/* The total number of registers saved by software */

#define XCPTCONTEXT_REGS    (FPU_CONTEXT_REGS + ARM_CONTEXT_REGS)
#define XCPTCONTEXT_SIZE    (4 * XCPTCONTEXT_REGS)

/* Friendly register names */

#define REG_A1              REG_R0
#define REG_A2              REG_R1
#define REG_A3              REG_R2
#define REG_A4              REG_R3
#define REG_V1              REG_R4
#define REG_V2              REG_R5
#define REG_V3              REG_R6
#define REG_V4              REG_R7
#define REG_V5              REG_R8
#define REG_V6              REG_R9
#define REG_V7              REG_R10
#define REG_SB              REG_R9
#define REG_SL              REG_R10
#ifdef CONFIG_ARM_THUMB
  #define REG_FP            REG_R7
#else
  #define REG_FP            REG_R11
#endif /* CONFIG_ARM_THUMB */
#define REG_IP              REG_R12
#define REG_SP              REG_R13
#define REG_LR              REG_R14
#define REG_PC              REG_R15

/* The PIC register is usually R10. It can be R9 is stack checking is enabled
 * or if the user changes it with -mpic-register on the GCC command line.
 */

#define REG_PIC             REG_R10

/* Multiprocessor Affinity Register (MPIDR): CRn=c0, opc1=0, CRm=c0, opc2=5 */

#define MPIDR_CPUID_SHIFT   (0)       /* Bits 0-1: CPU ID */
#define MPIDR_CPUID_MASK    (3 << MPIDR_CPUID_SHIFT)
#  define MPIDR_CPUID_CPU0  (0 << MPIDR_CPUID_SHIFT)
#  define MPIDR_CPUID_CPU1  (1 << MPIDR_CPUID_SHIFT)
#  define MPIDR_CPUID_CPU2  (2 << MPIDR_CPUID_SHIFT)
#  define MPIDR_CPUID_CPU3  (3 << MPIDR_CPUID_SHIFT)
                                      /* Bits 2-7: Reserved */
#define MPIDR_CLUSTID_SHIFT (8)       /* Bits 8-11: Cluster ID value */
#define MPIDR_CLUSTID_MASK  (15 << MPIDR_CLUSTID_SHIFT)
                                      /* Bits 12-29: Reserved */

#define MPIDR_TO_CPU(mpidr)  (((mpidr) & MPIDR_CPUID_MASK) >> MPIDR_CPUID_SHIFT)
#define MPIDR_TO_CLUSTER(mpidr)  (((mpidr) & MPIDR_CLUSTID_MASK) >> MPIDR_CLUSTID_SHIFT)

#define MPIDR_U             (1 << 30) /* Bit 30: Multiprocessing Extensions. */
#define ARM_MODE_MASK     (31UL)
#define ARM_MODE_USR      (16UL)
#define ARM_MODE_SYS      (31UL)
#define I_BIT_MASK        (1UL << 7U)

#define ICC_PMR_PRI_SHIFT (3UL)
#define ICC_PMR_PRI_MASK  (0x1f << ICC_PMR_PRI_SHIFT) /* Bits 3-7: PMR Priority */

#define ICC_RPR_PRI_SHIFT (3UL)
#define ICC_RPR_PRI_MASK  (0x1f << ICC_RPR_PRI_SHIFT) /* Bits 3-7: RPR Priority */

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
#ifdef CONFG_SMP
extern uintptr_t *g_interrupt_context[CONFIG_SMP_NCPUS][CONFIG_ARCH_IRQ_NESTING_LIMIT + 1];
#else
extern uintptr_t *g_interrupt_context[CONFIG_ARCH_IRQ_NESTING_LIMIT + 1];
#define g_interrupt_context this_cpu_var(g_interrupt_context)
#endif
#endif
/* This structure represents the return state from a system call */

#ifdef CONFIG_LIB_SYSCALL
struct xcpt_syscall_s
{
#ifdef CONFIG_BUILD_KERNEL
  uint32_t cpsr;        /* The CPSR value */
#endif
  uint32_t sysreturn;   /* The return PC */
};
#endif

/* This struct defines the way the registers are stored.  We need to save:
 *
 *  1  CPSR
 *  7  Static registers, v1-v7 (aka r4-r10)
 *  1  Frame pointer, fp (aka r11)
 *  1  Stack pointer, sp (aka r13)
 *  1  Return address, lr (aka r14)
 * ---
 * 11  (XCPTCONTEXT_USER_REG)
 *
 * On interrupts, we also need to save:
 *  4  Volatile registers, a1-a4 (aka r0-r3)
 *  1  Scratch Register, ip (aka r12)
 *---
 *  5  (XCPTCONTEXT_IRQ_REGS)
 *
 * For a total of 17 (XCPTCONTEXT_REGS)
 */

struct xcptcontext
{
  /* These are saved copies of the context used during
   * signal processing.
   */

  uint32_t *saved_regs;

#ifdef CONFIG_BUILD_KERNEL
  /* This is the saved address to use when returning from a user-space
   * signal handler.
   */

  uint32_t sigreturn;
#endif

  /* Register save area with XCPTCONTEXT_SIZE, only valid when:
   * 1.The task isn't running or
   * 2.The task is interrupted
   * otherwise task is running, and regs contain the stale value.
   */

  uint32_t *regs;

  /* Extra fault address register saved for common paging logic.  In the
   * case of the pre-fetch abort, this value is the same as regs[REG_R15];
   * For the case of the data abort, this value is the value of the fault
   * address register (FAR) at the time of data abort exception.
   */

#ifdef CONFIG_LEGACY_PAGING
  uintptr_t far;
#endif

#ifdef CONFIG_LIB_SYSCALL
  /* The following array holds the return address and the exc_return value
   * needed to return from each nested system call.
   */

  uint8_t nsyscalls;
  struct xcpt_syscall_s syscall[CONFIG_SYS_NNEST];
#endif

#ifdef CONFIG_ARCH_ADDRENV
#ifdef CONFIG_ARCH_STACK_DYNAMIC
  /* This array holds the physical address of the level 2 page table used
   * to map the thread's stack memory.  This array will be initially of
   * zeroed and would be back-up up with pages during page fault exception
   * handling to support dynamically sized stacks for each thread.
   */

  uintptr_t *ustack[ARCH_STACK_NSECTS];
#endif

#ifdef CONFIG_ARCH_KERNEL_STACK
  /* In this configuration, all syscalls execute from an internal kernel
   * stack.  Why?  Because when we instantiate and initialize the address
   * environment of the new user process, we will temporarily lose the
   * address environment of the old user process, including its stack
   * contents.  The kernel C logic will crash immediately with no valid
   * stack in place.
   */

  uint32_t *ustkptr;  /* Saved user stack pointer */
  uint32_t *kstack;   /* Allocate base of the (aligned) kernel stack */
  uint32_t *kstkptr;  /* Saved kernel stack pointer */
#endif
#endif
};

/****************************************************************************
 * Inline functions
 ****************************************************************************/

/* Name: up_irq_save, up_irq_restore, and friends.
 *
 * NOTE: This function should never be called from application code and,
 * as a general rule unless you really know what you are doing, this
 * function should not be called directly from operation system code either:
 * Typically, the wrapper functions, enter_critical_section() and
 * leave_critical section(), are probably what you really want.
 */

/* Return the current IRQ state */

static inline irqstate_t irqstate(void)
{
  unsigned int cpsr;

  __asm__ __volatile__
    (
      "\tmrs    %0, cpsr\n"
      : "=r" (cpsr)
      :
      : "memory"
    );

  return cpsr;
}

/* Disable IRQs and return the previous IRQ state */

noinstrument_function static inline irqstate_t up_irq_save(void)
{
  unsigned int cpsr;

  __asm__ __volatile__
    (
      "\tmrs    %0, cpsr\n"
      "\tcpsid  i\n"
#if defined(CONFIG_ARMV8R_DECODEFIQ)
      "\tcpsid  f\n"
#endif
      : "=r" (cpsr)
      :
      : "memory"
    );

  return cpsr;
}

/* Enable IRQs and return the previous IRQ state */

static inline irqstate_t up_irq_enable(void)
{
  unsigned int cpsr;

  __asm__ __volatile__
    (
      "\tmrs    %0, cpsr\n"
      "\tcpsie  i\n"
#if defined(CONFIG_ARMV8R_DECODEFIQ)
      "\tcpsie  f\n"
#endif
      : "=r" (cpsr)
      :
      : "memory"
    );

  return cpsr;
}

/* Disable IRQs and return the previous IRQ state */

static inline irqstate_t up_irq_disable(void)
{
  unsigned int cpsr;

  __asm__ __volatile__
    (
      "\tmrs    %0, cpsr\n"
      "\tcpsid  i\n"
      : "=r" (cpsr)
      :
      : "memory"
    );

  return cpsr;
}

/* Restore saved IRQ & FIQ state */

noinstrument_function static inline void up_irq_restore(irqstate_t flags)
{
  __asm__ __volatile__
    (
      "msr    cpsr_c, %0"
      :
      : "r" (flags)
      : "memory"
    );
}

/****************************************************************************
 * Name: up_cpu_index
 *
 * Description:
 *   Return the real core number regardless CONFIG_SMP setting
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_HAVE_MULTICPU
noinstrument_function
static inline_function int up_cpu_index(void)
{
  uint32_t mpidr;

  /* Read the Multiprocessor Affinity Register (MPIDR) */

  mpidr = CP15_GET(MPIDR);

#ifdef CONFIG_ARM_HAVE_MULTICLUSTER
  return MPIDR_TO_CPU(mpidr) + arm_cpu_index_prefix(MPIDR_TO_CLUSTER(mpidr));
#else
  return MPIDR_TO_CPU(mpidr);
#endif
}
#endif /* CONFIG_ARCH_HAVE_MULTICPU */

static inline_function uint32_t up_getsp(void)
{
  register uint32_t sp;

  __asm__ __volatile__
  (
    "mov %0, sp\n"
    : "=r" (sp)
  );

  return sp;
}

/****************************************************************************
 * Name:
 *   up_current_regs/up_set_current_regs
 *
 * Description:
 *   We use the following code to manipulate the TPIDRPRW register,
 *   which exists uniquely for each CPU and is primarily designed to store
 *   current thread information. Currently, we leverage it to store interrupt
 *   information, with plans to further optimize its use for storing both
 *   thread and interrupt information in the future.
 *
 ****************************************************************************/

noinstrument_function
static inline_function uint32_t *up_current_regs(void)
{
#if CONFIG_ARCH_IRQ_NESTING_LIMIT > 1
#ifdef CONFG_SMP
  return (uintptr_t *)g_interrupt_context[up_cpu_index()][CP15_GET(TPIDRURW)];
#else
  return (uintptr_t *)g_interrupt_context[CP15_GET(TPIDRURW)];
#endif
#else
  return (uint32_t *)CP15_GET(TPIDRPRW);
#endif
}

noinstrument_function
static inline_function void up_set_current_regs(uint32_t *regs)
{
#if CONFIG_ARCH_IRQ_NESTING_LIMIT > 1
#ifdef CONFG_SMP
  g_interrupt_context[up_cpu_index()][CP15_GET(TPIDRURW)] = regs;
#else
  g_interrupt_context[CP15_GET(TPIDRURW)] = regs;
#endif
#else
  CP15_SET(TPIDRPRW, regs);
#endif
}

noinstrument_function
static inline_function bool up_interrupt_context(void)
{
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  return CP15_GET(TPIDRURW) != 0;
#else
  return up_current_regs() != NULL;
#endif
}

static inline_function uint8_t up_get_irq_nesting(void)
{
  return CP15_GET(TPIDRURW);
}

static inline_function uint8_t up_increase_irq_nesting(void)
{
  uint32_t irq_nesting = CP15_GET(TPIDRURW);

  irq_nesting ++ ;
  CP15_SET(TPIDRURW, irq_nesting);

  return (uint8_t)irq_nesting;
}

static inline_function uint8_t up_decrease_irq_nesting(void)
{
  uint32_t irq_nesting = CP15_GET(TPIDRURW);

  irq_nesting -- ;
  CP15_SET(TPIDRURW, irq_nesting);

  return (uint8_t)irq_nesting;
}

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)

static inline_function irqlevel_t up_get_irq_mask_level(void)
{
  return (((CP15_GET(ICC_PMR)) & ICC_PMR_PRI_MASK) >> ICC_PMR_PRI_SHIFT);
}

static inline_function void up_set_irq_mask_level(irqlevel_t level)
{
  irqstate_t irq = up_irq_save();
  CP15_SET(ICC_PMR, level << ICC_PMR_PRI_SHIFT);
  up_irq_restore(irq);
}

static inline_function irqlevel_t up_get_irq_running_level(void)
{
  return (((CP15_GET(ICC_RPR)) & ICC_RPR_PRI_MASK) >> ICC_RPR_PRI_SHIFT);
}

static inline_function irqlevel_t up_irq_level_save(irqlevel_t level)
{
  irqlevel_t pre_level = up_get_irq_mask_level();

  if (level < pre_level) {
    up_set_irq_mask_level(level);
  }
  return pre_level;
}

static inline_function void up_irq_level_restore(irqlevel_t level)
{
  up_set_irq_mask_level(level);
}

static inline_function irqlevel_t up_kernel_int_save(void)
{
#if defined(CONFIG_RT_FRAMEWORK_INT_LEVEL) && (CONFIG_RT_FRAMEWORK_INT_LEVEL == 1)
  return up_irq_level_save(CONFIG_RT_FRAMEWORK_MAX_TP_INT_LEVEL);
#else
  return up_irq_save();
#endif
}

static inline_function void up_kernel_int_restore(irqlevel_t level)
{
#if defined(CONFIG_RT_FRAMEWORK_INT_LEVEL) && (CONFIG_RT_FRAMEWORK_INT_LEVEL == 1)
  return up_irq_level_restore(level);
#else
  return up_irq_restore(level);
#endif
}

noinstrument_function inline_function void up_set_contexthdl(void *const ctxhdl)
{
	CP15_SET(CONTEXTIDR, ctxhdl);
}

noinstrument_function inline_function void *up_get_contexthdl()
{
	return (void *)CP15_GET(CONTEXTIDR);
}

#if defined(CONFIG_RT_FRAMEWORK_SYSCALL) && (CONFIG_RT_FRAMEWORK_SYSCALL == 1)

bool up_is_privilege_mode(void);

/* called directly */

static inline_function void up_set_privilege_to_user(void)
{
	uintptr_t cpsr;

	__asm volatile("mrs %0, cpsr" : "=r" (cpsr));
	cpsr &= ~ARM_MODE_MASK;
	cpsr |= ARM_MODE_USR;
	__asm volatile("msr cpsr, %0" : : "r" (cpsr));
}

/*called by trap*/

static inline_function void up_syscall_set_enter_int_state(uint32_t *regs)
{
	if (0U == (regs[REG_CPSR] & I_BIT_MASK)) {
		up_irq_enable();
	}
}

static inline_function void up_syscall_set_exit_int_state(uint32_t *regs)
{
	if (0U == (regs[REG_CPSR] & I_BIT_MASK)) {
		up_irq_disable();
	}
}

static inline_function bool up_syscall_get_prev_privilege(uint32_t *regs)
{
	uintptr_t cpsr;

	cpsr = regs[REG_CPSR];

	return (ARM_MODE_USR == (cpsr & ARM_MODE_MASK)) ? FALSE : TRUE;
}

static inline_function void up_syscall_set_prev_privilege(uint32_t *regs, const bool is_privilege)
{
	uintptr_t cpsr;

	cpsr = regs[REG_CPSR];
	cpsr &= ~ARM_MODE_MASK;
	cpsr |= (true == is_privilege)? ARM_MODE_SYS : ARM_MODE_USR;
	regs[REG_CPSR] = cpsr;
}

static inline_function void up_syscall_set_prev_int_state(uint32_t *regs)
{
	uintptr_t cur_cpsr;
	uintptr_t user_cpsr;

	__asm volatile("mrs %0, cpsr" : "=r" (cur_cpsr));
	cur_cpsr &= I_BIT_MASK;
	user_cpsr = regs[REG_CPSR];
	user_cpsr &= ~I_BIT_MASK;
	user_cpsr |= cur_cpsr;
	regs[REG_CPSR] = user_cpsr;
}
#endif /* CONFIG_RT_FRAMEWORK_SYSCALL */

#endif /* CONFIG_RT_FRAMEWORK */

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif /* __ASSEMBLY__ */

#endif /* __ARCH_ARM_INCLUDE_ARMV8_R_IRQ_H */
