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
import sys
import os
from func_module.health_monitor.logger import logger

G_CANNM_RXPDU_UL_RXINDICATION_INDEX = 1
g_canif_multiocre_channel_core_dict = {}

def get_can_hoh_order_and_range_info(CanConfigSet, sys_config):
    can_hoh_order_type = "CANIF_CAN_HOH_ORDERED_IN_CAN_HRH_FIRST"
    if sys_config["target"] == "E3650":
        can_hoh_order_type = "CANIF_CAN_HOH_ORDERED_IN_MIXED"

    return can_hoh_order_type
    
def get_g_canif_multiocre_channel_core_dict():
    global g_canif_multiocre_channel_core_dict
    return g_canif_multiocre_channel_core_dict

def set_g_canif_multiocre_channel_core_dict(input_dict):
    global g_canif_multiocre_channel_core_dict
    
    for ctrl_name, ctrl_attr in input_dict.items():
        g_canif_multiocre_channel_core_dict[ctrl_name] = ctrl_attr

def get_core_cnt_multicore_enbale():
    global g_canif_multiocre_channel_core_dict
    core_id_list = []
    for _, ctrl_attr in g_canif_multiocre_channel_core_dict.items():
        core_id = ctrl_attr["core_id"]
        core_id_list.append(core_id)
        
    core_id_list = list(set(core_id_list))
    if len(core_id_list) == 0:
        core_id_list = [0]

    core_id_name_list = []
    for core_id in core_id_list:
        core_id_name_list.append("CORE" + str(core_id))
    return core_id_name_list, core_id_list, len(core_id_list)
    
def checkIsNotNone(input):
    return (input is not None) and (input != "") and (input != "<empty>") and (input != "None")

def get_mastercore_from_resourcem(resourcem_cfg_dict):
    master_core_id = 0
    if resourcem_cfg_dict != {} and resourcem_cfg_dict != None:
        for key1, value1 in resourcem_cfg_dict.items():
            if key1 == "DefinitionRef":
                continue
            
            if value1["DefinitionRef"].endswith("/ResourceM/ResourceMMcalConfig"): 
                for key2, value2 in value1.items():
                    if key2 == "DefinitionRef":
                        continue
                    
                    if value2["DefinitionRef"].endswith("ResourceM/ResourceMMcalConfig/ResourceMMasterCore"): 
                        master_core_id = int(value2["Value"][-1])
                        break
                break
    return master_core_id


def get_all_cdd_dict(config_dict_bsw: dict):
    cdds_dict = {}
    for _, _bsw_attr in config_dict_bsw.items():
        found = False
        _bsw_dict = _bsw_attr.get("ArPackages", {}).get("ActiveEcuC", {})
        for bsw_name, bsw_attr in _bsw_dict.items():
            for key, value in bsw_attr.items(): 
                if key == "DefinitionRef" and value == "/AUTOSAR/EcucDefs/Cdd":
                    found = True
                    break
            if found:  
                cdds_dict[bsw_name] = bsw_attr
                break
            
    return cdds_dict

def get_cddcanif_from_allcdd(all_cdd_dict: dict):
    cddcanif_dict = {}
    for _cdd_name, _cdd_attr in all_cdd_dict.items():
        for key1, value1 in _cdd_attr.items():
            if key1 == "DefinitionRef":
                continue
            if value1["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution":
                for key2, value2 in value1.items():
                    if key2 == "DefinitionRef":
                        continue
                    if value2["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution/CddComIfUpperLayerContribution":
                        cddcanif_dict[_cdd_name] = _cdd_attr
    return cddcanif_dict

def get_cdd_pdu_id(cddcanif_dict: dict, ecuc_dict: dict):
    cdd_txpdu_id_dict = {}
    cdd_rxpdu_id_dict = {}
    for _cdd_name, _cdd_attr in cddcanif_dict.items():
        for key1, value1 in _cdd_attr.items():
            if key1 == "DefinitionRef":
                continue
            
            if value1["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution":
                for key2, value2 in value1.items():
                    if key2 == "DefinitionRef":
                        continue
                    if value2["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution/CddComIfUpperLayerContribution":
                        for key3, value3 in value2.items():
                            if key3 == "DefinitionRef":
                                continue
                                
                            if value3["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution/CddComIfUpperLayerContribution/CddComIfUpperLayerTxPdu":
                                for key4, value4 in value3.items():
                                    if key4 == "DefinitionRef":
                                        continue
                                    if value4["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution/CddComIfUpperLayerContribution/CddComIfUpperLayerTxPdu/CddComIfPduRef":
                                        ecuc_pdu_ref_name = value4["ValueRef"].split("/")[-1]
                                        cdd_txpdu_id_dict[ecuc_pdu_ref_name] = {"cdd_module_name": _cdd_name, "cdd_txpdu_shortname": key3}
                                        
                            if value3["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution/CddComIfUpperLayerContribution/CddComIfUpperLayerRxPdu":
                                for key4, value4 in value3.items():
                                    if key4 == "DefinitionRef":
                                        continue
                                    if value4["DefinitionRef"] == "/AUTOSAR/EcucDefs/Cdd/CddComStackContribution/CddComIfUpperLayerContribution/CddComIfUpperLayerRxPdu/CddComIfPduRef":
                                        ecuc_pdu_ref_name = value4["ValueRef"].split("/")[-1]
                                        cdd_rxpdu_id_dict[ecuc_pdu_ref_name] = {"cdd_module_name": _cdd_name, "cdd_rxpdu_shortname": key3}
                        
    return cdd_txpdu_id_dict, cdd_rxpdu_id_dict

def get_canif_os_application_core_map(Config_dict):
    app_coreid_dict = {}

    ecuc_dict = Config_dict['bsw']['ecuc']['ArPackages']['ActiveEcuC']['EcuC']
    os_dict = Config_dict.get('bsw', {}).get('os', {}).get('ArPackages', {}).get('ActiveEcuC', {}).get('Os', {})
    if ecuc_dict != {} and os_dict != {}:
        os_core_dict = {}
        for key, value in os_dict.items():
            if key == "DefinitionRef" or key == "Tag":
                continue
            if isinstance(value, dict):
                if value["DefinitionRef"] == '/AUTOSAR/EcucDefs/Os/OsCore':
                    os_core_dict[key] = value
        for os_core, value in os_core_dict.items():
            app_name_value = value['OsCoreBswApplicationRef']['ValueRef']
            app_name_value = app_name_value[app_name_value.rfind("/") + 1:]
            ecuc_core_ref = os_dict[app_name_value]['OsApplicationCoreRef']['ValueRef']
            ecuc_core_ref = ecuc_core_ref[ecuc_core_ref.rfind("/") + 1:]
            coreId = ecuc_dict['EcucHardware'][ecuc_core_ref]['EcucCoreId']['Value']
            app_coreid_dict[coreId] = app_name_value.upper()
    else:
        app_coreid_dict[0] = ""
        app_coreid_dict[1] = ""
        app_coreid_dict[2] = ""
        app_coreid_dict[3] = ""
        app_coreid_dict[4] = ""
        app_coreid_dict[5] = ""

    return app_coreid_dict

def get_template_input_parameter_dict(config_dict_all: dict, sys_config: dict) -> dict:
    config_dict_bsw = config_dict_all["bsw"]
    
    ecuc_dict = config_dict_bsw["ecuc"]["ArPackages"]["ActiveEcuC"]["EcuC"]
    all_cdd_dict = get_all_cdd_dict(config_dict_bsw)
    cddcanif_dict = get_cddcanif_from_allcdd(all_cdd_dict)
    cdd_txpdu_id_dict, cdd_rxpdu_id_dict = get_cdd_pdu_id(cddcanif_dict, ecuc_dict)
    
    config_dict_raw = config_dict_bsw["canif"]
    can_cfg_dict = config_dict_bsw["can"]
    
    resourcem_cfg_dict = config_dict_bsw.get("resourcem", {}).get("ArPackages", {}).get("ActiveEcuC", {}).get("ResourceM", None)
    master_core_id = get_mastercore_from_resourcem(resourcem_cfg_dict)
        
    cantrcv1043_cfg_dict = {}
    cantrcv1145_cfg_dict = {}
    
    if "cantrcv1043" in config_dict_bsw.keys():
        cantrcv1043_cfg_dict = config_dict_bsw["cantrcv1043"]["ArPackages"]["ActiveEcuC"]["CanTrcv1043"]
    if "cantrcv1145" in config_dict_bsw.keys():
        cantrcv1145_cfg_dict = config_dict_bsw["cantrcv1145"]["ArPackages"]["ActiveEcuC"]["CanTrcv1145"]
    
    all_cantrcv_cfg_dict = {
        "cantrcv1043": cantrcv1043_cfg_dict,
        "cantrcv1145": cantrcv1145_cfg_dict,
    }
    
    app_coreid_dict = get_canif_os_application_core_map(config_dict_all)

    tx_pdus = []
    rx_pdus = []
    buffer_list = []
    hrh_cfglist = []
    
    is_module_tx_used = {
        "is_cannm_used": "false",
        "is_cantp_used": "false",
        "is_pdur_used": "false",
        "is_xcp_used": "false",
        "is_dlt_used": "false",
        "is_ftrace_used": "false",
        "is_cdd_used": "false",
        "is_cantsyn_used": "false", 
        "cdd_confirmation_name": "NULL",
        "cdd_indication_name": "NULL",
        "ul_moudle_count": 0
    }
    
    is_module_rx_used = {
        "is_cannm_used": "false",
        "is_cantp_used": "false",
        "is_pdur_used": "false",
        "is_xcp_used": "false",
        "is_dlt_used": "false",
        "is_ftrace_used": "false",
        "is_cdd_used": "false",
        "is_cantsyn_used": "false", 
        "cdd_confirmation_name": "NULL",
        "cdd_indication_name": "NULL",
        "ul_moudle_count": 0
    }
    
    is_module_used = {
        "is_cannm_used": "false",
        "is_cantp_used": "false",
        "is_pdur_used": "false",
        "is_xcp_used": "false",
        "is_dlt_used": "false",
        "is_ftrace_used": "false",
        "is_cdd_used": "false",
        "is_cantsyn_used": "false", 
        "cdd_confirmation_name": "NULL",
        "cdd_indication_name": "NULL",
        "ul_moudle_count": 0
    }
    
    canif_controllers_config = []
    canif_transceivers_config = []
    config_dict = config_dict_raw["ArPackages"]["ActiveEcuC"]["CanIf"]
    CanConfigSet = can_cfg_dict["ArPackages"]["ActiveEcuC"]["Can"]["CanConfigSet"]
    CanGeneral = can_cfg_dict["ArPackages"]["ActiveEcuC"]["Can"]["CanGeneral"]
    
    can_hoh_order_type = get_can_hoh_order_and_range_info(CanConfigSet, sys_config)

    canif_version = get_can_version(CanGeneral["CanVersion"]["Value"])
    canif_public_cfg = get_Publiccfgs(config_dict)
    canif_private_cfg = get_Privatecfgs(config_dict)
    canif_dispatch_cfg = get_Dispatchcfgs(config_dict)
    
    raw_tx_pducfgs = get_txpdus_cfgs(config_dict)
    raw_rx_pducfgs = get_rxpdus_cfgs(config_dict)
    raw_txbuffercfgs = get_txbuffer_cfgs(config_dict, CanConfigSet)
    raw_Ctrlcfgs = get_Ctrlcfgs(config_dict)
    raw_Trcvcfgs = get_Trcvcfgs(config_dict)
    raw_hrhcfgs = get_hrh_cfgs(config_dict, CanConfigSet)
    
    canif_ctrl_core_dict = {}
    canif_ctrl_multicore_enable = False
    canif_ctrl_single_core_id = 0
    canif_ctrl_core_dict = update_core_id(canif_ctrl_core_dict, raw_Ctrlcfgs, ecuc_dict)
    set_g_canif_multiocre_channel_core_dict(canif_ctrl_core_dict)
    multicore_enable, first_core_id = has_multiple_values()
    if multicore_enable:
        canif_ctrl_multicore_enable = True
    else:
        canif_ctrl_single_core_id = first_core_id
        canif_ctrl_multicore_enable = False
    
    all_core_id_name_list, all_core_id_list, canif_activated_core_cnt = get_core_cnt_multicore_enbale()
    
    canif_max_activated_core_id = -1
    for core_id in all_core_id_list:
        if core_id > canif_max_activated_core_id:
            canif_max_activated_core_id = core_id
    
    all_core_id_enbale_list = []
    for _ in range(canif_max_activated_core_id + 1):
        all_core_id_enbale_list.append("FALSE")

    for _core_id in all_core_id_list:
        all_core_id_enbale_list[_core_id] = "TRUE"
        
    for key in raw_txbuffercfgs.keys():
        buffer_cfg = raw_txbuffercfgs[key]
        temp_txbuffer = get_transmit_buffer_info(key, buffer_cfg, CanConfigSet)
        buffer_list.append(temp_txbuffer)
        
    max_lpdu_num_txbuf = 1
    for buffer in buffer_list:
        max_lpdu_num_txbuf += buffer["buffer_size"]
    
    canif_txbuffer_size_used_flag = "FALSE"
    for buffer in buffer_list:
        if buffer["buffer_size"] > 0:
            canif_txbuffer_size_used_flag = "TRUE"
            break
        
    canif_txbuffer_size_memmap_section = "ZERO_INIT"
    if canif_txbuffer_size_used_flag == "TRUE":
        canif_txbuffer_size_memmap_section = "INIT"
        
    multicore_txbuffer_map_dict, multicore_txbuffercfgs_dict = split_multicore_txbuffercfgs(buffer_list)
    
    len_multicore_txbuffercfgs_dict = {}
    for k, v in multicore_txbuffercfgs_dict.items():
        len_multicore_txbuffercfgs_dict[k] = len(v)
        
    all_core_txbuffercfgs_inorder_list = []
    for _, core_attr in multicore_txbuffercfgs_dict.items():
        all_core_txbuffercfgs_inorder_list = all_core_txbuffercfgs_inorder_list + core_attr
    
    all_core_txbuffercfgs_inorder_dict = {}
    for item in all_core_txbuffercfgs_inorder_list:
        all_core_txbuffercfgs_inorder_dict[item["buffer_name"]] = item
    
    multicore_txbuffercfgs_start_end_index_dict = {}
    txbuffercfgs_offset_start_index = 0
    for core_id_name in all_core_id_name_list:
        if core_id_name in multicore_txbuffercfgs_dict.keys():
            txbuffercfgs_offset_start_index_offset = len(multicore_txbuffercfgs_dict[core_id_name])
            multicore_txbuffercfgs_start_end_index_dict[core_id_name] = {"range": [txbuffercfgs_offset_start_index, txbuffercfgs_offset_start_index + txbuffercfgs_offset_start_index_offset - 1], "core_id": int(core_id_name[-1])}
            txbuffercfgs_offset_start_index = txbuffercfgs_offset_start_index + txbuffercfgs_offset_start_index_offset
        else:
            multicore_txbuffercfgs_start_end_index_dict[core_id_name] = {"range": [0xffff, 0xffff], "core_id": int(core_id_name[-1])}
            
    max_lpdu_num_cores_txbuf_dict = {}
    lpdu_cur_core_start_index = 0
    lpdu_cur_core_end_index = 0
    for core_id_name in all_core_id_name_list:
        lpdu_num_cur_core = 0
        if core_id_name in multicore_txbuffercfgs_dict.keys():
            txbuffers_list = multicore_txbuffercfgs_dict[core_id_name]
            for txbuffer in txbuffers_list:
                lpdu_num_cur_core += txbuffer["buffer_size"]
        else:
            lpdu_num_cur_core = 0
        
        lpdu_cur_core_end_index = lpdu_cur_core_start_index + lpdu_num_cur_core - 1
        if lpdu_num_cur_core > 0:
            max_lpdu_num_cores_txbuf_dict[core_id_name] = {"range": [lpdu_cur_core_start_index, lpdu_cur_core_end_index], "core_id": core_id_name[-1], "lpdu_num": lpdu_num_cur_core}
        else:
            max_lpdu_num_cores_txbuf_dict[core_id_name] = {"range": [0xffff, 0xffff], "core_id": core_id_name[-1], "lpdu_num": 0}
        lpdu_cur_core_start_index = lpdu_cur_core_end_index + 1
        
    for key in raw_tx_pducfgs.keys():
        pdu = raw_tx_pducfgs[key]
        head, tail = os.path.split(pdu["CanIfTxPduRef"]["ValueRef"])
        pdu["ShortName"] = tail
        temp_txpdu = get_txpdu_info(pdu, all_core_txbuffercfgs_inorder_dict, ecuc_dict, cdd_txpdu_id_dict)
        tx_pdus.append(temp_txpdu)
        if temp_txpdu["CanIfUserTxConfirmation_index"] == 0:
            is_module_tx_used["is_pdur_used"] = "true"
            is_module_used["is_pdur_used"] = "true"
        elif temp_txpdu["CanIfUserTxConfirmation_index"] == 1:
            is_module_tx_used["is_cannm_used"] = "true"
            is_module_used["is_cannm_used"] = "true"
        elif temp_txpdu["CanIfUserTxConfirmation_index"] == 2:
            is_module_tx_used["is_cantp_used"] = "true"
            is_module_used["is_cantp_used"] = "true"
        elif temp_txpdu["CanIfUserTxConfirmation_index"] == 3:
            is_module_tx_used["is_xcp_used"] = "true"
            is_module_used["is_xcp_used"] = "true"
        elif temp_txpdu["CanIfUserTxConfirmation_index"] == 4:
            is_module_tx_used["is_cdd_used"] = "true"
            is_module_tx_used["cdd_confirmation_name"] = pdu["CanIfTxPduUserTxConfirmationName"]["Value"]
            is_module_used["is_cdd_used"] = "true"
            is_module_used["cdd_confirmation_name"] = pdu["CanIfTxPduUserTxConfirmationName"]["Value"]
        elif temp_txpdu["CanIfUserTxConfirmation_index"] == 5:
            is_module_tx_used["is_cantsyn_used"] = "true"
            is_module_used["is_cantsyn_used"] = "true"
        elif temp_txpdu["CanIfUserTxConfirmation_index"] == 6:
            is_module_tx_used["is_dlt_used"] = "true"
            is_module_used["is_dlt_used"] = "true"
        elif temp_txpdu["CanIfUserTxConfirmation_index"] == 7:
            is_module_tx_used["is_ftrace_used"] = "true"
            is_module_used["is_ftrace_used"] = "true"            
        else:
            logger.debug("error CanIfTxPduUserTxConfirmationUL config")
    
    for key in raw_rx_pducfgs.keys():
        pdu = raw_rx_pducfgs[key]
        head, tail = os.path.split(pdu["CanIfRxPduRef"]["ValueRef"])
        pdu["ShortName"] = tail
        temp_rxpdu = get_rxpdu_info(pdu, raw_hrhcfgs, ecuc_dict, cdd_rxpdu_id_dict)
        rx_pdus.append(temp_rxpdu)
        if temp_rxpdu["CanIfUserRxIndication_index"] == 0:
            is_module_rx_used["is_pdur_used"] = "true"
            is_module_used["is_pdur_used"] = "true"
        elif temp_rxpdu["CanIfUserRxIndication_index"] == 1:
            is_module_rx_used["is_cannm_used"] = "true"
            is_module_used["is_cannm_used"] = "true"
        elif temp_rxpdu["CanIfUserRxIndication_index"] == 2:
            is_module_rx_used["is_cantp_used"] = "true"
            is_module_used["is_cantp_used"] = "true"
        elif temp_rxpdu["CanIfUserRxIndication_index"] == 3:
            is_module_rx_used["is_xcp_used"] = "true"
            is_module_used["is_xcp_used"] = "true"
        elif temp_rxpdu["CanIfUserRxIndication_index"] == 4:
            is_module_rx_used["is_cdd_used"] = "true"
            is_module_rx_used["cdd_indication_name"] = pdu["CanIfRxPduUserRxIndicationName"]["Value"]
            is_module_used["is_cdd_used"] = "true"
            is_module_used["cdd_indication_name"] = pdu["CanIfRxPduUserRxIndicationName"]["Value"]
        elif temp_rxpdu["CanIfUserRxIndication_index"] == 5:
            is_module_rx_used["is_cantsyn_used"] = "true"
            is_module_used["is_cantsyn_used"] = "true"
        elif temp_rxpdu["CanIfUserRxIndication_index"] == 6:
            is_module_rx_used["is_dlt_used"] = "true"
            is_module_used["is_dlt_used"] = "true"
        elif temp_rxpdu["CanIfUserRxIndication_index"] == 7:
            is_module_rx_used["is_ftrace_used"] = "true"
            is_module_used["is_ftrace_used"] = "true"
        else:
            logger.debug("error CanIfRxPduUserRxIndicationUL  config")
          
    rx_pdus.sort(key=lambda rx_pdu: rx_pdu["hrh_ref"])
    
    for key in is_module_tx_used.keys():
        if is_module_tx_used[key] == "true":
            is_module_tx_used["ul_moudle_count"] += 1
    
    for key in is_module_rx_used.keys():
        if is_module_rx_used[key] == "true":
            is_module_rx_used["ul_moudle_count"] += 1
    
    for key in is_module_used.keys():
        if is_module_used[key] == "true":
            is_module_used["ul_moudle_count"] += 1
    
    hrh_index = 0
    for key in raw_hrhcfgs.keys():
        hrh_cfg = raw_hrhcfgs[key]
        if "CanIfHrhIdSymRef" in hrh_cfg:
            hrh_cfg["ShortName"] = key
            temp_hrhcfg = get_hrh_cfgdata(hrh_cfg, rx_pdus, hrh_index, CanConfigSet, canif_ctrl_multicore_enable)
            hrh_cfglist.append(temp_hrhcfg)
            hrh_index += 1
    
    ctrl_wakeup_found = "FALSE"
    ctrl_index = 0
    for key in raw_Ctrlcfgs.keys():
        Ctrlcfg = raw_Ctrlcfgs[key]
        Ctrlcfg["ShortName"] = key
        canif_controller_config = get_controller_cfgdata(Ctrlcfg, multicore_txbuffercfgs_dict, CanConfigSet, ctrl_index, ecuc_dict, canif_ctrl_multicore_enable)
        canif_controllers_config.append(canif_controller_config)
        if canif_controller_config["is_wakeup"] == "TRUE":
            ctrl_wakeup_found = "TRUE"
        ctrl_index = ctrl_index + 1
    
    canif_controllers_config = sorted(canif_controllers_config, key=lambda x: x["controller_id"])
    for _index, _ in enumerate(canif_controllers_config):
        canif_controllers_config[_index]["ctrl_chnl_id"] = _index

    trcv_api_cantrcv1043 = {
        "CanIfTrcvTrcvModeIndicationFctName": "NULL",
    }
    trcv_api_cantrcv1145 = {
        "CanIfTrcvCheckTrcvWakeFlagIndicationFctName": "NULL",
        "CanIfTrcvClearTrcvWufFlagIndicationFctName": "NULL",
        "CanIfTrcvTrcvModeIndicationFctName": "NULL",
        "CanIfTrcvConfirmPnAvailabilityFctName": "NULL",
    }
    
    trcv_pn_support_found = "FALSE"
    trcv_wakeup_found = "FALSE"
    
    trcv_index = 0
    for key in raw_Trcvcfgs.keys():
        Trcvcfg = raw_Trcvcfgs[key]
        Trcvcfg["ShortName"] = key
        canif_transceiver_config = get_canif_transceiver_config(Trcvcfg, canif_controllers_config, all_cantrcv_cfg_dict, trcv_index) 
        canif_transceivers_config.append(canif_transceiver_config)
        if canif_transceiver_config["is_pnSupport"]:
            trcv_pn_support_found = "TRUE"
        if canif_transceiver_config["is_wakeup"]:
            trcv_wakeup_found = "TRUE"
            
            
        if canif_transceiver_config["module_referenced"] == "CanTrcv1043":
            trcv_api_cantrcv1043["CanIfTrcvTrcvModeIndicationFctName"] = "CanIf_Tja1043_TrcvModeIndication"
        
        if canif_transceiver_config["module_referenced"] == "CanTrcv1145":
            trcv_api_cantrcv1145["CanIfTrcvCheckTrcvWakeFlagIndicationFctName"] = "CanIf_Tja1145_CheckTrcvWakeFlagIndication"
            trcv_api_cantrcv1145["CanIfTrcvClearTrcvWufFlagIndicationFctName"] = "CanIf_Tja1145_ClearTrcvWufFlagIndication"
            trcv_api_cantrcv1145["CanIfTrcvTrcvModeIndicationFctName"] = "canif_tja1145_trcv_mode_indication"
            trcv_api_cantrcv1145["CanIfTrcvConfirmPnAvailabilityFctName"] = "CanIf_Tja1145_ConfirmPnAvailability"
        
        trcv_index = trcv_index + 1
        
    for _index, _ in enumerate(canif_transceivers_config):
        canif_transceivers_config[_index]["trcv_chnl_id"] = _index
        
    trcv_api_enable = "STD_OFF"
    trcv_type_enable = {
        "CanTrcv1043": "False",
        "CanTrcv1145": "False"
    }
    trcv_start_index = 0
    first_into_flag_cantrcv1043 = 0
    first_into_flag_cantrcv1145 = 0
    offset_index_from_start_index_cantrcv1043 = 0
    offset_index_from_start_index_cantrcv1145 = 0
    
    trcv_enable_list_tmp = []
    for trcv in canif_transceivers_config:
        if trcv["cantrcv_used"]:
            trcv_api_enable = "STD_ON"
        if trcv["module_referenced"] == "CanTrcv1043":
            if len(trcv_enable_list_tmp) == 0:
                trcv_enable_list_tmp.append("CanTrcv1043")
            trcv_type_enable["CanTrcv1043"] = "True"
            if first_into_flag_cantrcv1043 == 0:
                offset_index_from_start_index_cantrcv1043 = trcv_start_index
                first_into_flag_cantrcv1043 = first_into_flag_cantrcv1043 + 1
                
            trcv_start_index = trcv_start_index + 1
        elif trcv["module_referenced"] == "CanTrcv1145":
            if len(trcv_enable_list_tmp) == 0:
                trcv_enable_list_tmp.append("CanTrcv1145")
            trcv_type_enable["CanTrcv1145"] = "True"
            if first_into_flag_cantrcv1145 == 0:
                offset_index_from_start_index_cantrcv1145 = trcv_start_index
                first_into_flag_cantrcv1145 = first_into_flag_cantrcv1145 + 1
                
            trcv_start_index = trcv_start_index + 1
        else:
            pass
    
    trcv_enable_list = []
    if trcv_type_enable["CanTrcv1043"] == "True" and trcv_type_enable["CanTrcv1145"] == "True":
        if "CanTrcv1043" in trcv_enable_list_tmp:
            trcv_enable_list = ["CanTrcv1043", "CanTrcv1145"]
        elif "CanTrcv1145" in trcv_enable_list_tmp:
            trcv_enable_list = ["CanTrcv1145", "CanTrcv1043"]
    elif trcv_type_enable["CanTrcv1043"] == "True" and trcv_type_enable["CanTrcv1145"] == "False":
        trcv_enable_list = ["CanTrcv1043"]
    elif trcv_type_enable["CanTrcv1043"] == "False" and trcv_type_enable["CanTrcv1145"] == "True":
        trcv_enable_list = ["CanTrcv1145"]
    else:
        trcv_enable_list = []
    
    if ((len(trcv_enable_list) == 0) or (len(trcv_enable_list) == 1)):
        for _index, _ in enumerate(canif_transceivers_config):
            canif_transceivers_config[_index]["CanIfCanTrcvFctGrpId"] = 0
    elif len(trcv_enable_list) == 2:
        if trcv_enable_list == ["CanTrcv1043", "CanTrcv1145"]:
            for _index, _item_trcv in enumerate(canif_transceivers_config):
                if _item_trcv["module_referenced"] == "CanTrcv1145":
                    canif_transceivers_config[_index]["CanIfCanTrcvFctGrpId"] = 1
                if _item_trcv["module_referenced"] == "CanTrcv1043":
                    canif_transceivers_config[_index]["CanIfCanTrcvFctGrpId"] = 0
                    
        if trcv_enable_list == ["CanTrcv1145", "CanTrcv1043"]:
            for _index, _item_trcv in enumerate(canif_transceivers_config):
                if _item_trcv["module_referenced"] == "CanTrcv1145":
                    canif_transceivers_config[_index]["CanIfCanTrcvFctGrpId"] = 0
                if _item_trcv["module_referenced"] == "CanTrcv1043":
                    canif_transceivers_config[_index]["CanIfCanTrcvFctGrpId"] = 1
        
    trcv_channel_enable_type_count = 0
    if trcv_type_enable["CanTrcv1043"] == "True":
        trcv_channel_enable_type_count = trcv_channel_enable_type_count + 1
    if trcv_type_enable["CanTrcv1145"] == "True":
        trcv_channel_enable_type_count = trcv_channel_enable_type_count + 1
        
    trcvApi_for_canif_cfg_h = {
        "trcv_api_cantrcv1145": trcv_api_cantrcv1145, 
        "trcv_api_cantrcv1043": trcv_api_cantrcv1043
    }
        
    canif_init_config = {
        "canif_all_rx_pdu_num": len(rx_pdus),
        "canif_all_tx_pdu_num": len(tx_pdus),
        "canif_number_of_dynamic_can_tx_pdu_Ids": 0,
        "canif_all_tx_buf_num": len(buffer_list)
    }
   
    multicore_tx_pducfgs_map_dict, multicore_tx_pducfgs_dict = split_multicore_tx_pducfgs(tx_pdus)
    multicore_hrhcfgs_map_dict, multicore_hrhcfgs_dict = split_multicore_hrhcfgs(hrh_cfglist)
    multicore_rx_pducfgs_map_dict, multicore_rx_pducfgs_dict = split_multicore_rx_pducfgs(rx_pdus)
    
    len_multicore_rx_pducfgs_dict = {}
    for k, v in multicore_rx_pducfgs_dict.items():
        len_multicore_rx_pducfgs_dict[k] = len(v)
        
    len_multicore_tx_pducfgs_dict = {}
    for k, v in multicore_tx_pducfgs_dict.items():
        len_multicore_tx_pducfgs_dict[k] = len(v)
        
    len_multicore_hrhcfgs_dict = {}
    for k, v in multicore_hrhcfgs_dict.items():
        len_multicore_hrhcfgs_dict[k] = len(v)
    
    all_core_tx_pducfgs_list = []
    for _, core_attr in multicore_tx_pducfgs_dict.items():
        all_core_tx_pducfgs_list = all_core_tx_pducfgs_list + core_attr
    
    canif_manager_var_multi_app_or_private_master_core = "SYSTEMAPPLICATION_CORE" + str(master_core_id) + "_PRIVATE"
    if canif_ctrl_multicore_enable:
        canif_manager_var_multi_app_or_private_master_core = "MULTI_APP"

    cantrcv_enable_list = []
    cantrcv_enable_list_flag = "FALSE"
    for trcv_item in canif_transceivers_config:
        cantrcv_enable_list.append(trcv_item["cantrcv_header_file"])
    cantrcv_enable_list = list(set(cantrcv_enable_list))
    if len(cantrcv_enable_list) > 0:
        cantrcv_enable_list_flag = "TRUE"
        
    global G_CANNM_RXPDU_UL_RXINDICATION_INDEX
    jinja2_micro_define_list = ["#ifdef", "# undef", "#endif"]
    input_dict = {
        "tx_pdus": tx_pdus,
        "buffer_list": buffer_list,
        "hrh_cfglist": hrh_cfglist,
        "rx_pdus": rx_pdus,
        "controller_channel_count": len(canif_controllers_config),
        "trcv_channel_count": len(canif_transceivers_config),
        "max_lpdu_num_txbuf": max_lpdu_num_txbuf,
        "max_dyn_pdus": len(tx_pdus),
        "max_txbufs": len(buffer_list),
        "canif_ctrl_core_dict": get_g_canif_multiocre_channel_core_dict(),
        "canif_ctrl_multicore_enable": canif_ctrl_multicore_enable,
        "canif_ctrl_single_core_id": canif_ctrl_single_core_id,
        "canif_init_config": canif_init_config,
        "canif_version": canif_version,
        "is_module_used": is_module_used,
        "is_module_rx_used": is_module_rx_used,
        "is_module_tx_used": is_module_tx_used,
        "canif_public_cfg": canif_public_cfg,
        "canif_private_cfg": canif_private_cfg,
        "canif_dispatch_cfg": canif_dispatch_cfg,
        "trcvApi_for_canif_cfg_h": trcvApi_for_canif_cfg_h,
        "trcv_api_enable": trcv_api_enable,
        "multicore_tx_pducfgs_dict": multicore_tx_pducfgs_dict,
        "multicore_txbuffercfgs_dict": multicore_txbuffercfgs_dict,
        "multicore_hrhcfgs_dict": multicore_hrhcfgs_dict,
        'multicore_rx_pducfgs_dict': multicore_rx_pducfgs_dict,
        "canif_controllers_config": canif_controllers_config,
        "canif_transceivers_config": canif_transceivers_config,
        'len_multicore_rx_pducfgs_dict': len_multicore_rx_pducfgs_dict,
        'len_multicore_tx_pducfgs_dict': len_multicore_tx_pducfgs_dict,
        "len_multicore_hrhcfgs_dict": len_multicore_hrhcfgs_dict,
        "len_multicore_txbuffercfgs_dict": len_multicore_txbuffercfgs_dict,
        "canif_activated_core_cnt": canif_activated_core_cnt,
        "master_core_id": master_core_id,
        "all_core_id_list": all_core_id_list,
        "all_core_id_name_list": all_core_id_name_list,
        "all_core_id_enbale_list": all_core_id_enbale_list,
        "canif_max_activated_core_id": canif_max_activated_core_id,
        "all_core_txbuffercfgs_inorder_list": all_core_txbuffercfgs_inorder_list,
        "all_core_tx_pducfgs_list": all_core_tx_pducfgs_list,
        "multicore_txbuffercfgs_start_end_index_dict": multicore_txbuffercfgs_start_end_index_dict,
        "max_lpdu_num_cores_txbuf_dict":  max_lpdu_num_cores_txbuf_dict,
        "canif_manager_var_multi_app_or_private_master_core": canif_manager_var_multi_app_or_private_master_core,
        "jinja2_micro_define_list": jinja2_micro_define_list,
        "canif_txbuffer_size_memmap_section": canif_txbuffer_size_memmap_section,
        "trcv_pn_support_found": trcv_pn_support_found,
        "trcv_wakeup_found": trcv_wakeup_found,
        "ctrl_wakeup_found": ctrl_wakeup_found,
        "cantrcv_enable_list_flag": cantrcv_enable_list_flag,
        "cantrcv_enable_list": cantrcv_enable_list,
        "trcv_type_enable": trcv_type_enable,
        "trcv_enable_list": trcv_enable_list,
        "trcv_channel_enable_type_count": trcv_channel_enable_type_count,
        "offset_index_from_start_index_cantrcv1043": offset_index_from_start_index_cantrcv1043,
        "offset_index_from_start_index_cantrcv1145": offset_index_from_start_index_cantrcv1145,
        "G_CANNM_RXPDU_UL_RXINDICATION_INDEX": G_CANNM_RXPDU_UL_RXINDICATION_INDEX,
        "can_hoh_order_type": can_hoh_order_type,
        "app_coreid_dict": app_coreid_dict,
    }

    return input_dict

def has_multiple_values():
    global g_canif_multiocre_channel_core_dict
    core_id_list = []
    for ctrl_name, ctrl_attr in g_canif_multiocre_channel_core_dict.items():
        core_id_list.append(ctrl_attr["core_id"]) 
    unique_values = set(core_id_list)
    return len(unique_values) > 1, core_id_list[0]

def get_core_id_from_canifctrlcoreref(core_id_ref, ecuc_dict):
    core_id = 0
    if checkIsNotNone(core_id_ref):
        core_id = int(ecuc_dict['EcucHardware'][core_id_ref.split("/")[-1]]['EcucCoreId']['Value'])
    return core_id

def update_core_id(canif_ctrl_core_dict, raw_Ctrlcfgs, ecuc_dict):
    for ctrl_name, ctrl_attr in raw_Ctrlcfgs.items():
        canif_ctrl_core_dict[ctrl_name] = {
            "controller_id": ctrl_attr["CanIfCtrlId"]["Value"], \
            "core_id": get_core_id_from_canifctrlcoreref(ctrl_attr["CanIfCtrlCoreRef"]["ValueRef"], ecuc_dict)
            }
    return canif_ctrl_core_dict

def get_can_version(canversion_value):
    canif_version = "CANIF_AUTOSAR_VERSION_431"
    if (canversion_value == "AUTOSAR_VERSION_431"):
        canif_version = "CANIF_AUTOSAR_VERSION_431"
    elif (canversion_value == "AUTOSAR_VERSION_421"):
        canif_version = "AUTOSAR_CANIF_VERSION_421"
    elif (canversion_value == "AUTOSAR_VERSION_403"):
        canif_version = "AUTOSAR_CANIF_VERSION_403"
    else:
        logger.error("error CanVersion config")
        
    return canif_version

def get_template_config_json_dict(config_dict: dict) -> dict:
    rx_number = 0
    tx_number = 0
    com_config_json_dict = {}
    can_controller_list = []
    for dbc in config_dict["dbc"]:
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
            if "DiagRequest" in msg_value.message_attributes or "DiagResponse" in msg_value.message_attributes or "DiagState" in msg_value.message_attributes:
                msg_dict["module"] = "CANTP"
            elif "NmAsrMessage" in msg_value.message_attributes:
                msg_dict["module"] = "CANNM"
            elif "GenMsgILSupport" in msg_value.message_attributes and "GenMsgCycleTime" not in msg_value.message_attributes:
                msg_dict["module"] = "NONE"
            else:
                msg_dict["module"] = "PDUR"
            msg_dict["senders"] = msg_value.senders[0]
            if "FBCM" == msg_dict["senders"]:
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
        com_config_json_dict[dbc["dbc_name"]] = config_json_dict
        com_config_json_dict["max_rx_number"] = rx_number
        com_config_json_dict["max_tx_number"] = tx_number
    com_config_json_dict["can_controller"] = can_controller_list

    return com_config_json_dict

def get_hrh_cfgs(config_dict: dict, CanConfigSet: dict) -> dict:
    hoh_cfgs = config_dict["CanIfInitCfg"]["CanIfInitHohCfg"]
    hrh_cfgs = {}
    for key in hoh_cfgs.keys():
        if "CanIfHrhIdSymRef" in hoh_cfgs[key] and key != "init_CanIfHrhCfg":
            hrh_cfgs[key] = hoh_cfgs[key]
    for key in hrh_cfgs.keys():
        hrh_name = hrh_cfgs[key]["CanIfHrhIdSymRef"]["ValueRef"].split("/")[-1]
        hrh_cfgs[key]["CanObjectId"] = CanConfigSet[hrh_name]["CanObjectId"]["Value"]
    sorted_hrh_cfgs = dict(sorted(hrh_cfgs.items(), key=lambda d: d[1]["CanObjectId"]))
    return sorted_hrh_cfgs

def get_txpdus_cfgs(config_dict: dict) -> dict:
    canifinitcfgs = config_dict["CanIfInitCfg"]
    tx_pducfgs = {}
    for key in canifinitcfgs.keys():
        if "CanIfTxPduCanId" in canifinitcfgs[key]:
            tx_pducfgs[key] = canifinitcfgs[key]
    return tx_pducfgs

def get_rxpdus_cfgs(config_dict: dict) -> dict:
    canifinitcfgs = config_dict["CanIfInitCfg"]
    rx_pducfgs = {}
    for key in canifinitcfgs.keys():
        if "CanIfRxPduCanId" in canifinitcfgs[key]:
            rx_pducfgs[key] = canifinitcfgs[key]
    return rx_pducfgs

def get_txbuffer_cfgs(config_dict: dict, CanConfigSet: dict) -> dict:
    canifinitcfgs = config_dict["CanIfInitCfg"]
    txbuffercfgs = {}
    for key in canifinitcfgs.keys():
        if "CanIfBufferSize" in canifinitcfgs[key]:
            txbuffercfgs[key] = canifinitcfgs[key]
    for key in txbuffercfgs.keys():
        hth_name = txbuffercfgs[key]["CanIfBufferHthRef"]["ValueRef"].split("/")[-1]
        txbuffercfgs[key]["CanObjectId"] = CanConfigSet[hth_name]["CanObjectId"]["Value"]
    sorted_txbuffercfgs = dict(sorted(txbuffercfgs.items(), key=lambda d: d[1]["CanObjectId"]))
    return sorted_txbuffercfgs

def get_Ctrlcfgs(config_dict: dict) -> dict:
    Ctrlcfgs = {}
    driverIdx = 0
    for key, value in config_dict.items():
        if isinstance(value, dict):
            if value["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg':
                for key in value.keys():
                    if "CanIfCtrlId" in value[key] and key != "init_CanIfCtrlCfg":
                        Ctrlcfgs[key] = value[key]
                        Ctrlcfgs[key]["driverIdx"] = driverIdx
                driverIdx += 1
    return Ctrlcfgs

def get_Trcvcfgs(config_dict: dict) -> dict:
    Trcvcfgs = {}
    driverIdx = 0
    for key1, value1 in config_dict.items():
        if isinstance(value1, dict):
            if value1["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanIf/CanIfTrcvDrvCfg':
                if key1 == "DefinitionRef":
                    continue
                
                for key2, value2 in value1.items():
                    if "CanIfTrcvId" in value2:
                        Trcvcfgs[key2] = value2
                        Trcvcfgs[key2]["driverIdx"] = driverIdx
                    driverIdx += 1
    
    return Trcvcfgs

def get_Publiccfgs(config_dict: dict) -> dict:
    Publiccfgs = {}
    
    Publiccfgs["CanIfPublicCddHeaderFile"] = []
    Publiccfgs["CanIfPublicCddHeaderFileUsed"] = False
    
    Publiccfgs["CanIfPublicUserCbkHeaderFile"] = []
    Publiccfgs["CanIfPublicUserCbkHeaderFileUsed"] = False
    
    for key1, value1 in config_dict.items():
        if isinstance(value1, dict):
            if value1["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanIf/CanIfPublicCfg':
                    
                cddHeaderFile_list = []
                cbkHeaderFile_list = []
                for key2, value2 in value1.items():
                    if key2 == 'DefinitionRef':
                        continue
                    
                    if key2 == "CanIfPublicCddHeaderFile":
                        if checkIsNotNone(value2["Value"]):
                            if isinstance(value2["Value"], list):
                                for file in value2["Value"]:
                                    if file is not None:
                                        cddHeaderFile_list.append(file)
                            elif isinstance(value2["Value"], str):
                                cddHeaderFile_list.append(value2["Value"])
                            else:
                                pass
                        else:
                            cddHeaderFile_list = []
                        Publiccfgs["CanIfPublicCddHeaderFile"] = cddHeaderFile_list
                    elif key2 == "CanIfPublicUserCbkHeaderFile":
                        if value2["Value"] is not None:
                            if isinstance(value2["Value"], list):
                                for file in value2["Value"]:
                                    if file is not None:
                                        cbkHeaderFile_list.append(file)
                            elif isinstance(value2["Value"], str):
                                cbkHeaderFile_list.append(value2["Value"])
                            else:
                                pass
                        else:
                            cbkHeaderFile_list = []
                            
                        Publiccfgs["CanIfPublicUserCbkHeaderFile"] = cbkHeaderFile_list
                    else:
                        Publiccfgs[key2] = value2
                
                if len(cddHeaderFile_list) > 0:
                    Publiccfgs["CanIfPublicCddHeaderFileUsed"] = True
                else:
                    Publiccfgs["CanIfPublicCddHeaderFileUsed"] = False
                
                if len(cbkHeaderFile_list) > 0:
                    Publiccfgs["CanIfPublicUserCbkHeaderFileUsed"] = True
                else:
                    Publiccfgs["CanIfPublicUserCbkHeaderFileUsed"] = False
                    
    return Publiccfgs

def get_Privatecfgs(config_dict: dict) -> dict:
    return config_dict["CanIfPrivateCfg"]

def get_Dispatchcfgs(config_dict: dict) -> dict:
    Dispatchcfgs = {}
    dispatch_aimUL_list = [
        "CanIfDispatchUserCheckTrcvWakeFlagIndicationUL", \
        "CanIfDispatchUserClearTrcvWufFlagIndicationUL", \
        "CanIfDispatchUserConfirmPnAvailabilityUL", \
        "CanIfDispatchUserCtrlBusOffUL", \
        "CanIfDispatchUserCtrlModeIndicationUL", \
        "CanIfDispatchUserTrcvModeIndicationUL", \
        "CanIfDispatchUserValidateWakeupEventUL", \
    ]
    dispatch_aimName_list = [
        "CanIfDispatchUserCheckTrcvWakeFlagIndicationName", \
        "CanIfDispatchUserClearTrcvWufFlagIndicationName", \
        "CanIfDispatchUserConfirmPnAvailabilityName",  \
        "CanIfDispatchUserCtrlBusOffName", \
        "CanIfDispatchUserCtrlModeIndicationName", \
        "CanIfDispatchUserTrcvModeIndicationName", \
        "CanIfDispatchUserValidateWakeupEventName", \
    ]
    for key1, value1 in config_dict.items():
        if isinstance(value1, dict):
            if value1["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanIf/CanIfDispatchCfg':
                for key2, value2 in value1.items():
                    if key2 == 'DefinitionRef':
                        continue
                    elif key2 in dispatch_aimName_list or key2 in dispatch_aimUL_list:
                        Dispatchcfgs[key2] = value2
                    else:
                        pass
    return Dispatchcfgs

def split_multicore_tx_pducfgs(tx_pducfgs):
    multicore_txpdu_map_dict = {}
    for txpdu_item in tx_pducfgs:
        cur_txpdu_coreid = txpdu_item["core_id"]
        core_key = "CORE" + str(cur_txpdu_coreid)
        if core_key not in multicore_txpdu_map_dict.keys():
            multicore_txpdu_map_dict[core_key] = [txpdu_item["name"]]
        else:
            multicore_txpdu_map_dict[core_key].append(txpdu_item["name"])
    multicore_txpdu_map_dict = dict(sorted(multicore_txpdu_map_dict.items(), key=lambda item: int(item[0][-1]))) 
    
    multicore_txpdu_dict = {}
    for txpdu_item in tx_pducfgs:
        cur_txpdu_coreid = txpdu_item["core_id"]
        core_key = "CORE" + str(cur_txpdu_coreid)
        if core_key not in multicore_txpdu_dict.keys():
            multicore_txpdu_dict[core_key] = [txpdu_item]
        else:
            multicore_txpdu_dict[core_key].append(txpdu_item)
    multicore_txpdu_dict = dict(sorted(multicore_txpdu_dict.items(), key=lambda item: int(item[0][-1]))) 
    
    return multicore_txpdu_map_dict, multicore_txpdu_dict

def split_multicore_rx_pducfgs(rx_pducfgs):
    multicore_rxpdu_map_dict = {}
    for txpdu_item  in rx_pducfgs:
        cur_rxpdu_coreid = txpdu_item["core_id"]
        core_key = "CORE" + str(cur_rxpdu_coreid)
        if core_key not in multicore_rxpdu_map_dict.keys():
            multicore_rxpdu_map_dict[core_key] = [txpdu_item["name"]]
        else:
            multicore_rxpdu_map_dict[core_key].append(txpdu_item["name"])
    multicore_rxpdu_map_dict = dict(sorted(multicore_rxpdu_map_dict.items(), key=lambda item: int(item[0][-1]))) 
    
    
    multicore_rxpdu_dict = {}
    for txpdu_item  in rx_pducfgs:
        cur_rxpdu_coreid = txpdu_item["core_id"]
        core_key = "CORE" + str(cur_rxpdu_coreid)
        if core_key not in multicore_rxpdu_dict.keys():
            multicore_rxpdu_dict[core_key] = [txpdu_item]
        else:
            multicore_rxpdu_dict[core_key].append(txpdu_item)
    multicore_rxpdu_dict = dict(sorted(multicore_rxpdu_dict.items(), key=lambda item: int(item[0][-1])))
    
    return multicore_rxpdu_map_dict, multicore_rxpdu_dict

def split_multicore_txbuffercfgs(txbuffercfgs):
    multicore_txbuffer_map_dict = {}
    for txbuffer_item in txbuffercfgs:
        cur_txbuffer_coreid = txbuffer_item["core_id"]
        core_key = "CORE" + str(cur_txbuffer_coreid)
        if core_key not in multicore_txbuffer_map_dict.keys():
            multicore_txbuffer_map_dict[core_key] = [txbuffer_item["buffer_name"]]
        else:
            multicore_txbuffer_map_dict[core_key].append(txbuffer_item["buffer_name"])
    multicore_txbuffer_map_dict = dict(sorted(multicore_txbuffer_map_dict.items(), key=lambda item: int(item[0][-1]))) 
    
    multicore_txbuffer_dict = {}
    for txbuffer_item in txbuffercfgs:
        cur_txbuffer_coreid = txbuffer_item["core_id"]
        core_key = "CORE" + str(cur_txbuffer_coreid)
        if core_key not in multicore_txbuffer_dict.keys():
            multicore_txbuffer_dict[core_key] = [txbuffer_item]
        else:
            multicore_txbuffer_dict[core_key].append(txbuffer_item)
    multicore_txbuffer_dict = dict(sorted(multicore_txbuffer_dict.items(), key=lambda item: int(item[0][-1]))) 

    for _core_name, txbuffer_items in multicore_txbuffer_dict.items():
        buffer_id = 0
        for _index, txbuffer_item in enumerate(txbuffer_items):
            multicore_txbuffer_dict[_core_name][_index]["buffer_id"] = buffer_id
            buffer_id = buffer_id + 1

    return multicore_txbuffer_map_dict, multicore_txbuffer_dict

def split_multicore_ctrlcfgs(canif_controllers_config):
    canif_multiocre_channel_core_dict = get_g_canif_multiocre_channel_core_dict()
    
    multicore_ctrl_map_dict = {}
    for ctrl_name, ctrl_attr in canif_multiocre_channel_core_dict.items():
        core_key = "CORE" + str(ctrl_attr["core_id"])
        if core_key not in multicore_ctrl_map_dict.keys():
            multicore_ctrl_map_dict[core_key] = [ctrl_name]
        else:
            multicore_ctrl_map_dict[core_key].append(ctrl_name)
    
    multicore_ctrl_map_dict = dict(sorted(multicore_ctrl_map_dict.items(), key=lambda item: int(item[0][-1]))) 
    
    multicore_ctrl_dict = {}
    for ctrl_attr in canif_controllers_config:
        for ctrl_n, ctrl_a in canif_multiocre_channel_core_dict.items():
            if ctrl_attr["controller_id"] == ctrl_a["controller_id"]:
                core_key = "CORE" + str(ctrl_a["core_id"]) 
                if core_key not in multicore_ctrl_dict.keys():
                    multicore_ctrl_dict[core_key] = [ctrl_attr]
                else:
                    multicore_ctrl_dict[core_key].append(ctrl_attr)
    
    multicore_ctrl_dict_sorted = dict()
    for key, value in sorted(multicore_ctrl_dict.items(), key=lambda item: int(item[0][-1])):
        multicore_ctrl_dict_sorted[key] = sorted(value, key=lambda x: x['controller_id'])
    
    ctrl_chnl_id = 0
    for _core_name, _core_attr in multicore_ctrl_dict_sorted.items():
        for _index, _item_attr in enumerate(_core_attr):
            multicore_ctrl_dict_sorted[_core_name][_index]["ctrl_chnl_id"] = ctrl_chnl_id
            ctrl_chnl_id = ctrl_chnl_id + 1
            
    return multicore_ctrl_map_dict, multicore_ctrl_dict_sorted
    
def split_multicore_trcvcfgs(trcvcfgs, canif_transceivers_config, canif_controllers_config):
    canif_multiocre_channel_core_dict = get_g_canif_multiocre_channel_core_dict()
    
    multicore_trcv_map_dict = {}
    for trcv_name, trcv_attr in trcvcfgs.items():
        if checkIsNotNone(trcv_attr["CanIfTrcvChnlToCanIfCtrlChnlRef"]["ValueRef"]):
            ctrl_name = trcv_attr["CanIfTrcvChnlToCanIfCtrlChnlRef"]["ValueRef"].split("/")[-1]
            core_key = "CORE" + str(canif_multiocre_channel_core_dict[ctrl_name]["core_id"])
            if core_key not in multicore_trcv_map_dict.keys():
                multicore_trcv_map_dict[core_key] = [trcv_name]
            else:
                multicore_trcv_map_dict[core_key].append(trcv_name)
    multicore_trcv_map_dict = dict(sorted(multicore_trcv_map_dict.items(), key=lambda item: int(item[0][-1]))) 
    
    multicore_trcv_dict = {}
    for trcv_attr in canif_transceivers_config:
        core_key = ""
        controller_id = 0
        ctrlChn_index = trcv_attr["CanIfTrcvChnlToCanIfCtrlChnlRef"]
        for ctrl_cfg in canif_controllers_config:
            if ctrlChn_index == ctrl_cfg["ctrlChn_index"]:
                controller_id = ctrl_cfg["controller_id"]
                break
                
        for ctrl_name, core_attr in canif_multiocre_channel_core_dict.items():
            if controller_id == core_attr["controller_id"]:
                core_key = "CORE" + str(core_attr["core_id"]) 
                if core_key not in multicore_trcv_dict.keys():
                    multicore_trcv_dict[core_key] = [trcv_attr]
                else:
                    multicore_trcv_dict[core_key].append(trcv_attr)
                break
    
    multicore_trcv_dict_sorted = dict()
    for key, value in sorted(multicore_trcv_dict.items(), key=lambda item: int(item[0][-1])):
        multicore_trcv_dict_sorted[key] = sorted(value, key=lambda x: x['transceiver_id'])
    
    trcv_chnl_id = 0
    for _core_name, _core_attr in multicore_trcv_dict_sorted.items():
        for _index, _item_attr in enumerate(_core_attr):
            multicore_trcv_dict_sorted[_core_name][_index]["trcv_chnl_id"] = trcv_chnl_id
            trcv_chnl_id = trcv_chnl_id + 1
            
    return multicore_trcv_map_dict, multicore_trcv_dict_sorted

def split_multicore_hrhcfgs(hrhcfgs):
    multicore_hrhcfgs_map_dict = {}
    for hrh_item in hrhcfgs:
        cur_hrh_coreid = hrh_item["core_id"]
        core_key = "CORE" + str(cur_hrh_coreid)
        if core_key not in multicore_hrhcfgs_map_dict.keys():
            multicore_hrhcfgs_map_dict[core_key] = [hrh_item["name"]]
        else:
            multicore_hrhcfgs_map_dict[core_key].append(hrh_item["name"])
    multicore_hrhcfgs_map_dict = dict(sorted(multicore_hrhcfgs_map_dict.items(), key=lambda item: int(item[0][-1]))) 
    
    multicore_hrh_dict = {}
    for hrh_item in hrhcfgs:
        cur_hrh_coreid = hrh_item["core_id"]
        core_key = "CORE" + str(cur_hrh_coreid)
        if core_key not in multicore_hrh_dict.keys():
            multicore_hrh_dict[core_key] = [hrh_item]
        else:
            multicore_hrh_dict[core_key].append(hrh_item)
    multicore_hrh_dict = dict(sorted(multicore_hrh_dict.items(), key=lambda item: int(item[0][-1])))
     
    return multicore_hrhcfgs_map_dict, multicore_hrh_dict

def get_multiCddRxPduIdName(pdu: dict, cdd_rxpdu_id_dict: dict):
    prefix = ""
    postfix = pdu["CanIfRxPduRef"]["ValueRef"].split("/")[-1]
    if postfix in cdd_rxpdu_id_dict.keys():
        prefix = cdd_rxpdu_id_dict[postfix]["cdd_module_name"].upper() + "_PDU_ID_"
    else:
        prefix = "CDD_PDU_ID_"
        postfix = pdu["ShortName"]
        
    pdu_id_name = prefix + postfix
    return pdu_id_name

def get_multiCddTxPduIdName(pdu: dict, cdd_txpdu_id_dict: dict):
    prefix = ""
    postfix = pdu["CanIfTxPduRef"]["ValueRef"].split("/")[-1]
    if postfix in cdd_txpdu_id_dict.keys():
        prefix = cdd_txpdu_id_dict[postfix]["cdd_module_name"].upper() + "_PDU_ID_"
    else:
        prefix = "CDD_PDU_ID_"
        postfix = pdu["ShortName"]
        
    pdu_id_name = prefix + postfix
    return pdu_id_name

def get_txpdu_info(pdu: object, txbuffercfgs: object, ecuc_dict: dict, cdd_txpdu_id_dict: dict) -> dict:
    tx_pdu = dict.fromkeys([
        "shortname", "name", "canid", "txpdu_type", "dlc", "canid_type", "CanIfUserTxConfirmation_index", "CanIfTxPduBufferRef", "CanIfTxPduBufferRefName"
        "txCalloutFunctionName", "CanIfTxPduPnFilterEnable", "txpduRefName", "core_id"
    ])
    
    tx_pdu["shortname"] = pdu["ShortName"]
    tx_pdu["canid"] = hex(pdu["CanIfTxPduCanId"]["Value"])
    
    if pdu["CanIfTxPduPnFilterPdu"]["Value"] == False:
        tx_pdu["CanIfTxPduPnFilterEnable"] = "FALSE"
    else:
        tx_pdu["CanIfTxPduPnFilterEnable"] = "TRUE"
        
    if "CanIfTxPduUserTxCalloutName" not in pdu or pdu["CanIfTxPduUserTxCalloutName"]["Value"] == '0' or pdu[
            "CanIfTxPduUserTxCalloutName"]["Value"] == None or pdu["CanIfTxPduUserTxCalloutName"]["Value"] == '':
        tx_pdu["txCalloutFunctionName"] = "NULL"
    else:
        tx_pdu["txCalloutFunctionName"] = "&" + pdu["CanIfTxPduUserTxCalloutName"]["Value"]

    if pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "PDUR":
        tx_pdu["CanIfUserTxConfirmation_index"] = 0
        tx_pdu["name"] = "PDUR_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "CAN_NM":
        tx_pdu["CanIfUserTxConfirmation_index"] = 1
        tx_pdu["name"] = "CANNM_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "CAN_TP":
        tx_pdu["CanIfUserTxConfirmation_index"] = 2
        tx_pdu["name"] = "CANTP_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "XCP":
        tx_pdu["CanIfUserTxConfirmation_index"] = 3
        tx_pdu["name"] = "XCP_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "CDD":
        tx_pdu["CanIfUserTxConfirmation_index"] = 4
        tx_pdu["name"] = get_multiCddTxPduIdName(pdu, cdd_txpdu_id_dict)
        
    elif pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "CAN_TSYN":
        tx_pdu["CanIfUserTxConfirmation_index"] = 5
        tx_pdu["name"] = "CANTSYN_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "DLT":
        tx_pdu["CanIfUserTxConfirmation_index"] = 6
        tx_pdu["name"] = "DLT_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfTxPduUserTxConfirmationUL"]["Value"] == "Ftrace":
        tx_pdu["CanIfUserTxConfirmation_index"] = 7
        tx_pdu["name"] = "FTRACE_PDU_ID_" + pdu["ShortName"]
    else:
        logger.debug("error CanIfTxPduUserTxConfirmationUL config")
    
    tx_pdu["subname"] = "CANIF_PDU_ID_" + pdu["ShortName"]
    buffer_index = 0
    head, tail = os.path.split(pdu["CanIfTxPduBufferRef"]["ValueRef"])
    for key in txbuffercfgs.keys():
        if tail == key:
            tx_pdu["CanIfTxPduBufferRef"] = buffer_index
            tx_pdu["CanIfTxPduBufferRefName"] = key
            break
        buffer_index += 1
    if buffer_index >= len(txbuffercfgs):
        logger.error('%s :txbuffer reference error' % tx_pdu["name"])
    tx_pdu["CanIfTxPduBufferRef"] = buffer_index
    if pdu["CanIfTxPduType"]["Value"] in ["STATIC", "DYNAMIC"]:
        tx_pdu["txpdu_type"] = pdu["CanIfTxPduType"]["Value"]
    else:
        logger.error("error CanIfTxPduType")
    tx_pdu["dlc"] = str(pdu["CanIfTxPduDataLength"]["Value"])
    if pdu["CanIfTxPduCanIdType"]["Value"] == "STANDARD_FD_CAN":
        tx_pdu["canid_type"] = "CANIF_CAN_FD_ID_TYPE_11"
    elif pdu["CanIfTxPduCanIdType"]["Value"] == "EXTENDED_CAN":
        tx_pdu["canid_type"] = "CANIF_CAN_ID_TYPE_29"
    elif pdu["CanIfTxPduCanIdType"]["Value"] == "EXTENDED_FD_CAN":
        tx_pdu["canid_type"] = "CANIF_CAN_FD_ID_TYPE_29"
    elif pdu["CanIfTxPduCanIdType"]["Value"] == "STANDARD_CAN":
        tx_pdu["canid_type"] = "CANIF_CAN_ID_TYPE_11"
    else:
        logger.debug("error canid")
    
    tx_pdu["txpduRefName"] = ""
    if checkIsNotNone(pdu["CanIfTxPduRef"]["ValueRef"]):
        tx_pdu["txpduRefName"] = pdu["CanIfTxPduRef"]["ValueRef"].split("/")[-1]
    else:
        logger.debug("ERROR: CanIfTxPduRef is None")
    
    if tx_pdu["txpduRefName"] in ecuc_dict["EcucConfigSet"]["EcucPduCollection"].keys():  
        coreid_ref = ecuc_dict["EcucConfigSet"]["EcucPduCollection"][tx_pdu["txpduRefName"]]["EcucPduDefaultCoreRef"]["ValueRef"]
        txpdu_core_id = 0
        if checkIsNotNone(coreid_ref):
            txpdu_core_id = int(ecuc_dict['EcucHardware'][coreid_ref.split("/")[-1]]['EcucCoreId']['Value'])
        tx_pdu["core_id"] = txpdu_core_id
    else:
        tx_pdu["core_id"] = 0
    
    return tx_pdu

def get_transmit_buffer_info(key: str, buffer_cfg: dict, CanConfigSet: dict) -> dict:
    txbuffer = dict.fromkeys(["buffer_name", "buffer_id", "buffer_size", "hth_name", "hth_type", "hth_id", "controller_id", "core_id"])
    txbuffer["core_id"] = 0
    txbuffer["buffer_name"] = key
    txbuffer["buffer_size"] = buffer_cfg["CanIfBufferSize"]["Value"]
    hth_name = buffer_cfg["CanIfBufferHthRef"]["ValueRef"].split("/")[-1]
    txbuffer["hth_type"] = CanConfigSet[hth_name]["CanHandleType"]["Value"]
    txbuffer["hth_id"] = CanConfigSet[hth_name]["CanObjectId"]["Value"]
    txbuffer["hth_name"] = hth_name
    head, controller_idref = os.path.split(CanConfigSet[hth_name]["CanControllerRef"]["ValueRef"])
    txbuffer["controller_id"] = CanConfigSet[controller_idref]["CanControllerId"]["Value"]
    
    canif_multiocre_channel_core_dict = get_g_canif_multiocre_channel_core_dict()
    for _, ctrl_attr in canif_multiocre_channel_core_dict.items():
        if ctrl_attr["controller_id"] == txbuffer["controller_id"]:
            txbuffer["core_id"] = ctrl_attr["core_id"]
            
    return txbuffer

def get_hth_cfgdata(tx_message_index_offset: int, base_tx_message_index: int, channel_name: str) -> list:
    hthcfg = []
    hthcfg.append(str(tx_message_index_offset + base_tx_message_index))
    hthcfg.append("CANIF_HANDLE_TYPE_FULL")
    hthcfg.append("CanIfConf_CanIfCtrlCfg_" + channel_name)
    hthcfg.append("4")
    return hthcfg

def get_rxpdu_info(pdu: object, hrhcfgs: dict, ecuc_dict: dict, cdd_rxpdu_id_dict: dict) -> dict:
    rx_pdu = dict.fromkeys([
        "name", "lower_canid", "upper_canid", "canid", "is_extend", "dlc", "CanIfUserRxIndication_index", "hrh_ref",
        "calloutfunction_name", "rxpduRefName"
    ])
    if "CanIfRxPduUserRxCalloutName" not in pdu or pdu["CanIfRxPduUserRxCalloutName"]["Value"] == '0' or pdu["CanIfRxPduUserRxCalloutName"]["Value"] == '' or pdu[
            "CanIfRxPduUserRxCalloutName"]["Value"] == None:
        rx_pdu["calloutfunction_name"] = "NULL"
    else:
        rx_pdu["calloutfunction_name"] = "&" + pdu["CanIfRxPduUserRxCalloutName"]["Value"]
    rx_pdu["canid"] = hex(pdu["CanIfRxPduCanId"]["Value"])
    can_id_value = int(rx_pdu["canid"], base=16)
    if pdu["CanIfRxPduDataLengthCheck"]["Value"] == True:
        if pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "CAN_TP":
            logger.error("error CanIfRxPduDataLengthCheck config,diag message must switch off DLC check")
        else:
            rx_pdu["dlc"] = str(pdu["CanIfRxPduDataLength"]["Value"])
    else:
        rx_pdu["dlc"] = 0
    if "CanIfRxPduCanIdRange" in pdu:
        rx_pdu["lower_canid"] = hex(pdu["CanIfRxPduCanIdRange"]["CanIfRxPduCanIdRangeLowerCanId"]["Value"])
        rx_pdu["upper_canid"] = hex(pdu["CanIfRxPduCanIdRange"]["CanIfRxPduCanIdRangeUpperCanId"]["Value"])
    else:
        rx_pdu["lower_canid"] = "CANIF_INVALID_CANID"
        rx_pdu["upper_canid"] = "CANIF_INVALID_CANID"
    hrh_ref_index = 0
    head, tail = os.path.split(pdu["CanIfRxPduHrhIdRef"]["ValueRef"])
    for key in hrhcfgs.keys():
        if tail == key:
            rx_pdu["hrh_ref"] = hrh_ref_index
            break
        hrh_ref_index += 1
    if pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "PDUR":
        rx_pdu["CanIfUserRxIndication_index"] = 0
        rx_pdu["name"] = "PDUR_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "CAN_NM":
        global G_CANNM_RXPDU_UL_RXINDICATION_INDEX
        rx_pdu["CanIfUserRxIndication_index"] = G_CANNM_RXPDU_UL_RXINDICATION_INDEX
        rx_pdu["name"] = "CANNM_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "CAN_TP":
        rx_pdu["CanIfUserRxIndication_index"] = 2
        rx_pdu["name"] = "CANTP_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "XCP":
        rx_pdu["CanIfUserRxIndication_index"] = 3
        rx_pdu["name"] = "XCP_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "DLT":
        rx_pdu["CanIfUserRxIndication_index"] = 6
        rx_pdu["name"] = "DLT_PDU_ID_" + pdu["ShortName"]
    elif pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "Ftrace":
        rx_pdu["CanIfUserRxIndication_index"] = 7
        rx_pdu["name"] = "FTRACE_PDU_ID_" + pdu["ShortName"]        
    elif pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "CDD":
        rx_pdu["CanIfUserRxIndication_index"] = 4
        rx_pdu["name"] = get_multiCddRxPduIdName(pdu, cdd_rxpdu_id_dict)
    elif pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "CAN_TSYN":
        rx_pdu["CanIfUserRxIndication_index"] = 5
        rx_pdu["name"] = "CANTSYN_PDU_ID_" + pdu["ShortName"]
    else:
        logger.debug("error CanIfRxPduUserRxIndicationUL config")
    rx_pdu["subname"] = "CANIF_PDU_ID_" + pdu["ShortName"]
    if can_id_value <= 0x7ff:
       rx_pdu["is_extend"] = "FALSE"
    else:
        rx_pdu["is_extend"] = "TRUE"
    
    rx_pdu["rxpduRefName"] = ""
    if checkIsNotNone(pdu["CanIfRxPduRef"]["ValueRef"]):
        rx_pdu["txpduRefName"] = pdu["CanIfRxPduRef"]["ValueRef"].split("/")[-1]
    else:
        logger.debug("ERROR: CanIfRxPduRef is None")
    
    if rx_pdu["txpduRefName"] in ecuc_dict["EcucConfigSet"]["EcucPduCollection"].keys():  
        coreid_ref = ecuc_dict["EcucConfigSet"]["EcucPduCollection"][rx_pdu["txpduRefName"]]["EcucPduDefaultCoreRef"]["ValueRef"]
        rxpdu_core_id = 0
        if checkIsNotNone(coreid_ref):
            rxpdu_core_id = int(ecuc_dict['EcucHardware'][coreid_ref.split("/")[-1]]['EcucCoreId']['Value'])
        rx_pdu["core_id"] = rxpdu_core_id
    else:
        rx_pdu["core_id"] = 0
    
    return rx_pdu

def get_hrh_cfgdata(hrh_cfg: dict, rx_pdus: list, hrh_index: int, CanConfigSet: dict, canif_ctrl_multicore_enable: bool) -> dict:
    pdu_index = 0
    min_index = sys.maxsize
    hrhcfg = dict.fromkeys([
        "rxpdulist_name", "name", "hrh_id", "hrh_type", "is_software_fliter", "controller_id", "rx_pdulist_start_index",
        "rx_pducount", "core_id"
    ])
    hrhcfg["core_id"] = 0
    
    receiveobject_name = hrh_cfg["CanIfHrhIdSymRef"]["ValueRef"].split("/")[-1]
    hrhcfg["name"] = hrh_cfg["ShortName"]
    hrh_cfg_dict = CanConfigSet[receiveobject_name]
    hrhcfg["hrh_id"] = hrh_cfg_dict["CanObjectId"]["Value"]
    hrhcfg["hrh_type"] = hrh_cfg_dict["CanHandleType"]["Value"]
    if hrhcfg["hrh_type"] == "BASIC":
        hrhcfg["is_software_fliter"] = "TRUE"
    elif hrhcfg["hrh_type"] == "FULL":
        hrhcfg["is_software_fliter"] = "FALSE"
    head, controllername = os.path.split(hrh_cfg["CanIfHrhCanCtrlIdRef"]["ValueRef"])
    hrhcfg["controller_id"] = CanConfigSet[controllername]["CanControllerId"]["Value"]
    
    canif_multiocre_channel_core_dict = get_g_canif_multiocre_channel_core_dict()
    for _, ctrl_attr in canif_multiocre_channel_core_dict.items():
        if ctrl_attr["controller_id"] == hrhcfg["controller_id"]:
            hrhcfg["core_id"] = ctrl_attr["core_id"]
            
    hrhcfg["rx_pducount"] = 0
    for temp_rxpdu in rx_pdus:
        if temp_rxpdu["hrh_ref"] == hrh_index:
            hrhcfg["rx_pducount"] += 1
            if pdu_index <= min_index:
                min_index = pdu_index
        pdu_index += 1
    hrhcfg["rx_pdulist_start_index"] = min_index
    if hrhcfg["rx_pdulist_start_index"] == sys.maxsize:
        hrhcfg["rxpdulist_name"] = "NULL"
    else:
        hrhcfg["rxpdulist_name"] = "&hrh_rx_pdu_canif_hrh_cfg[" + str(hrhcfg["rx_pdulist_start_index"]) + "]"
            
    return hrhcfg

def get_controller_cfgdata(Ctrlcfg: dict, txbuffercfgs: dict, CanConfigSet: dict, ctrlChn_index: int, ecuc_dict: dict, canif_ctrl_multicore_enable: bool) -> dict:
    canif_controller_config = dict.fromkeys([
        "controller_name", "controller_id", "ctrl_chnl_id", "txbuffer_count", "txbuffer_startindex", "is_wakeup", "wakeupsource",
        "is_pnfilter", "txBufferRefList", "txBufferRefList_startname", "core_id"
    ])
    
    buffer_count = 0
    canif_controller_config["controller_name"] = Ctrlcfg["ShortName"]
    canif_controller_config["ctrlChn_index"]   = ctrlChn_index
    canif_controller_config["ctrl_chnl_id"] = 0
    canif_controller_config["driverIdx"] = Ctrlcfg["driverIdx"]
    canif_controller_config["txBufferRefList_startname"] = "INVALID_HTH_NAME"
    canif_controller_config["core_id"] = get_core_id_from_canifctrlcoreref(Ctrlcfg["CanIfCtrlCoreRef"]["ValueRef"], ecuc_dict)
    
    start_index_flag = False
    start_index_hth_name = ""
    start_index = sys.maxsize
    
    index_txbuffer_list = 0
    for _core_name, _txbuffer_list in txbuffercfgs.items():
        for buffer_cfg in _txbuffer_list:
            hth_name = buffer_cfg["hth_name"]
            head, controller_idref = os.path.split(CanConfigSet[hth_name]["CanControllerRef"]["ValueRef"])
            if int(CanConfigSet[controller_idref]["CanControllerId"]["Value"]) == Ctrlcfg["CanIfCtrlId"]["Value"]:
                buffer_count += 1
                if start_index_flag == False:
                    start_index = index_txbuffer_list
                    start_index_hth_name = hth_name
                    start_index_flag = True
            index_txbuffer_list = index_txbuffer_list + 1
    
    if start_index == sys.maxsize:
        canif_controller_config["txBufferRefList"] = "NULL"
    else:
        canif_controller_config["txBufferRefList"] = "&canif_ctrl_cfg_buffer_list[" + str(start_index) + "]"
        canif_controller_config["txBufferRefList_startname"] = start_index_hth_name
        
    canif_controller_config["txbuffer_startindex"] = start_index
    canif_controller_config["txbuffer_count"] = buffer_count
    canif_controller_config["controller_id"] = Ctrlcfg["CanIfCtrlId"]["Value"]
    if Ctrlcfg["CanIfCtrlWakeupSupport"]["Value"] == True:
        canif_controller_config["is_wakeup"] = 'TRUE'
    else:
        canif_controller_config["is_wakeup"] = 'FALSE'
    canif_controller_config["wakeupsource"] = 0
    canif_controller_config["is_pnfilter"] = "STD_ON"
    return canif_controller_config

def get_cantrcv1043_api(apiType: str) -> str:
    all_api_dict = {
        "CanIfTrcvCheckWakeUpFlagFctName": "NULL", \
        "CanIfTrcvClearTrcvWuFlagFctName": "NULL", \
        "CanIfTrcvCheckWakeUpFctName": "CanTrcv_Tja1043_CheckWakeup", \
        "CanIfTrcvSetOpModeFctName": "CanTrcv_Tja1043_SetOpMode", \
        "CanIfTrcvSetWakeupModeFctName": "CanTrcv_Tja1043_SetWakeupMode", \
        "CanIfTrcvGetBusWuReasonFctName": "CanTrcv_Tja1043_GetBusWuReason", \
        "CanIfTrcvGetOpModeFctName": "CanTrcv_Tja1043_GetOpMode", \
            
        "CanIfTrcvCheckTrcvWakeFlagIndicationFctName": "NULL", \
        "CanIfTrcvClearTrcvWufFlagIndicationFctName": "NULL", \
        "CanIfTrcvTrcvModeIndicationFctName": "canif_tja1043_trcv_mode_indication", \
        "CanIfTrcvConfirmPnAvailabilityFctName": "NULL"
        }
    ret_api = "NULL"
    if apiType in all_api_dict.keys():
        ret_api = all_api_dict[apiType]
        
    return ret_api

def get_cantrcv1145_api(apiType: str) -> str:
    all_api_dict = {
        "CanIfTrcvCheckWakeUpFlagFctName": "cantrcv_wk_flag_check", \
        "CanIfTrcvClearTrcvWuFlagFctName": "cantrcv_wk_flag_clear", \
        "CanIfTrcvCheckWakeUpFctName": "cantrcv_wk_check", \
        "CanIfTrcvSetOpModeFctName": "cantrcv_op_mode_set", \
        "CanIfTrcvSetWakeupModeFctName": "cantrcv_wk_mode_set", \
        "CanIfTrcvGetBusWuReasonFctName": "cantrcv_bus_wk_reason_get", \
        "CanIfTrcvGetOpModeFctName": "cantrcv_op_mode_get", \
            
        "CanIfTrcvCheckTrcvWakeFlagIndicationFctName": "canif_tja1145_check_trcv_wake_flag_indication", \
        "CanIfTrcvClearTrcvWufFlagIndicationFctName": "canif_tja1145_clear_trcv_wuf_flag_indication", \
        "CanIfTrcvTrcvModeIndicationFctName": "canif_tja1145_trcv_mode_indication", \
        "CanIfTrcvConfirmPnAvailabilityFctName": "canif_tja1145_confirm_pn_availability"
        }
    ret_api = "NULL"
    if apiType in all_api_dict.keys():
        ret_api = all_api_dict[apiType]
        
    return ret_api

def get_cantrcv_api(cantrcv:str, apiType: str) -> str:    
    ret_api = "NULL"
    if cantrcv == "CanTrcv1145":
        ret_api = get_cantrcv1145_api(apiType)
    elif cantrcv == "CanTrcv1043":
        ret_api = get_cantrcv1043_api(apiType)
    else:
        pass
    return ret_api

def get_cantrcv1145_pnsupport(cantrcv1145_dict: dict, cantrcvChnlName: str):
    hwPnSupport = False
    pnEnabled = False
    
    ret = False 
    
    for key1, value1 in cantrcv1145_dict.items():
            if key1 == "DefinitionRef":
                continue
            
            if value1["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanTrcv1145/CanTrcvConfigSet':
                for key2, value2 in value1.items():
                    if key2 == "DefinitionRef":
                        continue
                    if key2 == "CanTrcvSPICommRetries" or key2 == "CanTrcvSPICommTimeout":
                        continue
                
                    if key2 == cantrcvChnlName:
                        hwPnSupport = (value2["CanTrcvHwPnSupport"]["Value"] == True)
                        for key3, value3 in value2.items():
                            if key3 == "DefinitionRef":
                                continue
                            if value3["DefinitionRef"] == "/AUTOSAR/EcucDefs/CanTrcv1145/CanTrcvConfigSet/CanTrcvChannel/CanTrcvPartialNetwork":
                                pnEnabled = (value3["CanTrcvPnEnabled"]["Value"] == True)
                                  
                        if hwPnSupport and pnEnabled:
                            ret = True
                        break
    return ret
                    
def get_pnEnbaled_from_trcv(cantrcv_type: str, all_cantrcv_cfg_dict: dict, cantrcvChnlName: str) -> str:
    pnEnabled = False
    if cantrcv_type == "CanTrcv1145":
        cantrcv1145_dict = all_cantrcv_cfg_dict["cantrcv1145"]
        pnEnabled = get_cantrcv1145_pnsupport(cantrcv1145_dict, cantrcvChnlName)
    return pnEnabled

def get_cantrcv_header_file(cantrcv_type: str) -> str:
    header_file = ""
    if cantrcv_type == "CanTrcv1145":
        header_file = "CanTrcv_Tja1145.h"
    elif cantrcv_type == "CanTrcv1043":
        header_file = "CanTrcv_Tja1043.h"
    else:
        logger.warning("CanIf does not adapt the %s module and cannot infer the header file!" % cantrcv_type)
    
    return header_file

def get_cantrcv1043AllWakeUpSource(cantrcvCfg: dict, trcvref_name: str) -> dict:
    all_src = dict.fromkeys([
        "STD", "POR", "SYSERR"
    ])
    
    all_src["STD"] = ""
    all_src["POR"] = ""
    all_src["SYSERR"] = ""
    
    for key1, value1 in cantrcvCfg.items():
        if key1 == "DefinitionRef":
            continue
        
        if value1["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanTrcv1043/CanTrcvConfigSet':
            for key2, value2 in value1.items():
                if key2 == "DefinitionRef":
                    continue
                if value2["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanTrcv1043/CanTrcvConfigSet/CanTrcvChannel' and (key2 == trcvref_name):
                    for key3, value3 in value2.items():
                        if key3 == "DefinitionRef":
                            continue
                        if key3 == "CanTrcvWakeupSourceRef" and checkIsNotNone(value3["ValueRef"]):
                            all_src["STD"] = value3["ValueRef"].split('/')[-1]
                       
    return all_src

def get_cantrcv1145AllWakeUpSource(cantrcvCfg: dict, trcvref_name: str) -> dict:
    all_src = dict.fromkeys([
        "STD", "POR", "SYSERR"
    ])
    
    all_src["STD"] = ""
    all_src["POR"] = ""
    all_src["SYSERR"] = ""
    
    for key1, value1 in cantrcvCfg.items():
        if key1 == "DefinitionRef":
            continue
        
        if value1["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanTrcv1145/CanTrcvConfigSet':
            for key2, value2 in value1.items():
                if key2 == "DefinitionRef":
                    continue
                if value2["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanTrcv1145/CanTrcvConfigSet/CanTrcvChannel' and (key2 == trcvref_name):
                    for key3, value3 in value2.items():
                        if key3 == "DefinitionRef":
                            continue
                        if key3 == "CanTrcvWakeupSourceRef" and checkIsNotNone(value3["ValueRef"]):
                            all_src["STD"] = value3["ValueRef"].split('/')[-1]
                        elif key3 == "CanTrcvPorWakeupSourceRef" and checkIsNotNone(value3["ValueRef"]):
                            all_src["POR"] = value3["ValueRef"].split('/')[-1]
                        elif key3 == "CanTrcvSyserrWakeupSourceRef" and checkIsNotNone(value3["ValueRef"]):
                            all_src["SYSERR"] = value3["ValueRef"].split('/')[-1]
                        else:
                            pass
                        
    return all_src

def get_cantrcvWakeUpSource(module_referenced: str, trcvref_name: str, cantrcvCfg: dict) -> dict:
    wuSrc = dict.fromkeys([
        "STD", "POR", "SYSERR"
    ])
    wuSrc["STD"] = ""
    wuSrc["POR"] = ""
    wuSrc["SYSERR"] = ""
    
    if module_referenced == "CanTrcv1145":
        wuSrc = get_cantrcv1145AllWakeUpSource(cantrcvCfg, trcvref_name)
    elif module_referenced == "CanTrcv1043":
        wuSrc = get_cantrcv1043AllWakeUpSource(cantrcvCfg, trcvref_name)
    else:
        pass
    
    return wuSrc

def get_cantrcv_chnlRef(module_referenced: str, CanIfTrcvCanTrcv: str)-> str:
    chnlName: str = "0xff"
    if module_referenced == "CanTrcv1145":
        chnlName = "CanTrcv_Tja1145_"
    elif module_referenced == "CanTrcv1043":
        chnlName = "CanTrcv_Tja1043_"
    else:
        pass
    
    if chnlName != "0xff":
        chnlName = chnlName + CanIfTrcvCanTrcv
    return chnlName

def get_canif_transceiver_config(Trcvcfg: dict, Ctrlcfgs: dict, all_cantrcv_cfg_dict: dict, trcvChn_index: int) -> dict: 
    canif_transceiver_config = dict.fromkeys([
        "cantrcv_used", "core_id", "trcv_chnl_id", \
        "cantrcv_header_file", \
        "transceiver_name", "trcvChn_index", "transceiver_id", "is_wakeup",  "wakeupsource", "driverIdx", "is_pnSupport", \
        "CanIfTrcvCheckWakeUpFlagFctName", "CanIfTrcvClearTrcvWuFlagFctName", "CanIfTrcvCheckWakeUpFctName", \
        "CanIfTrcvSetOpModeFctName", "CanIfTrcvSetWakeupModeFctName", "CanIfTrcvGetBusWuReasonFctName", "CanIfTrcvGetOpModeFctName", \
        "CanIfTrcvCheckTrcvWakeFlagIndicationFctName", "CanIfTrcvClearTrcvWufFlagIndicationFctName", "CanIfTrcvTrcvModeIndicationFctName", "CanIfTrcvConfirmPnAvailabilityFctName", \
        "CanIfTrcvCanTrcv", "CanIfTrcvCanTrcvRefName", \
        "CanIfTrcvChnlToCanIfCtrlChnlRef", \
        "module_referenced", \
        "CanIfCanTrcvFctGrpId",
    ])
    
    canif_transceiver_config["trcv_chnl_id"] = 0
    canif_transceiver_config["cantrcv_used"] = False
    canif_transceiver_config["cantrcv_header_file"] = ""
    canif_transceiver_config["CanIfTrcvCanTrcvRefName"] = "0xff"

    canif_transceiver_config["transceiver_name"] = Trcvcfg["ShortName"]
    canif_transceiver_config["trcvChn_index"] = trcvChn_index
    
    canif_transceiver_config["transceiver_id"] = Trcvcfg["CanIfTrcvId"]["Value"] 
    
    if Trcvcfg["CanIfTrcvWakeupSupport"]["Value"] == False:
        canif_transceiver_config["is_wakeup"] = 'FALSE'
    else:
        canif_transceiver_config["is_wakeup"] = 'TRUE'
    
    canif_transceiver_config["wakeupsource"] = 0 
    canif_transceiver_config["driverIdx"] = Trcvcfg["driverIdx"]
    canif_transceiver_config["module_referenced"] = "NULL"
    
    module_referenced = ""
    
    canif_transceiver_config["CanIfTrcvCanTrcv"] = Trcvcfg["CanIfTrcvCanTrcv"]["Value"]
    if checkIsNotNone(canif_transceiver_config["CanIfTrcvCanTrcv"]): 
        canif_transceiver_config["cantrcv_used"] = True
        module_referenced = Trcvcfg["CanTrcvType"]["Value"]
        canif_transceiver_config["module_referenced"] = module_referenced
        canif_transceiver_config["CanIfTrcvCanTrcvRefName"] = get_cantrcv_chnlRef(module_referenced, canif_transceiver_config["CanIfTrcvCanTrcv"])
        canif_transceiver_config["cantrcv_header_file"] = get_cantrcv_header_file(module_referenced)
        canif_transceiver_config["is_pnSupport"] = get_pnEnbaled_from_trcv(module_referenced, all_cantrcv_cfg_dict, canif_transceiver_config["CanIfTrcvCanTrcv"])
        
        # wu_exist = False
        # for _, trcvAttr in all_cantrcv_cfg_dict.items():
        #     if bool(trcvAttr):
        #         wu_exist =True
                
        # if wu_exist:
        #     cantrcvCfg = {}
        #     if module_referenced == "CanTrcv1145":
        #         cantrcvCfg = all_cantrcv_cfg_dict["cantrcv1145"]
        #     elif module_referenced == "CanTrcv1043":
        #         cantrcvCfg = all_cantrcv_cfg_dict["cantrcv1043"]
        #     else:
        #         pass
        #     cantrcv_ref_name = canif_transceiver_config["CanIfTrcvCanTrcv"]
        #     canif_transceiver_config["wakeupsource"] = get_cantrcvWakeUpSource(module_referenced, cantrcv_ref_name, cantrcvCfg)["STD"]
        #     if canif_transceiver_config["wakeupsource"] == "":
        #         canif_transceiver_config["wakeupsource"] = 0
        # else:
        #     canif_transceiver_config["wakeupsource"] = 0 
        wu_exist = False
        if checkIsNotNone(Trcvcfg["CanTrcvWakeupSourceRef"]["ValueRef"]):
            wu_exist = True
        if wu_exist:
            canif_transceiver_config["wakeupsource"] = Trcvcfg["CanTrcvWakeupSourceRef"]["ValueRef"].split('/')[-1]
        else:
            canif_transceiver_config["wakeupsource"] = 0 
    else:
        canif_transceiver_config["CanIfTrcvCanTrcvRefName"] = "0xff"
        canif_transceiver_config["cantrcv_used"] = False
        canif_transceiver_config["cantrcv_header_file"] = ""
        canif_transceiver_config["is_pnSupport"] = False
        canif_transceiver_config["wakeupsource"] = 0 
        
    canif_transceiver_config["CanIfTrcvCheckWakeUpFlagFctName"] =  get_cantrcv_api(module_referenced, "CanIfTrcvCheckWakeUpFlagFctName")
    canif_transceiver_config["CanIfTrcvClearTrcvWuFlagFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvClearTrcvWuFlagFctName")
    canif_transceiver_config["CanIfTrcvCheckWakeUpFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvCheckWakeUpFctName")
    canif_transceiver_config["CanIfTrcvSetOpModeFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvSetOpModeFctName")
    canif_transceiver_config["CanIfTrcvSetWakeupModeFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvSetWakeupModeFctName")
    canif_transceiver_config["CanIfTrcvGetBusWuReasonFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvGetBusWuReasonFctName")
    canif_transceiver_config["CanIfTrcvGetOpModeFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvGetOpModeFctName")
    
    canif_transceiver_config["CanIfTrcvCheckTrcvWakeFlagIndicationFctName"] =  get_cantrcv_api(module_referenced, "CanIfTrcvCheckTrcvWakeFlagIndicationFctName")
    canif_transceiver_config["CanIfTrcvClearTrcvWufFlagIndicationFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvClearTrcvWufFlagIndicationFctName")
    canif_transceiver_config["CanIfTrcvTrcvModeIndicationFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvTrcvModeIndicationFctName")
    canif_transceiver_config["CanIfTrcvConfirmPnAvailabilityFctName"] = get_cantrcv_api(module_referenced, "CanIfTrcvConfirmPnAvailabilityFctName")    
    
    canif_transceiver_config["core_id"] = 0
    if "CanIfTrcvChnlToCanIfCtrlChnlRef" in Trcvcfg.keys() and \
        Trcvcfg["CanIfTrcvChnlToCanIfCtrlChnlRef"]["ValueRef"] is not None and \
            Trcvcfg["CanIfTrcvChnlToCanIfCtrlChnlRef"]["ValueRef"] != '':
        _, ctrlChnl = os.path.split(Trcvcfg["CanIfTrcvChnlToCanIfCtrlChnlRef"]["ValueRef"])
        for Ctrlcfg in Ctrlcfgs:
            if Ctrlcfg["controller_name"] == ctrlChnl:
                canif_transceiver_config["core_id"] = Ctrlcfg["core_id"]
                canif_transceiver_config["CanIfTrcvChnlToCanIfCtrlChnlRef"] = Ctrlcfg["ctrl_chnl_id"]
                break
    else:
        canif_transceiver_config["CanIfTrcvChnlToCanIfCtrlChnlRef"] = 0xff
    
    canif_transceiver_config["CanIfCanTrcvFctGrpId"] = 0
    return canif_transceiver_config