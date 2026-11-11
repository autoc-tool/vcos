# /*
#  * Copyright (c) 2025 Li Auto Inc. and its affiliates
#  * Licensed under the Apache License, Version 2.0(the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
#  */
"""
整理EthIf模块配置信息模块
"""

from func_module.health_monitor.logger import logger

def ethif_get_dict_by_definition_ref(input_dict: dict, definition_ref: str) -> dict:
    for value in input_dict.values():
        if isinstance(value, dict):
            if "DefinitionRef" in value.keys():
                if value["DefinitionRef"].split("/")[-1] == definition_ref:
                    return value
    return {}

def get_template_input_parameter_dict(config_dict: dict) -> dict:
    origin_config_dict = config_dict
    ethif_config_dict_raw = config_dict["ethif"]
    eth_config_dict_raw = {"ArPackages":{"ActiveEcuC":{"Eth":{}}}}

    ethif_general_dict = {}
    ethif_frameowner_config_list = []
    ethif_rxindication_config_list = []
    ethif_txconfirmation_config_list = []
    ethif_trcvlinkstatechg_config_list = []
    ethif_controller_list = []
    ethif_switch_list = []
    ethif_switchportgroup_list = []
    ethif_transceiver_list = []

    flag = False
    if "ArPackages" in ethif_config_dict_raw:
        if "ActiveEcuC" in ethif_config_dict_raw["ArPackages"]:
            if "EthIf" in ethif_config_dict_raw["ArPackages"]["ActiveEcuC"]:
                flag = True
    if flag == False:
        logger.warning("EthIf Config is not exist. VCOS Tools don't generate C code")
        return {}

    flag = False
    if "ArPackages" in eth_config_dict_raw:
        if "ActiveEcuC" in eth_config_dict_raw["ArPackages"]:
            if "Eth" in eth_config_dict_raw["ArPackages"]["ActiveEcuC"]:
                flag = True
    if flag == False:
        logger.warning("Eth Config is not exist. VCOS Tools don't generate C code")
        return {}


    ethmcal = origin_config_dict.get("ethmcal", {}).get("ArPackages", {}).get("ActiveEcuC", {}).get("EthMcal", {})
    ethmcal_enable_flag = "FALSE"
    ethmcal_version = ""
    if ethmcal != {}:
        ethmcal_enable_flag = "TRUE"
        if ethmcal["EthBasicConfig"]["EthMcalVersion"]["Value"] == "ETH_MCAL_AS422":
            ethmcal_version = "AS422"
        else:
            ethmcal_version = "AS440"

    ethif_config_dict = ethif_config_dict_raw["ArPackages"]["ActiveEcuC"]["EthIf"]
    eth_config_dict = eth_config_dict_raw["ArPackages"]["ActiveEcuC"]["Eth"]

    EthIfConfigSet_dict = ethif_get_dict_by_definition_ref(ethif_config_dict, "EthIfConfigSet")
    EthIfGeneral_dict = ethif_get_dict_by_definition_ref(ethif_config_dict, "EthIfGeneral")
    EthGeneral_dict = ethif_get_dict_by_definition_ref(eth_config_dict, "EthGeneral")


    ethif_general_dict = get_ethif_general(EthIfGeneral_dict)
    eth_general_dict = get_eth_general(EthGeneral_dict)

    for key, value in EthIfConfigSet_dict.items():
        if key == "DefinitionRef":
            continue
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfFrameOwnerConfig":
            cfg = get_ethif_frameowner_config(key, value)
            ethif_frameowner_config_list.append(cfg)
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfRxIndicationConfig":
            cfg = get_ethif_rxindication_config(key, value)
            ethif_rxindication_config_list.append(cfg)
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfTxConfirmationConfig":
            cfg = get_ethif_txconfirmation_config(key, value)
            ethif_txconfirmation_config_list.append(cfg)
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfTrcvLinkStateChgConfig":
            cfg = get_ethif_trcvlinkstatechg_config(key, value)
            ethif_trcvlinkstatechg_config_list.append(cfg)
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfController":
            controller = get_ethif_controller(key, value, origin_config_dict)
            ethif_controller_list.append(controller)
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfSwitch":
            switch = get_ethif_switch(key, value)
            ethif_switch_list.append(switch)
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfSwitchPortGroup":
            swtportgroup = get_ethif_swtportgroup(key, value, origin_config_dict)
            ethif_switchportgroup_list.append(swtportgroup)
        elif value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfTransceiver":
            trcv = get_ethif_trcv(key, value, origin_config_dict)
            ethif_transceiver_list.append(trcv)
    input_dict = {
        "EthIfGeneral": ethif_general_dict,
        "EthIfFrameOwnerConfigs": ethif_frameowner_config_list,
        "EthIfRxIndicationConfigs": ethif_rxindication_config_list,
        "EthIfTxConfirmationConfigs": ethif_txconfirmation_config_list,
        "EthIfTrcvLinkStateChgConfigs": ethif_trcvlinkstatechg_config_list,
        "EthIfControllers": ethif_controller_list,
        "EthIfSwitchs": ethif_switch_list,
        "EthIfSwitchPortGroups": ethif_switchportgroup_list,
        "EthIfTransceivers": ethif_transceiver_list,
        "EthGeneral": eth_general_dict,
        "ethmcal_enable_flag": ethmcal_enable_flag,
        "ethmcal_version": ethmcal_version,
    }
    for item_frame in input_dict["EthIfFrameOwnerConfigs"]:
        item_frame["EthIfRxIndicationHandle"] = "INVALID_ETHIF_HANDLE"
        item_frame["EthIfTxConfirmationHandle"] = "INVALID_ETHIF_HANDLE"
        for index, current_dict in enumerate(input_dict["EthIfRxIndicationConfigs"]):
            if current_dict["FrameOwnerRef"] == item_frame["ShortName"]:
                item_frame["EthIfRxIndicationHandle"] = str(index)
                break
        for index, current_dict in enumerate(input_dict["EthIfTxConfirmationConfigs"]):
            if current_dict["FrameOwnerRef"] == item_frame["ShortName"]:
                item_frame["EthIfTxConfirmationHandle"] = str(index)
                break

    return input_dict


def get_ethif_general(data: dict) -> dict:
    ethif_general_dict = {}
    ethif_general_dict["EthIfDevErrorDetect"] = change_boolean(data["EthIfDevErrorDetect"]["Value"])
    ethif_general_dict["EthIfEnableRxInterrupt"] = change_boolean(data["EthIfEnableRxInterrupt"]["Value"])
    ethif_general_dict["EthIfEnableTxInterrupt"] = change_boolean(data["EthIfEnableTxInterrupt"]["Value"])
    ethif_general_dict["EthIfGetBaudRate"] = change_boolean(data["EthIfGetBaudRate"]["Value"])
    ethif_general_dict["EthIfGetCounterState"] = change_boolean(data["EthIfGetCounterState"]["Value"])
    ethif_general_dict["EthIfGlobalTimeSupport"] = change_boolean(data["EthIfGlobalTimeSupport"]["Value"])
    if "EthIfMainFunctionPeriod" in data and data["EthIfMainFunctionPeriod"]["Value"] != None:
        ethif_general_dict["EthIfMainFunctionPeriod"] = data["EthIfMainFunctionPeriod"]["Value"]
    else:
        ethif_general_dict["EthIfMainFunctionPeriod"] = 10

    if "EthIfMainFunctionRxTimeout" in data and data["EthIfMainFunctionRxTimeout"]["Value"] != None:
        ethif_general_dict["EthIfMainFunctionRxTimeout"] = data["EthIfMainFunctionRxTimeout"]["Value"]
    else:
        ethif_general_dict["EthIfMainFunctionRxTimeout"] = 0

    if "EthIfMainFunctionStatePeriod" in data and data["EthIfMainFunctionStatePeriod"]["Value"] != None:
        ethif_general_dict["EthIfMainFunctionStateEnable"] = "STD_ON"
    else:
        ethif_general_dict["EthIfMainFunctionStateEnable"] = "STD_OFF"

    if "EthIfMaxTrcvsTotal" in data and data["EthIfMaxTrcvsTotal"]["Value"] != None:
        ethif_general_dict["EthIfMaxTrcvsTotal"] = data["EthIfMaxTrcvsTotal"]["Value"]
    else:
        ethif_general_dict["EthIfMaxTrcvsTotal"] = "1"

    if "EthIfMaxTxBufsTotal" in data and data["EthIfMaxTxBufsTotal"]["Value"] != None:
        ethif_general_dict["EthIfMaxTxBufsTotal"] = data["EthIfMaxTxBufsTotal"]["Value"]
    else:
        ethif_general_dict["EthIfMaxTxBufsTotal"] = ""

    ethif_general_dict["EthIfStartAutoNegotiation"] = change_boolean(data["EthIfStartAutoNegotiation"]["Value"])
    ethif_general_dict["EthIfTrcvLinkStateChgMainReload"] = data["EthIfTrcvLinkStateChgMainReload"]["Value"]
    ethif_general_dict["EthIfVersionInfoApi"] = change_boolean(data["EthIfVersionInfoApi"]["Value"])
    ethif_general_dict["EthIfVersionInfoApiMacro"] = change_boolean(data["EthIfVersionInfoApiMacro"]["Value"])
    ethif_general_dict["EthIfWakeUpSupport"] = change_boolean(data["EthIfWakeUpSupport"]["Value"])
    if "EthIfRxIndicationIterations" in data and data["EthIfRxIndicationIterations"]["Value"] != None:
        ethif_general_dict["EthIfRxIndicationIterations"] = data["EthIfRxIndicationIterations"]["Value"]
    else:
        ethif_general_dict["EthIfRxIndicationIterations"] = 1
    if "EthIfMapMcalApiPrefix" in data and data["EthIfMapMcalApiPrefix"]["Value"] != None:
        ethif_general_dict["EthIfMapMcalApiPrefix"] = data["EthIfMapMcalApiPrefix"]["Value"]
    else:
        ethif_general_dict["EthIfMapMcalApiPrefix"] = "Eth_17_GEthMac"
    if "EthIfPublicCddHeaderFile" in data and data["EthIfPublicCddHeaderFile"]["Value"] != None:
        if isinstance(data["EthIfPublicCddHeaderFile"]["Value"], str):
            # 如果value是单个字符串，将其转换为单个元素的列表
           data["EthIfPublicCddHeaderFile"]["Value"] = [data["EthIfPublicCddHeaderFile"]["Value"]]
        ethif_general_dict["EthIfPublicCddHeaderFile"] = data["EthIfPublicCddHeaderFile"]["Value"]
    if "EthIfEthSetCorrectionTimeApi" in data and data["EthIfEthSetCorrectionTimeApi"]["Value"] != None and data["EthIfEthSetCorrectionTimeApi"]["Value"] != '':
        ethif_general_dict["EthIfEthSetCorrectionTimeApi"] = data["EthIfEthSetCorrectionTimeApi"]["Value"]
    else:
        ethif_general_dict["EthIfEthSetCorrectionTimeApi"] = None
    if "EthIfEthSetGlobalTimeApi" in data and data["EthIfEthSetGlobalTimeApi"]["Value"] != None and data["EthIfEthSetGlobalTimeApi"]["Value"] != '':
        ethif_general_dict["EthIfEthSetGlobalTimeApi"] = data["EthIfEthSetGlobalTimeApi"]["Value"]
    else:
        ethif_general_dict["EthIfEthSetGlobalTimeApi"] = None
    if "EthIfTrcvGetLinkStateApi" in data and data["EthIfTrcvGetLinkStateApi"]["Value"] != None and data["EthIfTrcvGetLinkStateApi"]["Value"] != '':
        ethif_general_dict["EthIfTrcvGetLinkStateApi"] = data["EthIfTrcvGetLinkStateApi"]["Value"]
    else:
        ethif_general_dict["EthIfTrcvGetLinkStateApi"] = "ethif_ethtrcv_get_linkstate"
    if "EthIfSwtGetLinkStateApi" in data and data["EthIfSwtGetLinkStateApi"]["Value"] != None and data["EthIfSwtGetLinkStateApi"]["Value"] != '':
        ethif_general_dict["EthIfSwtGetLinkStateApi"] = data["EthIfSwtGetLinkStateApi"]["Value"]
    else:
        ethif_general_dict["EthIfSwtGetLinkStateApi"] = "EthIf_Swt_GetLinkState"
    return ethif_general_dict


def get_ethif_frameowner_config(name: str, data: dict) -> dict:
    ethif_frameowner_config_dict = {}
    ethif_frameowner_config_dict["ShortName"] = name
    if "EthIfFrameType" in data and data["EthIfFrameType"]["Value"] != None and data["EthIfFrameType"]["Value"] != "":
        ethif_frameowner_config_dict["EthIfFrameType"] = data["EthIfFrameType"]["Value"]
    else:
        ethif_frameowner_config_dict["EthIfFrameType"] = 0
    if "EthIfOwner" in data and data["EthIfOwner"]["Value"] != None and data["EthIfOwner"]["Value"] != "":
        ethif_frameowner_config_dict["EthIfOwner"] = data["EthIfOwner"]["Value"]
    else:
        ethif_frameowner_config_dict["EthIfOwner"] = 0
    return ethif_frameowner_config_dict


def get_ethif_rxindication_config(name: str, data: dict) -> dict:
    ethif_rxindication_config_dict = {}
    if "EthIfRxIndicationFunction" in data and data["EthIfRxIndicationFunction"]["Value"] != None:
        ethif_rxindication_config_dict["EthIfRxIndicationFunction"] = data["EthIfRxIndicationFunction"]["Value"]
    else:
        ethif_rxindication_config_dict["EthIfRxIndicationFunction"] = ""
    if "FrameOwnerRef" in data and data["FrameOwnerRef"]["ValueRef"] != None:
        ethif_rxindication_config_dict["FrameOwnerRef"] = get_name(data["FrameOwnerRef"]["ValueRef"])
    else:
        ethif_rxindication_config_dict["FrameOwnerRef"] = ""
    return ethif_rxindication_config_dict


def get_ethif_txconfirmation_config(name: str, data: dict) -> dict:
    ethif_txconfirmation_config_dict = {}
    if "EthIfTxConfirmationFunction" in data and data["EthIfTxConfirmationFunction"]["Value"] != None:
        ethif_txconfirmation_config_dict["EthIfTxConfirmationFunction"] = data["EthIfTxConfirmationFunction"]["Value"]
    else:
        ethif_txconfirmation_config_dict["EthIfTxConfirmationFunction"] = ""
    if "FrameOwnerRef" in data and data["FrameOwnerRef"]["ValueRef"] != None:
        ethif_txconfirmation_config_dict["FrameOwnerRef"] = get_name(data["FrameOwnerRef"]["ValueRef"])
    else:
        ethif_txconfirmation_config_dict["FrameOwnerRef"] = ""
    return ethif_txconfirmation_config_dict


def get_ethif_trcvlinkstatechg_config(name: str, data: dict) -> dict:
    ethif_trcvlinkstatechg_config_dict = {}
    if "EthIfTrcvLinkStateChgFunction" in data and data["EthIfTrcvLinkStateChgFunction"]["Value"] != None:
        ethif_trcvlinkstatechg_config_dict["EthIfTrcvLinkStateChgFunction"] = data["EthIfTrcvLinkStateChgFunction"][
            "Value"]
    else:
        ethif_trcvlinkstatechg_config_dict["EthIfTrcvLinkStateChgFunction"] = ""
    return ethif_trcvlinkstatechg_config_dict


def get_ethif_controller(name: str, data: dict, origin: dict) -> dict:
    ethif_controller_dict = {}
    ethif_controller_dict["ShortName"] = name
    ethif_controller_dict["EthIfCtrlIdx"] = data["EthIfCtrlIdx"]["Value"]
    ethif_controller_dict["EthIfCtrlMtu"] = data["EthIfCtrlMtu"]["Value"]
    ethif_controller_dict["EthIfLinkAggrThreshold"] = data["EthIfLinkAggrThreshold"]["Value"]

    EthIfEthCtrlIdx = 0
    if "EthIfPhysControllerRef" not in data or data["EthIfPhysControllerRef"][
            "ValueRef"] == "<empty>" or data["EthIfPhysControllerRef"]["ValueRef"] == "" or data["EthIfPhysControllerRef"]["ValueRef"] is None:
        ethif_controller_dict["EthIfPhysControllerRef"] = ""
    else:
        phycontroller_name = get_name(data["EthIfPhysControllerRef"]["ValueRef"])
        v = origin["ethif"]["ArPackages"]["ActiveEcuC"]["EthIf"]["EthIfConfigSet"][phycontroller_name]
        if "EthIfEthCtrlRef" in v and v["EthIfEthCtrlRef"]["ValueRef"] != None and v["EthIfEthCtrlRef"][
                "ValueRef"] != "<empty>" and v["EthIfEthCtrlRef"]["ValueRef"] != "":
            ethctrl_name = get_name(v["EthIfEthCtrlRef"]["ValueRef"])
            try:
                eth = origin["eth"]["ArPackages"]["ActiveEcuC"]["Eth"]["EthConfigSet"]
                for k1, v1 in eth.items():
                    if k1 == ethctrl_name and v1["DefinitionRef"] == "/AUTOSAR/EcucDefs/Eth/EthConfigSet/EthCtrlConfig":
                        if "EthCtrlIdx" in v1 and v1["EthCtrlIdx"]["Value"] != None:
                            EthIfEthCtrlIdx = int(v1["EthCtrlIdx"]["Value"])
            except KeyError:
                EthIfEthCtrlIdx = 0
    ethif_controller_dict["EthIfEthCtrlIdx"] = EthIfEthCtrlIdx


    EthIfEthTrcvIdx = "INVALID_TRCV_ID"
    if "EthIfEthTrcvRef" not in data or data["EthIfEthTrcvRef"][
            "ValueRef"] == "<empty>" or data["EthIfEthTrcvRef"][ "ValueRef"] == "" or data["EthIfEthTrcvRef"]["ValueRef"] is None:
        ethif_controller_dict["EthIfEthTrcvRef"] = ""
    else:
        phytrcv_name = get_name(data["EthIfEthTrcvRef"]["ValueRef"])
        v = origin["ethif"]["ArPackages"]["ActiveEcuC"]["EthIf"]["EthIfConfigSet"][phytrcv_name]
        if "EthIfEthTrcvRef" in v and v["EthIfEthTrcvRef"]["ValueRef"] != None and v["EthIfEthTrcvRef"][
                "ValueRef"] != "<empty>"  and v["EthIfEthTrcvRef"]["ValueRef"] != "":
            ethtrcv_name = get_name(v["EthIfEthTrcvRef"]["ValueRef"])
            try:
                eth = origin["ethtrcv"]["ArPackages"]["ActiveEcuC"]["EthTrcv"]["EthTrcvConfigSet"]
                for k1, v1 in eth.items():
                    if k1 == ethtrcv_name and v1["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthTrcv/EthTrcvConfigSet/EthTrcvConfig":
                        if "EthTrcvIdx" in v1 and v1["EthTrcvIdx"]["Value"] != None:
                            EthIfEthTrcvIdx = int(v1["EthTrcvIdx"]["Value"])
            except KeyError:
                EthIfEthTrcvIdx = "INVALID_TRCV_ID"
        else:
            EthIfEthTrcvIdx = v["EthIfTransceiverIdx"]["Value"]

    ethif_controller_dict["EthIfEthTrcvIdx"] = EthIfEthTrcvIdx

    EthIfSwitchPortGroupIdx = "INVALID_SWTGROUP_ID"
    if "EthIfSwitchRefOrPortGroupRef" not in data or data["EthIfSwitchRefOrPortGroupRef"][
            "ValueRef"] == "<empty>" or data["EthIfSwitchRefOrPortGroupRef"][
            "ValueRef"] == "" or data["EthIfSwitchRefOrPortGroupRef"]["ValueRef"] is None:
        ethif_controller_dict["EthIfSwitchRefOrPortGroupRef"] = ""
    else:
        EthIfSwitchRefOrPortGroup_name = get_name(data["EthIfSwitchRefOrPortGroupRef"]["ValueRef"])
        v = origin["ethif"]["ArPackages"]["ActiveEcuC"]["EthIf"]["EthIfConfigSet"][EthIfSwitchRefOrPortGroup_name]
        EthIfSwitchPortGroupIdx = v["EthIfSwitchPortGroupIdx"]["Value"]
    ethif_controller_dict["EthIfSwitchPortGroupIdx"] = EthIfSwitchPortGroupIdx

    if "EthIfMaxTxBufsTotal" not in data:
        ethif_controller_dict["EthIfMaxTxBufsTotal"] = 1
    else:
        ethif_controller_dict["EthIfMaxTxBufsTotal"] = data["EthIfMaxTxBufsTotal"]["Value"]

    if "EthIfVlanId" in data and data["EthIfVlanId"]["Value"] != None and data["EthIfVlanId"]["Value"] != "":
        ethif_controller_dict["EthIfVlanId"] = data["EthIfVlanId"]["Value"]
    else:
        ethif_controller_dict["EthIfVlanId"] = 0

    if "EthIfTransceiverRef" not in data or data["EthIfTransceiverRef"][
            "ValueRef"] == "<empty>" or data["EthIfTransceiverRef"][
            "ValueRef"] == "" or data["EthIfTransceiverRef"]["ValueRef"] is None:
        ethif_controller_dict["EthIfTransceiverRef"] = ""
    else:
        ethif_controller_dict["EthIfTransceiverRef"] = get_name(data["EthIfTransceiverRef"]["ValueRef"])
    return ethif_controller_dict


def get_ethif_switch(name: str, data: dict) -> dict:
    ethif_switch_dict = {}
    ethif_switch_dict["ShortName"] = name
    ethif_switch_dict["EthIfSwitchIdx"] = data["EthIfSwitchIdx"]["Value"]
    if "EthIfSwitchRef" not in data or data["EthIfSwitchRef"][
            "ValueRef"] == "<empty>" or data["EthIfSwitchRef"]["ValueRef"] == "" or data["EthIfSwitchRef"]["ValueRef"] is None:
        ethif_switch_dict["EthIfSwitchRef"] = ""
    else:
        ethif_switch_dict["EthIfSwitchRef"] = get_name(data["EthIfSwitchRef"]["ValueRef"])
    return ethif_switch_dict

def get_ethif_swtportgroup(name: str, data: dict, origin: dict) -> dict:
    ethif_swtportgroup_dict = {}
    ethif_swtportgroup_dict["ShortName"] = name
    ethif_swtportgroup_dict["EthIfSwitchPortGroupIdx"] = data["EthIfSwitchPortGroupIdx"]["Value"]
    ethif_swtportgroup_dict["EthIfSwitchPortlist"] = []
    if "EthIfPortRef" in data and data["EthIfPortRef"]["ValueRef"] != None and data["EthIfPortRef"][
                "ValueRef"] != "<empty>" and data["EthIfPortRef"]["ValueRef"] != "":
        swt = origin["ethswt"]["ArPackages"]["ActiveEcuC"]["EthSwt"]["EthSwtConfig"]
        for item in data["EthIfPortRef"]["ValueRef"]:
            ethifswtportdict = {}
            swtport_name = get_name(item)
            try:
                for k1, v1 in swt.items():
                    if k1 == swtport_name and v1["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthSwt/EthSwtConfig/EthSwtPort":
                        if "EthSwtPortIdx" in v1 and v1["EthSwtPortIdx"]["Value"] != None:
                            ethifswtportdict["EthIfSwtPortIdx"] = v1["EthSwtPortIdx"]["Value"]
                        if "EthSwtPortIdx" in v1 and v1["EthSwtPortIdx"]["Value"] != None:
                            ethifswtportdict["EthIfSwtPortIdx"] = v1["EthSwtPortIdx"]["Value"]
                        if "EthSwtPortRole" not in v1 or v1["EthSwtPortRole"]["Value"] == "<empty>" or v1["EthSwtPortRole"]["Value"] == "" or \
                            v1["EthSwtPortRole"]["Value"] is None:
                            ethifswtportdict["EthIfSwtPortRole"] = "ETHSWT_NO_ROLE_PORT"
                        else:
                            ethifswtportdict["EthIfSwtPortRole"] = v1["EthSwtPortRole"]["Value"]
                        ethifswtportdict["EthIfSwtIdx"] = 0
                        ethif_swtportgroup_dict["EthIfSwitchPortlist"].append(ethifswtportdict)
            except KeyError:
                ethif_swtportgroup_dict["EthIfSwitchPortlist"] = []
    else:
        for key, value in data.items():
            if key == "DefinitionRef":
                continue
            if value["DefinitionRef"] == "/AUTOSAR/EcucDefs/EthIf/EthIfConfigSet/EthIfSwitchPortGroup/EthIfSwtPort":
                ethifswtportdict = {}
                ethifswtportdict["EthIfSwtPortIdx"] = value["EthIfSwtPortIdx"]["Value"]
                ethifswtportdict["EthIfSwtIdx"] = value["EthIfSwtIdx"]["Value"]
                if "EthIfSwtPortRole" not in value or value["EthIfSwtPortRole"]["Value"] == "<empty>" or \
                    value["EthIfSwtPortRole"]["Value"] == "" or value["EthIfSwtPortRole"]["Value"] is None:
                    ethifswtportdict["EthIfSwtPortRole"] = "ETHSWT_NO_ROLE_PORT"
                else:
                    ethifswtportdict["EthIfSwtPortRole"] = value["EthIfSwtPortRole"]["Value"]
                ethif_swtportgroup_dict["EthIfSwitchPortlist"].append(ethifswtportdict)

    return ethif_swtportgroup_dict


def get_ethif_trcv(name: str, data: dict, origin: dict) -> dict:
    ethif_trcv_dict = {}
    ethif_trcv_dict["ShortName"] = name
    ethif_trcv_dict["EthIftrcvIdx"] = data["EthIfTransceiverIdx"]["Value"]
    if "EthIfEthTrcvRef" not in data or data["EthIfEthTrcvRef"][
            "ValueRef"] == "<empty>" or data["EthIfEthTrcvRef"]["ValueRef"] == "" or data["EthIfEthTrcvRef"]["ValueRef"] is None:
        ethif_trcv_dict["EthIftrcvRef"] = ""
    else:
        ethif_trcv_dict["EthIftrcvRef"] = get_name(data["EthIfEthTrcvRef"]["ValueRef"])
    return ethif_trcv_dict

# ETH 相关配置信息获取
def get_eth_general(data: dict) -> dict:
    eth_general_dict = {}
    eth_general_dict["EnableOffloadChecksumICMP"] = "STD_ON"
    eth_general_dict["EnableOffloadChecksumTCP"] = "STD_ON"
    eth_general_dict["EnableOffloadChecksumIPv4"] = "STD_ON"
    eth_general_dict["EnableOffloadChecksumUDP"] = "STD_ON"
    # TODO: Modify Eth related logic
    # eth_general_dict["EnableOffloadChecksumICMP"] = change_boolean(data["EthCtrlOffloading"]["EthCtrlEnableOffloadChecksumICMP"]["Value"])
    # eth_general_dict["EnableOffloadChecksumTCP"] = change_boolean(data["EthCtrlOffloading"]["EthCtrlEnableOffloadChecksumTCP"]["Value"])
    # eth_general_dict["EnableOffloadChecksumIPv4"] = change_boolean(data["EthCtrlOffloading"]["EthCtrlEnableOffloadChecksumIPv4"]["Value"])
    # eth_general_dict["EnableOffloadChecksumUDP"] = change_boolean(data["EthCtrlOffloading"]["EthCtrlEnableOffloadChecksumUDP"]["Value"])
    return eth_general_dict

def change_boolean(b: bool) -> str:
    if b == True:
        return "STD_ON"
    else:
        return "STD_OFF"


def get_name(key_name: str) -> str:
    key_name = key_name.split("/")[-1]
    return key_name
