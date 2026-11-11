/****************************************************************************
 * arch/tricore/src/tc3xx/tc3xx_timerisr.c
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

#include <nuttx/irq.h>
#include <nuttx/kmalloc.h>
#include <nuttx/hrtimer.h>
#include <nuttx/timers/oneshot.h>
#include <nuttx/timers/arch_alarm.h>

#include "tricore.h"

void up_timer_initialize(void)
{
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
	up_hrtimer_set_ops(tc3xx_hrtimer_initialize());
#elif defined(CONFIG_ONESHOT)
	up_alarm_set_lowerhalf(tc3xx_systimer_initialize());
#endif /* CONFIG_ONESHOT */
}
