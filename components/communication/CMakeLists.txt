
# ##############################################################################
# components/CMakeLists.txt
#
# Copyright (c) 2025 Li Auto Inc. and its affiliates
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ##############################################################################
# include corresponding subdirectory
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(NET_PROTOCOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/net_protocols")
set(NET_DEV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/net_dev")
set(NET_MANAGER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/net_manager")
set(PDU_ROUTER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pdu_router")
set(RT_COM_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/rt_com")
set(RT_ETH_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/rt_eth")
set(SOAD_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/soad")
nuttx_add_subdirectory()
nuttx_generate_kconfig(MENUDESC "communication")
