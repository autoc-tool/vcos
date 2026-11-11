#include <nuttx/config.h>
#include <nuttx/signal.h>
#include <nuttx/arch.h>
#include <arch/irq.h>
#include "Os.h"
#include "Os_Types.h"
#include "rtfw_ioc.h"
#include "rtfw_ipicall_internal.h"
#include "task_cfg.h"
#include "ioc_cfg.h"
#include "syslog.h"
#include "rtfw_compiler_cfg.h"

/*
    修改变量开启测试
    step1: 关闭trace(从rt_demo/system_cfg/a2g_tc397_5v_tft/user.config 中删除 CONFIG_TRACED=y
    step2: 切换到tasking 编译器编译构建（tasking 比 gcc 编译生成的镜像性能更好）
    step3: 通过调试器修改变量
    TEST_NOTHING = 0
    TEST_TASK_SWITCH = 1
    TEST_ISR_RESP = 2
    TEST_MULTI_CORE_API =3
    TEST_MULTI_CORE_IPC =4
    TEST_SERVICE_PROTECTION = 5
    TEST_PROTECTION_MEMORY = 6
    TEST_PROTECTION_EXCEPTION = 7
    使用方法：os_perf_test_type = TEST_TASK_SWITCH; 测试task切换性能,测试结果保存在os_test_task_switch_perf_ret中, 中位数结果os_test_task_switch_time
    使用方法：os_perf_test_type = TEST_ISR_RESP; 测试ISR响应性能,测试结果保存在os_test_isr_resp_perf_ret中，中位数结果 os_test_isr_resp_time
    使用方法：os_perf_test_type = TEST_MULTI_CORE_API; 测试多核API性能,测试结果保存在os_test_multi_core_api_perf_ret中，中位数结果 os_test_multi_core_api_time
    使用方法：os_perf_test_type = TEST_MULTI_CORE_IPC; 测试多核IPC性能,测试结果保存在os_test_multi_core_ipc_perf_ret中，中位数结果 os_test_multi_core_api_time
    使用方法：os_perf_test_type = TEST_SERVICE_PROTECTION; 测试OS服务接口保护, 测试结果在 error_hook_is_called 和 wait_event_return_value 变量中
    使用方法：os_perf_test_type = TEST_PROTECTION_MEMORY; 测试内存保护, 测试结果在 protection_error_type 中， 如果 protection_error_type == OS_STATUS_PROTECTION_MEMORY  测试PASS
    使用方法：os_perf_test_type = TEST_PROTECTION_EXCEPTION;测试 Os使用寄存器保护, 测试结果在 protection_error_type中， 如果 protection_error_type == E_OS_PROTECTION_EXCEPTION  测试PASS
    os_perf_test_type = TEST_NOTHING;仅用于初始化
    测量结果单位：tricore 平台上为 10ns
*/

/*
    命令行测试步骤：
        step1: 关闭trace(从rt_demo/system_cfg/a2g_tc397_5v_tft/user.config 中删除 CONFIG_TRACED=y
        step2: 切换到tasking 编译器编译构建（tasking 比 gcc 编译生成的镜像性能更好）
        step3: shell 中输入 os_perf_test 查看结果
*/

typedef unsigned char Ifx_UReg_8Bit;
typedef unsigned short Ifx_UReg_16Bit;
typedef unsigned int Ifx_UReg_32Bit;
typedef signed char Ifx_SReg_8Bit;
typedef signed short Ifx_SReg_16Bit;
typedef signed int Ifx_SReg_32Bit;

typedef struct _Ifx_SRC_SRCR_Bits {
	Ifx_UReg_32Bit SRPN : 8; /**< \brief [7:0] Service Request Priority Number (rw) */
	Ifx_UReg_32Bit reserved_8 : 2; /**< \brief [9:8] \internal Reserved */
	Ifx_UReg_32Bit SRE : 1; /**< \brief [10:10] Service Request Enable (rw) */
	Ifx_UReg_32Bit TOS : 3; /**< \brief [13:11] Type of Service Control (rw) */
	Ifx_UReg_32Bit reserved_14 : 2; /**< \brief [15:14] \internal Reserved */
	Ifx_UReg_32Bit ECC : 5; /**< \brief [20:16] Error Correction Code (rwh) */
	Ifx_UReg_32Bit reserved_21 : 3; /**< \brief [23:21] \internal Reserved */
	Ifx_UReg_32Bit SRR : 1; /**< \brief [24:24] Service Request Flag (rh) */
	Ifx_UReg_32Bit CLRR : 1; /**< \brief [25:25] Request Clear Bit (w) */
	Ifx_UReg_32Bit SETR : 1; /**< \brief [26:26] Request Set Bit (w) */
	Ifx_UReg_32Bit IOV : 1; /**< \brief [27:27] Interrupt Trigger Overflow Bit (rh) */
	Ifx_UReg_32Bit IOVCLR : 1; /**< \brief [28:28] Interrupt Trigger Overflow Clear Bit (w) */
	Ifx_UReg_32Bit SWS : 1; /**< \brief [29:29] SW Sticky Bit (rh) */
	Ifx_UReg_32Bit SWSCLR : 1; /**< \brief [30:30] SW Sticky Clear Bit (w) */
	Ifx_UReg_32Bit reserved_31 : 1; /**< \brief [31:31] \internal Reserved */
} Ifx_SRC_SRCR_Bits;

typedef union {
	Ifx_UReg_32Bit U; /**< \brief Unsigned access */
	Ifx_SReg_32Bit I; /**< \brief Signed access */
	Ifx_SRC_SRCR_Bits B; /**< \brief Bitfield access */
} Ifx_SRC_SRCR;

#define SRC_CPU_CPU0_SB /*lint --e(923, 9078)*/ (*(volatile Ifx_SRC_SRCR *)0xF0038000u)

#define IRQ_TO_SRCR(irq) (Ifx_SRC_SRCR *)((unsigned int)(&SRC_CPU_CPU0_SB) + (irq))

#define PERF_TEST_CNT 1000

#define IOC_U8_DATA (0x11)

#define IOC_U16_DATA (0x1111)

#define IOC_U32_DATA (0x11111111)

#define IOC_U64_DATA (0x1111111111111111)

typedef enum { INDEX0 = 0, INDEX1, INDEX2, INDEX3, INDEX_CNT } OsPerfIndexType;

typedef enum {
	TEST_NOTHING = 0,
	TEST_TASK_SWITCH = 1,
	TEST_ISR_RESP = 2,
	TEST_IOC_API = 3,
	TEST_MULTI_CORE_IOC = 4,
	TEST_MULTI_CORE_API = 5,
	TEST_MULTI_CORE_IPC = 6,
	TEST_SERVICE_PROTECTION = 7,
	TEST_PROTECTION_MEMORY = 8,
	TEST_PROTECTION_EXCEPTION = 9
} OsPerfTestType;

locate_code(OS_GLOBAL_BSS_SECTION) static uint8 os_perf_test_type;
locate_code(OS_GLOBAL_BSS_SECTION) os_status_t protection_error_type;
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_temp_rec[INDEX_CNT];
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_task_switch_perf_ret[PERF_TEST_CNT];
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_isr_resp_perf_ret[PERF_TEST_CNT];
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_ioc_api_perf_ret[PERF_TEST_CNT];
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_ioc_multi_api_perf_ret[PERF_TEST_CNT];
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_multi_core_api_perf_ret[PERF_TEST_CNT];
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_multi_core_ipc_perf_ret[PERF_TEST_CNT];
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_task_switch_time;
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_isr_resp_time;
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_ioc_api_time;
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_ioc_multi_api_time;
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_multi_core_api_time;
locate_code(OS_GLOBAL_BSS_SECTION) clock_t os_test_multi_core_ipc_time;
locate_code(OS_GLOBAL_BSS_SECTION) uint8_t os_perf_test_ioc_data;
locate_code(OS_GLOBAL_BSS_SECTION) uint8_t os_perf_test_ioc_multi_data;
locate_code(OS_GLOBAL_BSS_SECTION) uint32_t os_perf_test_index;
locate_code(OS_GLOBAL_BSS_SECTION) StatusType wait_event_return_value;
locate_code(OS_GLOBAL_BSS_SECTION) uint8 error_hook_is_called;

void os_task_core0_set1_kernel_mode_test_task(void *arg)
{
	if (os_perf_test_type == TEST_PROTECTION_MEMORY) {
		const os_task_cfg_t *task_cfg = os_task_cfg_ref_table[OS_TASK_CORE0_PERFTEST_SECOND_HIGHEST_TASK_ID];
		os_task_inst_t *task_inst = OS_TASK_GET_INSTANCE_REF(task_cfg);
		task_inst->thread.act_sem.semcount = 0; // 修改内核数据，触发TEST_PROTECTION_MEMORY保护异常
	}

	if (os_perf_test_type == TEST_PROTECTION_EXCEPTION) {
		uint32_t syscon = __mfcr(TC3XX_CPU_SYSCON);
		syscon &= ~SYSCON_PROTEN;
		__mtcr(TC3XX_CPU_SYSCON, syscon); // 修改系统寄存器，触发TEST_PROTECTION_EXCEPTION保护异常
	}

	os_perf_test_type = TEST_NOTHING; // 结束测试
	TerminateTask();
}

os_status_t os_ipicall_recv_ipc_test_function(os_ipicall_data_t *param)
{
	up_timer_value(&os_test_temp_rec[INDEX1]);
	up_timer_value(&os_test_temp_rec[INDEX2]);
	clock_t get_time_use = os_test_temp_rec[INDEX2] - os_test_temp_rec[INDEX1];
	os_test_multi_core_ipc_perf_ret[os_perf_test_index] =
		os_test_temp_rec[INDEX1] - os_test_temp_rec[INDEX0] - get_time_use;
	os_perf_test_index++;
	return OS_STATUS_OK;
}

void perf_test_isr_entry(void)
{
	up_timer_value(&os_test_temp_rec[INDEX2]);
	up_timer_value(&os_test_temp_rec[INDEX3]);
	clock_t get_time_use = os_test_temp_rec[INDEX3] - os_test_temp_rec[INDEX2]; // 时间戳接口需要的耗时
	clock_t isr_trigger_time =
		(os_test_temp_rec[INDEX1] - os_test_temp_rec[INDEX0] - get_time_use); // 触发 ISR 需要的时间
	os_test_isr_resp_perf_ret[os_perf_test_index] =
		os_test_temp_rec[INDEX2] - os_test_temp_rec[INDEX1] - get_time_use - isr_trigger_time;
	os_perf_test_index++;
}

static int perf_record_compare(const void *a, const void *b)
{
	uint64 va = *(const uint64 *)a;
	uint64 vb = *(const uint64 *)b;

	if (va < vb)
		return -1;
	else if (va > vb)
		return 1;
	else
		return 0;
}

void os_task_core0_perftest_highest_task(void *arg)
{
	up_timer_value(&os_test_temp_rec[INDEX1]);
	up_timer_value(&os_test_temp_rec[INDEX2]);
	clock_t get_time_use = os_test_temp_rec[INDEX2] - os_test_temp_rec[INDEX1];
	os_test_task_switch_perf_ret[os_perf_test_index] =
		os_test_temp_rec[INDEX1] - os_test_temp_rec[INDEX0] - get_time_use;
	os_perf_test_index++;
	(void)TerminateTask();
}

void os_task_core0_perftest_second_highest_task(void *arg)
{
	if (os_perf_test_index < PERF_TEST_CNT) {
		/* 开始测量 PERF_TEST_CNT 次 */
		switch (os_perf_test_type) {
			case TEST_TASK_SWITCH:
				/* 测试任务切换 */
				up_timer_value(&os_test_temp_rec[INDEX0]);
				ChainTask(OS_TASK_CORE0_PERFTEST_HIGHEST_TASK_ID);
				break;
			case TEST_ISR_RESP: {
				/*测试中断响应时间 */
				volatile Ifx_SRC_SRCR *src = IRQ_TO_SRCR(2484); // 固定使用2484 中断源进行测试
				up_timer_value(&os_test_temp_rec[INDEX0]);
				src->B.SETR = 0; // 获取触发中断的写寄存器需要的时间
				up_timer_value(&os_test_temp_rec[INDEX1]);
				src->B.SETR = 1; // 写寄存器触发中断
				task_tickdelay(1); // 保证中断已经得到响应
				break;
			}
			case TEST_IOC_API: {
				/* 测试同核 IOC API */
				os_perf_test_ioc_data = IOC_U8_DATA;
				up_timer_value(&os_test_temp_rec[INDEX0]);
				up_timer_value(&os_test_temp_rec[INDEX1]);
				IocWrite(OS_IOC_WRITE_OSIOCWRITE08PERFTEST_ID, &os_perf_test_ioc_data);
				up_timer_value(&os_test_temp_rec[INDEX2]);
				os_test_ioc_api_perf_ret[os_perf_test_index] =
					os_test_temp_rec[INDEX2] - os_test_temp_rec[INDEX1] - ((clock_t)(os_test_temp_rec[INDEX1] - os_test_temp_rec[INDEX0]));
				os_perf_test_index++;
				break;
			}
			case TEST_MULTI_CORE_IOC: {
				/* 测试跨核 IOC API */
				os_perf_test_ioc_multi_data = IOC_U8_DATA;
				up_timer_value(&os_test_temp_rec[INDEX0]);
				up_timer_value(&os_test_temp_rec[INDEX1]);
				IocWrite(OS_IOC_WRITE_OSIOCWRITE08PERFTESTMULTICORE_ID, &os_perf_test_ioc_multi_data);
				up_timer_value(&os_test_temp_rec[INDEX2]);
				os_test_ioc_multi_api_perf_ret[os_perf_test_index] =
					os_test_temp_rec[INDEX2] - os_test_temp_rec[INDEX1] - ((clock_t)(os_test_temp_rec[INDEX1] - os_test_temp_rec[INDEX0]));
				os_perf_test_index++;
				break;
			}
			case TEST_MULTI_CORE_API:
				/* 测试多核API */
				up_timer_value(&os_test_temp_rec[INDEX0]);
				up_timer_value(&os_test_temp_rec[INDEX1]);
				ActivateTask(OS_TASK_CORE1_PERFTEST_HIGHEST_TASK_ID);
				up_timer_value(&os_test_temp_rec[INDEX2]);
				clock_t get_time_use = os_test_temp_rec[INDEX1] - os_test_temp_rec[INDEX0];
				os_test_multi_core_api_perf_ret[os_perf_test_index] =
					os_test_temp_rec[INDEX2] - os_test_temp_rec[INDEX1] - get_time_use;
				os_perf_test_index++;
				break;
			case TEST_MULTI_CORE_IPC:
				/* 测试IPC */
				ActivateTask(OS_TASK_CORE1_PERFTEST_HIGHEST_TASK_ID);
				break;
			default:
				break;
		}
	} else if (os_perf_test_index == PERF_TEST_CNT) {
		switch (os_perf_test_type) {
			case TEST_TASK_SWITCH:
				/* 排序 */
				qsort(&os_test_task_switch_perf_ret[0], PERF_TEST_CNT, sizeof(uint64),
				      perf_record_compare);
				os_test_task_switch_time = os_test_task_switch_perf_ret[PERF_TEST_CNT / 2];
				os_perf_test_type = TEST_NOTHING; // 结束测试
				os_perf_test_index = 0;
				break;
			case TEST_ISR_RESP:
				/* 排序 */
				qsort(&os_test_isr_resp_perf_ret[0], PERF_TEST_CNT, sizeof(uint64),
				      perf_record_compare);
				os_test_isr_resp_time = os_test_isr_resp_perf_ret[PERF_TEST_CNT / 2];
				os_perf_test_type = TEST_NOTHING; // 结束测试
				os_perf_test_index = 0; // 结束测试
			case TEST_IOC_API:
				/* 排序 */
				qsort(&os_test_ioc_api_perf_ret[0], PERF_TEST_CNT, sizeof(uint64),
				      perf_record_compare);
				os_test_ioc_api_time = os_test_ioc_api_perf_ret[PERF_TEST_CNT / 2];
				os_perf_test_type = TEST_NOTHING; // 结束测试
				os_perf_test_index = 0; // 结束测试
			case TEST_MULTI_CORE_IOC:
				qsort(&os_test_ioc_multi_api_perf_ret[0], PERF_TEST_CNT, sizeof(uint64),
						perf_record_compare);
				os_test_ioc_multi_api_time = os_test_ioc_multi_api_perf_ret[PERF_TEST_CNT / 2];
				os_perf_test_type = TEST_NOTHING; // 结束测试
				os_perf_test_index = 0; // 结束测试
			case TEST_MULTI_CORE_API:
				/* 多核API测试结束 */
				qsort(&os_test_multi_core_api_perf_ret[0], PERF_TEST_CNT, sizeof(uint64),
				      perf_record_compare);
				os_test_multi_core_api_time = os_test_multi_core_api_perf_ret[PERF_TEST_CNT / 2];
				os_perf_test_type = TEST_NOTHING; // 结束测试
				os_perf_test_index = 0; // 结束测试
			case TEST_MULTI_CORE_IPC:
				/* 排序 */
				qsort(&os_test_multi_core_ipc_perf_ret[0], PERF_TEST_CNT, sizeof(uint64),
				      perf_record_compare);
				os_test_multi_core_ipc_time = os_test_multi_core_ipc_perf_ret[PERF_TEST_CNT / 2];
				os_perf_test_type = TEST_NOTHING; // 结束测试
				os_perf_test_index = 0;
				break;
			default:
				break;
		}
	}

	TerminateTask();
}

void os_task_core1_perftest_highest_task(void *arg)
{
	if (os_perf_test_type == TEST_MULTI_CORE_IPC) {
		/* 测试多核IPC */
		up_timer_value(&os_test_temp_rec[INDEX0]);
		os_ipicall_ipc_test_function(0, 0x12345678);
	}
	TerminateTask();
}

void activatetask_os_task_perftest_task(void)
{
	if ((os_perf_test_type > TEST_NOTHING) && (os_perf_test_type < TEST_SERVICE_PROTECTION)) {
		ActivateTask(OS_TASK_CORE0_PERFTEST_SECOND_HIGHEST_TASK_ID);
	}

	if (os_perf_test_type == TEST_SERVICE_PROTECTION) {
		error_hook_is_called = 0;
		wait_event_return_value = 0;
		DisableAllInterrupts(); // 禁止所有中断
		wait_event_return_value = WaitEvent(0x01); // 等待事件1
		EnableAllInterrupts(); // 允许所有中断
		os_perf_test_type = TEST_NOTHING; // 结束测试
	}

	if ((os_perf_test_type == TEST_PROTECTION_MEMORY) || (os_perf_test_type == TEST_PROTECTION_EXCEPTION)) {
		ActivateTask(OS_TASK_CORE0_SET1_KERNEL_MODE_TEST_TASK_ID); // 激活用户模式任务
	}
}

int main(int argc, FAR char *argv[])
{
	uint32 max_try_cnt = 10000; // 最大尝试次数
	os_perf_test_type = TEST_TASK_SWITCH;
	(void)task_tickdelay(1000);
	while ((os_perf_test_index != 0) || (os_perf_test_type != TEST_NOTHING)) {
		if (--max_try_cnt == 0) {
			break;
		}
		/* delay 1000us */
		task_tickdelay(1);
	}

	if ((os_perf_test_index == 0) && (os_perf_test_type == TEST_NOTHING)) {
		syslog(LOG_INFO, "Task switch time: %d ns\n",
		       (int)(os_test_task_switch_time * (1000000000 / up_timer_freq())));
	} else {
		syslog(LOG_ERR, "Task switch perf test failed, os_perf_test_index: %d, os_perf_test_type: %d\n",
		       (int)os_perf_test_index, (int)os_perf_test_type);
	}

	os_perf_test_type = TEST_ISR_RESP;
	max_try_cnt = 100000;
	task_tickdelay(1000); // 确保系统测试已开启
	while ((os_perf_test_index != 0) && (os_perf_test_type != TEST_NOTHING)) {
		if (--max_try_cnt == 0) {
			break;
		}
		/* delay 1000us */
		task_tickdelay(1);
	}

	if ((os_perf_test_index == 0) && (os_perf_test_type == TEST_NOTHING)) {
		syslog(LOG_INFO, "ISR response time: %d ns\n",
		       (int)((os_test_isr_resp_time) * (1000000000 / up_timer_freq())));
	} else {
		syslog(LOG_ERR, "ISR response perf test failed, os_perf_test_index: %d, os_perf_test_type: %d\n",
		       (int)os_perf_test_index, (int)os_perf_test_type);
	}

	os_perf_test_type = TEST_IOC_API;
	max_try_cnt = 100000;
	(void)task_tickdelay(1000);
	while ((os_perf_test_index != 0) || (os_perf_test_type != TEST_NOTHING)) {
		if (--max_try_cnt == 0) {
			break;
		}
		/* delay 1000us */
		task_tickdelay(1);
	}

	if ((os_perf_test_index == 0) && (os_perf_test_type == TEST_NOTHING)) {
		syslog(LOG_INFO, "IOC API time: %d ns\n",
		       (int)(os_test_ioc_api_time * (1000000000 / up_timer_freq())));
	} else {
		syslog(LOG_ERR, "IOC API perf test failed, os_perf_test_index: %d, os_perf_test_type: %d\n",
		       (int)os_perf_test_index, (int)os_perf_test_type);
	}

	os_perf_test_type = TEST_MULTI_CORE_IOC;
	max_try_cnt = 100000;
	(void)task_tickdelay(1000);
	while ((os_perf_test_index != 0) || (os_perf_test_type != TEST_NOTHING)) {
		if (--max_try_cnt == 0) {
			break;
		}
		/* delay 1000us */
		task_tickdelay(1);
	}

	if ((os_perf_test_index == 0) && (os_perf_test_type == TEST_NOTHING)) {
		syslog(LOG_INFO, "IOC MULTI API time: %d ns\n",
		       (int)(os_test_ioc_multi_api_time * (1000000000 / up_timer_freq())));
	} else {
		syslog(LOG_ERR, "IOC MULTI API perf test failed, os_perf_test_index: %d, os_perf_test_type: %d\n",
		       (int)os_perf_test_index, (int)os_perf_test_type);
	}

	os_perf_test_type = TEST_MULTI_CORE_API;
	max_try_cnt = 100000;
	task_tickdelay(1000); // 确保系统测试已开启
	while ((os_perf_test_index != 0) && (os_perf_test_type != TEST_NOTHING)) {
		if (--max_try_cnt == 0) {
			break;
		}
		/* delay 1000us */
		task_tickdelay(1);
	}

	if ((os_perf_test_index == 0) && (os_perf_test_type == TEST_NOTHING)) {
		syslog(LOG_INFO, "Multi-core API ActivateTask time: %d ns\n",
		       (int)(os_test_multi_core_api_time * (1000000000 / up_timer_freq())));
	} else {
		syslog(LOG_ERR,
		       "Multi-core API ActivateTask perf test failed, os_perf_test_index: %d, os_perf_test_type: %d\n",
		       (int)os_perf_test_index, (int)os_perf_test_type);
	}

	os_perf_test_type = TEST_MULTI_CORE_IPC;
	max_try_cnt = 100000;
	(void)task_tickdelay(1000);
	while ((os_perf_test_index != 0) || (os_perf_test_type != TEST_NOTHING)) {
		if (--max_try_cnt == 0) {
			break;
		}
		/* delay 1000us */
		task_tickdelay(1);
	}

	if ((os_perf_test_index == 0) && (os_perf_test_type == TEST_NOTHING)) {
		syslog(LOG_INFO, "Multi-core IPC time: %d ns\n",
		       (int)(os_test_task_switch_time * (1000000000 / up_timer_freq())));
	} else {
		syslog(LOG_ERR, "ulti-core IPC perf test failed, os_perf_test_index: %d, os_perf_test_type: %d\n",
		       (int)os_perf_test_index, (int)os_perf_test_type);
	}

	os_perf_test_type = TEST_SERVICE_PROTECTION;
	task_tickdelay(100); // delay 100ms 确保系统测试已开启

	if ((error_hook_is_called == 1) && (wait_event_return_value = E_OS_DISABLEDINT)) {
		syslog(LOG_INFO,
		       "os service protection test success!! waitevent return value: %d, error hook is called: %d \n",
		       wait_event_return_value, error_hook_is_called);
	} else {
		syslog(LOG_ERR,
		       "os service protection test failed!! waitevent return value: %d, error hook is called: %d \n",
		       wait_event_return_value, error_hook_is_called);
	}

	os_perf_test_type = TEST_PROTECTION_MEMORY;
	task_tickdelay(100); // delay 100ms 确保系统测试已开启

	os_perf_test_type = TEST_PROTECTION_EXCEPTION;
	task_tickdelay(100); // delay 100ms 确保系统测试已开启

	return 0;
}