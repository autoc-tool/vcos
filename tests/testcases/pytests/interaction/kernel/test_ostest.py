# This Python file uses the following encoding: utf-8
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

import pytest
import logging

from vptest.cmds import kernel

class TestOstest:
    def teardown_class(cls):
        logging.info("执行类后置, 强制sc 0")
        kernel.execute_cmd_and_check_result_of_sc(0)

    @pytest.mark.R0
    @pytest.mark.timeout(300)
    def test_ostest_on_core0(self):
        kernel.execute_cmd_and_check_result_of_sc(0)
        kernel.execute_cmd_and_check_result_of_ostest()

    @pytest.mark.R1
    @pytest.mark.timeout(2000)
    def test_ostest_on_all_cores(self):
        core_id = 0
        while True:
            if kernel.execute_cmd_and_check_result_of_sc(core_id) == kernel.SWITCH_CORE_SUCCESS:
                kernel.execute_cmd_and_check_result_of_ostest()
            else:
                break
            core_id += 1