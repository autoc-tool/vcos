# ##############################################################################
# components/communication/rt_com/CMakeLists.txt
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

if(CONFIG_MODULE_COM)
    # 头文件引入
    set( INCLUDE_DIR
        # GENERAL
        ${VCOS_ROOT}/components/include/autosar/
        ${VCOS_ROOT}/components/include/
        ${GENDATA_DIR}/Os
        ${GENDATA_DIR}/Com
        ${GENDATA_DIR}/Rte
        ${GENDATA_DIR}/PduR
        ${GENDATA_DIR}/CanIf
        ${GENDATA_DIR}/Memmap
        ${VCOS_APP_DIR}/apps
        ${VCOS_APP_DIR}/apps/rte_datamapping_apps
        ${VCOS_ROOT}/components/rt_framework/inc

        #vcos log
        ${VCOS_ROOT}/components/maintaining/vcos_log/inc
        #Com inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../rt_com/inc
        #CanIf inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../net_dev/canif/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #PduR inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../pdu_router/inc
    )

    if(CONFIG_ARCH_CHIP_E3650)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_E3XX_DIR}
        )
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_FC7300F8MDT_DIR}
        )
    endif()

    # 收集 Com 目录下的所需 .c 文件
    file(GLOB RTCOM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )

    # 合并所有源文件
    set( RT_COM_SRCS
        ${RTCOM_SOURCES}
        ${GENDATA_DIR}/Com/com_cfg.c
        ${GENDATA_DIR}/Com/com_pbcfg.c
    )

    nuttx_add_library(rt_com STATIC)
    target_sources(rt_com PRIVATE ${RT_COM_SRCS})
    target_include_directories(rt_com PRIVATE ${INCLUDE_DIR})
    if (CONFIG_SIM_CAN_DRIVER)
        target_link_libraries(rt_com PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(rt_com PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(rt_com PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(rt_com PRIVATE j6_can_driver)
    elseif(CONFIG_ARCH_CHIP_FC7300F8MDT)
        target_link_libraries(rt_com PRIVATE fc7300f8mdt_can_driver)
    endif()
endif()
