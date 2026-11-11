#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
BswM配置文件解析模块.
解析输入的BswM配置文件, 然后生成模板config.josn
"""
import os
import json
from func_module.logger import logger

CUR_FILE_PATH = os.path.dirname(os.path.realpath(__file__))


def load_configuration_file(sys_config: dict) -> dict:
    """
    Parse BswM module configuration file
    Return:
      bswM module config.json
    """
    bswm_cfg_path = os.path.join(sys_config["config_path"], "Bsw/BswM/BswM_Cfg.json")
    if os.path.isfile(bswm_cfg_path) is False:
        logger.error(f"File {bswm_cfg_path} Not Exist !")
    bswm_cfg_dict = None
    with open(bswm_cfg_path, "r", encoding="utf-8") as f:
        bswm_cfg_dict = json.load(f)
    return bswm_cfg_dict["ArPackages"]


def get_config_json_dict(bswm_config: dict) -> dict:

    return bswm_config
