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


# ComMPncComSignalChannelRef 动态配置项缺失
class ComM012005Rule(BasicRule):
    ruleset_name = "ComM012005"
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

                @when_all((m.direction == 'RX') & (m.kind == 'EIRA'))
                def auto_gen(c):
                    self.__class__.validation_result = True

                @when_all((m.channel != '') & (m.channel != None))
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

        for pnc_key, pnc_value in comm_dict.items():
            validation_data = {}
            validation_data['direction'] = 'RX'
            validation_data['kind'] = 'EIRA'
            validation_data['channel'] = None
            if not isinstance(pnc_value, dict):
                continue
            if 'ComMPnc' == pnc_value['DefinitionRef'].split('/')[-1]:
                pnc_path = base_path + '/' + pnc_key
                for signal_name, signal_value in pnc_value.items():
                    if not isinstance(signal_value, dict):
                        continue
                    if 'ComMPncComSignal' == signal_value['DefinitionRef'].split('/')[-1]:
                        validataion_data_path = pnc_path + '/' + signal_name + "/ComMPncComSignalChannelRef"
                        validation_data['direction'] = signal_value['ComMPncComSignalDirection']['Value']
                        validation_data['kind'] = signal_value['ComMPncComSignalKind']['Value']
                        validation_data['channel'] = signal_value['ComMPncComSignalChannelRef']['ValueRef']

                        # 2） 开始校验
                        try:
                            self.__class__.validation_result = False
                            post(self.__class__.ruleset_name, validation_data)
                        except Exception as e:
                            self.__class__.validation_result = False

                        # 3） 打印日志
                        if self.__class__.validation_result:
                            self.vlog("COMM012005", "Signal channel should be defined.",
                                    f"{pnc_key} {signal_name}Signal channel Missing Check Success.",
                                    validataion_data_path, "pass")
                        else:
                            self.vlog("COMM012005", "Signal channel should be defined.",
                                    f"{pnc_key} {signal_name}Signal channel should be defined.",
                                    validataion_data_path, "fail")
                            valid = False

                        retract_facts(self.__class__.ruleset_name, validation_data)
        return valid

    def add_rule_time(self):
        pass
