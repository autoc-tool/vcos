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

class PduR016006Rule(BasicRule):
    ruleset_name = "PduR016006"
    validation_result = False

    def __init__(self, logs, bsw_ecuc_cfg_class_dict) -> None:
        super().__init__()
        self.logs = logs
        self.validation_code = self.__class__.ruleset_name
        self.validation_summary = "SrcPdu and DestPdu should only be IF type or TP type in one RoutingPath!"
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

        for name, routingPath in PduRRoutingPaths.items():
            valid = True
            error_type = 0
            if "DefinitionRef" not in routingPath or \
                routingPath["DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath":
                continue
            if "PduRSrcPduRRef" not in routingPath:
                valid = False
                all_correct = False
            SrcPdu = routingPath['PduRSrcPduRRef']['ValueRef']
            if valid is True and SrcPdu is None: # SrcPdu为空
                valid = False
                all_correct = False
                error_type = 1
            if valid is True and "PduRDestPduRRef" not in routingPath:
                valid = False
                all_correct = False
            if valid is True:
                DestPduList = []
                if isinstance(routingPath['PduRDestPduRRef']['ValueRef'], list): # 确保全部封装为List，减少后面判断的逻辑
                    DestPduList = routingPath['PduRDestPduRRef']['ValueRef']
                else:
                    if routingPath['PduRDestPduRRef']['ValueRef'] is not None:
                        DestPduList.append(routingPath['PduRDestPduRRef']['ValueRef'])
                if len(DestPduList) == 0: # DestPduList为空
                    valid = False
                    all_correct = False
                    error_type = 2
            if valid is True:
                isSrcCdd = check_is_src_cdd(SrcPdu, PduRRoutingPaths)
                if isSrcCdd is True:
                    continue
                isDestCdd = check_is_dest_cdd(DestPduList, PduRRoutingPaths)
                if isDestCdd is True:
                    continue
                isSrcTp = find_src_pdu_container_by_value_ref(SrcPdu, PduRRoutingPaths)
                isDstTpList = find_dst_pdu_container_by_value_ref(DestPduList, PduRRoutingPaths)
                for isDstTp in isDstTpList:
                    if isDstTp != isSrcTp:
                        valid = False
                        all_correct = False
                        break
            # 2） 开始校验
            # 格式："错误码", "摘要", "错误信息", "错误索引路径" "结果"
            path = '/ActiveEcuC/PduR/' + PduRRoutingPaths_Name + '/' + name
            if valid is True:
                self.vlog(
                    self.validation_code, self.validation_summary,
                    f"Src Pdu Type and Dest Pdu(s) Type are corresponding to each other!", path, "pass")
            elif error_type == 0:
                self.vlog(
                    self.validation_code, self.validation_summary,
                    f"Src Pdu Type and Dest Pdu(s) Type are not corresponding to each other!", path, "fail")
            elif error_type == 1:
                self.vlog(
                    self.validation_code, self.validation_summary,
                    f"Src Pdu is empty!", path, "fail")
            elif error_type == 2:
                self.vlog(
                    self.validation_code, self.validation_summary,
                    f"Dest Pdu is empty!", path, "fail")
            else:
                self.vlog(
                    self.validation_code, self.validation_summary,
                    f"Src Pdu Type and Dest Pdu(s) Type are not corresponding to each other!", path, "fail")
            # 3) 初始化标志位
            valid = True
        return all_correct

    def add_rule_time(self):
        pass
def check_is_src_cdd(valueRef: str, routingpath: dict):
    src_pdu_name = valueRef.split('/')[-1]
    for name, routingPathItem in routingpath.items():
        if name == src_pdu_name:
            pdu_ref = routingPathItem['PduRSrcPduRef']['ValueRef']
            relStringList = pdu_ref.split('_o') # 需要考虑模块名内含有'_'
            if len(relStringList) != 0:
                relString = relStringList[-1]
                if relString.find("Cdd") != -1:
                    return True
    return False

def check_is_dest_cdd(valueRefList: list, routingpath: dict):
    for valueRef in valueRefList:
        dest_pdu_name = valueRef.split('/')[-1]
        for name, routingPathItem in routingpath.items():
            if name == dest_pdu_name:
                pdu_ref = routingPathItem['PduRDestPduRef']['ValueRef']
                relStringList = pdu_ref.split('_o') # 需要考虑模块名内含有'_'
                if len(relStringList) != 0:
                    relString = relStringList[-1]
                    if relString.find("Cdd") != -1:
                        return True
    return False

def find_src_pdu_container_by_value_ref(valueRef: str, routingpath: dict):
    src_pdu_name = valueRef.split('/')[-1]
    for name, routingPathItem in routingpath.items():
        if name == src_pdu_name:
            pdu_ref = routingPathItem['PduRSrcPduRef']['ValueRef']
            isSrcTp = extract_module_from_ecuc_pdu_ref(pdu_ref, "Src")
    return isSrcTp
def find_dst_pdu_container_by_value_ref(valueRefList: list, routingpath: dict):
    isDestTpList = []
    for valueRef in valueRefList:
        dst_pdu_name = valueRef.split('/')[-1]
        for name, routingPathItem in routingpath.items():
            if name == dst_pdu_name:
                pdu_ref = routingPathItem['PduRDestPduRef']['ValueRef']
                isDestTp = extract_module_from_ecuc_pdu_ref(pdu_ref, "Dest")
                isDestTpList.append(isDestTp)
    return isDestTpList

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