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
from .CanIf_utils import CanIfRuleUtils, checkIsNotNone


class CanIf013014Rule(BasicRule):
    ruleset_name = "CanIf013014"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Check if canifctrl and canctrl match."
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

                @when_all(m.name_cur == m.name_aim)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    def check_validation_run(self):
        if not self.modules_are_exist(["CanIf"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary,
                    "Data required for validation is missing",
                    self.validation_summary,
                    "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self, ecuc_canif_dict, ecuc_mcal_can_dict):
        # 1）准备校验数据
        canctrlcfgs = CanIfRuleUtils().validate_utils_get_CanController_list(ecuc_mcal_can_dict)
        canifctrlcfgs = CanIfRuleUtils().validate_utils_get_CanIfCtrlCfg_list(ecuc_canif_dict)

        canctrlcfg_name_list = []
        for canctrlcfg_name, _ in canctrlcfgs.items():
            canctrlcfg_name_list.append(canctrlcfg_name)

        canifctrlcfg_name_list = []
        for canifctrlcfg_name, _ in canifctrlcfgs.items():
            canifctrlcfg_name_list.append(canifctrlcfg_name)

        # 以canctrlcfgs为基准，canifctrlcfgs缺失的container
        lost_list = []
        for name in canctrlcfg_name_list:
            if name not in canifctrlcfg_name_list:
                lost_list.append(name)

        for lst_name in lost_list:
            valid = True
            # 2） 开始校验
            test_dict = {}
            test_dict["name_cur"] = lst_name
            test_dict["name_aim"] = ""

            try:
                self.__class__.validation_result = False
                assert_fact(self.__class__.ruleset_name, test_dict)
            except Exception as e:
                self.__class__.validation_result = False
                pass

            if self.__class__.validation_result == False:
                valid = False

            # 3）清空ruleset中的所有fact，为下一次校验做准备
            retract_facts(self.__class__.ruleset_name, test_dict)
            pass

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if valid == True:
                self.vlog(self.validation_code, self.validation_summary, "All canifctrlCfg is valid.",    \
                        f"/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/{test_dict['name_cur']}", "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,    \
                        f"The mapping of CanController {test_dict['name_cur']} in the CanIf layer is missing. The configuration of the CanIfCtrlCfg(s) should be checked.",
                        "/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/" + f"{test_dict['name_cur']}",
                        "fail")

        # 以canctrlcfgs为基准，canifctrlcfgs多余的container
        more_list = []
        for name in canifctrlcfg_name_list:
            if name not in canctrlcfg_name_list:
                more_list.append(name)

        for more_name in more_list:
            valid = True
            # 2） 开始校验
            test_dict = {}
            test_dict["name_cur"] = more_name
            test_dict["name_aim"] = ""

            try:
                self.__class__.validation_result = False
                assert_fact(self.__class__.ruleset_name, test_dict)
            except Exception as e:
                self.__class__.validation_result = False
                pass

            if self.__class__.validation_result == False:
                valid = False

            # 3）清空ruleset中的所有fact，为下一次校验做准备
            retract_facts(self.__class__.ruleset_name, test_dict)
            pass

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if valid == True:
                self.vlog(self.validation_code, self.validation_summary, "All canifctrlCfg is valid.",
                        f"/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/" + f"{test_dict['name_cur']}", "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,    \
                        f"There are more CanIfCtrlCfg(s) mappings in the CanIf layer than there are CanController(s) for the MCAL CAN Driver.", \
                        "/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/" + f"{test_dict['name_cur']}", "fail")



    def add_rule_time(self):
        pass
