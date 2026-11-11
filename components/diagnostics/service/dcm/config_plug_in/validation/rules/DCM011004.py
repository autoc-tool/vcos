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


# DcmDslProtocolRxPduRef 重复校验
class Dcm011004Rule(BasicRule):
    ruleset_name = 'Dcm011004'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DcmDslProtocolRxPduRef should not be same!"

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

                @when_all(+m.rx_pdu_ref)
                def fact(c):
                    self.__class__.validation_result = True

    @timing_decorator
    def validation_run(self, ecuc_dcm_dict):
        valid = True
        # 1）准备校验数据
        base_path, dsl_protocol_row_list = DcmRuleUtils().get_dcm_dsl_protocol_row_list(ecuc_dcm_dict)

        validation_data_list = []
        validataion_data_path_list = []
        for protocol_key, protocol_value in dsl_protocol_row_list.items():
            protocol_row_path = base_path + '/' + protocol_key
            dsl_connection_list = DcmRuleUtils().dcm_rule_get_dict_rows_by_definition_ref(
              protocol_value, 'DcmDslConnection')
            for connection_key, connection_value in dsl_connection_list.items():
                connection_path = protocol_row_path + '/' + connection_key
                main_connection_list = DcmRuleUtils().dcm_rule_get_dict_rows_by_definition_ref(
                  connection_value, 'DcmDslMainConnection')
                for main_connection_key, main_connection_value in main_connection_list.items():
                    main_connection_path = connection_path + '/' + main_connection_key
                    protocol_rx_list = DcmRuleUtils().dcm_rule_get_dict_rows_by_definition_ref(
                      main_connection_value, 'DcmDslProtocolRx')
                    for protocol_rx_key, protocol_rx_value in protocol_rx_list.items():
                        rx_pdu_path = main_connection_path + '/' + protocol_rx_key
                        rx_pdu_ref_path = rx_pdu_path + '/' + 'DcmDslProtocolRxPduRef'
                        validation_data = {}
                        validation_data['rx_pdu_ref'] = protocol_rx_value['DcmDslProtocolRxPduRef']['ValueRef'].split(
                          '/')[-1]
                        validation_data_list.append(validation_data)
                        validataion_data_path_list.append(rx_pdu_ref_path)
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
                          f"Rx Pdu {validation_data['rx_pdu_ref']} is not duplicated.",
                          validataion_data_path_list[index], 'pass')
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"Rx Pdu {validation_data['rx_pdu_ref']} is duplicated.", validataion_data_path_list[index],
                          'fail')
                valid = False

        # 3）清空ruleset中的所有fact，为下一次校验做准备
        if len(validation_data_list) > 0:
            retract_facts(self.__class__.ruleset_name, validation_data_list)
        return valid

    def add_rule_time(self):
        pass
