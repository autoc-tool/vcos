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

#ifndef LWIP_ETHERNETIF_H
#define LWIP_ETHERNETIF_H

#if (CONFIG_MODULE_ETHIF == 1)
#include "ethif.h"
#endif
#include "Eth_GeneralTypes.h"
#include "lwip/netif.h"
#include "EthTrcv_GeneralTypes.h"

typedef struct {
    uint8 ctrl_id;
    uint8 priority;
} lwip_netif_ctrl_type;

#define TCPIP_START_SEC_CODE
#include "tcpip_memmap.h"
FUNC(void, TCPIP_CODE) lwip_rxIndication( uint8 ctrl_idx, eth_frame_type frame_type, boolean is_broadcast, const uint8* phys_addr_ptr,const eth_data_type* data_ptr, uint16 len_byte );
// FUNC(void, TCPIP_CODE) LwIp_TxConfirmation(uint16 SocketId, TcpIp_Up_TxConfirmation fn);
FUNC(err_t, TCPIP_CODE) lwip_ethernetif_init(struct netif *netif);
FUNC(void, TCPIP_CODE) lwip_add_default_netif(void);
FUNC(void, TCPIP_CODE) lwip_set_phys_addr(struct netif *netif, const uint8* phys_addr_ptr);
#define TCPIP_STOP_SEC_CODE
#include "tcpip_memmap.h"

#endif
