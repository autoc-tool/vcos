
#include "Os.h"
#include "Os_Types.h"

#ifdef CONFIG_SYSTEM_NSH
static char *argv_stub[] = {
    NULL,
};

extern int nsh_main(int argc, FAR char *argv[]);
#else
#define nsh_main(a, b)
#endif

void os_task_core0_shell(void *arg)
{
    nsh_main(0,argv_stub);
    TerminateTask();
}

#if (CONFIG_NR_CPUS > 1)
void os_task_core1_shell(void *arg)
{
    nsh_main(0,argv_stub);
    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 2)
void os_task_core2_shell(void *arg)
{
    nsh_main(0,argv_stub);
    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 3)
void os_task_core3_shell(void *arg)
{
    nsh_main(0,argv_stub);
    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 4)
void os_task_core4_shell(void *arg)
{
    nsh_main(0,argv_stub);
    TerminateTask();
}
#endif

#if (CONFIG_NR_CPUS > 5)
void os_task_core5_shell(void *arg)
{
    nsh_main(0,argv_stub);
    TerminateTask();
}
#endif
