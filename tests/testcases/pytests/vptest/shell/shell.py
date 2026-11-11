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

import time
import logging

from vptest import utils as utils
from env.config import config

def read_shell(timeout=1, match_str="", clear_history=False):
    """
    读串口数据

    Args:
        timeout (int): 读多长时间的串口日志
        match_str (string): 匹配到字符串则终止读
        clear_history (bool): 读之前清除串口缓存

    Returns:
        string: 以字符串的形式返回读出来的所有串口日志
    """
    if clear_history:
        utils.OUTPUT_BUFFERS.clear()
    buffer_string = ""
    start_time = time.time()
    while time.time() - start_time < timeout:
        if utils.OUTPUT_BUFFERS:
            buffer_string += utils.OUTPUT_BUFFERS.pop(0)
        if match_str != "" and match_str in buffer_string:
            break
    ret_str = buffer_string.encode('utf-8', errors='ignore').decode('utf-8', errors='ignore')
    logging.info(ret_str)
    return ret_str

def read_shell_lines(number=1, timeout=0.1, match_str="", clear_history=False):
    """
    按行读串口数据

    Args:
        number (int): 读几行日志 # 可能会读到空行, 需要自行判断
        timeout (int): 读多长时间的串口日志
        match_str (string): 匹配到字符串则终止读
        clear_history (bool): 读之前清除串口缓存

    Returns:
        string: 以字符串的形式返回读出来的所有串口日志
    """
    if clear_history:
        utils.OUTPUT_BUFFERS.clear()
        utils.OUTPUT_BUFFERS_AS_LINES.clear()

    buffer_string = ""
    start_time = time.time()
    while time.time() - start_time < timeout:
        if utils.OUTPUT_BUFFERS:
            buffer_string += utils.OUTPUT_BUFFERS.pop(0)

    for line in buffer_string.splitlines():
        utils.OUTPUT_BUFFERS_AS_LINES.append(line)

    ret_str = ""
    for i in range(number):
        if utils.OUTPUT_BUFFERS_AS_LINES:
            line = utils.OUTPUT_BUFFERS_AS_LINES.pop(0)
            if line.strip() == "":
                pass
            else:
                ret_str += line
            if match_str != "" and match_str in ret_str:
                break
    ret_str = ret_str.encode('utf-8', errors='ignore').decode('utf-8', errors='ignore')
    logging.info(ret_str)
    return ret_str

def write_shell(data, enter=True, timeout=10, clear_history=True):
    """
    往串口写命令

    Args:
        data (string): 需要往串口写入的命令
        enter (bool): 是否携带回车键, 默认携带, 写入的命令才生效
        timeout (int): 写串口的超时时间, 目前写命令, 每个字符间隔0.01秒写入
        clear_history (bool): 写命令的时候, 清除串口输出缓存, 后续读的时候不会读到历史数据

    Returns:
        None
    """
    if clear_history:
        utils.OUTPUT_BUFFERS.clear()
        utils.OUTPUT_BUFFERS_AS_LINES.clear()

    if utils.INPUT_FINISHED == True:
        utils.INPUT_BUFFERS = data
        utils.INPUT_BUFFERS_ENTER_KEY = enter

    start_time = time.time()

    while (utils.INPUT_FINISHED) and (time.time() - start_time < timeout):
        pass

    utils.INPUT_FINISHED = True
    seconds = time.time() - start_time
    logging.info(f"Shell命令输入: {data}")

def set_shell_record_mode(end_with_enter = True):
    utils.LINE_MARKER = end_with_enter