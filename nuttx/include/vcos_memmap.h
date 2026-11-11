#ifndef __VCOS_MEMMAP_H__
#define __VCOS_MEMMAP_H__

/**
 * @brief maps to read-only data area. Maybe you should use C keyworld ‘const’ instead of this.
 */
#define MMAP_CONST_SECTION         __attribute__((section(".rodata.Const")))

/**
 * @brief maps to share memory area, where variables can be
 *        used by multiple cores with cache, aligned with cacheline.
 */
#define MMAP_SHARE_CACHE_VAR_INIT_SECTION        __attribute__((section(".data.Share.Cached.VAR_INIT")))
#define MMAP_SHARE_CACHE_VAR_NO_INIT_SECTION     __attribute__((section(".bss.Share.Cached.VAR_NO_INIT")))
#define MMAP_SHARE_CACHE_VAR_ZERO_INIT_SECTION   __attribute__((section(".bss.Share.Cached.VAR_ZERO_INIT")))

/**
 * @brief maps to share memory area, where variables can be
 *        used by multiple cores, No-Cache
 */
#define MMAP_SHARE_VAR_INIT_SECTION         __attribute__((section(".data.Share.VAR_INIT")))
#define MMAP_SHARE_VAR_NO_INIT_SECTION      __attribute__((section(".bss.Share.VAR_NO_INIT")))
#define MMAP_SHARE_VAR_ZERO_INIT_SECTION    __attribute__((section(".bss.Share.VAR_ZERO_INIT")))

/**
 * @brief maps to core0 area, where variables can only be
 *        used by core0.
 */
#define MMAP_CORE0_VAR_INIT_SECTION         __attribute__((section(".data.Private.Core0_VAR_INIT")))
#define MMAP_CORE0_VAR_NO_INIT_SECTION      __attribute__((section(".bss.Private.Core0_VAR_NO_INIT")))
#define MMAP_CORE0_VAR_ZERO_INIT_SECTION    __attribute__((section(".bss.Private.Core0_VAR_ZERO_INIT")))

/**
 * @brief maps to core1 area, where variables can only be
 *        used by core1.
 */
#define MMAP_CORE1_VAR_INIT_SECTION         __attribute__((section(".data.Private.Core1_VAR_INIT")))
#define MMAP_CORE1_VAR_NO_INIT_SECTION      __attribute__((section(".bss.Private.Core1_VAR_NO_INIT")))
#define MMAP_CORE1_VAR_ZERO_INIT_SECTION    __attribute__((section(".bss.Private.Core1_VAR_ZERO_INIT")))

/**
 * @brief maps to core2 area, where variables can only be
 *        used by core2.
 */
#define MMAP_CORE2_VAR_INIT_SECTION         __attribute__((section(".data.Private.Core2_VAR_INIT")))
#define MMAP_CORE2_VAR_NO_INIT_SECTION      __attribute__((section(".bss.Private.Core2_VAR_NO_INIT")))
#define MMAP_CORE2_VAR_ZERO_INIT_SECTION    __attribute__((section(".bss.Private.Core2_VAR_ZERO_INIT")))

/**
 * @brief maps to core3 area, where variables can only be
 *        used by core3.
 */
#define MMAP_CORE3_VAR_INIT_SECTION         __attribute__((section(".data.Private.Core3_VAR_INIT")))
#define MMAP_CORE3_VAR_NO_INIT_SECTION      __attribute__((section(".bss.Private.Core3_VAR_NO_INIT")))
#define MMAP_CORE3_VAR_ZERO_INIT_SECTION    __attribute__((section(".bss.Private.Core3_VAR_ZERO_INIT")))

/* DDR */
#define MMAP_DDR_VAR_INIT_SECTION    __attribute__((section(".data.DDR.VAR_INIT")))
#define MMAP_DDR_VAR_NO_INIT_SECTION  __attribute__((section(".bss.DDR.VAR_NO_INIT")))
#define MMAP_DDR_VAR_ZERO_INIT_SECTION  __attribute__((section(".bss.DDR.VAR_ZERO_INIT")))

#define MMAP_DDR_TEXT_SECTION    __attribute__((section(".text.DDR")))

/**
 * @brief maps to tcm area
 */
#define MMAP_TCM_TXT_SECTION        __attribute__((section(".tcm.text")))
#define MMAP_TCM_DATA_SECTION       __attribute__((section(".tcm.data")))
#define MMAP_TCM_BSS_SECTION        __attribute__((section(".tcm.bss")))

/* STACK */
#define MMAP_STACK_NO_INIT_SECTION    __attribute__((section(".pbss.stack")))

#endif /*__VCOS_MEMMAP_H__*/
