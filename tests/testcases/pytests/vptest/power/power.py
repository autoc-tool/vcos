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

class Power:
    def power_on_board(self, delay):
        """
        开发板上电

        Args:
        delay (float): 操作继电器的延时时间, 默认值为 0.1 秒, 使用默认值继电器上下电更稳定
        """
        cmd = config.RELAY.PORT.ON_CMD
        control_power = serial.Serial(port=config.RELAY.PORT, baudrate=config.RELAY.BAUDRATE)
        control_power.write(bytes.fromhex(cmd))
        if delay != 0:
            time.sleep(delay)
        control_power.close()

    def power_off_board(self, delay):
        """
        开发板下电

        Args:
        delay (float): 操作继电器的延时时间, 默认值为 0.1 秒, 使用默认值继电器上下电更稳定
        """
        cmd = config.RELAY.PORT.OFF_CMD
        control_power = serial.Serial(port=config.RELAY.PORT, baudrate=config.RELAY.BAUDRATE)
        control_power.write(bytes.fromhex(cmd))
        if delay != 0:
            time.sleep(delay)
        control_power.close()

    def reset_board(self, delay=0.1, sep=1.0):
        self.power_off_board(delay)
        time.sleep(sep)
        self.power_on_board(delay)

    def reset_board_and_check_alive(self, delay=0.1, sep=1.0, retry = 3):
        """
        硬重启开发板(电源上下电)

        Args:
        delay (float): 操作继电器的延时时间, 默认值为 0.1 秒, 使用默认值继电器上下电更稳定
        sep (float): 下电到上电之间的间隔时间, 默认值为 1 秒, 使用默认值继电器上下电更稳定
        retry (int): 重启n次, 直到version命令有回显, 如果没有回显则assert失败
        """
        for i in range(retry):
            self.reset_board(delay, sep)
            time.sleep(1) # 等待1秒，等待系统完全启动
            shell.write_shell("\n", clear_history=True)
            output = shell.read_shell(match_str=config.SHELL.PROMPT)
            if config.SHELL.PROMPT in output:
                assert True, "system is alive after restart!"
                break
            else:
                logging.warning(f"RESET {i+1} times, system is not alive!")

power_control = Power()
