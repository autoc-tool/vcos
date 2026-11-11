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
整理Dcm模块配置信息模块
"""


def get_template_input_parameter_dict(config_dict_Dcm: dict) -> dict:
    dcm_config = {}
    dcm_dict = config_dict_Dcm["ArPackages"]["ActiveEcuC"]["Dcm"]

    # DcmGeneral
    DcmGeneral = dcm_tools_get_dict_by_definition_ref(dcm_dict, "DcmGeneral")
    # HeaderFile
    DcmHeaderFileInclusion = []
    if DcmGeneral["DcmHeaderFileInclusion"]["Value"] is not None:
        if isinstance(DcmGeneral["DcmHeaderFileInclusion"]["Value"], str):
            DcmHeaderFileInclusion.append(DcmGeneral["DcmHeaderFileInclusion"]["Value"])
        else:
            DcmHeaderFileInclusion = DcmGeneral["DcmHeaderFileInclusion"]["Value"]
    DcmHeaderFileInclusion = [x for x in DcmHeaderFileInclusion if x is not None and x != "<empty>" and x != '' and x != 'Dcm_Callout_Stubs.h']

    DcmConfigSet = dcm_tools_get_dict_by_definition_ref(dcm_dict, "DcmConfigSet")

    # Dsl
    DcmDsl = dcm_tools_get_dict_by_definition_ref(DcmConfigSet, "DcmDsl")
    DcmDslProtocol = dcm_tools_get_dict_by_definition_ref(DcmDsl, "DcmDslProtocol")
    DcmDslProtocolRows = dcm_tools_get_dict_rows_by_definition_ref_for_protocol_row(DcmDslProtocol, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow")
    DcmDslConnections = dcm_tools_get_dict_by_definition_ref_from_rows_for_dsl_connection(DcmDslProtocolRows, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection")
    DcmDslMainConnections = dcm_tools_get_dict_by_definition_ref_from_rows_for_main_connection(DcmDslConnections, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslMainConnection")
    DcmDslProtocolTxs = dcm_tools_get_dict_by_definition_ref_from_rows_for_rx_tx(DcmDslMainConnections, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslMainConnection/DcmDslProtocolTx")
    DcmDslProtocolRxs = dcm_tools_get_dict_by_definition_ref_from_rows_for_rx_tx(DcmDslMainConnections, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslMainConnection/DcmDslProtocolRx")
    DcmDslPeriodicTransmissions = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDslConnections, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslProtocol/DcmDslProtocolRow/DcmDslConnection/DcmDslPeriodicTransmission")
    DcmDslDiagResp = dcm_tools_get_dict_by_definition_ref(DcmDsl, "DcmDslDiagResp")
    DcmDslBuffers = dcm_tools_get_dict_rows_by_definition_ref(DcmDsl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslBuffer")
    DcmDslCallbackDCMRequestServices = dcm_tools_get_dict_rows_by_definition_ref(DcmDsl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsl/DcmDslCallbackDCMRequestService")

    # Dsd
    DcmDsd = dcm_tools_get_dict_by_definition_ref(DcmConfigSet, "DcmDsd")
    DcmDsdServiceTable = dcm_tools_get_dict_rows_by_definition_ref(DcmDsd, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceTable")
    DcmDsdService = dcm_tools_get_dict_by_definition_ref_from_rows_for_dsd_service(DcmDsdServiceTable, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceTable/DcmDsdService")
    DcmDsdServiceIdList = dcm_tools_get_sid_list(DcmDsdService)
    DcmDsdServiceRequestManufacturerNotification = dcm_tools_get_dict_rows_by_definition_ref(DcmDsd, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceRequestManufacturerNotification")
    DcmDsdServiceRequestSupplierNotification = dcm_tools_get_dict_rows_by_definition_ref(DcmDsd, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsd/DcmDsdServiceRequestSupplierNotification")

    # Dsp
    DcmDsp = dcm_tools_get_dict_by_definition_ref(DcmConfigSet, "DcmDsp")
    DcmDspRequestControls = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRequestControl")
    DcmDspDids = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDid")
    DcmDspDidRanges = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidRange")
    DcmDspDatas = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspData")
    DcmDspDidInfos = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDidInfo")
    DcmDspDataInfos = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspDataInfo")
    DcmDspPids = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspPid")
    DcmDspRoutines = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspRoutine")
    DcmDspVehInfos = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspVehInfo")
    DcmDspVehInfoDatas = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDspVehInfos, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspVehInfo/DcmDspVehInfoData")
    DcmDspCommonAuthorizations = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspCommonAuthorization")
    DcmDspComControl = dcm_tools_get_dict_by_definition_ref(DcmDsp, "DcmDspComControl")
    DcmDspComControl_allChannels = dcm_tools_get_dict_rows_by_definition_ref(DcmDspComControl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspComControl/DcmDspComControlAllChannel")
    DcmDspComControl_specificChannels = dcm_tools_get_dict_rows_by_definition_ref(DcmDspComControl, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspComControl/DcmDspComControlSpecificChannel")
    DcmDspComControlList = dcm_tools_get_com_control_channel(DcmDslMainConnections, DcmDspComControl_allChannels, DcmDspComControl_specificChannels)
    DcmDspMemory = dcm_tools_get_dict_by_definition_ref(DcmDsp, "DcmDspMemory")
    DcmDspMemoryIdInfoList = dcm_tools_get_dict_rows_by_definition_ref(DcmDspMemory, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspMemory/DcmDspMemoryIdInfo")
    DcmDspMemoryTransfer = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspMemoryTransfer")
    DcmDspRequestFileTransfer = dcm_tools_get_dict_by_definition_ref(DcmDsp, "DcmDspRequestFileTransfer")
    DcmDspSecurity = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity")
    DcmDspSecurityRow = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDspSecurity, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSecurity/DcmDspSecurityRow")
    DcmDspSession = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSession")
    DcmDspSessionRow = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDspSession, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspSession/DcmDspSessionRow")
    DcmDspVehicleSystemGroup = dcm_tools_get_dict_rows_by_definition_ref(DcmDsp, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspVehicleSystemGroup")
    DcmDspVehicleSystemGroupRow = dcm_tools_get_dict_by_definition_ref_from_rows(DcmDspVehicleSystemGroup, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmDsp/DcmDspVehicleSystemGroup/DcmDspVehicleSystemGroupRow")

    # ProcessingConditions
    DcmProcessingConditions = dcm_tools_get_dict_by_definition_ref(DcmConfigSet, "DcmProcessingConditions")
    DcmModeConditions = dcm_tools_get_dict_rows_by_definition_ref_to_add_index(DcmProcessingConditions, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmProcessingConditions/DcmModeCondition")
    DcmModeRules = dcm_tools_get_dict_rows_by_definition_ref_to_add_index(DcmProcessingConditions, "/AUTOSAR/EcucDefs/Dcm/DcmConfigSet/DcmProcessingConditions/DcmModeRule")



    dcm_config["dcm_DcmGeneral"] = DcmGeneral
    dcm_config["dcm_HeaderFile"] = DcmHeaderFileInclusion
    # dcm_config["dcm_DcmDsl"] = DcmDsl
    # dcm_config["dcm_DcmDslProtocol"] = DcmDslProtocol
    dcm_config["dcm_DcmDslProtocolRows"] = DcmDslProtocolRows
    dcm_config["dcm_DcmDslConnections"] = DcmDslConnections
    dcm_config["dcm_DcmDslMainConnections"] = DcmDslMainConnections
    dcm_config["dcm_DcmDslProtocolRxs"] = DcmDslProtocolRxs
    dcm_config["dcm_DcmDslProtocolTxs"] = DcmDslProtocolTxs
    dcm_config["dcm_DcmDslPeriodicTransmissions"] = DcmDslPeriodicTransmissions
    dcm_config["dcm_DcmDslDiagResp"] = DcmDslDiagResp
    dcm_config["dcm_DcmDslBuffers"] = DcmDslBuffers
    dcm_config["dcm_DcmDslCallbackDCMRequestServices"] = DcmDslCallbackDCMRequestServices
    # dcm_config["dcm_DcmDsd"] = DcmDsd
    dcm_config["dcm_DcmDsdServiceTable"] = DcmDsdServiceTable
    # dcm_config["dcm_DcmDsdSidTabId"] = DcmDsdSidTabId
    dcm_config["dcm_DcmDsdService"] = DcmDsdService
    dcm_config["dcm_DcmDsdServiceIdList"] = DcmDsdServiceIdList
    dcm_config["dcm_DcmDsdServiceRequestManufacturerNotification"] = DcmDsdServiceRequestManufacturerNotification
    dcm_config["dcm_DcmDsdServiceRequestSupplierNotification"] = DcmDsdServiceRequestSupplierNotification
    dcm_config["dcm_DcmDsp"] = DcmDsp
    dcm_config["dcm_DcmDspRequestControls"] = DcmDspRequestControls
    dcm_config["dcm_DcmDspDids"] = DcmDspDids
    dcm_config["dcm_DcmDspDidRanges"] = DcmDspDidRanges
    dcm_config["dcm_DcmDspDatas"] = DcmDspDatas
    dcm_config["dcm_DcmDspDidInfos"] = DcmDspDidInfos
    dcm_config["dcm_DcmDspDataInfos"] = DcmDspDataInfos
    dcm_config["dcm_DcmDspPids"] = DcmDspPids
    dcm_config["dcm_DcmDspRoutines"] = DcmDspRoutines
    dcm_config["dcm_DcmDspVehInfos"] = DcmDspVehInfos
    dcm_config["dcm_DcmDspVehInfoDatas"] = DcmDspVehInfoDatas
    dcm_config["dcm_DcmDspCommonAuthorizations"] = DcmDspCommonAuthorizations
    dcm_config["dcm_DcmDspComControl"] = DcmDspComControl
    dcm_config["dcm_DcmDspComControl_allChannels"] = DcmDspComControl_allChannels
    dcm_config["dcm_DcmDspComControl_specificChannels"] = DcmDspComControl_specificChannels
    dcm_config["dcm_DcmDspComControlList"] = DcmDspComControlList
    dcm_config["dcm_DcmDspMemory"] = DcmDspMemory
    dcm_config["dcm_DcmDspMemoryIdInfoList"] = DcmDspMemoryIdInfoList
    dcm_config["dcm_DcmDspMemoryTransfer"] = DcmDspMemoryTransfer
    dcm_config["dcm_DcmDspRequestFileTransfer"] = DcmDspRequestFileTransfer
    dcm_config["dcm_DcmDspSecurityRow"] = DcmDspSecurityRow
    dcm_config["dcm_DcmDspSessionRow"] = DcmDspSessionRow
    dcm_config["dcm_DcmDspVehicleSystemGroupRow"] = DcmDspVehicleSystemGroupRow

    # dcm_config["DcmProcessingConditions"] = ProcessingConditions
    dcm_config["dcm_DcmModeConditions"] = DcmModeConditions
    dcm_config["dcm_DcmModeRules"] = DcmModeRules

    return dcm_config

def dcm_tools_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"].split("/")[-1] == definition_ref:
                    return v
    return {}

def dcm_tools_get_dict_rows_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"] == definition_ref:
                    out_dict.update({k: v})
    return out_dict


def dcm_tools_get_dict_by_definition_ref_from_rows(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"] == definition_ref:
                            out_dict.update({k1: v1})
    return out_dict


def dcm_tools_get_dict_by_definition_ref_from_rows_for_dsd_service(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    index = -1
    for k, v in input_dict.items():
        if isinstance(v, dict):
            index = index + 1
            v.update({'dsd_service_table_index': index})
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"] == definition_ref and v1['DcmDsdServiceUsed']['Value']:
                            v1.update({'dsd_service_table_index': index})
                            out_dict.update({k1 + '_table' + str(index): v1})
    return out_dict


def dcm_tools_get_dict_rows_by_definition_ref_for_protocol_row(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    index = -1
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"] == definition_ref:
                    index = index + 1
                    v.update({'protocol_row_index': index})
                    out_dict.update({k: v})
    return out_dict

def dcm_tools_get_dict_rows_by_definition_ref_to_add_index(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    index = -1
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"] == definition_ref:
                    index = index + 1
                    v.update({'index': index})
                    out_dict.update({k: v})
    return out_dict

def dcm_tools_get_dict_by_definition_ref_from_rows_for_dsl_connection(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"] == definition_ref:
                            v1.update({'protocol_row_index': v['protocol_row_index']})
                            out_dict.update({k1 + '_protocol' + str(v['protocol_row_index']): v1})
    return out_dict


def dcm_tools_get_dict_by_definition_ref_from_rows_for_main_connection(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    index = -1
    connection_index = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"] == definition_ref:
                            index = index + 1
                            if str(v['protocol_row_index']) in connection_index.keys():
                                connection_index.update(
                                    {str(v['protocol_row_index']): connection_index[str(v['protocol_row_index'])] + 1})
                            else:
                                connection_index.update({str(v['protocol_row_index']): 0})
                            v1.update({'protocol_row_index': v['protocol_row_index']})
                            v1.update({'connection_index': connection_index[str(v['protocol_row_index'])]})
                            v1.update({'main_connection_index': index})
                            out_dict.update({
                                k1 + '_row' + str(v['protocol_row_index']) + '_connection' + str(connection_index[str(v['protocol_row_index'])]):
                                    v1
                            })
    return out_dict


def dcm_tools_get_dict_by_definition_ref_from_rows_for_rx_tx(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"] == definition_ref:
                            v1.update({'protocol_row_index': v['protocol_row_index']})
                            v1.update({'connection_index': v['connection_index']})
                            v1.update({'main_connection_index': v['main_connection_index']})
                            out_dict.update({k1 + '_main_connection' + str(v['main_connection_index']): v1})
    return out_dict


def dcm_tools_get_sid_list(input_dict):
    out_list = []
    for k, v in input_dict.items():
        if v["DcmDsdSidTabServiceId"]["Value"] not in out_list:
            out_list.append(v["DcmDsdSidTabServiceId"]["Value"])
    return out_list


def dcm_tools_get_com_control_channel(DcmDslMainConnections, DcmDspComControl_allChannels,
                                      DcmDspComControl_specificChannels) -> list:
    out_dict = []
    for k, v in DcmDslMainConnections.items():
        comm_channel = v['DcmDslProtocolComMChannelRef']['ValueRef'].split("/")[-1]
        if comm_channel not in out_dict:
            out_dict.append(comm_channel)
    for k, v in DcmDspComControl_allChannels.items():
        comm_channel = v['DcmDspAllComMChannelRef']['ValueRef'].split("/")[-1]
        if comm_channel not in out_dict:
            out_dict.append(comm_channel)
    for k, v in DcmDspComControl_specificChannels.items():
        comm_channel = v['DcmDspSpecificComMChannelRef']['ValueRef'].split("/")[-1]
        if comm_channel not in out_dict:
            out_dict.append(comm_channel)
    return out_dict
