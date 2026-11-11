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
import threading
import serial
import time
import os
import allure
import json
from pathlib import Path
from datetime import datetime

from env.config import config
from vptest import utils
from vptest.shell import shell
from vptest.power import power

buffer_lock = threading.Lock()

def read_shell_data(stop_event):
    ser = None
    try:
        ser = serial.Serial(config.SHELL.PORT, config.SHELL.BAUDRATE, timeout=1, write_timeout=0.1, inter_byte_timeout=0.1)
        logging.info(f"open shell successful: {config.SHELL.PORT} {config.SHELL.BAUDRATE}")
    except serial.SerialException as e:
        logging.error(f"can not open shell '{config.SHELL.PORT}', please check: {e}")
        exit(1)

    utils.OUTPUT_BUFFERS_HISTORY.append(f"====== start recording shell log {datetime.now()} ======\n")
    left_data = ""
    while not stop_event.is_set():
        try:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting).decode('utf-8', errors="ignore")
                with buffer_lock:
                    if data:
                        utils.OUTPUT_BUFFERS_HISTORY.append(data)
                        left_data = left_data + data

                        lines = left_data.split("\n")
                        if len(lines) == 0: # no data
                            pass
                        elif len(lines) == 1: # only one line
                            if left_data.endswith("\n"): # whole line
                                utils.OUTPUT_BUFFERS.append(left_data)
                                left_data = ""
                            else: # not a whole line
                                pass
                        else: # multi lines
                            if left_data.endswith("\n"): # last line is a whole line
                                for line in lines:
                                    utils.OUTPUT_BUFFERS.append(line + "\n")
                                left_data = ""
                            else: # last line is not a whole line
                                for line in lines[:-1]:
                                    utils.OUTPUT_BUFFERS.append(line + "\n")
                                left_data = lines[-1]
                        if left_data != "" and utils.LINE_MARKER == False:
                            utils.OUTPUT_BUFFERS.append(left_data)
                            left_data = ""
            else:
                if utils.INPUT_BUFFERS != "":
                    ser.flushOutput()
                    if isinstance(utils.INPUT_BUFFERS, bytes):
                        ser.write(utils.INPUT_BUFFERS)
                        if utils.INPUT_BUFFERS_ENTER_KEY:
                            ser.write('\n'.encode("UTF-8"))
                    else:
                        strlist = list(str(utils.INPUT_BUFFERS))
                        for i in strlist:
                            ser.write(i.encode("UTF-8"))
                            time.sleep(0.01)
                        if utils.INPUT_BUFFERS_ENTER_KEY:
                            ser.write('\n'.encode("UTF-8"))
                    ser.flush()
                    utils.INPUT_BUFFERS = ""
                    utils.INPUT_BUFFERS_ENTER_KEY = False
                    utils.INPUT_FINISHED = False
                else:
                    pass
        except Exception as ex:
            print(ex)
    if ser:
        ser.close()
    utils.OUTPUT_BUFFERS_HISTORY.append(f"====== finished recording shell logs {datetime.now()} ======\n")

def write_shell_data(stop_event):
    current_file_path = os.path.abspath(__file__)
    current_directory = os.path.dirname(current_file_path)
    os.makedirs(os.path.join(os.path.dirname(current_directory), "logs"), exist_ok=True)
    now = datetime.now()
    shell_log_path = os.path.join(os.path.dirname(current_directory), "logs", f"{now.year}-{now.month}-{now.day:02d}_{now.hour:02d}-{now.minute:02d}-{now.second:02d}_shell.log")
    if shell_log_path:
        logging.info(f"check whole shell log: {shell_log_path}")

    while not stop_event.is_set():
        shell_logs = ""
        if utils.OUTPUT_BUFFERS_HISTORY:
            with open(file=shell_log_path, mode='a', encoding='utf-8') as f:
                while utils.OUTPUT_BUFFERS_HISTORY:
                    shell_logs += utils.OUTPUT_BUFFERS_HISTORY.pop(0)
                f.write(shell_logs)
        time.sleep(1)

    logging.info(f"check whole shell log: {shell_log_path}")

@pytest.fixture(scope='module', autouse=True)
def check_system_up(pytestconfig):
    studio = pytestconfig.getoption("--no_shell")
    if not studio:
        shell.write_shell("\n", clear_history=True)
        output = shell.read_shell(match_str=config.SHELL.PROMPT)
        if config.SHELL.PROMPT in output:
            logging.info("System is up!")
        else:
            logging.error("System is not up!")
    else:
        logging.info("Tests not need shell port")

@pytest.fixture(scope='session', autouse=True)
def interaction_setting(pytestconfig):
    studio = pytestconfig.getoption("--no_shell")
    if not studio:
        logging.info(f"Open shell {config.SHELL.PORT} {config.SHELL.BAUDRATE}")
        stop_event = threading.Event()
        read_thread = threading.Thread(target=read_shell_data, args=(stop_event,))
        read_thread.start()

        write_thread = threading.Thread(target=write_shell_data, args=(stop_event,))
        write_thread.start()

    report_dir = pytestconfig.getoption("--alluredir")
    if report_dir:
        utils.REPORT_DIR = report_dir

    yield

    if not studio:
        try:
            time.sleep(3)
            stop_event.set()
            time.sleep(2)
            read_thread.join()
            write_thread.join()
            logging.info("Close shell")

        except Exception as ex:
            logging.error(f"{ex}")


def pytest_runtest_call(item):
    logging.info(utils.make_helper_descriptions())
    logging.info(f"开始测试")

@pytest.fixture(scope="function", autouse=True)
def links(pytestconfig):
    shell_log_path = pytestconfig.getoption("--shell_log")
    console_log_path = pytestconfig.getoption("--console_log")
    if shell_log_path:
        utils.SHELL_LOG_PATH = shell_log_path
        allure.attach(f"{shell_log_path}", name="shell logs", attachment_type=allure.attachment_type.URI_LIST)
    if console_log_path:
        utils.CONSOLE_LOG_PATH = console_log_path
        allure.attach(f"{console_log_path}", name="console logs", attachment_type=allure.attachment_type.URI_LIST)

@pytest.hookimpl(trylast=True)
def pytest_sessionfinish(session, exitstatus):
    """在测试会话结束后处理allure结果"""
    parent_name = session.config.getoption("--parent_name")
    allure_dir = session.config.getoption("--alluredir")

    if allure_dir and os.path.exists(allure_dir) and parent_name:
        utils.ALLURE_PARENT_NAME = parent_name
        # 遍历所有结果文件
        for file_path in Path(allure_dir).glob("*-result.json"):
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    data = json.load(f)

                if "historyId" in data.keys():
                    del data['historyId']

                # 处理labels
                new_labels = []
                parent_suite_added = False
                new_suite = ""

                for label in data.get("labels", []):
                    if label.get("name") == "parentSuite":
                        new_suite = label["value"]

                for label in data.get("labels", []):
                    if label.get("name") == "suite":
                        new_suite = new_suite + "." + label["value"]

                for label in data.get("labels", []):
                    if label.get("name") == "parentSuite":
                        if not parent_suite_added:
                            # 只保留第一个，并替换值
                            label["value"] = parent_name
                            new_labels.append(label)
                            parent_suite_added = True
                    elif label.get("name") == "suite":
                        # 只保留第一个，并替换值
                        label["value"] = new_suite
                        new_labels.append(label)
                    else:
                        new_labels.append(label)

                # 如果没有parentSuite，添加一个
                if not parent_suite_added:
                    new_labels.append({
                        "name": "parentSuite",
                        "value": parent_name
                    })

                data["labels"] = new_labels

                # 写回文件
                with open(file_path, 'w', encoding='utf-8') as f:
                    json.dump(data, f, ensure_ascii=False, indent=2)

            except Exception as e:
                logging.error(f"处理文件 {file_path} 时出错: {e}")