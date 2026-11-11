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

if(CONFIG_MODULE_NVM) # 模块使能开关
  nuttx_add_library(mem_nvm STATIC) # 当前模块编译为静态库，库名称为libautosar_os
  set(CSRCS)  # 清空CSRCS list
  set(CSRCS src/nvm.c) # 模块包含的c代码文件，包含相对当前CMakeList.txt文件的路径，如果有多个c文件，用' '分隔
  # 如果需要追加c源文件，可以用
  list(APPEND CSRCS src/nvm_crc.c src/nvm_fsm.c src/nvm_fsmact.c src/nvm_fsmevent.c src/nvm_queue.c  ${GENDATA_DIR}/NvM/nvm_cfg.c)
  # 设置该模块编译依赖的头文件路径
  set(INCLUDE_DIRS ${VCOS_ROOT}/components/include/autosar
                   ${VCOS_ROOT}/components/rt_framework/inc
                   ${VCOS_ROOT}/components/maintaining/vcos_log/inc
                   ${VCOS_ROOT}/components/memory/memif/inc
                   ${VCOS_ROOT}/components/include
                   ${VCOS_ROOT}/components/security/crc/inc
                   ${NUTTX_APPS_DIR}/include/memory/memif/inc
                   ${GENDATA_DIR}/Os
                   ${GENDATA_DIR}/Memmap
                   ${GENDATA_DIR}/MemIf
                   ${GENDATA_DIR}/Rte
                   ${GENDATA_DIR}/NvM
                   ${GENDATA_DIR}/SchM
                   ${GENDATA_DIR}/Crc
                   ${VCOS_APP_DIR}/apps
                   ${CMAKE_CURRENT_LIST_DIR}/src
                   ${NUTTX_DIR}/include/)

  if(CONFIG_MODULE_DEM)
      list(APPEND INCLUDE_DIRS
          #dem inc
          ${VCOS_ROOT}/components/diagnostics/service/dem/inc
          ${GENDATA_DIR}/Dem
      )
  endif()

  if(CONFIG_MODULE_DCM)
      list(APPEND INCLUDE_DIRS
          #dcm inc
          ${VCOS_ROOT}/components/diagnostics/service/dcm/inc
          ${GENDATA_DIR}/Dcm
      )
  endif()

  # 设置autosar_os库编译包含的CSRCS
  target_sources(mem_nvm PRIVATE ${CSRCS})
  target_include_directories(mem_nvm PRIVATE ${INCLUDE_DIRS} ${VENDOR_THIRD_PARTY_DRIVER_INCDIR})
  target_include_directories(mem_nvm PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/inc)

endif()