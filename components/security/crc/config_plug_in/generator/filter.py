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
"""
os配置代码生成模块.
根据输入的config.json, 构造NvM_config.json
"""


def get_template_input_parameter_dict(config_dict: dict) -> dict:
    """
    Get input parameter for template generated code.
    Arg:
      config_dict : All config dictionary
    Return:
      Returns a dictionary of template input parameter
    """
    return config_dict


def get_template_dynamic_code_dict(config_dict: dict) -> dict:
    return config_dict
