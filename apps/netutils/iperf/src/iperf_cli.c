/*
 * Copyright (C) 2022 理想汽车股份有限公司版权所有.
 *
 * Description:  TCPIP模块流量测试命令的源文件.
 */

#include <stdlib.h>
#include <string.h>
#include "iperf.h"

void iperf_cli_help_cmd(void)
{
    IPERF_PRINTF("Usage: iperf [-s|-c] [options]\n");
    IPERF_PRINTF("       iperf [-h]\n");
    IPERF_PRINTF("Client/Server:\n");
    IPERF_PRINTF("  -u,        use UDP rather than TCP\n");
    IPERF_PRINTF("  -p,        #server port to listen on/connect to (default 5001)\n");
    IPERF_PRINTF("  -n,        #[kmKM]    number of bytes to transmit\n");
    IPERF_PRINTF("Server specific:\n");
    IPERF_PRINTF("  -s,        run in server mode\n");
    IPERF_PRINTF("Client specific:\n");
    IPERF_PRINTF("  -c,        <ip>run in client mode, connect to <ip>\n");
    IPERF_PRINTF("  -w,        #[kmKM]    TCP window size\n");
    IPERF_PRINTF("  -t,        #pkt num to transmit\n");
    IPERF_PRINTF("  -d         #send num per period\n");
    IPERF_PRINTF("  -l,        local addr id\n");
    IPERF_PRINTF("  -h,        print this message and quit\n");
    IPERF_PRINTF("Command: \n");
    IPERF_PRINTF("Example:\n");
    IPERF_PRINTF("Iperf TCP Server: iperf -s -p <port> -n <total recv>\n");
    IPERF_PRINTF("Iperf UDP Server: iperf -s -p <port> -n <total recv> -u\n");
    IPERF_PRINTF("Iperf TCP Client: iperf -c <ip> -d <pkt num> -w <window size> -t <total pkt num> -n <total send> -p <port>\n");
    IPERF_PRINTF("Iperf UDP Client: iperf -c <ip> -u -d <pkt_num> -w <window size> -t <total pkt num> -n <total send> -p <port>\n");
}