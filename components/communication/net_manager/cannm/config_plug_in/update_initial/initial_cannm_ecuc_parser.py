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


class InitialCanNmEcucParser():
    """
    All CanNm module page jump, display, save logic code.
    """

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict

    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, dict):
        """
        Update the tree structure of the CanNm module based on the imported DBC file and update the data.
        Arg:
            dict : All config dictionary
        """
        self.update_the_cannm_tree(initial_autosar, dict)

    def cannm_cannm_channel_config_by_dbc(self, messages, initial_autosar):
        """
        cannm ipdu update data by dbc.
        Arg:
            message : All properties of message, including name, length, etc
            message_name : message name
        """
        # CanNmMsgCycleOffset/CanNmMsgCycleTime/CanNmMsgReducedTime/CanNmRepeatMessageTime/CanNmTimeoutTime/CanNmWaitBusSleepTime/CanNmNodeId
        if 0 == len(messages):
            return

        create_sub_container_and_fill_value_prameter_dict = {
            "module_name": "CanNm",
            "parent_short_name": "CanNmGlobalConfig",
            "short_name": "CanNmChannelConfig",
            "container_name": messages[0]["bus_name"] + "_CAN",
            "fill_values": {},
        }

        fill_value_dict_channel_config = {
            # container_value.REFERENCE_VALUES.ECUC_ADD_INFO_PARAM_VALUE
            "add_info_prameter_values": {},
            # container_value.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
            "numerical_prameter_values": {},
            # container_value.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
            "textual_prameter_values": {},
            # container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
            "referance_values": {
                '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmComMNetworkHandleRef':
                    "/ActiveEcuC/ComM/ComMConfigSet/" + messages[0]["bus_name"],
            },
            # container_value.REFERENCE_VALUES.ECUC_INSTANCE_REFERENCE_VALUE
            "instance_referance_values": {}
        }
        create_sub_container_and_fill_value_prameter_dict["fill_values"] = fill_value_dict_channel_config
        create_sub_container_and_fill_value(self, create_sub_container_and_fill_value_prameter_dict, initial_autosar,
                                            messages)

    def update_cannm_channel_cfgs(self, initial_autosar):
        """
        update the cannm tree and init tx/rx pdu data.
        """
        for comm_diag_name, value in self.cannm_pdus_json_config.items():
            self.cannm_cannm_channel_config_by_dbc(value, initial_autosar)

    def remove_cannm_base_mainfunction_in_initial(self, initial_autosar):
        CanNmGlobalConfigSet = initial_autosar.find("/InitialEcuC/CanNm/CanNmGlobalConfig").SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        for ele in reversed(CanNmGlobalConfigSet):
            if get_definition_ref(ele) == "/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmMainFunction":
                AutosarUtil.remove_container_value(CanNmGlobalConfigSet, ele)

    def update_cannm_mainfunction_cfgs(self, initial_autosar, cannm_mainfunction_config_json_dict):
        self.remove_cannm_base_mainfunction_in_initial(initial_autosar)
        parent = initial_autosar.find("/InitialEcuC/CanNm/CanNmGlobalConfig")
        if (len(cannm_mainfunction_config_json_dict) == 1):
            mod_autosar_obj = self.mod_autosar_dict["CanNm"]
            modules_def = mod_autosar_obj.find("/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmMainFunction")
            cannm_mainfunction_name = "CanNm_MainFunction"
            modules_container = AutosarUtil.create_sub_container_value(cannm_mainfunction_name, modules_def, parent)
        else:
            for cannm_mainfunction_key, cannm_mainfunction_value in cannm_mainfunction_config_json_dict.items():
                mod_autosar_obj = self.mod_autosar_dict["CanNm"]
                modules_def = mod_autosar_obj.find("/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmMainFunction")
                cannm_mainfunction_name = "CanNm_MainFunction" + "_Core" + str(cannm_mainfunction_value)
                modules_container = AutosarUtil.create_sub_container_value(cannm_mainfunction_name, modules_def, parent)

    def update_the_cannm_tree(self, initial_autosar, dict):
        """
        update the cannm tree by DBC.
        Arg:
            config_dict : All config dictionary
        """
        cannm_config_json_dict = get_template_config_json_dict(dict)
        cannm_mainfunction_config_json_dict = get_template_mainfunction_config_json_dict(dict)
        can_controller_list = get_canif_can_controller(cannm_config_json_dict)
        can_controller_num = get_canif_can_controller_num(cannm_config_json_dict)
        self.cannm_pdus_json_config = get_cannm_json_config(cannm_config_json_dict)
        self.update_cannm_channel_cfgs(initial_autosar)
        self.update_cannm_mainfunction_cfgs(initial_autosar, cannm_mainfunction_config_json_dict)

def get_template_mainfunction_config_json_dict(config_dict: dict) -> dict:
    """
    Get input parameter for template.
    Arg:
        config_dict : All config dictionary

    Return:
        Returns a dictionary of template input parameter
    """
    coreid_num = 0
    template_mainfunction_config_json_dict = {}
    template_mainfunction_config_dict = {}
    coreId = 0
    for dbc in config_dict["dbc"]:
        if "coreId" in dbc:
            if dbc["coreId"] == None:
                coreId = 0
            else:
                coreId = dbc["coreId"]
        template_mainfunction_config_json_dict[coreid_num] = coreId
        coreid_num = coreid_num + 1
    unique_values = sorted(set(template_mainfunction_config_json_dict.values()))
    template_mainfunction_config_dict = {index: value for index, value in enumerate(unique_values)}
    return template_mainfunction_config_dict

def get_template_config_json_dict(config_dict: dict) -> dict:
    """
    Get input parameter for template.
    Arg:
        config_dict : All config dictionary

    Return:
        Returns a dictionary of template input parameter
    """
    rx_number = 0
    tx_number = 0
    cannm_config_json_dict = {}
    can_controller_list = []
    for dbc in config_dict["dbc"]:
        node_selected = dbc['node_selected']
        config_json_dict = []
        for msg_key, msg_value in dbc["details"].items():
            msg_dict = {}
            msg_dict["comm_diag_type"] = "dbc"
            msg_dict["message_name"] = msg_value.message_name
            msg_dict["bus_name"] = msg_value.bus_name
            if msg_value.bus_name not in can_controller_list:
                can_controller_list.append(msg_value.bus_name)
            msg_dict["message_size"] = msg_value.length
            msg_dict["frame_id"] = hex(msg_value.frame_id)
            msg_dict["frame_type"] = msg_value.protocol
            if (("DiagRequest" in msg_value.message_attributes) and
                (0 != msg_value.message_attributes["DiagRequest"].value)) or (
                    ("DiagResponse" in msg_value.message_attributes) and
                    (0 != msg_value.message_attributes["DiagResponse"].value)) or (
                        ("DiagState" in msg_value.message_attributes) and
                        (0 != msg_value.message_attributes["DiagState"].value)):
                msg_dict["module"] = "CANTP"
                continue
            elif ("NmAsrMessage"
                  in msg_value.message_attributes) and (0 != msg_value.message_attributes["NmAsrMessage"].value):
                msg_dict["module"] = "CANNM"
            elif (("GenMsgILSupport" in msg_value.message_attributes) and
                  (0 == msg_value.message_attributes["GenMsgILSupport"].value)) and (
                      ("GenMsgCycleTime" in msg_value.message_attributes) and
                      (0 == msg_value.message_attributes["GenMsgCycleTime"].value)):
                msg_dict["module"] = "XCP"
                continue
            else:
                msg_dict["module"] = "PDUR"
                continue
            msg_dict["senders"] = msg_value.senders[0]
            if node_selected == msg_dict["senders"]:
                msg_dict["direction"] = "Tx"
                tx_number = tx_number + 1
                msg_dict["number"] = tx_number
                msg_dict["max_tx_number"] = tx_number
            else:
                msg_dict["direction"] = "Rx"
                rx_number = rx_number + 1
                msg_dict["number"] = rx_number
                msg_dict["max_rx_number"] = rx_number

            config_json_dict.append(msg_dict)

        cannm_config_json_dict[dbc["dbc_name"]] = config_json_dict
        cannm_config_json_dict["max_rx_number"] = rx_number
        cannm_config_json_dict["max_tx_number"] = tx_number
    cannm_config_json_dict["can_controller"] = can_controller_list

    return cannm_config_json_dict


def get_cannm_json_config(cannm_config_json_dict):
    """
    Get input parameter for template CanNm_Cfg.json.
    Arg:
        cannm_config_json_dict : All config dictionary, cannm config json

    Return:
        Returns a dictionary of cannm_json_config
    """
    cannm_json_config = cannm_config_json_dict.copy()
    if "max_rx_number" in cannm_json_config.keys():
        del cannm_json_config["max_rx_number"]
    if "max_tx_number" in cannm_json_config.keys():
        del cannm_json_config["max_tx_number"]

    return cannm_json_config


def create_tx_rx_pdu_container_and_fill_value(self, messages, container_value_channel_config, ecuc_def):
    for message in messages:
        if "CANNM" == message["module"]:
            if "Rx" == message["direction"]:
                cur_conf_def_channel_config = ecuc_def.find(
                    "/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmRxPdu")
                container_value_tx_rx_pdu = AutosarUtil.create_sub_container_value(message["message_name"],
                                                                                   cur_conf_def_channel_config,
                                                                                   container_value_channel_config)
                fill_value_dict_pdu_rx = {
                    "referance_values": {
                        '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmRxPdu/CanNmRxPduRef':
                            "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + message["message_name"] + "_" +
                            "oCanIf_CanNm_Rx"
                    },
                }
                fill_referance(self, container_value_tx_rx_pdu, fill_value_dict_pdu_rx)
                break
    for message in messages:
        path = get_definition_ref(container_value_channel_config)
        if "CANNM" == message["module"]:
            if "Tx" == message["direction"]:
                cur_conf_def_channel_config = ecuc_def.find(
                    "/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmTxPdu")
                container_value_tx_rx_pdu = AutosarUtil.create_sub_container_value(message["message_name"],
                                                                                   cur_conf_def_channel_config,
                                                                                   container_value_channel_config)
                fill_value_dict_pdu_tx = {
                    "referance_values": {
                        '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmTxPdu/CanNmTxPduRef':
                            "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + message["message_name"] + "_" +
                            "oCanNm_CanIf_Tx"
                    },
                }
                fill_referance(self, container_value_tx_rx_pdu, fill_value_dict_pdu_tx)
                cur_conf_def_channel_config = ecuc_def.find(
                    "/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmUserDataTxPdu")
                container_value_tx_rx_pdu = AutosarUtil.create_sub_container_value(message["message_name"] + "_Tx",
                                                                                   cur_conf_def_channel_config,
                                                                                   container_value_channel_config)
                fill_value_dict_pdu_tx_tx = {
                    "referance_values": {
                        '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmUserDataTxPdu/CanNmTxUserDataPduRef':
                            "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + message["message_name"] + "_" +
                            "oCom_CanNm_Tx"
                    },
                }
                fill_referance(self, container_value_tx_rx_pdu, fill_value_dict_pdu_tx_tx)


def create_sub_container_and_fill_value(self, prameter_dict, initial_autosar, messages):
    module_name = prameter_dict["module_name"]
    parent_short_name = prameter_dict["parent_short_name"]
    short_name = prameter_dict["short_name"]
    container_name = prameter_dict["container_name"]
    fill_value_dict = prameter_dict["fill_values"]
    if module_name in self.mod_autosar_dict:
        ecuc_def = self.mod_autosar_dict[module_name]

    parent = initial_autosar.find(f"/InitialEcuC/{module_name}/" + parent_short_name)
    path = get_definition_ref(parent)

    cur_conf_def_channel_config = ecuc_def.find(path.replace("/MICROSAR", "/AUTOSAR/EcucDefs") + "/" + short_name)
    ecuc_container_values = parent.SUB_CONTAINERS.parent_object_.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
    for i in range(len(ecuc_container_values)):
        if container_name == ecuc_container_values[i].SHORT_NAME.valueOf_:
            return
    container_value_channel_config = AutosarUtil.create_sub_container_value(container_name, cur_conf_def_channel_config,
                                                                            parent)
    fill_prameter_and_referance(self, container_value_channel_config, fill_value_dict)

    create_tx_rx_pdu_container_and_fill_value(self, messages, container_value_channel_config, ecuc_def)
    return


def fill_prameter_and_referance(self, container_value, fill_value_dict):

    if hasattr(container_value.PARAMETER_VALUES, "ECUC_ADD_INFO_PARAM_VALUE"):
        for para in container_value.PARAMETER_VALUES.ECUC_ADD_INFO_PARAM_VALUE:
            def_ref = get_definition_ref(para)
            for definition in fill_value_dict["add_info_prameter_values"].keys():
                if def_ref == definition:
                    set_value(para, fill_value_dict["add_info_prameter_values"][definition])
                    AutosarUtil.add_annotation_value(para, "AutoDerived")

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

    if hasattr(container_value.REFERENCE_VALUES, "ECUC_INSTANCE_REFERENCE_VALUE"):
        for para in container_value.REFERENCE_VALUES.ECUC_INSTANCE_REFERENCE_VALUE:
            def_ref = get_definition_ref(para)
            for definition in fill_value_dict["instance_referance_values"].keys():
                if def_ref == definition:
                    set_value(para, fill_value_dict["instance_referance_values"][definition])
                    AutosarUtil.add_annotation_value(para, "AutoDerived")

    return container_value


def fill_referance(self, container_value, fill_value_dict):

    if hasattr(container_value.REFERENCE_VALUES, "ECUC_REFERENCE_VALUE"):
        for para in container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
            def_ref = get_definition_ref(para)
            for definition in fill_value_dict["referance_values"].keys():
                if def_ref == definition:
                    set_value_ref(para, fill_value_dict["referance_values"][definition])
                    AutosarUtil.add_annotation_value(para, "AutoDerived")

    return container_value


def get_canif_can_controller(canif_config_json_dict):
    """
    Get all the can controllers.
    Arg:
        config_dict : All config dictionary

    Return:
        Returns a dictionary of can_controller_list
    """

    can_controller_list = canif_config_json_dict["can_controller"]

    return can_controller_list


def get_canif_can_controller_num(canif_config_json_dict):
    """
    Get all the can controllers num.
    Arg:
        config_dict : All config dictionary

    Return:
        Returns can_controller_num
    """

    can_controller_num = len(canif_config_json_dict["can_controller"])
    del canif_config_json_dict["can_controller"]

    return can_controller_num
