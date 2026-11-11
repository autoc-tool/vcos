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
import copy

# 通过definition 获取对应的dict
def com_tools_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"].split("/")[-1] == definition_ref:    
                    return v    
    return {}

def check_prefix(string):
    prefix = 'Com_MainFunction'
    # 检查字符串前3个字符是否为'Com_'
    if string.startswith(prefix):
        return True
    else:
        return False
    
def Find_MainFunction_Core_Map(ComMainFunction_tx, ComMainFunction_rx, Rte_dict, EcuC_dict, Os_dict):
    Rte_Com_TaskMapping = {}
    Os_Application = {}
    if "Com" in Rte_dict.keys():
        for key, value in Rte_dict["Com"].items():
            if isinstance(value, dict):
                if value["DefinitionRef"] == '/AUTOSAR/EcucDefs/Rte/RteBswModuleInstance/RteBswEventToTaskMapping':
                    Rte_Com_TaskMapping[key] = value
    
    # 将mainfuncitonTx的task name找到
    for mainfunction_tx in list(ComMainFunction_tx.keys()):
        Rte_MainFunction_Key = mainfunction_tx + "TimingEvent0"
        if Rte_MainFunction_Key in Rte_Com_TaskMapping.keys():
            task_name = Rte_Com_TaskMapping[Rte_MainFunction_Key]['RteBswMappedToTaskRef']['ValueRef']
            task_name = task_name[task_name.rfind("/") + 1:]
            ComMainFunction_tx[mainfunction_tx]["task_name"] = task_name
    

    # 将mainfuncitonRx的task name找到
    for mainfunction_rx in list(ComMainFunction_rx.keys()):
        Rte_MainFunction_Key = mainfunction_rx + "TimingEvent0"
        if Rte_MainFunction_Key in Rte_Com_TaskMapping.keys():
            task_name = Rte_Com_TaskMapping[Rte_MainFunction_Key]['RteBswMappedToTaskRef']['ValueRef']
            task_name = task_name[task_name.rfind("/") + 1:]
            ComMainFunction_rx[mainfunction_rx]["task_name"] = task_name
    
    
    # 找到OS的所有Application
    for key, value in Os_dict.items():
         if isinstance(value, dict):
            if value["DefinitionRef"] == '/AUTOSAR/EcucDefs/Os/OsApplication':
                Os_Application[key] = value
            
    # 将Application的OsAppTaskRef转成list 防止有str的情况
    Os_Application_temp = copy.deepcopy(Os_Application)
    Templist = []
    for key, value in Os_Application_temp.items():
        if value['OsAppTaskRef']["ValueRef"] != None:
            if isinstance(value['OsAppTaskRef']["ValueRef"], str):
                Templist.append(value['OsAppTaskRef']["ValueRef"])
                value['OsAppTaskRef']["ValueRef"] = Templist.copy()
                Templist.clear()
    
    # 将mainfuncitonTx的CoreRef
    for mainfunction_tx in list(ComMainFunction_tx.keys()):
        for key, value in Os_Application_temp.items():
            if isinstance(value['OsAppTaskRef']["ValueRef"], list):
                for task_name in list(value['OsAppTaskRef']["ValueRef"]):
                    task_name = task_name[task_name.rfind("/") + 1:]
                    if "task_name" in ComMainFunction_tx[mainfunction_tx].keys():
                        if task_name == ComMainFunction_tx[mainfunction_tx]["task_name"]:
                            coreRef = value['OsApplicationCoreRef']["ValueRef"]
                            coreRef = coreRef[coreRef.rfind("/") + 1:]
                            ComMainFunction_tx[mainfunction_tx]["CoreRef"] =  coreRef
    
    # 将mainfuncitonRx的CoreRef
    for mainfunction_rx in list(ComMainFunction_rx.keys()):
        for key, value in Os_Application_temp.items():
            if isinstance(value['OsAppTaskRef']["ValueRef"], list):
                for task_name in list(value['OsAppTaskRef']["ValueRef"]):
                    task_name = task_name[task_name.rfind("/") + 1:]
                    if "task_name" in ComMainFunction_rx[mainfunction_rx].keys():
                        if task_name == ComMainFunction_rx[mainfunction_rx]["task_name"]:
                            coreRef = value['OsApplicationCoreRef']["ValueRef"]
                            coreRef = coreRef[coreRef.rfind("/") + 1:]
                            ComMainFunction_rx[mainfunction_rx]["CoreRef"] =  coreRef

# 互斥校验：CAN多核部署中 Com IPDU绑定的MainFunction部署的Core和Ecuc关联的Core必须一致
class Com013007Rule(BasicRule):
    ruleset_name = "Com013007"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.bsw_ecuc_cfg_class_dict = bsw_ecuc_cfg_class_dict
        self.ecuc_com_dict = bsw_ecuc_cfg_class_dict
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "Validation Of IPDU&MainFunction CoreRef!"

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


    @timing_decorator
    def validation_run(self):
        validation_data_list = []
        validataion_data_path_list = []
        valid = True
        check_result = 0

        # 1）准备校验数据
        ComConfig = {}
        EcuC_dict = {}
        Rte_dict = {}
        Os_dict = {}

        # 提取Com Ecuc Rte Os的dict
        ecuc_com_dict = self.ecuc_com_dict
        if "Com" in ecuc_com_dict.keys():
            com_dict = ecuc_com_dict['Com']['ArPackages']['ActiveEcuC']['Com']
            com_config_dict = com_tools_get_dict_by_definition_ref(com_dict,"ComConfig")
        else:
            com_config_dict = com_tools_get_dict_by_definition_ref(ecuc_com_dict,"ComConfig") 

        if "EcuC" in ecuc_com_dict.keys():
            EcuC_dict = ecuc_com_dict['EcuC']['ArPackages']['ActiveEcuC']['EcuC']
        

        if "Rte" in ecuc_com_dict.keys():
            Rte_dict = ecuc_com_dict['Rte']['ArPackages']['ActiveEcuC']['Rte']
        
        if "Os" in ecuc_com_dict.keys():
            Os_dict = ecuc_com_dict['Os']['ArPackages']['ActiveEcuC']['Os']
    
        com_ipdu_dict = {}
        ComMainFunction_tx = {}
        ComMainFunction_rx = {}
        ComMainFunction_gw = {}
        
        # 提取Com 详细信息
        for ComConfigKey, value in com_config_dict.items():
            if ComConfigKey == "DefinitionRef" or ComConfigKey == "Tag":
                continue
            if isinstance(value, dict):
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComIPdu":
                    com_ipdu_dict[ComConfigKey] = value
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionTx":
                    ComMainFunction_tx[ComConfigKey] = value
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionRx":
                    ComMainFunction_rx[ComConfigKey] = value
                if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/Com/ComConfig/ComMainFunctionGw":
                    ComMainFunction_gw[ComConfigKey] = value
        
        # 通过Rte Ecuc Os找到MainFucntion关联的Core
        Find_MainFunction_Core_Map(ComMainFunction_tx, ComMainFunction_rx, Rte_dict, EcuC_dict, Os_dict)
        
        ComConfig["ComIPdus"] = com_ipdu_dict
        base_path = "/ActiveEcuC/Com/ComConfig/"
        ComIpdus_sort = ComConfig["ComIPdus"]

        # 2）每个IPDU进行校验
        for IPdu in list(ComIpdus_sort.keys()):
            validation_data = {}
            validation_data["comipdu_name"] = IPdu
            com_ipdu_ref_path = base_path + "/" + IPdu

            check_result = 0
            # check
            #step1 找到该IPDU ref的Ecuc pdu的Core信息
            ComPduRef = ComIpdus_sort[IPdu]["ComPduIdRef"]["ValueRef"]
            ComPduRef_name = ComPduRef[ComPduRef.rfind("/") + 1:]

            # keep message name correct
            ComIpdus_sort[IPdu]["MessageName"] = ComPduRef_name
            EcucPduCoreId = 0
            if ComPduRef_name in EcuC_dict["EcucConfigSet"]["EcucPduCollection"]:
                ComIpdus_sort[IPdu]["PduRef"] = EcuC_dict["EcucConfigSet"]["EcucPduCollection"][ComPduRef_name]
                # 提取IPDU的CoreRef信息
                if "EcucPduDefaultCoreRef" in ComIpdus_sort[IPdu]["PduRef"].keys():
                    EcucPduDefaultCoreRef = ComIpdus_sort[IPdu]["PduRef"]["EcucPduDefaultCoreRef"]["ValueRef"]
                    if isinstance(EcucPduDefaultCoreRef, list):
                        EcucPduDefaultCoreRef = EcucPduDefaultCoreRef[0]
                else:
                    EcucPduDefaultCoreRef = None

            if EcucPduDefaultCoreRef in {None , "" ," "}:
                # 当前CoreRef未映射 则默认是Core0  该场景是单核的情况 不做校验
                ComIpdus_sort[IPdu]["CoreRef"] = None
            else:
                # 提取/最后的字符
                EcucPduDefaultCore = EcucPduDefaultCoreRef[EcucPduDefaultCoreRef.rfind("/") + 1:]
                ComIpdus_sort[IPdu]["CoreRef"] = EcucPduDefaultCore


            #step2 找到该IPDU ref的mainfunction的taskmapping 关联的Core信息
            IPduMainFunctionRef = ComIpdus_sort[IPdu]['IPduMainFunctionRef']['ValueRef']
            IPduMainFunctionRef = IPduMainFunctionRef[IPduMainFunctionRef.rfind("/") + 1:]

            if ComIpdus_sort[IPdu]["ComIPduDirection"]["Value"] == "SEND":
                if ComIpdus_sort[IPdu]["CoreRef"] != None:
                    if "CoreRef" in ComMainFunction_tx[IPduMainFunctionRef].keys(): 
                        if (ComMainFunction_tx[IPduMainFunctionRef]["CoreRef"] != ComIpdus_sort[IPdu]["CoreRef"]):
                            check_result = -1
            else:
                if ComIpdus_sort[IPdu]["CoreRef"] != None:
                    if "CoreRef" in ComMainFunction_rx[IPduMainFunctionRef].keys(): 
                        if (ComMainFunction_rx[IPduMainFunctionRef]["CoreRef"] != ComIpdus_sort[IPdu]["CoreRef"]):
                            check_result = -1

            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            if check_result == 0:
                self.vlog(self.validation_code, self.validation_summary,
                            f"ComIPdu  {validation_data['comipdu_name']} Ecuc Pdu CoreRef == mainfunctionref task Core ",
                            com_ipdu_ref_path, "pass")
            else:
                self.vlog(self.validation_code, self.validation_summary,
                            f"ComIPdu  {validation_data['comipdu_name']} Ecuc Pdu CoreRef != mainfunctionref task Core  ",
                            com_ipdu_ref_path, "fail")
                valid = False
        return valid
