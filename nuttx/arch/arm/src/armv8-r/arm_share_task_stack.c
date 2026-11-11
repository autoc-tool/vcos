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

#include <nuttx/sched.h>
#include <nuttx/tls.h>
#include <arch/irq.h>
#include <string.h>
#include "syslog.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline void save_context_to_temp(struct tcb_s *tcb)
{
	uintptr_t *sp = (uintptr_t *)tcb->xcp.regs[REG_SP];
	uintptr_t end_stack = (uintptr_t)tcb->stack_base_ptr + (uintptr_t)tcb->adj_stack_size;
	uint32_t use_len = end_stack - ((uintptr_t)sp - XCPTCONTEXT_SIZE);

	memcpy(tcb->share_stack_info->temp_stack->temp_tls_buffer,
		   tcb->stack_alloc_ptr,
		   (uintptr_t)tcb->stack_base_ptr - (uintptr_t)tcb->stack_alloc_ptr);

	if (use_len <= CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ)
	{
		memcpy(tcb->share_stack_info->temp_stack->temp_stack_buffer,
			   (void *)((uintptr_t)end_stack - CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ),
			   CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ);
	} else
	{
		syslog(LOG_ERR,
			   "\nline %d, shared stack saving error, max copylen is %lu\n",
			   __LINE__, (uint32_t)CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ);
		syslog(LOG_ERR,
			   "use_len = %lu, end = 0x%x, start = 0x%x\n",
			   use_len, end_stack, (uintptr_t)sp);
		syslog(LOG_ERR,
			   "task name: %s\n", tcb->name);
		DEBUGASSERT(0);
	}
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline void restore_context_from_temp(struct tcb_s *tcb)
{
	uintptr_t end_stack = (uintptr_t)tcb->stack_base_ptr + (uintptr_t)tcb->adj_stack_size;

	memcpy((void *)tcb->stack_alloc_ptr,
		   tcb->share_stack_info->temp_stack->temp_tls_buffer,
		   (uintptr_t)tcb->stack_base_ptr - (uintptr_t)tcb->stack_alloc_ptr);

	memcpy((void *)((uintptr_t)end_stack - CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ),
			tcb->share_stack_info->temp_stack->temp_stack_buffer,
			CONFIG_RT_FRAMEWORK_SHARE_TASK_TEMP_STACK_BUF_SZ);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void up_share_stack_copy(struct tcb_s *tcb)
{
	const struct task_share_stack_s *share_stack_info = tcb->share_stack_info;
	struct tcb_s *tcb_in_stack;

	struct tcb_s **owner = share_stack_info->share_stack_owner;

	if (*owner == tcb)
	{
		/* Already current task in stack */
		return;
	} else if (*owner != NULL)
	{
		/* Save old task context */
		tcb_in_stack = *owner;
		save_context_to_temp(tcb_in_stack);

		/* Restore new task context */
		restore_context_from_temp(tcb);
	} else
	{
		/* No previous owner, just restore */
		restore_context_from_temp(tcb);
	}

	*owner = tcb;
}