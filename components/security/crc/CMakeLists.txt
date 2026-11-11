# ##############################################################################
# components/security/crc/CMakeLists.txt
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

if(CONFIG_SECURITY_CRC) # 模块使能开关
  nuttx_add_library(security_crc STATIC) # 当前模块编译为静态库，库名称为libsecurity_crc
  set(CSRCS)  # 清空CSRCS list
  set(CSRCS src/crc_8.c) # 模块包含的c代码文件，包含相对当前CMakeList.txt文件的路径，如果有多个c文件，用' '分隔
  # 如果需要追加c源文件，可以用
  list(APPEND CSRCS src/crc_16.c src/crc_32.c src/crc_64.c)
  # 设置该模块编译依赖的头文件路径
  set(INCLUDE_DIRS ${VCOS_ROOT}/components/include/
                   ${VCOS_ROOT}/components/include/autosar
                   ${VCOS_ROOT}/components/rt_framework/inc
                   ${NUTTX_APPS_DIR}/security/crc/inc
                   ${GENDATA_DIR}/Crc
                   ${GENDATA_DIR}/Memmap)

  target_sources(security_crc PRIVATE ${CSRCS})
  target_include_directories(security_crc PRIVATE ${INCLUDE_DIRS} ${VENDOR_THIRD_PARTY_DRIVER_INCDIR})
  target_include_directories(security_crc PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/inc)
endif()