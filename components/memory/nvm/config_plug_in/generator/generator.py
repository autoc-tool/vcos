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
配置代码生成模块.
根据输入的config.json, 然后生成模块代码.
"""

import os,shutil
from . import filter
from . import context
from func_module.health_monitor.logger import logger
import copy
from jinja2 import Environment, FileSystemLoader, StrictUndefined
from func_module.generator.common.user_code_extract import UserCodeExtract

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    user_code_extract = UserCodeExtract()
    logger.debug("Generate NvM dynamic code")
    input_dict = filter.get_template_dynamic_code_dict(config_dict["bsw"]["nvm"])
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "NvM"))
    nvm_callout_output_file_dir = sys_config['path']['bswcallout_path']

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    # Create required directories

    os.makedirs(output_dir, exist_ok=True)
    # Generate block_dict only contain blocks
    input_dict = input_dict["ArPackages"]["ActiveEcuC"]['NvM']
    block_dict = copy.deepcopy(input_dict)
    if block_dict.get('DefinitionRef'):
        del block_dict['DefinitionRef']
    if block_dict.get('NvMCommon'):
        del block_dict['NvMCommon']
    if block_dict.get('NvMCommonVendorParams'):
        del block_dict['NvMCommonVendorParams']

    input_dict_bsw_callout = user_code_extract.bsw_callout_user_code(nvm_callout_output_file_dir)
    if not input_dict_bsw_callout.get("NvMUserCallout", {}):
        # 没有文件时，赋空
        input_dict_bsw_callout["NvMUserCallout"] = {}
    NvM_config = {
        "NvM_General": input_dict, 
        "NvM_Blocks": block_dict, 
        "sys_cfg": sys_config,
        "functions": input_dict_bsw_callout["NvMUserCallout"]}

    template = env.get_template("nvm_user_callout.c.jinja2")
    if template is not None:
        file_path = os.path.join(nvm_callout_output_file_dir, "nvm_user_callout.c")
        if os.path.exists(nvm_callout_output_file_dir) is False:
            os.makedirs(nvm_callout_output_file_dir)
        # 对原文件进行保存
        if os.path.exists(file_path):
            dest_name = f'nvm_user_callout.c.back'
            dest_path = os.path.join(nvm_callout_output_file_dir, dest_name)
            shutil.copy(file_path, dest_path)
            # 删除原文件
            os.remove(file_path)
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(NvM_config))
            logger.info("Finish generating nvm_user_callout.c file")

    template = env.get_template("nvm_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "nvm_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(NvM_config))
            logger.info("Finish generating nvm_cfg.h file")

    template = env.get_template("nvm_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "nvm_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(NvM_config))
            logger.info("Finish generating nvm_cfg.c file")
    return True
