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

#include <arch/irq.h>

bool up_is_privilege_mode(void)
{
	uintptr_t cpsr;

	__asm volatile("mrs %0, cpsr" : "=r"(cpsr));

	return (ARM_MODE_USR == (cpsr & ARM_MODE_MASK)) ? FALSE : TRUE;
}

#if defined(CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION) && (CONFIG_RT_FRAMEWORK_MEMORY_PROTECTION == 1)
uint8_t up_get_mpu_set(void)
{
	const struct mpu_cfg_s *set0_mpu_cfg = os_core_access_right_ref_table[up_cpu_index()]->set0_mpu_cfg;
	uint32_t prlar = 0U;

	if (0U != set0_mpu_cfg->region_cnt) {
		CP15_SET(PRSELR, set0_mpu_cfg->region_cfg->num);
		prlar = CP15_GET(PRLAR);
		prlar &= REGION_ENABLE;
	}

	return ((prlar & REGION_ENABLE) != 0U) ? 0U : 1U;
}
#endif
