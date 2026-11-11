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
os模板内函数定义模块.

定义jinja2模板中所使用的函数.

用法示例:
{{get_core_id_list()}}
"""

from jinja2 import pass_context
from func_module.generator.common.common import FunctionBase


def get_group_id_from_group_dict(dict):
    return int(dict["DemGroupDTCs"]["Value"])


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """

    @staticmethod
    @pass_context
    def get_dem_json_config(context):
        dem_json_config = context["dem_json"]
        return dem_json_config

    @staticmethod
    @pass_context
    def get_dem_config_set_config(context):
        dem_config_set_config = context["dem_json"]["DemConfigSet"]
        return dem_config_set_config

    @staticmethod
    @pass_context
    def get_dem_debounce_classes_config(context):
        dem_debounce_classes_config = context["dem_json"]["DemConfigSet"]["DemDebounceClasses"]
        return dem_debounce_classes_config

    @staticmethod
    @pass_context
    def get_dem_counter_based_classes_config(context):
        dem_counter_based_class_config = context["dem_json"]["DemConfigSet"]["DemCounterBasedDebounces"]
        return dem_counter_based_class_config

    @staticmethod
    @pass_context
    def get_dem_time_based_classes_config(context):
        dem_time_based_class_config = context["dem_json"]["DemConfigSet"]["DemTimeBasedDebouces"]
        return dem_time_based_class_config

    @staticmethod
    @pass_context
    def get_dem_dtc_classes_config(context):
        dem_dtc_classes_client_config = context["dem_json"]["DemConfigSet"]["DemDTCClasses"]
        return dem_dtc_classes_client_config

    @staticmethod
    @pass_context
    def get_dem_event_classes_config(context):
        dem_event_classes_client_config = context["dem_json"]["DemConfigSet"]["DemEventClasses"]
        return dem_event_classes_client_config

    @staticmethod
    @pass_context
    def get_dem_event_parameters_config(context):
        dem_event_parameters_config = context["dem_json"]["DemConfigSet"]["DemEventParameters"]
        return dem_event_parameters_config

    @staticmethod
    @pass_context
    def get_dem_nvram_block_num_config(context):
        dem_general_config = context["dem_json"]["DemGeneral"]
        dem_nvram_block_num_config = dem_general_config["DemMaxNumberEventEntryPrimary"] + dem_general_config[
            "DemMaxNumberEventEntrySecondary"]
        return dem_nvram_block_num_config

    @staticmethod
    @pass_context
    def get_dem_indicator_attribute_groups_config(context):
        dem_indicators_config = context["dem_json"]["DemConfigSet"]["DemIndicatorAttributeGroups"]
        return dem_indicators_config

    @staticmethod
    @pass_context
    def get_dem_dtc_group_list_config(context):
        dem_dtc_group_list_config = context["dem_json"]["DemConfigSet"]["DemDTCGroup"]
        list_before_sort = dem_dtc_group_list_config
        # dem_dtc_group_list_config_after_sort = {k: v for k, v in sorted(list_before_sort.items(), key=lambda item: item["DemGroupDTCs"]["Value"])}
        # dem_dtc_group_list_config_after_sort = sorted(list_before_sort, key=get_group_id_from_group_dict)
        # for groupName, groupItem in dem_dtc_group_list_config_before_sort.items():
        #     groupItem
        dem_dtc_group_list_config_after_sort = sorted(list_before_sort.items(),
                                                      key=lambda i: int(i[1]["DemGroupDTCs"]["Value"]))
        return dem_dtc_group_list_config_after_sort

    @staticmethod
    @pass_context
    def get_dem_clear_dtc_notification_start_list_config(context):
        return context["dem_json"]["DemConfigSet"]["DemClearDTCNotification"]["DemClearDTCNotification_start"]

    @staticmethod
    @pass_context
    def get_dem_clear_dtc_notification_finish_list_config(context):
        return context["dem_json"]["DemConfigSet"]["DemClearDTCNotification"]["DemClearDTCNotification_finish"]

    @staticmethod
    @pass_context
    def get_dem_general_config(context):
        dem_general_config = context["dem_json"]["DemGeneral"]
        return dem_general_config

    @staticmethod
    @pass_context
    def get_dem_client_config(context):
        dem_client_config = context["dem_json"]["DemGeneral"]["DemClient"]['clients']
        return dem_client_config

    @staticmethod
    @pass_context
    def get_dem_dtc_status_changed_cbk_config(context):
        dem_cbk_config = context["dem_json"]["DemGeneral"]["DemClient"]['cbks']
        return dem_cbk_config

    @staticmethod
    @pass_context
    def get_dem_enable_conditions_config(context):
        dem_enable_conditions_config = context["dem_json"]["DemGeneral"]["DemEnableConditions"]
        return dem_enable_conditions_config

    @staticmethod
    @pass_context
    def get_dem_enable_condition_groups_config(context):
        dem_enable_condition_groups_config = context["dem_json"]["DemGeneral"]["DemEnableConditionGroups"]
        return dem_enable_condition_groups_config

    @staticmethod
    @pass_context
    def get_dem_storage_conditions_config(context):
        dem_storage_conditions_config = context["dem_json"]["DemGeneral"]["DemStorageConditions"]
        return dem_storage_conditions_config

    @staticmethod
    @pass_context
    def get_dem_storage_condition_groups_config(context):
        dem_storage_condition_groups_config = context["dem_json"]["DemGeneral"]["DemStorageConditionGroups"]
        return dem_storage_condition_groups_config

    @staticmethod
    @pass_context
    def get_dem_event_memory_set_config(context):
        dem_event_memory_set_config = context["dem_json"]["DemGeneral"]["DemEventMemorySet"]
        return dem_event_memory_set_config

    @staticmethod
    @pass_context
    def get_dem_indicators_config(context):
        dem_indicators_config = context["dem_json"]["DemGeneral"]["DemIndicators"]
        return dem_indicators_config

    @staticmethod
    @pass_context
    def get_dem_indicator_attributes_config(context):
        dem_indicator_attributes_config = context["dem_json"]["DemGeneral"]["DemIndicatorAttributes"]
        return dem_indicator_attributes_config

    @staticmethod
    @pass_context
    def get_dem_data_classes_config(context):
        dem_data_classes_config = context["dem_json"]["DemGeneral"]["DemDataClass"]
        return dem_data_classes_config

    @staticmethod
    @pass_context
    def get_dem_ext_data_record_config(context):
        dem_ext_data_record_config = context["dem_json"]["DemGeneral"]["DemExtendedDataRecordList"]
        return dem_ext_data_record_config

    @staticmethod
    @pass_context
    def get_dem_ext_data_class_config(context):
        dem_ext_data_class_config = context["dem_json"]["DemGeneral"]["DemExtendedDataClass"]
        return dem_ext_data_class_config

    @staticmethod
    @pass_context
    def get_dem_nvram_blockid_config(context):
        dem_nvram_blockid_config = context["dem_json"]["DemGeneral"]["DemNvRamBlockId"]
        return dem_nvram_blockid_config

    @staticmethod
    @pass_context
    def get_dem_did_pid_class_config(context):
        dem_did_class_config = context["dem_json"]["DemGeneral"]["DemDidPidClass"]
        return dem_did_class_config

    @staticmethod
    @pass_context
    def get_dem_freeze_frame_class_config(context):
        dem_freeze_frame_class_config = context["dem_json"]["DemGeneral"]["DemFreezeFrameClass"]
        return dem_freeze_frame_class_config

    @staticmethod
    @pass_context
    def get_dem_obd_freeze_frame_class_config(context):
        FFIdClassRefIndex = 0xFFFFFFFF
        dem_freeze_frame_class_config = context["dem_json"]["DemGeneral"]["DemFreezeFrameClass"]
        for ff_name, ff in dem_freeze_frame_class_config.items():
            if ff['Type'] == 'DEM_FREEZE_FRAME_OBD':
                FFIdClassRefIndex = ff['freezeframeClassIndex']
        return FFIdClassRefIndex

    @staticmethod
    @pass_context
    def get_dem_freeze_frame_recnum_class_config(context):
        dem_freeze_frame_recnum_class_config = context["dem_json"]["DemGeneral"]["DemFreezeFrameRecNumClass"]
        return dem_freeze_frame_recnum_class_config

    @staticmethod
    @pass_context
    def get_dem_freeze_frame_record_class_config(context):
        dem_freeze_frame_record_class_config = context["dem_json"]["DemGeneral"]["DemFreezeFrameRecordClass"]
        return dem_freeze_frame_record_class_config

    @staticmethod
    @pass_context
    def get_dem_nvm_blocks_config(context):
        dem_nvm_blocks_config = context["dem_json"]["DemGeneral"]["DemNvRamBlockId"]
        return dem_nvm_blocks_config

    @staticmethod
    @pass_context
    def get_dem_ext_data_max_nof_record_config(context):
        dem_ext_data_max_nof_record_config = 0
        dem_ext_data_class_config = context["dem_json"]["DemGeneral"]["DemExtendedDataClass"]
        for _, ext_data_class in dem_ext_data_class_config.items():
            if ext_data_class["ExtendedDataRecordClassRefNum"] > dem_ext_data_max_nof_record_config:
                dem_ext_data_max_nof_record_config = ext_data_class["ExtendedDataRecordClassRefNum"]
        return dem_ext_data_max_nof_record_config

    @staticmethod
    @pass_context
    def get_dem_freeze_frame_max_nof_did_config(context):
        dem_freeze_frame_max_nof_did_config = 0
        dem_freeze_frame_class_config = context["dem_json"]["DemGeneral"]["DemFreezeFrameClass"]
        for _, freeze_frame_class in dem_freeze_frame_class_config.items():
            if freeze_frame_class["FFIdClassRefNum"] > dem_freeze_frame_max_nof_did_config:
                dem_freeze_frame_max_nof_did_config = freeze_frame_class["FFIdClassRefNum"]
        return dem_freeze_frame_max_nof_did_config

    @staticmethod
    @pass_context
    def get_dem_obd_freeze_frame_max_nof_pid_config(context):
        dem_freeze_frame_max_nof_pid_config = 0
        dem_freeze_frame_class_config = context["dem_json"]["DemGeneral"]["DemFreezeFrameClass"]
        for freeze_frame_class_name, freeze_frame_class in dem_freeze_frame_class_config.items():
            if freeze_frame_class_name != 'OBDFF':
                continue
            if freeze_frame_class["FFIdClassRefNum"] > dem_freeze_frame_max_nof_pid_config:
                dem_freeze_frame_max_nof_pid_config = freeze_frame_class["FFIdClassRefNum"]
        return dem_freeze_frame_max_nof_pid_config

    @staticmethod
    @pass_context
    def get_dem_freeze_frame_max_nof_record_config(context):
        dem_freeze_frame_max_nof_record_config = 0
        dem_event_parameters_config = context["dem_json"]["DemConfigSet"]["DemEventParameters"]
        for _, event_parameter in dem_event_parameters_config.items():
            if event_parameter["MaxNumberFreezeFrameRecords"] > dem_freeze_frame_max_nof_record_config:
                dem_freeze_frame_max_nof_record_config = event_parameter["MaxNumberFreezeFrameRecords"]
        return dem_freeze_frame_max_nof_record_config

    @staticmethod
    @pass_context
    def get_dem_ext_data_max_record_num_config(context):
        dem_ext_data_max_record_num = 0
        dem_ext_data_record_config = context["dem_json"]["DemGeneral"]["DemExtendedDataRecordList"]
        for _, ext_data_record in dem_ext_data_record_config.items():
            if ext_data_record["DemExtendedDataRecordNumber"]["Value"] > dem_ext_data_max_record_num:
                dem_ext_data_max_record_num = ext_data_record["DemExtendedDataRecordNumber"]["Value"]
        return dem_ext_data_max_record_num

    @staticmethod
    @pass_context
    def get_dem_last_event_id_config(context):
        dem_last_event_id_config = 0
        dem_event_parameters_config = context["dem_json"]["DemConfigSet"]["DemEventParameters"]
        for _, event_parameter in dem_event_parameters_config.items():
            if event_parameter["EventId"] > dem_last_event_id_config:
                dem_last_event_id_config = event_parameter["EventId"]
        return dem_last_event_id_config

    @staticmethod
    @pass_context
    def get_dem_opcycles_config(context):
        dem_opcycles_config = context["dem_json"]["DemGeneral"]["DemOperationCycle"]
        return dem_opcycles_config

    @staticmethod
    @pass_context
    def get_dem_satellite_config(context):
        dem_satellite_config = context["dem_json"]["DemGeneral"]["DemSatellite"]
        return dem_satellite_config

    @staticmethod
    @pass_context
    def get_dem_satellite_event_id_type_config(context):
        dem_satellite_event_max = 0
        dem_satellite_config = context["dem_json"]["DemGeneral"]["DemSatellite"]
        for satellite_name, satellite in dem_satellite_config.items():
            if dem_satellite_event_max < len(satellite["EventList"]):
                dem_satellite_event_max = len(satellite["EventList"])
        if dem_satellite_event_max <= 255:
            dem_satellite_event_id_type = "uint8"
        else:
            dem_satellite_event_id_type = "uint16"
        return dem_satellite_event_id_type

    @staticmethod
    @pass_context
    def get_dem_event_available_data_length_config(context):
        dem_event_availiable_data_length = 0
        dem_event_parameters_config = context["dem_json"]["DemConfigSet"]["DemEventParameters"]
        dem_event_availiable_data_length = int((len(dem_event_parameters_config) - 1)/8 + 1)
        return dem_event_availiable_data_length


    @staticmethod
    @pass_context
    def get_dem_callback_file_config(context):
        dem_callback_file_config = 0
        dem_event_parameters_config = context["dem_json"]["DemConfigSet"]["DemEventParameters"]
        for _, event_parameter in dem_event_parameters_config.items():
            if event_parameter["CallbackClearEventAllowed"] != "NULL":
                dem_callback_file_config += 1

        dem_cbk_config = context["dem_json"]["DemGeneral"]["DemClient"]['cbks']
        if len(dem_cbk_config) > 0:
            dem_callback_file_config += 1

        if len(context["dem_json"]["DemConfigSet"]["DemClearDTCNotification"]["DemClearDTCNotification_finish"]) > 0:
            dem_callback_file_config += 1

        if len(context["dem_json"]["DemConfigSet"]["DemClearDTCNotification"]["DemClearDTCNotification_start"]) > 0:
            dem_callback_file_config += 1

        return dem_callback_file_config

    @staticmethod
    @pass_context
    def get_dem_event_combined_groups_config(context):
        dem_event_combined_groups_config = context["dem_json"]["DemConfigSet"]["DemEventCombinedGroups"]
        combined_event_id_index = 0
        for dem_event_combined_group in dem_event_combined_groups_config.values():
            dem_event_combined_group["StartCombinedEventIdIndex"] = combined_event_id_index
            combined_event_id_index += len(dem_event_combined_group["EventList"])
            dem_event_combined_group["EndCombinedEventIdIndex"] = combined_event_id_index
        return dem_event_combined_groups_config

    @staticmethod
    @pass_context
    def get_dem_mil_indicator_id_config(context):
        MIL_indicator_id = "DEM_INVALID_INDICATOR"
        dem_event_memory_set_config = context["dem_json"]["DemGeneral"]["DemEventMemorySet"]
        for event_memory_set_name, event_memory_set in dem_event_memory_set_config.items():
            if event_memory_set["MILIndicatorId"]["Value"] != "DEM_INVALID_INDICATOR":
                MIL_indicator_id = event_memory_set["MILIndicatorId"]["Value"]
        return MIL_indicator_id

    @staticmethod
    @pass_context
    def get_dem_master_memmap_tag_str(context):
        dem_master_task_mapping_info = context["dem_json"]["DemGeneral"]["DemMasterTaskMappingInfo"]
        if dem_master_task_mapping_info is None:
            return ""
        return dem_master_task_mapping_info["DemMasterApplication"].upper() + "_"

    @staticmethod
    @pass_context
    def get_dem_master_core(context):
        return context["dem_json"]["DemGeneral"]["DemMasterCoreId"]