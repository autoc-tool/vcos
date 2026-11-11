# ##############################################################################
# components/communication/net_manager/cansm/CMakeLists.txt
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

if(CONFIG_MODULE_CANSM)
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
        ${GENDATA_DIR}/Com
        ${GENDATA_DIR}/PduR
        ${GENDATA_DIR}/NvM
        ${GENDATA_DIR}/Memmap
        ${VCOS_APP_DIR}/apps
        ${VCOS_ROOT}/components/rt_framework/inc

        #vcos log
        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        #CanIf inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../net_dev/canif/inc
        #Com inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../rt_com/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../pdu_router/inc
        #ComM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../comm/inc
        #CanSM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #NvM inc
        ${VCOS_ROOT}/components/memory/nvm/inc
    )

    if(CONFIG_MODULE_CANNM)
        list(APPEND INCLUDE_DIR
            #CanNm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../cannm/inc
            ${GENDATA_DIR}/CanNm
        )
    endif()

    if(CONFIG_MODULE_NM)
        list(APPEND INCLUDE_DIR
            #Nm inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../nm/inc
            ${GENDATA_DIR}/Nm
        )
    endif()

    if(CONFIG_SERVM_BSWM)
        list(APPEND INCLUDE_DIR
            #BswM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/service_management/sysm_core/inc
            ${GENDATA_DIR}/sysm/BswM
        )
    endif()

    if(CONFIG_SERVM_ECUM)
        list(APPEND INCLUDE_DIR
            #EcuM inc
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../system_services/service_management/sysm_init/inc
            ${GENDATA_DIR}/sysm/EcuM
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

    # 收集 CanNm 目录下的所需 .c 文件
    file(GLOB CANSM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
         ${GENDATA_DIR}/CanSM/cansm_cfg.c
         ${GENDATA_DIR}/CanSM/cansm_lcfg.c
    )

    # 合并所有源文件
    set( CANSM_CAN_SRCS
        ${CANSM_SOURCES}
    )

    nuttx_add_library(can_cansm STATIC)
    target_sources(can_cansm PRIVATE ${CANSM_CAN_SRCS})
    target_include_directories(can_cansm PRIVATE ${INCLUDE_DIR})
    if (CONFIG_SIM_CAN_DRIVER)
        target_link_libraries(can_cansm PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(can_cansm PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(can_cansm PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(can_cansm PRIVATE j6_can_driver)
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        target_link_libraries(can_cansm PRIVATE fc7300f8mdt_can_driver)
    endif()
endif()
