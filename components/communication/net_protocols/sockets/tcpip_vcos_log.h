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

#ifndef TCPIP_VCOS_LOG_H
#define TCPIP_VCOS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define TCPIP_DLT_CHECK_ID0 "[TcpIp] Recv ZeroWind(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID1 "[TcpIp] RecvRst(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID2 "[TcpIp] RecvFin(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID3 "[TcpIp] SendFin(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID4 "[TcpIp] SendRst(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID5 "[TcpIp] Send ZeroWind(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID6 "[TcpIp] Tcp retrans(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID7 "[TcpIp] connected(%u,%u|%u,%u)\r\n"
#define TCPIP_DLT_CHECK_ID8 "[TcpIp] accepted(%u,%u|%u,%u)\r\n"

#define TCPIP_DLT_CHECK_ID9 "[TcpIp] MemPool usage:%d/%d\r\n"

#define TCPIP_DLT_CHECK_ID10 "[TcpIp] MemPool len %d allocation failed\r\n"
#define TCPIP_DLT_CHECK_ID11 "[TcpIp] tcp transmit failed, ret:%d\r\n"
#define TCPIP_DLT_CHECK_ID12 "[TcpIp] udp transmit failed, ret:%d\r\n"
#define TCPIP_DLT_CHECK_ID13 "[TcpIp] port:%d bind failed\r\n"

#ifdef __cplusplus
}
#endif

#endif /* TCPIP_VCOS_LOG_H */
