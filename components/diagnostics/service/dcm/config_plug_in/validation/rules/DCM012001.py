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
from .Dcm_utils import DcmRuleUtils


# DcmDspSecurityUsePort 动态配置项缺失
class Dcm012001Rule(BasicRule):
    ruleset_name = "Dcm012001"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "SecurityUsePort & CompareKeyFnc Missing Check."

        self.durable_init()

    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(m.use_port != 'USE_ASYNCH_FNC')
                def auto_gen(c):
                    self.__class__.validation_result = True

                @when_all((m.compare_key != '') & (m.compare_key != None))
                def dynamic_lost(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self, ecuc_dcm_dict):
        valid = True
        # 1）准备校验数据
        base_path, dsp_security_row_list = DcmRuleUtils().get_dcm_dsp_security_row_list(ecuc_dcm_dict)

        for security_row_key, security_row_value in dsp_security_row_list.items():
            if security_row_key == 'Locked':
                continue
            security_row_path = base_path + '/' + security_row_key
            validation_data = {}
            validation_data['use_port'] = security_row_value['DcmDspSecurityUsePort']['Value']
            validation_data['compare_key'] = security_row_value['DcmDspSecurityCompareKeyFnc']['Value']
            validataion_data_path = security_row_path + '/' + 'DcmDspSecurityCompareKeyFnc'

            # 2） 开始校验
            try:
                self.__class__.validation_result = False
                post(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False

            # 3） 打印日志
            if self.__class__.validation_result:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DspSecurity {security_row_key} UsePort & CompareKeyFnc Missing Check Success.",
                          validataion_data_path, "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DspSecurity {security_row_key} UsePort is USE_ASYNCH_FNC, DcmDspSecurityCompareKeyFnc is missing.",
                          validataion_data_path, "fail")
                valid = False

            retract_facts(self.__class__.ruleset_name, validation_data)
        return valid

    def add_rule_time(self):
        pass
