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


class NvM011002Rule(BasicRule):
    ruleset_name = "NvM011002Rule"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict, nvm_dict) -> None:
        super().__init__()
        self.logs = logs
        self.nvm_dict = nvm_dict
        self.durable_init()

    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            # print('get_ruleset() exception message: {0}'.format(e.args))
            with ruleset(self.__class__.ruleset_name):

                @when_all(+m.status)
                def event(c):
                    pass

                @when_all(+m.BlockIdentifier)
                def fact(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self):
        valid = True
        BlockIdentifier = 0
        validation_data_list = []
        validataion_data_path_list = []

        # 1）准备校验数据
        for name, blockdescriptor in self.nvm_dict.items():
            if "DefinitionRef" not in blockdescriptor or blockdescriptor[
              "DefinitionRef"] != "/AUTOSAR/EcucDefs/NvM/NvMBlockDescriptor":
                continue
            path = '/ActiveEcuC/NvM'
            path = path + '/' + name
            BlockIdentifier = blockdescriptor["NvMNvramBlockIdentifier"]["Value"]
            validation_data = {}
            if blockdescriptor["NvMNvramDeviceId"]["Value"] == 0:
                validation_data['BlockIdentifier'] = BlockIdentifier
                validation_data_list.append(validation_data)
                validataion_data_path_list.append(path)

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
                pass
            else:
                valid = False
                self.vlog('NvM011002', 'NvMNvramBlockIdentifier should be unique.',
                          f"NvMNvramBlockIdentifier {validation_data['BlockIdentifier']} should be unique.",
                          validataion_data_path_list[index], 'fail')

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)

        return valid
