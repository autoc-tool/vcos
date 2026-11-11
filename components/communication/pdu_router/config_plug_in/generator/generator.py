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
pdur配置代码生成模块.
根据输入的config.json, 然后生成pdur代码.
"""
import os
from . import filter
from . import context
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")

def generate_dynamic_code(sys_config: dict, config_dict: dict):
    logger.debug("Generate PduR_Cfg dynamic code")
    input_dict = filter.get_template_dynamic_code_dict(config_dict["bsw"], config_dict["rte"])
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "PduR"))

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    # Create required directories

    generate_dynamic_code_config(env, sys_config, config_dict, input_dict, output_dir)
    generate_dynamic_code_interface(env, sys_config, input_dict, output_dir)
    generate_dynamic_code_interface_main(env, sys_config, input_dict, output_dir)
    return True

def generate_dynamic_code_config(env, sys_config: dict, config_dict: dict, input_dict: dict, output_dir: dict):
    os.makedirs(output_dir, exist_ok=True)
    template = env.get_template("pdur_cfg.h.jinja2")
    pdur_config = {"pdur_json": input_dict, "sys_cfg": sys_config}
    pdur_cfg = config_dict["bsw"]["pdur"]
    config = {"pdur_cfg": pdur_cfg, "sys_cfg": sys_config}
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_cfg.h file")

    template = env.get_template("pdur_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_cfg.c file")

    template = env.get_template("pdur_pbcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_pbcfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_pbcfg.h file")

    template = env.get_template("pdur_pbcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_pbcfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_pbcfg.c file")

    template = env.get_template("pdur_bufferpoolcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_bufferpoolcfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_bufferpoolcfg.c file")

    template = env.get_template("pdur_bufferpoolcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_bufferpoolcfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_bufferpoolcfg.h file")

    template = env.get_template("pdur_evcfifocfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_evcfifocfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_evcfifocfg.c file")

    template = env.get_template("pdur_routingcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_routingcfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_routingcfg.h file")

    template = env.get_template("pdur_routingcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "pdur_routingcfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(pdur_config))
            logger.info("Finish generating pdur_routingcfg.c file")

def generate_dynamic_code_interface(env, sys_config, input_dict: dict, output_dir: dict):
    PduRBswModules = input_dict['PduRBswModules']
    for _, module in PduRBswModules.items():
        if "PduRBswModuleRef" not in module:
            continue
        moduleRef = module['PduRBswModuleRef']['ValueRef']
        moduleName = moduleRef.split("/")[-1]
        moduleConfig = {}
        if moduleName == "DoIP" or moduleName == "SoAd":
            if 'PduRLowerModule' in module and module['PduRLowerModule']['Value'] is True:
                if 'PduRCommunicationInterface' in module and module['PduRCommunicationInterface']['Value'] is True:
                    type = 1
                if 'PduRTransportProtocol' in module and module['PduRTransportProtocol']['Value'] is True:
                    type = 2
        else:
            if 'PduRLowerModule' in module and module['PduRLowerModule']['Value'] is True:
                if 'PduRCommunicationInterface' in module and module['PduRCommunicationInterface']['Value'] is True:
                    type = 3
                if 'PduRTransportProtocol' in module and module['PduRTransportProtocol']['Value'] is True:
                    type = 4
            if 'PduRUpperModule' in module and module['PduRUpperModule']['Value'] is True:
                if 'PduRCommunicationInterface' in module and module['PduRCommunicationInterface']['Value'] is True:
                    type = 5
                if 'PduRTransportProtocol' in module and module['PduRTransportProtocol']['Value'] is True:
                    type = 6
        moduleConfig['moduleName'] = moduleName
        moduleConfig['type'] = type
        # pdur_config = {"pdur_json": input_dict, "sys_cfg": sys_config}

        pdur_config = {"moduleConfig": moduleConfig, "sys_cfg": sys_config}
        templateName_c = "pdur_" + moduleName.lower() + ".c"
        templateName_h = "pdur_" + moduleName.lower() + ".h"
        template_c = env.get_template("pdur_module_interface.c.jinja2")
        template_h = env.get_template("pdur_module_interface.h.jinja2")
        if template_c is not None:
            file_path = os.path.join(output_dir, templateName_c)
            with open(file_path, "w", encoding="utf-8") as out_stream:
                out_stream.write(template_c.render(pdur_config))
                loggerInfo = "Finish generating " + templateName_c + " file"
                logger.info(loggerInfo)
        if template_h is not None:
            file_path = os.path.join(output_dir, templateName_h)
            with open(file_path, "w", encoding="utf-8") as out_stream:
                out_stream.write(template_h.render(pdur_config))
                loggerInfo = "Finish generating " + templateName_h + " file"
                logger.info(loggerInfo)

def generate_dynamic_code_interface_main(env, sys_config, input_dict: dict, output_dir: dict):
    PduRBswModules = input_dict['PduRBswModules']
    name_list = []
    for _, module in PduRBswModules.items():
        if "PduRBswModuleRef" not in module:
            continue
        moduleRef = module['PduRBswModuleRef']['ValueRef']
        moduleName = moduleRef.split("/")[-1]
        name_list.append(moduleName)

    pdur_config = {"moduleConfig": name_list, "sys_cfg": sys_config}
    templateName_h = "pdur_interface.h"
    template_h = env.get_template("pdur_interface.h.jinja2")
    if template_h is not None:
        file_path = os.path.join(output_dir, templateName_h)
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template_h.render(pdur_config))
            loggerInfo = "Finish generating " + templateName_h + " file"
            logger.info(loggerInfo)
