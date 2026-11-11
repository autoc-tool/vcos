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


class InitialComMEcucParser():

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict
        self.index = 0

    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, cfg_dict):
        self.index = 0
        self.update_comm_cfg_dbc(initial_autosar, cfg_dict)

    def update_comm_cfg(self, initial_autosar, cfg_dict):
        comm_cfg_dict = self.get_template_config_json_dict(cfg_dict)
        self.update_comm_general_tree(initial_autosar, comm_cfg_dict)
        self.update_comm_global_config_tree(initial_autosar, comm_cfg_dict)

    def update_comm_cfg_dbc(self, initial_autosar, cfg_dict):
        comm_cfg_dict_dbc = self.get_template_config_json_dict_dbc(cfg_dict)
        self.update_comm_global_config_tree_dbc(initial_autosar, comm_cfg_dict_dbc)

    def update_comm_general_tree(self, initial_autosar, cfg_dict):
        pass

    def update_comm_global_config_tree(self, initial_autosar, cfg_dict):

        module_name = "ComM"
        parent_short_name = "ComMConfigSet"
        if module_name in self.mod_autosar_dict:
            ecuc_def = self.mod_autosar_dict[module_name]

        # ecuc_autosar = self.ecuc_autosar_dict[module_name]
        # parent = ecuc_autosar.find(f"/ActiveEcuC/{module_name}/" + parent_short_name)
        parent = initial_autosar.find(f"/InitialEcuC/{module_name}/" + parent_short_name)
        path = get_definition_ref(parent)
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel")
        cur_conf_def_user = ecuc_def.find("/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMUser")
        for ldf, ldf_item in cfg_dict["comm_channel_config"].items():
            fill_value_dict = {
                "textual_prameter_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMBusType': "COMM_BUS_TYPE_LIN",
                },
                "numerical_prameter_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMChannelId': str(self.index),
                },
                "referance_values": {},
            }

            fill_value_dict_user = {
                "numerical_prameter_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMUser/ComMUserIdentifier': str(self.index),
                },
                "referance_values": {},
            }

            channel_name = f"{ldf}"
            user_name = f"ComMUser_{ldf}"
            channel_value = AutosarUtil.create_sub_container_value(channel_name, cur_conf_def, parent)
            user_value = AutosarUtil.create_sub_container_value(user_name, cur_conf_def_user, parent)
            self.fill_prameter_and_referance(channel_value, fill_value_dict)
            self.fill_prameter_and_referance(user_value, fill_value_dict_user)

            # 创建子container ComMUserPerChannel
            fill_value_dict_userperchannel = {
                "referance_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMUserPerChannel/ComMUserChannel':
                        "/ActiveEcuC/ComM/ComMConfigSet/" + user_name,
                },
            }
            userperchannel_ref = "/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMUserPerChannel"
            userperchannel_conf_def = ecuc_def.find(userperchannel_ref)
            userperchannel = AutosarUtil.create_sub_container_value("ComMUserPerChannel", userperchannel_conf_def,
                                                                    channel_value)
            self.fill_prameter_and_referance(userperchannel, fill_value_dict_userperchannel)

            # 创建子container ComMNetworkManagement
            management_ref = "/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMNetworkManagement"
            management_conf_def = ecuc_def.find(management_ref)
            management = AutosarUtil.create_sub_container_value("ComMNetworkManagement", management_conf_def,
                                                                channel_value)

            # channel数加1
            self.index += 1

    def update_comm_global_config_tree_dbc(self, initial_autosar, cfg_dict):

        module_name = "ComM"
        parent_short_name = "ComMConfigSet"
        if module_name in self.mod_autosar_dict:
            ecuc_def = self.mod_autosar_dict[module_name]

        # ecuc_autosar = self.ecuc_autosar_dict[module_name]
        # parent = ecuc_autosar.find(f"/ActiveEcuC/{module_name}/" + parent_short_name)
        parent = initial_autosar.find(f"/InitialEcuC/{module_name}/" + parent_short_name)
        path = get_definition_ref(parent)
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel")
        cur_conf_def_user = ecuc_def.find("/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMUser")
        for can_channel, dbc_item in cfg_dict["comm_channel_config"].items():
            fill_value_dict = {
                "textual_prameter_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMBusType': "COMM_BUS_TYPE_CAN",
                },
                "numerical_prameter_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMChannelId': str(self.index),
                },
                "referance_values": {},
            }

            fill_value_dict_user = {
                "numerical_prameter_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMUser/ComMUserIdentifier': str(self.index),
                },
                "referance_values": {},
            }

            channel_name = f"{can_channel}"
            user_name = f"ComMUser_{can_channel}"
            channel_value = AutosarUtil.create_sub_container_value(channel_name, cur_conf_def, parent)
            user_value = AutosarUtil.create_sub_container_value(user_name, cur_conf_def_user, parent)
            self.fill_prameter_and_referance(channel_value, fill_value_dict)
            self.fill_prameter_and_referance(user_value, fill_value_dict_user)

            # 创建子container ComMUserPerChannel
            fill_value_dict_userperchannel = {
                "referance_values": {
                    '/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMUserPerChannel/ComMUserChannel':
                        "/ActiveEcuC/ComM/ComMConfigSet/" + user_name,
                },
            }
            userperchannel_ref = "/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMUserPerChannel"
            userperchannel_conf_def = ecuc_def.find(userperchannel_ref)
            userperchannel = AutosarUtil.create_sub_container_value("ComMUserPerChannel", userperchannel_conf_def,
                                                                    channel_value)
            self.fill_prameter_and_referance(userperchannel, fill_value_dict_userperchannel)

            # 创建子container ComMNetworkManagement
            management_ref = "/AUTOSAR/EcucDefs/ComM/ComMConfigSet/ComMChannel/ComMNetworkManagement"
            management_conf_def = ecuc_def.find(management_ref)
            management = AutosarUtil.create_sub_container_value("ComMNetworkManagement", management_conf_def,
                                                                channel_value)

            # channel数加1
            self.index += 1

    def comm_channel_config_filter(self, config_dict: dict) -> dict:
        id = 0
        channel_config_output = {}
        for ldf in config_dict:
            channel_config = {}
            channel_config.update({"channel_id": id})
            id += 1
            channel_config_output.update({ldf["channel_name"]: channel_config})
        print(channel_config_output)
        return channel_config_output

    def comm_channel_config_filter_dbc(self, config_dict: dict) -> dict:
        id = 0
        channel_config_output = {}
        for dbc in config_dict:
            channel_config = {}
            channel_config.update({"channel_id": id})
            id += 1
            channel_config_output.update({dbc["attributes"]["network"]["define_value"]["DBName"]: channel_config})
        print(channel_config_output)
        return channel_config_output

    def comm_channel_table_filter(self, config_dict: dict) -> dict:
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
        comm_config_json_dict = {}
        ldf_config = config_dict["ldf"]
        comm_config_json_dict.update({"comm_number_of_channels": len(ldf_config)})
        comm_config_json_dict.update({"comm_channel_config": self.comm_channel_config_filter(ldf_config)})
        comm_config_json_dict.update({"comm_channel_table": self.comm_channel_table_filter(ldf_config)})

        return comm_config_json_dict

    def get_template_config_json_dict_dbc(self, config_dict: dict) -> dict:
        comm_config_json_dict = {}
        dbc_config = config_dict["dbc"]
        comm_config_json_dict.update({"comm_number_of_channels": len(dbc_config)})
        comm_config_json_dict.update({"comm_channel_config": self.comm_channel_config_filter_dbc(dbc_config)})

        return comm_config_json_dict

    def get_template_dynamic_code_dict(config_dict: dict) -> dict:
        return config_dict

    def fill_prameter_and_referance(self, container_value, fill_value_dict):
        """
        Fill Parameters
        """
        if hasattr(container_value.PARAMETER_VALUES, "ECUC_NUMERICAL_PARAM_VALUE"):
            for para in container_value.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
                def_ref = get_definition_ref(para)
                for definition in fill_value_dict["numerical_prameter_values"].keys():
                    if def_ref == definition:
                        set_value(para, fill_value_dict["numerical_prameter_values"][definition])
                        AutosarUtil.add_annotation_value(para, "AutoDerived")

        if hasattr(container_value.PARAMETER_VALUES, "ECUC_TEXTUAL_PARAM_VALUE"):
            for para in container_value.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE:
                def_ref = get_definition_ref(para)
                for definition in fill_value_dict["textual_prameter_values"].keys():
                    if def_ref == definition:
                        set_value(para, fill_value_dict["textual_prameter_values"][definition])
                        AutosarUtil.add_annotation_value(para, "AutoDerived")

        if hasattr(container_value.REFERENCE_VALUES, "ECUC_REFERENCE_VALUE"):
            for para in container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
                def_ref = get_definition_ref(para)
                for definition in fill_value_dict["referance_values"].keys():
                    if def_ref == definition:
                        set_value_ref(para, fill_value_dict["referance_values"][definition])
                        AutosarUtil.add_annotation_value(para, "AutoDerived")

        return container_value
