# ##############################################################################
# components/communication/pdu_router/CMakeLists.txt
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

if(CONFIG_MODULE_PDUR)
    # 头文件引入
    set( INCLUDE_DIR
        # GENERAL
        ${VCOS_ROOT}/components/include/autosar/
        ${VCOS_ROOT}/components/include/
        ${GENDATA_DIR}/Os
        ${GENDATA_DIR}/Com
        ${GENDATA_DIR}/PduR
        ${GENDATA_DIR}/CanIf
        ${GENDATA_DIR}/Rte
        ${GENDATA_DIR}/Memmap
        ${VCOS_APP_DIR}/apps
        ${VCOS_ROOT}/components/rt_framework/inc

        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        #Com inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../rt_com/inc
        #CanIf inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../net_dev/canif/inc
        #CanNm inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../net_manager/cannm/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )

    if(CONFIG_MODULE_CANTP)
        list(APPEND INCLUDE_DIR
            #CanTp inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../diagnostics/transport/cantp/inc
            ${GENDATA_DIR}/CanTp
        )
    endif()

    if(CONFIG_MODULE_DOIP)
        list(APPEND INCLUDE_DIR
            #DOIP inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../diagnostics/transport/doip/inc
            ${GENDATA_DIR}/DoIP
            ${GENDATA_DIR}/TcpIp
            ${VCOS_ROOT}/components/communication/net_protocols/sockets
            ${VCOS_ROOT}/components/communication/soad/inc

        )
    endif()

    if(CONFIG_MODULE_DCM)
        list(APPEND INCLUDE_DIR
            #Dcm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../diagnostics/service/dcm/inc
            ${GENDATA_DIR}/Dcm
        )
    endif()

    if(CONFIG_MODULE_CANNM)
        list(APPEND INCLUDE_DIR
            #CanNm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../net_manager/cannm/inc
            ${GENDATA_DIR}/CanNm
        )
    endif()

    if(CONFIG_MODULE_NM)
        list(APPEND INCLUDE_DIR
            #Nm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../net_manager/nm/inc
            ${GENDATA_DIR}/Nm
        )
    endif()

    if(CONFIG_MODULE_CANSM)
        list(APPEND INCLUDE_DIR
            #CanSM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../net_manager/cansm/inc
            ${GENDATA_DIR}/CanSM
        )
    endif()

    if(CONFIG_MODULE_COMM)
        list(APPEND INCLUDE_DIR
            #ComM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../net_manager/comm/inc
            ${GENDATA_DIR}/ComM
        )
    endif()

    if(CONFIG_SERVM_BSWM)
        list(APPEND INCLUDE_DIR
            #BswM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../system_services/service_management/sysm_core/inc
            ${GENDATA_DIR}/sysm/BswM
        )
    endif()

    if(CONFIG_SERVM_ECUM)
        list(APPEND INCLUDE_DIR
            #EcuM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../system_services/service_management/sysm_init/inc
            ${GENDATA_DIR}/sysm/EcuM
        )
    endif()

    # 收集 PduR 目录下的所需 .c 文件
    file(GLOB PDUROUTER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../pdu_router/inc
    )

    if (CONFIG_ARCH_SIM)
        list(APPEND INCLUDE_DIR
            ${SIM_DRIVERS_CANSTACK_INCLUDE_DIRS}
        )
    elseif(CONFIG_ARCH_CHIP_E3650)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_E3XX_DIR}
        )
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_FC7300F8MDT_DIR}
        )
    endif()

    # 收集 PduR 目录下的所需 .c 文件
    file(GLOB PDUROUTER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    file(GLOB PduRGen_SOURCES
        "${GENDATA_DIR}/PduR/*.c"
    )

    # 合并所有源文件
    set( PDUROUTER_SRCS
        ${PDUROUTER_SOURCES}
        ${PduRGen_SOURCES}
    )

    nuttx_add_library(pdurouter STATIC)
    target_sources(pdurouter PRIVATE ${PDUROUTER_SRCS})
    target_include_directories(pdurouter PRIVATE ${INCLUDE_DIR} ${GENDATA_INC_DIRS})
    if (CONFIG_ARCH_SIM)
        target_link_libraries(pdurouter PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(pdurouter PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(pdurouter PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(pdurouter PRIVATE j6_can_driver)
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        target_link_libraries(pdurouter PRIVATE fc7300f8mdt_can_driver)
    endif()
endif()
