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


class CanIfRule():

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

    def validate_diag_message_dlc_switch(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        for key in rx_pducfgs.keys():
            pdu = rx_pducfgs[key]
            if pdu["CanIfRxPduDataLengthCheck"]["Value"] == True:
                if pdu["CanIfRxPduUserRxIndicationUL"]["Value"] == "CAN_TP":
                    valid = False
                    self.log = self.log + "diag message must shutdown dlc check,key:" + key + "\n"
        return valid

    def validate_busoffname_busofful(self, canifcfg: dict) -> bool:
        valid = True
        canifdispatchcfgs = self.validate_utils_get_canifdispatch_cfgs(canifcfg)
        if canifdispatchcfgs["CanIfDispatchUserCtrlBusOffUL"]["Value"] == "CAN_SM":
            if canifdispatchcfgs["CanIfDispatchUserCtrlBusOffName"]["Value"] != "cansm_controller_bus_off":
                valid = False
                self.log = self.log + "for cansm CanIfDispatchUserCtrlBusOffName is fixed,user cann't modify it" + "\n"
        return valid

    def validate_CtrlModeIndicationName_CtrlModeIndicationUL(self, canifcfg: dict) -> bool:
        valid = True
        canifdispatchcfgs = self.validate_utils_get_canifdispatch_cfgs(canifcfg)
        if canifdispatchcfgs["CanIfDispatchUserCtrlModeIndicationUL"]["Value"] == "CAN_SM":
            if canifdispatchcfgs["CanIfDispatchUserCtrlModeIndicationName"]["Value"] != "cansm_controller_mode_indication":
                valid = False
                self.log = self.log + "for cansm CanIfDispatchUserCtrlModeIndicationName is fixed,user cann't modify it" + "\n"
        return valid

    def validate_txmessage_bufferref_outofbound(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        txbuffercfgs = self.validate_utils_get_txbuffer_cfgs(canifcfg)
        for key in tx_pducfgs.keys():
            pdu = tx_pducfgs[key]
            buffer_index = 0
            head, tail = os.path.split(pdu["CanIfTxPduBufferRef"]["ValueRef"])
            for key in txbuffercfgs.keys():
                if tail.split("_Tx")[0] == key.split("_Tx")[0]:
                    break
                buffer_index += 1
            if buffer_index >= len(txbuffercfgs):
                valid = False
                self.log = self.log + "CanIfTxPduBufferRef error" + key + "\n"
        return valid

    def validate_txbuffer_size(self, canifcfg: dict) -> bool:
        valid = True
        txbuffercfgs = self.validate_utils_get_txbuffer_cfgs(canifcfg)
        for key in txbuffercfgs.keys():
            if txbuffercfgs[key]["CanIfBufferSize"]["Value"] < 0:
                valid = False
                self.log = self.log + "CanIfBufferSize must bigger than zero" + key + "\n"
        return valid

    def validate_txmessage_bufferref_empty(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        for key in tx_pducfgs.keys():
            pdu = tx_pducfgs[key]
            if len(pdu["CanIfTxPduBufferRef"]["ValueRef"]) == 0 or pdu["CanIfTxPduBufferRef"]["ValueRef"] == "<empty>":
                valid = False
                self.log = self.log + "CanIfTxPduBufferRef is empty" + key + "\n"
        return valid

    def validate_txmessage_pduref_empty(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        for key in tx_pducfgs.keys():
            pdu = tx_pducfgs[key]
            if len(pdu["CanIfTxPduRef"]["ValueRef"]) == 0 or pdu["CanIfTxPduBufferRef"]["ValueRef"] == "<empty>":
                valid = False
                self.log = self.log + "invalid configuration CanIfTxPduRef is empty" + key + "\n"
        return valid

    def validate_rxmessage_HrhIdRef_empty(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        for key in rx_pducfgs.keys():
            pdu = rx_pducfgs[key]
            if len(pdu["CanIfRxPduHrhIdRef"]["ValueRef"]) == 0 or pdu["CanIfRxPduHrhIdRef"]["ValueRef"] == "<empty>":
                self.log = self.log + "CanIfRxPduHrhIdRef is empty" + key + "\n"
                valid = False
        return valid

    def validate_rxmessage_HrhIdRef_key_error(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        for key in rx_pducfgs.keys():
            pdu = rx_pducfgs[key]
            if len(pdu["CanIfRxPduHrhIdRef"]["ValueRef"]) == 0 or pdu["CanIfRxPduHrhIdRef"]["ValueRef"] == "<empty>":
                valid = False
            else:
                canifhohcfgs = self.validate_utils_get_hoh_cfgs(canifcfg)
                head, tail = os.path.split(pdu["CanIfRxPduHrhIdRef"]["ValueRef"])
                if tail in canifhohcfgs.keys():
                    valid = True
                else:
                    self.log = self.log + "CanIfRxPduHrhIdRef is error:" + key + "\n"
                    valid = False
                    break
        return valid

    def validate_txmessage_txpdu_bufferRef_key_error(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        for key in tx_pducfgs.keys():
            pdu = tx_pducfgs[key]
            if len(pdu["CanIfTxPduBufferRef"]["ValueRef"]) == 0 or pdu["CanIfTxPduBufferRef"]["ValueRef"] == "<empty>":
                valid = False
            else:
                canifhohcfgs = self.validate_utils_get_hoh_cfgs(canifcfg)
                head, tail = os.path.split(pdu["CanIfTxPduBufferRef"]["ValueRef"])
                if tail in canifhohcfgs:
                    valid = True
                else:
                    self.log = self.log + "CanIfTxPduBufferRef is error:" + key + "\n"
                    valid = False
                    break
        return valid

    def validate_rxmessage_length_type_matched(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        for key in rx_pducfgs.keys():
            pdu = rx_pducfgs[key]
            if pdu["CanIfRxPduCanIdType"]["Value"] == "STANDARD_CAN":
                if pdu["CanIfRxPduDataLength"]["Value"] > 8:
                    valid = False
                    self.log = self.log + "CanIfRxPduDataLength is error" + key + "\n"
        return valid

    def validate_rxmessage_id_type_matched(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        for key in rx_pducfgs.keys():
            pdu = rx_pducfgs[key]
            if pdu["CanIfRxPduCanIdType"]["Value"] == "STANDARD_CAN" or pdu["CanIfRxPduCanIdType"][
              "Value"] == "STANDARD_FD_CAN":
                if pdu["CanIfRxPduCanId"]["Value"] > 0x7ff:
                    valid = False
                    self.log = self.log + "CanIfRxPduCanId is error" + key + "\n"
        return valid

    def validate_rxmessage_id_valid(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        for key in rx_pducfgs.keys():
            pdu = rx_pducfgs[key]
            if pdu["CanIfRxPduCanId"]["Value"] < 0 or pdu["CanIfRxPduCanId"]["Value"] > 536870911:
                valid = False
                self.log = self.log + " CanIfRxPduCanId is error" + key + "\n"
                break
        return valid

    def validate_rxmessage_pduref_valid(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        for key in rx_pducfgs.keys():
            pdu = rx_pducfgs[key]
            if len(pdu["CanIfRxPduRef"]["ValueRef"]) == 0 or pdu["CanIfRxPduRef"]["ValueRef"] == "<empty>":
                valid = False
                self.log = self.log + " invalid configuration CanIfRxPduRef is error" + key + "\n"
                break
        return valid

    def validate_rxmessage_pduid_Duplicated(self, canifcfg: dict) -> bool:
        valid = True
        rx_pducfgs = self.validate_utils_get_rxpdus_cfgs(canifcfg)
        pduid_list = set()
        for key in rx_pducfgs.keys():
            pduid_list.add(rx_pducfgs[key]["CanIfRxPduId"]["Value"])
        if len(pduid_list) != len(rx_pducfgs):
            valid = False
            self.log = self.log + " CanIfRxPduId is Duplicated" + "\n"
        return valid

    def validate_txmessage_length_type_matched(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        for key in tx_pducfgs.keys():
            pdu = tx_pducfgs[key]
            if pdu["CanIfTxPduCanIdType"]["Value"] == "STANDARD_CAN":
                if pdu["CanIfTxPduDataLength"]["Value"] > 8:
                    valid = False
                    self.log = self.log + "CanIfTxPduDataLength is error" + key + "\n"
                    break
        return valid

    def validate_txmessage_id_type_matched(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        for key in tx_pducfgs.keys():
            pdu = tx_pducfgs[key]
            if pdu["CanIfTxPduCanIdType"]["Value"] == "STANDARD_CAN" or pdu["CanIfTxPduCanIdType"][
              "Value"] == "STANDARD_FD_CAN":
                if pdu["CanIfTxPduCanId"]["Value"] > 0x7ff:
                    valid = False
                    self.log = self.log + "CanIfTxPduCanId is error" + key + "\n"
                    break
        return valid

    def validate_txmessage_id_valid(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        for key in tx_pducfgs.keys():
            pdu = tx_pducfgs[key]
            if pdu["CanIfTxPduCanId"]["Value"] < 0 or pdu["CanIfTxPduCanId"]["Value"] > 536870911:
                valid = False
                self.log = self.log + " CanIfTxPduCanId is error" + key + "\n"
                break
        return valid

    def validate_txmessage_pduid_Duplicated(self, canifcfg: dict) -> bool:
        valid = True
        tx_pducfgs = self.validate_utils_get_txpdus_cfgs(canifcfg)
        pduid_list = set()
        for key in tx_pducfgs.keys():
            pduid_list.add(tx_pducfgs[key]["CanIfTxPduId"]["Value"])
        if len(pduid_list) != len(tx_pducfgs):
            valid = False
            self.log = self.log + " CanIfTxPduId is Duplicated" + "\n"
        return valid

    def validate_controllerRef_valid(self, canifcfg: dict, cancfg: dict) -> bool:
        valid = True
        CanIfCtrlDrvCfg = canifcfg["CanIfCtrlDrvCfg"]
        Ctrlcfgs = {}
        for key in CanIfCtrlDrvCfg.keys():
            if "CanIfCtrlId" in CanIfCtrlDrvCfg[key] and key != "init_CanIfCtrlCfg":
                Ctrlcfgs[key] = CanIfCtrlDrvCfg[key]
        for key in Ctrlcfgs.keys():
            head, tail = os.path.split(Ctrlcfgs[key]["CanIfCtrlCanCtrlRef"]["ValueRef"])
            if tail in cancfg["CanConfigSet"]:
                valid = True
            else:
                valid = False
                self.log = self.log + " CanIfCtrlCanCtrlRef is error" + key + "\n"
                break
        return valid

    def validate_controllerId_Duplicated(self, canifcfg: dict, cancfg: dict) -> bool:
        valid = True
        CanIfCtrlDrvCfg = canifcfg["CanIfCtrlDrvCfg"]
        Ctrlcfgs = {}
        for key in CanIfCtrlDrvCfg.keys():
            if "CanIfCtrlId" in CanIfCtrlDrvCfg[key] and key != "init_CanIfCtrlCfg":
                Ctrlcfgs[key] = CanIfCtrlDrvCfg[key]
        list_ctilId = set()
        for key in Ctrlcfgs.keys():
            list_ctilId.add(Ctrlcfgs[key]["CanIfCtrlId"]["Value"])
        if len(list_ctilId) != len(Ctrlcfgs):
            valid = False
            self.log = self.log + " CanIfCtrlId is Duplicated" + "\n"
        return valid

    def validate_controllerRef_Duplicated(self, canifcfg: dict, cancfg: dict) -> bool:
        valid = True
        CanIfCtrlDrvCfg = canifcfg["CanIfCtrlDrvCfg"]
        Ctrlcfgs = {}
        for key in CanIfCtrlDrvCfg.keys():
            if "CanIfCtrlId" in CanIfCtrlDrvCfg[key] and key != "init_CanIfCtrlCfg":
                Ctrlcfgs[key] = CanIfCtrlDrvCfg[key]
        list_ctilref = set()
        for key in Ctrlcfgs.keys():
            list_ctilref.add(Ctrlcfgs[key]["CanIfCtrlCanCtrlRef"]["ValueRef"])
        if len(list_ctilref) != len(Ctrlcfgs):
            valid = False
            self.log = self.log + " CanIfCtrlCanCtrlRef is Duplicated" + "\n"
        return valid

    def validate(self, ecu_cfg):
        valid = True
        canif_cfg = ecu_cfg["ActiveEcuC"]["CanIf"]
        can_cfg = ecu_cfg["ActiveEcuC"]["Can"]
        valid = self.validate_diag_message_dlc_switch(canif_cfg) and valid
        valid = self.validate_busoffname_busofful(canif_cfg) and valid
        valid = self.validate_CtrlModeIndicationName_CtrlModeIndicationUL(canif_cfg) and valid
        valid = self.validate_txmessage_bufferref_outofbound(canif_cfg) and valid
        valid = self.validate_txbuffer_size(canif_cfg) and valid
        valid = self.validate_txmessage_bufferref_empty(canif_cfg) and valid
        valid = self.validate_txmessage_pduref_empty(canif_cfg) and valid
        valid = self.validate_rxmessage_HrhIdRef_empty(canif_cfg) and valid
        valid = self.validate_rxmessage_length_type_matched(canif_cfg) and valid
        valid = self.validate_rxmessage_id_type_matched(canif_cfg) and valid
        valid = self.validate_txmessage_length_type_matched(canif_cfg) and valid
        valid = self.validate_txmessage_id_type_matched(canif_cfg) and valid
        valid = self.validate_rxmessage_HrhIdRef_key_error(canif_cfg) and valid
        valid = self.validate_txmessage_txpdu_bufferRef_key_error(canif_cfg) and valid
        valid = self.validate_txmessage_id_valid(canif_cfg) and valid
        valid = self.validate_txmessage_pduid_Duplicated(canif_cfg) and valid
        valid = self.validate_rxmessage_id_valid(canif_cfg) and valid
        valid = self.validate_rxmessage_pduref_valid(canif_cfg) and valid
        valid = self.validate_rxmessage_pduid_Duplicated(canif_cfg) and valid
        valid = self.validate_controllerRef_valid(canif_cfg, can_cfg) and valid
        valid = self.validate_controllerId_Duplicated(canif_cfg, can_cfg) and valid
        valid = self.validate_controllerRef_Duplicated(canif_cfg, can_cfg) and valid
        self.ecu_cfg = ecu_cfg

        # TODO: run validation funcs

        return valid
