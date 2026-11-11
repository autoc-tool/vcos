/*
* Copyright (c) 2025 Li Auto Inc. and its affiliates
* Licensed under the Apache License, Version 2.0(the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*	 http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __ARCH_ARM_SRC_ARMV8_R_ARM_MPU_H
#define __ARCH_ARM_SRC_ARMV8_R_ARM_MPU_H

#include <arch/irq.h>
#include "sched/sched.h"

static inline_function void arm_switch_to_kernel_mpu_set(void)
{
	up_mpu_enable_set(0U);
}

static inline_function void arm_switch_to_task_mpu_set(struct tcb_s *tcb)
{
	if ((0U == (tcb->flags & TCB_FLAG_THREAD_IN_KERNEL_MPU_SET)) && (NULL != tcb->contexthdl)) {
		up_mpu_enable_set(
			((struct mpu_access_right_s *)(*(uintptr_t *)(*(uintptr_t *)(tcb->contexthdl))))->set_num);
	}
}

#endif /* __ARCH_ARM_SRC_ARMV8_R_ARM_MPU_H */