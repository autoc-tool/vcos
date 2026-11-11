#ifndef __VENDOR_AUTOC_CHIPS_SCM7_INCLUDE_PER_CPU_DEF_H
#define __VENDOR_AUTOC_CHIPS_SCM7_INCLUDE_PER_CPU_DEF_H

#define __percpu_data locate_data(".data..percpu")
#define __percpu_bss  locate_data(".bss..percpu")

#define this_cpu_var(s) (s)

#endif
