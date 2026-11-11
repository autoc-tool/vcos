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

import logging
import pytest

def pytest_addoption(parser):
    parser.addoption("--shell_log", action="store", default="", help="shell log path")
    parser.addoption("--console_log", action="store", default="", help="console log path")
    parser.addoption("--no_shell", action="store_true", help="run studio cases")
    parser.addoption("--parent_name", action="store", default="", help="allure report parent name")


def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "allure_label(name, value): Add label to test"
    )