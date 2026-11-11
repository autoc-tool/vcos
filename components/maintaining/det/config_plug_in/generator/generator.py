#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
配置代码生成模块.
根据输入的config.json, 然后生成模块代码.
"""
import os,shutil
from . import filter
from . import context
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined
from func_module.generator.common.user_code_extract import UserCodeExtract

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    logger.debug("Generate Det dynamic code")
    input_dict = filter.get_template_dynamic_code_dict(config_dict["bsw"]["det"])
    user_code_extract = UserCodeExtract()
    # json_dir = os.path.realpath(os.path.join(sys_config["config_path"], "Bsw/Det"))
    # file_path = os.path.join(json_dir, "Det_config.json")
    # with open(file_path, 'w') as f:
    #     json.dump(input_dict, f, indent=4)
    det_callout_output_file_dir = sys_config['path']['bswcallout_path']
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "Det"))

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    # Create required directories

    input_dict_bsw_callout = user_code_extract.bsw_callout_user_code(det_callout_output_file_dir)
    if not input_dict_bsw_callout.get("DetUserCallout", {}):
        # 没有文件时，赋空
        input_dict_bsw_callout["DetUserCallout"] = {}


    input_dict = input_dict["ArPackages"]["ActiveEcuC"]['Det']
    os.makedirs(output_dir, exist_ok=True)

    det_config = {"det_json": input_dict, "sys_cfg": sys_config,"functions": input_dict_bsw_callout["DetUserCallout"]}

    template = env.get_template("det_user_callout.c.jinja2")
    if template is not None:
        file_path = os.path.join(det_callout_output_file_dir, "det_user_callout.c")
        if os.path.exists(det_callout_output_file_dir) is False:
            os.makedirs(det_callout_output_file_dir)
        # 对原文件进行保存
        if os.path.exists(file_path):
            dest_name = f'det_user_callout.c.back'
            dest_path = os.path.join(det_callout_output_file_dir, dest_name)
            shutil.copy(file_path, dest_path)
            # 删除原文件
            os.remove(file_path)
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(det_config))
            logger.info("Finish generating det_user_callout.c file")

    template = env.get_template("det_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "det_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(det_config))
            logger.info("Finish generating det_cfg.h file")

    template = env.get_template("det_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "det_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(det_config))
            logger.info("Finish generating det_cfg.c file")
    return True