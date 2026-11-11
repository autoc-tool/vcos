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

# ComGwMappings容器下重复ComGwMappings测试(反向测试)
class Com011009Rule(BasicRule):
    ruleset_name = "Com011009"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_com_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Validation Of ComSignalGwRef In ComGwMapping !"

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

                @when_all(+m.comsignalgw_ref2)
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
        ecuc_com_dict = self.ecuc_com_dict
        if "Com" in ecuc_com_dict.keys():
            com_dict = ecuc_com_dict['Com']['ArPackages']['ActiveEcuC']['Com']
            com_config_dict = com_tools_get_dict_by_definition_ref(com_dict,"ComConfig")
        else:
            com_config_dict = com_tools_get_dict_by_definition_ref(ecuc_com_dict,"ComConfig") 
        com_gwmapping_dict = {}
        for ComConfigKey, value in com_config_dict.items():
            if ComConfigKey == "DefinitionRef" or ComConfigKey == "Tag":
                continue
            if isinstance(value, dict):
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComGwMapping":
                    com_gwmapping_dict[ComConfigKey] = value

        base_path = "/ActiveEcuC/Com/ComConfig"

        for gw_key, gw_value in com_gwmapping_dict.items():
            Templist = []
            com_gwsignal_path = base_path + "/" + gw_key
            if not isinstance(gw_value, dict):
                continue
            if "ComGwSource" in gw_value.keys():
                if "ComGwSignalRef" in gw_value["ComGwSource"]['ComGwSignal']:
                    Templist.append(gw_value['ComGwSource']['ComGwSignal']['ComGwSignalRef']['ValueRef'])
                else:
                    continue
            else:
                continue
            com_signal_gw_ref_path = com_gwsignal_path + "/" + "ComGwSignalRef"
            validataion_data_path_list.append(com_signal_gw_ref_path)

            for gwsignal in list(Templist):
                validation_data = {}
                validation_data["comsignalgw_ref2"] = gwsignal[gwsignal.rfind("/") + 1:]
                validation_data_list.append(validation_data)

        # 2） 开始校验
        for index, validation_data in enumerate(validation_data_list):
            try:
                self.__class__.validation_result = False
                assert_fact(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False
                print('assert_fact() exception message {0}'.format(e.args))

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if self.__class__.validation_result:
                self.vlog(self.validation_code, self.validation_summary,
                          f"ComGwMapping  {validation_data['comsignalgw_ref2']} is not duplicated",
                          validataion_data_path_list[index], "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"ComGwMapping  {validation_data['comsignalgw_ref2']} is duplicated",
                          validataion_data_path_list[index], "fail")
                valid = False

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        return valid

    def add_rule_time(self):
        pass
