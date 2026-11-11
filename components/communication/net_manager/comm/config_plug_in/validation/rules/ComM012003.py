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


# ComMNmVariant 动态配置项缺失
class ComM012003Rule(BasicRule):
    ruleset_name = "ComM012003"
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

                @when_all(m.PncNmRequest == False)
                def auto_gen(c):
                    self.__class__.validation_result = True

                @when_all((m.NmVariant == 'FULL') | (m.NmVariant == 'PASSIVE'))
                def dynamic_lost(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_comm_dict = self.comm_dict

        comm_dict = ecuc_comm_dict['ComMConfigSet']
        base_path = '/ActiveEcuC/ComM/ComMConfigSet'

        for channel_key, channel_value in comm_dict.items():
            validation_data = {}
            validation_data['PncNmRequest'] = False
            validation_data['NmVariant'] = "FULL"
            if not isinstance(channel_value, dict):
                continue
            if 'ComMChannel' == channel_value['DefinitionRef'].split('/')[-1]:
                channel_path = base_path + '/' + channel_key
                validataion_data_path = channel_path

                validation_data['channel'] = channel_key
                if 'ComMNetworkManagement' in channel_value:
                    validataion_data_path = channel_path + '/ComMNetworkManagement/ComMNmVariant'
                    validation_data['PncNmRequest'] = channel_value['ComMNetworkManagement']['ComMPncNmRequest']['Value']
                    validation_data['NmVariant'] = channel_value['ComMNetworkManagement']['ComMNmVariant']['Value']

                # 2） 开始校验
                try:
                    self.__class__.validation_result = False
                    post(self.__class__.ruleset_name, validation_data)
                except Exception as e:
                    self.__class__.validation_result = False

                # 3） 打印日志
                if self.__class__.validation_result:
                    self.vlog("COMM012003", "ComMPncNmRequest is true, ComMNmVariant should be FULL/PASSIVE.",
                              f"{channel_key} ComMPncNmRequest & ComMNmVariant Missing Check Success.",
                              validataion_data_path, "pass")
                else:
                    self.vlog("COMM012003", "ComMPncNmRequest is true, ComMNmVariant should be FULL/PASSIVE.",
                              f"{channel_key} ComMPncNmRequest is true, ComMNmVariant should be FULL/PASSIVE.",
                              validataion_data_path, "fail")
                    valid = False

                retract_facts(self.__class__.ruleset_name, validation_data)
        return valid

    def add_rule_time(self):
        pass
