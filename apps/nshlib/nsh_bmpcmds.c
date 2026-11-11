/****************************************************************************
 * apps/nshlib/nsh_bmpcmds.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdlib.h>
#include <nuttx/config.h>
#include <nuttx/arch.h>
#include "nsh.h"
#include "nsh_console.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: cmd_sc, switch to a cpu core
 ****************************************************************************/
#ifdef CONFIG_NSH_SC
int cmd_sc(FAR struct nsh_vtbl_s *vtbl, int argc, FAR char **argv)
{
  FAR char *endptr;
  int dst_core_id = 0;

  if (argc == 2)
    {
      dst_core_id = strtol(argv[1], &endptr, 0);
      if (*endptr != '\0')
          goto out;
      if ((dst_core_id >= 0) && (dst_core_id != up_cpu_index()) \
        && (dst_core_id < CONFIG_NR_CPUS))
        {
#ifdef CONFIG_NSH_CUSTOM_SC
          up_custom_sc(dst_core_id);
#else
          up_affinity_irq(CONFIG_NSH_UART_IRQ, 1 << dst_core_id);
#endif
          return OK;
        }
      else if(dst_core_id == up_cpu_index())
        {
          return OK;
        }
      else
        {
          nsh_output(vtbl, "sc %d failed.\n", dst_core_id);
          goto out;
        }
    }

out:
  nsh_output(vtbl, "Usage: sc <dst-core-id>. coreid range: %d - %d.\n", \
            0, CONFIG_NR_CPUS-1);
  return ERROR;
}
#endif
