#include <nuttx/config.h>

#include <stdint.h>
#include <sys/types.h>

#include <nuttx/arch.h>
#include "arm_internal.h"

extern uint8_t _sram_end[];

void up_allocate_heap(void **heap_start, size_t *heap_size)
{
  *heap_start = (void *)g_idle_topstack;
  *heap_size = (size_t)((uintptr_t)_sram_end - g_idle_topstack);
}
