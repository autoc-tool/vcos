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

#ifndef TCPIP_TYPES_H
#define TCPIP_TYPES_H

#include "Platform_Types.h"
#include "comstack_types.h"
#include "tcpip_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TCPIP_SA_DATA_SIZE_IPV6 (20u) ///< IPv6地址的大小
#define TCPIP_SA_DATA_SIZE_IPV4 (4u) ///< IPv4地址的大小

#if (TCPIP_IPV6_ENABLED == STD_ON)
#define TCPIP_SA_DATA_SIZE TCPIP_SA_DATA_SIZE_IPV6 ///< IP地址的大小
#else
#define TCPIP_SA_DATA_SIZE TCPIP_SA_DATA_SIZE_IPV4 ///< IP地址的大小
#endif

/*!*********************************************************************************************************************
 * @brief    定义TCPIP协议栈配置
 * @swsid    SWS_TCPIP_00009
 **********************************************************************************************************************/
typedef uint16 tcpip_domain_type; ///< TcpIp地址族

#define TCPIP_AF_INET (0x02u) ///< IPv4地址族
#define TCPIP_AF_INET6 (0x1Cu) ///< IPv6地址族

/*!*********************************************************************************************************************
 * @brief    定义TCPIP协议类型
 * @swsid     SWS_TCPIP_00010
 **********************************************************************************************************************/
typedef enum {
	TCPIP_IPPROTO_TCP = 0x06, ///< TCP协议
	TCPIP_IPPROTO_UDP = 0x11, ///< UDP协议
	TCPIP_IPPROTO_INVALID = 0xFF, ///< 无效协议
} tcpip_protocol_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP套接字地址类型
 * @swsid    SWS_TCPIP_00012
 **********************************************************************************************************************/
typedef struct {
	tcpip_domain_type domain; ///< TcpIp地址族
	uint16 port; ///< 端口号
	uint8 addr[TCPIP_SA_DATA_SIZE]; ///< IP地址
} tcpip_sock_addr_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP套接字IPv4地址扩展类型
 * @swsid    SWS_TCPIP_00013
 **********************************************************************************************************************/
typedef struct {
	tcpip_domain_type domain; ///< TcpIp地址族
	uint16 port; ///< 端口号
	uint32 addr[1]; ///< IP地址
} tcpip_sock_addr_inet_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP套接字IPv6地址扩展类型
 * @swsid    SWS_TCPIP_00014
 **********************************************************************************************************************/
typedef struct {
	tcpip_domain_type domain; ///< TcpIp地址族
	uint16 port; ///< 端口号
	uint32 addr[4]; ///< IP地址
} tcpip_sock_addr_inet6_type;

/*!*********************************************************************************************************************
 * @brief    定义本地地址索引类型
 * @swsid    SWS_TCPIP_00135
 **********************************************************************************************************************/
typedef uint8 tcpip_local_addr_id_type; ///< 本地地址索引类型

/*!*********************************************************************************************************************
 * @brief    定义套接字序号类型
 * @swsid    SWS_TCPIP_00017
 **********************************************************************************************************************/
typedef uint16 tcpip_socket_id_type; ///< 套接字序号类型

#define TCPIP_SOCKETID_INVALID (0xFFFFu) ///< 无效套接字序号
#define TCPIP_INV_SOCKET_ID (TCPIP_SOCKETID_INVALID) ///< 无效套接字序号

#define TCPIP_P2V(ptrtype) P2VAR(ptrtype, AUTOMATIC, TCPIP_APPL_VAR) ///< 定义TcpIp类型申明
#define TCPIP_P2C(ptrtype) P2CONST(ptrtype, AUTOMATIC, TCPIP_APPL_DATA) ///< 定义TcpIp常量类型申明

/*!*********************************************************************************************************************
 * @brief    定义协议栈状态类型
 * @swsid    SWS_TCPIP_00073
 **********************************************************************************************************************/
typedef enum {
	TCPIP_STATE_ONLINE, ///< 协议栈处于在线状态
	TCPIP_STATE_ONHOLD, ///< 协议栈处于挂起状态
	TCPIP_STATE_OFFLINE, ///< 协议栈处于离线状态
	TCPIP_STATE_STARTUP, ///< 协议栈处于启动状态
	TCPIP_STATE_SHUTDOWN, ///< 协议栈处于关闭状态
} tcpip_state_type;

/*!*********************************************************************************************************************
 * @brief    定义协议栈套接字状态类型
 **********************************************************************************************************************/
typedef enum {
	TCPIP_TCP_SOCK_STATE_INVALID, ///< 无效套接字状态
	TCPIP_TCP_SOCK_STATE_LISTEN, ///< 套接字处于监听状态
	TCPIP_TCP_SOCK_STATE_SYNSENT, ///< 套接字处于SYN已发送状态
	TCPIP_TCP_SOCK_STATE_SYNRCVD, ///< 套接字处于SYN已接收状态
	TCPIP_TCP_SOCK_STATE_ESTABLISHED, ///< 套接字处于已建立状态
	TCPIP_TCP_SOCK_STATE_CLOSEWAIT, ///< 套接字处于关闭等待状态
	TCPIP_TCP_SOCK_STATE_LASTACK, ///< 套接字处于LASTACK状态
	TCPIP_TCP_SOCK_STATE_FINWAIT1, ///< 套接字处于FINWAIT1状态
	TCPIP_TCP_SOCK_STATE_FINWAIT2, ///< 套接字处于FINWAIT2状态
	TCPIP_TCP_SOCK_STATE_CLOSING, ///< 套接字处于CLOSING状态
	TCPIP_TCP_SOCK_STATE_TIMEWAIT, ///< 套接字处于TIMEWAIT状态
	TCPIP_TCP_SOCK_STATE_CLOSED ///< 套接字处于关闭状态
} tcpip_socket_state_type;

/*!*********************************************************************************************************************
 * @brief    定义IP地址状态类型
 * @swsid    SWS_TCPIP_00082
 **********************************************************************************************************************/
typedef enum {
	TCPIP_IPADDR_STATE_ASSIGNED, ///< IP地址已经分配
	TCPIP_IPADDR_STATE_ONHOLD, ///< IP地址处于挂起状态
	TCPIP_IPADDR_STATE_UNASSIGNED, ///< IP地址未分配
} tcpip_ip_addr_state_type;

/*!*********************************************************************************************************************
 * @brief    定义TcpIp事件类型
 * @swsid    SWS_TCPIP_00031
 **********************************************************************************************************************/
typedef enum {
	TCPIP_TCP_RESET = 0x01u, ///< TCP的RST事件
	TCPIP_TCP_CLOSED, ///< TCP的连接关闭事件
	TCPIP_TCP_FIN_RECEIVED, ///< 接收到TCP挥手报文事件
	TCPIP_UDP_CLOSED, ///< UDP的关闭事件
	TCPIP_TLS_HANDSHAKE_SUCCEEDED, ///< TLS握手成功事件
} tcpip_event_type;

/*!*********************************************************************************************************************
 * @brief    定义IP地址分配策略
 * @swsid    SWS_TCPIP_00065
 **********************************************************************************************************************/
typedef enum {
	TCPIP_IPADDR_ASSIGNMENT_STATIC, ///< 静态IP地址分配类型
	TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP, ///< 本地DOIP地址分配类型
	TCPIP_IPADDR_ASSIGNMENT_DHCP, ///< DHCP地址分配类型
	TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL, ///< 本地IP地址分配类型
	TCPIP_IPADDR_ASSIGNMENT_IPV6_ROUTER, ///< IPv6路由器地址分配类型
	TCPIP_IPADDR_ASSIGNMENT_ALL ///< 所有IP地址分配类型
} tcpip_ip_addr_assignment_type;

/*!*********************************************************************************************************************
 * @brief    定义TcpIp函数返回类型
 * @swsid    SWS_TCPIP_00066
 **********************************************************************************************************************/
typedef enum {
	TCPIP_OK, ///< 函数返回成功
	TCPIP_E_NOT_OK, ///< 函数返回失败
	TCPIP_E_PHYS_ADDR_MISS, ///< 物理地址缺失
} tcpip_return_type;

/*!*********************************************************************************************************************
 * @brief    定义TcpIp套接字参数类型
 * @swsid    SWS_TCPIP_00126
 **********************************************************************************************************************/
typedef enum {
	TCPIP_PARAMID_TCP_RXWND_MAX, ///< TCP最大接收窗口大小设置
	TCPIP_PARAMID_FRAMEPRIO, ///< 帧优先级设置
	TCPIP_PARAMID_TCP_NAGLE, ///< TCP的Nagle算法使能
	TCPIP_PARAMID_TCP_KEEPALIVE, ///< TCP的KeepAlive使能
	TCPIP_PARAMID_TTL, ///< 生存时间设置
	TCPIP_PARAMID_TCP_KEEPALIVE_TIME, ///< TCP的KeepAlive时间设置
	TCPIP_PARAMID_TCP_KEEPALIVE_PROBES_MAX, ///< TCP的KeepAlive探测次数设置
	TCPIP_PARAMID_TCP_KEEPALIVE_INTERVAL, ///< TCP的KeepAlive间隔设置
	TCPIP_PARAMID_TCP_OPTIONFILTER, ///< TCP的选项过滤使能
	TCPIP_PARAMID_PATHMTU_ENABLE, ///< 路径MTU探测使能
	TCPIP_PARAMID_FLOWLABEL, ///< 流量标签使能
	TCPIP_PARAMID_DSCP, ///< 服务类型（DSCP）设置
	TCPIP_PARAMID_UDP_CHECKSUM, ///< UDP校验和使能
	TCPIP_PARAMID_TLS_CONNECTION_ASSIGNMENT, ///< TLS连接分配使能
	TCPIP_PARAMID_TCP_SNDBUF_MAX = 0x7fu, ///< TCP发送缓冲区最大值设置
	TCPIP_PARAMID_VENDOR_SPECIFIC = 0x80u, ///< 厂商自定义参数
	TCPIP_PARAMID_UDP_TASK_UPLOAD ///< UDP通过任务或者中断上传设置
} tcpip_param_id_type;

/*!*********************************************************************************************************************
 * @brief    定义TcpIp测量数据类型
 * @swsid    SWS_TCPIP_91010
 **********************************************************************************************************************/
typedef enum {
	TCPIP_MEAS_DROP_TCP = 0x01, ///< TCP数据丢失的数量
	TCPIP_MEAS_DROP_UDP = 0x02, ///< UDP数据丢失的数量
	TCPIP_MEAS_DROP_IPV4 = 0x03, ///< IPv4数据丢失的数量
	TCPIP_MEAS_DROP_IPV6 = 0x04, ///< IPv6数据丢失的数量
	TCPIP_MEAS_ALL = 0xFF, ///< 数据类型最大值
} tcpip_measurement_idx_type;

#define TCPIP_IPADDR_ANY 0x00u ///< 任意IPv4地址类型
#define TCPIP_IP6ADDR_ANY 0x00u ///< 任意IPv6地址类型
#define TCPIP_PORT_ANY 0x00u ///< 任意端口类型
#define TCPIP_LOCALADDRID_ANY 0x00u ///< 任意地址类型

/*!*********************************************************************************************************************
 * @brief    定义TcpIp地址类型
 **********************************************************************************************************************/
typedef enum {
	TCPIP_MULTICAST, ///< 组播地址类型
	TCPIP_UNICAST, ///< 单播地址类型
	TCPIP_ANYCAST, ///< 任意地址类型
} tcpip_address_type_type;

/*!*********************************************************************************************************************
 * @brief    定义动态地址分配存活时间类型
 **********************************************************************************************************************/
typedef enum {
	TCPIP_FORGET, ///< 在动态IP地址分配之后，仅在此次连接期间有效
	TCPIP_STORE, ///< 在动态IP地址分配之后，地址永久有效
} tcpip_assignment_lifetime_type;

/*!*********************************************************************************************************************
 * @brief    定义地址分配触发类型
 **********************************************************************************************************************/
typedef enum {
	TCPIP_AUTOMATIC, ///< 协议栈自动触发地址分配
	TCPIP_MANUAL, ///< 调用函数触发地址分配
} tcpip_assignment_trigger_type;

/*!*********************************************************************************************************************
 * @brief    定义接收数据通知上层回调函数指针类型
 **********************************************************************************************************************/
typedef void (*tcpip_up_rx_indication)(tcpip_socket_id_type socket_id, const tcpip_sock_addr_type *remote_addr_ptr,
				      uint8 *buf_ptr, uint16 length);

/*!*********************************************************************************************************************
 * @brief    定义协议栈事件通知上层回调函数指针类型
 **********************************************************************************************************************/
typedef void (*tcpip_up_tcpip_event)(tcpip_socket_id_type socket_id, tcpip_event_type event);

/*!*********************************************************************************************************************
 * @brief    定义发送成功通知上层回调函数指针类型
 **********************************************************************************************************************/
typedef void (*tcpip_up_tx_confirmation)(tcpip_socket_id_type socket_id, uint16 length);

/*!*********************************************************************************************************************
 * @brief    定义被连接成功通知上层回调函数指针类型
 **********************************************************************************************************************/
typedef Std_ReturnType (*tcpip_up_tcp_accepted)(tcpip_socket_id_type socket_id, tcpip_socket_id_type soecket_id_connected,
					       const tcpip_sock_addr_type *remote_addr_ptr);

/*!*********************************************************************************************************************
 * @brief    定义主动连接成功通知上层回调函数指针类型
 **********************************************************************************************************************/
typedef void (*tcpip_up_tcp_connected)(tcpip_socket_id_type socket_id);

/*!*********************************************************************************************************************
 * @brief    定义上层拷贝发送数据回调函数指针类型
 **********************************************************************************************************************/
typedef BufReq_ReturnType (*tcpip_up_copy_tx_data)(tcpip_socket_id_type socket_id, uint8 *buf_ptr, uint16 buf_length);

/*!*********************************************************************************************************************
 * @brief    定义IP地址分配成功通知上层回调函数指针类型
 **********************************************************************************************************************/
typedef void (*tcpip_up_local_ip_addr_assignment_chg)(tcpip_local_addr_id_type ip_addr_id, tcpip_ip_addr_state_type state);

/*!*********************************************************************************************************************
 * @brief    定义ARP缓存数据结构
 * @swsid    SWS_TCPIP_91004
 **********************************************************************************************************************/
typedef struct {
	uint32 inet_addr[1]; ///< IP地址
	uint8 phys_addr[6]; ///< 物理地址
	uint8 state; ///< ARP表状态
} tcpip_arp_cache_entry_type;

/*!*********************************************************************************************************************
 * @brief    定义地址分配数据结构
 **********************************************************************************************************************/
typedef struct {
	const tcpip_assignment_lifetime_type assignment_lifetime; ///< 动态地址分配存活时间
	const tcpip_ip_addr_assignment_type assignment_method; ///< 动态地址分配策略
	const uint8 assignment_priority; ///< 动态地址分配优先级
	const tcpip_assignment_trigger_type assignment_trigger; ///< 动态地址分配触发类型
} tcpip_address_assignment_type;

/*!*********************************************************************************************************************
 * @brief    定义静态IP地址类型
 **********************************************************************************************************************/
typedef struct {
	uint32 static_ip_address; ///< 静态IP地址
	uint32 default_router; ///< 默认路由器
	uint8 netmask; ///< 子网掩码
} tcpip_static_ip_address_config_type;

/*!*********************************************************************************************************************
 * @brief    定义本地地址类型
 **********************************************************************************************************************/
typedef struct {
	const tcpip_local_addr_id_type address_id; ///< 本地地址索引
	const tcpip_address_type_type address_type; ///< 本地地址类型
	const tcpip_domain_type domain; ///< 本地地址族
	const uint8 tcpip_ctrl_ref; ///< 控制器引用
	const tcpip_address_assignment_type *address_assignment; ///< 地址分配配置
	tcpip_static_ip_address_config_type *static_ip_addr_config; ///< 静态IP地址配置
} tcpip_local_addr_type;

/*!*********************************************************************************************************************
 * @brief    定义协议栈套接字管理的信息
 **********************************************************************************************************************/
typedef struct {
	tcpip_protocol_type protocol_type; ///< 协议类型
	uint8 socket_owner_id; ///< 套接字所有者ID
	const tcpip_local_addr_type *ptr_local_addr; ///< 本地地址相关信息
	tcpip_sock_addr_type remote_addr; ///< 远程地址信息
} tcpip_socket_admin_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP上层模块的类型
 **********************************************************************************************************************/
typedef uint8 tcpip_socket_owner_upper_layer_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP的IP地址类型
 **********************************************************************************************************************/
typedef struct {
	uint32 addr; ///< IP地址
} tcpip_ip_addr_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP的MAC地址类型
 **********************************************************************************************************************/
typedef struct {
	uint8 addr[6]; ///< MAC地址
} tcpip_eth_addr_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP的MAC地址类型
 **********************************************************************************************************************/
typedef struct {
	tcpip_ip_addr_type ipaddr; ///< IP地址
	tcpip_eth_addr_type ethaddr; ///< MAC地址
} tcpip_arp_table_entry_type;

/*!*********************************************************************************************************************
 * @brief    定义ARP配置数据类型
 **********************************************************************************************************************/
typedef struct {
	const uint8 arp_num_gratuitous_arp_on_startup; ///< 启动时发送免费ARP的数量
	const boolean arp_packet_queue_enabled; ///< 是否启用ARP数据包队列
	const uint16 arp_table_size_max; ///< ARP表的最大大小
	const uint32 arp_request_timeout; ///< ARP请求超时时间
	const uint32 arp_table_entry_timeout; ///< ARP表项超时时间
	const uint32 static_arp_table_len; ///< 静态ARP表的长度
	const tcpip_arp_table_entry_type *static_arp_table; ///< 静态ARP表
} tcpip_arp_config_type;

/*!*********************************************************************************************************************
 * @brief    定义ICMP配置数据类型
 **********************************************************************************************************************/
typedef struct {
	const uint8 icmp_ttl; ///< ICMP报文生存时间
} tcpip_icmp_config_type;

/*!*********************************************************************************************************************
 * @brief    定义IP分片配置数据类型
 **********************************************************************************************************************/
typedef struct {
	const boolean fragmentation_rx_enabled; ///< 是否启用接收分片
	const uint8 num_fragments; ///< 分片的最大数量
	const uint16 num_reass_dgrams; ///< 重组数据报文的最大数量
	const uint32 reass_timeout; ///< 重组超时时间
	const uint16 arc_max_receive_buffer_size; ///< 接收缓冲区的大小
} tcpip_frag_config_type;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP控制数据类型
 **********************************************************************************************************************/
typedef struct {
	const uint8 frame_prio_default; ///< 默认帧优先级
	const uint8 ethif_ctrl_ref; ///< EthIf控制器
} tcpip_tcpip_ctrl_type;

/*!*********************************************************************************************************************
 * @brief    定义IPv4配置数据类型
 **********************************************************************************************************************/
typedef struct {
	const tcpip_arp_config_type *arp_config_list; ///< ARP配置列表
	const tcpip_icmp_config_type *icmp_config; ///< ICMP配置
} tcpip_ipv4_config_type;

/*!*********************************************************************************************************************
 * @brief    定义配置数据类型
 **********************************************************************************************************************/
typedef struct {
	const tcpip_ipv4_config_type *ipv4_config; ///< IPv4配置
} tcpip_ip_config_type;

/*!*********************************************************************************************************************
 * @brief    定义TcpIp套接字所属模块类型
 **********************************************************************************************************************/
typedef struct {
	const tcpip_up_copy_tx_data socket_owner_copy_tx_data_fnc_ptr;
	const tcpip_up_local_ip_addr_assignment_chg socket_owner_local_ip_addr_assignment_chg_fnc_ptr;
	const tcpip_up_rx_indication socket_owner_rx_indication_fnc_ptr;
	const tcpip_up_tcp_accepted socket_owner_tcp_accepted_fnc_ptr;
	const tcpip_up_tcp_connected socket_owner_tcp_connected_fnc_ptr;
	const tcpip_up_tcpip_event socket_owner_tcpip_event_fnc_ptr;
	const tcpip_up_tx_confirmation socket_owner_tx_confirmation_fnc_ptr;
	const tcpip_socket_owner_upper_layer_type socket_owner_upper_layer_type;
} tcpip_socket_owner_type;

/*!*********************************************************************************************************************
 * @brief    定义TcpIp套接字所属模块配置
 **********************************************************************************************************************/
typedef struct {
	const tcpip_socket_owner_type *socket_owner_list; ///< 套接字所属模块列表
} tcpip_socket_owner_config_type;

/*!*********************************************************************************************************************
 * @brief    定义TCP协议配置
 **********************************************************************************************************************/
typedef struct {
	const boolean tcp_congestion_avoidance_enabled; ///< 拥塞避免启用标志
	const boolean tcp_fast_recovery_enabled; ///< 快速恢复启用标志
	const boolean tcp_fast_retransmit_enabled; ///< 快速重传启用标志
	const uint32 tcp_fin_wait_2_timeout; ///< FIN_WAIT_2超时时间
	const boolean tcp_keep_alive_enabled; ///< 保持连接启用标志
	const uint32 tcp_keep_alive_interval; ///< 保持连接间隔
	const uint16 tcp_keep_alive_probes_max; ///< 保持连接最大探测次数
	const uint32 tcp_keep_alive_time; ///< 保持连接超时时间
	const uint8 *tcp_max_rtx; ///< TCP最大重传次数
	const uint32 tcp_msl; ///< MSL（最大报文段寿命）
	const boolean tcp_nagle_enabled; ///< Nagle算法启用标志
	const uint16 tcp_receive_windows_max; ///< TCP接收窗口最大值
	const uint32 *tcp_retransmission_timeout; ///< TCP重传超时时间
	const boolean tcp_slow_start_enabled; ///< 慢启动启用标志
	const uint8 tcp_syn_max_rtx; ///< TCP SYN最大重传次数
	const uint32 tcp_syn_received_timeout; ///< TCP SYN接收超时时间
	const uint8 tcp_ttl; ///< TCP报文生存时间
} tcpip_tcp_config;

/*!*********************************************************************************************************************
 * @brief    定义UDP协议配置
 **********************************************************************************************************************/
typedef struct {
	const uint8 udp_ttl; ///< UDP报文生存时间
} tcpip_udp_config;

/*!*********************************************************************************************************************
 * @brief    定义TCPIP协议栈配置
 **********************************************************************************************************************/
typedef struct {
	const tcpip_tcpip_ctrl_type *ctrl_list; ///< 控制模块列表
	const tcpip_ip_config_type *ip_config; ///< IP配置
	const tcpip_local_addr_type *local_addr_list; ///< 本地地址列表
	const tcpip_socket_owner_config_type socket_owner_config; ///< 套接字所属模块配置
} tcpip_tcpip_config_type;

/*!*********************************************************************************************************************
 * @brief   定义TcpIp配置信息类型
 * @swsid   SWS_TCPIP_00067
 **********************************************************************************************************************/
typedef struct {
	const tcpip_tcpip_config_type config; ///< TCPIP协议栈配置
} tcpip_config_type;

#ifdef __cplusplus
}
#endif

#endif
