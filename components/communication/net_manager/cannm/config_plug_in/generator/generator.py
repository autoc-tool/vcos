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
from jinja2 import Environment, FileSystemLoader, StrictUndefined, UndefinedError
from func_module.health_monitor.logger import logger

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    """
    generate cannm_cfg.c/h & cannm_lcfg.c code
    """
    result = True
    logger.debug("Generate cannm_cfg dynamic code")
    output_file_dir = os.path.join(sys_config["gendata_path"])
    input_dict = filter.get_template_input_parameter_dict(config_dict)
    sys_config_dict = {
        "template_dir": DEFAULT_TEMPLATE_FILE_DIR,
        "output_dir": output_file_dir,
        "input_dict": input_dict,
        "sys_config": sys_config
    }

    # Build our rendering environment.
    env = Environment(loader=FileSystemLoader(os.path.abspath(sys_config_dict["template_dir"])),
                      undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    # Create required directories
    rte_output_dir = os.path.realpath(os.path.join(sys_config_dict["output_dir"], "CanNm"))
    # logger.info(f"Creating {rte_output_dir} directories if they don"t already exist")
    os.makedirs(rte_output_dir, exist_ok=True)

    # Render all of the files.
    # We use a gross while True loop to allow state.additional_files to
    # be appended to as it is processed.

    template_file_list = env.list_templates()
    for template_file in template_file_list:
        if template_file == "cannm_cfg.json.jinja2":
            continue
        template = env.get_template(template_file)
        try:
            render_dynamic_code_files(sys_config, template_file, template, sys_config_dict)
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
            result = False
    return result


def render_dynamic_code_files(sys_config, template_name, template, config_dict):
    """
    Render dynamic code template_file.
    """
    output_dir = config_dict.get("output_dir")
    if output_dir is None:
        logger.error("Output directory is not set in config")

    input_dict = config_dict["input_dict"]

    template_name = template_name[:template_name.find(".jinja")]

    result = template.render(CanNmConfig=input_dict, sys_cfg=sys_config)

    with open(output_dir + "/CanNm/" + template_name, "w+", encoding="utf-8") as destfile:
        destfile.write(result)
    logger.info(f"Finish generating {template_name} file")
