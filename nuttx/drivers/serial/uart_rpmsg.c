/****************************************************************************
 * drivers/serial/uart_rpmsg.c
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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <nuttx/fs/ioctl.h>
#include <nuttx/kmalloc.h>
#include <nuttx/mutex.h>
#include <nuttx/rpmsg/rpmsg.h>
#include <nuttx/serial/serial.h>
#include <nuttx/serial/uart_rpmsg.h>

/****************************************************************************
 * Pre-processor definitions
 ****************************************************************************/

#define UART_RPMSG_DEV_CONSOLE          "/dev/console"
#define UART_RPMSG_DEV_PREFIX           "/dev/tty"
#define UART_RPMSG_EPT_PREFIX           "rpmsg-tty"

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct uart_rpmsg_priv_s
{
  struct rpmsg_endpoint ept;
  mutex_t               lock;
  FAR const char        *devname;
  FAR const char        *cpuname;
  FAR void              *recv_data;
  size_t                recv_size;
  bool                  last_upper;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  uart_rpmsg_setup(FAR struct uart_dev_s *dev);
static void uart_rpmsg_shutdown(FAR struct uart_dev_s *dev);
static int  uart_rpmsg_attach(FAR struct uart_dev_s *dev);
static void uart_rpmsg_detach(FAR struct uart_dev_s *dev);
static void uart_rpmsg_rxint(FAR struct uart_dev_s *dev, bool enable);
static bool uart_rpmsg_rxflowcontrol(FAR struct uart_dev_s *dev,
                                     unsigned int nbuffered, bool upper);
static void uart_rpmsg_dmasend(FAR struct uart_dev_s *dev);
static void uart_rpmsg_dmareceive(FAR struct uart_dev_s *dev);
static void uart_rpmsg_dmarxfree(FAR struct uart_dev_s *dev);
static void uart_rpmsg_dmatxavail(FAR struct uart_dev_s *dev);
static void uart_rpmsg_send(FAR struct uart_dev_s *dev, int ch);
static void uart_rpmsg_txint(FAR struct uart_dev_s *dev, bool enable);
static bool uart_rpmsg_txready(FAR struct uart_dev_s *dev);
static bool uart_rpmsg_txempty(FAR struct uart_dev_s *dev);
static void uart_rpmsg_device_created(FAR struct rpmsg_device *rdev,
                                      FAR void *priv_);
static void uart_rpmsg_device_destroy(FAR struct rpmsg_device *rdev,
                                      FAR void *priv_);
static int  uart_rpmsg_ept_cb(struct rpmsg_endpoint *ept, FAR void *data,
                              size_t len, uint32_t src, FAR void *priv_);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct uart_ops_s g_uart_rpmsg_ops =
{
  .setup         = uart_rpmsg_setup,
  .shutdown      = uart_rpmsg_shutdown,
  .attach        = uart_rpmsg_attach,
  .detach        = uart_rpmsg_detach,
  .rxint         = uart_rpmsg_rxint,
  .rxflowcontrol = uart_rpmsg_rxflowcontrol,
  .dmasend       = uart_rpmsg_dmasend,
  .dmareceive    = uart_rpmsg_dmareceive,
  .dmarxfree     = uart_rpmsg_dmarxfree,
  .dmatxavail    = uart_rpmsg_dmatxavail,
  .send          = uart_rpmsg_send,
  .txint         = uart_rpmsg_txint,
  .txready       = uart_rpmsg_txready,
  .txempty       = uart_rpmsg_txempty,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int uart_rpmsg_setup(FAR struct uart_dev_s *dev)
{
  return OK;
}

static void uart_rpmsg_shutdown(FAR struct uart_dev_s *dev)
{
}

static int uart_rpmsg_attach(FAR struct uart_dev_s *dev)
{
  return OK;
}

static void uart_rpmsg_detach(FAR struct uart_dev_s *dev)
{
}

static void uart_rpmsg_rxint(FAR struct uart_dev_s *dev, bool enable)
{
}

static bool uart_rpmsg_rxflowcontrol(FAR struct uart_dev_s *dev,
                                     unsigned int nbuffered, bool upper)
{
  return false;
}

static void uart_rpmsg_dmasend(FAR struct uart_dev_s *dev)
{
  FAR struct uart_rpmsg_priv_s *priv = dev->priv;
  FAR struct uart_dmaxfer_s *xfer = &dev->dmatx;

  void *msg;
  size_t len = xfer->length + xfer->nlength;
  uint32_t space;

  msg = rpmsg_get_tx_payload_buffer(&priv->ept, &space, true);
  if (!msg)
    {
      dev->dmatx.length = 0;
      return;
    }

  if (len > space)
    {
      len = space;
    }

  memset(msg, 0, len);

  if (len > xfer->length)
    {
      memcpy(msg, xfer->buffer, xfer->length);
      memcpy(msg + xfer->length, xfer->nbuffer, len - xfer->length);
    }
  else
    {
      memcpy(msg, xfer->buffer, len);
    }

  if (rpmsg_send_nocopy(&priv->ept, msg, len) < 0)
    {
      rpmsg_release_tx_buffer(&priv->ept, msg);
    }

  uart_xmitchars_done(dev);
  dev->xmit.tail = dev->xmit.head;
}

static void uart_rpmsg_dmareceive(FAR struct uart_dev_s *dev)
{
  FAR struct uart_rpmsg_priv_s *priv = dev->priv;
  FAR struct uart_dmaxfer_s *xfer = &dev->dmarx;

  void *msg    = priv->recv_data;
  uint32_t len = priv->recv_size;
  size_t space = xfer->length + xfer->nlength;

  if (len > space)
    {
      len = space;
    }

  if (len > xfer->length)
    {
      memcpy(xfer->buffer, msg, xfer->length);
      memcpy(xfer->nbuffer, msg + xfer->length, len - xfer->length);
    }
  else
    {
      memcpy(xfer->buffer, msg, len);
    }

  xfer->nbytes = len;
  uart_recvchars_done(dev);
}

static void uart_rpmsg_dmarxfree(FAR struct uart_dev_s *dev)
{
}

static void uart_rpmsg_dmatxavail(FAR struct uart_dev_s *dev)
{
  FAR struct uart_rpmsg_priv_s *priv = dev->priv;

  nxmutex_lock(&priv->lock);

  if (is_rpmsg_ept_ready(&priv->ept))
    {
      uart_xmitchars_dma(dev);
    }

  nxmutex_unlock(&priv->lock);
}

static void uart_rpmsg_send(FAR struct uart_dev_s *dev, int ch)
{
  int nexthead;

  nexthead = dev->xmit.head + 1;
  if (nexthead >= dev->xmit.size)
    {
      nexthead = 0;
    }

  if (nexthead != dev->xmit.tail)
    {
      /* No.. not full.  Add the character to the TX buffer and return. */

      dev->xmit.buffer[dev->xmit.head] = ch;
      dev->xmit.head = nexthead;
    }

  uart_rpmsg_dmatxavail(dev);
}

static void uart_rpmsg_txint(FAR struct uart_dev_s *dev, bool enable)
{
}

static bool uart_rpmsg_txready(FAR struct uart_dev_s *dev)
{
  int nexthead;

  nexthead = dev->xmit.head + 1;
  if (nexthead >= dev->xmit.size)
    {
      nexthead = 0;
    }

  return nexthead != dev->xmit.tail;
}

static bool uart_rpmsg_txempty(FAR struct uart_dev_s *dev)
{
  return true;
}

static void uart_rpmsg_device_created(FAR struct rpmsg_device *rdev,
                                      FAR void *priv_)
{
  FAR struct uart_dev_s *dev = priv_;
  FAR struct uart_rpmsg_priv_s *priv = dev->priv;
  char eptname[RPMSG_NAME_SIZE];

  if (strcmp(priv->cpuname, rpmsg_get_cpuname(rdev)) == 0)
    {
      priv->ept.priv = dev;
      snprintf(eptname, sizeof(eptname), "%s-%s",
               UART_RPMSG_EPT_PREFIX, priv->devname);
      rpmsg_create_ept(&priv->ept, rdev, eptname,
                       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                       uart_rpmsg_ept_cb, NULL);
    }
}

static void uart_rpmsg_device_destroy(FAR struct rpmsg_device *rdev,
                                      FAR void *priv_)
{
  FAR struct uart_dev_s *dev = priv_;
  FAR struct uart_rpmsg_priv_s *priv = dev->priv;

  if (priv->ept.priv != NULL &&
      strcmp(priv->cpuname, rpmsg_get_cpuname(rdev)) == 0)
    {
      rpmsg_destroy_ept(&priv->ept);
    }

  dev->dmatx.nbytes = dev->dmatx.length + dev->dmatx.nlength;
  uart_xmitchars_done(dev);
}

static int uart_rpmsg_ept_cb(FAR struct rpmsg_endpoint *ept, FAR void *data,
                             size_t len, uint32_t src, FAR void *priv_)
{
  FAR struct uart_dev_s *dev = priv_;
  FAR struct uart_rpmsg_priv_s *priv = dev->priv;

  nxmutex_lock(&dev->recv.lock);
  priv->recv_data = data;
  priv->recv_size = len;
  uart_recvchars_dma(dev);
  priv->recv_data = NULL;
  priv->recv_size = 0;
  nxmutex_unlock(&dev->recv.lock);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int uart_rpmsg_init(FAR const char *cpuname, FAR const char *devname,
                    int buf_size, bool isconsole)
{
  FAR struct uart_rpmsg_priv_s *priv;
  FAR struct uart_dev_s *dev;
  char dev_name[32];
  int ret = -ENOMEM;

  dev = kmm_zalloc(sizeof(struct uart_dev_s));
  if (!dev)
    {
      return ret;
    }

  dev->ops       = &g_uart_rpmsg_ops;
  dev->isconsole = isconsole;
  dev->recv.size = buf_size;
  dev->xmit.size = buf_size;

  dev->recv.buffer = kmm_malloc(dev->recv.size);
  if (!dev->recv.buffer)
    {
      goto fail;
    }

  dev->xmit.buffer = kmm_malloc(dev->xmit.size);
  if (!dev->xmit.buffer)
    {
      goto fail;
    }

  priv = kmm_zalloc(sizeof(struct uart_rpmsg_priv_s));
  if (!priv)
    {
      goto fail;
    }

  nxmutex_init(&priv->lock);
  priv->cpuname = cpuname;
  priv->devname = devname;

  dev->priv = priv;

  ret = rpmsg_register_callback(dev,
                                uart_rpmsg_device_created,
                                uart_rpmsg_device_destroy,
                                NULL,
                                NULL);
  if (ret < 0)
    {
      nxmutex_destroy(&priv->lock);
      goto fail;
    }

  snprintf(dev_name, sizeof(dev_name), "%s%s",
           UART_RPMSG_DEV_PREFIX, devname);
  uart_register(dev_name, dev);

  if (dev->isconsole)
    {
      uart_register(UART_RPMSG_DEV_CONSOLE, dev);
    }

  return OK;

fail:
  kmm_free(dev->recv.buffer);
  kmm_free(dev->xmit.buffer);
  kmm_free(dev->priv);
  kmm_free(dev);

  return ret;
}
