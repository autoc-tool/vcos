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

# 通过definition 获取对应的dict
def com_tools_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"].split("/")[-1] == definition_ref:    
                    return v    
    return {}

# 互斥校验：Signal的数据类型与Signal Size匹配
class Com013001Rule(BasicRule):
    ruleset_name = "Com013001"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_com_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Validation Of ComSignalSize and ComSignal Type!"

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

                @when_all(+m.comsignal_name)
                def fact(c):
                    self.__class__.validation_result = True

    def check_validation_run(self):
        if not self.modules_are_exist(["Com"], self.bsw_ecuc_cfg_class_dict):
            self.vlog(self.validation_code, self.validation_summary, "Data required for validation is missing",
                self.validation_summary, "fail")
            return False
        return True

    def check_basic_type(self, ComConfig, signal, signal_type, signal_size):
        '''
        常见的类型有boolean sint8 sint16 sint32 sint64 uint8 uint16 uint32 uint64 float16 float32 float64
        UINT8_N这种signal的数组样式也支持
        struct 暂不支持        
        '''
        check_result = 0
        sig_initValue = 0
        if signal_type == "BOOLEAN":
            sig_initValue = int(ComConfig["ComSignals"][signal]["ComSignalInitValue"]["Value"])
            if signal_size > 1 or sig_initValue > 1:
                check_result = -1
        elif signal_type == "UINT8" or signal_type == "SINT8":
            sig_initValue = int(ComConfig["ComSignals"][signal]["ComSignalInitValue"]["Value"])
            if signal_size > 8 or signal_size <= 1 or sig_initValue > 0xFF:
                check_result = -1
        elif signal_type == "UINT16" or signal_type == "SINT16":
            sig_initValue = int(ComConfig["ComSignals"][signal]["ComSignalInitValue"]["Value"])
            if signal_size > 16 or signal_size <= 8 or sig_initValue > 0xFFFF:
                check_result = -1
        elif signal_type == "UINT32" or signal_type == "SINT32" or signal_type == "FLOAT32":
            sig_initValue = int(ComConfig["ComSignals"][signal]["ComSignalInitValue"]["Value"])
            if signal_size > 32 or signal_size <= 16 or sig_initValue > 0xFFFFFFFF:
                check_result = -1
        elif signal_type == "UINT64" or signal_type == "SINT64" or signal_type == "FLOAT64":
            sig_initValue = int(ComConfig["ComSignals"][signal]["ComSignalInitValue"]["Value"])
            if signal_size > 64 or signal_size <= 32 or sig_initValue > 0xFFFFFFFFFFFFFFFF:
                check_result = -1
        else:
            if signal_type == "UINT8_N" or signal_type == "UINT8_DYN":
                if signal_size <= 32 or signal_size % 8 != 0:
                    check_result = -1
        return check_result


    @timing_decorator
    def validation_run(self):
        validation_data_list = []
        validataion_data_path_list = []
        valid = True

        # 1）准备校验数据
        ComConfig = {}
        ecuc_com_dict = self.ecuc_com_dict
        if "Com" in ecuc_com_dict.keys():
            com_dict = ecuc_com_dict['Com']['ArPackages']['ActiveEcuC']['Com']
            com_config_dict = com_tools_get_dict_by_definition_ref(com_dict,"ComConfig")
        else:
            com_config_dict = com_tools_get_dict_by_definition_ref(ecuc_com_dict,"ComConfig") 
        com_signal_dict = {}
        for ComConfigKey, value in com_config_dict.items():
            if ComConfigKey == "DefinitionRef" or ComConfigKey == "Tag":
                continue
            if isinstance(value, dict):
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComSignal":
                    com_signal_dict[ComConfigKey] = value
        
        ComConfig["ComSignals"] = com_signal_dict
        base_path = "/ActiveEcuC/Com/ComConfig/"

        # 2）每个信号进行校验
        for signal in list(ComConfig["ComSignals"].keys()):
            validation_data = {}
            validation_data["comsignal_name"] = signal
            com_signal_ref_path = base_path + "/" + signal

            signal_type = ""
            signal_size = 0
            if "ComSignalType" in ComConfig["ComSignals"][signal]:
                signal_type = ComConfig["ComSignals"][signal]["ComSignalType"]["Value"]
                if ComConfig["ComSignals"][signal]["ComSignalType"]["Value"] == "UINT8_N" or ComConfig["ComSignals"][signal]["ComSignalType"]["Value"] == "UINT8_DYN":
                    signal_size = 8 * ComConfig["ComSignals"][signal]["ComSignalLength"]["Value"]
                else:
                    if "ComBitSize" in ComConfig["ComSignals"][signal]:
                        signal_size = ComConfig["ComSignals"][signal]["ComBitSize"]["Value"]
            check_result = self.check_basic_type(ComConfig, signal, signal_type, signal_size)
            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if check_result == 0:
                self.vlog(self.validation_code, self.validation_summary,
                          f"ComSignal  {validation_data['comsignal_name']} Type and Size and Initial Value matching ",
                          com_signal_ref_path, "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                          f"ComSignal  {validation_data['comsignal_name']} Type and Size and Initial Value is not match",
                          com_signal_ref_path, "fail")
                valid = False
        return valid
