#include <nuttx/config.h>

#include <stdint.h>
#include <debug.h>

#include <nuttx/arch.h>
#include "nvic.h"
#include "arm_internal.h"
#include "clock/clock.h"

#define SYSTICK_RELOAD(coreclk) (((coreclk) / CLK_TCK) - 1)

static int autoc_timerisr(int irq, void *regs, void *arg)
{
  UNUSED(irq);
  UNUSED(regs);
  UNUSED(arg);
  nxsched_process_timer();
  return OK;
}

void up_timer_initialize(void)
{
  uint32_t reload = SYSTICK_RELOAD(CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC);

  DEBUGASSERT(reload <= 0x00ffffff);

  putreg32(NVIC_SYSTICK_CTRL_CLKSOURCE, NVIC_SYSTICK_CTRL);
  putreg32(reload, NVIC_SYSTICK_RELOAD);
  irq_attach(AUTOC_IRQ_SYSTICK, autoc_timerisr, NULL);

  putreg32(NVIC_SYSTICK_CTRL_CLKSOURCE | NVIC_SYSTICK_CTRL_TICKINT |
           NVIC_SYSTICK_CTRL_ENABLE, NVIC_SYSTICK_CTRL);
  up_enable_irq(AUTOC_IRQ_SYSTICK);
}

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
void up_systick_isr_handler(void)
{
  nxsched_process_timer();
}
#endif
