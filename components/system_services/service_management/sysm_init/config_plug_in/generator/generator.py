#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
EcuM配置代码生成模块.
根据输入的config.json, 然后生成EcuM配置代码.
"""
import os, shutil
from . import context
import json
import copy
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined
from func_module.generator.common.user_code_extract import UserCodeExtract

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
        ecum_container_group_dict = {"Tag": "CONTAINER-GROUP"}
        ecum_container_group_dict.update({container_name: cur_dict})
        parent_dict.update({container_group_name: ecum_container_group_dict})
    del parent_dict[container_name]


def arxml_dict_to_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "EcuMWakeupSource":
        _add_contianer_group_in_dict(cur_key_name, "EcuMWakeupSources", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "EcuMDriverInitItem":
        _add_contianer_group_in_dict(cur_key_name, "EcuMDriverInitItems", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "EcuMShutdownCause":
        _add_contianer_group_in_dict(cur_key_name, "EcuMShutdownCauses", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "EcuMResetMode":
        _add_contianer_group_in_dict(cur_key_name, "EcuMResetModes", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "EcuMSleepMode":
        _add_contianer_group_in_dict(cur_key_name, "EcuMSleepModes", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "EcuMFlexUserConfig":
        _add_contianer_group_in_dict(cur_key_name, "EcuMFlexUserConfigs", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            arxml_dict_to_dict(container_dict, container_name, cur_dict, cur_key_name)


def get_bsw_config(config_dict: dict):
    res = {}

    dict_comm = config_dict["bsw"]["comm"]["ArPackages"] if "comm" in config_dict["bsw"] else {}
    dict_ecuc = config_dict["bsw"]["ecuc"]["ArPackages"] if "ecuc" in config_dict["bsw"] else {}

    res["dict_ecuc"] = dict_ecuc
    res["dict_comm"] = dict_comm
    return res


def child_convert(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "ComMUserPerChannel":
        _add_contianer_group_in_dict(cur_key_name, "ComMUserPerChannels", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            child_convert(container_dict, container_name, cur_dict, cur_key_name)


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    user_code_extract = UserCodeExtract()
    output_file_dir = os.path.join(sys_config["gendata_path"], "sysm/EcuM")
    ecum_callout_output_file_dir = sys_config['path']['bswcallout_path']
    os.makedirs(output_file_dir, exist_ok=True)
    env = Environment(loader=FileSystemLoader(os.path.abspath(TEMPLATE_DIR)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    # tmp add
    ######################################################################################
    dict_base = copy.deepcopy(config_dict["bsw"]["ecum"]["ArPackages"])
    dict_to_arxml_dict(dict_base["ActiveEcuC"]["EcuM"], "EcuM", dict_base["ActiveEcuC"], "ActiveEcuC")

    dict_config = copy.deepcopy(config_dict["bsw"]["ecum"]["ArPackages"])

    # child_convert(dict_config["ActiveEcuC"]["EcuM"]["EcuMConfiguration"], "EcuMConfiguration",
    #                    dict_config["ActiveEcuC"]["EcuM"], "EcuM")
    arxml_dict_to_dict(dict_config["ActiveEcuC"]["EcuM"]["EcuMConfiguration"], "EcuMConfiguration",
                       dict_config["ActiveEcuC"]["EcuM"], "EcuM")

    dict_bsw = get_bsw_config(config_dict)
    ######################################################################################

    # with open('test.json', 'w') as f:
    #     json.dump(dict_config, f, indent=4)
    input_dict_bsw_callout = user_code_extract.bsw_callout_user_code(ecum_callout_output_file_dir)
    if not input_dict_bsw_callout.get("sysm_init_user_callout", {}):
        # 没有文件时，赋空
        input_dict_bsw_callout["sysm_init_user_callout"] = {}
    dic_jinja = {
        "dict_base": dict_base,
        "dict_config": dict_config,
        "dict_bsw": dict_bsw,
        "sys_cfg": sys_config,
        "functions": input_dict_bsw_callout["sysm_init_user_callout"]
    }

    template = env.get_template("sysm_init_user_callout.c.jinja2")
    if template is not None:
        file_path = os.path.join(ecum_callout_output_file_dir, "sysm_init_user_callout.c")
        if os.path.exists(ecum_callout_output_file_dir) is False:
            os.makedirs(ecum_callout_output_file_dir)
        # 对原文件进行保存
        if os.path.exists(file_path):
            dest_name = f'sysm_init_user_callout.c.back'
            dest_path = os.path.join(ecum_callout_output_file_dir, dest_name)
            shutil.copy(file_path, dest_path)
            # 删除原文件
            os.remove(file_path)
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating sysm_init_user_callout.c file")

    template = env.get_template("sysm_init_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "sysm_init_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating sysm_init_cfg.h file")

    template = env.get_template("sysm_init_generated_types.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "sysm_init_generated_types.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating sysm_init_generated_types.h file")

    template = env.get_template("sysm_init_pb_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "sysm_init_pb_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating sysm_init_pb_cfg.c file")

    template = env.get_template("sysm_init_pb_header.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "sysm_init_pb_header.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating sysm_init_pb_header.c file")

    template = env.get_template("sysm_init_pb_types.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "sysm_init_pb_types.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating sysm_init_pb_types.h file")

    template = env.get_template("precompiled_data_hash.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "precompiled_data_hash.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dic_jinja))
            logger.info("Finish generating precompiled_data_hash.h file")
    return True