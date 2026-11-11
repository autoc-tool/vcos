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

#ifndef LWIP_HDR_LWIPOPTS_H
#define LWIP_HDR_LWIPOPTS_H

#include "string.h"
#include "Lwip_Cfg.h"
#include "ethif_cfg.h"
#include "Compiler.h"
#if (CONFIG_RT_FRAMEWORK_MEMPOOL == 1)
#include "mempool_cfg.h"
#endif

#define VCOS_MODULE_CONFIG_MEMPOOL STD_OFF



#define LWIP_NOASSERT                  1
#define PKTPRINT_DEBUG                 LWIP_DBG_ON
#define TCP_AUTOSAR_DEBUG              LWIP_DBG_OFF
#define UDP_AUTOSAR_DEBUG              LWIP_DBG_OFF
#define FAST_UDP_OPTIMIZED             (1u)                            ///< Fast UDP 优化开关
#define TCPIP_CROSS_CORE_API_ENABLE    (0u)

#define MEM_ALIGNMENT                   4
#define ETH_PAD_SIZE                    2

#define LWIP_TESTMODE                   1

#define LWIP_IPV6                       0
#define LWIP_RAW                        1
#define LWIP_SOCKET                     0
#define LWIP_DNS                        0
#define LWIP_IGMP                       1
#define LWIP_SO_RCVBUF                  1
#define LWIP_BROADCAST_PING             1

#define TCP_CHECKSUM_ON_COPY_SANITY_CHECK 1
#define TCP_CHECKSUM_ON_COPY_SANITY_CHECK_FAIL(printfmsg) LWIP_ASSERT("TCP_CHECKSUM_ON_COPY_SANITY_CHECK_FAIL", 0)

#define NO_SYS_NO_TIMERS                0
#define LWIP_TIMERS                     1

#define LWIP_NETCONN_SEM_PER_THREAD     0
#define LWIP_NETBUF_RECVINFO            1
#define LWIP_HAVE_LOOPIF                1

#define MEMP_NUM_TCP_SEG                TCP_SND_QUEUELEN
#define MEMP_NUM_ALTCP_TCP_SEG          TCP_SND_QUEUELEN

#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   0

#define LWIP_MDNS_RESPONDER             1
#define LWIP_NUM_NETIF_CLIENT_DATA      (LWIP_MDNS_RESPONDER)

#define ETHARP_SUPPORT_STATIC_ENTRIES   1

#define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 8)

#define LWIP_NETIF_EXT_STATUS_CALLBACK  0

#define LWIP_MEM_ILLEGAL_FREE(msg)


#define TCPIP_THREAD_STACKSIZE          2048
#define TCPIP_THREAD_PRIO               9

#define TCPIP_MBOX_SIZE                 MAX_QUEUE_ENTRIES
#undef DEFAULT_TCP_RECVMBOX_SIZE
#define DEFAULT_TCP_RECVMBOX_SIZE       MAX_QUEUE_ENTRIES

#define DEFAULT_UDP_RECVMBOX_SIZE		MAX_QUEUE_ENTRIES

#undef DEFAULT_ACCEPTMBOX_SIZE
#define DEFAULT_ACCEPTMBOX_SIZE         MAX_QUEUE_ENTRIES

#undef	DEFAULT_RAW_RECVMBOX_SIZE
#define DEFAULT_RAW_RECVMBOX_SIZE		MAX_QUEUE_ENTRIES

#undef MEMP_NUM_NETBUF
#define MEMP_NUM_NETBUF                 MAX_QUEUE_ENTRIES

#define E_NO_DTC_AVAILABLE      (Std_ReturnType)2u
#define E_SESSION_NOT_ALLOWED   (Std_ReturnType)4u
#define E_PROTOCOL_NOT_ALLOWED  (Std_ReturnType)5u
#define E_REQUEST_NOT_ACCEPTED  (Std_ReturnType)8u
#define E_REQUEST_ENV_NOK       (Std_ReturnType)9u
#define E_PENDING               (Std_ReturnType)10u
#define E_COMPARE_KEY_FAILED    (Std_ReturnType)11u
#define E_FORCE_RCRRP           (Std_ReturnType)12u

#define LWIP_TCPIP_CORE_LOCKING       1
#define LWIP_TCPIP_CORE_LOCKING_INPUT 0
#define LWIP_TCP_TX_USE_MAILBOX 0
#define LWIP_TCP_LISTEN_USE_MAILBOX 0
#define LWIP_TCP_CONNECT_USE_MAILBOX 0
#define LWIP_TCP_CLOSE_USE_MAILBOX 0
#define LWIP_DISABLE_SYS_MUTEX 0
#define LWIP_NETIF_API 1
#define LWIP_MTU_SIZE 1500u

#define LWIP_SO_RCVTIMEO            1
#define LWIP_CHKSUM_ALGORITHM       3
/* Use hostname */
#define LWIP_NETIF_HOSTNAME 1
// We want to be notified when the link changes status.
#define LWIP_NETIF_LINK_CALLBACK 1

/**
 * SYS_LIGHTWEIGHT_PROT==true: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    1

/**
 * NO_SYS==true: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  0

/*   ---------- Sequential layer options ---------- */
/**
 * LWIP_NETCONN==true: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                     0

/* ---------- Memory options ---------- */
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 1

/**
 * MEM_LIBC_MALLOC==true: Use malloc/free/realloc provided by your C-library
 * instead of the lwip internal allocator. Can save code size if you
 * already use it.
 */
#define MEM_LIBC_MALLOC                 0

/**
* MEMP_MEM_MALLOC==true: Use mem_malloc/mem_free instead of the lwip pool allocator.
* Especially useful with MEM_LIBC_MALLOC but handle with care regarding execution
* speed and usage from interrupts!
*/
#define MEMP_MEM_MALLOC                 0

#if defined (tcpip_mempool_auto_alloc)
/**
 * MEM_USE_POOLS==1: Use an alternative to malloc() by allocating from a set
 * of memory pools of various sizes. When mem_malloc is called, an element of
 * the smallest pool that can provide the length needed is returned.
 * To use this, MEMP_USE_CUSTOM_POOLS also has to be enabled.
 */
#define MEM_USE_POOLS                  (0)

/**
 * MEMP_USE_CUSTOM_POOLS==1: whether to include a user file lwippools.h
 * that defines additional pools beyond the "standard" ones required
 * by lwIP. If you set this to 1, you must have lwippools.h in your
 * include path somewhere.
 */
#define MEMP_USE_CUSTOM_POOLS          (0)


#define MEM_USE_GLOBAL_POOLS           (1)
#else
#define MEM_USE_POOLS                  (1)
#define MEMP_USE_CUSTOM_POOLS          (1)
#define MEM_USE_GLOBAL_POOLS           (0)
#endif

/**
 * MEM_USE_POOLS_TRY_BIGGER_POOL==1: if one malloc-pool is empty, try the next
 * bigger pool - WARNING: THIS MIGHT WASTE MEMORY but it can make a system more
 * reliable. */
#define MEM_USE_POOLS_TRY_BIGGER_POOL  (1)
#define MAX_LISTENING_SOCKETS_TCP  MAX_SOCKETS_TCP
#define MEMP_NUM_NETCONN    (MAX_SOCKETS_TCP + \
                             MAX_LISTENING_SOCKETS_TCP + \
                             MAX_SOCKETS_UDP)

#define MEMP_NUM_UDP_PCB               (0)
#define MEMP_NUM_TCP_PCB                MAX_SOCKETS_TCP
#define MEMP_NUM_TCP_PCB_LISTEN         MAX_SOCKETS_TCP
#define MEMP_NUM_TCPIP_MSG_INPKT           20

#define AUTOSAR_TCP_SEND_DONTWAIT          0
#define LWIP_CHECKSUM_ON_COPY              1
#define LWIP_DSTIP_ROUTE                   1
#define LWIP_TC8_SWITCH                    1
#define TCPIP_ENABLE_UDP_TX_CONFIRMATION   0
#define TCPIP_FAST_TRANS_ENABLE            1

#if LWIP_TC8_SWITCH
#define TCP_TMR_INTERVAL 125
#endif

/* buffer copy handling. */
#if defined (CFG_ZYNQ)
#define MEMCPY(dst,src,len)             memcpy((uint8*)dst,(uint8*)src,len)

#define SMEMCPY(dst,src,len)            memcpy((uint8*)dst,(uint8*)src,len)

#define ip_addr_copy(dest, src)         IPADDR2_COPY(&(dest).addr,&(src).addr)
#endif

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          0

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       1540

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        (5 * (TCP_SND_BUF / TCP_MSS))

/* TCP receive window. */
#define LWIP_TCP_RCVWND_MAX     1

/**
 * TCP_LISTEN_BACKLOG: Enable the backlog option for tcp listen pcb.
 */
#define TCP_LISTEN_BACKLOG              1

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           (MAX_SOCKETS_UDP + TCP_SND_QUEUELEN)

#define LWIP_PROVIDE_ERRNO     1

/* --------------------------------------
   ---------- Checksum options ----------
   -------------------------------------- */
#if 0

/* CHECKSUM_BY_HARDWARE==true: Enables the setting of the checksums by hardware for UDPTCPICMP on stm32 and mpc*/
#define CHECKSUM_BY_HARDWARE            1
/* CHECKSUM_GEN_IP==true: Generate checksums in software for outgoing IP packets.*/
#define CHECKSUM_GEN_IP                 0
/* CHECKSUM_GEN_UDP==true: Generate checksums in software for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP                0
/* CHECKSUM_GEN_TCP==true: Generate checksums in software for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP                0
/* CHECKSUM_GEN_ICMP==true: Generate checksums in software for incoming TCP packets.*/
#define CHECKSUM_GEN_ICMP               0

/* CHECKSUM_CHECK_IP==true: Check checksums in software for incoming IP packets.*/
#define CHECKSUM_CHECK_IP               0
/* CHECKSUM_CHECK_UDP==true: Check checksums in software for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP              0
/* CHECKSUM_CHECK_TCP==true: Check checksums in software for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP              0
/**
 * LWIP_CHECKSUM_CTRL_PER_NETIF==1: Checksum generation/check can be enabled/disabled
 * per netif.
 * ATTENTION: if enabled, the CHECKSUM_GEN_* and CHECKSUM_CHECK_* defines must be enabled!
 */
#define LWIP_CHECKSUM_CTRL_PER_NETIF    0

#else

#define CHECKSUM_BY_HARDWARE            0
/* CHECKSUM_GEN_IP==true: Generate checksums in software for outgoing IP packets.*/
#define CHECKSUM_GEN_IP                 1
/* CHECKSUM_GEN_UDP==true: Generate checksums in software for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP                1
/* CHECKSUM_GEN_TCP==true: Generate checksums in software for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP                1
/* CHECKSUM_GEN_ICMP==true: Generate checksums in software for incoming TCP packets.*/
#define CHECKSUM_GEN_ICMP               1

/* CHECKSUM_CHECK_IP==true: Check checksums in software for incoming IP packets.*/
#define CHECKSUM_CHECK_IP               1
/* CHECKSUM_CHECK_UDP==true: Check checksums in software for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP              1
#ifdef HOST_TEST
/* For tests , checksum validation is disabled */
#define CHECKSUM_CHECK_TCP              0
#else
/* CHECKSUM_CHECK_TCP==true: Check checksums in software for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP              1
#endif
/**
 * LWIP_CHECKSUM_CTRL_PER_NETIF==1: Checksum generation/check can be enabled/disabled
 * per netif.
 * ATTENTION: if enabled, the CHECKSUM_GEN_* and CHECKSUM_CHECK_* defines must be enabled!
 */
#define LWIP_CHECKSUM_CTRL_PER_NETIF    1

#endif

#endif /* LWIP_HDR_LWIPOPTS_H */
