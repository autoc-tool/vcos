/*
 * Copyright (c) 2025 Li Auto Inc. and its affiliates
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/* Including File */
#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <time.h>

 typedef struct {
  uint32_t curtime;
  uint32_t oldtime;
  uint32_t gaptime;
  uint32_t nonidletime;
  uint32_t totaltime;
  uint32_t interval;
} CpuLdTimeType;

volatile static CpuLdTimeType cpuldtime[CONFIG_NR_CPUS] = { 0 };
volatile float cpuload[CONFIG_NR_CPUS] = { 0.0f };


static inline void nxsched_cpu_load_monitor(int cpuidx, clock_t switchtime)
{
  clock_t timertick = 0;
  up_timer_value(&timertick);
  cpuldtime[cpuidx].curtime = (uint32_t)timertick;

  if (cpuldtime[cpuidx].curtime > cpuldtime[cpuidx].oldtime)
    {
      cpuldtime[cpuidx].gaptime = cpuldtime[cpuidx].curtime - cpuldtime[cpuidx].oldtime;
    }
  else
    {
      cpuldtime[cpuidx].gaptime = (0xFFFFFFFFU - cpuldtime[cpuidx].oldtime) + cpuldtime[cpuidx].curtime + 1U;
    }

  /* Add 50% safety margin */

  if (cpuldtime[cpuidx].gaptime > (switchtime + (switchtime >> 1)))
    {
      cpuldtime[cpuidx].nonidletime += cpuldtime[cpuidx].gaptime - switchtime;
    }
  cpuldtime[cpuidx].totaltime += cpuldtime[cpuidx].gaptime;

  /* Record history */

  cpuldtime[cpuidx].oldtime = cpuldtime[cpuidx].curtime;  /* Record history */
}

float nxsched_get_cpuload(int cpuidx)
{
  return cpuload[cpuidx];
}

void nxsched_cpuload_update(void)
{
  int cpuidx = up_cpu_index();

  cpuldtime[cpuidx].interval++;
  if (cpuldtime[cpuidx].interval < CONFIG_CPULOAD_CALCULATE_INTERVAL)
    {
      return;
    }
  else
    {
      cpuldtime[cpuidx].interval = 0;
    }

  cpuload[cpuidx] = (float)(cpuldtime[cpuidx].nonidletime) / (float)(cpuldtime[cpuidx].totaltime);
  cpuldtime[cpuidx].totaltime = 0;
  cpuldtime[cpuidx].nonidletime = 0;
}

void nxsched_cpuload_idletask_infiniteloop(void)
{
  int cpuidx = up_cpu_index();
  clock_t starttick = 0;
  clock_t endtick = 0;

  /* Calculate the execution time of the function */

  irqstate_t flags = enter_critical_section();
  up_timer_value(&starttick);
  nxsched_cpu_load_monitor(cpuidx, 0);
  up_timer_value(&endtick);
  leave_critical_section(flags);

  /* Monitor CPU load by adjacent time intervals */

  while (1)
    {
      nxsched_cpu_load_monitor(cpuidx, (endtick - starttick));
    }
}
