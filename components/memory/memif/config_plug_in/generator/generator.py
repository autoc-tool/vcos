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
MemIf配置代码生成模块.
根据输入的config.json, 然后生成memif_cfg.h代码.
"""
import os
from . import filter
from . import context
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined
CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    logger.debug("Generate MemIf dynamic code")
    input_dict = filter.get_template_dynamic_code_dict(config_dict["bsw"]["memif"])
    template_dir = os.path.join(DEFAULT_TEMPLATE_FILE_DIR)
    output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "MemIf"))

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(template_dir)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())
    # Create required directories

    os.makedirs(output_dir, exist_ok=True)
    input_dict = input_dict["ArPackages"]["ActiveEcuC"]['MemIf']
    Memif_config = {"Memif_General": input_dict, "sys_cfg": sys_config}

    template = env.get_template("memif_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "memif_cfg.h")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(Memif_config))
            logger.info("Finish generating memif_cfg.h file")

    template = env.get_template("memif_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "memif_cfg.c")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(Memif_config))
            logger.info("Finish generating memif_cfg.c file")
    return True