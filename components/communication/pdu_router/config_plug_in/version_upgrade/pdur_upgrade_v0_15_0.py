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
from basic_func_module.autosar_utils.utils import set_value, set_value_ref, set_definition_ref, get_definition_ref, get_value, get_short_name, get_value_ref
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.health_monitor.logger import logger
from basic_func_module.file_handler.file_io_handler import FileIOHandler

class PduRUpgradeV0150():

    def __init__(self):
        pass
    def set_textual_parameter_value(self, parameter_def_name, textual_param_list, value):
        for textual_parameter in textual_param_list:
            if get_definition_ref(textual_parameter).split("/")[-1] == parameter_def_name:
                set_value(textual_parameter, value)
                AutosarUtil.add_annotation_value(textual_parameter, "AutoDerived")

    def set_reference_value(self, reference_def_name, reference_list, value):
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name:
                set_value_ref(reference_parameter, value)

    def add_reference_value(self, reference_def, container_obj, value):
        # reference_def - 在bswmd文件中的定义
        # container_obj - 父节点（本函数直接索引其REFERENCE_VALUE）
        # value - Ref的值
        parent_node = container_obj.REFERENCE_VALUES
        reference_val_obj = AutosarUtil.set_reference(parent_node, reference_def)
        set_value_ref(reference_val_obj, value)

    def remove_reference_value(self, reference_def_name, container_obj, value):
        reference_list = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name and get_value_ref(
                    reference_parameter) == value:
                reference_list.remove(reference_parameter)
                break

    def set_numerical_param_value(self, parameter_def_name, numerical_param_list, value):
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                AutosarUtil.add_annotation_value(numerical_parameter, "AutoDerived")

    def upgrade_module(self, bsw_ecuc_autosar_class, autosar_ib_class, mod_autosar_dict, sys_info_dict) -> bool:
        logger.info(f"{'upgrade PduR module'}")
        if 'PduR' not in sys_info_dict.bsw_cfg: # 没有PduR的ARXML，直接结束
            return True
        pdurArxmlPath = sys_info_dict.bsw_cfg['PduR']
        pdurClass = FileIOHandler.parse_arxml_file(pdurArxmlPath)
        # pdurClass = bsw_ecuc_autosar_class["PduR"]
        ecucClass = bsw_ecuc_autosar_class["EcuC"]
        pdurStruct = pdurClass.find("/ActiveEcuC/PduR")
        ecucStruct = ecucClass.find("/ActiveEcuC/EcuC")
        # 从PduR中提取所有DestPdu和SrcPdu内容的核信息配置
        PduAndCoreList = list()
        for pdurContainer in pdurStruct.CONTAINERS.ECUC_CONTAINER_VALUE:
            containerDef = pdurContainer.DEFINITION_REF.valueOf_
            if containerDef is None or containerDef == '':
                continue
            if containerDef != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
                continue
            DestSrcPduSettingList = pdurContainer.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
            for item in DestSrcPduSettingList:
                DestSrcPduSettingDef = item.DEFINITION_REF.valueOf_
                if DestSrcPduSettingDef is None or DestSrcPduSettingDef == '':
                    continue
                if DestSrcPduSettingDef == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu':
                    SrcRefList = item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
                    PduName = ''
                    CoreRef = ''
                    for SrcRef in SrcRefList:
                        if SrcRef.VALUE_REF is None:
                            continue
                        if SrcRef.DEFINITION_REF.valueOf_.split("/")[-1] == 'PduRSrcPduRef':
                            PduName = SrcRef.VALUE_REF.valueOf_
                        if SrcRef.DEFINITION_REF.valueOf_.split("/")[-1] == 'PduRSrcPduCoreRef':
                            CoreRef = SrcRef.VALUE_REF.valueOf_
                    pair = {'PduName': PduName.split('/')[-1], 'CoreRef': CoreRef}
                    PduAndCoreList.append(pair)
                if DestSrcPduSettingDef == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu':
                    DestRefList = item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
                    PduName = ''
                    CoreRef = ''
                    for DestRef in DestRefList:
                        if DestRef.VALUE_REF is None:
                            continue
                        if DestRef.DEFINITION_REF.valueOf_.split('/')[-1] == 'PduRDestPduRef':
                            PduName = DestRef.VALUE_REF.valueOf_
                        if DestRef.DEFINITION_REF.valueOf_.split('/')[-1] == 'PduRDestPduCoreRef':
                            CoreRef = DestRef.VALUE_REF.valueOf_
                    pair = {'PduName': PduName.split('/')[-1], 'CoreRef': CoreRef}
                    PduAndCoreList.append(pair)
        # 把EcuC的核信息配置全部置位默认值Core0
        for ecucContainer in ecucStruct.CONTAINERS.ECUC_CONTAINER_VALUE:
            containerDef = ecucContainer.DEFINITION_REF.valueOf_
            if containerDef is None or containerDef == '':
                continue
            if containerDef != '/AUTOSAR/EcucDefs/EcuC/EcucConfigSet':
                continue
            for ecucConfigSetContainer in ecucContainer.SUB_CONTAINERS.ECUC_CONTAINER_VALUE:
                if ecucConfigSetContainer.DEFINITION_REF.valueOf_ != '/AUTOSAR/EcucDefs/EcuC/EcucConfigSet/EcucPduCollection':
                    continue
                PduList = ecucConfigSetContainer.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
                for PduItem in PduList:
                    if PduItem.DEFINITION_REF.valueOf_ != '/AUTOSAR/EcucDefs/EcuC/EcucConfigSet/EcucPduCollection/Pdu':
                        continue
                    self.set_reference_value('EcucPduDefaultCoreRef', PduItem.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                            "/ActiveEcuC/EcuC/EcucHardware/EcucCoreDefinition0") # 先全部设为默认值
                for PduItem in PduList:
                    for PduCoreInfo in PduAndCoreList:
                        if PduCoreInfo['CoreRef'] is None or PduCoreInfo['CoreRef'] == '':
                            continue
                        if PduItem.SHORT_NAME.valueOf_ == PduCoreInfo['PduName']:
                            self.set_reference_value('EcucPduDefaultCoreRef', PduItem.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                                    "/ActiveEcuC/EcuC/EcucHardware/EcucCoreDefinition" + PduCoreInfo['CoreRef'][-1])
        return True
