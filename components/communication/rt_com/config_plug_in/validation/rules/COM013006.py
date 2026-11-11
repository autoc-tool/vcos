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
from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *

# 通过definition 获取对应的dict
def com_tools_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"].split("/")[-1] == definition_ref:    
                    return v    
    return {}

def check_prefix(string):
    prefix = 'Com_MainFunction'
    # 检查字符串前3个字符是否为'Com_'
    if string.startswith(prefix):
        return True
    else:
        return False

# 互斥校验：Com MainFunction必须有Ref关联
class Com013006Rule(BasicRule):
    ruleset_name = "Com013006"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_com_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Validation Of ComMainFunction Name!"

        self.durable_init()

    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(+m.status)
                def event(c):
                    pass

                @when_all(+m.comsignal_name)
                def fact(c):
                    self.__class__.validation_result = True

    def check_validation_run(self):
        if not self.modules_are_exist(["Com"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True


    @timing_decorator
    def validation_run(self):
        validation_data_list = []
        validataion_data_path_list = []
        valid = True

        # 1）准备校验数据
        ComConfig = {}
        ecuc_com_dict = self.ecuc_com_dict
        if "Com" in ecuc_com_dict.keys():
            com_dict = ecuc_com_dict['Com']['ArPackages']['ActiveEcuC']['Com']
            com_config_dict = com_tools_get_dict_by_definition_ref(com_dict,"ComConfig")
        else:
            com_config_dict = com_tools_get_dict_by_definition_ref(ecuc_com_dict,"ComConfig") 
        com_ipdu_dict = {}
        ComMainFunction_tx = {}
        ComMainFunction_rx = {}
        ComMainFunction_gw = {}
        for ComConfigKey, value in com_config_dict.items():
            if ComConfigKey == "DefinitionRef" or ComConfigKey == "Tag":
                continue
            if isinstance(value, dict):
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu":
                    com_ipdu_dict[ComConfigKey] = value
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx":
                    ComMainFunction_tx[ComConfigKey] = value
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx":
                    ComMainFunction_rx[ComConfigKey] = value
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionGw":
                    ComMainFunction_gw[ComConfigKey] = value
        
        ComConfig["ComIPdus"] = com_ipdu_dict
        base_path = "/ActiveEcuC/Com/ComConfig"

        check_result = True
        # 2）每个MainFunctionTx进行校验
        for ComMainFunction_tx in list(ComMainFunction_tx.keys()):
            validation_data = {}
            validation_data["mainfunction_name"] = ComMainFunction_tx
            com_mainfunction_ref_path = base_path + "/" + ComMainFunction_tx
            check_result = check_prefix(ComMainFunction_tx)

            if  check_result == False:
                self.vlog(self.validation_code, self.validation_summary,
                            f"ComMainFunction  {validation_data['mainfunction_name']} container name prefix must be Com_MainFunction",
                            com_mainfunction_ref_path, "fail")
                valid = False

        # 3）每个MainFunctionRx进行校验
        for ComMainFunction_rx in list(ComMainFunction_rx.keys()):
            validation_data = {}
            validation_data["mainfunction_name"] = ComMainFunction_rx
            com_mainfunction_ref_path = base_path + "/" + ComMainFunction_rx
            check_result = check_prefix(ComMainFunction_rx)

            if  check_result == False:
                self.vlog(self.validation_code, self.validation_summary,
                            f"ComMainFunction  {validation_data['mainfunction_name']} container name prefix must be Com_MainFunction", 
                            com_mainfunction_ref_path, "fail")
                valid = False

        # 3）每个MainFunctionGw进行校验
        for ComMainFunction_gw in list(ComMainFunction_gw.keys()):
            validation_data = {}
            validation_data["mainfunction_name"] = ComMainFunction_gw
            com_mainfunction_ref_path = base_path + "/" + ComMainFunction_gw
            check_result = check_prefix(ComMainFunction_gw)

            if  check_result == False:
                self.vlog(self.validation_code, self.validation_summary,
                            f"ComMainFunction  {validation_data['mainfunction_name']} container name prefix must be Com_MainFunction", 
                            com_mainfunction_ref_path, "fail")
                valid = False

        return valid
