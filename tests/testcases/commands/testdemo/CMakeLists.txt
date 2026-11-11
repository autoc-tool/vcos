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

if(CONFIG_TEST_CMD_TESTDEMO)

	set(CSRCS
		${CMAKE_CURRENT_SOURCE_DIR}/testdemo_main.c)

	set(INCLUDE_DIRS
		${CMAKE_CURRENT_SOURCE_DIR})

	nuttx_add_application(
		NAME
		${CONFIG_TEST_CMD_TESTDEMO_PROGNAME}
		SRCS
		${CSRCS}
		INCLUDE_DIRECTORIES
		${INCLUDE_DIRS}
		COMPILE_FLAGS
		STACKSIZE
		${CONFIG_TEST_CMD_TESTDEMO_STACKSIZE}
		PRIORITY
		${CONFIG_TEST_CMD_TESTDEMO_PRIORITY})

endif()
