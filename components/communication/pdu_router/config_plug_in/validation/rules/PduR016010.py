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
from .PduR_rule_func import pdur_validate_get_routing_path
TPMOD = ["DCM", "CANTP", "LINTP", "DOIP"]

class PduR016010Rule(BasicRule):
    ruleset_name = "PduR016010"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "DestPdu of IF Message cannot have two or more SrcPdu!"
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
        all_correct = True
        # 1）准备校验数据
        ecuc_pdur_dict = self.bsw_ecuc_cfg_class_dict["PduR"]["ArPackages"]["ActiveEcuC"]["PduR"]
        PduRRoutingPaths, PduRRoutingPaths_Name, valid = pdur_validate_get_routing_path(ecuc_pdur_dict)
        if valid is False:
            service_id_path = "/ActiveEcuC/PduR/"
            self.vlog(self.validation_code, self.validation_summary, f"Cannot Find Container PduRRoutingPaths!",
                      service_id_path, "fail")
            return False
        IfDestList = list()
        for routingPathName, routingPath in PduRRoutingPaths.items():
            valid = True
            if "DefinitionRef" not in routingPath or \
                routingPath["DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath":
                continue
            if "PduRSrcPduRRef" not in routingPath:
                continue
            SrcPdu = routingPath['PduRSrcPduRRef']['ValueRef']
            if SrcPdu is None: # SrcPdu为空
                continue # 不在这里校验
            if "PduRDestPduRRef" not in routingPath:
                continue # 不在这里校验
            DestPduList = []
            if isinstance(routingPath['PduRDestPduRRef']['ValueRef'], list): # 确保全部封装为List，减少后面判断的逻辑
                DestPduList = routingPath['PduRDestPduRRef']['ValueRef']
            else:
                if routingPath['PduRDestPduRRef']['ValueRef'] is not None:
                    DestPduList.append(routingPath['PduRDestPduRRef']['ValueRef'])
            if len(DestPduList) == 0: # DestPduList为空
                continue

            for valueRef in DestPduList:
                dst_pdu_name = valueRef.split('/')[-1]
                for destPduName, routingPathItem in PduRRoutingPaths.items():
                    if destPduName != dst_pdu_name:
                        continue
                    pdu_ref = routingPathItem['PduRDestPduRef']['ValueRef']
                    isDestTp = extract_module_from_ecuc_pdu_ref(pdu_ref, "Dest")
                    if isDestTp == True:
                        continue
                    for item in IfDestList:
                        if dst_pdu_name == item['destpduname']:
                            path1 = '/ActiveEcuC/PduR/' + PduRRoutingPaths_Name + '/' + routingPathName
                            path2 = '/ActiveEcuC/PduR/' + PduRRoutingPaths_Name + '/' + item['routingpath']
                            self.vlog(
                                self.validation_code, self.validation_summary,
                                f"IF DestPdu is existed in two or more RoutingPath! ANOTHER ROUTINGPATH NAME: {item['routingpath']}", path1, "fail")
                            all_correct = False
                            break
                    else: # 如果没有break，代表没有重复，入队
                        IfDestListTemp = {
                            'destpduname': dst_pdu_name,
                            'routingpath': routingPathName
                        }
                        IfDestList.append(IfDestListTemp) # 重复不入队
                    break # 一个路由路径里面DestPdu不会重复
        return all_correct

    def add_rule_time(self):
        pass

def extract_module_from_ecuc_pdu_ref(valueRef: str, direction: str) -> str:
    ModName = ''
    if direction == 'Src':
        if valueRef.split('_')[1] == 'MasterReq':
            ModName = 'LINTP'
        elif valueRef.split('_')[1] == 'SlaveResp':
            ModName = 'LINTP'
        else:
            valueRef = valueRef.split('/')[-1]
            ModName = valueRef.split('_')[-3]
            ModName = ModName[1:]
            ModName = ModName.upper()
    elif direction == 'Dest':
        if valueRef.split('_')[1] == 'MasterReq':
            ModName = 'LINTP'
        elif valueRef.split('_')[1] == 'SlaveResp':
            ModName = 'LINTP'
        else:
            valueRef = valueRef.split('/')[-1]
            ModName = valueRef.split('_')[-2]
            ModName = ModName
            ModName = ModName.upper()
    if ModName in TPMOD:
        return True
    else:
        return False