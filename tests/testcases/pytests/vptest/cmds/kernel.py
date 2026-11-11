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

import serial
import time
import logging

from env.config import config
from vptest.shell import shell

SWITCH_CORE_FAILED = "switch core failed"
SWITCH_CORE_SUCCESS = "switch core success"

def execute_cmd_and_check_result_of_hello():
    shell.write_shell("hello")
    assert "Hello, World" in shell.read_shell(), "no reply from command 'hello'"

def execute_cmd_on_core_and_check_result_of_ps(core_id = 0):
    shell.write_shell("ps")
    console_output = shell.read_shell(match_str="hpwork", timeout=3)
    assert f"CPU{core_id} IDLE" in console_output

def execute_cmd_and_check_result_of_sc(core_id = 0):
    shell.write_shell(f"sc {core_id}")
    console_output = shell.read_shell()
    if f"sc {core_id} failed" in console_output:
        return SWITCH_CORE_FAILED
    else:
        execute_cmd_on_core_and_check_result_of_ps(core_id)
        return SWITCH_CORE_SUCCESS

def execute_cmd_and_check_result_of_ostest():
    shell.write_shell("ostest", clear_history=True)
    console_output = shell.read_shell(timeout=180, match_str="ostest_main: Exiting with status 0")
    assert "ostest_main: Exiting with status 0" in console_output