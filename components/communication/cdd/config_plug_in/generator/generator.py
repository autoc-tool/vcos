#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
配置代码生成模块.
根据输入的config.json, 然后生成模块代码.
"""
import os
import shutil
from . import filter
from . import context
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined, TemplateNotFound

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    logger.debug("Generate Cdd_Cfg dynamic code")
    # print(json.dumps(config_dict["bsw"]["cdd"]))
    cdd_configs = filter.get_template_dynamic_code_dict(config_dict["bsw"])
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    # output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "Cdd"))

    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    cdd_name_list = sys_config['modules_info']['codegen_module_list']
    owercase_list = [element.lower() for element in cdd_name_list]

    for cdd_key, cdd_info in cdd_configs.items():
        if cdd_key not in owercase_list:
            continue
        cdd_dict = config_dict['bsw'].get(cdd_key.lower(), None)
        cdd_key_formatted = cdd_key.capitalize()        # Cdd名称为默认值'Cdd_'开始时有效，作为默认值
        if cdd_dict is not None and len(cdd_dict['ArPackages']['ActiveEcuC']) == 1: # 里面只会有一个元素，其名称为原始的模块名
            cdd_key_formatted = list(cdd_dict['ArPackages']['ActiveEcuC'].keys())[0]
        output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], cdd_key_formatted)) # 创建目录
        if os.path.exists(output_dir):
            shutil.rmtree(output_dir)
            os.mkdir(output_dir)
        else:
            os.mkdir(output_dir)
        filename = f"{cdd_key_formatted.lower()}.h"         # 生成文件名
        file_path = os.path.join(output_dir, filename)  # 生成文件路径
        # 渲染模板并写入到指定文件
        template = env.get_template("Cdd_Cbk.h.jinja2")  # 获取渲染模板
        # 如果模板存在，则渲染模板并写入到指定文件
        try:
            template = env.get_template("Cdd_Cbk.h.jinja2")
        except TemplateNotFound:
            logger.error(f"Template Cdd_Cbk.h.jinja2 not found in {DEFAULT_TEMPLATE_FILE_DIR}")
            # 使用continue语句跳过当前循环的其余部分，继续下一次循环
            continue
        with open(file_path, 'w', encoding='utf-8') as out_file:
            out_file.write(template.render({"cddInfo": cdd_info, "sys_cfg": sys_config}))
            logger.info(f"Finished generating {filename}")

        # if template is not None:
        #     with open(file_path, 'w', encoding='utf-8') as out_file:
        #         # 在模板中可以通过cddInfo和sys_cfg来访问传入的配置数据
        #         out_file.write(template.render({"cddInfo": cdd_info, "sys_cfg": sys_config}))
        #         logger.info(f"Finished generating {filename}")
    return True
