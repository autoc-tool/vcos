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
from jinja2 import Environment, FileSystemLoader, StrictUndefined
from func_module.health_monitor.logger import logger
from components.rt_framework.config_plug_in.generator.generator import prepare_os_base_config, render_os_template
from components.rt_framework.config_plug_in.generator import utils as OsUtils
from .utils import *

from . import context

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")

def generate_memmap_module_header(template, out_dir, config):
    memmap_cfg = context.Function.get_memmap_cfg(config)
    gen_modules = memmap_cfg['modules'].keys()
    for mod_name in gen_modules:
        gen_file_name = f'{mod_name}_memmap.h'
        module_config = {**config, 'module_name': mod_name}
        render_os_template(template, out_dir, gen_file_name, module_config)

def generate_memmap(env_ui, config):
    memmap_output_dir = os.path.realpath(os.path.join(config['sys_cfg']['gendata_path'], "Memmap"))
    if not os.path.exists(memmap_output_dir):
        os.makedirs(memmap_output_dir)
    template = env_ui.get_template("module_memmap.h.jinja2")
    generate_memmap_module_header(template, memmap_output_dir, config)

    template = env_ui.get_template("memmap_int.h.jinja2")
    render_os_template(template, memmap_output_dir, "memmap_int.h", config)

    template = env_ui.get_template("rtfw_compiler_cfg.h.jinja2")
    render_os_template(template, memmap_output_dir, "rtfw_compiler_cfg.h", config)

def generate_autosar_memmap_module_header(template, out_dir, config):
    memmap_cfg = context.Function.get_memmap_cfg(config)
    gen_modules = memmap_cfg['swc_modules']
    for mod_name in gen_modules.keys():
        gen_file_name = f'{mod_name}_MemMap.h'
        module_config = {**config, 'module_name': mod_name}
        render_os_template(template, out_dir, gen_file_name, module_config)

def generate_autosar_style_memmap(env_ui, config):
    memmap_output_dir = os.path.realpath(os.path.join(config['sys_cfg']['gendata_path'], "Appl"))
    if not os.path.exists(memmap_output_dir):
        os.makedirs(memmap_output_dir)
    template = env_ui.get_template("module_memmap.h.jinja2")
    generate_autosar_memmap_module_header(template, memmap_output_dir, config)

def generate_ld_scripts(env_ui, config):
    memmap_output_dir = os.path.realpath(os.path.join(config['sys_cfg']['gendata_path'], "scripts"))
    if not os.path.exists(memmap_output_dir):
        os.makedirs(memmap_output_dir)

    compiler_name = config['sys_cfg']['compiler']
    target_name = config['sys_cfg']['target']

    # We provide special jinja2 template to meet platform specific requirements
    if target_name == 'E3650':
        template = env_ui.get_template(f"ld_scripts_{compiler_name}_e3650.ld.jinja2")
    else:
        template = env_ui.get_template(f"ld_scripts_{compiler_name}.ld.jinja2")

    default_ram_scope = ['multi_app_share', 'semishared', 'private']
    default_rom_scope = ['const', 'code']
    module_config = config.copy()

    if compiler_name == 'gcc':
        ram_table_template = env_ui.get_template("gcc_ram_table.ld.jinja2")
        module_config['target_data_type'] = '.bss'
        render_os_template(ram_table_template,
                           memmap_output_dir, 'gcc_clear_table.ld', module_config)

        module_config['target_data_type'] = '.data'
        render_os_template(ram_table_template, memmap_output_dir,
                           'gcc_copy_table.ld', module_config)
        module_config['target_data_type'] = ''

    def _render_ld_current_module_config():
        filename = module_config['filename'] + '.ld'
        render_os_template(template, memmap_output_dir,
                           filename, module_config)

    module_config['core'] = 'global'

    module_config['owner'] = ['all_apps']
    module_config['scope'] = 'multi_app_share'
    module_config['filename'] = f'default_all_apps_multi_app_share'
    _render_ld_current_module_config()

    module_config['owner'] = ['bsw_apps']
    for scope in default_ram_scope:
        module_config['scope'] = scope
        module_config['filename'] = f'default_bsw_apps_{scope}'
        _render_ld_current_module_config()

    for scope in default_rom_scope:
        module_config['owner'] = ['all_apps', 'bsw_apps']
        module_config['scope'] = scope
        module_config['filename'] = f'default_{scope}'
        _render_ld_current_module_config()

    core_num = len(module_config['os_core'])
    for coreid in range(core_num):
        module_config['core'] = f'core{coreid}'

        module_config['owner'] = [f'core{coreid}_apps']
        for scope in default_ram_scope:
            module_config['scope'] = scope
            module_config['filename'] = f'default_core{coreid}_apps_{scope}'
            _render_ld_current_module_config()

        module_config['owner'] = ['unspecified_app']
        for scope in ['semishared', 'private']:
            module_config['scope'] = scope
            module_config['filename'] = f'default_app_{scope}_core{coreid}'
            _render_ld_current_module_config()

        module_config['owner'] = ['unspecified_task', 'unspecified_hook']
        module_config['scope'] = f'core{coreid}_stack'
        module_config['filename'] = f'default_stacks_core{coreid}'
        _render_ld_current_module_config()

        module_config['owner'] = [f'core{coreid}_apps', 'unspecified_app']
        for scope in default_rom_scope:
            module_config['scope'] = scope
            module_config['filename'] = f'default_{scope}_core{coreid}'
            _render_ld_current_module_config()


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    context.Function.clear_caches()
    config = prepare_os_base_config(sys_config, config_dict)

    env_ui = Environment(loader=FileSystemLoader(DEFAULT_TEMPLATE_FILE_DIR),
                         undefined=StrictUndefined)
    env_ui.filters.update(context.get_filters())
    env_ui.globals.update(context.get_functions())

    config['memmap'] = OsUtils.get_dict_path(config_dict, ['bsw', 'memmap', 'ArPackages', 'ActiveEcuC', 'Memmap'])

    # ################################################## #
    #    MemMap                                          #
    # ################################################## #
    generate_memmap(env_ui, config)

    # ################################################## #
    #   Autosar CP MemMap                                #
    # ################################################## #
    generate_autosar_style_memmap(env_ui, config)

    # ################################################## #
    #    ld script                                       #
    # ################################################## #
    generate_ld_scripts(env_ui, config)

    return True
