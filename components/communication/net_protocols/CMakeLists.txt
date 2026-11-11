# ##############################################################################
# components/communication/eth_stack/CMakeLists.txt
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

if(CONFIG_ETH_STACK)
  set(LWIP_2_1_3_DIR ${EXTERNAL_DIR}/lwip/lwip-STABLE-2_1_3_RELEASE)
  # 头文件引入
  set( INCLUDE_DIR
    # GENERAL
    ${EXTERNAL_DIR}/lwip/
    ${VCOS_ROOT}/components/include/
    ${VCOS_ROOT}/components/include/autosar/
    ${GENDATA_DIR}/Os
    ${GENDATA_DIR}/Memmap
    ${GENDATA_DIR}/EthIf
    ${GENDATA_DIR}/Lwip
    ${GENDATA_DIR}/TcpIp
    ${GENDATA_DIR}/MemPool
    ${VCOS_APP_DIR}/apps
    ${VCOS_ROOT}/components/rt_framework/inc
    ${VCOS_ROOT}/components/maintaining/vcos_log/inc

    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/sockets/
    ${NET_DEV_DIR}/eth
    ${NUTTX_DIR}/../components/include/
  )
  file(GLOB_RECURSE LWIP_HEADER "${LWIP_2_1_3_DIR}/*.h")

  set(APPEND INCLUDE_DIR ${LWIP_HEADER})
  if(CONFIG_RT_FRAMEWORK_MEMPOOL)
      list(APPEND INCLUDE_DIR ${VCOS_ROOT}/components/include/autosar/)
  endif()
  if(CONFIG_MODULE_SOAD)
      list(APPEND INCLUDE_DIR ${VCOS_ROOT}/components/communication/soad/inc)
  endif()

  if(CONFIG_TEST_CMD_IPERF)
      list(APPEND INCLUDE_DIR ${NUTTX_DIR}/../apps/netutils/iperf/inc/)
  endif()

  if(CONFIG_ARCH_CHIP_E3650)
    list(APPEND INCLUDE_DIR
      ${ETH_STACK_E3XX_GETHMAC_DIR}/e3650
    )
  elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
    list(APPEND INCLUDE_DIR
      ${ETH_STACK_TC3xx_GETHMAC_DIR}
    )
  else(CONFIG_ARCH_CHIP_J6E)
    list(APPEND INCLUDE_DIR
      ${ETH_STACK_J6_GETHMAC_DIR}
    )
  endif()



  # 收集sockets目录下和codege目录下的所需 .c 文件
  file(GLOB C_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/sockets/*.c"
    ${GENDATA_DIR}/TcpIp/tcpip_cfg.c
  )

  # 收集 lwip 目录下的所需 .c 文件
  file(GLOB_RECURSE LWIP_SOURCES
    # 顶层
    "${EXTERNAL_DIR}/lwip/ethernetif.c"
    "${EXTERNAL_DIR}/lwip/lwip.c"
    # lwip_osal
    "${EXTERNAL_DIR}/lwip/lwip_osal/sys_arch.c"

    # LwIp
    "${LWIP_2_1_3_DIR}/src/api/tcpip_lwip.c"
    "${LWIP_2_1_3_DIR}/src/api/netifapi.c"
    "${LWIP_2_1_3_DIR}/src/core/def.c"
    "${LWIP_2_1_3_DIR}/src/core/inet_chksum.c"
    "${LWIP_2_1_3_DIR}/src/core/init_lwip.c"
    "${LWIP_2_1_3_DIR}/src/core/ip.c"
    "${LWIP_2_1_3_DIR}/src/core/mem.c"
    "${LWIP_2_1_3_DIR}/src/core/memp.c"
    "${LWIP_2_1_3_DIR}/src/core/netif.c"
    "${LWIP_2_1_3_DIR}/src/core/pbuf.c"
    "${LWIP_2_1_3_DIR}/src/core/raw.c"
    "${LWIP_2_1_3_DIR}/src/core/stats.c"
    "${LWIP_2_1_3_DIR}/src/core/sys.c"
    "${LWIP_2_1_3_DIR}/src/core/tcp_in.c"
    "${LWIP_2_1_3_DIR}/src/core/tcp_out.c"
    "${LWIP_2_1_3_DIR}/src/core/tcp.c"
    "${LWIP_2_1_3_DIR}/src/core/timeouts.c"
    "${LWIP_2_1_3_DIR}/src/core/ipv4/etharp.c"
    "${LWIP_2_1_3_DIR}/src/core/ipv4/icmp.c"
    "${LWIP_2_1_3_DIR}/src/core/ipv4/igmp.c"
    "${LWIP_2_1_3_DIR}/src/core/ipv4/ip4_addr.c"
    "${LWIP_2_1_3_DIR}/src/core/ipv4/ip4_frag.c"
    "${LWIP_2_1_3_DIR}/src/core/ipv4/ip4.c"
    "${LWIP_2_1_3_DIR}/src/netif/ethernet.c"
    "${LWIP_2_1_3_DIR}/src/core/udp.c"
  )

  # 合并所有源文件
  set( ETH_STACK_SRCS
    ${C_SOURCES}
    ${LWIP_SOURCES}
  )
  if(CONFIG_TEST_CMD_IPERF)
    list(APPEND ETH_STACK_SRCS "${NUTTX_DIR}/../apps/netutils/iperf/src/iperf_task.c")
  endif()

  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if(NOT CONFIG_SIM_ETH_DRIVER)
      add_compile_options(-Werror)
    endif()
    foreach(lwip_file IN LISTS LWIP_SOURCES)
      set_source_files_properties(${lwip_file} PROPERTIES COMPILE_FLAGS "-Wno-error -Wno-implicit-function-declaration -Wno-unused-value -Wno-undef")
    endforeach()
  endif()

  nuttx_add_library(socket_auto_eth STATIC)
  target_sources(socket_auto_eth PRIVATE ${ETH_STACK_SRCS})
  target_include_directories(socket_auto_eth PRIVATE ${INCLUDE_DIR})

  target_include_directories(socket_auto_eth SYSTEM PRIVATE
    "${LWIP_2_1_3_DIR}/src/include"               # lwIP 官方 include
    "${EXTERNAL_DIR}/lwip/lwip_osal"  # OSAL/移植层 (sys_arch.h 等)
    "${EXTERNAL_DIR}/lwip"          # 顶层(有 lwipopts.h, Lwip.h, ethernetif.h 等)
  )

  if (NOT CONFIG_SIM_ETH_DRIVER)
    target_link_libraries(eth_if PRIVATE eth_gethmac)
  endif()
endif()
