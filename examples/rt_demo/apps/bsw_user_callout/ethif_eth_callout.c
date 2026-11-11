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
/*
 * Generation Time : 2026-11-11 20:45:04
 * Project Name    : rt_demo
 * Tool Version    : V0.1.0-DEV
 * Description:  EthIf对Eth部分接口适配文件.
 */

#include "ethif_types.h"
#include "ethif_cfg.h"
#include "ethif_eth_callout.h"

#include "Eth.h"

/******************USER CODE STARTS!************THIS COMMENT WILL BE SAVED!***************/
/******************USER CODE ENDS!**************THIS COMMENT WILL BE SAVED!***************/

#if (STD_ON == ETHIF_GLOBAL_TIME_SUPPORT)
#endif

#define ETHIF_START_SEC_CALLOUT_CODE
#include "ethif_memmap.h"

FUNC(Std_ReturnType, ETHIF_CALLOUT_CODE) ethif_ethtrcv_get_linkstate( uint8 trcv_idx, eth_trcv_linkstate_type* trcv_linkstate )
{
/******************USER CODE STARTS!************THIS COMMENT WILL BE SAVED!***************/
/******************USER CODE ENDS!**************THIS COMMENT WILL BE SAVED!***************/
}

#define ETHIF_STOP_SEC_CALLOUT_CODE
#include "ethif_memmap.h"