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

#include <string.h>
#include "tcpip.h"
#include "tcpip_cbk.h"
#include "tcpip_types.h"
#include "tcpip_priv_types.h"
#if (CONFIG_TEST_CMD_IPERF == 1)
#include "iperf.h"
#endif

#include "tcpip_wrapper.h"
#if (CONFIG_MODULE_ETHIF == 1)
#include "ethif.h"
#endif

#include "vcos_log.h"
#include "tcpip_vcos_log.h"
#define TCPIP_DEBUG_LOG(...) VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_DEBUG, __VA_ARGS__)
#define TCPIP_INFO_LOG(...) VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_INFO, __VA_ARGS__)
#define TCPIP_WARN_LOG(...) VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_WARN, __VA_ARGS__)
#define TCPIP_ERROR_LOG(...) VCOS_LOG(TCPIP_MODULE_ID, VCOS_LOG_ERROR, __VA_ARGS__)

#define TCPIP_DET_REPORTERROR(errid)

#define VALIDATE(_exp, errid)                                                                                          \
do {                                                                                                                   \
	if (!(_exp)) {                                                                                                 \
		TCPIP_DET_REPORTERROR(errid);                                                                          \
		return E_NOT_OK;                                                                                       \
	}                                                                                                      \
} while (0)

#define VALIDATE_RV(_exp, errid, _rv)                                                                                  \
do {                                                                                                               \
	if (!(_exp)) {                                                                                             \
		TCPIP_DET_REPORTERROR(errid);                                                                      \
		return _rv;                                                                                        \
	}                                                                                                  \
} while (0)

#define VALIDATE_NO_RV(_exp, errid)                                                                                    \
do {                                                                                                               \
	if (!(_exp)) {                                                                                             \
		TCPIP_DET_REPORTERROR(errid);                                                                      \
		return;                                                                                            \
	}                                                                                                  \
} while (0)

#define VALIDATE_NO_RV_AND_NO_DEL(_exp, _api, _err)                                                                    \
do {                                                                                                               \
	if (!(_exp)) {                                                                                             \
		return E_NOT_OK;                                                                                   \
	}                                                                                                  \
} while (0)

#define TCPIP_MAX_ONHOLD_STATE_CNT (TCPIP_MAX_ONHOLD_STATE_TIMEOUT_MS / TCPIP_MAIN_FUNCTION_PERIOD_MS)

#define INVALID_LOCALADDR_IDX TCPIP_NOF_LOCALADDR

#define TcpIp_GetSocketOwnerListOfTcpIpConfig() tcpip_config_ptr->config.socket_owner_config.socket_owner_list

#define TCPIP_NOF_ERR_BEFORE_RESET 100

extern tcpip_ctrl_admin_type tcpip_ctrl_admin[TCPIP_NOF_CONTROLLERS];

extern tcpip_socket_admin_type tcpip_socket_admin[TCPIP_MAX_NOF_SOCKETS];

extern tcpip_ip_addr_state_type tcpip_ip_addr_state[TCPIP_NOF_LOCALADDR];

extern tcpip_socket_id_type tcpip_buf_idx_to_socket_id[ETHIf_MAX_TXBUFSTOTAL];

#define ETH_START_SEC_PRIVATE_BSS
#include "eth_memmap.h"

P2VAR(tcpip_config_type, ETH_PRIVATE_BSS, TCPIP_MEMMAP_BSW) tcpip_config_ptr;

VAR(boolean, ETH_PRIVATE_BSS) tcpip_initialized;
#define ETH_STOP_SEC_PRIVATE_BSS
#include "eth_memmap.h"


#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
static FUNC(void, TCPIP_CODE) tcpip_notify_local_ip_addr_assignment_chg(uint8 addr_id, tcpip_ip_addr_state_type ip_addr_state);

static FUNC(void, TCPIP_CODE) tcpip_netif_init(const tcpip_config_type *config);

/****************************************************************************
 * Address
 ****************************************************************************/
FUNC(void, TCPIP_CODE) tcpip_ip_addr_32_to_8(uint32 src, uint8 *dest)
{
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
	dest[0] = (uint8)((src & 0xff000000U) >> 24);
	dest[1] = (uint8)((src & 0x00ff0000U) >> 16);
	;
	dest[2] = (uint8)((src & 0x0000ff00U) >> 8);
	;
	dest[3] = (uint8)(src & 0x000000ffU);
#else
	dest[3] = (uint8)((src & 0xff000000U) >> 24);
	dest[2] = (uint8)((src & 0x00ff0000U) >> 16);
	;
	dest[1] = (uint8)((src & 0x0000ff00U) >> 8);
	;
	dest[0] = (uint8)(src & 0x000000ffU);
#endif
}

FUNC(uint32_t, TCPIP_CODE) tcpip_ip_addr_8_to_32(const uint8 *src)
{
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
	return (((uint32)((src[0]) & 0xffu) << 24) | ((uint32)((src[1]) & 0xffu) << 16) |
		((uint32)((src[2]) & 0xffu) << 8) | (uint32)((src[3]) & 0xffu));
#else

	return (((uint32)(((uint32)src[3]) & 0xffu) << 24) | ((uint32)(((uint32)src[2]) & 0xffu) << 16) |
		((uint32)(((uint32)src[1]) & 0xffu) << 8) | (uint32)(((uint32)src[0]) & 0xffu));
#endif
}

static FUNC(void, TCPIP_CODE) ip4_netmask_to_ip_addr(ip4_addr_t *net_mask_addr, uint8 net_mask)
{
	uint8 addr[4];

	if (net_mask < 8u) {
		addr[0] = (uint8)(0xff00u >> net_mask);
		addr[1] = 0;
		addr[2] = 0;
		addr[3] = 0;
	} else if (net_mask < 16u) {
		addr[0] = 255;
		addr[1] = (uint8)(0xff00u >> (net_mask - 8u));
		addr[2] = 0;
		addr[3] = 0;
	} else if (net_mask < 24u) {
		addr[0] = 255;
		addr[1] = 255;
		addr[2] = (uint8)(0xff00u >> (net_mask - 16u));
		addr[3] = 0;
	} else if (net_mask < 32u) {
		addr[0] = 255;
		addr[1] = 255;
		addr[2] = 255;
		addr[3] = (uint8)(0xff00u >> (net_mask - 24u));
	} else {
		addr[0] = 255;
		addr[1] = 255;
		addr[2] = 255;
		addr[3] = 255;
	}

	net_mask_addr->addr = tcpip_ip_addr_8_to_32(addr);
}

static FUNC(void, TCPIP_CODE) tcpip_notify_local_ip_addr_assignment_chg(uint8 addr_id, tcpip_ip_addr_state_type ip_addr_state)
{
	uint8 ctrl_idx;
	/* Call upper layer */
	for (uint32 idx = 0; idx < (uint32)TCPIP_NOF_SOCKETOWNERS; idx++) {
		if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[idx].socket_owner_local_ip_addr_assignment_chg_fnc_ptr != NULL) {
			TcpIp_GetSocketOwnerListOfTcpIpConfig()[idx].socket_owner_local_ip_addr_assignment_chg_fnc_ptr(addr_id,
				ip_addr_state);
		}
	}
	if (addr_id < TCPIP_NOF_LOCALADDR) {
		ctrl_idx = tcpip_config_ptr->config.local_addr_list[addr_id].tcpip_ctrl_ref;
		for (uint32 addr_idx = 0; addr_idx < (uint32)TCPIP_NOF_LOCALADDR; addr_idx++) {
			if (tcpip_config_ptr->config.local_addr_list[addr_idx].tcpip_ctrl_ref == ctrl_idx) {
				tcpip_ip_addr_state[addr_idx] = ip_addr_state;
			}
		}
	}
}

static inline FUNC(uint8, TCPIP_CODE) get_local_addr_list_config_idx(uint8 address_id)
{
	uint8 ret = INVALID_LOCALADDR_IDX;

	for (uint8 i = 0; i < TCPIP_NOF_LOCALADDR; i++) {
		if (tcpip_config_ptr->config.local_addr_list[i].address_id == address_id) {
			ret = i;
			break;
		}
	}
	return ret;
}

static FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_local_addr_idx_by_ctrl_idx(uint8 ctrl_idx, uint8 *local_addr_idx)
{
	Std_ReturnType result = E_NOT_OK;

	for (uint8 index = 0; index < TCPIP_NOF_LOCALADDR; index++) {
		if ((tcpip_config_ptr->config.local_addr_list[index].tcpip_ctrl_ref == ctrl_idx) &&
		    (tcpip_config_ptr->config.local_addr_list[index].address_type == TCPIP_UNICAST)) {
			*local_addr_idx = index;
			result = E_OK;
			break;
		}
	}

	return result;
}

static FUNC(void, TCPIP_CODE) tcpip_add_static_arp_entry(void)
{
	const tcpip_arp_config_type *arp_config = &tcpip_config_ptr->config.ip_config->ipv4_config->arp_config_list[0];
	uint32 len = arp_config->static_arp_table_len;

	const tcpip_arp_table_entry_type *static_arp_table = arp_config->static_arp_table;

	for (uint16 i = 0; i < len; i++) {
		if (IPADDR_NONE != static_arp_table[i].ipaddr.addr) {
			(void)netifapi_arp_add((const ip4_addr_t *)&static_arp_table[i].ipaddr,
					       (struct eth_addr *)&static_arp_table[i].ethaddr, NETIFAPI_ARP_PERM);
		}
	}
}

static FUNC(boolean, TCPIP_CODE) tcpip_local_addr_id_is_online(tcpip_local_addr_id_type local_addr_id)
{
	boolean ret;
	const tcpip_local_addr_type *ptr_local_addr;
	uint8 local_addr_idx = get_local_addr_list_config_idx(local_addr_id);

	if (local_addr_idx != INVALID_LOCALADDR_IDX) {
		ptr_local_addr = &tcpip_config_ptr->config.local_addr_list[local_addr_idx];
		if (tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_ethif_ctrl_state == TCPIP_STATE_ONLINE) {
			ret = TRUE;
		} else {
			ret = FALSE;
		}
	} else {
		ret = FALSE;
	}
	return ret;
}

/****************************************************************************
 * Socket
 ****************************************************************************/
static FUNC(void, TCPIP_CODE) tcpip_close_sockets(const uint8 ctrl_idx, const uint8 addr_id)
{
	for (uint8 socket_idx = 0; socket_idx < TCPIP_MAX_NOF_SOCKETS; socket_idx++) {
		if ((tcpip_socket_admin[socket_idx].ptr_local_addr != NULL) &&
		    (tcpip_socket_admin[socket_idx].ptr_local_addr->address_id == addr_id) &&
		    (tcpip_socket_admin[socket_idx].ptr_local_addr->tcpip_ctrl_ref == ctrl_idx)) {
			(void)tcpip_close(socket_idx, TRUE);
		}
	}
}

static FUNC(void, TCPIP_CODE) tcpip_tcp_connected_cb(uint16_t id)
{
	uint8 owner_id = tcpip_socket_admin[id].socket_owner_id;

	if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcp_connected_fnc_ptr != NULL) {
		TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcp_connected_fnc_ptr(id);
	}
	return;
}

static FUNC(boolean, TCPIP_CODE) tcpip_socket_id_is_online(tcpip_socket_id_type socket_id)
{
	boolean ret;
	const tcpip_local_addr_type *ptr_local_addr;

	ptr_local_addr = tcpip_socket_admin[socket_id].ptr_local_addr;
	if (ptr_local_addr == NULL) {
		ret = TRUE;
	} else if (tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_ethif_ctrl_state == TCPIP_STATE_ONLINE) {
		ret = TRUE;
	} else {
		ret = FALSE;
	}
	return ret;
}

/****************************************************************************
 * State Manager
 ****************************************************************************/
static FUNC(Std_ReturnType, TCPIP_CODE) tcpip_state_chg_to_online(const tcpip_local_addr_type *loacl_addr)
{
	uint8 ctrl_idx = loacl_addr->tcpip_ctrl_ref;
	uint8 ethif_ctrl_idx;

	struct netif *netif;

	ethif_ctrl_idx = tcpip_ctrl_admin[ctrl_idx].tcpip_netif_ctrl.ctrl_id;
	uint8 addr_id = loacl_addr->address_id;
	ip4_addr_t ipaddr, gw, netmask;
	uint8 mac_adrs[NETIF_MAX_HWADDR_LEN] = { 0 };

	netif = &tcpip_ctrl_admin[ctrl_idx].tcpip_netif;
	ethif_get_phys_addr(ethif_ctrl_idx, mac_adrs);
	for (uint8 i = 0; i < NETIF_MAX_HWADDR_LEN; i++) {
		netif->hwaddr[i] = mac_adrs[i];
	}

	ipaddr.addr = loacl_addr->static_ip_addr_config->static_ip_address;

	gw.addr = loacl_addr->static_ip_addr_config->default_router;
	ip4_netmask_to_ip_addr(&netmask, loacl_addr->static_ip_addr_config->netmask);

	(void)netifapi_netif_set_addr(netif, &ipaddr, &netmask, &gw);

	(void)netifapi_netif_set_up(netif);
	tcpip_add_static_arp_entry();
	tcpip_ctrl_admin[ctrl_idx].tcpip_ethif_requested_ctrl_state = TCPIP_STATE_ONLINE;
	tcpip_ctrl_admin[ctrl_idx].tcpip_ethif_ctrl_state = TCPIP_STATE_ONLINE;
	tcpip_notify_local_ip_addr_assignment_chg(addr_id, TCPIP_IPADDR_STATE_ASSIGNED);

	return ERR_OK;
}

static FUNC(Std_ReturnType, TCPIP_CODE) tcpip_state_chg_to_onhold(const tcpip_local_addr_type *local_addr)
{
	uint8 ctrl_idx = local_addr->tcpip_ctrl_ref;
	uint8 addr_id = local_addr->address_id;

	tcpip_ctrl_admin[ctrl_idx].tcpip_ethif_ctrl_onhold_state_cnt = 0;
	tcpip_ctrl_admin[ctrl_idx].tcpip_ethif_requested_ctrl_state = TCPIP_STATE_ONHOLD;
	tcpip_ctrl_admin[ctrl_idx].tcpip_ethif_ctrl_state = TCPIP_STATE_ONHOLD;
	tcpip_notify_local_ip_addr_assignment_chg(addr_id, TCPIP_IPADDR_STATE_ONHOLD);

	return ERR_OK;
}

static FUNC(Std_ReturnType, TCPIP_CODE) tcpip_state_chg_to_offline(const tcpip_local_addr_type *local_addr)
{
	uint8 ctrl_idx = local_addr->tcpip_ctrl_ref;
	uint8 addr_id = local_addr->address_id;

	tcpip_close_sockets(ctrl_idx, addr_id);

	(void)netifapi_netif_set_down(&tcpip_ctrl_admin[ctrl_idx].tcpip_netif);
	tcpip_ctrl_admin[ctrl_idx].tcpip_ethif_requested_ctrl_state = TCPIP_STATE_OFFLINE;
	tcpip_ctrl_admin[ctrl_idx].tcpip_ethif_ctrl_state = TCPIP_STATE_OFFLINE;
	tcpip_notify_local_ip_addr_assignment_chg(addr_id, TCPIP_IPADDR_STATE_UNASSIGNED);

	return ERR_OK;
}

static FUNC(Std_ReturnType, TCPIP_CODE) tcpip_state_change(const tcpip_local_addr_type *local_addr,
	const uint8 ctrl_idx, const tcpip_state_type state)
{
	(void)ctrl_idx;

	Std_ReturnType result = E_NOT_OK;

	switch (state) {
	case TCPIP_STATE_ONLINE:
		result = tcpip_state_chg_to_online(local_addr);
		break;
	case TCPIP_STATE_ONHOLD:
		result = tcpip_state_chg_to_onhold(local_addr);
		break;
	case TCPIP_STATE_OFFLINE:
		result = tcpip_state_chg_to_offline(local_addr);
		break;

	default:
		result = E_NOT_OK;
		break;
	}

	return result;
}

static FUNC(void, TCPIP_CODE) tcpip_netif_init(const tcpip_config_type *config)
{
	const tcpip_local_addr_type *ptr_local_addr;

	struct netif *netif;
	lwip_netif_ctrl_type *netif_ctrl;
	ip4_addr_t ipaddr;
	ip4_addr_t netmask;
	ip4_addr_t gw;

	for (uint8 local_addr_idx = 0; local_addr_idx < TCPIP_NOF_LOCALADDR; local_addr_idx++) {
		if ((config->config.local_addr_list[local_addr_idx].address_type == TCPIP_UNICAST) &&
		    (config->config.local_addr_list[local_addr_idx].address_assignment->assignment_method ==
		     TCPIP_IPADDR_ASSIGNMENT_STATIC)) {
			ptr_local_addr = &config->config.local_addr_list[local_addr_idx];
			netif = &tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_netif;
			netif_ctrl = &tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_netif_ctrl;
			netif_ctrl->ctrl_id = config->config.ctrl_list[ptr_local_addr->tcpip_ctrl_ref].ethif_ctrl_ref;
			netif_ctrl->priority = config->config.ctrl_list[ptr_local_addr->tcpip_ctrl_ref].frame_prio_default;
			netif->next = NULL;

			ipaddr.addr = ptr_local_addr->static_ip_addr_config->static_ip_address;

			gw.addr = ptr_local_addr->static_ip_addr_config->default_router;

			netif_set_hostname(netif, (const char *)netif_ctrl);
			ip4_netmask_to_ip_addr(&netmask, ptr_local_addr->static_ip_addr_config->netmask);

			if (E_OK == netifapi_netif_add(netif, &ipaddr, &netmask, &gw, NULL, lwip_ethernetif_init,
						       tcpip_input)) {
				if (local_addr_idx == 0u) {
					(void)netifapi_netif_set_default(netif);
				}
				tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].ptr_local_unicast_addr = ptr_local_addr;
			} else {
				TCPIP_ERROR_LOG("tcpip_netif_init: add local addr id %d failed!\n",
						ptr_local_addr->address_id);
			}
		}
	}
}

/****************************************************************************
 * Receive/Transimit
 ****************************************************************************/
static FUNC(err_t, TCPIP_CODE) tcpip_tcp_recv_cb(uint16_t id, struct pbuf *p, uint32_t remote_ip, uint16_t remote_port)
{
	tcpip_sock_addr_type remote_addr;

	err_t result = (err_t)ERR_OK;
	uint8 owner_id = tcpip_socket_admin[id].socket_owner_id;

	struct pbuf *tmp;
	u16_t len = 0;

	if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_rx_indication_fnc_ptr != NULL) {
		len = p->tot_len;
		remote_addr.domain = tcpip_socket_admin[id].remote_addr.domain;
		remote_addr.port = remote_port;
		tcpip_ip_addr_32_to_8(remote_ip, remote_addr.addr);
		TCPIP_DEBUG_LOG("tcpip recv from callback id %d len %u ip %lu, port %u\n", id, p->tot_len, remote_ip,
				remote_port);

		for (tmp = p; (len != 0u) && (tmp != NULL); tmp = tmp->next) {
			/* copy the necessary parts of the buffer */
			TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_rx_indication_fnc_ptr
				((tcpip_socket_id_type)id, &remote_addr, (uint8 *)tmp->payload, tmp->len);
			len = len - tmp->len;
		}
	}

	return result;
}

static FUNC(err_t, TCPIP_CODE) tcpip_tcp_accept_cb(uint16_t id, uint16_t accept_id,
	uint32_t *remote_ip, uint16_t remote_port)
{
	tcpip_sock_addr_type remote_addr;

	err_t result = (err_t)ERR_OK;
	Std_ReturnType ret;
	uint8 owner_id = tcpip_socket_admin[id].socket_owner_id;

	tcpip_socket_admin[accept_id].protocol_type = TCPIP_IPPROTO_TCP;
	tcpip_socket_admin[accept_id].remote_addr.domain = TCPIP_AF_INET;
	tcpip_socket_admin[accept_id].socket_owner_id = owner_id;
	if (tcpip_socket_admin[id].ptr_local_addr != NULL) {
		tcpip_socket_admin[accept_id].ptr_local_addr = tcpip_socket_admin[id].ptr_local_addr;
	}
	remote_addr.domain = tcpip_socket_admin[id].remote_addr.domain;
	remote_addr.port = remote_port;
	tcpip_ip_addr_32_to_8(*remote_ip, remote_addr.addr);
	TCPIP_DEBUG_LOG("tcpip accept from callback accept id %u listen id %u ip %lu, port %u\n", accept_id, id,
			*remote_ip, remote_port);

	if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcp_accepted_fnc_ptr != NULL) {
		ret = TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcp_accepted_fnc_ptr(id, accept_id,
												    &remote_addr);
		if (ret != E_OK) {
			result = (err_t)ERR_CONN;
		}
	}

	return result;
}

static FUNC(err_t, TCPIP_CODE) tcpip_udp_recv_cb(uint16_t id, struct pbuf *p,
	const uint32_t *remote_ip, uint16_t remote_port)
{
	uint8 *data_ptr = NULL;

	err_t result = (err_t)E_OK;
	uint8 owner_id = tcpip_socket_admin[id].socket_owner_id;

	if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_rx_indication_fnc_ptr != NULL) {
		data_ptr = (uint8 *)tcpip_buffer_get(p->tot_len);
		if (data_ptr != NULL) {
			(void)pbuf_copy_partial(p, data_ptr, p->tot_len, 0);
			tcpip_socket_admin[id].remote_addr.port = remote_port;
			tcpip_ip_addr_32_to_8(*remote_ip, tcpip_socket_admin[id].remote_addr.addr);
			TCPIP_DEBUG_LOG("tcpip udp recv from callback id %d len %u ip %lu, port %u\n", id, p->tot_len,
					*remote_ip, remote_port);
			TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_rx_indication_fnc_ptr
				((tcpip_socket_id_type)id, &tcpip_socket_admin[id].remote_addr, data_ptr, p->tot_len);
			tcpip_buffer_free(data_ptr);
		} else {
			return (err_t)E_NOT_OK;
		}
	}

	return result;
}

static FUNC(void, TCPIP_CODE) tcpip_tcp_sent_cb(uint16_t id, uint16_t len)
{
	uint8 owner_id = tcpip_socket_admin[id].socket_owner_id;

	if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tx_confirmation_fnc_ptr != NULL) {
		TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tx_confirmation_fnc_ptr(id, len);
	}
	return;
}

static FUNC(void, TCPIP_CODE) tcpip_tcp_err_cb(uint16_t id, err_t err)
{
	tcpip_event_type event;
	struct tcpip_wrapper_tcp_ctx *tcp_ctx;
	uint8 owner_id = tcpip_socket_admin[id].socket_owner_id;

	if (ERR_CONN == (err_enum_t)err) {
		event = TCPIP_TCP_FIN_RECEIVED;
	} else {
		event = TCPIP_TCP_CLOSED;
		tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(id);
		if (tcp_ctx != NULL) {
			(void)tcpip_wrapper_tcp_ctx_free(tcp_ctx, TRUE);
		}

		(void)memset(&tcpip_socket_admin[id], 0, sizeof(tcpip_socket_admin_type));
	}

	if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcpip_event_fnc_ptr != NULL) {
		TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcpip_event_fnc_ptr(id, event);
	}

	return;
}

FUNC(void, TCPIP_CODE) tcpip_tx_confirmation(uint8 ethif_ctrl_idx, eth_buf_idx_type eth_buf_idx)
{
#if (CONFIG_MODULE_ETHIF == 1)
	(void)ethif_ctrl_idx;
	uint8 owner_id;
	tcpip_socket_id_type socket_id;
	socket_id = tcpip_buf_idx_to_socket_id[eth_buf_idx];
	if (socket_id >= TCPIP_MAX_NOF_SOCKETS) {
		return;
	}
	if (tcpip_socket_admin[socket_id].protocol_type == TCPIP_IPPROTO_UDP) {
		owner_id = tcpip_socket_admin[socket_id].socket_owner_id;
		if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tx_confirmation_fnc_ptr != NULL) {
			TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tx_confirmation_fnc_ptr(socket_id, 1);
			tcpip_buf_idx_to_socket_id[eth_buf_idx] = TCPIP_MAX_NOF_SOCKETS;
		}
	}
#endif
}

/**
 * @brief By this API service the TCP/IP stack gets an indication and the data of a received frame.
 * @param ctrl_idx - Index of the EthIf controller.
 * @param frame_type - Frame type of received Ethernet frame
 * @param is_broadcast - Parameter to indicate a broadcast frame
 * @param phys_addr_ptr - Pointer to Physical source address (MAC address in network byte
 *                      order) of received Ethernet frame
 * @param data_ptr - Pointer to payload of the received Ethernet frame (i.e. Ethernet
 *                  header is not provided).
 * @param len_byte - Length of received data.
 * @return void
 */
/** @req 4.2.2/SWS_TCPIP_00029 */
FUNC(void, TCPIP_CODE) tcpip_rx_indication(uint8 ctrl_idx, eth_frame_type frame_type,
	boolean is_broadcast, const uint8 *phys_addr_ptr, const uint8 *data_ptr, uint16 len_byte)
{
	if (tcpip_initialized == TRUE) {
		tcpip_wrapper_rx_indication(ctrl_idx, frame_type, is_broadcast, phys_addr_ptr, data_ptr, len_byte);
	}
}

/****************************************************************************
 * Address
 ****************************************************************************/
static FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_change_parameter(tcpip_socket_id_type socket_id,
	tcpip_param_id_type parameter_id, const uint8 *parameter_value)
{
	Std_ReturnType result = E_NOT_OK;
	const struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	err_t err = (err_t)ERR_ARG;

	tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);
	if (tcp_ctx == NULL) {
		return result;
	}

	switch (parameter_id) {
	case TCPIP_PARAMID_TCP_RXWND_MAX: {
		err = tcpip_wrapper_tcp_set_rxwnd(tcp_ctx, *((uint16 *)parameter_value));
		break;
	}
	case TCPIP_PARAMID_TCP_SNDBUF_MAX: {
		err = tcpip_wrapper_tcp_set_sndbuf(tcp_ctx, *((uint16 *)parameter_value));
		break;
	}
	case TCPIP_PARAMID_FRAMEPRIO: {
		err = tcpip_wrapper_tcp_set_iptos(tcp_ctx, *((const uint8 *)parameter_value));
		break;
	}
	case TCPIP_PARAMID_TCP_NAGLE: {
		err = tcpip_wrapper_tcp_set_nagle(tcp_ctx, *((boolean *)parameter_value));
		break;
	}
	case TCPIP_PARAMID_TCP_KEEPALIVE: {
		err = tcpip_wrapper_tcp_set_keepalive_enable(tcp_ctx, *((boolean *)parameter_value));
		break;
	}
	case TCPIP_PARAMID_TCP_KEEPALIVE_TIME: {
		err = tcpip_wrapper_tcp_set_keepalive_time(tcp_ctx, *((uint16 *)parameter_value));
		break;
	}
#if LWIP_TCP_KEEPALIVE
	case TCPIP_PARAMID_TCP_KEEPALIVE_PROBES_MAX: {
		err = tcpip_wrapper_tcp_set_keepalive_probes_max(tcp_ctx, *((uint16 *)parameter_value));
		break;
	}
	case TCPIP_PARAMID_TCP_KEEPALIVE_INTERVAL: {
		err = tcpip_wrapper_tcp_set_keepalive_interval(tcp_ctx, *((uint16 *)parameter_value));
		break;
	}
#endif
	case TCPIP_PARAMID_TTL: {
		err = tcpip_wrapper_tcp_set_ipttl(tcp_ctx, *((const uint8 *)parameter_value));
		break;
	}
	default: {
		/*Do nothing */
		break;
	}
	}

	if (ERR_OK == (err_t)err) {
		result = E_OK;
	} else {
		TCPIP_ERROR_LOG("tcpip_tcp_change_parameter err %d\n", err);
	}

	return result;
}

static FUNC(Std_ReturnType, TCPIP_CODE) tcpip_udp_change_parameter(tcpip_socket_id_type socket_id,
	tcpip_param_id_type parameter_id, const uint8 *parameter_value)
{
	Std_ReturnType result = E_NOT_OK;
	const struct tcpip_wrapper_udp_ctx *udp_ctx = NULL;
	err_t err = (err_t)ERR_ARG;
	const tcpip_local_addr_type *ptr_local_addr = NULL;

	udp_ctx = tcpip_wrapper_get_udp_ctx_by_id(socket_id);
	if (udp_ctx == NULL) {
		return result;
	}

	switch (parameter_id) {
	case TCPIP_PARAMID_FRAMEPRIO: {
		err = tcpip_wrapper_udp_set_iptos(udp_ctx, *parameter_value);
		break;
	}
	case TCPIP_PARAMID_TTL: {
		err = tcpip_wrapper_udp_set_ipttl(udp_ctx, *parameter_value);
		break;
	}

	case TCPIP_PARAMID_UDP_CHECKSUM: {
		ptr_local_addr = tcpip_socket_admin[socket_id].ptr_local_addr;
		if (ptr_local_addr != NULL) {
			err = tcpip_wrapper_udp_set_chksum(&tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_netif,
						     (boolean)(*parameter_value));
		}
		break;
	}
	default: {
		/* Do nothing*/
		break;
	}
	}

	if (ERR_OK == (err_enum_t)err) {
		result = E_OK;
	} else {
		TCPIP_ERROR_LOG("tcpip_udp_change_parameter err %d\n", err);
	}

	return result;
}

/****************************************************************************
 * Init and Main
 ****************************************************************************/
static FUNC(void, TCPIP_CODE) tcpip_inner_init(void)
{
	tcpip_netif_init((const tcpip_config_type *)tcpip_config_ptr);

	/* All net card online */
	uint8 tcpip_ctrl_idx = TCPIP_NOF_CONTROLLERS;
	uint8 local_addr_idx = 0;
	const tcpip_local_addr_type *local_addr = NULL;

	for (uint8 i = 0; i < TCPIP_NOF_CONTROLLERS; i++) {
		tcpip_ctrl_idx = i;

		if (tcpip_get_local_addr_idx_by_ctrl_idx(tcpip_ctrl_idx, &local_addr_idx) == E_OK) {
			local_addr = &tcpip_config_ptr->config.local_addr_list[local_addr_idx];
			switch (local_addr->address_assignment->assignment_method) {
			case TCPIP_IPADDR_ASSIGNMENT_STATIC:
				(void)tcpip_state_change(local_addr, tcpip_ctrl_idx, TCPIP_STATE_ONLINE);
				break;
			default:
				/* Do nothing */
				break;
			}
		}
	}

	tcpip_initialized = TRUE;
}

static FUNC(void, TCPIP_CODE) tcpip_main_function_tx(void)
{
	const struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;

	for (uint16 i = 0; i < TCPIP_MAX_NOF_SOCKETS; i++) {
		switch (tcpip_socket_admin[i].protocol_type) {
		case TCPIP_IPPROTO_TCP:
			tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(i);
			if (tcp_ctx != NULL) {
				if (tcp_ctx->unsent != NULL) {
					(void)tcpip_wrapper_tcp_output(tcp_ctx);
				}
			}
			break;

		default:
			/* Do nothing */
			break;
		}
	}
}

/**
 * @brief TcpIp main function.
 * @param  void
 * @return void
 */
/** @req 4.2.2/SWS_TCPIP_00026 */
/** !req 4.2.2/SWS_TCPIP_00094 */
FUNC(void, TCPIP_CODE) tcpip_main_function(void)
{
	tcpip_main_function_tx();
#if (CONFIG_TEST_CMD_IPERF == 1)
		iperf_main_function();
#endif
}

FUNC(void, TCPIP_CODE) TcpIp_MainFunction(void)
{
	tcpip_main_function();
}

/****************************************************************************
 * Public Interface
 ****************************************************************************/
/**
 * @brief This service initializes the TCP/IP Stack
 * @param config_ptr - Configuration data ptr
 * @return void
 */
/** @req 4.4.0/SWS_TCPIP_00002 */
int tcpip_init_done = 0;
FUNC(void, TCPIP_CODE) bsw_tcpip_init(const tcpip_config_type *config_ptr)
{
	uint16 i;

	tcpip_wrapper_stack_init();

	if (config_ptr != NULL) {
		tcpip_config_ptr = (tcpip_config_type *)config_ptr;

		for (i = 0; i < TCPIP_NOF_CONTROLLERS; i++) {
			memset(&tcpip_ctrl_admin[i], 0, sizeof(tcpip_ctrl_admin[i]));
			tcpip_ctrl_admin[i].tcpip_ethif_ctrl_state = TCPIP_STATE_OFFLINE;
			tcpip_ctrl_admin[i].tcpip_ethif_requested_ctrl_state = TCPIP_STATE_OFFLINE;
			tcpip_ctrl_admin[i].tcpip_ethif_ctrl_onhold_state_cnt = 0;
			tcpip_ctrl_admin[i].ptr_local_unicast_addr = NULL;
			tcpip_ctrl_admin[i].link_up = FALSE;
		}

		for (i = 0; i < TCPIP_MAX_NOF_SOCKETS; i++) {
			tcpip_socket_admin[i].ptr_local_addr = NULL;
			tcpip_socket_admin[i].protocol_type = TCPIP_IPPROTO_INVALID;
		}
		for (i = 0; i < TCPIP_NOF_LOCALADDR; i++) {
			tcpip_ip_addr_state[i] = TCPIP_IPADDR_STATE_UNASSIGNED;
		}
#if (CONFIG_MODULE_ETHIF == 1)
		for (i = 0; i < ETHIf_MAX_TXBUFSTOTAL; i++) {
			tcpip_buf_idx_to_socket_id[i] = TCPIP_MAX_NOF_SOCKETS;
		}
#endif
		tcpip_inner_init();
	}

	tcpip_init_done = 1;
	tcpip_request_com_mode(0, TCPIP_STATE_ONLINE);
	tcpip_request_com_mode(1, TCPIP_STATE_ONLINE);
	tcpip_request_com_mode(2, TCPIP_STATE_ONLINE);
	tcpip_request_com_mode(3, TCPIP_STATE_ONLINE);
	extern void eth_enable_eth_mac(void);
	eth_enable_eth_mac();
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_close(tcpip_socket_id_type socket_id, boolean abort)
{
	uint8_t is_tcp = 0;
	struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	struct tcpip_wrapper_udp_ctx *udp_ctx = NULL;
	tcpip_event_type event;
	err_t err;
	uint8 owner_id;
	enum tcp_state state = CLOSED;

	VALIDATE((socket_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_CLOSE_SERVICE_ID_TCPIP_E_INV_ARG);
	owner_id = tcpip_socket_admin[socket_id].socket_owner_id;
	is_tcp = tcpip_wrapper_tcpip_ctx_is_tcp(socket_id);
	if (is_tcp != 0u) {
		event = TCPIP_TCP_CLOSED;
	} else {
		event = TCPIP_UDP_CLOSED;
	}

	if (is_tcp != 0u) {
		tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);
		if (tcp_ctx == NULL) {
			return E_NOT_OK;
		}

		state = (enum tcp_state)tcpip_wrapper_tcp_get_state(tcp_ctx);
		if ((state == CLOSED) || (state == SYN_SENT) || (state == LISTEN)) {
			abort = TRUE;
		}
		/** @req 4.4.0/SWS_TCPIP_00109 */
		err = tcpip_wrapper_tcp_ctx_free(tcp_ctx, abort);
		if (err != (err_t)ERR_OK) {
			return E_NOT_OK;
		}
	} else {
		udp_ctx = tcpip_wrapper_get_udp_ctx_by_id(socket_id);
		if (udp_ctx == NULL) {
			return E_NOT_OK;
		}
		tcpip_wrapper_udp_ctx_free(udp_ctx);
	}
	if (abort || (is_tcp == 0u)) {
		(void)memset(&tcpip_socket_admin[socket_id], 0, sizeof(tcpip_socket_admin_type));
		if (TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcpip_event_fnc_ptr != NULL) {
			TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_tcpip_event_fnc_ptr(socket_id, event);
		}
	}
	return E_OK;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_bind(tcpip_socket_id_type socket_id,
	tcpip_local_addr_id_type local_addr_id, const uint16 *port_ptr)
{
	err_t ret;
	uint8_t is_tcp = 0u;
	const struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	struct tcpip_wrapper_udp_ctx *udp_ctx = NULL;
	ip_addr_t ip_addr;

	ip_addr_t *ip_addr_ptr = NULL;
	const tcpip_local_addr_type *ptr_local_addr = NULL;

	VALIDATE((socket_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_BIND_SERVICE_ID_TCPIP_E_INV_ARG);

	if (tcpip_local_addr_id_is_online(local_addr_id) != TRUE) {
		return E_NOT_OK;
	}

	for (uint32 i = 0; i < TCPIP_NOF_LOCALADDR; i++) {
		if (tcpip_config_ptr->config.local_addr_list[i].address_id == local_addr_id) {
			ptr_local_addr = &tcpip_config_ptr->config.local_addr_list[i];
		}
	}

	if (ptr_local_addr != NULL) {
		ip_addr.addr = ptr_local_addr->static_ip_addr_config->static_ip_address;
	} else {
		TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID13, *port_ptr);
		return E_NOT_OK;
	}

	if (ptr_local_addr->address_type != TCPIP_ANYCAST) {
		ip_addr_ptr = &ip_addr;
	}

	is_tcp = tcpip_wrapper_tcpip_ctx_is_tcp(socket_id);
	if (is_tcp != 0u) {
		tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);
		if (tcp_ctx == NULL) {
			TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID13, *port_ptr);
			return E_NOT_OK;
		}
		if (tcp_ctx->conn != NULL) {
			if (ptr_local_addr->address_type != TCPIP_ANYCAST) {
				tcp_ctx->conn->netif_idx =
					tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_netif.num + 1;
			}
		}
		ret = tcpip_wrapper_tcp_bind(tcp_ctx, ip_addr_ptr, *port_ptr);
	} else {
		udp_ctx = tcpip_wrapper_get_udp_ctx_by_id(socket_id);
		if (udp_ctx == NULL) {
			TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID13, *port_ptr);
			return E_NOT_OK;
		}
		if (udp_ctx->conn != NULL) {
			if (ptr_local_addr->address_type != TCPIP_ANYCAST) {
				udp_ctx->conn->netif_idx =
					tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_netif.num + 1;
			}
		}
		if (tcpip_wrapper_udp_bind(udp_ctx, ip_addr_ptr, *port_ptr) != (err_t)ERR_OK) {
			TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID13, *port_ptr);
			return E_NOT_OK;
		} else {
			ret = (err_t)ERR_OK;
		}
		if (ptr_local_addr->address_type == TCPIP_MULTICAST) {
			ret = tcpip_wrapper_udp_join_multicast_group(netif_ip4_addr
				(&tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_netif), &ip_addr);
		}
	}

	if (ret != (err_t)ERR_OK) {
		TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID13, *port_ptr);
		return E_NOT_OK;
	}

	tcpip_socket_admin[socket_id].ptr_local_addr = ptr_local_addr;

	return E_OK;
}

/**
 * @brief By this API service the TCP/IP stack is requested to establish a TCP connection to
 *        the configured peer.
 * @param socket_id - Socket handle identifying the local socket resource
 * @param remote_addr_ptr - IP address and port of the remote host to connect to.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00022 */
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_connect(tcpip_socket_id_type socket_id, const tcpip_sock_addr_type *remote_addr_ptr)
{
	Std_ReturnType result = E_OK;
	uint8_t is_tcp = 0;
	struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	ip4_addr_t remote_addr;

	/** @req 4.4/SWS_TCPIP_00129 */

	VALIDATE((socket_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_TCPCONNECT_SERVICE_ID_TCPIP_E_INV_ARG);

	VALIDATE((NULL != remote_addr_ptr), TCPIP_TCPIP_TCPCONNECT_SERVICE_ID_TCPIP_E_PARAM_POINTER);

	if (tcpip_socket_id_is_online(socket_id) != TRUE) {
		return E_NOT_OK;
	}

	is_tcp = tcpip_wrapper_tcpip_ctx_is_tcp(socket_id);
	if (is_tcp != 0u) {
		tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);
		if (tcp_ctx == NULL) {
			return E_NOT_OK;
		}

		remote_addr.addr = tcpip_ip_addr_8_to_32(remote_addr_ptr->addr);
		(void)tcpip_wrapper_tcp_connect(tcp_ctx, &remote_addr, remote_addr_ptr->port);
	}

	return result;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_listen(tcpip_socket_id_type socket_id, uint16 max_channels)
{
	Std_ReturnType result;
	uint8_t is_tcp = 0;
	struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	err_t err;

	VALIDATE((socket_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_TCPLISTEN_SERVICE_ID_TCPIP_E_INV_ARG);
	is_tcp = tcpip_wrapper_tcpip_ctx_is_tcp(socket_id);
	if (is_tcp != 0u) {
		tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);
		if (tcp_ctx == NULL) {
			result = E_NOT_OK;
		} else {
			err = tcpip_wrapper_tcp_listen(tcp_ctx, (uint8)max_channels);
			if (err == (err_t)ERR_OK) {
				result = E_OK;
			} else {
				TCPIP_ERROR_LOG("TcpIp_TcpListen: tcpip_wrapper_tcp_listen failed %d\n", err);
				result = E_NOT_OK;
			}
		}
	} else {
		result = E_NOT_OK;
	}
	return result;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_received(tcpip_socket_id_type socket_id, uint32 length)
{
	Std_ReturnType result = E_OK;
	const struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	uint8 is_tcp = 0;

	VALIDATE((socket_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_TCPRECEIVED_SERVICE_ID_TCPIP_E_INV_ARG);
	is_tcp = tcpip_wrapper_tcpip_ctx_is_tcp(socket_id);
	if (is_tcp != 0u) {
		tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);
		if (tcp_ctx == NULL) {
			return E_NOT_OK;
		}
		tcpip_wrapper_tcp_recved(tcp_ctx, (uint16)length);
	}
	return result;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_change_parameter(tcpip_socket_id_type socket_id,
	tcpip_param_id_type parameter_id, const uint8 *parameter_value)
{
	Std_ReturnType result;
	uint8_t is_tcp = 0;

	is_tcp = tcpip_wrapper_tcpip_ctx_is_tcp(socket_id);
	if (is_tcp != 0u) {
		result = tcpip_tcp_change_parameter(socket_id, parameter_id, parameter_value);
	} else if (tcpip_wrapper_tcpip_ctx_is_udp(socket_id) != 0u) {
		result = tcpip_udp_change_parameter(socket_id, parameter_id, parameter_value);
	} else {
		result = E_NOT_OK;
	}
	return result;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_ip_addr(tcpip_local_addr_id_type local_addr_id,
	tcpip_sock_addr_type *ip_addr_ptr, uint8 *netmask_ptr, tcpip_sock_addr_type *default_router_ptr)
{
	const tcpip_local_addr_type *ptr_local_addr;
	Std_ReturnType result;

	VALIDATE(ip_addr_ptr != NULL, TCPIP_TCPIP_GETIPADDR_SERVICE_ID_TCPIP_E_PARAM_POINTER);

	uint8 local_addr_idx = get_local_addr_list_config_idx(local_addr_id);

	if (local_addr_idx != INVALID_LOCALADDR_IDX) {
		ptr_local_addr = &tcpip_config_ptr->config.local_addr_list[local_addr_idx];

		/** @req 4.2.2/SWS_TCPIP_00205 */

		VALIDATE(ip_addr_ptr->domain == ptr_local_addr->domain, TCPIP_TCPIP_GETIPADDR_SERVICE_ID_TCPIP_E_INV_ARG);
		/** @req 4.2.2/SWS_TCPIP_00206 */

		VALIDATE(default_router_ptr->domain == ip_addr_ptr->domain,
			 TCPIP_TCPIP_GETIPADDR_SERVICE_ID_TCPIP_E_INV_ARG);

		tcpip_ip_addr_32_to_8(ptr_local_addr->static_ip_addr_config->static_ip_address, ip_addr_ptr->addr);
		tcpip_ip_addr_32_to_8(ptr_local_addr->static_ip_addr_config->default_router, default_router_ptr->addr);
		*netmask_ptr = ptr_local_addr->static_ip_addr_config->netmask;
		result = E_OK;
	} else {
		result = E_NOT_OK;
	}
	return result;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_phys_addr(tcpip_local_addr_id_type local_addr_id, uint8 *phys_addr_ptr)
{
	Std_ReturnType result;
	uint8 ethif_ctrl_idx;
	uint8 local_addr_idx = get_local_addr_list_config_idx(local_addr_id);

	VALIDATE((phys_addr_ptr != NULL), TCPIP_TCPIP_GETPHYSADDR_SERVICE_ID_TCPIP_E_PARAM_POINTER);
	if (local_addr_idx != INVALID_LOCALADDR_IDX) {
#if (CONFIG_MODULE_ETHIF == 1)
		const tcpip_local_addr_type *ptr_local_addr = &tcpip_config_ptr->config.local_addr_list[local_addr_idx];

		ethif_ctrl_idx = tcpip_ctrl_admin[ptr_local_addr->tcpip_ctrl_ref].tcpip_netif_ctrl.ctrl_id;
		ethif_get_phys_addr(ethif_ctrl_idx, phys_addr_ptr);
#endif
		result = E_OK;
	} else {
		result = E_NOT_OK;
	}

	return result;
}

/**
 * @brief TcpIp_GetCtrlIdx returns the index of the controller related to LocalAddrId.
 * @param local_addr_id - Local address identifier referring to the local IP address which
 *                      shall be obtained.
 * @param ctrl_idx_ptr - Pointer to the memory where the index of the controller related to
 *                     LocalAddrId is stored
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00140 */
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_ctrl_idx(tcpip_local_addr_id_type local_addr_id, uint8 *ctrl_idx_ptr)
{
	Std_ReturnType result;
	uint8 ctrl_idx;
	uint8 local_addr_idx = get_local_addr_list_config_idx(local_addr_id);

	if (local_addr_idx != INVALID_LOCALADDR_IDX) {
		/** @req 4.2.2/SWS_TCPIP_00141 */
		const tcpip_local_addr_type *ptr_local_addr = &tcpip_config_ptr->config.local_addr_list[local_addr_idx];

		ctrl_idx = ptr_local_addr->tcpip_ctrl_ref;
		*ctrl_idx_ptr = tcpip_ctrl_admin[ctrl_idx].tcpip_netif_ctrl.ctrl_id;
		result = E_OK;
	} else {
		result = E_NOT_OK;
	}

	return result;
}

/**
 * @brief This service transmits data via UDP to a remote node. The transmission of the
 * data is immediately performed with this function call by forwarding it to EthIf.
 * @param socket_id - Socket handle identifying the local socket resource
 * @param data_ptr - Pointer to a linear buffer of TotalLength bytes containing the
 *                  data to be transmitted.
 *                  In case DataPtr is a NULL_PTR, TcpIp shall retrieve data from
 *                  upper layer via callback <Up>_CopyTxData().
 * @param remote_addr_ptr - IP address and port of the remote host to transmit to.
 * @param total_length - indicates the payload size of the UDP datagram.
 * @return result of the function.
 */
/** @req 4.2.2/SWS_TCPIP_00025 */
/** @req 4.2.2/SWS_TCPIP_00225 */
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_udp_transmit(tcpip_socket_id_type socket_id,
	const uint8 *data_ptr, const tcpip_sock_addr_type *remote_addr_ptr, uint16 total_length)
{
	Std_ReturnType result = TCPIP_ERR_OK;
	uint8_t is_udp = 0;
	struct tcpip_wrapper_udp_ctx *udp_ctx = NULL;
	ip4_addr_t dst_addr;
	const ip4_addr_t *dst_ip = &dst_addr;
	uint8 owner_id;

	const uint8 *send_buf_ptr = NULL;
	uint16 len = total_length;
	uint8 *buf_ptr = NULL;
	BufReq_ReturnType buf_req;
	err_t err;

	VALIDATE_RV((socket_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_UDPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG,
		    TCPIP_ERR_INVPARAM);
	is_udp = tcpip_wrapper_tcpip_ctx_is_udp(socket_id);

	VALIDATE_RV((is_udp != 0u), TCPIP_TCPIP_UDPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG, TCPIP_ERR_INVPARAM);
	udp_ctx = tcpip_wrapper_get_udp_ctx_by_id(socket_id);

	VALIDATE_RV((udp_ctx != NULL), TCPIP_TCPIP_UDPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG, TCPIP_ERR_INVPARAM);

	VALIDATE_RV((remote_addr_ptr != NULL), TCPIP_TCPIP_UDPTRANSMIT_SERVICE_ID_TCPIP_E_PARAM_POINTER,
		    TCPIP_ERR_INVPARAM);

	if (tcpip_socket_id_is_online(socket_id) != TRUE) {
		TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID12, TCPIP_ERR_OFFLINE);
		return TCPIP_ERR_OFFLINE;
	}

	dst_addr.addr = tcpip_ip_addr_8_to_32(remote_addr_ptr->addr);
	owner_id = tcpip_socket_admin[socket_id].socket_owner_id;

	if (NULL == data_ptr) {
		VALIDATE((TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_copy_tx_data_fnc_ptr != NULL),
			 TCPIP_TCPIP_UDPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG);
		buf_ptr = (uint8 *)tcpip_buffer_get(len);

		VALIDATE((buf_ptr != NULL), TCPIP_TCPIP_UDPTRANSMIT_SERVICE_ID_TCPIP_E_NOBUFS);
		buf_req = TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_copy_tx_data_fnc_ptr(socket_id, buf_ptr,
												       len);
		if (buf_req != BUFREQ_OK) {
			tcpip_buffer_free(buf_ptr);
			return result;
		}
		send_buf_ptr = buf_ptr;
	} else {
		send_buf_ptr = data_ptr;
	}
	err = tcpip_wrapper_udp_send(udp_ctx, dst_ip, remote_addr_ptr->port, send_buf_ptr, len);
	if (err == (err_t)ERR_OK) {
		result = E_OK;
	} else {
		TCPIP_ERROR_LOG("TcpIp_UdpTransmit: tcpip_wrapper_udp_send failed %d\n", err);
	}
	tcpip_buffer_free(buf_ptr);

	return result;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_transmit(tcpip_socket_id_type socket_id,
	const uint8 *data_ptr, uint32 available_length, boolean force_retrieve)
{
	Std_ReturnType result = TCPIP_ERR_OK;
	BufReq_ReturnType buf_req;
	uint8 *buf_ptr = NULL;
	struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	uint8_t is_tcp = 0;
	uint16 len = (uint16)available_length;
	uint16 snd_len = 0;
	uint8_t apiflags = 0;
	uint8 owner_id;
	(void)force_retrieve;
	err_t err;

	VALIDATE_RV((socket_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_TCPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG,
		    TCPIP_ERR_INVPARAM);
	is_tcp = tcpip_wrapper_tcpip_ctx_is_tcp(socket_id);

	VALIDATE_RV((is_tcp != 0u), TCPIP_TCPIP_TCPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG, TCPIP_ERR_INVPARAM);
	tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);

	VALIDATE_RV((tcp_ctx != NULL), TCPIP_TCPIP_TCPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG, TCPIP_ERR_INVPARAM);

	snd_len = tcpip_wrapper_tcp_get_sndbuf(tcp_ctx);
	if (snd_len < len) {
		TCPIP_WARN_LOG(TCPIP_DLT_CHECK_ID11, TCPIP_ERR_SNDBUF);
		return TCPIP_ERR_SNDBUF;
	}

	if (tcpip_socket_id_is_online(socket_id) != TRUE) {
		TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID11, TCPIP_ERR_OFFLINE);
		return TCPIP_ERR_OFFLINE;
	}

	if (available_length == 0u) {
		return TCPIP_ERR_OK;
	}
	owner_id = tcpip_socket_admin[socket_id].socket_owner_id;
	/** @todo 4.4/SWS_TCPIP_00074 */
	if (data_ptr == NULL) {
		/** @todo 4.4/SWS_TCPIP_00228 */
		/* Copy data callback to upper layer  */

		VALIDATE_RV((TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_copy_tx_data_fnc_ptr != NULL),
			    TCPIP_TCPIP_TCPTRANSMIT_SERVICE_ID_TCPIP_E_INV_ARG, TCPIP_ERR_INVPARAM);
		do {
			uint16 trans_len;

			trans_len = (len < TCP_MSS)? len:TCP_MSS;

			buf_ptr = (uint8 *)tcpip_buffer_get(trans_len);

			VALIDATE_RV((buf_ptr != NULL), TCPIP_TCPIP_TCPTRANSMIT_SERVICE_ID_TCPIP_E_NOBUFS, TCPIP_ERR_MEM);
			buf_req = TcpIp_GetSocketOwnerListOfTcpIpConfig()[owner_id].socket_owner_copy_tx_data_fnc_ptr(socket_id,
				buf_ptr, trans_len);
			if (buf_req != BUFREQ_OK) {
				tcpip_buffer_free(buf_ptr);
				TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID11, TCPIP_ERR_COPYTXDATA);
				return TCPIP_ERR_COPYTXDATA;
			}

			data_ptr = buf_ptr;
			err = tcpip_wrapper_tcp_send(tcp_ctx, data_ptr, trans_len, apiflags);
			if (err != (err_t)ERR_OK) {
				tcpip_buffer_free(buf_ptr);
				if (err == (err_t)ERR_CONN) {
					result = TCPIP_ERR_CONN;
				} else if (err == (err_t)ERR_MEM) {
					result = TCPIP_ERR_MEM;
				} else {
					result = (Std_ReturnType)err;
				}
				TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID11, result);
				return result;
			} else {
				result = E_OK;
			}
			len = len - trans_len;
		} while (len > 0u);
	} else {
		apiflags |= (uint8_t)TCP_WRITE_FLAG_COPY;
		err = tcpip_wrapper_tcp_send(tcp_ctx, data_ptr, len, apiflags);
		if (err != (err_t)ERR_OK) {
			tcpip_buffer_free(buf_ptr);
			if (err == (err_t)ERR_CONN) {
				result = TCPIP_ERR_CONN;
			} else if (err == (err_t)ERR_MEM) {
				result = TCPIP_ERR_MEM;
			} else {
				result = (Std_ReturnType)err;
			}
			TCPIP_DEBUG_LOG(TCPIP_DLT_CHECK_ID11, result);
		} else {
			result = E_OK;
		}
	}
	return result;
}

FUNC(tcpip_socket_state_type, TCPIP_CODE) tcpip_get_socket_state_of_socket_tcp(tcpip_socket_id_type socket_id)
{
	tcpip_socket_state_type state = TCPIP_TCP_SOCK_STATE_INVALID;
	enum tcp_state tcpip_wrapper_state;
	const struct tcpip_wrapper_tcp_ctx *tcp_ctx;

	if (socket_id >= TCPIP_MAX_NOF_SOCKETS) {
		return state;
	}

	if (tcpip_wrapper_tcpip_ctx_is_tcp(socket_id) == 0u) {
		return state;
	}

	tcp_ctx = tcpip_wrapper_get_tcp_ctx_by_id(socket_id);
	if (tcp_ctx == NULL) {
		return state;
	}

	tcpip_wrapper_state = (enum tcp_state)tcpip_wrapper_tcp_get_state(tcp_ctx);
	switch (tcpip_wrapper_state) {
	case CLOSED:

		state = TCPIP_TCP_SOCK_STATE_CLOSED;
		break;
	case LISTEN:

		state = TCPIP_TCP_SOCK_STATE_LISTEN;
		break;
	case SYN_SENT:

		state = TCPIP_TCP_SOCK_STATE_SYNSENT;
		break;
	case SYN_RCVD:

		state = TCPIP_TCP_SOCK_STATE_SYNRCVD;
		break;
	case ESTABLISHED:

		state = TCPIP_TCP_SOCK_STATE_ESTABLISHED;
		break;
	case FIN_WAIT_1:

		state = TCPIP_TCP_SOCK_STATE_FINWAIT1;
		break;
	case FIN_WAIT_2:

		state = TCPIP_TCP_SOCK_STATE_FINWAIT2;
		break;
	case CLOSE_WAIT:

		state = TCPIP_TCP_SOCK_STATE_CLOSEWAIT;
		break;
	case CLOSING:

		state = TCPIP_TCP_SOCK_STATE_CLOSING;
		break;
	case LAST_ACK:

		state = TCPIP_TCP_SOCK_STATE_LASTACK;
		break;
	case TIME_WAIT:

		state = TCPIP_TCP_SOCK_STATE_TIMEWAIT;
		break;
	default:
		/* Do nothing */
		break;
	}
	return state;
}

FUNC(Std_ReturnType, TCPIP_CODE) tcpip_request_com_mode(uint8 ctrl_idx, tcpip_state_type state)
{
	Std_ReturnType result = E_OK;
	uint8 local_addr_idx = 0;
	const tcpip_local_addr_type *local_addr = NULL;
	uint8 tcpip_ctrl_idx = TCPIP_NOF_CONTROLLERS;

	for (uint8 i = 0; i < TCPIP_NOF_CONTROLLERS; i++) {
		if (tcpip_ctrl_admin[i].tcpip_netif_ctrl.ctrl_id == ctrl_idx) {
			tcpip_ctrl_idx = i;
			break;
		}
	}
	if (tcpip_ctrl_idx >= TCPIP_NOF_CONTROLLERS) {
		return E_NOT_OK;
	}

	VALIDATE((state != TCPIP_STATE_STARTUP), TCPIP_TCPIP_REQUESTCOMMODE_SERVICE_ID_TCPIP_E_INV_ARG);

	VALIDATE((state != TCPIP_STATE_SHUTDOWN), TCPIP_TCPIP_REQUESTCOMMODE_SERVICE_ID_TCPIP_E_INV_ARG);

	if (tcpip_get_local_addr_idx_by_ctrl_idx(tcpip_ctrl_idx, &local_addr_idx) == E_OK) {
		local_addr = &tcpip_config_ptr->config.local_addr_list[local_addr_idx];
		switch (local_addr->address_assignment->assignment_method) {
		case TCPIP_IPADDR_ASSIGNMENT_STATIC:
			(void)tcpip_state_change(local_addr, tcpip_ctrl_idx, state);
			break;
		default:
			/* Do nothing */
			break;
		}
	}

	return result;
}

/**
 * @brief This API creates/allocates a new socket , needed to be global for TcpIp configuration file to use this
 * @param socket_owner_id - Socket Id identifying the socket owner
 * @param domain - IP address family.
 * @param protocol - Socket protocol as sub-family of parameter type.
 * @param socket_id_ptr - Pointer to socket identifier representing the requested socket.
 * This socket identifier must be provided for all further API calls which requires a SocketId.
 * @return E_OK or E_NOT_OK
 */
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_socket(uint8 socket_owner_id,
	tcpip_domain_type domain, tcpip_protocol_type protocol, tcpip_socket_id_type *socket_id_ptr)
{
	Std_ReturnType result = E_OK;
	struct tcpip_wrapper_tcp_ctx *tcp_ctx = NULL;
	struct tcpip_wrapper_udp_ctx *udp_ctx = NULL;

	/* @req 4.4.0/SWS_TCPIP_00128 */

	VALIDATE(((domain == TCPIP_AF_INET)), TCPIP_TCPIP_GETSOCKET_SERVICE_ID_TCPIP_E_AFNOSUPPORT);
	// No specified autosar error for protocol check, use TCPIP_E_NOPROTOOPT as it seems most logical
	VALIDATE((TCPIP_IPPROTO_UDP == protocol) || (TCPIP_IPPROTO_TCP == protocol),
		 TCPIP_TCPIP_GETSOCKET_SERVICE_ID_TCPIP_E_NOPROTOOPT);
	VALIDATE((socket_owner_id < TCPIP_MAX_NOF_SOCKETS), TCPIP_TCPIP_GETSOCKET_SERVICE_ID_TCPIP_E_INV_ARG);

	if (TCPIP_IPPROTO_TCP == protocol) {
		tcp_ctx = tcpip_wrapper_tcp_ctx_new();
		if (tcp_ctx == NULL) {
			return E_NOT_OK;
		}

		(void)tcpip_wrapper_regist_tcp_recv(tcp_ctx, tcpip_tcp_recv_cb);
		(void)tcpip_wrapper_regist_tcp_accept(tcp_ctx, tcpip_tcp_accept_cb);
		(void)tcpip_wrapper_regist_tcp_err(tcp_ctx, tcpip_tcp_err_cb);
		(void)tcpip_wrapper_regist_tcp_sent(tcp_ctx, tcpip_tcp_sent_cb);
		(void)tcpip_wrapper_regist_tcp_connected(tcp_ctx, tcpip_tcp_connected_cb);
		*socket_id_ptr = tcpip_wrapper_tcp_get_id_by_ctx(tcp_ctx);
	} else {
		udp_ctx = tcpip_wrapper_udp_ctx_new();
		if (udp_ctx == NULL) {
			return E_NOT_OK;
		}
		*socket_id_ptr = tcpip_wrapper_udp_get_id_by_ctx(udp_ctx);
		udp_ctx->socket_id = *socket_id_ptr;

		(void)tcpip_wrapper_regist_udp_recv(udp_ctx, tcpip_udp_recv_cb);
	}

	tcpip_socket_admin[*socket_id_ptr].protocol_type = protocol;
	tcpip_socket_admin[*socket_id_ptr].remote_addr.domain = domain;
	tcpip_socket_admin[*socket_id_ptr].socket_owner_id = socket_owner_id;

	return result;
}

FUNC(void, TCPIP_CODE) Core0_TcpIp_Mainfunction_5ms(void *arg)
{
	if (tcpip_initialized == TRUE) {
		tcpip_main_function();
	}
}

FUNC(sint32, TCPIP_CODE) tcpip_get_state(NetworkHandleType network_handle)
{
	(void)network_handle;
	sint32 ret = TCPIP_STATE_ONHOLD;

	if (tcpip_initialized == TRUE) {
		ret = TCPIP_STATE_ONLINE;
	}
	return ret;
}

FUNC(tcpip_ip_addr_state_type, TCPIP_CODE) tcpip_get_assignment_state_of_local_addr(uint8 addr_id)
{
	return tcpip_ip_addr_state[addr_id];
}

#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"