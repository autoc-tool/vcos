# ##############################################################################
# components/system_services/service_management/sysm_init/CMakeLists.txt
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
if(CONFIG_SERVM_BSWM) # 模块使能开关
  nuttx_add_library(sysm_core STATIC) # 当前模块编译为静态库，库名称为libautosar_os
  set(CSRCS)  # 清空CSRCS list
  set(CSRCS src/bswm.c) # 模块包含的c代码文件，包含相对当前CMakeList.txt文件的路径，如果有多个c文件，用' '分隔
  # 如果需要追加c源文件，可以用
  list(APPEND CSRCS ${GENDATA_DIR}/sysm/BswM/bswm_cfg.c
                    ${GENDATA_DIR}/sysm/BswM/bswm_pbcfg.c)
  # 设置该模块编译依赖的头文件路径
  set(INCLUDE_DIRS ${VCOS_ROOT}/components/include/autosar
                   ${VCOS_ROOT}/components/rt_framework/inc
                   ${VCOS_ROOT}/components/maintaining/vcos_log/inc
                   ${VCOS_ROOT}/components/memory/memif/inc
                   ${VCOS_ROOT}/components/memory/nvm/inc
                   ${VCOS_ROOT}/components/system_services/service_management/sysm_init/inc
                   ${VCOS_ROOT}/components/include
                    ${VCOS_ROOT}/components/maintaining/det/inc
                   ${GENDATA_DIR}/Os
                   ${GENDATA_DIR}/Memmap
                   ${GENDATA_DIR}/MemIf
                   ${GENDATA_DIR}/Rte
                   ${GENDATA_DIR}/Det
                   ${GENDATA_DIR}/NvM
                   ${GENDATA_DIR}/SchM
                   ${GENDATA_DIR}/sysm/BswM
                   ${VCOS_APP_DIR}/apps
                   ${VCOS_APP_DIR}/apps/bsw_user_callout/
                   ${CMAKE_CURRENT_LIST_DIR}/src
                   ${NUTTX_DIR}/include/)

  if(CONFIG_SERVM_ECUM)
    list(APPEND INCLUDE_DIRS ${GENDATA_DIR}/sysm/EcuM)
  endif()

  if(CONFIG_MODULE_CANTSYN)
      list(APPEND INCLUDE_DIR
          ${GENDATA_DIR}/CanTSyn
      )
  endif()

  if(CONFIG_MODULE_CANSM)
    list(APPEND INCLUDE_DIRS
        #CanSM inc
        ${VCOS_ROOT}/components/communication/net_manager/cansm/inc
        ${GENDATA_DIR}/CanSM
    )
  endif()

  if(CONFIG_MODULE_CANNM)
    list(APPEND INCLUDE_DIRS
        #CanNm inc
        ${VCOS_ROOT}/components/communication/net_manager/cannm/inc
        ${GENDATA_DIR}/CanNm
    )
  endif()

  if(CONFIG_MODULE_COMM)
    list(APPEND INCLUDE_DIRS
        #ComM inc
        ${VCOS_ROOT}/components/communication/net_manager/comm/inc
        ${GENDATA_DIR}/ComM
    )
  endif()

  if(CONFIG_MODULE_NM)
    list(APPEND INCLUDE_DIRS
        #NM inc
        ${VCOS_ROOT}/components/communication/net_manager/nm/inc
        ${GENDATA_DIR}/Nm
    )
  endif()

  if(CONFIG_MODULE_CANIF)
    list(APPEND INCLUDE_DIRS
        #CanIf inc
        ${VCOS_ROOT}/components/communication/net_dev/canif/inc
        ${GENDATA_DIR}/CanIf
    )
  endif()

  if(CONFIG_MODULE_COM)
    list(APPEND INCLUDE_DIRS
        #Com inc
        ${VCOS_ROOT}/components/communication/rt_com/inc
        ${GENDATA_DIR}/Com
    )
  endif()

  if(CONFIG_MODULE_PDUR)
    list(APPEND INCLUDE_DIRS
        #PduR inc
        ${VCOS_ROOT}/components/communication/pdu_router/inc
        ${GENDATA_DIR}/PduR
    )
  endif()

  # diag
  if(CONFIG_MODULE_DEM)
    list(APPEND INCLUDE_DIRS
      ${VCOS_ROOT}/components/diagnostics/service/dem/inc
      ${GENDATA_DIR}/Dem
    )
  endif()

  if(CONFIG_MODULE_DCM)
    list(APPEND INCLUDE_DIRS
      ${VCOS_ROOT}/components/diagnostics/service/dcm/inc
      ${GENDATA_DIR}/Dcm
    )
  endif()

  if(CONFIG_MODULE_CANTP)
    list(APPEND INCLUDE_DIRS
      ${VCOS_ROOT}/components/diagnostics/transport/cantp/inc
      ${GENDATA_DIR}/CanTp
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
  endif()

  # 设置autosar_os库编译包含的CSRCS
  target_sources(sysm_core PRIVATE ${CSRCS})
  target_include_directories(sysm_core PRIVATE ${INCLUDE_DIRS})
  target_include_directories(sysm_core PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/inc)
  if (CONFIG_SIM_CAN_DRIVER)
      target_link_libraries(sysm_core PRIVATE sim_can)
  elseif(CONFIG_ARCH_CHIP_CUSTOM_TC397)
      target_link_libraries(sysm_core PRIVATE can_driver)
  elseif(CONFIG_ARCH_CHIP_E3650)
      target_link_libraries(sysm_core PRIVATE e3_can_driver)
  elseif(CONFIG_ARCH_CHIP_J6E)
      target_link_libraries(sysm_core PRIVATE j6_can_driver)
  endif()

endif()
