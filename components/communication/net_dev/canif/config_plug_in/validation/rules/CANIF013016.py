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


class CanIf013016Rule(BasicRule):
    ruleset_name = "CanIf013016"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Check if canif hth and canif txbuffer match."
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
        caniftxbuffercfgs = CanIfRuleUtils().validate_utils_get_txbuffer_cfgs(ecuc_canif_dict)
        canifhthcfgs = CanIfRuleUtils().validate_utils_get_canif_hth_cfgs(ecuc_canif_dict)

        caniftxbuffercfg_name_list = []
        for caniftxbuffercfg_name, _ in caniftxbuffercfgs.items():
            caniftxbuffercfg_name_list.append(caniftxbuffercfg_name)

        canifhthcfg_name_list = []
        for canifhthcfg_name, _ in canifhthcfgs.items():
            canifhthcfg_name_list.append(canifhthcfg_name)

        # 以canifhthcfgs为基准，caniftxbuffercfgs缺失的container
        lost_list = []
        for name in canifhthcfg_name_list:
            if name not in caniftxbuffercfg_name_list:
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
                self.vlog(self.validation_code, self.validation_summary, "All CanIfHthCfg is same as CanIfTxBufferCfg.",    \
                        f"/ActiveEcuC/CanIf/CanIfInitCfg/{test_dict['name_cur']}", "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,    \
                        f"The CanIfBufferCfg mapping for CanIfInitHthCfg is missing. The configuration of the CanIfBufferCfg(s) should be checked.",
                        "/ActiveEcuC/CanIf/CanIfInitCfg/" + f"{test_dict['name_cur']}",
                        "fail")

        # 以canhthcfgs为基准，canifhthcfgs多余的container
        more_list = []
        for name in caniftxbuffercfg_name_list:
            if name not in canifhthcfg_name_list:
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
                self.vlog(self.validation_code, self.validation_summary, "All CanIfHthCfg is same as CanIfTxBufferCfg.",
                        f"/ActiveEcuC/CanIf/CanIfInitCfg/" + f"{test_dict['name_cur']}", "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,    \
                        f"There are more CanIfBufferCfg(s) mappings than there are CanIfInitHthCfg(s).", \
                        "/ActiveEcuC/CanIf/CanIfInitCfg/" + f"{test_dict['name_cur']}", "fail")



    def add_rule_time(self):
        pass
