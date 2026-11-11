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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/fs/ioctl.h>
#include <sys/types.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "trace.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: note_handshake
 *
 * Description:
 *   dump trace handshake data.
 *
 ****************************************************************************/

int note_handshake(int step_len)
{
  int fd;
  int ret;

  /* Open note for read */

  fd = open("/dev/note/handshake", 0);
  if (fd < 0)
    {
      fprintf(stderr, "trace: cannot open /dev/note/handshake\n");
      return ERROR;
    }

  ret = ioctl(fd, 0, (unsigned long)&step_len);
  if (ret < 0)
    {
      fprintf(stderr, "trace: ioctl error: %d, errno:%d\n", ret, errno);
      close(fd);
      fd = -1;
      return ERROR;
    }

  /* Close note */

  close(fd);
  fd = -1;
  return 0;
}

/****************************************************************************
 * Name: note_handshake_clean
 *
 * Description:
 *   clean handshake history.
 *
 ****************************************************************************/

int note_handshake_clean(void)
{
  int fd;
  int ret;

  /* Open note for read */

  fd = open("/dev/note/handshake", 0);
  if (fd < 0)
    {
      fprintf(stderr, "trace: cannot open /dev/note/handshake\n");
      return ERROR;
    }

  ret = ioctl(fd, 1, NULL);
  if (ret < 0)
    {
      fprintf(stderr, "trace: ioctl error: %d, errno:%d\n", ret, errno);
      close(fd);
      fd = -1;
      return ERROR;
    }

  /* Close note */

  close(fd);
  fd = -1;
  return 0;
}
