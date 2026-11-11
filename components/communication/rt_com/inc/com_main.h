
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

#ifndef COM_MAIN_H
#define COM_MAIN_H

#include "com.h"

#ifdef __cplusplus
extern "C" {
#endif
#define COM_START_SEC_CODE
#include "com_memmap.h"
/****************************************************************************
 * name: com_main_function_rx_run
 *
 * description: com rx internal main function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_main_function_rx_run(uint8 main_function_id);

/****************************************************************************
 * name: com_main_function_tx_run
 *
 * description: com tx internal main function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_main_function_tx_run(uint8 main_function_id);

/****************************************************************************
 * name: com_main_function_gw_run
 *
 * description: com signal gateway main function.
 *
 ****************************************************************************/
FUNC(void, COM_CODE) com_main_function_gw_run(uint8 main_function_id);
#define COM_STOP_SEC_CODE
#include "com_memmap.h"
#ifdef __cplusplus
}
#endif
#endif /* COM_SCHED_H */
