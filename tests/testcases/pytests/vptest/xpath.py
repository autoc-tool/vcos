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
from env.config import config


_CURRENT_PATH = os.path.dirname(os.path.abspath(__file__))

TESTS_TESTCASES_PYTEST_PATH = os.path.dirname(_CURRENT_PATH)
TESTS_TESTCASES_PATH = os.path.dirname(TESTS_TESTCASES_PYTEST_PATH)
TESTS_PATH = os.path.dirname(TESTS_TESTCASES_PATH)
VCOS_TESTS_PATH = os.path.dirname(TESTS_PATH)
TESTS_EXAMPLES_PATH = os.path.join(TESTS_PATH, "examples")
ROOT_PATH = os.path.dirname(VCOS_TESTS_PATH)
VCOS_PATH = os.path.join(ROOT_PATH, "vcos")

VCOS_STUDIO_PATH = os.path.join(VCOS_PATH, "vcos_studio")
VCOS_STUDIO_CONFIGURATOR_PATH = os.path.join(VCOS_STUDIO_PATH, "configurator")
VCOS_STUDIO_CONFIGURATOR_FUNC_MODULE_PATH = os.path.join(VCOS_STUDIO_CONFIGURATOR_PATH, "func_module")
VCOS_STUDIO_CONFIGURATOR_THIRD_PARTY_PATH = os.path.join(VCOS_STUDIO_CONFIGURATOR_PATH, "third_party")

EXAMPLES_PATH = os.path.join(VCOS_PATH, "examples")
COMPONENTS_PATH = os.path.join(VCOS_PATH, "components")
OUTPUT_PATH = os.path.join(ROOT_PATH, "output")

EXAMPLE_DIR_PATH = ""
if os.path.exists(os.path.join(EXAMPLES_PATH, f"{config.EXAMPLE_NAME}")):
    EXAMPLE_DIR_PATH = os.path.join(EXAMPLES_PATH, config.EXAMPLE_NAME)
elif os.path.exists(os.path.join(TESTS_EXAMPLES_PATH, f"{config.EXAMPLE_NAME}")):
    EXAMPLE_DIR_PATH = os.path.join(TESTS_EXAMPLES_PATH, config.EXAMPLE_NAME)
