# ##############################################################################
# apps/examples/CMakeLists.txt
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################
if(DEFINED BOARD_NAME)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/system_cfg/${BOARD_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${BOARD_NAME})
    set(CSRCS ${VCOS_APP_DIR}/apps/system.c
              ${VCOS_APP_DIR}/apps/shell.c
              ${VCOS_APP_DIR}/apps/test_stubs.c)

    if(CONFIG_RT_FRAMEWORK_HOOK)
      list(APPEND CSRCS ${VCOS_APP_DIR}/apps/bsw_user_callout/hook_user_callout.c)
    endif()

    if(CONFIG_ETH_STACK)
      list(APPEND CSRCS ${GENDATA_DIR}/EthIf/ethif_lcfg.c
                        ${GENDATA_DIR}/TcpIp/tcpip_cfg.c)
    endif()

    if(CONFIG_RT_FRAMEWORK)
      set(INCLUDE_DIRS  ${CMAKE_CURRENT_LIST_DIR}
                      ${VCOS_APP_DIR}/apps
                      ${VCOS_APP_DIR}/apps/asw_swc
                      ${VCOS_APP_DIR}/apps/bsw_user_callout
                      ${VCOS_ROOT}/components/rt_framework/inc
                      ${VCOS_ROOT}/components/communication/net_dev/eth
                      ${VCOS_ROOT}/components/communication/net_protocols/sockets
                      ${VCOS_ROOT}/components/communication/external/lwip
                      ${VCOS_ROOT}/components/communication/external/lwip/lwip_osal
                      ${VCOS_ROOT}/components/communication/external/lwip/lwip-STABLE-2_1_3_RELEASE/src/include
                      ${VCOS_ROOT}/components/include/autosar
                      ${VCOS_ROOT}/components/include/)

      if(CONFIG_ARCH_TRICORE)
        nuttx_add_application(
          NAME
          os_perf_test
          SRCS
          ${VCOS_APP_DIR}/apps/test_perf.c
          STACKSIZE
          1024
          PRIORITY
          100
          INCLUDE_DIRECTORIES
          ${INCLUDE_DIRS}
          ${GENDATA_INC_DIRS}
        )
      endif()

      nuttx_add_library(start_rt_framework STATIC)
      target_sources(start_rt_framework PRIVATE ${CSRCS} ${GENDATA_SRC_FILES})
      target_include_directories(start_rt_framework PRIVATE ${INCLUDE_DIRS} ${GENDATA_INC_DIRS} ${PLATFORM_INCLUDE_DIRS})
      target_link_libraries(start_rt_framework PRIVATE ${SIM_LIBS})
    endif()

endif()
