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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "tcpip_types.h"
#include "tcpip.h"
#include "syslog.h"



/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TRACE_IP_SIZE            4
#define TRACE_LOCAL_ADDR_ID      3
/****************************************************************************
 * Name: eth_udpdata_send
 *
 * Description:
 *   Clear all contents of the buffer
 *
 ****************************************************************************/
int eth_udpdata_send(const uint8_t *dataptr, uint16_t totallen)
{
	int ret;
	uint16_t server_port = CONFIG_TRACE_SERVER_PORT;
	const char server_ip[] = CONFIG_TRACE_SERVER_IP;
	uint16_t bind_port = 0;
	in_addr_t addr_ip4 = 0;
	tcpip_local_addr_id_type local_addr_id = TRACE_LOCAL_ADDR_ID;
	tcpip_socket_id_type socketid;
	tcpip_sock_addr_type remoteaddr;

	if (E_OK != tcpip_get_socket(0, TCPIP_AF_INET, TCPIP_IPPROTO_UDP, &socketid)) {
		return ERROR;
	}

	if (E_OK != tcpip_bind(socketid, local_addr_id, (uint16 *)&bind_port)) {
		(void)tcpip_close(socketid, TRUE);
		return ERROR;
	}

	if (dataptr == NULL || totallen == 0) {
		(void)tcpip_close(socketid, TRUE);
		return ERROR;
	}

	remoteaddr.domain = TCPIP_AF_INET;
	remoteaddr.port = htons(server_port);

	ret = inet_pton(AF_INET, server_ip, &addr_ip4);
	if (ret <= 0) {
		(void)tcpip_close(socketid, TRUE);
		return ERROR;
	}

	memcpy(remoteaddr.addr, &addr_ip4, TRACE_IP_SIZE);

	ret = tcpip_udp_transmit(socketid, dataptr,
	(const tcpip_sock_addr_type *)&remoteaddr, totallen);

	if (ret != E_OK) {
		(void)tcpip_close(socketid, TRUE);
		return ERROR;
	}

	(void)tcpip_close(socketid, TRUE);

	return ret;
}
