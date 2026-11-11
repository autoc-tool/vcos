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

class InitialCanTpEcucParser():
    """
    All CanTp module page jump, display, save logic code.
    """

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict
        self.g_msg_list = []
        self.g_diag_connection_list = []
        self.is_fd = False
        self.channel_index = 0

    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, cfg_dict):
        """
        Update the tree structure of the CanTp module based on the imported DBC file and update the data.
        Arg:
            dict : All config dictionary
        """
        self.g_msg_list = []
        self.g_diag_connection_list = []
        self.is_fd = False
        self.channel_index = 0
        self.diag_dbc_info_pre_process(cfg_dict["dbc"])

        if len(self.g_msg_list) == 0:
            return

        self.update_cantp_general_cfg(initial_autosar)
        self.update_cantp_channel_cfg(initial_autosar)
        self.update_cantp_mainfunction_cfg(initial_autosar, cfg_dict)
        self.is_fd = False
        self.channel_index = 0

    def diag_dbc_info_pre_process(self, original_dbc):
        for dbc in original_dbc:
            dbc_name = dbc["attributes"]["network"]["define_value"]["DBName"]
            node_name = dbc["node_list"][0]
            for msg_name, msg_value in dbc["details"].items():
                connection = dbc_name + str(msg_value.message_attributes["DiagConnection"].value)
                is_tx = True if msg_value.senders[0] == node_name else False
                is_func = None
                if msg_value.message_attributes["DiagRequest"].value == 1 or msg_value.message_attributes["DiagResponse"].value == 1:
                    is_func = False
                if msg_value.message_attributes["DiagState"].value == 1:
                    is_func = True
                if is_func != None:
                    if msg_value.message_attributes["DiagConnection"].value == 57346:
                        logger.warning(f"Diag message connection id is invalid (57346)! --{dbc_name} --{msg_name}")
                    self.g_msg_list.append({"name":msg_name , "is_func": is_func, "is_tx": is_tx, "connection": connection})
                    self.g_diag_connection_list.append(connection)
                    self.is_fd |= msg_value.is_fd

    def update_cantp_general_cfg(self, initial: object):
        parent = initial.find("/InitialEcuC/CanTp/CanTpGeneral")
        # Set CanFd_Support
        self.set_numerical_param_value("CanTpFlexibleDataRateSupport", parent, str(self.is_fd).lower())

    def update_cantp_channel_cfg(self, initial: object):
        self.g_diag_connection_list = sorted(set(self.g_diag_connection_list)) # 去重、排序
        for connection_id in self.g_diag_connection_list:
            sdu_info = self.get_sdu_info_by_connection_id(connection_id)
            if sdu_info["is_func"] and len(sdu_info) != 3:
                continue
            if sdu_info["is_func"] == False and len(sdu_info) != 4:
                continue
            if sdu_info["is_func"] == True:
                channel_name = f"CanTpChannel_Func_{sdu_info['channelName']}"
            else:
                channel_name = f"CanTpChannel_{sdu_info['channelName']}"
            self.channel_index += 1
            ecuc_def = self.mod_autosar_dict["CanTp"]
            parent = initial.find("/InitialEcuC/CanTp/CanTpConfig")
            cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel")
            container_value = AutosarUtil.create_sub_container_value(channel_name, cur_conf_def, parent)
            if sdu_info["is_func"]:
                if "is_rx" in sdu_info.keys():
                    self.add_rx_nsdu_cfg_func(container_value, sdu_info)
                if "is_tx" in sdu_info.keys():
                    self.add_tx_nsdu_cfg_func(container_value, sdu_info)
            else:
                self.add_rx_nsdu_cfg_phy(container_value, sdu_info)
                self.add_tx_nsdu_cfg_phy(container_value, sdu_info)

    # 根据从dbc获取的Core信息后，根据Core信息自动推理CanTpMainFunction容器
    def update_cantp_mainfunction_cfg(self, initial_autosar: object, cfg_dict):
        dbcCoreList = self.process_dict_core_info(cfg_dict)  # 获取当前核信息CoreList
        ecuc_def = self.mod_autosar_dict["CanTp"]
        parent_container = initial_autosar.find("/InitialEcuC/CanTp/CanTpConfig")
        main_function_period = "0.005"

        # 单核情况
        if len(dbcCoreList) == 1 and dbcCoreList[0] == 0:
            container_def = "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpMainFunction"
            cur_conf_def = ecuc_def.find(container_def)
            container_name = "CanTp_MainFunction"
            container_value = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, parent_container)
            self.set_numerical_param_value('CanTpMainFunctionPeriod', container_value, main_function_period)
        # 多核情况
        else:
            for coreid in dbcCoreList:
                container_def = "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpMainFunction"
                cur_conf_def = ecuc_def.find(container_def)
                container_name = "CanTp_MainFunction_Core" + str(coreid)
                container_value = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, parent_container)
                self.set_numerical_param_value('CanTpMainFunctionPeriod', container_value, main_function_period)

    # 遍历dbc，获取Core信息
    def process_dict_core_info(self, dict):
        dbcList = dict['dbc']
        coreidList = list()
        for dbc in dbcList:
            if "coreId" not in dbc:
                coreid = 0
            elif dbc['coreId'] == None:
                coreid = 0
            else:
                coreid = dbc['coreId']
            coreidList.append(coreid)
        outputList = list(set(coreidList))
        return outputList

    def set_numerical_param_value(self, parameter_def_name, container_obj, value):
        numerical_param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                AutosarUtil.add_annotation_value(numerical_parameter, "AutoDerived")
                return

    def get_sdu_info_by_connection_id(self, connection_id):
        sdu_info = {}
        for msg in self.g_msg_list:
            if msg["connection"] == connection_id:
                dir = "is_tx" if msg["is_tx"] else "is_rx"
                sdu_info.update({"is_func": msg["is_func"]})
                sdu_info.update({dir: msg["name"]})
                sdu_info.update({"channelName": '_'.join(msg["name"].split("_")[2:])})
        return sdu_info

    def add_rx_nsdu_cfg_func(self, parent: object, sdu_info):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpRxNSdu")
        fun_container_value = AutosarUtil.create_sub_container_value("CanTpRxNSdu_Func", cur_conf_def, parent)
        self.set_textual_param_value("CanTpRxTaType", fun_container_value, "CANTP_FUNCTIONAL")  # Set CanTpRxTaType
        sdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + sdu_info["is_rx"] + "_oCanTp_PduR_Rx"
        self.set_reference_value("CanTpRxNSduRef", fun_container_value, sdu_ref)  # Set CanTpRxNSduRef
        self.add_rx_npdu_cfg(fun_container_value, sdu_info["is_rx"])

    def add_tx_nsdu_cfg_func(self, parent: object, sdu_info):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpTxNSdu")
        container_value = AutosarUtil.create_sub_container_value("CanTpTxNSdu", cur_conf_def, parent)
        self.set_textual_param_value("CanTpTxTaType", container_value, "CANTP_FUNCTIONAL")  # Set CanTpTxTaType
        sdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + sdu_info["is_tx"] + "_oPduR_CanTp_Tx"
        self.set_reference_value("CanTpTxNSduRef", container_value, sdu_ref)  # Set CanTpTxNSduRef
        self.add_tx_npdu_cfg(container_value, sdu_info["is_tx"])

    def add_rx_nsdu_cfg_phy(self, parent: object, sdu_info):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpRxNSdu")
        phy_container_value = AutosarUtil.create_sub_container_value("CanTpRxNSdu", cur_conf_def, parent)
        self.set_textual_param_value("CanTpRxTaType", phy_container_value, "CANTP_PHYSICAL")  # Set CanTpRxTaType
        sdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + sdu_info["is_rx"] + "_oCanTp_PduR_Rx"
        self.set_reference_value("CanTpRxNSduRef", phy_container_value, sdu_ref)  # Set CanTpRxNSduRef
        self.add_rx_npdu_cfg(phy_container_value, sdu_info["is_rx"])
        self.add_tx_fc_npdu_cfg(phy_container_value, sdu_info["is_tx"])

    def add_tx_nsdu_cfg_phy(self, parent: object, sdu_info):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpTxNSdu")
        container_value = AutosarUtil.create_sub_container_value("CanTpTxNSdu", cur_conf_def, parent)
        self.set_textual_param_value("CanTpTxTaType", container_value, "CANTP_PHYSICAL")  # Set CanTpTxTaType
        sdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + sdu_info["is_tx"] + "_oPduR_CanTp_Tx"
        self.set_reference_value("CanTpTxNSduRef", container_value, sdu_ref)  # Set CanTpTxNSduRef
        self.add_tx_npdu_cfg(container_value, sdu_info["is_tx"])
        self.add_rx_fc_npdu_cfg(container_value, sdu_info["is_rx"])

    def add_rx_npdu_cfg(self, parent: object, pdu_name: str):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpRxNSdu/CanTpRxNPdu")
        container_value = AutosarUtil.create_sub_container_value("CanTpRxNPdu", cur_conf_def, parent)
        pdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + pdu_name + "_oCanIf_CanTp_Rx"
        self.set_reference_value("CanTpRxNPduRef", container_value, pdu_ref)  # Set CanTpRxNPduRef

    def add_tx_npdu_cfg(self, parent: object, pdu_name: str):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpTxNSdu/CanTpTxNPdu")
        container_value = AutosarUtil.create_sub_container_value("CanTpTxNPdu", cur_conf_def, parent)
        pdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + pdu_name + "_oCanTp_CanIf_Tx"
        self.set_reference_value("CanTpTxNPduRef", container_value, pdu_ref)  # Set CanTpTxNPduRef

    def add_tx_fc_npdu_cfg(self, parent: object, pdu_name: str):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpRxNSdu/CanTpTxFcNPdu")
        container_value = AutosarUtil.create_sub_container_value("CanTpTxFcNPdu", cur_conf_def, parent)
        pdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + pdu_name + "_oCanTp_CanIf_Tx"
        self.set_reference_value("CanTpTxFcNPduRef", container_value, pdu_ref)  # Set CanTpTxFcNPduRef

    def add_rx_fc_npdu_cfg(self, parent: object, pdu_name: str):
        ecuc_def = self.mod_autosar_dict["CanTp"]
        cur_conf_def = ecuc_def.find("/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpTxNSdu/CanTpRxFcNPdu")
        container_value = AutosarUtil.create_sub_container_value("CanTpRxFcNPdu", cur_conf_def, parent)
        pdu_ref = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/" + pdu_name + "_oCanIf_CanTp_Rx"
        self.set_reference_value("CanTpRxFcNPduRef", container_value, pdu_ref)  # Set CanTpRxFcNPduRef

    def set_numerical_param_value(self, parameter_def_name, container_obj, value):
        numerical_param_list = container_obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                AutosarUtil.add_annotation_value(numerical_parameter, "AutoDerived")
                return

    def set_reference_value(self, reference_def_name, container_obj, value):
        reference_list = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name:
                set_value_ref(reference_parameter, value)
                AutosarUtil.add_annotation_value(reference_parameter, "AutoDerived")
                return

    def set_textual_param_value(self, parameter_def_name, container_obj, value):
        textual_param_list = container_obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE
        for textual_parameter in textual_param_list:
            if get_definition_ref(textual_parameter).split("/")[-1] == parameter_def_name:
                set_value(textual_parameter, value)
                AutosarUtil.add_annotation_value(textual_parameter, "AutoDerived")
                return
