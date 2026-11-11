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
TPMOD = ["DCM", "CANTP", "LINTP", "DOIP"]

class PduR016008Rule(BasicRule):
    ruleset_name = "PduR016008"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Configs of PduRBswModules should be compatible with each others!!"
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict

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
                @when_all(+m.frame_id)
                def fact(c):
                    self.__class__.validation_result = True
            pass

    def check_validation_run(self):
        if not self.modules_are_exist(["PduR"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True

    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        ecuc_pdur_dict = self.bsw_ecuc_cfg_class_dict["PduR"]["ArPackages"]["ActiveEcuC"]["PduR"]
        isLoTTModuleExisted = False
        isUpTTModuleExisted = False
        LoTTUpTTpath = '/ActiveEcuC/PduR/'
        valid = True
        for name, BswModule in ecuc_pdur_dict.items():
            path = '/ActiveEcuC/PduR/' + name
            error_info = ''
            if "DefinitionRef" not in BswModule or \
                BswModule["DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRBswModules":
                continue
            bsw_module_name = BswModule['PduRBswModuleRef']['ValueRef'].split('/')[-1]
            if bsw_module_name == 'PduR': # 特殊模块无需校验
                continue
            # 只能是Lower或Upper中的一者，不能同时是或同时不是
            if BswModule['PduRLowerModule']['Value'] is True and BswModule['PduRUpperModule']['Value'] is True:
                error_info = f'BswModule should only be UpperModule or LowerModule'
                self.vlog(self.validation_code, self.validation_summary, error_info, path, "fail")
                valid = False
            if BswModule['PduRLowerModule']['Value'] is False and BswModule['PduRUpperModule']['Value'] is False:
                error_info = f'BswModule should only be UpperModule or LowerModule'
                self.vlog(self.validation_code, self.validation_summary, error_info, path, "fail")
                valid = False
            # Lower必须是If或Tp的一者
            if BswModule['PduRLowerModule']['Value'] is True and \
                BswModule['PduRCommunicationInterface']['Value'] is False and \
                BswModule['PduRTransportProtocol']['Value'] is False:
                error_info = f'LowerBswModule should be If-Type or Tp-Type or Both'
                self.vlog(self.validation_code, self.validation_summary, error_info, path, "fail")
                valid = False
            # Upper必须是If或Tp的一者
            if BswModule['PduRUpperModule']['Value'] is True and \
                BswModule['PduRCommunicationInterface']['Value'] is False and \
                BswModule['PduRTransportProtocol']['Value'] is False:
                error_info = f'UpperBswModule should be If-Type or Tp-Type or Both'
                self.vlog(self.validation_code, self.validation_summary, error_info, path, "fail")
                valid = False
            # TT必须是If
            if BswModule['PduRTriggertransmit']['Value'] is True and \
                BswModule['PduRCommunicationInterface']['Value'] is False:
                error_info = f'TriggerTransmit Module should only be If-Type'
                self.vlog(self.validation_code, self.validation_summary, error_info, path, "fail")
                valid = False
            # 如果有LoTT存在，那必须有UpTT存在
            if BswModule['PduRLowerModule']['Value'] is True and BswModule['PduRTriggertransmit']['Value'] is True:
                isLoTTModuleExisted = True
                LoTTUpTTpath =  '/ActiveEcuC/PduR/' + name
            if BswModule['PduRUpperModule']['Value'] is True and BswModule['PduRTriggertransmit']['Value'] is True:
                isUpTTModuleExisted = True
        if isLoTTModuleExisted is True and isUpTTModuleExisted is False:
            error_info = f'At least one UpperModule-TriggerTransmit Existed if LowerModule-TriggerTransmit Existed'
            self.vlog(self.validation_code, self.validation_summary, error_info, LoTTUpTTpath, "fail")
            valid = False
        return valid

    def add_rule_time(self):
        pass