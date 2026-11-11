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
from basic_func_module.autosar_utils.autosar_util import AutosarUtil

class PduR016009Rule(BasicRule):
    ruleset_name = "PduR016009"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "ShortName of PduRMainFunction Container should start with PduR_MainFunction!"
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict

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

                @when_all(+m.frame_id)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    def check_validation_run(self):
        if not self.modules_are_exist(["PduR"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        valid = True
        ecuc_PduR_dict = self.bsw_ecuc_cfg_class_dict["PduR"]["ArPackages"]["ActiveEcuC"]["PduR"]
        for itemName, item in ecuc_PduR_dict.items():
            if "DefinitionRef" not in item or item["DefinitionRef"].split('/')[-1] != "PduRMainFunction":
                continue
            if 'PduR_MainFunction' not in itemName:
                valid = False
            if itemName[0:17] != 'PduR_MainFunction': # 判断是不是以PduR_MainFunction开头
                valid = False
            if valid == False:
                service_id_path = "/ActiveEcuC/PduR/" + itemName
                self.vlog(
                    self.validation_code, self.validation_summary,
                    f"Name of PduRMainFunctionContainer [{itemName}] incorrect!",
                    service_id_path, "fail")
        return valid

    def add_rule_time(self):
        pass