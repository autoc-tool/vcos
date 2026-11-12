#ifndef __VENDOR_AUTOC_CHIPS_MCM7_CHIP_H
#define __VENDOR_AUTOC_CHIPS_MCM7_CHIP_H

#include <arch/chip/irq.h>

#define ARMV7M_PERIPHERAL_INTERRUPTS AUTOC_IRQ_NEXTINT
#define ARMV7M_DCACHE_LINESIZE 32
#define ARMV7M_ICACHE_LINESIZE 32

#define g_cpu1_idlestack ((uintptr_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE)
#define g_cpu2_idlestack ((uintptr_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE * 2)
#define g_cpu3_idlestack ((uintptr_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE * 3)

#endif
