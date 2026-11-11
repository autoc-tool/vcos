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

import os
import re
import importlib.util


OUTPUT_BUFFERS = []
OUTPUT_BUFFERS_HISTORY = []
INPUT_BUFFERS = ""
INPUT_BUFFERS_ENTER_KEY = True
INPUT_FINISHED = True
OUTPUT_BUFFERS_AS_LINES = []
LINE_MARKER = True

SHELL_LOG_PATH = ""
CONSOLE_LOG_PATH = ""

REPORT_DIR = ""
ALLURE_PARENT_NAME = "cunit"

def make_helper_descriptions():
    global SHELL_LOG_PATH, CONSOLE_LOG_PATH
    helper_messgae = ""
    if SHELL_LOG_PATH != "":
        helper_messgae += f"shell日志: {SHELL_LOG_PATH}\n"
    if CONSOLE_LOG_PATH != "":
        helper_messgae += f"窗口日志: {CONSOLE_LOG_PATH}\n"

    return helper_messgae

def find_package_path(package_name):
    spec = importlib.util.find_spec(package_name)
    if spec is not None:
        package_path = spec.origin
        package_dir_path = os.path.dirname(package_path)
        relative_path = os.path.relpath(package_dir_path)
    else:
        relative_path = ""

    return relative_path

def count_path_levels(path):
    normalized_path = os.path.normpath(path)
    path_parts = normalized_path.split(os.sep)
    path_parts = list(filter(None, path_parts))

    return len(path_parts)

def collect_cases(collect_cmd):
    output = []
    result = os.popen(collect_cmd).read()
    print(result)
    inside_package_scope = True
    for line in result.splitlines():
        line = line.strip()
        if line.startswith("<Package"):
            current_package = re.findall(r"<Package (.+?)>", line)[0]
            relative_path = find_package_path(current_package)
        elif line.startswith("<Module"):
            current_module = re.findall(r"<Module (.+?)>", line)[0]
            if count_path_levels(current_module) == 1:
                current_module = os.path.join(relative_path, current_module)
            inside_class_scope = False
        elif line.startswith("<Class"):
            class_name = re.findall(r"<Class (.+?)>", line)[0]
            output.append(f"{current_module}::{class_name}")
            inside_class_scope = True
        elif line.startswith("<Function") and not inside_class_scope:
            # If a function is found and we're not in a class scope, it's a top-level function
            function_name = re.findall(r"<Function (.+?)>", line)[0]
            output.append(f"{current_module}::{function_name}")
    return output