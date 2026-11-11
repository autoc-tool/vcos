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
ComM配置代码生成模块.
根据输入的config.json, 然后生成ComM配置代码.
"""
import os
from . import context
import json
import copy
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined

INPUT_FILE_Dir = os.path.dirname(__file__)
TEMPLATE_DIR = os.path.join(INPUT_FILE_Dir, "template")


def dict_to_arxml_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):
    if not isinstance(cur_dict, dict):
        return
    if "Tag" in cur_dict and cur_dict["Tag"] == "CONTAINER-GROUP":
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)
            parent_dict.update({container_name: container_dict})
        del parent_dict[cur_key_name]
    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)


def _add_contianer_group_in_dict(container_name: str, container_group_name: str, parent_dict: dict, cur_dict: dict):
    if container_group_name in parent_dict:
        parent_dict[container_group_name].update({container_name: cur_dict})
    else:
        comm_container_group_dict = {"Tag": "CONTAINER-GROUP"}
        comm_container_group_dict.update({container_name: cur_dict})
        parent_dict.update({container_group_name: comm_container_group_dict})
    del parent_dict[container_name]


def arxml_dict_to_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "ComMChannel":
        _add_contianer_group_in_dict(cur_key_name, "ComMChannels", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "ComMUser":
        _add_contianer_group_in_dict(cur_key_name, "ComMUsers", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "ComMPnc":
        _add_contianer_group_in_dict(cur_key_name, "ComMPncs", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            arxml_dict_to_dict(container_dict, container_name, cur_dict, cur_key_name)


def child_convert(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "ComMUserPerChannel":
        _add_contianer_group_in_dict(cur_key_name, "ComMUserPerChannels", parent_dict, cur_dict)

    if cur_item_definition_ref_val == "ComMPncComSignal":
        _add_contianer_group_in_dict(cur_key_name, "ComMPncComSignals", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            child_convert(container_dict, container_name, cur_dict, cur_key_name)


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    output_file_dir = os.path.join(sys_config["gendata_path"], "ComM")
    os.makedirs(output_file_dir, exist_ok=True)
    env = Environment(loader=FileSystemLoader(os.path.abspath(TEMPLATE_DIR)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    # tmp add
    ######################################################################################
    dict_base = copy.deepcopy(config_dict["bsw"]["comm"]["ArPackages"])
    dict_to_arxml_dict(dict_base["ActiveEcuC"]["ComM"], "ComM", dict_base["ActiveEcuC"], "ActiveEcuC")

    dict_config = copy.deepcopy(config_dict["bsw"]["comm"]["ArPackages"])

    child_convert(dict_config["ActiveEcuC"]["ComM"]["ComMConfigSet"], "ComMConfigSet",
                  dict_config["ActiveEcuC"]["ComM"], "ComM")
    arxml_dict_to_dict(dict_config["ActiveEcuC"]["ComM"]["ComMConfigSet"], "ComMConfigSet",
                       dict_config["ActiveEcuC"]["ComM"], "ComM")
    ######################################################################################

    # with open('test.json', 'w') as f:
    #     json.dump(dict_config, f, indent=4)

    dic_jinja = {"dict_base": dict_base, "dict_config": dict_config, "sys_cfg": sys_config}

    template = env.get_template("comm_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "comm_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating comm_cfg.h file")

    template = env.get_template("comm_pbcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "comm_pbcfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating comm_pbcfg.c file")

    template = env.get_template("comm_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "comm_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating comm_cfg.c file")
    return True

def main():
    sys_config = {}
    config_dict = {"bsw": {}}
    sys_config["gendata_path"] = os.path.join(INPUT_FILE_Dir, "../../../../../../Projects/BCM_Sample/GenData")
    print(os.path.realpath(sys_config["gendata_path"]))
    JSON_CONFIG = "../../../../../../Projects/BCM_Sample/Config/Bsw/ComM/ComM_Cfg.json"
    f = open(JSON_CONFIG, "r", encoding="utf-8")
    info_date_dict = json.load(f)
    info_date_dict_input = info_date_dict.get("ArPackages")
    config_dict["bsw"]["comm"] = info_date_dict_input
    generate_dynamic_code(sys_config, config_dict)


if __name__ == "__main__":
    main()
