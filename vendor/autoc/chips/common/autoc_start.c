#include <nuttx/config.h>

#include <stdint.h>
#include <nuttx/init.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "nvic.h"
#include "mpu.h"

const uintptr_t g_idle_topstack = (uintptr_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE;

#ifdef CONFIG_BMP
unsigned long g_cpu_data_size;
#endif

void __start(void)
{
  const uint8_t *src;
  uint8_t *dest;

  __asm__ __volatile__("cpsid i");

  mpu_early_reset();

  for (src = _eronly, dest = _sdata; dest < _edata; )
    {
      *dest++ = *src++;
    }

  for (dest = _sbss; dest < _ebss; )
    {
      *dest++ = 0;
    }

#ifdef CONFIG_ARCH_FPU
  arm_fpuconfig();
#endif

#ifdef USE_EARLYSERIALINIT
  arm_earlyserialinit();
#endif
  nx_start();

  for (;;)
    {
    }
}
