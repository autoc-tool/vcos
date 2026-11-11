
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
import string
from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase
from components.rt_framework.config_plug_in.generator.context.functions import Function as OsFunction
from components.rt_framework.config_plug_in.generator import utils as OsUtils
from components.memory.memmap.config_plug_in.generator import utils

def format_map_info(key, val):
    return {
        'map_object': key,
        'map_target': val
    }

def common_gen_apps_map_info(scope_map, apps, layer):
    data_types = ['DATA', 'BSS']
    ret = []
    if isinstance(apps, str):
        for scope_key, scope_val in scope_map.items():
            for d_type in data_types:
                ret.append(format_map_info(f'{scope_key}_{d_type}',
                                                f'{apps}_{scope_val}_{layer}_{d_type}'))
    elif isinstance(apps, list):
        for app in apps:
            for scope_key, scope_val in scope_map.items():
                for d_type in data_types:
                    ret.append(format_map_info(f'{app}_{scope_key}_{d_type}',
                                                    f'{app}_{scope_val}_{layer}_{d_type}'))
    return ret

def gen_single_app_map_info(layer, apps):
    ret = []
    var_scope_property_map = {
        "PRIVATE":"PRIVATE",
        "SEMISHARED":"SEMISHARED"
    }
    ret += common_gen_apps_map_info(var_scope_property_map,
                                                apps[0], layer)

    ret += common_gen_apps_map_info(var_scope_property_map,
                                                apps, layer)

    var_scope_property_map = {
        "MULTI_APP":"MULTI_APP_SHARE"
    }
    app = 'BSW_APPS'
    ret += common_gen_apps_map_info(var_scope_property_map,
                                                app, layer)

    return ret

def gen_no_app_map_info(layer):
    var_scope_property_map = {
        "MULTI_APP":"MULTI_APP_SHARE",
        "SEMISHARED":"SEMISHARED",
        "PRIVATE":"MULTI_APP_SHARE"
    }
    app = 'BSW_APPS'
    ret = common_gen_apps_map_info(var_scope_property_map,
                                        app, layer)

    return ret

def gen_flash_map_info(layer, apps):
    if len(apps) != 1:
        app = 'BSW_APPS'
    else:
        app = apps[0]
    ret = [
        format_map_info("CALLOUT_CODE", f'{app}_CODE_{layer}_TEXT'),
        format_map_info("CODE", f'{app}_CODE_{layer}_TEXT'),
        format_map_info("CONST", f'{app}_CONST_{layer}_RODATA')
    ]

    return ret

def gen_calib_map_info(layer, calib_app):
    ret = [
        format_map_info("CALIB_CONST", f'{calib_app}_CALIBRATION_{layer}_RODATA')
    ]
    return ret

def gen_common_bsw_app_map_info(layer, bsw_apps):
    var_scope_property_map = {"PRIVATE":"PRIVATE"}
    return common_gen_apps_map_info(var_scope_property_map, bsw_apps, layer)

def gen_multi_app_map_info(layer, apps):
    ret = []
    var_scope_property_map = {
        "PRIVATE":"PRIVATE",
        "SEMISHARED":"SEMISHARED"
    }
    ret += common_gen_apps_map_info(var_scope_property_map,
                                      apps, layer)

    var_scope_property_map = {
        "MULTI_APP":"MULTI_APP_SHARE"
    }
    app = 'BSW_APPS'
    ret += common_gen_apps_map_info(var_scope_property_map,
                                      app, layer)
    return ret

def gen_memmap_internal_info(apps, attr_pairs, layers):
    ret = {}
    for app in apps:
        for attr in attr_pairs:
            for layer in layers:
                map_key = (f'{app}_{attr[0]}_{layer}_{attr[1]}').upper()
                map_val = (f'.{attr[1]}.{attr[0]}.{app}.{layer}').lower()
                ret[map_key] = map_val
    return ret


def gen_autosar_flash_map_info(layer, apps, calib_app):
    app = apps[0]
    ret = [
        format_map_info("CALLOUT_CODE", f'{app}_CODE_{layer}_TEXT'),
        format_map_info("CODE", f'{app}_CODE_{layer}_TEXT'),
        format_map_info("CONST_UNSPECIFIED", f'{app}_CONST_{layer}_RODATA'),
        format_map_info("CALIB_CONST_UNSPECIFIED", f'{calib_app}_CALIBRATION_{layer}_RODATA')
    ]
    return ret

def gen_autosar_single_app_map_info(layer, apps):
    ret = []
    var_scope_property_map = {
        "PRIVATE":"PRIVATE",
        "SEMISHARED":"SEMISHARED"
    }

    data_type_mapping = {
        'VAR_INIT_UNSPECIFIED': 'DATA',
        'VAR_ZERO_INIT_UNSPECIFIED': 'DATA',
        'VAR_NO_INIT_UNSPECIFIED': 'BSS'
    }
    def __append_as_map_info():
        ret.append(format_map_info(f'{scope_key}_{d_type}',
                                   f'{app}_{scope_val}_{layer}_{d_type_inner}'))
    app = apps[0]
    for scope_key, scope_val in var_scope_property_map.items():
        for d_type, d_type_inner in data_type_mapping.items():
            __append_as_map_info()

    scope_key = 'SHARED'
    scope_val = 'MULTI_APP_SHARE'
    app = 'ALL_APPS'
    for d_type, d_type_inner in data_type_mapping.items():
        __append_as_map_info()

    return ret

class Function(FunctionBase):
    _cache = []
    @staticmethod
    def clear_caches():
        OsFunction.clear_caches()
        for ch in Function._cache:
            ch.clear_cache()

    @staticmethod
    @pass_context
    def get_memmap_module_list(context, module_path):
        return OsUtils.get_module_list(context['memmap'], module_path)

    @staticmethod
    @pass_context
    def get_auto_memmap_module_dict(context, default_layer):
        IGNORED_MODULES = ['os', 'memmap', 'rte']
        candidate_modules = OsUtils.get_dict_path(context, ['sys_cfg', 'modules_info', 'codegen_module_dict']).keys()
        auto_gen_modules = [x.lower() for x in candidate_modules if x.lower() not in IGNORED_MODULES]
        rte_config = OsUtils.get_dict_path(context, ['config_dict', 'rte'])

        bsw2swc_mapping = {}
        bsw_instances = rte_config.get('bsw_instances', {})
        lower_bsw_instances = {}
        for k, v in bsw_instances.items():
            lower_bsw_instances[k.lower()] = v.get("event_to_task_mapping_entitys", [])
        for bsw_name in auto_gen_modules:
            bsw2swc_mapping[bsw_name] = lower_bsw_instances.get(bsw_name, [])

        swc2app_mapping = {}
        task_mappings_info = rte_config.get('task_mappings_info', {})
        for task_map_cfg in task_mappings_info.values():
            app_name = task_map_cfg['application']
            for swc_entity in task_map_cfg['schedulable_entity_info_list']:

                swc2app_mapping[swc_entity['name']] = app_name

        ret = {}
        for bsw_name, bsw_inst_cfg in bsw2swc_mapping.items():
            ret[bsw_name.lower()] = {
                'use_calib': False,
                'apps': [swc2app_mapping.get(x) for x in bsw_inst_cfg],
                'layer': default_layer
            }

        return ret

    @staticmethod
    @pass_context
    def get_autosar_memmap_module_dict(context, use_layer):
        asw_config = OsUtils.get_dict_path(context, ['config_dict', 'rte', 'asw_config'])
        ret = {}

        for swc_name, swc_cfg in asw_config.items():
            ret[swc_name] = {
                'use_calib': True,
                'apps': [swc_cfg['osapplication']],
                'standard': 'autosar',
                'layer': use_layer
            }
        return ret

    @staticmethod
    @pass_context
    @OsUtils.cache_once(_cache)
    def get_memmap_cfg(context):
        internal_layer = ''
        layer_dict = {}
        module_dict = {}

        layer_dict = Function.get_memmap_module_list(context, '/AUTOSAR/EcucDefs/Memmap/MemmapLayer')
        for key, mem_cfg in layer_dict.items():
            layer = {
                'internal': mem_cfg['InternalLayer']['Value']
            }
            if layer['internal']:
                internal_layer = key
            layer_dict[key.lower()] = layer

        module_dict.update(Function.get_auto_memmap_module_dict(context, internal_layer))

        # Auto inferenced memmap cfg is overrided by manual configuration.
        module_cfg_dict = Function.get_memmap_module_list(context, '/AUTOSAR/EcucDefs/Memmap/MemmapModule')
        for key, mem_cfg in module_cfg_dict.items():
            mod_dict = {
                'use_calib': mem_cfg['MemmapUseCalibration']['Value'],
                'apps': OsUtils.resolve_ref_list('MemmapApplicationRef', mem_cfg),
                'layer': OsUtils.get_last_item(mem_cfg['MemmapUseLayer']['ValueRef'])
            }
            module_dict[key.lower()] = mod_dict

        core_num = OsFunction.get_core_cnt(context)
        bsw_apps_list = [OsFunction.get_bsw_app_from_coreid(context, x) for x in range(core_num)]

        memmap_cfg = {
            "layers": layer_dict,
            "modules": module_dict,
            "swc_modules": Function.get_autosar_memmap_module_dict(context, internal_layer),
            "internal_layer": internal_layer,
            "bsw_apps":bsw_apps_list
        }

        return memmap_cfg

    @staticmethod
    @pass_context
    def get_memmap_info(context, module):
        memmap_cfg = Function.get_memmap_cfg(context)

        if module in memmap_cfg['modules']:
            mod_cfg = memmap_cfg['modules'][module]
            use_rtfw_style = True
        else:
            mod_cfg = memmap_cfg['swc_modules'][module]
            use_rtfw_style = False    # use autosar style

        ref_apps = mod_cfg['apps']
        ref_layer = mod_cfg['layer']

        # We now use the bsw app of the master core as the
        # default calibration app as a temporary strategy.
        default_calib_app = memmap_cfg['bsw_apps'][0]

        memmap_info = []

        if use_rtfw_style:
            memmap_info += gen_flash_map_info(ref_layer, ref_apps)
            if mod_cfg['use_calib']:
                memmap_info += gen_calib_map_info(ref_layer, default_calib_app)

            if len(ref_apps) == 0:
                memmap_info += gen_no_app_map_info(ref_layer)
            elif len(ref_apps) == 1:
                memmap_info += gen_single_app_map_info(ref_layer, ref_apps)
            else:
                memmap_info += gen_multi_app_map_info(ref_layer, ref_apps)

            gen_bsw_apps = [x for x in memmap_cfg['bsw_apps'] if x not in ref_apps]
            memmap_info += gen_common_bsw_app_map_info(ref_layer, gen_bsw_apps)

        else:
            memmap_info += gen_autosar_flash_map_info(ref_layer, ref_apps, default_calib_app)
            memmap_info += gen_autosar_single_app_map_info(ref_layer, ref_apps)

        ret_info = {'mapping_info': memmap_info}

        return ret_info

    @staticmethod
    @pass_context
    @OsUtils.cache_once(_cache)
    def get_memmap_internal_cfg(context):
        ret = {}
        memmap_cfg = Function.get_memmap_cfg(context)
        app_cfg = OsFunction.get_app_cfg(context)

        int_layer = memmap_cfg['internal_layer']
        all_layer = memmap_cfg['layers']
        ret.update({
            'CODE': f'.text.code.all_apps.{int_layer}',
            'CALLOUT_CODE': f'.text.code.all_apps.{int_layer}',
            'CONST': f'.rodata.const.all_apps.{int_layer}'
        })

        apps = ['all_apps']
        attr_pairs = [('multi_app_share', 'data'), ('multi_app_share', 'bss'),
                      ('code', 'text'), ('const', 'rodata')]
        ret.update(gen_memmap_internal_info(apps, attr_pairs, all_layer))

        apps = ['bsw_apps']
        core_num = OsFunction.get_core_cnt(context)
        apps += [f'core{x}_apps' for x in range(core_num)]

        attr_pairs = [('multi_app_share', 'data'), ('multi_app_share', 'bss'),
                      ('semishared', 'data'), ('semishared', 'bss'),
                      ('private', 'data'), ('private', 'bss'),
                      ('code', 'text'), ('const', 'rodata')]
        ret.update(gen_memmap_internal_info(apps, attr_pairs, all_layer))

        # per application
        attr_pairs = [('semishared', 'data'), ('private', 'data'),
                      ('semishared', 'bss'), ('private', 'bss'),
                      ('calibration', 'rodata'), ('const', 'rodata'),
                      ('code', 'text')]
        ret.update(gen_memmap_internal_info(app_cfg.keys(), attr_pairs, all_layer))

        return ret

    @staticmethod
    @pass_context
    @OsUtils.cache_once(_cache)
    def get_task_stacks_sections(context):
        ret_key = []
        ret_values = []
        memmap_cfg = Function.get_memmap_cfg(context)
        internal_layer = memmap_cfg['internal_layer']
        app_cfg = OsFunction.get_app_cfg(context)
        task_cfg = OsFunction.get_task_cfg(context)
        for app_cfg in app_cfg.values():
            core_id = app_cfg['CoreNumber']
            task_list = app_cfg['AppTaskList']
            ret_values += [(f'.bss.core{core_id}_stack.{x}.{internal_layer}').lower() for x in task_list]
            ret_key += [task_cfg[x]['TaskStackSection'] for x in task_list]

        """ add share task stack section """
        task_stack_sharing_info, task_stack_sharing_ref = OsFunction.get_task_stack_sharing(context)
        for core_id , combs in task_stack_sharing_info.items():
            for comb, comb_value in combs.items():
                ret_values.append(f'.bss.core{core_id}_stack.{comb}.{internal_layer}'.lower())
                ret_key.append(f'OS_CORE{core_id}_{comb.upper()}_STACK_SECTION')

        return dict(zip(ret_key, ret_values))

    @staticmethod
    @pass_context
    @OsUtils.cache_once(_cache)
    def get_hook_stacks_sections(context):
        ret_key = []
        ret_values = []
        memmap_cfg = Function.get_memmap_cfg(context)
        internal_layer = memmap_cfg['internal_layer']
        hook_cfg, _ = OsFunction.get_os_hook_cfg(context)
        core_cfg = OsFunction.get_core_configs(context)

        for hook_name, hook_val in hook_cfg.items():
            if hook_val['enabled'] == True:
                for core_val in core_cfg.values():
                    core_id = core_val['core_id'][-1]
                    ret_values.append(f'.bss.core{core_id}_stack.{hook_name}.{internal_layer}')
                    ret_key.append(f'OS_CORE{core_id}_{hook_name.upper()}_STACK_SECTION')
        return dict(zip(ret_key, ret_values))

    @staticmethod
    @pass_context
    @OsUtils.cache_once(_cache)
    def get_app_hook_stacks_sections(context):
        ret_key = []
        ret_values = []
        memmap_cfg = Function.get_memmap_cfg(context)
        internal_layer = memmap_cfg['internal_layer']
        app_cfg = OsFunction.get_app_cfg(context)

        all_apps_hook_cfg = OsFunction.get_app_hook_cfg(context)
        for app_name, app_hook_cfg in all_apps_hook_cfg.items():
            for hook_cfg in app_hook_cfg.values():
                ret_key.append(hook_cfg['stack_name'])
                ret_values.append(f'.bss.core{app_cfg[app_name]["CoreNumber"]}_stack.{hook_cfg["app_hook_name"]}.{internal_layer}'.lower())

        return dict(zip(ret_key, ret_values))

    @staticmethod
    @pass_context
    def get_global_sections(context):
        memmap_cfg = Function.get_memmap_cfg(context)
        internal_layer = memmap_cfg['internal_layer']
        ret = {
            'OS_CODE_SECTION': f'.text.code.all_apps.{internal_layer}',
            'OS_CONST_SECTION': f'.rodata.const.all_apps.{internal_layer}',
            'OS_GLOBAL_BSS_SECTION': f'.bss.multi_app_share.all_apps.{internal_layer}',
            'OS_GLOBAL_DATA_SECTION': f'.data.multi_app_share.all_apps.{internal_layer}'
        }

        return ret

    @staticmethod
    @pass_context
    def get_core_sections(context):
        ret = {}
        memmap_cfg = Function.get_memmap_cfg(context)
        core_cfg = OsFunction.get_core_configs(context)
        internal_layer = memmap_cfg['internal_layer']

        for core_val in core_cfg.values():
            core_id = core_val["core_id"][-1]
            ret.update({
                f'OS_CORE{core_id}_BSS_SECTION': f'.bss.multi_app_share.core{core_id}_apps.{internal_layer}',
                f'OS_CORE{core_id}_DATA_SECTION': f'.data.multi_app_share.core{core_id}_apps.{internal_layer}'
            })

        return ret

    @staticmethod
    @pass_context
    def get_app_sections(context):
        ret = {}
        memmap_cfg = Function.get_memmap_cfg(context)
        app_cfg = OsFunction.get_app_cfg(context)
        internal_layer = memmap_cfg['internal_layer']

        for app_name in app_cfg.keys():
            app_name_u = app_name.upper()
            app_name_l = app_name.lower()
            ret.update({
                f'OS_{app_name_u}_BSS_SECTION': f'.bss.private.{app_name_l}.{internal_layer}',
                f'OS_{app_name_u}_DATA_SECTION': f'.data.private.{app_name_l}.{internal_layer}'
            })

        return ret

    @staticmethod
    @pass_context
    @OsUtils.cache_once(_cache)
    def get_all_map_targets(context):
        memmap_cfg = Function.get_memmap_internal_cfg(context)
        map_targets = list(set(memmap_cfg.values()))
        map_targets += Function.get_task_stacks_sections(context).values()
        map_targets += Function.get_hook_stacks_sections(context).values()
        map_targets += Function.get_app_hook_stacks_sections(context).values()
        return map_targets

    @staticmethod
    @pass_context
    @OsUtils.cache_once(_cache)
    def group_ld_config_by_attribute(context):
        map_targets = Function.get_all_map_targets(context)
        cfg_list = []

        for map_val in map_targets:
            data_type, scope, owner, layer = [p for p in map_val.split(".") if p]
            ld_cfg = {
                'scope': scope,
                'data_type': data_type,
                'layer': layer,
                'owner': owner,
                'target': map_val
            }
            cfg_list.append(ld_cfg)

        cfg_list = sorted(cfg_list, key=lambda x: x['target'])
        return OsUtils.recursive_group(cfg_list, ['scope', 'owner', 'layer', 'data_type'])

    @staticmethod
    @pass_context
    def get_ld_config(context):
        ld_cfg_dict = Function.group_ld_config_by_attribute(context)
        target_scope_ld_cfg = ld_cfg_dict.get(context['scope'], {})
        task_stack_sharing_info, task_stack_sharing_ref = OsFunction.get_task_stack_sharing(context)
        if not target_scope_ld_cfg:
            return {}

        target_owner = [x for x in context['owner'] if 'unspecified' not in x]

        if context['core'] != 'global':
            target_apps = OsFunction.get_app_name_list_by_core(context, coreid=int(context['core'][-1]))
            if 'unspecified_app' in context['owner']:
                target_owner += [x.lower() for x in target_apps]
            if 'unspecified_task' in context['owner']:
                app_cfgs = OsFunction.get_app_cfg(context)
                for app in target_apps:
                    target_owner += [x.lower() for x in app_cfgs[app]['AppTaskList']]
                """ add share task stack section """
                combs = task_stack_sharing_info.get(context['core'][-1], {})
                for comb, comb_value in combs.items():
                    target_owner += [f'{comb.lower()}']
            if 'unspecified_hook' in context['owner']:
                hook_cfg, _ = OsFunction.get_os_hook_cfg(context)
                if hook_cfg["os_sharehook"]["enabled"]:
                    target_owner += ["os_sharehook"]
                else:
                    for hook_name, hook_val in hook_cfg.items():
                        if hook_name != "os_sharehook" and hook_val['enabled'] == True:
                            target_owner += [hook_name]
                    all_apps_hook_cfg = OsFunction.get_app_hook_cfg(context)
                    for app in target_apps:
                        for hook_cfg in all_apps_hook_cfg[app].values():
                            target_owner.append(hook_cfg['app_hook_name'])

        cfg_dict = {}
        for t_owner in target_owner:
            cfg_dict[t_owner] = target_scope_ld_cfg.get(t_owner, {})
        return cfg_dict

    @staticmethod
    @pass_context
    def get_all_ram_section_config(context):
        ret = []
        map_targets = Function.get_all_map_targets(context)

        for cfg in map_targets:
            if context['target_data_type'] in cfg:
                ret.append(cfg)

        return ret
