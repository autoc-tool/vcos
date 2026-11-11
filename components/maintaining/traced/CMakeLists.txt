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
if(CONFIG_TRACED)
	set(CSRCS src/traced.c
						src/trace_eth_wrapper.c)

	set(INCLUDE_DIRS ${NUTTX_DIR}/sched
									 ${VCOS_ROOT}/components/include/autosar
									 ${VCOS_ROOT}/components/rt_framework/inc
									 ${NUTTX_DIR}/../components/include/
									 ${NUTTX_DIR}/../drivers/ETH/MAC/Tricore/TC3xx/
									 ${VCOS_ROOT}/components/communication/net_protocols/sockets
									 ${NUTTX_APPS_DIR}/system/trace/
									 ${NUTTX_DIR}/include/)

  list(APPEND INCLUDE_DIRS ${GENDATA_DIR}/Os
													 ${GENDATA_DIR}/Memmap
													 ${GENDATA_DIR}/TcpIp
													 ${VCOS_APP_DIR}/apps)

	if(CONFIG_ARCH_CHIP_E3650)
		list(APPEND INCLUDE_DIRS
			${ETH_STACK_E3XX_GETHMAC_DIR}/e3650
		)
	endif()

	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-Wno-undef)
		add_compile_options(-Werror -pedantic)
	endif()
	nuttx_add_library(traced STATIC)
	target_sources(traced PRIVATE ${CSRCS})
if(NOT "${CONFIG_TRACE_VAR_SECTION}" STREQUAL "")
    target_compile_options(traced PRIVATE -DTRACE_VAR_SECTION="${CONFIG_TRACE_VAR_SECTION}")
endif()
	target_include_directories(traced PRIVATE ${INCLUDE_DIRS})
	target_include_directories(traced PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)
endif()