#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
EcuM配置文件解析模块.
解析输入的EcuM配置文件, 然后生成模板config.josn
"""
import os
import json
from func_module.logger import logger

CUR_FILE_PATH = os.path.dirname(os.path.realpath(__file__))


def load_configuration_file(sys_config: dict) -> dict:
    """
    Parse EcuM module configuration file
    Return:
      EcuM module config.json
    """
    ecum_cfg_path = os.path.join(sys_config["config_path"], "Bsw/EcuM/EcuM_Cfg.json")
    if os.path.isfile(ecum_cfg_path) is False:
        logger.error(f"File {ecum_cfg_path} Not Exist !")
    ecum_cfg_dict = None
    with open(ecum_cfg_path, "r", encoding="utf-8") as f:
        ecum_cfg_dict = json.load(f)
    return ecum_cfg_dict["ArPackages"]


def get_config_json_dict(ecum_config: dict) -> dict:

    return ecum_config
