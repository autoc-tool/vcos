/****************************************************************************
 * arch/tricore/include/tc3xx/irq.h
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

/* This file should never be included directly but, rather,
 * only indirectly through nuttx/irq.h
 */

#ifndef __ARCH_TRICORE_INCLUDE_TC3XX_IRQ_H
#define __ARCH_TRICORE_INCLUDE_TC3XX_IRQ_H

#include <nuttx/config.h>
#include <IfxCpu_Intrinsics.h>

/* Upper CSA */
#define REG_UPCXI        0
#define REG_PSW          1
#define REG_A10          2
#define REG_UA11         3
#define REG_D8           4
#define REG_D9           5
#define REG_D10          6
#define REG_D11          7
#define REG_A12          8
#define REG_A13          9
#define REG_A14          10
#define REG_A15          11
#define REG_D12          12
#define REG_D13          13
#define REG_D14          14
#define REG_D15          15

/* Lower CSA */
#define REG_LPCXI        0
#define REG_LA11         1
#define REG_A2           2
#define REG_A3           3
#define REG_D0           4
#define REG_D1           5
#define REG_D2           6
#define REG_D3           7
#define REG_A4           8
#define REG_A5           9
#define REG_A6           10
#define REG_A7           11
#define REG_D4           12
#define REG_D5           13
#define REG_D6           14
#define REG_D7           15

#define REG_RA           REG_UA11
#define REG_SP           REG_A10
#define REG_UPC          REG_UA11

#define REG_LPC          REG_LA11

#define TC_CONTEXT_REGS  (16)

#define XCPTCONTEXT_REGS (TC_CONTEXT_REGS)
#define XCPTCONTEXT_SIZE (sizeof(void *) * TC_CONTEXT_REGS)

#define NR_IRQS          (255)

/* PSW: Program Status Word Register */
#define PSW_CDE           (1 << 7) /* Bits 7: Call Depth Count Enable */
#define PSW_IS            (1 << 9) /* Bits 9: Interrupt Stack Control */
#define PSW_IO            (10)     /* Bits 10-11: Access Privilege Level Control (I/O Privilege) */
#define PSW_IO_USER0      (0 << PSW_IO)
#define PSW_IO_USER1      (1 << PSW_IO)
#define PSW_IO_SUPERVISOR (2 << PSW_IO)
#define PSW_GW            (1 << 8)
#define PSW_MPU_SET_PART0_OFFSET  (12U) /* Bits 12-13: Protection Register Set bits part0 */
#define PSW_MPU_SET_PART1_OFFSET  (15U) /* Bits 15: Protection Register Set bits part1*/
#define PSW_MPU_SET_PART0_MASK    (3 << PSW_MPU_SET_PART0_OFFSET)
#define PSW_MPU_SET_PART1_MASK    (1 << PSW_MPU_SET_PART1_OFFSET)
#define PSW_MPU_SET_PART0_VALUE(PSW)   (((PSW) & PSW_MPU_SET_PART0_MASK) >> PSW_MPU_SET_PART0_OFFSET)
#define PSW_MPU_SET_PART1_VALUE(PSW)   ((((PSW) & PSW_MPU_SET_PART1_MASK) >> PSW_MPU_SET_PART1_OFFSET) << 2U)

#define PSW_MPU_SET(PSW)  (PSW_MPU_SET_PART0_VALUE(PSW) + PSW_MPU_SET_PART1_VALUE(PSW))

/* PCXI: Previous Context Information and Pointer Register */
#define PCXI_UL         (1 << 20) /* Bits 20: Upper or Lower Context Tag */
#define PCXI_PIE        (1 << 21) /* Bits 21: Previous Interrupt Enable */
#define PCXI_PIE_BIT    (21) /* Bits 21: Previous Interrupt Enable */
#define PCXI_PIE_MASK   (1 << PCXI_PIE_BIT)
#define PCXI_PCPN_BIT    (22) /* Bits 22-29: Previous CPU Priority Number */
#define PCXI_PCPN_MASK   (0xff << PCXI_PCPN_BIT)

/* FCX: Free CSA List Head Pointer Register */
#define FCX_FCXO        (0)       /* Bits 0-15: FCX Offset Address */
#define FCX_FCXS        (16)      /* Bits 16-19: FCX Segment Address */
#define FCX_FCXO_MASK   (0xffff << FCX_FCXO)
#define FCX_FCXS_MASK   (0xf    << FCX_FCXS)
#define FCX_FREE        (FCX_FCXS_MASK | FCX_FCXO_MASK) /* Free CSA manipulation */

/* ICR: Interrupt Control Register */
#define ICR_IE          (15)         /* Bits 15: Global Interrupt Enable Bit */
#define ICR_PIPN        (16)         /* Bits 16-23: Pending Interrupt Priority Number */
#define ICR_CCPN        (0)          /* Bits 0-7: Current CPU Priority Number */

#define ICR_IE_MASK     (0x1 << ICR_IE)
#define ICR_PIPN_MASK   (0xff << ICR_PIPN)
#define ICR_CCPN_MASK   (0xff << ICR_CCPN)

/** Ifx CPUs register */
/** \brief 0x00000100, CPU_PSW_GW */
#define CPU_PSW_GW_MASK ( 0x00000100UL )    /* bit:8 */

/* Tc3xx Core ID Reg */
#define TC3XX_CPU_CORE_ID 0xFE1C
/* Tc3xx Customer ID Reg */
#define TC3XX_CPU_CUS_ID 0xFE50
/* Tc3xx Program Status Word Reg */
#define TX3XX_CPU_PSW 0xFE04
/* Tc3xx Previous Context Information Reg */
#define TC3XX_CPU_PCXI 0xFE00
/* Tc3xx Interrupt Control Reg */
#define TC3XX_CPU_ICR 0xFE2C
/* Tc3XX Program Counter Reg */
#define TC3XX_CPU_PC 0xFE08
/* Tc3xx Interrupt Stack Pointer*/
#define TC3XX_CPU_ISP 0xFE28
/* Tc3xx System Configuration Reg */
#define TC3XX_CPU_SYSCON 0xFE14
/* Tc3xx Data Protection Range 0, Lower Bound Register */
#define TC3XX_CPU_DPR0_L 0xC000
/* Tc3xx Data Protection Range 0, Upper Bound Register */
#define TC3XX_CPU_DPR0_U 0xC004
/* Tc3xx Data Protection Range 1, Lower Bound Register */
#define TC3XX_CPU_DPR1_L 0xC008
/* Tc3xx Data Protection Range 1, Upper Bound Register */
#define TC3XX_CPU_DPR1_U 0xC00C
/* Tc3xx Data Protection Range 2, Lower Bound Register */
#define TC3XX_CPU_DPR2_L 0xC010
/* Tc3xx Data Protection Range 2, Upper Bound Register */
#define TC3XX_CPU_DPR2_U 0xC014
/* Tc3xx Data Protection Range 3, Lower Bound Register */
#define TC3XX_CPU_DPR3_L 0xC018
/* Tc3xx Data Protection Range 3, Upper Bound Register */
#define TC3XX_CPU_DPR3_U 0xC01C
/* Tc3xx Data Protection Range 4, Lower Bound Register */
#define TC3XX_CPU_DPR4_L 0xC020
/* Tc3xx Data Protection Range 4, Upper Bound Register */
#define TC3XX_CPU_DPR4_U 0xC024
/* Tc3xx Data Protection Range 5, Lower Bound Register */
#define TC3XX_CPU_DPR5_L 0xC028
/* Tc3xx Data Protection Range 5, Upper Bound Register */
#define TC3XX_CPU_DPR5_U 0xC02C
/* Tc3xx Data Protection Range 6, Lower Bound Register */
#define TC3XX_CPU_DPR6_L 0xC030
/* Tc3xx Data Protection Range 6, Upper Bound Register */
#define TC3XX_CPU_DPR6_U 0xC034
/* Tc3xx Data Protection Range 7, Lower Bound Register */
#define TC3XX_CPU_DPR7_L 0xC038
/* Tc3xx Data Protection Range 7, Upper Bound Register */
#define TC3XX_CPU_DPR7_U 0xC03C
/* Tc3xx Data Protection Range 8, Lower Bound Register */
#define TC3XX_CPU_DPR8_L 0xC040
/* Tc3xx Data Protection Range 8, Upper Bound Register */
#define TC3XX_CPU_DPR8_U 0xC044
/* Tc3xx Data Protection Range 9, Lower Bound Register */
#define TC3XX_CPU_DPR9_L 0xC048
/* Tc3xx Data Protection Range 9, Upper Bound Register */
#define TC3XX_CPU_DPR9_U 0xC04C
/* Tc3xx Data Protection Range 10, Lower Bound Register */
#define TC3XX_CPU_DPR10_L 0xC050
/* Tc3xx Data Protection Range 10, Upper Bound Register */
#define TC3XX_CPU_DPR10_U 0xC054
/* Tc3xx Data Protection Range 11, Lower Bound Register */
#define TC3XX_CPU_DPR11_L 0xC058
/* Tc3xx Data Protection Range 11, Upper Bound Register */
#define TC3XX_CPU_DPR11_U 0xC05C
/* Tc3xx Data Protection Range 12, Lower Bound Register */
#define TC3XX_CPU_DPR12_L 0xC060
/* Tc3xx Data Protection Range 12, Upper Bound Register */
#define TC3XX_CPU_DPR12_U 0xC064
/* Tc3xx Data Protection Range 13, Lower Bound Register */
#define TC3XX_CPU_DPR13_L 0xC068
/* Tc3xx Data Protection Range 13, Upper Bound Register */
#define TC3XX_CPU_DPR13_U 0xC06C
/* Tc3xx Data Protection Range 14, Lower Bound Register */
#define TC3XX_CPU_DPR14_L 0xC070
/* Tc3xx Data Protection Range 14, Upper Bound Register */
#define TC3XX_CPU_DPR14_U 0xC074
/* Tc3xx Data Protection Range 15, Lower Bound Register */
#define TC3XX_CPU_DPR15_L 0xC078
/* Tc3xx Data Protection Range 15, Upper Bound Register */
#define TC3XX_CPU_DPR15_U 0xC07C
/* Tc3xx Data Protection Range 16, Lower Bound Register */
#define TC3XX_CPU_DPR16_L 0xC080
/* Tc3xx Data Protection Range 16, Upper Bound Register */
#define TC3XX_CPU_DPR16_U 0xC084
/* Tc3xx Data Protection Range 17, Lower Bound Register */
#define TC3XX_CPU_DPR17_L 0xC088
/* Tc3xx Data Protection Range 17, Upper Bound Register */
#define TC3XX_CPU_DPR17_U 0xC08C
/* Tc3xx Code Protection Range 0 Lower Bound Register */
#define TC3XX_CPU_CPR0_L 0xD000
/* Tc3xx Code Protection Range 0 Upper Bound Register */
#define TC3XX_CPU_CPR0_U 0xD004
/* Tc3xx Code Protection Range 1 Lower Bound Register */
#define TC3XX_CPU_CPR1_L 0xD008
/* Tc3xx Code Protection Range 1 Upper Bound Register */
#define TC3XX_CPU_CPR1_U 0xD00C
/* Tc3xx Code Protection Range 2 Lower Bound Register */
#define TC3XX_CPU_CPR2_L 0xD010
/* Tc3xx Code Protection Range 2 Upper Bound Register */
#define TC3XX_CPU_CPR2_U 0xD014
/* Tc3xx Code Protection Range 3 Lower Bound Register */
#define TC3XX_CPU_CPR3_L 0xD018
/* Tc3xx Code Protection Range 3 Upper Bound Register */
#define TC3XX_CPU_CPR3_U 0xD01C
/* Tc3xx Code Protection Range 4 Lower Bound Register */
#define TC3XX_CPU_CPR4_L 0xD020
/* Tc3xx Code Protection Range 4 Upper Bound Register */
#define TC3XX_CPU_CPR4_U 0xD024
/* Tc3xx Code Protection Range 5 Lower Bound Register */
#define TC3XX_CPU_CPR5_L 0xD028
/* Tc3xx Code Protection Range 5 Upper Bound Register */
#define TC3XX_CPU_CPR5_U 0xD02C
/* Tc3xx Code Protection Range 6 Lower Bound Register */
#define TC3XX_CPU_CPR6_L 0xD030
/* Tc3xx Code Protection Range 6 Upper Bound Register */
#define TC3XX_CPU_CPR6_U 0xD034
/* Tc3xx Code Protection Range 7 Lower Bound Register */
#define TC3XX_CPU_CPR7_L 0xD038
/* Tc3xx Code Protection Range 7 Upper Bound Register */
#define TC3XX_CPU_CPR7_U 0xD03C
/* Tc3xx Code Protection Range 8 Lower Bound Register */
#define TC3XX_CPU_CPR8_L 0xD040
/* Tc3xx Code Protection Range 8 Upper Bound Register */
#define TC3XX_CPU_CPR8_U 0xD044
/* Tc3xx Code Protection Range 9 Lower Bound Register */
#define TC3XX_CPU_CPR9_L 0xD048
/* Tc3xx Code Protection Range 9 Upper Bound Register */
#define TC3XX_CPU_CPR9_U 0xD04C
/* Tc3xx Code Protection Execute Enable Register Set 0 */
#define TC3XX_CPU_CPXE_0 0xE000
/* Tc3xx Code Protection Execute Enable Register Set 1 */
#define TC3XX_CPU_CPXE_1 0xE004
/* Tc3xx Code Protection Execute Enable Register Set 2 */
#define TC3XX_CPU_CPXE_2 0xE008
/* Tc3xx Code Protection Execute Enable Register Set 3 */
#define TC3XX_CPU_CPXE_3 0xE00C
/* Tc3xx Code Protection Execute Enable Register Set 4 */
#define TC3XX_CPU_CPXE_4 0xE040
/* Tc3xx Code Protection Execute Enable Register Set 5 */
#define TC3XX_CPU_CPXE_5 0xE044
/* Tc3xx Data Protection Read Enable Register Set 0 */
#define TC3XX_CPU_DPRE_0 0xE010
/* Tc3xx Data Protection Read Enable Register Set 1 */
#define TC3XX_CPU_DPRE_1 0xE014
/* Tc3xx Data Protection Read Enable Register Set 2 */
#define TC3XX_CPU_DPRE_2 0xE018
/* Tc3xx Data Protection Read Enable Register Set 3 */
#define TC3XX_CPU_DPRE_3 0xE01C
/* Tc3xx Data Protection Read Enable Register Set 4 */
#define TC3XX_CPU_DPRE_4 0xE050
/* Tc3xx Data Protection Read Enable Register Set 5 */
#define TC3XX_CPU_DPRE_5 0xE054
/* Tc3xx Data Protection Write Enable Register Set 0 */
#define TC3XX_CPU_DPWE_0 0xE020
/* Tc3xx Data Protection Write Enable Register Set 1 */
#define TC3XX_CPU_DPWE_1 0xE024
/* Tc3xx Data Protection Write Enable Register Set 2 */
#define TC3XX_CPU_DPWE_2 0xE028
/* Tc3xx Data Protection Write Enable Register Set 3 */
#define TC3XX_CPU_DPWE_3 0xE02C
/* Tc3xx Data Protection Write Enable Register Set 4 */
#define TC3XX_CPU_DPWE_4 0xE060
/* Tc3xx Data Protection Write Enable Register Set 5 */
#define TC3XX_CPU_DPWE_5 0xE064

#define CSA_TO_ADDR(csa) ((uintptr_t *)((((csa) & 0x000F0000) << 12) | (((csa) & 0x0000FFFF) << 6)))

#ifndef __ASSEMBLY__
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
#include <arch/tc3xx/atomic.h>
#include <arch/tc3xx/mpu.h>
#endif /* CONFIG_RT_FRAMEWORK */
struct xcptcontext {
  /* These are saved copies of the context used during
   * signal processing.
   */

  uintptr_t *saved_regs;

  /* Register save area with XCPTCONTEXT_SIZE, only valid when:
   * 1.The task isn't running or
   * 2.The task is interrupted
   * otherwise task is running, and regs contain the stale value.
   */

  uintptr_t *regs;
};

#if defined(CONFIG_RT_FRAMEWORK_SYSCALL ) && (CONFIG_RT_FRAMEWORK_SYSCALL == 1)
bool up_is_privilege_mode(void);
uint8_t up_get_mpu_set(void);

/* called directly */

static inline_function void up_set_privilege_to_user(void)
{
	uintptr_t psw;

	psw = (uintptr_t)__mfcr(TX3XX_CPU_PSW);
	psw &= ~(PSW_IO_USER0 | PSW_IO_USER1 | PSW_IO_SUPERVISOR);
	psw |= PSW_IO_USER0;
	__mtcr(TX3XX_CPU_PSW, psw);
	__isync();
}

/*called by trap*/

static inline_function void up_syscall_set_enter_int_state(uint32_t *regs)
{
	uintptr_t icr;
	uintptr_t pcxi;
	uintptr_t *plcsa, *pucsa;

	__dsync();

	plcsa = CSA_TO_ADDR(regs[REG_UPCXI]);
	pcxi = plcsa[REG_LPCXI];
	pucsa = CSA_TO_ADDR(pcxi);

#ifdef CONFIG_TRICORE_TOOLCHAIN_TASKING
	__asm volatile("mov.a  a10, %0" ::"d"(pucsa[REG_A10]) : "a10");
#else
	__setareg(a10, pucsa[REG_A10]);
#endif	
	icr = (uintptr_t)__mfcr(TC3XX_CPU_ICR);

#if defined(CONFIG_RT_FRAMEWORK_INT_LEVEL) && (CONFIG_RT_FRAMEWORK_INT_LEVEL == 1)
	icr &= (~ICR_CCPN_MASK);
	icr |= ((pcxi & PCXI_PCPN_MASK) >> PCXI_PCPN_BIT);
#endif
	icr &= (~ICR_IE_MASK);
	icr |= ((pcxi & PCXI_PIE_MASK) >> PCXI_PIE_BIT << ICR_IE);

	__mtcr(TC3XX_CPU_ICR, icr);
	__isync();
}

static inline_function void up_syscall_set_exit_int_state(uint32_t *regs)
{

}

static inline_function bool up_syscall_get_prev_privilege(uint32_t *regs)
{
	uintptr_t *plcsa;
	uintptr_t *pucsa;

	plcsa = CSA_TO_ADDR(regs[REG_UPCXI]);

	__dsync();

	pucsa = CSA_TO_ADDR(plcsa[REG_LPCXI]);

	return ((pucsa[REG_PSW] & PSW_IO_SUPERVISOR) != 0) ? true : false;
}

static inline_function void up_syscall_set_prev_privilege(uint32_t *regs, const bool is_privilege)
{
	uintptr_t *plcsa;
	uintptr_t *pucsa;

	plcsa = CSA_TO_ADDR(regs[REG_UPCXI]);

	__dsync();

	pucsa = CSA_TO_ADDR(plcsa[REG_LPCXI]);

	pucsa[REG_PSW] &= ~(PSW_IO_USER0 | PSW_IO_USER1 | PSW_IO_SUPERVISOR);
	pucsa[REG_PSW] |= (is_privilege == true) ? PSW_IO_SUPERVISOR : PSW_IO_USER0;

}

static inline_function void up_syscall_set_prev_int_state(uint32_t *regs)
{
	uintptr_t icr;
	uintptr_t icr_ie;
	uintptr_t pcxi;
	uintptr_t *plcsa;

	icr = (uintptr_t)__mfcr(TC3XX_CPU_ICR);
	plcsa = CSA_TO_ADDR(regs[REG_UPCXI]);

	__dsync();

	pcxi = plcsa[REG_LPCXI];

	pcxi &= ~PCXI_PIE_MASK;
	icr_ie = (icr & ICR_IE_MASK) >> ICR_IE;
	pcxi |= icr_ie << PCXI_PIE_BIT;

#if defined(CONFIG_RT_FRAMEWORK_INT_LEVEL) && (CONFIG_RT_FRAMEWORK_INT_LEVEL == 1)
	pcxi &= ~PCXI_PCPN_MASK;
	pcxi |= ((icr & ICR_CCPN_MASK) >> ICR_CCPN) << PCXI_PCPN_BIT;
#endif

	plcsa[REG_LPCXI] = pcxi;
}

#endif /* CONFIG_RT_FRAMEWORK_SYSCALL */

#endif /* __ASSEMBLY__ */

#endif /* __ARCH_TRICORE_INCLUDE_TC3XX_IRQ_H */
