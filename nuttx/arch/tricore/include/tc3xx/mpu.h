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

#ifndef __ARCH_TRICORE_INCLUDE_TC3XX_RTFW_MPU_H
#define __ARCH_TRICORE_INCLUDE_TC3XX_RTFW_MPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <arch/irq.h>

#define SYSCON_PROTEN          (0x01UL << 1U)
#define PSW_PRS_MASK           (0x0000b000UL)
#define PSW_PRS_PS0            (0x00000000uL)
#define PSW_PRS_PS1            (0x00001000uL)
#define PSW_PRS_PS2            (0x00002000uL)
#define PSW_PRS_PS3            (0x00003000uL)
#define PSW_PRS_PS4            (0x00008000uL)
#define PSW_PRS_PS5            (0x00009000uL)
#define PSW_PRS_MAX            (6u)

#define DATA_REGION_MAX        (18u)
#define CODE_REGION_MAX        (10u)
#define DATA_ALIGNMENT_MASK    (0xfffffff8UL)
#define CODE_ALIGNMENT_MASK    (0xffffffe0UL)

#define READ_ENABLE            (1U << 0U)
#define WRITE_ENABLE           (1U << 1U)
#define EXECUTE_ENABLE         (1U << 2U)

struct mpu_region_cfg_s {
	uint8_t num;
	uint8_t attr;
	uint32_t base;
	uint32_t limit;
};

struct mpu_cfg_s {
	uint8_t data_region_cnt;
	uint8_t code_region_cnt;
	const struct mpu_region_cfg_s *data_region_cfg;
	const struct mpu_region_cfg_s *code_region_cfg;
};

struct mpu_access_right_s {
	uint8_t set_num;
	const struct mpu_cfg_s *app_mpu_cfg;
	const struct mpu_cfg_s *thread_mpu_cfg;
};

struct mpu_core_access_right_s{
	const struct mpu_cfg_s *core_mpu_cfg;
	const struct mpu_cfg_s *set0_mpu_cfg;
};

extern const uint32_t tricore_psw_prs_regs[PSW_PRS_MAX];
extern const struct mpu_cfg_s empty_mpu_cfg;
extern const struct mpu_access_right_s empty_thread_access_right;
extern const struct mpu_core_access_right_s *const os_core_access_right_ref_table[];
extern const uint16_t os_core_access_right_ref_table_size;

static inline_function void up_mpu_enable_set(uint8_t set_num)
{
	DEBUGASSERT(set_num < PSW_PRS_MAX);
	__mtcr(TX3XX_CPU_PSW, ((__mfcr(TX3XX_CPU_PSW) & (~PSW_PRS_MASK)) | (tricore_psw_prs_regs[set_num])));
}

static inline_function uint8_t up_get_prev_mpu_set_from_syscall(uint32_t *regs)
{
	uintptr_t *plcsa;
	uintptr_t *pucsa;

	plcsa = CSA_TO_ADDR(regs[REG_UPCXI]);
	pucsa = CSA_TO_ADDR(plcsa[REG_LPCXI]);

	return (uint8_t)(PSW_MPU_SET(pucsa[REG_PSW]));
}

static inline_function void up_set_prev_mpu_set_from_syscall(uint32_t *regs, const uint8_t set_num)
{
	uintptr_t *plcsa;
	uintptr_t *pucsa;

	DEBUGASSERT(set_num < PSW_PRS_MAX);
	plcsa = CSA_TO_ADDR(regs[REG_UPCXI]);
	pucsa = CSA_TO_ADDR(plcsa[REG_LPCXI]);

	pucsa[REG_PSW] &= ~PSW_PRS_MASK;
	pucsa[REG_PSW] |= tricore_psw_prs_regs[set_num];
}

void up_mpu_enable(uint8_t enable);
int up_mpu_check_region_cfg(uint8_t region_cnt, const struct mpu_region_cfg_s *region_cfg, uint8_t max_region_num, uint32_t alignment_mask);
int up_mpu_core_init(const struct mpu_core_access_right_s *core_access_right);
void up_mpu_enable_stack_region(uint32_t low_addr, uint32_t high_addr);
void up_mpu_switch_access_right(const struct mpu_access_right_s *current_access_right, const struct mpu_access_right_s *next_access_right);
void up_mpu_switch(void *rtcb, void *tcb);

#ifdef __cplusplus
}
#endif

#endif // __ARCH_TRICORE_INCLUDE_TC3XX_RTFW_MPU_H
