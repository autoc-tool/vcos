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

const uint8_t memory_attribute[MEMATTR_TYPE_MAX] =
{
	[MEMATTR_TYPE_00_WRITE_BACK_WRTIE_ALLOCATE_CACHEABLE] = MEMATTR_WRITE_BACK_WRTIE_ALLOCATE_CACHEABLE,
	[MEMATTR_TYPE_01_WRITE_THROUGH_CACHEABLE] = MEMATTR_WRITE_THROUGH_CACHEABLE,
	[MEMATTR_TYPE_02_NO_CACHEABLE] = MEMATTR_NO_CACHEABLE,
	[MEMATTR_TYPE_03_DEVICE_NGNRNE] = MEMATTR_DEVICE_NGNRNE,
	[MEMATTR_TYPE_04_DEVICE_NGNRE] = MEMATTR_DEVICE_NGNRE,
	[MEMATTR_TYPE_05_WRITE_THROUGH_READ_ALLOCATE_WRTIE_ALLOCATE] = MEMATTR_WRITE_THROUGH_READ_ALLOCATE_WRTIE_ALLOCATE
};

const struct mpu_cfg_s empty_mpu_cfg = {
	.region_cnt = 0,
	.region_cfg = NULL
};

const struct mpu_access_right_s empty_thread_access_right = {
	.set_num = 0,
	.app_mpu_cfg = &empty_mpu_cfg,
	.thread_mpu_cfg = &empty_mpu_cfg
};

void up_mpu_enable(uint8_t enable)
{
	uint32_t sctlr = CP15_GET(SCTLR);

	if (1U == enable) {
		sctlr |= MPU_SCTLR_M;
	} else {
		sctlr &= ~MPU_SCTLR_M;
	}

	CP15_SET(SCTLR, sctlr);
}

int up_mpu_check_region_cfg(uint8_t region_cnt, const struct mpu_region_cfg_s *region_cfg, uint8_t max_region_num, uint32_t alignment_mask)
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
		if (0U != ((region_cfg[i].limit - region_cfg[i].base + 1U) & (~alignment_mask))) {
			return -4;
		}
	}

	return 0;
}

int up_mpu_core_init(const struct mpu_core_access_right_s *core_access_right)
{
	uint8_t max_region_num = MPUIR_DREGION(CP15_GET(MPUIR));
	int ret = 0;

	up_mpu_enable(0U);
	mpu_enable_backgroud(0U);
	memory_attribute_init();
	ret = up_mpu_check_region_cfg(core_access_right->core_mpu_cfg->region_cnt,
		core_access_right->core_mpu_cfg->region_cfg,
		max_region_num, MPU_ALIGNMENT_MASK);
	if (0 != ret) {
		return ret;
	}
	ret = up_mpu_check_region_cfg(core_access_right->set0_mpu_cfg->region_cnt,
		core_access_right->set0_mpu_cfg->region_cfg,
		max_region_num, MPU_ALIGNMENT_MASK);
	if (0 != ret) {
		return ret;
	}
	mpu_set_region(core_access_right->core_mpu_cfg->region_cnt,
		core_access_right->core_mpu_cfg->region_cfg);
	mpu_set_region(core_access_right->set0_mpu_cfg->region_cnt,
		core_access_right->set0_mpu_cfg->region_cfg);
	up_mpu_enable(1U);

	return ret;
}

#if defined(CONFIG_KERNEL_MEMORY_PROTECTION) && (CONFIG_KERNEL_MEMORY_PROTECTION == 1)
void up_mpu_enable_stack_region(uint32_t low_addr, uint32_t high_addr)
{
	CP15_SET(PRSELR, 0U);
	CP15_SET(PRBAR, PRBAR_SET(low_addr, USER_RW_SUPERVISOR_RW));
	CP15_SET(PRLAR, PRLAR_SET((high_addr - 1U), MEMATTR_TYPE_00_WRITE_BACK_WRTIE_ALLOCATE_CACHEABLE, REGION_ENABLE));
}

void up_mpu_enable_set(uint8_t set_num)
{
	uint8_t begin_num, end_num;
	const struct mpu_cfg_s *set0_mpu_cfg = os_core_access_right_ref_table[up_cpu_index()]->set0_mpu_cfg;

	if (0U != set0_mpu_cfg->region_cnt) {
		begin_num = set0_mpu_cfg->region_cfg->num;
		end_num = set0_mpu_cfg->region_cfg->num + set0_mpu_cfg->region_cnt;
		mpu_enable_region(begin_num, end_num, (uint8_t)(0U == set_num));
	}
}

void up_mpu_switch_access_right(const struct mpu_access_right_s *current_access_right, const struct mpu_access_right_s *next_access_right)
{
	uint8_t begin_num, end_num;
	const struct mpu_cfg_s *current_app_mpu_cfg, *current_thread_mpu_cfg;
	const struct mpu_cfg_s *next_app_mpu_cfg, *next_thread_mpu_cfg;

	if (current_access_right == next_access_right) {
		return;
	}
	current_app_mpu_cfg = current_access_right->app_mpu_cfg;
	current_thread_mpu_cfg = current_access_right->thread_mpu_cfg;
	next_app_mpu_cfg = next_access_right->app_mpu_cfg;
	next_thread_mpu_cfg = next_access_right->thread_mpu_cfg;
	if (current_app_mpu_cfg != next_app_mpu_cfg) {
		if (0U != current_app_mpu_cfg->region_cnt) {
			begin_num = current_app_mpu_cfg->region_cfg->num;
			end_num = current_app_mpu_cfg->region_cfg->num + current_app_mpu_cfg->region_cnt + current_thread_mpu_cfg->region_cnt;
			mpu_enable_region(begin_num, end_num, 0U);
		} else if (0U != current_thread_mpu_cfg->region_cnt) {
			begin_num = current_thread_mpu_cfg->region_cfg->num;
			end_num = current_thread_mpu_cfg->region_cfg->num + current_thread_mpu_cfg->region_cnt;
			mpu_enable_region(begin_num, end_num, 0U);
		} else {
			//do nothing
		}
		mpu_set_region(next_app_mpu_cfg->region_cnt, next_app_mpu_cfg->region_cfg);
		mpu_set_region(next_thread_mpu_cfg->region_cnt, next_thread_mpu_cfg->region_cfg);
		return;
	}
	if (0U != current_thread_mpu_cfg->region_cnt) {
		begin_num = current_thread_mpu_cfg->region_cfg->num;
		end_num = current_thread_mpu_cfg->region_cfg->num + current_thread_mpu_cfg->region_cnt;
		mpu_enable_region(begin_num, end_num, 0U);
	}
	mpu_set_region(next_thread_mpu_cfg->region_cnt, next_thread_mpu_cfg->region_cfg);
}

void up_mpu_switch(void *rtcb, void *tcb)
{
	const struct mpu_access_right_s *cur_access_right, *next_access_right;
	struct tcb_s *cur_tcb = (struct tcb_s *)rtcb;
	struct tcb_s *next_tcb = (struct tcb_s *)tcb;
#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION== 1)
	uintptr_t *cur_context, *next_context;

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
		up_mpu_enable_stack_region((uint32_t)(next_tcb->stack_alloc_ptr), (uint32_t)(next_tcb->stack_base_ptr) + next_tcb->adj_stack_size);
	}
#else
	if ((NULL != cur_tcb) && (NULL != cur_tcb->mpu_access_right)) {
		cur_access_right = cur_tcb->mpu_access_right;
	} else {
		cur_access_right = &empty_thread_access_right;
	}
	if ((NULL != next_tcb) && (NULL != next_tcb->mpu_access_right)) {
		next_access_right = next_tcb->mpu_access_right;
	} else {
		next_access_right = &empty_thread_access_right;
	}
	up_mpu_switch_access_right(cur_access_right, next_access_right);
	if ((NULL != next_tcb) && (0U != next_tcb->mpu_stack_protection_flag)) {
		up_mpu_enable_stack_region((uint32_t)(next_tcb->stack_alloc_ptr), (uint32_t)(next_tcb->stack_base_ptr) + next_tcb->adj_stack_size);
	} else {
		mpu_enable_region(0U, 1U, 0U);
	}
#endif
}
#endif
