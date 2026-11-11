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

class CanSM016001Rule(BasicRule):
    ruleset_name = "CanSM016001"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "ShortName of CanSMMainFunction Container should start with CanSM_MainFunction!"
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
        if not self.modules_are_exist(["CanSM"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        Totalvalid = True
        ecuc_CanSM_dict = self.bsw_ecuc_cfg_class_dict["CanSM"]["ArPackages"]["ActiveEcuC"]["CanSM"]
        for itemName, item in ecuc_CanSM_dict.items():
            if "DefinitionRef" not in item or item["DefinitionRef"].split('/')[-1] != "CanSMConfiguration":
                continue
            for itemName2, item2 in item.items():
                if "DefinitionRef" not in item2 or item2["DefinitionRef"].split('/')[-1] != "CanSMMainFunction":
                    continue
                if itemName2[0:18] != 'CanSM_MainFunction': # 判断是不是以CanSM_MainFunction开头
                    Totalvalid = False
                    service_id_path = "/ActiveEcuC/CanSM/CanSMConfiguration/" + itemName2
                    self.vlog(
                        self.validation_code, self.validation_summary,
                        f"Name of CanSMMainFunctionContainer [{itemName2}] incorrect!",
                        service_id_path, "fail")
        return Totalvalid

    def add_rule_time(self):
        pass
