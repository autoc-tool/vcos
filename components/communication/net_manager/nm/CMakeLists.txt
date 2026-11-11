# ##############################################################################
# components/communication/net_manager/nm/CMakeLists.txt
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

if(CONFIG_MODULE_NM)
    # 头文件引入
    set( INCLUDE_DIR
        # GENERAL
        ${VCOS_ROOT}/components/include/autosar/
        ${VCOS_ROOT}/components/include/
        ${GENDATA_DIR}/Os
        ${GENDATA_DIR}/Rte
        ${GENDATA_DIR}/ComM
        ${GENDATA_DIR}/Nm
        ${GENDATA_DIR}/CanNm
        ${GENDATA_DIR}/CanSM
        ${GENDATA_DIR}/CanIf
        ${GENDATA_DIR}/Com
        ${GENDATA_DIR}/PduR
        ${GENDATA_DIR}/Memmap
        ${GENDATA_DIR}/NvM
        ${VCOS_APP_DIR}/apps
        ${VCOS_ROOT}/components/rt_framework/inc

        #vcos log
        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        #NM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #CanNM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../cannm/inc
        #ComM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../comm/inc
        #CanSM inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../cansm/inc
        #CanIf inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../net_dev/canif/inc
        #Com inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../rt_com/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../pdu_router/inc
        #NvM inc
        ${VCOS_ROOT}/components/memory/nvm/inc
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
            #Dcm inc
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

    # 收集 NM 目录下的所需 .c 文件
    file(GLOB NM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    # 合并所有源文件
    set( NET_NM_SRCS
        ${NM_SOURCES}
        ${GENDATA_DIR}/Nm/nm_cfg.c
    )

    nuttx_add_library(net_nm STATIC)
    target_sources(net_nm PRIVATE ${NET_NM_SRCS})
    target_include_directories(net_nm PRIVATE ${INCLUDE_DIR})
    if (CONFIG_SIM_CAN_DRIVER)
        target_link_libraries(net_nm PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(net_nm PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(net_nm PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(net_nm PRIVATE j6_can_driver)
    endif()
endif()
