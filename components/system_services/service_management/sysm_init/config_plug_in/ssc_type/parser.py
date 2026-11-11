#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
ComM配置文件解析模块.
解析输入的Com配置文件, 然后生成模板config.josn
"""
import os
import json
from func_module.logger import logger

CUR_FILE_PATH = os.path.dirname(os.path.realpath(__file__))


def load_configuration_file(sys_config: dict) -> dict:
    """
    Parse ComM module configuration file
    Return:
      ComM module config.json
    """
    comm_cfg_path = os.path.join(sys_config["config_path"], "Bsw/ComM/ComM_Cfg.json")
    if os.path.isfile(comm_cfg_path) is False:
        logger.error(f"File {comm_cfg_path} Not Exist !")
    comm_cfg_dict = None
    with open(comm_cfg_path, "r", encoding="utf-8") as f:
        comm_cfg_dict = json.load(f)
    return comm_cfg_dict["ArPackages"]


def get_config_json_dict(comm_config: dict) -> dict:

    return comm_config
