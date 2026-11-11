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
import os
import inspect
from . import filter
from . import context
from jinja2 import Environment, FileSystemLoader, StrictUndefined, UndefinedError, Template
from func_module.health_monitor.logger import logger

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    output_file_dir = os.path.join(sys_config["gendata_path"])

    input_dict = filter.get_template_input_parameter_dict(config_dict, sys_config)
    config_dict = {
        "template_dir": DEFAULT_TEMPLATE_FILE_DIR,
        "output_dir": output_file_dir,
        "input_dict": input_dict,
        "sys_config": sys_config
    }

    env = Environment(loader=FileSystemLoader(os.path.abspath(DEFAULT_TEMPLATE_FILE_DIR)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    canif_output_dir = os.path.realpath(os.path.join(config_dict["output_dir"], "CanIf"))
    os.makedirs(canif_output_dir, exist_ok=True)

    template_file_list = env.list_templates()
    template_dict = {}
    for template_file in template_file_list:
        template_dict[template_file] = env.get_template(template_file)
    try:
        render_files(sys_config, template_dict, config_dict)
    except UndefinedError as e:
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


def generate_config_json(sys_config: dict, config_dict: dict):
    input_dict = filter.get_template_config_json_dict(config_dict)
    output_dir = os.path.realpath(os.path.join(sys_config["config_path"], "CanIf"))

    env = Environment(loader=FileSystemLoader(os.path.abspath(DEFAULT_TEMPLATE_FILE_DIR)), undefined=StrictUndefined)
    env.filters.update(context.get_filters())
    env.globals.update(context.get_functions())

    os.makedirs(output_dir, exist_ok=True)
    template = env.get_template("canif_cfg.json.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "canif_cfg.json")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            canif_config = {"canif_json": input_dict}
            out_stream.write(template.render(canif_config))
    logger.info(f"Finish generating canif_cfg.json file")


def render_files(sys_config, template_dict, config_dict):
    output_dir = config_dict.get("output_dir")
    if output_dir is None:
        logger.error("Output directory is not set in config")

    input_dict = config_dict["input_dict"]
    render_canif_cfg_file(sys_config, template_dict, input_dict, output_dir)
    render_canif_pbcfg_file(sys_config, template_dict, input_dict, output_dir)
    render_canif_can_file(sys_config, template_dict, input_dict, output_dir)
    render_canif_cantrcv_file(sys_config, template_dict, input_dict, output_dir)
    render_canif_lcfg_file(sys_config, template_dict, input_dict, output_dir)
    
def render_canif_cfg_file(sys_config, template_dict, input_dict, output_dir):
    template: Template = template_dict.get("canif_cfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "CanIf/canif_cfg.c")
        logger.debug("Start generating canif_cfg.c file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            config = {
                "sys_cfg": sys_config,
                "tx_pdus": input_dict["tx_pdus"],
                "rx_pdus": input_dict["rx_pdus"],
                "buffer_list": input_dict["buffer_list"],
                "hrh_cfglist": input_dict["hrh_cfglist"],
                "is_module_used": input_dict["is_module_used"],
                "is_module_rx_used": input_dict["is_module_rx_used"],
                "is_module_tx_used": input_dict["is_module_tx_used"],
                "canif_controllers_config": input_dict["canif_controllers_config"],
                "canif_ctrl_core_dict": input_dict["canif_ctrl_core_dict"],
                "canif_ctrl_multicore_enable": input_dict["canif_ctrl_multicore_enable"],
                "canif_ctrl_single_core_id": input_dict["canif_ctrl_single_core_id"],
                "canif_transceivers_config": input_dict["canif_transceivers_config"],
                "controller_channel_count": input_dict["controller_channel_count"],
                "trcv_channel_count": input_dict["trcv_channel_count"],
                "canif_init_config": input_dict["canif_init_config"],
                "canif_version": input_dict["canif_version"],
                "canif_public_cfg": input_dict["canif_public_cfg"],
                "canif_private_cfg": input_dict["canif_private_cfg"],
                "canif_dispatch_cfg": input_dict["canif_dispatch_cfg"],
                "trcv_api_enable": input_dict["trcv_api_enable"],
                "multicore_tx_pducfgs_dict": input_dict["multicore_tx_pducfgs_dict"],
                "multicore_txbuffercfgs_dict": input_dict["multicore_txbuffercfgs_dict"],
                "multicore_hrhcfgs_dict": input_dict["multicore_hrhcfgs_dict"],
                'multicore_rx_pducfgs_dict': input_dict["multicore_rx_pducfgs_dict"],
                'len_multicore_rx_pducfgs_dict': input_dict["len_multicore_rx_pducfgs_dict"],
                'len_multicore_tx_pducfgs_dict': input_dict["len_multicore_tx_pducfgs_dict"],
                "len_multicore_hrhcfgs_dict": input_dict["len_multicore_hrhcfgs_dict"],
                "len_multicore_txbuffercfgs_dict": input_dict["len_multicore_txbuffercfgs_dict"],
                "canif_activated_core_cnt": input_dict["canif_activated_core_cnt"],
                "master_core_id": input_dict["master_core_id"],
                "all_core_id_list": input_dict["all_core_id_list"],
                "all_core_id_name_list": input_dict["all_core_id_name_list"],
                "all_core_id_enbale_list": input_dict["all_core_id_enbale_list"],
                "canif_max_activated_core_id": input_dict["canif_max_activated_core_id"],
                "all_core_txbuffercfgs_inorder_list": input_dict["all_core_txbuffercfgs_inorder_list"],
                "all_core_tx_pducfgs_list": input_dict["all_core_tx_pducfgs_list"],
                "multicore_txbuffercfgs_start_end_index_dict": input_dict["multicore_txbuffercfgs_start_end_index_dict"],
                "max_lpdu_num_cores_txbuf_dict": input_dict["max_lpdu_num_cores_txbuf_dict"],
                "canif_manager_var_multi_app_or_private_master_core": input_dict["canif_manager_var_multi_app_or_private_master_core"],
                "jinja2_micro_define_list": input_dict["jinja2_micro_define_list"],
                "canif_txbuffer_size_memmap_section": input_dict["canif_txbuffer_size_memmap_section"],
                "app_coreid_dict": input_dict["app_coreid_dict"],
                "trcv_type_enable": input_dict["trcv_type_enable"],
                "trcv_enable_list": input_dict["trcv_enable_list"],
                "trcv_channel_enable_type_count": input_dict["trcv_channel_enable_type_count"],
                "offset_index_from_start_index_cantrcv1043": input_dict["offset_index_from_start_index_cantrcv1043"],
                "offset_index_from_start_index_cantrcv1145": input_dict["offset_index_from_start_index_cantrcv1145"],
            }
            out_stream.write(template.render(config))
            logger.info("Finish generating canif_cfg.c file")
            
    template = template_dict.get("canif_cfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "CanIf/canif_cfg.h")
        logger.debug("Start generating canif_cfg.h file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            config = {
                "sys_cfg": sys_config,
                "controller_channel_count": input_dict["controller_channel_count"],
                "trcv_channel_count": input_dict["trcv_channel_count"],
                "max_lpdu_num_txbuf": input_dict["max_lpdu_num_txbuf"],
                "is_module_used": input_dict["is_module_used"],
                "is_module_rx_used": input_dict["is_module_rx_used"],
                "is_module_tx_used": input_dict["is_module_tx_used"],
                "max_txbufs": input_dict["max_txbufs"],
                "max_dyn_pdus": input_dict["max_dyn_pdus"],
                "canif_controllers_config": input_dict["canif_controllers_config"],
                "canif_ctrl_core_dict": input_dict["canif_ctrl_core_dict"],
                "canif_ctrl_multicore_enable": input_dict["canif_ctrl_multicore_enable"],
                "canif_ctrl_single_core_id": input_dict["canif_ctrl_single_core_id"],
                "canif_transceivers_config": input_dict["canif_transceivers_config"],
                "controller_channel_count": input_dict["controller_channel_count"],
                "trcv_channel_count": input_dict["trcv_channel_count"],
                "canif_init_config": input_dict["canif_init_config"],
                "canif_version": input_dict["canif_version"],
                "canif_public_cfg": input_dict["canif_public_cfg"],
                "canif_private_cfg": input_dict["canif_private_cfg"],
                "hrh_cfglist": input_dict["hrh_cfglist"],
                "trcvApi_for_canif_cfg_h": input_dict["trcvApi_for_canif_cfg_h"],
                "trcv_api_enable": input_dict["trcv_api_enable"],
                "multicore_tx_pducfgs_dict": input_dict["multicore_tx_pducfgs_dict"],
                "multicore_txbuffercfgs_dict": input_dict["multicore_txbuffercfgs_dict"],
                "multicore_hrhcfgs_dict": input_dict["multicore_hrhcfgs_dict"],
                'multicore_rx_pducfgs_dict': input_dict["multicore_rx_pducfgs_dict"],
                "len_multicore_rx_pducfgs_dict": input_dict["len_multicore_rx_pducfgs_dict"],
                "len_multicore_tx_pducfgs_dict": input_dict["len_multicore_tx_pducfgs_dict"],
                "len_multicore_hrhcfgs_dict": input_dict["len_multicore_hrhcfgs_dict"],
                "len_multicore_txbuffercfgs_dict": input_dict["len_multicore_txbuffercfgs_dict"],
                "canif_activated_core_cnt": input_dict["canif_activated_core_cnt"],
                "master_core_id": input_dict["master_core_id"],
                "all_core_id_list": input_dict["all_core_id_list"],
                "all_core_id_name_list": input_dict["all_core_id_name_list"],
                "all_core_id_enbale_list": input_dict["all_core_id_enbale_list"],
                "canif_max_activated_core_id": input_dict["canif_max_activated_core_id"],
                "all_core_txbuffercfgs_inorder_list": input_dict["all_core_txbuffercfgs_inorder_list"],
                "all_core_tx_pducfgs_list": input_dict["all_core_tx_pducfgs_list"],
                "multicore_txbuffercfgs_start_end_index_dict": input_dict["multicore_txbuffercfgs_start_end_index_dict"],
                "max_lpdu_num_cores_txbuf_dict": input_dict["max_lpdu_num_cores_txbuf_dict"],
                "canif_manager_var_multi_app_or_private_master_core": input_dict["canif_manager_var_multi_app_or_private_master_core"],
                "jinja2_micro_define_list": input_dict["jinja2_micro_define_list"],
                "canif_txbuffer_size_memmap_section": input_dict["canif_txbuffer_size_memmap_section"],
                "ctrl_wakeup_found": input_dict["ctrl_wakeup_found"],
                "app_coreid_dict": input_dict["app_coreid_dict"],
                "trcv_wakeup_found": input_dict["trcv_wakeup_found"],
                "trcv_pn_support_found": input_dict["trcv_pn_support_found"],
                "trcv_type_enable": input_dict["trcv_type_enable"],
                "trcv_enable_list": input_dict["trcv_enable_list"],
                "trcv_channel_enable_type_count": input_dict["trcv_channel_enable_type_count"],
                "offset_index_from_start_index_cantrcv1043": input_dict["offset_index_from_start_index_cantrcv1043"],
                "offset_index_from_start_index_cantrcv1145": input_dict["offset_index_from_start_index_cantrcv1145"],
                "G_CANNM_RXPDU_UL_RXINDICATION_INDEX": input_dict["G_CANNM_RXPDU_UL_RXINDICATION_INDEX"],
                "can_hoh_order_type": input_dict["can_hoh_order_type"]
            }
            out_stream.write(template.render(config))
            logger.info("Finish generating canif_cfg.h file")


def render_canif_pbcfg_file(sys_config, template_dict, input_dict, output_dir):
    template: Template = template_dict.get("canif_pbcfg.c.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "CanIf/canif_pbcfg.c")
        logger.debug("Start generating canif_pbcfg.c file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            config = {
                "sys_cfg": sys_config,
            }
            out_stream.write(template.render(config))
            logger.info("Finish generating canif_pbcfg.c file")
            
    template = template_dict.get("canif_pbcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "CanIf/canif_pbcfg.h")
        logger.debug("Start generating canif_pbcfg.h file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            config = {
                "sys_cfg": sys_config,
                "tx_pdus": input_dict["tx_pdus"],
                "rx_pdus": input_dict["rx_pdus"],
                "all_core_tx_pducfgs_list": input_dict["all_core_tx_pducfgs_list"],
                "app_coreid_dict": input_dict["app_coreid_dict"],
            }
            out_stream.write(template.render(config))
            logger.info("Finish generating canif_pbcfg.h file")

def render_canif_can_file(sys_config, template_dict, input_dict, output_dir):
    template: Template = template_dict.get("canif_can.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "CanIf/canif_can.h")
        logger.debug("Start generating canif_can.h file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            config = {
                "sys_cfg": sys_config,
                "canif_public_cfg": input_dict["canif_public_cfg"],
            }
            out_stream.write(template.render(config))
            logger.info("Finish generating canif_can.h file")

def render_canif_cantrcv_file(sys_config, template_dict, input_dict, output_dir):
    template: Template = template_dict.get("canif_cantrcv.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "CanIf/canif_cantrcv.h")
        logger.debug("Start generating canif_cantrcv.h file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            config = {
                "sys_cfg": sys_config,
                "cantrcv_enable_list": input_dict["cantrcv_enable_list"],
                "cantrcv_enable_list_flag": input_dict["cantrcv_enable_list_flag"],
                "app_coreid_dict": input_dict["app_coreid_dict"],
            }
            out_stream.write(template.render(config))
            logger.info("Finish generating canif_cantrcv.h file")
                  
def render_canif_lcfg_file(sys_config, template_dict, input_dict, output_dir):
    template: Template = template_dict.get("canif_lcfg.h.jinja2")
    if template is not None:
        file_path = os.path.join(output_dir, "CanIf/canif_lcfg.h")
        logger.debug("Start generating canif_lcfg.h file")
        with open(file_path, "w", encoding="utf-8") as out_stream:
            config = {
                "sys_cfg": sys_config,
            }
            out_stream.write(template.render(config))
            logger.info("Finish generating canif_lcfg.h file")