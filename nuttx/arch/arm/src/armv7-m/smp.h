/****************************************************************************
 * arch/arm/src/armv7-m/smp.h
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

#ifndef __ARCH_ARM_SRC_ARMV7_M_SMP_H
#define __ARCH_ARM_SRC_ARMV7_M_SMP_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#if defined(CONFIG_SMP) || defined(CONFIG_BMP)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* ARM requires at least a 4-byte stack alignment.  For use with EABI and
 * floating point, the stack must be aligned to 8-byte addresses.  We will
 * always use the EABI stack alignment
 */

#define SMP_STACK_ALIGNMENT  8
#define SMP_STACK_MASK       7
#define SMP_STACK_SIZE       ((CONFIG_IDLETHREAD_STACKSIZE + 7) & ~7)
#define SMP_STACK_WORDS      (SMP_STACK_SIZE >> 2)

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/


#endif /* __ASSEMBLY__ */
#endif /* CONFIG_SMP */
#endif /* __ARCH_ARM_SRC_ARMV7_M_SMP_H */
