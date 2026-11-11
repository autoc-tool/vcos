# ##############################################################################
# components/communication/net_dev/canif/CMakeLists.txt
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

if(CONFIG_MODULE_CANIF)
    # 头文件引入
    set( INCLUDE_DIR
        # GENERAL
        ${VCOS_ROOT}/components/include/autosar/
        ${VCOS_ROOT}/components/include/
        ${GENDATA_DIR}/Os
        ${GENDATA_DIR}/Rte
        ${GENDATA_DIR}/PduR
        ${GENDATA_DIR}/CanIf
        ${GENDATA_DIR}/CanNm
        ${GENDATA_DIR}/Nm
        ${GENDATA_DIR}/Memmap
        ${VCOS_APP_DIR}/apps
        ${VCOS_ROOT}/components/rt_framework/inc

        #vcos log
        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        #CanIf inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../pdu_router/inc
    )

    if(CONFIG_MODULE_CANTP)
        list(APPEND INCLUDE_DIR
            #CanTp inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../diagnostics/transport/cantp/inc
            ${GENDATA_DIR}/CanTp
        )
    endif()

    if(CONFIG_MODULE_CANNM)
        list(APPEND INCLUDE_DIR
            #CanNm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../net_manager/cannm/inc
            ${GENDATA_DIR}/CanNm
        )
    endif()
    if(CONFIG_MODULE_NM)
        list(APPEND INCLUDE_DIR
            #Nm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../net_manager/nm/inc
            ${GENDATA_DIR}/Nm
        )
    endif()
    if(CONFIG_MODULE_COMM)
        list(APPEND INCLUDE_DIR
            #ComM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../net_manager/comm/inc
            ${GENDATA_DIR}/ComM
        )
    endif()
    if(CONFIG_MODULE_CANSM)
        list(APPEND INCLUDE_DIR
            #ComM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../net_manager/cansm/inc
            ${GENDATA_DIR}/CanSM
        )
    endif()
    if(CONFIG_SERVM_ECUM)
        list(APPEND INCLUDE_DIR
            #EcuM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/service_management/sysm_init/inc
            ${GENDATA_DIR}/sysm/EcuM
        )
    endif()
    if (CONFIG_MODULE_CANTRCV1145)
        list(APPEND INCLUDE_DIR
            ${GENDATA_DIR}/CanTrcv1145
        )
    endif()
    if (CONFIG_ARCH_SIM)
        list(APPEND INCLUDE_DIR
            ${SIM_DRIVERS_DIR}/tja1145/inc
        )
    elseif(NOT CONFIG_ARCH_CHIP_E3650 AND CONFIG_MODULE_CANTRCV1145)
        list(APPEND INCLUDE_DIR
            #CanTrcv1145 inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../net_dev/cantrcv1145/inc
        )
    endif()
    if(CONFIG_MODULE_DEM)
        list(APPEND INCLUDE_DIR
            #dem inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../diagnostics/service/dem/inc
            ${GENDATA_DIR}/Dem
        )
    endif()
    if(CONFIG_MODULE_CANTSYN)
        list(APPEND INCLUDE_DIR
            #tsync inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/time/tsync/inc
            ${GENDATA_DIR}/CanTSyn
        )
    endif()
    if(CONFIG_MODULE_STBM)
        list(APPEND INCLUDE_DIR
            #stbm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/time/time_manager/inc
            ${GENDATA_DIR}/StbM
        )
    endif()
    if(CONFIG_MODULE_ETHIF)
        list(APPEND INCLUDE_DIR
            #ethif inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../net_dev/eth
            ${GENDATA_DIR}/EthIf
            ${VCOS_ROOT}/components/communication/net_protocols/sockets
            ${VCOS_ROOT}/components/communication/external/lwip
            ${GENDATA_DIR}/EthIf
            ${GENDATA_DIR}/Lwip
            ${GENDATA_DIR}/TcpIp
        )
        if(CONFIG_ARCH_CHIP_E3650)
            #ethdriver inc
            list(APPEND INCLUDE_DIR
            ${ETH_STACK_E3XX_GETHMAC_DIR}/e3650
            )
        elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
            list(APPEND INCLUDE_DIR
                ${ETH_STACK_TC3xx_GETHMAC_DIR}
            )
        endif()
    endif()

    # Can driver
    if (CONFIG_ARCH_SIM)
        list(APPEND INCLUDE_DIR
            ${SIM_DRIVERS_CANSTACK_INCLUDE_DIRS}
            ${SIM_DRIVERS_DIR}/spi/inc
        )
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_TC3XX_DIR}
        )
    elseif(CONFIG_ARCH_CHIP_E3650)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_E3XX_DIR}
        )
    elseif(CONFIG_ARCH_CHIP_J6E)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_J6E_DIR}
        )
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_FC7300F8MDT_DIR}
        )
    endif()
    # 收集 CanIf 目录下的所需 .c 文件
    file(GLOB CANIF_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    # 合并所有源文件
    set( CANIF_CAN_SRCS
        ${CANIF_SOURCES}
        ${GENDATA_DIR}/CanIf/canif_cfg.c
        ${GENDATA_DIR}/CanIf/canif_pbcfg.c
    )

    nuttx_add_library(can_canif STATIC)
    target_sources(can_canif PRIVATE ${CANIF_CAN_SRCS})
    target_include_directories(can_canif PRIVATE ${INCLUDE_DIR})

    if (CONFIG_ARCH_SIM)
        target_link_libraries(can_canif PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(can_canif PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(can_canif PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(can_canif PRIVATE j6_can_driver)
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        target_link_libraries(can_canif PRIVATE fc7300f8mdt_can_driver)
    endif()
endif()
