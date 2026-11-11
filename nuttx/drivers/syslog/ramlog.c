/****************************************************************************
 * drivers/syslog/ramlog.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>
#include <ctype.h>
#include <sys/boardctl.h>

#include <nuttx/arch.h>
#include <nuttx/atomic.h>
#include <nuttx/kmalloc.h>
#include <nuttx/kthread.h>
#include <nuttx/spinlock.h>
#include <nuttx/semaphore.h>
#include <nuttx/fs/fs.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/syslog/ramlog.h>
#include <nuttx/compiler.h>
#include <nuttx/list.h>
#include <nuttx/irq.h>
#include <nuttx/init.h>

#include "vcos_ipicall.h"

#ifdef CONFIG_RAMLOG

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define RAMLOG_MAGIC_NUMBER 0x12345678

#ifdef CONFIG_RAMLOG_FLUSH_WORKER_TIMEOUT_MS
#  define RAMLOG_FLUSH_WORKER_TIMEOUT_TICKS \
          MSEC2TICK(CONFIG_RAMLOG_FLUSH_WORKER_TIMEOUT_MS)
#endif

/*Buffer alignment size*/
#ifndef RAMLOG_BUF_ALIGNMENT_SIZE
#define RAMLOG_BUF_ALIGNMENT_SIZE 4U
#endif

/* Calculate buffer size for an aligned buffer - returns the next highest */
#ifndef RAMLOG_ALIGNED_BUFSIZE
#define RAMLOG_ALIGNED_BUFSIZE(size) (((size) + RAMLOG_BUF_ALIGNMENT_SIZE - 1U) & ~(RAMLOG_BUF_ALIGNMENT_SIZE-1U))
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct ramlog_header_s
{
  uint32_t          rl_magic;    /* The rl_magic number for ramlog buffer init */
  volatile uint32_t rl_head;     /* The head index (where data is added,natural growth) */
  char              rl_buffer[]; /* Circular RAM buffer */
};

struct ramlog_user_s
{
  struct list_node  rl_node;       /* The list_node of reader */
  volatile uint32_t rl_tail;       /* The tail index (where data is removed) */
  uint32_t          rl_threashold; /* The threashold of the reader to read log */
#ifndef CONFIG_RAMLOG_NONBLOCKING
  sem_t             rl_waitsem;    /* Used to wait for data */
#endif

  /* The following the poll structures of threads waiting for driver events.
   * The 'struct pollfd' reference for each open is also  retained in the
   * f_priv field of the 'struct file'.
   */

  FAR struct pollfd *rl_fds;
};

struct ramlog_dev_s
{
  /* The following is the header of the RAM buffer,
   * Store the RAM BUFFER init rl_magic number,
   * and read/write pointers
   */

  FAR struct ramlog_header_s *rl_header;

  uint32_t                   rl_bufsize; /* Size of the Circular RAM buffer */
  struct list_node           rl_list;    /* The head of ramlog_user_s list */
#ifdef CONFIG_RAMLOG_FLUSH_WORKER
  struct work_s              rl_work;    /* For deferring notifications to LPWORK queue */
#endif
#ifdef CONFIG_RAMLOG_FLUSH
  volatile uint32_t          rl_tail;    /* The tail index (where data is removed) */
  atomic_uint                rl_expmask; /* The exp mask of all core */
#endif
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* Helper functions */
#ifdef CONFIG_RAMLOG_SYSLOG
static void    ramlog_syslog_initialize(void);
#endif

#ifndef CONFIG_RAMLOG_NONBLOCKING
static void    ramlog_readnotify(FAR struct ramlog_dev_s *priv);
#endif
static void    ramlog_pollnotify(FAR struct ramlog_dev_s *priv);

/* Character driver methods */

static int     ramlog_file_open(FAR struct file *filep);
static int     ramlog_file_close(FAR struct file *filep);
static ssize_t ramlog_file_read(FAR struct file *filep, FAR char *buffer,
                                size_t buflen);
static ssize_t ramlog_file_write(FAR struct file *filep,
                                 FAR const char *buffer, size_t buflen);
static int     ramlog_file_ioctl(FAR struct file *filep, int cmd,
                                 unsigned long arg);
static int     ramlog_file_poll(FAR struct file *filep,
                                FAR struct pollfd *fds, bool setup);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_ramlogfops =
{
  ramlog_file_open,  /* open */
  ramlog_file_close, /* close */
  ramlog_file_read,  /* read */
  ramlog_file_write, /* write */
  NULL,              /* seek */
  ramlog_file_ioctl, /* ioctl */
  NULL,              /* mmap */
  NULL,              /* truncate */
  ramlog_file_poll   /* poll */
};

/* This is the pre-allocated buffer used for the console RAM log and/or
 * for the syslogging function.
 */

#ifdef CONFIG_RAMLOG_SYSLOG
#ifdef RAMLOG_BUFFER_SECTION
static uint8_t g_sysbuffer[RAMLOG_ALIGNED_BUFSIZE(CONFIG_RAMLOG_BUFSIZE)]
                       locate_data(RAMLOG_BUFFER_SECTION);
#else
static uint8_t g_sysbuffer[RAMLOG_ALIGNED_BUFSIZE(CONFIG_RAMLOG_BUFSIZE)];
#endif
/* This is the device structure for the console or syslogging function.  It
 * must be statically initialized because the RAMLOG ramlog_putc function
 * could be called before the driver initialization logic executes.
 */
static struct ramlog_dev_s g_sysdev;

static spinlock_t g_ramlog_addbuf_lock = SP_UNLOCKED;
static spinlock_t g_ramlog_flush_lock = SP_UNLOCKED;

#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ramlog_bufferused
 ****************************************************************************/

static uint32_t ramlog_bufferused(FAR struct ramlog_dev_s *priv,
                                  FAR struct ramlog_user_s *upriv)
{
  uint32_t used = priv->rl_header->rl_head - upriv->rl_tail;
  return used > priv->rl_bufsize ? priv->rl_bufsize : used;
}

/****************************************************************************
 * Name: ramlog_readnotify
 ****************************************************************************/

#ifndef CONFIG_RAMLOG_NONBLOCKING
static void ramlog_readnotify(FAR struct ramlog_dev_s *priv)
{
  FAR struct ramlog_user_s *upriv;

  /* Notify all waiting readers that they can read from the FIFO */

  list_for_every_entry(&priv->rl_list, upriv, struct ramlog_user_s, rl_node)
    {
      for (; ; )
        {
          int semcount = 0;

          nxsem_get_value(&upriv->rl_waitsem, &semcount);
          if (semcount >= 0)
            {
              break;
            }

          nxsem_post(&upriv->rl_waitsem);
        }
    }
}
#endif

/****************************************************************************
 * Name: ramlog_pollnotify
 ****************************************************************************/

static void ramlog_pollnotify(FAR struct ramlog_dev_s *priv)
{
  FAR struct ramlog_user_s *upriv;

  /* This function may be called from an interrupt handler */

  list_for_every_entry(&priv->rl_list, upriv, struct ramlog_user_s, rl_node)
    {
      if (ramlog_bufferused(priv, upriv) >= upriv->rl_threashold)
        {
          /* Notify all poll/select waiters that they can read from
           * the FIFO
           */

          poll_notify(&upriv->rl_fds, 1, POLLIN);
        }
    }
}

/****************************************************************************
 * Name: ramlog_bufferflush
 ****************************************************************************/

static void ramlog_bufferflush(FAR struct ramlog_dev_s *priv)
{
  FAR struct ramlog_user_s *upriv;

  priv->rl_header->rl_head = 0;
  list_for_every_entry(&priv->rl_list, upriv, struct ramlog_user_s, rl_node)
    {
      upriv->rl_tail = 0;
    }
}

#ifdef CONFIG_RAMLOG_FLUSH

static _Atomic pid_t ramlog_flush_tid[CONFIG_NR_CPUS] = {0};

static sem_t ramlog_flush_sem[CONFIG_NR_CPUS];

void ramlog_flush_internal(FAR struct ramlog_dev_s *priv)
{
  FAR struct ramlog_header_s *header = priv->rl_header;
  FAR char *buf = header->rl_buffer;
  FAR const char *pos;
  char buffer[CONFIG_SYSLOG_BUFSIZE];
  irqstate_t flags;
  uint32_t begin;
  uint32_t end;
  uint32_t size;

  flags = spin_lock_irqsave(&g_ramlog_flush_lock);

  do
    {
      /* Take a snapshot of the rl_head */
      uint32_t rl_head_local = header->rl_head;

      if (rl_head_local == priv->rl_tail)
        {
          break;
        }

      if (rl_head_local - priv->rl_tail > priv->rl_bufsize)
        {
          priv->rl_tail = rl_head_local - priv->rl_bufsize;
        }

      begin = priv->rl_tail % priv->rl_bufsize;

      end = rl_head_local % priv->rl_bufsize;
      if (end <= begin)
        {
          end = priv->rl_bufsize;
        }

      buf = header->rl_buffer + begin;
      pos = buf;

      while (pos != (header->rl_buffer + end) && *pos++ != '\n' &&
             pos - buf < sizeof(buffer));

      size = pos - buf;
      memcpy(buffer, buf, size);
      priv->rl_tail += size;

      spin_unlock_irqrestore(&g_ramlog_flush_lock, flags);

      up_nputs(buffer, size);

      flags = spin_lock_irqsave(&g_ramlog_flush_lock);
    }
  while (header->rl_head != priv->rl_tail);

  spin_unlock_irqrestore(&g_ramlog_flush_lock, flags);
}

static void ramlog_flush_execute(FAR struct ramlog_dev_s *priv)
{
  FAR struct ramlog_header_s *header = priv->rl_header;

  /* Only scheduled by the specified buffer-flush core */
  if (priv == &g_sysdev && OSINIT_MM_READY())
  {
    /*Schedule only when "buffer has data" and "threshold is met" */
    if ((header->rl_head - priv->rl_tail) >= CONFIG_RAMLOG_POLLTHRESHOLD)
    {
      ramlog_flush_internal(priv);
    }
  }
}

static int ramlog_flush_kthread(void)
{
  int cur_core_id = up_cpu_index();

  while (1) {
    nxsem_wait(&ramlog_flush_sem[cur_core_id]);
    ramlog_flush_execute(&g_sysdev);
  }

  return 0;
}

static pid_t ramlog_flush_thread_create(void)
{
  int cur_core_id = up_cpu_index();
  sem_init(&ramlog_flush_sem[cur_core_id], 0, 0);

  /* Returns the positive, non-zero process ID of the new task or a negated
     errno value to indicate the nature of any failure */
  pid_t tid = kthread_create("ramlog_flush_thread",
                             CONFIG_RAMLOG_FLUSH_THREAD_PRIO,
                             CONFIG_RAMLOG_FLUSH_THREAD_STACKSIZE,
                             ramlog_flush_kthread,
                             NULL);
  if (tid > 0)
  {
    atomic_store(&ramlog_flush_tid[cur_core_id], tid);
  }
  return tid;
}

/* Mark the specified core status as an exception state */
static inline void ramlog_mark_core_exp(int core_id)
{
  DEBUGASSERT(core_id >= 0 && core_id < CONFIG_NR_CPUS);
  atomic_fetch_or_explicit(&g_sysdev.rl_expmask, (1U << core_id), memory_order_seq_cst);
}

/* Determine whether all cores are in an abnormal state */
static inline bool ramlog_is_all_core_exp(void)
{
  uint32_t ramlog_all_exp_mask = (1U << CONFIG_NR_CPUS) - 1;

  /* First perform an atomic read to ensure atomicity */
  uint32_t core_expmask = atomic_load(&g_sysdev.rl_expmask);
  return (core_expmask & ramlog_all_exp_mask) == ramlog_all_exp_mask;
}

/* Select a new healthy core to act as the buffer-flushing core */
static int ramlog_select_flush_core(int init_core_id)
{
  uint32_t core_expmask = atomic_load(&g_sysdev.rl_expmask);
  uint32_t core_alive_mask = (~core_expmask) & ((1u << CONFIG_NR_CPUS) - 1);
  /* All cores are in an abnormal state */
  if (core_alive_mask == 0) return -1;

  uint32_t shift_mask = (core_alive_mask >> init_core_id)
                      | (core_alive_mask << (CONFIG_NR_CPUS - init_core_id));
  uint32_t offset = __builtin_ctz(shift_mask);
  return (init_core_id + offset) % CONFIG_NR_CPUS;
}

/* Logic for handling a core failure */
static void ramlog_flush_exp_handle(void)
{
  int cur_core_id = up_cpu_index();
  DEBUGASSERT(cur_core_id >= 0 && cur_core_id < CONFIG_NR_CPUS);
  /* Only mark a core as ‘abnormal’ once it has actually crashed */
  if (g_nx_initstate == OSINIT_PANIC)
  {
    ramlog_mark_core_exp(cur_core_id);
  }

  /* In the event that all cores fail, the final core will perform a synchronous buffer flush */
  if (ramlog_is_all_core_exp() == true)
  {
    ramlog_flush_internal(&g_sysdev);
  }
}

#endif

/****************************************************************************
 * Name: ramlog_copybuf
 ****************************************************************************/

static void ramlog_copybuf(FAR struct ramlog_dev_s *priv,
                          FAR const char *buffer, size_t len)
{
  FAR struct ramlog_header_s *header = priv->rl_header;
  FAR char *buf = header->rl_buffer;
  uint32_t offset;
  uint32_t tail;

  if (len <= 0)
    {
      return;
    }

  offset = header->rl_head % priv->rl_bufsize;
  tail = priv->rl_bufsize - offset;

  if (len > tail)
    {
      memcpy(&buf[offset], buffer, tail);
      memcpy(buf, buffer + tail, len - tail);
    }
  else
    {
      memcpy(&buf[offset], buffer, len);
    }

  header->rl_head += len;
}

/****************************************************************************
 * Name: ramlog_addbuf
 ****************************************************************************/

static ssize_t ramlog_addbuf(FAR struct ramlog_dev_s *priv,
                             FAR const char *buffer, size_t len)
{
#ifdef CONFIG_RAMLOG_SYSLOG
  FAR struct ramlog_header_s *header;
#endif
  size_t buflen = len;
  irqstate_t flags;

  flags = spin_lock_irqsave(&g_ramlog_addbuf_lock);

#ifdef CONFIG_RAMLOG_SYSLOG
  ramlog_syslog_initialize();

  header = priv->rl_header;

  if (header->rl_magic != RAMLOG_MAGIC_NUMBER && priv == &g_sysdev)
    {
#  ifdef RAMLOG_BUFFER_SECTION
      memset(header, 0, sizeof(g_sysbuffer));
#  endif
      header->rl_magic = RAMLOG_MAGIC_NUMBER;
    }
#endif

  if (buflen > priv->rl_bufsize)
    {
      buffer += buflen - priv->rl_bufsize;
      buflen = priv->rl_bufsize;
    }

  ramlog_copybuf(priv, buffer, buflen);

  /* Was anything written? */

  if (len > 0)
    {
      /* Lock the scheduler do NOT switch out */

      if (!list_is_empty(&priv->rl_list))
        {
          if (!up_interrupt_context())
            {
              sched_lock();
            }

#ifndef CONFIG_RAMLOG_NONBLOCKING
          /* Are there threads waiting for read data? */

          ramlog_readnotify(priv);
#endif
          /* Notify all poll/select waiters that they
           * can read from the FIFO
           */

          ramlog_pollnotify(priv);

          /* Unlock the scheduler */

          if (!up_interrupt_context())
            {
              sched_unlock();
            }
        }

#ifdef CONFIG_RAMLOG_FLUSH
      else if (priv == &g_sysdev && OSINIT_MM_READY()) {
        int selected_core_id = ramlog_select_flush_core((int)CONFIG_RAMLOG_FLUSH_CORE_ID_INIT);
        if (selected_core_id != -1)
        {
          if(atomic_load(&ramlog_flush_tid[selected_core_id]) > 0)
          {
            int cur_core_id = up_cpu_index();
            if(cur_core_id == selected_core_id)
            {
              nxsem_post(&ramlog_flush_sem[selected_core_id]);
            }
            else
            {
              vcos_ipicall_nxsem_post(&ramlog_flush_sem[selected_core_id], false, selected_core_id, 0);
            }
          }
        }
      }
#endif

    }

  /* We always have to return the number of bytes requested and NOT the
   * number of bytes that were actually written.  Otherwise, callers
   * probably retry, causing same error condition again.
   */

  spin_unlock_irqrestore(&g_ramlog_addbuf_lock, flags);

  return len;
}

/****************************************************************************
 * Name: ramlog_read
 ****************************************************************************/

static ssize_t ramlog_file_read(FAR struct file *filep, FAR char *buffer,
                                size_t len)
{
  FAR struct inode *inode = filep->f_inode;
  FAR struct ramlog_dev_s *priv = inode->i_private;
  FAR struct ramlog_header_s *header = priv->rl_header;
  FAR struct ramlog_user_s *upriv = filep->f_priv;
  irqstate_t flags;
  uint32_t ncopy;
  ssize_t nread;
  uint32_t tail;
  uint32_t pos;

  /* If the circular buffer is empty, then wait for something to be written
   * to it.  This function may NOT be called from an interrupt handler.
   */

  DEBUGASSERT(!up_interrupt_context());

  /* Get exclusive access to the rl_tail index */

  flags = enter_critical_section();

  /* Loop until something is read */

  for (nread = 0; (size_t)nread < len; )
    {
      /* Get the next byte from the buffer */

      if (header->rl_head == upriv->rl_tail)
        {
          /* The circular buffer is empty. */

#ifdef CONFIG_RAMLOG_NONBLOCKING
          /* Return what we have (with zero mean the end-of-file) */

          break;
#else
          int ret;

          /* Did we read anything? */

          if (nread > 0)
            {
              /* Yes.. break out to return what we have. */

              break;
            }

          /* If the driver was opened with O_NONBLOCK option, then don't
           * wait.
           */

          if (filep->f_oflags & O_NONBLOCK)
            {
              nread = -EAGAIN;
              break;
            }

          /* We may now be pre-empted!  But that should be okay because we
           * have already incremented nwaiters.  Pre-emptions is disabled
           * but will be re-enabled while we are waiting.
           */

          ret = nxsem_wait(&upriv->rl_waitsem);

          /* Did we successfully get the rl_waitsem? */

          if (ret < 0)
            {
              /* No.. nxsem_wait's failed. */

              /* Return the error. We did handle the case where we read
               * anything already before waiting.
               */

              return ret;
            }
#endif /* CONFIG_RAMLOG_NONBLOCKING */
        }
      else
        {
          /* Determine whether the read pointer is overwritten */

          if (header->rl_head - upriv->rl_tail > priv->rl_bufsize)
            {
              upriv->rl_tail = header->rl_head - priv->rl_bufsize;
            }

          /* The circular buffer is not empty, get the next byte from the
           * tail index.
           */

          pos = upriv->rl_tail % priv->rl_bufsize;
          ncopy = header->rl_head - upriv->rl_tail;
          tail = priv->rl_bufsize - pos;

          if (ncopy > len - nread)
            {
              ncopy = len - nread;
            }

          if (ncopy > tail)
            {
              memcpy(&buffer[nread], &header->rl_buffer[pos], tail);
              memcpy(&buffer[nread + tail], header->rl_buffer, ncopy - tail);
            }
          else
            {
              memcpy(&buffer[nread], &header->rl_buffer[pos], ncopy);
            }

          upriv->rl_tail += ncopy;
          nread += ncopy;
        }
    }

  leave_critical_section(flags);

  /* Return the number of characters actually read */

  return nread;
}

/****************************************************************************
 * Name: ramlog_file_write
 ****************************************************************************/

static ssize_t ramlog_file_write(FAR struct file *filep,
                                 FAR const char *buffer, size_t len)
{
  FAR struct inode *inode = filep->f_inode;
  FAR struct ramlog_dev_s *priv = inode->i_private;

  return ramlog_addbuf(priv, buffer, len);
}

/****************************************************************************
 * Name: ramlog_file_ioctl
 ****************************************************************************/

static int ramlog_file_ioctl(FAR struct file *filep, int cmd,
                             unsigned long arg)
{
  FAR struct inode *inode = filep->f_inode;
  FAR struct ramlog_dev_s *priv = inode->i_private;
  FAR struct ramlog_user_s *upriv = filep->f_priv;
  irqstate_t flags;
  int ret = 0;

  flags = enter_critical_section();

  switch (cmd)
    {
      case FIONREAD:
        *(FAR int *)((uintptr_t)arg) = ramlog_bufferused(priv, upriv);
        break;
      case PIPEIOC_POLLINTHRD:
        upriv->rl_threashold = (uint32_t)arg;
        break;
      case BIOC_FLUSH:
        ramlog_bufferflush(priv);
        break;
      default:
        ret = -ENOTTY;
        break;
    }

  leave_critical_section(flags);
  return ret;
}

/****************************************************************************
 * Name: ramlog_file_poll
 ****************************************************************************/

static int ramlog_file_poll(FAR struct file *filep, FAR struct pollfd *fds,
                            bool setup)
{
  FAR struct inode *inode = filep->f_inode;
  FAR struct ramlog_dev_s *priv = inode->i_private;
  FAR struct ramlog_user_s *upriv = filep->f_priv;
  pollevent_t eventset = POLLOUT;
  irqstate_t flags;

  /* Get exclusive access to the poll structures */

  flags = enter_critical_section();

  /* Are we setting up the poll?  Or tearing it down? */

  if (setup)
    {
      /* This is a request to set up the poll.  Find an available
       * slot for the poll structure reference.
       */

      if (!upriv->rl_fds)
        {
          upriv->rl_fds = fds;
          fds->priv     = &upriv->rl_fds;
        }

      /* Should immediately notify on any of the requested events? */

      /* Check if the receive buffer is not empty. */

      if (ramlog_bufferused(priv, upriv) >= upriv->rl_threashold)
        {
          eventset |= POLLIN;
        }

      poll_notify(&fds, 1, eventset);
    }
  else if (fds->priv)
    {
      /* This is a request to tear down the poll. */

      FAR struct pollfd **slot = (FAR struct pollfd **)fds->priv;

      /* Remove all memory of the poll setup */

      *slot     = NULL;
      fds->priv = NULL;
    }

  leave_critical_section(flags);
  return 0;
}

/****************************************************************************
 * Name: ramlog_file_open
 ****************************************************************************/

static int ramlog_file_open(FAR struct file *filep)
{
  FAR struct inode *inode = filep->f_inode;
  FAR struct ramlog_dev_s *priv = inode->i_private;
  FAR struct ramlog_header_s *header = priv->rl_header;
  FAR struct ramlog_user_s *upriv;
  irqstate_t flags;

  /* Get exclusive access to the rl_tail index */

  upriv = kmm_zalloc(sizeof(FAR struct ramlog_user_s));
  if (upriv == NULL)
    {
      return -ENOMEM;
    }

  upriv->rl_threashold = CONFIG_RAMLOG_POLLTHRESHOLD;
#ifndef CONFIG_RAMLOG_NONBLOCKING
  nxsem_init(&upriv->rl_waitsem, 0, 0);
#endif

  flags = enter_critical_section();
  list_add_tail(&priv->rl_list, &upriv->rl_node);
  upriv->rl_tail = header->rl_head > priv->rl_bufsize ?
                   header->rl_head - priv->rl_bufsize : 0;
  leave_critical_section(flags);

  filep->f_priv = upriv;
  return 0;
}

/****************************************************************************
 * Name: ramlog_file_close
 ****************************************************************************/

static int ramlog_file_close(FAR struct file *filep)
{
  FAR struct ramlog_user_s *upriv = filep->f_priv;
  irqstate_t flags;

  /* Get exclusive access to the rl_tail index */

  flags = enter_critical_section();
  list_delete(&upriv->rl_node);
  leave_critical_section(flags);

#ifndef CONFIG_RAMLOG_NONBLOCKING
  nxsem_destroy(&upriv->rl_waitsem);
#endif
  kmm_free(upriv);
  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ramlog_flush
 *
 * Description:
 *   This is called by system crash-handling logic.  It must flush any
 *   buffered data to the SYSLOG device.
 *
 *   Interrupts are disabled at the time of the crash and this logic must
 *   perform the flush using low-level, non-interrupt driven logic.
 *
 ****************************************************************************/

#ifdef CONFIG_RAMLOG_FLUSH
int ramlog_flush(FAR syslog_channel_t *channel)
{
#ifdef CONFIG_RAMLOG_SYSLOG
  ramlog_syslog_initialize();
#endif
#ifdef CONFIG_RAMLOG_FLUSH
  ramlog_flush_exp_handle();
#endif
  return 0;
}
#endif

/****************************************************************************
 * Name: ramlog_register
 *
 * Description:
 *   Create the RAM logging device and register it at the specified path.
 *
 ****************************************************************************/

int ramlog_register(FAR const char *devpath, FAR char *buffer, size_t buflen)
{
  FAR struct ramlog_dev_s *priv;
  int ret = -ENOMEM;

  /* Sanity checking */

  DEBUGASSERT(devpath && buffer && buflen > sizeof(struct ramlog_header_s));

  /* Allocate a RAM logging device structure */

  priv = kmm_zalloc(sizeof(struct ramlog_dev_s));
  if (priv != NULL)
    {
      /* Initialize the non-zero values in the RAM logging device structure */

      list_initialize(&priv->rl_list);
      priv->rl_bufsize = buflen - sizeof(struct ramlog_header_s);
      priv->rl_header = (FAR struct ramlog_header_s *)buffer;

      /* Register the character driver */

      ret = register_driver(devpath, NULL, 0666, priv);
      if (ret < 0)
        {
          kmm_free(priv);
        }
    }

  return ret;
}

/****************************************************************************
 * Name: ramlog_syslog_register
 *
 * Description:
 *   Use a pre-allocated RAM logging device and register it at the path
 *   specified by CONFIG_RAMLOG_SYSLOG
 *
 ****************************************************************************/

#ifdef CONFIG_RAMLOG_SYSLOG
static void ramlog_syslog_initialize(void)
{
  /* Register the syslog character driver */

  if (g_sysdev.rl_header == NULL)
    {
      g_sysdev.rl_header = (FAR void *)g_sysbuffer;
      g_sysdev.rl_bufsize = sizeof(g_sysbuffer) -
                            sizeof(struct ramlog_header_s);
      list_initialize(&g_sysdev.rl_list);
    }

#ifdef CONFIG_RAMLOG_FLUSH
  /* Create the initial buffer-flush thread */
  int cur_core_id = up_cpu_index();
  if (atomic_load(&ramlog_flush_tid[cur_core_id]) == 0)
  {
    ramlog_flush_thread_create();
  }
#endif
}

void ramlog_syslog_register(void)
{
  /* Register the syslog character driver */

  ramlog_syslog_initialize();

  register_driver(CONFIG_SYSLOG_DEVPATH, NULL, 0666, &g_sysdev);
}
#endif

/****************************************************************************
 * Name: ramlog_putc
 *
 * Description:
 *   This is the low-level system logging interface.
 *
 ****************************************************************************/

#ifdef CONFIG_RAMLOG_SYSLOG
int ramlog_putc(FAR syslog_channel_t *channel, int ch)
{
  char cch = ch;

  UNUSED(channel);

  /* Add the character to the RAMLOG */

  ramlog_addbuf(&g_sysdev, &cch, 1);

  /* Return the character added on success */

  return ch;
}

ssize_t ramlog_write(FAR syslog_channel_t *channel,
                     FAR const char *buffer, size_t buflen)
{
  return ramlog_addbuf(&g_sysdev, buffer, buflen);
}
#endif

#endif /* CONFIG_RAMLOG */
