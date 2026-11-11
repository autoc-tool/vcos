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

"""
Cfg updater for Dcm module.

Update Dcm cfg info according to imported configuration file, e.g. *.cdd
"""
import os
from typing import Dict
from basic_func_module.autosar_utils.utils import get_definition_ref, set_value, set_value_ref, get_value_ref
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.module_management.arxml_handler.arxml_parse_save import ArxmlParseHandler
from func_module.module_management.arxml_handler.config_comparator import ConfigComparator
from func_module.health_monitor.logger import logger

CUR_DIR = os.path.dirname(__file__)


class DemCfgConverter():

    def __init__(self, base_arxml_path) -> None:
        self.base_arxml_path = base_arxml_path
        pass

    def isOBDdtcincluded(self):
        if self.cdd_info is not None and self.cdd_info.obd_dtcs is not None and len(self.cdd_info.obd_dtcs) > 0:
            return True
        else:
            return False

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

    def add_reference_value(self, reference_def, container_obj, value):
        parent_node = container_obj.REFERENCE_VALUES
        reference_val_obj = AutosarUtil.set_reference(parent_node, reference_def)
        set_value_ref(reference_val_obj, value)

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

    def get_sub_containers(self, parent_container_obj, container_def_name=None):
        """
        Get the containers of a specific class

        Args:
            parent_contaienr_obj: The container in which the function will find target subcontainers
            container_def_name: Name of the specific contaienr class, e.g. DemEventParameter.
                If this parameter is None, function will return all subcontainers of the parent container

        Return：
            List of the target contaienrs
        """

        sub_containers_obj = getattr(parent_container_obj, "SUB_CONTAINERS", None)
        ecuc_container_value_list = getattr(sub_containers_obj, "ECUC_CONTAINER_VALUE", [])
        if not container_def_name:
            return ecuc_container_value_list

        target_ecuc_container_value_list = [
            obj for obj in ecuc_container_value_list if get_definition_ref(obj).split("/")[-1] == container_def_name
        ]

        return target_ecuc_container_value_list


class DemCfgConverterWithCdd(DemCfgConverter):

    def __init__(self,
                 base_arxml_path,
                 db_cdd_dict,
                 all_excel_dict,
                 mod_autosar_dict,
                 ecuc_autosar_dict,
                 system_autosar_dict=None) -> None:
        super().__init__(base_arxml_path)
        self.db_cdd_dict: Dict = db_cdd_dict
        # Now only support import one cdd file
        self.cdd_info = list(self.db_cdd_dict.values())[0]
        self.mod_autosar_dict: Dict = mod_autosar_dict
        self.ecuc_autosar_dict: Dict = ecuc_autosar_dict
        self.system_autosar_dict = system_autosar_dict

        self.dem_event_block_list = []
        self.event_parameter_count = 1
        self.freeze_frame_class_count = 0
        self.ext_data_class_count = 0
        self.enable_condition_count = 0
        self.freeze_frame_recnum = 2  # 冻结帧record数量
        self.dem_nvm_block_list = []
        self.event_block_index = 0
        self.event_block_index_max = 0
        self.obd_dtc_ref_map = {}
        self.IsAgedCountUsed = False
        self.IndicatorStatusBlockSize = 0
        self.PermanentBlockSize = 0
        self.OBDFreezeFrameBlockSize = 0

        if all_excel_dict is not None and "DemCfg.xlsx" in all_excel_dict:
            self.excel_info_dict = all_excel_dict["DemCfg.xlsx"]
        else:
            self.excel_info_dict = None

    def get_dem_cycle_name(self, cycle_str):
        default_operation_list = [
            "Dem_Active", "Dem_Power", "Dem_Ignition", "Dem_Warmup", "Dem_Obd_Cycle", "Dem_Time", "Dem_HVcycle"
        ]
        if cycle_str is None:
            return "Dem_Power"
        for cycle in default_operation_list:
            if cycle.upper() == cycle_str.upper():
                return cycle
        return "Dem_Power"


    def create_dem_dtc(self, parent_obj, dtc_info):
        severity_str_map = {
            None: "DEM_SEVERITY_NO_SEVERITY",
            "noSeverity": "DEM_SEVERITY_NO_SEVERITY",
            "maintenanceOnly": "DEM_SEVERITY_MAINTENANCE_ONLY",
            "checkAtNextHalt": "DEM_SEVERITY_CHECK_AT_NEXT_HALT",
            "checkImmediately": "DEM_SEVERITY_CHECK_IMMEDIATELY"
        }
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDTC")
        dtc_name = f"DTCClass_DTC_0x{dtc_info.value:06X}"
        container_val = AutosarUtil.create_sub_container_value(dtc_name, container_def, parent_obj)

        # Set "DemDtcValue" value
        parameter_def_name = "DemDtcValue"
        self.set_numerical_param_value(parameter_def_name, container_val, str(dtc_info.value))
        # Set "DemDTCFunctionalUnit" value
        parameter_def_name = "DemDTCFunctionalUnit"
        if dtc_info.record_info_dict["dtc_functional_unit_value"] is not None:
            self.set_numerical_param_value(parameter_def_name, container_val,
                                           str(dtc_info.record_info_dict["dtc_functional_unit_value"]))
        # Set "DemDTCSeverity" value
        parameter_def_name = "DemDTCSeverity"
        self.set_textual_param_value(parameter_def_name, container_val,
                                     severity_str_map[dtc_info.record_info_dict["dtc_severity_value"]])
        # Set "DemDTCAttributesRef" value
        dtc_attribute_name = f"DTCAttribute_DTC_0x{dtc_info.value:06X}"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemConfigSet/{dtc_attribute_name}"
        reference_def_name = "DemDTCAttributesRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # Set "DemObdDTCRef" value
        dtc_sae_value = dtc_info.sae_format
        if dtc_sae_value in self.obd_dtc_ref_map.keys():
            obd_dtc_ref_name = self.obd_dtc_ref_map.get(dtc_sae_value, None)
            ref_data_info_path = f"/ActiveEcuC/Dem/DemConfigSet/{obd_dtc_ref_name}"
            reference_def_name = "DemObdDTCRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_dtc_attribute(self, parent_obj, dtc_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDTCAttributes")
        dtc_attribute_name = f"DTCAttribute_DTC_0x{dtc_info.value:06X}"
        container_val = AutosarUtil.create_sub_container_value(dtc_attribute_name, container_def, parent_obj)

        # get aging_cycle_name and aging_cycle_count_threshold
        aging_cycle_name = "Dem_Power"
        aging_cycle_count_threshold = '40'
        if "self_healing_aging" in dtc_info.record_info_dict.keys() and dtc_info.record_info_dict["self_healing_aging"] == "supported":
            if "healing_cycle" in dtc_info.record_info_dict.keys():
                aging_cycle_name = self.get_dem_cycle_name(dtc_info.record_info_dict["healing_cycle"])
            elif "dtc_aging_cycle" in dtc_info.record_info_dict.keys():
                aging_cycle_name = self.get_dem_cycle_name(dtc_info.record_info_dict["dtc_aging_cycle"])
            if "healing_cycles" in dtc_info.record_info_dict.keys() and dtc_info.record_info_dict["healing_cycles"] != None and dtc_info.record_info_dict["healing_cycles"] != '0':
                aging_cycle_count_threshold = dtc_info.record_info_dict["healing_cycles"]
            elif "dtc_aging_counter" in dtc_info.record_info_dict.keys() and dtc_info.record_info_dict["dtc_aging_counter"] != None and dtc_info.record_info_dict["dtc_aging_counter"] != '0':
                aging_cycle_count_threshold = dtc_info.record_info_dict["dtc_aging_counter"]

        # Set "DemAgingCycleRef" value
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{aging_cycle_name}"
        reference_def_name = "DemAgingCycleRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # Set "DemAgingCycleCounterThreshold" value
        parameter_def_name = "DemAgingCycleCounterThreshold"
        self.set_numerical_param_value(parameter_def_name, container_val, str(aging_cycle_count_threshold))

        # Set "DemExtendedDataClassRef" value
        if self.ext_data_class_count >= 1:
            ext_data_class_name = "ExtendedDataClass_0"
            ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{ext_data_class_name}"
            reference_def_name = "DemExtendedDataClassRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # Set "DemFreezeFrameClassRef" value
        if self.freeze_frame_class_count >= 1:
            freeze_frame_class_name = "FreezeFrameClass_0"
            ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{freeze_frame_class_name}"
            reference_def_name = "DemFreezeFrameClassRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # Set "DemFreezeFrameRecNumClassRef" value
        freeze_frame_recnum_name = "DemFreezeFrameRecNumClass_0"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{freeze_frame_recnum_name}"
        reference_def_name = "DemFreezeFrameRecNumClassRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # Set "DemMemoryDestinationRef" value
        memory_destination_name = "DemPrimaryMemory"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/DemEventMemorySet/{memory_destination_name}"
        reference_def_name = "DemMemoryDestinationRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_debounce_counter_based(self, parent_obj, dtc_info):
        container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemDebounceAlgorithmClass/DemDebounceCounterBased")
        debounce_counter_based_name = "DemDebounceCounterBased"
        container_val = AutosarUtil.create_sub_container_value(debounce_counter_based_name, container_def, parent_obj)

        # Set "DemDebounceCounterBasedClassRef" value
        counter_debounce_name = f"CounterBased_0x{dtc_info.value:06X}"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemConfigSet/{counter_debounce_name}"
        reference_def_name = "DemDebounceCounterBasedClassRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_debounce_time_base(self, parent_obj, dtc_info):
        container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemDebounceAlgorithmClass/DemDebounceTimeBase")
        debounce_time_base_name = "DemDebounceTimeBase"
        container_val = AutosarUtil.create_sub_container_value(debounce_time_base_name, container_def, parent_obj)

        # Set "DemDebounceTimeBaseRef" value
        time_debounce_name = f"TimeBase_0x{dtc_info.value:06X}"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemConfigSet/{time_debounce_name}"
        reference_def_name = "DemDebounceTimeBaseRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_debounce_algorithm_class(self, parent_obj, dtc_info):
        container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemDebounceAlgorithmClass")
        debounce_algorithm_class_name = "DemDebounceAlgorithmClass"
        container_val = AutosarUtil.create_sub_container_value(debounce_algorithm_class_name, container_def, parent_obj)

        if dtc_info.record_info_dict["dtc_debouncing_algorithm"] == "Counter based":
            self.create_dem_debounce_counter_based(container_val, dtc_info)
        if dtc_info.record_info_dict["dtc_debouncing_algorithm"] == "Time based":
            self.create_dem_debounce_time_base(container_val, dtc_info)

    def create_dem_event_parameter(self, parent_obj, dtc_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter")
        event_parameter_name = f"DTC_0x{dtc_info.value:06x}"
        container_val = AutosarUtil.create_sub_container_value(event_parameter_name, container_def, parent_obj)
        if self.event_block_index < self.event_block_index_max:
            self.dem_event_block_list.append({"shortname": "NvMBlockDescriptor_Dem_PriMemEventBlock" + str(self.event_block_index),\
                                            "NvMBlockJobPriority": "127", "NvMNvBlockLength": str(self.event_block_length), \
                                            "NvMRamBlockDataAddress": "dem_get_pri_mem_idx" + str(self.event_block_index) + "_addr()"})
            self.event_block_index += 1

        # Set "DemEventId" value
        parameter_def_name = "DemEventId"
        self.set_numerical_param_value(parameter_def_name, container_val, str(self.event_parameter_count))
        self.event_parameter_count += 1
        # Set "DemDTCRef" value
        dtc_name = f"DTCClass_DTC_0x{dtc_info.value:06X}"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemConfigSet/{dtc_name}"
        reference_def_name = "DemDTCRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)
        # Set "DemEnableConditionGroupRef" value
        if dtc_info.enable_condition_group_index is not None:
            enable_condition_group_name = f"EnableConditionGroup_{dtc_info.enable_condition_group_index + 1}"
            ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{enable_condition_group_name}"
            reference_def_name = "DemEnableConditionGroupRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)
        # Set "DemOperationCycleRef" value
        opcycle_name = "Dem_Power"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{opcycle_name}"
        reference_def_name = "DemOperationCycleRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        self.create_dem_debounce_algorithm_class(container_val, dtc_info)

    def create_dem_debounce_counter_based_class(self, parent_obj, dtc_info):
        container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDebounceCounterBasedClass")
        counter_debounce_name = f"CounterBased_0x{dtc_info.value:06X}"
        container_val = AutosarUtil.create_sub_container_value(counter_debounce_name, container_def, parent_obj)

        record_info_dict: Dict = dtc_info.record_info_dict

        # Set "DemDebounceCounterJumpDown" value
        parameter_def_name = "DemDebounceCounterJumpDown"
        self.set_numerical_param_value(parameter_def_name, container_val,
                                       "true" if record_info_dict["dtc_jump_down_prepassed"] == "Yes" else "false")
        # Set "DemDebounceCounterJumpUp" value
        parameter_def_name = "DemDebounceCounterJumpUp"
        self.set_numerical_param_value(parameter_def_name, container_val,
                                       "true" if record_info_dict["dtc_jump_up_prefailed"] == "Yes" else "false")
        # Set "DemDebounceCounterDecrementStepSize" value
        parameter_def_name = "DemDebounceCounterDecrementStepSize"
        dem_dtc_step_size_prepassed = int(record_info_dict["dtc_step_size_prepassed"])
        if dem_dtc_step_size_prepassed >= 1 and dem_dtc_step_size_prepassed <= 32767:
            self.set_numerical_param_value(parameter_def_name, container_val, str(dem_dtc_step_size_prepassed))
        else:
            self.set_numerical_param_value(parameter_def_name, container_val, str(1))  #default value 1
        # Set "DemDebounceCounterFailedThreshold" value
        parameter_def_name = "DemDebounceCounterFailedThreshold"
        dem_dtc_failed_threshold = record_info_dict.get("dtc_failed_threshold", None)
        if dem_dtc_failed_threshold is not None:
            self.set_numerical_param_value(parameter_def_name, container_val, dem_dtc_failed_threshold)
        else:
            self.set_numerical_param_value(parameter_def_name, container_val, str(127))  #default value 127

        # Set "DemDebounceCounterIncrementStepSize" value
        parameter_def_name = "DemDebounceCounterIncrementStepSize"
        dem_dtc_step_size_prefailed = int(record_info_dict["dtc_step_size_prefailed"])
        if dem_dtc_step_size_prefailed >= 1 and dem_dtc_step_size_prefailed <= 32767:
            self.set_numerical_param_value(parameter_def_name, container_val, str(dem_dtc_step_size_prefailed))
        else:
            self.set_numerical_param_value(parameter_def_name, container_val, str(1))  #default value 1
        # Set "DemDebounceCounterJumpDownValue" value
        parameter_def_name = "DemDebounceCounterJumpDownValue"
        self.set_numerical_param_value(parameter_def_name, container_val, record_info_dict["counter_jump_down_value"])
        # Set "DemDebounceCounterJumpUpValue" value
        parameter_def_name = "DemDebounceCounterJumpUpValue"
        self.set_numerical_param_value(parameter_def_name, container_val, record_info_dict["counter_jump_up_value"])
        # Set "DemDebounceCounterPassedThreshold" value
        parameter_def_name = "DemDebounceCounterPassedThreshold"
        dtc_passed_threshold = record_info_dict.get("dtc_passed_threshold", None)
        if dtc_passed_threshold:
            if int(dtc_passed_threshold) > 0:
                dtc_passed_threshold = "-" + dtc_passed_threshold
            self.set_numerical_param_value(parameter_def_name, container_val, dtc_passed_threshold)
        else:
            self.set_numerical_param_value(parameter_def_name, container_val, str(-128))  #default value -128

    def create_dem_debounce_time_base_class(self, parent_obj, dtc_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDebounceTimeBaseClass")
        counter_debounce_name = f"TimeBase_0x{dtc_info.value:06X}"
        container_val = AutosarUtil.create_sub_container_value(counter_debounce_name, container_def, parent_obj)

        record_info_dict: Dict = dtc_info.record_info_dict

        # Set "DemDebounceTimeFailedThreshold" value
        parameter_def_name = "DemDebounceTimeFailedThreshold"
        self.set_numerical_param_value(parameter_def_name, container_val,
                                       str(int(record_info_dict["dtc_timeout_value_prefailed"]) / 1000.0))
        # Set "DemDebounceTimePassedThreshold" value
        parameter_def_name = "DemDebounceTimePassedThreshold"
        self.set_numerical_param_value(parameter_def_name, container_val,
                                       str(int(record_info_dict["dtc_timeout_value_prepassed"]) / 1000.0))

    def create_dem_freeze_frame_class(self, parent_obj, freeze_frame_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemFreezeFrameClass")
        freeze_frame_class_name = f"FreezeFrameClass_{self.freeze_frame_class_count}"
        self.freeze_frame_class_count += 1
        container_val = AutosarUtil.create_sub_container_value(freeze_frame_class_name, container_def, parent_obj)

        # Set "DemDidClassRef" value
        reference_def_name = "DemDidClassRef"
        did_flag = False
        for did in freeze_frame_info.dids:
            did_flag = True
            did_name = f"DID_0x{did.identifier:04X}"
            ref_did_info_path = f"/ActiveEcuC/Dem/DemGeneral/{did_name}"
            reference_def = self.mod_autosar_dict["Dem"].find(container_def.get_path() + f"/{reference_def_name}")
            self.add_reference_value(reference_def, container_val, ref_did_info_path)
        # Remove the reference item with None value if it has actually did ref
        if did_flag:
            self.remove_reference_value(reference_def_name, container_val, None)

    def create_dem_did_class(self, parent_obj, did_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDidClass")
        did_name = f"DID_0x{did_info.identifier:04X}"
        container_val = AutosarUtil.create_sub_container_value(did_name, container_def, parent_obj)

        # Set "DemDidIdentifier" value
        parameter_def_name = "DemDidIdentifier"
        self.set_numerical_param_value(parameter_def_name, container_val, str(did_info.identifier))

        # Set "DemDidDataElementClassRef" value
        reference_def_name = "DemDidDataElementClassRef"
        data_name = f"DID_0x{did_info.identifier:04X}_{did_info.name}"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{data_name}"
        reference_def = self.mod_autosar_dict["Dem"].find(container_def.get_path() + f"/{reference_def_name}")
        self.add_reference_value(reference_def, container_val, ref_data_info_path)
        self.remove_reference_value(reference_def_name, container_val, None)

        # data_flag = False
        # for data in did_info.datas:
        #     data_flag = True
        #     data_name = f"DID_0x{did_info.identifier:04X}_{data.name}"
        #     ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{data_name}"
        #     reference_def = self.mod_autosar_dict["Dem"].find(container_def.get_path() + f"/{reference_def_name}")
        #     self.add_reference_value(reference_def, container_val, ref_data_info_path)
        # # Remove the reference item with None value if it has actually did ref
        # if data_flag:
        #     self.remove_reference_value(reference_def_name, container_val, None)

    def create_dem_data_elem_class_for_did(self, parent_obj, did_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass")
        data_name = f"DID_0x{did_info.identifier:04X}_{did_info.name}"
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, parent_obj)
        self.data_elem_data_list.append(data_name)

        container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass")
        data_name = "DemExternalCSDataElementClass"
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, container_val)

        parameter_name = "DemDataElementArraySize"
        self.set_numerical_param_value(parameter_name, container_val, str(did_info.length))

    def create_dem_ext_data_class(self, parent_obj, ext_data_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemExtendedDataClass")
        ext_data_class_name = f"ExtendedDataClass_{self.ext_data_class_count}"
        self.ext_data_class_count += 1
        container_val = AutosarUtil.create_sub_container_value(ext_data_class_name, container_def, parent_obj)

        # Set "DemExtendedDataRecordClassRef" value
        ext_data_record_class_name = f"ExtendedDataRecordClass_{ext_data_info.name}"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{ext_data_record_class_name}"
        reference_def_name = "DemExtendedDataRecordClassRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_ext_data_record_class(self, parent_obj, ext_data_info):
        trigger_str_map = {
            "testFailed": "DEM_TRIGGER_ON_TEST_FAILED",
            "fdcThreshold": "DEM_TRIGGER_ON_FDC_THRESHOLD",
            "pending": "DEM_TRIGGER_ON_PENDING",
            "confirmed": "DEM_TRIGGER_ON_CONFIRMED",
            "custom": "Unknow"
        }
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemExtendedDataRecordClass")
        ext_data_class_name = f"ExtendedDataRecordClass_{ext_data_info.name}"
        container_val = AutosarUtil.create_sub_container_value(ext_data_class_name, container_def, parent_obj)

        # Set "DemExtendedDataRecordNumber" value
        parameter_def_name = "DemExtendedDataRecordNumber"
        self.set_numerical_param_value(parameter_def_name, container_val, ext_data_info.record_num)
        # Set "DemExtendedDataRecordTrigger" value
        parameter_def_name = "DemExtendedDataRecordTrigger"
        self.set_textual_param_value(parameter_def_name, container_val, trigger_str_map[ext_data_info.trigger])
        # Set "DemExtendedDataRecordUpdate" value
        parameter_def_name = "DemExtendedDataRecordUpdate"
        update_textual = "DEM_UPDATE_RECORD_NO" if ext_data_info.update == "0" else "DEM_UPDATE_RECORD_YES"
        self.set_textual_param_value(parameter_def_name, container_val, update_textual)
        ext_data_flag = False
        for data in ext_data_info.datas:
            ext_data_flag = True
            # Set "DemDataElementClassRef" value
            reference_def_name = "DemDataElementClassRef"
            data_name = data.name
            ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{data_name}"
            reference_def = self.mod_autosar_dict["Dem"].find(container_def.get_path() + f"/{reference_def_name}")
            self.add_reference_value(reference_def, container_val, ref_data_info_path)
        if ext_data_flag is True:
            self.remove_reference_value(reference_def_name, container_val, None)

        # Extended data record defaultlly has one data element
        # else:
        #     # Set "DemDataElementClassRef" value
        #     data_elem_class_name = f"DataElement_{ext_data_info.name}"
        #     ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{data_elem_class_name}"
        #     reference_def_name = "DemDataElementClassRef"
        #     self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_data_elem_class_for_ext_data(self, parent_obj, ext_data_info, data_info=None):
        internal_data_elem_map = {
            "Aged_counter": "DEM_AGED_COUNTER",
            "occurrence_counter": "DEM_OCCCTR",
            "Ageing_counter": "DEM_AGINGCTR_UPCNT",
            "Aging_counter": "DEM_AGINGCTR_UPCNT",
            "Fault_pending_counter": "DEM_FAULT_PENDING_COUNTER",
        }

        if data_info:
            container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass")
            # 去除重复data_elem，冻结帧的也一样
            data_name = data_info.name
            if data_name in self.data_elem_data_list:
                return
            container_val = AutosarUtil.create_sub_container_value(data_name, container_def, parent_obj)
            self.data_elem_data_list.append(data_name)

            container_def = self.mod_autosar_dict["Dem"].find(
                "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemInternalDataElementClass")
            data_name = "DemInternalDataElementClass"
            container_val = AutosarUtil.create_sub_container_value(data_name, container_def, container_val)

            # Set "DemDataElementDataSize" value
            parameter_name = "DemDataElementDataSize"
            self.set_numerical_param_value(parameter_name, container_val, str(1))

            # Set "DemInternalDataElement" value
            parameter_name = "DemInternalDataElement"
            if data_info.name in internal_data_elem_map.keys():
                if data_info.name == "Aged_counter":
                    self.IsAgedCountUsed = True
                data_elem_text = internal_data_elem_map[data_info.name]
            else:
                data_elem_text = data_info.name
            self.set_textual_param_value(parameter_name, container_val, data_elem_text)

        # Extended data record defaultlly has one uint8 data element
        # else:
        #     container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass")
        #     data_name = f"DataElement_{ext_data_info.name}"
        #     container_val = AutosarUtil.create_sub_container_value(data_name, container_def, parent_obj)

    def create_default_dem_opcycle(self, parent_obj):
        default_operation_list = [
            "Dem_Active", "Dem_Power", "Dem_Ignition", "Dem_Warmup", "Dem_Obd_Cycle", "Dem_Time", "Dem_HVcycle"
        ]
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemOperationCycle")
        for i, opcycle_name in enumerate(default_operation_list):
            container_val = AutosarUtil.create_sub_container_value(opcycle_name, container_def, parent_obj)
            # Set "DemOperationCycleId" value
            parameter_def_name = "DemOperationCycleId"
            self.set_numerical_param_value(parameter_def_name, container_val, str(i))

    def create_dem_enable_condition(self, parent_obj, enable_condition_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEnableCondition")
        enable_condition_name = enable_condition_info
        container_val = AutosarUtil.create_sub_container_value(enable_condition_name, container_def, parent_obj)

        # Set "DemEnableConditionId" value
        parameter_def_name = "DemEnableConditionId"
        self.set_numerical_param_value(parameter_def_name, container_val, str(self.enable_condition_count))
        self.enable_condition_count += 1

    def create_dem_enable_conditions(self, dem_general_val, enable_conditions):
        enable_condition_list = []
        for enable_condition in enable_conditions:
            if enable_condition.strip() not in enable_condition_list:
                enable_condition_list.append(enable_condition.strip())
        enable_condition_list.sort()
        for enable_condition in enable_condition_list:
            self.create_dem_enable_condition(dem_general_val, enable_condition)

    def create_dem_enable_condition_group(self, parent_obj, enable_condition_group_info):
        index = enable_condition_group_info[0]
        enable_condition_list = enable_condition_group_info[1]
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEnableConditionGroup")
        enable_condition_group_name = f"EnableConditionGroup_{index}"
        container_val = AutosarUtil.create_sub_container_value(enable_condition_group_name, container_def, parent_obj)

        # Set "DemEnableConditionRef" value
        reference_def_name = "DemEnableConditionRef"
        enable_condition_group_flag = False
        for enable_condition in enable_condition_list:
            enable_condition_group_flag = True
            enable_condition_name = enable_condition.strip()
            ref_enable_condition_info_path = f"/ActiveEcuC/Dem/DemGeneral/{enable_condition_name}"
            reference_def = self.mod_autosar_dict["Dem"].find(container_def.get_path() + f"/{reference_def_name}")
            self.add_reference_value(reference_def, container_val, ref_enable_condition_info_path)
        # Remove the reference item with None value if it has actually enable condition ref
        if enable_condition_group_flag:
            self.remove_reference_value(reference_def_name, container_val, None)

    def create_dem_primary_memory(self, parent_obj):
        # Update DemEventMemorySet with DemPrimaryMemory
        primemory_container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemPrimaryMemory")
        primemory_name = "DemPrimaryMemory"
        primemory_container_val = AutosarUtil.create_sub_container_value(primemory_name, primemory_container_def,
                                                                         parent_obj)

        if self.excel_info_dict is not None:
            # Set "DemMaxNumberEventEntryPrimary" value
            parameter_def_name = "DemMaxNumberEventEntryPrimary"
            if self.excel_info_dict["DemMaxNumberEventEntryPrimary"] is not None:
                self.set_numerical_param_value(parameter_def_name, primemory_container_val,
                                               str(self.excel_info_dict["DemMaxNumberEventEntryPrimary"]))

            # Set "DemTypeOfFreezeFrameRecordNumeration" value
            parameter_def_name = "DemTypeOfFreezeFrameRecordNumeration"
            self.set_numerical_param_value(parameter_def_name, primemory_container_val,
                                           str(self.excel_info_dict["DemTypeOfFreezeFrameRecordNumeration"]))
        else:
            parameter_def_name = "DemMaxNumberEventEntryPrimary"
            self.set_numerical_param_value(parameter_def_name, primemory_container_val, str(255))  # default value

    def create_dem_user_defined_memory_class(self, parent_obj):
        if self.excel_info_dict is not None and self.excel_info_dict["DemMaxNumberEventEntryUserDefined"] > 0:
            # Update DemEventMemorySet with DemUserDefinedMemory
            secondary_memory_container_def = self.mod_autosar_dict["Dem"].find(
                "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemUserDefinedMemory")
            secondary_memory_name = "DemUserDefinedMemory"
            secondary_memory_container_val = AutosarUtil.create_sub_container_value(secondary_memory_name,
                                                                                    secondary_memory_container_def,
                                                                                    parent_obj)

            # Set "DemMaxNumberEventEntryUserDefined" value
            parameter_def_name = "DemMaxNumberEventEntryUserDefined"
            self.set_numerical_param_value(parameter_def_name, secondary_memory_container_val,
                                           str(self.excel_info_dict["DemMaxNumberEventEntryUserDefined"]))

            # Set "DemTypeOfFreezeFrameRecordNumeration" value
            parameter_def_name = "DemTypeOfFreezeFrameRecordNumeration"
            self.set_numerical_param_value(parameter_def_name, secondary_memory_container_val,
                                           str(self.excel_info_dict["DemTypeOfFreezeFrameRecordNumeration"]))

            # Set "DemUserDefinedMemoryIdentifier" value
            parameter_def_name = "DemUserDefinedMemoryIdentifier"
            self.set_numerical_param_value(parameter_def_name, secondary_memory_container_val, str(0))

    def create_dem_event_memory_set(self, parent_obj):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet")
        event_memory_set_name = "DemEventMemorySet"
        container_val = AutosarUtil.create_sub_container_value(event_memory_set_name, container_def, parent_obj)

        # Set "DemDtcStatusAvailabilityMask" value
        parameter_def_name = "DemDtcStatusAvailabilityMask"
        self.set_numerical_param_value(parameter_def_name, container_val, str(0x09))

        # Set "DemMaxNumberEventEntryPermanent" value
        if self.isOBDdtcincluded():
            parameter_def_name = "DemMaxNumberEventEntryPermanent"
            self.set_numerical_param_value(parameter_def_name, container_val, "4")

        self.create_dem_primary_memory(container_val)
        self.create_dem_user_defined_memory_class(container_val)

    def create_dem_client(self, parent_obj):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemClient")
        container_name = "DemClient"
        container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

        # Set "DemClientId" value
        parameter_name = "DemClientId"
        self.set_numerical_param_value(parameter_name, container_val, str(0))

        # Set "DemEventMemorySetRef" value
        event_memory_set_name = "DemEventMemorySet"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{event_memory_set_name}"
        reference_def_name = "DemEventMemorySetRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_freeze_frame_record_class(self, parent_obj):
        for i in range(1, self.freeze_frame_recnum + 1):
            container_def = self.mod_autosar_dict["Dem"].find(
                "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemFreezeFrameRecordClass")
            parameter_name = f"DemFreezeFrameRecordClass_{i - 1}"
            container_val = AutosarUtil.create_sub_container_value(parameter_name, container_def, parent_obj)

            # Set "DemFreezeFrameRecordTrigger" value
            parameter_name = "DemFreezeFrameRecordTrigger"
            self.set_textual_param_value(parameter_name, container_val, "DEM_TRIGGER_ON_TEST_FAILED")

            # Set "DemFreezeFrameRecordUpdate" value
            parameter_name = "DemFreezeFrameRecordUpdate"
            if i == self.freeze_frame_recnum:
                self.set_textual_param_value(parameter_name, container_val, "DEM_UPDATE_RECORD_YES")
            else:
                self.set_textual_param_value(parameter_name, container_val, "DEM_UPDATE_RECORD_NO")

            # Set "DemFreezeFrameRecordNumber" value
            parameter_name = "DemFreezeFrameRecordNumber"
            self.set_numerical_param_value(parameter_name, container_val, str(i))

    def create_dem_freeze_frame_recnum(self, parent_obj):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemFreezeFrameRecNumClass")
        freeze_frame_recnum_name = "DemFreezeFrameRecNumClass_0"
        container_val = AutosarUtil.create_sub_container_value(freeze_frame_recnum_name, container_def,
                                                               parent_obj)

        # Set "DemFreezeFrameRecordClassRef" value
        reference_def_name = "DemFreezeFrameRecordClassRef"
        freeze_frame_record_class_flag = False
        for i in range(1, self.freeze_frame_recnum + 1):
            freeze_frame_record_class_flag = True
            freeze_frame_record_class_name = f"DemFreezeFrameRecordClass_{i - 1}"
            ref_info_path = f"/ActiveEcuC/Dem/DemGeneral/{freeze_frame_record_class_name}"
            reference_def = self.mod_autosar_dict["Dem"].find(container_def.get_path() + f"/{reference_def_name}")
            self.add_reference_value(reference_def, container_val, ref_info_path)

        if freeze_frame_record_class_flag:
            self.remove_reference_value(reference_def_name, container_val, None)

    def clear_sub_container(self, ecuc_container_value_list: list, clear_container_name_list):
        abandonded_container_list = list(
            filter(lambda container: get_definition_ref(container).split("/")[-1] in clear_container_name_list,
                   ecuc_container_value_list))
        for container_value in abandonded_container_list:
            AutosarUtil.remove_container_value(ecuc_container_value_list, container_value)

    def update_dem_general_by_excel(self, container_val):
        if self.excel_info_dict is None:
            return

        # Set "DemBswErrorBufferSize" value
        parameter_def_name = "DemBswErrorBufferSize"
        self.set_numerical_param_value(parameter_def_name, container_val,
                                       str(self.excel_info_dict["DemBswErrorBufferSize"]))

        # Set "DemMaxNumberPrestoredFF" value
        parameter_def_name = "DemMaxNumberPrestoredFF"
        self.set_numerical_param_value(parameter_def_name, container_val,
                                       str(self.excel_info_dict["DemMaxNumberPrestoredFF"]))

    def update_dem_general(self, container_val):
        dem_obd_support_type = "DEM_OBD_NO_OBD_SUPPORT"
        if self.isOBDdtcincluded():
            dem_obd_support_type = "DEM_OBD_PRIMARY_ECU"

            # Set "DemOBDEventMemorySetRef" value
            event_memory_set_name = "DemEventMemorySet"
            ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{event_memory_set_name}"
            reference_def_name = "DemOBDEventMemorySetRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # Set "DemOBDSupport" value
        parameter_def_name = "DemOBDSupport"
        self.set_textual_param_value(parameter_def_name, container_val, dem_obd_support_type)

        self.update_dem_general_by_excel(container_val)

    def create_dem_nvram_block_id(self, parent_obj):
        pri_nvram_block_id_num = 0
        if self.event_block_index_max > (self.event_parameter_count - 1):
            pri_nvram_block_id_num = self.event_parameter_count - 1
        else:
            pri_nvram_block_id_num = self.event_block_index_max

        # if sec_mem_event_entry > (self.event_parameter_count - 1):
        #     sec_nvram_block_id_num = self.event_parameter_count - 1
        # else:
        #     sec_nvram_block_id_num = sec_mem_event_entry

        for i in range(0, pri_nvram_block_id_num):
            container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId")
            container_name = f"NvMConf_NvMBlockDescriptor_Dem_PriMemEventBlock{i}"
            container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

            # Set "DemNvRamBlockIdRef" value
            nvram_block_id_name = f"NvMBlockDescriptor_Dem_PriMemEventBlock{i}"
            ref_data_info_path = f"/ActiveEcuC/NvM/{nvram_block_id_name}"
            reference_def_name = "DemNvRamBlockIdRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # update "DemNvRamBlockId" with EventDataMgmt
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId")
        container_name = "NvMConf_NvMBlockDescriptor_Dem_MemEventMgmt"
        container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

        # Set "DemNvRamBlockIdRef" value
        nvram_block_id_name = "NvMBlockDescriptor_Dem_MemEventMgmt"
        ref_data_info_path = f"/ActiveEcuC/NvM/{nvram_block_id_name}"
        reference_def_name = "DemNvRamBlockIdRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        # update "DemNvRamBlockId" with Status
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId")
        container_name = "NvMConf_NvMBlockDescriptor_Dem_Status"
        container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

        # Set "DemNvRamBlockIdRef" value
        nvram_block_id_name = "NvMBlockDescriptor_Dem_Status"
        ref_data_info_path = f"/ActiveEcuC/NvM/{nvram_block_id_name}"
        reference_def_name = "DemNvRamBlockIdRef"
        self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        if self.IsAgedCountUsed == True:
            # update "DemNvRamBlockId" with AgedCounter
            container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId")
            container_name = "NvMConf_NvMBlockDescriptor_Dem_AgedCounter"
            container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

            # Set "DemNvRamBlockIdRef" value
            nvram_block_id_name = "NvMBlockDescriptor_Dem_AgedCounter"
            ref_data_info_path = f"/ActiveEcuC/NvM/{nvram_block_id_name}"
            reference_def_name = "DemNvRamBlockIdRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

        if self.isOBDdtcincluded():
            # update "DemNvRamBlockId" with PerMemEventBuffer
            container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId")
            container_name = "NvMConf_NvMBlockDescriptor_Dem_PerMemEventBlock"
            container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

            # update "DemNvRamBlockId" with PerMemEventBuffer
            nvram_block_id_name = "NvMBlockDescriptor_Dem_PerMemEventBlock"
            ref_data_info_path = f"/ActiveEcuC/NvM/{nvram_block_id_name}"
            reference_def_name = "DemNvRamBlockIdRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

            # update "DemNvRamBlockId" with OBD_Freeze_Frame
            container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId")
            container_name = "NvMConf_NvMBlockDescriptor_Dem_OBD_FreezeFrame"
            container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

            # update "DemNvRamBlockId" with OBD_Freeze_Frame
            nvram_block_id_name = "NvMBlockDescriptor_Dem_OBD_FreezeFrame"
            ref_data_info_path = f"/ActiveEcuC/NvM/{nvram_block_id_name}"
            reference_def_name = "DemNvRamBlockIdRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

            # update "DemNvRamBlockId" with PerMemEventBuffer
            container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId")
            container_name = "NvMConf_NvMBlockDescriptor_Dem_IndicatorStatusBlock"
            container_val = AutosarUtil.create_sub_container_value(container_name, container_def, parent_obj)

            # update "DemNvRamBlockId" with PerMemEventBuffer
            nvram_block_id_name = "NvMBlockDescriptor_Dem_IndicatorStatusBlock"
            ref_data_info_path = f"/ActiveEcuC/NvM/{nvram_block_id_name}"
            reference_def_name = "DemNvRamBlockIdRef"
            self.set_reference_value(reference_def_name, container_val, ref_data_info_path)

    def create_dem_obd_dtc(self, parent_obj, obd_dtc):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemObdDTC")
        obd_dtc_name = f"DTCClass_OBDDTC_0x{obd_dtc.value:04X}"
        container_val = AutosarUtil.create_sub_container_value(obd_dtc_name, container_def, parent_obj)

        # Set "DemDtcValue" value
        parameter_def_name = "DemDtcValue"
        self.set_numerical_param_value(parameter_def_name, container_val, str(obd_dtc.value))
        dtc_safe_value = obd_dtc.record_info_dict.get("uds_dtc_reference", None)
        if dtc_safe_value:
            self.obd_dtc_ref_map.update({dtc_safe_value: obd_dtc_name})

    def create_dem_pid_class(self, parent_obj, pid_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemPidClass")
        pid_name = f"PID_0x{int(pid_info.identifier):X}"
        container_val = AutosarUtil.create_sub_container_value(pid_name, container_def, parent_obj)

        # Set "DemPidIdentifier" value
        parameter_def_name = "DemPidIdentifier"
        self.set_numerical_param_value(parameter_def_name, container_val, str(pid_info.identifier))

        # Create "DemPidDataElement" subcontainer
        container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemPidClass/DemPidDataElement")
        pid_data_elem_name = "DemPidDataElement"
        container_val = AutosarUtil.create_sub_container_value(pid_data_elem_name, container_def, container_val)

        # Set "DemPidDataElementClassRef" value
        reference_def_name = "DemPidDataElementClassRef"
        data_name = f"PID_0x{int(pid_info.identifier):X}"
        ref_data_info_path = f"/ActiveEcuC/Dem/DemGeneral/{data_name}"
        reference_def = self.mod_autosar_dict["Dem"].find(container_def.get_path() + f"/{reference_def_name}")
        self.add_reference_value(reference_def, container_val, ref_data_info_path)
        self.remove_reference_value(reference_def_name, container_val, None)

    def create_dem_data_elem_class_for_pid(self, parent_obj, pid_info):
        container_def = self.mod_autosar_dict["Dem"].find("/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass")
        data_name = f"PID_0x{int(pid_info.identifier):X}"
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, parent_obj)
        self.data_elem_data_list.append(data_name)

        container_def = self.mod_autosar_dict["Dem"].find(
            "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass")
        data_name = "DemExternalCSDataElementClass"
        container_val = AutosarUtil.create_sub_container_value(data_name, container_def, container_val)

        array_size = 0
        if "2" in pid_info.service_info.keys():
            array_size = int(pid_info.service_info["2"]["data_list"][0].length / 8)

        parameter_name = "DemDataElementArraySize"
        self.set_numerical_param_value(parameter_name, container_val, str(array_size))

    def create_diagnostic_description_data(self):
        new_ddd = ArxmlParseHandler.parse_diagnostics_description_data_arxml(self.base_arxml_path)
        self.data_elem_data_list = []

        # Update DemGeneral
        dem_general_val = new_ddd.find("/InitialEcuC/Dem/DemGeneral")
        self.update_dem_general(dem_general_val)

        for snapshot in self.cdd_info.snapshots:
            self.create_dem_freeze_frame_class(dem_general_val, snapshot)
            for did in snapshot.dids:
                self.create_dem_did_class(dem_general_val, did)
                self.create_dem_data_elem_class_for_did(dem_general_val, did)

        # Update DemGeneral with extended data
        for ext_data in self.cdd_info.extended_datas:
            self.create_dem_ext_data_class(dem_general_val, ext_data)
            self.create_dem_ext_data_record_class(dem_general_val, ext_data)
            if ext_data.datas:
                for data in ext_data.datas:
                    self.create_dem_data_elem_class_for_ext_data(dem_general_val, ext_data, data)
                    # Todo 给ext_data加ref

        # Update DemGeneral with default operation cycles
        self.create_default_dem_opcycle(dem_general_val)
        self.create_dem_enable_conditions(dem_general_val, self.cdd_info.enable_conditions)
        for enable_condition_group in enumerate(self.cdd_info.enable_condition_groups, 1):
            self.create_dem_enable_condition_group(dem_general_val, enable_condition_group)

        # Update DemGeneral with DemEventMemorySet
        self.create_dem_event_memory_set(dem_general_val)

        # Update DemGeneral with DemClient
        self.create_dem_client(dem_general_val)

        # Update DemGeneral with DemFreezeFrameRecordClass
        self.create_dem_freeze_frame_record_class(dem_general_val)

        # Update DemGeneral with DemFreezeFrameRecNumClass
        self.create_dem_freeze_frame_recnum(dem_general_val)

        dem_config_set_val = new_ddd.find("/InitialEcuC/Dem/DemConfigSet")

        if self.excel_info_dict is not None:
            self.event_block_index_max = self.excel_info_dict["DemMaxNumberEventEntryPrimary"]
            self.event_block_length = self.excel_info_dict["DemEventEntryPrimaryBlockLength"]

            if  "DemEventEntryPermanentBlockLength" in self.excel_info_dict:
                self.PermanentBlockSize = self.excel_info_dict["DemEventEntryPermanentBlockLength"]
            else:
                self.PermanentBlockSize = 100

            if  "IndicatorStatusBlockLength" in self.excel_info_dict:
                self.IndicatorStatusBlockSize = self.excel_info_dict["IndicatorStatusBlockLength"]
            else:
                self.IndicatorStatusBlockSize = 100

            if  "DemEventEntryOBDFFBlockLength" in self.excel_info_dict:
                self.OBDFreezeFrameBlockSize = self.excel_info_dict["DemEventEntryOBDFFBlockLength"]
            else:
                self.OBDFreezeFrameBlockSize = 100

        else:
            self.event_block_index_max = 255
            self.event_block_length = 92
            self.IndicatorStatusBlockSize = 100
            self.PermanentBlockSize = 100
            self.OBDFreezeFrameBlockSize = 100

        # Update DemConfigSet with DemPidClass
        for pid_name, pid in self.cdd_info.pids.items():
            if (int(pid.identifier) % 0x20 == 0) or ("2" not in pid.service_info.keys()):
                continue
            pid.name = pid_name
            self.create_dem_pid_class(dem_config_set_val, pid)
            self.create_dem_data_elem_class_for_pid(dem_general_val, pid)

        # Update DemConfigSet with OBD DTC
        for obd_dtc in self.cdd_info.obd_dtcs:
            self.create_dem_obd_dtc(dem_config_set_val, obd_dtc)

        # Update DemConfigSet with UDS DTC
        for dtc in self.cdd_info.dtcs:
            self.create_dem_dtc(dem_config_set_val, dtc)
            self.create_dem_dtc_attribute(dem_config_set_val, dtc)
            self.create_dem_event_parameter(dem_config_set_val, dtc)
            if dtc.record_info_dict["dtc_debouncing_algorithm"] == "Counter based":
                self.create_dem_debounce_counter_based_class(dem_config_set_val, dtc)
            if dtc.record_info_dict["dtc_debouncing_algorithm"] == "Time based":
                self.create_dem_debounce_time_base_class(dem_config_set_val, dtc)

        Dem_Status_block_length = 82 + self.event_block_index * 2
        dem_fix_block_list = [{
            "shortname": "NvMBlockDescriptor_Dem_MemEventMgmt",
            "NvMBlockJobPriority": "127",
            "NvMNvBlockLength": "24",
            "NvMRamBlockDataAddress": "dem_get_mgmt_mem_addr()"
        }, {
            "shortname": "NvMBlockDescriptor_Dem_Status",
            "NvMBlockJobPriority": "127",
            "NvMNvBlockLength": str(Dem_Status_block_length),
            "NvMRamBlockDataAddress": "dem_get_status_mem_addr()",
        }]

        self.dem_nvm_block_list = self.dem_event_block_list + dem_fix_block_list

        if self.IsAgedCountUsed == True:
            self.dem_nvm_block_list.append({
            "shortname": "NvMBlockDescriptor_Dem_AgedCounter",
            "NvMBlockJobPriority": "127",
            "NvMNvBlockLength": "4",
            "NvMRamBlockDataAddress": "dem_get_aged_mem_addr()",
        })

        if self.isOBDdtcincluded():
            dem_obd_block_list = [{
            "shortname": "NvMBlockDescriptor_Dem_PerMemEventBlock",
            "NvMBlockJobPriority": "127",
            "NvMNvBlockLength": str(self.PermanentBlockSize),
            "NvMRamBlockDataAddress": "dem_get_perm_mem_addr()"
            }, {
            "shortname": "NvMBlockDescriptor_Dem_OBD_FreezeFrame",
            "NvMBlockJobPriority": "127",
            "NvMNvBlockLength": str(self.OBDFreezeFrameBlockSize),
            "NvMRamBlockDataAddress": "dem_get_obd_ff_mem_addr()",
            }, {
            "shortname": "NvMBlockDescriptor_Dem_IndicatorStatusBlock",
            "NvMBlockJobPriority": "127",
            "NvMNvBlockLength": str(self.IndicatorStatusBlockSize),
            "NvMRamBlockDataAddress": "dem_get_indicator_mem_addr()"
            }]
            self.dem_nvm_block_list = self.dem_nvm_block_list + dem_obd_block_list

        # Update DemGeneral with DemNvRamBlockId
        self.create_dem_nvram_block_id(dem_general_val)

        return new_ddd

    def update_dem_ddd(self, new_dem_ddd):
        old_ddd = self.system_autosar_dict["DiagnosticsDescriptionData"]
        old_dem_ddd = old_ddd.find("/InitialEcuC/Dem")
        new_dem_ddd.parent_object_ = None

        ecuc_module_cfg_val_list: list = old_dem_ddd.get_parent().ECUC_MODULE_CONFIGURATION_VALUES
        # Delete old dem info
        AutosarUtil.remove_container_value(ecuc_module_cfg_val_list, old_dem_ddd)
        # Add new dem info
        ecuc_module_cfg_val_list.append(new_dem_ddd)

        new_ddd = AutosarUtil.update_autosar(self.system_autosar_dict["DiagnosticsDescriptionData"])
        self.system_autosar_dict["DiagnosticsDescriptionData"] = new_ddd

    def post_rearrange_event_parameter_event_id(self, ecuc_obj):
        dem_event_parameter_list = self.get_sub_containers(ecuc_obj.find("/ActiveEcuC/Dem/DemConfigSet"),
                                                           "DemEventParameter")
        event_id = 1
        for event_param in dem_event_parameter_list:
            if not hasattr(event_param, "PARAMETER_VALUES") or event_param.PARAMETER_VALUES is None:
                logger.error(f"Cannot get PARAMETER_VALUES from container {event_param.get_path()}")
                return

            for param_value in event_param.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                def_ref: str = get_definition_ref(param_value)
                if def_ref.endswith("/DemEventId"):
                    set_value(param_value, str(event_id))
                    event_id += 1

    def post_rearrange_enable_condition_id(self, ecuc_obj):
        dem_enable_condition_list = self.get_sub_containers(ecuc_obj.find("/ActiveEcuC/Dem/DemGeneral"),
                                                            "DemEnableCondition")
        enable_condition_id = 0
        for event_param in dem_enable_condition_list:
            if not hasattr(event_param, "PARAMETER_VALUES") or event_param.PARAMETER_VALUES is None:
                logger.error(f"Cannot get PARAMETER_VALUES from container {event_param.get_path()}")
                return

            for param_value in event_param.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                def_ref: str = get_definition_ref(param_value)
                if def_ref.endswith("/DemEnableConditionId"):
                    set_value(param_value, str(enable_condition_id))
                    enable_condition_id += 1

    def post_update_ecuc(self, ecuc_obj):
        # Rearrange DemEventId of DemEventParameter
        self.post_rearrange_event_parameter_event_id(ecuc_obj)
        # Rearrange DemEnableConditionId of DemEnableCond
        self.post_rearrange_enable_condition_id(ecuc_obj)

    def update_from_cdd(self) -> list:
        # Create new DiagnosticDescriptionData according to new cdd file
        new_ddd = self.create_diagnostic_description_data()
        new_dem_ddd = new_ddd.find("/InitialEcuC/Dem")
        old_dem_ddd = self.system_autosar_dict["DiagnosticsDescriptionData"].find("/InitialEcuC/Dem")

        # Compare the difference between two config
        dem_initial_ecuc_comparator = ConfigComparator(old_dem_ddd, new_dem_ddd)
        dem_initial_ecuc_comparator.config_comparator()
        new_ecuc = dem_initial_ecuc_comparator.update_ecuc_config(self.ecuc_autosar_dict["Dem"])

        self.post_update_ecuc(new_ecuc)

        self.ecuc_autosar_dict["Dem"] = new_ecuc
        self.update_dem_ddd(new_dem_ddd)
        return self.dem_nvm_block_list

    def legacy_update_from_cdd(self):
        dem_ecuc_obj = self.ecuc_autosar_dict["Dem"]

        dem_config_set_val = dem_ecuc_obj.find("/ActiveEcuC/Dem/DemConfigSet")

        # Clear generated DmConfigSet info imported from last cdd file
        clear_container_name_list = [
            "DemDTC", "DemDTCAttributes", "DemEventParameter", "DemDebounceTimeBaseClass",
            "DemDebounceCounterBasedClass"
        ]
        self.clear_sub_container(dem_config_set_val.SUB_CONTAINERS.ECUC_CONTAINER_VALUE, clear_container_name_list)

        # Update DemConfigSet
        for dtc in self.cdd_info.dtcs:
            self.create_dem_dtc(dem_config_set_val, dtc)
            self.create_dem_dtc_attribute(dem_config_set_val, dtc)
            self.create_dem_event_parcreate_dem_dtc_attributeameter(dem_config_set_val, dtc)
            if dtc.record_info_dict["dtc_debouncing_algorithm"] == "Counter based":
                self.create_dem_debounce_counter_based_class(dem_config_set_val, dtc)
            if dtc.record_info_dict["dtc_debouncing_algorithm"] == "Time based":
                self.create_dem_debounce_time_base_class(dem_config_set_val, dtc)

        # Update DemGeneral
        dem_general_val = dem_ecuc_obj.find("/ActiveEcuC/Dem/DemGeneral")

        # Clear generated DemGeneral info imported from last cdd file
        clear_container_name_list = [
            "DemFreezeFrameClass", "DemDidClass", "DemDataElementClass", "DemExtendedDataClass",
            "DemExtendedDataRecordClass", "DemOperationCycle", "DemEnableCondition", "DemEnableConditionGroup"
        ]
        self.clear_sub_container(dem_general_val.SUB_CONTAINERS.ECUC_CONTAINER_VALUE, clear_container_name_list)

        for snapshot in self.cdd_info.snapshots:
            self.create_dem_freeze_frame_class(dem_general_val, snapshot)
            for did in snapshot.dids:
                self.create_dem_did_class(dem_general_val, did)
                self.create_dem_data_elem_class_for_did(dem_general_val, did)

        # Update DemGeneral with extended data
        for ext_data in self.cdd_info.extended_datas:
            # Now only support one extended data class with one extended data record
            self.create_dem_ext_data_class(dem_general_val, ext_data)
            self.create_dem_ext_data_record_class(dem_general_val, ext_data)
            if ext_data.datas:
                for data in ext_data.datas:
                    pass
            else:
                self.create_dem_data_elem_class_for_ext_data(dem_general_val, ext_data)
        # Update DemGeneral with default operation cycles
        self.create_default_dem_opcycle(dem_general_val)
        self.create_dem_enable_conditions(dem_general_val, self.cdd_info.enable_conditions)
        for enable_condition_group in enumerate(self.cdd_info.enable_condition_groups, 1):
            self.create_dem_enable_condition_group(dem_general_val, enable_condition_group)
