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

import os
import inspect
from . import context
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined, UndefinedError

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")

def generate_arxml(config_path: dict, config_dict: dict):
    logger.debug("Generate NvM service component arxml")
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    output_dir = os.path.realpath(os.path.join(config_path, "ServiceComponents"))
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    # Create required directories
    # with open('nvm_swc.json', 'w') as f:     
    #    json.dump(config_dict, f, indent=4)
    template = env.get_template("NvM_swc.arxml.jinja2")
   
    if template is not None:
        file_path = os.path.join(output_dir, "NvM_swc.arxml")
    try:
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(config_dict))
            logger.info("Finish generating NvM_swc.arxml file")
    except UndefinedError as e:
        # find the error location in the template
        error_info = None
        frame_info_list = inspect.trace()
        frame_info_list.reverse()
        for frame_info in frame_info_list:
            if frame_info.function == "top-level template code":
                error_info = frame_info
                break
        filename = os.path.basename(error_info.filename)
        error_line = error_info.lineno
        error_code = error_info.code_context[0].strip()
        logger.error(f"UndefinedError: {filename}:{error_line} {error_code} {e}")
