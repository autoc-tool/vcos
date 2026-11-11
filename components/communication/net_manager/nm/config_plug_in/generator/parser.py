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
Nm配置文件解析模块.
解析输入的Nm配置文件, 然后生成模板config.josn
"""

import os
import json
from func_module.health_monitor.logger import logger

CUR_FILE_PATH = os.path.dirname(os.path.realpath(__file__))


def load_configuration_file(sys_config: dict) -> dict:
    """
    Parse Nm module configuration file
    Return:
      Nm module config.json
    """
    Nm_cfg_path = os.path.join(sys_config["config_path"], "Bsw/Nm/nm_config.json")
    if os.path.isfile(Nm_cfg_path) is False:
        logger.error(f"File {Nm_cfg_path} Not Exist !")
    Nm_cfg_dict = None
    with open(Nm_cfg_path, "r", encoding="utf-8") as f:
        Nm_cfg_dict = json.load(f)
    return Nm_cfg_dict


def get_config_json_dict(nm_config: dict) -> dict:

    return nm_config
