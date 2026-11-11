/*
 * Copyright (c) 2025 Li Auto Inc. and its affiliates
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <nuttx/config.h>
#include "sched/sched.h"

static inline_function void up_irq_dispatch_enter(uintptr_t *regs)
{
	uint8_t irq_nesting;
	uintptr_t *pucsa;
	struct tcb_s *tcb = this_task();

	__mtcr(TX3XX_CPU_PSW, __mfcr(TX3XX_CPU_PSW) | CPU_PSW_GW_MASK);
	__isync();

	tricore_set_isp();

	irq_nesting = up_increase_irq_nesting();

	DEBUGASSERT(irq_nesting <= CONFIG_ARCH_IRQ_NESTING_LIMIT);

	up_set_current_regs(regs);

	if (irq_nesting == 1) {
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
		up_mpu_switch(tcb, NULL);
#endif
		tcb->xcp.regs = regs;
	}
}

static inline_function uintptr_t *up_irq_dispatch_exit(uintptr_t *regs)
{
	struct tcb_s *tcb;

	if (up_get_irq_nesting() == 1) {
		tcb = this_task();
		if (regs != tcb->xcp.regs) {
			nxsched_suspend_scheduler(g_running_tasks[this_cpu()]);
			nxsched_resume_scheduler(tcb);

#ifdef CONFIG_ARCH_ADDRENV
			addrenv_switch(NULL);
#endif
			g_running_tasks[this_cpu()] = tcb;

			regs = tcb->xcp.regs;
		}
#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
		up_mpu_switch(NULL, tcb);
#endif
#if defined(CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK) && (CONFIG_RT_FRAMEWORK_SHARE_TASK_STACK == 1)
		if (tcb->share_stack_info != NULL) {
			up_share_stack_copy(tcb);
		}
#endif
		up_set_contexthdl(tcb->contexthdl);
#ifdef CONFIG_STACK_OVERFLOW_CHECK
		if (0U != up_check_tcbstack_overflow(tcb)) {
			// TODO 待添加TRAP后处理
		}
#endif
	}

	(void)up_decrease_irq_nesting();

	return regs;
}
