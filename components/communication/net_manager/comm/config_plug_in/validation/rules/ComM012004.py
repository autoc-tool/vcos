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


# ComMPncSupport 动态配置项缺失
class ComM012004Rule(BasicRule):
    ruleset_name = "ComM012004"
    validation_result = False

    def __init__(self, logs, bsw_dict, comm_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_dict = bsw_dict
        self.comm_dict = comm_dict

        self.durable_init()

    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(m.PncEnabled == False)
                def auto_gen(c):
                    self.__class__.validation_result = True

                @when_all(m.PncSupport == True)
                def dynamic_lost(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_comm_dict = self.comm_dict

        validation_data = {}
        validation_data['PncEnabled'] = ecuc_comm_dict['ComMConfigSet']['ComMPncEnabled']['Value']
        validation_data['PncSupport'] = ecuc_comm_dict['ComMGeneral']['ComMPncSupport']['Value']
        validataion_data_path = '/ActiveEcuC/ComM/ComMGeneral/ComMPncSupport'

        # 2） 开始校验
        try:
            self.__class__.validation_result = False
            post(self.__class__.ruleset_name, validation_data)
        except Exception as e:
            self.__class__.validation_result = False

        # 3） 打印日志
        if self.__class__.validation_result:
            self.vlog("COMM012004", "ComMPncEnabled is true, ComMPncSupport should be true.",
                        f"ComMPncEnabled & ComMPncSupport Missing Check Success.",
                        validataion_data_path, "pass")
        else:
            self.vlog("COMM012004", "ComMPncEnabled is true, ComMPncSupport should be true.",
                        f"ComMPncEnabled is true, ComMPncSupport should be true.",
                        validataion_data_path, "fail")
            valid = False

        retract_facts(self.__class__.ruleset_name, validation_data)
        return valid

    def add_rule_time(self):
        pass
