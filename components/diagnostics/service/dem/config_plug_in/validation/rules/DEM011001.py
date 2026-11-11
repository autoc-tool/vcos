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

class DEM011001Rule(BasicRule):
    ruleset_name = "DEM011001"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_dict: dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DemDtcValue should not be duplicated!"
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

                @when_all(+m.service_id)
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
        dem_config_set_name, dem_config_set = DemRuleUtils.dem_get_dict_by_definition_ref(self.bsw_ecuc_cfg_class_dict["Dem"]["ArPackages"]["ActiveEcuC"]["Dem"], "DemConfigSet")
        base_path = "/ActiveEcuC/Dem/" + dem_config_set_name
        validation_data_list = []
        validataion_data_path_list = []

        for demdtc_key, demdtc_value in dem_config_set.items():
            if not isinstance(demdtc_value, dict):
                continue
            if "DemDTC" == demdtc_value["DefinitionRef"].split("/")[-1]:
                demdtc_path = base_path + "/" + demdtc_key
                for dtcvalue_key, dtcvalue_value in demdtc_value.items():
                    if not isinstance(dtcvalue_value, dict):
                        continue
                    if "DemDtcValue" == dtcvalue_value["DefinitionRef"].split("/")[-1]:
                        service_path = demdtc_path + "/" + dtcvalue_key
                        validation_data = {}
                        validation_data["service_id"] = dtcvalue_value["Value"]
                        validation_data_list.append(validation_data)
                        validataion_data_path_list.append(service_path)
        # 2） 开始校验
        vaild = True
        for index, validation_data in enumerate(validation_data_list):
            try:
                self.__class__.validation_result = False
                assert_fact(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False
                pass
            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if self.__class__.validation_result == False:
                vaild = False

            if self.__class__.validation_result:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DemDtcValue {validation_data['service_id']} is not duplicated.",
                          validataion_data_path_list[index], "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DemDtcValue {validation_data['service_id']} should not be duplicated.",
                          validataion_data_path_list[index], "fail")
        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        pass

        return vaild

    def add_rule_time(self):
        pass
