# ##############################################################################
# components/diagnostics/service/dem/CMakeLists.txt
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

if(CONFIG_MODULE_DEM) # 模块使能开关
    nuttx_add_library(diag_dem STATIC) # 当前模块编译为静态库，库名称为libdiag_dem
    set(CSRCS)  # 清空CSRCS list

    # 收集 dem 目录下的所需 .c 文件
    file(GLOB DEM_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    )
    set(CSRCS ${DEM_SOURCES}
              ${GENDATA_DIR}/Dem/dem_lcfg.c
              ${VCOS_APP_DIR}/apps/bsw_user_callout/dem_callout_stub.c) # 模块包含的c代码文件，包含相对当前CMakeList.txt文件的路径，如果有多个c文件，用' '分隔

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
                    ${GENDATA_DIR}/Dem
                    ${VCOS_APP_DIR}/apps
                    ${VCOS_APP_DIR}/apps/bsw_user_callout/
                    ${CMAKE_CURRENT_LIST_DIR}/inc)

    if(CONFIG_MODULE_NVM)
        list(APPEND INCLUDE_DIRS
            #nvm inc
            ${VCOS_ROOT}/components/memory/nvm/inc
            ${GENDATA_DIR}/NvM
        )
    endif()

    # 设置diag_dem库编译包含的CSRCS
    target_sources(diag_dem PRIVATE ${CSRCS})
    target_include_directories(diag_dem PRIVATE ${INCLUDE_DIRS} ${VENDOR_THIRD_PARTY_DRIVER_INCDIR})

endif()
