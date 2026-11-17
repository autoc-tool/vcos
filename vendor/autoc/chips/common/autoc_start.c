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

extern char _cpu_data_size[];

/* Must match AUTOC_CORE_RELEASE_ADDR in the mcm7 platform. */

#define AUTOC_CORE_RELEASE_ADDR  0x40001000u

extern int up_cpu_index(void);

static void autoc_copy(void *dst, const void *src, uintptr_t len)
{
  uint8_t *out = dst;
  const uint8_t *in = src;
  uintptr_t i;

  for (i = 0; i < len; i++)
    {
      out[i] = in[i];
    }
}

static void autoc_release_core(int cpu)
{
  putreg32((uint32_t)cpu, AUTOC_CORE_RELEASE_ADDR);
  __asm__ __volatile__("dsb" ::: "memory");
}
#endif

static void autoc_start_c(void)
{
#ifdef CONFIG_BMP
  int cpu = up_cpu_index();
  int i;
#else
  const int cpu = 0;
#endif

  mpu_early_reset();

  if (cpu == 0)
    {
      const uint8_t *src;
      uint8_t *dest;

      for (src = _eronly, dest = _sdata; dest < _edata; )
        {
          *dest++ = *src++;
        }

      for (dest = _sbss; dest < _ebss; )
        {
          *dest++ = 0;
        }

#ifdef CONFIG_BMP
      g_cpu_data_size = (uintptr_t)_cpu_data_size;

      for (i = 1; i < CONFIG_BMP_NCPUS; i++)
        {
          autoc_copy((void *)((uintptr_t)_sdata + g_cpu_data_size * i),
                     _sdata, g_cpu_data_size);
        }
#endif
    }

#ifdef CONFIG_ARCH_FPU
  arm_fpuconfig();
#endif

#ifdef USE_EARLYSERIALINIT
  if (cpu == 0)
    {
      arm_earlyserialinit();
    }
#endif

#ifdef CONFIG_BMP
  if (cpu == 0)
    {
      __asm__ __volatile__("dsb" ::: "memory");
      for (i = 1; i < CONFIG_BMP_NCPUS; i++)
        {
          autoc_release_core(i);
        }
    }
#endif

  nx_start();

  for (;;)
    {
    }
}

__attribute__((naked, noreturn)) void __start(void)
{
#ifdef CONFIG_BMP
  __asm__ __volatile__
    (
      "cpsid  i\n\t"
      "ldr    r3, =0x40000000\n\t"
      "ldr    r0, [r3]\n\t"
      "cbz    r0, 1f\n\t"
      "ldr    r1, =g_cpu_data_size\n\t"
      "ldr    r1, [r1]\n\t"
      "ldr    r2, =g_idle_topstack\n\t"
      "ldr    r2, [r2]\n\t"
      "mul    r1, r1, r0\n\t"
      "add    r2, r2, r1\n\t"
      "msr    msp, r2\n\t"
      "1:\n\t"
      "ldr    r3, =autoc_start_c\n\t"
      "bx     r3\n\t"
    );
#else
  __asm__ __volatile__
    (
      "cpsid  i\n\t"
      "ldr    r3, =autoc_start_c\n\t"
      "bx     r3\n\t"
    );
#endif
}
