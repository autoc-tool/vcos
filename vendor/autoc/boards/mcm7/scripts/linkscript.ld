#include <nuttx/config.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(__start)

MEMORY
{
  flash (rx)  : ORIGIN = 0x00000000, LENGTH = 8M
  sram  (rwx) : ORIGIN = 0x20000000, LENGTH = 2M
}

SECTIONS
{
  .vectors :
  {
    KEEP(*(.vectors))
  } > flash

  .text :
  {
    _stext = .;
    *(.text .text.*)
    *(.rodata .rodata.*)
    *(.gnu.linkonce.t.*)
    *(.glue_7)
    *(.glue_7t)
    *(.got)
    *(.gcc_except_table)
    *(.ARM.extab*)
    *(.gnu.linkonce.r.*)
    _etext = .;
  } > flash

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } > flash

  .data :
  {
    _sdata = .;
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
    _edata = .;
  } > sram AT > flash

  _eronly = LOADADDR(.data);

  .bss :
  {
    _sbss = .;
    *(.bss .bss.*)
    *(COMMON)
    _ebss = .;
  } > sram

#if defined(CONFIG_BMP)
  .percpu_reserve (NOLOAD) :
  {
    . = ALIGN(16);
    _core0_percpu_reserve_start = .;
    . += 12K;
    _core0_percpu_reserve_end = .;
    _core1_percpu_reserve_start = .;
    . += 12K;
    _core1_percpu_reserve_end = .;
    _core2_percpu_reserve_start = .;
    . += 12K;
    _core2_percpu_reserve_end = .;
  } > sram
#endif

  _sram_end = ORIGIN(sram) + LENGTH(sram);
}
