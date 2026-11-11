#ifndef __VENDOR_AUTOC_CHIPS_SCM7_INCLUDE_IRQ_H
#define __VENDOR_AUTOC_CHIPS_SCM7_INCLUDE_IRQ_H

#include <nuttx/config.h>

#define AUTOC_IRQ_RESERVED    (0)
#define AUTOC_IRQ_NMI         (2)
#define AUTOC_IRQ_HARDFAULT   (3)
#define AUTOC_IRQ_MEMFAULT    (4)
#define AUTOC_IRQ_BUSFAULT    (5)
#define AUTOC_IRQ_USAGEFAULT  (6)
#define AUTOC_IRQ_SVCALL      (11)
#define AUTOC_IRQ_DBGMONITOR  (12)
#define AUTOC_IRQ_PENDSV      (14)
#define AUTOC_IRQ_SYSTICK     (15)
#define AUTOC_IRQ_EXINT       (16)
#define AUTOC_IRQ_NIRQS       (48)
#define AUTOC_IRQ_NEXTINT     (AUTOC_IRQ_NIRQS - AUTOC_IRQ_EXINT)
#define NR_IRQS               AUTOC_IRQ_NIRQS

#ifndef __ASSEMBLY__
#include <stdbool.h>
void up_init_irq(int irq, int irq_prio);
bool up_irq_is_enabled(int irq);
void up_clear_irq(int irq);
#endif

#endif
