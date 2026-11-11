/****************************************************************************
 * arch/arm/include/irq.h
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

#ifndef __ARCH_ARM_INCLUDE_IRQ_H
#define __ARCH_ARM_INCLUDE_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>
#ifndef __ASSEMBLY__
#  include <stdbool.h>
#  include <arch/syscall.h>
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

/* Include chip-specific IRQ definitions (including IRQ numbers) */

#include <arch/chip/irq.h>

/* Include NuttX-specific IRQ definitions */

#include <nuttx/irq.h>

/* Include ARM architecture-specific IRQ definitions (including register
 * save structure and up_irq_save()/up_irq_restore() functions)
 */

#if defined(CONFIG_ARCH_ARMV7A)
#  include <arch/armv7-a/irq.h>
#elif defined(CONFIG_ARCH_ARMV7R)
#  include <arch/armv7-r/irq.h>
#elif defined(CONFIG_ARCH_ARMV8R)
#  include <arch/armv8-r/irq.h>
#elif defined(CONFIG_ARCH_ARMV7M)
#  include <arch/armv7-m/irq.h>
#elif defined(CONFIG_ARCH_ARMV8M)
#  include <arch/armv8-m/irq.h>
#elif defined(CONFIG_ARCH_ARMV6M)
#  include <arch/armv6-m/irq.h>
#else
#  include <arch/arm/irq.h>
#endif

/****************************************************************************
 * Pre-processor Prototypes
 ****************************************************************************/

#ifndef __ASSEMBLY__

#if defined(CONFIG_RT_FRAMEWORK_HOOK) && (CONFIG_RT_FRAMEWORK_HOOK == 1)
#define up_switch_hook_context(param0, param1, param2)                         \
  do {                                                                         \
      sys_call3(SYS_switch_hook_context, (uintptr_t)param0, (uintptr_t)param1, \
                (uintptr_t)param2);                                            \
  } while (0)
#endif

#if defined(CONFIG_RT_FRAMEWORK_SYSCALL) && (CONFIG_RT_FRAMEWORK_SYSCALL == 1)
#define up_syscall(syscall_args)                                               \
  do {                                                                         \
	    sys_call2(SYS_syscall_return, (uintptr_t)syscall_args, (uintptr_t)0);    \
  }while (0)

#endif

#if defined(CONFIG_RT_FRAMEWORK_ISOLATE) && (CONFIG_RT_FRAMEWORK_ISOLATE == 1)
#define up_ppc_switch(ppc_flag, func_id, ppc_ctx_args)                       \
  do {                                                                       \
    sys_call3(SYS_ppc_switch, ppc_flag, func_id, (uintptr_t)ppc_ctx_args);   \
  } while (0)

#endif

#ifndef up_switch_context
#define up_switch_context(tcb, rtcb)                              \
  do {                                                            \
    if (!up_interrupt_context())                                  \
      {                                                           \
        sys_call2(SYS_switch_context, (uintptr_t)&rtcb->xcp.regs, \
                  (uintptr_t)tcb->xcp.regs);                      \
      }                                                           \
  } while (0)
#endif

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: up_getusrpc
 ****************************************************************************/

#define up_getusrpc(regs) \
    (((uint32_t *)((regs) ? (regs) : up_current_regs()))[REG_PC])

#endif /* __ASSEMBLY__ */

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __ARCH_ARM_INCLUDE_IRQ_H */
