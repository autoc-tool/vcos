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
BswM配置代码生成模块.
根据输入的config.json, 然后生成BswM_Cfg.h代码.
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
        bswm_container_group_dict = {"Tag": "CONTAINER-GROUP"}
        bswm_container_group_dict.update({container_name: cur_dict})
        parent_dict.update({container_group_name: bswm_container_group_dict})
    del parent_dict[container_name]


def arxml_dict_to_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "BswMLogicalExpression":
        _add_contianer_group_in_dict(cur_key_name, "BswMLogicalExpressions", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "BswMModeCondition":
        _add_contianer_group_in_dict(cur_key_name, "BswMModeConditions", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "BswMModeRequestPort":
        _add_contianer_group_in_dict(cur_key_name, "BswMModeRequestPorts", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "BswMRule":
        _add_contianer_group_in_dict(cur_key_name, "BswMRules", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "BswMActionList":
        _add_contianer_group_in_dict(cur_key_name, "BswMActionLists", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "BswMAction":
        _add_contianer_group_in_dict(cur_key_name, "BswMActions", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "BswMRteModeRequestPort":
        _add_contianer_group_in_dict(cur_key_name, "BswMRteModeRequestPorts", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "BswMSwitchPort":
        _add_contianer_group_in_dict(cur_key_name, "BswMSwitchPorts", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            arxml_dict_to_dict(container_dict, container_name, cur_dict, cur_key_name)


def get_bsw_config(config_dict: dict):
    res = {}

    dict_linsm = config_dict["bsw"]["linsm"]["ArPackages"] if "linsm" in config_dict["bsw"] else {}
    dict_pdur = config_dict["bsw"]["pdur"]["ArPackages"] if "pdur" in config_dict["bsw"] else {}
    dict_ecuc = config_dict["bsw"]["ecuc"]["ArPackages"] if "ecuc" in config_dict["bsw"] else {}

    res["dict_ecuc"] = dict_ecuc
    res["dict_linsm"] = dict_linsm
    res["dict_pdur"] = dict_pdur
    return res


def child_convert(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "BswMActionListItem":
        _add_contianer_group_in_dict(cur_key_name, "BswMActionListItems", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            child_convert(container_dict, container_name, cur_dict, cur_key_name)


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    output_file_dir = os.path.join(sys_config["gendata_path"], "sysm/BswM")
    os.makedirs(output_file_dir, exist_ok=True)
    env = Environment(loader=FileSystemLoader(os.path.abspath(TEMPLATE_DIR)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    # tmp add
    ######################################################################################
    dict_base = copy.deepcopy(config_dict["bsw"]["bswm"]["ArPackages"])
    dict_to_arxml_dict(dict_base["ActiveEcuC"]["BswM"], "BswM", dict_base["ActiveEcuC"], "ActiveEcuC")

    dict_config = copy.deepcopy(config_dict["bsw"]["bswm"]["ArPackages"])
    child_convert(dict_config["ActiveEcuC"]["BswM"]["BswMConfig"], "BswMConfig", dict_config["ActiveEcuC"]["BswM"],
                  "BswM")
    arxml_dict_to_dict(dict_config["ActiveEcuC"]["BswM"]["BswMConfig"], "BswMConfig", dict_config["ActiveEcuC"]["BswM"],
                       "BswM")

    dict_bsw = get_bsw_config(config_dict)
    ######################################################################################

    # with open('test.json', 'w') as f:
    #     json.dump(dict_config, f, indent=4)

    dic_jinja = {"dict_base": dict_base, "dict_config": dict_config, "dict_bsw": dict_bsw, "sys_cfg": sys_config}

    template = env.get_template("bswm_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "bswm_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating bswm_cfg.h file")

    template = env.get_template("bswm_pbcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "bswm_pbcfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating bswm_pbcfg.c file")

    template = env.get_template("bswm_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "bswm_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating bswm_cfg.c file")
    return True