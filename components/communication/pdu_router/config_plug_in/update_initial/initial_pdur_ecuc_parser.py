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
from basic_func_module.autosar_utils.utils import set_value, set_value_ref, set_definition_ref, get_definition_ref, get_value, get_short_name, get_value_ref
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
from basic_func_module.autosar_utils.types import PARAMETERS_CLASS

PDUR_UI_INDEX_MAP = {
    "pdur_general_ui": 1,
    "pdur_cfg_canif_ui": 2,
    "pdur_routing_path_group_ui": 3,
    "pdur_dest_pdu_ui": 4,
    "pdur_src_pdu_ui": 5
}
PDUR_MODULE_CONFIGURATION_MAP = {
    'CanIf': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'LinIf': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        True,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'CanTp': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'LinTp': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'DoIP': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'CanNm': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            True,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        True,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            False,
        'PduRUseTag':                 False
    },
    'Com': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': True,
        'PduRLowerModule':            False,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      False,
        'PduRTriggertransmit':        True,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            True,
        'PduRUseTag':                 False
    },
    'Dcm': {
        'PduRCancelReceive':          False,
        'PduRCancelTransmit':         False,
        'PduRCommunicationInterface': False,
        'PduRLowerModule':            False,
        'PduRRetransmission':         False,
        'PduRTransportProtocol':      True,
        'PduRTriggertransmit':        False,
        'PduRTxConfirmation':         False,
        'PduRUpperModule':            True,
        'PduRUseTag':                 False
    }
}
class InitialPduREcucParser():

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict
        self.src_pdu_count = 0
        self.dest_pdu_count = 0

    def update_initial_ecuc_from_dbc_ldf(self, initial_autosar, dict_cfg):
        """
        Update the tree structure of the CanNm module based on the imported DBC file and update the data.
        Arg:
            dict : All config dictionary
        """
        self.src_pdu_count = 0
        self.dest_pdu_count = 0
        self.xlsx_info_dict = {}
        self.pdur_core_info = []
        self.process_info_from_xlsx(dict_cfg)
        self.process_info_from_dbc(dict_cfg)
        self.process_info_from_ldf(dict_cfg)
        self.process_routing_path(initial_autosar)
        self.process_modules_list(initial_autosar)
        self.get_pdur_mainfunction_from_dbc(initial_autosar)

    def get_pdur_mainfunction_from_dbc(self, initial_autosar):
        ecuc_def = self.mod_autosar_dict["PduR"]
        parent_container = initial_autosar.find(f"/InitialEcuC/PduR")
        container = ecuc_def.find( "/AUTOSAR/EcucDefs/PduR/")
        nameList = self.get_pdur_mainfunction_from_dbc_exist_mainfunc(parent_container)
        if len(self.pdur_core_info) > 1:
            for coreid in self.pdur_core_info:
                container_def = "/AUTOSAR/EcucDefs/PduR/PduRMainFunction"
                cur_conf_def = ecuc_def.find(container_def)
                container_name = "PduR_MainFunction_Core" + str(coreid)
                if container_name not in nameList:
                    container_value = AutosarUtil.create_sub_container_value(container_name, cur_conf_def, parent_container)
    def get_pdur_mainfunction_from_dbc_exist_mainfunc(self, parent_container):
        nameList = list()
        for container in parent_container.CONTAINERS.ECUC_CONTAINER_VALUE:
            if container.DEFINITION_REF.valueOf_.split('/')[-1] == 'PduRMainFunction':
                nameList.append(container.SHORT_NAME.valueOf_)
        return nameList
    def get_pdur_bsw_modules_from_dbc(self):
        module_list = []
        for _, dest_pdu_value in self.pdur_dst_pdu_list.items():
            if "PduSuffix" not in dest_pdu_value or dest_pdu_value["PduSuffix"] == "":
                continue
            split_list = dest_pdu_value["PduSuffix"].split("_")
            for item in split_list:
                if item == 'Tx' or item == 'Rx' or item == 'PduR':
                    continue
                if item not in module_list:
                    module_list.append(item)
        for _, src_pdu_value in self.pdur_src_pdu_list.items():
            if "PduSuffix" not in src_pdu_value or src_pdu_value["PduSuffix"] == "":
                continue
            split_list = src_pdu_value["PduSuffix"].split("_")
            for item in split_list:
                if item == 'Tx' or item == 'Rx' or item == 'PduR':
                    continue
                if item not in module_list:
                    module_list.append(item)
        return module_list

    def pdur_pdu_mapping_src_to_dest(self):
        pdur_routing_path_pdu = {}
        for src_pdu_name, src_pdu_value in self.pdur_src_pdu_list.items():
            # 按照Src组织RoutingPath，每个RoutingPath只会有一个Src
            pdur_routing_path_pdu[src_pdu_name] = {"PduRSrcPdu": src_pdu_value, "PduRDestPduList": []}
            if src_pdu_name in self.pdur_dst_pdu_list: # 有和Src同名的Dst，两者为一组 - 普通的协议栈上下层模块间的一对一路由
                dest_pdu_name = src_pdu_name
                pdur_routing_path_pdu[src_pdu_name]["PduRDestPduList"].append(self.pdur_dst_pdu_list[dest_pdu_name])
            # 检查xlsx，确认是否牵涉到路由
            src_bus = src_pdu_value["Bus"]
            src_frame_id = src_pdu_value["frame_id"]
            dst_info_list = self.find_dest_info_by_src_info_from_excel(src_bus, src_frame_id)
            if len(dst_info_list) != 0: # Excel里面有相关SrcPdu的信息
                for dst_pdu_name, dst_pdu_value in self.pdur_dst_pdu_list.items():
                    for dst_info in dst_info_list:
                        if dst_pdu_value['Bus'] == dst_info['DstBus'] and dst_pdu_value['frame_id'] == dst_info['DstId']:
                            pdur_routing_path_pdu[src_pdu_name]['PduRDestPduList'].append(dst_pdu_value)
        return pdur_routing_path_pdu
    def find_dest_info_by_src_info_from_excel(self, src_bus, src_frame_id):
        dst_info_list = []
        for row, item in self.xlsx_info_dict.items():
            if row == 2: # 第二行是特殊信息
                continue
            if item['SrcBus'] == src_bus and item['SrcId'] == src_frame_id:
                dst_info = {}
                dst_info['DstBus'] = item['DstBus']
                dst_info['DstId'] = item['DstId']
                dst_info_list.append(dst_info)
        return dst_info_list


    def _get_can_pdu_connect_list(self, msg_key, msg_value, node_select):
        connect_list = []
        connect_dict = {
            "com": {
                "tx": ["Com_PduR_Tx", "PduR_CanIf_Tx"],
                "rx": ["CanIf_PduR_Rx", "PduR_Com_Rx"],
            },
            "cannm": {
                "tx": ["Com_CanNm_Tx", "Com_CanNm_Tx"],
                "rx": []
            },
            "dcm": {
                "tx": ["Dcm_PduR_Tx", "PduR_CanTp_Tx"],
                "rx": ["CanTp_PduR_Rx", "PduR_Dcm_Rx"],
            },
            "xcp": {
                "tx": [],
                "rx": []
            },
            "others": {
                "tx": ["Com_PduR_Tx", "PduR_CanIf_Tx"],
                "rx": ["CanIf_PduR_Rx", "PduR_Com_Rx"]
            }
        }
        if ("DiagRequest" in msg_value.message_attributes and msg_value.message_attributes["DiagRequest"].value == 1) or \
            ("DiagResponse" in msg_value.message_attributes and msg_value.message_attributes["DiagResponse"].value == 1) or \
            ("DiagState" in msg_value.message_attributes and msg_value.message_attributes["DiagState"].value == 1):
            module = "dcm"
        elif "NmAsrMessage" in msg_value.message_attributes and msg_value.message_attributes["NmAsrMessage"].value == 1:
            module = "cannm"
        elif "GenMsgILSupport" in msg_value.message_attributes and msg_value.message_attributes[
                "GenMsgILSupport"].value == 1:
            module = "com"
        elif "XCP" in msg_key or "CCP" in msg_key:
            module = "xcp"
        else:
            module = "others"

        if msg_value.senders[0] == node_select:
            dir = "tx"
        else:
            dir = "rx"

        if module is not None:
            connect_list = connect_dict[module][dir]
        return connect_list

    def get_pdur_src_pdu_from_dbc(self, pdur_dbc_list, ecuc_autosar):
        pdur_src_pdu_list = {}

        # 获取PduRSrcPduCoreRef
        mod_path = '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu/PduRSrcPduCoreRef'
        core_ref = self.get_core_name_from_os(mod_path, ecuc_autosar)

        for dbc_name, dbc_value in pdur_dbc_list.items():
            for msg_value in dbc_value:
                pdur_src_pdu = {}
                pdur_src_pdu.update({"Core": core_ref})
                pdur_src_pdu.update({"Bus": msg_value["bus_name"]})
                pdur_src_pdu.update({"frame_id": msg_value["frame_id"]})
                pdur_src_pdu.update({"ShortName": {"Value": msg_value["message_name"]}})
                pdur_src_pdu.update({"DestPduQueueDepth": {"Value": "1"}})
                pdur_src_pdu.update({"PduRSrcPduDirection": {"Value": "RECEIVE"}})
                pdur_src_pdu.update({"PduRTpThreshld": {"Value": ""}})
                pdur_src_pdu.update({"TxBufferId": {"Value": "PDUR_NO_BUFFER"}})
                if len(msg_value["pdu_suffix"]) == 0:
                    print(msg_value["message_name"])
                pdur_src_pdu.update({"PduSuffix": msg_value["pdu_suffix"][0]})
                if msg_value["node_selected"] == None:
                    print("No node selected")
                if msg_value["senders"][0] == msg_value["node_selected"]: # 板子为发
                    if "IfRoutingMsg" in msg_value and msg_value["IfRoutingMsg"] == True:
                        continue
                    if "TpRoutingMsg" in msg_value and msg_value["TpRoutingMsg"] == True: # 路由 - 不用生成Dcm一段
                        continue
                    if "DiagState" in msg_value and msg_value["DiagState"] == 1: # 功能寻址0x7DF只有收的Src，没有发的Src
                        continue
                    if "DiagResponse" in msg_value and msg_value["DiagResponse"] == 1:
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_DCM"}})
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"DCM_PDU_ID_{msg_value['message_name']}"}})
                    else:
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_COM"}})
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"COM_PDU_ID_{msg_value['message_name']}"}})
                    pdur_src_pdu.update({"PduRDestPdus": {"Value": f"List_{msg_value['message_name']}"}})
                else:
                    if "NmAsrMessage" in msg_value and msg_value["NmAsrMessage"] == 1:
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_CANNM"}})
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"CANNM_PDU_ID_{msg_value['message_name']}"}})
                    elif "DiagRequest" in msg_value and msg_value["DiagRequest"] == 1:
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_CANTP"}})
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"CANTP_PDU_ID_{msg_value['message_name']}"}})
                    elif "DiagState" in msg_value and msg_value["DiagState"] == 1:
                        if msg_value["RightFuncBus"] == False: # 从非excel标记的总线来的功能寻址
                            continue
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_CANTP"}})
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"CANTP_PDU_ID_{msg_value['message_name']}"}})
                    else:
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_CANIF"}})
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"CANIF_PDU_ID_{msg_value['message_name']}"}})
                    pdur_src_pdu.update({"PduRDestPdus": {"Value": f"List_{msg_value['message_name']}"}})
                pdur_src_pdu_list[msg_value['message_name']] = pdur_src_pdu
        return pdur_src_pdu_list

    def get_pdur_dest_pdu_from_dbc(self, pdur_dbc_list, ecuc_autosar):
        pdur_dest_pdu_list = {}
        # 获取PduRDestPduCoreRef
        mod_path = '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu/PduRDestPduCoreRef'
        core_ref = self.get_core_name_from_os(mod_path, ecuc_autosar)
        for dbc_name, dbc_value in pdur_dbc_list.items():
            for msg_value in dbc_value:
                pdur_dest_pdu = {}
                pdur_dest_pdu.update({"Core": core_ref})
                pdur_dest_pdu.update({"Bus": msg_value["bus_name"]})
                pdur_dest_pdu.update({"frame_id": msg_value["frame_id"]})
                pdur_dest_pdu.update({"ShortName": {"Value": msg_value["message_name"]}})
                pdur_dest_pdu.update({"PduSuffix": msg_value["pdu_suffix"][1]})
                pdur_dest_pdu.update({"PduRDestPduDirection": {"Value": "RECEIVE"}})
                pdur_dest_pdu.update({"IdxNum": {"Value": "0"}})
                pdur_dest_pdu.update({"StartIdx": {"Value": "0"}})
                if msg_value["node_selected"] == None:
                    print("No node selected")
                if msg_value["senders"][0] == msg_value["node_selected"]:
                    if "NmAsrMessage" in msg_value and msg_value["NmAsrMessage"] == 1:
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_CANNM"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": f"CANNM_PDU_ID_{msg_value['message_name']}"}})
                        pdur_dest_pdu.update({"DestPduDataProvision": {"Value": "PDUR_TRIGGERTRANSMIT"}})
                    elif "DiagResponse" in msg_value and msg_value["DiagResponse"] == 1:
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_CANTP"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": f"CANTP_PDU_ID_{msg_value['message_name']}"}})
                        pdur_dest_pdu.update({"DestPduDataProvision": {"Value": "PDUR_DIRECT"}})
                    else:
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_CANIF"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": f"CANIF_PDU_ID_{msg_value['message_name']}"}})
                        pdur_dest_pdu.update({"DestPduDataProvision": {"Value": "PDUR_DIRECT"}})
                else: # 板子为收
                    if "IfRoutingMsg" in msg_value and msg_value["IfRoutingMsg"] == True: # 路由 - 不用生成Com一段
                        continue
                    if "TpRoutingMsg" in msg_value and msg_value["TpRoutingMsg"] == True: # 路由 - 不用生成Dcm一段
                        continue
                    if "DiagRequest" in msg_value and msg_value["DiagRequest"] == 1:
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_DCM"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": f"DCM_PDU_ID_{msg_value['message_name']}"}})
                    elif "DiagState" in msg_value and msg_value["DiagState"] == 1: # DestPdu与RoutingPath不挂钩，生成也不影响
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_DCM"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": "DCM_RX_FUN_PDU_ID_RX"}})
                    else:
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_COM"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": f"COM_PDU_ID_{msg_value['message_name']}"}})
                    pdur_dest_pdu.update({"DestPduDataProvision": {"Value": "PDUR_DIRECT"}})
                pdur_dest_pdu_list[msg_value['message_name']] = pdur_dest_pdu
        return pdur_dest_pdu_list
    def get_pdur_core_id_from_dbc(self, pdur_dbc_list):
        for dbc in pdur_dbc_list:
            if "coreId" not in dbc:
                coreid = 0
            elif dbc['coreId'] == None:
                coreid = 0
            else:
                coreid = dbc['coreId']
            self.pdur_core_info.append(coreid)
        self.pdur_core_info = list(set(self.pdur_core_info))
    def get_pdur_core_id_from_ldf(self, pdur_ldf_list):
        for ldf in pdur_ldf_list:
            if "coreId" not in ldf:
                coreid = 0
            elif ldf['coreId'] == None:
                coreid = 0
            else:
                coreid = ldf['coreId']
            self.pdur_core_info.append(coreid)
        self.pdur_core_info = list(set(self.pdur_core_info))

    def _get_lin_pdu_connect_list(self, msg_key, msg_value, node_select):
        connect_list = []
        connect_dict = {
            "com": {
                "tx": ["Com_PduR_Tx", "PduR_LinIf_Tx"],
                "rx": ["LinIf_PduR_Rx", "PduR_Com_Rx"],
            },
            "dcm": {
                "tx": ["Dcm_PduR_Tx", "PduR_LinTp_Tx"],
                "rx": ["LinTp_PduR_Rx", "PduR_Dcm_Rx"],
            }
        }
        if msg_value.node_name == node_select:
            dir = "tx"
        else:
            dir = "rx"
        if msg_value.frame_id != 0x3C and msg_value.frame_id != 0x3D: # 判断诊断帧
            module = "com"
        else:
            module = "dcm"
        connect_list = connect_dict[module][dir]
        return connect_list

    def get_pdur_src_pdu_from_ldf(self, pdur_ldf_list, ecuc_autosar):
        pdur_src_pdu_list = {}
        mod_path = '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu/PduRSrcPduCoreRef'
        core_ref = self.get_core_name_from_os(mod_path, ecuc_autosar)

        for ldf_name, ldf_value in pdur_ldf_list.items():
            for node_type, node_value in ldf_value.items():
                for msg_name, msg_value in node_value.items():
                    pdur_src_pdu = {}
                    if msg_value["node_selected"] == None:
                        print("No node selected")
                    pdur_src_pdu.update({"Core": core_ref})
                    pdur_src_pdu.update({"Bus": msg_value["bus_name"]})
                    pdur_src_pdu.update({"frame_id": msg_value["frame_id"]})
                    pdur_src_pdu.update({"ShortName": {"Value": msg_value["message_name"]}})
                    pdur_src_pdu.update({"PduRTpThreshld": {"Value": ""}})
                    pdur_src_pdu.update({"DestPduQueueDepth": {"Value": "1"}})
                    pdur_src_pdu.update({"TxBufferId": {"Value": "PDUR_NO_BUFFER"}})
                    pdur_src_pdu.update({"PduRSrcPduDirection": {"Value": "RECEIVE"}})
                    if msg_value["node_name"] == msg_value["node_selected"]:
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"COM_PDU_ID_{msg_value['message_name']}"}})
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_COM"}})
                        pdur_src_pdu.update({"PduRDestPdus": {"Value": f"List_{msg_value['message_name']}"}})
                        pdur_src_pdu.update({"PduSuffix": msg_value["pdu_suffix"][0]})
                        pdur_src_pdu_list[pdur_src_pdu['ShortName']["Value"]] = pdur_src_pdu
                        pdur_src_pdu = {}
                    else:
                        pdur_src_pdu.update({"PduRSrcPduPduRBswModulesRef": {"Value": "ARC_PDUR_LINIF"}})
                        pdur_src_pdu.update({"PduRSourcePduHandleId": {"Value": f"LINIF_PDU_ID_{msg_value['message_name']}"}})
                        pdur_src_pdu.update({"PduRDestPdus": {"Value": f"List_{msg_value['message_name']}"}})
                        pdur_src_pdu.update({"PduSuffix": msg_value["pdu_suffix"][0]})
                        pdur_src_pdu_list[pdur_src_pdu['ShortName']["Value"]] = pdur_src_pdu
                        pdur_src_pdu = {}

                    if msg_value["node_selected"] == None:
                        print("No node selected")

                    if msg_value["node_name"] != msg_value["node_selected"]:
                        # 过滤掉LIN诊断报文，后面需要时通过手动配置添加 xww1103
                        continue
        return pdur_src_pdu_list

    def get_pdur_dest_pdu_from_ldf(self, pdur_ldf_list, ecuc_autosar):
        pdur_dest_pdu_list = {}
        mod_path = '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu/PduRDestPduCoreRef'
        core_ref = self.get_core_name_from_os(mod_path, ecuc_autosar)
        for ldf_name, ldf_value in pdur_ldf_list.items():
            for node_type, node_value in ldf_value.items():
                for msg_name, msg_value in node_value.items():
                    pdur_dest_pdu = {}
                    pdur_dest_pdu.update({"Core": core_ref})
                    pdur_dest_pdu.update({"Bus": msg_value["bus_name"]})
                    pdur_dest_pdu.update({"frame_id": msg_value["frame_id"]})
                    pdur_dest_pdu.update({"PduRDestPduDirection": {"Value": "RECEIVE"}})
                    pdur_dest_pdu.update({"DestPduDataProvision": {"Value": "PDUR_TRIGGERTRANSMIT"}})
                    pdur_dest_pdu.update({"IdxNum": {"Value": "0"}})
                    pdur_dest_pdu.update({"StartIdx": {"Value": "0"}})
                    if msg_value["node_selected"] == None:
                        print("No node selected")
                    if msg_value["node_name"] == msg_value["node_selected"]:
                        pdur_dest_pdu.update({"ShortName": {"Value": msg_value['message_name']}})
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_LINIF"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": f"LINIF_PDU_ID_{msg_value['message_name']}"}})
                        pdur_dest_pdu.update({"PduSuffix": msg_value["pdu_suffix"][1]})
                        pdur_dest_pdu_list[pdur_dest_pdu['ShortName']["Value"]] = pdur_dest_pdu
                        pdur_dest_pdu = {}
                    else:
                        pdur_dest_pdu.update({"ShortName": {"Value": msg_value['message_name']}})
                        pdur_dest_pdu.update({"PduRDestPduPduRBswModulesRef": {"Value": "ARC_PDUR_COM"}})
                        pdur_dest_pdu.update({"PduRDestPduHandleId": {"Value": f"COM_PDU_ID_{msg_value['message_name']}"}})
                        pdur_dest_pdu.update({"PduSuffix": msg_value["pdu_suffix"][1]})
                        pdur_dest_pdu_list[pdur_dest_pdu['ShortName']["Value"]] = pdur_dest_pdu
                        pdur_dest_pdu = {}
                    if msg_value["node_selected"] == None:
                        print("No node selected")
                    if msg_value["node_name"] != msg_value["node_selected"]:
                        # 过滤掉LIN诊断报文，后面需要时通过手动配置添加 xww1103
                        continue
        return pdur_dest_pdu_list

    def get_template_config_json_dict_from_dbc(self, dict_cfg) -> dict:
        pdur_config_json_dict = {}
        if "excel" in dict_cfg and "PduRCfg.xlsx" in dict_cfg["excel"]:
            xlsx = dict_cfg["excel"]["PduRCfg.xlsx"]
            self_req = xlsx[2]['self_diag_req'] # 第二行特殊的一行，记录了本节点的一些信息
            self_rsp = xlsx[2]['self_diag_rsp']
            func_bus = xlsx[2]['func_bus']
        else:
            self_req = None
            self_rsp = None
            func_bus = None
        for dbc in dict_cfg["dbc"]:
            config_json_dict = []
            diag_request_flag = False
            diag_response_flag = False
            for key, value in dbc["details"].items():
                dict = {}
                dict["comm_diag_type"] = "dbc"
                dict["frame_id"] = value.frame_id
                dict["message_name"] = value.message_name
                dict["node_selected"] = dbc["node_selected"]
                dict["pdu_suffix"] = self._get_can_pdu_connect_list(key, value, dbc["node_selected"])
                dict["bus_name"] = value.bus_name
                dict["senders"] = value.senders
                dict["TpRoutingMsg"] = False # 默认生成PduR_Dcm的一段
                dict["RightFuncBus"] = True # ZCAN和CHCAN1上面都会有功能寻址，需要判断，但默认为True
                if not dict["pdu_suffix"]:
                    continue
                if "GenMsgILSupport" in value.message_attributes and 0 == value.message_attributes[
                        'GenMsgILSupport'].value:
                    dict["IfRoutingMsg"] = True # 置位True则不生成PduR_Com的一段
                else:
                    dict["IfRoutingMsg"] = False
                if "NmAsrMessage" in value.message_attributes:
                    dict["NmAsrMessage"] = value.message_attributes["NmAsrMessage"].value
                else:
                    dict["NmAsrMessage"] = 0
                if "DiagRequest" in value.message_attributes:
                    dict["DiagRequest"] = value.message_attributes["DiagRequest"].value
                    if dict["DiagRequest"] == 1 and self_req != None and self_req != dict["frame_id"]: # 诊断路由
                        dict["TpRoutingMsg"] = True # 不生成PduR_Dcm的一段
                else:
                    dict["DiagRequest"] = 0
                if "DiagResponse" in value.message_attributes:
                    dict["DiagResponse"] = value.message_attributes["DiagResponse"].value
                    if dict["DiagResponse"] == 1 and self_rsp != None and self_rsp != dict["frame_id"]: # 诊断路由
                        dict["TpRoutingMsg"] = True # 不生成PduR_Dcm的一段
                else:
                    dict["DiagResponse"] = 0
                if "DiagState" in value.message_attributes:
                    dict["DiagState"] = value.message_attributes["DiagState"].value
                    if dict["DiagState"] == 1:
                        dict["TpRoutingMsg"] = False # 功能寻址一定是需要到Dcm的
                        if func_bus is not None and dict["bus_name"] != func_bus:
                            dict["RightFuncBus"] = False
                else:
                    dict["DiagState"] = 0
                config_json_dict.append(dict)
                pdur_config_json_dict[dbc["dbc_name"]] = config_json_dict

        return pdur_config_json_dict

    def get_template_config_json_dict_from_ldf(self, dict_cfg) -> dict:
        pdur_config_json_dict = {}
        for ldf in dict_cfg["ldf"]:
            pdur_config_json_dict[ldf["ldf_name"]] = {}
            dict = {"LIN_SLAVE_RESPONSE": {}, "LIN_MASTER_RESPONSE": {}}
            for key, value in ldf["details"].items():
                if value.frame_id == 0x3C or value.frame_id == 0x3D: # 跳过诊断帧不处理
                    continue
                if value.direction == "LIN_SLAVE_RESPONSE":
                    dict["LIN_SLAVE_RESPONSE"][key] = {}
                    dict["LIN_SLAVE_RESPONSE"][key]["comm_diag_type"] = "ldf"
                    dict["LIN_SLAVE_RESPONSE"][key]["message_name"] = value.message_name
                    dict["LIN_SLAVE_RESPONSE"][key]["node_name"] = value.node_name
                    dict["LIN_SLAVE_RESPONSE"][key]["bus_name"] = value.bus_name
                    dict["LIN_SLAVE_RESPONSE"][key]["direction"] = value.direction
                    dict["LIN_SLAVE_RESPONSE"][key]["node_selected"] = ldf["node_selected"]
                    dict["LIN_SLAVE_RESPONSE"][key]["frame_id"] = value.frame_id
                    dict["LIN_SLAVE_RESPONSE"][key]["pdu_suffix"] = self._get_lin_pdu_connect_list(
                        key, value, ldf["node_selected"])
                else:
                    dict["LIN_MASTER_RESPONSE"][key] = {}
                    dict["LIN_MASTER_RESPONSE"][key]["comm_diag_type"] = "ldf"
                    dict["LIN_MASTER_RESPONSE"][key]["message_name"] = value.message_name
                    dict["LIN_MASTER_RESPONSE"][key]["node_name"] = value.node_name
                    dict["LIN_MASTER_RESPONSE"][key]["bus_name"] = value.bus_name
                    dict["LIN_MASTER_RESPONSE"][key]["direction"] = value.direction
                    dict["LIN_MASTER_RESPONSE"][key]["node_selected"] = ldf["node_selected"]
                    dict["LIN_MASTER_RESPONSE"][key]["frame_id"] = value.frame_id
                    dict["LIN_MASTER_RESPONSE"][key]["pdu_suffix"] = self._get_lin_pdu_connect_list(
                        key, value, ldf["node_selected"])
            pdur_config_json_dict[ldf["ldf_name"]] = dict
        return pdur_config_json_dict

    def set_textual_parameter_value(self, parameter_def_name, textual_param_list, value):
        for textual_parameter in textual_param_list:
            if get_definition_ref(textual_parameter).split("/")[-1] == parameter_def_name:
                set_value(textual_parameter, value)
                AutosarUtil.add_annotation_value(textual_parameter, "AutoDerived")

    def set_reference_value(self, reference_def_name, reference_list, value):
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name:
                set_value_ref(reference_parameter, value)
                AutosarUtil.add_annotation_value(reference_parameter, "AutoDerived")

    def add_reference_value(self, reference_def, container_obj, value):
        # reference_def - 在bswmd文件中的定义
        # container_obj - 父节点（本函数直接索引其REFERENCE_VALUE）
        # value - Ref的值
        parent_node = container_obj.REFERENCE_VALUES
        reference_val_obj = AutosarUtil.set_reference(parent_node, reference_def)
        set_value_ref(reference_val_obj, value)

    def remove_reference_value(self, reference_def_name, container_obj, value):
        reference_list = container_obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE
        for reference_parameter in reference_list:
            if get_definition_ref(reference_parameter).split("/")[-1] == reference_def_name and get_value_ref(
                    reference_parameter) == value:
                reference_list.remove(reference_parameter)
                break

    def set_numerical_param_value(self, parameter_def_name, numerical_param_list, value):
        for numerical_parameter in numerical_param_list:
            if get_definition_ref(numerical_parameter).split("/")[-1] == parameter_def_name:
                set_value(numerical_parameter, value)
                AutosarUtil.add_annotation_value(numerical_parameter, "AutoDerived")

    def set_fake_string_param(self, parent, value, fake_param_name):
        """
        Temporarily set fake non standard parameter as string parameter.
        Must be removed after v0.5.1!!!!!
        """
        parameter_tag = "ECUC-TEXTUAL-PARAM-VALUE"
        tag = "ECUC-STRING-PARAM-DEF"
        parameter_obj = PARAMETERS_CLASS[parameter_tag]()
        parameter_obj.set_tag(parameter_tag)
        parameter_obj.set_parent(parent)
        set_definition_ref(parameter_obj, tag, f"/Autosar/PduR/FakeParameter/{fake_param_name}")
        set_value(parameter_obj, value)
        AutosarUtil.add_annotation_value(parameter_obj, "AutoDerived")
        getattr(parent, parameter_tag.replace("-", "_")).append(parameter_obj)
        return parameter_obj

    def create_routing_path_container(self, parent_obj, routing_path_name, src_pdu_container, dest_pdu_container_list,
                                      src_info_dict):
        mod_autosar_obj = self.mod_autosar_dict["PduR"]
        routing_path_def = mod_autosar_obj.find("/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath")
        routing_path_container = AutosarUtil.create_sub_container_value(routing_path_name, routing_path_def, parent_obj)
        # Set "PduRQueueDepth" value
        param_def_name = "PduRQueueDepth"
        self.set_numerical_param_value(param_def_name,
                                       routing_path_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                       src_info_dict["DestPduQueueDepth"]["Value"]) # RoutingPath的属性都来自于SrcPdu
        # Set "PduRTpThreshold" value
        param_def_name = "PduRTpThreshold"
        self.set_numerical_param_value(param_def_name,
                                       routing_path_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                       src_info_dict["PduRTpThreshld"]["Value"])
        # Set "PduRDestTxBufferId" value
        param_def_name = "PduRDestTxBufferId"
        self.set_textual_parameter_value(param_def_name,
                                         routing_path_container.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE,
                                         src_info_dict["TxBufferId"]["Value"])
        # Set "PduRSrcPduRRef" value
        reference_def_name = "PduRSrcPduRRef"
        self.set_reference_value(reference_def_name, routing_path_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                AutosarUtil.get_path(src_pdu_container).replace("/InitialEcuC", "/ActiveEcuC"))

        # Set "PduRDestPduRRef" value
        reference_def_name = "PduRDestPduRRef"
        reference_def = mod_autosar_obj.find("/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath/PduRDestPduRRef")
        if reference_def is not None:
            if len(dest_pdu_container_list) != 0:
                self.remove_reference_value(reference_def_name, routing_path_container, None) # 把原本的空的给删掉
            for dest_pdu_container in dest_pdu_container_list:
                self.add_reference_value(reference_def, routing_path_container,
                                        AutosarUtil.get_path(dest_pdu_container).replace("/InitialEcuC", "/ActiveEcuC"))
        return routing_path_container

    def create_dest_pdu_container(self, parent_obj, info_dict):
        mod_autosar_obj = self.mod_autosar_dict["PduR"]
        dest_pdu_def = mod_autosar_obj.find("/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu")
        container_name = info_dict["ShortName"]["Value"] + "_DstPdu"
        dest_pdu_container = AutosarUtil.create_sub_container_value(container_name, dest_pdu_def, parent_obj)
        # Set "PduRDestPduDataProvision" value
        param_def_name = "PduRDestPduDataProvision"
        self.set_textual_parameter_value(param_def_name, dest_pdu_container.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE,
                                         info_dict["DestPduDataProvision"]["Value"])
        # Set "PduRDestPduHandleId" value
        param_def_name = "PduRDestPduHandleId"
        self.set_numerical_param_value(param_def_name, dest_pdu_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                       str(self.dest_pdu_count))
        self.dest_pdu_count += 1
        # Set "PduRDestinationModuleRef" value
        parameter_def_name = "PduRDestinationModuleRef"
        bsw_module_name = "Com"
        self.set_textual_parameter_value(parameter_def_name,
                                         dest_pdu_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                         bsw_module_name.upper())
        # Set "PduRDestPduRef" value
        reference_def_name = "PduRDestPduRef"
        default_pdu_path = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/"
        pdu_suffix = info_dict["PduSuffix"]
        pdu_name = info_dict["ShortName"]["Value"] + f"_o{pdu_suffix}"
        default_pdu_path = default_pdu_path + pdu_name

        self.set_reference_value(reference_def_name, dest_pdu_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                 default_pdu_path)
        # Set "PduRDestPduCoreRef" value
        reference_def_name = "PduRDestPduCoreRef"
        default_pdu_path = info_dict['Core']
        self.set_reference_value(reference_def_name, dest_pdu_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                 default_pdu_path)
        return dest_pdu_container

    def create_module_container(self, parent_obj, info_dict):
        mod_autosar_obj = self.mod_autosar_dict["PduR"]
        modules_def = mod_autosar_obj.find("/AUTOSAR/EcucDefs/PduR/PduRBswModules")
        modules_name = info_dict
        modules_container = AutosarUtil.create_sub_container_value(modules_name, modules_def, parent_obj)
        return modules_container

    def create_src_pdu_container(self, parent_obj, info_dict):
        mod_autosar_obj = self.mod_autosar_dict["PduR"]
        src_pdu_def = mod_autosar_obj.find("/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu")
        container_name = info_dict["ShortName"]["Value"] + "_SrcPdu"
        src_pdu_container = AutosarUtil.create_sub_container_value(container_name, src_pdu_def, parent_obj)
        # Set "PduRSourcePduHandleId" value
        parameter_def_name = "PduRSourcePduHandleId"
        self.set_numerical_param_value(parameter_def_name,
                                       src_pdu_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                       str(self.src_pdu_count))
        self.src_pdu_count += 1
        # Set "PduRSrcModuleRef" value
        parameter_def_name = "PduRSrcModuleRef"
        bsw_module_name = "Com"
        self.set_textual_parameter_value(parameter_def_name,
                                         src_pdu_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                         bsw_module_name.upper())
        # Set "PduRSrcPduRef" value
        reference_def_name = "PduRSrcPduRef"
        default_pdu_path = "/ActiveEcuC/EcuC/EcucConfigSet/EcucPduCollection/"
        pdu_suffix = info_dict["PduSuffix"]
        pdu_name = info_dict["ShortName"]["Value"] + f"_o{pdu_suffix}"
        default_pdu_path = default_pdu_path + pdu_name
        self.set_reference_value(reference_def_name, src_pdu_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                 default_pdu_path)
        # Set "PduRSrcPduCoreRef" value
        reference_def_name = "PduRSrcPduCoreRef"
        default_pdu_path = info_dict['Core']
        self.set_reference_value(reference_def_name, src_pdu_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                 default_pdu_path)
        return src_pdu_container

    def update_routing_paths(self, info_dict, initial_autosar):
        pdur_ecuc_val = initial_autosar.find(f"/InitialEcuC/PduR")
        routing_paths_container_val = pdur_ecuc_val.find("/InitialEcuC/PduR/PduRRoutingPaths")
        for path_name, path_value in info_dict.items():
            dest_info_list = path_value["PduRDestPduList"]
            src_pdu_info_dict = path_value["PduRSrcPdu"]
            src_pdu_val = self.create_src_pdu_container(routing_paths_container_val, src_pdu_info_dict)
            dest_pdu_val_list = []
            for dest_pdu_info_dict in dest_info_list:
                dest_pdu_val = self.create_dest_pdu_container(routing_paths_container_val, dest_pdu_info_dict)
                dest_pdu_val_list.append(dest_pdu_val)
            self.create_routing_path_container(routing_paths_container_val, path_name, src_pdu_val, dest_pdu_val_list,
                                                src_pdu_info_dict) # routingpath的信息都来自SrcPduInfo，不需要传入DestPduInfo

        # Set "PduRMaxRoutingPathCnt" value
        parameter_def_name = "PduRMaxRoutingPathCnt"
        self.set_numerical_param_value(parameter_def_name,
                                       routing_paths_container_val.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                       '0')
        # Set "PduRMaxRoutingPathGroupCnt" value
        parameter_def_name = "PduRMaxRoutingPathGroupCnt"
        self.set_numerical_param_value(parameter_def_name,
                                       routing_paths_container_val.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                       '0')

    def update_bsw_modules(self, info_dict, initial_autosar):
        pdur_ecuc_val = initial_autosar.find(f"/InitialEcuC/PduR") # PduRBswModules是直接平铺在PduR一层
        for module_name in info_dict:
            is_existed = False
            container = pdur_ecuc_val.CONTAINERS.ECUC_CONTAINER_VALUE
            for itr in range(len(pdur_ecuc_val.CONTAINERS.ECUC_CONTAINER_VALUE)):
                if container[itr].DEFINITION_REF.valueOf_ == '/AUTOSAR/EcucDefs/PduR/PduRBswModules':
                    if module_name == container[itr].SHORT_NAME.valueOf_:
                        is_existed = True
                        break
            if is_existed is True:
                continue # 如果已经存在这个模块的BswModules容器，则不再创建，不再更新
            module_container = self.create_module_container(pdur_ecuc_val, module_name)
            if module_name not in PDUR_MODULE_CONFIGURATION_MAP:
                continue # 如果这个模块不存在于预设的列表中则不推理其内容
            config = PDUR_MODULE_CONFIGURATION_MAP[module_name]
            for parameter_def_name, parameter_def_value in config.items():
                self.set_numerical_param_value(parameter_def_name,
                                       module_container.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE,
                                       str(parameter_def_value).lower())
            ref_value = '/ActiveEcuC/' + module_name
            self.set_reference_value('PduRBswModuleRef',
                                     module_container.REFERENCE_VALUES.ECUC_REFERENCE_VALUE,
                                     ref_value)
        return
    def process_info_from_xlsx(self, dict_cfg):
        if "excel" in dict_cfg and "PduRCfg.xlsx" in dict_cfg["excel"]:
            self.xlsx_info_dict = dict_cfg["excel"]["PduRCfg.xlsx"]
    def process_info_from_dbc(self, dict_cfg):
        dict = {"dbc": {}}
        dict["dbc"] = self.get_template_config_json_dict_from_dbc(dict_cfg)
        self.pdur_src_pdu_list_from_dbc = self.get_pdur_src_pdu_from_dbc(dict["dbc"], dict_cfg["ecuc_autosar"])
        self.pdur_dest_pdu_list_from_dbc = self.get_pdur_dest_pdu_from_dbc(dict["dbc"], dict_cfg["ecuc_autosar"])
        self.get_pdur_core_id_from_dbc(dict["dbc"])
    def process_info_from_ldf(self, dict_cfg):
        dict = {"ldf": {}}
        dict["ldf"] = self.get_template_config_json_dict_from_ldf(dict_cfg)
        self.pdur_src_pdu_list_from_ldf = self.get_pdur_src_pdu_from_ldf(dict["ldf"], dict_cfg["ecuc_autosar"])
        self.pdur_dest_pdu_list_from_ldf = self.get_pdur_dest_pdu_from_ldf(dict["ldf"], dict_cfg["ecuc_autosar"])
        self.get_pdur_core_id_from_ldf(dict["ldf"])
    def process_routing_path(self, initial_autosar):
        self.pdur_src_pdu_list = {**self.pdur_src_pdu_list_from_dbc, **self.pdur_src_pdu_list_from_ldf}
        self.pdur_dst_pdu_list = {**self.pdur_dest_pdu_list_from_dbc, **self.pdur_dest_pdu_list_from_ldf}
        pdur_routing_path_pdu = self.pdur_pdu_mapping_src_to_dest()
        self.update_routing_paths(pdur_routing_path_pdu, initial_autosar)
    def process_modules_list(self, initial_autosar):
        module_list = self.get_pdur_bsw_modules_from_dbc()
        self.update_bsw_modules(module_list, initial_autosar)

    def get_core_name_from_os(self, mod_path, ecuc_autosar):
        os_ecuc = ecuc_autosar.get("Os", None)
        if os_ecuc is None:
            return ""
        ecuc_container_value_s = os_ecuc.find_nodes("ECUC-CONTAINER-VALUE")
        for ecuc_container_value in ecuc_container_value_s:
            if get_definition_ref(ecuc_container_value).split("/")[-1] != "OsPhysicalCore":
                continue
            for ecuc_numerical_param_value in getattr(ecuc_container_value.PARAMETER_VALUES, "ECUC_NUMERICAL_PARAM_VALUE", []):
                core_id = get_value(ecuc_numerical_param_value)
                if core_id not in [0, "0"]:
                    continue
                container_path =  ecuc_container_value.get_path()
                if isinstance(container_path, str):
                    return container_path
        return ""

class PduRModuleInitData:

    def __init__(self, mod_autosar_dict):
        self.mod_autosar_dict = mod_autosar_dict


def get_numerical_parameter_value(obj, param_name):
    for numerical_param in obj.PARAMETER_VALUES.ECUC_NUMERICAL_PARAM_VALUE:
        if get_definition_ref(numerical_param).split("/")[-1] == param_name:
            return get_value(numerical_param)
    raise AttributeError("Cannot find numerical parameter!!!")


def get_textual_parameter_value(obj, param_name):
    for textual_param in obj.PARAMETER_VALUES.ECUC_TEXTUAL_PARAM_VALUE:
        if get_definition_ref(textual_param).split("/")[-1] == param_name:
            return get_value(textual_param)
    return ""


def get_reference_value_ref(obj, reference_name):
    for reference in obj.REFERENCE_VALUES.ECUC_REFERENCE_VALUE:
        if get_definition_ref(reference).split("/")[-1] == reference_name:
            return get_value_ref(reference)
    raise AttributeError("Cannot find reference parameter!!!")

