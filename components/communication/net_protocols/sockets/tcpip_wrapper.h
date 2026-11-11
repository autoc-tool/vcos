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

#ifndef AUTOSAR_TCPIP_H_
#define AUTOSAR_TCPIP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/etharp.h"
#include "lwip/debug.h"
#include "lwip/netifapi.h"
#include "ethernetif.h"
#include "lwip/tcpbase.h"
#include "lwip/stats.h"
#include "lwip/ip4.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/altcp.h"
#include "lwip/sys.h"

#include "tcpip_cfg.h"

#define AUTOSAR_TCP_ID_BASE (0u)
#define AUTOSAR_TCP_CONN_MAX TCPIP_TCP_SOCKET_MAX

#define AUTOSAR_UDP_ID_BASE (AUTOSAR_TCP_ID_BASE + AUTOSAR_TCP_CONN_MAX)
#define AUTOSAR_UDP_CONN_MAX TCPIP_UDP_SOCKET_MAX

#define AUTOSAR_CTX_OCCUPIED (1u)
#define AUTOSAR_CTX_UNOCCUPIED (0u)

/* connection state enum */
typedef enum {
	/** Accepted */
	AUTOSAR_TCP_CONNECT_ACCEPTED = 0,
	/** Refused protocol version */
	AUTOSAR_TCP_CONNECT_REFUSED_PROTOCOL_VERSION = 1,
	/** Refused identifier */
	AUTOSAR_TCP_CONNECT_REFUSED_IDENTIFIER = 2,
	/** Refused server */
	AUTOSAR_TCP_CONNECT_REFUSED_SERVER = 3,
	/** Refused user credentials */
	AUTOSAR_TCP_CONNECT_REFUSED_USERNAME_PASS = 4,
	/** Refused not authorized */
	AUTOSAR_TCP_CONNECT_REFUSED_NOT_AUTHORIZED_ = 5,
	/** Disconnected */
	AUTOSAR_TCP_CONNECT_DISCONNECTED = 256,
	/** Timeout */
	AUTOSAR_TCP_CONNECT_TIMEOUT = 257
} tcpip_wrapper_tcp_connection_status_t;

typedef err_t (*tcpip_wrapper_tcp_recv_fn)(uint16_t id, struct pbuf *p, uint32_t remote_ip, uint16_t remote_port);
typedef err_t (*tcpip_wrapper_tcp_accept_fn)(uint16_t id, uint16_t accept_id, uint32_t *remote_ip, uint16_t remote_port);
typedef void (*tcpip_wrapper_tcp_err_fn)(uint16_t id, err_t err);
typedef void (*tcpip_wrapper_tcp_sent_fn)(uint16_t id, uint16_t len);
typedef void (*tcpip_wrapper_tcp_connected_fn)(uint16_t id);
typedef err_t (*tcpip_wrapper_udp_recv_fn)(uint16_t id, struct pbuf *p, const uint32_t *remote_ip, uint16_t remote_port);

typedef struct tcpip_wrapper_tcp_ctx {
	uint8_t occupied;
	uint8_t id;
	uint8_t conn_state;
	struct altcp_pcb *conn;
	struct altcp_pcb *server;
	altcp_tcp_seg_t *unsent;
	uint16_t sent_len;
	/* Upper layer user callback */
	tcpip_wrapper_tcp_recv_fn recv;
	tcpip_wrapper_tcp_accept_fn accept;
	tcpip_wrapper_tcp_sent_fn sent;
	tcpip_wrapper_tcp_connected_fn connected;
	tcpip_wrapper_tcp_err_fn err;
} tcpip_wrapper_tcp_ctx_t;

typedef struct tcpip_wrapper_udp_ctx {
	uint8_t occupied;
	uint8_t id;

	uint8_t conn_state;
	struct udp_pcb *conn;
	/* Upper layer user callback */
	tcpip_wrapper_udp_recv_fn recv;
	uint16 socket_id;
} tcpip_wrapper_udp_ctx_t;

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
FUNC(uint8_t, TCPIP_CODE) tcpip_wrapper_tcpip_ctx_is_tcp(uint16_t ctx_id);
FUNC_P2VAR(struct tcpip_wrapper_tcp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_tcp_ctx_new(void);
FUNC_P2VAR(struct tcpip_wrapper_tcp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_get_tcp_ctx_by_id(uint16_t ctx_id);
FUNC(uint16_t, TCPIP_CODE) tcpip_wrapper_tcp_get_id_by_ctx(const struct tcpip_wrapper_tcp_ctx *tcp_ctx);

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_ctx_free(struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean abort);
FUNC(err_t, TCPIP_CODE)
tcpip_wrapper_tcp_bind(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, const ip_addr_t *ip_addr, uint16_t port);
FUNC(err_t, TCPIP_CODE)
tcpip_wrapper_tcp_connect(struct tcpip_wrapper_tcp_ctx *tcp_ctx, const ip_addr_t *remote_addr, u16_t remote_port);
FUNC(err_t, TCPIP_CODE)
tcpip_wrapper_tcp_send(struct tcpip_wrapper_tcp_ctx *tcp_ctx, const uint8_t *payload, uint16 payload_length, uint8_t apiflags);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_listen(struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint8 max_channels);
FUNC(void, TCPIP_CODE) tcpip_wrapper_tcp_recved(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 len);

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_recv(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_recv_fn recv);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_accept(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_accept_fn accept);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_sent(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_sent_fn sent);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_err(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_err_fn err);
FUNC(err_t, TCPIP_CODE)
tcpip_wrapper_regist_tcp_connected(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_connected_fn connected);

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_output(const struct tcpip_wrapper_tcp_ctx *tcp_ctx);

FUNC(uint8_t, TCPIP_CODE) tcpip_wrapper_tcpip_ctx_is_udp(uint16_t ctx_id);
FUNC_P2VAR(struct tcpip_wrapper_udp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_udp_ctx_new(void);
FUNC(void, TCPIP_CODE) tcpip_wrapper_udp_ctx_free(struct tcpip_wrapper_udp_ctx *udp_ctx);
FUNC_P2VAR(struct tcpip_wrapper_udp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_get_udp_ctx_by_id(uint16_t ctx_id);
FUNC(uint16_t, TCPIP_CODE) tcpip_wrapper_udp_get_id_by_ctx(const struct tcpip_wrapper_udp_ctx *udp_ctx);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_bind(struct tcpip_wrapper_udp_ctx *udp_ctx, const ip_addr_t *ip_addr, uint16_t port);
FUNC(err_t, TCPIP_CODE)
tcpip_wrapper_udp_send(const struct tcpip_wrapper_udp_ctx *udp_ctx, const ip_addr_t *remote_ip_addr, uint16_t remote_port,
		 const uint8_t *payload, uint16 payload_length);

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_udp_recv(struct tcpip_wrapper_udp_ctx *udp_ctx, tcpip_wrapper_udp_recv_fn recv);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_set_iptos(const struct tcpip_wrapper_udp_ctx *udp_ctx, const uint8 tos);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_set_ipttl(const struct tcpip_wrapper_udp_ctx *udp_ctx, const uint8 ttl);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_set_chksum(struct netif *netif, boolean enable);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_join_multicast_group(const ip_addr_t *ifaddr, ip_addr_t *groupaddr);

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_rxwnd(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 len);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_sndbuf(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 len);
FUNC(uint16, TCPIP_CODE) tcpip_wrapper_tcp_get_sndbuf(const struct tcpip_wrapper_tcp_ctx *tcp_ctx);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_iptos(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, const uint8 tos);
FUNC(uint8, TCPIP_CODE) tcpip_wrapper_tcp_get_state(const struct tcpip_wrapper_tcp_ctx *tcp_ctx);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_nagle(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean enable);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_icmp_send(struct netif *netif,
	const ip4_addr_t *ipaddr, uint8 ttl, uint8 type, uint8 code, uint16 payload_length, const uint8 *payload);

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_enable(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean enable);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_time(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 time);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_probes_max(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 times);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_interval(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 interval);
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_ipttl(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, const uint8 ttl);

FUNC(void, TCPIP_CODE) tcpip_prepare(void);
FUNC(void, TCPIP_CODE) lwip_tcpip_start(void);
FUNC(void, TCPIP_CODE) tcpip_wrapper_stack_init(void);

static inline FUNC(void, TCPIP_CODE) tcpip_wrapper_rx_indication(uint8 ctrl_idx, eth_frame_type frame_type,
	boolean is_broadcast, const uint8 *phys_addr_ptr, const uint8 *data_ptr, uint16 len_byte)
{
	lwip_rxIndication(ctrl_idx, frame_type, is_broadcast, phys_addr_ptr, data_ptr, len_byte);
}

static inline FUNC_P2VAR(void, AUTOMATIC, TCPIP_CODE) tcpip_buffer_get(uint16 size)
{
	return mem_malloc(size);
}

static inline FUNC(void, TCPIP_CODE) tcpip_buffer_free(uint8 *buff_ptr)
{
	if (buff_ptr != NULL) {
		mem_free(buff_ptr);
	} else {
		return;
	}
}

#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"

#ifdef __cplusplus
}
#endif /* __cplusplus     */
#endif /* AUTOSAR_TCPIP_H_ */
