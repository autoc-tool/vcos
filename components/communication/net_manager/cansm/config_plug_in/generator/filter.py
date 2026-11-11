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
from func_module.health_monitor.logger import logger


def get_template_input_parameter_dict(config_dict: dict, config_dict_rte: dict) -> dict:
    cansm_general_dict = {}
    cansm_network_list = []
    cansm_mainfunction_list = []

    flag = False
    if "cansm" in config_dict:
        if "ArPackages" in config_dict["cansm"]:
            if "ActiveEcuC" in config_dict["cansm"]["ArPackages"]:
                if "CanSM" in config_dict["cansm"]["ArPackages"]["ActiveEcuC"]:
                    flag = True
    if flag == False:
        logger.warning("CanSM Config is not exist. VCOS Tools don't generate C code")
        return cansm_general_dict

    cansm_config_dict = config_dict["cansm"]["ArPackages"]["ActiveEcuC"]["CanSM"]
    cansm_trcv_support = "STD_OFF"
    cansm_pnc_support = "STD_OFF"
    for key1, value1 in cansm_config_dict.items():
        if key1 == "DefinitionRef":
            continue
        if get_name(value1["DefinitionRef"]) == "CanSMGeneral":
            cansm_general_dict["CanSMDevErrorDetect"] = change_boolean(value1["CanSMDevErrorDetect"]["Value"])
            cansm_general_dict["BusOffBeginSupport"] = "STD_OFF" if value1["BusOffBeginFunction"]["Value"] == "" or value1["BusOffBeginFunction"]["Value"] == None else "STD_ON"
            cansm_general_dict["BusOffEndSupport"] = "STD_OFF" if value1["BusOffEndFunction"]["Value"] == "" or value1["BusOffEndFunction"]["Value"] == None else "STD_ON"
            cansm_general_dict["BusOffBeginFunction"] = value1["BusOffBeginFunction"]["Value"]
            cansm_general_dict["BusOffEndFunction"] = value1["BusOffEndFunction"]["Value"]
            cansm_general_dict["SpinlockRetryCounter"] = "255" if value1["CanSMSpinlockRetryCounter"]["Value"] == "" or value1["CanSMSpinlockRetryCounter"]["Value"] == None else value1["CanSMSpinlockRetryCounter"]["Value"]
            cansm_general_dict["CanSMInitFastExecuteThreshold"] = "3" if value1["CanSMInitFastExecuteThreshold"]["Value"] == "" or value1["CanSMInitFastExecuteThreshold"]["Value"] == None else value1["CanSMInitFastExecuteThreshold"]["Value"]
            cansm_general_dict["CanSMMainFuncFastExecuteThreshold"] = "3" if value1["CanSMMainFuncFastExecuteThreshold"]["Value"] == "" or value1["CanSMMainFuncFastExecuteThreshold"]["Value"] == None else value1["CanSMMainFuncFastExecuteThreshold"]["Value"]
        if get_name(value1["DefinitionRef"]) == "CanSMConfiguration":
            cansm_general_dict["CanSMModeRequestRepetitionTime"] = int(value1["CanSMModeRequestRepetitionTime"]["Value"] * 1000)
            cansm_general_dict["CanSMModeRequestRepetitionMax"] = value1["CanSMModeRequestRepetitionMax"]["Value"]
            for key2, value2 in value1.items():
                if key2 == "DefinitionRef" or key2 == "CanSMModeRequestRepetitionTime" or key2 == "CanSMModeRequestRepetitionMax":
                    continue
                if get_name(value2["DefinitionRef"]) == 'CanSMManagerNetwork':
                    network = get_cansm_network(value2, config_dict)
                    if network["CanTrcvPnSupport"] == "TRUE":
                        cansm_pnc_support = "STD_ON"
                    if network["CanTrcvAvailable"] == "TRUE":
                        cansm_trcv_support = "STD_ON"
                    cansm_network_list.append(network)
                if get_name(value2["DefinitionRef"]) == 'CanSMMainFunction':
                    mainfunc_info = {
                        'name': key2,
                        'period': value2['CanSMMainFunctionPeriod']['Value']
                    }
                    cansm_mainfunction_list.append(mainfunc_info)
    cansm_general_dict["CanSMPncSupport"] = cansm_pnc_support
    cansm_general_dict["CanSMTrcvSupport"] = cansm_trcv_support
    cansm_network_list_by_coreid, max_core_id = rearrange_network_list_by_coreid(cansm_network_list)
    cansm_mainfunction_list, support_multicore = set_main_function_info(cansm_mainfunction_list, config_dict_rte)
    cansm_general_dict["CanSMSupportMultiCore"] = support_multicore
    cansm_network_list_by_coreid = add_main_func_period_and_compensation_value(cansm_mainfunction_list, cansm_network_list_by_coreid)
    input_dict = {"cansm_general": cansm_general_dict, "cansm_networks": cansm_network_list,
                  "cansm_network_coreid": cansm_network_list_by_coreid, "cansm_network_max_coreid": max_core_id,
                   'cansm_mainfunc': cansm_mainfunction_list}
    return input_dict

def set_main_function_info(cansm_mainfunction_list:list, config_dict_rte:dict):
    cansm_mainfunction_list_output = []
    mapped_mainfunc_num = 0
    is_multicore = 'STD_OFF'
    if len(cansm_mainfunction_list) == 0:
        return cansm_mainfunction_list_output, is_multicore # 直接退出，保留空
    for mainfunc in cansm_mainfunction_list:
        mainfunc_mapped = False
        mainfunc_dict = mainfunc
        if "task_mappings_info" not in config_dict_rte.keys():
            mainfunc_dict['coreid'] = 0
            cansm_mainfunction_list_output.append(mainfunc_dict)
            continue
        
        for task_name in list(config_dict_rte["task_mappings_info"].keys()):
            if "schedulable_entity_info_list" not in config_dict_rte["task_mappings_info"][task_name].keys():
                continue
            if isinstance(config_dict_rte["task_mappings_info"][task_name]["schedulable_entity_info_list"], list) == False:
                continue
            for i in range(len(config_dict_rte["task_mappings_info"][task_name]["schedulable_entity_info_list"])):
                if config_dict_rte["task_mappings_info"][task_name]["schedulable_entity_info_list"][i]["name"] == mainfunc['name']:
                    MainfuncCoreId = int(config_dict_rte["task_mappings_info"][task_name]["core_id"])
                    mainfunc_dict = mainfunc
                    mainfunc_dict['coreid'] = MainfuncCoreId
                    cansm_mainfunction_list_output.append(mainfunc_dict)
                    mainfunc_mapped = True # 本MainFunc在RTE的配置面板中已经关联Runnable
                    mapped_mainfunc_num = mapped_mainfunc_num + 1
        if mainfunc_mapped == False:
            mainfunc_dict['coreid'] = 0 # 默认为Core0
            cansm_mainfunction_list_output.append(mainfunc_dict)
    if mapped_mainfunc_num > 1:
        is_multicore = 'STD_ON'
    return cansm_mainfunction_list_output, is_multicore

def add_main_func_period_and_compensation_value(cansm_mainfunction_list:list, cansm_network_list_by_coreid:list):
    for mainfunc in cansm_mainfunction_list:
        for coreid, networksInCore in cansm_network_list_by_coreid.items():
            if mainfunc['coreid'] == coreid:
                for network in networksInCore:
                    network['CanSMBusoffCompensationValue'] = int(10 // (mainfunc['period'] * 1000))
                    network['CanSMNetworkCoreMainFuncPeriod'] = int(mainfunc['period'] * 1000)
    return cansm_network_list_by_coreid

def rearrange_network_list_by_coreid(cansm_network_list:list):
    cansm_network_list_by_coreid = {}
    max_core_id = 0
    for network in cansm_network_list:
        if network['CanCtrlCoreId'] > max_core_id:
            max_core_id = network['CanCtrlCoreId']
        if network['CanCtrlCoreId'] not in cansm_network_list_by_coreid.keys():
            network_in_specific_core = []
            cansm_network_list_by_coreid[network['CanCtrlCoreId']] = network_in_specific_core
        cansm_network_list_by_coreid[network['CanCtrlCoreId']].append(network)
    for coreid in range(max_core_id):
        if coreid not in cansm_network_list_by_coreid.keys():
            cansm_network_list_by_coreid[coreid] = []
    cansm_network_list_by_coreid_sorted = {key: cansm_network_list_by_coreid[key] for key in sorted(cansm_network_list_by_coreid)}
    return cansm_network_list_by_coreid_sorted, max_core_id

def get_cansm_network(network_input: dict, config_dict: dict) -> dict:
    canif_trcvs_dict = get_canif_trcv_list(config_dict)
    canif_ctrl_dict = get_canif_ctrl_list(config_dict)
    ecuc_hardware_dict = get_ecuc_hardware_list(config_dict)
    network = {
        "CanSMBorTimeL1": int(network_input["CanSMBorTimeL1"]["Value"] * 1000),
        "CanSMBorTimeL2": int(network_input["CanSMBorTimeL2"]["Value"] * 1000),
        "CanSMBorTimeTxEnsured": int(network_input["CanSMBorTimeTxEnsured"]["Value"] * 1000),
        "CanSMBorCounterL1ToL2": network_input["CanSMBorCounterL1ToL2"]["Value"],
        "CanSMComMNetworkHandleRef": "INVALID_NETWORK_ID",
        "CanSMControllerId": "INVALID_CONTROLLER_ID",
        "CanSMDemBusOffEventId": "DEM_EVENT_ID_NULL",
        "CanTrcvAvailable": "FALSE",
        "CanTrcvPnSupport": "FALSE",
        "CanSMTransceiverId": "INVALID_TRANSCEIVER_ID",
        "CanCtrlCoreId": 0,
        "CanSMBusoffCompensationValue": 1,
        "CanSMNetworkCoreMainFuncPeriod": 5
    }

    if network_input["CanSMComMNetworkHandleRef"]["ValueRef"] is not None and network_input["CanSMComMNetworkHandleRef"]["ValueRef"] != '':
        network["CanSMComMNetworkHandleRef"] = "ComMConf_ComMChannel_" + get_name(network_input["CanSMComMNetworkHandleRef"]["ValueRef"])

    if len(canif_trcvs_dict) > 0 and network_input["CanSMTransceiverId"]["ValueRef"] is not None and network_input["CanSMTransceiverId"]["ValueRef"] != '':
        network["CanTrcvAvailable"] = "TRUE"
        CanSMTransceiverIdRef = network_input["CanSMTransceiverId"]["ValueRef"]
        network["CanSMTransceiverId"] = "CanIfConf_CanIfTrcvCfg_" + get_name(CanSMTransceiverIdRef)
        network["CanTrcvPnSupport"] = get_cantrcvref_pnsupport(CanSMTransceiverIdRef, config_dict)
    for key, value in network_input.items():
        if key == "DefinitionRef":
            continue
        if get_name(value["DefinitionRef"]) == "CanSMController":
            if value["CanSMControllerId"]["ValueRef"] is not None and value["CanSMControllerId"]["ValueRef"] != '':
                CtrlName = get_name(value["CanSMControllerId"]["ValueRef"])
                network["CanSMControllerId"] = "CanIfConf_CanIfCtrlCfg_" + CtrlName
                CoreRef = canif_ctrl_dict[CtrlName]['CanIfCtrlCoreRef']['ValueRef']
                if CoreRef is None or CoreRef == '':
                    network['CanCtrlCoreId'] = 0
                else:
                    CoreName = CoreRef.split('/')[-1]
                    network['CanCtrlCoreId'] = ecuc_hardware_dict[CoreName]['EcucCoreId']['Value']
        if get_name(value["DefinitionRef"]) == "CanSMDemEventParameterRefs":
            if value["CANSM_E_BUS_OFF"]["ValueRef"] is not None and value["CANSM_E_BUS_OFF"]["ValueRef"] != '':
                network["CanSMDemBusOffEventId"] = get_name(value["CANSM_E_BUS_OFF"]["ValueRef"])
    return network

# cantrcv_ref: canif对trcv的引用
# cantrcv_dict: 1145或1043或1042等模块配置
# cantrcvChnlName： 目标channel名字
def get_cantrcvref_pnsupport(CanSMTransceiverId:str, config_dict: dict)->str:
    pn_support = False
    if CanSMTransceiverId == '' or CanSMTransceiverId is None:
        return pn_support 
    cansm_cantrcvref = CanSMTransceiverId.split('/')[-1]  
    canif_dict = config_dict["canif"]["ArPackages"]["ActiveEcuC"]["CanIf"]
    for key1, value1 in canif_dict.items():
        if key1 == "DefinitionRef":
            continue
        if value1["DefinitionRef"] == "/AUTOSAR/EcucDefs/CanIf/CanIfTrcvDrvCfg":
            for key2, value2 in value1.items():
                if key2 != cansm_cantrcvref:
                    continue
                cantrcv_ref_by_canif = value2["CanIfTrcvCanTrcv"]["Value"]
                cantrcv_ref_type = value2["CanTrcvType"]
                cantrcv_ref_chnlName = cantrcv_ref_by_canif
                if cantrcv_ref_by_canif and cantrcv_ref_type  == "CanTrcv1145":
                    cantrcv1145_cfg_dict = {}
                    if "cantrcv1145" in config_dict.keys():
                        cantrcv1145_cfg_dict = config_dict["cantrcv1145"]["ArPackages"]["ActiveEcuC"]["CanTrcv1145"]
                    if len(cantrcv1145_cfg_dict) != 0: 
                        pn_support = get_cantrcv1145_pnsupport(cantrcv1145_cfg_dict , cantrcv_ref_chnlName)
                    else:
                        logger.ui_error(f"Cannot Get Info of CanTrcv1145")
            break
    if pn_support is True:
        return "TRUE"
    else:
        return "FALSE"
# cantrcv1145_dict： 1145配置
# cantrcvChnlName：cansm要获取的目标channelname
# ret: pnsupport value
def get_cantrcv1145_pnsupport(cantrcv1145_dict: dict, cantrcvChnlName: str):
    hwPnSupport = False
    pnEnabled = False
    for key1, value1 in cantrcv1145_dict.items():
            if key1 == "DefinitionRef":
                continue
            if value1["DefinitionRef"] != '/AUTOSAR/EcucDefs/CanTrcv1145/CanTrcvConfigSet':
                continue
            for key2, value2 in value1.items():
                if key2 == "DefinitionRef":
                    continue
                if key2 == "CanTrcvSPICommRetries" or key2 == "CanTrcvSPICommTimeout":
                    continue
                if key2 != cantrcvChnlName:
                    continue
                hwPnSupport = (value2["CanTrcvHwPnSupport"]["Value"] == True)
                for key3, value3 in value2.items():
                    if key3 == "DefinitionRef":
                        continue
                    if value3["DefinitionRef"] == "/AUTOSAR/EcucDefs/CanTrcv1145/CanTrcvConfigSet/CanTrcvChannel/CanTrcvPartialNetwork":
                        pnEnabled = (value3["CanTrcvPnEnabled"]["Value"] == True)
                if hwPnSupport and pnEnabled:
                    return True
    return False

def get_canif_trcv_list(config_dict: dict):
    canif_trcvs_dict = {}
    if "canif" in config_dict:
        if "ArPackages" in config_dict["canif"]:
            if "ActiveEcuC" in config_dict["canif"]["ArPackages"]:
                if "CanIf" in config_dict["canif"]["ArPackages"]["ActiveEcuC"]:
                    canif_dict = config_dict["canif"]["ArPackages"]["ActiveEcuC"]["CanIf"]
                    canif_trcvs_dict = get_dict_by_definition_ref(canif_dict, "CanIfTrcvCfg")
    return canif_trcvs_dict

def get_canif_ctrl_list(config_dict: dict):
    canif_trcvs_dict = {}
    if "canif" in config_dict:
        if "ArPackages" in config_dict["canif"]:
            if "ActiveEcuC" in config_dict["canif"]["ArPackages"]:
                if "CanIf" in config_dict["canif"]["ArPackages"]["ActiveEcuC"]:
                    canif_dict = config_dict["canif"]["ArPackages"]["ActiveEcuC"]["CanIf"]
                    canif_trcvs_dict = get_dict_by_definition_ref(canif_dict, "CanIfCtrlCfg")
    return canif_trcvs_dict

def get_ecuc_hardware_list(config_dict: dict):
    ecuc_hardware_dict = {}
    if "ecuc" in config_dict:
        if "ArPackages" in config_dict["ecuc"]:
            if "ActiveEcuC" in config_dict["ecuc"]["ArPackages"]:
                if "EcuC" in config_dict["ecuc"]["ArPackages"]["ActiveEcuC"]:
                    ecuc_dict = config_dict["ecuc"]["ArPackages"]["ActiveEcuC"]["EcuC"]
                    ecuc_hardware_dict = get_dict_by_definition_ref(ecuc_dict, "EcucCoreDefinition")
    return ecuc_hardware_dict

def get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"].split("/")[-1] == definition_ref:
                            out_dict.update({k1: v1})
    return out_dict


def change_boolean(b: bool) -> str:
    if b == True:
        return "STD_ON"
    else:
        return "STD_OFF"


def get_name(key_name: str) -> str:
    key_name = key_name.split("/")[-1]
    return key_name
