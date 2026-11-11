#
# Copyright (C) 2015-2022 Lixiang Group Holding Limited
#
"""
TcpIp 配置代码生成模块.
"""
import os
import json
from . import context
from . import filter
from jinja2 import Environment, FileSystemLoader, StrictUndefined
from func_module.health_monitor.logger import logger

INPUT_FILE_Dir = os.path.dirname(__file__)
TEMPLATE_DIR = os.path.join(INPUT_FILE_Dir, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    result1 = False
    result2 = False
    result3 = False
    result4 = False
    result5 = False
    logger.debug("Generate tcpip dynamic code")

    with open(os.path.join(os.path.dirname(__file__), 'dict_initconfig.json'), 'w') as f:
        json.dump(config_dict["bsw"]["tcpip"]['ArPackages']['ActiveEcuC']['TcpIp'], f, indent=4)

    tcpip = filter.TcpIpInfo("tcpip", config_dict["bsw"]["tcpip"]['ArPackages']['ActiveEcuC']['TcpIp'], config_dict["rte"],)
    filter_config = tcpip.get_template_dynamic_code_dict()
    with open(os.path.join(os.path.dirname(__file__), 'dict_initfilter.json'), 'w') as f:
        json.dump(filter_config, f, indent=4)

    filter.set_tcpip_default_value(filter_config)
    with open(os.path.join(os.path.dirname(__file__), 'dict.json'), 'w') as f:
        json.dump(filter_config, f, indent=4)

    output_file_dir = os.path.join(sys_config["gendata_path"], "TcpIp")
    os.makedirs(output_file_dir, exist_ok=True)

    tcpip_config = {"json": filter_config, "sys_cfg": sys_config}
    env = Environment(loader=FileSystemLoader(os.path.abspath(TEMPLATE_DIR)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    template = env.get_template("tcpip_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "tcpip_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(tcpip_config))
            logger.info("Finish generating tcpip_cfg.h file")
            result1 = True

    template = env.get_template("tcpip_lcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "tcpip_lcfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(tcpip_config))
            logger.info("Finish generating tcpip_lcfg.h file")
            result5 = True

    template = env.get_template("tcpip_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "tcpip_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(tcpip_config))
            logger.info("Finish generating tcpip_cfg.c file")
            result2 = True

    output_file_dir = os.path.join(sys_config["gendata_path"], "Lwip")
    os.makedirs(output_file_dir, exist_ok=True)
    template = env.get_template("Lwip_Cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "Lwip_Cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(tcpip_config))
            logger.info("Finish generating Lwip_Cfg.h file")
            result3 = True

    template = env.get_template("Lwippool_Cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_file_dir, "Lwippool_Cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(tcpip_config))
            logger.info("Finish generating Lwippool_Cfg.h file")
            result4 = True
    return result1 & result2 & result3 & result4 & result5
