# ##############################################################################
# Copyright (c) 2025 Li Auto Inc. and its affiliates
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ##############################################################################

if(DEFINED BOARD_NAME)
    set(GENDATA_DIR "${VCOS_APP_DIR}/system_cfg/${BOARD_NAME}/gen_data" CACHE INTERNAL "project gendata directory")
else()
    set(GENDATA_DIR "${VCOS_APP_DIR}/cfg_prj/gen_data" CACHE INTERNAL "project gendata directory")
endif()

list(APPEND CMAKE_MODULE_PATH ${NUTTX_DIR}/../build/vcos_makesystem/cmake_framwork)
include(vcos_is_subdirectory)

is_subdirectory(
    ${VCOS_APP_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
    IS_SUBDIR
)

if(IS_SUBDIR)
    set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    nuttx_add_subdirectory()
    nuttx_generate_kconfig(MENUDESC "Tests")
endif()
