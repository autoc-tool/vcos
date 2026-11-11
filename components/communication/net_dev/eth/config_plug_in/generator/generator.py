# /*
#  * Copyright (c) 2025 Li Auto Inc. and its affiliates
#  * Licensed under the Apache License, Version 2.0(the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
#  */
"""
ethif配置代码生成模块.
根据输入的EthIf_config.json, 然后生成EthIf_Cfg.h代码.
"""
import os,shutil
import inspect
from . import filter
from . import context
from jinja2 import Environment, FileSystemLoader, StrictUndefined, UndefinedError
from func_module.health_monitor.logger import logger
from func_module.generator.common.user_code_extract import UserCodeExtract

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    user_code_extract = UserCodeExtract()
    result = True
    logger.debug("Generate EthIf_Cfg dynamic code")
    output_file_dir = os.path.join(sys_config["gendata_path"])
    input_dict = filter.get_template_input_parameter_dict(config_dict["bsw"])
    input_dict.update({'sys_cfg': sys_config})
    ethif_callout_output_file_dir = sys_config['path']['bswcallout_path']
    input_dict_bsw_callout = user_code_extract.bsw_callout_user_code(ethif_callout_output_file_dir)
    if not input_dict_bsw_callout.get("EthIf_Eth_Callout", {}):
        # 没有文件时，赋空
        input_dict_bsw_callout["EthIf_Eth_Callout"] = {}
    input_dict.update({"functions": input_dict_bsw_callout["EthIf_Eth_Callout"]})
    # EthVersion_raw =  sys_config["module_cfg"]["Eth"]["autosar_version"]
    # EthVersion_raw = EthVersion_raw.replace(".", "")
    # TODO: Modify Eth related logic
    EthVersion_raw = "422"
    input_dict["EthGeneral"]["EthVersion"] = "ETHMCAL_AUTOSAR_VERSION_" + EthVersion_raw
    sys_config_dict = {
        "template_dir": DEFAULT_TEMPLATE_FILE_DIR,
        "output_dir": output_file_dir,
        "input_dict": input_dict,
        "sys_config": sys_config,
        "output_callout_dir": ethif_callout_output_file_dir
    }

    env = Environment(loader=FileSystemLoader(os.path.abspath(sys_config_dict["template_dir"])),
                      undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    # Create required directories
    rte_output_dir = os.path.realpath(os.path.join(sys_config_dict["output_dir"], "EthIf"))
    # logger.info(f"Creating {rte_output_dir} directories if they don"t already exist")
    os.makedirs(rte_output_dir, exist_ok=True)

    template_file_list = env.list_templates()
    for template_file in template_file_list:
        template = env.get_template(template_file)
        try:
            render_dynamic_code_files(template_file, template, sys_config_dict)
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


def render_dynamic_code_files(template_name, template, config_dict):
    """
    Render dynamic code template_file.
    """
    if template_name == "ethif_eth_callout.c.jinja2":
        input_dict = config_dict["input_dict"]
        ethif_callout_output_file_dir = config_dict["output_callout_dir"]
        file_path = os.path.join(ethif_callout_output_file_dir, "ethif_eth_callout.c")
        if os.path.exists(ethif_callout_output_file_dir) is False:
            os.makedirs(ethif_callout_output_file_dir)
        # 对原文件进行保存
        if os.path.exists(file_path):
            # 删除原文件
            os.remove(file_path)
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(input_dict))
            logger.info("Finish generating ethif_eth_callout.c file")
    elif template_name == "ethif_eth_callout.h.jinja2":
        input_dict = config_dict["input_dict"]
        ethif_callout_output_file_dir = config_dict["output_callout_dir"]
        file_path = os.path.join(ethif_callout_output_file_dir, "ethif_eth_callout.h")
        if os.path.exists(ethif_callout_output_file_dir) is False:
            os.makedirs(ethif_callout_output_file_dir)
        # 对原文件进行保存
        if os.path.exists(file_path):
            # 删除原文件
            os.remove(file_path)
        with open(file_path, "w", encoding="utf-8") as out_stream:
            out_stream.write(template.render(input_dict))
            logger.info("Finish generating ethif_eth_callout.h file")
    else:
        output_dir = config_dict.get("output_dir")
        if output_dir is None:
            logger.error("Output directory is not set in config")

        input_dict = config_dict["input_dict"]
        template_name = template_name[:template_name.find(".jinja2")]
        result = template.render(input_dict)

        with open(output_dir + "/EthIf/" + template_name, "w+", encoding="utf-8") as destfile:
            destfile.write(result)
        logger.info(f"Finish generating {template_name} file")
