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
from . import filter
from . import context
from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined, UndefinedError, Template

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    """
    generate cansm_*.c/h code
    """
    output_dir = os.path.realpath(os.path.join(sys_config["gendata_path"], "CanSM"))
    os.makedirs(output_dir, exist_ok=True)

    # Create cansm_input_dict
    input_dict = filter.get_template_input_parameter_dict(config_dict["bsw"], config_dict["rte"])
    if len(input_dict) == 0 or len(input_dict['cansm_general']) == 0:
        return False
    input_dict.update({'sys_cfg': sys_config})

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(DEFAULT_TEMPLATE_FILE_DIR)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    # Render all of the files.
    # We use a gross while True loop to allow state.additional_files to
    # be appended to as it is processed.
    template_file_list = env.list_templates()
    template_dict = {}
    for template_file in template_file_list:
        template_dict[template_file] = env.get_template(template_file)
    try:
        render_files(template_dict, input_dict, output_dir)
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
        return False
    return True


def render_files(template_dict, input_dict, output_dir):
    """
    Render template_file.
    """
    if output_dir is None:
        logger.error("Output directory is not set in config")

    template: Template = template_dict.get("cansm_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "cansm_cfg.c")
        logger.debug("Start generating cansm_cfg.c file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            # process template file
            out_stream.write(template.render(input_dict))
            logger.info("Finish generating cansm_cfg.c file")

    template: Template = template_dict.get("cansm_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "cansm_cfg.h")
        logger.debug("Start generating cansm_cfg.h file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            # process template file
            out_stream.write(template.render(input_dict))
            logger.info("Finish generating cansm_cfg.h file")

    template = template_dict.get("cansm_lcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "cansm_lcfg.c")
        logger.debug("Start generating cansm_lcfg.c file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            # process template file
            out_stream.write(template.render(input_dict))
            logger.info("Finish generating cansm_lcfg.c file")

    template = template_dict.get("cansm_lcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "cansm_lcfg.h")
        logger.debug("Start generating cansm_lcfg.h file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            # process template file
            out_stream.write(template.render(input_dict))
            logger.info("Finish generating cansm_lcfg.h file")
