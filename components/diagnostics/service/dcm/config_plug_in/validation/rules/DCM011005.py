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


# DcmDspSecurityLevel 重复校验
class Dcm011005Rule(BasicRule):
    ruleset_name = 'Dcm011005'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DcmDspSecurityLevel should not be same!"

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

                @when_all(+m.security_level)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self, ecuc_dcm_dict):
        valid = True
        # 1）准备校验数据
        base_path, dsp_security_row_list = DcmRuleUtils().get_dcm_dsp_security_row_list(ecuc_dcm_dict)

        validation_data_list = []
        validataion_data_path_list = []
        key_name_list = []
        for security_row_key, security_row_value in dsp_security_row_list.items():
            security_row_path = base_path + '/' + security_row_key
            security_level_path = security_row_path + '/' + 'DcmDspSecurityLevel'
            validation_data = {}
            validation_data['security_level'] = security_row_value['DcmDspSecurityLevel']['Value']
            validation_data_list.append(validation_data)
            validataion_data_path_list.append(security_level_path)
            key_name_list.append(security_row_key)

        # 2） 开始校验
        for index, validation_data in enumerate(validation_data_list):
            try:
                self.__class__.validation_result = False
                assert_fact(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False

            # 格式：'错误码', '摘要', '错误信息', '错误索引路径' '结果'
            if self.__class__.validation_result:
                self.vlog(self.validation_code, self.validation_summary,
                          f"Security Level {key_name_list[index]} {validation_data['security_level']} is not duplicated.",
                          validataion_data_path_list[index], 'pass')
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"Security Level {key_name_list[index]} {validation_data['security_level']} is duplicated.",
                          validataion_data_path_list[index], 'fail')
                valid = False

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        return valid

    def add_rule_time(self):
        pass
