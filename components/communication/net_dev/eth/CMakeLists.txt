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
  # 头文件引入
  set( INCLUDE_DIR

    # GENERAL
    ${VCOS_ROOT}/components/include/autosar/
    ${GENDATA_DIR}/Os
    ${GENDATA_DIR}/Memmap
    ${GENDATA_DIR}/EthIf
    ${GENDATA_DIR}/Lwip
    ${GENDATA_DIR}/TcpIp
    ${VCOS_APP_DIR}/apps
    ${VCOS_ROOT}/components/rt_framework/inc
    ${VCOS_ROOT}/components/maintaining/vcos_log/inc
    ${VCOS_APP_DIR}/apps/mvbs_app/mvbs_adapter/dds_autosar_intf/

    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VCOS_ROOT}/components/include/
    ${VCOS_ROOT}/components/communication/net_protocols/sockets/
  )

  if(${BOARD_NAME} MATCHES "j6e_adpro_4_0")
    list(APPEND INCLUDE_DIRS ${VCOS_ROOT}/drivers/can/j6/j6e)
    list(APPEND INCLUDE_DIRS ${VENDOR_THIRD_PARTY_DRIVER_INCDIR})
  endif()

  if(CONFIG_RT_FRAMEWORK_MEMPOOL)
    list(APPEND INCLUDE_DIR
    ${GENDATA_DIR}/MemPool
    )
  endif()
  if(CONFIG_ARCH_CHIP_E3650)
    list(APPEND INCLUDE_DIR
      ${ETH_STACK_E3XX_GETHMAC_DIR}/e3650
    )
  elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
    list(APPEND INCLUDE_DIR
        ${ETH_STACK_TC3xx_GETHMAC_DIR}
    )
  elseif(CONFIG_ARCH_CHIP_J6E)
    list(APPEND INCLUDE_DIR
        ${ETH_STACK_J6_GETHMAC_DIR}
    )
  endif()

  # 收集 EthIf 目录下的所需 .c 文件
  file(GLOB C_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
  )

  # 合并所有源文件
  set( ETH_STACK_SRCS
    ${C_SOURCES}
    ${GENDATA_DIR}/EthIf/ethif_lcfg.c
  )

  if ((CMAKE_C_COMPILER_ID STREQUAL "GNU") AND (NOT CONFIG_SIM_ETH_DRIVER))
    add_compile_options(-Wno-undef)
    add_compile_options(-Werror)
  endif()

  nuttx_add_library(eth_if STATIC)
  target_sources(eth_if PRIVATE ${ETH_STACK_SRCS})
  target_include_directories(eth_if PRIVATE ${INCLUDE_DIR})

  target_include_directories(eth_if SYSTEM PRIVATE
    "${LWIP_2_1_3_DIR}/src/include"               # lwIP 官方 include
    "${EXTERNAL_DIR}/lwip/lwip_osal"  # OSAL/移植层 (sys_arch.h 等)
    "${EXTERNAL_DIR}/lwip"          # 顶层(有 lwipopts.h, Lwip.h, ethernetif.h 等)
  )
  if (NOT CONFIG_SIM_ETH_DRIVER)
    target_link_libraries(eth_if PRIVATE eth_gethmac)
  endif()
endif()
