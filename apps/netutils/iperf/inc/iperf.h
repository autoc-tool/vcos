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

#ifndef _IPERF_H_
#define _IPERF_H_
#include <nuttx/config.h>
#include <stdio.h>

#include "syslog.h"
#include "tcpip_types.h"
#include "tcpip_cfg.h"

typedef struct {
  uint8 type;
  uint8 tcp;
  uint8 start;
  char **iperf_param;
} iperf_run_info_t;

typedef struct {
  tcpip_sock_addr_type remote_addr;
  int32_t send_time;
  uint32 total_send;
  uint32 win_size;
  uint32 bandwidth;
  uint32 total_rcv;
  int32_t pkt_delay;
} iperf_conn_setting_t;

typedef struct {
  /* 1=tcp, 0=udp */
  uint8 start;
  uint8 tcp;
  /* 1=server, 0=client */
  uint8 server;
  uint8 socket_id;
  uint32 time_started;
  uint32 bytes_sent;
  uint32 pkt_sent;
  uint32 bytes_send;
  uint32 send_err;
  uint32 bytes_recvd;
  uint32 pkt_recvd;
  uint16 p_socket;
  uint32 pkt_txconfirmed;
  iperf_conn_setting_t settings;
}iperf_conn_info_t;

#define IPERF_COMMAND_BUFFER_NUM        (18u)
#define IPERF_COMMAND_BUFFER_SIZE       (20u)
#define IPERF_PRINTF(...)               syslog(LOG_NOTICE, __VA_ARGS__);
#define IPERF_CLIENT                    (0u)
#define IPERF_SERVER                    (1u)
#define IPERF_ADDR_ID                   (0u)

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
FUNC(void, TCPIP_CODE) iperf_cli_help_cmd(void);
FUNC(void, TCPIP_CODE) iperf_cli_run_task(sint32 argc, char * const argv[], uint8_t type);
FUNC(uint8_t, TCPIP_CODE) iperf_main_function(void);
FUNC(Std_ReturnType, TCPIP_CODE) iperf_tcp_accepted( tcpip_socket_id_type SocketId, tcpip_socket_id_type SocketIdConnected, const tcpip_sock_addr_type* RemoteAddrPtr );


FUNC(void, TCPIP_CODE) iperf_rx_indication( tcpip_socket_id_type SocketId, const tcpip_sock_addr_type* RemoteAddrPtr, uint8* BufPtr, uint16 Length );
FUNC(BufReq_ReturnType, TCPIP_CODE) iperf_copy_tx_data( tcpip_socket_id_type SocketId, uint8* BufPtr, uint16 BufLength );
FUNC(void, TCPIP_CODE) iperf_tx_confirmation( tcpip_socket_id_type SocketId, uint16 Length );
FUNC(Std_ReturnType, TCPIP_CODE) iperf_tcp_accepted( tcpip_socket_id_type SocketId, tcpip_socket_id_type SocketIdConnected, const tcpip_sock_addr_type* RemoteAddrPtr );
FUNC(void, TCPIP_CODE) iperf_tcp_connected( tcpip_socket_id_type SocketId );
FUNC(void, TCPIP_CODE) iperf_tcpip_event( tcpip_socket_id_type SocketId, tcpip_event_type Event );
FUNC(void, TCPIP_CODE) iperf_local_ip_addr_assignment_chg( tcpip_local_addr_id_type IpAddrId, tcpip_ip_addr_state_type State );
#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"

#endif