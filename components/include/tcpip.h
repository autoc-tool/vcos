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
/*!*********************************************************************************************************************
 * @file        TcpIp.h
 * @brief       TcpIp头文件
 * @details     主要包含TcpIp功能相关的API、宏定义等
 * @date        2023-3-17
 * @version     V1.0
 **********************************************************************************************************************/
#ifndef TCPIP_H
#define TCPIP_H

#include "comstack_types.h"
#include "tcpip_cbk.h"
#include "tcpip_lcfg.h"
#include "Eth_GeneralTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TCPIP_MODULE_ID (170u) ///< 模块ID

#define TCPIP_AR_RELEASE_MAJOR_VERSION (4u) ///< Autosar主版本号
#define TCPIP_AR_RELEASE_MINOR_VERSION (2u) ///< Autosar次版本号
#define TCPIP_AR_RELEASE_PATCH_VERSION (2u) ///< Autosar修订版本号

#define TCPIP_SW_MAJOR_VERSION TCPIP_AR_RELEASE_MAJOR_VERSION ///< Autosar主版本号
#define TCPIP_SW_MINOR_VERSION TCPIP_AR_RELEASE_MINOR_VERSION ///< Autosar次版本号
#define TCPIP_SW_PATCH_VERSION TCPIP_AR_RELEASE_PATCH_VERSION ///< Autosar修订版本号

/* Error IDs  */
#define TCPIP_E_UNINIT (0x01u) ///< (DET) 未初始化
#define TCPIP_E_PARAM_POINTER (0x02u) ///< (DET) 无效指针
#define TCPIP_E_INV_ARG (0x04u) ///< (DET) 无效参数
#define TCPIP_E_MSGSIZE (0x08u) ///< (DET) 消息大小错误
#define TCPIP_E_ADDRINUSE (0x09u) ///< (DET) 地址已被占用
#define TCPIP_E_ADDRNOTAVAIL (0x0Au) ///< (DET) 地址不可用
#define TCPIP_E_ISCONN (0x0Bu) ///< (DET) 连接已建立
#define TCPIP_E_NOTCONN (0x0Cu) ///< (DET) 连接未建立
#define TCPIP_E_NOPROTOOPT (0x0Du) ///< (DET) 无协议选项
#define TCPIP_E_AFNOSUPPORT (0x0Eu) ///< (DET) 不支持的地址族
#define TCPIP_E_INIT_FAILED (0x0Fu) ///< (DET) 初始化失败

/* Service IDs */
#define TCPIP_INIT_SERVICE_ID (0x01u) ///< (Service ID) TcpIp_Init
#define TCPIP_GETVERSIONINFO_SERVICE_ID (0x02u) ///< (Service ID) TcpIp_GetVersionInfo
#define TCPIP_GETSOCKET_SERVICE_ID (0x03u) ///< (Service ID) TcpIp_GetSocket
#define TCPIP_CLOSE_SERVICE_ID (0x04u) ///< (Service ID) TcpIp_Close
#define TCPIP_BIND_SERVICE_ID (0x05u) ///< (Service ID) TcpIp_Bind
#define TCPIP_TCPCONNECT_SERVICE_ID (0x06u) ///< (Service ID) TcpIp_TcpConnect
#define TCPIP_TCPLISTEN_SERVICE_ID (0x07u) ///< (Service ID) TcpIp_TcpListen
#define TCPIP_TCPRECEIVED_SERVICE_ID (0x08u) ///< (Service ID) TcpIp_TcpReceived
#define TCPIP_REQUESTCOMMODE_SERVICE_ID (0x09u) ///< (Service ID) TcpIp_RequestCommMode
#define TCPIP_REQUESTIPADDRASSIGNMENT_SERVICE_ID (0x0au) ///< (Service ID) TcpIp_RequestIpAddrAssignment
#define TCPIP_RELEASEIPADDRASSIGNMENT_SERVICE_ID (0x0bu) ///< (Service ID) TcpIp_ReleaseIpAddrAssignment
#define TCPIP_ICMPTRANSMIT_SERVICE_ID (0x0cu) ///< (Service ID) TcpIp_IcmpTransmit
#define TCPIP_DHCPREADOPTION_SERVICE_ID (0x0du) ///< (Service ID) TcpIp_DhcpReadOption
#define TCPIP_DHCPWRITEOPTION_SERVICE_ID (0x0eu) ///< (Service ID) TcpIp_DhcpWriteOption
#define TCPIP_CHANGEPARAMETER_SERVICE_ID (0x0fu) ///< (Service ID) TcpIp_ChangeParameter
#define TCPIP_GETIPADDR_SERVICE_ID (0x10u) ///< (Service ID) TcpIp_GetIpAddr
#define TCPIP_GETPHYSADDR_SERVICE_ID (0x11u) ///< (Service ID) TcpIp_GetPhyAddr
#define TCPIP_UDPTRANSMIT_SERVICE_ID (0x12u) ///< (Service ID) TcpIp_UdpTransmit
#define TCPIP_TCPTRANSMIT_SERVICE_ID (0x13u) ///< (Service ID) TcpIp_TcpTransmit
#define TCPIP_RXINDICATION_SERVICE_ID (0x14u) ///< (Service ID) TcpIp_RxIndication
#define TCPIP_MAINFUNCTION_SERVICE_ID (0x15u) ///< (Service ID) TcpIp_MainFunction
#define TCPIP_GETREMOTEPHYSADDR_SERVICE_ID (0x16u) ///< (Service ID) TcpIp_GetRemotePhyAddr
#define TCPIP_GETCTRLIDX_SERVICE_ID (0x17u) ///< (Service ID) TcpIp_GetCtrlIdx
#define TCPIP_ICMPV6TRANSMIT_SERVICE_ID (0x18u) ///< (Service ID) TcpIp_IcmpV6Transmit
#define TCPIP_DHCPV6READOPTION_SERVICE_ID (0x19u) ///< (Service ID) TcpIp_DhcpV6ReadOption
#define TCPIP_DHCPV6WRITEOPTION_SERVICE_ID (0x1au) ///< (Service ID) TcpIp_DhcpV6WriteOption
#define TCPIP_RESETIPASSIGNMENT_SERVICE_ID (0x1bu) ///< (Service ID) TcpIp_ResetIpAssignment
#define TCPIP_GETARPCACHEENTRIES_SERVICE_ID (0x1cu) ///< (Service ID) TcpIp_GetArpCacheEntries
#define TCPIP_SETIPADDR_SERVICE_ID (0x1du) ///< (Service ID) TcpIp_SetIpAddr
#define TCPIP_GETANDRESETMEASUREMENTDATA_SERVICE_ID (0x1eu) ///< (Service ID) TcpIp_GetAndResetMeasurementData

#define TCPIP_ERR_OK (0x00u) ///< 接口返回成功
#define TCPIP_ERR_INVPARAM (0x01u) ///< 函数传入参数无效
#define TCPIP_ERR_CONN (0x02u) ///< socket处于未绑定或者未连接成功的状态
#define TCPIP_ERR_MEM (0x03u) ///< 内存池申请失败
#define TCPIP_ERR_ARPQUERY (0x04u) ///< 查询ARP返回失败，未匹配或申请到合适的ARP表项
#define TCPIP_ERR_OVERLEN (0x05u) ///< 未使能IP分片功能的情况下，发送过长UDP报文
#define TCPIP_ERR_BUFREQ (0x06u) ///< 申请底层驱动buffer失败
#define TCPIP_ERR_COPYTXDATA (0x07u) ///< 调用上层接口拷贝数据返回失败
#define TCPIP_ERR_TRANSDATA (0x08u) ///< 调用EthIf接口发送UDP报文返回失败
#define TCPIP_ERR_SNDBUF (0x09u) ///< TCP的发送缓存耗尽
#define TCPIP_ERR_SNDQUEUE (0x0au) ///< 发送缓存报文的最大条目数超最大值
#define TCPIP_ERR_ARPFULL (0x0bu) ///< ARP表项存储的报文满了，返回失败
#define TCPIP_ERR_GATWAT (0x0cu) ///< 跨网段发送未匹配到网关地址
#define TCPIP_ERR_OFFLINE (0x0du) ///< 套接字处于offline的状态
#define TCPIP_ERR_RESERVE (0xffu) ///< 保留的错误码

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
/*!*********************************************************************************************************************
 * @brief          初始化TcpIp
 * @details        获取TcpIp的配置信息,初始化TcpIp相关结构体
 * @param[in]      config_ptr   TcpIp配置信息结构体指针
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00002
 **********************************************************************************************************************/
FUNC(void, TCPIP_CODE) bsw_tcpip_init(const tcpip_config_type *config_ptr);

/*!*********************************************************************************************************************
 * @brief          关闭一个已有的TcpIp套接字
 * @details        协议栈关闭套接字并释放套接字资源
 * @param[in]      socket_id    套接字的索引
 * @param[in]      abort       是否通过发送RST报文强制关闭套接字
 * @retval         E_OK     关闭套接字成功
 * @retval         E_NOT_OK 关闭套接字失败,可能的原因是套接字不存在或者套接字已经被关闭
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00017
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_close(tcpip_socket_id_type socket_id, boolean abort);

/*!*********************************************************************************************************************
 * @brief          绑定一个TcpIp套接字
 * @details        协议栈绑定一个TcpIp套接字,并申请套接字资源
 * @param[in]      socket_id       套接字的索引
 * @param[in]      local_addr_id    本地地址索引
 * @param[in]      port_ptr        端口号指针
 * @retval         E_OK     绑定套接字成功
 * @retval         E_NOT_OK 绑定套接字失败,可能的原因是套接字不存在或者套接字已经被绑定
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00015
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_bind(tcpip_socket_id_type socket_id,
	tcpip_local_addr_id_type local_addr_id, const uint16 *port_ptr);

/*!*********************************************************************************************************************
 * @brief          连接一个远程TCP服务器
 * @details        协议栈通过一个已有套接字连接远程的TCP服务器
 * @param[in]      socket_id        套接字的索引
 * @param[in]      remote_addr_ptr   远程地址指针
 * @retval         E_OK     连接成功
 * @retval         E_NOT_OK 连接失败,可能的原因是套接字不存在或者套接字已经被连接
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    异步
 * @swsid          SWS_TCPIP_00022
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_connect(tcpip_socket_id_type socket_id, const tcpip_sock_addr_type *remote_addr_ptr);

/*!*********************************************************************************************************************
 * @brief          监听一个TCP套接字
 * @details        协议栈监听一个TCP套接字,等待TCP客户端连接
 * @param[in]      socket_id       套接字的索引
 * @param[in]      max_channels    最大连接数
 * @retval         E_OK     监听成功
 * @retval         E_NOT_OK 监听失败
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00023
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_listen(tcpip_socket_id_type socket_id, uint16 max_channels);

/*!*********************************************************************************************************************
 * @brief          通知协议栈数据已经收到
 * @details        上层应用通知协议栈数据已经收到,协议栈将根据套接字类型进行相应的处理
 * @param[in]      socket_id       套接字的索引
 * @param[in]      length         数据长度
 * @retval         E_OK     协议栈通知成功
 * @retval         E_NOT_OK 协议栈通知失败
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00024
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_received(tcpip_socket_id_type socket_id, uint32 length);

/*!*********************************************************************************************************************
 * @brief          修改套接字参数
 * @details        修改一个已有协议栈套接字的属性
 * @param[in]      socket_id             套接字的索引
 * @param[in]      parameter_id          参数类型的索引
 * @param[in]      parameter_value       参数的值指针
 * @retval         E_OK     修改参数成功
 * @retval         E_NOT_OK 修改参数失败
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00016
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_change_parameter(tcpip_socket_id_type socket_id,
	tcpip_param_id_type parameter_id, const uint8 *parameter_value);

/*!*********************************************************************************************************************
 * @brief          获取IP地址
 * @details        获取本地的IP地址,掩码,默认路由信息
 * @param[in]      local_addr_id          本地地址索引
 * @param[out]     ip_addr_ptr            获取的IP地址指针
 * @param[out]     netmask_ptr           获取的掩码指针
 * @param[out]     default_router_ptr     获取的默认路由指针
 * @retval         E_OK     获取IP地址成功
 * @retval         E_NOT_OK 获取IP地址失败
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00032
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_ip_addr(tcpip_local_addr_id_type local_addr_id,
	tcpip_sock_addr_type *ip_addr_ptr, uint8 *netmask_ptr, tcpip_sock_addr_type *default_router_ptr);

/*!*********************************************************************************************************************
 * @brief          获取MAC地址
 * @details        获取本地的MAC地址
 * @param[in]      local_addr_id          本地地址索引
 * @param[out]     phys_addr_ptr          获取的MAC地址指针
 * @retval         E_OK     获取MAC地址成功
 * @retval         E_NOT_OK 获取MAC地址失败
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00033
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_phys_addr(tcpip_local_addr_id_type local_addr_id, uint8 *phys_addr_ptr);

/*!*********************************************************************************************************************
 * @brief          获取控制器索引
 * @details        获取某个IP地址索引对应的控制器索引
 * @param[in]      local_addr_id          本地地址索引
 * @param[out]     ctrl_idx_ptr           获取的控制器索引指针
 * @retval         E_OK     获取控制器索引成功
 * @retval         E_NOT_OK 获取控制器索引失败
 * @reentrant      可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00140
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_ctrl_idx(tcpip_local_addr_id_type local_addr_id, uint8 *ctrl_idx_ptr);

/*!*********************************************************************************************************************
 * @brief          发送一帧UDP报文
 * @details        协议栈往指定的远端地址发送一帧UDP报文
 * @param[in]      socket_id             套接字ID
 * @param[in]      data_ptr              待发送的数据指针
 * @param[in]      remote_addr_ptr        远端地址指针
 * @param[in]      total_length          待发送的数据长度
 * @retval         E_OK     UDP报文发送成功
 * @retval         E_NOT_OK UDP报文发送失败
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00025
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_udp_transmit(tcpip_socket_id_type socket_id, const uint8 *data_ptr,
	const tcpip_sock_addr_type *remote_addr_ptr, uint16 total_length);

/*!*********************************************************************************************************************
 * @brief          组一帧UDP报文
 * @details        组一帧UDP报文，可绕过协议栈通过驱动接口直接发送
 * @param[in]      src_addr_ptr           源地址信息，包含IP地址和端口
 * @param[in]      dst_addr_ptr           目的地址信息，包含IP地址和端口
 * @param[in]      vlan_id               VLAN ID
 * @param[in]      buf_ptr               报文数据缓冲区指针
 * @param[in]      data_ptr              UDP报文数据指针
 * @param[in]      data_len              UDP报文数据长度指针
 * @retval         E_OK                 UDP组包成功
 * @retval         E_NOT_OK             UDP组包失败
 * @reentrant      不可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_udp_gen_packet(const tcpip_sock_addr_type *src_addr_ptr,
	const tcpip_sock_addr_type *dst_addr_ptr, uint16 vlan_id,
	eth_data_type *buf_ptr, const uint8 *data_ptr, uint16 *data_len);

/*!*********************************************************************************************************************
 * @brief          发送一帧TCP报文
 * @details        协议栈通过套接字往已经建立连接的远端地址发送一帧TCP报文
 * @param[in]      socket_id             套接字ID
 * @param[in]      data_ptr              待发送的数据指针
 * @param[in]      available_length      待发送的数据长度
 * @param[in]      force_retrieve        暂不生效
 * @retval         E_OK     TCP报文发送成功
 * @retval         E_NOT_OK TCP报文发送失败
 * @reentrant      不同的套接字可重入,同一个套接字不可重入
 * @synchronous    异步
 * @swsid          SWS_TCPIP_00050
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_tcp_transmit(tcpip_socket_id_type socket_id,
	const uint8 *data_ptr, uint32 available_length, boolean force_retrieve);

/*!*********************************************************************************************************************
 * @brief          获取TCP套接字的状态
 * @details        获取协议栈TCP套接字的状态
 * @param[in]      socket_id             套接字ID
 * @retval         TCPIP_TCP_SOCK_STATE_INVALID  无效套接字
 * @retval         TCPIP_TCP_SOCK_STATE_LISTEN   套接字处于listen状态
 * @retval         TCPIP_TCP_SOCK_STATE_SYNSENT  套接字处于SYN已发送状态
 * @retval         TCPIP_TCP_SOCK_STATE_SYNRCVD  套接字处于SYN已收到状态
 * @retval         TCPIP_TCP_SOCK_STATE_ESTABLISHED  套接字处于已建立连接状态
 * @retval         TCPIP_TCP_SOCK_STATE_CLOSEWAIT  套接字处于FIN_WAIT1状态
 * @retval         TCPIP_TCP_SOCK_STATE_LASTACK  套接字处于LAST_ACK状态
 * @retval         TCPIP_TCP_SOCK_STATE_FINWAIT1  套接字处于FIN_WAIT1状态
 * @retval         TCPIP_TCP_SOCK_STATE_FINWAIT2  套接字处于FIN_WAIT2状态
 * @retval         TCPIP_TCP_SOCK_STATE_CLOSING  套接字处于关闭中状态
 * @retval         TCPIP_TCP_SOCK_STATE_TIMEWAIT  套接字处于TIME_WAIT状态
 * @retval         TCPIP_TCP_SOCK_STATE_CLOSED  套接字处于关闭状态
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(tcpip_socket_state_type, TCPIP_CODE) tcpip_get_socket_state_of_socket_tcp(tcpip_socket_id_type socket_id);

/*!*********************************************************************************************************************
 * @brief          请求改变TCPIP的状态
 * @details        请求改变指定控制器的TCPIP状态
 * @param[in]      ctrl_idx              控制器索引
 * @param[in]      state                请求的状态
 * @retval         E_OK     请求改变TCPIP状态成功
 * @retval         E_NOT_OK 请求改变TCPIP状态失败
 * @reentrant      不可重入
 * @synchronous    同步
 * @swsid          SWS_TCPIP_00070
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_request_com_mode(uint8 ctrl_idx, tcpip_state_type state);

/*!*********************************************************************************************************************
 * @brief          申请一个新套接字
 * @details        上层模块通过协议栈申请套接字资源
 * @param[in]      socket_owner_id 套接字所属者序号
 * @param[in]      domain        IP地址族
 * @param[in]      protocol      套接字协议
 * @param[out]     socket_id_ptr   套接字序号
 * @retval         E_OK     申请套接字成功
 * @retval         E_NOT_OK 申请套接字失败
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(Std_ReturnType, TCPIP_CODE) tcpip_get_socket(uint8 socket_owner_id, tcpip_domain_type domain,
	tcpip_protocol_type protocol, tcpip_socket_id_type *socket_id_ptr);

/*!*********************************************************************************************************************
 * @brief          获取IP地址分配状态
 * @details        获取协议栈指定IP地址的分配状态
 * @param[in]      addr_id               地址索引
 * @retval         E_OK     获取IP地址分配状态成功
 * @retval         E_NOT_OK 获取IP地址分配状态失败
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(tcpip_ip_addr_state_type, TCPIP_CODE) tcpip_get_assignment_state_of_local_addr(uint8 addr_id);

/*!*********************************************************************************************************************
 * @brief          获取协议栈状态
 * @details        获取协议栈状态，DDS使用
 * @param[in]      network_handle               网络句柄
 * @retval         0     协议栈ONLINE
 * @retval         1     协议栈OFFLINE
 * @reentrant      可重入
 * @synchronous    同步
 **********************************************************************************************************************/
FUNC(sint32, TCPIP_CODE) tcpip_get_state(NetworkHandleType network_handle);

#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"

#ifdef __cplusplus
}
#endif

#endif
