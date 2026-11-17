#ifndef __VENDOR_AUTOC_CHIPS_MCM7_CHIP_H
#define __VENDOR_AUTOC_CHIPS_MCM7_CHIP_H

#include <arch/chip/irq.h>

#define ARMV7M_PERIPHERAL_INTERRUPTS AUTOC_IRQ_NEXTINT
#define ARMV7M_DCACHE_LINESIZE 32
#define ARMV7M_ICACHE_LINESIZE 32

#ifndef __ASSEMBLY__
extern uint32_t _cpu1_idlestack[];
extern uint32_t _cpu2_idlestack[];
extern uint32_t _cpu3_idlestack[];
#endif

#define g_cpu1_idlestack ((const uint32_t *)_cpu1_idlestack)
#define g_cpu2_idlestack ((const uint32_t *)_cpu2_idlestack)
#define g_cpu3_idlestack ((const uint32_t *)_cpu3_idlestack)

#endif
