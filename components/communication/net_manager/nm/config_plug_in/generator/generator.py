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
配置代码生成模块.
根据输入的config.json, 然后生成模块代码.
"""

import os
from . import filter
from . import context
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def get_key_name(key_name: str) -> str:
    key_name = key_name.split("/")[-1]
    return key_name


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    logger.debug("Generate Nm dynamic code")
    comm_dict = get_bsw_config(config_dict)
    input_dict = filter.get_template_dynamic_code_dict(config_dict["bsw"]["nm"])
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "Nm"))
    json_dir = os.path.realpath(os.path.join(sys_config["config_path"], "Bsw/Nm"))

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    # Create required directories

    os.makedirs(output_dir, exist_ok=True)
    # dump json to Project BCM directory
    # file_path = os.path.join(json_dir, "nm_config.json")
    # with open(file_path, 'w') as f:
    #     json.dump(input_dict, f, indent=4)
    # Generate block_dict only contain blocks
    input_dict = input_dict["ArPackages"]["ActiveEcuC"]['Nm']
    comm_dict = comm_dict["dict_comm"]["ActiveEcuC"]['ComM']
    block_dict = input_dict.copy()
    del input_dict['NmGlobalConfig']
    del comm_dict["ComMGeneral"]
    del comm_dict["DefinitionRef"]
    del comm_dict["ComMConfigSet"]["DefinitionRef"]
    comm_list_cannel = []
    for action_name, action_value in comm_dict["ComMConfigSet"].items():
        if (get_key_name(action_value["DefinitionRef"]) == "ComMChannel"):
            comm_list_cannel.append(action_value)
    comm_dict_cannel = {"Channel": comm_list_cannel}

    nm_config = {
        "NM_General": input_dict,
        "nm_config": block_dict,
        "ComM_Confg": comm_dict,
        "ComM_Channel": comm_dict_cannel["Channel"],
        "sys_cfg": sys_config
    }
    template = env.get_template("nm_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "nm_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(nm_config))

    template = env.get_template("nm_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "nm_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(nm_config))
    return True

def get_bsw_config(config_dict: dict):
    res = {}
    dict_comm = config_dict["bsw"]["comm"]["ArPackages"]
    res["dict_comm"] = dict_comm
    return res
