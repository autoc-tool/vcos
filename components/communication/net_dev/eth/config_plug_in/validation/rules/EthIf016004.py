# /*
#  * Copyright (c) 2025 Li Auto Inc. and its affiliates
#  * Licensed under the Apache License, Version 2.0(the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
#  */
from func_module.validation.common.basic_rule import BasicRule, timing_decorator
from durable.lang import *
from durable.engine import *
from .EthIf_utils import EthIfRuleUtils
from ui_adaptor.data_management import data_management

# EthCoreRef 校验
class EthIf016004Rule(BasicRule):
    ruleset_name = 'ETHIF016004'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Eth EthCoreRef should be consistent with Eth Mcal!"

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

                @when_all(m.ethcore == m.mcalethcore)
                def fact(c):
                    self.__class__.validation_result = True

    def check_validation_run(self):
        if not self.modules_are_exist(["Eth","ResourceM"], self.bsw_ecuc_cfg_class_dict):
            return False
        return True

    def get_ethmcal_core(self,resourcemcfg_dict):
        coreid = 'CORE0'
        for k, v in resourcemcfg_dict.items():
            if not isinstance(v, dict):
                continue
            if v["DefinitionRef"].split("/")[-1] in ["ResourceMMasterCore", "ResourceMMcalCore"]:
                for k1,v1 in v.items():
                    if not isinstance(v1, dict):
                        continue
                    if "ResourceMAllocation" == v1["DefinitionRef"].split("/")[-1] and "ETH" == v1["ResourceMModuleName"]["Value"]:
                        coreid =  v["ResourceMCoreID"]["Value"]
                        return coreid
        return coreid

    @timing_decorator
    def validation_run(self):
        self.ecuc_eth_dict = self.bsw_ecuc_cfg_class_dict['Eth']['ArPackages']['ActiveEcuC']['Eth']
        ecuc_eth_dict = self.ecuc_eth_dict
        target = EthIfRuleUtils().EthIf_Target
        ecuc_resourcem_dict = self.bsw_ecuc_cfg_class_dict['ResourceM']['ArPackages']['ActiveEcuC']['ResourceM']
        # 1）准备校验数据
        ethcfg_name, ethcfg_dict = EthIfRuleUtils.ethif_get_dict_by_definition_ref(ecuc_eth_dict, "EthConfigSet")
        base_path = "/ActiveEcuC/Eth/" + ethcfg_name

        resourcemcfg_name, resourcemcfg_dict = EthIfRuleUtils.ethif_get_dict_by_definition_ref(ecuc_resourcem_dict, "ResourceMMcalConfig")

        mcalethcore = self.get_ethmcal_core(resourcemcfg_dict)
        for k, v in ethcfg_dict.items():
            if not isinstance(v, dict):
                continue
            if "EthCtrlConfig" == v["DefinitionRef"].split("/")[-1]:
                for k1,v1 in v.items():
                    if not isinstance(v1, dict):
                        continue
                    if "EthCoreRef" == v1["DefinitionRef"].split("/")[-1]:
                        validataion_data_path = base_path + "/" + k + "/" + k1 + "EthCoreRef"
                        validation_data = {}
                        if v1["ValueRef"]:
                            validation_data["ethcore"] = int(v1["ValueRef"][-1])
                            validation_data["mcalethcore"] =int(mcalethcore[-1])

                            # 2） 开始校验
                            try:
                                self.__class__.validation_result = False
                                post(self.__class__.ruleset_name, validation_data)
                            except Exception as e:
                                self.__class__.validation_result = False
                                pass
                            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
                            if self.__class__.validation_result:
                                self.vlog(self.validation_code,  self.validation_summary,
                                            f"EthCoreRef coreid={validation_data['ethcore']} is consistent with that defined in ResourceM.",
                                            validataion_data_path, "pass")
                            else:
                                self.vlog(self.validation_code,  self.validation_summary,
                                            f"EthCoreRef coreid={validation_data['ethcore']} is not consistent with that defined in ResourceM.",
                                            validataion_data_path, "fail")

                            # 3）清空ruleset中的所有fact，为下一次校验做准备
                            if len(validation_data) > 0:
                                retract_facts(self.__class__.ruleset_name, validation_data)
                            pass
                            break



    def add_rule_time(self):
        pass

