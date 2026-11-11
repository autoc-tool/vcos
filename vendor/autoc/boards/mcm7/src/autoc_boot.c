#include <nuttx/config.h>
#include <nuttx/board.h>
#include <nuttx/fs/fs.h>
#include <stdint.h>
#include <syslog.h>

extern void platform_init(void);
extern void StartOS(void);

#ifdef CONFIG_BOARD_EARLY_INITIALIZE
void board_early_initialize(void)
{
#if defined(CONFIG_RT_FRAMEWORK) && (CONFIG_RT_FRAMEWORK == 1)
  platform_init();
  StartOS();
#endif
}
#endif

#ifdef CONFIG_BOARD_LATE_INITIALIZE
void board_late_initialize(void)
{
#ifdef CONFIG_FS_PROCFS
  int ret;

  ret = nx_mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to mount procfs at /proc: %d\n", ret);
    }
#endif
}
#endif

int board_app_initialize(uintptr_t arg)
{
  (void)arg;
  return 0;
}
