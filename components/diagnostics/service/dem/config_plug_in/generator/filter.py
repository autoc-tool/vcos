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
整理Dem模块配置信息模块
"""
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from operator import itemgetter

def is_valid_seq(list, start=0):
    for i in range(len(list)):
        if list[i] != start + i:
            return False
    return True

    # size = len(list)
    # list_flag = [0] * size

    # for i in range(size):
    #     index = list[i] - start
    #     if (index < 0) or (index >= size) or (list_flag[index] == 1):
    #         return False
    #     list_flag[index] = 1
    # return True

def get_dem_cfg_set_short_name(ref_path):
    def_ref = "/AUTOSAR/EcucDefs/Dem/DemConfigSet"
    DemConfigSets = ref_path.get(def_ref, [])
    for DemConfigSet in DemConfigSets:
        return DemConfigSet
    return None

def get_dem_general_short_name(ref_path):
    def_ref = "/AUTOSAR/EcucDefs/Dem/DemGeneral"
    DemGenerals = ref_path.get(def_ref, [])
    for DemGeneral in DemGenerals:
        return DemGeneral
    return None

def dem_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for value in input_dict.values():
        if isinstance(value, dict):
            if "DefinitionRef" in value.keys():
                if value["DefinitionRef"].split("/")[-1] == definition_ref:
                    return value
    return {}

def dem_check_if_config_is_not_none(config):
    if config is not None and config != "" and config != "<empty>":
        return True
    else:
        return False


def get_template_dynamic_code_dict(root_dict: dict) -> dict:
    dem_general = process_dem_general(root_dict)
    dem_config_set = process_dem_config_set(root_dict["bsw"]["dem"], dem_general)
    input_dict = {"DemGeneral": dem_general, "DemConfigSet": dem_config_set}

    set_dem_event_config(root_dict["bsw"], dem_general, dem_config_set)
    process_dem_general_bsw_buffer_size(root_dict["bsw"], dem_general, dem_config_set)
    return input_dict

def process_dem_general_bsw_buffer_size(config_dict: dict, dem_general: dict, dem_config_set: dict):
    bsw_buffer_size = 0
    dem_general_in_json = dem_get_dict_by_definition_ref(config_dict["dem"]["ArPackages"]["ActiveEcuC"]["Dem"], "DemGeneral")
    dem_event_parameters = dem_config_set["DemEventParameters"]

    if "DemBswErrorBufferSize" in dem_general_in_json.keys():
        bsw_buffer_size = dem_general_in_json["DemBswErrorBufferSize"]["Value"]
    else:
        for _, event_parameter in dem_event_parameters.items():
            if "ReportBehavior" not in event_parameter.keys(
            ) or event_parameter["ReportBehavior"] == "REPORT_BEFORE_INIT":
                bsw_buffer_size += 2
    dem_general["DemBswErrorBufferSize"] = bsw_buffer_size


# process containers in DemConfigSet
def process_dem_config_set(dem_config_dict: dict, dem_general: dict) -> dict:
    config_set_in_json = dem_get_dict_by_definition_ref(dem_config_dict["ArPackages"]["ActiveEcuC"]["Dem"], "DemConfigSet")
    dem_time_based_debounces = process_dem_time_based_debounces(config_set_in_json)
    dem_counter_based_debounces = process_dem_counter_based_debounces(config_set_in_json)
    dem_debounce_classes = process_dem_debouce_classes(dem_config_dict, config_set_in_json, dem_time_based_debounces,
                                                       dem_counter_based_debounces)
    dem_dtc_classes, dem_immediate_nv_storage = process_dem_config_set_dtc_classes(dem_config_dict, config_set_in_json)
    dtc_attributes = process_dem_config_set_dtc_attributes(config_set_in_json)
    dem_event_classes = process_dem_config_set_event_classes(dem_config_dict, config_set_in_json, dem_general,
                                                             dem_debounce_classes)
    dem_event_parameters = process_dem_config_set_event_parameter(dem_config_dict, config_set_in_json, dem_dtc_classes,
                                                                  dem_event_classes, dem_general)
    dem_event_combined_groups = process_dem_config_set_event_combined_groups(dem_general, dem_event_parameters, dem_dtc_classes)
    dem_indicator_attribute_groups = process_dem_indicator_attribute_groups_config(
        dem_event_classes, dem_general["DemIndicatorAttributes"])
    process_dem_general_indicator_event_groups_config(dem_event_classes, dem_general["DemIndicatorAttributes"],
                                                      dem_general["DemIndicators"], dem_general)
    dem_dtc_group = process_dem_config_set_dtc_group(dem_general["DemEventMemorySet"])
    dem_clear_dtc_notification = process_dem_config_set_clear_dtc_notification(dem_general["DemEventMemorySet"])
    set_dem_general_max_number_event_entry(dem_general, dem_event_classes)

    ConfigSet = {
        "DemDTCClasses": dem_dtc_classes,
        "DemEventParameters": dem_event_parameters,
        "DemEventClasses": dem_event_classes,
        "DemIndicatorAttributeGroups": dem_indicator_attribute_groups,
        "DemTimeBasedDebouces": dem_time_based_debounces,
        "DemCounterBasedDebounces": dem_counter_based_debounces,
        "DemDebounceClasses": dem_debounce_classes,
        "DemDTCGroup": dem_dtc_group,
        "DemClearDTCNotification": dem_clear_dtc_notification,
        "DemEventCombinedGroups": dem_event_combined_groups,
    }
    dem_general["DemImmediateNvStorage"] = dem_immediate_nv_storage
    return ConfigSet


# DemConfigSet process functions
def process_dem_debouce_classes(dem_config_dict: dict, config_set_dict: dict, dem_time_based_debounces: dict,
                                dem_counter_based_debounces: dict) -> dict:
    dem_debounce_classes = {}
    dem_debounce_class_index = -1

    for debounce_name, debounce in config_set_dict.items():
        if "DefinitionRef" not in debounce or debounce[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDebounceCounterBasedClass":
            continue
        dem_debounce_class_index += 1
        dem_debounce_classes[debounce_name] = {}
        dem_debounce_classes[debounce_name]["PreDebounceName"] = "DEM_PRE_DEBOUNCE_COUNTER_BASED"
        dem_debounce_classes[debounce_name]["PreDebounceAlgorithmIndex"] = dem_counter_based_debounces[debounce_name][
            "DemDebounceCounterIndex"]
        dem_debounce_classes[debounce_name]["DebounceClassIndex"] = dem_debounce_class_index
    for debounce_name, debounce in config_set_dict.items():
        if "DefinitionRef" not in debounce or debounce[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDebounceTimeBaseClass":
            continue
        dem_debounce_class_index += 1
        dem_debounce_classes[debounce_name] = {}
        dem_debounce_classes[debounce_name]["PreDebounceName"] = "DEM_PRE_DEBOUNCE_TIME_BASED"
        dem_debounce_classes[debounce_name]["PreDebounceAlgorithmIndex"] = dem_time_based_debounces[debounce_name][
            "DemDebounceTimeIndex"]
        dem_debounce_classes[debounce_name]["DebounceClassIndex"] = dem_debounce_class_index

    return dem_debounce_classes


def process_dem_time_based_debounces(config_set_dict: dict) -> dict:
    dem_time_based_debounces = {}
    dem_time_base_debounce_index = -1

    for debounce_name, debounce in config_set_dict.items():
        if "DefinitionRef" not in debounce or debounce[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDebounceTimeBaseClass":
            continue
        dem_time_based_debounces[debounce_name] = debounce
        dem_time_based_debounces[debounce_name]["DemDebounceTimeFailedThreshold"] = int(
            dem_time_based_debounces[debounce_name]["DemDebounceTimeFailedThreshold"]["Value"] * 1000.0)
        dem_time_based_debounces[debounce_name]["DemDebounceTimePassedThreshold"] = int(
            dem_time_based_debounces[debounce_name]["DemDebounceTimePassedThreshold"]["Value"] * 1000.0)
        dem_time_base_debounce_index += 1
        dem_time_based_debounces[debounce_name]["DemDebounceTimeIndex"] = dem_time_base_debounce_index

    return dem_time_based_debounces


def process_dem_counter_based_debounces(config_set_dict: dict) -> dict:
    dem_counter_based_debounces = {}
    dem_counter_base_debounce_index = -1

    for debounce_name, debounce in config_set_dict.items():
        if "DefinitionRef" not in debounce or debounce[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDebounceCounterBasedClass":
            continue
        dem_counter_based_debounces[debounce_name] = debounce
        if dem_counter_based_debounces[debounce_name]["DemDebounceCounterJumpDown"]["Value"] == True:
            dem_counter_based_debounces[debounce_name]["DemDebounceCounterJumpDown"] = "TRUE"
        else:
            dem_counter_based_debounces[debounce_name]["DemDebounceCounterJumpDown"] = "FALSE"
        if dem_counter_based_debounces[debounce_name]["DemDebounceCounterJumpUp"]["Value"] == True:
            dem_counter_based_debounces[debounce_name]["DemDebounceCounterJumpUp"] = "TRUE"
        else:
            dem_counter_based_debounces[debounce_name]["DemDebounceCounterJumpUp"] = "FALSE"
        dem_counter_base_debounce_index += 1
        dem_counter_based_debounces[debounce_name]["DemDebounceCounterIndex"] = dem_counter_base_debounce_index

    return dem_counter_based_debounces


def process_dem_obd_dtc_classes(dem_config_set: dict, obd_class_ref: str):
    OBDDTC = "DEM_NO_OBDDTC"
    if obd_class_ref is not None:
        for item_name, item in dem_config_set.items():
            if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemObdDTC':
                continue
            searched_name = obd_class_ref.split('/')[-1]
            if item_name == searched_name:
                OBDDTC = item['DemDtcValue']['Value']
    return OBDDTC


def process_dem_config_set_dtc_classes(dem_config_dict: dict, config_set_dict: dict) -> dict:
    dtc_classes = {}
    dem_immediate_nv_storage = "STD_OFF"
    dtc_index = 0

    for dtc_name, dtc in config_set_dict.items():
        if "DefinitionRef" not in dtc or dtc["DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDTC":
            continue
        dem_config_set = dem_get_dict_by_definition_ref(dem_config_dict['ArPackages']['ActiveEcuC']['Dem'], "DemConfigSet")
        dtc_classes[dtc_name] = dtc
        dtc_classes[dtc_name]["DTCIndex"] = dtc_index
        dtc_classes[dtc_name]["OBDDTC"] = process_dem_obd_dtc_classes(dem_config_set, dtc['DemObdDTCRef']['ValueRef'])
        if dtc["DemNvStorageStrategy"]["Value"] == "DURING_SHUTDOWN":
            dtc_classes[dtc_name]["ImmediateNvStorage"] = "FALSE"
        else:
            dtc_classes[dtc_name]["ImmediateNvStorage"] = "TRUE"
            dem_immediate_nv_storage = "STD_ON"
        dtc_index += 1

    return dtc_classes, dem_immediate_nv_storage


def process_dem_config_set_dtc_attributes(config_set_dict: dict) -> dict:
    dtc_attributes = {}
    dtc_attribute_index = 0

    for DTCAttributeName, DTCAttribute in config_set_dict.items():
        if "DefinitionRef" not in DTCAttribute or DTCAttribute[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDTCAttributes":
            continue
        dtc_attributes[DTCAttributeName] = DTCAttribute
        dtc_attributes[DTCAttributeName]["DTCAttributesIndex"] = dtc_attribute_index
        dtc_attribute_index += 1

    return dtc_attributes


def get_dtc_class_ref_index_by_name(dem_config_dict: dict, dtc_class_name: str, dtc_classes_dict: dict):
    dtc_class_ref_index = -1
    isOBDDTC = False
    for dtc_name, dtc in dtc_classes_dict.items():
        isOBDDTC = False
        if dtc_class_name == dtc_name:
            dtc_class_ref_index = dtc["DTCIndex"]
            if dtc['DemObdDTCRef']['ValueRef'] is not None:
                dest_dtc = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                            dtc['DemObdDTCRef']['ValueRef'])
                if dest_dtc is not None:
                    isOBDDTC = True
                break
    return dtc_class_ref_index, isOBDDTC


def get_dtc_attribute_from_event_parameter(dem_config_dict: dict, event_parameter_dict: dict) -> dict:
    dtc_attribute = {}

    if "DefinitionRef" in event_parameter_dict["DemDTCRef"] and event_parameter_dict["DemDTCRef"][
            "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemDTCRef" and \
            event_parameter_dict["DemDTCRef"]["ValueRef"] is not None:
        dest_dtc = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                     event_parameter_dict["DemDTCRef"]["ValueRef"])
        if dest_dtc is not None:
            if "DefinitionRef" in dest_dtc["DemDTCAttributesRef"] and dest_dtc["DemDTCAttributesRef"][
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemDTC/DemDTCAttributesRef":
                dest_dtc_attribute = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                                       dest_dtc["DemDTCAttributesRef"]["ValueRef"])
                if dest_dtc_attribute is not None:
                    dtc_attribute = dest_dtc_attribute

    return dtc_attribute

def process_dem_config_set_event_parameter(dem_config_dict: dict, config_set_dict: dict, dtc_classes_dict: dict,
                                           eventclasses_dict: dict, dem_general: dict) -> dict:
    event_parameters = {}

    ext_data_class_dict = dem_general["DemExtendedDataClass"]
    freeze_frame_class_dict = dem_general["DemFreezeFrameClass"]
    freeze_frame_recnum_class_dict = dem_general["DemFreezeFrameRecNumClass"]

    for event_parameter_name, event_parameter in config_set_dict.items():
        if "DefinitionRef" not in event_parameter or event_parameter[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter":
            continue
        dtc_attribute = {}
        max_number_freeze_frame_records = 0
        event_class_ref_index = -1
        ext_data_class_ref_index = -1
        freeze_frame_class_ref_index = -1
        freeze_frame_recnum_class_ref_index = -1
        DemEventAgedData = -1
        dtc_attribute = get_dtc_attribute_from_event_parameter(dem_config_dict, event_parameter)
        if dtc_attribute is not None and event_parameter["DemDTCRef"]["ValueRef"] is not None:
            dtc_class_ref_index, isOBD = get_dtc_class_ref_index_by_name(
                dem_config_dict, event_parameter["DemDTCRef"]["ValueRef"].split('/')[-1], dtc_classes_dict)
            max_number_freeze_frame_records = dtc_attribute["DemMaxNumberFreezeFrameRecords"]["Value"]
            if "DemExtendedDataClassRef" in dtc_attribute.keys() and dtc_attribute["DemExtendedDataClassRef"][
                "ValueRef"] is not None:
                ext_data_class_ref_name = dtc_attribute["DemExtendedDataClassRef"]["ValueRef"].split("/")[-1]
                if ext_data_class_ref_name in ext_data_class_dict:
                    ext_data_class_ref_index = ext_data_class_dict[ext_data_class_ref_name][
                        "ExtendedDataClassIndex"]
            if "DemFreezeFrameClassRef" in dtc_attribute.keys() and dtc_attribute["DemFreezeFrameClassRef"][
                "ValueRef"] is not None:
                freeze_frame_class_ref_name = dtc_attribute["DemFreezeFrameClassRef"]["ValueRef"].split("/")[-1]
                if freeze_frame_class_ref_name in freeze_frame_class_dict:
                    freeze_frame_class_ref_index = freeze_frame_class_dict[freeze_frame_class_ref_name][
                        "freezeframeClassIndex"]
            if "DemFreezeFrameRecNumClassRef" in dtc_attribute.keys() and dtc_attribute["DemFreezeFrameRecNumClassRef"][
                "ValueRef"] is not None:
                freeze_frame_recnum_class_ref_name = dtc_attribute["DemFreezeFrameRecNumClassRef"][
                    "ValueRef"].split("/")[-1]
                if freeze_frame_recnum_class_ref_name in freeze_frame_recnum_class_dict:
                    freeze_frame_recnum_class_ref_index = freeze_frame_recnum_class_dict[
                        freeze_frame_recnum_class_ref_name]["FreezeFrameRecordNumberClassIndex"]
        if event_parameter_name in eventclasses_dict.keys():
            event_class_ref_index = eventclasses_dict[event_parameter_name]["EventClassIndex"]
            DemEventAgedData = eventclasses_dict[event_parameter_name]["DemEventAgedData"]

        event_dtc_kind = 'DEM_DTC_KIND_ALL_DTCS'
        if isOBD == True:
            event_dtc_kind = 'DEM_EMISSION_REL_DTCS_KIND'
        event_parameters[event_parameter_name] = {
            "DTCClassRefIndex": dtc_class_ref_index,
            "DTCClassName": event_parameter["DemDTCRef"]["ValueRef"].split('/')[-1],
            "EventDTCKind": event_dtc_kind,
            "EventClassRefIndex": event_class_ref_index,
            "EventId": int(event_parameter["DemEventId"]["Value"]),
            "DemEventAgedData": DemEventAgedData,
            "EventKind": event_parameter["DemEventKind"]["Value"],
            "ExtendedDataClassRefIndex": ext_data_class_ref_index,
            "FreezeFrameClassRef": freeze_frame_class_ref_index,
            "FreezeFrameRecNumClassRef": freeze_frame_recnum_class_ref_index,
            "MaxNumberFreezeFrameRecords": max_number_freeze_frame_records,
            "DemOsApplicationRef": event_parameter["DemOsApplicationRef"]["ValueRef"],
            "CallbackInitMforE": "NULL",
            "CallbackEventDataChanged": "NULL",
            "CallbackClearEventAllowed": "NULL",
            "CallbackEventStatusChanged": "NULL",
        }
        if "DemReportBehavior" in event_parameter and dem_check_if_config_is_not_none(event_parameter["DemReportBehavior"]["Value"]):
            event_parameters[event_parameter_name]["ReportBehavior"] = event_parameter["DemReportBehavior"]["Value"]

        for value in event_parameter.values():
            if "DefinitionRef" not in value:
                continue
            path_name = value["DefinitionRef"].split('/')[-1]
            if path_name == "DemCallbackInitMForE":
                if dem_check_if_config_is_not_none(value["DemCallbackInitMForEFnc"]["Value"]):
                    event_parameters[event_parameter_name]["CallbackInitMforE"] = value["DemCallbackInitMForEFnc"]["Value"]
                if event_parameters[event_parameter_name]["CallbackInitMforE"] != "NULL":
                    dem_general["DemCallbackInitMforESupport"] = "STD_ON"
            elif path_name == "DemCallbackEventDataChanged":
                if dem_check_if_config_is_not_none(value["DemCallbackEventDataChangedFnc"]["Value"]):
                    event_parameters[event_parameter_name]["CallbackEventDataChanged"] = value["DemCallbackEventDataChangedFnc"]["Value"]
                else:
                    event_parameters[event_parameter_name][
                        "CallbackEventDataChanged"] = "Rte_Call_CBDataEvt_" + event_parameter_name + "_EventDataChanged"
                if event_parameters[event_parameter_name]["CallbackEventDataChanged"] != "NULL":
                    dem_general["DemCallbackEventDataChangedSupport"] = "STD_ON"
            elif  path_name == "DemCallbackClearEventAllowed":
                if dem_check_if_config_is_not_none(value["DemCallbackClearEventAllowedFnc"]["Value"]):
                    event_parameters[event_parameter_name]["CallbackClearEventAllowed"] = value["DemCallbackClearEventAllowedFnc"]["Value"]
                if event_parameters[event_parameter_name]["CallbackClearEventAllowed"] != "NULL":
                    dem_general["DemCallbackClearEventAllowedSupport"] = "STD_ON"
            elif path_name == "DemCallbackEventUdsStatusChanged":
                if dem_check_if_config_is_not_none(value["DemCallbackEventUdsStatusChangedFnc"]["Value"]):
                    event_parameters[event_parameter_name]["CallbackEventStatusChanged"] = value["DemCallbackEventUdsStatusChangedFnc"]["Value"]
                else:
                    event_parameters[event_parameter_name][
                        "CallbackEventStatusChanged"] = "Rte_Call_CBEventUdsStatusChanged_" + event_parameter_name + "_DemCallbackEventUdsStatusChanged_CallbackEventUdsStatusChanged"
                if event_parameters[event_parameter_name]["CallbackEventStatusChanged"] != "NULL":
                    dem_general["DemCallbackEventUdsStatusChangedSupport"] = "STD_ON"
            else:
                pass

        if event_parameter["DemEventAvailable"]["Value"] == True:
            event_parameters[event_parameter_name]["EventAvailable"] = "TRUE"
        else:
            event_parameters[event_parameter_name]["EventAvailable"] = "FALSE"

    sorted_event_parameters = dict(sorted(event_parameters.items(), key=lambda x: itemgetter('EventId')(x[1])))
    return sorted_event_parameters

def set_event_combined_group_index(dem_event_parameters: dict, dtc_class_ref_index, event_combined_groups_ref_index):
    for event_parameter in dem_event_parameters.values():
        if event_parameter["DTCClassRefIndex"] == dtc_class_ref_index:
            event_parameter["EventCombinedGroupIndex"] = event_combined_groups_ref_index

def process_dem_config_set_event_combined_groups(dem_general:dict, dem_event_parameters: dict, dtc_classes_dict:dict) -> dict:
    dtc_used_dict = {}
    event_combined_groups = {}
    event_combined_groups_index = -1
    combined_event_number = 0

    for event_parameter_name, event_parameter in dem_event_parameters.items():
        event_combined_groups_ref_index = -1
        dtc_class_ref_index = event_parameter["DTCClassRefIndex"]
        event_id = event_parameter["EventId"]
        if dtc_class_ref_index not in dtc_used_dict.keys():
            dtc_used_dict[dtc_class_ref_index] = {
                "EventId": event_id
            }
        elif dtc_class_ref_index in event_combined_groups.keys():
            combined_event_number += 1
            event_combined_groups[dtc_class_ref_index]["EventList"].append(event_id)
            event_combined_groups_ref_index = event_combined_groups[dtc_class_ref_index]["EventCombinedGroupIndex"]
        else:
            combined_event_number += 2
            event_combined_groups_index += 1
            event_combined_groups_ref_index = event_combined_groups_index
            event_combined_groups[dtc_class_ref_index] = {
                "EventList": [dtc_used_dict[dtc_class_ref_index]["EventId"], event_id],
                "EventCombinedGroupIndex": event_combined_groups_index
            }
            set_event_combined_group_index(dem_event_parameters, dtc_class_ref_index, event_combined_groups_ref_index)

        event_parameter["EventCombinedGroupIndex"] = event_combined_groups_ref_index

    dem_general["DemCombinedEventNumber"] = combined_event_number
    return event_combined_groups

def get_enable_condition_group_ref_index(dem_config_dict: dict, event_parameter: dict,
                                         enable_condition_groups: dict) -> dict:
    enable_condition_group_ref_index = -1

    if event_parameter["DemEnableConditionGroupRef"]["ValueRef"] is not None:
        enable_condition_group_ref = AutosarUtil.get_by_path_from_dict(
            dem_config_dict["ArPackages"], event_parameter["DemEnableConditionGroupRef"]["ValueRef"])
        if enable_condition_group_ref is not None:
            dest_enable_condition_group_name = event_parameter["DemEnableConditionGroupRef"]["ValueRef"].split("/")[-1]
            for enable_condition_group_name, enable_condition_group in enable_condition_groups.items():
                if enable_condition_group_name == dest_enable_condition_group_name:
                    enable_condition_group_ref_index = enable_condition_group["EnableConditiongroupIndex"]
    return enable_condition_group_ref_index


def get_storage_condition_group_ref_index(dem_config_dict: dict, event_parameter: dict,
                                          storage_condition_groups: dict) -> dict:
    storage_condition_group_ref_index = -1

    if event_parameter["DemStorageConditionGroupRef"]["ValueRef"] is not None:
        storage_condition_group_ref = AutosarUtil.get_by_path_from_dict(
            dem_config_dict["ArPackages"], event_parameter["DemStorageConditionGroupRef"]["ValueRef"])
        if storage_condition_group_ref is not None:
            dest_storage_condition_group_name = event_parameter["DemStorageConditionGroupRef"]["ValueRef"].split(
                "/")[-1]
            for storage_condition_group_name, storage_condition_group in storage_condition_groups.items():
                if storage_condition_group_name == dest_storage_condition_group_name:
                    storage_condition_group_ref_index = storage_condition_group["StorageConditiongroupIndex"]
    return storage_condition_group_ref_index


def process_dem_config_set_event_classes(dem_config_dict: dict, config_set_dict: dict, dem_general: dict,
                                         dem_debounce_classes: dict) -> dict:
    event_classes = {}
    event_class_index = 0
    global_indicator_attribute_group_ref_index = 0
    dtc_attribute = {}
    enable_condition_groups = dem_general["DemEnableConditionGroups"]
    storage_condition_groups = dem_general["DemStorageConditionGroups"]
    dem_max_number_aged_dtcs = 0
    dem_general["global_debounce_storage_event_count"] = 0

    for event_parameter_name, event_parameter in config_set_dict.items():
        indicator_attribute_ref_name_list = []
        debounce_class_ref_index = -1
        indicator_attribute_group_ref_used = -1
        if "DefinitionRef" not in event_parameter or event_parameter[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter":
            continue
        dtc_attribute = get_dtc_attribute_from_event_parameter(dem_config_dict, event_parameter)
        if dtc_attribute is None:
            continue
        else:
            pass

        dtc_origin = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                       dtc_attribute["DemMemoryDestinationRef"]["ValueRef"])
        if dtc_origin is None or "DefinitionRef" not in dtc_origin:
            event_destination = "DEM_DTC_ORIGIN_NOT_USED"
        elif dtc_origin["DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemPrimaryMemory":
            event_destination = "DEM_DTC_ORIGIN_PRIMARY_MEMORY"
            opcycle_ref = event_parameter["DemOperationCycleRef"]["ValueRef"].split("/")[-1].upper()
        elif dtc_origin["DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemUserDefinedMemory":
            event_destination = "DEM_DTC_ORIGIN_SECOND_MEMORY"
        else:
            event_destination = "DEM_DTC_ORIGIN_NOT_USED"

        enable_condition_group_ref_index = get_enable_condition_group_ref_index(dem_config_dict, event_parameter,
                                                                                enable_condition_groups)
        if enable_condition_group_ref_index >= 0:
            dem_general["DemEnableConditionGroupsUsed"] = "TRUE"
        storage_condition_group_ref_index = get_storage_condition_group_ref_index(dem_config_dict, event_parameter,
                                                                                  storage_condition_groups)
        if storage_condition_group_ref_index >= 0:
            dem_general["DemStorageConditionGroupsUsed"] = "TRUE"

        for indicator_attribute_name, indicator_attribute in event_parameter.items():
            if "DefinitionRef" not in indicator_attribute or indicator_attribute[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemIndicatorAttribute":
                continue
            indicator_attribute_ref_name_list.append(indicator_attribute_name)
            indicator_attribute_group_ref_used = 0

        debounce_class_ref_index = get_debounce_class_ref_index(dem_config_dict, dem_general, event_parameter, dem_debounce_classes)

        opcycle = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                            event_parameter["DemOperationCycleRef"]["ValueRef"])
        if opcycle is not None:
            opcycle_ref = event_parameter["DemOperationCycleRef"]["ValueRef"].split("/")[-1].upper()
        else:
            opcycle_ref = "NULL"

        aging_cycle_ref = "NULL"
        if dtc_attribute["DemAgingCycleRef"]["ValueRef"] is not None:
            aging_cycle = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                            dtc_attribute["DemAgingCycleRef"]["ValueRef"])
            if aging_cycle is not None:
                aging_cycle_ref = dtc_attribute["DemAgingCycleRef"]["ValueRef"].split("/")[-1].upper()

        event_classes[event_parameter_name] = {
            "EventDestination": event_destination,
            "IndicatorAttributeRefNameList": indicator_attribute_ref_name_list,
            "EventPriority": dtc_attribute["DemDTCPriority"]["Value"],
            "OperationCycleRef": opcycle_ref,
            "AgingCounterThreshold": dtc_attribute["DemAgingCycleCounterThreshold"]["Value"],
            "AgingCycleRef": aging_cycle_ref,
            "FailureCycleRef": opcycle_ref,
            "FailureCounterThreshold": event_parameter["DemEventConfirmationThreshold"]["Value"],
            "EventSignificance": dtc_attribute["DemDTCSignificance"]["Value"],
            "EnableConditionGroupRefIndex": enable_condition_group_ref_index,
            "StorageConditionGroupRefIndex": storage_condition_group_ref_index,
            "DebounceAlgorithmClassRefIndex": debounce_class_ref_index,
            "EventClassIndex": event_class_index,
            "EventId": event_parameter["DemEventId"]["Value"],
            "FreezeFramePrestorageSupport": str(event_parameter["DemFFPrestorageSupported"]["Value"]).upper()
        }
        if dtc_attribute["DemAgingAllowed"]["Value"] == True:
            event_classes[event_parameter_name]["AgingAllowed"] = "TRUE"
            event_classes[event_parameter_name]["DemEventAgedData"] = dem_max_number_aged_dtcs
            dem_max_number_aged_dtcs += 1
        else:
            event_classes[event_parameter_name]["DemEventAgedData"] = -1
            event_classes[event_parameter_name]["AgingAllowed"] = "FALSE"

        event_class_index += 1
        if indicator_attribute_group_ref_used >= 0:
            event_classes[event_parameter_name]["IndicatorAttributeGroupRefIndex"] = global_indicator_attribute_group_ref_index
            global_indicator_attribute_group_ref_index += len(indicator_attribute_ref_name_list) + 1
        else:
            event_classes[event_parameter_name]["IndicatorAttributeGroupRefIndex"] = -1

    dem_general["dem_max_number_aged_dtcs"] = dem_max_number_aged_dtcs
    return event_classes


def get_debounce_class_ref_index(dem_config_dict: dict, dem_general:dict, event_parameter: dict, dem_debounce_classes: dict) -> int:
    debounce_class_ref_index = -1

    if "DemDebounceAlgorithmClass" in event_parameter.keys():
        for debounce_name, debounce in event_parameter["DemDebounceAlgorithmClass"].items():
            if "DefinitionRef" not in debounce:
                continue
            if debounce["DefinitionRef"].split("/")[-1] == "DemDebounceCounterBased":
                counter_base_debounce = AutosarUtil.get_by_path_from_dict(
                    dem_config_dict["ArPackages"], debounce["DemDebounceCounterBasedClassRef"]["ValueRef"])
                if counter_base_debounce is None:
                    continue
                elif counter_base_debounce["DemDebounceCounterStorage"]["Value"] == True:
                    dem_general["global_debounce_storage_event_count"] += 1
                else:
                    pass
                debounce_name = debounce["DemDebounceCounterBasedClassRef"]["ValueRef"].split("/")[-1]
            elif debounce["DefinitionRef"].split("/")[-1] == "DemDebounceTimeBase":
                counter_base_debounce = AutosarUtil.get_by_path_from_dict(
                    dem_config_dict["ArPackages"], debounce["DemDebounceTimeBaseRef"]["ValueRef"])
                if counter_base_debounce is None:
                    continue
                debounce_name = debounce["DemDebounceTimeBaseRef"]["ValueRef"].split("/")[-1]
            else:
                continue

            if debounce_name in dem_debounce_classes.keys():
                debounce_class_ref_index = dem_debounce_classes[debounce_name]["DebounceClassIndex"]

    return debounce_class_ref_index

def get_dem_master_task_mapping_info(root_dict: dict) -> dict:
    dem_master_task_mapping_info = {}
    dem_master_candidate_main_functions = ["Dem_MasterMainFunction", "Dem_MainFunction", "dem_master_main_function", "dem_main_function"]
    if "rte" in root_dict.keys():
        rte_dict = root_dict['rte']
        if "task_mappings_info" in rte_dict.keys():
            for task_name in list(rte_dict["task_mappings_info"].keys()):
                if "schedulable_entity_info_list" in rte_dict["task_mappings_info"][task_name].keys():
                    if isinstance(rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"], list):
                        for i in range(len(rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                            if rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] in dem_master_candidate_main_functions:
                                dem_master_task_mapping_info["DemMasterCoreId"] = int(rte_dict["task_mappings_info"][task_name]["core_id"])
                                dem_master_task_mapping_info["DemMasterApplication"] = rte_dict["task_mappings_info"][task_name]["application"]
                                return dem_master_task_mapping_info
    return None

# DemGeneral process functions
def process_dem_general(root_dict: dict) -> dict:
    dem_master_task_mapping_info = get_dem_master_task_mapping_info(root_dict)
    dem_config_dict = root_dict["bsw"]["dem"]
    dem_general_pre = {}
    dem_general_in_json = dem_get_dict_by_definition_ref(dem_config_dict["ArPackages"]["ActiveEcuC"]["Dem"], "DemGeneral")
    dem_enable_conditions = process_dem_general_enableconditions(dem_general_in_json)
    dem_enable_condition_groups = process_dem_general_enableconditiongroups(dem_general_pre, dem_config_dict, dem_general_in_json,
                                                                            dem_enable_conditions)
    dem_storage_conditions = process_dem_general_storageconditions(dem_general_in_json)
    dem_storage_condition_groups = process_dem_general_storageconditiongroups(dem_general_pre, dem_config_dict, dem_general_in_json,
                                                                              dem_storage_conditions)
    dem_indicators = process_dem_general_indicators_config(dem_general_in_json)
    dem_indicator_attributes = process_dem_general_indicator_attributes_config(dem_config_dict, dem_indicators)
    dem_event_memory_set = process_dem_general_event_memory_set(dem_general_in_json, dem_indicators)
    dem_clients_and_cbks = process_dem_general_client(dem_config_dict, dem_general_in_json, dem_event_memory_set)
    dem_data_class_container = process_dem_general_data_class_container_config(dem_general_in_json)
    dem_data_class = dem_data_class_container["dem_data_class"]
    dem_ext_data_record_list = process_dem_general_ext_data_record_list(dem_config_dict, dem_general_in_json,
                                                                                  dem_data_class)
    dem_ext_data_class = process_dem_ext_data_class(dem_config_dict, dem_general_in_json,
                                                              dem_ext_data_record_list)
    dem_nvram_blockid = process_dem_nvram_blockid(root_dict["bsw"], dem_general_in_json)
    dem_did_pid_class_dict = process_dem_did_pid_class_list(dem_config_dict, dem_general_in_json, dem_data_class)
    dem_did_pid_class_list = dem_did_pid_class_dict["dem_did_pid_class_list"]
    dem_freeze_frame_class = process_dem_general_freeze_frame_class(dem_config_dict, dem_general_in_json,
                                                                    dem_did_pid_class_list)
    dem_freeze_frame_record_class = process_dem_general_freeze_frame_record_class(dem_general_in_json)
    dem_freeze_frame_recnum_class = process_dem_general_freeze_frame_recnum_class(
        dem_config_dict, dem_general_in_json)
    dem_opcycles = process_dem_general_opcycles(dem_general_in_json)
    dem_ratio = process_dem_general_ratio(dem_config_dict, dem_general_in_json)
    dem_general_OBD = Process_dem_general_OBD(dem_general_in_json)

    dem_general = {
        "DemClient": dem_clients_and_cbks,
        "DemEnableConditions": dem_enable_conditions,
        "DemEnableConditionGroups": dem_enable_condition_groups,
        "DemStorageConditions": dem_storage_conditions,
        "DemStorageConditionGroups": dem_storage_condition_groups,
        "DemEventMemorySet": dem_event_memory_set,
        "DemIndicators": dem_indicators,
        "DemIndicatorAttributes": dem_indicator_attributes,
        "DemDataClass": dem_data_class,
        "DemExtendedDataRecordList": dem_ext_data_record_list,
        "DemExtendedDataClass": dem_ext_data_class,
        "DemNvRamBlockId": dem_nvram_blockid,
        "DemDidPidClass": dem_did_pid_class_list,
        "DemFreezeFrameClass": dem_freeze_frame_class,
        "DemFreezeFrameRecordClass": dem_freeze_frame_record_class,
        "DemFreezeFrameRecNumClass": dem_freeze_frame_recnum_class,
        "DemOperationCycle": dem_opcycles,
        "DemMaxSizeOBDFFData": dem_did_pid_class_dict["DemMaxSizeOBDFFData"],
        "DemRatio": dem_ratio,
        "DemGeneralOBD": dem_general_OBD,
        "DemEnableConditionUsed": "FALSE",
        "DemStorageConditionUsed": "FALSE",
        "DemDebounceDisable": "STD_OFF",
        "DemCallbackEventUdsStatusChangedSupport": "STD_OFF",
        "DemCallbackClearEventAllowedSupport": "STD_OFF",
        "DemCallbackEventDataChangedSupport": "STD_OFF",
        "DemCallbackInitMforESupport": "STD_OFF",
        "DemEventKindSupport": "STD_OFF",
        "DemMasterCoreId": 0,
        "DemMasterTaskMappingInfo": dem_master_task_mapping_info,
    }
    dem_general.update(dem_general_pre)
    process_dem_general_definition(dem_general, dem_general_in_json, root_dict["bsw"], dem_data_class_container)
    process_dem_general_configuration_id(dem_general)
    if dem_general["OBDSupport"] == "STD_ON":
        process_dem_Permanent_configuration_id(dem_general)

    return dem_general

def calculate_crc16(data:str, len:int):
    crc = 0xFFFF
    while len > 0:
        len -= 1
        crc ^= ord(data[len])
        for i in range(0, 8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0x1021
            else:
                crc >>= 1

    return crc

def process_dem_general_configuration_id(dem_general: dict):
    crc_data = str(dem_general["DemConfigurationId"])
    dem_nvram_block = dem_general["DemNvRamBlockId"]
    block_idx = 0
    for block in dem_nvram_block.values():
        crc_data += str(block_idx)
        if block != "NULL":
            crc_data += str(block["NvRamBlockId"]) + str(block["NvRamBlockLength"]) + block["NvMRamBlockDataAddress"]
    data_len = len(crc_data)
    dem_general["DemConfigurationId"] = calculate_crc16(crc_data, data_len)

def process_dem_Permanent_configuration_id(dem_general: dict):
    crc_data = ""
    dem_nvram_block = dem_general["DemNvRamBlockId"]
    if "NvMConf_NvMBlockDescriptor_Dem_PerMemEventBlock" in dem_nvram_block:
        block = dem_nvram_block["NvMConf_NvMBlockDescriptor_Dem_PerMemEventBlock"]
        crc_data += str(block["NvRamBlockId"]) + str(block["NvRamBlockLength"]) + block["NvMRamBlockDataAddress"]
        data_len = len(crc_data)
        dem_general["DemPermanentConfigurationId"] = calculate_crc16(crc_data, data_len)
    else:
        dem_general["DemPermanentConfigurationId"] = 0

def process_dem_general_definition(dem_general: dict, dem_general_in_json: dict, config_dict: dict, dem_data_class_container:dict):
    dtc_status_availability_mask = 0
    max_size_of_freeze_frame_data = 0
    max_number_event_entry_primary = 0
    max_number_event_entry_secondary = 0
    dem_event_memory_set = dem_general["DemEventMemorySet"]
    dem_internal_data_elem = dem_data_class_container["dem_internal_data_elem"]
    max_number_event_entry_permanent = 0

    # 默认配置
    dem_general["OpCycleStorage"] = "STD_OFF"
    dem_general["GlobalPermanentSize"] = 4
    dem_general["GlobalFreezeFrameSize"] = 1
    dem_general["DemEvMemOverflowIndicationSupport"] = "STD_ON"
    if "DEM_CURRENT_FDC" in dem_internal_data_elem:
        dem_general["DataCurrentFdc"] = "STD_ON"
    else:
        dem_general["DataCurrentFdc"] = "STD_OFF"
    if "DEM_CYCLES_SINCE_FIRST_FAILED" in dem_internal_data_elem:
        dem_general["DataCyclesSinceFirstFailed"] = "STD_ON"
    else:
        dem_general["DataCyclesSinceFirstFailed"] = "STD_OFF"
    if "DEM_CYCLES_SINCE_LAST_FAILED" in dem_internal_data_elem:
        dem_general["DataCyclesSinceLastFailed"] = "STD_ON"
    else:
        dem_general["DataCyclesSinceLastFailed"] = "STD_OFF"
    if "DEM_FAILED_CYCLES" in dem_internal_data_elem:
        dem_general["DataFailedCycles"] = "STD_ON"
    else:
        dem_general["DataFailedCycles"] = "STD_OFF"
    if "DEM_MAX_FDC_DURING_CURRENT_CYCLE" in dem_internal_data_elem:
        dem_general["DataMaxFdcDuringCurrentCycle"] = "STD_ON"
    else:
        dem_general["DataMaxFdcDuringCurrentCycle"] = "STD_OFF"
    if "DEM_OCCCTR" in dem_internal_data_elem:
        dem_general["DataOccurCounter"] = "STD_ON"
    else:
        dem_general["DataOccurCounter"] = "STD_OFF"
    if "DEM_OVFLIND" in dem_internal_data_elem:
        dem_general["DataOverflow"] = "STD_ON"
    else:
        dem_general["DataOverflow"] = "STD_OFF"
    if "DEM_AGED_COUNTER" in dem_internal_data_elem:
        dem_general["DataAgedCounter"] = "STD_ON"
    else:
        dem_general["DataAgedCounter"] = "STD_OFF"
    if "DEM_FAULT_PENDING_COUNTER" in dem_internal_data_elem:
        dem_general["DataFailedPendingCounter"] = "STD_ON"
    else:
        dem_general["DataFailedPendingCounter"] = "STD_OFF"
    dem_general["DemStorageConditionGroupsUsed"] = "FALSE"
    dem_general["DemEnableConditionGroupsUsed"] = "FALSE"
    dem_general["DemSupportIndicator"] = "FALSE"
    dem_general["DemEventCombinationSupport"] = dem_general_in_json["DemEventCombinationSupport"]["Value"]
    if "DemStoreImmediatelyTime" in dem_general_in_json:
        if dem_general_in_json["DemStoreImmediatelyTime"]["Value"] is None:
            dem_general["DemStoreImmediatelyTime"] = int(dem_general_in_json["DemTaskTime"]["Value"] * 1000.0)
        else:
            dem_general["DemStoreImmediatelyTime"] = int(dem_general_in_json["DemStoreImmediatelyTime"]["Value"] * 1000.0)
    '''
    # 暂不支持配置
    if dem_general_in_json["DemVersionInfoApi"]["Value"] == True:
        dem_general["VersionInfoApi"] = "STD_ON"
    else:
        dem_general["VersionInfoApi"] = "STD_OFF"

    if dem_general_in_json["DemTriggerFiMReports"]["Value"] == True:
        dem_general["TriggerFiMReports"] = "STD_ON"
    else:
        dem_general["TriggerFiMReports"] = "STD_OFF"

    if dem_general_in_json["DemPTOSupport"]["Value"] == True:
        dem_general["PTOSupport"] = "STD_ON"
    else:
        dem_general["PTOSupport"] = "STD_OFF"
    '''
    if dem_general_in_json["DemConfigurationId"]["Value"]:
        dem_general["DemConfigurationId"] = dem_general_in_json["DemConfigurationId"]["Value"]
    else:
        dem_general["DemConfigurationId"] = 0

    dem_general["SupportIumpr"] = "STD_OFF"
    if dem_general_in_json["DemOBDSupport"]["Value"] != "DEM_OBD_NO_OBD_SUPPORT" and dem_general_in_json["DemOBDSupport"]["Value"] \
            is not None and dem_general_in_json["DemOBDSupport"]["Value"] != "":
        dem_general["OBDSupport"] = "STD_ON"
        dem_ratio = dem_general_in_json.get("DemRatio", None)
        if dem_ratio:
            if len(dem_general_in_json["DemRatio"]) > 0:
                dem_general["SupportIumpr"] = "STD_ON"
    else:
        dem_general["OBDSupport"] = "STD_OFF"

    if dem_general_in_json["DemMaxNumberPrestoredFF"]["Value"]:
        dem_general["MaxNumberPrestoredFF"] = dem_general_in_json["DemMaxNumberPrestoredFF"]["Value"]
    else:
        dem_general["MaxNumberPrestoredFF"] = 0

    dem_general["TaskTime"] = int(dem_general_in_json["DemTaskTime"]["Value"] * 1000.0)
    dem_general["GeneralInterfaceSupport"] = dem_general_in_json["DemGeneralInterfaceSupport"]["Value"]

    if "DemDebounceDisable" in dem_general_in_json and dem_general_in_json["DemDebounceDisable"]["Value"] == True:
        dem_general["DemDebounceDisable"] = "STD_ON"
    else:
        dem_general["DemDebounceDisable"] = "STD_OFF"

    if dem_general_in_json["DemDevErrorDetect"]["Value"] == True:
        dem_general["DevelopmentErrorDetect"] = "STD_ON"
    else:
        dem_general["DevelopmentErrorDetect"] = "STD_OFF"

    if dem_general_in_json["DemTriggerFiMReports"]["Value"] == True:
        dem_general["TriggerFiMReports"] = "STD_ON"
    else:
        dem_general["TriggerFiMReports"] = "STD_OFF"

    if dem_general_in_json["DemSuppressionSupport"]["Value"] == "DEM_DTC_SUPPRESSION":
        dem_general["SuppressionSupport"] = "STD_ON"
    else:
        dem_general["SuppressionSupport"] = "STD_OFF"

    if dem_general_in_json["DemAvailabilitySupport"]["Value"] == "DEM_EVENT_AVAILABILITY":
        dem_general["DemAvailabilitySupport"] = "STD_ON"
    else:
        dem_general["DemAvailabilitySupport"] = "STD_OFF"

    for _, event_memory_set in dem_event_memory_set.items():
        dtc_status_availability_mask = dtc_status_availability_mask | event_memory_set["DemDtcStatusAvailabilityMask"][
            "Value"]
        if event_memory_set["DemTypeOfDTCSupported"]["Value"] == "DEM_DTC_TRANSLATION_SAE_J2012_DA_DTCFORMAT_04":
            dem_general["DemTypeOfDTCSupported"] = "DEM_DTC_TRANSLATION_J2012DA_FORMAT_04"
        elif event_memory_set["DemTypeOfDTCSupported"]["Value"] == "DEM_DTC_TRANSLATION_ISO11992_4":
            dem_general["DemTypeOfDTCSupported"] = "DEM_DTC_TRANSLATION_ISO11992_4"
        elif event_memory_set["DemTypeOfDTCSupported"]["Value"] == "DEM_DTC_TRANSLATION_ISO14229_1":
            dem_general["DemTypeOfDTCSupported"] = "DEM_DTC_TRANSLATION_ISO14229_1"
        elif event_memory_set["DemTypeOfDTCSupported"]["Value"] == "DEM_DTC_TRANSLATION_ISO15031_6":
            dem_general["DemTypeOfDTCSupported"] = "DEM_DTC_TRANSLATION_ISO15031_6"
        elif event_memory_set["DemTypeOfDTCSupported"]["Value"] == "DEM_DTC_TRANSLATION_SAEJ1939_73":
            dem_general["DemTypeOfDTCSupported"] = "DEM_DTC_TRANSLATION_SAEJ1939_73"
        else:
            dem_general["DemTypeOfDTCSupported"] = event_memory_set["DemTypeOfDTCSupported"]["Value"]
        max_number_event_entry_permanent += event_memory_set["max_number_event_entry_permanent"]["Value"]
        for key, value in event_memory_set.items():
            if value is {} or isinstance(value, dict) is False:
                continue
            if "DefinitionRef" in value and value[
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemPrimaryMemory":
                max_number_event_entry_primary += value["DemMaxNumberEventEntryPrimary"]["Value"]
            if "DefinitionRef" in value and value[
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemUserDefinedMemory":
                max_number_event_entry_secondary += value["DemMaxNumberEventEntryUserDefined"]["Value"]
    dem_general["DTCStatusAvailabilityMask"] = dtc_status_availability_mask
    dem_general["DemMaxNumberEventEntryPrimary"] = max_number_event_entry_primary
    dem_general["DemMaxNumberEventEntrySecondary"] = max_number_event_entry_secondary
    dem_general["DemMaxNumberEventEntryPermanent"] = max_number_event_entry_permanent

    dem_general["MaxSizeOfFreezeFrameData"] = max_size_of_freeze_frame_data

    master_application_ref = dem_general_in_json["DemMasterOsApplicationRef"]["ValueRef"]
    if master_application_ref is not None and master_application_ref != "" and master_application_ref != "<empty>":
        master_application_name = master_application_ref.split("/")[-1]
        dem_general["DemMasterApplicationName"] = master_application_name
        dem_general["DemMasterApplicationValue"] = AutosarUtil.get_by_path_from_dict(config_dict["os"]["ArPackages"], master_application_ref)
        dem_general["DemSatelliteEnable"] = "True"
    else:
        # 如果没有设置 DemMasterOsApplicationRef 时默认使用Os下的第一个Application,默认值"0"
        dem_general["DemMasterApplicationName"] = "DEM_DEFAULT_SINGLE_SATELLITE_ID"
        dem_general["DemMasterApplicationValue"] = 0
        dem_general["DemSatelliteEnable"] = "False"
        pass

    return


def process_dem_general_client(dem_config_dict: dict, general_dict: dict, dem_event_memory_set: dict) -> dict:
    clients = {}
    dem_dtc_cbk_list = {}
    clients_and_cbks = {}
    dem_dtc_cbk_index = 0
    dtc_cbk_start = 0

    for client_name, client in general_dict.items():
        if "DefinitionRef" not in client or client["DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemClient":
            continue
        clients[client_name] = client
        clients[client_name]["EventMemorySetIndex"] = -1
        for dtc_cbk_name, dtc_cbk in client.items():
            if dtc_cbk_name == 'EventMemorySetIndex':
                continue
            if "DefinitionRef" not in dtc_cbk or dtc_cbk["DefinitionRef"] != \
                "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemClient/DemCallbackDTCStatusChanged":
                continue
            client_cbk_name = client_name + "_" + dtc_cbk_name
            if dtc_cbk['DemCallbackDTCStatusChangedFnc']['Value'] == '' or dtc_cbk['DemCallbackDTCStatusChangedFnc'][
                    'Value'] == None:
                dem_dtc_cbk_list[client_cbk_name] = 'NULL'
            else:
                dem_dtc_cbk_list[client_cbk_name] = dtc_cbk['DemCallbackDTCStatusChangedFnc']['Value']
            dem_dtc_cbk_index += 1
        clients[client_name]["startIdx"] = dtc_cbk_start
        clients[client_name]["num"] = dem_dtc_cbk_index
        dtc_cbk_start = dem_dtc_cbk_index
        dem_dtc_cbk_index = 0

        if "DefinitionRef" not in client["DemEventMemorySetRef"] or client["DemEventMemorySetRef"][
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemClient/DemEventMemorySetRef":
            continue

        if client["DemEventMemorySetRef"]["ValueRef"] is None:
            continue

        DestEventMemorySet = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                               client["DemEventMemorySetRef"]["ValueRef"])
        if DestEventMemorySet is None:
            continue

        dest_event_memory_set_name = client["DemEventMemorySetRef"]["ValueRef"].split("/")[-1]
        for event_memory_set_name, event_memory_set in dem_event_memory_set.items():
            if event_memory_set_name == dest_event_memory_set_name:
                clients[client_name]["EventMemorySetIndex"] = event_memory_set["EventMemorySetIndex"]["Value"]
    clients_and_cbks['clients'] = clients
    clients_and_cbks['cbks'] = dem_dtc_cbk_list
    return clients_and_cbks


def process_dem_general_enableconditions(general_dict: dict) -> dict:
    EnableConditions = {}
    EnableConditionIndex = 0

    for EnableConditonName, EnableConditon in general_dict.items():
        if "DefinitionRef" not in EnableConditon or EnableConditon[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEnableCondition":
            continue
        EnableConditions[EnableConditonName] = EnableConditon
        if EnableConditions[EnableConditonName]["DemEnableConditionStatus"]["Value"] == True:
            EnableConditions[EnableConditonName]["DemEnableConditionStatus"] = "TRUE"
        else:
            EnableConditions[EnableConditonName]["DemEnableConditionStatus"] = "FALSE"
        EnableConditions[EnableConditonName]["EnableConditionIndex"] = EnableConditionIndex
        EnableConditionIndex += 1
    return EnableConditions


def process_dem_general_enableconditiongroups(dem_general_pre:dict, dem_config_dict: dict, general_dict: dict,
                                              dem_enable_conditions: dict) -> dict:
    EnableConditiongroups = {}
    EnableConditiongroupIndex = 0
    EnableConditionGroupClassListIndex = 0

    for EnableConditongroupName, EnableConditongroup in general_dict.items():
        if "DefinitionRef" not in EnableConditongroup or EnableConditongroup[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEnableConditionGroup":
            continue
        if "DemEnableConditionRef" not in EnableConditongroup or EnableConditongroup["DemEnableConditionRef"][
                "ValueRef"] == "":
            #error
            continue

        EnableConditionList = []
        if isinstance(EnableConditongroup["DemEnableConditionRef"]["ValueRef"], str):
            EnableCondition = AutosarUtil.get_by_path_from_dict(
                dem_config_dict["ArPackages"], EnableConditongroup["DemEnableConditionRef"]["ValueRef"])
            if EnableCondition == None:
                continue
            EnableConditionList.append(EnableCondition["EnableConditionIndex"])
            dem_general_pre["DemEnableConditionUsed"] = "TRUE"
        elif isinstance(EnableConditongroup["DemEnableConditionRef"]["ValueRef"], list):
            for EnableConditionRef in EnableConditongroup["DemEnableConditionRef"]["ValueRef"]:
                EnableCondition = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"], EnableConditionRef)
                if EnableCondition is None:
                    continue
                for enable_condition_name, enable_condition in dem_enable_conditions.items():
                    if enable_condition_name == EnableConditionRef.split("/")[-1]:
                        EnableConditionList.append(enable_condition["EnableConditionIndex"])
                        dem_general_pre["DemEnableConditionUsed"] = "TRUE"

        nofEnableConditions = len(EnableConditionList)
        EnableConditiongroups[EnableConditongroupName] = {
            "nofEnableConditions": nofEnableConditions,
            "DemEnableConditionRef": EnableConditionList,
            "EnableConditiongroupIndex": EnableConditiongroupIndex,
            "EnableConditionListIndex": EnableConditionGroupClassListIndex
        }
        EnableConditiongroupIndex += 1
        EnableConditionGroupClassListIndex += nofEnableConditions
    return EnableConditiongroups


def process_dem_general_storageconditions(general_dict: dict) -> dict:
    StorageConditions = {}
    StorageConditionIndex = 0

    for StorageConditonName, StorageConditon in general_dict.items():
        if "DefinitionRef" not in StorageConditon or StorageConditon[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemStorageCondition":
            continue
        StorageConditions[StorageConditonName] = StorageConditon
        if StorageConditions[StorageConditonName]["DemStorageConditionStatus"]["Value"] == True:
            StorageConditions[StorageConditonName]["DemStorageConditionStatus"] = "TRUE"
        else:
            StorageConditions[StorageConditonName]["DemStorageConditionStatus"] = "FALSE"
        StorageConditions[StorageConditonName]["StorageConditionIndex"] = StorageConditionIndex
        StorageConditionIndex += 1
    return StorageConditions


def process_dem_general_storageconditiongroups(dem_general_pre:dict, dem_config_dict: dict, general_dict: dict,
                                               dem_storage_conditions: dict) -> dict:
    StorageConditiongroups = {}
    StorageConditiongroupIndex = 0
    StorageConditionGroupClassListIndex = 0

    for StorageConditongroupName, StorageConditongroup in general_dict.items():
        if "DefinitionRef" not in StorageConditongroup or StorageConditongroup[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemStorageConditionGroup":
            continue
        if "DemStorageConditionRef" not in StorageConditongroup or StorageConditongroup["DemStorageConditionRef"][
                "ValueRef"] == "":
            #error
            continue

        StorageConditionList = []
        if isinstance(StorageConditongroup["DemStorageConditionRef"]["ValueRef"], str):
            StorageCondition = AutosarUtil.get_by_path_from_dict(
                dem_config_dict["ArPackages"], StorageConditongroup["DemStorageConditionRef"]["ValueRef"])
            if StorageCondition == None:
                continue
            StorageConditionList.append(StorageCondition["StorageConditionIndex"])
            dem_general_pre["DemStorageConditionUsed"] = "TRUE"
        elif isinstance(StorageConditongroup["DemStorageConditionRef"]["ValueRef"], list):
            for StorageConditionRef in StorageConditongroup["DemStorageConditionRef"]["ValueRef"]:
                StorageCondition = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"], StorageConditionRef)
                if StorageCondition is None:
                    continue
                for storage_condition_name, storage_condition in dem_storage_conditions.items():
                    if storage_condition_name == StorageConditionRef.split("/")[-1]:
                        StorageConditionList.append(storage_condition["StorageConditionIndex"])
                        dem_general_pre["DemStorageConditionUsed"] = "TRUE"

        nofStorageConditions = len(StorageConditionList)
        StorageConditiongroups[StorageConditongroupName] = {
            "nofStorageConditions": nofStorageConditions,
            "DemStorageConditionRef": StorageConditionList,
            "StorageConditiongroupIndex": StorageConditiongroupIndex,
            "StorageConditionListIndex": StorageConditionGroupClassListIndex
        }
        StorageConditiongroupIndex += 1
        StorageConditionGroupClassListIndex += nofStorageConditions
    return StorageConditiongroups


def process_dem_general_event_memory_set(general_dict: dict, DemIndicators_dict: dict) -> dict:
    event_memory_set = {}
    EventMemorySetIndex = 0

    for event_memory_set_name, EventMemorySetItem in general_dict.items():
        if "DefinitionRef" not in EventMemorySetItem or EventMemorySetItem[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet":
            continue
        event_memory_set[event_memory_set_name] = EventMemorySetItem
        for pri_memory in EventMemorySetItem.values():
            if "DefinitionRef" not in pri_memory or pri_memory[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemPrimaryMemory":
                continue
            event_memory_set[event_memory_set_name]["DemPrimaryMemory"] = pri_memory
        event_memory_set[event_memory_set_name]["DemUserDefinedMemoryList"] = {}
        for user_defined_memory_name, user_defined_memory in EventMemorySetItem.items():
            if "DefinitionRef" not in user_defined_memory or user_defined_memory[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemUserDefinedMemory":
                continue
            event_memory_set[event_memory_set_name]["DemUserDefinedMemoryList"][
                user_defined_memory_name] = user_defined_memory
        event_memory_set[event_memory_set_name]["EventMemorySetIndex"] = {"Value": EventMemorySetIndex}
        event_memory_set[event_memory_set_name]["MILIndicatorRefIndex"] = {"Value": -1}
        event_memory_set[event_memory_set_name]["MILIndicatorId"] = {"Value": "DEM_INVALID_INDICATOR"}
        event_memory_set[event_memory_set_name]["max_number_event_entry_permanent"] = {"Value": EventMemorySetItem["DemMaxNumberEventEntryPermanent"]["Value"]}
        event_memory_set[event_memory_set_name]["DemDtcStatusAvailabilityMask"] = {"Value": EventMemorySetItem["DemDtcStatusAvailabilityMask"]["Value"]}
        event_memory_set[event_memory_set_name]["DemTypeOfDTCSupported"] = {"Value": EventMemorySetItem["DemTypeOfDTCSupported"]["Value"]}
        if "DemOBDSupport" not in general_dict or general_dict["DemOBDSupport"]["Value"] is None or general_dict[
                "DemOBDSupport"]["Value"] == "":
            pass
        elif general_dict["DemOBDSupport"]["Value"] == "DEM_OBD_MASTER_ECU" or general_dict["DemOBDSupport"][
                "Value"] == "DEM_OBD_PRIMARY_ECU":
            if "DemMILIndicatorRef" in EventMemorySetItem and EventMemorySetItem["DemMILIndicatorRef"][
                    "ValueRef"] is not None:
                indicator_name = EventMemorySetItem["DemMILIndicatorRef"]["ValueRef"].split('/')[-1]
                event_memory_set[event_memory_set_name]["MILIndicatorRefIndex"]["Value"] = get_indicator_index_by_name(
                    indicator_name, DemIndicators_dict)
                event_memory_set[event_memory_set_name]["MILIndicatorId"]["Value"] = indicator_name
        EventMemorySetIndex += 1
    return event_memory_set


def process_dem_general_indicators_config(general_dict: dict) -> dict:
    indicators = {}
    indicator_tmp = {}
    indicator_index = 0

    for event_memory_set_name, event_memory_set in general_dict.items():
        if "DefinitionRef" not in event_memory_set or event_memory_set[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet":
            continue
        for indicator_name, indicator in event_memory_set.items():
            if "DefinitionRef" not in indicator or indicator[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemIndicator":
                continue
            indicator_tmp[indicator_name] = indicator

    #indicators = sorted(indicator_tmp.items(), key = lambda x:x["DemIndicatorID"])
    indicators = indicator_tmp
    for indicator_name, indicator in indicator_tmp.items():
        indicators[indicator_name]["IndicatorIndex"] = indicator_index
        indicator_index += 1
    return indicators


def get_indicator_index_by_name(Name: str, DemIndicators_dict: dict) -> int:
    indicator_index = -1

    for indicator_name, indicator in DemIndicators_dict.items():
        if indicator_name == Name:
            indicator_index = indicator["IndicatorIndex"]

    return indicator_index

def get_indicator_id_by_name(Name: str, DemIndicators_dict: dict) -> int:
    indicator_id = -1

    for indicator_name, indicator in DemIndicators_dict.items():
        if indicator_name == Name:
            indicator_id = indicator["IndicatorIndex"]

    return indicator_id

def process_dem_general_indicator_attributes_config(dem_config_dict: dict, DemIndicators_dict: dict) -> int:
    indicator_attributes = {}
    indicator_attribute_index = 0

    config_set_in_json = dem_get_dict_by_definition_ref(dem_config_dict["ArPackages"]["ActiveEcuC"]["Dem"], "DemConfigSet")

    for event_parameter_name, event_parameter in config_set_in_json.items():
        if "DefinitionRef" not in event_parameter or event_parameter[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter":
            continue

        for indicator_attribute_name, indicator_attribute in event_parameter.items():
            if "DefinitionRef" not in indicator_attribute or indicator_attribute[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemEventParameter/DemIndicatorAttribute":
                continue
            if indicator_attribute_name in indicator_attributes.keys():
                indicator_attribute_name += "_" + event_parameter_name
            indicator_name = indicator_attribute["DemIndicatorRef"]["ValueRef"].split('/')[-1]
            opcycle_ref = ""
            opcycle = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                                event_parameter["DemOperationCycleRef"]["ValueRef"])
            if opcycle is not None:
                opcycle_ref = event_parameter["DemOperationCycleRef"]["ValueRef"].split("/")[-1].upper()
            else:
                opcycle_ref = "NULL"

            indicator_attributes[indicator_attribute_name] = {
            "IndicatorBufferIndex":
                indicator_attribute_index,
            "IndicatorRefIndex":
                get_indicator_index_by_name(indicator_name, DemIndicators_dict),
            "IndicatorBehaviour":
                indicator_attribute["DemIndicatorBehaviour"]["Value"],
            "IndicatorFailureCycle":
                opcycle_ref,
            "IndicatorFailureCycleSource":
                "DEM_FAILURE_CYCLE_INDICATOR",
            "IndicatorFailureCycleThreshold":
                indicator_attribute["DemIndicatorFailureCycleCounterThreshold"]["Value"],
            "IndicatorHealingCycle":
                opcycle_ref,
            "IndicatorHealingCycleThreshold":
                indicator_attribute["DemIndicatorHealingCycleCounterThreshold"]["Value"]
            }
            indicator_attribute_index += 1

    return indicator_attributes


def process_dem_indicator_attribute_groups_config(DemEventClasses_dict: dict, IndicatorAttributes_dict: dict) -> dict:
    indicator_attribute_groups = {}
    indicator_attribute_index = 0

    for event_class_name, event_class in DemEventClasses_dict.items():
        event_class_name_list = []
        if event_class["IndicatorAttributeRefNameList"] != []:
            for indicator_attribute_name in event_class["IndicatorAttributeRefNameList"]:
                event_class_name_list.append(indicator_attribute_index)
                indicator_attribute_index += 1

            event_class_name_list.append(len(IndicatorAttributes_dict))
            indicator_attribute_groups[event_class_name] = event_class_name_list
    return indicator_attribute_groups


def process_dem_general_indicator_event_groups_config(DemEventClasses_dict: dict, indicator_attributes_dict: dict,
                                                      DemIndicators_dict: dict, dem_general:dict):
    event_list_ref_index = 0

    for indicator_name, indicator in DemIndicators_dict.items():
        indicator_event_list = []
        indicator["EventListSize"] = 0
        event_list_size = 0

        for _, event_class in DemEventClasses_dict.items():
            for indicator_attribute_name in event_class["IndicatorAttributeRefNameList"]:
                indicator_ref_index = get_indicator_ref_index_from_indicator_attribute(
                    indicator_attributes_dict, indicator_attribute_name)
                if indicator_ref_index == indicator["IndicatorIndex"]:
                    indicator_event_list.append(event_class["EventClassIndex"])
                    event_list_size += 1
                    indicator["EventListSize"] = event_list_size
                    dem_general["DemSupportIndicator"] = "TRUE"
        if event_list_size > 0:
            indicator["EventListRefIndex"] = event_list_ref_index
            event_list_ref_index = event_list_ref_index + event_list_size + 1
            indicator["IndicatorEventGroupList"] = indicator_event_list
        else:
            indicator["EventListRefIndex"] = -1
    return


def process_dem_config_set_dtc_group(event_memory_set_list: dict) -> dict:
    dem_dtc_group = {}
    dem_dtc_group_index = 0

    for _, event_memory_set in event_memory_set_list.items():
        for _, primary_memory in event_memory_set.items():
            if "DefinitionRef" not in primary_memory or primary_memory[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemPrimaryMemory":
                continue
            for dtc_group_name, dtc_group in primary_memory.items():
                if "DemGroupDTCs" in dtc_group:
                    dem_dtc_group[dtc_group_name] = dtc_group
                    dem_dtc_group[dtc_group_name]["DTCGroupIndex"] = dem_dtc_group_index
                    dem_dtc_group_index += 1
        for _, user_defined_memory in event_memory_set.items():
            if "DefinitionRef" not in user_defined_memory or user_defined_memory[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemUserDefinedMemory":
                continue
            for dtc_group_name, dtc_group in user_defined_memory.items():
                if "DemGroupDTCs" in dtc_group:
                    dem_dtc_group[dtc_group_name] = dtc_group
                    dem_dtc_group[dtc_group_name]["DTCGroupIndex"] = dem_dtc_group_index
                    dem_dtc_group_index += 1
    return dem_dtc_group

def process_dem_config_set_clear_dtc_notification(event_memory_set_list: dict) -> dict:
    clear_dtc_notification_start_list = []
    clear_dtc_notification_finish_list = []
    clear_dtc_notification = {}

    for event_memory_set_name, event_memory_set in event_memory_set_list.items():
        if "DefinitionRef" not in event_memory_set or event_memory_set[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet":
            continue

        for clear_dtc_notification_name, clear_dtc_notification in event_memory_set.items():
            if "DefinitionRef" not in clear_dtc_notification or clear_dtc_notification[
                    "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet/DemClearDTCNotification":
                continue
            if clear_dtc_notification["DemClearDtcNotificationFnc"]["Value"] is None or clear_dtc_notification[
                    "DemClearDtcNotificationFnc"]["Value"] == "":
                continue # 暂时不支持走RTE
                # notification_fnc = "Rte_Call_ClearDtcNotification_" + event_memory_set_name + "_" + clear_dtc_notification_name + "_ClearDtcNotification"
            else:
                notification_fnc = clear_dtc_notification["DemClearDtcNotificationFnc"]["Value"]
            if clear_dtc_notification["DemClearDtcNotificationTime"]["Value"] == "START":
                clear_dtc_notification_start_list.append(notification_fnc)
            elif clear_dtc_notification["DemClearDtcNotificationTime"]["Value"] == "FINISH":
                clear_dtc_notification_finish_list.append(notification_fnc)
            else:
                continue

    clear_dtc_notification = {
        "DemClearDTCNotification_start": clear_dtc_notification_start_list,
        "DemClearDTCNotification_finish": clear_dtc_notification_finish_list,
    }

    return clear_dtc_notification

def get_indicator_ref_index_from_indicator_attribute(indicator_attributes_dict: dict,
                                                     target_indicator_attribute_name: str) -> int:
    indicator_ref_index = -1
    for indicator_attribute_name, indicator_attribute in indicator_attributes_dict.items():
        if indicator_attribute_name == target_indicator_attribute_name:
            indicator_ref_index = indicator_attribute["IndicatorRefIndex"]
    return indicator_ref_index


def set_dem_general_max_number_event_entry(dem_general: dict, dem_event_classes: dict):
    dem_number_event_entry_primary = 0
    dem_number_event_entry_secondary = 0

    for event_class_name, event_class in dem_event_classes.items():
        if event_class["EventDestination"] == "DEM_DTC_ORIGIN_PRIMARY_MEMORY":
            dem_number_event_entry_primary += 1

        if event_class["EventDestination"] == "DEM_DTC_ORIGIN_SECOND_MEMORY":
            dem_number_event_entry_secondary += 1

    if dem_number_event_entry_primary < dem_general["DemMaxNumberEventEntryPrimary"]:
        dem_general["DemMaxNumberEventEntryPrimary"] = dem_number_event_entry_primary
    if dem_number_event_entry_secondary < dem_general["DemMaxNumberEventEntrySecondary"]:
        dem_general["DemMaxNumberEventEntrySecondary"] = dem_number_event_entry_secondary

    return


def process_dem_general_data_class_container_config(dem_general_in_json: dict) -> dict:
    dem_data_class = {}
    dem_data_class_container = {}
    dem_data_class_index = 0
    dem_internal_data_elem = []

    for data_class_name, data_class in dem_general_in_json.items():
        if "DefinitionRef" not in data_class or data_class[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass":
            continue
        for _, value in data_class.items():
            if "DefinitionRef" in value and value[
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass":
                dem_data_class[data_class_name] = {
                    "Endianness": "DEM_DATA_OPAQUE",  #目前仅支持DEM_DATA_OPAQUE
                    "DataSize": value["DemDataElementArraySize"]["Value"],
                    "DataElementReadFnc": value["DemDataElementReadFnc"]["Value"],
                    "DataElementDataType": "uint8",  #value["DemDataElementDataType"]["Value"], 工具传进来的类型有误，暂时规避
                    "InternalDataType": "DEM_NO_ELEMENT"
                }
                if value["DemDataElementReadFnc"]["Value"] == "" or value["DemDataElementReadFnc"][
                        "Value"] == "<empty>" or value["DemDataElementReadFnc"]["Value"] is None:
                    dem_data_class[data_class_name]["DataElementReadFnc"] = "NULL"
                if value["DemDataElementUsePort"]["Value"] == True:
                    dem_data_class[data_class_name]["DataElementUsePort"] = "DEM_CS_PORT"
                    dem_data_class[data_class_name][
                        "DataElementReadFnc"] = "Rte_Call_DataServices_" + data_class_name + "_ReadData"
                else:
                    dem_data_class[data_class_name]["DataElementUsePort"] = "FALSE"

                dem_data_class[data_class_name]["DataClassIndex"] = dem_data_class_index
                dem_data_class_index += 1
            elif "DefinitionRef" in value and value[
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalSRDataElementClass":
                dem_data_class[data_class_name] = {
                    "Endianness": value["DemDataElementEndianness"]["Value"],
                    "DataSize": value["DemDataElementArraySize"]["Value"],
                    "DataElementReadFnc": "NULL",
                    "DataElementUsePort": "FALSE",
                    "InternalDataType": value["DemDataElementDataType"]["Value"],
                }
                dem_data_class[data_class_name]["DataClassIndex"] = dem_data_class_index
                dem_data_class_index += 1
            elif "DefinitionRef" in value and value[
                    "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemInternalDataElementClass":
                dem_data_class[data_class_name] = {
                    "Endianness": "DEM_DATA_OPAQUE",
                    "DataSize": value["DemDataElementDataSize"]["Value"],
                    "DataElementReadFnc": "NULL",
                    "DataElementUsePort": "DEM_INTERNAL"
                }
                if "AGINGCTR" in value["DemInternalDataElement"]["Value"]:
                    dem_data_class[data_class_name]["InternalDataType"] = "DEM_AGINGCTR"
                else:
                    dem_data_class[data_class_name]["InternalDataType"] = value["DemInternalDataElement"]["Value"]
                if value["DemInternalDataElement"]["Value"] not in dem_internal_data_elem:
                    dem_internal_data_elem.append(value["DemInternalDataElement"]["Value"])
                dem_data_class[data_class_name]["DataClassIndex"] = dem_data_class_index
                dem_data_class_index += 1
            else:
                #error
                continue
    dem_data_class_container["dem_data_class"] = dem_data_class
    dem_data_class_container["dem_internal_data_elem"] = dem_internal_data_elem

    return dem_data_class_container


def process_dem_general_ext_data_record_list(dem_config_dict: dict, dem_general_in_json: dict,
                                                  dem_data_class: dict) -> dict:
    dem_ext_data_record = {}
    dem_ext_data_record_index = 0
    data_class_ref_start = 0

    for ext_data_record_name, ext_data_record in dem_general_in_json.items():
        if "DefinitionRef" not in ext_data_record or ext_data_record[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemExtendedDataRecordClass":
            continue

        data_class_ref_list = []
        data_class_ref_number = 0

        if "DemDataElementClassRef" in ext_data_record:
            if isinstance(ext_data_record["DemDataElementClassRef"]["ValueRef"], str):
                data_class = AutosarUtil.get_by_path_from_dict(
                    dem_config_dict["ArPackages"], ext_data_record["DemDataElementClassRef"]["ValueRef"])
                if data_class is None:
                    continue
                data_class_name = ext_data_record["DemDataElementClassRef"]["ValueRef"].split("/")[-1]
                if data_class_name in dem_data_class:
                    data_class_ref_list.append(dem_data_class[data_class_name]["DataClassIndex"])
                    data_class_ref_number += 1
            elif isinstance(ext_data_record["DemDataElementClassRef"]["ValueRef"], list):
                for data_class_ref_path in ext_data_record["DemDataElementClassRef"]["ValueRef"]:
                    data_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"], data_class_ref_path)
                    if data_class is None:
                        continue
                    data_class_name = data_class_ref_path.split("/")[-1]
                    if data_class_name in dem_data_class:
                        data_class_ref_list.append(dem_data_class[data_class_name]["DataClassIndex"])
                        data_class_ref_number += 1
            else:
                #error
                continue
        dem_ext_data_record[ext_data_record_name] = ext_data_record
        dem_ext_data_record[ext_data_record_name][
            "DemExtendedDataRecordIndex"] = dem_ext_data_record_index
        dem_ext_data_record[ext_data_record_name]["DataClassRefStart"] = data_class_ref_start
        dem_ext_data_record[ext_data_record_name]["DataClassNum"] = data_class_ref_number
        dem_ext_data_record[ext_data_record_name]["DataClassRefList"] = data_class_ref_list
        data_class_ref_start += data_class_ref_number
        dem_ext_data_record_index += 1

    return dem_ext_data_record


def process_dem_ext_data_class(dem_config_dict: dict, dem_general_in_json: dict,
                                    dem_ext_data_record_list: dict) -> dict:
    dem_ext_data_class = {}
    dem_ext_data_class_index = 0

    for ext_data_class_name, ext_data_class in dem_general_in_json.items():
        if "DefinitionRef" not in ext_data_class or ext_data_class[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemExtendedDataClass":
            continue
        ext_data_record_class_ref_list = []
        if isinstance(ext_data_class["DemExtendedDataRecordClassRef"]["ValueRef"], str):
            ext_data_record_class = AutosarUtil.get_by_path_from_dict(
                dem_config_dict["ArPackages"], ext_data_class["DemExtendedDataRecordClassRef"]["ValueRef"])
            if ext_data_record_class is None:
                continue
            ext_data_record_class_name = ext_data_class["DemExtendedDataRecordClassRef"]["ValueRef"].split(
                "/")[-1]
            if ext_data_record_class_name in dem_ext_data_record_list:
                ext_data_record_class_ref_list.append(
                    dem_ext_data_record_list[ext_data_record_class_name]["DemExtendedDataRecordIndex"])
        elif isinstance(ext_data_class["DemExtendedDataRecordClassRef"]["ValueRef"], list):
            for ext_data_record_class_ref_path in ext_data_class["DemExtendedDataRecordClassRef"]["ValueRef"]:
                ext_data_record_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                                               ext_data_record_class_ref_path)
                if ext_data_record_class is None:
                    continue
                ext_data_record_class_name = ext_data_record_class_ref_path.split("/")[-1]
                if ext_data_record_class_name in dem_ext_data_record_list:
                    ext_data_record_class_ref_list.append(
                        dem_ext_data_record_list[ext_data_record_class_name]["DemExtendedDataRecordIndex"])
        else:
            #error
            continue
        dem_ext_data_class[ext_data_class_name] = ext_data_class
        dem_ext_data_class[ext_data_class_name][
            "ExtendedDataRecordClassRefList"] = ext_data_record_class_ref_list
        dem_ext_data_class[ext_data_class_name]["ExtendedDataRecordClassRefNum"] = len(
            ext_data_record_class_ref_list)
        dem_ext_data_class[ext_data_class_name]["ExtendedDataClassIndex"] = dem_ext_data_class_index
        dem_ext_data_class_index += 1

    return dem_ext_data_class


def process_dem_nvram_blockid(config_dict: dict, dem_general_in_json: dict) -> dict:
    dem_nvram_blockid = {}
    nvram_is_valid = True

    for _, nvram_blockid in dem_general_in_json.items():
        if "DefinitionRef" not in nvram_blockid or nvram_blockid[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemNvRamBlockId":
            continue
        nvram_blockid_ref = AutosarUtil.get_by_path_from_dict(
            config_dict, "/nvm/ArPackages" + nvram_blockid["DemNvRamBlockIdRef"]["ValueRef"])
        if nvram_blockid_ref is None:
            nvram_is_valid = False
            break
        else:
            nvram_blockid_name = "NvMConf_" + nvram_blockid["DemNvRamBlockIdRef"]["ValueRef"].split('/')[-1]
            dem_nvram_blockid[nvram_blockid_name] = {}
            dem_nvram_blockid[nvram_blockid_name]["NvRamBlockId"] = nvram_blockid_ref["NvMNvramBlockIdentifier"]["Value"]
            dem_nvram_blockid[nvram_blockid_name]["NvRamBlockLength"] = nvram_blockid_ref["NvMNvBlockLength"]["Value"]
            dem_nvram_blockid[nvram_blockid_name]["NvMRamBlockDataAddress"] = nvram_blockid_ref["NvMRamBlockDataAddress"]["Value"]

    # 如果所有的ref都无效，则不开启转储
    if nvram_is_valid is False:
        dem_nvram_blockid = {}
    return dem_nvram_blockid

def process_dem_did_pid_class_list(dem_config_dict: dict, dem_general_in_json: dict, dem_data_class: dict) -> dict:
    dem_did_pid_class_list = {}
    dem_did_pid_class_index = 0
    data_class_ref_start = 0
    DemMaxSizeOBDFFData = 0
    dem_did_pid_class_dict = {}

    # did
    for did_class_name, did_class in dem_general_in_json.items():
        if "DefinitionRef" not in did_class or did_class[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDidClass":
            continue

        data_class_ref_list = []
        data_class_ref_number = 0

        if "DemDidDataElementClassRef" in did_class is False:
            continue
        if isinstance(did_class["DemDidDataElementClassRef"]["ValueRef"], str):
            data_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                           did_class["DemDidDataElementClassRef"]["ValueRef"])
            if data_class is None:
                continue
            data_class_name = did_class["DemDidDataElementClassRef"]["ValueRef"].split("/")[-1]
            if data_class_name in dem_data_class:
                data_class_ref_list.append(dem_data_class[data_class_name]["DataClassIndex"])
                data_class_ref_number += 1
        elif isinstance(did_class["DemDidDataElementClassRef"]["ValueRef"], list):
            for data_class_ref_path in did_class["DemDidDataElementClassRef"]["ValueRef"]:
                data_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"], data_class_ref_path)
                if data_class is None:
                    continue
                data_class_name = data_class_ref_path.split("/")[-1]
                if data_class_name in dem_data_class:
                    data_class_ref_list.append(dem_data_class[data_class_name]["DataClassIndex"])
                    data_class_ref_number += 1
        else:
            #error
            continue
        dem_did_pid_class_list[did_class_name] = did_class
        dem_did_pid_class_list[did_class_name]["DemFFidClassIndex"] = dem_did_pid_class_index
        dem_did_pid_class_list[did_class_name]["DataClassRefStart"] = data_class_ref_start
        dem_did_pid_class_list[did_class_name]["DataClassNum"] = data_class_ref_number
        dem_did_pid_class_list[did_class_name]["DataClassRefList"] = data_class_ref_list
        data_class_ref_start += data_class_ref_number
        dem_did_pid_class_index += 1

    #pid
    dem_config_set = dem_get_dict_by_definition_ref(dem_config_dict['ArPackages']['ActiveEcuC']['Dem'], "DemConfigSet")
    for pid_class_name, pid_class in dem_config_set.items():
        if "DefinitionRef" not in pid_class or pid_class["DefinitionRef"] !=\
            "/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemPidClass":
            continue

        # pid 0 0x20 0x40 这些表示支持哪些pid的pid，以及只有$ 01 02服务才会用到的pid不需要放到OBD冻结帧里
        if pid_class["DemPidIdentifier"]["Value"] % 0x20 == 0 or pid_class["DemPidIdentifier"]["Value"] in [1, 2]:
            continue

        data_class_ref_list = []
        data_class_ref_number = 0
        for _, item in pid_class.items():
            if "DefinitionRef" not in item or item["DefinitionRef"] !=\
                '/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemPidClass/DemPidDataElement':
                continue

            if "DemPidDataElementClassRef" in pid_class['DemPidDataElement'] is False:
                continue

            if isinstance(pid_class['DemPidDataElement']["DemPidDataElementClassRef"]["ValueRef"], str):
                data_class = AutosarUtil.get_by_path_from_dict(
                    dem_config_dict["ArPackages"],
                    pid_class['DemPidDataElement']["DemPidDataElementClassRef"]["ValueRef"])
                if data_class is None:
                    continue
                DemMaxSizeOBDFFData += data_class["DemExternalCSDataElementClass"]["DemDataElementArraySize"]["Value"]
                data_class_name = item["DemPidDataElementClassRef"]["ValueRef"].split("/")[-1]
                if data_class_name in dem_data_class:
                    data_class_ref_list.append(dem_data_class[data_class_name]["DataClassIndex"])
                    data_class_ref_number += 1
        dem_did_pid_class_list[pid_class_name] = pid_class
        dem_did_pid_class_list[pid_class_name]['DemDidIdentifier'] = {}
        dem_did_pid_class_list[pid_class_name]['DemDidIdentifier'][
            'Value'] = pid_class['DemPidIdentifier']['Value']
        dem_did_pid_class_list[pid_class_name]["DemFFidClassIndex"] = dem_did_pid_class_index
        dem_did_pid_class_list[pid_class_name]["DataClassRefStart"] = data_class_ref_start
        dem_did_pid_class_list[pid_class_name]["DataClassNum"] = data_class_ref_number
        dem_did_pid_class_list[pid_class_name]["DataClassRefList"] = data_class_ref_list
        data_class_ref_start += data_class_ref_number
        dem_did_pid_class_index += 1

    dem_did_pid_class_dict = {
        "dem_did_pid_class_list": dem_did_pid_class_list,
        "DemMaxSizeOBDFFData": DemMaxSizeOBDFFData
    }

    return dem_did_pid_class_dict

def process_dem_general_freeze_frame_class(dem_config_dict: dict, dem_general_in_json: dict,
                                           dem_did_pid_class_list: dict) -> dict:
    dem_freeze_frame_class = {}

    # UDSFF
    dem_freeze_frame_class_index = 0
    freeze_frame_did_class_ref_index = 0

    for freeze_frame_class_name, freeze_frame_class in dem_general_in_json.items():
        if "DefinitionRef" not in freeze_frame_class or\
                freeze_frame_class["DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemFreezeFrameClass" or\
                "DemDidClassRef" not in freeze_frame_class:
            continue

        did_class_ref_list = []
        did_class_ref_number = 0

        if isinstance(freeze_frame_class["DemDidClassRef"]["ValueRef"], str):
            did_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                          freeze_frame_class["DemDidClassRef"]["ValueRef"])
            if did_class is None:
                continue
            did_class_name = freeze_frame_class["DemDidClassRef"]["ValueRef"].split("/")[-1]
            if did_class_name in dem_did_pid_class_list:
                did_class_ref_list.append(dem_did_pid_class_list[did_class_name]["DemFFidClassIndex"])
                did_class_ref_number += 1
        elif isinstance(freeze_frame_class["DemDidClassRef"]["ValueRef"], list):
            for did_class_ref_path in freeze_frame_class["DemDidClassRef"]["ValueRef"]:
                did_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"], did_class_ref_path)
                if did_class is None:
                    continue
                did_class_name = did_class_ref_path.split("/")[-1]
                if did_class_name in dem_did_pid_class_list:
                    did_class_ref_list.append(dem_did_pid_class_list[did_class_name]["DemFFidClassIndex"])
                    did_class_ref_number += 1
        else:
            #error
            continue
        dem_freeze_frame_class[freeze_frame_class_name] = freeze_frame_class
        dem_freeze_frame_class[freeze_frame_class_name]['Type'] = 'DEM_FREEZE_FRAME_NON_OBD'
        dem_freeze_frame_class[freeze_frame_class_name]["freezeframeClassIndex"] = dem_freeze_frame_class_index
        dem_freeze_frame_class[freeze_frame_class_name]["FFIdClassRefIndex"] = freeze_frame_did_class_ref_index
        dem_freeze_frame_class[freeze_frame_class_name]["FFIdClassRefList"] = did_class_ref_list
        dem_freeze_frame_class[freeze_frame_class_name]["FFIdClassRefNum"] = did_class_ref_number
        freeze_frame_did_class_ref_index = freeze_frame_did_class_ref_index + did_class_ref_number + 1
        dem_freeze_frame_class_index += 1

    #OBDFF
    pid_class_ref_list = []
    pid_class_valueref_list = []
    pid_class_ref_number = 0
    for name, did_pid in dem_did_pid_class_list.items():
        if did_pid['DefinitionRef'] != '/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemPidClass':
            continue
        pid_class_ref_list.append(did_pid['DemFFidClassIndex'])
        did_pid_name = '/ActiveEcuC/Dem/DemConfigSet/' + name
        pid_class_valueref_list.append(did_pid_name)
        pid_class_ref_number += 1
    if 0 != len(pid_class_ref_list):
        dem_freeze_frame_class['OBDFF'] = {}
        dem_freeze_frame_class['OBDFF']['Type'] = 'DEM_FREEZE_FRAME_OBD'
        dem_freeze_frame_class['OBDFF']['DemDidClassRef'] = {}
        dem_freeze_frame_class['OBDFF']["freezeframeClassIndex"] = dem_freeze_frame_class_index
        dem_freeze_frame_class['OBDFF']["FFIdClassRefIndex"] = freeze_frame_did_class_ref_index
        dem_freeze_frame_class['OBDFF']["FFIdClassRefList"] = pid_class_ref_list
        dem_freeze_frame_class['OBDFF']["FFIdClassRefNum"] = pid_class_ref_number
        if 1 == len(pid_class_ref_list):
            dem_freeze_frame_class['OBDFF']['DemDidClassRef']['ValueRef'] = pid_class_valueref_list[0]
        else:
            dem_freeze_frame_class['OBDFF']['DemDidClassRef']['ValueRef'] = []
            dem_freeze_frame_class['OBDFF']['DemDidClassRef']['ValueRef'] = pid_class_valueref_list

    return dem_freeze_frame_class


def process_dem_general_freeze_frame_record_class(dem_general_in_json: dict) -> dict:
    dem_freeze_frame_record_class = {}
    dem_freeze_frame_record_class["FreezeFrameTriggger"] = "DEM_TRIGGER_ON_TEST_FAILED"  # Default value

    for _, freeze_frame_record_class in dem_general_in_json.items():
        if "DefinitionRef" not in freeze_frame_record_class or freeze_frame_record_class[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemFreezeFrameRecordClass":
            continue

        dem_freeze_frame_record_class["FreezeFrameTriggger"] = freeze_frame_record_class["DemFreezeFrameRecordTrigger"][
            "Value"]

    return dem_freeze_frame_record_class


def process_dem_general_freeze_frame_recnum_class(dem_config_dict: dict, dem_general_in_json: dict) -> dict:
    dem_freeze_frame_recnum_class = {}
    dem_freeze_frame_recnum_class_index = 0

    for freeze_frame_recnum_class_name, freeze_frame_recnum_class in dem_general_in_json.items():
        if "DefinitionRef" not in freeze_frame_recnum_class or freeze_frame_recnum_class[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemFreezeFrameRecNumClass":
            continue

        freeze_frame_recnum_list = []
        freeze_frame_recnum_number = 0
        if isinstance(freeze_frame_recnum_class["DemFreezeFrameRecordClassRef"]["ValueRef"], str):
            freeze_frame_record_class = AutosarUtil.get_by_path_from_dict(
                dem_config_dict["ArPackages"],
                freeze_frame_recnum_class["DemFreezeFrameRecordClassRef"]["ValueRef"])
            if freeze_frame_record_class is not None and "DemFreezeFrameRecordNumber" in freeze_frame_record_class:
                freeze_frame_recnum_list.append(freeze_frame_record_class["DemFreezeFrameRecordNumber"]["Value"])
        elif isinstance(freeze_frame_recnum_class["DemFreezeFrameRecordClassRef"]["ValueRef"], list):
            for freeze_frame_recnum_class_ref_path in freeze_frame_recnum_class[
                    "DemFreezeFrameRecordClassRef"]["ValueRef"]:
                freeze_frame_record_class = AutosarUtil.get_by_path_from_dict(
                    dem_config_dict["ArPackages"], freeze_frame_recnum_class_ref_path)
                if freeze_frame_record_class is not None and "DemFreezeFrameRecordNumber" in freeze_frame_record_class:
                    freeze_frame_recnum_list.append(
                        freeze_frame_record_class["DemFreezeFrameRecordNumber"]["Value"])
        else:
            #error
            continue
        dem_freeze_frame_recnum_class[freeze_frame_recnum_class_name] = freeze_frame_recnum_class
        dem_freeze_frame_recnum_class[freeze_frame_recnum_class_name][
            "FreezeFrameRecordNumberList"] = freeze_frame_recnum_list
        dem_freeze_frame_recnum_class[freeze_frame_recnum_class_name][
            "FreezeFrameRecordNumberClassIndex"] = dem_freeze_frame_recnum_class_index
        dem_freeze_frame_recnum_class_index += 1
    return dem_freeze_frame_recnum_class


def process_dem_general_ratio(dem_config_dict:dict, dem_general_in_json: dict) -> dict:
    dem_ratio = {}
    return dem_ratio
    for ratio_name, ratio in dem_general_in_json.items():
        if "DefinitionRef" not in ratio or ratio[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemRatio":
            continue
        dem_ratio[ratio_name]["DemIUMPRDenGroup"] = ratio["DemIUMPRDenGroup"]["Value"]
        dem_ratio[ratio_name]["DemIUMPRGroup"] = ratio["DemIUMPRGroup"]["Value"]
        dem_ratio[ratio_name]["DemRatioKind"] = ratio["DemRatioKind"]["Value"]
        dem_ratio[ratio_name]["DemRatioId"] = ratio["DemRatioId"]["Value"]
        dem_event_parameter = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                    ratio["DemDiagnosticEventRef"]["ValueRef"])
        if dem_event_parameter is not None:
            dem_ratio[ratio_name]["EventId"] = dem_event_parameter["EventId"]["Value"]
        else:
            dem_ratio[ratio_name]["EventId"] = "DEM_EVENT_ID_NULL" # 非法值


    return dem_ratio


def process_dem_general_opcycles(dem_general_in_json: dict) -> dict:
    dem_opcycles = {}
    for opcycle_name, opcycle in dem_general_in_json.items():
        if "DefinitionRef" not in opcycle or opcycle[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemOperationCycle":
            continue
        dem_opcycles[opcycle_name] = opcycle
    return dem_opcycles

def Process_dem_general_OBD(dem_general_in_json: dict) -> dict:
    dem_general_OBDs = {}
    for general_OBD_name, general_OBD in dem_general_in_json.items():
        if "DefinitionRef" not in general_OBD or general_OBD[
            "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemGeneralOBD":
            continue
        dem_general_OBDs[general_OBD_name] = general_OBD
    return dem_general_OBDs

def get_dem_Mil_indicator_name(general_dict: dict) -> str:
    indicator_name = ""

    for event_memory_set_name, EventMemorySetItem in general_dict.items():
        if "DefinitionRef" not in EventMemorySetItem or EventMemorySetItem[
                "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemEventMemorySet":
            continue
        if "DemOBDSupport" not in general_dict or general_dict["DemOBDSupport"]["Value"] is None or general_dict[
                "DemOBDSupport"]["Value"] == "":
            pass
        elif general_dict["DemOBDSupport"]["Value"] == "DEM_OBD_MASTER_ECU" or general_dict["DemOBDSupport"][
                "Value"] == "DEM_OBD_PRIMARY_ECU":
            if "DemMILIndicatorRef" in EventMemorySetItem and EventMemorySetItem["DemMILIndicatorRef"][
                    "ValueRef"] is not None:
                indicator_name = EventMemorySetItem["DemMILIndicatorRef"]["ValueRef"].split('/')[-1]

    return indicator_name

def get_dem_odb_pfc_cycle(general_dict: dict) -> str:
    OBDPFC_cycle_ref = ""

    for general_OBD_name, general_OBD in general_dict.items():
        if "DefinitionRef" not in general_OBD or general_OBD[
            "DefinitionRef"] != "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemGeneralOBD":
            continue
        if "DemOBDSupport" not in general_dict or general_dict["DemOBDSupport"]["Value"] is None or general_dict[
                "DemOBDSupport"]["Value"] == "":
            pass
        elif general_dict["DemOBDSupport"]["Value"] == "DEM_OBD_MASTER_ECU" or general_dict["DemOBDSupport"][
                "Value"] == "DEM_OBD_PRIMARY_ECU":
            if "DemOBDPFCCycleRef" in general_OBD and general_OBD["DemOBDPFCCycleRef"][
                "ValueRef"] is not None:
                OBDPFC_cycle_ref = general_OBD["DemOBDPFCCycleRef"]["ValueRef"].split("/")[-1].upper()

    return OBDPFC_cycle_ref

def set_dem_event_config(config_dict, dem_general, dem_config_set):
    dem_config_dict = config_dict["dem"]
    dem_freeze_frame_class = dem_general["DemFreezeFrameClass"]
    dem_ext_data_records = dem_general["DemExtendedDataRecordList"]
    dem_event_parameters = dem_config_set["DemEventParameters"]

    dem_general["DemMemoryStatusBitBufferSize"] = int(len(dem_event_parameters) / 4 + 1)

    dem_general["DemMaxSizeFFData"] = get_dem_max_size_ff_data(dem_config_dict, dem_freeze_frame_class)
    dem_general["DemMaxSizeExtData"] = get_dem_max_size_ext_data(dem_config_dict, dem_ext_data_records)

    if len(dem_general["DemEnableConditions"]) > 0 and dem_general["DemEnableConditionUsed"] == "TRUE" and dem_general["DemEnableConditionGroupsUsed"] == "TRUE":
        dem_general["EnableConditionSupport"] = "STD_ON"
    else:
        dem_general["EnableConditionSupport"] = "STD_OFF"
    if len(dem_general["DemStorageConditions"]) > 0 and dem_general["DemStorageConditionUsed"] == "TRUE" and dem_general["DemStorageConditionGroupsUsed"] == "TRUE":
        dem_general["StorageConditionSupport"] = "STD_ON"
    else:
        dem_general["StorageConditionSupport"] = "STD_OFF"

    return


def get_dem_max_size_ext_data(dem_config_dict, dem_ext_data_records):
    dem_max_size_ext_data = 0
    for _, ext_data in dem_ext_data_records.items():
        data_size = 0
        if isinstance(ext_data["DemDataElementClassRef"]["ValueRef"], str):
            data_elem_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                                   ext_data["DemDataElementClassRef"]["ValueRef"])
            if data_elem_class is None:
                continue
            data_size += get_data_elem_size(data_elem_class)
        elif isinstance(ext_data["DemDataElementClassRef"]["ValueRef"], list):
            for data_elem_class_path in ext_data["DemDataElementClassRef"]["ValueRef"]:
                data_elem_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                                       data_elem_class_path)
                if data_elem_class is None:
                    continue
                data_size += get_data_elem_size(data_elem_class)
        if dem_max_size_ext_data < data_size:
            dem_max_size_ext_data = data_size
    return dem_max_size_ext_data


def get_dem_max_size_ff_data(dem_config_dict, dem_freeze_frame_class):
    dem_max_size_ff_data = 0

    for _, freeze_frame_class in dem_freeze_frame_class.items():
        data_size = 0
        if isinstance(freeze_frame_class["DemDidClassRef"]["ValueRef"], str): # 单个时为Str
            did_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                          freeze_frame_class["DemDidClassRef"]["ValueRef"])
            if did_class is None:
                continue
            if "DemDidDataElementClassRef" in did_class: # Did
                if isinstance(did_class["DemDidDataElementClassRef"]["ValueRef"], str):
                    data_elem_class = AutosarUtil.get_by_path_from_dict(
                        dem_config_dict["ArPackages"], did_class["DemDidDataElementClassRef"]["ValueRef"])
                    if data_elem_class is None:
                        continue
                    data_size += get_data_elem_size(data_elem_class)
                elif isinstance(did_class["DemDidDataElementClassRef"]["ValueRef"], list):
                    for data_elem_class_path in did_class["DemDidDataElementClassRef"]["ValueRef"]:
                        data_elem_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                                            data_elem_class_path)
                        if data_elem_class is None:
                            continue
                        data_size += get_data_elem_size(data_elem_class)
            # else: #Pid
            #     for _, data_elem_class_path in did_class.items():
            #         if isinstance(data_elem_class_path, int):
            #             continue
            #         if "DefinitionRef" not in data_elem_class_path or data_elem_class_path['DefinitionRef'] != \
            #             '/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemPidClass/DemPidDataElement':
            #             continue
            #         data_elem_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
            #                                                             data_elem_class_path['DemPidDataElementClassRef']["ValueRef"])
            #         if data_elem_class is None:
            #             continue
            #         data_size += get_data_elem_size(data_elem_class)

        elif isinstance(freeze_frame_class["DemDidClassRef"]["ValueRef"], list): # 多个时为List
            for did_class_ref_path in freeze_frame_class["DemDidClassRef"]["ValueRef"]:
                did_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"], did_class_ref_path)
                if did_class is None:
                    continue
                if "DemDidDataElementClassRef" in did_class: # Did
                    if isinstance(did_class["DemDidDataElementClassRef"]["ValueRef"], str):
                        data_elem_class = AutosarUtil.get_by_path_from_dict(
                            dem_config_dict["ArPackages"], did_class["DemDidDataElementClassRef"]["ValueRef"])
                        if data_elem_class is None:
                            continue
                        data_size += get_data_elem_size(data_elem_class)
                    elif isinstance(did_class["DemDidDataElementClassRef"]["ValueRef"], list):
                        for data_elem_class_path in did_class["DemDidDataElementClassRef"]["ValueRef"]:
                            data_elem_class = AutosarUtil.get_by_path_from_dict(dem_config_dict["ArPackages"],
                                                                                   data_elem_class_path)
                            if data_elem_class is None:
                                continue
                            data_size += get_data_elem_size(data_elem_class)
                # else: # Pid
                #     for _, data_elem_class_path in did_class.items():
                #         if isinstance(data_elem_class_path, int):
                #             continue
                #         if "DefinitionRef" not in data_elem_class_path or data_elem_class_path['DefinitionRef'] != \
                #             '/AUTOSAR/EcucDefs/Dem/DemConfigSet/DemPidClass/DemPidDataElement':
                #             continue
                #         data_elem_class = AutosarUtil.get_by_path_from_dict(
                #             dem_config_dict["ArPackages"],
                #             data_elem_class_path['DemPidDataElementClassRef']["ValueRef"])
                #         if data_elem_class is None:
                #             continue
                #         data_size += get_data_elem_size(data_elem_class)

        if dem_max_size_ff_data < data_size:
            dem_max_size_ff_data = data_size
    return dem_max_size_ff_data


def get_data_elem_size(data_elem_class: dict):
    data_size = 0
    for key, value in data_elem_class.items():
        if "DefinitionRef" in value and value[
                "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalCSDataElementClass":
            data_size = value["DemDataElementArraySize"]["Value"]
        elif "DefinitionRef" in value and value[
                "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemExternalSRDataElementClass":
            data_size = value["DemDataElementArraySize"]["Value"]
        elif "DefinitionRef" in value and value[
                "DefinitionRef"] == "/AUTOSAR/EcucDefs/Dem/DemGeneral/DemDataElementClass/DemInternalDataElementClass":
            data_size = value["DemDataElementDataSize"]["Value"]
    return data_size

def get_dem_application_id_name(os_core_config:dict, module_list:list, name:str, application_value:dict):
    if "CONFIG_MODULE_E3640_GW" in module_list:
        ApplicationName = name.split("_")[0]
    else:
        ApplicationName = name

    os_core_id = os_core_config['Os_Core' + application_value["OsApplicationCoreRef"]["ValueRef"][-1]]["CoreId"][-1]
    if "System" in name:
        # system application
        applicaton_id_name = "OS_CORE" + os_core_id + "_SYSTEMAPPLICATION" + "_CORE" + os_core_id + "_ID"
    else:
        # user application
        applicaton_id_name = "OS_CORE" + os_core_id  + "_" + ApplicationName + "_CORE" + os_core_id + "_ID"
    return applicaton_id_name

def process_dem_general_satellite(os_core_config:dict, module_list:list, dem_general:dict, dem_event_parameters:dict, config_dict:dict):
    dem_satellite = {}
    start_index = 0

    for event_parameter in dem_event_parameters.values():
        application_ref = event_parameter["DemOsApplicationRef"]
        if application_ref is not None and application_ref != "" and application_ref != "<empty>":
            application_name = application_ref.split("/")[-1]
            application_value = AutosarUtil.get_by_path_from_dict(config_dict["os"]["ArPackages"], application_ref)
        else:
            # event parameter没有配置DemOsApplicationRef时使用Master的DemMasterOsApplicationRef
            application_name = dem_general["DemMasterApplicationName"]
            application_value = dem_general["DemMasterApplicationValue"]
        event_parameter["DemApplicationIdName"] = application_name
        event_parameter["DemApplicationIdValue"] = application_value
        application_id = application_value
        if application_name != "DEM_DEFAULT_SINGLE_SATELLITE_ID":
            application_id = get_dem_application_id_name(os_core_config, module_list, application_name, application_value)
        event_parameter["DemApplicationId"] = application_id
        event_parameter["DemSatellitId"] = application_name
        event_id = int(event_parameter["EventId"])
        if application_name not in dem_satellite.keys():
            dem_satellite[application_name] = {
                "ApplicationId": application_id,
                "EventCount": 1,
                "EventList": [event_id]
            }
            dem_satellite[application_name]["EventCount"] = 1
            dem_satellite[application_name]["EventList"] = [event_id]
        else:
            dem_satellite[application_name]["EventCount"] += 1
            dem_satellite[application_name]["EventList"].append(event_id)
        event_parameter["SatelliteEventIdx"] = dem_satellite[application_name]["EventCount"] - 1

    for satellite in dem_satellite.values():
        satellite["EventTableIndStartIdx"] = start_index
        start_index += satellite["EventCount"]

    dem_general["DemSatellite"] = dem_satellite
    return

def set_application_id_name(config_dict:dict, dem_dict:dict, os_core_config:dict, module_list:list):
    dem_general = dem_dict["DemGeneral"]
    dem_event_parameters = dem_dict["DemConfigSet"]["DemEventParameters"]
    master_application_name = dem_general["DemMasterApplicationName"]
    master_application_value = dem_general["DemMasterApplicationValue"]

    application_id = master_application_value
    if master_application_name != "DEM_DEFAULT_SINGLE_SATELLITE_ID":
        application_id = get_dem_application_id_name(os_core_config, module_list, master_application_name, master_application_value)
        dem_general["DemMasterCoreId"] = os_core_config['Os_Core' + master_application_value["OsApplicationCoreRef"]["ValueRef"][-1]]["CoreId"][-1]

    dem_general["DemMasterApplicationId"] = application_id
    process_dem_general_satellite(os_core_config, module_list, dem_general, dem_event_parameters, config_dict)

# transfer core information to predefined format
def Os_CoreInfo_Get(os_dict: dict, ecuc_dict: dict):
    os_dict['OsCores'] = {}
    HW_Platform = 'INITAL_HW'
    # Verify hardware consistency
    for k, v in ecuc_dict["EcucHardware"].items():
        if k != 'DefinitionRef':
            if HW_Platform == 'INITAL_HW' and v['EcucCoreHwRef']['ValueRef']:
                HW_Platform = v['EcucCoreHwRef']['ValueRef'].split('/')[3]
            elif HW_Platform != v['EcucCoreHwRef']['ValueRef'].split('/')[3]:
                # logger.info("Inconsistent hardware coreid mapping!!!")
                break
    hw_coreinfo = os_dict["ArPackages"]["ActiveEcuC"]["Os"][HW_Platform]["OsDerivativeInformation"]
    for k, v in ecuc_dict["EcucHardware"].items():
        if k != 'DefinitionRef':
            os_dict['OsCores']['Os_Core' + str(v['EcucCoreId']['Value'])] = \
            {\
                'CoreId': 'OS_CORE_ID_' + str(v['EcucCoreId']['Value']), \
                'PhyId' : 'OS_CORE_PHYSICAL_ID_' + str(hw_coreinfo[v['EcucCoreHwRef']['ValueRef'].split('/')[-1]]["OsPhysicalCoreId"]["Value"])      \
            }
    for k, v in hw_coreinfo.items():
        if k != 'DefinitionRef':
            if (v['DefinitionRef'].split('/')[-1]
                    == 'OsPhysicalCore') and "OsPhysicalCoreId" in v and v['OsPhysicalCoreId']['Value'] is not None:
                os_dict['HWCores'][k] = {'CoreId': v['OsPhysicalCoreId']['Value']}
