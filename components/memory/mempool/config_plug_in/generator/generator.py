############################################################################
#  Copyright (c) 2025 Li Auto Inc. and its affiliates
#  Licensed under the Apache License, Version 2.0(the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
############################################################################

import os

from func_module.health_monitor.logger import logger
from jinja2 import Environment, FileSystemLoader, StrictUndefined
from .utils import get_last_item, get_os_mempool_all_config_dict
from components.rt_framework.config_plug_in.generator.utils import get_dict_path

from . import context

CUR_FILE_PATH = os.path.dirname(__file__)
DEFAULT_TEMPLATE_FILE_DIR = os.path.join(CUR_FILE_PATH, "template")

def Os_CoreInfo_Get(ecuc_dict: dict):
    ret = {}

    for k, v in ecuc_dict["EcucHardware"].items():
        if "EcucCoreDefinition" not in k:
            continue
        coreid = str(v['EcucCoreId']['Value'])
        hw_coreid = str(get_last_item(v['EcucCoreHwRef']['ValueRef'])[-1])
        ret['Os_Core' + coreid] = {
            'core_id': 'OS_CORE_ID_' + coreid,
            'PhyId': 'OS_CORE_PHYSICAL_ID_' + hw_coreid
        }

    return ret

def render_os_template(template, out_dir, file_name, config, os_mempool_all_cfg=None):
    if template is not None:
        file_path = os.path.join(out_dir, file_name)
        with open(file_path, "w", encoding="utf-8") as out_stream:
            if os_mempool_all_cfg is not None:
                out_stream.write(template.render(config, os_mempool_all_cfg=os_mempool_all_cfg))
            else:
                out_stream.write(template.render(config))
            out_stream.flush()
        logger.info("Finish generating %s file" % (file_name))


def generate_mempool(env_ui, config, config_dict, output_dir):
    os_mempool_ui_dict = config_dict.get("bsw",{}).get("mempool",{}).get("ArPackages",{}).get("ActiveEcuC",{}).get("MemPool",{})
    os_mempool_all_cfg = get_os_mempool_all_config_dict(os_mempool_ui_dict, config_dict)
    template = env_ui.get_template("mempool_cfg.h.jinja2")
    render_os_template(template, output_dir, "mempool_cfg.h", config, os_mempool_all_cfg)
    template = env_ui.get_template("mempool_cfg.c.jinja2")
    render_os_template(template, output_dir, "mempool_cfg.c", config, os_mempool_all_cfg)

    template = env_ui.get_template("mempool_lcfg.h.jinja2")
    render_os_template(template, output_dir, "mempool_lcfg.h", config, os_mempool_all_cfg)


def generate_dynamic_code(sys_config: dict, config_dict: dict):
    context.Function.clear_caches()

    # get EcuC Configs
    ecuc_dict = get_dict_path(config_dict, ['bsw', 'ecuc', 'ArPackages', 'ActiveEcuC', 'EcuC'])

    # get core information and hw coreid information from UI input and add to os dict
    core_dict = Os_CoreInfo_Get(ecuc_dict)

    project_target = sys_config.get("target", "")
    if project_target in sys_config["sys_target_dict"]["validate"]["ignore_option"]:
        sys_config["sys_target_dict"]["validate"]["ignore_option"] = \
            sys_config["sys_target_dict"]["validate"]["ignore_option"][project_target]

    output_dir = os.path.realpath(
        os.path.join(sys_config["gendata_path"], "MemPool"))
    # more hardware platform template on the way~

    # save the configuration of ui
    input_ui_dict = get_dict_path(config_dict, ['bsw', 'os', 'ArPackages', 'ActiveEcuC', 'Os'])

    # Build our rendering environment.
    env_ui = Environment(loader=FileSystemLoader(DEFAULT_TEMPLATE_FILE_DIR),
                         undefined=StrictUndefined)
    env_ui.filters.update(context.get_filters())
    env_ui.globals.update(context.get_functions())

    config = {
        "os_ui": input_ui_dict,
        "sys_cfg": sys_config,
        "os_core": core_dict
    }

    # ################################################## #
    #    Os Memory Pool                                  #
    # ################################################## #
    generate_mempool(env_ui, config, config_dict, output_dir)

    return True
