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

#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/err.h"
#include "lwip/debug.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tcp.h"
#include "lwip/igmp.h"
#include "lwip/tcpbase.h"
#include "lwip/init.h"

#include "tcpip_wrapper.h"
#include "tcpip.h"
#include "vcos_log.h"
#include "tcpip_vcos_log.h"

#include <string.h>
#define AUTOSAR_TCP_TX_MBOX_SIZE 10
#define AUTOSAR_TCP_RX_MBOX_SIZE 10

/* connection state enum */
enum { AUTOSAR_TCP_DISCONNECTED, AUTOSAR_TCP_CONNECTING, AUTOSAR_TCP_CONNECTED, AUTOSAR_TCP_LISTENING };
extern tcpip_wrapper_tcp_ctx_t tcp_ctx_array[AUTOSAR_TCP_CONN_MAX];
extern struct tcpip_wrapper_udp_ctx udp_ctx_array[AUTOSAR_UDP_CONN_MAX];
extern struct udp_pcb udp_pcb_array[AUTOSAR_UDP_CONN_MAX];

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_close(tcpip_wrapper_tcp_ctx_t *tcp_ctx, boolean abort);
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_ctx_free_internal(struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean abort);
static FUNC_P2VAR(struct tcpip_wrapper_tcp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_tcp_ctx_new_internal(struct altcp_pcb *pcb);
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_shutdown(struct tcpip_wrapper_tcp_ctx *tcp_ctx);
#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"

typedef struct {
	uint8_t *data;
	uint16 data_len;
} tcpip_wrapper_tcp_msg_t;

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
FUNC_P2VAR(struct tcpip_wrapper_tcp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_tcp_ctx_new(void)
{
	struct tcpip_wrapper_tcp_ctx *ctx = NULL;

	LOCK_TCPIP_CORE();
	ctx = tcpip_wrapper_tcp_ctx_new_internal(NULL);
	UNLOCK_TCPIP_CORE();

	return ctx;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_ctx_free(struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean abort)
{
	err_t err;

	LOCK_TCPIP_CORE();
	err = tcpip_wrapper_tcp_ctx_free_internal(tcp_ctx, abort);
	UNLOCK_TCPIP_CORE();
	return err;
}

FUNC_P2VAR(struct tcpip_wrapper_tcp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_get_tcp_ctx_by_id(uint16_t ctx_id)
{
	uint16_t tcp_ctx_id = ctx_id - AUTOSAR_TCP_ID_BASE;

	if (tcp_ctx_id < AUTOSAR_TCP_CONN_MAX) {
		if ((AUTOSAR_CTX_OCCUPIED == tcp_ctx_array[tcp_ctx_id].occupied) &&
		    (ctx_id == tcp_ctx_array[tcp_ctx_id].id)) {
			return &tcp_ctx_array[tcp_ctx_id];
		}
	}

	return NULL;
}

FUNC(uint16_t, TCPIP_CODE) tcpip_wrapper_tcp_get_id_by_ctx(const struct tcpip_wrapper_tcp_ctx *tcp_ctx)
{
	return tcp_ctx->id;
}

static FUNC_P2VAR(struct tcpip_wrapper_tcp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_tcp_ctx_new_internal(struct altcp_pcb *pcb)
{
	tcpip_wrapper_tcp_ctx_t *tcp_ctx = NULL;
	uint32 i;

	for (i = 0; i < AUTOSAR_TCP_CONN_MAX; i++) {
		if (AUTOSAR_CTX_UNOCCUPIED == tcp_ctx_array[i].occupied) {
			tcp_ctx = &tcp_ctx_array[i];
			tcp_ctx->occupied = AUTOSAR_CTX_OCCUPIED;
			tcp_ctx->id = (uint8_t)(i + AUTOSAR_TCP_ID_BASE);
			break;
		}
	}

	if (tcp_ctx == NULL) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "failed to alloc tcp ctx");
		goto err;
	}

	if (pcb != NULL) {
		tcp_ctx->conn = pcb;
	} else {
		tcp_ctx->conn = altcp_tcp_new_ip_type((u8_t)IPADDR_TYPE_ANY);
		if (NULL == tcp_ctx->conn) {
			VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "altcp_tcp_new_ip_type failed");
			goto err;
		}
	}

	/* Set arg pointer for callbacks */
	altcp_arg(tcp_ctx->conn, tcp_ctx);
	return tcp_ctx;

err:
	if (tcp_ctx != NULL) {
		(void)tcpip_wrapper_tcp_ctx_free_internal(tcp_ctx, TRUE);
	}

	return NULL;
}

static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_ctx_free_internal(struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean abort)
{
	err_t err;

	ASSERT(tcp_ctx != NULL);

	if (abort || (tcp_ctx->server != NULL)) {
		err = tcpip_wrapper_tcp_close(tcp_ctx, abort);
		if (err == (err_t)ERR_OK) {
			(void)memset(tcp_ctx, 0, sizeof(tcpip_wrapper_tcp_ctx_t));
		}
	} else {
		err = tcpip_wrapper_tcp_shutdown(tcp_ctx);
	}

	return err;
}

static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_output_send(const tcpip_wrapper_tcp_ctx_t *tcp_ctx, struct altcp_pcb *tpcb)
{
	err_t err = (err_t)ERR_OK;

	ASSERT(tcp_ctx != NULL);
	ASSERT(tpcb != NULL);
	ASSERT(tcp_ctx->occupied == AUTOSAR_CTX_OCCUPIED);

	/* Flush */
	if (tcp_ctx->unsent != NULL) {
		err = altcp_output(tpcb);
		if (err != (err_t)ERR_OK) {
			VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "altcp_output failed %d (%s)\n", err,
				       lwip_strerr(err));
		}
	}
	return err;
}

static FUNC(void, TCPIP_CODE) tcpip_wrapper_tcp_clear_tx_requests(const tcpip_wrapper_tcp_ctx_t *tcp_ctx)
{
	altcp_tcp_seg_t *unsent;
	altcp_tcp_seg_t *p_next;

	ASSERT(tcp_ctx != NULL);

	unsent = tcp_ctx->unsent;
	while (unsent != NULL) {
		if (unsent->tcp_tx_data != NULL) {
			mem_free((void *)unsent->tcp_tx_data);
			VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG,
				       "tcpip_wrapper_tcp_clear_tx_requests: free unsent data %p, len %d\n",
				       unsent->tcp_tx_data, unsent->tcp_tx_len);
		}
		p_next = unsent->next;
		(void)memp_free(MEMP_ALTCP_TCP_SEG, unsent);
		unsent = p_next;
	}
}

static FUNC(void, TCPIP_CODE) tcpip_wrapper_tcp_clear_rx_data(const tcpip_wrapper_tcp_ctx_t *tcp_ctx)
{
	ASSERT(tcp_ctx != NULL);
}

static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_shutdown(struct tcpip_wrapper_tcp_ctx *tcp_ctx)
{
	err_t res = (err_t)ERR_OK;
	ASSERT(tcp_ctx != NULL);

	/* Bring down TCP connection if not already done */
	if (tcp_ctx->conn != NULL) {
		res = altcp_close(tcp_ctx->conn);
		if (res != (err_t)ERR_OK) {
			VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_close: Close err=%s\n",
				       lwip_strerr(res));
		}
	}

	/* Notify upper layer of disconnection if changed state */
	if (tcp_ctx->conn_state != (uint8_t)AUTOSAR_TCP_DISCONNECTED) {
		tcp_ctx->conn_state = (uint8_t)AUTOSAR_TCP_DISCONNECTED;
	}

	return res;
}

/**
 * Close connection to server
 * @param client tcp context
 * @param reason Reason for disconnection
 */
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_close(tcpip_wrapper_tcp_ctx_t *tcp_ctx, boolean abort)
{
	err_t res = (err_t)ERR_OK;
	ASSERT(tcp_ctx != NULL);

	/* Bring down TCP connection if not already done */
	if (tcp_ctx->conn != NULL) {
		altcp_arg(tcp_ctx->conn, NULL);
		altcp_recv(tcp_ctx->conn, NULL);
		altcp_err(tcp_ctx->conn, NULL);
		altcp_sent(tcp_ctx->conn, NULL);
		altcp_poll(tcp_ctx->conn, NULL, 0);
		if (!abort) {
			res = altcp_close(tcp_ctx->conn);
			if (res != (err_t)ERR_OK) {
				VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_close: Close err=%s\n",
					       lwip_strerr(res));
			} else {
				/* Do nothing */
			}
		} else {
			altcp_abort(tcp_ctx->conn);
		}

		tcp_ctx->conn = NULL;
	} else if (tcp_ctx->server != NULL) {
		res = altcp_close(tcp_ctx->server);
		ASSERT(res == (err_t)ERR_OK);
	} else {
		/* Do nothing */
	}

	/* Remove all pending tx requests */
	tcpip_wrapper_tcp_clear_tx_requests(tcp_ctx);
	/* Remove all pending rx data */
	tcpip_wrapper_tcp_clear_rx_data(tcp_ctx);

	/* Notify upper layer of disconnection if changed state */
	if (tcp_ctx->conn_state != (uint8_t)AUTOSAR_TCP_DISCONNECTED) {
		tcp_ctx->conn_state = AUTOSAR_TCP_DISCONNECTED;
	}

	return res;
}

/**
 * TCP received callback function. @see tcp_recv_fn
 * @param arg TCP context
 * @param p PBUF chain of received data
 * @param err Passed as return value if not ERR_OK
 * @return ERR_OK or err passed into callback
 */
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_recv_cb(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err)
{
	tcpip_wrapper_tcp_ctx_t *tcp_ctx = (tcpip_wrapper_tcp_ctx_t *)arg;
	ip_addr_t *ip_addr;
	u16_t remote_port;
	err_t err_num;
	ASSERT(tcp_ctx != NULL);
	ASSERT(tcp_ctx->conn == pcb);
	ASSERT(err == (err_t)ERR_OK);

	if (p == NULL) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG,
			       "tcpip_wrapper_tcp_recv_cb: Recv pbuf=NULL, remote has closed connection\n");
		/* Notify upper layer conn closed */
		tcp_ctx->err(tcp_ctx->id, ERR_CONN);
	} else {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "recv from callback id %d len %u this buf len %u %p\n",
			       tcp_ctx->id, p->tot_len, p->len, (char *)p->payload);
		if (p->tot_len > 0u) {
			ip_addr = altcp_get_ip(pcb, FALSE);
			remote_port = lwip_htons(altcp_get_port(pcb, FALSE));
			err_num = tcp_ctx->recv(tcp_ctx->id, p, *((uint32_t *)ip_addr), remote_port);
			if (err_num != (err_t)ERR_OK) {
				VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_tcp_recv_cb: Recv err=%d\n",
					       err_num);
				return err_num;
			}
		}

		/* do something here,if diectly callback rx mbox may not needed */
		/* call rx indication ? */
		(void)pbuf_free(p);
	}
	return (err_t)ERR_OK;
}

/**
 * TCP data sent callback function. @see tcp_sent_fn
 * @param arg TCP context
 * @param tpcb TCP connection handle
 * @param len Number of bytes sent
 * @return ERR_OK
 */
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_sent_cb(void *arg, struct altcp_pcb *tpcb, u16_t len)
{
	(void)tpcb;
	tcpip_wrapper_tcp_ctx_t *tcp_ctx = (tcpip_wrapper_tcp_ctx_t *)arg;
	altcp_tcp_seg_t *unsent = NULL;
	altcp_tcp_seg_t *next = NULL;

	tcp_ctx->sent_len += len;
	unsent = tcp_ctx->unsent;
	while (unsent != NULL) {
		if (tcp_ctx->sent_len >= unsent->tcp_tx_len) {
			tcp_ctx->sent_len -= unsent->tcp_tx_len;
			if (unsent->tcp_tx_data != NULL) {
				mem_free((void *)unsent->tcp_tx_data);
				VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG,
					       "tcpip_wrapper_tcp_sent_cb: free sent data %p, len %d\n", unsent->tcp_tx_data,
					       unsent->tcp_tx_len);
			}
			next = unsent->next;
			memp_free(MEMP_ALTCP_TCP_SEG, unsent);
			unsent = next;
		} else {
			break;
		}
	}
	tcp_ctx->unsent = unsent;
	tcp_ctx->sent(tcp_ctx->id, len);

	return (err_t)ERR_OK;
}

/**
 * TCP error callback function. @see tcp_err_fn
 * @param arg TCP context
 * @param err Error encountered
 */
static FUNC(void, TCPIP_CODE) tcpip_wrapper_tcp_err_cb(void *arg, err_t err)
{
	tcpip_wrapper_tcp_ctx_t *tcp_ctx = (tcpip_wrapper_tcp_ctx_t *)arg;
	VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_err_cb: TCP error callback: error %d, arg: %p\n",
		       err, arg);
	ASSERT(tcp_ctx != NULL);
	/* Set conn to null before calling close as pcb is already deallocated*/
	tcp_ctx->conn = NULL;
	tcp_ctx->server = NULL;
	tcp_ctx->err(tcp_ctx->id, err);
}

/**
 * TCP poll callback function. @see tcp_poll_fn
 * @param arg TCP context
 * @param tpcb TCP connection handle
 * @return err ERR_OK
 */
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_poll_cb(void *arg, struct altcp_pcb *tpcb)
{
	const tcpip_wrapper_tcp_ctx_t *tcp_ctx = (tcpip_wrapper_tcp_ctx_t *)arg;

	(void)tpcb;

	if (tcp_ctx->conn_state == (uint8_t)AUTOSAR_TCP_CONNECTED) {
		/* send remaining data */
		(void)tcpip_wrapper_tcp_output_send(tcp_ctx, tcp_ctx->conn);
	}
	return (err_t)ERR_OK;
}

/**
 * TCP connect callback function. @see tcp_connected_fn
 * @param arg autosar tcp context
 * @param err Always ERR_OK, tcp_err_cb is called in case of error
 * @return ERR_OK
 */
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_connect_cb(void *arg, struct altcp_pcb *tpcb, err_t err)
{
	tcpip_wrapper_tcp_ctx_t *tcp_ctx = (tcpip_wrapper_tcp_ctx_t *)arg;

	if (err != (err_t)ERR_OK) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_connect_cb: TCP connect error %d\n", err);
		return err;
	}

	/* Setup TCP callbacks */
	altcp_recv(tpcb, tcpip_wrapper_tcp_recv_cb);
	altcp_sent(tpcb, tcpip_wrapper_tcp_sent_cb);
	altcp_poll(tpcb, tcpip_wrapper_tcp_poll_cb, 2);
	tcp_ctx->conn_state = AUTOSAR_TCP_CONNECTED;
	VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_WARN, TCPIP_DLT_CHECK_ID7, (unsigned int)ip4_addr_get_u32(&(tpcb->local_ip)),
		       tpcb->local_port, (unsigned int)ip4_addr_get_u32(&(tpcb->remote_ip)), tpcb->remote_port);
	/* Notify upper layer */
	tcp_ctx->connected(tcp_ctx->id);

	/* Start transmission from tx mbox */
	(void)tcpip_wrapper_tcp_output_send(tcp_ctx, tcp_ctx->conn);

	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_connect(struct tcpip_wrapper_tcp_ctx *tcp_ctx,
	const ip_addr_t *remote_addr, u16_t remote_port)
{
	err_t ret;
	u16_t port;
	ASSERT(tcp_ctx != NULL);
	ASSERT(remote_addr != NULL);

	if (NULL == tcp_ctx->conn) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_connect: unalloc tcp conn\n");
		return (err_t)ERR_CONN;
	}

	if (tcp_ctx->conn_state != (uint8_t)AUTOSAR_TCP_DISCONNECTED) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_connect: Already connected\n");
		return (err_t)ERR_ISCONN;
	}
	VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG,
		       "tcpip_wrapper_tcp_connect: Connecting to host: %s at port:%" U16_F "\n", ipaddr_ntoa(remote_addr),
		       remote_port);

	/* Connect to server */
	port = lwip_htons(remote_port);
	ret = altcp_connect(tcp_ctx->conn, remote_addr, port, tcpip_wrapper_tcp_connect_cb);
	if (ret != (err_t)ERR_OK) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG,
			       "tcpip_wrapper_tcp_connect: Error connecting to remote ip/port, %d\n", ret);
		return (err_t)ERR_CONN;
	}

	tcp_ctx->conn_state = AUTOSAR_TCP_CONNECTING;

	/* Set error callback */
	altcp_err(tcp_ctx->conn, tcpip_wrapper_tcp_err_cb);
	return (err_t)ERR_OK;
}

static FUNC_P2VAR(struct altcp_tcp_seg, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_tcp_create_segment(void)
{
	struct altcp_tcp_seg *seg;
	seg = (struct altcp_tcp_seg *)memp_malloc(MEMP_ALTCP_TCP_SEG);
	if (seg == NULL) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "autosar tcp_create_segment: no memory.\n");
		return NULL;
	}
	seg->next = NULL;
	return seg;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_send(struct tcpip_wrapper_tcp_ctx *tcp_ctx,
	const uint8_t *payload, uint16 payload_length, uint8_t apiflags)
{
	err_t ret;
	struct altcp_tcp_seg *seg;
	struct altcp_tcp_seg *last_seg;

	ASSERT(tcp_ctx);
	ASSERT(payload);
	LOCK_TCPIP_CORE();
	if ((tcp_ctx->conn == NULL) || (tcp_ctx->conn_state != (uint8_t)AUTOSAR_TCP_CONNECTED)) {
		UNLOCK_TCPIP_CORE();

		return (err_t)ERR_CONN;
	}

	seg = tcpip_wrapper_tcp_create_segment();
	if (seg == NULL) {
		UNLOCK_TCPIP_CORE();
		return (err_t)ERR_MEM;
	}

	ret = altcp_write(tcp_ctx->conn, payload, payload_length, apiflags);
	if (ret == (err_t)ERR_OK) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_send:%p,%d,%d", payload, payload_length,
			       apiflags);
		if ((apiflags & (uint8_t)TCP_WRITE_FLAG_COPY) == 0u) {
			seg->tcp_tx_data = payload;
		} else {
			seg->tcp_tx_data = NULL;
		}

		seg->tcp_tx_len = payload_length;
		if (tcp_ctx->unsent == NULL) {
			tcp_ctx->unsent = seg;
		} else {
			for (last_seg = tcp_ctx->unsent; last_seg->next != NULL; last_seg = last_seg->next) {
			};

			last_seg->next = seg;
		}
#if !AUTOSAR_TCP_SEND_DONTWAIT
		(void)altcp_output(tcp_ctx->conn);
#endif
	} else {
		memp_free(MEMP_ALTCP_TCP_SEG, seg);
	}
	UNLOCK_TCPIP_CORE();

	return ret;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_bind(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, const ip_addr_t *ip_addr, uint16_t port)
{
	err_t ret;

	ASSERT(tcp_ctx);

	if (NULL == tcp_ctx->conn) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_bind invalid conn");
		return (err_t)ERR_CONN;
	}

	LOCK_TCPIP_CORE();

	ret = altcp_bind(tcp_ctx->conn, ip_addr, port);

	UNLOCK_TCPIP_CORE();

	if (ret != (err_t)ERR_OK) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcp_bind failed");
		return ret;
	}

	return (err_t)ERR_OK;
}

/**
 * A new incoming connection has been accepted.
 */
static FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_accept(void *arg, struct altcp_pcb *tpcb, err_t err)
{
	tcpip_wrapper_tcp_ctx_t *new_ctx;

	const tcpip_wrapper_tcp_ctx_t *listen_ctx = (tcpip_wrapper_tcp_ctx_t *)arg;
	ip_addr_t *ip_addr;

	VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_accept %p / %p\n", (void *)tpcb, arg);

	if ((err != (err_t)ERR_OK) || (NULL == tpcb)) {
		return (err_t)ERR_VAL;
	}

	new_ctx = tcpip_wrapper_tcp_ctx_new_internal(tpcb);
	if (NULL == new_ctx) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_accept: Out of memory, RST\n");
		return (err_t)ERR_MEM;
	}

	/* Set up the upper layer callback functions */
	new_ctx->recv = listen_ctx->recv;
	new_ctx->accept = listen_ctx->accept;
	new_ctx->sent = listen_ctx->sent;
	new_ctx->err = listen_ctx->err;

	/* Set up the various callback functions */
	altcp_recv(tpcb, tcpip_wrapper_tcp_recv_cb);
	altcp_sent(tpcb, tcpip_wrapper_tcp_sent_cb);
	altcp_poll(tpcb, tcpip_wrapper_tcp_poll_cb, 2);
	altcp_err(tpcb, tcpip_wrapper_tcp_err_cb);

	new_ctx->conn_state = AUTOSAR_TCP_CONNECTED;
	ip_addr = altcp_get_ip(tpcb, FALSE);
	VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_WARN, TCPIP_DLT_CHECK_ID8, (unsigned int)ip4_addr_get_u32(&(tpcb->local_ip)),
		       tpcb->local_port, (unsigned int)ip4_addr_get_u32(&(tpcb->remote_ip)), tpcb->remote_port);
	(void)new_ctx->accept(listen_ctx->id, new_ctx->id, &ip_addr->addr, altcp_get_port(tpcb, FALSE));

	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_listen(struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint8 max_channels)
{
	struct altcp_pcb *pcb;

	ASSERT(tcp_ctx != NULL);

	if (NULL == tcp_ctx->conn) {
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_listen invalid conn");
		return (err_t)ERR_CONN;
	}

	pcb = altcp_listen_with_backlog(tcp_ctx->conn, max_channels);
	if (NULL == pcb) {
		(void)altcp_close(tcp_ctx->conn);
		tcp_ctx->conn = NULL;
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, "tcpip_wrapper_tcp_listen listen failed");
		return (err_t)ERR_CONN;
	} else {
		tcp_ctx->server = pcb;
		tcp_ctx->conn = NULL;
	}
	altcp_accept(pcb, tcpip_wrapper_tcp_accept);
	tcp_ctx->conn_state = AUTOSAR_TCP_LISTENING;

	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_recv(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_recv_fn recv)
{
	ASSERT(tcp_ctx != NULL);
	tcp_ctx->recv = recv;
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_accept(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_accept_fn accept)
{
	ASSERT(tcp_ctx != NULL);
	tcp_ctx->accept = accept;
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_err(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_err_fn err)
{
	ASSERT(tcp_ctx != NULL);
	tcp_ctx->err = err;
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_sent(struct tcpip_wrapper_tcp_ctx *tcp_ctx, tcpip_wrapper_tcp_sent_fn sent)
{
	ASSERT(tcp_ctx != NULL);
	tcp_ctx->sent = sent;
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_tcp_connected(struct tcpip_wrapper_tcp_ctx *tcp_ctx,
	tcpip_wrapper_tcp_connected_fn connected)
{
	ASSERT(tcp_ctx != NULL);
	tcp_ctx->connected = connected;
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_output(const struct tcpip_wrapper_tcp_ctx *tcp_ctx)
{
	LOCK_TCPIP_CORE();
	if (tcp_ctx->conn_state == (uint8_t)AUTOSAR_TCP_CONNECTED) {
		/* send remaining data */
		(void)tcpip_wrapper_tcp_output_send(tcp_ctx, tcp_ctx->conn);
	}
	UNLOCK_TCPIP_CORE();

	return (err_t)ERR_OK;
}

FUNC(void, TCPIP_CODE) tcpip_wrapper_tcp_recved(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 len)
{
	LOCK_TCPIP_CORE();
	if (tcp_ctx->conn_state == (uint8_t)AUTOSAR_TCP_CONNECTED) {
		altcp_recved(tcp_ctx->conn, len);
	}
	UNLOCK_TCPIP_CORE();
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_rxwnd(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 len)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_set_rxwnd(tcp_ctx->conn, ((u32_t)len));
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG,
			       "tcpip_wrapper_tcp_set_rxwnd(%d, IPPROTO_TCP, TCP_RCVWND_MAX) -> %" U32_F "\n", tcp_ctx->id,
			       tcp_ctx->conn->rcv_wnd_max);
	}
	UNLOCK_TCPIP_CORE();

	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_sndbuf(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 len)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_set_sndbuf(tcp_ctx->conn, ((u32_t)len));
		VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG,
			       "tcpip_wrapper_tcp_set_sndbuf(%d, IPPROTO_TCP, TCP_SNDBUF_MAX) -> %" U32_F "\n", tcp_ctx->id,
			       tcp_ctx->conn->snd_buf);
	}
	UNLOCK_TCPIP_CORE();

	return (err_t)ERR_OK;
}

FUNC(uint16, TCPIP_CODE) tcpip_wrapper_tcp_get_sndbuf(const struct tcpip_wrapper_tcp_ctx *tcp_ctx)
{
	uint16 avail_send_len = 0;

	LOCK_TCPIP_CORE();
	if (tcp_ctx->conn != NULL) {
		avail_send_len = (uint16)altcp_sndbuf(tcp_ctx->conn);
	}
	UNLOCK_TCPIP_CORE();
	return avail_send_len;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_iptos(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, const uint8 tos)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_set_iptos(tcp_ctx->conn, tos);
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(uint8, TCPIP_CODE) tcpip_wrapper_tcp_get_state(const struct tcpip_wrapper_tcp_ctx *tcp_ctx)
{
	uint8 state = (uint8)CLOSED;

	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		state = (uint8)altcp_dbg_get_tcp_state(tcp_ctx->conn);
	} else if (NULL != tcp_ctx->server) {
		state = (uint8)altcp_dbg_get_tcp_state(tcp_ctx->server);
	} else {
		/* Do nothing */
	}
	UNLOCK_TCPIP_CORE();
	return state;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_nagle(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean enable)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		if (FALSE != enable) {
			altcp_nagle_enable(tcp_ctx->conn);
		} else {
			altcp_nagle_disable(tcp_ctx->conn);
		}
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_ipttl(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, const uint8 ttl)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_set_ipttl(tcp_ctx->conn, ttl);
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_enable(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, boolean enable)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_keepalive_enable(tcp_ctx->conn, enable);
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_time(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 time)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_set_keepalive_time(tcp_ctx->conn, time);
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(void, TCPIP_CODE) tcpip_wrapper_stack_init(void) {
    tcpip_prepare();
#if !defined CONFIG_MODULE_TCPIP && defined CONFIG_MODULE_ETHIF && defined(ETH_IP_ADDR)
    lwip_add_default_netif();
#endif
}

FUNC(void, TCPIP_CODE) os_task_default_tcpip_task(void *arg){
    lwip_tcpip_start();
}

#if LWIP_TCP_KEEPALIVE
FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_probes_max(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 times)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_set_keepalive_probes_max(tcp_ctx->conn, times);
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_tcp_set_keepalive_interval(const struct tcpip_wrapper_tcp_ctx *tcp_ctx, uint16 interval)
{
	LOCK_TCPIP_CORE();
	if (NULL != tcp_ctx->conn) {
		altcp_set_keepalive_interval(tcp_ctx->conn, interval);
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(uint8_t, TCPIP_CODE) tcpip_wrapper_tcpip_ctx_is_tcp(uint16_t ctx_id)
{
	if (ctx_id < (AUTOSAR_TCP_CONN_MAX + AUTOSAR_TCP_ID_BASE)) {
		return 1;
	}

	return 0;
}

FUNC(uint8_t, TCPIP_CODE) tcpip_wrapper_tcpip_ctx_is_udp(uint16_t ctx_id)
{
	if ((ctx_id >= AUTOSAR_UDP_ID_BASE) && (ctx_id < (AUTOSAR_UDP_CONN_MAX + AUTOSAR_UDP_ID_BASE))) {
		return 1;
	}

	return 0;
}

static FUNC(void, TCPIP_CODE) tcpip_wrapper_udp_recv(void *arg, struct udp_pcb *upcb,
	struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	(void)upcb;
	u16_t remote_port;
	const struct tcpip_wrapper_udp_ctx *udp_ctx = (struct tcpip_wrapper_udp_ctx *)arg;

	if (p != NULL) {
		remote_port = lwip_htons(port);
		(void)udp_ctx->recv(udp_ctx->id, p, &addr->addr, remote_port);
		(void)pbuf_free(p);
	}
}

FUNC_P2VAR(struct tcpip_wrapper_udp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_udp_ctx_new(void)
{
	struct tcpip_wrapper_udp_ctx *udp_ctx = NULL;
	struct udp_pcb *pcb = NULL;
	uint32 i;

	LOCK_TCPIP_CORE();

	for (i = 0; i < AUTOSAR_UDP_CONN_MAX; i++) {
		if (AUTOSAR_CTX_UNOCCUPIED == udp_ctx_array[i].occupied) {
			udp_ctx = &udp_ctx_array[i];
			udp_ctx->occupied = AUTOSAR_CTX_OCCUPIED;
			udp_ctx->id = (uint8_t)(i + AUTOSAR_UDP_ID_BASE);
			pcb = &udp_pcb_array[i];
			memset(pcb, 0, sizeof(struct udp_pcb));
			pcb->ttl = UDP_TTL;
#if LWIP_MULTICAST_TX_OPTIONS
			udp_set_multicast_ttl(pcb, UDP_TTL);
#endif /* LWIP_MULTICAST_TX_OPTIONS */

			udp_ctx->conn = pcb;
			break;
		}
	}

	if (NULL == udp_ctx) {
		LWIP_DEBUGF(UDP_AUTOSAR_DEBUG, ("failed to alloc udp ctx"));
	}

	UNLOCK_TCPIP_CORE();

	return udp_ctx;
}

FUNC(void, TCPIP_CODE) tcpip_wrapper_udp_ctx_free(struct tcpip_wrapper_udp_ctx *udp_ctx)
{
	LWIP_ASSERT("Invalid udp ctx", udp_ctx != NULL);

	LOCK_TCPIP_CORE();

	if (NULL != udp_ctx->conn) {
		udp_remove(udp_ctx->conn);
	}
	(void)memset(udp_ctx, 0, sizeof(struct tcpip_wrapper_udp_ctx));

	UNLOCK_TCPIP_CORE();
}

FUNC_P2VAR(struct tcpip_wrapper_udp_ctx, AUTOMATIC, TCPIP_CODE) tcpip_wrapper_get_udp_ctx_by_id(uint16_t ctx_id)
{
	uint16_t udp_ctx_id = ctx_id - AUTOSAR_UDP_ID_BASE;

	if (udp_ctx_id < AUTOSAR_UDP_CONN_MAX) {
		if ((AUTOSAR_CTX_OCCUPIED == udp_ctx_array[udp_ctx_id].occupied) &&
		    (ctx_id == udp_ctx_array[udp_ctx_id].id)) {
			return &udp_ctx_array[udp_ctx_id];
		}
	}

	return NULL;
}

FUNC(uint16_t, TCPIP_CODE) tcpip_wrapper_udp_get_id_by_ctx(const struct tcpip_wrapper_udp_ctx *udp_ctx)
{
	return udp_ctx->id;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_bind(struct tcpip_wrapper_udp_ctx *udp_ctx, const ip_addr_t *ip_addr, uint16_t port)
{
	err_t ret;

	LWIP_ASSERT("tcpip_wrapper_udp_bind: udp_ctx != NULL", udp_ctx);

	if (NULL == udp_ctx->conn) {
		LWIP_DEBUGF(UDP_AUTOSAR_DEBUG, ("tcpip_wrapper_udp_bind invalid conn"));
		return (err_t)ERR_CONN;
	}

	LOCK_TCPIP_CORE();

	ret = udp_bind(udp_ctx->conn, ip_addr, port);
	if (ret != (err_t)ERR_OK) {
		LWIP_DEBUGF(UDP_AUTOSAR_DEBUG, ("tcp_bind failed"));
		UNLOCK_TCPIP_CORE();
		return ret;
	}

	/* set recv callback after bind success */
	udp_recv(udp_ctx->conn, tcpip_wrapper_udp_recv, (void *)udp_ctx);

	UNLOCK_TCPIP_CORE();

	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_send(const struct tcpip_wrapper_udp_ctx *udp_ctx,
	const ip_addr_t *remote_ip_addr, uint16_t remote_port,
	const uint8_t *payload, uint16 payload_length)
{
	struct pbuf *p;
	err_t ret;
	uint16_t port;

	LWIP_ASSERT("tcpip_wrapper_udp_send: udp_ctx != NULL", udp_ctx);

	if (NULL == udp_ctx->conn) {
		LWIP_DEBUGF(UDP_AUTOSAR_DEBUG, ("tcpip_wrapper_udp_send invalid conn"));
		return (err_t)ERR_CONN;
	}

	p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_REF);
	if (NULL == p) {
		LWIP_DEBUGF(UDP_AUTOSAR_DEBUG, ("tcpip_wrapper_udp_send pbuf_alloc failed"));
		return (err_t)ERR_MEM;
	}
	p->socket_idx = udp_ctx->socket_id;
	p->payload = (void *)payload;
	p->len = payload_length;
	p->tot_len = p->len;
	port = lwip_htons(remote_port);

	LOCK_TCPIP_CORE();
	ret = udp_sendto(udp_ctx->conn, p, remote_ip_addr, port);
	UNLOCK_TCPIP_CORE();
	LWIP_DEBUGF(UDP_AUTOSAR_DEBUG, ("tcpip_wrapper_udp_send udp_sendto ret %d\n", ret));

	(void)pbuf_free(p);

	return ret;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_regist_udp_recv(struct tcpip_wrapper_udp_ctx *udp_ctx, tcpip_wrapper_udp_recv_fn recv)
{
	if (NULL != udp_ctx) {
		udp_ctx->recv = recv;
	}
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_set_iptos(const struct tcpip_wrapper_udp_ctx *udp_ctx, const uint8 tos)
{
	LOCK_TCPIP_CORE();
	if (NULL != udp_ctx->conn) {
		udp_ctx->conn->tos = tos + 1u;
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_set_ipttl(const struct tcpip_wrapper_udp_ctx *udp_ctx, const uint8 ttl)
{
	LOCK_TCPIP_CORE();
	if (NULL != udp_ctx->conn) {
		udp_ctx->conn->ttl = ttl;
#if LWIP_MULTICAST_TX_OPTIONS
		udp_ctx->conn->mcast_ttl = ttl;
#endif
	}
	UNLOCK_TCPIP_CORE();
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_set_chksum(struct netif *netif, boolean enable)
{
#if LWIP_CHECKSUM_CTRL_PER_NETIF
	LOCK_TCPIP_CORE();
	if (NULL != netif) {
		if (FALSE != enable) {
			netif->chksum_flags = (uint16_t)(netif->chksum_flags | NETIF_CHECKSUM_GEN_UDP);
		} else {
			netif->chksum_flags =
				(uint16_t)((netif)->chksum_flags & (uint16_t)(~(NETIF_CHECKSUM_GEN_UDP) & 0xffff));
		}
	}
	UNLOCK_TCPIP_CORE();
#endif
	return (err_t)ERR_OK;
}

FUNC(err_t, TCPIP_CODE) tcpip_wrapper_udp_join_multicast_group(const ip_addr_t *ifaddr, ip_addr_t *groupaddr)
{
	err_t ret;

	LOCK_TCPIP_CORE();
	ret = igmp_joingroup(ifaddr, groupaddr);
	UNLOCK_TCPIP_CORE();
	return ret;
}
#endif
#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"
