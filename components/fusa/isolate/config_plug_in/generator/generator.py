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
from components.rt_framework.config_plug_in.generator.utils import *

from . import context

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    context.Function.clear_caches()
    config = prepare_os_base_config(sys_config, config_dict)

    env_ui = Environment(loader=FileSystemLoader(DEFAULT_TEMPLATE_FILE_DIR),
                         undefined=StrictUndefined)
    env_ui.filters.update(context.get_filters())
    env_ui.globals.update(context.get_functions())

    output_dir = os.path.realpath(
        os.path.join(sys_config["gendata_path"], "Isolate"))

    config['isolate'] = get_dict_path(config_dict, ['bsw', 'isolate', 'ArPackages', 'ActiveEcuC', 'Isolate'])

    # ################################################## #
    #   protected procedure call                         #
    # ################################################## #
    template = env_ui.get_template("isolate_ppc_cfg.h.jinja2")
    render_os_template(template, output_dir, "isolate_ppc_cfg.h", config)

    template = env_ui.get_template("isolate_ppc_cfg.c.jinja2")
    render_os_template(template, output_dir, "isolate_ppc_cfg.c", config)

    isolate_ppc_cfg = context.Function.get_isolate_ppc_cfg(config)
    template = env_ui.get_template("isolate_ppc_module.h.jinja2")
    for mod_name in isolate_ppc_cfg.keys():
        config['module_name'] = mod_name
        render_os_template(template, output_dir, f"{mod_name}_ppc_cfg.h", config)

    return True
