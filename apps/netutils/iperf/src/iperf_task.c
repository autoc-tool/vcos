/*
 * Copyright (C) 2022 理想汽车股份有限公司版权所有.
 *
 * Description:  TCPIP模块流量测试任务的源文件.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iperf.h"
#include "tcpip.h"
#include "tcpip_priv_types.h"

// #if (TCPIP_IPERF_ENABLED == STD_ON)
#include "tcpip_wrapper.h"
#define TCPIP_IPERF_MAX_CONN TCPIP_MAX_NOF_SOCKETS
#define TCPIP_IPERF_TESTBUF_LEN 160u

/** List of active iperf sessions */
#define ETH_START_SEC_PRIVATE_BSS
#include "eth_memmap.h"
static VAR(iperf_conn_info_t, ETH_PRIVATE_BSS) iperf_all_connections[TCPIP_IPERF_MAX_CONN];
static VAR(iperf_run_info_t, ETH_PRIVATE_BSS) g_iperf_run_info;
#define ETH_STOP_SEC_PRIVATE_BSS
#include "eth_memmap.h"

#define IPERF_DEBUG_RECEIVE     (1u<<0)
#define IPERF_DEBUG_SEND        (1u<<1)
#define IPERF_DEBUG_REPORT      (1u<<2)
#define DEFAULT_SERVER_PORT        (12346u)
#define IPERF_TCP               (1u)
#define IPERF_UDP               (2u)
#define IPERF_INVALID           (0u)
#define MAX_WIN_SIZE               (1024*1024)
#define IPERF_DEFAULT_PORT      (5001u)
#define IPERF_DEFAULT_UDP_RATE  (1024 * 1024)
#define IPERF_TEST_BUFFER_SIZE  (2048u)

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct
{
    uint32 Bytes;
    uint32 KBytes;
    uint32 MBytes;
    uint32 GBytes;
    uint32 times;
} count_t;

/******************************************************
 *               Function Declarations
 ******************************************************/
#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
static FUNC(count_t, TCPIP_CODE) iperf_calculate_result( sint32 pkt_size, count_t pkt_count, sint32 need_to_convert );
static FUNC(void, TCPIP_CODE) iperf_display_report( const iperf_conn_info_t *conn_info );
static FUNC(count_t, TCPIP_CODE) iperf_reset_count( count_t pkt_count );
static FUNC(count_t, TCPIP_CODE) iperf_copy_count( count_t pkt_count, count_t tmp_count );
static FUNC(count_t, TCPIP_CODE) iperf_diff_count( count_t pkt_count, count_t tmp_count );
static FUNC(sint32, TCPIP_CODE) iperf_format_transform( char *param );
static FUNC(void, TCPIP_CODE) iperf_generate_txbuf( uint8_t *dest, int32_t len );
#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"
 
#define TCPIP_START_SEC_CONST
#include "tcpip_memmap.h"

static CONST(u8_t, TCPIP_CONST) iperf_testbuf_const[TCPIP_IPERF_TESTBUF_LEN] = {
    /* coverity[misra_c_2012_rule_10_3_violation:SUPPRESS] */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    /* coverity[misra_c_2012_rule_10_3_violation:SUPPRESS] */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    /* coverity[misra_c_2012_rule_10_3_violation:SUPPRESS] */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};
#define TCPIP_STOP_SEC_CONST
#include "tcpip_memmap.h"

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
static inline FUNC_P2VAR(void, AUTOMATIC, TCPIP_CODE) iperf_buffer_get(uint16 size)
{
    return mem_malloc(size);
}

static inline FUNC(void, TCPIP_CODE) iperf_buffer_free(uint8* buffPtr)
{
    if(buffPtr != NULL) {
        mem_free(buffPtr);
    }else {
        return;
    }
}

static FUNC(void, TCPIP_CODE) iperf_ip_addr_32_to_8(uint32 src, uint8 *dest)
{
#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
    dest[0] = (uint8)((src & 0xff000000U) >> 24);
    dest[1] = (uint8)((src & 0x00ff0000U) >> 16);;
    dest[2] = (uint8)((src & 0x0000ff00U) >> 8);;
    dest[3] = (uint8)(src  & 0x000000ffU);
#else
    dest[3] = (uint8)((src & 0xff000000U) >> 24);
    dest[2] = (uint8)((src & 0x00ff0000U) >> 16);;
    dest[1] = (uint8)((src & 0x0000ff00U) >> 8);;
    dest[0] = (uint8)(src  & 0x000000ffU);
#endif
}

FUNC(void, TCPIP_CODE) iperf_rx_indication( tcpip_socket_id_type SocketId, const tcpip_sock_addr_type* RemoteAddrPtr, uint8* BufPtr, uint16 Length ) {
    (void)BufPtr;
    if(iperf_all_connections[SocketId].tcp == IPERF_INVALID) {
        return;
    }
    if(iperf_all_connections[SocketId].tcp == IPERF_TCP) {
        (void)tcpip_tcp_received(SocketId, Length);
    }else {
        if((iperf_all_connections[SocketId].bytes_recvd == 0u) && (iperf_all_connections[SocketId].server == 1u)) {
            iperf_all_connections[SocketId].time_started = sys_now();
        }else{
            /* Do nothing*/
        }
    }
    iperf_all_connections[SocketId].bytes_recvd += Length;
    iperf_all_connections[SocketId].pkt_recvd++;
    if(iperf_all_connections[SocketId].bytes_recvd >= iperf_all_connections[SocketId].settings.total_rcv) {
        iperf_display_report(&iperf_all_connections[SocketId]);
        iperf_all_connections[SocketId].start = 0;
        (void)IPERF_PRINTF("iperf_rx_indication: Socket %d close %ld port:%d\n", SocketId, iperf_all_connections[SocketId].bytes_recvd, RemoteAddrPtr->port);
        (void)tcpip_close(SocketId, TRUE);
        if(iperf_all_connections[SocketId].p_socket != 0xffffu) {
            (void)tcpip_close(iperf_all_connections[SocketId].p_socket,TRUE);
            (void)IPERF_PRINTF("iperf_rx_indication: Listen Socket %d closed\n", iperf_all_connections[SocketId].p_socket);
            (void)memset(&iperf_all_connections[iperf_all_connections[SocketId].p_socket], 0, sizeof(iperf_conn_info_t));
        }
        (void)memset(&iperf_all_connections[SocketId], 0, sizeof(iperf_conn_info_t));
    }
    return;
}

FUNC(BufReq_ReturnType, TCPIP_CODE) iperf_copy_tx_data( tcpip_socket_id_type SocketId, uint8* BufPtr, uint16 BufLength ){
    if(iperf_all_connections[SocketId].tcp == IPERF_INVALID) {
        return BUFREQ_E_NOT_OK;
    }
    iperf_generate_txbuf(BufPtr, (int32_t)BufLength);
    if(iperf_all_connections[SocketId].tcp == IPERF_UDP) {
        if((iperf_all_connections[SocketId].bytes_sent >= iperf_all_connections[SocketId].settings.total_send) ||\
            (iperf_all_connections[SocketId].pkt_sent >= (uint32_t)iperf_all_connections[SocketId].settings.send_time)) {
            struct udp_pcb *pcb;
            struct tcpip_wrapper_udp_ctx *udp_ctx = NULL;
            uint8 priority;
            iperf_display_report(&iperf_all_connections[SocketId]);
            udp_ctx = tcpip_wrapper_get_udp_ctx_by_id(SocketId);
            if(udp_ctx == NULL) {
                return BUFREQ_E_NOT_OK;
            }
            pcb = udp_ctx->conn;
            if (pcb->tos > 0u) {
                priority = pcb->tos - 1u;
                (void)IPERF_PRINTF("pkt priority:%d\n",priority);
            }
            (void)IPERF_PRINTF("pkt_sent:%ld,pkt_txconfirmed:%ld\n",iperf_all_connections[SocketId].pkt_sent,iperf_all_connections[SocketId].pkt_txconfirmed);
            iperf_all_connections[SocketId].start = 0;
            (void)tcpip_close(SocketId, TRUE);
            (void)memset(&iperf_all_connections[SocketId], 0, sizeof(iperf_conn_info_t));
        }
    }else {
        if (BufLength > 1460u) {
            (void)IPERF_PRINTF("iperf_copy_tx_data, bufferlen:%d over_flow\n", BufLength);
        }
        iperf_all_connections[SocketId].bytes_send += BufLength;
        iperf_all_connections[SocketId].pkt_sent++;
    }

    return BUFREQ_OK;
}

FUNC(void, TCPIP_CODE) iperf_tx_confirmation( tcpip_socket_id_type SocketId, uint16 Length ){
    if(iperf_all_connections[SocketId].tcp == IPERF_INVALID) {
        return;
    }
    if(iperf_all_connections[SocketId].tcp == IPERF_UDP) {
        iperf_all_connections[SocketId].pkt_txconfirmed++;
        return;
    } else {
        iperf_all_connections[SocketId].bytes_sent += Length;
        if(((iperf_all_connections[SocketId].bytes_sent >= iperf_all_connections[SocketId].settings.total_send) ||\
            ((iperf_all_connections[SocketId].pkt_sent >= (uint32_t)iperf_all_connections[SocketId].settings.send_time) && (iperf_all_connections[SocketId].start != 0u)))) {
            iperf_display_report(&iperf_all_connections[SocketId]);
            iperf_all_connections[SocketId].start = 0;
            (void)tcpip_close(SocketId, TRUE);
            (void)IPERF_PRINTF("Socket %d Closed at %ld, sent %ld, pkt count %ld\n", SocketId, sys_now(), iperf_all_connections[SocketId].bytes_sent, iperf_all_connections[SocketId].pkt_sent);
            (void)memset(&iperf_all_connections[SocketId], 0, sizeof(iperf_conn_info_t));
        }
        return;
    }
}

FUNC(Std_ReturnType, TCPIP_CODE) iperf_tcp_accepted( tcpip_socket_id_type SocketId, tcpip_socket_id_type SocketIdConnected, const tcpip_sock_addr_type* RemoteAddrPtr ){
    (void)RemoteAddrPtr;
    (void) memcpy(&iperf_all_connections[SocketIdConnected], &iperf_all_connections[SocketId], sizeof(iperf_conn_info_t));
    iperf_all_connections[SocketIdConnected].start = 1;
    iperf_all_connections[SocketIdConnected].p_socket = SocketId;
    iperf_all_connections[SocketIdConnected].time_started = sys_now();
    (void)IPERF_PRINTF("Socket %d accepted at %ld\n", SocketIdConnected, iperf_all_connections[SocketId].time_started);
    return 0;
}

FUNC(void, TCPIP_CODE) iperf_tcp_connected( tcpip_socket_id_type SocketId ){
    iperf_all_connections[SocketId].start = 1;
    iperf_all_connections[SocketId].time_started = sys_now();
    (void)IPERF_PRINTF("Socket %d connected at %ld\n", SocketId, iperf_all_connections[SocketId].time_started);
    return;
}

FUNC(void, TCPIP_CODE) iperf_tcpip_event( tcpip_socket_id_type SocketId, tcpip_event_type Event ){
    switch(Event) {
        case TCPIP_TCP_RESET:
        case TCPIP_TCP_CLOSED:
        case TCPIP_UDP_CLOSED:
            if(iperf_all_connections[SocketId].start != 0u) {
                iperf_display_report(&iperf_all_connections[SocketId]);
                (void)IPERF_PRINTF("iperf_tcpip_event: Socket %d closed\n", SocketId);
                if(iperf_all_connections[SocketId].p_socket != 0xffffu) {
                    iperf_all_connections[iperf_all_connections[SocketId].p_socket].start = 0;
                }
                (void)memset(&iperf_all_connections[SocketId], 0, sizeof(iperf_conn_info_t));
            }
            break;
        case TCPIP_TCP_FIN_RECEIVED:
            (void)tcpip_close(SocketId, FALSE);
            if(0u != iperf_all_connections[SocketId].start) {
                iperf_display_report(&iperf_all_connections[SocketId]);
                (void)IPERF_PRINTF("iperf_tcpip_event: Socket %d closed\n", SocketId);
                if(iperf_all_connections[SocketId].p_socket != 0xffffu) {
                    iperf_all_connections[iperf_all_connections[SocketId].p_socket].start = 0;
                }
                (void)memset(&iperf_all_connections[SocketId], 0, sizeof(iperf_conn_info_t));
            }
            break;
        default:
            /* Do nothing*/
            break;
    }
    return;
}

FUNC(void, TCPIP_CODE) iperf_local_ip_addr_assignment_chg( tcpip_local_addr_id_type IpAddrId, tcpip_ip_addr_state_type State ){
    (void) IpAddrId;
    (void) State;
    return;
}

static FUNC(void, TCPIP_CODE) iperf_udp_run_server( char *parameters[] )
{
    tcpip_socket_id_type SocketId;
    uint16_t server_port;
    sint32 i;

    uint32_t total_send = 0;
    volatile sint32 interval_time = 10;

    tcpip_local_addr_id_type localAddrId = IPERF_ADDR_ID;

    server_port = DEFAULT_SERVER_PORT;
    sint32 offset = (sint32)(IPERF_COMMAND_BUFFER_SIZE / sizeof(char *));

    for ( i = 0; i < 13; i++ ) {
        if ( strncmp( (char *) &parameters[i * offset], "-p", 2 ) == 0 ) {
            i++;
            server_port = (uint16)atoi( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-n", 2 ) == 0 ) {
            i++;
            total_send = (uint32_t)iperf_format_transform( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-i", 2 ) == 0 ) {
            #if 0
            #endif
            i++;
            if((interval_time = atoi((char*) &parameters[i * offset])) == 0) {
                i--;
            }
        } else if ( strncmp( (char *) &parameters[i * offset], "-l", 2 ) == 0 ) {
            i++;
            localAddrId = (tcpip_local_addr_id_type) atoi( (char *) &parameters[i * offset] );
        }else{
            /* Do nothing*/
        }
    }

    if(total_send == 0u) {
        total_send = 1024 * 1024 * 10;
    }
    if(E_OK != TcpIp_IperfGetSocket(TCPIP_AF_INET, TCPIP_IPPROTO_UDP, &SocketId)) {
        (void)IPERF_PRINTF("Get tcpip socket failed\n");
        return;
    }
    (void)memset(&iperf_all_connections[SocketId], 0,sizeof(iperf_conn_info_t));
    iperf_all_connections[SocketId].socket_id = (u8_t)SocketId;
    iperf_all_connections[SocketId].tcp = IPERF_UDP;
    iperf_all_connections[SocketId].server = 1;
    iperf_all_connections[SocketId].settings.total_rcv = total_send;

    if(E_OK != tcpip_bind(SocketId, localAddrId, &server_port)) {
        (void)IPERF_PRINTF("Tcpip bind failed\n");
        (void)tcpip_close(SocketId, TRUE);
        return;
    }
    iperf_all_connections[SocketId].p_socket = 0xffff;
    iperf_all_connections[SocketId].time_started = sys_now();
    iperf_all_connections[SocketId].start = 1;
    (void)IPERF_PRINTF("Udp server start %d\n", SocketId);
    return;
}

static FUNC(void, TCPIP_CODE) iperf_tcp_run_server( char *parameters[] )
{
    tcpip_socket_id_type SocketId;
    sint32 server_port;
    sint32 i;

    sint32 total_rcv = 0;
    volatile sint32 interval_time = 10;
    sint32 offset = (sint32)(IPERF_COMMAND_BUFFER_SIZE / sizeof(char *));
    tcpip_local_addr_id_type localAddrId = IPERF_ADDR_ID;

    server_port = DEFAULT_SERVER_PORT;

    /* Handle input parameters */
    for ( i = 0; i < 9; i++ ) {
        if ( strncmp( (char *) &parameters[i * offset], "-p", 2 ) == 0 ) {
            i++;
            server_port = atoi( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-n", 2 ) == 0 ) {
            i++;
            total_rcv = iperf_format_transform( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-l", 2 ) == 0 ) {
            i++;
            localAddrId = (tcpip_local_addr_id_type)atoi( (char *) &parameters[i * offset] );
        } else {
            if ( strncmp( (char *) &parameters[i * offset], "-i", 2 ) == 0 ) {
                i++;
                if((interval_time = (sint32)atoi((char*) &parameters[i * offset])) == 0) {
                    i--;
                }
            }
        }
    }

    if(total_rcv == 0) {
        total_rcv = 1024 * 1024 * 10;
    }
    if(E_OK != TcpIp_IperfGetSocket(TCPIP_AF_INET, TCPIP_IPPROTO_TCP, &SocketId)) {
        (void)IPERF_PRINTF("Get tcpip socket failed\n");
        return;
    }
    (void)memset(&iperf_all_connections[SocketId], 0,sizeof(iperf_conn_info_t));
    iperf_all_connections[SocketId].socket_id = (u8_t)SocketId;
    iperf_all_connections[SocketId].tcp = IPERF_TCP;
    iperf_all_connections[SocketId].server = 1;
    iperf_all_connections[SocketId].settings.total_rcv = (uint32_t)total_rcv;
    if(E_OK != tcpip_bind(SocketId, localAddrId, (uint16*)&server_port)) {
        (void)IPERF_PRINTF("Tcpip bind failed\n");
        (void)tcpip_close(SocketId, TRUE);
        return;
    }

    if(E_OK != tcpip_tcp_listen(SocketId, 2)) {
        (void)IPERF_PRINTF("Tcpip listen failed\n");
        (void)tcpip_close(SocketId, TRUE);
        return;
    }
    iperf_all_connections[SocketId].p_socket = 0xffff;
    iperf_all_connections[SocketId].start = 1;
    (void)IPERF_PRINTF("Tcp server start %d\n", SocketId);
    return;
}

static FUNC(void, TCPIP_CODE) iperf_tcp_run_client( char *parameters[] )
{
    char *Server_IP = NULL;
    sint32 total_send = 0;
    sint32 num_tag = 0;
    sint32 i;
    sint32 win_size, send_time, server_port, pkt_delay;
    sint32 offset = (sint32)(IPERF_COMMAND_BUFFER_SIZE / sizeof(char *));
    tcpip_socket_id_type SocketId;
    tcpip_sock_addr_type RemoteAddr;
    ip4_addr_t ip_addr;
    tcpip_local_addr_id_type localAddrId = IPERF_ADDR_ID;

    win_size = 0;
    send_time = 0;
    server_port = DEFAULT_SERVER_PORT;
    pkt_delay = 0;
    /* Handle input parameters */
    Server_IP = (char *) &parameters[0];
    (void)IPERF_PRINTF("Servr IP %s \n", Server_IP );
    for ( i = 1; i < 18; i++ ) {
        if ( strncmp( (char *) &parameters[i * offset], "-w", 2 ) == 0 ) {
            i++;
            win_size = iperf_format_transform( (char *) &parameters[i * offset] );
            if ( win_size > MAX_WIN_SIZE ) {
                win_size = MAX_WIN_SIZE;
            }
        }else if ( strncmp( (char *) &parameters[i * offset], "-t", 2 ) == 0 ) {
            i++;
            send_time = atoi( (char *) &parameters[i * offset] );
        }else if ( strncmp( (char *) &parameters[i * offset], "-p", 2 ) == 0 ) {
            i++;
            server_port = atoi( (char *) &parameters[i * offset] );
        }else if ( strncmp( (char *) &parameters[i * offset], "-d", 2 ) == 0 ) {
            i++;
            pkt_delay = atoi( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-n", 2 ) == 0 ) {
            i++;
            total_send = iperf_format_transform( (char *) &parameters[i * offset] );
            num_tag = 1;
        } else if ( strncmp( (char *) &parameters[i * offset], "-l", 2 ) == 0 ) {
            i++;
            localAddrId = (tcpip_local_addr_id_type)atoi( (char *) &parameters[i * offset] );
        }else{
            /* Do nothing */
        }
    }

    if ( win_size == 0 ) {
        win_size = 1460;
    }
    if ( send_time == 0 ) {
        if ( num_tag == 1 ) {
            send_time = 999999;
        }
        else {
            send_time = 10;
        }
    }

    if (total_send == 0) {
        total_send = 1024 * 1024 * 1024;
    }

    if (pkt_delay == 0) {
        pkt_delay = 1;
    }
    if(E_OK != TcpIp_IperfGetSocket(TCPIP_AF_INET, TCPIP_IPPROTO_TCP, &SocketId)) {
        (void)IPERF_PRINTF("Get tcpip socket failed\n");
        return;
    }

    if (localAddrId < 255u) {
        uint16 bind_port = 0;
        if(E_OK != tcpip_bind(SocketId, localAddrId, (uint16*)&bind_port)) {
            (void)IPERF_PRINTF("Tcpip bind failed\n");
            (void)tcpip_close(SocketId, TRUE);
            return;
        }
    }
    (void)memset(&iperf_all_connections[SocketId], 0,sizeof(iperf_conn_info_t));
    iperf_all_connections[SocketId].socket_id = (u8_t)SocketId;
    iperf_all_connections[SocketId].tcp = IPERF_TCP;
    iperf_all_connections[SocketId].server = 0;
    iperf_all_connections[SocketId].settings.win_size = (uint32_t)win_size;
    iperf_all_connections[SocketId].settings.pkt_delay = pkt_delay;
    iperf_all_connections[SocketId].settings.send_time = send_time;
    iperf_all_connections[SocketId].settings.total_rcv = 1024 * 1024 * 1024;
    iperf_all_connections[SocketId].settings.total_send = (uint32_t)total_send;
    iperf_all_connections[SocketId].p_socket = 0xffff;
    RemoteAddr.domain = TCPIP_AF_INET;
    RemoteAddr.port = lwip_htons((uint16)server_port);
    ip_addr.addr = inet_addr(Server_IP);
    iperf_ip_addr_32_to_8(ip_addr.addr, RemoteAddr.addr);
    if(E_OK != tcpip_tcp_connect(SocketId, &RemoteAddr)) {
        (void)IPERF_PRINTF("tcpip_tcp_connect failed\n");
    }
    iperf_all_connections[SocketId].time_started = sys_now();
    (void)IPERF_PRINTF("Tcp client start %d\n", SocketId);
    return;
}

static FUNC(void, TCPIP_CODE) iperf_udp_run_client( char *parameters[] )
{
    tcpip_socket_id_type SocketId;
    ip4_addr_t ip_addr;
    const char *Server_IP = NULL;
    uint32_t total_send = 0;
    sint32 num_tag = 0;
    sint32 interval_time = 10;
    sint32 i;
    sint32 data_size, send_time, server_port, pkt_delay;
    sint32 offset = (sint32)(IPERF_COMMAND_BUFFER_SIZE / sizeof(char *));
    tcpip_local_addr_id_type localAddrId = IPERF_ADDR_ID;

    data_size = 0;
    send_time = 0;
    server_port = DEFAULT_SERVER_PORT;
    pkt_delay = 0;

    Server_IP = (char *) &parameters[0];
    for ( i = 1; i < 18; i++ ) {
        if ( strncmp( (char *) &parameters[i * offset], "-w", 2 ) == 0 ) {
            i++;
            data_size = iperf_format_transform( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-t", 2 ) == 0 ) {
            i++;
            send_time = (sint32) atoi( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-p", 2 ) == 0 ) {
            i++;
            server_port = (sint32)atoi( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-d", 2 ) == 0 ) {
            i++;
            pkt_delay = (sint32)atoi( (char *) &parameters[i * offset] );
        } else if ( strncmp( (char *) &parameters[i * offset], "-n", 2 ) == 0 ) {
            i++;
            total_send = (uint32_t)iperf_format_transform( (char *) &parameters[i * offset] );
            num_tag = 1;
        } else if ( strncmp( (char *) &parameters[i * offset], "-i", 2 ) == 0 ) {
            i++;
            interval_time = (sint32)atoi((char*) &parameters[i * offset]);
            if(interval_time == 0) {
                i--;
            }
        } else if ( strncmp( (char *) &parameters[i * offset], "-l", 2 ) == 0 ) {
            i++;
            localAddrId = (tcpip_local_addr_id_type)atoi( (char *) &parameters[i * offset] );
        } else {
            /* Do nothing */
        }
    }

    if ( data_size == 0 ) {
        data_size = 1460;
    }
    if (pkt_delay == 0) {
        pkt_delay = 1;
    }

    if ( send_time == 0 ) {
        if ( num_tag == 1 ) {
            send_time = 999999;
        } else {
            send_time = 10;
        }
    }

    if (total_send == 0u) {
        total_send = 0xFFFFFFFFu;
    }

    if(E_OK != TcpIp_IperfGetSocket(TCPIP_AF_INET, TCPIP_IPPROTO_UDP, &SocketId)) {
        (void)IPERF_PRINTF("Get tcpip socket failed\n");
        return;
    }

    uint16 port = 0;
    if (localAddrId < 255u) {
        if(E_OK != tcpip_bind(SocketId, localAddrId, &port)) {
            (void)IPERF_PRINTF("TcpIp Bind failed\n");
            (void)tcpip_close(SocketId, TRUE);
            return;
        }
    }
    (void)memset(&iperf_all_connections[SocketId], 0,sizeof(iperf_conn_info_t));
    iperf_all_connections[SocketId].socket_id = (u8_t)SocketId;
    iperf_all_connections[SocketId].tcp = IPERF_UDP;
    iperf_all_connections[SocketId].server = 0;
    iperf_all_connections[SocketId].settings.win_size = (uint32_t)data_size;
    iperf_all_connections[SocketId].settings.pkt_delay = pkt_delay;
    iperf_all_connections[SocketId].settings.send_time = send_time;
    iperf_all_connections[SocketId].settings.total_send = total_send;
    iperf_all_connections[SocketId].settings.total_rcv = 1024 * 1024 * 1024;
    iperf_all_connections[SocketId].p_socket = 0xffff;
    iperf_all_connections[SocketId].settings.remote_addr.domain = TCPIP_AF_INET;
    iperf_all_connections[SocketId].settings.remote_addr.port = lwip_htons((uint16)server_port);
    ip_addr.addr = 0u;
    if(Server_IP != NULL) {
        ip_addr.addr = inet_addr(Server_IP);
    }
    iperf_ip_addr_32_to_8(ip_addr.addr, iperf_all_connections[SocketId].settings.remote_addr.addr);
    iperf_all_connections[SocketId].time_started = sys_now();
    iperf_all_connections[SocketId].start = 1;
    (void)IPERF_PRINTF("Udp client start %d\n", SocketId);

    return;
}

static FUNC(count_t, TCPIP_CODE) iperf_calculate_result( sint32 pkt_size, count_t pkt_count, sint32 need_to_convert )
{
    if ( pkt_size > 0 ) {
        pkt_count.Bytes += (uint32)pkt_size;
        pkt_count.times++;
    }

    if ( need_to_convert == 1 ) {
        if ( pkt_count.Bytes >= 1024u ) {
            pkt_count.KBytes += (pkt_count.Bytes / 1024u);
            pkt_count.Bytes = pkt_count.Bytes % 1024u;
        }

        if ( pkt_count.KBytes >= 1024u ) {
            pkt_count.MBytes += (pkt_count.KBytes / 1024u);
            pkt_count.KBytes = pkt_count.KBytes % 1024u;
        }

        if ( pkt_count.MBytes >= 1024u ) {
            pkt_count.GBytes += (pkt_count.MBytes / 1024u);
            pkt_count.MBytes = pkt_count.MBytes % 1024u;
        }
    }

    return pkt_count;
}

static FUNC(void, TCPIP_CODE) iperf_display_report(const iperf_conn_info_t *conn_info)
{
    uint32_t tmp_time;
    uint32_t time;
    uint32_t h_ms_time;
    count_t pkt_count;

    time = sys_now() - conn_info->time_started;
    h_ms_time = time % 1000u;
    time = time / 1000u;
    tmp_time = (time * 10u);
    (void)memset(&pkt_count, 0, sizeof(count_t));
    if(conn_info->server != 0u) {
        pkt_count.Bytes = conn_info->bytes_recvd;
        pkt_count.times = conn_info->pkt_recvd;
    }else{
        pkt_count.Bytes = conn_info->bytes_sent;
        pkt_count.times = conn_info->pkt_sent;
    }

    pkt_count = iperf_calculate_result( 0, pkt_count, 1 );

    (void)IPERF_PRINTF("\nTransfer in %ld.%ld seconds: ", time, h_ms_time);
    if (pkt_count.GBytes != 0u) {
        (void)IPERF_PRINTF("%ld GBytes ", pkt_count.GBytes);
    }

    if (pkt_count.MBytes != 0u) {
        (void)IPERF_PRINTF("%ld MBytes ", pkt_count.MBytes);
    }

    if (pkt_count.KBytes != 0u) {
        (void)IPERF_PRINTF("%ld KBytes ", pkt_count.KBytes);
    }

    (void)IPERF_PRINTF("iperf_display_report, time = %ld, h_ms_time = %ld, GBytes = %ld, MBytes = %ld, KBytes= %ld, Bytes= %ld, PktCount= %ld, ErrCount %ld\n", time, h_ms_time, pkt_count.GBytes, pkt_count.MBytes, pkt_count.KBytes, pkt_count.Bytes, pkt_count.times,conn_info->send_err);
    (void)IPERF_PRINTF("Socket id %d, Trans type %d, Run Type %d", conn_info->socket_id, conn_info->tcp, conn_info->server);

    if ( tmp_time != 0u ) {
        pkt_count.Bytes = ((((pkt_count.KBytes * 8u * 10u) % tmp_time) * 1024u) + (pkt_count.Bytes * 8u * 10u)) / tmp_time;
        pkt_count.KBytes = ((((pkt_count.MBytes * 8u * 10u) % tmp_time) * 1024u) + (pkt_count.KBytes * 8u * 10u)) / tmp_time;
        pkt_count.MBytes = ((((pkt_count.GBytes * 8u * 10u) % tmp_time) * 1024u) + (pkt_count.MBytes * 8u * 10u)) / tmp_time;
        pkt_count.GBytes = (pkt_count.GBytes * 8u * 10u) / tmp_time;
    } else {
        pkt_count.Bytes = 0u;
        pkt_count.KBytes = 0u;
        pkt_count.MBytes = 0u;
        pkt_count.GBytes = 0u;
    }

    pkt_count = iperf_calculate_result( 0, pkt_count, 1 );

    (void)IPERF_PRINTF("Bandwidth: %ld Gbits %ld Mbits %ld Kbits %ld bits/sec Receive times: %ld\n", pkt_count.GBytes, pkt_count.MBytes, pkt_count.KBytes, pkt_count.Bytes, pkt_count.times);
}

static FUNC(sint32, TCPIP_CODE) iperf_format_transform( char *param )
{
    char *temp;
    sint32 win_size = 0;
    sint32 i;

    temp = param;

    for ( i = 0; temp[i] != '\0'; i++ ) {
        if ( temp[i] == 'k' ) {
            temp[i] = '\0';
            win_size = (1000 * atoi( temp ));
        } else if ( temp[i] == 'm' ) {
            temp[i] = '\0';
            win_size = (1000 * 1000 * atoi( temp ));
        } else if ( temp[i] == 'K' ) {
            temp[i] = '\0';
            win_size = 1024 * atoi( temp );
        } else if ( temp[i] == 'M' ) {
            temp[i] = '\0';
            win_size = 1024 * 1024 * atoi( temp );
        } else {
            win_size = atoi( param );
        }
    }

    return win_size;
}

static FUNC(void, TCPIP_CODE) iperf_generate_txbuf( uint8_t *dest, int32_t len )
{
    uint32_t count= ((uint32_t)len) /TCPIP_IPERF_TESTBUF_LEN;
    uint32_t offset = 0;

    for (uint32 i = 0; i < count; i++ ) {
        (void)memcpy(dest+offset, iperf_testbuf_const, TCPIP_IPERF_TESTBUF_LEN);
        offset += TCPIP_IPERF_TESTBUF_LEN;

    }
    (void)memcpy(dest+offset, iperf_testbuf_const, (uint32_t)len % TCPIP_IPERF_TESTBUF_LEN);
}

FUNC(uint8_t, TCPIP_CODE) iperf_check_task(void){
    uint8_t ret = E_NOT_OK;
    if(g_iperf_run_info.start != 0u) {
        if(g_iperf_run_info.type == IPERF_CLIENT) {
            if(g_iperf_run_info.tcp == IPERF_TCP) {
                iperf_tcp_run_client(g_iperf_run_info.iperf_param);
            }else {
                iperf_udp_run_client(g_iperf_run_info.iperf_param);
            }
        }else {
            if(g_iperf_run_info.tcp == IPERF_TCP) {
                iperf_tcp_run_server(g_iperf_run_info.iperf_param);
            }else {
                iperf_udp_run_server(g_iperf_run_info.iperf_param);
            }
        }
        iperf_buffer_free((uint8*)g_iperf_run_info.iperf_param);
        g_iperf_run_info.start = 0;
        ret = E_OK;
    }
    return ret;
}

FUNC(uint8_t, TCPIP_CODE) iperf_main_function(void){
    uint32_t count = 0;
    uint8_t ret = E_NOT_OK;
    if(E_OK == iperf_check_task()) {
        return E_OK;
    }
    for(uint8_t i=0;i<TCPIP_IPERF_MAX_CONN;i++) {
        count = 0;
        if((iperf_all_connections[i].start != 0u) && (iperf_all_connections[i].tcp != IPERF_INVALID)) {
            if ((iperf_all_connections[i].send_err > (u32_t)iperf_all_connections[i].settings.send_time) || \
                ((iperf_all_connections[i].send_err * iperf_all_connections[i].settings.win_size) > iperf_all_connections[i].settings.total_send)) {
                continue;
            }

            if(iperf_all_connections[i].tcp == IPERF_TCP) {
                while(count < (uint32_t)iperf_all_connections[i].settings.pkt_delay){
                    ret = tcpip_tcp_transmit((uint16)i, NULL, iperf_all_connections[i].settings.win_size, TRUE);
                    if(ret != E_OK) {
                        sys_msleep( 1 );
                        iperf_all_connections[i].send_err++;
                    }
                    count++;
                }
            }else {
                while(count < (uint32_t)iperf_all_connections[i].settings.pkt_delay){
                    ret = tcpip_udp_transmit(i, NULL, &iperf_all_connections[i].settings.remote_addr, (uint16)iperf_all_connections[i].settings.win_size);
                    if(ret != E_OK) {
                        iperf_all_connections[i].send_err++;
                        sys_msleep( 1 );
                    }else {
                        iperf_all_connections[i].bytes_sent += iperf_all_connections[i].settings.win_size;
                        iperf_all_connections[i].pkt_sent++;
                    }
                    count++;
                }
            }
            ret = E_OK;
        }
    }

    return ret;
}

FUNC(void, TCPIP_CODE) iperf_cli_run_task(sint32 argc, char * const argv[], uint8_t type) {
    sint32 i;
    char **iperf_param = NULL;
    uint8_t tcp = IPERF_TCP;
    sint32 offset = (sint32)(IPERF_COMMAND_BUFFER_SIZE / sizeof(char *));
    iperf_param = iperf_buffer_get( IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE );
    if ( iperf_param == NULL ) {
        (void)IPERF_PRINTF("Warning: No enough memory to running iperf\n" );
        return;
    }
    (void)memset( iperf_param, 0, IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE );

    for ( i = 0; i < argc; i++ ) {
        if((i * offset) < (IPERF_COMMAND_BUFFER_NUM * IPERF_COMMAND_BUFFER_SIZE)) {
            (void)strncpy( (char *) &iperf_param[i * offset], argv[i] , IPERF_COMMAND_BUFFER_SIZE);
        }
    }

    for ( i = 0; i < argc; i++ ) {
        if ( strncmp( argv[i], "-u", 2 ) == 0 ) {
            tcp = IPERF_UDP;
            break;
        }
    }
    g_iperf_run_info.tcp = tcp;
    g_iperf_run_info.type = type;
    g_iperf_run_info.iperf_param = iperf_param;
    g_iperf_run_info.start = 1;
}

#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"

// #endif
