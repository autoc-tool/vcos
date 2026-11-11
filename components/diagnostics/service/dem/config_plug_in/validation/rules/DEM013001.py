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
from .Dem_utils import DemRuleUtils

class DEM013001Rule(BasicRule):
    ruleset_name = "DEM013001"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_dict: dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DemDataElementReadFnc should be valid!"
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_dict

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

                @when_all((m.portvalue == True) & (m.fncvalue == ''))  # 待修改
                def fact(c):
                    self.__class__.validation_result = True

            pass

    def check_validation_run(self):
        if not self.modules_are_exist(["Dem"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary,
                    "Data required for validation is missing",
                    self.validation_summary,
                    "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        dem_general_name, dem_general = DemRuleUtils.dem_get_dict_by_definition_ref(self.bsw_ecuc_cfg_class_dict["Dem"]["ArPackages"]["ActiveEcuC"]["Dem"], "DemGeneral")
        base_path = "/ActiveEcuC/Dem/" + dem_general_name
        valid = True
        for dem_data_key, dem_data_value in dem_general.items():
            if not isinstance(dem_data_value, dict):
                continue
            if "DemDataElementClass" == dem_data_value["DefinitionRef"].split(
              "/")[-1]:  # 找到 DemDataElementClass  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
                for element_class_key, element_class_value in dem_data_value.items():
                    if not isinstance(element_class_value, dict):
                        continue
                    if "DemExternalCSDataElementClass" == element_class_value["DefinitionRef"].split(
                      "/")[-1]:  # 第一次调试   问题
                        readfunc_path = base_path + "/" + element_class_key

                        for cs_data_class_key, cs_data_class_value in element_class_value.items():
                            if not isinstance(cs_data_class_value, dict):
                                continue

                            if "DemDataElementUsePort" == cs_data_class_value["DefinitionRef"].split(
                              "/")[-1]:  # DemDataElementUsePort  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
                                dem_data_elem_use_support_value = cs_data_class_value["Value"]

                            if "DemDataElementReadFnc" == cs_data_class_value["DefinitionRef"].split("/")[-1]:
                                if cs_data_class_value["Value"] is not None:
                                    dem_data_elem_read_fnc_value = cs_data_class_value["Value"]
                                else:
                                    dem_data_elem_read_fnc_value = ''

                            service_path = readfunc_path + "/" + cs_data_class_key

                            # 2） 开始校验
                        # vaild = True
                        if dem_data_elem_use_support_value == True:
                            test_dict = {}
                            test_dict["portvalue"] = dem_data_elem_use_support_value
                            test_dict["fncvalue"] = dem_data_elem_read_fnc_value
                            try:
                                self.__class__.validation_result = True
                                assert_fact(self.__class__.ruleset_name, test_dict)
                            except Exception as e:
                                self.__class__.validation_result = False
                                pass

                            if self.__class__.validation_result == False:
                                valid = False
                                # break
                            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
                            if valid == True:
                                self.vlog(self.validation_code, self.validation_summary,
                                          "DemDataElementReadFnc is valid.", f"{service_path}/DemDataElementReadFnc",
                                          "pass")
                            else:
                                self.vlog(self.validation_code, self.validation_summary,
                                          "DemDataElementReadFnc is not valid.",
                                          f"{service_path}/DemDataElementReadFnc", "fail")
                            # 3）清空ruleset中的所有fact，为下一次校验做准备
                            retract_facts(self.__class__.ruleset_name, test_dict)
                            pass

        return valid

    def add_rule_time(self):
        pass
