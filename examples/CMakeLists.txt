# ##############################################################################
# applications/CMakeLists.txt
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################


# include corresponding subdirectory
if(DEFINED BOARD_NAME)
    set(GENDATA_DIR "${VCOS_APP_DIR}/system_cfg/${BOARD_NAME}/gen_data" CACHE INTERNAL "project gendata directory")
else()
    set(GENDATA_DIR "${VCOS_APP_DIR}/cfg_prj/gen_data" CACHE INTERNAL "project gendata directory")
endif()

if(${BOARD_CONFIG} MATCHES "/boards/sim/")
    set(GENDATA_CONFIG ${NUTTX_DEFCONFIG} CACHE INTERNAL "project GENDATA_CONFIG directory")
else()
    set(GENDATA_CONFIG ${GENDATA_DIR}/gen_data.config CACHE INTERNAL "project GENDATA_CONFIG directory")
endif()

list(APPEND CMAKE_MODULE_PATH ${NUTTX_DIR}/../build/vcos_makesystem/cmake_framwork)
include(vcos_is_subdirectory)

is_subdirectory(
    ${VCOS_APP_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
    IS_SUBDIR
)

if(IS_SUBDIR)
    if(DEFINED APPS_DIR)
        add_subdirectory(${VCOS_APP_DIR})
    else()
        message(FATAL_ERROR "APPS_DIR not defined")
    endif()
endif()
