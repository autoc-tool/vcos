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
from func_module.health_monitor.logger import logger
from basic_func_module.autosar_utils.autosar_util import AutosarUtil
"""
PduR配置代码生成模块.
"""


def get_template_dynamic_code_dict(config_dict: dict, config_dict_rte: dict) -> dict:
    PduRMainFuncList, PduRMultiCore_MainFunc = process_pdur_main_function(config_dict, config_dict_rte)
    PduRIsr, PduRMultiCore_Isr = process_pdur_isr(config_dict)
    PduRBswModules = process_pdur_bsw_module(config_dict)
    PduRGeneral = process_pdur_general(config_dict)
    PduRTxBuffers = process_pdur_tx_buffers(config_dict)
    MemPoolBuffers = process_mempool_buffer_pools(config_dict)
    PduRBufferPools = process_pdur_buffer_pools(config_dict, MemPoolBuffers)
    PduRRoutingPathGroups = process_pdur_routing_path_groups(config_dict)
    PduRRoutingPaths, PduRDestPduToRoutingPathGroupList = process_pdur_routing_paths(
        config_dict, PduRRoutingPathGroups)
    PduRRoutingPathGroupToDestPduList, PduRRoutingPathPduMacroList = process_pdur_routing_path_group_change_ref_into_id(
        config_dict, PduRRoutingPathGroups)
    PduREvCFifos, PduREvCFifosIF = process_pdur_event_caches_fifo(PduRRoutingPaths)
    PduRTxBuffers = process_pdur_tx_buffer(config_dict)
    PduRRoutingPaths = process_pdur_add_tx_buffer_info(PduRRoutingPaths, PduRTxBuffers)
    input_dict = {
      "PduRMultiCore": (PduRMultiCore_MainFunc or PduRMultiCore_Isr), # MainFunc或Isr有一个需要多核则需要支持
      "PduRMainFunctionList": PduRMainFuncList,
      "PduRBswModules": PduRBswModules,
      "PduRGeneral": PduRGeneral,
      "PduRRoutingPaths": PduRRoutingPaths,
      "PduRRoutingPathGroups": PduRRoutingPathGroupToDestPduList,
      "PduREvCFifos": PduREvCFifos,
      "PduREvCFifosIF": PduREvCFifosIF,
      "PduRBufferPools": PduRBufferPools,
      "PduRRoutingPathGroupList": PduRDestPduToRoutingPathGroupList,
      "PduRRoutingPathPduMacroList": PduRRoutingPathPduMacroList,
      "PduRTxBuffers": PduRTxBuffers,
      "PduRIsr": PduRIsr
    }
    return input_dict

def process_pdur_add_tx_buffer_info(PduRRoutingPaths, PduRTxBuffers):
    for routingPathName, routingPath in PduRRoutingPaths.items():
        if routingPath['PduRTxBuffer']['name'] == '':
            routingPath['PduRTxBuffer']['idx'] = 'PDUR_NO_BUFFER'
            continue
        idx = 0
        for txBufferName, txBuffer in PduRTxBuffers.items():
            if routingPath['PduRTxBuffer']['name'] == txBufferName:
                routingPath['PduRTxBuffer']['idx'] = str(idx) + 'u'
            idx = idx + 1
    return PduRRoutingPaths

def process_pdur_tx_buffer(config_dict: dict):
    PduRTxBuffers = {}
    PduRTxBuffer = {}
    PduRRoutingPaths_inJson = {}
    for _, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
            continue
        PduRRoutingPaths_inJson = item
        break
    if len(PduRRoutingPaths_inJson) == 0:
        logger.error(f"PduRRoutingPaths not exist!")
        return {}
    for key, item in PduRRoutingPaths_inJson.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRTxBuffer':
            BufferMaxLen = 1
            if item['PduRPduMaxLength']['Value'] is not None:
                BufferMaxLen = item['PduRPduMaxLength']['Value']
            PduRTxBuffer = {"PduRPduMaxLength": BufferMaxLen}
            PduRTxBuffers[key] = PduRTxBuffer
    return PduRTxBuffers

def process_pdur_main_function(config_dict: dict, config_dict_rte:dict):
    mainfuncNum = 0
    isMultiCoreSupport = False
    mainfunclist = []
    for name, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item :
            continue
        if item["DefinitionRef"].split('/')[-1] == 'PduRMainFunction':
            mainfuncNum = mainfuncNum + 1
            mainfunc_info = {
                'name': name,
                'period': item['PduRMainFunctionPeriod']['Value']
            }
            mainfunclist.append(mainfunc_info)
    if mainfuncNum == 0:
        return mainfunclist, isMultiCoreSupport
    elif mainfuncNum > 0:
        isMultiCoreSupport = True
    for mainfunc in mainfunclist:
        mainfunc_mapped = False
        if "task_mappings_info" not in config_dict_rte.keys():
            mainfunc['coreid'] = 0
            continue
        for task_name in list(config_dict_rte["task_mappings_info"].keys()):
            if "schedulable_entity_info_list" not in config_dict_rte["task_mappings_info"][task_name].keys():
                continue
            if isinstance(config_dict_rte["task_mappings_info"][task_name]["schedulable_entity_info_list"], list) == False:
                continue
            for i in range(len(config_dict_rte["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                if config_dict_rte["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == mainfunc['name']:
                    MainfuncCoreId = int(config_dict_rte["task_mappings_info"][task_name]["core_id"])
                    mainfunc['coreid'] = MainfuncCoreId
                    mainfunc_mapped = True # 本MainFunc在RTE的配置面板中已经关联Runnable
        if mainfunc_mapped == False:
            mainfunc['coreid'] = 0 # 默认为Core0
    return mainfunclist, isMultiCoreSupport

def process_pdur_isr(config_dict: dict):
    isrlist = []
    PduRMultiCore = False
    for name, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item :
            continue
        if item["DefinitionRef"].split('/')[-1] == 'PduRInterruptServiceRoutine':
            if item['PduROsIsrRef']['ValueRef'] is None and item['PduROsIsrRef']['ValueRef'] == '':
                continue
            isr_info = {
                'name': name,
                'isrref': item['PduROsIsrRef']['ValueRef'].split("/")[-1]
            }
            isrlist.append(isr_info)
    
    for name, item in config_dict['os']['ArPackages']['ActiveEcuC']['Os'].items():
        if "DefinitionRef" not in item :
            continue
        if item["DefinitionRef"].split('/')[-1] == 'OsApplication':
            templist = []
            if isinstance(item['OsAppIsrRef']['ValueRef'], str):
                templist.append(item['OsAppIsrRef']['ValueRef'])
            else:
                templist = item['OsAppIsrRef']['ValueRef']
            if templist is None or len(templist) == 0:
                continue
            for pdurIsr in isrlist:
                for osaIsr in templist:
                    if pdurIsr['isrref'] == osaIsr.split('/')[-1]:
                        pdurIsr['coreid'] = item['OsApplicationCoreRef']['ValueRef'][-1]
        if item["DefinitionRef"].split('/')[-1] == 'OsIsr':
            for pdurIsr in isrlist:
                if name == pdurIsr['isrref']:
                    if item['OsIsrInterruptSource']['Value'] is None or item['OsIsrInterruptSource']['Value'] == '':
                        pdurIsr['source'] = '0xFFFF'
                    else:
                        pdurIsr['source'] = str(item['OsIsrInterruptSource']['Value'])
    isrlist.sort(key = lambda x:x["coreid"])
    if len(isrlist) > 0: # 配置则为多核
        PduRMultiCore = True
    return isrlist, PduRMultiCore

def process_pdur_bsw_module(config_dict: dict) -> dict:
    PduRBswModules = {}
    PduR_Root = config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"]
    for key, item in PduR_Root.items():
        if key == "DefinitionRef":  # PduR目录下的DefinitionRef先排除
            continue
        if item["DefinitionRef"] == "/AUTOSAR/EcucDefs/PduR/PduRBswModules":
            PduRBswModules[key] = item
    return PduRBswModules


def process_pdur_general(config_dict: dict) -> dict:
    PduRGeneral_new = {}
    for _, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRGeneral':
            continue
        PduRGeneral_new = item
        break
    if len(PduRGeneral_new) == 0:
        logger.error(f"PduRGeneral not exist!")
        return {}
    PduRGeneral = {
      "ShortName": "PduRGeneral",
      "DevErrorDetect": PduRGeneral_new["PduRDevErrorDetect"],
      "MetaDataSupport": PduRGeneral_new["PduRMetaDataSupport"],
      "VersionInfoApi": PduRGeneral_new["PduRVersionInfoApi"],
      "ZeroCostOperation": PduRGeneral_new["PduRZeroCostOperation"],
      "PduRSpinlockRetryCounter": PduRGeneral_new["PduRSpinlockRetryCounter"],
    }
    return PduRGeneral


def process_pdur_routing_paths(config_dict: dict, PduRRoutingPathGroups: dict) -> dict:
    PduRRoutingPaths = {}
    PduRRoutingPath = {}
    RoutingPathGroupList = {}
    PduRRoutingPaths_inJson = {}
    for _, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
            continue
        PduRRoutingPaths_inJson = item
        break
    if len(PduRRoutingPaths_inJson) == 0:
        logger.error(f"PduRRoutingPaths not exist!")
        return {}, {}
    for pathName, pathItem in PduRRoutingPaths_inJson.items(): # 遍历RoutingPath的地方
        if pathName == "DefinitionRef":
            continue
        if (pathName == "PduRConfigurationId") | (pathName == "PduRMaxRoutingPathCnt") | (
            pathName == "PduRMaxRoutingPathGroupCnt"):
            continue
        if pathItem['DefinitionRef'] == "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath":
            PduRDestPdus, RoutingPathGroupList = process_pdur_routing_paths_dest_info(pathItem, config_dict, \
                PduRRoutingPathGroups, RoutingPathGroupList)
            PduRSrcPdus = process_pdur_routing_paths_src_info(pathName, pathItem, config_dict)
            PduRoutingPathName = process_pdur_path_name_dest(pathItem, config_dict)
            PduRDirectGateway, PduRTpThreshld =  process_pdur_direct_gateway(pathItem, config_dict)
            PduRQueueDepth = process_pdur_queue_depth(pathItem)
            PduRTxBufferRef = process_pdur_tx_buffer_ref_info(pathItem)
            PduRTxBufferRefDict = {"name": PduRTxBufferRef, 'idx': 'PDUR_NO_BUFFER'}
            PduRRoutingPath.clear
            PduRRoutingPath = {
              "PduPathName": pathName,
              "PduRoutingPathName": PduRoutingPathName,
              "SrcModule": PduRSrcPdus['SrcModule'],
              "SrcPduId": PduRSrcPdus['SrcPduId'],
              "PduRSrcPduCore": PduRSrcPdus['PduRSrcPduCore'],
              "SrcPduName": PduRSrcPdus['SrcPduName'],
              "PduRDestPdus": PduRDestPdus,
              "PduRDirectGateway": PduRDirectGateway,
              "PduRTpThreshld": PduRTpThreshld,
              "PduRDestinatnNum": len(PduRDestPdus),
              "PduRQueueDepth": PduRQueueDepth,
              "PduRTxBuffer": PduRTxBufferRefDict
            }
            PduRRoutingPaths[pathName] = PduRRoutingPath
    return PduRRoutingPaths, RoutingPathGroupList

def process_pdur_tx_buffer_ref_info(pathItem):
    RefInfo = pathItem['PduRDestTxBufferRef']['ValueRef']
    if RefInfo == None:
        return ''
    else:
        return RefInfo.split('/')[-1]

def process_pdur_routing_paths_dest_info(routingPath: dict, config_dict: dict, \
        PduRRoutingPathGroups: dict, RoutingPathGroupList: dict) -> dict:
    PduRDestPdu = {}
    destInfoStruct = {}
    for _, item in routingPath.items(): # 每次只传入一个DestPdu的信息，这里遍历处理DestPdu相关的数据
        if "DefinitionRef" not in item or \
            item['DefinitionRef'] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath/PduRDestPduRRef":
            continue
        temp_dest_info_list = []
        if isinstance(item['ValueRef'], str):
            temp_dest_info_list.append(item['ValueRef'])
        else:
            for ref in item['ValueRef']:
                temp_dest_info_list.append(ref)
        for ref_str in temp_dest_info_list:
            destInfoStruct = AutosarUtil.get_by_path_from_dict(config_dict["pdur"]["ArPackages"], ref_str[1:])
            if destInfoStruct is None or "DefinitionRef" not in destInfoStruct or \
                destInfoStruct["DefinitionRef"] != "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRDestPdu":
                continue
            DestModule, DestPduId, DestPduName = process_dest_info_module_id_name(destInfoStruct)
            PduRDestPduCore = process_dest_info_core_id(config_dict, destInfoStruct['PduRDestPduRef']['ValueRef'])
            DataProvision = process_dest_info_data_provision_processing(destInfoStruct)
            PduLenStrategy, PduLenInEcuC = process_dest_info_data_len_handling_strategy(destInfoStruct, config_dict)
            IdxNum, StartIdx, RoutingPathGroupList = process_dest_info_path_group(destInfoStruct, config_dict,
                PduRRoutingPathGroups, RoutingPathGroupList)
            PduRDestPdu_Value = {
                "DestModule": DestModule,
                "DestPduId": DestPduId,
                "PduRDestPduCore": PduRDestPduCore,
                "DestPduName": DestPduName,
                "IdxNum": IdxNum,
                "StartIdx": StartIdx,
                "DataProvision": DataProvision,
                "PduLenStrategy": PduLenStrategy,
                "PduLenInEcuC": PduLenInEcuC
            }
            PduRDestPdu[DestPduName] = PduRDestPdu_Value
    return PduRDestPdu, RoutingPathGroupList

def process_dest_info_module_id_name(destInfoStruct):
    ValueRefs = destInfoStruct['PduRDestPduRef']['ValueRef']
    DestModule = "PDUR_"
    DestModule_Temp = ""
    DestPduName_Temp = ""
    if isinstance(ValueRefs, str):
        DestPduName_Temp = ValueRefs.split("/")[-1]
        DestModule_Temp = extract_module_from_ecuc_pdu_ref(ValueRefs, 'Dest')
        DestModule += DestModule_Temp
        if DestModule.upper() == "PDUR_DOIP": 
            DestModule += "_TP" # 当前不考虑DoIP_If
        if DestModule.upper() == "PDUR_SOAD":
            DestModule += "_IF" # 当前不考虑SoAd_Tp

    elif isinstance(ValueRefs, list):
        for ref in ValueRefs:
            DestPduName_Temp = ref.split("/")[-1]
            DestModule_Temp = extract_module_from_ecuc_pdu_ref(ref, "Dest")
            DestModule += DestModule_Temp
            if DestModule.upper() == "PDUR_DOIP":
                DestModule += "_TP" # 当前不考虑DoIP_If
            if DestModule.upper() == "PDUR_SOAD":
                DestModule += "_IF" # 当前不考虑SoAd_Tp
    DestPduId = DestModule_Temp.upper() + "_PDU_ID_" + DestPduName_Temp
    return DestModule, DestPduId, DestPduName_Temp

def process_dest_info_core_id(config_dict, pduRef):
    ecucStruct = config_dict['ecuc']['ArPackages']['ActiveEcuC']['EcuC']
    pduCollection = ecucStruct['EcucConfigSet']['EcucPduCollection']
    if pduRef == ''or pduRef is None:
        return 0
    PduName = pduRef.split('/')[-1]
    if PduName not in pduCollection:
        return 0
    PduStruct = pduCollection[PduName]
    CoreRef = PduStruct['EcucPduDefaultCoreRef']['ValueRef']
    if CoreRef == '' or CoreRef is None:
        return 0
    CoreRefName = CoreRef.split('/')[-1]
    CoreEcucHardWare = ecucStruct['EcucHardware'][CoreRefName]
    CoreId = CoreEcucHardWare['EcucCoreId']['Value']
    if CoreId is None:
        return 0
    return CoreId

def process_dest_info_data_provision_processing(destInfoStruct):
    DataProvision = "PDUR_DIRECT"
    if destInfoStruct['PduRDestPduDataProvision']['Value'] is not None:
        DataProvision = destInfoStruct['PduRDestPduDataProvision']['Value']
    return DataProvision

def process_dest_info_data_len_handling_strategy(destInfoStruct: dict, config_dict: dict):
    PduLenStrategy = "PDUR_STRATEGY_UNUSED"
    if destInfoStruct['PduRDestPduLengthHandlingStrategy']['Value'] is not None:
        PduLenStrategy = destInfoStruct['PduRDestPduLengthHandlingStrategy']['Value']
    DestPduRef = destInfoStruct['PduRDestPduRef']['ValueRef']
    DestPduInEcuC = AutosarUtil.get_by_path_from_dict(config_dict["ecuc"]["ArPackages"], DestPduRef[1:])
    DestPduLenInEcuC = DestPduInEcuC['PduLength']['Value']
    if DestPduLenInEcuC is None:
        DestPduLenInEcuC = 0
    return PduLenStrategy, DestPduLenInEcuC

def process_dest_info_path_group(destInfoStruct: dict, config_dict: dict, PduRRoutingPathGroups: dict,
                                 RoutingPathGroupList: dict):
    destPduRef = destInfoStruct['PduRDestPduRef']['ValueRef']
    destPduName = destPduRef.split("/")[-1]
    destPduRoutingGroupIdx = []
    IdxNum = 0
    
    if destPduName in RoutingPathGroupList:
        for name, groupList in RoutingPathGroupList.items():
            if name == destPduName:
                IdxNum = groupList['IdxNum']
                StartIdx = groupList['StartIdx']
    else:
        for _, destPduRefInGroup in PduRRoutingPathGroups.items():
            destPduListInGroup = destPduRefInGroup['PduRDestPduList']
            for destPduInPathGroupRef in destPduListInGroup:
                if destPduInPathGroupRef is None:
                    continue
                destPduInPathGroup = AutosarUtil.get_by_path_from_dict(config_dict["pdur"]["ArPackages"],
                                                                       destPduInPathGroupRef[1:])
                if destPduInPathGroup is None: # 没找到，异常
                    continue
                if destPduRef == destPduInPathGroup['PduRDestPduRef']['ValueRef']:
                    IdxNum = IdxNum + 1
                    destPduRoutingGroupIdx.append(destPduRefInGroup['PduRRoutingPathGroupId'])
                    break # 该Group里面已经找到了这个DestPdu了，直接Break继续下一个
        if len(destPduRoutingGroupIdx) == 0:
            StartIdx = 0xFF
        else:
            StartIdx = 0
            for _, groupList in RoutingPathGroupList.items():
                StartIdx = StartIdx + len(groupList['destPduRoutingGroupIdx'])
            RoutingPathGroupList[destPduName] = {
                'IdxNum' : IdxNum,
                'StartIdx' : StartIdx,
                'destPduRoutingGroupIdx' : destPduRoutingGroupIdx,
            }
    return IdxNum, StartIdx, RoutingPathGroupList

def process_pdur_routing_paths_src_info(name: str, dict: dict, config_dict: dict) -> dict:
    PduRSrcPdus = {}
    srcInfoStruct = {}
    for key, item in dict.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == "/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath/PduRSrcPduRRef":
            srcInfoStruct = AutosarUtil.get_by_path_from_dict(config_dict["pdur"]["ArPackages"], item['ValueRef'][1:])
            if srcInfoStruct == None:
                continue
            if "DefinitionRef" in srcInfoStruct is False:
                continue
            if srcInfoStruct['DefinitionRef'] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu':
                continue
            SrcModule = "PDUR_"
            SrcModule_Temp = ""
            if isinstance(srcInfoStruct['PduRSrcPduRef']['ValueRef'], str):
                SrcPduName_Temp = srcInfoStruct['PduRSrcPduRef']['ValueRef'].split("/")[-1]
                SrcModule_Temp = extract_module_from_ecuc_pdu_ref(srcInfoStruct['PduRSrcPduRef']['ValueRef'], 'Src')
                SrcModule += SrcModule_Temp
                if SrcModule.upper() == "PDUR_DOIP":
                    SrcModule += "_TP" # 当前不考虑DoIP_If
                if SrcModule.upper() == "PDUR_SOAD":
                    SrcModule += "_IF" # 当前不考虑SoAd_Tp
            elif isinstance(srcInfoStruct['PduRSrcPduRef']['ValueRef'], list):
                for ref in srcInfoStruct['PduRSrcPduRef']['ValueRef']:
                    SrcPduName_Temp = ref.split("/")[-1]
                    SrcModule_Temp = extract_module_from_ecuc_pdu_ref(ref, 'Src')
                    SrcModule += SrcModule_Temp
                    if SrcModule.upper() == "PDUR_DOIP":
                        SrcModule += "_TP" # 当前不考虑DoIP_If
                    if SrcModule.upper() == "PDUR_SOAD":
                        SrcModule += "_IF" # 当前不考虑SoAd_Tp
            SrcPduId = SrcModule_Temp.upper() + "_PDU_ID_" + SrcPduName_Temp
            PduRSrcPduCore = process_dest_info_core_id(config_dict, srcInfoStruct['PduRSrcPduRef']['ValueRef'])
            PduRSrcPdus = {"SrcModule": SrcModule, "SrcPduId": SrcPduId, "SrcPduName": SrcPduName_Temp,
                           "PduRSrcPduCore": PduRSrcPduCore}
            break  # Src只可能有一个
    return PduRSrcPdus

def process_pdur_path_name_dest(dict: dict, config_dict: dict) -> str:
    for key, item in dict.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath/PduRSrcPduRRef':
            srcInfoStruct = AutosarUtil.get_by_path_from_dict(config_dict["pdur"]["ArPackages"], item['ValueRef'][1:])
            if srcInfoStruct == None:
                continue
            if "DefinitionRef" in srcInfoStruct is False:
                continue
            if srcInfoStruct['DefinitionRef'] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRSrcPdu':
                continue
            if isinstance(srcInfoStruct['PduRSrcPduRef']['ValueRef'], str):
                SrcPduName_Temp = srcInfoStruct['PduRSrcPduRef']['ValueRef'].split("/")[-1]
            break  # Src只可能有一个
    return SrcPduName_Temp

def process_pdur_direct_gateway(pathItem: dict, config_dict: dict) -> str:
    PduRTpThreshld = 0
    PduRDirectGateway = 'PDUR_GATEWAY_DIRECT'
    if pathItem['PduRTpThreshold']['Value'] != '' and pathItem['PduRTpThreshold']['Value'] != None:
        PduRTpThreshld = pathItem["PduRTpThreshold"]["Value"]
        PduRDirectGateway = 'PDUR_GATEWAY_ONTHEFLY'
    return PduRDirectGateway, PduRTpThreshld

def process_pdur_queue_depth(pathItem: dict) -> str:
    PduRQueueDepth = 1
    if pathItem['PduRQueueDepth']['Value'] != '' and pathItem['PduRQueueDepth']['Value'] != None:
        PduRQueueDepth = pathItem["PduRQueueDepth"]["Value"]
    return PduRQueueDepth

def process_pdur_routing_path_groups_info(name: str, dict: dict, config_dict: dict) -> dict:
    PduRRoutingPathGroupsInfo = {}
    idx = 0
    for key, item in dict.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPath/PduRRoutingPathGroupRef':
            GroupRef = item['ValueRef']
            if GroupRef is None:
                continue
            if isinstance(GroupRef, str):
                if GroupRef != '':
                    Group = AutosarUtil.get_by_path_from_dict(config_dict["pdur"]["ArPackages"], GroupRef[1:])
                    if Group is not None:
                        groupNameFromGroupList = GroupRef.split('/')[-1]
                        PduRRoutingPathGroupsInfo[groupNameFromGroupList] = Group
            else:
                for ref in GroupRef:
                    Group = AutosarUtil.get_by_path_from_dict(config_dict["pdur"]["ArPackages"], ref[1:])
                    if Group is None:
                        continue
                    groupNameFromGroupList = ref.split('/')[-1]
                    PduRRoutingPathGroupsInfo[groupNameFromGroupList] = Group
                    idx += 1
    return PduRRoutingPathGroupsInfo

def process_pdur_routing_path_groups(config_dict: dict) -> dict:
    PduRRoutingPathGroups = {}
    RoutingPathGroup = {}
    PduRRoutingPaths_inJson = {}
    for _, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
            continue
        PduRRoutingPaths_inJson = item
        break
    if len(PduRRoutingPaths_inJson) == 0:
        logger.error(f"PduRRoutingPaths not exist!")
        return {}
    for key, item in PduRRoutingPaths_inJson.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRRoutingPathGroup':
            temp_enabled_at_init = 'FALSE'
            if item['PduRIsEnabledAtInit']['Value'] is True:
                temp_enabled_at_init = 'TRUE'
            dest_pdu_list_of_routing_path_group = []
            if isinstance(item['PduRDestPduRRef']['ValueRef'], list): # 确保全部封装为List，减少后面判断的逻辑
                dest_pdu_list_of_routing_path_group = item['PduRDestPduRRef']['ValueRef']
            else:
                dest_pdu_list_of_routing_path_group.append(item['PduRDestPduRRef']['ValueRef'])
            RoutingPathGroup = {
                "PduRIsEnabledAtInit": temp_enabled_at_init,
                "PduRRoutingPathGroupId": item['PduRRoutingPathGroupId']['Value'],
                "PduRDestPduList": dest_pdu_list_of_routing_path_group,
            }
            PduRRoutingPathGroups[key] = RoutingPathGroup
    return PduRRoutingPathGroups

def process_pdur_routing_path_group_change_ref_into_id(config_dict: dict, PduRRoutingPathGroups: dict) -> dict:
    PduRRoutingPathGroupsAfter = {}
    GlobalPduMacroList = []
    GlobalRoutingPathGroupPduMacroDict = {}
    for name, item in PduRRoutingPathGroups.items():
        temp = []
        idxNum = 0
        startIdx = 0
        isStartIdxSet = False
        isListNone = False
        for pduRef in item['PduRDestPduList']:
            if pduRef is None:
                isListNone = True
                break
            destPduContain = AutosarUtil.get_by_path_from_dict(config_dict["pdur"]["ArPackages"], pduRef[1:])
            pduMacro = "PDUR_PDU_ID_" + destPduContain['PduRDestPduRef']['ValueRef'].split("/")[-1]
            if isStartIdxSet is False:
                startIdx = len(GlobalPduMacroList)
                isStartIdxSet = True
            GlobalPduMacroList.append(pduMacro)
            temp.append(pduMacro)
            idxNum = idxNum + 1
        if isListNone is True:
            continue
        PduRRoutingPathGroupsAfter[name] = item
        PduRRoutingPathGroupsAfter[name]['idxNum'] = idxNum
        PduRRoutingPathGroupsAfter[name]['startIdx'] = startIdx
        GlobalRoutingPathGroupPduMacroDict[name] = {}
        GlobalRoutingPathGroupPduMacroDict[name]['macro'] = temp
        GlobalRoutingPathGroupPduMacroDict[name]['groupid'] = item['PduRRoutingPathGroupId']

    return PduRRoutingPathGroupsAfter, GlobalRoutingPathGroupPduMacroDict

def process_pdur_tx_buffers(config_dict: dict) -> dict:
    PduRTxBuffers = {}
    PduRTxBuffer = {}
    BufferIdx = 0
    PduRRoutingPaths_inJson = {}
    for _, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
            continue
        PduRRoutingPaths_inJson = item
        break
    if len(PduRRoutingPaths_inJson) == 0:
        logger.error(f"PduRRoutingPaths not exist!")
        return {}
    for key, item in PduRRoutingPaths_inJson.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRTxBuffer':
            MaxLength = 1
            if item['PduRPduMaxLength']['Value'] is not None:
                MaxLength = item['PduRPduMaxLength']['Value']
            PduRTxBuffer = {"PduRPduMaxLength": MaxLength, "PduRTxBufferIdx": BufferIdx}
            BufferIdx += 1
            PduRTxBuffers[key] = PduRTxBuffer
    return PduRTxBuffers


def process_pdur_event_caches_fifo(PduRRoutingPaths:dict) -> dict:
    TP_MODULE = ['cantp', 'lintp', 'doip']
    NumTab = {}
    PduREvCFifoList_TP = []
    PduREvCFifoList_IF = []
    for RoutingPathName, RoutingPath in PduRRoutingPaths.items():
        DestCoreList = []
        for destPduName, destPdu in RoutingPath['PduRDestPdus'].items():
            if destPdu["PduRDestPduCore"] == RoutingPath['PduRSrcPduCore']: # 无跨核情况
                continue
            for tpmodule in TP_MODULE:
                if (RoutingPath['SrcModule'].lower()).find(tpmodule) != -1 or (destPdu['DestModule'].lower()).find(tpmodule) != -1:
                    DestCoreList.append(destPdu['PduRDestPduCore']) # 是TP类的
                    break
            else: # FOR循环遍历完毕没有找到，说明不是TP类型的，则归入IF类型跨核
                append_temp = {'name':RoutingPathName, 'coreid': destPdu["PduRDestPduCore"]}
                PduREvCFifoList_IF.append(append_temp)
        if len(DestCoreList) == 0:
            continue
        DestCoreList = list(set(DestCoreList))
        for DestCoreId in DestCoreList:
            if str(DestCoreId) not in NumTab.keys():
                NumTab[str(DestCoreId)] = 1
            else:
                NumTab[str(DestCoreId)] = NumTab[str(DestCoreId)] + 1
    for coreId, fifolen in NumTab.items():
        PduREvCFifo = {"PduREvCFifoLength": fifolen, "PduREventQueueCore": int(coreId)}
        PduREvCFifoList_TP.append(PduREvCFifo)
    return PduREvCFifoList_TP, PduREvCFifoList_IF

def process_pdur_event_caches(config_dict: dict, PduRRoutingPaths:dict) -> dict:
    PduREvCFifos = {}
    PduREvCFifo = {}
    PduRRoutingPaths_inJson = {}
    for _, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
            continue
        PduRRoutingPaths_inJson = item
        break
    if len(PduRRoutingPaths_inJson) == 0:
        logger.error(f"PduRRoutingPaths not exist!")
        return {}
    for key, item in PduRRoutingPaths_inJson.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduREvCFifo':
            EventCacheLength = 1
            if item['PduREvCFifoLength']['Value'] is not None:
                EventCacheLength = item['PduREvCFifoLength']['Value']
                EventQueueCoreRef = item['PduREventQueueCoreRef']['ValueRef'].split("/")[-1]
            PduREvCFifo = {"PduREvCFifoLength": EventCacheLength, "PduREventQueueCore": EventQueueCoreRef[-1]}

            PduREvCFifos[key] = PduREvCFifo
    return PduREvCFifos


def process_mempool_buffer_pools(config_dict: dict) -> dict:
    TotalNum = 0
    if 'mempool' not in config_dict:
        return {}
    mempool = config_dict['mempool']['ArPackages']['ActiveEcuC']['MemPool']
    poolOfMemPool = []
    MemPoolPduRRef = []
    for key, item in mempool.items():
        if "DefinitionRef" not in item:
            continue
        if item["DefinitionRef"] == '/AUTOSAR/EcucDefs/MemPool/MemPool':
            poolInfo = {"name": key, "BlockNum": item['MemBlockCount']}
            poolOfMemPool.append(poolInfo)
        if item["DefinitionRef"] == '/AUTOSAR/EcucDefs/MemPool/MemModule':
            if item['BswModuleRef']['ValueRef'] == '' or item['BswModuleRef']['ValueRef'] == None:
                continue
            if item['BswModuleRef']['ValueRef'].split("/")[-1] == "PduR":
                if isinstance(item['MemPoolRef']['ValueRef'], list):
                    for item1 in item['MemPoolRef']['ValueRef']:
                        MemPoolPduRRef.append(item1.split('/ActiveEcuC/MemPool/')[-1])
                else:
                    MemPoolPduRRef.append(item['MemPoolRef']['ValueRef'].split('/ActiveEcuC/MemPool/')[-1])
    for item in MemPoolPduRRef:
        for item1 in poolOfMemPool:
            if item == item1['name']:
                TotalNum = TotalNum + item1['BlockNum']['Value']
    return {"name": "MemPool",
                      "block_size": 0, # 无意义变量
                      "block_num": TotalNum}



def process_pdur_buffer_pools(config_dict: dict, MemPoolBuffers: dict) -> dict:
    PduRBufferPools = []
    PduRBufferPool = {}
    PduRRoutingPaths_inJson = {}
    for _, item in config_dict["pdur"]["ArPackages"]["ActiveEcuC"]["PduR"].items():
        if "DefinitionRef" not in item or item["DefinitionRef"] != '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths':
            continue
        PduRRoutingPaths_inJson = item
        break
    if len(PduRRoutingPaths_inJson) == 0:
        logger.error(f"PduRRoutingPaths not exist!")
        return {}
    for key, item in PduRRoutingPaths_inJson.items():
        if key == "DefinitionRef":
            continue
        if item['DefinitionRef'] == '/AUTOSAR/EcucDefs/PduR/PduRRoutingPaths/PduRBufferPool':
            BufferPoolBlockSize = 0
            BufferPoolBlockNumber = 0
            if item['PduRBufferPoolBlockSize']['Value'] is not None:
                BufferPoolBlockSize = item['PduRBufferPoolBlockSize']['Value']
                BufferPoolBlockNumber = item['PduRBufferPoolBlockNumber']['Value']
            if BufferPoolBlockSize == 0 or BufferPoolBlockNumber == 0: # 缓存区块长度或个数配置为0的缓存区直接跳过
                continue
            PduRBufferPool = {"name": key,
                            "block_size": BufferPoolBlockSize,
                            "block_num": BufferPoolBlockNumber}
            PduRBufferPools.append(PduRBufferPool)
    PduRBufferPools.sort(key = lambda x:x["block_size"])
    if MemPoolBuffers != {}:
        PduRBufferPools.append(MemPoolBuffers) # 为MemPool提供一组BufferManager
    return PduRBufferPools


def add_dest_pdu_info_into_routing_path_group(PduRRoutingPathGroups: dict, PduRRoutingPaths: dict) -> dict:
    output = PduRRoutingPathGroups
    for pathName, path in PduRRoutingPaths.items():
        if (pathName == "PduRConfigurationId") | (pathName == "PduRMaxRoutingPathCnt") | (
          pathName == "PduRMaxRoutingPathGroupCnt"):
            continue
        if path['PduRRoutingPathGroup']:
            for groupRefNameFromPath, groupRefFromPath in path['PduRRoutingPathGroup'].items():
                groupNameInRefFromPath = groupRefFromPath['DefinitionRef'].split('/')[-1]
                for groupNameFromGroupList, groupValueFromGroupList in output.items():
                    if groupRefNameFromPath == groupNameFromGroupList:
                        append_temp = pathName
                        groupValueFromGroupList['PduRDestPdu'].append(append_temp)
                        groupValueFromGroupList['PduRDestPduNum'].append(len(path['PduRDestPdus']))
    return output

def extract_module_from_ecuc_pdu_ref(valueRef: str, direction: str) -> str:
    ModName = ''
    OtherModule = ['_Rx', '_Tx', '_RX', '_TX', '_PduR', 'PduR_', '_PDUR', 'PDUR_', '_pdur', 'pdur_']
    if direction == 'Src':
        if valueRef.split('_')[1] == 'MasterReq':
            ModName = 'LINTP'
        elif valueRef.split('_')[1] == 'SlaveResp':
            ModName = 'LINTP'
        else:
            valueRef = valueRef.split('/')[-1]
            relStringList = valueRef.split('_o') # 需要考虑模块名内含有'_'
            if len(relStringList) != 0: # 按照'_o'搜索，非原有逻辑
                relString = relStringList[-1]
                if relString.find("PduR") != -1: # 兼容现在CanNm和Com之间的链接，需要修改，【参见ChangedIdI3a4995eaab853f71455f947cea3e5354213d6a73】
                    ModName = relString
                    for item in OtherModule:
                        ModName = ModName.replace(item, "")
            if ModName == '': # 原有逻辑
                ModName = valueRef.split('_')[-3]
                ModName = ModName[1:]
                ModName = ModName.upper()
    elif direction == 'Dest':
        if valueRef.split('_')[1] == 'MasterReq':
            ModName = 'LINTP'
        elif valueRef.split('_')[1] == 'SlaveResp':
            ModName = 'LINTP'
        else:
            valueRef = valueRef.split('/')[-1]
            relStringList = valueRef.split('_o') # 需要考虑模块名内含有'_'
            if len(relStringList) != 0: # 按照'_o'搜索，非原有逻辑
                relString = relStringList[-1]
                if relString.find("PduR") != -1:
                    ModName = relString
                    for item in OtherModule:
                        ModName = ModName.replace(item, "")
            if ModName == '': # 原有逻辑
                ModName = valueRef.split('_')[-2]
                ModName = ModName
                ModName = ModName.upper()
    return ModName
