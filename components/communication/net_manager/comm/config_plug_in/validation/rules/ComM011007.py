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

def contains_user_id_zero(dict_list):
    for d in dict_list:
        if d.get('user_id') == 0:
            return True
    return False

# ComMUserIdentifier 连续性校验
class ComM011007Rule(BasicRule):
    ruleset_name = 'ComM011007'
    validation_result = False

    def __init__(self, logs, bsw_dict, comm_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_dict = bsw_dict
        self.comm_dict = comm_dict

    @timing_decorator
    def validation_run(self):
        valid = True
        # 1）准备校验数据
        ecuc_comm_dict = self.comm_dict

        comm_dict = ecuc_comm_dict['ComMConfigSet']
        base_path = '/ActiveEcuC/ComM/ComMConfigSet'
        log_path = '/ActiveEcuC/ComM/ComMConfigSet/ComMUser'
        validation_data_list = []
        for user_key, user_value in comm_dict.items():
            if not isinstance(user_value, dict):
                continue
            if 'ComMUser' == user_value['DefinitionRef'].split('/')[-1]:
                service_table_path = base_path + '/' + user_key
                service_path = service_table_path + '/' + 'ComMUserIdentifier'

                validation_data = {}
                validation_data['user_id'] = user_value['ComMUserIdentifier']['Value']
                validation_data_list.append(validation_data)

        # 2） 开始校验
        try:
            res = contains_user_id_zero(validation_data_list)
        except Exception as e:
            print(f"{type(e)} {format(e)} dose not support config validation")
            pass

        # 格式：'错误码', '摘要', '错误信息', '错误索引路径' '结果'
        if (res == True):
            self.vlog('ComM011007', 'ComMUserIdentifier should start from 0.',
                        f"ComMUser(s) ComMUserIdentifier should start from 0.", log_path,
                        'pass')
        else:
            self.vlog('ComM011007', 'ComMUserIdentifier should start from 0.',
                        f"ComMUser(s) ComMUserIdentifier should start from 0.", log_path,
                        'fail')
            valid = False

        return valid
