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


class CanIf013012Rule(BasicRule):
    ruleset_name = "CanIf013012"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Check if canifhrh and canhrh match."
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
        canifhrhcfgs = CanIfRuleUtils().validate_utils_get_canif_hrh_cfgs(ecuc_canif_dict)
        canhrhcfgs = CanIfRuleUtils().validate_utils_get_can_hrh_cfgs(ecuc_mcal_can_dict)

        canhrhcfg_name_list = []
        for canhrhcfg_name, _ in canhrhcfgs.items():
            canhrhcfg_name_list.append(canhrhcfg_name)

        canifhrhcfg_name_list = []
        for canifhrhcfg_name, _ in canifhrhcfgs.items():
            canifhrhcfg_name_list.append(canifhrhcfg_name)

        # 以canhrhcfgs为基准，canifhrhcfgs缺失的container
        lost_list = []
        for name in canhrhcfg_name_list:
            if name not in canifhrhcfg_name_list:
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
                self.vlog(self.validation_code, self.validation_summary, "All CanIfHrhCfg is same as Rx CanHardwareObject.",    \
                        f"/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/{test_dict['name_cur']}", "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,    \
                        f"The mapping of CanHardwareObject(Rx) {test_dict['name_cur']} in the CanIf layer is missing. The configuration of the CanIfInitHrhCfg(s) should be checked.",
                        "/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/" + f"{test_dict['name_cur']}",
                        "fail")

        # 以canhrhcfgs为基准，canifhrhcfgs多余的container
        more_list = []
        for name in canifhrhcfg_name_list:
            if name not in canhrhcfg_name_list:
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
                self.vlog(self.validation_code, self.validation_summary, "All CanIfHrhCfg is same as Rx CanHardwareObject.",
                        f"/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/" + f"{test_dict['name_cur']}", "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,    \
                        f"There are more CanIfInitHrhCfg(s) mappings in the CanIf layer than there are CanHardwareObject(s) for the MCAL CAN Driver.", \
                        "/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/" + f"{test_dict['name_cur']}", "fail")



    def add_rule_time(self):
        pass
