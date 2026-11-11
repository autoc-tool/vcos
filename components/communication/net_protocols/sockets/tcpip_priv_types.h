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

#if !defined(TCPIP_PRIV_TYPES_H)
#define TCPIP_PRIV_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************************************************
 *  INCLUDES
 *********************************************************************************************************************/
#include "Std_Types.h"
#include "tcpip_types.h"
#include "tcpip_cfg.h"
#include "ethernetif.h"
#include "Platform_Types.h"

/**********************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *********************************************************************************************************************/

typedef struct {
	const tcpip_local_addr_type *ptr_local_unicast_addr;

	tcpip_state_type tcpip_ethif_ctrl_state;
	tcpip_state_type tcpip_ethif_requested_ctrl_state;

	uint32 tcpip_ethif_ctrl_onhold_state_cnt;

	lwip_netif_ctrl_type tcpip_netif_ctrl;
	struct netif tcpip_netif;
	boolean link_up; /* Link status */
} tcpip_ctrl_admin_type;

#if (CPU_BYTE_ORDER == HIGH_BYTE_FIRST)
#define IP4_ADDR_CHR_TO_UINT32(a, b, c, d)                                                                             \
	(((uint32)((a) & 0xff) << 24) | ((uint32)((b) & 0xff) << 16) | ((uint32)((c) & 0xff) << 8) |                   \
	 (uint32)((d) & 0xff)) ///< 将IP地址转换为uint32类型
#else
#define IP4_ADDR_CHR_TO_UINT32(a, b, c, d)                                                                             \
	(((uint32)((d) & 0xff) << 24) | ((uint32)((c) & 0xff) << 16) | ((uint32)((b) & 0xff) << 8) |                   \
	 (uint32)((a) & 0xff)) ///< 将IP地址转换为uint32类型
#endif

#define TCPIP_STATIC (TCPIP_IPADDR_ASSIGNMENT_STATIC) ///< 静态IP地址分配类型
#define TCPIP_LINKLOCAL_DOIP (TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) ///< 本地DOIP地址分配类型
#define TCPIP_DHCP (TCPIP_IPADDR_ASSIGNMENT_DHCP) ///< DHCP地址分配类型
#define TCPIP_LINKLOCAL (TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL) ///< 本地IP地址分配类型
#define TCPIP_IPV6_ROUTER (TCPIP_IPADDR_ASSIGNMENT_IPV6_ROUTER) ///< IPv6路由器地址分配类型

#define TCPIP_UDP_MAX_PYLOAD_LEN (1472u) ///< UDP最大数据长度

#ifdef __cplusplus
}
#endif /* __cplusplus     */

#endif
/* TCPIP_PRIV_TYPES_H */
/**********************************************************************************************************************
 *  END OF FILE: TcpIp_Priv_Types.h
 *********************************************************************************************************************/
