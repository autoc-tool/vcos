#include <nuttx/config.h>

#include <inttypes.h>
#include <nuttx/clock.h>
#include <nuttx/compiler.h>
#include <nuttx/sched.h>
#include "rtfw_compiler_cfg.h"
#include "Os.h"
#include "Os_Types.h"
#include "rtfw_ipicall_internal.h"
#include "task_cfg.h"
#include "isr_cfg.h"
#include "syslog.h"
#include <nuttx/irq.h>
#include "core_cfg.h"

extern uint32_t up_timer_freq(void);
extern int up_timer_value(clock_t *ticks);
extern void activatetask_os_task_perftest_task(void);

/* Public function declarations */
void manual_calibration_test(void);

/* --- Manual Calibration --- */
/* !! IMPORTANT !! YOU MUST MANUALLY CALIBRATE THIS VALUE FOR YOUR TARGET HARDWARE. */
/* This value represents the number of loop iterations needed to consume 1 microsecond. */
/* A value that is too high will cause task overruns and system instability. */
#ifdef CONFIG_ARCH_CHIP_TC397
#define MANUALLY_CALIBRATED_LOOPS_PER_US 5.8
#elif defined(CONFIG_ARCH_CHIP_E3650)
#define MANUALLY_CALIBRATED_LOOPS_PER_US 12.96
#elif defined(CONFIG_ARCH_CHIP_J6E)
#define MANUALLY_CALIBRATED_LOOPS_PER_US 73.8
#else
#define MANUALLY_CALIBRATED_LOOPS_PER_US 5.8  /* Default value */
#endif

#define SEED_INIT_VALUE     17
#define LCG_MULTIPLIER      69069
#define LCG_INCREMENT       1
#define SAMPLE_COUNT        12
#define MAX_DEVIATION       25
#define SMOOTHING_FACTOR    8

#define GLOBAL_TEST_BSS         locate_data(OS_GLOBAL_BSS_SECTION)

static volatile GLOBAL_TEST_BSS uint32_t g_dummy_sink[OS_CORE_CNT];

typedef struct {
    uint32_t seed;
    uint32_t last_value;
    uint32_t call_count;
} random_state_t;

static GLOBAL_TEST_BSS random_state_t g_random_state[OS_CORE_CNT];

/**
 * A predictable, work-consuming delay loop based on arithmetic operations.
 * This simulates CPU workload and is resistant to compiler optimization.
 *
 * @param loop_count - Number of loop iterations to execute.
 */
static inline void delay_loop(uint32_t loop_count)
{
    volatile uint32_t i;
    volatile uint32_t temp = 0;
    uint32_t cpu_id = (uint32_t)GetCoreID();

    if (cpu_id >= OS_CORE_CNT) {
        cpu_id = 0;
    }

    for (i = 0; i < loop_count; i++) {
        temp += (i ^ 0x55); /* XOR and ADD */
        temp *= 3;          /* MULTIPLY */
        temp &= 0xFFFFFFFF; /* AND mask */

        /* Prevent optimization by writing to a core-specific volatile sink periodically. */
        if ((i & 0xF) == 0) {
            g_dummy_sink[cpu_id] = temp;
        }
    }
}

/**
 * Provides a CPU cycle-based delay for a specified number of microseconds.
 * This function relies on the manually calibrated `MANUALLY_CALIBRATED_LOOPS_PER_US` value.
 *
 * @param us - Delay time in microseconds.
 */
static inline void cpu_cycle_delay(uint32_t us)
{
    uint64_t total_loops;

    if (us == 0) {
        return;
    }

    /* Calculate the total number of loops required for the delay. */
    total_loops = (uint64_t)us * MANUALLY_CALIBRATED_LOOPS_PER_US;

    delay_loop(total_loops);
}

/**
 * Generates a random delay time with improved variation
 * Uses Xorshift algorithm for better randomness and ±30% variation range
 *
 * @param mean_us - Mean delay time in microseconds
 * @return Random delay time with ±30% variation around mean value (50% ~ 150% of mean)
 */
uint32_t random_normal_delay(uint32_t mean_us)
{
    uint32_t cpu_id = (uint32_t)GetCoreID();
    random_state_t *state;
    uint32_t random_us;
    clock_t current_time;
    irqstate_t flags;

    if (cpu_id >= OS_CORE_CNT) {
        cpu_id = 0;
    }

    state = &g_random_state[cpu_id];

    flags = up_irq_save();

    /* Initialize seed if needed */
    if (state->seed == 0) {
        up_timer_value(&current_time);
        state->seed = (uint32_t)current_time + cpu_id * 1000 + mean_us;
        if (state->seed == 0) {
            state->seed = SEED_INIT_VALUE + cpu_id * 1000 + mean_us;
        }
    }

    state->call_count++;

    /* 每次调用都更新种子，增加随机性 */
    up_timer_value(&current_time);
    state->seed ^= (uint32_t)current_time + state->call_count;

    /* 使用更好的随机数生成算法 - Xorshift */
    state->seed ^= state->seed << 13;
    state->seed ^= state->seed >> 17;
    state->seed ^= state->seed << 5;

    /* 生成 -30% 到 +30% 的随机偏差 */
    uint32_t deviation_range = mean_us * 60 / 100;  /* 60% 的总范围 (±30%) */
    uint32_t deviation = state->seed % deviation_range;

    /* 将偏差从 0~60% 转换为 -30%~+30% */
    int32_t signed_deviation = (int32_t)deviation - (int32_t)(deviation_range / 2);

    int32_t result = (int32_t)mean_us + signed_deviation;

    /* 限制结果在合理范围内：50% ~ 150% 的平均值 */
    if (result < (int32_t)(mean_us / 2)) {
        result = mean_us / 2;  /* 最小值为平均值的50% */
    } else if (result > (int32_t)(mean_us * 3 / 2)) {
        result = mean_us * 3 / 2;  /* 最大值为平均值的150% */
    }

    random_us = (uint32_t)result;

    if (state->last_value != 0 && state->call_count < 3) {
        /* 只对前3次调用进行轻微平滑 */
        random_us = (random_us * 3 + state->last_value) / 4;
    }

    state->last_value = random_us;

    up_irq_restore(flags);

    return random_us;
}

#ifndef CONFIG_ARCH_TRICORE
weak_function void os_task_core0_set1_kernel_mode_test_task(void *arg)
{

}
weak_function void os_task_core0_perftest_second_highest_task(void *arg)
{

}

weak_function void os_task_core0_perftest_highest_task(void *arg)
{

}

weak_function void os_task_core1_perftest_highest_task(void *arg)
{

}

weak_function void os_task_default_tcpip_task(void *arg)
{

}

weak_function void perf_test_isr_entry(void)
{

}

weak_function void activatetask_os_task_perftest_task(void)
{

}

weak_function void ethisr6_isr(void)
{

}

weak_function void ethisr2_isr(void)
{

}
weak_function os_status_t os_ipicall_recv_ipc_test_function(os_ipicall_data_t *param)
{
    return 0;
}

weak_function void core0_cat1_test_isr_entry(void)
{

}
#endif

/*Runnables define*/
void core0_demo_runnable_1ms(void)
{
#ifdef CONFIG_ARCH_TRICORE
    activatetask_os_task_perftest_task();
#endif
}
void core0_demo_runnable_5ms(void)
{

}
void core0_demo_runnable_10ms(void)
{

}
void core0_bswapp_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(7));
}
void core0_asil_bsw_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(13));
}
void core0_bswapp_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(9));
}
void core0_bsw_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(7));
}
void core0_asil_bsw_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(8));
}
void core0_actuatorin_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(9));
}
void core0_actuatorout_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(14));
}
void core0_bswapp_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(11));
}
void core0_bsw_runnable_10ms(void)
{
#ifdef CONFIG_CPULOAD_MONITOR_IN_IDLE
    nxsched_cpuload_update();
#endif
    cpu_cycle_delay(random_normal_delay(12));
}
void core0_bsw_runnable_10ms_lowpriority(void)
{
    cpu_cycle_delay(random_normal_delay(14));
}
void core1_asilb_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(3));
}
void core1_actuatorin_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(4));
}
void core1_actuatorout_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(4));
}
void core1_bswapp_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(2));
}
void core1_bswapp_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(5));
}
void core1_asilb_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(11));
}
void core1_asild_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(12));
}
void core1_actuatorin_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(10));
}
void core1_actuatorout_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(14));
}
void core1_bswapp_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(5));
}
void core1_bsw_runnable_10ms(void)
{
#ifdef CONFIG_CPULOAD_MONITOR_IN_IDLE
    nxsched_cpuload_update();
#endif
    cpu_cycle_delay(random_normal_delay(10));
}
void core1_nmm_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(10));
}
void core2_bsw_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(7));
}
void core2_bsw_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(12));
}
void core2_bsw_runnable_10ms(void)
{
#ifdef CONFIG_CPULOAD_MONITOR_IN_IDLE
    nxsched_cpuload_update();
#endif
    cpu_cycle_delay(random_normal_delay(16));
}
void core3_bsw_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(9));
}
void core3_bsw_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(11));
}
void core3_bsw_runnable_10ms(void)
{
#ifdef CONFIG_CPULOAD_MONITOR_IN_IDLE
    nxsched_cpuload_update();
#endif
    cpu_cycle_delay(random_normal_delay(10));
}
void core4_bsw_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(7));
}
void core4_bsw_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(12));
}
void core4_bsw_runnable_10ms(void)
{
#ifdef CONFIG_CPULOAD_MONITOR_IN_IDLE
    nxsched_cpuload_update();
#endif
    cpu_cycle_delay(random_normal_delay(36));
}
void core5_bsw_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(7));
}
void core5_bsw_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(12));
}
void core5_bsw_runnable_10ms(void)
{
#ifdef CONFIG_CPULOAD_MONITOR_IN_IDLE
    nxsched_cpuload_update();
#endif
    cpu_cycle_delay(random_normal_delay(36));
}
void NETIO_Autosar_TcpIp_udp_rx_indication(void)
{

}
void core0_bsw_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(17));
}
void core0_bswapp_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(16));
}
void core0_bsw_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(16));
}
void core0_bswapp_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(26));
}
void core0_bswapp_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(11));
}
void core0_bsw_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(10));
}
void core1_bsw_qm_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bsw_qm_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(21));
}
void core1_bsw_qm_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bsw_qm_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bswapp_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bsw_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_asilb_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_asild_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bswapp_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bsw_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_asilb_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_asild_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bswapp_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_bsw_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_asilb_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core1_asild_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
}
void core2_bswapp_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(7));
}
void core2_bswapp_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(12));
}
void core2_bswapp_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(12));
}
void core2_bswapp_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core2_bsw_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core2_bswapp_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core2_bswapp_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core3_bswapp_runnable_1ms(void)
{
    cpu_cycle_delay(random_normal_delay(15));
}
void core3_bswapp_runnable_5ms(void)
{
    cpu_cycle_delay(random_normal_delay(21));
}
void core3_bswapp_runnable_10ms(void)
{
    cpu_cycle_delay(random_normal_delay(19));
}
void core3_bswapp_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core3_bsw_runnable_20ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core3_bswapp_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core3_bsw_runnable_50ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core3_bswapp_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}
void core3_bsw_runnable_100ms(void)
{
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
    cpu_cycle_delay(random_normal_delay(800));
}

/**
 * @brief Manual calibration test function.
 *
 * This function measures the actual execution time of `cpu_cycle_delay(100)`
 * and prints the result, allowing for manual tuning of the
 * `MANUALLY_CALIBRATED_LOOPS_PER_US` macro.
 */
void manual_calibration_test(void)
{
    clock_t start_time, end_time;
    uint64_t elapsed_ticks;
    uint64_t elapsed_ns;
    uint32_t timer_freq;

    irqstate_t flags;

    syslog(LOG_INFO, "--- Manual Calibration Test ---\n");
    syslog(LOG_INFO, "Current MANUALLY_CALIBRATED_LOOPS_PER_US: %d.%02d\n",
           (int)MANUALLY_CALIBRATED_LOOPS_PER_US,
           (int)((MANUALLY_CALIBRATED_LOOPS_PER_US * 100)) % 100);

    timer_freq = up_timer_freq();
    if (timer_freq == 0) {
        syslog(LOG_ERR, "ERROR: Timer frequency is 0. Cannot perform calibration test.\n");
        return;
    }

    /* --- Critical Section Start --- */
    flags = up_irq_save();

    /* Measure the time for a 100us delay to get a more stable reading */
    up_timer_value(&start_time);
    cpu_cycle_delay(100);
    up_timer_value(&end_time);

    /* --- Critical Section End --- */
    up_irq_restore(flags);

    elapsed_ticks = (uint64_t)end_time - (uint64_t)start_time;

    /* Convert ticks to nanoseconds */
    elapsed_ns = (elapsed_ticks * 1000000000) / timer_freq;

    syslog(LOG_INFO, "cpu_cycle_delay(100) took %" PRIu64 " ns\n", elapsed_ns);
    syslog(LOG_INFO, "Average time per microsecond: %" PRIu64 " ns\n", elapsed_ns / 100);
    syslog(LOG_INFO, "Goal is 1000 ns per microsecond.\n");
    syslog(LOG_INFO, "---------------------------------\n");
}
