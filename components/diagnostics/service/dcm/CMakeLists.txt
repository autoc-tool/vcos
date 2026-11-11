# ##############################################################################
# components/diagnostics/service/dcm/CMakeLists.txt
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

if(CONFIG_MODULE_DCM) # 模块使能开关
    nuttx_add_library(diag_dcm STATIC) # 当前模块编译为静态库，库名称为libdiag_dcm
    set(CSRCS)  # 清空CSRCS list

    # 收集 dcm 目录下的所需 .c 文件
    file(GLOB DCM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )
    set(CSRCS ${DCM_SOURCES}
            ${GENDATA_DIR}/Dcm/dcm_lcfg.c
            ${VCOS_APP_DIR}/apps/bsw_user_callout/dcm_callout_stubs.c) # 模块包含的c代码文件，包含相对当前CMakeList.txt文件的路径，如果有多个c文件，用' '分隔

    # 设置该模块编译依赖的头文件路径
    set(INCLUDE_DIRS ${NUTTX_DIR}/include
                    ${VCOS_ROOT}/components/include/autosar
                    ${VCOS_ROOT}/components/rt_framework/inc
                    ${VCOS_ROOT}/components/maintaining/vcos_log/inc
                    ${VCOS_ROOT}/components/include
                    ${GENDATA_DIR}/Memmap
                    ${GENDATA_DIR}/Os
                    ${GENDATA_DIR}/Rte
                    ${GENDATA_DIR}/SchM
                    ${GENDATA_DIR}/Dcm
                    ${VCOS_APP_DIR}/apps
                    ${VCOS_APP_DIR}/apps/bsw_user_callout/
                    ${CMAKE_CURRENT_LIST_DIR}/inc)

    if(CONFIG_MODULE_DEM)
        list(APPEND INCLUDE_DIRS
            #dem inc
            ${VCOS_ROOT}/components/diagnostics/service/dem/inc
            ${GENDATA_DIR}/Dem
        )
    endif()

    if(CONFIG_MODULE_NVM)
        list(APPEND INCLUDE_DIRS
            #nvm inc
            ${VCOS_ROOT}/components/memory/nvm/inc
            ${GENDATA_DIR}/NvM
        )
    endif()

    if(CONFIG_MODULE_PDUR)
        list(APPEND INCLUDE_DIRS
            #pdur inc
            ${VCOS_ROOT}/components/communication/pdu_router/inc
            ${GENDATA_DIR}/PduR
        )
    endif()

    if(CONFIG_MODULE_COMM)
        list(APPEND INCLUDE_DIRS
            #comm inc
            ${VCOS_ROOT}/components/communication/net_manager/comm/inc
            ${GENDATA_DIR}/ComM
        )
    endif()

    if(CONFIG_MODULE_NM)
        list(APPEND INCLUDE_DIRS
            #nm inc
            ${VCOS_ROOT}/components/communication/net_manager/nm/inc
            ${GENDATA_DIR}/Nm
        )
    endif()

    if(CONFIG_MODULE_CANSM)
        list(APPEND INCLUDE_DIRS
            #cansm inc
            ${VCOS_ROOT}/components/communication/net_manager/cansm/inc
            ${GENDATA_DIR}/CanSM
        )
    endif()

    if(CONFIG_MODULE_CANIF)
        list(APPEND INCLUDE_DIRS
            #canif inc
            ${VCOS_ROOT}/components/communication/net_dev/canif/inc
            ${GENDATA_DIR}/CanIf
        )
    endif()

    if(CONFIG_SERVM_BSWM)
        list(APPEND INCLUDE_DIRS
            #BswM inc
            ${VCOS_ROOT}/components/system_services/service_management/sysm_core/inc
            ${GENDATA_DIR}/sysm/BswM
        )
    endif()

    if(CONFIG_SERVM_ECUM)
        list(APPEND INCLUDE_DIRS
            #EcuM inc
            ${VCOS_ROOT}/components/system_services/service_management/sysm_init/inc
            ${GENDATA_DIR}/sysm/EcuM
        )
    endif()

    # Can driver
    if (CONFIG_ARCH_SIM)
        list(APPEND INCLUDE_DIRS
            ${SIM_DRIVERS_CANSTACK_INCLUDE_DIRS}
        )
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        list(APPEND INCLUDE_DIRS
            ${CAN_STACK_TC3XX_DIR}
        )
    elseif(CONFIG_ARCH_CHIP_E3650)
        list(APPEND INCLUDE_DIRS
            ${CAN_STACK_E3XX_DIR}
        )
    elseif(CONFIG_ARCH_CHIP_J6E)
        list(APPEND INCLUDE_DIR
            ${CAN_STACK_J6E_DIR}
        )
    endif()

    # 设置diag_dcm库编译包含的CSRCS
    target_sources(diag_dcm PRIVATE ${CSRCS})
    target_include_directories(diag_dcm PRIVATE ${INCLUDE_DIRS} ${VENDOR_THIRD_PARTY_DRIVER_INCDIR})

    if (CONFIG_SIM_CAN_DRIVER)
        target_link_libraries(diag_dcm PRIVATE sim_can)
    elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
        target_link_libraries(diag_dcm PRIVATE can_driver)
    elseif(CONFIG_ARCH_CHIP_E3650)
        target_link_libraries(diag_dcm PRIVATE e3_can_driver)
    elseif(CONFIG_ARCH_CHIP_J6E)
        target_link_libraries(diag_dcm PRIVATE j6_can_driver)
    endif()

endif()
