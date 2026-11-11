#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
配置文件解析模块.
解析输入的配置文件, 然后生成模板config.josn
"""
import os
import json

CUR_FILE_PATH = os.path.dirname(os.path.realpath(__file__))


def load_configuration_file(sys_config: dict) -> dict:
    """
    Parse module configuration file
    Return:
      module config.json
    """
    cdd_cfg_path = os.path.join(sys_config["config_path"], "Bsw/Cdd/Cdd_Cfg.json")
    cdd_cfg_dict = None
    with open(cdd_cfg_path, "r", encoding="utf-8") as f:
        cdd_cfg_dict = json.load(f)
    return cdd_cfg_dict["ArPackages"]


def get_config_json_dict(cdd_config: dict) -> dict:
    print(json.dumps(cdd_config))
    return cdd_config
