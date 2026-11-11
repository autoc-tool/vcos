#include <nuttx/config.h>
#include <stdint.h>

#include "arm_internal.h"

#define AUTOC_COREID_ADDR 0x40000000

int up_cpu_index(void)
{
  return (int)getreg32(AUTOC_COREID_ADDR);
}
