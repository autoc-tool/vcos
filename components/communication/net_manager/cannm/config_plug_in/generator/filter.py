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
配置代码生成模块.
"""
# import json
from func_module.health_monitor.logger import logger

CanNmCoreChannelMappingDict = {0: {}}
CanNmChannelIdDict = {0: {}}
CanNmMainFunctionCoreIdDict = {}
CanNmMainFunctionPeriodDict = {}
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


def get_template_input_parameter_dict(config_dict: dict) -> dict:
    """
    Get input parameter for template generated code.
    Arg:
      config_dict : All config dictionary
    Return:
      Returns a dictionary of template input parameter
    """
    CanNmConfigDict = {}
    EcuCConfigDict = {}
    RteConfigDict = {}
    CanNmGlobalConfigDict = {}
    try:
        CanNmConfigDict = config_dict["bsw"]["cannm"]["ArPackages"]["ActiveEcuC"]["CanNm"]
        EcuCConfigDict = config_dict["bsw"]["ecuc"]["ArPackages"]["ActiveEcuC"]["EcuC"]
        if "rte" in config_dict.keys():
            RteConfigDict = config_dict["rte"]
    except KeyError:
        logger.warning("CanTSyn Config is not exist. VCOS Tools don't generate C code")
        return {}
    for CanNmConfigKey, CanNmConfigValue in CanNmConfigDict.items():
        if CanNmConfigKey == "DefinitionRef":
            continue
        if CanNmConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig':
            CanNmGlobalConfigDict = get_cannm_global_config(CanNmConfigKey, CanNmConfigValue, EcuCConfigDict, RteConfigDict)

    # with open('test.json', 'w') as f:
    #   json.dump(CanNmGlobalConfigDict, f, indent = 4)
    return CanNmGlobalConfigDict

def cannm_global_config_boolean_para():
    CanNmGlobalConfigBooleanParaList = ["CanNmBusLoadReductionEnabled", "CanNmBusSynchronizationEnabled",
        "CanNmComControlEnabled", "CanNmComUserDataSupport", "CanNmCoordinatorSyncSupport", "CanNmDevErrorDetect",
        "CanNmGlobalPnSupport", "CanNmImmediateRestartEnabled", "CanNmImmediateTxconfEnabled",
        "CanNmPassiveModeEnabled", "CanNmPduRxIndicationEnabled", "CanNmPnEiraCalcEnabled",
        "CanNmRemoteSleepIndEnabled", "CanNmStateChangeIndEnabled", "CanNmUserDataEnabled", "CanNmVersionInfoApi"]
    return CanNmGlobalConfigBooleanParaList

def get_cannm_global_config(name:str, data:dict, EcuCConfigDict:dict, RteConfigDict:dict) -> dict:
    CanNmGlobalDict = {}
    CanNmGlobalConfigDict = {}
    CanNmPnInfoDict = {}
    CanNmMainFuncDict = {}
    CanNmChannelConfigDict = {}
    for CanNmGlobalConfigKey, CanNmGlobalConfigValue in data.items():
        if CanNmGlobalConfigKey == "DefinitionRef":
            continue
        if CanNmGlobalConfigKey in cannm_global_config_boolean_para():
            if data[CanNmGlobalConfigKey]["Value"] is None:
                CanNmGlobalConfigDict[CanNmGlobalConfigKey] = False
            else:
                CanNmGlobalConfigDict[CanNmGlobalConfigKey] = data[CanNmGlobalConfigKey]["Value"]
        if CanNmGlobalConfigKey == "CanNmPnResetTime":
            if data[CanNmGlobalConfigKey]["Value"] is None:
                CanNmGlobalConfigDict[CanNmGlobalConfigKey] = 1950
            else:
                CanNmGlobalConfigDict[CanNmGlobalConfigKey] = int(data[CanNmGlobalConfigKey]["Value"] * 1000)
        if CanNmGlobalConfigKey == "CanNmTransmitCalloutFunc":
            if data[CanNmGlobalConfigKey]["Value"] == None or data[CanNmGlobalConfigKey]["Value"] == "":
                CanNmGlobalConfigDict["CanNmCalloutConfig"] = False
                CanNmGlobalConfigDict["CanNmCallbackFunction"] = "None"
            else:
                CanNmGlobalConfigDict["CanNmCalloutConfig"] = True
                CanNmGlobalConfigDict["CanNmCallbackFunction"] = data[CanNmGlobalConfigKey]["Value"]
        if CanNmGlobalConfigKey == "CanNmPnEiraRxNSduRef":
            if (data[CanNmGlobalConfigKey]["ValueRef"] == "" or data[CanNmGlobalConfigKey]["ValueRef"] == "<empty>"
                or data[CanNmGlobalConfigKey]["ValueRef"] is None):
                CanNmGlobalConfigDict["CanNmPnEiraRxNSduRef"] = "CANNM_PDU_ID"
            else:
                CanNmGlobalConfigDict["CanNmPnEiraRxNSduRef"] = "CANNM_PDU_ID_" + data[CanNmGlobalConfigKey]["ValueRef"].split('/')[-1]
    CanNmPnInfoDict = get_cannm_pninfo_config(name, data)
    CanNmChannelConfigDict = get_cannm_channel_config(name, data, EcuCConfigDict)
    CanNmMainFuncDict = get_cannm_mainfunction_config(name, data, RteConfigDict)
    CanNmGlobalDict = {
        "CanNmGlobalConfig": CanNmGlobalConfigDict,
        "CanNmPnInfo": CanNmPnInfoDict,
        "CanNmChannelConfig": CanNmChannelConfigDict,
        "CanNmMainFunction": CanNmMainFuncDict
    }
    return CanNmGlobalDict

def cannm_pninfo_config_para():
    CanNmPnInfoConfigParaList = ["CanNmPnInfoLength", "CanNmPnInfoOffset"]
    return CanNmPnInfoConfigParaList

def cannm_pn_filter_mask_byte_config_para():
    CanNmPnFilterMaskByteParaList = ["CanNmPnFilterMaskByteIndex", "CanNmPnFilterMaskByteValue"]
    return CanNmPnFilterMaskByteParaList

def cannm_cannmmainfunction_coreid(Rte_dict:dict, CanNmMainFunctionConfigKey:str):
    CanNmCoreId = 0
    if "task_mappings_info" in Rte_dict.keys():
        for task_name in list(Rte_dict["task_mappings_info"].keys()):
            if "schedulable_entity_info_list" in Rte_dict["task_mappings_info"][task_name].keys():
                if isinstance(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"], list):
                    for i in range(len(Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                        if Rte_dict["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == CanNmMainFunctionConfigKey:
                            CanNmCoreId = int(Rte_dict["task_mappings_info"][task_name]["core_id"])
    return CanNmCoreId

def get_cannm_mainfunction_config(name:str, data:dict, RteConfigDict:dict) -> dict:
    global CanNmMainFunctionCoreIdDict, CanNmMainFunctionPeriodDict
    CanNmMainFunctionConfigNum = 0
    CanNmMainFunctionConfigDict = {}
    CanNmMainFunctionDict = {}
    for CanNmMainFunctionConfigKey, CanNmMainFunctionConfigValue in data.items():
        if CanNmMainFunctionConfigKey == "DefinitionRef":
            continue
        if CanNmMainFunctionConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmMainFunction':
            CanNmMainFunctionDict[CanNmMainFunctionConfigNum] = CanNmMainFunctionConfigKey
            CanNmMainFunctionCoreIdDict[CanNmMainFunctionConfigNum] = cannm_cannmmainfunction_coreid(RteConfigDict, CanNmMainFunctionConfigKey)
            for CanNmMainFunctionKey, CanNmMainFunctionValue in CanNmMainFunctionConfigValue.items():
                if CanNmMainFunctionKey == 'DefinitionRef':
                    continue
                if CanNmMainFunctionValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmMainFunction/CanNmMainFunctionPeriod':
                    if data[CanNmMainFunctionConfigKey][CanNmMainFunctionKey]["Value"] is None:
                        CanNmMainFunctionPeriodDict[CanNmMainFunctionConfigNum] = 5
                    else:
                        CanNmMainFunctionPeriodDict[CanNmMainFunctionConfigNum] = int(data[CanNmMainFunctionConfigKey][CanNmMainFunctionKey]["Value"] * 1000)
            CanNmMainFunctionConfigNum = CanNmMainFunctionConfigNum + 1
    CanNmMainFunctionConfigDict = {
        "CanNmMainFunction": CanNmMainFunctionDict,
        "CanNmMainFunctionCoreId": CanNmMainFunctionCoreIdDict,
        "CanNmMainFunctionPeriod": CanNmMainFunctionPeriodDict
    }
    return CanNmMainFunctionConfigDict



def get_cannm_pninfo_config(name:str, data:dict) -> dict:
    CanNmPnInfoConfigDict = {}
    CanNmPnInfoDict = {}
    CanNmPnFilterMaskByteDict = {}
    CanNmPnFilterMaskByteNum = 0
    for CanNmPnInfoConfigKey, CanNmPnInfoConfigValue in data.items():
        if CanNmPnInfoConfigKey == "DefinitionRef":
            continue
        if CanNmPnInfoConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmPnInfo':
            for CanNmPnInfoKey, CanNmPnInfoValue in CanNmPnInfoConfigValue.items():
                if CanNmPnInfoKey == 'DefinitionRef':
                    continue
                if CanNmPnInfoKey in cannm_pninfo_config_para():
                    if data[CanNmPnInfoConfigKey][CanNmPnInfoKey]["Value"] == None or data[CanNmPnInfoConfigKey][CanNmPnInfoKey]["Value"] == "":
                        CanNmPnInfoDict[CanNmPnInfoKey] = 0
                    else:
                        CanNmPnInfoDict[CanNmPnInfoKey] = data[CanNmPnInfoConfigKey][CanNmPnInfoKey]["Value"]
                if CanNmPnInfoValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmPnInfo/CanNmPnFilterMaskByte':
                    CanNmPnFilterMaskByteDict[CanNmPnFilterMaskByteNum] = {}
                    for CanNmPnFilterMaskByteKey, CanNmPnFilterMaskByteValue in CanNmPnInfoValue.items():
                        if CanNmPnFilterMaskByteKey == 'DefinitionRef':
                            continue
                        if CanNmPnFilterMaskByteKey in cannm_pn_filter_mask_byte_config_para():
                            if data[CanNmPnInfoConfigKey][CanNmPnInfoKey][CanNmPnFilterMaskByteKey]["Value"] == None or data[CanNmPnInfoConfigKey][CanNmPnInfoKey][CanNmPnFilterMaskByteKey]["Value"] == "":
                                CanNmPnFilterMaskByteDict[CanNmPnFilterMaskByteNum][CanNmPnFilterMaskByteKey] = 0
                            else:
                                CanNmPnFilterMaskByteDict[CanNmPnFilterMaskByteNum][CanNmPnFilterMaskByteKey] = data[CanNmPnInfoConfigKey][CanNmPnInfoKey][CanNmPnFilterMaskByteKey]["Value"]
                    CanNmPnFilterMaskByteNum = CanNmPnFilterMaskByteNum + 1
    CanNmPnInfoConfigDict = {
        "CanNmPnInfoPara": CanNmPnInfoDict,
        "CanNmPnFilterMaskByte": CanNmPnFilterMaskByteDict
    }
    return CanNmPnInfoConfigDict

def cannm_channel_config_boolean_para():
    CanNmChannelConfigBooleanParaList = ["CanNmActiveWakeupBitEnabled", "CanNmAllNmMessagesKeepAwake",
        "CanNmBusLoadReductionActive", "CanNmCarWakeUpFilterEnabled", "CanNmCarWakeUpRxEnabled",
        "CanNmNodeDetectionEnabled", "CanNmNodeIdEnabled", "CanNmPnEnabled", "CanNmPnEraCalcEnabled",
        "CanNmPnHandleMultipleNetworkRequests", "CanNmFirstPduFailedRetry", "CanNmRepeatMsgIndEnabled"]
    return CanNmChannelConfigBooleanParaList

def cannm_channel_config_float_para():
    CanNmChannelConfigFloatParaList = ["CanNmImmediateNmCycleTime", "CanNmMsgCycleOffset", "CanNmMsgCycleTime", "CanNmMsgReducedTime",
        "CanNmMsgTimeoutTime", "CanNmRemoteSleepIndTime", "CanNmRepeatMessageTime", "CanNmTimeoutTime", "CanNmWaitBusSleepTime"]
    return CanNmChannelConfigFloatParaList

def cannm_channel_config_enum_para():
    CanNmChannelConfigEnumParaList = ["CanNmPduCbvPosition", "CanNmPduNidPosition"]
    return CanNmChannelConfigEnumParaList

def cannm_channel_config_int_para():
    CanNmChannelConfigIntParaList = ["CanNmCarWakeUpBitPosition", "CanNmCarWakeUpBytePosition",
                                    "CanNmCarWakeUpFilterNodeId", "CanNmImmediateNmTransmissions", "CanNmNodeId"]
    return CanNmChannelConfigIntParaList

def get_cannm_channel_config(name:str, data:dict, EcuCConfigDict:dict) -> dict:
    CanNmChannelConfigDict = {}
    CanNmChannelConfigNum = 0
    CanNmRxPduNum = 0
    for CanNmChannelConfigKey, CanNmChannelConfigValue in data.items():
        if CanNmChannelConfigKey == "DefinitionRef":
            continue
        if CanNmChannelConfigValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig':
            CanNmChannelConfigDict[CanNmChannelConfigNum] = {}
            for CanNmChannelKey, CanNmChannelValue in CanNmChannelConfigValue.items():
                if CanNmChannelKey == 'DefinitionRef':
                    continue
                if CanNmChannelKey in cannm_channel_config_boolean_para():
                    if data[CanNmChannelConfigKey][CanNmChannelKey]["Value"] is None:
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = False
                    else:
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = data[CanNmChannelConfigKey][CanNmChannelKey]["Value"]
                if CanNmChannelKey in cannm_channel_config_enum_para():
                    if data[CanNmChannelConfigKey][CanNmChannelKey]["Value"] is None:
                        if CanNmChannelKey == 'CanNmPduCbvPosition':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = "CANNM_PDU_BYTE_1"
                        else:
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = "CANNM_PDU_BYTE_0"
                    else:
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = data[CanNmChannelConfigKey][CanNmChannelKey]["Value"]
                if CanNmChannelKey in cannm_channel_config_float_para():
                    if data[CanNmChannelConfigKey][CanNmChannelKey]["Value"] is None:
                        if CanNmChannelKey == 'CanNmImmediateNmCycleTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 20
                        if CanNmChannelKey == 'CanNmMsgCycleOffset':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 0
                        if CanNmChannelKey == 'CanNmMsgCycleTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 500
                        if CanNmChannelKey == 'CanNmMsgReducedTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 50
                        if CanNmChannelKey == 'CanNmMsgTimeoutTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 60
                        if CanNmChannelKey == 'CanNmRemoteSleepIndTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 500
                        if CanNmChannelKey == 'CanNmRepeatMessageTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 1500
                        if CanNmChannelKey == 'CanNmTimeoutTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 2000
                        if CanNmChannelKey == 'CanNmWaitBusSleepTime':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 5000
                    else:
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = int(data[CanNmChannelConfigKey][CanNmChannelKey]["Value"] * 1000)
                if CanNmChannelKey in cannm_channel_config_int_para():
                    if data[CanNmChannelConfigKey][CanNmChannelKey]["Value"] is None:
                        if CanNmChannelKey == 'CanNmCarWakeUpBitPosition':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 0
                        if CanNmChannelKey == 'CanNmCarWakeUpBytePosition':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 0
                        if CanNmChannelKey == 'CanNmCarWakeUpFilterNodeId':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 0
                        if CanNmChannelKey == 'CanNmImmediateNmTransmissions':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 15
                        if CanNmChannelKey == 'CanNmNodeId':
                            CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 0
                    else:
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = data[CanNmChannelConfigKey][CanNmChannelKey]["Value"]
                if CanNmChannelKey == "CanNmComMNetworkHandleRef":
                    if (data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"] == "" or data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"] == "<empty>"
                        or data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"] is None):
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = 0
                    else:
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = "ComMConf_ComMChannel_" + data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"].split('/')[-1]
                        CanNmChannelIdDict[CanNmChannelConfigNum] =  "CANNM_CHANNEL_ID_" + data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"].split('/')[-1]
                if CanNmChannelKey == "CanNmPnEraRxNSduRef":
                    if (data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"] == "" or data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"] == "<empty>"
                        or data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"] is None):
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = "CANNM_PDU_ID"
                    else:
                        CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmChannelKey] = "CANNM_PDU_ID_" + data[CanNmChannelConfigKey][CanNmChannelKey]["ValueRef"].split('/')[-1]
                if CanNmChannelValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmRxPdu':
                    CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmRxPduNum] = {}
                    for CanNmRxPduKey, CanNmRxPduValue in CanNmChannelValue.items():
                        if CanNmRxPduKey == 'DefinitionRef':
                            continue
                        if CanNmRxPduKey == 'CanNmRxPduId':
                            if data[CanNmChannelConfigKey][CanNmChannelKey][CanNmRxPduKey]["Value"] is None:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmRxPduNum][CanNmRxPduKey] = 0
                            else:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmRxPduNum][CanNmRxPduKey] = data[CanNmChannelConfigKey][CanNmChannelKey][CanNmRxPduKey]["Value"]
                        if CanNmRxPduKey == "CanNmRxPduRef":
                            if (data[CanNmChannelConfigKey][CanNmChannelKey][CanNmRxPduKey]["ValueRef"] == ""
                                or data[CanNmChannelConfigKey][CanNmChannelKey][CanNmRxPduKey]["ValueRef"] == "<empty>"
                                or data[CanNmChannelConfigKey][CanNmChannelKey][CanNmRxPduKey]["ValueRef"] is None):
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmRxPduNum][CanNmRxPduKey] = "CANNM_PDU_ID"
                            else:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmRxPduNum][CanNmRxPduKey] = "CANNM_PDU_ID_" + data[CanNmChannelConfigKey][CanNmChannelKey][CanNmRxPduKey]["ValueRef"].split('/')[-1]
                    CanNmRxPduNum = CanNmRxPduNum + 1
                if CanNmChannelValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmTxPdu':
                    for CanNmTxPduKey, CanNmTxPduValue in CanNmChannelValue.items():
                        if CanNmTxPduKey == 'DefinitionRef':
                            continue
                        if CanNmTxPduKey == 'CanNmTxConfirmationPduId':
                            if data[CanNmChannelConfigKey][CanNmChannelKey][CanNmTxPduKey]["Value"] is None:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmTxPduKey] = 0
                            else:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmTxPduKey] = data[CanNmChannelConfigKey][CanNmChannelKey][CanNmTxPduKey]["Value"]
                        if CanNmTxPduKey == 'CanNmTxPduRef':
                            if (data[CanNmChannelConfigKey][CanNmChannelKey][CanNmTxPduKey]["ValueRef"] == ""
                                or data[CanNmChannelConfigKey][CanNmChannelKey][CanNmTxPduKey]["ValueRef"] == "<empty>"
                                or data[CanNmChannelConfigKey][CanNmChannelKey][CanNmTxPduKey]["ValueRef"] is None):
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmTxPduKey] = "CANNM_PDU_ID"
                                CanNmChannelConfigDict[CanNmChannelConfigNum]["CanNmCoreId"] = 0
                                CanNmCoreChannelMappingDict[CanNmChannelConfigNum] = 0
                            else:
                                CanNmTxRefPduId = data[CanNmChannelConfigKey][CanNmChannelKey][CanNmTxPduKey]["ValueRef"].split('/')[-1]
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmTxPduKey] = "CANIF_PDU_ID_" + CanNmTxRefPduId
                                if EcuCConfigDict["EcucConfigSet"]["EcucPduCollection"][CanNmTxRefPduId]["EcucPduDefaultCoreRef"]["ValueRef"] == None:
                                    CanNmChannelConfigDict[CanNmChannelConfigNum]["CanNmCoreId"] = 0
                                    CanNmCoreChannelMappingDict[CanNmChannelConfigNum] = 0
                                else:
                                    EcucPduDefaultCoreRef = EcuCConfigDict["EcucConfigSet"]["EcucPduCollection"][CanNmTxRefPduId]["EcucPduDefaultCoreRef"]["ValueRef"].split('/')[-1]
                                    CanNmChannelConfigDict[CanNmChannelConfigNum]["CanNmCoreId"] = EcuCConfigDict["EcucHardware"][EcucPduDefaultCoreRef]["EcucCoreId"]["Value"]
                                    CanNmCoreChannelMappingDict[CanNmChannelConfigNum] = EcuCConfigDict["EcucHardware"][EcucPduDefaultCoreRef]["EcucCoreId"]["Value"]
                                CanNmChannelConfigDict[CanNmChannelConfigNum]["CanNmTxConfirmationPduId"] = "CANNM_PDU_ID_" + CanNmTxRefPduId
                if CanNmChannelValue["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanNm/CanNmGlobalConfig/CanNmChannelConfig/CanNmUserDataTxPdu':
                    for CanNmUserDataTxPduKey, CanNmUserDataTxPduValue in CanNmChannelValue.items():
                        if CanNmUserDataTxPduKey == 'DefinitionRef':
                            continue
                        if CanNmUserDataTxPduKey == 'CanNmTxUserDataPduId':
                            if data[CanNmChannelConfigKey][CanNmChannelKey][CanNmUserDataTxPduKey]["Value"] is None:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmUserDataTxPduKey] = 0
                            else:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmUserDataTxPduKey] = data[CanNmChannelConfigKey][CanNmChannelKey][CanNmUserDataTxPduKey]["Value"]
                        if CanNmUserDataTxPduKey == 'CanNmTxUserDataPduRef':
                            if (data[CanNmChannelConfigKey][CanNmChannelKey][CanNmUserDataTxPduKey]["ValueRef"] == ""
                                or data[CanNmChannelConfigKey][CanNmChannelKey][CanNmUserDataTxPduKey]["ValueRef"] == "<empty>"
                                or data[CanNmChannelConfigKey][CanNmChannelKey][CanNmUserDataTxPduKey]["ValueRef"] is None):
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmUserDataTxPduKey] = "CANNM_PDU_ID"
                            else:
                                CanNmChannelConfigDict[CanNmChannelConfigNum][CanNmUserDataTxPduKey] = "PDUR_PDU_ID_" + data[CanNmChannelConfigKey][CanNmChannelKey][CanNmUserDataTxPduKey]["ValueRef"].split('/')[-1]
                                CanNmChannelConfigDict[CanNmChannelConfigNum]["CanNmTxUserDataPduId"] = "CANNM_PDU_ID_" + data[CanNmChannelConfigKey][CanNmChannelKey][CanNmUserDataTxPduKey]["ValueRef"].split('/')[-1]
            CanNmChannelConfigNum = CanNmChannelConfigNum + 1
    return CanNmChannelConfigDict

def get_template_dynamic_code_dict(config_dict: dict) -> dict:

    return config_dict
