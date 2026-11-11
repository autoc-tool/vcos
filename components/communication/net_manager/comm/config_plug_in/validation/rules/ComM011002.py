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


# ComMUserIdentifier 重复校验
class ComM011002Rule(BasicRule):
    ruleset_name = 'ComM011002'
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

                @when_all(+m.status)
                def event(c):
                    pass

                @when_all(+m.user_id)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_comm_dict = self.comm_dict

        comm_dict = ecuc_comm_dict['ComMConfigSet']
        base_path = '/ActiveEcuC/ComM/ComMConfigSet'
        validation_data_list = []
        validataion_data_path_list = []
        for user_key, user_value in comm_dict.items():
            if not isinstance(user_value, dict):
                continue
            if 'ComMUser' == user_value['DefinitionRef'].split('/')[-1]:
                service_table_path = base_path + '/' + user_key
                service_path = service_table_path + '/' + 'ComMUserIdentifier'

                validation_data = {}
                validation_data['user_id'] = user_value['ComMUserIdentifier']['Value']
                validation_data_list.append(validation_data)
                validataion_data_path_list.append(service_path)

        # 2） 开始校验
        for index, validation_data in enumerate(validation_data_list):
            try:
                self.__class__.validation_result = False
                assert_fact(self.__class__.ruleset_name, validation_data)
            except Exception as e:
                self.__class__.validation_result = False
                pass

            # 格式：'错误码', '摘要', '错误信息', '错误索引路径' '结果'
            if self.__class__.validation_result:
                self.vlog('ComM011002', 'User Id should be unique.',
                          f"User Id {validation_data['user_id']} should be unique.", validataion_data_path_list[index],
                          'pass')
            else:
                self.vlog('ComM011002', 'User Id should be unique.',
                          f"User Id {validation_data['user_id']} should be unique.", validataion_data_path_list[index],
                          'fail')
                valid = False

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        pass

        return valid

    def add_rule_time(self):
        pass
