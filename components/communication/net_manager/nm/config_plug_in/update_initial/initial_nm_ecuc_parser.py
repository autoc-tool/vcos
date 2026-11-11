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
from basic_func_module.autosar_utils.utils import *
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from func_module.health_monitor.logger import logger

class InitialNmEcucParser():

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict

    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, cfg_dict):
        self.update_nm_cfg_dbc(initial_autosar, cfg_dict)
        self.update_nm_cfg(initial_autosar, cfg_dict)

    def update_nm_cfg(self, initial_autosar, cfg_dict):
        nm_cfg_dict = self.get_template_config_json_dict(cfg_dict)
        self.update_nm_general_tree(initial_autosar, nm_cfg_dict)
        self.update_nm_global_config_tree(initial_autosar, nm_cfg_dict)

    def update_nm_cfg_dbc(self, initial_autosar, cfg_dict):
        nm_cfg_dict_dbc = self.get_template_config_json_dict_dbc(cfg_dict)
        self.update_nm_global_config_tree_dbc(initial_autosar, nm_cfg_dict_dbc)

    def update_nm_general_tree(self, initial_autosar, cfg_dict):
        pass

    def update_nm_global_config_tree(self, initial_autosar, cfg_dict):

        module_name = "Nm"
        parent_short_name = "NmChannelConfig"
        if module_name in self.mod_autosar_dict:
            ecuc_def = self.mod_autosar_dict[module_name]

        parent = initial_autosar.find(f"/InitialEcuC/{module_name}")
        path = get_definition_ref(parent)
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/Nm/NmChannelConfig")
        for ldf, ldf_item in cfg_dict["nm_channel_config"].items():
            channel_name = f"{ldf}"
            channel_value = AutosarUtil.create_sub_container_value(channel_name, cur_conf_def, parent)

            fill_value_dict = {
                "referance_values": {
                    '/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmComMChannelRef':
                        "/ActiveEcuC/ComM/ComMConfigSet/" + channel_name,
                }
            }
            self.fill_prameter_and_referance(channel_value, fill_value_dict)

            bus_type_ref = "/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmBusType"
            bus_conf_def = ecuc_def.find(bus_type_ref)
            bus_type = AutosarUtil.create_sub_container_value("NmBusType", bus_conf_def, channel_value)
            bus_dict = {}
            self.fill_prameter_and_referance(bus_type, bus_dict)

            bus_typevalue_ref = "/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmBusType/NmStandardBusNmConfig"
            bus_conf_def = ecuc_def.find(bus_typevalue_ref)
            bus_typevalue = AutosarUtil.create_sub_container_value("NmStandardBusNmConfig", bus_conf_def, bus_type)
            busvalue_dict = {
                "textual_prameter_values": {
                    '/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmBusType/NmStandardBusNmConfig/NmStandardBusType':
                        "NM_BUSNM_LINNM",
                }
            }
            self.fill_prameter_and_referance(bus_typevalue, busvalue_dict)

    def update_nm_global_config_tree_dbc(self, initial_autosar, cfg_dict):

        module_name = "Nm"
        parent_short_name = "NmChannelConfig"
        if module_name in self.mod_autosar_dict:
            ecuc_def = self.mod_autosar_dict[module_name]

        parent = initial_autosar.find(f"/InitialEcuC/{module_name}")
        path = get_definition_ref(parent)
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/Nm/NmChannelConfig")
        for dbc, dbc_item in cfg_dict["nm_channel_config"].items():
            channel_name = f"{dbc}"
            channel_value = AutosarUtil.create_sub_container_value(channel_name, cur_conf_def, parent)

            fill_value_dict = {
                "referance_values": {
                    '/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmComMChannelRef':
                        "/ActiveEcuC/ComM/ComMConfigSet/" + channel_name,
                },
            }
            self.fill_prameter_and_referance(channel_value, fill_value_dict)

            bus_type_ref = "/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmBusType"
            bus_conf_def = ecuc_def.find(bus_type_ref)
            bus_type = AutosarUtil.create_sub_container_value("NmBusType", bus_conf_def, channel_value)
            bus_dict = {}
            self.fill_prameter_and_referance(bus_type, bus_dict)

            bus_typevalue_ref = "/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmBusType/NmStandardBusNmConfig"
            bus_conf_def = ecuc_def.find(bus_typevalue_ref)
            bus_typevalue = AutosarUtil.create_sub_container_value("NmStandardBusNmConfig", bus_conf_def, bus_type)
            busvalue_dict = {
                "textual_prameter_values": {
                    '/AUTOSAR/EcucDefs/Nm/NmChannelConfig/NmBusType/NmStandardBusNmConfig/NmStandardBusType':
                        "NM_BUSNM_CANNM",
                }
            }
            self.fill_prameter_and_referance(bus_typevalue, busvalue_dict)

    def nm_channel_config_filter(self, config_dict: dict) -> dict:
        id = 0
        channel_config_output = {}
        for ldf in config_dict:
            channel_config = {}
            channel_config.update({"channel_id": id})
            id += 1
            channel_config_output.update({ldf["channel_name"]: channel_config})
        return channel_config_output

    def nm_channel_config_filter_dbc(self, config_dict: dict) -> dict:
        id = 0
        channel_config_output = {}
        for dbc in config_dict:
            for key, value in dbc["details"].items():
                if "NmAsrMessage" in value.message_attributes and value.message_attributes["NmAsrMessage"].value == 1:
                    channel_config = {}
                    channel_config.update({"channel_id": id})
                    id += 1
                    channel_config_output.update(
                        {dbc["attributes"]["network"]["define_value"]["DBName"]: channel_config})
                    break
                else:
                    continue
        return channel_config_output

    def nm_channel_table_filter(self, config_dict: dict) -> dict:
        sys_channel_index_start = 3
        id = 0
        channel_table_output = {}
        for ldf in config_dict:
            channel_table = {}
            channel_table.update({"lin_channel_index": id})
            channel_table.update({"sys_channel_index": sys_channel_index_start})
            id += 1
            sys_channel_index_start += 1
            channel_table_output.update({ldf["ldf_name"]: channel_table})
        return channel_table_output

    def get_template_config_json_dict(self, config_dict: dict) -> dict:
        nm_config_json_dict = {}
        ldf_config = config_dict["ldf"]
        nm_config_json_dict.update({"nm_number_of_channels": len(ldf_config)})
        nm_config_json_dict.update({"nm_channel_config": self.nm_channel_config_filter(ldf_config)})
        nm_config_json_dict.update({"nm_channel_table": self.nm_channel_table_filter(ldf_config)})

        return nm_config_json_dict

    def get_template_config_json_dict_dbc(self, config_dict: dict) -> dict:
        nm_config_json_dict = {}
        dbc_config = config_dict["dbc"]
        nm_config_json_dict.update({"nm_number_of_channels": len(dbc_config)})
        nm_config_json_dict.update({"nm_channel_config": self.nm_channel_config_filter_dbc(dbc_config)})        
        logger.debug(nm_config_json_dict)
        return nm_config_json_dict

    def get_template_dynamic_code_dict(config_dict: dict) -> dict:
        return config_dict

    def fill_prameter_and_referance(self, container_value, fill_value_dict):
        """
        Fill Parameters
        """

        if hasattr(container_value.REFERENCE_VALUES, "ECUC_REFERENCE_VALUE"):
            for para in container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
                def_ref = get_definition_ref(para)
                for definition in fill_value_dict["referance_values"].keys():
                    if def_ref == definition:
                        set_value_ref(para, fill_value_dict["referance_values"][definition])
                        AutosarUtil.add_annotation_value(para, "AutoDerived")

        if hasattr(container_value.PARAMETER_VALUES, "ECUC_TEXTUAL_PARAM_VALUE"):
            for para in container_value.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE:
                def_ref = get_definition_ref(para)
                for definition in fill_value_dict["textual_prameter_values"].keys():
                    if def_ref == definition:
                        set_value(para, fill_value_dict["textual_prameter_values"][definition])
                        AutosarUtil.add_annotation_value(para, "AutoDerived")

        return container_value
