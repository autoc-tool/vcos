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


# DcmDspSecurityAttemptCounterEnabled 与 DcmDspSecurityNumAttDelay 互斥校验
class Dcm013001Rule(BasicRule):
    ruleset_name = 'Dcm013001'
    validation_result = True

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DspSecurityAttemptCounterEnabled & SecurityNumAttDelay Mutually Exclusive Check."

        self.durable_init()

    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(m.att_enable == False)
                def auto_gen(c):
                    self.__class__.validation_result = True

                @when_all((m.att_delay != None) & (m.att_delay != 0))
                def auto_gen1(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self, ecuc_dcm_dict):
        valid = True
        # 1）准备校验数据
        base_path, dsp_security_row_list = DcmRuleUtils().get_dcm_dsp_security_row_list(ecuc_dcm_dict)

        for security_row_key, security_row_value in dsp_security_row_list.items():
            security_row_path = base_path + '/' + security_row_key
            security_att_enable_path = security_row_path + '/' + 'DcmDspSecurityAttemptCounterEnabled'
            validation_data = {}
            validation_data['att_enable'] = security_row_value['DcmDspSecurityAttemptCounterEnabled']['Value']
            validation_data['att_delay'] = security_row_value['DcmDspSecurityNumAttDelay']['Value']

            # 2） 开始校验
            try:
                self.__class__.validation_result = False
                post(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False

            # 3） 打印日志
            if self.__class__.validation_result:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DspSecurity {security_row_key} AttemptCounterEnabled & SecurityNumAttDelay Mutually Exclusive Check Success.",
                          security_att_enable_path, "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DspSecurity {security_row_key} AttemptCounterEnabled & SecurityNumAttDelay Mutually Exclusive Check Fail.",
                          security_att_enable_path, "fail")
                valid = False

            retract_facts(self.__class__.ruleset_name, validation_data)

        return valid

    def add_rule_time(self):
        pass
