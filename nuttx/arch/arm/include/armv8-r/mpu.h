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

#ifndef __ARCH_ARM_INCLUDE_ARMV8_RTFW_MPU_H
#define __ARCH_ARM_INCLUDE_ARMV8_RTFW_MPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/irq.h>
#define MPU_SCTLR_M            (1 << 0)  /* Bit 0:  MPU enable bit */
#define MPU_SCTLR_BR           (1 << 17) /* Bit 17: Background Region bit */

#define MPU_ALIGNMENT_MASK          (0xffffffc0UL)
#define MPU_BITS_SHIFT(x, h, l)     (((x) >> (l)) & ((1UL << ((h) - (l) + 1U)) - 1UL))
#define MPU_MASK(h, l)              (((~0U) << (l)) & ((~0U) >> (31U - (h))))

/*
* PRBAR
* BASE The base address of a memory region
* SH   Shareability for this memory region
* AP   Access permission for this memory region
* XN   Execute-Never bit for this memory region
*/
#define PRBAR_SH_NON_SHAREABLE      (0x00UL << 3U)
#define PRBAR_SH_OUTTER_SHAREABLE   (0x02UL << 3U)
#define PRBAR_SH_INNTER_SHAREABLE   (0x03UL << 3U)

#define PRBAR_AP_EL0_NON_EL1_RW     (0x00UL << 1U)
#define PRBAR_AP_EL0_RW_EL1_RW      (0x01UL << 1U)
#define PRBAR_AP_EL0_NON_EL1_RO     (0x02UL << 1U)
#define PRBAR_AP_EL0_RO_EL1_RO      (0x03UL << 1U)

#define PRBAR_EX                    (0x00UL << 0U)
#define PRBAR_XN                    (0x01UL << 0U)

#define PRBAR_MASK_XN               MPU_MASK(0U, 0U)
#define PRBAR_MASK_AP               MPU_MASK(2U, 1U)
#define PRBAR_MASK_SH               MPU_MASK(4U, 3U)
#define PRBAR_MASK_BASE             MPU_MASK(31U, 6U)

#define PRBAR_SET(BASE, ATTR)       ((BASE & PRBAR_MASK_BASE) | (ATTR))

/*
* PRLAR
* LIMIT    Limit address of selected MPU memory region
* EN       Region-enable
* AttrIndx Indexes a set of attributes in one of MAIRx registers
*/
#define REGION_DISABLE              (0x00UL << 0U)
#define REGION_ENABLE               (0x01UL << 0U)

#define PRLAR_MASK_EN               MPU_MASK(0U, 0U)
#define PRLAR_MASK_ATTRINDX         MPU_MASK(3U, 1U)
#define PRLAR_MASK_LIMIT            MPU_MASK(31U, 6U)

#define PRLAR_SET(LIMIT, ATTRIDX, EN) ((LIMIT & PRLAR_MASK_LIMIT) | (ATTRIDX << 1UL) | (EN))

/*
* MPUIR
* DREGION number of programmable regions
*/
#define MPUIR_DREGION(r)            MPU_BITS_SHIFT(r, 15, 8)

/*
* Attribute for normal/device memory
*/
#define MEMATTR_DEVICE_NGNRNE       (0U)
#define MEMATTR_DEVICE_NGNRE        (4U)
#define MEMATTR_DEVICE_NGRE         (8U)
#define MEMATTR_DEVICE_GRE          (12U)

#define MEMATTR_T                   (0U)
#define MEMATTR_NT                  (1U)
#define MEMATTR_WT                  (0U)
#define MEMATTR_WB                  (1U)
#define MEMATTR_NRA                 (0U)
#define MEMATTR_RA                  (1U)
#define MEMATTR_NWA                 (0U)
#define MEMATTR_WA                  (1U)
#define MEMATTR_NON_CACHEABLE       (4U)

#define MEMATTR(O, I)               (((O & 0xFU) << 4U) | (I & 0xFU))
#define MEMATTR_NORMAL(NT, WB, RA, WA) \
	(((NT & 1U) << 3U) | ((WB & 1U) << 2U) | ((RA & 1U) << 1U) | (WA & 1U))

#define MEMATTR_WRITE_BACK_WRTIE_ALLOCATE_CACHEABLE \
	MEMATTR(                                        \
		MEMATTR_NORMAL(MEMATTR_NT, MEMATTR_WB, MEMATTR_RA, MEMATTR_WA), \
		MEMATTR_NORMAL(MEMATTR_NT, MEMATTR_WB, MEMATTR_RA, MEMATTR_WA)  \
	)
#define MEMATTR_WRITE_THROUGH_CACHEABLE \
	MEMATTR(                            \
		MEMATTR_NORMAL(MEMATTR_NT, MEMATTR_WT, MEMATTR_NRA, MEMATTR_NWA), \
		MEMATTR_NORMAL(MEMATTR_NT, MEMATTR_WT, MEMATTR_NRA, MEMATTR_NWA)  \
	)
#define MEMATTR_NO_CACHEABLE \
	MEMATTR(MEMATTR_NON_CACHEABLE, MEMATTR_NON_CACHEABLE)

#define MEMATTR_WRITE_THROUGH_READ_ALLOCATE_WRTIE_ALLOCATE \
    MEMATTR(                                        \
        MEMATTR_NON_CACHEABLE, \
        MEMATTR_NORMAL(MEMATTR_NT, MEMATTR_WT, MEMATTR_RA, MEMATTR_WA)  \
    )

/*
* User configuration
*/
#define MEMATTR_TYPE_00_WRITE_BACK_WRTIE_ALLOCATE_CACHEABLE  (0U)
#define MEMATTR_TYPE_01_WRITE_THROUGH_CACHEABLE              (1U)
#define MEMATTR_TYPE_02_NO_CACHEABLE                         (2U)
#define MEMATTR_TYPE_03_DEVICE_NGNRNE                        (3U)
#define MEMATTR_TYPE_04_DEVICE_NGNRE                         (4U)
#define MEMATTR_TYPE_05_WRITE_THROUGH_READ_ALLOCATE_WRTIE_ALLOCATE (5U)
#define MEMATTR_TYPE_MAX                                     (6U)
#define USER_NON_SUPERVISOR_R  (PRBAR_SH_NON_SHAREABLE | PRBAR_AP_EL0_NON_EL1_RO | PRBAR_XN)
#define USER_NON_SUPERVISOR_RW (PRBAR_SH_NON_SHAREABLE | PRBAR_AP_EL0_NON_EL1_RW | PRBAR_XN)
#define USER_NON_SUPERVISOR_RX (PRBAR_SH_NON_SHAREABLE | PRBAR_AP_EL0_NON_EL1_RO | PRBAR_EX)
#define USER_R_SUPERVISOR_R    (PRBAR_SH_NON_SHAREABLE | PRBAR_AP_EL0_RO_EL1_RO | PRBAR_XN)
#define USER_RW_SUPERVISOR_RW  (PRBAR_SH_NON_SHAREABLE | PRBAR_AP_EL0_RW_EL1_RW | PRBAR_XN)
#define USER_RX_SUPERVISOR_RX  (PRBAR_SH_NON_SHAREABLE | PRBAR_AP_EL0_RO_EL1_RO | PRBAR_EX)
#define USER_RWX_SUPERVISOR_RWX  (PRBAR_SH_NON_SHAREABLE | PRBAR_AP_EL0_RW_EL1_RW | PRBAR_EX)

struct mpu_region_cfg_s {
	uint8_t num;
	uint8_t attr;
	uint8_t mem_attr_idx;
	uint32_t base;
	uint32_t limit;
};

struct mpu_cfg_s {
	uint8_t region_cnt;
	const struct mpu_region_cfg_s *region_cfg;
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

extern const uint8_t memory_attribute[MEMATTR_TYPE_MAX];
extern const struct mpu_cfg_s empty_mpu_cfg;
extern const struct mpu_access_right_s empty_thread_access_right;
extern const struct mpu_core_access_right_s *const os_core_access_right_ref_table[];
extern const uint16_t os_core_access_right_ref_table_size;

uint8_t up_get_mpu_set(void);
void up_mpu_enable(uint8_t enable);
int up_mpu_check_region_cfg(uint8_t region_cnt, const struct mpu_region_cfg_s *region_cfg, uint8_t max_region_num, uint32_t alignment_mask);
int up_mpu_core_init(const struct mpu_core_access_right_s *core_access_right);
void up_mpu_enable_stack_region(uint32_t low_addr, uint32_t high_addr);
void up_mpu_enable_set(uint8_t set_num);
void up_mpu_switch_access_right(const struct mpu_access_right_s *current_access_right, const struct mpu_access_right_s *next_access_right);
void up_mpu_switch(void *rtcb, void *tcb);

static inline void memory_attribute_init(void)
{
	uint32_t mair0 = 0UL;
	uint32_t mair1 = 0UL;
	uint8_t i;

	for (i = 0U; i < MEMATTR_TYPE_MAX; i++) {
		if (i <= 3UL) {
			mair0 |= ((uint32_t)(memory_attribute[i]) << (i * 8U));
		} else {
			mair1 |= ((uint32_t)(memory_attribute[i]) << ((i - 4U) * 8U));
		}
	}

	CP15_SET(MAIR0, mair0);
	CP15_SET(MAIR1, mair1);
}

static inline void mpu_enable_backgroud(uint8_t enable)
{
	uint32_t sctlr = CP15_GET(SCTLR);

	if (1U == enable) {
		sctlr |= MPU_SCTLR_BR;
	} else {
		sctlr &= ~MPU_SCTLR_BR;
	}

	CP15_SET(SCTLR, sctlr);
}

static inline void mpu_set_region(uint8_t region_cnt, const struct mpu_region_cfg_s *region_cfg)
{
	uint8_t i;

	for(i = 0U; i < region_cnt; i++) {
		CP15_SET(PRSELR, region_cfg[i].num);
		CP15_SET(PRBAR, PRBAR_SET(region_cfg[i].base, region_cfg[i].attr));
		CP15_SET(PRLAR, PRLAR_SET(region_cfg[i].limit, region_cfg[i].mem_attr_idx, REGION_ENABLE));
	}
}

static inline void mpu_enable_region(uint8_t begin_num, uint8_t end_num, uint8_t enable)
{
	uint32_t prlar;
	uint8_t i;

	for (i = begin_num; i < end_num; i++) {
		CP15_SET(PRSELR, i);
		if (1U == enable) {
			prlar = (CP15_GET(PRLAR) | REGION_ENABLE);
		} else {
			prlar = (CP15_GET(PRLAR) & ~REGION_ENABLE);
		}
		CP15_SET(PRLAR, prlar);
	}
}

static inline_function uint8_t up_get_prev_mpu_set_from_syscall(uint32_t *regs)
{
	(void) regs;
	return up_get_mpu_set();
}

static inline_function void up_set_prev_mpu_set_from_syscall(uint32_t *regs, const uint8_t set_num)
{
	(void) regs;
	up_mpu_enable_set(set_num);
}


#ifdef __cplusplus
}
#endif

#endif // __ARCH_ARM_INCLUDE_ARMV8_RTFW_MPU_H
