# ##############################################################################
# components/maintaining/Trace/CMakeLists.txt
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

if(CONFIG_MODULE_DET) # 模块使能开关
  nuttx_add_library(det STATIC) # 当前模块编译为静态库，库名称为libautosar_os
  set(CSRCS)  # 清空CSRCS list
  set(CSRCS src/det.c) # 模块包含的c代码文件，包含相对当前CMakeList.txt文件的路径，如果有多个c文件，用' '分隔
  # 如果需要追加c源文件，可以用
  list(APPEND CSRCS ${GENDATA_DIR}/Det/det_cfg.c)
  # 设置该模块编译依赖的头文件路径
  set(INCLUDE_DIRS ${VCOS_ROOT}/components/include/
                   ${VCOS_ROOT}/components/include/autosar
                   ${VCOS_ROOT}/components/rt_framework/inc
                   ${VCOS_ROOT}/components/maintaining/vcos_log/inc
                   ${CMAKE_CURRENT_LIST_DIR}/src
                   ${GENDATA_DIR}/Det
                   ${GENDATA_DIR}/Memmap
                   ${GENDATA_DIR}/Rte
                   ${GENDATA_DIR}/Os
                   ${VCOS_APP_DIR}/apps
                   ${NUTTX_DIR}/include/
                   ${CMAKE_CURRENT_LIST_DIR}/inc)

  # 设置autosar_os库编译包含的CSRCS
  target_sources(det PRIVATE ${CSRCS})
  target_include_directories(det PRIVATE ${INCLUDE_DIRS} ${VENDOR_THIRD_PARTY_DRIVER_INCDIR})
  target_include_directories(det PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/inc)

endif()