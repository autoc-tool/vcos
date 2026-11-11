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

#ifndef __INCLUDE_NUTTX_TRACE_ETH_WRAPPER_H
#define __INCLUDE_NUTTX_TRACE_ETH_WRAPPER_H

int eth_udpdata_send(const uint8_t *dataptr, uint16_t totallen);

#endif /* __INCLUDE_NUTTX_TRACE_ETH_WRAPPER_H */