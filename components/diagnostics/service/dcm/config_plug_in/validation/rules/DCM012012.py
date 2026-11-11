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


# DcmDspDataUsePort 动态配置项缺失
class Dcm012012Rule(BasicRule):
    ruleset_name = "Dcm012012"
    validation_result = True

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DcmDspDataUsePort & DcmDspDataResetToDefaultFnc Missing Check."

        self.durable_init()

    def durable_init(self):
        # 注：固定名称的规则集只能初始化一次，否则会异常报错
        try:
            get_host().get_ruleset(self.__class__.ruleset_name)
        except Exception as e:
            with ruleset(self.__class__.ruleset_name):

                @when_all(m.use_port != 'USE_DATA_ASYNCH_FNC')
                def auto_gen(c):
                    self.__class__.validation_result = True

                @when_all(m.is_default == False)
                def auto_gen1(c):
                    self.__class__.validation_result = True

                @when_all((m.default_fnc != '') & (m.default_fnc != None))
                def dynamic_lost(c):
                    self.__class__.validation_result = True

            pass

    @timing_decorator
    def validation_run(self, ecuc_dcm_dict):
        valid = True
        # 1）准备校验数据
        base_path, did_list = DcmRuleUtils().get_dcm_dsp_did_list(ecuc_dcm_dict)
        base_path, did_info_list = DcmRuleUtils().get_dcm_dsp_did_info_list(ecuc_dcm_dict)
        base_path, data_list = DcmRuleUtils().get_dcm_dsp_data_list(ecuc_dcm_dict)
        base_path, data_info_list = DcmRuleUtils().get_dcm_dsp_data_info_list(ecuc_dcm_dict)

        for did_key, did_value in did_list.items():
            did_info_ref = did_value["DcmDspDidInfoRef"]["ValueRef"].split("/")[-1]
            did_info_ref_dict = DcmRuleUtils().dcm_rule_get_value_from_dict_by_key(did_info_list, did_info_ref)

            if len(did_info_ref_dict) == 0:
                return valid
            if did_value["DcmDspDidUsePort"]["Value"] == "USE_DATA_ELEMENT_SPECIFIC_INTERFACES":
                for did_value_key, did_value_value in did_value.items():
                    if isinstance(did_value_value, dict):
                        if "DefinitionRef" in did_value_value.keys():
                            if did_value_value["DefinitionRef"].split("/")[-1] == "DcmDspDidSignal":
                                dsp_data_ref = did_value_value["DcmDspDidDataRef"]["ValueRef"].split("/")[
                                  -1]  #dsp_data_short_name
                                dsp_data_dict = DcmRuleUtils().dcm_rule_get_value_from_dict_by_key(
                                  data_list, dsp_data_ref)
                                validation_data = {}
                                validation_data['use_port'] = dsp_data_dict['DcmDspDataUsePort']['Value']
                                validation_data['is_default'] = DcmRuleUtils().dcm_rule_check_did_exist(
                                  did_info_ref_dict, 'ResetToDefault')
                                validation_data['default_fnc'] = dsp_data_dict['DcmDspDataResetToDefaultFnc']['Value']
                                validataion_data_path = base_path + '/' + dsp_data_ref + '/' + 'DcmDspDataResetToDefaultFnc'

                                # 2） 开始校验
                                try:
                                    self.__class__.validation_result = False
                                    post(self.__class__.ruleset_name, validation_data)
                                except Exception as e:
                                    self.__class__.validation_result = False

                                # 3） 打印日志
                                if self.__class__.validation_result:
                                    self.vlog(self.validation_code, self.validation_summary,
                                              f"Data {dsp_data_ref} DcmDspDataResetToDefaultFnc Missing Check Success.",
                                              validataion_data_path, "pass")
                                else:
                                    self.vlog(self.validation_code, self.validation_summary,
                                              f"Data {dsp_data_ref} UsePort is USE_DATA_ASYNCH_FNC, DcmDspDataResetToDefaultFnc is missing.",
                                              validataion_data_path, "fail")
                                    valid = False

                                retract_facts(self.__class__.ruleset_name, validation_data)
        return valid

    def add_rule_time(self):
        pass
