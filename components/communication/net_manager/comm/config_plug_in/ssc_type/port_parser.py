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

from . import filter
import hashlib
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
import copy


# Functions
def dict_to_arxml_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):
    if not isinstance(cur_dict, dict):
        return
    if "Tag" in cur_dict and cur_dict["Tag"] == "CONTAINER-GROUP":
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)
            parent_dict.update({container_name: container_dict})
        del parent_dict[cur_key_name]
    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            dict_to_arxml_dict(container_dict, container_name, cur_dict, cur_key_name)


def _add_contianer_group_in_dict(container_name: str, container_group_name: str, parent_dict: dict, cur_dict: dict):
    if container_group_name in parent_dict:
        parent_dict[container_group_name].update({container_name: cur_dict})
    else:
        comm_container_group_dict = {"Tag": "CONTAINER-GROUP"}
        comm_container_group_dict.update({container_name: cur_dict})
        parent_dict.update({container_group_name: comm_container_group_dict})
    del parent_dict[container_name]


def arxml_dict_to_dict(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "ComMChannel":
        _add_contianer_group_in_dict(cur_key_name, "ComMChannels", parent_dict, cur_dict)

    elif cur_item_definition_ref_val == "ComMUser":
        _add_contianer_group_in_dict(cur_key_name, "ComMUsers", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            arxml_dict_to_dict(container_dict, container_name, cur_dict, cur_key_name)


def child_convert(cur_dict: dict, cur_key_name: str, parent_dict: dict, parent_key_name: str):

    if not isinstance(cur_dict, dict):
        return
    if "DefinitionRef" not in cur_dict:
        return
    cur_item_definition_ref_val = cur_dict["DefinitionRef"].split("/")[-1]

    if cur_item_definition_ref_val == "ComMUserPerChannel":
        _add_contianer_group_in_dict(cur_key_name, "ComMUserPerChannels", parent_dict, cur_dict)

    else:
        for container_name, container_dict in list(cur_dict.items()):
            if container_name == "Tag" or container_name == "DefinitionRef":
                continue
            child_convert(container_dict, container_name, cur_dict, cur_key_name)


def check_enabled(enabled):
    if isinstance(enabled, str):
        if enabled == "true" or enabled == "TRUE":
            return True
    return False


def get_value_from_dict_by_key(input_dict, input_key):
    output_dict = {}
    for key, value in input_dict.items():
        if key == input_key:
            output_dict = value
    return output_dict


def update_short_name_length(short_name) -> str:
    if short_name and len(short_name) > 128:
        hash_obj = hashlib.sha1(short_name[120:].encode(encoding="utf-8"))
        hash_str = hash_obj.hexdigest()
        short_name = short_name[:120] + hash_str[:8]
    return short_name


def get_key_name(key_name: str) -> str:
    key_name = key_name.split("_")[-1]
    return key_name


class PortDerive(object):

    def __init__(self, ecucs_autosar_dict=None):
        self.ecucs_autosar_dict = ecucs_autosar_dict
        self.init_comm_config()
        self.update_data_types()
        # self.update_mode_declaration_to_date()
        self.update_main_function_runnable()
        self.update_server_runnables()
        self.update_invoked_events()
        self.config_module_dict = {}
        self.set_config_dict()

    def init_comm_config(self):
        self.mode_declaration_groups = [{
            "short_name": "ComMMode",  # 8.7.6.1
            "initial_value": "COMM_NO_COMMUNICATION",
            "modes": ["COMM_FULL_COMMUNICATION", "COMM_NO_COMMUNICATION", "COMM_SILENT_COMMUNICATION"]
        }]
        self.data_types = {
            "elements": [
                {
                    "short_name": "ComM_InhibitionStatusType",  # 8.7.4.1
                    "category": "Type",
                    "type_emitter": None,
                    "sw_data_def": {
                        "data_constr_ref": "ComM_InhibitionStatusType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "ComM_ModeType",  # 8.7.4.2
                    "category": "Type",
                    "type_emitter": None,
                    "sw_data_def": {
                        "compu_method_ref": "ComM_ModeType",
                        "data_constr_ref": "ComM_ModeType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "ComM_UserHandleType",  # 8.7.4.3
                    "category": "Type",
                    "type_emitter": None,
                    "sw_data_def": {
                        "data_constr_ref": "ComM_UserHandleType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                },
                {
                    "short_name": "NetworkHandleType",
                    "category": "Type",
                    "type_emitter": "ComM",
                    "sw_data_def": {
                        "data_constr_ref": "NetworkHandleType_Constr",
                        "IsType": False,
                        "implementation_data_type": "uint8",
                    }
                }
            ],
            "data_constrs": {
                "elements": [{
                    "short_name": "ComM_InhibitionStatusType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 3,
                        }
                    }]
                }, {
                    "short_name": "ComM_ModeType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 2,
                        }
                    }]
                }, {
                    "short_name": "ComM_UserHandleType_Constr",
                    "data_constr_rules": [{
                        "internal_consters": {
                            "lower_limit": 0,
                            "upper_limit": 255,
                        }
                    }]
                }],
            },
            "compu_methods": {
                "elements": [{
                    "short_name":
                        "ComM_ModeType",
                    "category":
                        "TEXTTABLE",
                    "compu_scales": [{
                        "short_label": "COMM_NO_COMMUNICATION",
                        "lower_limit": 0,
                        "upper_limit": 0,
                    }, {
                        "short_label": "COMM_SILENT_COMMUNICATION",
                        "lower_limit": 1,
                        "upper_limit": 1,
                    }, {
                        "short_label": "COMM_FULL_COMMUNICATION",
                        "lower_limit": 2,
                        "upper_limit": 2,
                    }]
                }],
            }
        }
        self.mode_switch_interfaces = [{
            "short_name": "ComM_CurrentMode",  # 8.7.3.1
            "is_server": True,
            "mode_group": "ComMMode",
            "prototype": "currentMode"
        }]
        self.sender_receiver_interfaces = []
        self.client_server_interfaces = [{
            "short_name":
                "ComM_UserRequest",  # 8.7.2.4
            "is_server":
                True,
            "port_type":
                "pport",
            "possible_errors": [{
                "short_name": "E_OK",
                "error_code": 0
            }, {
                "short_name": "E_NOT_OK",
                "error_code": 1
            }, {
                "short_name": "E_MODE_LIMITATION",
                "error_code": 2
            }],
            "operations": [{
                "short_name": "GetCurrentComMode",
                "arguments": [{
                    "short_name": "ComMode",
                    "IsType": True,
                    "TypeTref": "ComM_ModeType",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "short_name": "GetMaxComMode",
                "arguments": [{
                    "short_name": "ComMode",
                    "IsType": True,
                    "TypeTref": "ComM_ModeType",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "short_name": "GetRequestedComMode",
                "arguments": [{
                    "short_name": "ComMode",
                    "IsType": True,
                    "TypeTref": "ComM_ModeType",
                    "Direction": "OUT",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK"]
            }, {
                "short_name": "RequestComMode",
                "arguments": [{
                    "short_name": "ComMode",
                    "IsType": True,
                    "TypeTref": "ComM_ModeType",
                    "Direction": "IN",
                }],
                "possible_errors": ["E_OK", "E_NOT_OK", "E_MODE_LIMITATION"]
            }]
        }]
        self.pports = []
        self.rports = []
        self.main_function_runnable = []
        self.server_runnables = []
        self.operation_invoked_events = []

    def update_data_types(self):
        comm_module = self.ecucs_autosar_dict["ActiveEcuC"]["ComM"]
        comm_general = comm_module["ComMGeneral"]
        if "ComMChannels" in comm_module["ComMConfigSet"]:
            comm_channels = comm_module["ComMConfigSet"]["ComMChannels"]
        else:
            comm_channels = {"Tag": 0}
        if "ComMUsers" in comm_module["ComMConfigSet"]:
            comm_users = comm_module["ComMConfigSet"]["ComMUsers"]
        else:
            comm_users = {"Tag": 0}

        # Client-Server-interfaces
        if comm_general["ComMModeLimitationEnabled"]:
            channel_limitation = {
                "short_name":
                    "ComM_ChannelLimitation",  # 8.8.3.7
                "is_server":
                    True,
                "port_type":
                    "pport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }],
                "operations": [
                    {
                        "short_name": "GetInhibitionStatus",
                        "arguments": [{
                            "short_name": "Status",
                            "IsType": True,
                            "TypeTref": "ComM_InhibitionStatusType",
                            "Direction": "OUT",
                        }],
                        "possible_errors": ["E_OK", "E_NOT_OK"]
                    },
                    {
                        "short_name": "LimitChannelToNoComMode",
                        "arguments": [{
                            "short_name": "Status",
                            "IsType": False,
                            "TypeTref": "boolean",
                            "Direction": "IN",
                        }],
                        "possible_errors": ["E_OK", "E_NOT_OK"]
                    },
                ]
            }
            ecu_limitation = {
                "short_name":
                    "ComM_ECUModeLimitation",  # 8.8.3.11
                "is_server":
                    True,
                "port_type":
                    "pport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }],
                "operations": [{
                    "short_name": "LimitECUToNoComMode",
                    "arguments": [{
                        "short_name": "Status",
                        "IsType": False,
                        "TypeTref": "boolean",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "ReadInhibitCounter",
                    "arguments": [{
                        "short_name": "CounterValue",
                        "IsType": False,
                        "TypeTref": "uint16",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "ResetInhibitCounter",
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "SetECUGroupClassification",
                    "arguments": [{
                        "short_name": "Status",
                        "IsType": True,
                        "TypeTref": "ComM_InhibitionStatusType",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }]
            }
            self.client_server_interfaces.append(channel_limitation)
            self.client_server_interfaces.append(ecu_limitation)

        if comm_general["ComMWakeupInhibitionEnabled"]:
            channel_wakeup = {
                "short_name":
                    "ComM_ChannelWakeup",  # 8.8.3.11
                "is_server":
                    True,
                "port_type":
                    "pport",
                "possible_errors": [{
                    "short_name": "E_OK",
                    "error_code": 0
                }, {
                    "short_name": "E_NOT_OK",
                    "error_code": 1
                }],
                "operations": [{
                    "short_name": "GetInhibitionStatus",
                    "arguments": [{
                        "short_name": "Status",
                        "IsType": True,
                        "TypeTref": "ComM_InhibitionStatusType",
                        "Direction": "OUT",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }, {
                    "short_name": "PreventWakeUp",
                    "arguments": [{
                        "short_name": "Status",
                        "IsType": False,
                        "TypeTref": "boolean",
                        "Direction": "IN",
                    }],
                    "possible_errors": ["E_OK", "E_NOT_OK"]
                }]
            }
            self.client_server_interfaces.append(channel_wakeup)

        # P-Ports
        if comm_general["ComMModeLimitationEnabled"]:
            mode_limitation = {
                "short_name": "modeLimitation",  # 8.8.5.8
                "interface_type": "client_server",
                "interface_ref": "ComM_ECUModeLimitation",
                "argument": None
            }
            self.pports.append(mode_limitation)

        for channel_name, channel_value in list(comm_channels.items()):
            if channel_name != "Tag":

                # data types
                array_element = {
                    "short_name": "ComM_UserHandleArrayType_" + channel_name,  # 8.8.1.7
                    "category": "Structure",
                    "type_emitter": None,
                    "sw_data_def": {
                        "calibration_access": "READ-ONLY"
                    },
                    "sub_element": {
                        "numberOfRequesters": "uint8",
                        "handleArray": "ComM_UserHandleArrayType_" + channel_name
                    }
                }
                subarray_element = {
                    "short_name": "ComM_UserHandleSubArrayType_" + channel_name,  # 8.8.1.7
                    "category": "Array",
                    "type_emitter": None,
                    "sw_data_def": {
                        "calibration_access": "READ-ONLY"
                    },
                    "sub_element": {
                        "element_type": "uint8",
                        "array_size": 1
                    }
                }
                self.data_types["elements"].append(array_element)
                self.data_types["elements"].append(subarray_element)

                # Sender-Receiver-interfaces
                if channel_value["ComMFullCommRequestNotificationEnabled"]["Value"]:
                    sender_receiver_element = {
                        "short_name":
                            "ComM_CurrentChannelRequest_" + channel_name,  # 8.7.1.1
                        "is_server":
                            True,
                        "data_elements": [{
                            "short_name": "fullComRequestors",
                            "calibration_access": "READ-ONLY",
                            "type": "ComM_UserHandleArrayType_" + channel_name
                        }]
                    }
                    self.sender_receiver_interfaces.append(sender_receiver_element)

                # P-Ports
                if comm_general["ComMModeLimitationEnabled"]:
                    comm_cl = {
                        "short_name": "CL_" + channel_name,  # 8.7.5.1
                        "interface_type": "client_server",
                        "interface_ref": "ComM_ChannelLimitation",
                        "argument": {
                            "type": "NetworkHandleType",
                            "value": channel_value["ComMChannelId"]["Value"]
                        }
                    }
                    self.pports.append(comm_cl)

                if channel_value["ComMFullCommRequestNotificationEnabled"]["Value"]:
                    comm_cr = {
                        "short_name": "CR_" + channel_name,  # 8.7.5.2
                        "interface_type": "sender_receiver",
                        "interface_ref": "ComM_CurrentChannelRequest_" + channel_name,
                        "argument": None
                    }
                    self.pports.append(comm_cr)

                if comm_general["ComMWakeupInhibitionEnabled"]:
                    comm_cw = {
                        "short_name": "CW_" + channel_name,  # 8.7.5.3
                        "interface_type": "client_server",
                        "interface_ref": "ComM_ChannelWakeup",
                        "argument": {
                            "type": "NetworkHandleType",
                            "value": channel_value["ComMChannelId"]["Value"]
                        }
                    }
                    self.pports.append(comm_cw)

        for user_name, user_value in list(comm_users.items()):
            if user_name != "Tag":
                pports_user_element = [
                    {
                        "short_name": "UM_" + user_name,  # 8.7.5.5
                        "interface_type": "mode_switch",
                        "interface_ref": "ComM_CurrentMode",
                        "argument": None
                    },
                    {
                        "short_name": "UR_" + user_name,  # 8.7.5.6
                        "interface_type": "client_server",
                        "interface_ref": "ComM_UserRequest",
                        "argument": {
                            "type": "ComM_UserHandleType",
                            "value": user_value["ComMUserIdentifier"]["Value"]
                        }
                    },
                ]
                for element in pports_user_element:
                    self.pports.append(element)

    def update_main_function_runnable(self):
        if "ComMChannels" in self.ecucs_autosar_dict["ActiveEcuC"]["ComM"]["ComMConfigSet"]:
            comm_channels = self.ecucs_autosar_dict["ActiveEcuC"]["ComM"]["ComMConfigSet"]["ComMChannels"]
        else:
            comm_channels = {"Tag": 0}
        for channel_name, channel_value in list(comm_channels.items()):
            if channel_name != "Tag":
                mainfunction_element = {
                    "short_name": "ComM_MainFunction_" + channel_name,
                    "period": channel_value["ComMMainFunctionPeriod"]["Value"],
                    "mode_switch_points": [],  # mode_switch p
                    "server_call_points": [],  # cs r
                    "data_send_points": [],  # sr p
                    "data_receive_points": []  # sr r
                }
                self.main_function_runnable.append(mainfunction_element)

        for pport in self.pports:
            #mode-switch-points
            if pport["interface_type"] == "mode_switch":
                for sw_interface in self.mode_switch_interfaces:
                    if pport["interface_ref"] == sw_interface["short_name"]:
                        for main_function in self.main_function_runnable:
                            main_function["mode_switch_points"].append({
                                "short_name":
                                    update_short_name_length("SWITCH_" + pport["short_name"] + "_" +
                                                             sw_interface["mode_group"]),
                                "port_ref":
                                    pport["short_name"],
                                "interface_ref":
                                    sw_interface["short_name"],
                                "prototype_ref":
                                    sw_interface["prototype"]
                            })
            #sender_receiver-points
            elif pport["interface_type"] == "sender_receiver":
                for sr_interface in self.sender_receiver_interfaces:
                    if pport["interface_ref"] == sr_interface["short_name"]:
                        for main_function in self.main_function_runnable:
                            if get_key_name(main_function["short_name"]) == get_key_name(pport["short_name"]):
                                for data in sr_interface["data_elements"]:
                                    main_function["data_send_points"].append({
                                        "short_name":
                                            update_short_name_length("SEND_" + pport["short_name"] + "_" +
                                                                     data["type"]),
                                        "port_ref":
                                            pport["short_name"],
                                        "interface_ref":
                                            sr_interface["short_name"],
                                        "data":
                                            data
                                    })

    def update_server_runnables(self):
        for cs_interface in self.client_server_interfaces:
            if cs_interface["port_type"] == "rport":
                continue
            for operation in cs_interface["operations"]:
                element = {"short_name": operation["short_name"], "symbol": "ComM_" + operation["short_name"]}

                if element not in self.server_runnables:
                    self.server_runnables.append(element)

    def update_invoked_events(self):  # cs p
        for pport in self.pports:
            for cs_interface in self.client_server_interfaces:
                if pport["interface_ref"] == cs_interface["short_name"]:
                    for operation in cs_interface["operations"]:
                        self.operation_invoked_events.append({
                            "short_name":
                                update_short_name_length("OpEvent" + operation["short_name"] + "_" +
                                                         operation["short_name"] + "_" + pport["short_name"]),
                            "start_on_event_ref":
                                operation["short_name"],
                            "pport_ref":
                                pport["short_name"],
                            "interface_ref":
                                cs_interface["short_name"],
                            "operation_ref":
                                operation["short_name"]
                        })

    def set_config_dict(self):
        self.config_module_dict = {
            "comm_package": {
                "short_name": "ComM_swc",
                "mode_declaration": self.mode_declaration_groups,
                "datatypes_pkg": self.data_types,
                "interfaces_pkg": {
                    "mode_switch": self.mode_switch_interfaces,
                    "sender_receiver": self.sender_receiver_interfaces,
                    "client_server": self.client_server_interfaces
                },
                "componenttypes_pkg": {
                    "pports": self.pports,
                    "rports": self.rports,
                    "main_function": self.main_function_runnable,
                    "server_runnables": self.server_runnables,
                    "operation_events": self.operation_invoked_events
                }
            }
        }

    def get_config_dict(self):
        return self.config_module_dict

    def client_server_interfaces_append(self, input_dict):
        if input_dict:
            self.client_server_interfaces.append(input_dict)

    def mode_switch_interfaces_append(self, input_dict):
        if input_dict:
            self.mode_switch_interfaces.append(input_dict)

    def pports_append(self, input_dict):
        if input_dict:
            self.pports.append(input_dict)

    def rports_append(self, input_dict):
        if input_dict:
            self.rports.append(input_dict)


def get_config_dict(module_ecuc_cfg):
    comm_module_cfg = module_ecuc_cfg["ComM"] if "ComM" in module_ecuc_cfg else None
    if not comm_module_cfg:
        return {}
    comm_ecuc_cfg_dict = AutosarUtil.to_bsw_dict(comm_module_cfg)
    # tmp add
    ######################################################################################
    dict_config = copy.deepcopy(comm_ecuc_cfg_dict["ArPackages"])

    child_convert(dict_config["ActiveEcuC"]["ComM"]["ComMConfigSet"], "ComMConfigSet",
                  dict_config["ActiveEcuC"]["ComM"], "ComM")
    arxml_dict_to_dict(dict_config["ActiveEcuC"]["ComM"]["ComMConfigSet"], "ComMConfigSet",
                       dict_config["ActiveEcuC"]["ComM"], "ComM")
    ######################################################################################
    # print(dict_config)
    comm_ecuc_dict = filter.get_template_input_parameter_dict(dict_config)
    port_derive = PortDerive(comm_ecuc_dict)
    return port_derive.get_config_dict()
