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
from basic_func_module.base_logger import logger

from .initial_can_common_ecuc_parser import InitialCanCommonEcucParser

AUTO_DERIVED_FLAG = "AutoDerived"


class InitialCanIfEcucParser():
    """
    All CanIf module page jump, display, save logic code.
    """

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict
        self.xlsx_info_dict = {}
        
    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, dict):
        self.process_info_from_xlsx(dict)
        self.update_the_canif_tree(initial_autosar, dict)

    def process_info_from_xlsx(self, dict_cfg):
        if "excel" in dict_cfg and "CanIfCfg.xlsx" in dict_cfg["excel"]:
            self.xlsx_info_dict = dict_cfg["excel"]["CanIfCfg.xlsx"]

    def get_frame_type_by_message_frame_type(self, message_frame_type):
        frame_type = "STANDARD_CAN"
        if "ExtendedCAN" == message_frame_type:
            frame_type = "EXTENDED_CAN"
        elif "ExtendedCAN_FD" == message_frame_type:
            frame_type = "EXTENDED_FD_CAN"
        elif "EXTENDED_NO_FD_CAN" == message_frame_type:
            frame_type = "EXTENDED_NO_FD_CAN"
        elif "StandardCAN" == message_frame_type:
            frame_type = "STANDARD_CAN"
        elif "StandardCAN_FD" == message_frame_type:
            frame_type = "STANDARD_FD_CAN"
        elif "STANDARD_NO_FD_CAN" == message_frame_type:
            frame_type = "STANDARD_NO_FD_CAN"

        return frame_type

    def get_rx_indication_name_by_message_module(self, module):
        rx_indication_name = "pdur_canif_rx_indication"

        if "PDUR" == module:
            rx_indication_name = "pdur_canif_rx_indication"
        elif "CAN_TP" == module:
            rx_indication_name = "CanTp_RxIndication"
        elif "CAN_NM" == module:
            rx_indication_name = "cannm_rx_indication"
        elif "CAN_TSYN" == module:
            rx_indication_name = "cantsyn_tx_confirmation"
        elif "XCP" == module:
            rx_indication_name = "Xcp_CanIfRxIndication"
        elif "DLT" == module:
            rx_indication_name = "Dlt_CanIfRxIndication"
        elif "NONE" == module:
            rx_indication_name = None
        return rx_indication_name

    def canif_ipdu_rx_update_data_by_dbc(self, initial_autosar, message, message_name):
        create_sub_container_and_fill_value_prameter_dict = {
            "module_name": "CanIf",
            "parent_short_name": "CanIfInitCfg",
            "short_name": "CanIfRxPduCfg",
            "container_name": message_name,
            "fill_values": {},
        }

        canif_rx_pdu_ref_prefix = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + message['message_name'] + "_o"
        canif_usr_confirmation_ul = "PDUR"
        if "PDUR" == message["module"]:
            canif_rx_pdu_ref = canif_rx_pdu_ref_prefix + "CanIf_PduR_Rx"
            canif_usr_confirmation_ul = "PDUR"
            message["DLC"] = "true"
        elif "CAN_TP" == message["module"]:
            canif_rx_pdu_ref = canif_rx_pdu_ref_prefix + "CanIf_CanTp_Rx"
            canif_usr_confirmation_ul = "CAN_TP"
            message["DLC"] = "false"
        elif "CAN_NM" == message["module"]:
            canif_rx_pdu_ref = canif_rx_pdu_ref_prefix + "CanIf_CanNm_Rx"
            canif_usr_confirmation_ul = "CAN_NM"
            message["DLC"] = "true"
        elif "CAN_TSYN" == message["module"]:
            canif_rx_pdu_ref = canif_rx_pdu_ref_prefix + "CanIf_CanTSyn_Rx"
            canif_usr_confirmation_ul = "CAN_TSYN"
            message["DLC"] = "true"
        elif "XCP" == message["module"]:
            canif_rx_pdu_ref = canif_rx_pdu_ref_prefix + "CanIf_Xcp_Rx"
            canif_usr_confirmation_ul = "XCP"
            message["DLC"] = "false"
        elif "DLT" == message["module"]:
            canif_rx_pdu_ref = canif_rx_pdu_ref_prefix + "CanIf_Dlt_Rx"
            canif_usr_confirmation_ul = "DLT"
            message["DLC"] = "false"
        else:
            message["DLC"] = "true"
            canif_rx_pdu_ref = None
        frame_type = self.get_frame_type_by_message_frame_type(message["frame_type"])

        fill_value_dict = {
            "add_info_prameter_values": {},
            "numerical_prameter_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduCanId': str(message["frame_id"]),
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduDataLength': str(message["message_size"]),
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduId': str(message["number"] - 1),
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduDataLengthCheck': message["DLC"],
            },
            "textual_prameter_values": {
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduUserRxIndicationName":
                    self.get_rx_indication_name_by_message_module(message["module"]),
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduCanIdType':
                    frame_type,
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduUserRxIndicationUL':
                    str(message["module"]),
            },
            "referance_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduHrhIdRef': "",
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfRxPduCfg/CanIfRxPduRef': canif_rx_pdu_ref,
            },
            "instance_referance_values": {}
        }
        create_sub_container_and_fill_value_prameter_dict["fill_values"] = fill_value_dict
        create_sub_container_and_fill_value(self, initial_autosar, create_sub_container_and_fill_value_prameter_dict)

    def get_tx_confirmation_name_by_message_module(self, module):
        tx_confirmation_name = "pdur_canif_tx_confirmation"

        if "PDUR" == module:
            tx_confirmation_name = "pdur_canif_tx_confirmation"
        elif "CAN_TP" == module:
            tx_confirmation_name = "CanTp_TxConfirmation"
        elif "CAN_NM" == module:
            tx_confirmation_name = "cannm_tx_confirmation"
        elif "CAN_TSYN" == module:
            tx_confirmation_name = "cantsyn_tx_confirmation"
        elif "XCP" == module:
            tx_confirmation_name = "Xcp_CanIfTxConfirmation"
        elif "DLT" == module:
            tx_confirmation_name = "Dlt_CanIfTxConfirmation"
        elif "NONE" == module:
            tx_confirmation_name = None
        return tx_confirmation_name

    def canif_ipdu_tx_update_data_by_dbc(self, initial_autosar, message, message_name):
        create_sub_container_and_fill_value_prameter_dict = {
            "module_name": "CanIf",
            "parent_short_name": "CanIfInitCfg",
            "short_name": "CanIfTxPduCfg",
            "container_name": message_name,
            "fill_values": {},
        }

        canif_usr_confirmation_ul = "PDUR"
        canif_tx_pdu_ref_prefix = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + message['message_name'] + "_o"
        if "PDUR" == message["module"]:
            canif_tx_pdu_ref = canif_tx_pdu_ref_prefix + "PduR_CanIf_Tx"
            canif_usr_confirmation_ul = "PDUR"
        elif "CAN_TP" == message["module"]:
            canif_tx_pdu_ref = canif_tx_pdu_ref_prefix + "CanTp_CanIf_Tx"
            canif_usr_confirmation_ul = "CAN_TP"
        elif "CAN_NM" == message["module"]:
            canif_tx_pdu_ref = canif_tx_pdu_ref_prefix + "CanNm_CanIf_Tx"
            canif_usr_confirmation_ul = "CAN_NM"
        elif "CAN_TSYN" == message["module"]:
            canif_tx_pdu_ref = canif_tx_pdu_ref_prefix + "CanTSyn_CanIf_Tx"
        elif "XCP" == message["module"]:
            canif_tx_pdu_ref = canif_tx_pdu_ref_prefix + "Xcp_CanIf_Tx"
            canif_usr_confirmation_ul = "XCP"
        elif "DLT" == message["module"]:
            canif_tx_pdu_ref = canif_tx_pdu_ref_prefix + "Dlt_CanIf_Tx"
            canif_usr_confirmation_ul = "DLT"
        else:
            canif_tx_pdu_ref = None
        frame_type = self.get_frame_type_by_message_frame_type(message["frame_type"])

        fill_value_dict = {
            "add_info_prameter_values": {},
            "numerical_prameter_values": {
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduId": str(message["number"] - 1),
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduCanId": str(message["frame_id"]),
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduDataLength": str(message["message_size"]
                                                                                              ),
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduUserTxIndicationUL":
                    message["module"],
            },
            "textual_prameter_values": {
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduUserTxConfirmationName":
                    self.get_tx_confirmation_name_by_message_module(message["module"]),
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduUserTxConfirmationUL":
                    message["module"],
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduCanIdType":
                    frame_type,
            },
            "referance_values": {
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduRef": canif_tx_pdu_ref,
                "/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfTxPduCfg/CanIfTxPduBufferRef": "",
            },
            "instance_referance_values": {}
        }
        create_sub_container_and_fill_value_prameter_dict["fill_values"] = fill_value_dict
        create_sub_container_and_fill_value(self, initial_autosar, create_sub_container_and_fill_value_prameter_dict)

    def update_the_canif_init_cfg_tx_rx_pdu_cfgs(self, initial_autosar):
        for comm_diag_name, value in self.canif_pdus_json_config.items():
            for message in value:
                ipdu_name_alias = message["message_name"] + "_" + message["direction"]
                if "Rx" == message["direction"]:
                    self.canif_ipdu_rx_update_data_by_dbc(initial_autosar, message, ipdu_name_alias)
                elif "Tx" == message["direction"]:
                    self.canif_ipdu_tx_update_data_by_dbc(initial_autosar, message, ipdu_name_alias)

    def canif_ctrl_cfg_update_data_by_eb_arxml(self, initial_autosar, can_object_name, can_object_attr):
        create_sub_container_and_fill_value_prameter_dict = {
            "module_name": "CanIf",
            "parent_short_name": "CanIfCtrlDrvCfg",
            "short_name": "CanIfCtrlCfg",
            "container_name": can_object_name,
            "fill_values": {},
        }

        fill_value_dict = {
            "add_info_prameter_values": {},
            "numerical_prameter_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/CanIfCtrlCfg/CanIfCtrlId':
                    str(can_object_attr["CanControllerId"]["Value"]),
                '/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/CanIfCtrlCfg/CanIfCtrlWakeupSupport':
                    str(can_object_attr["CanWakeupSupport"]["Value"]),
            },
            "textual_prameter_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/CanIfCtrlCfg/CanIfCtrlJ1939DynAddrSupport': "DISABLED",
            },
            "referance_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/CanIfCtrlCfg/CanIfCtrlCanCtrlRef':
                    "/ActiveEcuC/Can/CanConfigSet/" + can_object_name,
            },
            "instance_referance_values": {}
        }
        create_sub_container_and_fill_value_prameter_dict["fill_values"] = fill_value_dict
        create_sub_container_and_fill_value(self, initial_autosar, create_sub_container_and_fill_value_prameter_dict)

    def canif_hrh_cfg_update_data_by_eb_arxml(self, initial_autosar, can_object_name, can_object_attr):
        if "BASIC" == can_object_attr['CanHandleType']['Value']:
            can_hrh_software_filter_value = "true"
        elif "FULL" == can_object_attr['CanHandleType']['Value']:
            can_hrh_software_filter_value = "false"

        create_sub_container_and_fill_value_prameter_dict = {
            "module_name": "CanIf",
            "parent_short_name": "CanIfInitCfg/CanIfInitHohCfg",
            "short_name": "CanIfHrhCfg",
            "container_name": can_object_name,
            "fill_values": {},
        }

        fill_value_dict = {
            "add_info_prameter_values": {},
            "numerical_prameter_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfInitHohCfg/CanIfHrhCfg/CanIfHrhSoftwareFilter':
                    str(can_hrh_software_filter_value),
            },
            "textual_prameter_values": {},
            "referance_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfInitHohCfg/CanIfHrhCfg/CanIfHrhCanCtrlIdRef':
                    "/ActiveEcuC/CanIf/CanIfCtrlDrvCfg/" +
                    can_object_attr["CanControllerRef"]["ValueRef"].split("/")[-1],
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfInitHohCfg/CanIfHrhCfg/CanIfHrhIdSymRef':
                    "/ActiveEcuC/Can/CanConfigSet/" + can_object_name,
            },
            "instance_referance_values": {}
        }
        create_sub_container_and_fill_value_prameter_dict["fill_values"] = fill_value_dict
        create_sub_container_and_fill_value(self, initial_autosar, create_sub_container_and_fill_value_prameter_dict)

    def canif_hth_cfg_update_data_by_eb_arxml(self, initial_autosar, can_object_name, can_object_attr):
        create_sub_container_and_fill_value_prameter_dict = {
            "module_name": "CanIf",
            "parent_short_name": "CanIfInitCfg/CanIfInitHohCfg",
            "short_name": "CanIfHthCfg",
            "container_name": can_object_name,
            "fill_values": {},
        }

        fill_value_dict = {
            "add_info_prameter_values": {},
            "numerical_prameter_values": {},
            "textual_prameter_values": {},
            "referance_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfInitHohCfg/CanIfHthCfg/CanIfHthCanCtrlIdRef':
                    "/ActiveEcuC/CanIf/CanIfCtrlDrvCfg/" +
                    can_object_attr["CanControllerRef"]["ValueRef"].split("/")[-1],
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfInitHohCfg/CanIfHthCfg/CanIfHthIdSymRef':
                    "/ActiveEcuC/Can/CanConfigSet/" + can_object_name,
            },
            "instance_referance_values": {}
        }
        create_sub_container_and_fill_value_prameter_dict["fill_values"] = fill_value_dict
        create_sub_container_and_fill_value(self, initial_autosar, create_sub_container_and_fill_value_prameter_dict)

    def canif_buffer_cfg_update_data_by_eb_arxml(self, initial_autosar, can_object_name, can_object_attr):
        if "FULL" == can_object_attr['CanHandleType']['Value']:
            canif_buffer_size = "0"
            canif_tx_buffer_mapped_tx_pdus = "0"
        elif ("BASIC" == can_object_attr['CanHandleType']['Value']):
            canif_buffer_size = "0"
            canif_tx_buffer_mapped_tx_pdus = "0"

        create_sub_container_and_fill_value_prameter_dict = {
            "module_name": "CanIf",
            "parent_short_name": "CanIfInitCfg",
            "short_name": "CanIfBufferCfg",
            "container_name": can_object_name,
            "fill_values": {},
        }

        fill_value_dict = {
            "add_info_prameter_values": {},
            "numerical_prameter_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfBufferCfg/CanIfBufferSize':
                    str(canif_buffer_size),
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfBufferCfg/CanIfTxBufferMappedTxPdus':
                    str(canif_tx_buffer_mapped_tx_pdus),
            },
            "textual_prameter_values": {},
            "referance_values": {
                '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfBufferCfg/CanIfBufferHthRef':
                    "/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/" + can_object_name,
            },
            "instance_referance_values": {}
        }

        create_sub_container_and_fill_value_prameter_dict["fill_values"] = fill_value_dict
        create_sub_container_and_fill_value(self, initial_autosar, create_sub_container_and_fill_value_prameter_dict)

    def sort_can_object(self, dict):
        can_json_from_arxml_can_cfg_set = dict["eb"]["arxml"]["Can"]["CanConfigSet"]
        can_json_from_arxml_can_cfg_set_tmp = {}
        for can_ctrl_name, can_ctrl_attr in can_json_from_arxml_can_cfg_set.items():
            if "Tag" == can_ctrl_name or "DefinitionRef" == can_ctrl_name:
                continue
            if ("CanObjectId" in can_ctrl_attr.keys()):
                can_json_from_arxml_can_cfg_set_tmp[int(
                    can_json_from_arxml_can_cfg_set[can_ctrl_name]["CanObjectId"]["Value"])] = can_ctrl_attr
                can_json_from_arxml_can_cfg_set_tmp[int(can_json_from_arxml_can_cfg_set[can_ctrl_name]["CanObjectId"]
                                                        ["Value"])]["object_name"] = can_ctrl_name
        can_object_tmp = sorted(can_json_from_arxml_can_cfg_set_tmp.items(), key=lambda x: x[0])
        can_object_tmp_sorted = {}
        for i in range(len(can_object_tmp)):
            can_object_tmp_sorted[can_object_tmp[i][1]['object_name']] = can_object_tmp[i][1]

        return can_object_tmp_sorted

    def sort_can_ctrl_cfg(self, dict):
        can_json_from_arxml_can_cfg_set = dict['eb']["arxml"]['Can']["CanConfigSet"]
        controllerId_info_list = []
        for key, value in can_json_from_arxml_can_cfg_set.items():
            if key == 'DefinitionRef':
                continue
            if "CanControllerId" in value:
                for k, v in value.items():
                    if k == 'CanControllerId' and "Value" in v:
                        controllerId_info_list.append({key: v['Value']})

        controllerId_info_sorted_list = sorted(controllerId_info_list,  key=lambda x: list(x.values())[0])

        can_json_from_arxml_can_cfg_set_sorted = {}
        for ctrl_info in controllerId_info_sorted_list:
            (key, value), = ctrl_info.items()
            can_json_from_arxml_can_cfg_set_sorted[key] = can_json_from_arxml_can_cfg_set[key]

        return can_json_from_arxml_can_cfg_set_sorted

    def update_the_canif_tree_ctrl_drv(self, initial_autosar, dict):
        can_object_sorted = self.sort_can_object(dict)
        can_json_from_arxml_can_cfg_set_sorted = self.sort_can_ctrl_cfg(dict)

        canif_ctrl_cfgs = {}
        canif_buffer_cfgs = {}
        canif_hth_and_hrh_cfgs_rx = {}
        canif_hth_and_hrh_cfgs_tx = {}
        for can_ctrl_name, can_ctrl_attr in can_json_from_arxml_can_cfg_set_sorted.items():
            if "Tag" == can_ctrl_name or "DefinitionRef" == can_ctrl_name:
                continue
            if "CanObjectType" in can_ctrl_attr.keys():
                pass
            else:
                self.canif_ctrl_cfg_update_data_by_eb_arxml(initial_autosar, can_ctrl_name, can_ctrl_attr)

        for can_object_name, can_object_attr in can_object_sorted.items():
            if "Tag" == can_object_name or "DefinitionRef" == can_object_name:
                continue
            if "CanObjectType" in can_object_attr.keys():
                if "RECEIVE" == can_object_attr["CanObjectType"]["Value"]:
                    self.canif_hrh_cfg_update_data_by_eb_arxml(initial_autosar, can_object_name, can_object_attr)
                elif "TRANSMIT" == can_object_attr["CanObjectType"]["Value"]:
                    self.canif_hth_cfg_update_data_by_eb_arxml(initial_autosar, can_object_name, can_object_attr)
                    self.canif_buffer_cfg_update_data_by_eb_arxml(initial_autosar, can_object_name, can_object_attr)

    def update_the_canif_tree(self, initial_autosar, cfg_dict):
        canif_config_json_dict = get_template_config_json_dict(cfg_dict)
        can_controller_list = get_canif_can_controller(canif_config_json_dict)
        can_controller_num = get_canif_can_controller_num(canif_config_json_dict)
        self.canif_pdus_json_config = get_canif_json_config(canif_config_json_dict)

        self.update_the_canif_init_cfg_tx_rx_pdu_cfgs(initial_autosar)
        self.update_rx_pdu_hrh_id_ref_item(initial_autosar, cfg_dict)
        self.update_tx_pdu_hth_id_ref_item(initial_autosar, cfg_dict)

    def get_can_controller_name_by_pdu_name_and_pdu_id(self, pdu_name, pdu_can_id, controller_namelist,direction):
        can_controller_name = None
        if bool(self.canif_pdus_json_config) == True:
            for comm_diag_name, value in self.canif_pdus_json_config.items():
                for message in value:
                    ipdu_name_alias = message["message_name"] + "_" + message["direction"]
                    if ipdu_name_alias == pdu_name and message["frame_id"] == pdu_can_id:
                        can_controller_name = message["bus_name"]
                        break
        else:
            for controllername in controller_namelist:
                if direction == "TRANSMIT":
                    if pdu_name.endswith("_o" + controllername+"_Tx") == True:
                        can_controller_name = controllername
                        break
                elif direction == "RECEIVE":
                    if pdu_name.endswith("_o" + controllername +"_Rx") == True:
                        can_controller_name = controllername
                        break

        return can_controller_name

    def update_rx_pdu_hrh_id_ref_item(self, initial_autosar, cfg_dict):
        can_json_from_arxml_can_cfg_set = cfg_dict.get("eb", {}).get("arxml", {}).get("Can", {}).get("CanConfigSet", None)
        if can_json_from_arxml_can_cfg_set is None:
            can_root_cointar = initial_autosar.find(f"/InitialEcuC/Can")
            if can_root_cointar is None:
                logger.debug('Can module information not found, Canif Rx Pdu Hrh not update!')
                return
            can_root_cointar_dict = AutosarUtil.to_ecuc_module_configuration_values_dict(can_root_cointar)
            can_json_from_arxml_can_cfg_set = can_root_cointar_dict.get("CanConfigSet", None)
            if can_json_from_arxml_can_cfg_set is None:
                logger.error('Can module CanConfigSet not found, Canif Rx Pdu Hrh not update!')
                return

        can_json_from_arxml_can_cfg_set_tmp = {}
        for can_ctrl_name, can_ctrl_attr in can_json_from_arxml_can_cfg_set.items():
            if "Tag" == can_ctrl_name or "DefinitionRef" == can_ctrl_name:
                continue
            if ("CanObjectId" in can_ctrl_attr.keys()):
                can_json_from_arxml_can_cfg_set_tmp[int(
                    can_json_from_arxml_can_cfg_set[can_ctrl_name]["CanObjectId"]["Value"])] = can_ctrl_attr
                can_json_from_arxml_can_cfg_set_tmp[int(can_json_from_arxml_can_cfg_set[can_ctrl_name]["CanObjectId"]
                                                        ["Value"])]["object_name"] = can_ctrl_name
        can_object_tmp = sorted(can_json_from_arxml_can_cfg_set_tmp.items(), key=lambda x: x[0])
        can_object_tmp_sorted = {}
        for i in range(len(can_object_tmp)):
            can_object_tmp_sorted[can_object_tmp[i][1]['object_name']] = can_object_tmp[i][1]

        initial_autosar_canif = initial_autosar.AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[1]
        parent = initial_autosar_canif.find(f"/InitialEcuC/CanIf/CanIfInitCfg")
        all_canif_init_cfg_container_values = parent.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        pdu_can_id = None
        for i in range(len(all_canif_init_cfg_container_values)):
            if "CanIfRxPduCfg" == all_canif_init_cfg_container_values[i].DEFINITION_REF.valueOf_.split("/")[-1]:
                pdu_name = all_canif_init_cfg_container_values[i].SHORT_NAME.valueOf_
                numerical_parma_values = all_canif_init_cfg_container_values[
                    i].PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
                reference_values = all_canif_init_cfg_container_values[i].REFERENCE_VALUES.ECUC_REFERENCE_VALUE
                for j in range(len(numerical_parma_values)):
                    if "CanIfRxPduCanId" == numerical_parma_values[j].DEFINITION_REF.valueOf_.split("/")[-1]:
                        pdu_can_id = numerical_parma_values[j].VALUE.valueOf_
                        break

                can_controller_namelist = get_canif_can_controller_name(can_json_from_arxml_can_cfg_set)
                can_controller_name_from_dbc = self.get_can_controller_name_by_pdu_name_and_pdu_id(pdu_name, pdu_can_id, can_controller_namelist, "RECEIVE")
                can_controller_name_from_eb_arxml = None

                for can_obj_name, can_obj_attr in can_object_tmp_sorted.items():
                    if "Tag" == can_obj_name or "DefinitionRef" == can_obj_name:
                        continue

                    if "CanObjectType" not in can_obj_attr.keys():
                        continue

                    can_controller_name_from_eb_arxml = can_obj_attr["CanControllerRef"]["ValueRef"].split("/")[-1]
                    if self.xlsx_info_dict:
                        for dbc_ctrl_name, eb_ctrl_name in self.xlsx_info_dict.items():
                            if eb_ctrl_name == can_controller_name_from_eb_arxml:
                                can_controller_name_from_eb_arxml = dbc_ctrl_name
                                break

                    cur_target = cfg_dict["target"]
                    if cur_target == 'RH850_P1MC' or cur_target == 'RH850_U2A16':
                        if (can_controller_name_from_eb_arxml == can_controller_name_from_dbc) and ("RECEIVE" == can_obj_attr["CanObjectType"]["Value"]):
                            if "CanHwFilter" in can_obj_attr.keys():
                                can_hw_filter_code = int(can_obj_attr["CanHwFilter"]["CanHwFilterCode"]["Value"])
                                can_hw_filter_mask = int(can_obj_attr["CanHwFilter"]["CanHwFilterMask"]["Value"])
                                if (int(pdu_can_id, 16) & can_hw_filter_mask) == (can_hw_filter_code & can_hw_filter_mask):
                                    reference_values[0].VALUE_REF.valueOf_ = "/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/" + can_obj_name
                                    break
                    else:
                        if (can_controller_name_from_eb_arxml == can_controller_name_from_dbc) and ("RECEIVE" == can_obj_attr["CanObjectType"]["Value"]):
                            if ("CanHwFilter_0" in can_obj_attr.keys()) and ("CanHwFilter_1" not in can_obj_attr.keys()):
                                can_hw_filter_code = int(can_obj_attr["CanHwFilter_0"]["CanHwFilterCode"]["Value"])
                                can_hw_filter_mask = int(can_obj_attr["CanHwFilter_0"]["CanHwFilterMask"]["Value"])
                                if (int(pdu_can_id, 16) & can_hw_filter_mask) == (can_hw_filter_code & can_hw_filter_mask):
                                    reference_values[0].VALUE_REF.valueOf_ = "/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/" + can_obj_name
                                    break

                            if ("CanHwFilter_0" in can_obj_attr.keys()) and ("CanHwFilter_1" in can_obj_attr.keys()):
                                can_hw_filter_range_upper = int(can_obj_attr["CanHwFilter_0"]["CanHwFilterCode"]["Value"])
                                can_hw_filter_range_lower = int(can_obj_attr["CanHwFilter_1"]["CanHwFilterCode"]["Value"])
                                if (int(pdu_can_id, 16) >= can_hw_filter_range_lower) and (int(pdu_can_id, 16) <= can_hw_filter_range_upper):
                                    reference_values[0].VALUE_REF.valueOf_ = "/ActiveEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg/" + can_obj_name
                                    break


    def set_canif_buffer_size(self, all_canif_init_cfg_container_values):
        for i in range(len(all_canif_init_cfg_container_values)):
            if "CanIfTxPduCfg" == all_canif_init_cfg_container_values[i].DEFINITION_REF.valueOf_.split("/")[-1]:
                can_ctrl_name = all_canif_init_cfg_container_values[i].REFERENCE_VALUES.ECUC_REFERENCE_VALUE[
                    0].VALUE_REF.valueOf_.split("/")[-1]
                for j in range(len(all_canif_init_cfg_container_values)):
                    if "CanIfBufferCfg" == all_canif_init_cfg_container_values[j].DEFINITION_REF.valueOf_.split(
                            "/")[-1] and can_ctrl_name == all_canif_init_cfg_container_values[j].SHORT_NAME.valueOf_:
                        numerical_parma_values = all_canif_init_cfg_container_values[
                            j].PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
                        numerical_parma_values[0].VALUE.content_[0].value = "0"
                        numerical_parma_values[0].VALUE.valueOf_ = "0"

        for i in range(len(all_canif_init_cfg_container_values)):
            if "CanIfTxPduCfg" == all_canif_init_cfg_container_values[i].DEFINITION_REF.valueOf_.split("/")[-1]:
                can_ctrl_name = all_canif_init_cfg_container_values[i].REFERENCE_VALUES.ECUC_REFERENCE_VALUE[
                    0].VALUE_REF.valueOf_.split("/")[-1]
                for j in range(len(all_canif_init_cfg_container_values)):
                    if "CanIfBufferCfg" == all_canif_init_cfg_container_values[j].DEFINITION_REF.valueOf_.split(
                            "/")[-1] and can_ctrl_name == all_canif_init_cfg_container_values[j].SHORT_NAME.valueOf_:
                        numerical_parma_values = all_canif_init_cfg_container_values[
                            j].PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
                        canif_buffer_size = int(numerical_parma_values[0].VALUE.valueOf_)
                        canif_buffer_size = canif_buffer_size + 1
                        numerical_parma_values[0].VALUE.content_[0].value = str(canif_buffer_size)
                        numerical_parma_values[0].VALUE.valueOf_ = str(canif_buffer_size)

        for j in range(len(all_canif_init_cfg_container_values)):
            if "CanIfBufferCfg" == all_canif_init_cfg_container_values[j].DEFINITION_REF.valueOf_.split("/")[-1]:
                numerical_parma_values = all_canif_init_cfg_container_values[
                    j].PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
                if numerical_parma_values[0].VALUE.valueOf_ == "1":
                    numerical_parma_values[0].VALUE.content_[0].value = "0"
                    numerical_parma_values[0].VALUE.valueOf_ = "0"

    def update_tx_pdu_hth_id_ref_item(self, initial_autosar, cfg_dict):
        can_json_from_arxml_can_cfg_set = cfg_dict.get("eb", {}).get("arxml", {}).get("Can", {}).get("CanConfigSet", None)
        if can_json_from_arxml_can_cfg_set is None:
            can_root_cointar = initial_autosar.find(f"/InitialEcuC/Can")
            if can_root_cointar is None:
                logger.debug('Can module information not found, Canif Rx Pdu Hrh not update!')
                return
            can_root_cointar_dict = AutosarUtil.to_ecuc_module_configuration_values_dict(can_root_cointar)
            can_json_from_arxml_can_cfg_set = can_root_cointar_dict.get("CanConfigSet", None)
            if can_json_from_arxml_can_cfg_set is None:
                logger.error('Can module CanConfigSet not found, Canif Rx Pdu Hrh not update!')
                return

        can_json_from_arxml_can_cfg_set_tmp = {}
        for can_obj_name, can_obj_attr in can_json_from_arxml_can_cfg_set.items():
            if "Tag" == can_obj_name or "DefinitionRef" == can_obj_name:
                continue
            if ("CanObjectId" in can_obj_attr.keys()):
                can_json_from_arxml_can_cfg_set_tmp[int(
                    can_json_from_arxml_can_cfg_set[can_obj_name]["CanObjectId"]["Value"])] = can_obj_attr
                can_json_from_arxml_can_cfg_set_tmp[int(can_json_from_arxml_can_cfg_set[can_obj_name]["CanObjectId"]
                                                        ["Value"])]["object_name"] = can_obj_name
        can_object_tmp = sorted(can_json_from_arxml_can_cfg_set_tmp.items(), key=lambda x: x[0])
        can_object_tmp_sorted = {}
        pdu_can_id = None
        for i in range(len(can_object_tmp)):
            can_object_tmp_sorted[can_object_tmp[i][1]['object_name']] = can_object_tmp[i][1]
        ecuc_autosar = initial_autosar.AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[1]
        parent = ecuc_autosar.find(f"/InitialEcuC/CanIf/CanIfInitCfg")
        all_canif_init_cfg_container_values = parent.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        for i in range(len(all_canif_init_cfg_container_values)):
            if "CanIfTxPduCfg" == all_canif_init_cfg_container_values[i].DEFINITION_REF.valueOf_.split("/")[-1]:
                pdu_name = all_canif_init_cfg_container_values[i].SHORT_NAME.valueOf_
                numerical_parma_values = all_canif_init_cfg_container_values[
                    i].PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
                reference_values = all_canif_init_cfg_container_values[i].REFERENCE_VALUES.ECUC_REFERENCE_VALUE
                for j in range(len(numerical_parma_values)):
                    if "CanIfTxPduCanId" == numerical_parma_values[j].DEFINITION_REF.valueOf_.split("/")[-1]:
                        pdu_can_id = numerical_parma_values[j].VALUE.valueOf_
                        break

                can_controller_namelist = get_canif_can_controller_name(can_json_from_arxml_can_cfg_set)
                can_controller_name_from_dbc = self.get_can_controller_name_by_pdu_name_and_pdu_id(pdu_name, pdu_can_id, can_controller_namelist, "TRANSMIT")
                can_controller_name_from_eb_arxml = None

                for can_obj_name, can_obj_attr in can_object_tmp_sorted.items():
                    if "Tag" == can_obj_name or "DefinitionRef" == can_obj_name:
                        continue

                    if "CanObjectType" not in can_obj_attr.keys():
                        continue

                    can_controller_name_from_eb_arxml = can_obj_attr["CanControllerRef"]["ValueRef"].split("/")[-1]
                    if self.xlsx_info_dict:
                        for dbc_ctrl_name, eb_ctrl_name in self.xlsx_info_dict.items():
                            if eb_ctrl_name == can_controller_name_from_eb_arxml:
                                can_controller_name_from_eb_arxml = dbc_ctrl_name
                                break

                    cur_target = cfg_dict["target"]
                    if cur_target == 'RH850_P1MC' or cur_target == 'RH850_U2A16':
                        if (can_controller_name_from_eb_arxml == can_controller_name_from_dbc) and ("TRANSMIT" == can_obj_attr["CanObjectType"]["Value"]):
                            if "CanHwFilter" in can_obj_attr.keys():
                                can_hw_filter_code = int(can_obj_attr["CanHwFilter"]["CanHwFilterCode"]["Value"])
                                can_hw_filter_mask = int(can_obj_attr["CanHwFilter"]["CanHwFilterMask"]["Value"])
                                if (int(pdu_can_id, 16) & can_hw_filter_mask) == (can_hw_filter_code & can_hw_filter_mask):
                                    reference_values[0].VALUE_REF.valueOf_ = "/ActiveEcuC/CanIf/CanIfInitCfg/" + can_obj_name
                                    break
                    else:
                        if (can_controller_name_from_eb_arxml == can_controller_name_from_dbc) and ("TRANSMIT" == can_obj_attr["CanObjectType"]["Value"]):
                            if ("CanHwFilter_0" in can_obj_attr.keys()) and ("CanHwFilter_1" not in can_obj_attr.keys()):
                                can_hw_filter_code = int(can_obj_attr["CanHwFilter_0"]["CanHwFilterCode"]["Value"])
                                can_hw_filter_mask = int(can_obj_attr["CanHwFilter_0"]["CanHwFilterMask"]["Value"])
                                if (int(pdu_can_id, 16) & can_hw_filter_mask == can_hw_filter_code & can_hw_filter_mask):
                                    reference_values[0].VALUE_REF.valueOf_ = "/ActiveEcuC/CanIf/CanIfInitCfg/" + can_obj_name
                                    break

                            if ("CanHwFilter_0" in can_obj_attr.keys()) and ("CanHwFilter_1" in can_obj_attr.keys()):
                                can_hw_filter_range_upper = int(can_obj_attr["CanHwFilter_0"]["CanHwFilterCode"]["Value"])
                                can_hw_filter_range_lower = int(can_obj_attr["CanHwFilter_1"]["CanHwFilterCode"]["Value"])
                                if (int(pdu_can_id, 16) >= can_hw_filter_range_lower) and (int(pdu_can_id, 16) <= can_hw_filter_range_upper):
                                    reference_values[0].VALUE_REF.valueOf_ = "/ActiveEcuC/CanIf/CanIfInitCfg/" + can_obj_name
                                    break


        self.set_canif_buffer_size(all_canif_init_cfg_container_values)

    def update_data_from_eb_can_arxml_slot(self, initial_autosar, dict):
        self.update_the_canif_tree_ctrl_drv(initial_autosar, dict)
        if False == hasattr(self, "canif_pdus_json_config"):
            self.canif_pdus_json_config = {}
        self.update_rx_pdu_hrh_id_ref_item(initial_autosar, dict)
        self.update_tx_pdu_hth_id_ref_item(initial_autosar, dict)

        init_autosar = initial_autosar.AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[1]
        parent = init_autosar.find(f"/InitialEcuC/CanIf/CanIfCtrlDrvCfg")
        all_canif_init_cfg_container_values = parent.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        for i in range(len(all_canif_init_cfg_container_values)):
            if "init_CanIfCtrlCfg" == all_canif_init_cfg_container_values[i].SHORT_NAME.valueOf_:
                all_canif_init_cfg_container_values.remove(all_canif_init_cfg_container_values[i])
                break
        parent = init_autosar.find(f"/InitialEcuC/CanIf/CanIfInitCfg/CanIfInitHohCfg")
        all_canif_init_cfg_container_values = parent.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        for i in range(len(all_canif_init_cfg_container_values)):
            if "init_CanIfHrhCfg" == all_canif_init_cfg_container_values[i].SHORT_NAME.valueOf_:
                all_canif_init_cfg_container_values.remove(all_canif_init_cfg_container_values[i])
                break
        
    def update_data_from_eb_resourcem_arxml_slot(self, initial_autosar: dict, sys_info_class: dict, ecuc_autosar_dict: dict):
        if (hasattr(sys_info_class, "enable_modules") and isinstance(sys_info_class.enable_modules, set) and ("ResourceM" in sys_info_class.enable_modules)) and \
            (("Can" in ecuc_autosar_dict.keys()) and ("CanIf" in ecuc_autosar_dict.keys())):
            resourcem_dict = ecuc_autosar_dict["ResourceM"].AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[0]
            can_dict = ecuc_autosar_dict["Can"].AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[0]
                
            net_to_core_dict = InitialCanCommonEcucParser.parser_net_and_core_mapping_by_resourcem_ecuc_arxml(resourcem_dict, can_dict)
            InitialCanCommonEcucParser.reset_net_to_core_dict()
            InitialCanCommonEcucParser.set_net_to_core_dict(net_to_core_dict)
            
            init_autosar = initial_autosar.AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[1]
            parent = init_autosar.find(f"/InitialEcuC/CanIf/CanIfCtrlDrvCfg")
            all_canif_init_cfg_container_values = parent.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        
            for i in range(len(all_canif_init_cfg_container_values)):
                _ctrl_name = all_canif_init_cfg_container_values[i].SHORT_NAME.valueOf_
                _core_id = InitialCanCommonEcucParser.get_coreid_of_net(_ctrl_name)
                container_value = all_canif_init_cfg_container_values[i]
            
                referance_value_key = "/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/CanIfCtrlCfg/CanIfCtrlCoreRef"
                referance_value_val = "/ActiveEcuC/EcuC/EcucHardware/EcucCoreDefinition" + str(_core_id)
                reset_canifctrlcoreref(self, container_value, referance_value_key)
                if InitialCanCommonEcucParser.resourcem_appeared_multicore_flag:
                    fill_canifctrlcoreref_by_resourcem(self, container_value, referance_value_key, referance_value_val)
        elif (hasattr(sys_info_class, "enable_modules") and isinstance(sys_info_class.enable_modules, set) and ("ResourceManager" in sys_info_class.enable_modules)) and \
            (("Can" in ecuc_autosar_dict.keys()) and ("CanIf" in ecuc_autosar_dict.keys())):
            resourcem_dict = ecuc_autosar_dict["ResourceManager"].AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[0]
            can_dict = ecuc_autosar_dict["Can"].AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[0]
                
            net_to_core_dict = InitialCanCommonEcucParser.parser_net_and_core_mapping_by_resourcemanager_ecuc_arxml(resourcem_dict, can_dict)
            InitialCanCommonEcucParser.reset_net_to_core_dict()
            InitialCanCommonEcucParser.set_net_to_core_dict(net_to_core_dict)
            
            init_autosar = initial_autosar.AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[1]
            parent = init_autosar.find(f"/InitialEcuC/CanIf/CanIfCtrlDrvCfg")
            all_canif_init_cfg_container_values = parent.SUB_CONTAINERS.ECUC_CONTAINER_VALUE
        
            for i in range(len(all_canif_init_cfg_container_values)):
                _ctrl_name = all_canif_init_cfg_container_values[i].SHORT_NAME.valueOf_
                _core_id = InitialCanCommonEcucParser.get_coreid_of_net(_ctrl_name)
                container_value = all_canif_init_cfg_container_values[i]
            
                referance_value_key = "/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/CanIfCtrlCfg/CanIfCtrlCoreRef"
                referance_value_val = "/ActiveEcuC/EcuC/EcucHardware/EcucCoreDefinition" + str(_core_id)
                reset_canifctrlcoreref(self, container_value, referance_value_key)
                if InitialCanCommonEcucParser.resourcem_appeared_multicore_flag:
                    fill_canifctrlcoreref_by_resourcem(self, container_value, referance_value_key, referance_value_val)

def get_template_config_json_dict(config_dict: dict) -> dict:
    rx_number = 0
    tx_number = 0
    canif_config_json_dict = {}
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
                msg_dict["module"] = "CAN_TP"
            elif ("NmAsrMessage"
                  in msg_value.message_attributes) and (0 != msg_value.message_attributes["NmAsrMessage"].value):
                msg_dict["module"] = "CAN_NM"
            elif ("XCP_Req_" in msg_dict["message_name"]) or ("XCP_Resp_" in msg_dict["message_name"]):
                msg_dict["module"] = "XCP"
            elif "SYNCTime" in msg_dict["message_name"]:
                msg_dict["module"] = "CAN_TSYN"
            else:
                msg_dict["module"] = "PDUR"

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
        canif_config_json_dict[dbc["dbc_name"]] = config_json_dict
        canif_config_json_dict["max_rx_number"] = rx_number
        canif_config_json_dict["max_tx_number"] = tx_number
    canif_config_json_dict["can_controller"] = can_controller_list

    return canif_config_json_dict


def create_sub_container_and_fill_value(self, initial_autosar, prameter_dict):
    module_name = prameter_dict["module_name"]
    parent_short_name = prameter_dict["parent_short_name"]
    short_name = prameter_dict["short_name"]
    container_name = prameter_dict["container_name"]
    fill_value_dict = prameter_dict["fill_values"]
    if module_name in self.mod_autosar_dict:
        ecuc_def = self.mod_autosar_dict[module_name]
   
    init_autosar = initial_autosar.AR_PACKAGES.AR_PACKAGE[0].ELEMENTS.ECUC_MODULE_CONFIGURATION_VALUES[1]
    parent = init_autosar.find(f"/InitialEcuC/{module_name}/" + parent_short_name)
    path = get_definition_ref(parent)

    cur_conf_def = ecuc_def.find(path + "/" + short_name)
    container_value = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, parent)
    
    return fill_prameter_and_referance(self, container_value, fill_value_dict)

def fill_canifctrlcoreref_by_resourcem(self, container_value, target_ref, target_val):    
    target_found = False
    
    if hasattr(container_value.REFERENCE_VALUES, "ECUC_REFERENCE_VALUE"):
        for ref in container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
            cur_defination_ref = get_definition_ref(ref)            
            if cur_defination_ref == target_ref:
                target_found = True
                set_value_ref(ref, target_val)
                AutosarUtil.add_annotation_value(ref, "AutoDerived")
                    
        if not target_found:         
            CanIfCtrlCoreRef_def = self.mod_autosar_dict["CanIf"].find(target_ref)
            AutosarUtil.set_reference(container_value.REFERENCE_VALUES, CanIfCtrlCoreRef_def)
            for ref in container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
                cur_defination_ref = get_definition_ref(ref) 
                if cur_defination_ref == target_ref:
                    set_value_ref(ref, target_val)
                    AutosarUtil.add_annotation_value(ref, "AutoDerived")
                
    return container_value

def reset_canifctrlcoreref(self, container_value, target_ref):    
    if hasattr(container_value.REFERENCE_VALUES, "ECUC_REFERENCE_VALUE"):
        for ref in container_value.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
            cur_defination_ref = get_definition_ref(ref)            
            if cur_defination_ref == target_ref:
                set_value_ref(ref, None)
                AutosarUtil.add_annotation_value(ref, "AutoDerived")
                
    return container_value

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


def get_canif_json_config(canif_config_json_dict):
    canif_json_config = canif_config_json_dict.copy()
    if "max_rx_number" in canif_json_config.keys():
        del canif_json_config["max_rx_number"]
    if "max_tx_number" in canif_json_config.keys():
        del canif_json_config["max_tx_number"]

    return canif_json_config


def get_canif_max_rx_num(canif_config_json_dict):
    canif_max_rx_num = canif_config_json_dict["max_rx_number"]

    return canif_max_rx_num


def get_canif_max_tx_num(canif_config_json_dict):
    canif_max_tx_num = canif_config_json_dict["max_tx_number"]

    return canif_max_tx_num


def get_canif_can_controller(canif_config_json_dict):
    can_controller_list = canif_config_json_dict["can_controller"]

    return can_controller_list


def get_canif_can_controller_num(canif_config_json_dict):
    can_controller_num = len(canif_config_json_dict["can_controller"])
    del canif_config_json_dict["can_controller"]

    return can_controller_num

def get_canif_can_controller_name(can_object_tmp_sorted):
    namelist = []
    for key,value in can_object_tmp_sorted.items():
        if isinstance(value, dict):
            if "Can/CanConfigSet/CanController"in value["DefinitionRef"]:
                namelist.append(key)
        else:
            pass
    return namelist