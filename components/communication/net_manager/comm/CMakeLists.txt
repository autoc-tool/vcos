# ##############################################################################
# components/communication/net_manager/comm/CMakeLists.txt
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

if(CONFIG_MODULE_COMM)
    # 头文件引入
    set( INCLUDE_DIR
        # GENERAL
        ${VCOS_ROOT}/components/include/autosar/
        ${VCOS_ROOT}/components/include/
        ${GENDATA_DIR}/Os
        ${GENDATA_DIR}/Rte
        ${GENDATA_DIR}/CanIf
        ${GENDATA_DIR}/ComM
        ${GENDATA_DIR}/CanSM
        ${GENDATA_DIR}/Nm
        ${GENDATA_DIR}/Memmap
        ${VCOS_APP_DIR}/apps
        ${VCOS_ROOT}/components/rt_framework/inc

        #vcos log
        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        #ComM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #CanIf inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../net_dev/canif/inc
        #CanSM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../cansm/inc
        #Nm inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../nm/inc
    )
    if(CONFIG_MODULE_NM)
        list(APPEND INCLUDE_DIR
            #Nm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../nm/inc
            ${GENDATA_DIR}/Nm
        )
    endif()
    if(CONFIG_SERVM_ECUM)
        list(APPEND INCLUDE_DIR
            #EcuM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/service_management/sysm_init/inc
            ${GENDATA_DIR}/sysm/EcuM
        )
    endif()
    if(CONFIG_SERVM_BSWM)
        list(APPEND INCLUDE_DIR
            #BswM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/service_management/sysm_core/inc
            ${GENDATA_DIR}/sysm/BswM
        )
    endif()
    if(CONFIG_MODULE_NVM)
        list(APPEND INCLUDE_DIR
            #NVM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../memory/nvm/inc
            ${GENDATA_DIR}/NvM
        )
    endif()

    if(CONFIG_MODULE_DCM)
        list(APPEND INCLUDE_DIR
            #dcm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../diagnostics/service/dcm/inc
            ${GENDATA_DIR}/Dcm
        )
    endif()

    if(CONFIG_MODULE_DEM)
        list(APPEND INCLUDE_DIR
            #dem inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../diagnostics/service/dem/inc
            ${GENDATA_DIR}/Dem
        )
    endif()

    # 收集 ComM 目录下的所需 .c 文件
    file(GLOB COMM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    # Can driver
    if (CONFIG_SIM_CAN_DRIVER)
        list(APPEND INCLUDE_DIR
            ${SIM_DRIVERS_CANSTACK_INCLUDE_DIRS}
        )
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_TC3XX_DIR}
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

    # 合并所有源文件
    set( NET_COMM_SRCS
        ${COMM_SOURCES}
        ${GENDATA_DIR}/ComM/comm_cfg.c
        ${GENDATA_DIR}/ComM/comm_pbcfg.c
    )

    nuttx_add_library(net_comm STATIC)
    target_sources(net_comm PRIVATE ${NET_COMM_SRCS})
    target_include_directories(net_comm PRIVATE ${INCLUDE_DIR})
    if (CONFIG_SIM_CAN_DRIVER)
        target_link_libraries(net_comm PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(net_comm PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(net_comm PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(net_comm PRIVATE j6_can_driver)
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        target_link_libraries(net_comm PRIVATE fc7300f8mdt_can_driver)
    endif()
endif()
