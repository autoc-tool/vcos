/****************************************************************************
 * drivers/rpmsg/rpmsg_ping.c
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
#include <nuttx/arch.h>
#include <nuttx/lib/lib.h>
#include <nuttx/signal.h>

#include <inttypes.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>
#include <syslog.h>

#include "rpmsg_ping.h"

/****************************************************************************
 * Pre-processor definitions
 ****************************************************************************/

#define RPMSG_PING_EPT_NAME         "rpmsg-ping"

#define RPMSG_PING_ACK_MASK         0x00000001
#define RPMSG_PING_CHECK_MASK       0x00000002
#define RPMSG_PING_RANDOMLEN_MASK   0x00000004

#define RPMSG_PING_CMD_MASK         0x000000f0
#  define RPMSG_PING_CMD_REQ        0x00000000
#  define RPMSG_PING_CMD_RSP        0x00000020

/****************************************************************************
 * Private Types
 ****************************************************************************/

begin_packed_struct struct rpmsg_ping_msg_s
{
  uint32_t cmd;
  uint32_t len;
  uint64_t cookie;
  uint8_t  data[1];
} end_packed_struct;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_CLOCK
static clock_t timespec_diff(struct timespec *start, struct timespec *end)
{
  struct timespec diff;
  if ((end->tv_nsec - start->tv_nsec) < 0)
    {
      diff.tv_sec = end->tv_sec - start->tv_sec - 1;
      diff.tv_nsec = NSEC_PER_SEC + end->tv_nsec - start->tv_nsec;
    }
  else
    {
      diff.tv_sec = end->tv_sec - start->tv_sec;
      diff.tv_nsec = end->tv_nsec - start->tv_nsec;
    }
  return (diff.tv_sec * CLOCKS_PER_SEC) + (diff.tv_nsec * CLOCKS_PER_SEC / NSEC_PER_SEC);
}

static uint64_t timespec_diff_ns(struct timespec *start, struct timespec *end)
{
  uint64_t diff_ns;

  if ((end->tv_nsec - start->tv_nsec) < 0)
    {
      diff_ns = (end->tv_sec - start->tv_sec - 1) * NSEC_PER_SEC + (NSEC_PER_SEC + end->tv_nsec - start->tv_nsec);
    }
  else
    {
      diff_ns = (end->tv_sec - start->tv_sec) * NSEC_PER_SEC + (end->tv_nsec - start->tv_nsec);
    }

  return diff_ns;
}
#endif

static int rpmsg_ping_ept_cb(FAR struct rpmsg_endpoint *ept,
                             FAR void *data, size_t len, uint32_t src,
                             FAR void *priv)
{
  FAR struct rpmsg_ping_msg_s *msg = data;
  FAR sem_t *sem = (FAR sem_t *)(uintptr_t)msg->cookie;

  /* Handle the ping response */

  if ((msg->cmd & RPMSG_PING_CMD_MASK) == RPMSG_PING_CMD_RSP)
    {
      nxsem_post(sem);
      return 0;
    }

  /* Handle the ping request */

  if ((msg->cmd & RPMSG_PING_ACK_MASK) == 0)
    {
      return 0;
    }

  if ((msg->cmd & RPMSG_PING_CHECK_MASK) != 0)
    {
      size_t data_len;
      size_t i;

      data_len = msg->len - sizeof(struct rpmsg_ping_msg_s) + 1;
      for (i = 0; i < data_len; i++)
        {
          if (msg->data[i] != msg->data[0])
            {
              syslog(LOG_ERR, "receive data error at %zu of %zu\n",
                     i, data_len);
              break;
            }
        }
    }

  if ((msg->cmd & RPMSG_PING_CMD_MASK) == RPMSG_PING_CMD_REQ)
    {
      msg->len = len;
    }
  else
    {
      msg->len = sizeof(*msg);
    }

  msg->cmd = RPMSG_PING_CMD_RSP;
  rpmsg_send(ept, msg, msg->len);
  return 0;
}

static int rpmsg_ping_once(FAR struct rpmsg_endpoint *ept, int len,
                           int cmd, uint32_t *buf_len, char i)
{
  FAR struct rpmsg_ping_msg_s *msg;
  int ret;

  msg = rpmsg_get_tx_payload_buffer(ept, buf_len, true);
  if (!msg)
    {
      return -ENOMEM;
    }

  msg->cmd = cmd;

  if ((msg->cmd & RPMSG_PING_RANDOMLEN_MASK) != 0)
    {
      msg->len = nrand(len);
    }
  else
    {
      msg->len = len;
    }

  msg->len = MAX(msg->len, sizeof(struct rpmsg_ping_msg_s));
  msg->len = MIN(msg->len, *buf_len);

  if ((msg->cmd & RPMSG_PING_CHECK_MASK) != 0)
    {
      memset(msg->data, i, msg->len - sizeof(struct rpmsg_ping_msg_s) + 1);
    }
  else
    {
      memset(msg->data, 0, msg->len - sizeof(struct rpmsg_ping_msg_s) + 1);
    }

  if ((msg->cmd & RPMSG_PING_ACK_MASK) != 0)
    {
      sem_t sem;

      msg->cookie = (uintptr_t)&sem;
      nxsem_init(&sem, 0, 0);

      ret = rpmsg_send_nocopy(ept, msg, msg->len);
      if (ret >= 0)
        {
          nxsem_wait_uninterruptible(&sem);
        }

      nxsem_destroy(&sem);
    }
  else
    {
      ret = rpmsg_send_nocopy(ept, msg, msg->len);
    }

  if (ret < 0)
    {
      rpmsg_release_tx_buffer(ept, msg);
    }

  return ret;
}

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
static void rpmsg_ping_logout(FAR const char *s, clock_t value)
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
static void rpmsg_ping_logout(FAR const char *s, struct timespec value)
#endif
{
#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
  struct timespec ts;

  perf_convert(value, &ts);

#ifdef CONFIG_SYSTEM_TIME64
  syslog(LOG_EMERG, "%s: %" PRIu64 " s, %ld ns\n", s, ts.tv_sec, ts.tv_nsec);
#else
  syslog(LOG_EMERG, "%s: %" PRIu32 " s, %ld ns\n", s, ts.tv_sec, ts.tv_nsec);
#endif
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
#ifdef CONFIG_SYSTEM_TIME64
  syslog(LOG_EMERG, "%s: %" PRIu64 " s, %ld ns\n", s, value.tv_sec, value.tv_nsec);
#else
  syslog(LOG_EMERG, "%s: %" PRIu32 " s, %ld ns\n", s, value.tv_sec, value.tv_nsec);
#endif
#endif
}

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
static void rpmsg_ping_logout_rate(uint64_t len, clock_t avg)
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
static void rpmsg_ping_logout_rate(uint64_t len, struct timespec avg)
#endif
{
#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
  struct timespec ts;
#endif
  size_t ratebits;
  size_t rateint;
  size_t ratedec;

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
  perf_convert(avg, &ts);
#endif

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
  ratebits = len * 8 * 1000000000 / (ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec);
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
  ratebits = len * 8 * 1000000000 / (avg.tv_sec * NSEC_PER_SEC + avg.tv_nsec);
#endif
  rateint = ratebits / 1000000;
  ratedec = ratebits - rateint * 1000000;

  syslog(LOG_EMERG, "rate: %zu.%06zu Mbits/sec\n", rateint, ratedec);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int rpmsg_ping(FAR struct rpmsg_endpoint *ept,
               FAR const struct rpmsg_ping_s *ping)
{
#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
  clock_t min = CLOCK_MAX;
  clock_t max = 0;
  uint64_t total = 0;
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
  struct timespec min = { .tv_sec = INT_MAX, .tv_nsec = INT_MAX };
  struct timespec max = { .tv_sec = 0, .tv_nsec = 0 };
  struct timespec total = { .tv_sec = 0, .tv_nsec = 0 };
  uint64_t min_ns = UINT64_MAX;
  uint64_t max_ns = 0;
  struct timespec avg;
#endif

  uint32_t buf_len = 0;
  int send_len = 0;
  int i;

  if (!ept || !ping || ping->times <= 0)
    {
      return -EINVAL;
    }

  for (i = 0; i < ping->times; i++)
    {
#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
      clock_t tm = perf_gettime();
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
      struct timespec start, end;
      uint64_t diff_ns;

      clock_gettime(CLOCK_MONOTONIC, &start);
#endif
      send_len = rpmsg_ping_once(ept, ping->len, ping->cmd, &buf_len, i);
      if (send_len < 0)
        {
          return send_len;
        }

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
      tm     = perf_gettime() - tm;
      min    = MIN(min, tm);
      max    = MAX(max, tm);
      total += tm;
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
      clock_gettime(CLOCK_MONOTONIC, &end);

      diff_ns = timespec_diff_ns(&start, &end);

      /* Update min */
      if (diff_ns < min_ns)
        {
          min_ns = diff_ns;
          min.tv_nsec = min_ns % NSEC_PER_SEC;
          min.tv_sec = min_ns / NSEC_PER_SEC;
        }

      /* Update max */
      if (diff_ns > max_ns)
        {
          max_ns = diff_ns;
          max.tv_nsec = max_ns % NSEC_PER_SEC;
          max.tv_sec = max_ns / NSEC_PER_SEC;
        }

      /* Update total */
      total.tv_nsec += diff_ns % NSEC_PER_SEC;
      total.tv_sec += diff_ns / NSEC_PER_SEC;

      while (total.tv_nsec >= NSEC_PER_SEC)
        {
          total.tv_nsec -= NSEC_PER_SEC;
          total.tv_sec++;
        }
#endif
      if (ping->sleep > 0)
        {
          nxsig_usleep(ping->sleep * USEC_PER_MSEC);
        }
    }

  syslog(LOG_EMERG, "ping times: %d\n", ping->times);
  syslog(LOG_EMERG, "buffer_len: %" PRIu32 ", send_len: %d\n", buf_len, send_len);

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
  rpmsg_ping_logout("avg", total / ping->times);
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
  rpmsg_ping_logout("avg", avg);
#endif

  rpmsg_ping_logout("min", min);
  rpmsg_ping_logout("max", max);

#ifdef CONFIG_RPMSG_PING_TIMESTAMP_PERF
  rpmsg_ping_logout_rate(send_len, total / ping->times);
#else /* CONFIG_RPMSG_PING_TIMESTAMP_CLOCK */
  avg.tv_sec = total.tv_sec / ping->times;
  avg.tv_nsec = total.tv_nsec / ping->times;
  rpmsg_ping_logout_rate(send_len, avg);
#endif

  return 0;
}

int rpmsg_ping_init(FAR struct rpmsg_device *rdev,
                    FAR struct rpmsg_endpoint *ept)
{
  return rpmsg_create_ept(ept, rdev, RPMSG_PING_EPT_NAME,
                          RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                          rpmsg_ping_ept_cb, NULL);
}

void rpmsg_ping_deinit(FAR struct rpmsg_endpoint *ept)
{
  rpmsg_destroy_ept(ept);
}
