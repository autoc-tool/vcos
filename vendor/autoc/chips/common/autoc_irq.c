#include <nuttx/config.h>

#include <stdint.h>
#include <debug.h>
#include <errno.h>

#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <arch/irq.h>

#include "chip.h"
#include "nvic.h"
#include "arm_internal.h"

extern const void * const _vectors[];

#define DEFPRIORITY32 \
  (NVIC_SYSH_PRIORITY_DEFAULT << 24 | NVIC_SYSH_PRIORITY_DEFAULT << 16 | \
   NVIC_SYSH_PRIORITY_DEFAULT << 8 | NVIC_SYSH_PRIORITY_DEFAULT)

#if defined(CONFIG_BMP)
volatile uint32_t *g_interrupt_context[CONFIG_BMP_NCPUS];
#else
volatile uint32_t *g_interrupt_context[1];
#endif

#define NVIC_ENA_OFFSET    (0)
#define NVIC_CLRENA_OFFSET (NVIC_IRQ0_31_CLEAR - NVIC_IRQ0_31_ENABLE)
#define NVIC_CLRPEND_OFFSET (NVIC_IRQ0_31_CLRPEND - NVIC_IRQ0_31_ENABLE)

#if defined(CONFIG_BMP) && CONFIG_ARCH_INTERRUPTSTACK > 7
static uint64_t g_irqstack_alloc[(CONFIG_BMP_NCPUS * CONFIG_ARCH_INTERRUPTSTACK) >> 3];

const uint32_t g_irqstack_top[CONFIG_BMP_NCPUS] =
{
  (uint32_t)g_irqstack_alloc + CONFIG_ARCH_INTERRUPTSTACK,
#if CONFIG_BMP_NCPUS > 1
  (uint32_t)g_irqstack_alloc + (2 * CONFIG_ARCH_INTERRUPTSTACK),
#endif
#if CONFIG_BMP_NCPUS > 2
  (uint32_t)g_irqstack_alloc + (3 * CONFIG_ARCH_INTERRUPTSTACK),
#endif
#if CONFIG_BMP_NCPUS > 3
  (uint32_t)g_irqstack_alloc + (4 * CONFIG_ARCH_INTERRUPTSTACK),
#endif
};

uintptr_t up_get_intstackbase(int cpu)
{
  return g_irqstack_top[cpu] - CONFIG_ARCH_INTERRUPTSTACK;
}
#endif

static int autoc_irqinfo(int irq, uintptr_t *regaddr, uint32_t *bit, uintptr_t offset)
{
  int n;

  if (irq >= AUTOC_IRQ_EXINT)
    {
      n = irq - AUTOC_IRQ_EXINT;
      *regaddr = NVIC_IRQ_ENABLE(n) + offset;
      *bit = (uint32_t)1 << (n & 0x1f);
    }
  else
    {
      *regaddr = NVIC_SYSHCON;
      if (irq == AUTOC_IRQ_MEMFAULT)
        {
          *bit = NVIC_SYSHCON_MEMFAULTENA;
        }
      else if (irq == AUTOC_IRQ_BUSFAULT)
        {
          *bit = NVIC_SYSHCON_BUSFAULTENA;
        }
      else if (irq == AUTOC_IRQ_USAGEFAULT)
        {
          *bit = NVIC_SYSHCON_USGFAULTENA;
        }
      else if (irq == AUTOC_IRQ_SYSTICK)
        {
          *regaddr = NVIC_SYSTICK_CTRL;
          *bit = NVIC_SYSTICK_CTRL_ENABLE;
        }
      else
        {
          return ERROR;
        }
    }

  return OK;
}

static void autoc_prioritize_syscall(int priority)
{
  uint32_t regval;

  regval = getreg32(NVIC_SYSH8_11_PRIORITY);
  regval &= ~NVIC_SYSH_PRIORITY_PR11_MASK;
  regval |= (priority << NVIC_SYSH_PRIORITY_PR11_SHIFT);
  putreg32(regval, NVIC_SYSH8_11_PRIORITY);
}

void up_irqinitialize(void)
{
  uint32_t regaddr;
  int nprio;
  int i;

  for (i = 0; i < AUTOC_IRQ_NEXTINT; i += 32)
    {
      putreg32(0xffffffff, NVIC_IRQ_CLEAR(i));
    }

  putreg32((uint32_t)_vectors, NVIC_VECTAB);
  putreg32(DEFPRIORITY32, NVIC_SYSH4_7_PRIORITY);
  putreg32(DEFPRIORITY32, NVIC_SYSH8_11_PRIORITY);
  putreg32(DEFPRIORITY32, NVIC_SYSH12_15_PRIORITY);

  nprio = (getreg32(NVIC_ICTR) + 1) * 8;
  regaddr = NVIC_IRQ0_3_PRIORITY;
  while (nprio--)
    {
      putreg32(DEFPRIORITY32, regaddr);
      regaddr += 4;
    }

  irq_attach(AUTOC_IRQ_SVCALL, arm_svcall, NULL);
  irq_attach(AUTOC_IRQ_HARDFAULT, arm_hardfault, NULL);
  autoc_prioritize_syscall(NVIC_SYSH_SVCALL_PRIORITY);

#ifndef CONFIG_SUPPRESS_INTERRUPTS
  up_irq_enable();
#endif
}

void up_disable_irq(int irq)
{
  uintptr_t regaddr;
  uint32_t regval;
  uint32_t bit;

  if (autoc_irqinfo(irq, &regaddr, &bit, NVIC_CLRENA_OFFSET) == 0)
    {
      if (irq >= AUTOC_IRQ_EXINT)
        {
          putreg32(bit, regaddr);
        }
      else
        {
          regval = getreg32(regaddr);
          regval &= ~bit;
          putreg32(regval, regaddr);
        }
    }
}

void up_enable_irq(int irq)
{
  uintptr_t regaddr;
  uint32_t regval;
  uint32_t bit;

  if (autoc_irqinfo(irq, &regaddr, &bit, NVIC_ENA_OFFSET) == 0)
    {
      if (irq >= AUTOC_IRQ_EXINT)
        {
          putreg32(bit, regaddr);
        }
      else
        {
          regval = getreg32(regaddr);
          regval |= bit;
          putreg32(regval, regaddr);
        }
    }
}

void arm_ack_irq(int irq)
{
  UNUSED(irq);
}

#ifdef CONFIG_ARCH_IRQPRIO
int up_prioritize_irq(int irq, int priority)
{
  uint32_t regaddr;
  uint32_t regval;
  int shift;
  int local = irq;

  if (local < AUTOC_IRQ_EXINT)
    {
      regaddr = NVIC_SYSH_PRIORITY(local);
      local -= 4;
    }
  else
    {
      local -= AUTOC_IRQ_EXINT;
      regaddr = NVIC_IRQ_PRIORITY(local);
    }

  regval = getreg32(regaddr);
  shift = (local & 3) << 3;
  regval &= ~(0xff << shift);
  regval |= (priority << shift);
  putreg32(regval, regaddr);
  return OK;
}
#endif

void up_init_irq(int irq, int irq_prio)
{
#ifdef CONFIG_ARCH_IRQPRIO
  up_prioritize_irq(irq, irq_prio);
#else
  UNUSED(irq);
  UNUSED(irq_prio);
#endif
}

void up_clear_irq(int irq)
{
  uintptr_t regaddr;
  uint32_t bit;

  if (autoc_irqinfo(irq, &regaddr, &bit, NVIC_CLRPEND_OFFSET) == 0)
    {
      putreg32(bit, regaddr);
    }
}

#ifdef CONFIG_BMP
void up_affinity_irq(int irq, cpu_set_t cpuset)
{
  UNUSED(irq);
  UNUSED(cpuset);
}
#endif

bool up_irq_is_enabled(int irq)
{
  uintptr_t regaddr;
  uint32_t bit;

  if (autoc_irqinfo(irq, &regaddr, &bit, NVIC_ENA_OFFSET) == 0)
    {
      return (getreg32(regaddr) & bit) != 0;
    }

  return false;
}
