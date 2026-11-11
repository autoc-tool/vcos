#ifndef __VENDOR_AUTOC_CHIPS_MCM7_INCLUDE_PER_CPU_DEF_H
#define __VENDOR_AUTOC_CHIPS_MCM7_INCLUDE_PER_CPU_DEF_H

#define __percpu_data locate_data(".data..percpu")
#define __percpu_bss  locate_data(".bss..percpu")

extern const unsigned long g_per_cpu_ram_base_addr[];
#define this_cpu_var(s) (*(__typeof__(s) *)((unsigned long)&(s) - g_per_cpu_ram_base_addr[0] + g_per_cpu_ram_base_addr[up_cpu_index()]))

#endif
