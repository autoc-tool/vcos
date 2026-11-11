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

ModuleConfig = {
    'CanIf': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'LinIf': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        True,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'CanTp': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'LinTp': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'DoIP': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'CanNm': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        True,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'Com': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            False,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        True,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            True,
        'PduRUseTag':                 False
    },
    'Dcm': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            False,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            True,
        'PduRUseTag':                 False
    }
}

class PduRUpgradeV0120():

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
                AutosarUtil.add_annotation_value(reference_parameter, "AutoDerived")

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
        pdurClass = bsw_ecuc_autosar_class["PduR"]
        pdurStruct = pdurClass.find("/ActiveEcuC/PduR")
        self.upgrade_module_bsw_modules(pdurStruct)
        return True

    def upgrade_module_bsw_modules(self, pdurStruct):
        for item in pdurStruct.CONTAINERS.ECUC_CONTAINER_VALUE:
            if item.DEFINITION_REF.valueOf_ != '/AUTOSAR/EcucDefs/PduR/PduRBswModules':
                continue
            moduleRef = item.REFERENCE_VALUES.ECUC_REFERENCE_VALUE[0].VALUE_REF.valueOf_
            moduleRName = moduleRef.split('/')[-1]
            for name, value in ModuleConfig.items():
                if moduleRName == name:
                    for configitem in item.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                        configName = configitem.DEFINITION_REF.valueOf_.split('/')[-1]
                        set_value(configitem, str(value[configName]).lower())
        return