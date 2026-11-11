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


# DcmDspSessionLevel 重复校验
class Dcm011009Rule(BasicRule):
    ruleset_name = 'Dcm011009'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DcmDspSessionResponseMultiplePendingFnc should be none when Default and Extended sessions!"

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

                @when_all(+m.session_level)
                def fact(c):
                    self.__class__.validation_result = True

    @timing_decorator
    def validation_run(self, ecuc_dcm_dict):
        valid = True
        # 1）准备校验数据
        base_path, dsp_session_row_list = DcmRuleUtils().get_dcm_dsp_session_row_list(ecuc_dcm_dict)
        validation_data_list = []
        for session_row_key, session_row_value in dsp_session_row_list.items():
            if session_row_key == 'Default' or session_row_key == 'Extended':
                if session_row_value['DcmDspSessionResponseMultiplePendingFnc']['Value'] != '' and session_row_value['DcmDspSessionResponseMultiplePendingFnc']['Value'] != None:
                    valid = False
            session_row_path = base_path + '/' + session_row_key
            session_level_path = session_row_path + '/' + 'DcmDspSessionResponseMultiplePendingFnc'

        # 2） 开始校验

            # 格式：'错误码', '摘要', '错误信息', '错误索引路径' '结果'
            if valid:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DcmDspSessionResponseMultiplePendingFnc should be None when { session_row_key } sessions!",
                          session_level_path, 'pass')
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"DcmDspSessionResponseMultiplePendingFnc should be None when { session_row_key } sessions!",
                          session_level_path, 'fail')
            valid = True
        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        return valid

    def add_rule_time(self):
        pass
