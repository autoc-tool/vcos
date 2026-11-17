#include <nuttx/config.h>

#include <stdint.h>
#include <sys/types.h>

#include <nuttx/arch.h>
#include "arm_internal.h"

extern uint8_t _sram_end[];

#ifdef CONFIG_BMP
extern char _cpu_data_size[];
#endif

void up_allocate_heap(void **heap_start, size_t *heap_size)
{
#ifdef CONFIG_BMP
  uintptr_t base = (uintptr_t)g_idle_topstack;
  size_t stride = (uintptr_t)_cpu_data_size;
  size_t reserve = stride * (CONFIG_BMP_NCPUS - 1);
  size_t total = (uintptr_t)_sram_end - base - reserve;
  size_t each = total / CONFIG_BMP_NCPUS;

  *heap_size = each;
  *heap_start = (void *)(base + reserve + each * (uintptr_t)up_cpu_index());
#else
  *heap_start = (void *)g_idle_topstack;
  *heap_size = (size_t)((uintptr_t)_sram_end - g_idle_topstack);
#endif
}
