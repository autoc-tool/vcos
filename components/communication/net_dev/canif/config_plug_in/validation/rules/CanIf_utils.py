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
import os

def checkIsNotNone(input):
    return (input is not None) and (input != "") and (input != "<empty>") and (input != "None")

class CanIfRuleUtils():

    def __init__(self) -> None:
        self.log = ""
        self.ecu_cfg = None

    def validate_utils_get_rxpdus_cfgs(self, config_dict: dict) -> dict:
        canifinitcfgs = config_dict["CanIfInitCfg"]
        rx_pducfgs = {}
        for key in canifinitcfgs.keys():
            if "CanIfRxPduCanId" in canifinitcfgs[key]:
                rx_pducfgs[key] = canifinitcfgs[key]
        return rx_pducfgs

    def validate_utils_get_canifdispatch_cfgs(self, config_dict: dict) -> dict:
        CanIfDispatchCfg = config_dict["CanIfDispatchCfg"]
        return CanIfDispatchCfg

    def validate_utils_get_canifpublic_cfgs(self, config_dict: dict) -> dict:
        CanIfPublicCfg = config_dict["CanIfPublicCfg"]
        return CanIfPublicCfg

    def validate_utils_get_txpdus_cfgs(self, config_dict: dict) -> dict:
        canifinitcfgs = config_dict["CanIfInitCfg"]
        tx_pducfgs = {}
        for key in canifinitcfgs.keys():
            if "CanIfTxPduCanId" in canifinitcfgs[key]:
                tx_pducfgs[key] = canifinitcfgs[key]
        return tx_pducfgs

    def validate_utils_get_txbuffer_cfgs(self, config_dict: dict) -> dict:
        canifinitcfgs = config_dict["CanIfInitCfg"]
        txbuffercfgs = {}
        for key in canifinitcfgs.keys():
            if "CanIfBufferSize" in canifinitcfgs[key]:
                txbuffercfgs[key] = canifinitcfgs[key]
        return txbuffercfgs

    def validate_utils_get_hoh_cfgs(self, config_dict: dict) -> dict:
        canifhohcfgs = config_dict["CanIfInitCfg"]["CanIfInitHohCfg"]
        return canifhohcfgs
    
    def validate_utils_get_canif_hrh_cfgs(self, config_dict: dict) -> dict:
        canifhrhcfgs = {}
        canifinithohcfgs = config_dict["CanIfInitCfg"]["CanIfInitHohCfg"]
        for k, v  in canifinithohcfgs.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfInitHohCfg/CanIfHrhCfg':
                canifhrhcfgs[k] = v
        return canifhrhcfgs
    
    def validate_utils_get_canif_hth_cfgs(self, config_dict: dict) -> dict:
        canifhthcfgs = {}
        canifinithohcfgs = config_dict["CanIfInitCfg"]["CanIfInitHohCfg"]
        for k, v  in canifinithohcfgs.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanIf/CanIfInitCfg/CanIfInitHohCfg/CanIfHthCfg':
                canifhthcfgs[k] = v
        return canifhthcfgs
    
    def validate_utils_get_can_hrh_cfgs(self, config_dict: dict) -> dict:
        canhrhcfgs = {}
        canconfigset = config_dict["CanConfigSet"]
        for k, v in canconfigset.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"].endswith("Can/CanConfigSet/CanHardwareObject"):
                if v["CanObjectType"]["Value"] == 'RECEIVE':
                    canhrhcfgs[k] = v
        return canhrhcfgs
    
    def validate_utils_get_can_hth_cfgs(self, config_dict: dict) -> dict:
        canhthcfgs = {}
        canconfigset = config_dict["CanConfigSet"]
        for k, v in canconfigset.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"].endswith("Can/CanConfigSet/CanHardwareObject"):
                if v["CanObjectType"]["Value"] == 'TRANSMIT':
                    canhthcfgs[k] = v
        return canhthcfgs
    
    def validate_utils_get_can_ho_id_list(self, config_dict: dict) -> dict:
        ho_name_list = []
        ho_id_list = []
        ho_name_id_map = {}
        canconfigset = config_dict["CanConfigSet"]
        for k, v in canconfigset.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"].endswith("Can/CanConfigSet/CanHardwareObject"):
                ho_id_list.append(int(v["CanObjectId"]["Value"]))
                ho_name_list.append(k)
                if ho_name_id_map.get(int(v["CanObjectId"]["Value"])) is None:
                    ho_name_id_map[int(v["CanObjectId"]["Value"])] = k
                else:
                    tmp = ho_name_id_map[int(v["CanObjectId"]["Value"])]
                    if not isinstance(tmp, list):
                        tmp = [tmp, k]
                    else:
                        tmp.append(k)
                    ho_name_id_map[int(v["CanObjectId"]["Value"])] = tmp
                    
        ho_id_list = sorted(ho_id_list, reverse=False)
        return ho_id_list, ho_name_id_map
    
    def validate_utils_get_can_ho_list(self, config_dict: dict) -> dict:
        can_ho_dict = {}
        canconfigset = config_dict["CanConfigSet"]
        for k, v in canconfigset.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"].endswith("Can/CanConfigSet/CanHardwareObject"):
                can_ho_dict[k] = v
                    
        return can_ho_dict

    def validate_utils_get_CanController_list(self, config_dict: dict) -> dict:
        CanController_dict = {}
        canconfigset = config_dict["CanConfigSet"]
        for k, v in canconfigset.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"].endswith("Can/CanConfigSet/CanController"):
                CanController_dict[k] = v
        return CanController_dict
    
    def validate_utils_get_CanIfCtrlCfg_list(self, config_dict: dict) -> dict:
        CanIfCtrlCfg_dict = {}
        CanIfCtrlCfgs = config_dict["CanIfCtrlDrvCfg"]
        for k, v  in CanIfCtrlCfgs.items():
            if k == "DefinitionRef":
                continue
            if v["DefinitionRef"] == '/AUTOSAR/EcucDefs/CanIf/CanIfCtrlDrvCfg/CanIfCtrlCfg':
                CanIfCtrlCfg_dict[k] = v
        return CanIfCtrlCfg_dict