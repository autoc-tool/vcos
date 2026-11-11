# ##############################################################################
# Copyright (c) 2025 Li Auto Inc. and its affiliates
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ##############################################################################
if(CONFIG_TEST_CMD_IPERF)
    file(GLOB IPERF_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    set(CSRCS
        ${IPERF_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/iperf_main.c)

    set(LWIP_2_1_3_DIR ${VCOS_ROOT}/components/communication/external/lwip/lwip-STABLE-2_1_3_RELEASE)
    set(NET_DEV_DIR "${VCOS_ROOT}/components/communication/net_dev")

    set(INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${VCOS_ROOT}/components/communication/net_protocols/sockets
        ${VCOS_ROOT}/components/include/autosar/
        ${GENDATA_DIR}/Os
        ${GENDATA_DIR}/Memmap
        ${GENDATA_DIR}/EthIf
        ${GENDATA_DIR}/Lwip
        ${GENDATA_DIR}/TcpIp
        ${VCOS_APP_DIR}/apps
        ${VCOS_ROOT}/components/rt_framework/inc
        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        ${NET_DEV_DIR}/eth
        ${NUTTX_DIR}/../components/include/
        ${LWIP_2_1_3_DIR}/src/include/               # lwIP 官方 include
        ${VCOS_ROOT}/components/communication/external/lwip/lwip_osal/        # OSAL/移植层 (sys_arch.h 等)
        ${VCOS_ROOT}/components/communication/external/lwip)                 # 顶层(有 lwipopts.h, Lwip.h, ethernetif.h 等)

    if(CONFIG_RT_FRAMEWORK_MEMPOOL)
        list(APPEND INCLUDE_DIRS
         ${GENDATA_DIR}/MemPool
      )
    endif()
    if(CONFIG_ARCH_CHIP_E3650)
      list(APPEND INCLUDE_DIRS
        ${ETH_STACK_E3XX_GETHMAC_DIR}/e3650
      )
    else(CONFIG_ARCH_CHIP_CUSTOM_TC397)
      list(APPEND INCLUDE_DIRS
        ${ETH_STACK_TC3xx_GETHMAC_DIR}
      )
    endif()

  nuttx_add_application(
    NAME
    ${CONFIG_TEST_CMD_IPERF_PROGNAME}
    SRCS
    ${CSRCS}
    INCLUDE_DIRECTORIES
    ${INCLUDE_DIRS}
    INCLUDE_DIRS
    COMPILE_FLAGS
    STACKSIZE
    ${CONFIG_TEST_CMD_IPERF_STACKSIZE}
    PRIORITY
    ${CONFIG_TEST_CMD_IPERF_PRIORITY})

endif()
