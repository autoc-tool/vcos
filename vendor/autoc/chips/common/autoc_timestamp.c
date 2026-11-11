#include <nuttx/config.h>

#include <stdint.h>
#include <sys/types.h>

#include "nvic.h"
#include "dwt.h"
#include "arm_internal.h"

static void autoc_dwt_enable(void)
{
  uint32_t demcr;

  demcr = getreg32(NVIC_DEMCR);
  putreg32(demcr | NVIC_DEMCR_TRCENA, NVIC_DEMCR);
  putreg32(0, DWT_CYCCNT);
  putreg32(getreg32(DWT_CTRL) | DWT_CTRL_CYCCNTENA_MASK, DWT_CTRL);
}

uint32_t up_timer_freq(void)
{
  return CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;
}

int up_timer_value(clock_t *ticks)
{
  autoc_dwt_enable();
  *ticks = getreg32(DWT_CYCCNT);
  return OK;
}
