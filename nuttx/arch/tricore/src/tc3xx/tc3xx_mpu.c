/*
 * Copyright (c) 2025 Li Auto Inc. and its affiliates
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <nuttx/irq.h>
#include <nuttx/sched.h>

static __percpu_bss uint8_t core_data_end;
#define core_data_end this_cpu_var(core_data_end)
static __percpu_bss uint8_t core_code_end;
#define core_code_end this_cpu_var(core_code_end)
static __percpu_bss uint8_t set0_data_end;
#define set0_data_end this_cpu_var(set0_data_end)
static __percpu_bss uint8_t set0_code_end;
#define set0_code_end this_cpu_var(set0_code_end)

const uint32_t tricore_psw_prs_regs[PSW_PRS_MAX] = { PSW_PRS_PS0, PSW_PRS_PS1, PSW_PRS_PS2,
						     PSW_PRS_PS3, PSW_PRS_PS4, PSW_PRS_PS5 };

const struct mpu_cfg_s empty_mpu_cfg = { .data_region_cnt = 0,
					 .code_region_cnt = 0,
					 .data_region_cfg = NULL,
					 .code_region_cfg = NULL };

const struct mpu_access_right_s empty_thread_access_right = { .set_num = 0,
							      .app_mpu_cfg = &empty_mpu_cfg,
							      .thread_mpu_cfg = &empty_mpu_cfg };

static inline void mpu_set_dpr_reg(uint8_t num, uint32_t base, uint32_t limit)
{
	switch (num) {
		case 0:
			__mtcr(TC3XX_CPU_DPR0_L, (base));
			__mtcr(TC3XX_CPU_DPR0_U, (limit));
			break;
		case 1:
			__mtcr(TC3XX_CPU_DPR1_L, (base));
			__mtcr(TC3XX_CPU_DPR1_U, (limit));
			break;
		case 2:
			__mtcr(TC3XX_CPU_DPR2_L, (base));
			__mtcr(TC3XX_CPU_DPR2_U, (limit));
			break;
		case 3:
			__mtcr(TC3XX_CPU_DPR3_L, (base));
			__mtcr(TC3XX_CPU_DPR3_U, (limit));
			break;
		case 4:
			__mtcr(TC3XX_CPU_DPR4_L, (base));
			__mtcr(TC3XX_CPU_DPR4_U, (limit));
			break;
		case 5:
			__mtcr(TC3XX_CPU_DPR5_L, (base));
			__mtcr(TC3XX_CPU_DPR5_U, (limit));
			break;
		case 6:
			__mtcr(TC3XX_CPU_DPR6_L, (base));
			__mtcr(TC3XX_CPU_DPR6_U, (limit));
			break;
		case 7:
			__mtcr(TC3XX_CPU_DPR7_L, (base));
			__mtcr(TC3XX_CPU_DPR7_U, (limit));
			break;
		case 8:
			__mtcr(TC3XX_CPU_DPR8_L, (base));
			__mtcr(TC3XX_CPU_DPR8_U, (limit));
			break;
		case 9:
			__mtcr(TC3XX_CPU_DPR9_L, (base));
			__mtcr(TC3XX_CPU_DPR9_U, (limit));
			break;
		case 10:
			__mtcr(TC3XX_CPU_DPR10_L, (base));
			__mtcr(TC3XX_CPU_DPR10_U, (limit));
			break;
		case 11:
			__mtcr(TC3XX_CPU_DPR11_L, (base));
			__mtcr(TC3XX_CPU_DPR11_U, (limit));
			break;
		case 12:
			__mtcr(TC3XX_CPU_DPR12_L, (base));
			__mtcr(TC3XX_CPU_DPR12_U, (limit));
			break;
		case 13:
			__mtcr(TC3XX_CPU_DPR13_L, (base));
			__mtcr(TC3XX_CPU_DPR13_U, (limit));
			break;
		case 14:
			__mtcr(TC3XX_CPU_DPR14_L, (base));
			__mtcr(TC3XX_CPU_DPR14_U, (limit));
			break;
		case 15:
			__mtcr(TC3XX_CPU_DPR15_L, (base));
			__mtcr(TC3XX_CPU_DPR15_U, (limit));
			break;
		case 16:
			__mtcr(TC3XX_CPU_DPR16_L, (base));
			__mtcr(TC3XX_CPU_DPR16_U, (limit));
			break;
		case 17:
			__mtcr(TC3XX_CPU_DPR17_L, (base));
			__mtcr(TC3XX_CPU_DPR17_U, (limit));
			break;
		default:
			/* do nothing */
			break;
	}
}

static inline void mpu_set_cpr_reg(uint8_t num, uint32_t base, uint32_t limit)
{
	switch (num) {
		case 0:
			__mtcr(TC3XX_CPU_CPR0_L, (base));
			__mtcr(TC3XX_CPU_CPR0_U, (limit));
			break;
		case 1:
			__mtcr(TC3XX_CPU_CPR1_L, (base));
			__mtcr(TC3XX_CPU_CPR1_U, (limit));
			break;
		case 2:
			__mtcr(TC3XX_CPU_CPR2_L, (base));
			__mtcr(TC3XX_CPU_CPR2_U, (limit));
			break;
		case 3:
			__mtcr(TC3XX_CPU_CPR3_L, (base));
			__mtcr(TC3XX_CPU_CPR3_U, (limit));
			break;
		case 4:
			__mtcr(TC3XX_CPU_CPR4_L, (base));
			__mtcr(TC3XX_CPU_CPR4_U, (limit));
			break;
		case 5:
			__mtcr(TC3XX_CPU_CPR5_L, (base));
			__mtcr(TC3XX_CPU_CPR5_U, (limit));
			break;
		case 6:
			__mtcr(TC3XX_CPU_CPR6_L, (base));
			__mtcr(TC3XX_CPU_CPR6_U, (limit));
			break;
		case 7:
			__mtcr(TC3XX_CPU_CPR7_L, (base));
			__mtcr(TC3XX_CPU_CPR7_U, (limit));
			break;
		case 8:
			__mtcr(TC3XX_CPU_CPR8_L, (base));
			__mtcr(TC3XX_CPU_CPR8_U, (limit));
			break;
		case 9:
			__mtcr(TC3XX_CPU_CPR9_L, (base));
			__mtcr(TC3XX_CPU_CPR9_U, (limit));
			break;
		default:
			/* do nothing */
			break;
	}
}

static inline void mpu_set_dpre_reg(uint8_t set_num, uint32_t data_read_enable)
{
	switch (set_num) {
		case 0:
			__mtcr(TC3XX_CPU_DPRE_0,
			       ((__mfcr(TC3XX_CPU_DPRE_0) & ~(0xffffffffUL << set0_data_end)) | data_read_enable));
			break;
		case 1:
			__mtcr(TC3XX_CPU_DPRE_1,
			       ((__mfcr(TC3XX_CPU_DPRE_1) & ~(0xffffffffUL << core_data_end)) | data_read_enable));
			break;
		case 2:
			__mtcr(TC3XX_CPU_DPRE_2,
			       ((__mfcr(TC3XX_CPU_DPRE_2) & ~(0xffffffffUL << core_data_end)) | data_read_enable));
			break;
		case 3:
			__mtcr(TC3XX_CPU_DPRE_3,
			       ((__mfcr(TC3XX_CPU_DPRE_3) & ~(0xffffffffUL << core_data_end)) | data_read_enable));
			break;
		case 4:
			__mtcr(TC3XX_CPU_DPRE_4,
			       ((__mfcr(TC3XX_CPU_DPRE_4) & ~(0xffffffffUL << core_data_end)) | data_read_enable));
			break;
		case 5:
			__mtcr(TC3XX_CPU_DPRE_5,
			       ((__mfcr(TC3XX_CPU_DPRE_5) & ~(0xffffffffUL << core_data_end)) | data_read_enable));
			break;
		default:
			/* do nothing */
			break;
	}
}

static inline void mpu_set_dpwe_reg(uint8_t set_num, uint32_t data_write_enable)
{
	switch (set_num) {
		case 0:
			__mtcr(TC3XX_CPU_DPWE_0,
			       ((__mfcr(TC3XX_CPU_DPWE_0) & ~(0xffffffffUL << set0_data_end)) | data_write_enable));
			break;
		case 1:
			__mtcr(TC3XX_CPU_DPWE_1,
			       ((__mfcr(TC3XX_CPU_DPWE_1) & ~(0xffffffffUL << core_data_end)) | data_write_enable));
			break;
		case 2:
			__mtcr(TC3XX_CPU_DPWE_2,
			       ((__mfcr(TC3XX_CPU_DPWE_2) & ~(0xffffffffUL << core_data_end)) | data_write_enable));
			break;
		case 3:
			__mtcr(TC3XX_CPU_DPWE_3,
			       ((__mfcr(TC3XX_CPU_DPWE_3) & ~(0xffffffffUL << core_data_end)) | data_write_enable));
			break;
		case 4:
			__mtcr(TC3XX_CPU_DPWE_4,
			       ((__mfcr(TC3XX_CPU_DPWE_4) & ~(0xffffffffUL << core_data_end)) | data_write_enable));
			break;
		case 5:
			__mtcr(TC3XX_CPU_DPWE_5,
			       ((__mfcr(TC3XX_CPU_DPWE_5) & ~(0xffffffffUL << core_data_end)) | data_write_enable));
			break;
		default:
			/* do nothing */
			break;
	}
}

static inline void mpu_set_cpxe_reg(uint8_t set_num, uint32_t code_execute_enable)
{
	switch (set_num) {
		case 0:
			__mtcr(TC3XX_CPU_CPXE_0,
			       ((__mfcr(TC3XX_CPU_CPXE_0) & ~(0xffffffffUL << set0_code_end)) | code_execute_enable));
			break;
		case 1:
			__mtcr(TC3XX_CPU_CPXE_1,
			       ((__mfcr(TC3XX_CPU_CPXE_1) & ~(0xffffffffUL << core_code_end)) | code_execute_enable));
			break;
		case 2:
			__mtcr(TC3XX_CPU_CPXE_2,
			       ((__mfcr(TC3XX_CPU_CPXE_2) & ~(0xffffffffUL << core_code_end)) | code_execute_enable));
			break;
		case 3:
			__mtcr(TC3XX_CPU_CPXE_3,
			       ((__mfcr(TC3XX_CPU_CPXE_3) & ~(0xffffffffUL << core_code_end)) | code_execute_enable));
			break;
		case 4:
			__mtcr(TC3XX_CPU_CPXE_4,
			       ((__mfcr(TC3XX_CPU_CPXE_4) & ~(0xffffffffUL << core_code_end)) | code_execute_enable));
			break;
		case 5:
			__mtcr(TC3XX_CPU_CPXE_5,
			       ((__mfcr(TC3XX_CPU_CPXE_5) & ~(0xffffffffUL << core_code_end)) | code_execute_enable));
			break;
		default:
			/* do nothing */
			break;
	}
}

static inline void mpu_set_data_region(uint8_t set_num, const struct mpu_cfg_s *app_mpu_cfg,
				       const struct mpu_cfg_s *thread_mpu_cfg)
{
	uint32_t read_enable = 0U;
	uint32_t write_enable = 0U;
	uint8_t i;
	uint8_t region_cnt;
	const struct mpu_region_cfg_s *region_cfg;

	region_cnt = app_mpu_cfg->data_region_cnt;
	region_cfg = app_mpu_cfg->data_region_cfg;
	for (i = 0U; i < region_cnt; i++) {
		mpu_set_dpr_reg(region_cfg[i].num, region_cfg[i].base, region_cfg[i].limit);
		if (0U != (region_cfg[i].attr & WRITE_ENABLE)) {
			write_enable |= (1U << region_cfg[i].num);
		}
		read_enable |= (1U << region_cfg[i].num);
	}
	region_cnt = thread_mpu_cfg->data_region_cnt;
	region_cfg = thread_mpu_cfg->data_region_cfg;
	for (i = 0U; i < region_cnt; i++) {
		mpu_set_dpr_reg(region_cfg[i].num, region_cfg[i].base, region_cfg[i].limit);
		if (0U != (region_cfg[i].attr & WRITE_ENABLE)) {
			write_enable |= (1U << region_cfg[i].num);
		}
		read_enable |= (1U << region_cfg[i].num);
	}
	if (0U != read_enable) {
		mpu_set_dpre_reg(set_num, read_enable);
	}
	if (0U != write_enable) {
		mpu_set_dpwe_reg(set_num, write_enable);
	}
}

static inline void mpu_set_code_region(uint8_t set_num, const struct mpu_cfg_s *app_mpu_cfg,
				       const struct mpu_cfg_s *thread_mpu_cfg)
{
	uint32_t execute_enable = 0U;
	uint8_t i;
	uint8_t region_cnt;
	const struct mpu_region_cfg_s *region_cfg;

	region_cnt = app_mpu_cfg->code_region_cnt;
	region_cfg = app_mpu_cfg->code_region_cfg;
	for (i = 0U; i < region_cnt; i++) {
		mpu_set_cpr_reg(region_cfg[i].num, region_cfg[i].base, region_cfg[i].limit);
		execute_enable |= (1U << region_cfg[i].num);
	}
	region_cnt = thread_mpu_cfg->code_region_cnt;
	region_cfg = thread_mpu_cfg->code_region_cfg;
	for (i = 0U; i < region_cnt; i++) {
		mpu_set_cpr_reg(region_cfg[i].num, region_cfg[i].base, region_cfg[i].limit);
		execute_enable |= (1U << region_cfg[i].num);
	}
	if (0U != execute_enable) {
		mpu_set_cpxe_reg(set_num, execute_enable);
	}
}

void up_mpu_enable(uint8_t enable)
{
	uint32_t syscon = __mfcr(TC3XX_CPU_SYSCON);

	if (1U == enable) {
		syscon |= SYSCON_PROTEN;
	} else {
		syscon &= ~SYSCON_PROTEN;
	}

	__mtcr(TC3XX_CPU_SYSCON, syscon);
}

int up_mpu_check_region_cfg(uint8_t region_cnt, const struct mpu_region_cfg_s *region_cfg, uint8_t max_region_num,
			    uint32_t alignment_mask)
{
	uint8_t i;

	for (i = 0U; i < region_cnt; i++) {
		if (region_cfg[i].num >= max_region_num) {
			return -1;
		}
		if (region_cfg[i].limit <= region_cfg[i].base) {
			return -2;
		}
		if (0U != (region_cfg[i].base & (~alignment_mask))) {
			return -3;
		}
		if (0U != (region_cfg[i].limit & (~alignment_mask))) {
			return -4;
		}
	}

	return 0;
}

int up_mpu_core_init(const struct mpu_core_access_right_s *core_access_right)
{
	int ret = 0;
	uint8_t i;

	up_mpu_enable(0U);
	ret = up_mpu_check_region_cfg(core_access_right->core_mpu_cfg->data_region_cnt,
				      core_access_right->core_mpu_cfg->data_region_cfg, DATA_REGION_MAX,
				      DATA_ALIGNMENT_MASK);
	if (0 != ret) {
		return ret;
	}
	ret = up_mpu_check_region_cfg(core_access_right->core_mpu_cfg->code_region_cnt,
				      core_access_right->core_mpu_cfg->code_region_cfg, CODE_REGION_MAX,
				      CODE_ALIGNMENT_MASK);
	if (0 != ret) {
		return ret;
	}
	ret = up_mpu_check_region_cfg(core_access_right->set0_mpu_cfg->data_region_cnt,
				      core_access_right->set0_mpu_cfg->data_region_cfg, DATA_REGION_MAX,
				      DATA_ALIGNMENT_MASK);
	if (0 != ret) {
		return ret;
	}
	ret = up_mpu_check_region_cfg(core_access_right->set0_mpu_cfg->code_region_cnt,
				      core_access_right->set0_mpu_cfg->code_region_cfg, CODE_REGION_MAX,
				      CODE_ALIGNMENT_MASK);
	if (0 != ret) {
		return ret;
	}
	mpu_set_data_region(0U, core_access_right->core_mpu_cfg, core_access_right->set0_mpu_cfg);
	mpu_set_code_region(0U, core_access_right->core_mpu_cfg, core_access_right->set0_mpu_cfg);
	for (i = 1U; i <= 5U; i++) {
		mpu_set_data_region(i, core_access_right->core_mpu_cfg, &empty_mpu_cfg);
		mpu_set_code_region(i, core_access_right->core_mpu_cfg, &empty_mpu_cfg);
	}
	core_data_end = core_access_right->core_mpu_cfg->data_region_cnt;
	core_code_end = core_access_right->core_mpu_cfg->code_region_cnt;
	set0_data_end =
		core_access_right->core_mpu_cfg->data_region_cnt + core_access_right->set0_mpu_cfg->data_region_cnt;
	set0_code_end =
		core_access_right->core_mpu_cfg->code_region_cnt + core_access_right->set0_mpu_cfg->code_region_cnt;
	up_mpu_enable(1U);

	return ret;
}

void up_mpu_enable_stack_region(uint32_t low_addr, uint32_t high_addr)
{
	mpu_set_dpr_reg(0U, low_addr, high_addr);
}

void up_mpu_switch_access_right(const struct mpu_access_right_s *current_access_right,
				const struct mpu_access_right_s *next_access_right)
{
	if (current_access_right == next_access_right) {
		return;
	}
	if ((0U != current_access_right->app_mpu_cfg->data_region_cnt) ||
	    (0U != current_access_right->thread_mpu_cfg->data_region_cnt)) {
		mpu_set_dpre_reg(current_access_right->set_num, 0);
		mpu_set_dpwe_reg(current_access_right->set_num, 0);
	}
	if ((0U != current_access_right->app_mpu_cfg->code_region_cnt) ||
	    (0U != current_access_right->thread_mpu_cfg->code_region_cnt)) {
		mpu_set_cpxe_reg(current_access_right->set_num, 0);
	}
	mpu_set_data_region(next_access_right->set_num, next_access_right->app_mpu_cfg,
			    next_access_right->thread_mpu_cfg);
	mpu_set_code_region(next_access_right->set_num, next_access_right->app_mpu_cfg,
			    next_access_right->thread_mpu_cfg);
}

void up_mpu_switch(void *rtcb, void *tcb)
{
	const struct mpu_access_right_s *cur_access_right, *next_access_right;
	uintptr_t *cur_context, *next_context;
	struct tcb_s *cur_tcb = (struct tcb_s *)rtcb;
	struct tcb_s *next_tcb = (struct tcb_s *)tcb;

#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION == 1)
	if (NULL == next_tcb) {
		return;
	}
	if (NULL == cur_tcb) {
		cur_context = (uintptr_t *)up_get_contexthdl();
	} else {
		cur_context = cur_tcb->contexthdl;
	}
	next_context = next_tcb->contexthdl;
	if (NULL != cur_context) {
		cur_access_right = (struct mpu_access_right_s *)(*(uintptr_t *)(*cur_context));
	} else {
		cur_access_right = &empty_thread_access_right;
	}
	if (NULL != next_context) {
		next_access_right = (struct mpu_access_right_s *)(*(uintptr_t *)(*next_context));
	} else {
		next_access_right = &empty_thread_access_right;
	}
	up_mpu_switch_access_right(cur_access_right, next_access_right);
	if (NULL != next_context) {
		up_mpu_enable_stack_region((uint32_t)(next_tcb->stack_alloc_ptr),
					   (uint32_t)(next_tcb->stack_base_ptr) + next_tcb->adj_stack_size);
	}
#else
	if ((NULL != cur_tcb) && (NULL != cur_tcb->mpu_access_right)) {
		cur_access_right = cur_tcb->mpu_access_right;
	} else {
		cur_access_right = &empty_thread_access_right;
	}
	if ((NULL != next_tcb) && (NULL != next_tcb->mpu_access_right)) {
		next_access_right = tcb->mpu_access_right;
	} else {
		next_access_right = &empty_thread_access_right;
	}
	up_mpu_switch_access_right(cur_access_right, next_access_right);
	if ((NULL != next_tcb) && (0U != next_tcb->mpu_stack_protection_flag)) {
		up_mpu_enable_stack_region((uint32_t)(next_tcb->stack_alloc_ptr),
					   (uint32_t)(next_tcb->stack_base_ptr) + next_tcb->adj_stack_size);
	} else {
		up_mpu_enable_stack_region(0x0UL, 0x8UL);
	}
#endif
}
