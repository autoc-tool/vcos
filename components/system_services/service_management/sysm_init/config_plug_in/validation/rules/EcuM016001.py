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
# DriverInitListBswM ETH配置核校验
class EcuM016001Rule(BasicRule):
    ruleset_name = "ECUM016001"
    validation_result = True

    def __init__(self, logs, bsw_dict, ecum_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_dict = bsw_dict
        self.ecum_dict = bsw_dict['EcuM']['ArPackages']['ActiveEcuC']['EcuM']
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "The modules related to Ethernet should be initialized on the same core of Eth."

    def check_validation_run(self):
        if not self.modules_are_exist(["EcuM","Eth"], self.bsw_dict):
            return False

        return True

    def get_dict_by_definition_ref(self, input_dict: dict, definition_ref: str) -> dict:
        for key, value in input_dict.items():
            if isinstance(value, dict):
                if "DefinitionRef" in value.keys():
                    if value["DefinitionRef"].split("/")[-1] == definition_ref:
                        return key, value
        return "", {}


    @timing_decorator
    def validation_run(self):
        # 1）准备校验数据
        if "EthMcal" not in self.bsw_dict.keys():
            ecuc_ecum_dict = self.ecum_dict
            ecum_dict = ecuc_ecum_dict['EcuMConfiguration']['EcuMFlexConfiguration']
            ecuc_eth_dict = self.bsw_dict['Eth']['ArPackages']['ActiveEcuC']['Eth']
            ethcfg_name, ethcfg_dict = self.get_dict_by_definition_ref(ecuc_eth_dict, "EthConfigSet")
            eth_module_list = {"Eth","EthIf", "EthSM", "EthTSyn", "SoAd", "DoIP", "TcpIp", "XcpOnEth", "StbM"}
            base_path = '/ActiveEcuC/EcuM/EcuMConfiguration/EcuMFlexConfiguration'
            ethcore = None
            for k, v in ethcfg_dict.items():
                if not isinstance(v, dict):
                    continue
                if "EthCtrlConfig" == v["DefinitionRef"].split("/")[-1]:
                    for k1,v1 in v.items():
                        if not isinstance(v1, dict):
                            continue
                        if "EthCoreRef" == v1["DefinitionRef"].split("/")[-1]:
                            if v1["ValueRef"]:
                                ethcore = v1["ValueRef"][-1]
                                break

            for k_ecum, v_ecum in ecum_dict.items():
                if not isinstance(v_ecum, dict):
                    continue
                if "EcuMDriverInitListBswM" == v_ecum["DefinitionRef"].split("/")[-1]:
                    # 获取当前初始化列表对应核
                    if k_ecum in ["EcuMDriverInitListTwo", "EcuMDriverInitListThree"]:
                        init_coreid = '0'
                    else:
                        init_coreid = k_ecum[-1]
                    for k1, v1 in v_ecum.items():
                        if not isinstance(v1, dict):
                            continue
                        if "EcuMDriverInitItem" == v1["DefinitionRef"].split("/")[-1]:
                            if v1['EcuMModuleRef']['ValueRef']:
                                module_name = v1['EcuMModuleRef']['ValueRef'].split('/')[-1]
                                if module_name in eth_module_list and init_coreid != ethcore:
                                    self.vlog(self.validation_code, self.validation_summary,
                                        f"{module_name} init core not consistent with Eth.", f"{base_path}/{k_ecum}/{k1}", "fail")

                            elif v1['EcuMAdditionalInitCode']['Value']:
                                for module_name in eth_module_list:
                                    if module_name in v1['EcuMAdditionalInitCode']['Value'] and init_coreid != ethcore:
                                        self.vlog(self.validation_code, self.validation_summary,
                                            f"{module_name} init core not consistent with Eth.", f"{base_path}/{k_ecum}/{k1}", "fail")

        pass


    def add_rule_time(self):
        pass
