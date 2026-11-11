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

if(CONFIG_UNITY_FRAMEWORK)

	set(CSRCS
		${CMAKE_CURRENT_SOURCE_DIR}/ut_main.c
		${CMAKE_CURRENT_SOURCE_DIR}/ut_init.c
		${CMAKE_CURRENT_SOURCE_DIR}/unity/unity.c)

	set(INCLUDE_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}/unity)

	nuttx_add_application(
		NAME
		${CONFIG_UNITY_FRAMEWORK_PROGNAME}
		SRCS
		${CSRCS}
		INCLUDE_DIRECTORIES
		${INCLUDE_DIRS}
		COMPILE_FLAGS
		STACKSIZE
		${CONFIG_UNITY_FRAMEWORK_STACKSIZE}
		PRIORITY
		${CONFIG_UNITY_FRAMEWORK_PRIORITY})

endif()
