/*
 * Copyright (c) 2025 Li auto Inc. and its affiliates
 * Licensed under the apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONdITIONS OF aNY KINd, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef TRICORE_INCLUDE_TRAP_H
#define TRICORE_INCLUDE_TRAP_H

#include <nuttx/config.h>
#include <syslog.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_BACKTRACE_NUM        10U
#if defined(CONFIG_UART0_SERIAL_CONSOLE) && (CONFIG_UART0_SERIAL_CONSOLE == 1)
#define EXP_INFO(...)            syslog(LOG_ERR, __VA_ARGS__)
#else
static inline int __my_printf(const void *fmt, ...) {
	(void)fmt;
}
#define EXP_INFO(fmt, ...)       __my_printf(fmt, ##__VA_ARGS__)
#endif /* CONFIG_UART0_SERIAL_CONSOLE */

typedef uintptr_t register_t;

typedef struct
{
	unsigned int trap_addr;
	unsigned int trap_id : 8;
	unsigned int trap_class : 8;
	unsigned int trap_cpu : 3;
} tricore_cpu_trap_t;

typedef enum
{
	tricore_trap_class_memory_management     = 0U,
	tricore_trap_class_internal_protection   = 1U,
	tricore_trap_class_instruction_errors    = 2U,
	tricore_trap_class_context_management    = 3U,
	tricore_trap_class_bus                  = 4U,
	tricore_trap_class_assertion            = 5U,
	tricore_trap_class_systemCall           = 6U,
	tricore_trap_class_non_maskable_interrupt = 7U
} tricore_trap_class;

#if (defined(CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER) && (CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER == 1))
typedef enum
{
	RTFW_STATUS_PROTECTION_EXCEPTION = 0U,
	RTFW_STATUS_PROTECTION_MEMORY = 1U,
} rtfw_protection_status_t;
#endif /* CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER */

typedef struct upper_csa {
	register_t pcxi;
	register_t psw;
	register_t a10;         /* SP */
	register_t a11;         /* Ra */

	register_t d8;
	register_t d9;
	register_t d10;
	register_t d11;

	register_t a12;
	register_t a13;
	register_t a14;
	register_t a15;

	register_t d12;
	register_t d13;
	register_t d14;
	register_t d15;

} upper_csa_t;

typedef struct lower_csa {
	register_t pcxi;
	register_t a11;         /* Ra */
	register_t a2;
	register_t a3;

	register_t d0;
	register_t d1;
	register_t d2;
	register_t d3;

	register_t a4;
	register_t a5;
	register_t a6;
	register_t a7;

	register_t d4;
	register_t d5;
	register_t d6;
	register_t d7;
} lower_csa_t;

typedef struct common_csa {
	register_t pcxi;
	register_t psw_a11;
	register_t a10_a2;
	register_t a11_a3;

	register_t d8_d0;
	register_t d9_d1;
	register_t d10_d2;
	register_t d11_d3;

	register_t a12_a4;
	register_t a13_a5;
	register_t a14_a6;
	register_t a15_a7;

	register_t d12_d4;
	register_t d13_d5;
	register_t d14_d6;
	register_t d15_d7;

} common_csa_t;

typedef struct {
	const lower_csa_t *lower_csa;
	const upper_csa_t *upper_csa;
	const tricore_cpu_trap_t *trap_info;
} up_expt_frame_t;

#if (defined(CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER) && (CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER == 1))
extern void rtfw_expt_detail_save(void);
extern void rtfw_expt_protection_hook(rtfw_protection_status_t rtfw_protection_status);
extern void rtfw_expt_frame_save(const up_expt_frame_t *expt_frame_args);
extern void rtfw_expt_detail_info_print(void);
extern void rtfw_expt_backtrace_addr_save(uint32_t backtrace_addr, uint8_t backtrace_depth);
#endif /* CONFIG_RT_FRAMEWORK_HAVE_TRAP_HANDLER */
extern void tricore_exception_handler(const tricore_cpu_trap_t *trap_info);

#ifdef __cplusplus
}
#endif


#endif  /* TRICORE_INCLUDE_TRAP_H */
