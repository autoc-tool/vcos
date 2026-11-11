# ##############################################################################
# components/communication/net_manager/cannm/CMakeLists.txt
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

if(CONFIG_MODULE_CANNM)
    # 头文件引入
    set( INCLUDE_DIR
        # GENERAL
        ${VCOS_ROOT}/components/include/autosar/
        ${VCOS_ROOT}/components/include/
        ${GENDATA_DIR}/Os
        ${GENDATA_DIR}/Rte
        ${GENDATA_DIR}/PduR
        ${GENDATA_DIR}/CanIf
        ${GENDATA_DIR}/ComM
        ${GENDATA_DIR}/CanNm
        ${GENDATA_DIR}/CanSM
        ${GENDATA_DIR}/Nm
        ${GENDATA_DIR}/Memmap
        ${VCOS_APP_DIR}/apps
        ${VCOS_ROOT}/components/rt_framework/inc

        #vcos log
        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        #CanNm inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #CanIf inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../net_dev/canif/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../pdu_router/inc
        #ComM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../comm/inc
        #Nm inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../nm/inc
        #CanSM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../cansm/inc
    )

    if (CONFIG_ARCH_SIM)
        list(APPEND INCLUDE_DIR
            ${SIM_DRIVERS_CANSTACK_INCLUDE_DIRS}
        )
    elseif(CONFIG_ARCH_CHIP_E3650)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_E3XX_DIR}
        )
    endif()
    if(CONFIG_SERVM_ECUM)
        list(APPEND INCLUDE_DIR
            #EcuM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/service_management/sysm_init/inc
            ${GENDATA_DIR}/sysm/EcuM
        )
    endif()
    # 收集 CanNm 目录下的所需 .c 文件
    file(GLOB CANNM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    # 合并所有源文件
    set( CANNM_CAN_SRCS
        ${CANNM_SOURCES}
        ${GENDATA_DIR}/CanNm/cannm_cfg.c
    )

    nuttx_add_library(can_cannm STATIC)
    target_sources(can_cannm PRIVATE ${CANNM_CAN_SRCS})
    target_include_directories(can_cannm PRIVATE ${INCLUDE_DIR})
    if (CONFIG_ARCH_SIM)
        target_link_libraries(can_cannm PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(can_cannm PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(can_cannm PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(can_cannm PRIVATE j6_can_driver)
    endif()
endif()
