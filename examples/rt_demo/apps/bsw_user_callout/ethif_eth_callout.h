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
 * @file        ethif_eth_callout.h
 * @brief       EthIf对Eth部分接口适配头文件
 * @details     EthIf对Eth部分接口适配头文件
 * @date        2026-11-11 20:45:04
 **********************************************************************************************************************/

#ifndef ETHIF_ETH_CALLOUT_H
#define ETHIF_ETH_CALLOUT_H

#include "ethif_types.h"
#include "ethif_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if (STD_ON == ETHIF_GLOBAL_TIME_SUPPORT)

#endif

#define ETHIF_START_SEC_CALLOUT_CODE
#include "ethif_memmap.h"

FUNC(Std_ReturnType, ETHIF_CALLOUT_CODE) ethif_ethtrcv_get_linkstate( uint8 trcv_id, eth_trcv_linkstate_type* trcv_linkstate );

#define ETHIF_STOP_SEC_CALLOUT_CODE
#include "ethif_memmap.h"

#ifdef __cplusplus
}
#endif

#endif /* ETHIF_ETH_CALLOUT_H */

