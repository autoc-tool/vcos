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
CanTp配置代码生成模块.
"""
from basic_func_module.autosar_utils.autosar_util import AutosarUtil


def get_template_dynamic_code_dict(config_dict: dict, config_dict_rte: dict) -> dict:
    cantp_config_dict = {}
    cantp_dict = config_dict["cantp"]["ArPackages"]["ActiveEcuC"]["CanTp"]
    cantp_mainfunction_list, cantp_channel_dict = process_cantp_configuration(cantp_dict, config_dict)
    cantp_channel_list_by_core, max_core_id = rearrange_network_list_by_coreid(cantp_channel_dict)
    cantp_mainfunction_list, cantp_core_app_map = set_main_function_info(cantp_mainfunction_list, config_dict_rte)
    CanTpGeneral = cantp_tools_get_dict_by_definition_ref(cantp_dict, "CanTpGeneral")
    CanTpConfig = cantp_tools_get_dict_by_definition_ref(cantp_dict, "CanTpConfig")
    CanTpChannels = cantp_tools_get_dict_rows_by_definition_ref(CanTpConfig, "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel")
    get_cantp_channel_nsdu(config_dict, CanTpChannels)
    get_cantp_channel_nsdu_by_core(config_dict, cantp_channel_list_by_core)
    cantp_config_dict = {"CanTpGeneral": CanTpGeneral, "CanTpConfig": CanTpConfig, "CanTpChannels": CanTpChannels,
                         "CanTpChannelByCoreId": cantp_channel_list_by_core, "CanTpMaxCore": max_core_id,
                         "CanTpMainFunc": cantp_mainfunction_list, "CanTpCoreAppMap": cantp_core_app_map,
                         }
    return cantp_config_dict

def process_cantp_configuration(cantp_dict: dict, config_dict: dict):
    cantp_mainfunction_list = []
    cantp_channel_dict = {}
    for key1, value1 in cantp_dict.items():
        if key1 == "DefinitionRef":
            continue
        if get_name(value1["DefinitionRef"]) == "CanTpConfig":
            for key2, value2 in value1.items():
                if key2 == "DefinitionRef":
                    continue
                # 获取CanTpMainFunction配置
                if get_name(value2["DefinitionRef"]) == "CanTpMainFunction":
                    mainfunc_info = {
                        'name': key2,
                        'period': value2['CanTpMainFunctionPeriod']['Value']
                    }
                    cantp_mainfunction_list.append(mainfunc_info)
                # 获取CanTpChannel配置
                if get_name(value2["DefinitionRef"]) == "CanTpChannel":
                    channel_cfg = get_cantp_channel(value2, config_dict) # 返回通道配置数据和Core信息
                    cantp_channel_dict[key2] = channel_cfg
                    # channel 到 core 的映射 -> 最终提取出来core下面的channel
    return cantp_mainfunction_list, cantp_channel_dict

def get_cantp_channel_nsdu_by_core(config_dict: dict, cantp_channel_list_by_core: list):
    for _, core_id in cantp_channel_list_by_core.items():
        for _, CanTpChannel in core_id.items():
            if isinstance(CanTpChannel, dict):
                CanTpChannel["Channel_Cfg"]["CanTpRxNSdus"] = cantp_tools_get_dict_rows_by_definition_ref(CanTpChannel["Channel_Cfg"], "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpRxNSdu")
                CanTpChannel["Channel_Cfg"]["CanTpTxNSdus"] = cantp_tools_get_dict_rows_by_definition_ref(CanTpChannel["Channel_Cfg"], "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpTxNSdu")
                for _, CanTpRxNSdu in CanTpChannel["Channel_Cfg"]["CanTpRxNSdus"].items():
                    CanTpRxNSdu["CanTpNPduLen"] = get_cantp_npdu_len(config_dict, CanTpRxNSdu["CanTpRxNPdu"]["CanTpRxNPduRef"]["ValueRef"])
                for _, CanTpTxNSdu in CanTpChannel["Channel_Cfg"]["CanTpTxNSdus"].items():
                    CanTpTxNSdu["CanTpNPduLen"] = get_cantp_npdu_len(config_dict, CanTpTxNSdu["CanTpTxNPdu"]["CanTpTxNPduRef"]["ValueRef"])
    return

def set_main_function_info(cantp_mainfunction_list:list, config_dict_rte:dict):
    cantp_default_coreid = 0
    cantp_default_app = "SYSTEMAPPLICATION_CORE0"
    cantp_mainfunc_by_coreid = {}
    cantp_mainfunction_list_output = []
    if len(cantp_mainfunction_list) == 0:
        return cantp_mainfunction_list_output # 直接退出，保留空
    for mainfunc in cantp_mainfunction_list:
        mainfunc_mapped = False
        mainfunc_dict = mainfunc
        if "task_mappings_info" not in config_dict_rte.keys():
            mainfunc_dict['coreid'] = cantp_default_coreid
            mainfunc_dict['app'] = cantp_default_app
            cantp_mainfunction_list_output.append(mainfunc_dict)
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
                    mainfunc_dict['app'] = config_dict_rte["task_mappings_info"][task_name]["application"]
                    cantp_mainfunction_list_output.append(mainfunc_dict)
                    mainfunc_mapped = True # 本MainFunc在RTE的配置面板中已经关联Runnable
        if mainfunc_mapped == False:
            mainfunc_dict['coreid'] = cantp_default_coreid # 默认为Core0
            mainfunc_dict['app'] = cantp_default_app
            cantp_mainfunction_list_output.append(mainfunc_dict)
    for mainfunc_by_core in cantp_mainfunction_list_output:
        if mainfunc_by_core['coreid'] not in cantp_mainfunc_by_coreid.keys():
            cantp_mainfunc_by_coreid[mainfunc_by_core['coreid']] = []
        cantp_mainfunc_by_coreid[mainfunc_by_core['coreid']].append(mainfunc_by_core)
    cantp_mainfunction_list_output_dict = {key: cantp_mainfunc_by_coreid[key] for key in sorted(cantp_mainfunc_by_coreid)}
    return cantp_mainfunction_list_output_dict, cantp_mainfunction_list_output

def rearrange_network_list_by_coreid(cantp_channel_dict: dict) -> dict:
    cantp_channel_list_by_coreid = {}
    max_core_id = 0
    for channelname, channelvalue in cantp_channel_dict.items():
        if channelvalue['CanCtrlCoreId'] > max_core_id:
            max_core_id = channelvalue['CanCtrlCoreId']
        if channelvalue['CanCtrlCoreId'] not in cantp_channel_list_by_coreid.keys():
            network_in_specific_core = {}
            cantp_channel_list_by_coreid[channelvalue['CanCtrlCoreId']] = network_in_specific_core
        cantp_channel_list_by_coreid[channelvalue['CanCtrlCoreId']][channelname] = channelvalue
    # cantp_channel_list_by_coreid_sorted 应该包含通道数量

    cantp_channel_list_by_coreid_sorted = {key: cantp_channel_list_by_coreid[key] for key in sorted(cantp_channel_list_by_coreid)}
    for key, value in cantp_channel_list_by_coreid_sorted.items():
        cantp_channel_list_by_coreid_sorted[key]["ChannelLen"] = len(value)
        cantp_channel_list_by_coreid_sorted[key]["CanCoreId"] = key
    return cantp_channel_list_by_coreid_sorted, max_core_id

def get_cantp_channel_nsdu(config_dict: dict, CanTpChannels: dict):
    for _, CanTpChannel in CanTpChannels.items():
        CanTpChannel["CanTpRxNSdus"] = cantp_tools_get_dict_rows_by_definition_ref(CanTpChannel, "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpRxNSdu")
        CanTpChannel["CanTpTxNSdus"] = cantp_tools_get_dict_rows_by_definition_ref(CanTpChannel, "/AUTOSAR/EcucDefs/CanTp/CanTpConfig/CanTpChannel/CanTpTxNSdu")
        for _, CanTpRxNSdu in CanTpChannel["CanTpRxNSdus"].items():
            CanTpRxNSdu["CanTpNPduLen"] = get_cantp_npdu_len(config_dict, CanTpRxNSdu["CanTpRxNPdu"]["CanTpRxNPduRef"]["ValueRef"])
        for _, CanTpTxNSdu in CanTpChannel["CanTpTxNSdus"].items():
            CanTpTxNSdu["CanTpNPduLen"] = get_cantp_npdu_len(config_dict, CanTpTxNSdu["CanTpTxNPdu"]["CanTpTxNPduRef"]["ValueRef"])
    return


def get_cantp_npdu_len(config_dict: dict, path: str) -> int:
    npdu_len = 8
    sdu = AutosarUtil.get_by_path_from_dict(config_dict["ecuc"]["ArPackages"], path)
    if sdu is not None:
        npdu_len = sdu["PduLength"]["Value"]
    return npdu_len

def cantp_tools_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"].split("/")[-1] == definition_ref:
                    return v
    return {}

# 根据"DefinitionRef"字段获取input_dict下的一级子字典
def cantp_tools_get_dict_rows_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            if "DefinitionRef" in v.keys():
                if v["DefinitionRef"] == definition_ref:
                    out_dict.update({k: v})
    return out_dict

def get_name(key_name: str) -> str:
    key_name =  key_name.split("/")[-1]
    return key_name

def get_cantp_channel(channel_input: dict, config_dict: dict) -> dict:
    ecuc_hardware_dict = get_ecuc_hardware_list(config_dict)
    ecuc_pdu_dict = get_ecuc_pdu_dict(config_dict)
    channel = {"Channel_Cfg": channel_input, "CanCtrlCoreId": 0}

    for key, value in channel_input.items():
        if key == "DefinitionRef":
            continue
        if get_name(value["DefinitionRef"]) == "CanTpRxNSdu":
            if value["CanTpRxNSduRef"]["ValueRef"] is not None and value["CanTpRxNSduRef"]["ValueRef"] != '':
                PduName = get_name(value["CanTpRxNSduRef"]["ValueRef"])
                CoreRef = ecuc_pdu_dict[PduName]["EcucPduDefaultCoreRef"]["ValueRef"]
                if CoreRef is None or CoreRef == '':
                    channel["CanCtrlCoreId"] = 0
                else:
                    CoreName = CoreRef.split("/")[-1]
                    channel["CanCtrlCoreId"] = ecuc_hardware_dict[CoreName]['EcucCoreId']['Value']
        elif get_name(value["DefinitionRef"]) == "CanTpTxNSdu":
            if value["CanTpTxNSduRef"]["ValueRef"] is not None and value["CanTpTxNSduRef"]["ValueRef"] != '':
                PduName = get_name(value["CanTpTxNSduRef"]["ValueRef"])
                CoreRef = ecuc_pdu_dict[PduName]["EcucPduDefaultCoreRef"]["ValueRef"]
                if CoreRef is None or CoreRef == '':
                    channel["CanCtrlCoreId"] = 0
                else:
                    CoreName = CoreRef.split("/")[-1]
                    channel["CanCtrlCoreId"] = ecuc_hardware_dict[CoreName]['EcucCoreId']['Value']
    return channel



def get_ecuc_pdu_dict(config_dict: dict) -> dict:
    eucu_pdu_dict = {}
    if "ecuc" in config_dict:
        if "ArPackages" in config_dict["ecuc"]:
            if "ActiveEcuC" in config_dict["ecuc"]["ArPackages"]:
                if "EcuC" in config_dict["ecuc"]["ArPackages"]["ActiveEcuC"]:
                    ecuc_dict = config_dict["ecuc"]["ArPackages"]["ActiveEcuC"]["EcuC"]["EcucConfigSet"]["EcucPduCollection"]
                    eucu_pdu_dict = get_dict_by_definition_ref(ecuc_dict, "EcucPduDefaultCoreRef")
    return eucu_pdu_dict

def get_ecuc_hardware_list(config_dict: dict):
    ecuc_hardware_dict = {}
    if "ecuc" in config_dict:
        if "ArPackages" in config_dict["ecuc"]:
            if "ActiveEcuC" in config_dict["ecuc"]["ArPackages"]:
                if "EcuC" in config_dict["ecuc"]["ArPackages"]["ActiveEcuC"]:
                    ecuc_dict = config_dict["ecuc"]["ArPackages"]["ActiveEcuC"]["EcuC"]
                    ecuc_hardware_dict = get_dict_by_definition_def(ecuc_dict, "EcucCoreDefinition")
    return ecuc_hardware_dict

def get_dict_by_definition_def(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"].split("/")[-1] == definition_ref:
                            out_dict.update({k1: v1})
    return out_dict

def get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    out_dict = {}
    for k, v in input_dict.items():
        if isinstance(v, dict):
            for k1, v1 in v.items():
                if isinstance(v1, dict):
                    if "DefinitionRef" in v1.keys():
                        if v1["DefinitionRef"].split("/")[-1] == definition_ref:
                            out_dict.update({k: v})
    return out_dict