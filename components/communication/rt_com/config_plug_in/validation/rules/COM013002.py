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

# 互斥校验：IPDU类型绑定MainFunction要匹配
class Com013002Rule(BasicRule):
    ruleset_name = "Com013002"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_com_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Validation Of ComIPduType and ComMainFunction Type!"

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
        # ComConfig["ComMainFunction"] = ComMainFunction_tx + ComMainFunction_rx + ComMainFunction_gw
        base_path = "/ActiveEcuC/Com/ComConfig/"

        # 2）每个IPDU进行校验
        for ipdu in list(ComConfig["ComIPdus"].keys()):
            validation_data = {}
            validation_data["comipdu_name"] = ipdu
            com_ipdu_ref_path = base_path + "/" + ipdu

            check_result = 0
            if "ComIPduDirection" in ComConfig["ComIPdus"][ipdu]:
                if "IPduMainFunctionRef" in ComConfig["ComIPdus"][ipdu]:
                    ipdu_direct = ComConfig["ComIPdus"][ipdu]["ComIPduDirection"]["Value"]
                    
                    # IPduMainFunctionRef
                    IPduMainFunctionRef = ComConfig["ComIPdus"][ipdu]['IPduMainFunctionRef']['ValueRef']
                    IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]

                    # check
                    if ipdu_direct == "SEND":
                        if IPduMainFunctionRef not in ComMainFunction_tx:
                            check_result = -1
                    
                    if ipdu_direct == "RECEIVE":
                        if IPduMainFunctionRef not in ComMainFunction_rx:
                            check_result = -1

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if check_result == 0:
                self.vlog(self.validation_code, self.validation_summary,
                          f"ComIPdu  {validation_data['comipdu_name']} IPDU Direct and MainFunction matching ",
                          com_ipdu_ref_path, "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"ComIPdu  {validation_data['comipdu_name']} IPDU Direct and MainFunction is not match",
                          com_ipdu_ref_path, "fail")
                valid = False
        return valid
