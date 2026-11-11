# ##############################################################################
# components/security/e2e/CMakeLists.txt
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

if(CONFIG_MODULE_E2E) # 模块使能开关
  nuttx_add_library(e2e STATIC) # 当前模块编译为静态库，库名称为libautosar_os
  set(CSRCS)  # 清空CSRCS list
  set(CSRCS src/e2e.c src/e2e_p01.c src/e2e_p02.c src/e2e_p04.c src/e2e_p05.c src/e2e_sm.c)
  set(INCLUDE_DIRS ${VCOS_ROOT}/components/include/
                   ${VCOS_ROOT}/components/include/autosar
                   ${VCOS_ROOT}/components/rt_framework/inc
                   ${VCOS_ROOT}/components/security/e2e/inc
                   ${VCOS_ROOT}/components/security/crc/inc
                   ${GENDATA_DIR}/Crc
                   ${CMAKE_CURRENT_LIST_DIR}/src
                   ${NUTTX_DIR}/include/
                   ${GENDATA_DIR}/Memmap)

  # 设置autosar_os库编译包含的CSRCS
  target_sources(e2e PRIVATE ${CSRCS})
  target_include_directories(e2e PRIVATE ${INCLUDE_DIRS} ${VENDOR_THIRD_PARTY_DRIVER_INCDIR})
  target_include_directories(e2e PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/inc)

endif()