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

import os
from copy import deepcopy
from random import randint
from typing import Dict
from basic_func_module.autosar_utils.utils import get_definition_ref, set_value, set_value_ref, get_value_ref, get_short_name, get_value
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.module_management.arxml_handler.arxml_parse_save import ArxmlParseHandler
from func_module.module_management.arxml_handler.config_comparator import ConfigComparator

CUR_DIR = os.path.dirname(__file__)


class NvmCfgConverter():

    def __init__(self, base_arxml_path) -> None:
        self.base_arxml_path = base_arxml_path
        pass

    def set_numerical_param_value(self, parameter_def_name, container_obj, value):
        numerical_param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                return
        # print(f"Cannot find {parameter_def_name}")

    def remove_reference_value(self, reference_def_name, container_obj, value):
        reference_list = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name and get_value_ref(
                    reference_parameter) == value:
                reference_list.remove(reference_parameter)
                break

    def remove_textual_param_value(self, param_def_name, container_obj, value):
        param_list = container_obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
        for param in param_list:
            if get_definition_ref(param).split("/")[-1] == param_def_name and get_value(param) == value:
                param_list.remove(param)
                break

    def remove_numerical_param_value(self, param_def_name, container_obj, value):
        param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for param in param_list:
            if get_definition_ref(param).split("/")[-1] == param_def_name and get_value(param) == value:
                param_list.remove(param)
                break

    def remove_add_info_param_value(self, param_def_name, container_obj, value):
        param_list = container_obj.PARAMETER_VALUES.ECUC_ADD_INFO_PARAM_VALUE
        for param in param_list:
            if get_definition_ref(param).split("/")[-1] == param_def_name and get_value(param) == value:
                param_list.remove(param)
                break

    def add_reference_value(self, reference_def, container_obj, value):
        parent_node = container_obj.REFERENCE_VALUES
        reference_val_obj = AutosarUtil.set_reference(parent_node, reference_def)
        set_value_ref(reference_val_obj, value)

    def add_param_value(self, param_def, container_obj, value):
        parent_node = container_obj.PARAMETER_VALUES
        param_val_obj = AutosarUtil.set_parameter(parent_node, param_def)
        set_value(param_val_obj, value)

    def set_reference_value(self, reference_def_name, container_obj, value):
        reference_list = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name:
                set_value_ref(reference_parameter, value)
                return
        # print(f"Cannot find {reference_def_name}")

    def set_textual_param_value(self, parameter_def_name, container_obj, value):
        textual_param_list = container_obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
        for textual_parameter in textual_param_list:
            if get_definition_ref(textual_parameter).split("/")[-1] == parameter_def_name:
                set_value(textual_parameter, value)
                return
        # print(f"Cannot find {parameter_def_name}")


class NvmCfgConverterwithCdd(NvmCfgConverter):

    def __init__(self,
                 base_arxml_path,
                 db_cdd_dict,
                 mod_autosar_dict,
                 ecuc_autosar_dict,
                 block_list,
                 system_autosar_dict=None) -> None:
        super().__init__(base_arxml_path)
        self.db_cdd_dict: Dict = db_cdd_dict
        # Now only support import one cdd file
        self.cdd_info = list(self.db_cdd_dict.values())[0]
        self.mod_autosar_dict: Dict = mod_autosar_dict
        self.ecuc_autosar_dict: Dict = ecuc_autosar_dict
        self.system_autosar_dict = system_autosar_dict

        # Test Block list
        # Test_list = [
        #     {'shortname': 'T4', 'NvMBlockJobPriority': '1', 'NvMNvBlockLength': '11', 'NvMRamBlockDataAddress': 'a'},
        #     {'shortname': 'T5', 'NvMBlockJobPriority': '2', 'NvMNvBlockLength': '22', 'NvMRamBlockDataAddress': 'b'},
        #     {'shortname': 'T6', 'NvMBlockJobPriority': '3', 'NvMNvBlockLength': '33', 'NvMRamBlockDataAddress': 'c'}
        # ]
        self.block_list = block_list

    def create_nvm_fee_ref(self, parent_obj):
        container_def = self.mod_autosar_dict["NvM"].find(
            "/AUTOSAR/EcucDefs/NvM/NvMBlockDescriptor/NvMTargetBlockReference/NvMFeeRef")
        AutosarUtil.create_sub_container_value("NvMFeeRef", container_def, parent_obj)

    def create_nvm_target_block_reference(self, parent_obj):
        container_def = self.mod_autosar_dict["NvM"].find(
            "/AUTOSAR/EcucDefs/NvM/NvMBlockDescriptor/NvMTargetBlockReference")
        container_val = AutosarUtil.create_sub_container_value("NvMTargetBlockReference", container_def, parent_obj)

        self.create_nvm_fee_ref(container_val)

    def create_nvm_block_descriptor(self, parent_obj, nvm_block_info: Dict):
        container_def = self.mod_autosar_dict["NvM"].find("/AUTOSAR/EcucDefs/NvM/NvMBlockDescriptor")
        container_val = AutosarUtil.create_sub_container_value(
            nvm_block_info.get("short_name", f"NvMBlockDescriptotr_Dem_{randint(1, 99999)}"), container_def, parent_obj)

        for param_name, param_val in nvm_block_info["numerical_param"].items():
            self.set_numerical_param_value(param_name, container_val, param_val)

        for param_name, param_val in nvm_block_info["textual_param"].items():
            self.set_textual_param_value(param_name, container_val, param_val)

    def check_duplicate_container(self, parent_obj, container_name):
        container_list = []
        if parent_obj.get_tag() == "ECUC-MODULE-CONFIGURATION-VALUES":
            container_list = parent_obj.CONTAINERS.ECUC_CONTAINER_VALUE
        elif parent_obj.get_tag() == "ECUC-CONTAINER-VALUE":
            container_list = parent_obj.SUB_CONTAINERS.ECUC_CONTAINER_VALUE

        for container in container_list:
            if get_short_name(container) == container_name:
                return True

        return False

    def create_diagnostic_description_data(self):
        new_ddd = ArxmlParseHandler.parse_diagnostics_description_data_arxml(self.base_arxml_path)

        # Create NvM config here
        new_nvm_ddd = new_ddd.find("/InitialEcuC/NvM")
        self.legacy_update_from_cdd(new_nvm_ddd, self.block_list)
        return new_ddd

    def update_nvm_ddd(self, new_nvm_ddd):
        old_ddd = self.system_autosar_dict["DiagnosticsDescriptionData"]
        old_nvm_ddd = old_ddd.find("/InitialEcuC/NvM")
        new_nvm_ddd.parent_object_ = None

        # Delete old dem info
        if old_nvm_ddd:
            ecuc_module_cfg_val_list: list = old_nvm_ddd.get_parent().ECUC_MODULE_CONFIGURATION_VALUES
            AutosarUtil.remove_container_value(ecuc_module_cfg_val_list, old_nvm_ddd)
        else:
            ecuc_module_cfg_val_list: list = old_ddd.find("/InitialEcuC").ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES

        # Add new dem info
        ecuc_module_cfg_val_list.append(new_nvm_ddd)

        new_ddd = AutosarUtil.update_autosar(self.system_autosar_dict["DiagnosticsDescriptionData"])
        self.system_autosar_dict["DiagnosticsDescriptionData"] = new_ddd

    def update_from_cdd(self):
        # Create new DiagnosticDescriptionData according to new cdd file
        new_ddd = self.create_diagnostic_description_data()
        new_nvm_ddd = new_ddd.find("/InitialEcuC/NvM")
        old_nvm_ddd = self.system_autosar_dict["DiagnosticsDescriptionData"].find("/InitialEcuC/NvM")
        if not old_nvm_ddd:
            old_nvm_ddd = ArxmlParseHandler.parse_diagnostics_description_data_arxml(
                self.base_arxml_path).find("/InitialEcuC/NvM")

        # Compare the difference between two config
        nvm_initial_ecuc_comparator = ConfigComparator(old_nvm_ddd, new_nvm_ddd)
        nvm_initial_ecuc_comparator.config_comparator()
        new_ecuc = nvm_initial_ecuc_comparator.update_ecuc_config(self.ecuc_autosar_dict["NvM"])

        self.ecuc_autosar_dict["NvM"] = new_ecuc
        self.update_nvm_ddd(new_nvm_ddd)

    def legacy_update_from_cdd(self, new_nvm_ddd, block_list):
        """
        Update nvm cfg when importing cdd file content

        1. Update nvm cfg when importing cdd file content

        """
        # Create new DiagnosticsDescriptionData according to new cdd file

        nvm_block_info_dict = {
            "numerical_param": {
                "NvMBlockUseAutoValidation": "false",
                "NvMBlockUseCrc": "true",
                "NvMBlockUseCRCCompMechanism": "false",
                "NvMBlockUseSetRamBlockStatus": "true",
                "NvMBlockUseSyncMechanism": "false",
                "NvMBlockWriteProt": "false",
                "NvMBswMBlockStatusInformation": "false",
                "NvMCalcRamBlockCrc": "true",
                "NvMResistantToChangedSw": "true",
                "NvMSelectBlockForFirstInitAll": "false",
                "NvMSelectBlockForReadAll": "true",
                "NvMSelectBlockForWriteAll": "true",
                "NvMStaticBlockIDCheck": "false",
                "NvMWriteBlockOnce": "false",
                "NvMWriteVerification": "false",
                "NvMBlockJobPriority": "127",
                "NvMMaxNumOfReadRetries": "0",
                "NvMMaxNumOfWriteRetries": "0",
                "NvMNvBlockBaseNumber": "1",
                "NvMNvBlockLength": "1",
                "NvMNvBlockNum": "1",
                "NvMNvramBlockIdentifier": "1",
                "NvMNvramDeviceId": "0",
                "NvMRomBlockNum": "0",
                "NvMWriteVerificationDataSize": "1"
            },
            "textual_param": {
                "NvMBlockCrcType": "NVM_CRC16",
                "NvMBlockManagementType": "NVM_BLOCK_NATIVE",
                "NvMInitBlockCallback": "NULL",
                "NvMReadRamBlockFromNvCallback": "NULL",
                "NvMSingleBlockCallback": "NULL",
                "NvMWriteRamBlockToNvCallback": "NULL",
                "NvMBlockHeaderInclude": "NULL",
                "NvMRamBlockDataAddress": "NULL",
                "NvMRomBlockDataAddress": "NULL"
            }
        }

        container_obj = new_nvm_ddd.find("/InitialEcuC/NvM/NvMCommonVendorParams")
        param_def = self.mod_autosar_dict["NvM"].find(
            "/AUTOSAR/EcucDefs/NvM/NvMCommonVendorParams/NvMCfgCbkIncludeList")
        self.remove_textual_param_value(param_def, container_obj, "dem.h")
        self.add_param_value(param_def, container_obj, "dem.h")

        for block_dict in block_list:
            nvm_block_info_dict["short_name"] = block_dict['shortname']
            nvm_block_info_dict["textual_param"]["NvMRamBlockDataAddress"] = block_dict['NvMRamBlockDataAddress']
            nvm_block_info_dict["numerical_param"]["NvMBlockJobPriority"] = block_dict['NvMBlockJobPriority']
            nvm_block_info_dict["numerical_param"]["NvMNvBlockLength"] = block_dict['NvMNvBlockLength']
            self.create_nvm_block_descriptor(new_nvm_ddd, nvm_block_info_dict)
