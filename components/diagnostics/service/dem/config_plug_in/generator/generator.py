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
Dem配置代码生成模块.
根据输入的config.json, 然后生成Dem代码.
"""
import os
import json
from func_module.health_monitor.logger import logger
from . import filter
from . import context
from jinja2 import Environment, FileSystemLoader, StrictUndefined

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    logger.debug("Generate Dem dynamic code")
    input_dict = filter.get_template_dynamic_code_dict(config_dict)
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "Dem"))

    os_core_config = {}
    if "os" in config_dict["bsw"].keys():
        os_input_dict = config_dict["bsw"]["os"]
        # get EcuC Configs
        os_ecuc_dict = config_dict["bsw"]["ecuc"]["ArPackages"]["ActiveEcuC"]["EcuC"]
        # get core information from UI input and add to os dict
        os_input_dict['OsCores'] = {}
        # get hw coreid information from UI input and add to os dict
        os_input_dict['HWCores'] = {}
        filter.Os_CoreInfo_Get(os_input_dict, os_ecuc_dict)

        os_core_config = os_input_dict['OsCores']
    module_list = sys_config['modules_info']['kfg_module_enabled_list']
    filter.set_application_id_name(config_dict["bsw"], input_dict, os_core_config, module_list)

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    # Create required directories

    os.makedirs(output_dir, exist_ok=True)
    dem_config = {"dem_json": input_dict, "sys_cfg": sys_config}

    template = env.get_template("dem_lcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "dem_lcfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dem_config))
            logger.info("Finish generating dem_lcfg.c file")
    template = env.get_template("dem_lcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "dem_lcfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dem_config))
            logger.info("Finish generating dem_lcfg.h file")
    template = env.get_template("dem_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "dem_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dem_config))
            logger.info("Finish generating dem_cfg.h file")
    template = env.get_template("dem_lcfg_internal.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "dem_lcfg_internal.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(dem_config))
            logger.info("Finish generating dem_lcfg_internal.h file")
    return True
