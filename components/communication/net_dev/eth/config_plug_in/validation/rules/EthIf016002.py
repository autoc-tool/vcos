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

# EthIfCtrlIdx 重复校验
class EthIf016002Rule(BasicRule):
    ruleset_name = 'ETHIF016002'
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_ethif_dict = bsw_ecuc_cfg_class_dict['EthIf']['ArPackages']['ActiveEcuC']['EthIf']
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "EthIf Controller EthIfMaxTxBufsTotal should be consistent with Eth!"

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

                @when_all(m.ethifmaxtxbufstotal == m.ethmaxtxbufstotal)
                def fact(c):
                    self.__class__.validation_result = True

    def check_validation_run(self):
        if not self.modules_are_exist(["Eth","EthIf"], self.bsw_ecuc_cfg_class_dict):
            return False
        return True

    def get_ethtxbuf_by_phy_ref(self,ethifconfigset_dict,ecuc_eth_dict,ethifphyctrlref):
        target = EthIfRuleUtils().EthIf_Target
        for controller_key, controller_value in ethifconfigset_dict.items():
            if not isinstance(controller_value, dict):
                continue
            if "EthIfPhysController" == controller_value["DefinitionRef"].split("/")[-1]:
                if ethifphyctrlref.split("/")[-1] == controller_key:
                    EthCtrlname = controller_value["EthIfEthCtrlRef"]["ValueRef"].split("/")[-1]
                    break

        if  EthCtrlname:
                ethconfigset_name, ethconfigset_dict = EthIfRuleUtils.ethif_get_dict_by_definition_ref(ecuc_eth_dict, "EthConfigSet")
                for ethctrl_key, ethctrl_value in ethconfigset_dict.items():
                    if not isinstance(ethctrl_value, dict):
                        continue
                    if "EthCtrlConfig" == ethctrl_value["DefinitionRef"].split("/")[-1]:
                        if  EthCtrlname == ethctrl_key:
                            if "TC" in target:
                                if ("EthTxBufTotal" not in ethctrl_value) or (ethctrl_value["EthTxBufTotal"] == None):
                                    continue
                                return ethctrl_value["EthTxBufTotal"]["Value"]
                            elif "U2A16" in target:
                                ethbuf = 0
                                for ethctrlsub_key, ethctrlsub_value in ethctrl_value.items():
                                    if not isinstance(ethctrlsub_value, dict):
                                        continue
                                    if "EthTxQueueConfig" == ethctrlsub_value["DefinitionRef"].split("/")[-1]:
                                        ethbuf += ethctrlsub_value["EthTxQueueBufs"]["Value"]
                                return ethbuf
                            elif "J6E" in target:
                                ethbuf = 0
                                for ethctrlsub_key, ethctrlsub_value in ethctrl_value.items():
                                    if not isinstance(ethctrlsub_value, dict):
                                        continue
                                    if "EthCtrlConfigEgress" == ethctrlsub_value["DefinitionRef"].split("/")[-1]:
                                        for ethctrlegresssub_key, ethctrlegresssub_value in ethctrlsub_value.items():
                                            if not isinstance(ethctrlegresssub_value, dict):
                                                continue
                                            if "EthCtrlConfigEgressFifo" == ethctrlegresssub_value["DefinitionRef"].split("/")[-1]:
                                                ethbuf += ethctrlegresssub_value["EthCtrlConfigEgressFifoBufTotal"]["Value"]
                                return ethbuf

        return None


    @timing_decorator
    def validation_run(self):
        self.ecuc_eth_dict = self.bsw_ecuc_cfg_class_dict['Eth']['ArPackages']['ActiveEcuC']['Eth']
        ecuc_ethif_dict = self.ecuc_ethif_dict
        ecuc_eth_dict = self.ecuc_eth_dict

        # 1）准备校验数据
        ethifconfigset_name, ethifconfigset_dict = EthIfRuleUtils.ethif_get_dict_by_definition_ref(ecuc_ethif_dict, "EthIfConfigSet")
        base_path = "/ActiveEcuC/EthIf/"+ ethifconfigset_name

        for controller_key, controller_value in ethifconfigset_dict.items():
            if not isinstance(controller_value, dict):
                continue
            if "EthIfController" == controller_value["DefinitionRef"].split("/")[-1]:
                validataion_data_path = base_path + "/" + controller_key + "/" + "EthIfMaxTxBufsTotal"
                validation_data = {}
                validation_data["ethifmaxtxbufstotal"] = controller_value["EthIfMaxTxBufsTotal"]["Value"]
                ethifphyctrlref = controller_value["EthIfPhysControllerRef"]["ValueRef"]
                validation_data["ethmaxtxbufstotal"] = self.get_ethtxbuf_by_phy_ref(ethifconfigset_dict,ecuc_eth_dict,ethifphyctrlref)
                if validation_data["ethmaxtxbufstotal"] == None:
                    continue


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
                                f"EthIfMaxTxBufsTotal {validation_data['ethifmaxtxbufstotal']} is consistent with Eth.",
                                validataion_data_path, "pass")
                else:
                    self.vlog(self.validation_code,  self.validation_summary,
                                f"EthIfMaxTxBufsTotal {validation_data['ethifmaxtxbufstotal']} is not consistent with Eth.",
                                validataion_data_path, "fail")

                # 3）清空ruleset中的所有fact，为下一次校验做准备
                if len(validation_data) > 0:
                    retract_facts(self.__class__.ruleset_name, validation_data)
                pass


    def add_rule_time(self):
        pass

