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
from components.rt_framework.config_plug_in.generator.utils import *

class Function(FunctionBase):
    _cache = []

    @staticmethod
    def clear_caches():
        OsFunction.clear_caches()
        for ch in Function._cache:
            ch.clear_cache()

    @staticmethod
    @pass_context
    def get_isolate_module_list(context, module_path):
        """get os module list"""
        return get_module_list(context['isolate'], module_path)

    @staticmethod
    @pass_context
    @cache_once(_cache)
    def get_isolate_ppc_cfg(context):
        isolate_ppc_cfg = {}
        app_cfg = OsFunction.get_app_cfg(context)
        num_of_cores = len(context['os_core'])
        ppc_func_id = 0
        isolate_ui_dict = Function.get_isolate_module_list(context, '/AUTOSAR/EcucDefs/Isolate/Module')
        for module_name, module_val in isolate_ui_dict.items():
            module_cfg = {
                "AppList": [],
                "PpcList": []
            }
            app_ref_list = resolve_ref_list('OsApplicationRef', module_val)
            core_app_dict = {}
            for app_name in app_ref_list:
                core_app_dict[app_cfg[app_name]['CoreNumber']] = app_cfg[app_name]['AppId']

            module_cfg['AppList'] = [core_app_dict.get(str(x), "OS_APPLICATION_CNT") for x in range(num_of_cores)]

            ppc_list = []
            for function_value in module_val.values():
                if isinstance(function_value, dict) and "PPCFunction" in function_value["DefinitionRef"]:

                    func_cfg = {
                        "FunctionName": function_value["PPCFunctionName"]["Value"],
                        "FunctionID": ppc_func_id,
                        "FunctionIDName": f'PPC_{(function_value["PPCFunctionName"]["Value"]).upper()}_ID',
                        "IsolateCallType": "ProtectedProcedureCall",
                        "IsolateCallRetRequired": function_value["IsolateCallRetRequired"]["Value"],
                        "IsolateCallRetType": "int",
                        "IsolateCallParamNumber": function_value["IsolateCallParamNumber"]["Value"],
                        "VariadicArguments": function_value["VariadicArguments"]["Value"]
                    }
                    ppc_func_id += 1

                    if function_value["IsolateCallRetType"]["Value"]:
                        func_cfg["IsolateCallRetType"] = function_value["IsolateCallRetType"]["Value"]

                    ppc_list.append(func_cfg)
            module_cfg["PpcList"] = ppc_list
            isolate_ppc_cfg[module_name] = module_cfg
        return isolate_ppc_cfg

    @staticmethod
    def get_isolate_call_param(isolate_func_dict):
        param_num = isolate_func_dict['IsolateCallParamNumber']
        is_variadic = isolate_func_dict['VariadicArguments']
        letters = list(string.ascii_lowercase[:param_num])

        if is_variadic:
            letters.append('...')

        return ', '.join(letters)
