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

#include <stdio.h>
#include <nuttx/config.h>
#include <sched/sched.h>
#include <debug.h>
#include <arch/irq.h>
#include "tricore_internal.h"
#include <arch/trap.h>

#define PRIVILEGED_INSTRUCTION 1U

static __percpu_data bool coredump_first_entry = true;
#define coredump_first_entry this_cpu_var(coredump_first_entry)

const char *separator = "------------------------------";

static void tricore_parse_csa_backtrace(const lower_csa_t *const lower_csa)
{
	uint32_t curr_pcxi;
	uint32_t next_pcxi = lower_csa->pcxi;
	uint8_t backtrace_depth = 0;

	while(tricore_csa2addr((uintptr_t)next_pcxi) != NULL){
		curr_pcxi = next_pcxi;

		if ((curr_pcxi & (1U << 20)) != 0) {
			if (backtrace_depth < MAX_BACKTRACE_NUM){
#if (defined(CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER) && (CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER == 1))
				rtfw_expt_backtrace_addr_save(((upper_csa_t *)tricore_csa2addr((uintptr_t)curr_pcxi))->a11, backtrace_depth);
#else
				EXP_INFO("Backtrace[%d]: 0x%08lx\r\n", backtrace_depth, ((upper_csa_t *)tricore_csa2addr((uintptr_t)curr_pcxi))->a11);
#endif /* CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER */
				backtrace_depth++;
			}
		}
		next_pcxi = ((common_csa_t *)tricore_csa2addr((uintptr_t)curr_pcxi))->pcxi;
	}
}


void tricore_coredump_handle(const tricore_cpu_trap_t *trap_info, up_expt_frame_t *expt_frame)
{
	const lower_csa_t *const lower_csa = expt_frame->lower_csa;
	const upper_csa_t *const upper_csa = expt_frame->upper_csa;
	const char* reg_fmt = "\t%s =  0x%08x\r\n";

	tricore_parse_csa_backtrace(lower_csa);
#if (defined(CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER) && (CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER == 1))
	rtfw_expt_detail_save();
	rtfw_expt_frame_save((const up_expt_frame_t*)expt_frame);
#endif /* CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER */

	if(!up_interrupt_context())	{
#if(CONFIG_RT_FRAMEWORK_NCORES > 1)
		EXP_INFO("\r\n\r\n%sAn Exception Occured On Core %d%s\r\n", separator, up_cpu_index(), separator);
#else
		EXP_INFO("\r\n\r\n%sAn Exception Occured%s\r\n", separator, separator);
#endif
		EXP_INFO("Trap Addr = 0x%08x\n", trap_info->trap_addr);
		EXP_INFO("Trap Class = %d\n", trap_info->trap_class);
		EXP_INFO("Trap ID = %d\n", trap_info->trap_id);
		EXP_INFO("\r\nRegister Dump:\r\n");
		EXP_INFO(reg_fmt, "PSW", upper_csa->psw);
		EXP_INFO(reg_fmt, "A0", tricore_get_a0reg());
		EXP_INFO(reg_fmt, "A1", tricore_get_a1reg());
		EXP_INFO(reg_fmt, "A2", lower_csa->a2);
		EXP_INFO(reg_fmt, "A3", lower_csa->a3);
		EXP_INFO(reg_fmt, "A4", lower_csa->a4);
		EXP_INFO(reg_fmt, "A5", lower_csa->a5);
		EXP_INFO(reg_fmt, "A6", lower_csa->a6);
		EXP_INFO(reg_fmt, "A7", lower_csa->a7);
		EXP_INFO(reg_fmt, "A8", tricore_get_a8reg());
		EXP_INFO(reg_fmt, "A9", tricore_get_a9reg());
		EXP_INFO(reg_fmt, "SP", upper_csa->a10);
		EXP_INFO(reg_fmt, "PC", upper_csa->a11);
		EXP_INFO(reg_fmt, "A12", upper_csa->a12);
		EXP_INFO(reg_fmt, "A13", upper_csa->a13);
		EXP_INFO(reg_fmt, "A14", upper_csa->a14);
		EXP_INFO(reg_fmt, "A15", upper_csa->a15);
		EXP_INFO(reg_fmt, "D0", lower_csa->d0);
		EXP_INFO(reg_fmt, "D1", lower_csa->d1);
		EXP_INFO(reg_fmt, "D2", lower_csa->d2);
		EXP_INFO(reg_fmt, "D3", lower_csa->d3);
		EXP_INFO(reg_fmt, "D4", lower_csa->d4);
		EXP_INFO(reg_fmt, "D5", lower_csa->d5);
		EXP_INFO(reg_fmt, "D6", lower_csa->d6);
		EXP_INFO(reg_fmt, "D7", lower_csa->d7);
		EXP_INFO(reg_fmt, "D8", upper_csa->d8);
		EXP_INFO(reg_fmt, "D9", upper_csa->d9);
		EXP_INFO(reg_fmt, "D10", upper_csa->d10);
		EXP_INFO(reg_fmt, "D11", upper_csa->d11);
		EXP_INFO(reg_fmt, "D12", upper_csa->d12);
		EXP_INFO(reg_fmt, "D13", upper_csa->d13);
		EXP_INFO(reg_fmt, "D14", upper_csa->d14);
		EXP_INFO(reg_fmt, "D15", upper_csa->d15);
	}
#if (defined(CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER) && (CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER == 1))
	rtfw_expt_detail_info_print();
#endif /* CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER */
}

static void tricore_exception_hook(const tricore_cpu_trap_t *trap_info)
{
#if defined(CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER) && (CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER == 1)
	rtfw_protection_status_t rtfw_protection_status = 0;

	if(trap_info->trap_class == tricore_trap_class_internal_protection) {
		if (trap_info->trap_id == PRIVILEGED_INSTRUCTION) {
			rtfw_protection_status = RTFW_STATUS_PROTECTION_EXCEPTION;
		} else {
			rtfw_protection_status = RTFW_STATUS_PROTECTION_MEMORY;
		}
	} else {
		rtfw_protection_status = RTFW_STATUS_PROTECTION_EXCEPTION;
	}
	rtfw_expt_protection_hook(rtfw_protection_status);
#endif /* CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER */
}

void tricore_exception_handler(const tricore_cpu_trap_t *trap_info)
{
	const uintptr_t* const csa_addr = (uintptr_t*)tricore_csa2addr(__mfcr(TC3XX_CPU_PCXI));
	up_expt_frame_t expt_frame;
	expt_frame.lower_csa = (const lower_csa_t*)csa_addr;
	expt_frame.upper_csa = (const upper_csa_t*)tricore_csa2addr(expt_frame.lower_csa->pcxi);
	expt_frame.trap_info = trap_info;
	if (coredump_first_entry == true) {
		coredump_first_entry = false;
		tricore_coredump_handle(trap_info, &expt_frame);
	} else {
		EXP_INFO("Core %d: Exception occurred, but coredump already recorded.\n", up_cpu_index());
	}
	tricore_exception_hook(trap_info);
	while (TRUE) {
		/* Do nothing*/
	}
}
