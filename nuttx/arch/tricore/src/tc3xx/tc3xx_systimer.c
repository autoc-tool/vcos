/****************************************************************************
 * arch/tricore/src/tc3xx/tc3xx_systimer.c
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

#include "tricore_internal.h"
#include "tricore.h"
#include "IfxStm.h"

#define SCU_FREQUENCY 100000000UL
#define STM0_SRC_NUM 768
#define STM1_SRC_NUM 776
#define STM2_SRC_NUM 784
#define STM3_SRC_NUM 792
#define STM4_SRC_NUM 800
#define STM5_SRC_NUM 808

struct systimer_map_s {
	volatile void *timer;
	int irq;
};

struct tc3xx_systimer_lowerhalf_s {
	struct oneshot_lowerhalf_s lower;
	volatile void *tbase;
	uint64_t freq;
	uint64_t alarm;
	oneshot_callback_t callback;
	void *arg;
};

static int tc3xx_systimer_max_delay(struct oneshot_lowerhalf_s *lower, struct timespec *ts);
static int tc3xx_systimer_start(struct oneshot_lowerhalf_s *lower, oneshot_callback_t callback, void *arg,
				const struct timespec *ts);
static int tc3xx_systimer_cancel(struct oneshot_lowerhalf_s *lower, struct timespec *ts);
static int tc3xx_systimer_current(struct oneshot_lowerhalf_s *lower, struct timespec *ts);

static clock_t tc3xx_hrtimer_current(struct hrtimer_queue_s *queue);
static void tc3xx_hrtimer_set_compare(struct hrtimer_queue_s *queue, clock_t value);
static void tc3xx_hrtimer_start(struct hrtimer_queue_s *queue);
static void tc3xx_hrtimer_trigger(struct hrtimer_queue_s *queue);

static const struct oneshot_operations_s g_tc3xx_systimer_ops = {
	.max_delay = tc3xx_systimer_max_delay,
	.start = tc3xx_systimer_start,
	.cancel = tc3xx_systimer_cancel,
	.current = tc3xx_systimer_current,
};

static const struct hrtimer_ops_s g_tc3xx_hrtimer_ops = {
	.current = tc3xx_hrtimer_current,
	.set_expire = tc3xx_hrtimer_set_compare,
	.start = tc3xx_hrtimer_start,
	.trigger = tc3xx_hrtimer_trigger,
};

FAR __percpu_bss struct tc3xx_systimer_lowerhalf_s *g_tc3xx_systimer_lower_ops;
#define g_tc3xx_systimer_lower_ops this_cpu_var(g_tc3xx_systimer_lower_ops)

static const struct systimer_map_s stm_maps[] = {
	{ &MODULE_STM0, STM0_SRC_NUM }, { &MODULE_STM1, STM1_SRC_NUM }, { &MODULE_STM2, STM2_SRC_NUM },
	{ &MODULE_STM3, STM3_SRC_NUM }, { &MODULE_STM4, STM4_SRC_NUM }, { &MODULE_STM5, STM5_SRC_NUM },
};

static clock_t tc3xx_hrtimer_current(struct hrtimer_queue_s *queue)
{
	(void)queue;
	irqstate_t flags;
	uint64_t ticks;

	flags = enter_critical_section();
	ticks = IfxStm_get(stm_maps[up_cpu_index()].timer);
	leave_critical_section(flags);

	return ticks;
}
static void tc3xx_hrtimer_set_compare(struct hrtimer_queue_s *queue, clock_t value)
{
	irqstate_t flags;

	flags = enter_critical_section();
	IfxStm_updateCompare(stm_maps[up_cpu_index()].timer, IfxStm_Comparator_0, value);
	leave_critical_section(flags);
}

static void tc3xx_hrtimer_start(struct hrtimer_queue_s *queue)
{
	(void)queue;

	up_enable_irq(stm_maps[up_cpu_index()].irq);
}

static void tc3xx_hrtimer_trigger(struct hrtimer_queue_s *queue)
{
	(void)queue;

	up_trigger_irq(stm_maps[up_cpu_index()].irq, 0);
}

static uint64_t tc3xx_systimer_get_time(struct tc3xx_systimer_lowerhalf_s *priv)
{
	irqstate_t flags;
	uint64_t ticks;

	flags = enter_critical_section();

	ticks = IfxStm_get(priv->tbase);

	leave_critical_section(flags);

	return ticks;
}

static void tc3xx_systimer_set_timecmp(const struct tc3xx_systimer_lowerhalf_s *priv, uint64_t value)
{
	irqstate_t flags;

	flags = enter_critical_section();

	IfxStm_updateCompare(priv->tbase, IfxStm_Comparator_0, value);

	leave_critical_section(flags);
}

static int tc3xx_systimer_max_delay(struct oneshot_lowerhalf_s *lower, struct timespec *ts)
{
	ts->tv_sec = UINT32_MAX;
	ts->tv_nsec = NSEC_PER_SEC - 1;

	return 0;
}

static int tc3xx_systimer_start(struct oneshot_lowerhalf_s *lower, oneshot_callback_t callback, void *arg,
				const struct timespec *ts)
{
	struct tc3xx_systimer_lowerhalf_s *priv = (struct tc3xx_systimer_lowerhalf_s *)lower;
	uint64_t mtime = tc3xx_systimer_get_time(priv);

	priv->alarm = mtime + ts->tv_sec * priv->freq + ts->tv_nsec * priv->freq / NSEC_PER_SEC;
	if (priv->alarm < mtime) {
		priv->alarm = UINT64_MAX;
	}

	priv->callback = callback;
	priv->arg = arg;

	tc3xx_systimer_set_timecmp(priv, priv->alarm);

	return 0;
}

static int tc3xx_systimer_cancel(struct oneshot_lowerhalf_s *lower, struct timespec *ts)
{
	struct tc3xx_systimer_lowerhalf_s *priv = (struct tc3xx_systimer_lowerhalf_s *)lower;
	uint64_t mtime;

	tc3xx_systimer_set_timecmp(priv, UINT64_MAX);

	mtime = tc3xx_systimer_get_time(priv);
	if (priv->alarm > mtime) {
		uint64_t nsec = (priv->alarm - mtime) * NSEC_PER_SEC / priv->freq;

		ts->tv_sec = nsec / NSEC_PER_SEC;
		ts->tv_nsec = nsec % NSEC_PER_SEC;
	} else {
		ts->tv_sec = 0;
		ts->tv_nsec = 0;
	}

	priv->alarm = 0;
	priv->callback = NULL;
	priv->arg = NULL;

	return 0;
}

static int tc3xx_systimer_current(struct oneshot_lowerhalf_s *lower, struct timespec *ts)
{
	struct tc3xx_systimer_lowerhalf_s *priv = (struct tc3xx_systimer_lowerhalf_s *)lower;
	uint64_t mtime = tc3xx_systimer_get_time(priv);
	uint64_t nsec = mtime / (priv->freq / USEC_PER_SEC) * NSEC_PER_USEC;

	ts->tv_sec = nsec / NSEC_PER_SEC;
	ts->tv_nsec = nsec % NSEC_PER_SEC;

	return 0;
}

#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
void up_systick_isr_handler(void *arg)
{
	struct tc3xx_systimer_lowerhalf_s *priv = g_tc3xx_systimer_lower_ops;

	tc3xx_systimer_set_timecmp(priv, UINT64_MAX);
	if (priv->callback != NULL) {
		priv->callback(&priv->lower, priv->arg);
	}
}
#else
static int tc3xx_systimer_interrupt(int irq, void *context, void *arg)
{
	struct tc3xx_systimer_lowerhalf_s *priv = arg;

	tc3xx_systimer_set_timecmp(priv, UINT64_MAX);
	if (priv->callback != NULL) {
		priv->callback(&priv->lower, priv->arg);
	}

	return 0;
}
#endif

struct oneshot_lowerhalf_s *tc3xx_systimer_initialize(void)
{
	struct tc3xx_systimer_lowerhalf_s *priv;

	priv = kmm_malloc(sizeof(*priv));
	if (priv == NULL) {
		return NULL;
	}

	g_tc3xx_systimer_lower_ops = priv;
	priv->tbase = stm_maps[up_cpu_index()].timer;
	priv->freq = SCU_FREQUENCY;
	priv->lower.ops = &g_tc3xx_systimer_ops;

	IfxStm_setCompareControl(priv->tbase, IfxStm_Comparator_0, IfxStm_ComparatorOffset_0,
				 IfxStm_ComparatorSize_32Bits, IfxStm_ComparatorInterrupt_ir0);

	IfxStm_clearCompareFlag(priv->tbase, IfxStm_Comparator_0);
	tc3xx_systimer_set_timecmp(priv, UINT64_MAX);
	IfxStm_enableComparatorInterrupt(priv->tbase, IfxStm_Comparator_0);
#if !defined(CONFIG_RT_FRAMEWORK) || (CONFIG_RT_FRAMEWORK == 0)
	irq_attach(stm_maps[up_cpu_index()].irq / 4, tc3xx_systimer_interrupt, priv);
	up_enable_irq(stm_maps[up_cpu_index()].irq);
#endif
	return (struct oneshot_lowerhalf_s *)priv;
}

const struct hrtimer_ops_s *tc3xx_hrtimer_initialize(void)
{
	irqstate_t flags;
	volatile void *vbase = stm_maps[up_cpu_index()].timer;

	IfxStm_setCompareControl(vbase, IfxStm_Comparator_0, IfxStm_ComparatorOffset_0, IfxStm_ComparatorSize_32Bits,
				 IfxStm_ComparatorInterrupt_ir0);

	IfxStm_clearCompareFlag(vbase, IfxStm_Comparator_0);

	flags = enter_critical_section();
	IfxStm_updateCompare(vbase, IfxStm_Comparator_0, UINT32_MAX);
	leave_critical_section(flags);
	IfxStm_enableComparatorInterrupt(vbase, IfxStm_Comparator_0);
	return &g_tc3xx_hrtimer_ops;
}
