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
#include "Os_Types.h"
#include "rtfw_core.h"
#include "Os.h"
#include "rtfw_counter.h"
#include "counter_cfg.h"
#include "task_cfg.h"
#include "isr_cfg.h"
#include "syslog.h"
#include <nuttx/signal.h>
#include "rtfw_spinlock.h"
#include "spinlock_cfg.h"
#include "event_cfg.h"
#if(CONFIG_ETH_STACK == 1)
#include "tcpip.h"
#include "ethif.h"
#endif
#if defined(CONFIG_RT_FRAMEWORK_HOOK) && (CONFIG_RT_FRAMEWORK_HOOK == 1)
#include "rtfw_hook.h"
#endif
#if defined(__NuttX__)
#include "nuttx/arch.h"
#endif

#if defined(CONFIG_RT_FRAMEWORK_TIMER) && (CONFIG_RT_FRAMEWORK_TIMER == 1)
#define SYSTICK_PROCESS(coreid) os_counter_hrt_handler()
#else
static const OS_COUNTER_DEF system_counterid_table[CONFIG_NR_CPUS] = {
	systemtimer0,
#if (CONFIG_NR_CPUS > 1)
	systemtimer1,
#endif
#if (CONFIG_NR_CPUS > 2)
	systemtimer2,
#endif
#if (CONFIG_NR_CPUS > 3)
	systemtimer3,
#endif
#if (CONFIG_NR_CPUS > 4)
	systemtimer4,
#endif
#if (CONFIG_NR_CPUS > 5)
	systemtimer5,
#endif
};
void up_systick_isr_handler(void);
#define SYSTICK_PROCESS(coreid) do { \
	up_systick_isr_handler(); \
	os_counter_increment(system_counterid_table[coreid]); \
}while (0)
#endif

extern void SchM_Init(void);
extern void Rte_Start(void);
#if(CONFIG_ETH_STACK == 1)
extern const ethif_config_type ethif_config_data;  /* EthIf配置数据 */
extern const tcpip_config_type TcpIp_Config;     /* TcpIp配置数据 */
extern void bsw_tcpip_init(const tcpip_config_type* ConfigPtr);
extern void eth_gethmac_init(void);
#endif

#if(CONFIG_ARCH_CHIP_E3650 == 1)
extern uint8 ea_eep_init(void);
extern void nvm_init(void);
void up_systick_isr_handler(void);

int e3650_init_main(int argc, FAR char *argv[])
{
    return 0;
}
#endif

#if defined(CONFIG_ARCH_CHIP_AUTOC) && (CONFIG_ARCH_CHIP_AUTOC == 1)
int autoc_init_main(int argc, FAR char *argv[])
{
    return 0;
}
#endif

#if(CONFIG_ARCH_CHIP_CUSTOM_TC397 == 1)
int tc397_init_main(int argc, FAR char *argv[])
{
    return 0;
}

static void uart_isr_init(void)
{
    const os_isr_cfg_t *UrtIsrCfg = os_isr_cfg_ref_table[OS_ISR_OS_ISR_UART_CORE0_ID];
    const os_arch_isr_cfg_t *UrtArchContextCfg = &(UrtIsrCfg->arch_isr_cfg);
    up_init_irq(UrtArchContextCfg->vector_num, UrtArchContextCfg->level);
    up_enable_irq(UrtArchContextCfg->vector_num);
}
#endif


#ifdef CONFIG_ARCH_CHIP_J6E
int j6e_init_main(int argc, FAR char *argv[])
{
    return 0;
}
#endif

void platform_init(void)
{

}

void os_task_core0_init(void *args)
{
#if(CONFIG_ARCH_CHIP_CUSTOM_TC397 == 1)
    uart_isr_init();
#endif

#if(CONFIG_ARCH_CHIP_E3650 == 1)
    ea_eep_init();
    nvm_init();
#endif

    SchM_Init();
    Rte_Start();

#if(CONFIG_ETH_STACK == 1)
    eth_gethmac_init();
    ethif_init(&ethif_config_data);
    bsw_tcpip_init(&TcpIp_Config);
#endif

    TerminateTask();
}

#if (CONFIG_NR_CPUS > 1)
void os_task_core1_init(void *arg)
{
    SchM_Init();
    Rte_Start();

    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 2)
void os_task_core2_init(void *arg)
{
#ifndef CONFIG_ARCH_CHIP_J6E
    SchM_Init();
    Rte_Start();
#endif
    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 3)
void os_task_core3_init(void *arg)
{
#ifndef CONFIG_ARCH_CHIP_J6E
    SchM_Init();
    Rte_Start();
#endif
    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 4)
void os_task_core4_init(void *arg)
{
#ifndef CONFIG_ARCH_CHIP_J6E
    SchM_Init();
    Rte_Start();
#endif
    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 5)
void os_task_core5_init(void *arg)
{
#ifndef CONFIG_ARCH_CHIP_J6E
    SchM_Init();
    Rte_Start();
#endif
    TerminateTask();
}
#endif

void core0_systick_handler(void)
{
	SYSTICK_PROCESS(0);
}

#if (CONFIG_NR_CPUS > 1)
void core1_systick_handler(void)
{
	SYSTICK_PROCESS(1);
}
#endif

#if (CONFIG_NR_CPUS > 2)
void core2_systick_handler(void)
{
	SYSTICK_PROCESS(2);
}
#endif

#if (CONFIG_NR_CPUS > 3)
void core3_systick_handler(void)
{
	SYSTICK_PROCESS(3);
}
#endif

#if (CONFIG_NR_CPUS > 4)
void core4_systick_handler(void)
{
	SYSTICK_PROCESS(4);
}
#endif

#if (CONFIG_NR_CPUS > 5)
void core5_systick_handler(void)
{
	SYSTICK_PROCESS(5);
}
#endif

#ifndef CONFIG_TRACED
void handshake_main_func(void)
{

}
void trace_dumpudp_main_func(void)
{

}
#endif