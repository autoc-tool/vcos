/****************************************************************************
 * arch/tricore/include/irq.h
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

#ifndef __ARCH_TRICORE_INCLUDE_IRQ_H
#define __ARCH_TRICORE_INCLUDE_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>
#ifndef __ASSEMBLY__
#  include <stdbool.h>
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
#  include <arch/syscall.h>
#endif
#endif

/* Include NuttX-specific IRQ definitions */

#include <nuttx/irq.h>

/* Include chip-specific IRQ definitions (including IRQ numbers) */

#include <arch/chip/irq.h>

#if defined(CONFIG_ARCH_TC3XX)
#  include <arch/tc3xx/irq.h>
#elif defined(CONFIG_ARCH_TC4XX)
#  include <arch/tc4xx/irq.h>
#else
# error "Unsupported tricore arch family"
#endif

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/
#if (CONFIG_ARCH_IRQ_NESTING_LIMIT > 1)
#ifdef CONFG_SMP
EXTERN uintptr_t *g_current_regs[CONFIG_SMP_NCPUS][CONFIG_ARCH_IRQ_NESTING_LIMIT];
#else
EXTERN uintptr_t *g_current_regs[CONFIG_ARCH_IRQ_NESTING_LIMIT];
#define g_current_regs this_cpu_var(g_current_regs)
#endif
#endif

#define CURRENT_REGS up_current_regs()

noinstrument_function static inline_function uint32_t tricore_get_a0reg(void)
{
  uint32_t val;
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
   __asm volatile( "mov.d  %0, a0" :"=d"(val)::);
#else
   __asm__ volatile ("mov.d %0, %%a0": "=d" (val) : :"a0");
#endif
  return val;
}

noinstrument_function static inline_function uint32_t tricore_get_a1reg(void)
{
  uint32_t val;
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
   __asm volatile( "mov.d  %0, a1" :"=d"(val)::);
#else
   __asm__ volatile ("mov.d %0, %%a1": "=d" (val) : :"a1");
#endif
  return val;
}

noinstrument_function static inline_function uint32_t tricore_get_a8reg(void)
{
  uint32_t val;
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
   __asm volatile( "mov.d  %0, a8" :"=d"(val)::);
#else
   __asm__ volatile ("mov.d %0, %%a8": "=d" (val) : :"a8");
#endif
  return val;
}

noinstrument_function static inline_function uint32_t tricore_get_a9reg(void)
{
  uint32_t val;
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
   __asm volatile( "mov.d  %0, a9" :"=d"(val)::);
#else
   __asm__ volatile ("mov.d %0, %%a9": "=d" (val) : :"a9");
#endif
  return val;
}

noinstrument_function static inline_function void tricore_set_a0reg(uint32_t val)
{
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
  __asm volatile("mov.a  a0, %0" ::"d"(val):"a0");
#else
  __setareg(a0,val);
#endif
}

noinstrument_function static inline_function void tricore_set_a1reg(uint32_t val)
{
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
  __asm volatile("mov.a  a1, %0" ::"d"(val):"a1");
#else
  __setareg(a1,val);
#endif
}

noinstrument_function static inline_function void tricore_set_a8reg(uint32_t val)
{
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
  __asm volatile("mov.a  a8, %0" ::"d"(val):"a8");
#else
  __setareg(a8,val);
#endif
}

noinstrument_function static inline_function void tricore_set_a9reg(uint32_t val)
{
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
  __asm volatile("mov.a  a9, %0" ::"d"(val):"a9");
#else
  __setareg(a9,val);
#endif
}

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: up_cpu_index
 *
 * Description:
 *   Return the real core number regardless CONFIG_SMP setting
 *
 ****************************************************************************/
static inline int up_cpu_index(void)
{
#ifdef CONFIG_ARCH_HAVE_MULTICPU
  int coreid = __mfcr(TC3XX_CPU_CORE_ID);
  if (coreid == 6)
    coreid = 5;
  return coreid;
#else
  return 0;
#endif
}

/****************************************************************************
 * Name: up_irq_enable
 *
 * Description:
 *   Enable interrupts globally.
 *
 ****************************************************************************/

void up_irq_enable(void);

/****************************************************************************
 * Inline functions
 ****************************************************************************/

noinstrument_function static inline uintptr_t up_getsp(void)
{
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
  return (uintptr_t)__get_sp();
#else
  return __builtin_frame_address(0);
#endif
}

/****************************************************************************
 * Name: up_irq_disable
 *
 * Description:
 *   Disable interrupts
 *
 ****************************************************************************/
noinstrument_function static inline void up_irq_disable(void)
{
  __disable();
}

/****************************************************************************
 * Name: up_irq_save
 *
 * Description:
 *   Disable interrupts and return the previous value of the mstatus register
 *
 ****************************************************************************/

noinstrument_function static inline irqstate_t up_irq_save(void)
{
  return __disable_and_save();
}

/****************************************************************************
 * Name: up_irq_restore
 *
 * Description:
 *   Restore the value of the mstatus register
 *
 ****************************************************************************/

noinstrument_function static inline void up_irq_restore(irqstate_t flags)
{
  __restore(flags);
}

static inline_function uint8_t up_get_irq_nesting(void)
{
  return tricore_get_a0reg();
}

static inline_function uint8_t up_increase_irq_nesting(void)
{
  uint32_t irq_nesting = tricore_get_a0reg();
  irq_nesting++;
  tricore_set_a0reg(irq_nesting);
  return (uint8_t)irq_nesting;
}

static inline_function uint8_t up_decrease_irq_nesting(void)
{
  uint32_t irq_nesting = tricore_get_a0reg();
  irq_nesting--;
  tricore_set_a0reg(irq_nesting);
  return (uint8_t)irq_nesting;
}

#ifdef CONFIG_BMP
noinstrument_function static inline_function void up_set_bmp_offset(uint32_t offset)
{
  tricore_set_a1reg(offset);
}

noinstrument_function static inline_function uint32_t up_get_bmp_offset(void)
{
  return tricore_get_a1reg();
}
#endif

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)

static inline_function irqlevel_t up_get_irq_mask_level(void)
{
  return (__mfcr(TC3XX_CPU_ICR) & ICR_CCPN_MASK);
}

static inline_function void up_set_irq_mask_level(irqlevel_t level)
{
  __mtcr( TC3XX_CPU_ICR, ((__mfcr( TC3XX_CPU_ICR ) & (~ICR_CCPN_MASK)) | level) );
  __isync();
}

static inline_function irqlevel_t up_get_irq_running_level(void)
{
  return up_get_irq_mask_level();
}

static inline_function irqlevel_t up_irq_level_save(irqlevel_t level)
{
  uintptr_t icr = (uintptr_t)__mfcr(TC3XX_CPU_ICR);
  irqlevel_t pre_level = (icr & ICR_CCPN_MASK) >> ICR_CCPN;

  if (level > pre_level) {
    icr &= ~ICR_CCPN_MASK;
    icr |= (level << ICR_CCPN);
    __mtcr(TC3XX_CPU_ICR, icr);
    __isync();
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

noinstrument_function static inline void up_set_contexthdl(void * const ctxhdl)
{
  tricore_set_a8reg((uint32_t)ctxhdl);
}

noinstrument_function static inline void * up_get_contexthdl(void)
{
  return (void *)tricore_get_a8reg();
}

#if defined(CONFIG_RT_FRAMEWORK_SYSCALL) && (CONFIG_RT_FRAMEWORK_SYSCALL == 1)
static inline_function void up_syscall(void *syscall_args)
{
  sys_call2(SYS_syscall_return, (uintptr_t)syscall_args, (uintptr_t)0);
}
#endif

#if defined(CONFIG_RT_FRAMEWORK_HOOK) && (CONFIG_RT_FRAMEWORK_HOOK == 1)
static inline_function void up_switch_hook_context(param0, param1, param2)
{
  sys_call3(SYS_switch_hook_context, (uintptr_t)param0, (uintptr_t)param1, (uintptr_t)param2);
}
#endif

#if defined(CONFIG_RT_FRAMEWORK_ISOLATE) && (CONFIG_RT_FRAMEWORK_ISOLATE == 1)
static inline_function void up_ppc_switch(const uint32_t ppc_flag, const uint32_t func_id, void *ppc_ctx_args)
{
  sys_call3(SYS_ppc_switch, ppc_flag, func_id, (uintptr_t)ppc_ctx_args);
}
#endif

static inline_function void tricore_set_isp(void)
{
	if (up_get_irq_nesting() == 0) {
		uintptr_t isp = __mfcr(TC3XX_CPU_ISP);
#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
		__asm volatile("mov.a  a10, %0" ::"d"(isp) : "a10");
#else
		__setareg(a10, isp);
#endif
	}
}

#endif

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

static inline_function uintptr_t *up_current_regs(void)
{
#if (CONFIG_ARCH_IRQ_NESTING_LIMIT >1)
  uint32_t index = up_get_irq_nesting();
#ifdef CONFG_SMP
  return (uintptr_t *)g_current_regs[up_cpu_index()][--index];
#else
  return (uintptr_t *)g_current_regs[--index];
#endif
#else
  return tricore_get_a9reg();
#endif

}

static inline_function void up_set_current_regs(uintptr_t *regs)
{
#if (CONFIG_ARCH_IRQ_NESTING_LIMIT >1)
  uint32_t index = up_get_irq_nesting();

#ifdef CONFG_SMP
  g_current_regs[up_cpu_index()][--index] = regs;
#else
  g_current_regs[--index] = regs;
#endif
#else
  tricore_set_a9reg((uintptr_t)regs);
#endif
}

/****************************************************************************
 * Name: up_getusrpc
 ****************************************************************************/
#define __tricore_csa2addr(csa) ((uintptr_t *)((((csa) & 0x000F0000) << 12) \
                                             | (((csa) & 0x0000FFFF) << 6)))
static inline uintptr_t * up_getusrpc(void *regs)
{
  uintptr_t *plcsa = (uintptr_t *)((regs) ? (regs) : up_current_regs());

  __dsync();

  uintptr_t *pucsa = (uintptr_t *)__tricore_csa2addr(plcsa[REG_UPCXI]);
  return (uintptr_t *)pucsa[REG_UPC];
}


/****************************************************************************
 * Name: up_interrupt_context
 *
 * Description:
 *   Return true is we are currently executing in the interrupt
 *   handler context.
 *
 ****************************************************************************/

noinstrument_function
static inline_function bool up_interrupt_context(void)
{
  return up_get_irq_nesting() != 0;
}
#endif /* __ASSEMBLY__ */

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __ARCH_TRICORE_INCLUDE_IRQ_H */
